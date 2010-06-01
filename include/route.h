/******************************************************************
 * Data : 17.06.201
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>


/**
 * @brief Estrutura com dados de rota.
 */
struct route
{
	/*@{*/
	struct in_addr dest;		/**< Endereço de destino */
	struct in_addr genmask;		/**< Endereço de netmask */
	struct in_addr gateway;		/**< Endereço de gateway */
	char *iface;				/**< Nome da interface de saída */
	struct route *next;			/**< Ponteiro para a próxima estrutura de rota */
	struct route *prev;			/**< Ponteiro para a estrutura de rota anterior */
	/*@}*/
};

struct route *add_client_route(const char *dest,
								const char *gateway,
								const char *genmask,
								char *iface);
int del_client_route(const char *dest);
struct route *get_route_by_daddr(const in_addr_t daddr);

void free_route(struct route *cr);
void show_route_table(void);
void cleanup_route_table(void);
int init_default_routes(void);
