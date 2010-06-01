/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

/**
 * @defgroup listener Funções de recebimento de dados.
 * @brief Funções relativas ao recebimento de pacotes.
 * @{
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

#include <route.h>
#include <listener.h>
#include <connection.h>
#include <cmd_parser.h>
#include <data.h>

/** Flag para sinalizar a saída ou não da thread. */
int exit_thread = 0;

/**
 * Função chamada para sinalizar a saída para a thread.
 * @return void
 */
void thread_exit(){
	exit_thread = 1;
}

/**
 * Checa a sanidade de um pacote.
 * @param ip Ponteiro para o ip header do pacote.
 * @return 0 se o CRC é inválido e 1 quando válido.
 */
int sanity_check(struct iphdr *ip)
{
	unsigned short old_check;

	old_check = ip->check;
	ip->check = 0;

	if (old_check == in_cksum((unsigned short *)ip, ip->tot_len)) {
		ip->check = old_check;

		return 1;
	}
	ip->check = old_check;

	return 0;
}

/**
 * Avalia se o pacote deve ser redirecionado ou salvo.
 * @param packet Ponteiro para o pacote.
 * @param usage_type Tipo do cliente que está chamando a função.
 * @return 0 em falha e !0 em sucesso.
 */
int where_to_send(char *packet, usage_type_t usage_type)
{
	struct iphdr *ip;
	struct udphdr *udp;
	struct data_info *data;
	struct in_addr tmp;
	int ret;

	ip = (struct iphdr *)packet;
	udp = get_udp_packet(packet);

	if (!sanity_check(ip)) {
		printf("Packet received with error, dropping.\n");
		cstats.lost_pkts++;
		return 0;
	}

	switch (usage_type) {
		case ROUTER_USAGE:
			//_dump_packet_headers(packet);
			/* Router esta fazendo forward do pacote, subtrai ttl */
			ip->ttl--;
			ip->check = 0;
			ip->check = in_cksum((unsigned short *)ip, ip->tot_len);
			cstats.fw_pkts += ip->tot_len;
			ret = send_data(packet);
			printf("Forwarding packet:\n");
			printf("Packet: %d bytes\n", ip->tot_len);
			tmp.s_addr = ip->saddr;
			printf("From: %s\n", inet_ntoa(tmp));
			tmp.s_addr = ip->daddr;
			printf("To: %s\n", inet_ntoa(tmp));
			break;
		default:
			data = get_packet_data(packet);
			ret = save_data(data);
			cstats.recv_pkts += ip->tot_len;
			printf("Data received:\n");
			printf("Packet: %d bytes\n", ip->tot_len);
			tmp.s_addr = ip->saddr;
			printf("From: %s\n", inet_ntoa(tmp));
			printf("File Name: %s\n", data->name);
			printf("File size: %ld bytes\n", data->size);
			break;
	}

	return ret;
}

/**
 * Thread que aguarda o recebimento de dados.
 * @param usage_type Tipo do cliente que está chamando a função.
 * @return void
 */
void *listener(void *usage_type)
{
	int sockfd;
	struct sockaddr_in server, client;
	size_t addr_len;
	char *buf;


	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		pthread_exit((void *)EXIT_FAILURE);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	switch ((usage_type_t)usage_type) {
		case ROUTER_USAGE:
			server.sin_port = htons(ROUTER_PORT);
			break;
		default:
			server.sin_port = htons(client_port);
			break;
	}

	if ((bind (sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) < 0) {
		perror("bind");
		close(sockfd);
		pthread_exit((void *)EXIT_FAILURE);
	}

	while (!exit_thread) {
		printf("Awaiting for packets.\n");

		addr_len = sizeof(struct sockaddr_in);

		buf = malloc(MAXSIZE);
		memset(buf, 0, MAXSIZE);

		if ((recvfrom(sockfd, buf, MAXSIZE - 1 , 0, (struct sockaddr *)&client, &addr_len)) == -1) {
			perror("recvfrom");
			pthread_exit((void *)EXIT_FAILURE);
		}

		where_to_send(buf, (usage_type_t)usage_type);
		sleep(1);
	}
	close(sockfd);
	free(buf);

	pthread_exit((void*)EXIT_SUCCESS);
}
/** @} */
