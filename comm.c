#include <comm.h>

/* A mess of system calls and socket torture to send and receive raw frames
	to the work server. 
*/
//https://gist.github.com/austinmarton/1922600
//https://gist.github.com/austinmarton/2862515
// macchanger --mac=DE:AD:BE:EF:CA:FE interface

char * if_name = "eth0";

int sockfd;

/* TX things */
struct ifreq if_idx;
struct ifreq if_mac;
struct sockaddr_ll socket_address;
uint8_t tx_buf[BUF_LEN];
struct ether_header * tx_eh = (struct ether_header *) tx_buf;

/* RX things */
int sock_opt;
uint8_t rx_buf[BUF_LEN];
struct ether_header * rx_eh = (struct ether_header *) rx_buf;

// Initialise data structures etc.
void comm_init(void) {
	/* Open socket with desired packet type */
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(TYPE))) == -1) perror("socket");

	/* Get index of interface */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, if_name, IFNAMSIZ - 1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) perror("SIOCGIFINDEX");

	/* Get MAC of the interface */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, if_name, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0) perror("SIOCGIFHWADDR");

	printf("My MAC is: %02x:%02x:%02x:%02x:%02x:%02x\n", 
		((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0], ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1], ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2], 
		((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3], ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4], ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5]);

	/* Clear buffer + set header things */
	memset(tx_buf, 0, BUF_LEN);
	/* Ethernet header */
	tx_eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
	tx_eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
	tx_eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
	tx_eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
	tx_eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
	tx_eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
	tx_eh->ether_dhost[0] = DEST_MAC0;
	tx_eh->ether_dhost[1] = DEST_MAC1;
	tx_eh->ether_dhost[2] = DEST_MAC2;
	tx_eh->ether_dhost[3] = DEST_MAC3;
	tx_eh->ether_dhost[4] = DEST_MAC4;
	tx_eh->ether_dhost[5] = DEST_MAC5;
	tx_eh->ether_type = htons(TYPE);

	/* Index of the network device */
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
	socket_address.sll_addr[0] = DEST_MAC0;
	socket_address.sll_addr[1] = DEST_MAC1;
	socket_address.sll_addr[2] = DEST_MAC2;
	socket_address.sll_addr[3] = DEST_MAC3;
	socket_address.sll_addr[4] = DEST_MAC4;
	socket_address.sll_addr[5] = DEST_MAC5;

	/* Setup receive - Could use IFF_PROMISC? */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof sock_opt) == -1) perror("SO_REUSEADDR");
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, &sock_opt, sizeof sock_opt) == -1) perror("SO_BINDTODEVICE");
}

// Send a frame
void comm_send(uint8_t * pl, uint8_t len) {
	int tx_len = sizeof(struct ether_header);
	uint8_t i;
	for (i = 0; i < len; i++) {
		tx_buf[tx_len + i] = pl[i];
	}
	tx_len += len;
	if (sendto(sockfd, tx_buf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) puts("Send failed\n");
}

// Receive a frame
void comm_recv(void) {
	while (1) {
		recvfrom(sockfd, rx_buf, BUF_LEN, 0, NULL, NULL);
		// Check that the MAC is right.
		if (rx_eh->ether_shost[0] == DEST_MAC0 &&
			rx_eh->ether_shost[1] == DEST_MAC1 &&
			rx_eh->ether_shost[2] == DEST_MAC2 &&
			rx_eh->ether_shost[3] == DEST_MAC3 &&
			rx_eh->ether_shost[4] == DEST_MAC4 &&
			rx_eh->ether_shost[5] == DEST_MAC5) break;
	}
}

uint8_t req_get[] = {REQW};

// returns a pointer to difficulty, bytes to follow.
uint8_t * get_block(void) {
	comm_send(req_get, 1);
	comm_recv();
	return &rx_buf[15];
}

uint8_t req_sub[] = {SUBW, 0x00, 0x00, 0x00, 0x00};

uint8_t submit_block(uint32_t solution) {
	req_sub[1] = solution >> 24;
	req_sub[2] = solution >> 16;
	req_sub[3] = solution >> 8;
	req_sub[4] = solution;
	comm_send(req_sub, 5);
	comm_recv();
	return rx_buf[15];
}
