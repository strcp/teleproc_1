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
#include <pthread.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <route.h>
#include <connection.h>
#include <cmd_parser.h>

#define MYPORT "6666"	// the port users will be connecting to
#define ROUTER_PORT 6666

#define MAXSIZE 1000

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

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

int where_to_send(char *packet)
{
	struct iphdr *ip;
	struct udphdr *udp;
	char *data;

	ip = (struct iphdr *)packet;
	udp = (struct udphdr *)(packet + sizeof(struct iphdr));
	data = ((char *)udp + sizeof(struct udphdr));

	if (!sanity_check(ip)) {
		printf("Packet received with error, dropping.\n");
		cstats.lost_pkts++;
		return -1;
	}

	ip->check = 0;
	/* Router esta fazendo forward do pacote, subtrai ttl */
	ip->ttl--;
	ip->check = in_cksum((unsigned short *)ip, ip->tot_len);
	printf("New CRC: %X\n", ip->check);
	cstats.fw_pkts += ip->tot_len;

	return (send_data(packet));
}

void *listener()
{
	int sockfd;
	struct sockaddr_in router, client;
	size_t addr_len;
	char *buf;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		pthread_exit((void *)EXIT_FAILURE);
	}

	router.sin_family = AF_INET;
	router.sin_addr.s_addr = htonl(INADDR_ANY);
	router.sin_port = htons(ROUTER_PORT);

	if ((bind (sockfd, (struct sockaddr *)&router, sizeof(struct sockaddr_in))) < 0) {
		perror("bind");
		close(sockfd);
		pthread_exit((void *)EXIT_FAILURE);
	}

	while (1) {
		printf("Router awaiting for packages.\n");

		addr_len = sizeof(struct sockaddr_in);

		buf = malloc(MAXSIZE);
		memset(buf, 0, MAXSIZE);

		if ((recvfrom(sockfd, buf, MAXSIZE - 1 , 0, (struct sockaddr *)&client, &addr_len)) == -1) {
			perror("recvfrom");
			pthread_exit((void *)EXIT_FAILURE);
		}

		where_to_send(buf);
		sleep(1);	/* FIXME: Do we need this? */
	}
	close(sockfd);
	free(buf);

	pthread_exit((void*)EXIT_SUCCESS);
}

int main()
{
	pthread_t th;
	void *status;
	char *cmd, *hist;
	char prompt[] = "router> ";

	init_default_routes();

	pthread_create(&th, NULL, listener, NULL);

	if (fork()) {
		pthread_join(th, &status);
		printf("Show Statistics\n");

	} else {
		while (1) {
			cmd = readline(prompt);
			if (!strcmp(cmd, "exit") || !strcmp(cmd, "quit"))
				break;
			hist = strdup(cmd);
			parse_cmds(cmd);
			free(cmd);
			if (hist && *hist)
				add_history(hist);
		}
	}
	clear_history();
	cleanup_route_table();
	return 0;
}
