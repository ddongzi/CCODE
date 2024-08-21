#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

/*** data ***/

struct termios orig_termios; // original termios

/*** terminal ***/
void die(char *message)
{
    perror(message);
    exit(1);
}
void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCIFLUSH, &orig_termios) == -1)
    {
        die("tcsetattr");
    }
}
void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = orig_termios;
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

int main()
{
    enableRawMode();

    while (1)
    {
        char c = '\0';
        // If timeout, read() returns 0 without setting the c variable.
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        if (iscntrl(c))
        {
            // ASCII 0-31, 127 are control characters. Such as Escape,Home,End,Enter,Ctrl-A..
            // Specially, Ctrl-s, Ctrl-q.Ctrl-z, Ctrl-C
            printf("%d\r\n", c);
        }
        else
        {
            // From now , we will use '\r\n' whenever we want to start a new line.
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q')
        {
            break;
        }
    }

    return 0;
}