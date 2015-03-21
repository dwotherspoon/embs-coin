#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
 
#define DEST_MAC0	0x00
#define DEST_MAC1	0x11
#define DEST_MAC2	0x22
#define DEST_MAC3	0x44
#define DEST_MAC4	0x00
#define DEST_MAC5	0x60

#define TYPE 		0x55AC

#define BUF_LEN		1024

#define REQW		0x01
#define REQWR		0x02
#define SUBW		0x03
#define SUBWR		0x04

void comm_init(void);
uint8_t * get_block(void);
uint8_t submit_block(uint32_t);
