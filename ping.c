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
void error(const char *msg)
{
      perror(msg);
      exit(0);
}
uint16_t checksum (uint16_t *, int);
int main(int argc, char **argv){

    int sockfd, new_sockfd, bytesSent, bytesRecieved;
    struct sockaddr_in their_addr; 
    
    // set up addrinfo struct for the info
    struct addrinfo pre_destaddr, *destaddr;
    memset(&pre_destaddr,0,sizeof(pre_destaddr));
    
    // init family for sa_family in sockddr
    pre_destaddr.ai_family = AF_INET;
    // setting a destination address structure from user "gnu.org".
    getaddrinfo(argv[1], "0", &pre_destaddr, &destaddr);
                                  
    
    struct icmp *packet_icmp;
    char *mesg = "test";
    int icmp_buflen = ICMP_HDRLEN + strlen(mesg);		/* ICMP header and data */
    char sent_packet[icmp_buflen];
    int income_len = IP4_HDRLEN + icmp_buflen;
    char income_packet[income_len]; 

    //casting sent_packet pointer to type icmp, and making packet_icmp point to buffer.
    packet_icmp = (struct icmp *)sent_packet;

    //fill the header:
    packet_icmp->icmp_type = ICMP_ECHO;
    packet_icmp->icmp_code = 0;
    packet_icmp->icmp_seq = 0;
    packet_icmp->icmp_id = 0;

    // insert data
    memcpy(packet_icmp->icmp_data, mesg, strlen(mesg));

	packet_icmp->icmp_cksum = 0;
    packet_icmp->icmp_cksum = checksum((uint16_t *) packet_icmp, icmp_buflen);

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // through this socket, send a sent_packet that includes only the icmp packet. Ip header and link layer header provided to me. 
    if(sockfd<0){
        error("error in socket opening");
    }
    bytesSent = sendto(sockfd, sent_packet, icmp_buflen, 0,destaddr->ai_addr, sizeof(struct sockaddr));
    if(bytesSent<0){
        error("error in sending data through socketfd");
    }
    if(bytesSent!=icmp_buflen){
        printf("should've sent %d bytes but instead send %d bytes ", icmp_buflen, bytesSent);
    } 
    printf("size of bytes sent: %d", bytesSent);

    socklen_t addr_size = sizeof(struct sockaddr_in);
    bytesRecieved = recvfrom(sockfd, income_packet, income_len, 0, (struct sockaddr*)&their_addr, &addr_size);
    struct sockaddr_in their_addr_in = (struct sockaddr_in)their_addr;
    if(bytesRecieved < 0){
    error("error on recvfrom");
    }
    printf("size of bytes recieved: %d", bytesRecieved);
    // bytesRecieved -> | i    p   | 20 bytes
    //               -> | i c m p  | 12 bytes (includes my data!)
    // validate echo reply from the destination
    // bytes recieved should include my data, and the dest address as source
    // decapsulate the packet.
    
    struct ip *recIp;// NOTE: should this be a pointer really? 
    char ipBytes[IP4_HDRLEN]; 
    recIp = (struct ip *)ipBytes;
    struct icmp *recIcmp;// NOTE: should this be a pointer really? 
    char icmpBytes[icmp_buflen]; 
    recIcmp = (struct icmp *)icmpBytes;
    memcpy(recIp, income_packet, IP4_HDRLEN);
    memcpy(recIcmp, (income_packet + IP4_HDRLEN), icmp_buflen);
    // validate echo reply!
    // validate ip address!
    // validate checksum?
    struct sockaddr_in * destaddr_in;
    destaddr_in = (struct sockaddr_in*)destaddr->ai_addr;
    if(destaddr_in->sin_addr.s_addr != recIp->ip_src.s_addr) // do my dest addr equals recieved src addres? 
    {
        printf("the host didn't respond. Though someone else did");
            exit(0);
    }else if(recIcmp->icmp_type != ICMP_ECHOREPLY){
        printf("something is wrong with your ping. you probably used the dest as router");
        exit(0);
    }else{
        printf("the host is up, and have pinged back! Congrats.");
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

