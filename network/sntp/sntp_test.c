/*
 * @*************************************: 
 * @FilePath: /network/sntp/sntp_test.c
 * @version: 
 * @Author: dof
 * @Date: 2023-06-01 15:25:42
 * @LastEditors: dof
 * @LastEditTime: 2023-06-01 15:48:29
 * @Descripttion: 
 * @**************************************: 
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <error.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull

typedef struct 
{
    uint8_t li_vn_mode;      // Leap indicator, version and mode
    uint8_t stratum;         // Stratum level of the local clock
    uint8_t poll;            // Maximum interval between successive messages
    uint8_t precision;       // Precision of the local clock
    uint32_t root_delay;     // Total round trip delay time
    uint32_t root_dispersion;// Max error aloud from primary clock source
    uint32_t ref_id;         // Reference clock identifier
    uint32_t ref_ts_sec;     // Reference timestamp seconds
    uint32_t ref_ts_frac;    // Reference timestamp fraction of a second
    uint32_t orig_ts_sec;    // Origination timestamp seconds
    uint32_t orig_ts_frac;   // Origination timestamp fraction of a second
    uint32_t rx_ts_sec;      // Received timestamp seconds
    uint32_t rx_ts_frac;     // Received timestamp fraction of a second
    uint32_t tx_ts_sec;      // Transmission timestamp seconds
    uint32_t tx_ts_frac;     // Transmission timestamp fraction of a second
} ntp_packet;


int main(int argc, char *argv[])
{
    int sockfd, n;
    uint8_t buffer[48];
    struct sockaddr_in serv_addr;
    struct timeval tv;
    ntp_packet *pkt;
    time_t tx_t, rx_t;
    double offset, delay;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <ntp server>\n", argv[0]);
        exit(1);
    }

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("ERROR opening socket");
    }

    // Set socket timeout
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("ERROR setting socket timeout");
    }

	// set SO_BROADCAST
	int optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int)) == -1) 
	{
		perror("setsockopt SO_BROADCAST: \n");
		close(sockfd);
		return -1;
	};

    // Set server address
    memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_port = htons(4444);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(123); // NTP port

    // Initialize request packet
    memset(buffer, 0, sizeof(buffer));
    pkt = (ntp_packet *) buffer;
    pkt->li_vn_mode = 0x1b; // NTPv3, client mode

    // Send request packet
    n = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (n < 0) {
        perror("ERROR sending packet");
    }


	int tries = 0;
	while ((n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL)) < 0) {
		if (n == 11 || n == 11) {
			if (++tries > 3) {
				perror("ERROR receiving packet: timeout");
			} else {
				printf("No data available, waiting...\n");
				sleep(1);
			}
		} else {
			perror("ERROR receiving packet");
		}
	}

	// Get transmit and receive timestamps
	pkt = (ntp_packet *) buffer;
	tx_t = ntohl(pkt->tx_ts_sec) - NTP_TIMESTAMP_DELTA;
	rx_t = time(NULL);

	// Calculate offset and delay
	offset = (rx_t - (double)tx_t) / 2;
	delay = ((double)ntohl(pkt->rx_ts_frac) - (double)ntohl(pkt->tx_ts_frac)) / 4294967296.0;

	// Print results
	printf("Offset: %.3lf s\n", offset);
	printf("Delay: %.3lf s\n", delay);

#if 0
    // Receive response packet
    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
    if (n < 0) {
        perror("ERROR receiving packet");
    }

    // Get transmit and receive timestamps
    pkt = (ntp_packet *) buffer;
    tx_t = ntohl(pkt->tx_ts_sec) - NTP_TIMESTAMP_DELTA;
    rx_t = time(NULL);

    // Calculate offset and delay
    offset = (rx_t - (double)tx_t) / 2;
    delay = ((double)ntohl(pkt->rx_ts_frac) - (double)ntohl(pkt->tx_ts_frac)) / 4294967296.0;

    // Print results
    printf("Offset: %.3lf s\n", offset);
    printf("Delay: %.3lf s\n", delay);
#endif
    return 0;
}