#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



// 处理stn-函数未以NUL字节结尾的字符串
size_t my_strlen(char const *s, size_t len)
{

}

// 统计字符
void stat_ctype(char const *s)
{
    int counts[7];
    memset(counts, 0, sizeof(int) * 7);
    for (char c = *s; *s != '\0'; s++)
    {
        /* code */
        if (iscntrl(c)) counts[0] += 1;
        if (isspace(c)) counts[1] += 1;
        if (isdigit(c)) counts[2] += 1;
        if (islower(c)) counts[3] += 1;
        if (isupper(c)) counts[4] += 1;
        if (ispunct(c)) counts[5] += 1;
        if (!isprint(c)) counts[6] += 1;
    }
    printf("cntrl : %d, space : %d, alpha : %d, lower : %d," 
        "upper : %d, punct : %d, !print : %d\n",
        counts[0], counts[1], counts[2], counts[3], counts[4], counts[5], counts[6]);
    
}

bool remove_dup(char *str)
{
    char *temp = str;
    int hash[256] = {0};
    int index = 0;
    for (char c = *temp; *temp != '\0'; temp++, c = *temp) {
        if (!isalpha(c)) return false;
        if (hash[(unsigned int) c] == 0) {
            // 没有出现过。
            hash[(unsigned int) c] = 1;
            str[index++] = c;
        }
    }
    str[index] = '\0';
    return true;
}

int prepare_key(char *key)
{
    if (key == NULL) return false;
    size_t len = 27;
    char *s = key;
    for (char  c = *s; *s != '\0'; s++) {
        *s = toupper(*s);
    }
    remove_dup(key);

    for (size_t i = strlen(key); i < len; i++) {
        // 开始扩充
        for (char c = 'A'; c <= 'Z'; c++) {
            // 如果不存在key中就可以填进去
            if (strchr(key, c) == NULL) {
                key[i] = c;
                break;
            }
        }
    }
    key[len - 1] = '\0';
    printf("key :%s\n", key);
}

void encrypt(char *data, char const *key)
{
    char *flag = data;
    for (char c = *data; *data != '\0'; data++, c = *data) {
        if (isalpha(c)) {
            /* 从key中寻找替代 */
            *data = key[(unsigned int)c - 0x41] ; 
        }
    }
    data = flag;
    printf("encrypt %s\n", data);
    
}

void decrypt(char *data, char const *key)
{
    char *flag = data;
    for (char d = *data; *data != '\0'; data++, d = *data) {
        for (size_t i = 0; i < strlen(key); i++) {
            if (d == key[i]) {
                *data = 'A' + i;
            }
        }
    }
    data = flag;
    printf("decrypt %s\n", data);
}

// 1 --> 0.01
// 12---> 0.12
// 123 ---> 1.23
// 12345 ---> 123,45
// 123456 ---> 1,234,56
// 112345678 ---> 1,123, 456.78
void dollars(char *dest, char const *src)
{
    char *flag = dest;
    size_t len = strlen(src);
    char buf[] = "$0.00";
    char stack[1024];
    memset(stack, '\0', 1024);
    char *sp = stack;
    if (len < 3) {
        strcpy(buf + 2 + 3 - len, src);
        strcpy(dest, buf);
        printf("%s\n", dest);
    } else {
        for (int i = len - 1; i >= 0; i--) {
            if (( len - 1 -i) == 2) {
                // 插入小数点
                *sp++ = '.';
            } 
            if ( (len - 1 - i) > 3 && (len - 1 - i + 1) % 3 == 0) {
                *sp++ = ',';
            }
            *sp++ = src[i];
        }
        *sp = '$';
        do {
            *dest++ = *sp;
            sp--;
        } while (sp != stack);
        *dest++ = *sp;
        dest = flag;
        printf("%s\n", dest);
    }

}

// 安装format_string格式 将digit修改到format_string， 从右到左代替
// 格式字符串：
//      # 正常替代
//      , 左边至少有一个数字，就不修改，否则空白代替
//      .  不动， 如果左边没数字，就在左边填一个； 右边填充0至有效数字
// 
int format(char *format_string, char const *digit_string)
{
    const char *dp = digit_string + strlen(digit_string) - 1; // 指向末尾'\0'
    char *fp = format_string + strlen(format_string) - 1; // 指向末尾'\0'

    while (fp >= format_string) {
        /* code */
        if (*fp == '#') {
            if (dp >= digit_string) {
                *fp-- = *dp--;
            } else {
                *fp-- = ' ';
                dp--;
            }
        } else if (*fp == ',') {
            if (isdigit(*dp)) {
                // 留下，
                fp--;
                *fp-- = *dp--;
            } else {
                *fp = ' ';    
                fp--;            
            }
        } else if (*fp == '.') {
            fp--;
            if (dp < digit_string) {
                {
                    // 左边扩充0
                    *fp = '0';
                    fp--;
                }
                {
                    // 右边填0
                    memset(fp + 3, '0', ( digit_string - dp - 1) * sizeof(char));
                }
            } else {
                // 保留数字
                *fp-- = *dp--;
            }
        }

    }
    printf("%s\n", format_string);
}

struct A {
    int a;
} x;

int main(int argc, char const *argv[])
{
    char format_str[100] ;
    char digit_str[100] ;
    memset(format_str, '\0', 100);
    memset(digit_str, '\0', 100);
    scanf("%s %s", format_str, digit_str);

    format(format_str, digit_str);

    // char des[1024];
    // memset(des, '\0', 1024);
    // char src[1024] ;
    // memset(src, '\0', 1024);
    // scanf("%s", src);
    // dollars(des, src);

    // char c = 'A';
    // printf("%u 0x%x %c\n",c,c, (char)(0x41));

    // char key[27] = "trailblazers";
    // prepare_key(key);
    // char data[] = "ATTACK AT DAWN";
    // encrypt(data, key);
    // decrypt(data, key);
    // char buf[2048];
    // scanf("%s", buf);
    // stat_ctype(buf);
    /* code */
    return 0;
}
