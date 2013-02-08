#include <GeoIP.h>
#include <libio.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include "nids.h"
#include "ddoslog.h"
#include "lzfx-0.1/lzfx.h"

char log_version_string[] = "#lzfx-0.1 - ddoslog.c v0.9\n";

FILE *fp;
char file_prefix[256];
char file_name[256];
char file_suffix[16];
int log_daynum;

// geoip replace unknown country code with --
static const char * _mk_NA( const char * p ){
 return p ? p : "--";
}
GeoIP * gi;

// log EST, CLOSE, RESET, DATA

void rotate_logfile(void) {
  time_t t0 = time(0);
  struct tm * logdate = localtime(&t0);

  if (log_daynum != logdate->tm_mday) {
    log_daynum = logdate->tm_mday;
    strftime(file_suffix, sizeof(file_suffix), ".%Y%m%d", logdate);
    if (fp) {
      fclose(fp);
    }
    strcpy(file_name, file_prefix);
    strcat(file_name, file_suffix);
    fp = fopen(file_name, "ab+");
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0) {
      fprintf(fp, log_version_string);
    } // if (ftell(fp) == 0)
  }
}

void tcp_callback (struct tcp_stream *a_tcp, void **junk) {
  char buf[16384];
  struct ddos_log payload;
  u_int payload_len;
  char * obuf[16384];
  unsigned int obuf_len = sizeof(obuf);
  int rc;
  time_t t0 = time(0);
  struct tm * logdate = localtime(&t0);

  if (log_daynum != logdate->tm_mday) {
    rotate_logfile();
  } // if (log_daynum != logdate->tm_mday)

    payload.time = t0;

// need to handle port 22, 443, other ssl ports
// right now, handle only web and smtp for testing
    if ( (a_tcp->addr.dest!=80) && (a_tcp->addr.dest!=25) ) return;

// short circuits
    if (a_tcp->nids_state == NIDS_CLOSE) {
      return;
    }
    if (a_tcp->nids_state == NIDS_RESET) {
      return;
    }

// track it
    if (a_tcp->nids_state == NIDS_JUST_EST) {
      a_tcp->server.collect++;
      return;
    }

// we've got data
    if (a_tcp->nids_state == NIDS_DATA) {
      payload.time = t0;
      payload.source_ip = a_tcp->addr.saddr;
      payload.source_port = a_tcp->addr.source;
      payload.dest_ip = a_tcp->addr.daddr;
      payload.dest_port = a_tcp->addr.dest;

      struct half_stream *hlf;

      if (!a_tcp->client.count_new) {
	  hlf = &a_tcp->server;
      }
      strcpy(payload.country, 
             _mk_NA(GeoIP_country_code_by_ipnum(gi, payload.source_ip)));
      payload_len = hlf->count_new;
      if (payload_len > sizeof(buf)) {
        payload_len = sizeof(buf) - 2;
      } // if (payload_len > sizeof(buf))
      memcpy(buf, hlf->data, payload_len);
      if (buf[payload_len] != '\n') {
        strcpy(buf + payload_len - 4, "\n\n");
      } // if (buf[payload_len] != '\n')
      // lzf compress buf
      rc = lzfx_compress(&buf, payload_len, obuf, &obuf_len);
      payload.payload_len = obuf_len;
      fwrite(&payload, sizeof(payload), 1, fp);
      fwrite(obuf, obuf_len, 1, fp);
      fflush(fp);
    }
  return;
}

int main (int argc, char *argv[]) {
  int loop;

  if (argc > 1) {
    strcpy(file_prefix, argv[1]);
  } else { // if (argc > 1)
    printf("Usage:\n%s /var/log/path/fileprefix [-i eth0]\n\n", argv[0]);
    exit(1);
  } // if (argc > 1)
  for (loop=2;loop<argc;loop++) {
    if (strcmp(argv[loop], "-i")) {
      if (loop+1 <= argc) {
        nids_params.device = argv[loop];
      } // if (loop+1 <= argc)
    } // if (argv[loop] == "-i")
  }

  // open logfile
  rotate_logfile();

  // nids_params.n_hosts=256;
  if (!nids_init ()) {
    fprintf(stderr,"%s\n", nids_errbuf);
    exit(1);
  }
  gi = GeoIP_new(GEOIP_MEMORY_CACHE);

  /* disable checksum checking for all packets */
  struct nids_chksum_ctl nochksumchk;
  nochksumchk.netaddr = 0;
  nochksumchk.mask = 0;
  nochksumchk.action = NIDS_DONT_CHKSUM;
  nids_register_chksum_ctl(&nochksumchk, 1);

  nids_register_tcp (tcp_callback);
  nids_run ();

  return 0;
}
