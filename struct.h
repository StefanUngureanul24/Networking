#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <errno.h>      

#define WINDOWSIZE 4
#define DATACOUNT 3

typedef struct packet{
    char id;
    char type; 
    short seqNum; 
    short acq;
    char ecn;
    char fenetre;
    char data[44];
    int length;
} Packet;


int GoBackNSender();

int SWSender();