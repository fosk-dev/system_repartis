#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "message.h"
#include <sys/stat.h>
#include <fcntl.h>

int csd;
int sd;
static int rfd;
static int wfd;
static int do_create(struct message *m1, struct message *m2);
static int do_read(struct message *m1, struct message *m2);
static int do_write(struct message *m1, struct message *m2);
static int do_delete(struct message *m1, struct message *m2);

static int initialize(char *server_addr, char *server_port);
static int release(void);


int main(int argc, char *argv[]){
  struct message m1, m2; /*incomming and outgoing message*/
  int r;   /* result code*/
  if(argc != 3){
    fprintf(stderr, "USAGE: %s <server_addr> <server_port>\n", argv[0]);
    return 0;
  }
  memset(&m1,0, sizeof(m1)); 
  memset(&m2,0, sizeof(m2));              
  initialize(argv[1], argv[2]);
  while(TRUE){
    struct sockaddr client_addr;
    int clen;
    if((csd = accept(sd, &client_addr, &clen)) < 0){
      fprintf(stderr, "Un petit problème lors du accept %d\n", errno);
      return -1;
    }
    pid_t pid = fork();
    if(pid == 0){
      close(sd);
      rfd = -1;
      wfd = -1;
      fprintf(stderr, "get connection from %d\n", csd);
      while(TRUE){           /*server runs forever*/
        ifri_receive(csd, &m1); /* block waiting for a message*/
        switch(m1.opcode){
        /*case CREATE: 
          r = do_create(&m1, &m2);
          break;*/
          case READ:
            r = do_read(&m1, &m2);
            break;
          case WRITE:
            r = do_write(&m1, &m2);
            break;
          /*case DELETE:
            r = do_delete(&m1, &m2);
            break;*/
          default:
            r = E_BAD_OPCODE;
        }
        m2.result = r;  /* return result to client */
        if(r > 0)
          m2.count = r;
          ifri_send(csd, &m2); /* send reply*/
      }
      break;
    }else{
      close(csd);
    }
  }
  release();
}

static int do_create(struct message *m1, struct message *m2){
  return OK;
}

static int do_read(struct message *m1, struct message *m2){
  int fd, r;
  if((rfd == -1) && (rfd = open(m1->name, O_RDONLY | S_IRUSR | S_IWUSR ))<0){
    fprintf(stderr, "error when opening file %d %d %s\n",fd, errno, m1->name);
    return E_IO;
  }
  lseek(rfd, m1->offset, SEEK_SET);
  r = read(rfd, m2->data, m1->count);
  if(r < 0){
    fprintf(stderr, "error when reading file %d %d\n",fd, errno);
    return  E_IO;
  }
  return r;
}

static int do_write(struct message *m1, struct message *m2){
  int r;
  if( (wfd == -1) && (wfd = open(m1->name, O_CREAT | O_WRONLY | S_IRWXU )) < 0){
    fprintf(stderr, "do_write error when opening (%s), %d:%d\n", m1->name, wfd, errno);
    return E_IO;
  }
  lseek(wfd, m1->offset, SEEK_SET);
  r = write(wfd, m1->data, m1->count);
  if(r < 0)
     return  E_IO;
  return r;
}

static int do_delete(struct message *m1, struct message *m2){
  return OK;
}


static int initialize(char *server_ipaddr, char *server_port){
   struct sockaddr server_addr;
   int salen;
   sd = socket(AF_INET, SOCK_STREAM, 0);
   if(resolve_address(&server_addr, &salen, server_ipaddr, server_port, AF_INET, 
      SOCK_STREAM, IPPROTO_TCP)!= 0){
      fprintf(stderr, "Erreur de configuration de sockaddr\n");
      return -1;
   }
    if(bind(sd, &server_addr, salen)!=0){
      fprintf(stderr, "Un petit problème lors du bind: %d\n", errno);
      return -1;
    }
    if(listen(sd, 10)!=0){
      fprintf(stderr, "Un petit problème lors du listen %d\n", errno);
      return -1;
    }
    fprintf(stderr, "Connexion acceptée %d \n", csd);   
}

static int release(void){
  return (close(sd) || close(csd) || close(rfd) || close(wfd));
}
