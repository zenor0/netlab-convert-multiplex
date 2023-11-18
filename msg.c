#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

#define INFO_TYPE 0
#define WARNING_TYPE 1
#define ERROR_TYPE 2
#define DEBUG_TYPE 3

void show_msg(int type, const char *fmt, ...)
{
    time_t current_time = time(NULL);
    struct tm *tm = localtime(&current_time);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm);

    char *prefix;
    if (type == INFO_TYPE)
    {
        prefix = "\033[32m[INFO] ";
    } else if (type ==  WARNING_TYPE) {
        prefix = "\033[33m[WARNING] ";
    } else if (type == ERROR_TYPE) {
        prefix = "\033[31m[ERROR] ";
    } else if (type == DEBUG_TYPE) {
        prefix = "\033[34m[DEBUG] ";
    }

    printf("%s | %13s%s", time_str, prefix, "\033[0m");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    return;
}
