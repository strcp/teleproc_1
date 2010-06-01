#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

/**
 * @brief Client Net Info
 */
struct clientnet_info
{
	/*@{*/
	char *iface;			/**< the interface comunication */
	struct in_addr addr;		/**< the interface address */
	struct in_addr netmask;		/**< the netmask of the address */
	struct in_addr netaddr;		/**< the network address */
	struct in_addr bcastaddr;	/**< the broadcast address */
	struct clientnet_info *next;    /**< the next clientnet_info node */
	/*@}*/
};

/** 
 * @brief Porta aonde o cliente ira receber dados, src_port na hora do envio 
 */
unsigned int client_port;
