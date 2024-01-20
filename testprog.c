#include <netinet/ip_icmp.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char ** argv){

    struct icmp *hdr_icmp;
    struct icmp *test_icmp;

    //form an icmp packet
    //char sent_packet[256];
//    char * sent_packet = malloc(256);
    char sent_packet[256];
    char test_packet[256];
    test_icmp = (struct icmp *)test_packet;
    hdr_icmp = (struct icmp *)sent_packet;
    hdr_icmp->icmp_type = ICMP_ECHO;
    hdr_icmp->icmp_code = 2;
    hdr_icmp->icmp_seq = 0;
    hdr_icmp->icmp_id = 0;
    // before checksum calc, insert data
    int datalen = 3;
    char *a = "hello";
    datalen = strlen(a);
    memcpy(hdr_icmp->icmp_data, a, datalen);
    //slap the data!
    /*
    memset(hdr_icmp->icmp_data, 'w',1);
    memset(hdr_icmp->icmp_data, 'i',1);
    memset(hdr_icmp->icmp_data, 'n',1);*/
    
	hdr_icmp->icmp_cksum = 3;
    memcpy(test_icmp, hdr_icmp,1);
    printf("if memcpy worked as planned, test_icmp should be 8 (%d), and icmp_code NOT 2 (%d)", test_icmp->icmp_type, test_icmp->icmp_code);
    printf(" %p", hdr_icmp);
    printf(":\n %d", sent_packet[0]);
    printf("args: %s : %s :%s", argv[0],argv[1], argv[2]);
    return 0;
}
