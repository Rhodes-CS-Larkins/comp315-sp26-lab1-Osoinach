/*
 * ping.c - UDP ping/pong client code
 *          author: <your name>
 */
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

#define PORTNO "1266"

int main(int argc, char **argv) {
  int ch, errors = 0;
  int nping = 1;                        // default packet count
  char *ponghost = strdup("localhost"); // default host
  char *pongport = strdup(PORTNO);      // default port
  int arraysize = 100;                  // default packet size

  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'h':
      ponghost = strdup(optarg);
      break;
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    case 's':
      arraysize = atoi(optarg);
      break;
    default:
      fprintf(stderr,
              "usage: ping [-h host] [-n #pings] [-p port] [-s size]\n");
    }
  }

  // UDP ping implemenation goes here
  //
  unsigned char *arr = malloc(arraysize * sizeof(char));
  if (arr == NULL) {
    perror("malloc");
    exit(1);
  }
  memset(arr, 200, arraysize);

  int sockfd;
  int rv;
  struct addrinfo hints, *servinfo, *p;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  rv = getaddrinfo(ponghost, pongport, &hints, &servinfo);
  if (rv != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("talker: socket");
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "talker: failed to create socket\n");
    return 2;
  }

  double t = 0;
  int e = 0;
  unsigned char *rec = malloc(arraysize * sizeof(char));
  if (rec == NULL) {
    perror("malloc");
    exit(1);
  }
  for (int i = 0; i < nping; i++) {
    memset(rec, 200, arraysize);
    memset(arr, 200, arraysize);
    double t1 = get_wctime();
    sendto(sockfd, arr, arraysize, 0, p->ai_addr, p->ai_addrlen);
    recvfrom(sockfd, rec, arraysize, 0, p->ai_addr, &p->ai_addrlen);
    double t2 = get_wctime();
    t += t2 - t1;
    for (int j = 0; j < arraysize; j++) {
      if ((unsigned char)rec[j] != 201) {
        fprintf(stderr, "expected 201, recieved %d at index %d\n", rec[j], j);
        e += 1;
        break;
      }
    }
    memset(rec, 200, arraysize);
    memset(arr, 200, arraysize);
    printf("ping[%d]: round-trip time: %f ms\n", i, (t2 - t1) * 1000);
  }
  if (e > 0) {
    printf("encountered %d errors\n", e);
  } else {
    printf("no errors detected\n");
  }

  printf("time to send %d packages of %d bytes %f ms (%f avg per packet)\n",
         nping, arraysize, t * 1000, (t / nping) * 1000);

  freeaddrinfo(servinfo);

  close(sockfd);
  free(rec);
  free(arr);
  printf("nping: %d arraysize: %d errors: %d ponghost: %s pongport: %s\n",
         nping, arraysize, errors, ponghost, pongport);

  return 0;
}
