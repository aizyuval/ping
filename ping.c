#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
void error(const char *msg)
{
      perror(msg);
      exit(0);
}
uint16_t checksum (uint16_t *, int);
int main(int argc, char **argv){

    int sockfd, new_sockfd, bytesSent;
    struct sockaddr_in their_addr; 
    
    // set up addrinfo struct for the info
    struct addrinfo pre_destaddr, *destaddr;
    memset(&pre_destaddr,0,sizeof(pre_destaddr));
    
    // init family for sa_family in sockddr
    pre_destaddr.ai_family = AF_INET;
    // setting a destination address structure from user "gnu.org".
    getaddrinfo(argv[1], "0", &pre_destaddr, &destaddr);
                                  
    
    struct icmp *packet_icmp;
    char *mesg = "Test";
    int len = 8 + strlen(mesg);		/* ICMP header and data */
    char sent_packet[len];
    //casting sent_packet pointer to type icmp, and making packet_icmp point to buffer.
    packet_icmp = (struct icmp *)sent_packet;

    //fill the header:
    packet_icmp->icmp_type = ICMP_ECHO;
    packet_icmp->icmp_code = 0;
    packet_icmp->icmp_seq = 0;
    packet_icmp->icmp_id = 0;

    // insert data
    memcpy(packet_icmp->icmp_data, mesg, strlen(mesg));
    printf("data sent should be: %s", packet_icmp->icmp_data);
    

	packet_icmp->icmp_cksum = 0;
    packet_icmp->icmp_cksum = checksum((uint16_t *) packet_icmp, len);

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // through this socket, send a sent_packet that includes only the icmp packet. the ip header and eth provided 
    if(sockfd<0){
        error("error in socket opening");
    }
  // connectionless means just sending it without promises.
    bytesSent = sendto(sockfd, sent_packet, 1460, 0,destaddr->ai_addr, sizeof(struct sockaddr));// sent_packet is malformed. it has data, but it doesnt seem to be considered as protocol.
    if(bytesSent<0){//or smaller from sent packet
        error("error in sending data through socketfd");
    }
   
/*

   if(listen(sockfd,5) != 0){
       error("error on listening");
   }
    socklen_t addr_size = sizeof(their_addr);
    new_sockfd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
    if(new_sockfd<0){
    error("error on accepting from sockfd"
    )
    
    }
    bytesRecieved = recvfrom(new_sockfd, buf, len, 0, &their_addr, their_len);
    if(bytesRecieved < 0){
    error("error on recvfrom");
    }
 */  
    // decapsulate the packet.
    close(sockfd);
     return 0;
}

uint16_t
checksum (uint16_t *addr, int len) {

  int count = len;
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

