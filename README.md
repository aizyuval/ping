State: Able to send 1 echo request packet without attached data. Only header.
TODO:
1. extend to sending data
1.1 fix checksum
2. extend to recieving echo reply message
3. extend to sending multiple echo req and rep.
4. make everything work while crafting the ip hdr by hand (IPPROTO_RAW, instead of IPPROTO_ICMP)
