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
#include <string.h>

#define MYPORT "6666"	// the port users will be connecting to

#define MAXBUFLEN 1000

unsigned short in_cksum(unsigned short *addr, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	answer = ~sum;

	return (answer);
}

void _dump_packet_headers(char *packet)
{
	struct in_addr tmp;
	struct udphdr *udp;
	struct iphdr *ip;
	char *fuu;

	printf("* IP packet dump *\n");
	ip = (struct iphdr *)packet;
	tmp.s_addr = ip->saddr;
	printf("ip saddr: %s\n", inet_ntoa(tmp));
	tmp.s_addr = ip->daddr;
	printf("ip daddr: %s\n", inet_ntoa(tmp));
	printf("ip ver: %d\n", ip->version);
	printf("packet total length: %d\n", ip->tot_len);
	printf("packet id: %d\n", ip->id);
	printf("packet ttl: %d\n", ip->ttl);
	printf("ip using proto: %d\n", ip->protocol);
	printf("ip checksum: %X\n", ip->check);
	ip->check = 0;
	printf("recalc: %X\n\n", (unsigned short)in_cksum((unsigned short *)ip, ip->tot_len));


	printf("* UDP packet dump *\n");
	udp = (struct udphdr *)(packet + sizeof(struct iphdr));
	printf("crc: %X\n", udp->check);
	printf("dest port: %d\n", ntohs(udp->dest));
	printf("src port: %d\n", ntohs(udp->source));

	printf("* DATA packet dump *\n");
	fuu = ((char *)udp + sizeof(struct udphdr));
	printf("DATA: (%s)\n", fuu);
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
	//char buf[MAXBUFLEN];
	size_t addr_len;
	char s[INET6_ADDRSTRLEN];
	char *buf;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP


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

	buf = malloc(MAXBUFLEN);
	memset(buf, 0, MAXBUFLEN);

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
	_dump_packet_headers(buf);

	close(sockfd);
	free(buf);

	return 0;
}
