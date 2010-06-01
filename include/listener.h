/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/


/**
 * @ingroup listener
 * @brief Estruturas e protótipos relacionados ao recebimento de dados através
 * da rede.
 * @{
 */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

/** Porta de operação do roteador. */
#define ROUTER_PORT 6666
#define MAXSIZE 100000

typedef enum usage_type_t {
	ROUTER_USAGE = 0,
	CLIENT_USAGE
} usage_type_t;

int sanity_check(struct iphdr *ip);
void *listener(void *usage_type);
int where_to_send(char *packet, usage_type_t usage_type);
void thread_exit();
/** @} */
