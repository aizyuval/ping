// what's stopping me from impersonating someone else with sending an ip packet including his ip instead of mine?
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define IP4_HDRLEN 20         // IPv4 header length
#define ICMP_HDRLEN 8         // ICMP header length for echo request, excludes data
#define SOCKADDR_LEN 16                              
void error(const char *msg)
{
      perror(msg);
      exit(0);
}
uint16_t checksum (uint16_t *, int);//pre
int main(int argc, char **argv){

    int sockfd, bytesSent, icmp_bytesRecieved;
    struct sockaddr_in their_addr; 
    
    // set up addrinfo struct for the info, with the address given by user
    struct addrinfo pre_destaddr, *destaddr;
    memset(&pre_destaddr,0,sizeof(pre_destaddr));
    pre_destaddr.ai_family = AF_INET;
    getaddrinfo(argv[1], "0", &pre_destaddr, &destaddr);
                                  
    
    struct icmp *packet_icmp; // actual structure of the packet
    char *mesg = "test";
    int icmp_buflen = ICMP_HDRLEN + strlen(mesg);		/* ICMP header and data */
    char sent_packet[icmp_buflen];

    int income_len = IP4_HDRLEN + icmp_buflen;
    char income_packet[income_len]; 

    //packet_icmp should point to sent_packet memory space
    packet_icmp = (struct icmp *)sent_packet;

    //fill the header
    packet_icmp->icmp_type = ICMP_ECHO;
    packet_icmp->icmp_code = 0;
    packet_icmp->icmp_seq = 0;
    packet_icmp->icmp_id = 0;

    // insert data
    memcpy(packet_icmp->icmp_data, mesg, strlen(mesg));
    
    //compute checksum
	packet_icmp->icmp_cksum = 0;
    packet_icmp->icmp_cksum = checksum((uint16_t *) packet_icmp, icmp_buflen);

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // initialize ipv4 raw socket of icmp protocol 
    if(sockfd<0){
        error("error in socket opening");
    }
    // send the packet to the address
    bytesSent = sendto(sockfd, sent_packet, icmp_buflen, 0,destaddr->ai_addr, SOCKADDR_LEN );

    if(bytesSent<0){
        error("error in sending data through socketfd");
    }
    if(bytesSent!=icmp_buflen){
        printf("should've sent %d bytes but instead send %d bytes ", icmp_buflen, bytesSent);
    } 
    

    socklen_t addr_size = SOCKADDR_LEN;
    // recieve the packet
    icmp_bytesRecieved = recvfrom(sockfd, income_packet, income_len, 0, (struct sockaddr*)&their_addr, &addr_size);
    if(icmp_bytesRecieved < 0){
    error("error on recvfrom");
    }
   
    // define structures and assign space for icmp and ip splitted data
    struct ip *recIp;// NOTE: should this be a pointer really? 
    char ipBytes[IP4_HDRLEN]; 
    recIp = (struct ip *)ipBytes;

    struct icmp *recIcmp;// NOTE: should this be a pointer really? 
    char icmpBytes[icmp_buflen]; 
    recIcmp = (struct icmp *)icmpBytes;
    
    // copy data from packet to structures
    memcpy(recIp, income_packet, IP4_HDRLEN);
    memcpy(recIcmp, (income_packet + IP4_HDRLEN), icmp_buflen);
    // validate checksum?
    
    // cast destination address to validate it seperately from src address (with sockaddr_in)
    struct sockaddr_in * destaddr_in;
    destaddr_in = (struct sockaddr_in*)destaddr->ai_addr;

    if(destaddr_in->sin_addr.s_addr != recIp->ip_src.s_addr) //compare adresses
    {
        printf("\nthe host didn't respond. Though someone else did");
            exit(0);
    }else if(recIcmp->icmp_type != ICMP_ECHOREPLY){// validate icmp type
        printf("\nsomething is wrong with your ping. you probably used the dest as router");
        exit(0);
    }else{
        printf("\nthe host is up, and have pinged back! Congrats.\n");
    }
    close(sockfd);
     return 0;
}

uint16_t
checksum (uint16_t *addr, int length) {

  int count = length;
  register uint32_t sum = 0;
  uint16_t answer = 0;

  // Sum up 2-byte values until none or only one byte left.
  while (count > 1) {
    sum += *(addr++);
    count -= 2;
  }

  // Add left-over byte, if any.
  if (count > 0) {
    sum += *(uint8_t *) addr;
  }

  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  // Checksum is one's compliment of sum.
  answer = ~sum;

  return (answer);
}

