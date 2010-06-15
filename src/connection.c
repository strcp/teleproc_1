/******************************************************************
 * Data	: 17.06.2010
 * Disciplina	: Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores	: Cristiano Bolla Fernandes
 *			: Benito Michelon
 *****************************************************************/

/**
 * @defgroup connections Funções de conexão
 * @brief Funções relativas ao envio e modelagem de pacotes
 * @{
 */
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
#include <data_structs.h>


/** Flag que indica se a inserção de erros está ativada ou desativada */
static int error_enable = 0;


/**
 * Ativa a inserção de erros no pacote a ser enviado.
 * @return void
 */
void enable_error(void)
{
	error_enable = 1;
}

/**
 * Desativa a inserção de erros no pacote a ser enviado.
 * @return void
 */
void disable_error(void)
{
	error_enable = 0;
}

/**
 * Expõe os dados de estatísticas de conexão para o usuário.
 * @return void
 */
void dump_statistics(void)
{
	printf("Connection Statistics:\n");
	printf("Sent data: %d bytes\n", cstats.sent_pkts);
	printf("Recived data: %d bytes\n", cstats.recv_pkts);
	printf("Lost packages: %d\n", cstats.lost_pkts);
	if (cstats.fw_pkts)
		printf("Forwarded data: %d bytes\n", cstats.fw_pkts);
}

/**
 * Calcula e retorna o valor de check de sanidade de um pacote.
 * @param addr Ponteiro para o pacote.
 * @param len Tamanho do pacote.
 * @return Retorna o valor do CRC calculado.
 */
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

/**
 * Libera a memória utilizada pela lista de informações de redes do usuário.
 * @param cinfo Ponteiro para a lista de redes.
 * @return void
 */
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

/**
 * Recebe as informações sobre as interfaces com endereço IPv4 do usuário.
 * @return Ponteiro para lista de redes do usuário.
 * @return NULL, se não houver interfaces.
 */
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

/**
 * Recebe as informações sobre uma interface com endereço IPv4 específica do usuário.
 * @param iface Nome da interface desejada
 * @return Ponteiro para as estrutura com informações de rede da interface específica.
 * @return NULL, se não houver interface.
 */
struct clientnet_info *get_iface_info(const char *iface)
{
	struct clientnet_info *cinfo, *ptr, *cinfo_ret;

	if (!iface)
		return NULL;

	cinfo = get_ifaces_info();
	for (ptr = cinfo; ptr; ptr = ptr->next) {
		if (!strcmp(iface, ptr->iface)) {
			if (!(cinfo_ret = malloc(sizeof(struct clientnet_info)))) {
				perror("get_iface_info: malloc");
				return NULL;
			}
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

/**
 * Comando para listar informações sobre as interfaces com rede IPv4 ativas.
 * @param iface Nome da interface desejada ou NULL para listar todas.
 * @return void
 */
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

/**
 * Retorna o header UDP do pacote.
 * @param packet Ponteiro para o pacote.
 * @return Ponteiro para a estrutura UDP contida no pacote.
 */
struct udphdr *get_udp_packet(char *packet)
{
	return packet ? (struct udphdr *)(packet + sizeof(struct iphdr)) : NULL;
}

/**
 * Retorna o ponteiro para estrutura de dados do pacote.
 * Essa função modifica dados do pacote, o que modifica seu CRC.
 * @param packet Ponteiro para o pacote.
 * @return Ponteiro para a estrutura de dados contida no pacote.
 */
struct data_info *get_packet_data(char *packet)
{
	struct data_info *data;

	if (!packet)
		return NULL;

	data = (struct data_info *)(packet + sizeof(struct iphdr) + sizeof(struct udphdr));

	return data;
}

/**
 * Seta os dados no header UDP.
 * @param udp Ponteiro para a estrutura de header UDP a ser setada.
 * @param dest Porta destino.
 * @param src Porta origem.
 * @param data Ponteiro para a estrutura de dados.
 * @param len Tamanho do pacote de dados que será adicionado ao UDP.
 * @return Ponteiro para a estrutura UDP devidamente setada.
 */
struct udphdr *set_udp_packet(struct udphdr *udp,
							unsigned short dest,
							unsigned short src,
							const void *data,
							size_t len)
{
	struct data_info *data_ptr;

	udp->source = src;
	udp->dest = dest;
	udp->len = sizeof(struct udphdr) + len;

	data_ptr = (struct data_info *)((char *)udp + sizeof(struct udphdr));
	memcpy((char *)data_ptr, (char *)((struct data_info *)data), len);

	return udp;
}

/**
 * Seta os dados no header IP.
 * @param ip Ponteiro para a estrutura de header IP a ser setada.
 * @param saddr Endereço IP de origem.
 * @param daddr Endereço IP de destino.
 * @param len Tamanho total do pacote que será enviado pelo IP (IP + UDP + DATA).
 * @return Ponteiro para a estrutura IP devidamente setada.
 */
struct iphdr *set_ip_packet(struct iphdr *ip, const in_addr_t saddr, const in_addr_t daddr, size_t len)
{
	if (!ip || !saddr || !daddr) {
		printf("Error setting ip packet.\n");
		return NULL;
	}
	/* FIXME: packet fragmentation */
	if (len > 0xFFFF) {
		printf("Package too big.\n");
		return NULL;
	}

	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + len;
	ip->id = htons(666);
	ip->ttl = IPDEFTTL;
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

/**
 * Cria um pacote zerado que será enviado pela rede.
 * @param data_length Tamanho total do pacote que será enviado pelo IP (IP + UDP + DATA).
 * @return Ponteiro para o pacote criado.
 */
char *create_packet(size_t data_length)
{
	char *packet;

	if (!(packet = (char *)malloc(sizeof(struct iphdr) +
								sizeof(struct udphdr) +
								data_length))) {
		printf("Error allocating packet.\n");
		return 0;
	}
	memset(packet, 0, sizeof(struct iphdr) + sizeof(struct udphdr) + data_length);

	return packet;
}

/**
 * Exibe os dados do pacote.
 * @param pkt Ponteiro para o pacote.
 * @return void
 */
void _dump_packet_headers(char *pkt)
{
	struct in_addr tmp;
	struct udphdr *udp;
	struct iphdr *ip;
	//struct data_info *dinfo;

	ip = (struct iphdr *)pkt;
	printf("* IP packet dump *\n");
	tmp.s_addr = ip->saddr;
	printf("ip saddr: %s\n", inet_ntoa(tmp));
	tmp.s_addr = ip->daddr;
	printf("ip daddr: %s\n", inet_ntoa(tmp));
	printf("ip ver: %d\n", ip->version);
	printf("packet total length: %d bytes\n", ip->tot_len);
	printf("packet id: %d\n", ip->id);
	printf("packet ttl: %d\n", ip->ttl);
	printf("ip using proto: %d\n", ip->protocol);
	printf("ip checksum: %X\n\n", ip->check);

	udp = (struct udphdr *)(pkt + sizeof(struct iphdr));
	printf("* UDP packet dump *\n");
	printf("dest port: %d\n", ntohs(udp->dest));
	printf("src port: %d\n", ntohs(udp->source));
	printf("length: %d bytes\n", ntohs(udp->len));
	printf("checksum: %X\n\n", udp->check);
/*
	dinfo = (struct data_info *)((char *)udp + sizeof(struct udphdr));
	if (dinfo) {
		printf("* DATA packet dump *\n");
		printf("File name: %s\n", (char *)dinfo + sizeof(struct data_info));
		printf("File name size: %ld bytes\n\n", dinfo->name_size);
		printf("File size: %ld bytes\n\n", dinfo->data_size);
	}
*/
}

/**
 * Envia um pacote já configurado pela rede.
 * @param packet Ponteiro para o pacote.
 * @return < 0 em falha, número de bytes enviados em sucesso.
 */
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
	udp = get_udp_packet((char *)packet);

	if (!(croute = get_route_by_daddr(ip->daddr))) {
		printf("Error getting route rule\n");
		return -1;
	}

	if (!(cinfo = get_iface_info(croute->iface))) {
		printf("Error getting interface.\n");
		return -1;
	}
	printf("Sending Data:\n");
	tmp.s_addr = ip->daddr;
	printf("Destination: %s:%d\n", inet_ntoa(tmp), udp->dest);
	tmp.s_addr = ip->saddr;
	printf("From: %s:%d\n", inet_ntoa(tmp), udp->source);
	printf("Gateway: %s\n", inet_ntoa(croute->gateway));
	printf("Packet size: %d bytes\n", ip->tot_len);

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
		si.sin_addr.s_addr = ip->daddr;
	} else {
		/* Quando houver um gateway, enviar para ele na porta de roteamento. */
		si.sin_port = htons(ROUTER_PORT);
		si.sin_addr.s_addr = croute->gateway.s_addr;
	}
	if (sendto(sockfd, packet, ip->tot_len, 0, (struct sockaddr *)&si, sizeof(si)) == -1)
		perror("Error sending packet: ");
	else
		cstats.sent_pkts += ip->tot_len;

	close(sockfd);

	free_clientnet_info(cinfo);

	return ip->tot_len;
}

/**
 * Configura um pacote e envia pela rede.
 * @param daddr Endereço de destino.
 * @param dport Porta de origem.
 * @param data Ponteiro para os dados a serem enviados.
 * @param len Tamanho do pacote a ser enviado.
 * @return < 0 em falha, número de bytes enviados em sucesso.
 */
int send_udp_data(const char *daddr,
				const unsigned short dport,
				void *data,
				size_t len)
{
	struct clientnet_info *cinfo;
	struct route *croute;
	struct iphdr *ip;
	char *packet;
	struct udphdr *udp;
	int ret;
	struct fragment_list *frags, *f;

	if (len > MAX_DATA_SIZE) {
		f = NULL;
		frags = NULL;
		frags = fragment_packet(data);
		for (f = frags; f; f = f->next) {
			ret += send_udp_data(daddr, dport, f->frag, f->frag->tot_len);
			free(f->frag);
		}
		free(frags);
		return ret;
	} else {
		packet = create_packet(len);
		/* TODO: Fragmenta o pacote */
	}
	udp = get_udp_packet(packet);
	ip = (struct iphdr *)packet;

	if (!(croute = get_route_by_daddr(inet_addr(daddr)))) {
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
	usleep(500);
	return ret;
}
/** @} */
