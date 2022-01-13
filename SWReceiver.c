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
#include "SWReceiver.h"

int main(int argc, char **argv) {
    int port_medium = atoi("5555");
    int port_local = atoi("6666");
    char buffer[1024];
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) {
        perror("[ERROR] Socket is not created.\n");
        return(errno);
    }
    else {
        printf("[SUCCESS] Socket created.\n");
    }

    char idFlux=0;
    Packet packet_send;
    Packet packet_recv;
    int ack_recv=1;

    
    struct sockaddr_in serverAddr, mediumAddr;
    socklen_t addr_size;
    memset(&serverAddr, 0, sizeof(serverAddr));
    memset(&mediumAddr, 0, sizeof(mediumAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port=htons(port_local);
    serverAddr.sin_addr.s_addr= inet_addr("127.0.0.1");
    mediumAddr.sin_family=AF_INET;
    mediumAddr.sin_port=htons(port_medium);
    mediumAddr.sin_addr.s_addr= inet_addr("127.0.0.1");



    if (bind(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        printf("Error\n");
/*
0- ACK
1- SEQ
*/

    //Connection establishement
    addr_size = sizeof(serverAddr);
    int f_recv_size= recvfrom(sockfd,&packet_recv, sizeof(Packet), 0,(struct sockaddr *)&serverAddr,&addr_size);
    if(f_recv_size >0 && packet_recv.type==1 ){
        printf("[+] SYN Packet Recieved\n");
    }
    else {
        printf("[-] SYN Packet not recieved\n");    
    }
    ack_recv=1;
    if(ack_recv=1) {
        packet_send.id=2; packet_send.type=17;
        packet_send.acq=packet_recv.seqNum+1;
        packet_send.seqNum=2000;
        printf("[+]SeqNum: %d\nACK Num:%d,Type:%d\n",packet_send.seqNum,packet_send.acq,packet_send.type);
        sendto(sockfd,&packet_send,sizeof(Packet), 0, (struct sockaddr *)&mediumAddr,sizeof(mediumAddr));
        printf("[+] SYN-ACK Packet Sent.\n");
        int f_recv_size= recvfrom(sockfd,&packet_recv, sizeof(Packet), 0,(struct sockaddr *)&serverAddr,&addr_size);
        printf("Received type=%d seq=%d ack=%d\n",packet_recv.type,packet_recv.seqNum,packet_recv.acq);
        if(f_recv_size >0 && packet_recv.type==16 && packet_recv.acq==packet_send.seqNum+1){
            printf("[+] ACK Packet Recieved\n");
            ack_recv=1;
            printf("Connection Established\n");
            }else{
                printf("[-] ACK Packet not recieved\n");
                ack_recv=0;
            }
        }

    int dataFinish=1;
    while (dataFinish) {
        int addr_size = sizeof(mediumAddr);
        int f_recv_size= recvfrom(sockfd,&packet_recv, sizeof(Packet), 0,(struct sockaddr *)&mediumAddr,&addr_size);
        if (f_recv_size > 0) {
            if (packet_recv.type == 16) {
                printf("[+] ACK Recieved\n ACK NO:%d\n Seq NO:%d\n",packet_recv.acq,packet_recv.seqNum);
                printf("Message recieved:%s\n",packet_recv.data);
                ack_recv=1;
            } if (packet_recv.type == 2) {
                packet_send.acq = (packet_recv.seqNum + 1);
                packet_send.type = 18;
                packet_send.seqNum = 97;
                if (sendto(sockfd, &packet_send, sizeof(Packet), 0,
                     (struct sockaddr *) &mediumAddr, sizeof(mediumAddr)) != sizeof(Packet))
                    printf("Error\n");
                int closeFinACKLen;
                int serverAddrLen = sizeof(serverAddr);
                if(closeFinACKLen = recvfrom(sockfd, &packet_recv, sizeof(Packet), 0,
                        (struct sockaddr *) &serverAddr, &serverAddrLen) == -1)
                        printf("[-] Error\n");
                if (packet_recv.acq==packet_send.seqNum+1){
                    printf("All messages has been received\n");
                    printf("Deconnecting...\n");
                    sleep(2);
                    dataFinish = 0;
                }    
            }
        } 
        else {
            printf("[-] ACK not recieved\n");
            ack_recv = 0;
        }
        idFlux++;
        if (ack_recv=1) {
            packet_send.seqNum=packet_recv.seqNum;
            packet_send.type=packet_recv.type;
            packet_send.ecn=packet_recv.ecn;
            packet_send.acq=(packet_recv.seqNum+1)%2;
            packet_send.id=packet_recv.id;
            sendto(sockfd,&packet_send,sizeof(Packet), 0, (struct sockaddr *)&serverAddr,sizeof(serverAddr));
            printf("[+]Packet Sent\n");
            printf("[+] Sending Packet %d\nSeqNum:%d\nACK Num:%d\n",packet_send.id,packet_send.seqNum,packet_send.acq);
        }
    }
    close(sockfd);
    
}
