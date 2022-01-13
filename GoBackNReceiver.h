#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>    

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
