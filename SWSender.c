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

int SWSender(char *ip, char* portlocal, char* portmedium) {    
    struct timeval tv;
    int port_local = atoi(portlocal);
    int port_dest = atoi(portmedium);
    char buffer[1024];
    int sockfd;

    if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0))<0) {
        perror("[ERROR] Socket is not created.\n");
        return(errno);
    }
    else {
        printf("[SUCCESS] Socket created.\n");
    }

    char idFlux=2;
    Packet packet_send;
    Packet packet_recv;
    int ack_recv=1;
    
    struct sockaddr_in originAddr, destAddr;
    socklen_t addr_size;
    memset(&originAddr, 0, sizeof(originAddr));
    memset(&destAddr, 0, sizeof(destAddr));
    originAddr.sin_family = AF_INET;
    originAddr.sin_port=htons(port_local);
    originAddr.sin_addr.s_addr= inet_addr(ip);
    destAddr.sin_family=AF_INET;
    destAddr.sin_port=htons(port_dest);
    destAddr.sin_addr.s_addr= inet_addr(ip);

    bind(sockfd,(const struct sockaddr*)&originAddr,sizeof(struct sockaddr_in));
/*
0- ACK
1- SEQ
*/
  
    //Connection establishing
    packet_send.id=1;
    packet_send.type=1;
    packet_send.seqNum=521;
    packet_send.ecn=0;
    
    printf("[+] SYN Packet Sent\n");
    sendto(sockfd,&packet_send,sizeof(Packet), 0, (struct sockaddr *)&destAddr,sizeof(destAddr));
    
    addr_size = sizeof(originAddr);
    int f_recv_size= recvfrom(sockfd,&packet_recv, sizeof(Packet), 0,(struct sockaddr *)&originAddr,&addr_size);
    printf("Received type=%d seq=%d ack=%d\n",packet_recv.type,packet_recv.seqNum,packet_recv.acq);
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

    }else{
        printf("[-] SYN-ACKPacket not recieved\n");
    }
    
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
    packet_send.fenetre=1;
    packet_send.id=1;
    packet_send.seqNum=0;
    
    int dataFinish=1;
    while(dataFinish==1)
    {
            if(ack_recv==1) {
            
            packet_send.type=16;
            packet_send.ecn=1;

            printf("Enter Data: ");
            scanf("%s",buffer);
            strcpy(packet_send.data,buffer);

            if((strcmp(buffer,"exit"))==0){
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
                        int originAddrLen=sizeof(originAddr);
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
                                dataFinish=0;
                                return(EXIT_SUCCESS);
                        }
                    }
                }
            }
            
            sendto(sockfd,&packet_send,sizeof(Packet), 0, (struct sockaddr *)&originAddr,sizeof(originAddr));
            printf("[+]Packet Sent\n");
            printf("[+] Sending Packet %d\nSeqNum:%d\n",packet_send.id,packet_send.seqNum);
        }
            
        int addr_size = sizeof(destAddr);
        int f_recv_size= recvfrom(sockfd,&packet_recv, sizeof(Packet), 0,(struct sockaddr *)&destAddr,&addr_size);
        if (f_recv_size >0 ) {
            printf("[+] ACK Recieved %d from %d\n",packet_recv.acq,packet_recv.id);
            packet_send.id=packet_recv.id;
            packet_send.seqNum=(packet_recv.seqNum+1)%2;
            packet_send.id++;
            ack_recv=1;
        } else {
            printf("[-] ACK not recieved\n");
            printf("Packet %d Resending...\n",packet_send.id);
            ack_recv=0;
        }
        
    }
    close(sockfd);    
}
