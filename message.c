#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "message.h"

#define SL sizeof(long)

int ifri_receive(int from, struct message *m){
  int ret;
  char *buff = malloc(sizeof(struct message));
  while((ret = recv(from, buff, sizeof(struct message), 0)) < 0 && errno ==EINTR);
  if(ret < 0)
    return (E_IO);
  memset(m, 0, sizeof(*m));
  memcpy(&m->source,   buff, SL);
  memcpy(&m->dest,     buff+SL, SL);
  memcpy(&m->opcode,   buff+SL+SL, SL);
  memcpy(&m->count,    buff+SL+SL+SL, SL);
  memcpy(&m->offset,   buff+SL+SL+SL+SL, SL);
  memcpy(&m->result,   buff+SL+SL+SL+SL+SL, SL);
  memcpy(&m->name_len, buff+SL+SL+SL+SL+SL+SL, SL);
  memcpy(&m->name,     buff+SL+SL+SL+SL+SL+SL+SL, m->name_len);
  memcpy(&m->data,     buff+SL+SL+SL+SL+SL+SL+SL+ m->name_len, m->count);
  return OK;
}
int ifri_send(int to, struct message *m){
  char *buff = malloc(sizeof(struct message));
  memcpy(buff, &m->source, SL);
  memcpy(buff+SL, &m->dest, SL);
  memcpy(buff+SL+SL, &m->opcode, SL);
  memcpy(buff+SL+SL+SL, &m->count, SL);
  memcpy(buff+SL+SL+SL+SL, &m->offset, SL);
  memcpy(buff+SL+SL+SL+SL+SL, &m->result, SL);
  memcpy(buff+SL+SL+SL+SL+SL+SL, &m->name_len, SL);
  memcpy(buff+SL+SL+SL+SL+SL+SL+SL, m->name, m->name_len);
  memcpy(buff+SL+SL+SL+SL+SL+SL+SL+m->name_len, m->data, m->count);
  int ret = send(to, buff, sizeof(struct message),0);
  return OK;
}

int resolve_address(struct sockaddr *sa, socklen_t *salen, const char *host, const char *port, int family, int type, int proto){
  struct addrinfo hints, *res;
  int err;
  memset(&hints,0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_socktype = type;
  hints.ai_protocol = proto;
  hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV | AI_PASSIVE;
  if((err = getaddrinfo(host, port, &hints, &res)) !=0 || res == NULL){
     fprintf(stderr, "failed to resolve address :%s:%s\n", host, port);
     return -1;
  }
  memcpy(sa, res->ai_addr, res->ai_addrlen);
  *salen = res->ai_addrlen;
  freeaddrinfo(res);
  return 0;
}
