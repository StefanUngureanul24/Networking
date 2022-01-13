#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <errno.h>  
#include "GoBackNSender.c" 
#include "SWSender.c"

int main(int argc,char ** argv){

	if(argc!=5){
		printf("./source <mode> <IP_distante> <port_local> <port_ecoute_src_pertubateur>\n");
		exit(EXIT_FAILURE);
	}
	
	int mode=atoi(argv[1]);
	char *ip=argv[2];
	char *port_local=argv[3];
	char *port_medium=argv[4];
	
	if(mode==1)
		GoBackNSender(ip,port_local,port_medium);
	else
		SWSender(ip,port_local,port_medium);

	return 0;
}
