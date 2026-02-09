/*
 * pong.c - UDP ping/pong server code
 *          author: <your name>
 */
#include <arpa/inet.h>
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
  int ch;
  int nping = 1;                   // default packet count
  char *pongport = strdup(PORTNO); // default port

  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    default:
      fprintf(stderr, "usage: pong [-n #pings] [-p port]\n");
    }
  }
  int sockfd;
  int rv;
  int numbytes;
  struct sockaddr_storage their_addr;
  socklen_t addr_len;
  struct addrinfo hints, *servinfo, *p;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  rv = getaddrinfo(NULL, pongport, &hints, &servinfo);
  if (rv != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("listener: socket");
      continue;
    }
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("listener: bind");
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "listener: failed to bind socket\n");
    return 2;
  }

  freeaddrinfo(servinfo);
  char arr[65535]; // largest udp packet size
  char s[INET_ADDRSTRLEN];

  addr_len = sizeof their_addr;
  //if ((numbytes = recvfrom(sockfd, arr, 65535 - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
  //  perror("recvfrom");
  //  exit(1);
  //}
  for (int i = 0; i < nping; i++) {
    if ((numbytes = recvfrom(sockfd, arr, 65535 - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
      perror("recvfrom");
      exit(1);
    }
    printf("pong[%d]: recieved packet from %s\n", i,
           inet_ntop(AF_INET, &(((struct sockaddr_in *)&their_addr)->sin_addr), s, sizeof s));
    for (int j = 0; j < numbytes; j++) {
      arr[j] += 1;
    }
    sendto(sockfd, arr, numbytes, 0, (struct sockaddr *)&their_addr, addr_len);
  }


  close(sockfd);
  // pong implementation goes here.
  printf("nping: %d pongport: %s\n", nping, pongport);

  return 0;
}
