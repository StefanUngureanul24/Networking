#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct packet{
    char id;
    char type; 
    short seqNum; 
    short acq;
    char ecn;
    char fenetre;
    char data[44];
}Packet;
