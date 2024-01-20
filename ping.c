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
    // set up a for loop. send, recieve and validate on demand
    if(argv[1]==NULL||argv[2]==NULL){
        printf("usage is: sudo %s google.com 2\n ", argv[0]);
        printf("it's sudo + execution + domain or ipv4 + number of pings\n");
        exit(0);
    }
    char *dummy;
    const long iterations = strtol(argv[2], &dummy, 10);  
    int sockfd;
    socklen_t  addr_size = SOCKADDR_LEN; // pointer to size 
    struct sockaddr_in their_addr; 
       
    // set up addrinfo struct for the info, with the address given by user
    struct addrinfo pre_destaddr, *destaddr;
    memset(&pre_destaddr,0,sizeof(pre_destaddr));
    pre_destaddr.ai_family = AF_INET;
    getaddrinfo(argv[1], "0", &pre_destaddr, &destaddr);
                                  
    // cast destination address to validate it seperately from src address (with sockaddr_in)
    struct sockaddr_in * destaddr_in;
    destaddr_in = (struct sockaddr_in*)destaddr->ai_addr;
    
    struct icmp *packet_icmp; // actual structure of the packet
    char *mesg = "test";
    int icmp_buflen = ICMP_HDRLEN + strlen(mesg);		/* ICMP header and data */
    char sent_packet[icmp_buflen];

    //packet_icmp should point to sent_packet memory space
    packet_icmp = (struct icmp *)sent_packet;

    int income_len = IP4_HDRLEN + icmp_buflen;
    char income_packet[income_len];

    //fill the header
    packet_icmp->icmp_seq = 0;
    packet_icmp->icmp_type = ICMP_ECHO;
    packet_icmp->icmp_code = 0;

    // insert data
    memcpy(packet_icmp->icmp_data, mesg, strlen(mesg));
    
    // define structures and assign space for icmp and ip splitted data
    struct ip *recIp;
    char ipBytes[IP4_HDRLEN]; 
    recIp = (struct ip *)ipBytes;

    struct icmp *recIcmp;
    char icmpBytes[icmp_buflen]; 
    recIcmp = (struct icmp *)icmpBytes;

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // initialize ipv4 raw socket of icmp protocol 
    if(sockfd<0){
        printf("usage is: sudo %s google.com 2\n ", argv[0]);
        printf("it's sudo + execution + domain or ipv4 + number of pings\n");
        error("error in socket opening");
    }
    // send the packet to the address
    
    // -> start for loop
    for(long i = 0; i<iterations; i++){

         
        packet_icmp->icmp_id = (short)i; // change id over iterations
        packet_icmp->icmp_cksum = 0;

        //compute checksum
        packet_icmp->icmp_cksum = checksum((uint16_t *) packet_icmp, icmp_buflen);
        int bytesSent, icmp_bytesRecieved;
        bytesSent = sendto(sockfd, sent_packet, icmp_buflen, 0,destaddr->ai_addr, SOCKADDR_LEN );

        if(bytesSent<0){
            error("error in sending data through socketfd");
        }
        if(bytesSent!=icmp_buflen){
            printf("should've sent %d bytes but instead send %d bytes ", icmp_buflen, bytesSent);
        } 
        
        // recieve the packet
        icmp_bytesRecieved = recvfrom(sockfd, income_packet, income_len, 0, (struct sockaddr*)&their_addr, &addr_size);
        if(icmp_bytesRecieved < 0){
        error("error on recvfrom");
        }
        
        // copy data from packet to structures
        struct ip *recIpP = memcpy(recIp, income_packet, IP4_HDRLEN);
        struct icmp *recIcmpP = memcpy(recIcmp, (income_packet + IP4_HDRLEN), icmp_buflen); //check that pointers work correctly
        // validate checksum?
        if(recIpP != recIp || recIcmpP != recIcmp){
            printf("structure init (with pointers) is wrong");
        } 

        if(destaddr_in->sin_addr.s_addr != recIp->ip_src.s_addr) //compare adresses
        {
            printf("\nthe host didn't respond. Though someone else did");
                exit(0);
        }else if(recIcmp->icmp_type != ICMP_ECHOREPLY){// validate icmp type
            printf("\nsomething is wrong with your ping. you probably used the dest as router");
            exit(0);
        }else{
            printf("\nhost is up. | Sequence: %d| Id: %ld\n", packet_icmp->icmp_seq,i);
        }
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

