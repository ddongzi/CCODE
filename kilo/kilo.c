#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>

// / * defines */
#define KILO_VERSION "0.0.1"

#define KILO_TAB_STOP 4
#define KILO_QUIT_TIMES 2
// 将普通按键转化为 CTRL组合。
// example: A 是0x41, 组合后为 0x01, 对应CTRL-A
#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKey
{
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
};

/*** data ***/
// 表示一行
typedef struct erow
{
    int size; // 不包含'\0', strlen长度
    char *chars;

    // 渲染后的行。  如渲染TAB为多个空格
    int rsize;
    char *render;
} erow_t;

struct editorConfig
{
    int cx, cy;    // 对应文件内cursor 位置。  从0开始, 行列需要+1
    int rowoff;    // scroll滚动偏移
    int columnoff; // scroll
    int screenrows;
    int screencols;
    struct termios orig_termios; // original termios

    char *filename;

    int numrows;
    erow_t *rows; // 行动态数组

    char statusmsg[80]; // 状态信息： 供用户search等
    time_t statusmsg_time;

    int dirty; // 文件和编辑器内容是否一致
};

struct editorConfig E; // editor全局状态

/*** prototypes ***/
void editor_set_status_msg(const char *fmt, ...);

/*** 行row操作 ***/

// 通过实际chars 渲染出 render
void editor_update_row(erow_t *row)
{
    int j;
    int tabs = 0;
    for (j = 0; j < row->size; j++)
    {
        if (row->chars[j] == '\t')
            tabs++;
    }

    free(row->render);
    row->render = malloc(row->size + tabs * (KILO_TAB_STOP - 1) + 1);

    int idx = 0;
    for (j = 0; j < row->size; j++)
    {
        if (row->chars[j] == '\t')
        {
            row->render[idx++] = ' ';
            while (idx % KILO_TAB_STOP != 0)
                row->render[idx++] = ' ';
        }
        else
        {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;
}

// 在at位置行插入新的一行
void editor_insert_row(int at, char *s, size_t len)
{
    if (at < 0 || at > E.numrows)
        return;
    E.rows = realloc(E.rows, (E.numrows + 1) * sizeof(erow_t));
    memmove(&E.rows[at + 1], &E.rows[at], sizeof(erow_t) * (E.numrows - at));

    E.rows[at].size = len;
    E.rows[at].chars = malloc(len + 1);
    memcpy(E.rows[at].chars, s, len);
    E.rows[at].chars[len] = '\0';

    E.rows[at].rsize = 0;
    E.rows[at].render = NULL;
    editor_update_row(&E.rows[at]);
    E.numrows++;
    E.dirty++;
}

// 在一个row中at位置插入字符c
void editor_row_insert_char(erow_t *row, int at, int c)
{
    if (at < 0 || at > row->size)
        at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->chars[at] = c;
    row->size++;
    editor_update_row(row);

    E.dirty++;
}

// 转化为string,
char *editor_row_to_string(int *buf_len)
{
    int totlen = 0;
    int j;
    for (j = 0; j < E.numrows; j++)
    {
        totlen += E.rows[j].size + 1;
    }
    *buf_len = totlen;
    char *buf = malloc(totlen);
    char *p = buf;
    for (j = 0; j < E.numrows; j++)
    {
        memcpy(p, E.rows[j].chars, E.rows[j].size);
        p += E.rows[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}

// 在row中at位置删除字符
void editor_row_del_char(erow_t *row, int at)
{
    if (at < 0 || at > row->size)
        return;
    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
    editor_update_row(row);
    E.dirty++;
}

// 释放一行
void editor_free_row(erow_t *row)
{
    free(row->chars);
    free(row->render);
}

// 删除一行
void editor_del_row(int at)
{
    if (at < 0 || at >= E.numrows)
        return;
    editor_free_row(&E.rows[at]);
    memmove(&E.rows[at], &E.rows[at + 1], sizeof(erow_t) * (E.numrows - at - 1));
    E.numrows--;
    E.dirty++;
}

// 行append 一个s
void editor_row_append_string(erow_t *row, const char *s, int len)
{
    // 之前内容不变
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    editor_update_row(row);
    E.dirty++;
}

// enter按键 换行，拆分换行
void editor_enter_newline()
{
    if (E.cx == 0)
    {
        editor_insert_row(E.cy, "", 0);
    }
    else
    {
        erow_t *row = &E.rows[E.cy];
        editor_insert_row(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.rows[E.cy];
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editor_update_row(row);
    }
    E.cy++;
    E.cx = 0;
}

/*** 分区：terminal ***/
// 错误处理
void die(char *message)
{
    // 清空屏幕
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(message);
    exit(1);
}
void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCIFLUSH, &E.orig_termios) == -1)
    {
        die("tcsetattr");
    }
}
void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = E.orig_termios;
    // turn off IXON : Ctrl-s,Ctrl-Q,
    // turn off ICRNL : Ctrl-M as 10 byte , but expected is 13. so turn off.  It equals Enter.
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // 1.  turn off echo feature: The ECHO feature causes each key you type to be printed to the terminal, so you can see what you’re typing. Such as sudo password example.
    // 2.  turn off canonical mode: The canonical mode is the default mode for reading input. we will finally be reading input byte-by-byte, instead of line-by-line.
    // 3.  turn off Ctrl-c SIGINT, Ctrl-Z SIGSTP
    // 4. turn off Ctrl-V

    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    // turn off OPOST: DEFAULT, '\n' normally translates into '\r\n'. but now, we only need cursor move down, not to left side
    raw.c_oflag &= ~(OPOST);

    // set read wait time. If timeout, read() returns null.
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 2;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

// 从stdin等待一个字符，返回
int editorReadKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        // 读取异常处理
        /* code */
        if (nread == -1 && errno != EAGAIN)
        {
            die("read");
        }
    }
    if (c == '\x1b')
    {
        // 特殊按键
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return '\x1b';

        if (seq[0] == '[')
        {
            // [3~
            if (seq[1] >= '0' && seq[1] <= '9')
            {
                //
                if (read(STDIN_FILENO, &seq[2], 1) != 1)
                    return '\x1b';
                if (seq[2] == '~')
                {
                    switch (seq[1])
                    {

                    case '3':
                        return DEL_KEY;
                    }
                }
            }
            else
            {
                // ：箭头 \x1b[A
                switch (seq[1])
                {
                case 'A':
                    return ARROW_UP;
                case 'B':
                    return ARROW_DOWN;
                case 'C':
                    return ARROW_RIGHT;
                case 'D':
                    return ARROW_LEFT;
                }
            }
        }

        return '\x1b';
    }
    return c;
}
// 获取光标位置
int getCursorPosition(int *rows, int *columns)
{

    char buf[32];
    unsigned int i = 0;

    // N cmd 查询终端状态，6返回光标状态, reply 是一个 esc序列 如 [24;80R
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;
    printf("\r\n");
    while (i < sizeof(buf) - 1)
    {
        /* code */
        if (read(STDOUT_FILENO, &buf[i], 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';

    printf("\r\n&buf[1]: '%s'\r\n ", &buf[1]);

    if (buf[0] != '\x1b' || buf[1] != '[')
        return -1;
    if (sscanf(&buf[2], "%d;%d", rows, columns) != 2)
        return -1;
    return 0;
}

// 获取window尺寸. 如果失败返回-1， 成功返回0，数据存在参数
// ioctl有的系统不可以使用， 则通过光标检测屏幕尺寸
int getWindowSize(int *rows, int *columns)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        return getCursorPosition(rows, columns);
    }
    else
    {
        *columns = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*** editor operations ***/
void editor_insert_char(int c)
{
    if (E.cy == E.numrows)
    {
        editor_insert_row(E.numrows, "", 0);
    }
    editor_row_insert_char(&E.rows[E.cy], E.cx, c);
    E.cx++;
}

void editor_del_char()
{
    if (E.cy == E.numrows)
        return; // 超出文件下部
    if (E.cx == 0 && E.cy == 0)
        return; // 文件开头
    erow_t *row = &E.rows[E.cy];
    if (E.cx > 0)
    {
        editor_row_del_char(row, E.cx);
    }
    else
    {
        // 行首 删除
        E.cx = E.rows[E.cy - 1].size;
        editor_row_append_string(&E.rows[E.cy - 1], row->chars, row->size);
        editor_del_row(E.cy);
        E.cy--;
    }
}

/* append buffer */
// buffer一定时候会刷新
struct abuf
{
    char *b;
    int len;
};
// 空buf
#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len)
{
    char *new = realloc(ab->b, ab->len + len);
    if (new == NULL)
        return; // out of memory
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}
void abFree(struct abuf *ab)
{
    free(ab->b);
    ab->b = NULL;
    ab->len = 0;
}

/*** file i/o ***/
void editorOpen(char *filename)
{
    free(E.filename);
    E.filename = strdup(filename);

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
        die("fopen");

    char *line = NULL;
    ssize_t line_len = 13;
    size_t line_capacity = 0;
    while ((line_len = getline(&line, &line_capacity, fp)) != -1)
    {
        // 去除每行的\r\n
        while (line_len > 0 && (line[line_len - 1] == '\n' ||
                                line[line_len - 1] == '\r'))
            line_len--;
        editor_insert_row(E.numrows, line, line_len);
    }

    free(line);
    fclose(fp);
    E.dirty = 0;
}
void editor_save()
{
    if (E.filename == NULL)
        return;

    int len;
    char *buf = editor_row_to_string(&len);

    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1)
    {
        if (ftruncate(fd, len) != -1)
        {
            if (write(fd, buf, len) == len)
            {
                close(fd);
                free(buf);
                editor_set_status_msg("%d bytes written to disk", len);
                E.dirty = 0;
                return;
            }
        }
        close(fd);
    }
    free(buf);
    editor_set_status_msg("Can't save. IO error: %s", strerror(errno));
}

/* input : 更高level 处理 */

// 按键移动光标
// 确保会先捕获 ESC (27)，然后读取 '['，然后才是这里
void editorMoveCursor(int key)
{
    erow_t *row = (E.cy >= E.numrows) ? NULL : &E.rows[E.cy];
    switch (key)
    {
    case ARROW_LEFT:
        if (E.cx != 0)
        {
            E.cx--;
        }
        break;
    case ARROW_RIGHT:
        // 空余光标位置
        if (row && E.cx < row->size)
            E.cx++;
        break;
    case ARROW_UP:
        if (E.cy != 0)
        {
            E.cy--;
        }
        break;
    case ARROW_DOWN:
        if (E.cy < E.numrows)
        {
            E.cy++;
        }
        break;
    }
    row = (E.cy >= E.numrows) ? NULL : &E.rows[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen)
    {
        E.cx = rowlen;
    }
}

// 读入字符并处理
void editorProcessKeypress()
{
    // 变量程序运行时一直存在，，
    static int quit_times = KILO_QUIT_TIMES;
    int c = editorReadKey();
    switch (c)
    {
    case '\r': // ENTER
        editor_enter_newline();
        break;
    case CTRL_KEY('q'):
        if (E.dirty && quit_times > 0)
        {
            editor_set_status_msg("WARNING!!! File has unsaved changes. "
                                  "Press Ctrl-Q %d more times to quit.",
                                  quit_times);
            quit_times--;
            break;
            ;
        }
        // 清空屏幕后推出
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        printf("Exiting...\r\n");
        exit(0);
    case CTRL_KEY('s'):
        editor_save();
        break;
    case BACKSPACE:
        // 删除光标左侧内容，光标左移
        editorMoveCursor(ARROW_LEFT);
        editor_del_char();
        break;
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_UP:
    case ARROW_RIGHT:
        editorMoveCursor(c);
        break;
    default:
        editor_insert_char(c);
        break;
    }
    quit_times = KILO_QUIT_TIMES;
}

/* output */

// 根据光标位置，更新偏移量
void editorScroll()
{
    if (E.cy < E.rowoff)
    {
        // 光标已经超过屏幕上方, 设置偏移量为光标行
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows)
    {
        // 光标行在屏幕下方，设置偏移量为..， 向上滚动，使光标行在最下面
        E.rowoff = E.cy - E.screenrows + 1;
    }
    // 横向scroll
    if (E.cx < E.columnoff)
    {
        E.columnoff = E.cx;
    }
    if (E.cx >= E.columnoff + E.screencols)
    {
        E.columnoff = E.cx - E.screencols + 1;
    }
}
void editorDrawRows(struct abuf *ab)
{
    // 在行首添加~.
    // 暂定是24行，
    int y;
    for (y = 0; y < E.screenrows; y++)
    {
        int filerow = y + E.rowoff;
        if (y >= E.numrows)
        {
            if (E.numrows == 0 && y == E.screenrows / 3)
            {
                /* code */
                char welcome[80];
                int welcome_len = snprintf(welcome, sizeof(welcome), "Kilo editor --- version %s", KILO_VERSION);
                if (welcome_len > E.screenrows)
                {
                    welcome_len = E.screencols;
                }
                abAppend(ab, welcome, welcome_len);
            }
            else
            {
                abAppend(ab, "~", 1);
            }
        }
        else
        {
            char temp[10] = "";
            int linenumber_len = snprintf(temp, sizeof(temp), "%3d ", filerow + 1);
            abAppend(ab, temp, linenumber_len);

            int len = E.rows[filerow].rsize - E.columnoff;
            if (len < 0)
                len = 0;
            if (len > E.screencols)
                len = E.screencols;

            abAppend(ab, &E.rows[filerow].render[E.columnoff], len);
        }

        abAppend(ab, "\x1b[K", 3); // 清除从光标位置到行尾的所有字符
        abAppend(ab, "\r\n", 2);   // 光标移到下一行开头
    }
}

// 状态栏绘制
void editor_draw_status_bar(struct abuf *ab)
{
    // 进入反显模式，
    abAppend(ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
                       E.filename ? E.filename : "[No Name]",
                       E.numrows,
                       E.dirty ? "(modified)" : "");
    if (len > E.screencols)
        len = E.screencols;
    abAppend(ab, status, len);

    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.numrows);
    while (len < E.screencols)
    {
        if (E.screencols - len == rlen)
        {
            abAppend(ab, rstatus, rlen);
            break;
        }
        else
        {
            /* code */
            abAppend(ab, " ", 1);
            len++;
        }
    }
    // 恢复正常模式
    abAppend(ab, "\x1b[m", 3);
    abAppend(ab, "\r\n", 2);
}

// 用户栏设置
void editor_draw_message_bar(struct abuf *ab)
{
    abAppend(ab, "\x1b[K", 3);
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screencols)
        msglen = E.screencols;
    // 如果下次按键超过5s,就会消失
    if (msglen && time(NULL) - E.statusmsg_time < 5)
    {
        abAppend(ab, E.statusmsg, msglen);
    }
}

// 刷新clear屏幕
void editorRefreshScreen()
{
    editorScroll();

    struct abuf ab = ABUF_INIT;
    abAppend(&ab, "\x1b[?25l", 6); // 刷新时候隐藏光标
    // \x1b 是一个字节 （escape）， [2J是3个字节
    abAppend(&ab, "\x1b[H", 3); // 光标移动到开头

    editorDrawRows(&ab);
    editor_draw_status_bar(&ab);
    editor_draw_message_bar(&ab);

    // 移动光标位置 到 原来位置
    char buf[32];
    // 行号3长度
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", ((E.cy - E.rowoff) + 1), ((E.cx - E.columnoff) + 1 + 4));
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6); // 刷新完成后显示光标
    write(STDIN_FILENO, ab.b, ab.len);

    abFree(&ab);
}

void editor_set_status_msg(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}

/* init */
void initEditor()
{
    E.cx = 0;
    E.cy = 0;
    E.numrows = 0;
    E.rows = NULL;
    E.rowoff = 0;
    E.columnoff = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.dirty = 0;

    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
    {
        die("getWindowSize");
    }
    E.screenrows -= 2; // 给状态栏空间

    write(STDOUT_FILENO, "\x1b[?7l", 5); // 禁用自动换行
}

int main(int argc, char *argv[])
{
    enableRawMode();
    initEditor();
    if (argc >= 2)
    {
        editorOpen(argv[1]);
    }
    editor_set_status_msg("HELP: Ctrl-S = save | Ctrl-Q = quit");
    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
        // other logic...  e.g. move cursor, edit text, etc.  (This is just a simple example)
    }

    return 0;
}