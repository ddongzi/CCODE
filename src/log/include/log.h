#ifndef LOG_LOG_H
#define LOG_LOG_H

#include <stdio.h>
#include <time.h>

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4

// 默认日志级别为 INFO
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

// 定义日志级别对应的字符串表示
#define LOG_LEVEL_STR(level) \
    ((level == LOG_LEVEL_ERROR) ? "ERROR" : \
    (level == LOG_LEVEL_WARNING) ? "WARNING" : \
    (level == LOG_LEVEL_INFO) ? "INFO" : \
    (level == LOG_LEVEL_DEBUG) ? "DEBUG" : "UNKNOWN")

// 获取当前时间的宏
#define CURRENT_TIME(buffer) \
    do { \
        time_t t = time(NULL); \
        struct tm tm_info = *localtime(&t); \
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", &tm_info); \
    } while(0)

// 获取当前文件名的宏
#define CURRENT_FILE __FILE__

// 定义日志输出函数
#define LOG(level, format, ...) \
    do { \
        if (level <= LOG_LEVEL) { \
            char time_buffer[26]; \
            CURRENT_TIME(time_buffer); \
            printf("[%s] [%s:%d] [%s] " format " \n", time_buffer, CURRENT_FILE, __LINE__, LOG_LEVEL_STR(level), ##__VA_ARGS__); \
        } \
    } while (0)

// 定义不同级别的日志输出宏
#define LOG_ERROR(format, ...) LOG(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) LOG(LOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) LOG(LOG_LEVEL_INFO, format, ##__VA_ARGS__)

#define LOG_DEBUG(format, ...) LOG(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)

#endif // LOG_LOG_H
