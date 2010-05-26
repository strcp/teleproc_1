#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include <client.h>


void free_clientnet_info(struct clientnet_info *cinfo);
struct clientnet_info *get_ifaces_info(void);
struct clientnet_info *get_iface_info(const char *iface);
void ifconfig(char *iface);
struct udphdr *get_udp_packet(char *packet);
struct udphdr *set_udp_packet(struct udphdr *udp,
								unsigned short src,
								unsigned short dest,
								const void *data,
								size_t len);
struct iphdr *set_ip_packet(struct iphdr *ip, const in_addr_t saddr, const in_addr_t daddr);
char *create_packet(size_t data_length);
int init_network(void);
void _dump_packet_headers(char *pkt);
int send_udp_data(const char *daddr,
				const unsigned short dport,
				const unsigned short sport,
				const void *data,
				size_t len);
