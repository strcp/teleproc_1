#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <ifaddrs.h>
#include <netdb.h>

#include <route.h>
#include <connection.h>
#include <data.h>

#define ROUTER_PORT 6666

static int error_enable = 0;

void enable_error(void)
{
	error_enable = 1;
}

void disable_error(void)
{
	error_enable = 0;
}

void dump_statistics(void)
{
	printf("Connection Statistics:\n");
	printf("Sent packages: %d bytes\n", cstats.sent_pkts);
	printf("Recived packages: %d bytes\n", cstats.recv_pkts);
	printf("Lost packages: %d\n", cstats.lost_pkts);
	if (cstats.fw_pkts)
		printf("Forwarded packages: %d bytes\n", cstats.fw_pkts);
}

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

void free_clientnet_info(struct clientnet_info *cinfo)
{
	struct clientnet_info *ptr, *next;

	if (!cinfo)
		return;

	for (ptr = cinfo; ptr; ptr = next) {
		if (ptr->iface)
			free(ptr->iface);
		next = ptr->next;
		free(ptr);
	}
}

struct clientnet_info *get_ifaces_info(void)
{
	struct ifaddrs *ifap, *tmpif;
	struct clientnet_info *cnet_info = NULL, *cnet_tmp;
	char addr[NI_MAXHOST], netmask[NI_MAXHOST];
	int s, family;

	if (getifaddrs(&ifap) == -1) {
		perror("getifaddrs");
		return NULL;
	}

	for (tmpif = ifap; tmpif; tmpif = tmpif->ifa_next) {
		family = tmpif->ifa_addr->sa_family;

		if (family == AF_INET) {
			if (!cnet_info) {
				cnet_info = malloc(sizeof(struct clientnet_info));
				cnet_tmp = cnet_info;
			} else {
				cnet_tmp->next = malloc(sizeof(struct clientnet_info));
				cnet_tmp = cnet_tmp->next;
			}
			memset(cnet_tmp, 0, sizeof(struct clientnet_info));
			if ((s = getnameinfo(tmpif->ifa_addr,
							sizeof(struct sockaddr_in),
							addr,
							NI_MAXHOST,
							NULL,
							0,
							NI_NUMERICHOST))) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				freeifaddrs(ifap);
				return NULL;
			}
			if ((s = getnameinfo(tmpif->ifa_netmask,
							sizeof(struct sockaddr_in),
							netmask,
							NI_MAXHOST,
							NULL,
							0,
							NI_NUMERICHOST))) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				freeifaddrs(ifap);
				return NULL;
			}
			cnet_tmp->iface = strdup(tmpif->ifa_name);
			cnet_tmp->addr.s_addr = inet_addr(addr);
			cnet_tmp->netmask.s_addr = inet_addr(netmask);
			cnet_tmp->netaddr.s_addr = inet_addr(addr) & inet_addr(netmask);
			cnet_tmp->bcastaddr.s_addr = inet_addr(addr) | ~inet_addr(netmask);
		}
	}
	freeifaddrs(ifap);

	return cnet_info;
}

struct clientnet_info *get_iface_info(const char *iface)
{
	struct clientnet_info *cinfo, *ptr, *cinfo_ret;

	if (!iface)
		return NULL;

	cinfo = get_ifaces_info();
	for (ptr = cinfo; ptr; ptr = ptr->next) {
		if (!strcmp(iface, ptr->iface)) {
			cinfo_ret = malloc(sizeof(struct clientnet_info));
			memset(cinfo_ret, 0, sizeof(struct clientnet_info));

			cinfo_ret->iface = strdup(ptr->iface);
			cinfo_ret->addr.s_addr = ptr->addr.s_addr;
			cinfo_ret->netmask.s_addr = ptr->netmask.s_addr;
			cinfo_ret->netaddr.s_addr = ptr->netaddr.s_addr;
			cinfo_ret->bcastaddr.s_addr = ptr->bcastaddr.s_addr;

			free_clientnet_info(cinfo);

			return cinfo_ret;
		}
	}
	free_clientnet_info(cinfo);

	return NULL;
}

void ifconfig(char *iface)
{
	struct clientnet_info *cinfo, *ptr;

	if (iface)
		cinfo = get_iface_info(iface);
	else
		cinfo = get_ifaces_info();

	for (ptr = cinfo; ptr; ptr = ptr->next) {
		printf("name: %s\n", ptr->iface);
		printf("inet addr: %s\n", inet_ntoa(ptr->addr));
		printf("mask addr: %s\n", inet_ntoa(ptr->netmask));
		printf("net addr: %s\n", inet_ntoa(ptr->netaddr));
		printf("bcast addr: %s\n\n", inet_ntoa(ptr->bcastaddr));
	}
	free_clientnet_info(cinfo);
}

struct udphdr *get_udp_packet(char *packet)
{
	return packet ? (struct udphdr *)(packet + sizeof(struct iphdr)) : NULL;
}

struct udphdr *set_udp_packet(struct udphdr *udp,
							unsigned short src,
							unsigned short dest,
							const void *data,
							size_t len)
{
	char *data_ptr;

	udp->source = htons(src);
	udp->dest = htons(dest);
	udp->len = htons(sizeof(struct udphdr) + len);

	data_ptr = ((char *)udp + sizeof(struct udphdr));
	memcpy(data_ptr, data, len);

	return udp;
}

struct iphdr *set_ip_packet(struct iphdr *ip, const in_addr_t saddr, const in_addr_t daddr, size_t len)
{
	if (!ip || !saddr || !daddr) {
		printf("Error setting ip packet.\n");
		return NULL;
	}

	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + len;
	ip->id = htons(666);
	ip->ttl = 64;
	ip->protocol = IPPROTO_UDP;
	ip->saddr = saddr;
	ip->daddr = daddr;
	ip->check = 0;
	ip->check = (unsigned short)in_cksum((unsigned short *)ip, ip->tot_len);
	/* Se envio de erro está habilitado, modificamos o CRC */
	if (error_enable) {
		if (random() % 2)
			ip->check = ip->check + 1;
	}

	return ip;
}

/* Returns 0 if error creating package and 1 on success */
char *create_packet(size_t data_length)
{
	char *packet;

	if (!(packet = (char *)malloc(sizeof(struct iphdr) + sizeof(struct udphdr) + data_length))) {
		printf("Error allocating packet.\n");
		return 0;
	}
	memset(packet, 0, sizeof(sizeof(struct iphdr) + sizeof(struct udphdr) + data_length));

	return packet;
}

void _dump_packet_headers(char *pkt)
{
	struct in_addr tmp;
	struct udphdr *udp;
	struct iphdr *ip;
	struct data_info *dinfo;

	ip = (struct iphdr *)pkt;
	printf("* IP packet dump *\n");
	tmp.s_addr = ip->saddr;
	printf("ip saddr: %s\n", inet_ntoa(tmp));
	tmp.s_addr = ip->daddr;
	printf("ip daddr: %s\n", inet_ntoa(tmp));
	printf("ip ver: %d\n", ip->version);
	printf("packet total length: %d\n", ip->tot_len);
	printf("packet id: %d\n", ip->id);
	printf("packet ttl: %d\n", ip->ttl);
	printf("ip using proto: %d\n", ip->protocol);
	printf("ip checksum: %X\n\n", ip->check);

	printf("* UDP packet dump *\n");
	udp = (struct udphdr *)(pkt + sizeof(struct iphdr));
	printf("dest port: %d\n", ntohs(udp->dest));
	printf("src port: %d\n", ntohs(udp->source));
	printf("length: %d\n", ntohs(udp->len));
	printf("checksum: %X\n\n", udp->check);

	printf("* DATA packet dump *\n");
	dinfo = (struct data_info *)((char *)udp + sizeof(struct udphdr));
	if (dinfo) {
		printf("File name: %s\n", dinfo->name);
		printf("File size: %d\n", dinfo->size);
	}
}

int send_data(const void *packet)
{
	struct clientnet_info *cinfo;
	struct route *croute;
	struct sockaddr_in si;
	int sockfd;
	struct iphdr *ip;
	struct udphdr *udp;
	struct in_addr tmp;

	ip = (struct iphdr *)packet;
	udp = (struct udphdr *)((char *)packet + sizeof(struct udphdr));

	if (!(croute = get_route_by_daddr(ip->daddr))) {
		printf("Error getting route rule\n");
		return -1;
	}

	if (!(cinfo = get_iface_info(croute->iface))) {
		printf("Error getting interface.\n");
		return -1;
	}
	tmp.s_addr = ip->daddr;
	printf("Destination: %s\n", inet_ntoa(tmp));
	tmp.s_addr = ip->saddr;
	printf("From: %s\n", inet_ntoa(tmp));
	printf("Gateway: %s\n", inet_ntoa(croute->gateway));

	memset(&si, 0, sizeof(si));
	si.sin_family = AF_INET;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		printf("Error creating socket.\n");
		return -1;
	}
	if (!croute->gateway.s_addr) {
		/* Quando o gateway escolhido for 0.0.0.0, o pacote deve ser enviado
		 * para o próprio ip escolhido na porta destino. */
		si.sin_port = htons(udp->dest);
		si.sin_addr.s_addr =  ip->daddr;
	} else {
		/* Quando houver um gateway, enviar para ele na porta de roteamento. */
		si.sin_port = htons(ROUTER_PORT);
		si.sin_addr.s_addr =  croute->gateway.s_addr;
	}
	if (sendto(sockfd, packet, ip->tot_len, 0, (struct sockaddr *)&si, sizeof(si)) == -1)
		printf("Error sending packet.\n");
	else
		cstats.sent_pkts += ip->tot_len;

	close(sockfd);

	free_clientnet_info(cinfo);

	return ip->tot_len;
}

int send_udp_data(const char *daddr,
				const unsigned short dport,
				const void *data,
				size_t len)
{
	struct clientnet_info *cinfo;
	struct route *croute;
	struct iphdr *ip;
	char *packet;
	struct udphdr *udp;
	int ret;

	packet = create_packet(len);
	udp = get_udp_packet(packet);
	ip = (struct iphdr *)packet;

	if (!(croute = get_route_by_daddr(ip->daddr))) {
		printf("Error getting route rule\n");
		return -1;
	}

	if (!(cinfo = get_iface_info(croute->iface))) {
		printf("Error getting interface.\n");
		return -1;
	}

	set_udp_packet(udp, dport, client_port, data, len);
	set_ip_packet(ip, cinfo->addr.s_addr, inet_addr(daddr), len);
	_dump_packet_headers(packet);
	ret = send_data(packet);

	free_clientnet_info(cinfo);
	free(packet);

	return ret;
}
