/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

/**
 * @ingroup connection
 * @{
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include <client.h>

#define ROUTER_PORT 6666
#define MAX_DATA_SIZE (15000 - (sizeof(struct iphdr) + sizeof(struct udphdr)))


/** Estruturas com os dados relativos às estatísticas de conexões do usuário. */
struct connection_stats
{
	unsigned int sent_pkts;		/**< Quantidade em bytes de opacotes enviados */
	unsigned int recv_pkts;		/**< Quantidade em bytes de opacotes recebidos */
	unsigned int lost_pkts;		/**< Quantidade de pacotes perdidos no recebimento */
	unsigned int fw_pkts;		/**< Quantidade em bytes de opacotes redirecionados */
};

/** Instância das estatísticas de conexão. */
struct connection_stats cstats;

void free_clientnet_info(struct clientnet_info *cinfo);
void ifconfig(char *iface);

struct clientnet_info *get_ifaces_info(void);
struct clientnet_info *get_iface_info(const char *iface);
struct udphdr *get_udp_packet(char *packet);
struct data_info *get_packet_data(char *packet);

struct udphdr *set_udp_packet(struct udphdr *udp,
								unsigned short src,
								unsigned short dest,
								const void *data,
								size_t len);
struct iphdr *set_ip_packet(struct iphdr *ip, const in_addr_t saddr, const in_addr_t daddr, size_t len);
char *create_packet(size_t data_length);

int init_network(void);
void _dump_packet_headers(char *pkt);

int send_udp_data(const char *daddr,
				const unsigned short dport,
				void *data,
				size_t len);
int send_data(const void *packet);

unsigned short in_cksum(unsigned short *addr, int len);

void enable_error(void);
void disable_error(void);
void dump_statistics(void);
/** @} */
