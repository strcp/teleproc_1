/******************************************************************
 * Data : 17.06.201
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

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
	char *iface;					/**< Nome da interface */
	struct in_addr addr;			/**< Endereço da interface. */
	struct in_addr netmask;			/**< Netmask da interface */
	struct in_addr netaddr;			/**< Endereço de rede da interface */
	struct in_addr bcastaddr;		/**< Endereço de broadcast da interface */
	struct clientnet_info *next;    /**< Ponteiro para o próximo clientnet_info */
	/*@}*/
};

/**
 * @brief Porta aonde o cliente ira receber dados, src_port na hora do envio
 */
unsigned int client_port;
