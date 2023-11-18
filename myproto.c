#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <time.h>
#include "myproto.h"

int myproto_decode(const void *buf, size_t len, struct myproto_hdr *hdr)
{
    if (len < sizeof(struct myproto_hdr))
        return -1;

    memcpy(hdr, buf, sizeof(struct myproto_hdr));
    hdr->timestamp = ntohl(hdr->timestamp);
    return 0;
}

int myproto_encode(void *buf, size_t len, const struct myproto_hdr *hdr)
{
    if (len < sizeof(struct myproto_hdr))
        return -1;

    memcpy(buf, hdr, sizeof(struct myproto_hdr));
    ((struct myproto_hdr *)buf)->timestamp = htonl(hdr->timestamp);
    return 0;
}

void _get_sys_flag(char *flag)
{
    struct utsname uts;
    uname(&uts);
    sprintf(flag, "%s %s", uts.nodename, uts.release);
}
long long _get_milliseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void _format_timestamp(long long timestamp, char *buffer, size_t buffer_size) {
    time_t seconds = timestamp / 1000;
    struct tm tm_info;

    localtime_r(&seconds, &tm_info);

    char _buffer[100];
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &tm_info);
    sprintf(_buffer, "%s:%d", buffer, (int)(timestamp % 1000));
    strcpy(buffer, _buffer);
}

int _millisecond_time_delta(long long t1, long long t2)
{
    return (int)(t1 - t2);
}

int systime_delta(long long t) {
    return _millisecond_time_delta(_get_milliseconds(), t);
}

void request(struct myproto_hdr *hdr, const char *msg)
{
    hdr->type = REQUEST;
    hdr->timestamp = _get_milliseconds();
    _get_sys_flag(hdr->flag);
    strcpy(hdr->msg, msg);
}

void response(struct myproto_hdr *hdr, const char *msg)
{
    hdr->type = RESPONSE;
    hdr->timestamp = _get_milliseconds();
    _get_sys_flag(hdr->flag);
    strcpy(hdr->msg, msg);
}

proto_hdr* create_request(const char *msg)
{
    proto_hdr *hdr = malloc(sizeof(proto_hdr));
    request(hdr, msg);
    return hdr;
}

proto_hdr* create_response(const char *msg)
{
    proto_hdr *hdr = malloc(sizeof(proto_hdr));
    response(hdr, msg);
    return hdr;
}


void print_proto_info(const proto_hdr obj)
{
    char _buffer[50];
    char formatted_time[100];
    _format_timestamp(obj.timestamp, _buffer, sizeof(_buffer));
    sprintf(formatted_time, "%s (%lld)", _buffer, obj.timestamp);

    printf("+-------+----------------------------------------------------+\n");
    printf("| %-5s | %-50s |\n", "Field", "Value");
    printf("+-------+----------------------------------------------------+\n");
    printf("| %-5s | %-50d |\n", "TYP", obj.type);
    printf("| %-5s | %-50s |\n", "TIM", formatted_time);
    printf("| %-5s | %-50s |\n", "FLG", obj.flag);
    printf("| %-5s | %-50s |\n", "MSG", obj.msg);
    printf("+-------+----------------------------------------------------+\n");
    return;
}