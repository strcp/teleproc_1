#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include <client.h>


void free_clientnet_info(struct clientnet_info *cinfo);
struct clientnet_info *get_ifaces_info(void);
struct clientnet_info *get_iface_info(const char *iface);
void ifconfig(char *iface);
struct cbphdr *get_cbp_packet(struct iphdr *ip);
struct cbphdr *set_cbp_packet(struct cbphdr *cbp, unsigned short src, unsigned short dest);
struct iphdr *set_ip_packet(struct iphdr *ip, const in_addr_t saddr, const in_addr_t daddr);
struct iphdr *create_packet(void);
int init_network(void);
void _dump_packet_headers(struct iphdr *pkt);
int send_cbp_data(const char *daddr,
				const unsigned short dport,
				const unsigned short sport,
				const void *data,
				size_t len);
