/*
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MYPORT "6666"	// the port users will be connecting to

#define MAXBUFLEN 1000

void _dump_packet_headers(struct iphdr *pkt)
{
	struct in_addr tmp;
	struct udphdr *udp;

	printf("* IP packet dump *\n");
	tmp.s_addr = pkt->saddr;
	printf("ip saddr: %s\n", inet_ntoa(tmp));
	tmp.s_addr = pkt->daddr;
	printf("ip daddr: %s\n", inet_ntoa(tmp));
	printf("ip ver: %d\n", pkt->version);
	printf("packet total length: %d\n", pkt->tot_len);
	printf("packet id: %d\n", pkt->id);
	printf("packet ttl: %d\n", pkt->ttl);
	printf("ip using proto: %d\n", pkt->protocol);
	printf("ip checksum: %1X\n\n", pkt->check);

	printf("* UDP packet dump *\n");
	udp = (struct udphdr *)(pkt + sizeof(struct iphdr));
	//udp = (struct udphdr *)(pkt + 20);
	printf("crc: %X\n", udp->check);
	printf("dest port: %d\n", ntohs(udp->dest));
	printf("src port: %d\n\n", ntohs(udp->source));
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	size_t addr_len;
	char s[INET6_ADDRSTRLEN];
	struct iphdr *ip;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	memset(buf, 0, MAXBUFLEN);

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("listener: got packet from %s\n",
					inet_ntop(their_addr.ss_family,
					get_in_addr((struct sockaddr *)&their_addr),
					s,
					sizeof(s)));
	printf("listener: packet is %d bytes long\n", numbytes);
	ip = (struct iphdr *)buf;
	_dump_packet_headers(ip);

	close(sockfd);

	return 0;
}
