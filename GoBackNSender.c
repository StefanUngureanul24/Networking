#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <errno.h>   
#include "struct.h"   


    
int GoBackNSender(char *ip, char* portlocal, char*portmedium) 
{
    
    int chunkSize = 3; //size of each packet data
    int windowSize = 10; //number of windows
    char buffer1[1024] = "oh god i love reseaux so much!";
    char buffer2[1024] = "whatever comes to my mind make";
    char buffer3[1024] = "lorem ipsum kods qsd sljjsq jk";

    char **buffer=malloc(DATACOUNT*sizeof(char*));
    buffer[0]=buffer1;
    buffer[1]=buffer2;
    buffer[2]=buffer3;

    int port_local = atoi(portlocal);
    int port_dest = atoi(portmedium);
    
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[-] Socket pas créé.\n");
        return(errno);
    }
    else {
        printf("[+] Socket créé.\n");
    }

    //int ack_recv = 1;
    
    struct sockaddr_in originAddr, destAddr;
    socklen_t addr_size;

    memset(&destAddr, 0, sizeof(destAddr));
    
    originAddr.sin_family = AF_INET;
    originAddr.sin_port=htons(port_local);
    originAddr.sin_addr.s_addr= inet_addr(ip);
    
    destAddr.sin_family=AF_INET;
    destAddr.sin_port=htons(port_dest);
    destAddr.sin_addr.s_addr= inet_addr(ip);

    bind(sockfd,(const struct sockaddr*)&originAddr,sizeof(struct sockaddr_in));
    
    int dataLength = 0;
    int probACK = 1;
    char idValue = 0;
    int respStringLen;
    int originAddrLen;
    int destAddrLen = sizeof(destAddr);
    Packet packet_send;
    Packet packet_recv;
    int indexBuffer=0;
    int connectionEstablished=0;

    //Connection establishing
    packet_send.id=1;
    packet_send.type=1;
    packet_send.seqNum=521;
    packet_send.ecn=0;
    
    printf("[+] SYN Packet Sent\n");
    sendto(sockfd,&packet_send,sizeof(Packet), 0, (struct sockaddr *)&destAddr,sizeof(destAddr));
    
    addr_size = sizeof(originAddr);
    int f_recv_size= recvfrom(sockfd,&packet_recv, sizeof(Packet), 0,(struct sockaddr *)&originAddr,&addr_size);
    if(f_recv_size >0 && packet_recv.type==17 && packet_recv.acq==packet_send.seqNum+1 ){
        printf("[+] SYN-ACK Packet Received.\n");
        int ack_recieved=packet_recv.acq;
        packet_send.id++;
        packet_send.type=16;
        packet_send.acq=packet_recv.seqNum+1;
        packet_send.seqNum=ack_recieved;
        printf("[+] ACK Packet Sent\n");
        sendto(sockfd,&packet_send,sizeof(Packet), 0, (struct sockaddr *)&destAddr,sizeof(destAddr));
        printf("[+] Connection Established.\n");
    } else {
        printf("[-] SYN-ACKPacket not recieved\n");
    }

    while (probACK && (indexBuffer<DATACOUNT)) {
        char seqNum = 0;
        int tailleBuffer = strlen(buffer[indexBuffer]);
        printf("%d\n",tailleBuffer);
        int numSegments = tailleBuffer / chunkSize;
        if (strlen(buffer[indexBuffer]) % chunkSize > 0)
            numSegments++;

        while (seqNum < numSegments) {

            char seg_data[chunkSize];
            strncpy(seg_data, (buffer[indexBuffer] + (idValue%10) * chunkSize), chunkSize);
        
            packet_send.id = idValue;
            packet_send.seqNum = seqNum;                
            packet_send.type = 16;
            packet_send.acq = idValue; 
            packet_send.fenetre = windowSize;
            packet_send.length = chunkSize;
            strcpy(packet_send.data, seg_data);
            printf("Sending Packet : %d - %s - ACK : %d - Data: %s\n", idValue, packet_send.data, packet_send.acq,packet_send.data);
            memset(seg_data,0,sizeof(seg_data));
            

            if (sendto(sockfd,&packet_send,sizeof(Packet), 0, (struct sockaddr *)&destAddr,sizeof(destAddr)) != sizeof(packet_send))
                printf("Erreur de l'envoi\n");
            
            seqNum++;
            idValue++;
            
        } 
        idValue++;
        
        printf("Waiting for the ACK number\n");
        originAddrLen = sizeof(originAddr);
        while (respStringLen = recvfrom(sockfd, &packet_recv, sizeof(Packet), 0,
                        (struct sockaddr *) &originAddr, &originAddrLen) != -1)    
        {     
            
                printf("Received Packet : ACK No: %d\n", packet_recv.acq);
                printf("Num Seq=%d\n",packet_recv.id);
                if((packet_recv.id%10)==9) break;
        }

        idValue--;
        indexBuffer++;
    } 

    //3WayHandshake Closing Connection
      
    Packet packet_fin;
    packet_fin.type=2;
    packet_fin.seqNum=92;
    printf("FIN packet sent\n");

    int closeConnection=0;
    int closeFinACKLen;
    while(closeConnection==0){
        if (sendto(sockfd,&packet_fin,sizeof(Packet), 0, (struct sockaddr *)&destAddr,sizeof(destAddr)) != sizeof(packet_send))
            printf("Erreur de l'envoi\n");
        int packetFinSeq=packet_fin.seqNum;
        printf("PacketFinSeqNum: %d\n",packetFinSeq);
        while(1){
            printf("Waiting for FIN-ACK\n");
            if(closeFinACKLen = recvfrom(sockfd, &packet_fin, sizeof(Packet), 0,
                        (struct sockaddr *) &originAddr, &originAddrLen) == -1)
                        printf("[-] Error");
            printf("Packet fin: acq=%d seqnum=%d\n",packet_fin.acq,packet_fin.seqNum);
            if(packet_fin.acq=packetFinSeq+1 && packet_fin.type==18){
                packet_fin.acq=packet_fin.seqNum+1;
                if (sendto(sockfd,&packet_fin,sizeof(Packet), 0, (struct sockaddr *)&destAddr,sizeof(destAddr)) != sizeof(packet_send))
                    printf("Erreur de l'envoi\n");
                    closeConnection=1;
                    printf("Disconnecting...\n");
                    sleep(2);
                    break;
            }
        }
    probACK=0;
    }
    close(sockfd);

}
