#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 限制6位数
// 16 312 ---> SIXTEEN THOUSAND THREE HUNDRED TWELVE
// 流式处理过滤
void written_amount(unsigned int amount, char *buffer)
{
    const char *ones[] = { "", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE" };
    const char *teens[] = { "TEN", "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN", "SIXTEEN", "SEVENTEEN", "EIGHTEEN", "NINETEEN" };
    const char *tens[] = { "", "", "TWENTY", "THIRTY", "FORTY", "FIFTY", "SIXTY", "SEVENTY", "EIGHTY", "NINETY" };
    const char *thousands[] = { "", "THOUSAND" };

    char buf[1024] = "\0";
    if (amount == 0) {
        sprintf(buf, "ZERO");
        return;
    }
    if (amount > 1000) {
        int d = amount / 1000;
        if (d > 100) {
            sprintf(buf + strlen(buf), "%s HUNDRED ", ones[d / 100]);
            d = d % 100;
        }
        if (d >= 20) {
            sprintf(buf + strlen(buf), "%s ", tens[d / 10]);
            d = d % 10;
        } else if (d >= 10) {
            sprintf(buf + strlen(buf), "%s ", teens[d % 10]);
            d = 0;
        } else {
            sprintf(buf + strlen(buf), "%s ", ones[d % 10]);
            d = 0;
        }
        if (d != 0) {
            sprintf(buf + strlen(buf), "%s ", ones[d % 10]);
        }
        sprintf(buf + strlen(buf), "THOUSAND ");

        amount = amount % 1000;
    }
    if (amount > 100) {
        sprintf(buf + strlen(buf), "%s HUNDRED ", ones[amount / 100]);
        amount = amount % 100;
    }

    if (amount > 20) {
        sprintf(buf + strlen(buf), "%s ",  tens[amount / 10]);
        amount = amount % 10;
    } else if (amount > 10) {
        sprintf(buf + strlen(buf), "%s ",  teens[amount - 10]);
        amount = 0;
    }

    if (amount > 0) {
        sprintf(buf + strlen(buf), "%s ",  ones[amount]);
    }

    strcpy(buffer, buf);
}

// 矩阵乘法
// 【 2 -6 】   【 4 2 -4 5 】          【 50 14 -44 -52 】
// 【 3  5 】 * 【 -7 -3 6 7】      =   【 
// 【 1 -1 】                                           】
// 
void matrix_multiply(int *m1, int *m2, int *r, int x, int y, int z)
{
    // 行列 计算 r位置元素
    for (size_t i = 0; i < x; i++) {
        for (size_t j = 0; j < z; j++) {
            
            int r_val = 0;
            int r_loc = i*x + j;

            for (size_t k = 0; k < y; k++) {
                /* code */
                int m1_loc = i * y + k;
                int m2_loc = k * z + j;
                printf(" m1[%d][%d](%d) * m2[%d][%d](%d) | ", i, k, *(m1 + m1_loc),k, j,  *(m2 + m2_loc));
                r_val += (*(m1 + m1_loc)) * (*(m2 + m2_loc));
            }
            *(r + r_loc) = r_val;
            printf("r[%d][%d] : %d\n", i, j, r_val);
        }
    }
    
}

// 多维数组内部一维展开 访问。
//      lo1,hi1, lo2,hi2,  lo3,hi3, buf0,buf1,buf2
// 【3, 4,6 ,    1,5,      -3,3】， s1,  s2, s3
//                                   4, 1, 3
int array_offset(int arrayinfo[], ...)
{
    int loc = 0;
    int buf[arrayinfo[0]];
    int buflen = 0;
    va_list list;
    va_start(list, arrayinfo);
    for (size_t wd = 0; wd < arrayinfo[0]; wd++) {
        buf[buflen++] = va_arg(list, int);
    }
    va_end(list);
    
    if (arrayinfo[0] == 3) {
        // 行主序存储
        loc += ((buf[0] - arrayinfo[1]) * (arrayinfo[4] - arrayinfo[3] + 1) + buf[1] - arrayinfo[3]) * 
            (arrayinfo[6] - arrayinfo[5] + 1)
            + buf[2] - arrayinfo[5]
            ;
    }
    printf("%d\n", loc);    

    return 0;
}
