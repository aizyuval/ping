#Ping utilty, pet project
##Workings:

1. get input from user, and fit that into the right structure, byte order, or 
2. create structures for later use. including icmp structure (header + data)
3. open a raw, ipv4, icmp protocol socket
4. iterate over sending the formed packet, recieving the returning one, encapsulating it, and verifying it.

##State:
Able to send echo requests, and recieve echo reply (although not 100% verifyable);

##Future:
Although there is some tiny things that should be added, I consider it done. This makes use of the basic principle of ping and then does some
tweaks. In other words, this is enough.
