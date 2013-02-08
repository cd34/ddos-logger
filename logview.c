#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nids.h"
#include "ddoslog.h"
#include "lzfx-0.1/lzfx.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

FILE *fp;
#define int_ntoa(x)     inet_ntoa(*((struct in_addr *)&x))

int main (int argc, char *argv[]) {
  struct ddos_log header;
  char log_header[1024];
  char * in_buffer[16384];
  char * obuf[16384];
  unsigned int obuf_len = sizeof(obuf);
  int rc;
  char timebuf[64];
  struct tm* tm_info;
  char source_ip[64];
  char dest_ip[64];
  char filename[256];

  if (argc > 1) {
    strncpy(filename, argv[1], sizeof(filename));
  } else { // if (argc > 1)
    printf("Usage:\n%s logfilename\n\n", argv[0]);
    exit(1);
  } // if (argc > 1)

  fp = fopen(filename, "rb");
  if (fp) {
    // this will eventually hold a version header
    fgets(log_header, sizeof(log_header), fp);
    while (1) {
      fread(&header, sizeof(header), 1, fp);
      fread(&in_buffer, header.payload_len, 1, fp);       
      if (!feof(fp)) {
        memset(obuf, 0, sizeof(obuf));
        obuf_len = sizeof(obuf);
        rc = lzfx_decompress(in_buffer, header.payload_len, obuf, &obuf_len);
        tm_info = localtime(&header.time);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);
        strcpy(source_ip, int_ntoa(header.source_ip));
        strcpy(dest_ip, int_ntoa(header.dest_ip));
        printf("%s %s:%d %s:%d %s %s", timebuf,
          source_ip, header.source_port, dest_ip, header.dest_port,
          header.country, obuf);
      } else { // if (!feof(fp))
        break;
      } // if (!feof(fp))
    } // while (1)
  } else { // if (fp)
    printf("Didn't find a valid file\n\n");
  } // if (fp)
  return 0;
}
