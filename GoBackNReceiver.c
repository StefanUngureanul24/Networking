#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>    
#include "GoBackNReceiver.h" 

#define ECHOMAX 50000     

int main(int argc, char *argv[])
{
	/*
		Verifying if the file is run
		with the correct number of
		arguments
	*/
    if (argc != 4) {
		printf("./source <IP_distante> <port_local> <port_ecoute_src_pertubateur>\n");
		exit(EXIT_FAILURE);
	}

    int sockfd;                             
    int recvMsgSize;                 
    int chunkSize;                   
    float loss_rate = 0;             

    int port_medium = atoi(argv[3]);
    int port_local = atoi(argv[2]);
    char buffer[1024];
    
    /* 
    	Creation of UDP socket
    */
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[ERROR] Socket is not created.\n");
    }    

    else {
        printf("[SUCCESS] Socket created.\n");
    }
    struct sockaddr_in mediumAddr, serverAddr;
    socklen_t addr_size;
    memset(&serverAddr, 0, sizeof(serverAddr));
    memset(&mediumAddr, 0, sizeof(mediumAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port=htons(port_local);
    serverAddr.sin_addr.s_addr= inet_addr(argv[1]);
    mediumAddr.sin_family=AF_INET;
    mediumAddr.sin_port=htons(port_medium);
    mediumAddr.sin_addr.s_addr= inet_addr(argv[1]);

    if (bind(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        printf("Error\n");

	/* 
		Initialising the base variables 
		that will be used
	*/
    int base = -2;
    int dataFinish = 0;
    memset(buffer, 0, sizeof(buffer));
    int mediumAddrLen = sizeof(mediumAddr);
    int serverAddrLen = sizeof(serverAddr);

    Packet packet_recv;
    Packet packet_send;

	/* Establishment of connection */
    addr_size = sizeof(serverAddr);
    int f_recv_size = recvfrom(sockfd, &packet_recv, sizeof(Packet), 0,(struct sockaddr *)&serverAddr, &addr_size);
    if (f_recv_size > 0 && packet_recv.type == 1) {
        printf("[+] SYN Packet Recieved\n");
    }
    
    else {
        printf("[-] SYN Packet not recieved\n");
        
    }
    
    int ack_recv = 1;
    if (ack_recv == 1) {
        
        packet_send.id=2;
        packet_send.type=17;
        packet_send.acq=packet_recv.seqNum+1;
        packet_send.seqNum=2000;
        
        sendto(sockfd,&packet_send,sizeof(Packet), 0, (struct sockaddr *)&mediumAddr,sizeof(mediumAddr));
        printf("[+] SYN-ACK Packet Sent.\n");

        int f_recv_size= recvfrom(sockfd, &packet_recv, sizeof(Packet), 0, (struct sockaddr *)&serverAddr, &addr_size);
        if (f_recv_size > 0 && packet_recv.type == 16 && packet_recv.acq == packet_send.seqNum + 1) {
            printf("[+] ACK Packet Recieved\n");
            ack_recv = 1;
            printf("Connection Established\n");
            }
        else {
                printf("[-] ACK Packet not recieved\n");
                ack_recv = 0;
		}
    }


    while (!dataFinish) 
    {
        int seqNumber = 0;
        
        if ((recvMsgSize = recvfrom(sockfd, &packet_recv, sizeof(Packet), 0,
            (struct sockaddr *) &serverAddr, &serverAddrLen)) < 0)
            
            printf("Packet re??u : %d\n", recvMsgSize);

            seqNumber = packet_recv.seqNum;
            
            /* 
            	If sequence number is 0
            	then a new collection of data
            	is built
            */
            if (packet_recv.seqNum == 0 && packet_recv.type == 16) {
                printf("Recieved Initial Packet from %s\n", inet_ntoa(mediumAddr.sin_addr));
                printf("Sequence Number #%d - \n %s : \n", packet_recv.seqNum, packet_recv.data);
                strcat(buffer, packet_recv.data);                
                base = packet_recv.id % 10;                
                short ack = packet_recv.acq;
                
            } 
            
            /* if base+1 then it's a subsequent in order packet */
            else if (packet_recv.seqNum == base + 1 && packet_recv.type == 16) {
                /* 
                	Then data will be concatenated
                	to buffer's content
                */
                printf("Sequence Number #%d - \n %s : \n", packet_recv.seqNum, packet_recv.data);
                strcat(buffer, packet_recv.data);                
                base = packet_recv.seqNum;
                short ack = packet_recv.acq;
            } 
            else if (packet_recv.type == 1 && packet_recv.seqNum != base + 1) {
                /* if recieved out of sync packet, send ACK with old base */
                printf("Recieved Out of Sync Packet #%d\n", packet_recv.seqNum);
                /* we send the ack with the old base */
                short ack = packet_recv.acq;
            }
            
            packet_send = packet_recv;
            
            /* 
            	Sending the ACK for a packet received
            */
            if (packet_recv.type==16 && base >= 0 && base < packet_send.fenetre){
                printf("-Sending ACKK #%d\n", packet_send.id);
                if (sendto(sockfd, &packet_send, sizeof(packet_send), 0,
                     (struct sockaddr *) &mediumAddr, sizeof(mediumAddr)) != sizeof(packet_send))
                    printf("Error\n");
            } 
            else if (base == -1 ) {
                printf("Recieved Teardown Packet\n");
                printf("Sending Terminal ACK\n");
                if (sendto(sockfd, &packet_send, sizeof(Packet), 0,
                     (struct sockaddr *) &mediumAddr, sizeof(mediumAddr)) != sizeof(Packet))
                    printf("Error\n");
            }
          
            /* 
            	If the packet is the last one
            	then a message will be displayed
            */
            if (((packet_recv.id)%10) == 9 && packet_recv.type != 2) {
                    printf("\nMESSAGE RECIEVED\n%s\n\n", buffer);
                    memset(buffer, 0, sizeof(buffer));
            }

            if(packet_recv.type == 2) {
                packet_send.acq=(packet_recv.seqNum+1);
                packet_send.type=18;
                packet_send.seqNum=97;
                if (sendto(sockfd, &packet_send, sizeof(Packet), 0,
                     (struct sockaddr *) &mediumAddr, sizeof(mediumAddr)) != sizeof(Packet))
                    printf("Error\n");
                int closeFinACKLen;
                if(closeFinACKLen = recvfrom(sockfd, &packet_recv, sizeof(Packet), 0,
                        (struct sockaddr *) &serverAddr, &serverAddrLen) == -1)
                        printf("[-] Error\n");
                if(packet_recv.acq==packet_send.seqNum+1){
                    printf("All messages has been received\n");
                    printf("Deconnecting...\n");
                    sleep(2);
                    dataFinish=1;
                }
            }
    }
    close(sockfd);
}
