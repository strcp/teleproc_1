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

#include <route.h>
#include <connection.h>

#define MYPORT "6666"	// the port users will be connecting to
#define ROUTER_PORT 6666

#define MAXBUFLEN 1000
#if 0
	printf("recalc: %X\n\n", (unsigned short)in_cksum((unsigned short *)ip, ip->tot_len));
#endif
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void where_to_send(char* msg){
	struct iphdr *ip;
	struct udphdr *udp;
	char *data;
	struct in_addr aux;
	unsigned short old_check;
	struct route *croute;
	struct clientnet_info *cinfo;
	struct sockaddr_in si;
	int sockfd;

	ip = (struct iphdr *)msg;
	udp = (struct udphdr *)(msg + sizeof(struct iphdr));
	data = ((char *)udp + sizeof(struct udphdr));
	
	old_check = ip->check;
	ip->check = 0;
	if(old_check == (unsigned short)in_cksum((unsigned short *)ip, ip->tot_len)){
		printf("IP CHECK OK: %X\n", old_check);
		ip->ttl--;
		ip->check = (unsigned short)in_cksum((unsigned short *)ip, ip->tot_len);
		printf("New CRC: %X\n", ip->check);
		
		aux.s_addr = ip->daddr;
		printf("Enviar pacote para %s\n", inet_ntoa(aux));
		printf("data = %s\n", data);
		
		if (!(croute = get_route_by_daddr(inet_ntoa(aux)))) {
         	       printf("Error getting route rule\n");
                	return -1;
        	}
        	if (!(cinfo = get_iface_info(croute->iface))) {
                	printf("Error getting interface.\n");
                	return -1;
        	}
		printf("Gateway: %s\n", inet_ntoa(croute->gateway));
		
		memset(&si, 0, sizeof(struct sockaddr_in));
		si.sin_family = AF_INET;
		if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
			printf("ERROR: creating socket\n");
			return -1;
		}
		if(!croute->gateway.s_addr){
			printf("Rede Local\n");
			si.sin_port = udp->dest;
			si.sin_addr.s_addr = ip->daddr; 
			printf("Enviar para %s:%d\n", inet_ntoa(si.sin_addr), ntohs(si.sin_port));
		} else {
			printf("Rede Remota\n");
			si.sin_port = htons(ROUTER_PORT);
			si.sin_addr.s_addr = croute->gateway.s_addr;
			printf("Enviar para %s:%d\n", inet_ntoa(si.sin_addr), ntohs(si.sin_port));
		}
		if (sendto(sockfd, msg, sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data), 0, (struct sockaddr *)&si, sizeof(si)) == -1)
                	printf("Error sending packet.\n");
			printf("Data Sent\n");
		close(sockfd);
		free_clientnet_info(cinfo);
	}
	//_dump_packet_headers(msg);
}

void *listener()
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	//char buf[MAXBUFLEN];
	size_t addr_len;
	//char s[INET6_ADDRSTRLEN];
	char *buf;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP


	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		pthread_exit((void*)1);
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
		pthread_exit((void *)2);
	}

	freeaddrinfo(servinfo);
	while(1){
		printf("listener: waiting to recvfrom...\n");

		addr_len = sizeof their_addr;

		buf = malloc(MAXBUFLEN);
		memset(buf, 0, MAXBUFLEN);

		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
						(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		
		where_to_send(buf);
		/*printf("listener: got packet from %s\n",
				inet_ntop(their_addr.ss_family,
					get_in_addr((struct sockaddr *)&their_addr),
					s,
					sizeof(s)));
		
		printf("listener: packet is %d bytes long\n", numbytes);
		_dump_packet_headers(buf);*/
		sleep(1);
	}
	close(sockfd);
	free(buf);

	pthread_exit((void*) 0);
}
int main(){
	pthread_t th;
	void *status;
	char cmd[123];
	
	ifconfig("eth0");
	add_client_route("192.168.1.0","0.0.0.0", "255.255.255.0","eth0");
	add_client_route("0.0.0.0", "192.168.1.1", "0.0.0.0", "eth0");
	add_client_route("127.0.0.1", "0.0.0.0", "255.255.255.255", "eth0");

	pthread_create(&th, NULL, listener, NULL);
	
	if(fork()){
		pthread_join(th, &status);
		printf("Show Statistics\n");

	}else{
		//Child still have access to opac instances
		show_route_table();
		while(1){
			printf("router> ");
			scanf("%s",cmd);
		}
	}
	return 0;
}