#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/resource.h>
#include "message.h"
#include <fcntl.h>

int sd;
static int copy(char *src, char *dst);
static int initialize(char *server_addr, char *server_port);
static int release(void);

static int copy(char *src, char *dst){
  struct message m1;
  long position;
  long client = 110;
  
  
  position = 0;
   int d_f = open(dst, O_CREAT | O_WRONLY | S_IRWXU ) ;
  do{
    memset(&m1, 0, sizeof(m1));
    m1.opcode = READ;
    m1.offset = position;
    m1.count = BUF_SIZE;
    m1.name_len = strlen(src);
    strncpy(m1.name, src, m1.name_len);
    ifri_send(sd, &m1);
    ifri_receive(sd, &m1);
    
    /*write the data just received to the destination file*/
       
    lseek(d_f, position, SEEK_SET);
    int    r = write(d_f, m1.data, m1.count);
    if( r < 0) {
    	fprintf(stderr, "erreur d ecriture dans le fichier de destination %d",errno);
        close(d_f);
	return 0;
    }
    ifri_send(sd, &m1);
    ifri_receive(sd, &m1);
    position+=m1.result;
  } while(m1.result > 0);
  close(d_f);
  return(m1.result >= 0 ? OK: m1.result);
}

static int initialize(char *server_ipaddr, char *server_port){
  struct sockaddr server_addr;
  sd = socket(AF_INET, SOCK_STREAM, 0);
  int salen, err;
  if(resolve_address(&server_addr, &salen, server_ipaddr, server_port, 
      AF_INET, SOCK_STREAM, IPPROTO_TCP)!= 0){
      fprintf(stderr, "Erreur de configuration de sockaddr\n");
      return -1;
   }
   if((err = connect(sd, &server_addr, salen))!=0){
     fprintf(stderr, "Erreur de connection au server %d\n", errno);
     return -1;
   }
   fprintf(stderr, "Connexion reussi au serveur \n");
   return OK;
}

int main(int argc, char * argv[]){
  if(argc != 5){
    fprintf(stderr, "USAGE %s <server_addr> <server_port> <src> <dst> \n", argv[0] );
    return 0;
  }
  initialize(argv[1], argv[2]);
  copy(argv[3], argv[4]);
  release();
  return 0;
}

static int release(void){
  return close(sd);
}
