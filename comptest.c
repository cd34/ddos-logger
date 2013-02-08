#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "ddoslog.h"
#include "lzfx-0.1/lzfx.h"

int main(void) {
    char buf[16384];
    struct ddos_log payload;
    u_int payload_len;
    char * ibuf[16384];
    unsigned int ibuf_len = sizeof(ibuf);
    char * obuf[16384];
    unsigned int obuf_len = sizeof(obuf);
    int rc;

    strcpy(buf, "This is a test of the compression system");
    payload_len = strlen(buf);

    rc = lzfx_compress(&buf, payload_len, obuf, &obuf_len);
    printf("rc: %d obuf_len: %d\n", rc, obuf_len);

    rc = lzfx_decompress(obuf, obuf_len, ibuf, &ibuf_len);
    printf("ibuf: %s\n", ibuf);

}
