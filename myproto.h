#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>

enum TYPE {
    REQUEST  = 1 << 0,
    RESPONSE = 1 << 1,
};

typedef struct myproto_hdr
{
    uint8_t     type;      // 1: request,
                           // 2: response
    long long   timestamp; // milliseconds
    char        flag[256]; // system flag
    char        msg[256];  // message
} proto_hdr;

int myproto_decode(const void *buf, size_t len, struct myproto_hdr *hdr);
int myproto_encode(void *buf, size_t len, const struct myproto_hdr *hdr);

void request(struct myproto_hdr *hdr, const char *msg);
void response(struct myproto_hdr *hdr, const char *msg);
proto_hdr* create_request(const char *msg);
proto_hdr* create_response(const char *msg);

void print_proto_info(const proto_hdr);
int systime_delta(long long t);
