/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

/**
 * @defgroup route Funções relativas às rotas do usuário.
 * @brief Funções para manipulação das rotas do usuário.
 * @{
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>

#include <connection.h>
#include <route.h>

/** Estrutura com a tabela de roteamento do usuário. */
static struct route *client_route = NULL;

/**
 * Inicia as rotas padrão do usuário.
 * A ordem da tabela afeta o roteamento, a prioridade é sempre da mais antiga
 * @return void
 */
void init_default_routes(void)
{
	/* TODO */
	struct clientnet_info *cinfo, *ptr;

	cinfo = get_ifaces_info();

	for (ptr = cinfo; ptr; ptr = ptr->next) {
		add_client_route(inet_ntoa(ptr->addr), "0.0.0.0", "255.255.255.255", ptr->iface);
	}
}

/**
 * Avalia se uma rota já existe baseado no endereço destino.
 * @param dest Endereço de destino.
 * @return 0 se não existe rota e 1 se já existe a rota.
 */
static int route_exists(const char *dest)
{
	struct route *ptr;

	if (!dest)
		return 0;

	for (ptr = client_route; ptr; ptr = ptr->next) {
		if (!(ptr->dest.s_addr ^ (inet_addr(dest)))) {
			return 1;
		}
	}

	return 0;
}

/**
 * Adiciona uma nova rota à tabela de rotas do usuário.
 * @param dest Endereço de destino.
 * @param gateway Endereço de gateway.
 * @param genmask Endereço de netmask.
 * @param iface Nome da interface.
 * @return Ponteiro para a estrutura com a rota criada.
 */
struct route *add_client_route(const char *dest,
					 const char *gateway,
					 const char *genmask,
					 char *iface)
{
	struct route *croute, *ptr, *tmp;
	struct clientnet_info *cinfo;

	if (!iface) {
		printf("Error adding route.\n");

		return NULL;
	}

	if (!ip_check(dest) || !ip_check(gateway) || !ip_check(genmask)) {
		printf("Wrong IP|Gateway|Genmask\n");

		return NULL;
	}

	/* Verificando se o device está disponível para possuir roteamento */
	if (!(cinfo = get_iface_info(iface))) {
		printf("Device unknown or not connected to a IPv4 network.\n");
		free_clientnet_info(cinfo);
		return NULL;
	}
	free_clientnet_info(cinfo);

	if (route_exists(dest)) {
		printf("Route already exists.\n");
		return NULL;
	}

	croute = malloc(sizeof(struct route));
	memset(croute, 0, sizeof(struct route));

	croute->dest.s_addr = inet_addr(dest);
	croute->genmask.s_addr = inet_addr(genmask);
	croute->gateway.s_addr = inet_addr(gateway);
	croute->iface = strdup(iface);
	croute->next = NULL;
	croute->prev = NULL;

	if (!client_route) {
		client_route = croute;

		return croute;
	}

	for (ptr = client_route; ptr; ptr = ptr->next)
		tmp = ptr;

	tmp->next = croute;
	croute->prev = tmp;

	return croute;
}

/**
 * Remove uma rota da tabela de rotas do usuário.
 * @param dest Endereço de destino.
 * @return 0 em erro e !0 em sucesso.
 */
int del_client_route(const char *dest)
{
	struct route *ptr;

	if (!dest) {
		printf("Error removing route.\n");
		return 0;
	}

	if ((ptr = get_route_by_daddr(inet_addr(dest)))) {
		if (ptr == client_route) {		// Se o nodo for o primeiro
			if (ptr->next) {
				client_route = ptr->next;
				client_route->prev = NULL;
			} else {
				client_route = NULL;
			}
		} else {
			if (ptr->next) {
				ptr->prev->next = ptr->next;
				ptr->next->prev = ptr->prev;
			} else {
				ptr->prev->next = NULL;
			}
		}
		free_route(ptr);

		return 1;
	}
	printf("Route dosen't exists.\n");
	return 0;
}

/**
 * Exibe a table de roteamento do usuário.
 * @return void
 */
void show_route_table(void)
{
	/*TODO: Acertar a identação da tabela na exibição */
	struct route *ptr;

	printf("IP routing table\n");
	printf("Destination \tGateway \tGenmask \t\tIface\n");
	for (ptr = client_route; ptr; ptr = ptr->next) {
		printf("%s \t", inet_ntoa(ptr->dest));
		printf("%s \t", inet_ntoa(ptr->gateway));
		printf("%s \t", inet_ntoa(ptr->genmask));
		printf("\t%s\n", ptr->iface);
	}
}

/**
 * Localiza uma rota na tabela de rotas baseado no endereço de destino.
 * @param daddr Endereço de destino em binario.
 * @return Ponteiro para a estrutura com a rota ou NULL se nenhuma rota for
 * encontrada.
 */
struct route *get_route_by_daddr(const in_addr_t daddr)
{
	struct route *ptr, *def;

	def = NULL;
	for (ptr = client_route; ptr; ptr = ptr->next) {
		if (ptr->dest.s_addr == 0)
			def = ptr;
		else if ((daddr & ptr->genmask.s_addr) == ptr->dest.s_addr)
			return ptr;
	}

	return def;
}

/**
 * Libera a memória usada por uma estrutura de rota.
 * @param cr Ponteiro para estrutura de rota.
 * @return void
 */
void free_route(struct route *cr)
{
	if (!cr)
		return;

	if (cr->iface)
		free(cr->iface);
	cr->next = NULL;
	cr->prev = NULL;
	free(cr);
}

/**
 * Libera a memória alocada por toda a tabela de roteamento.
 * @return void
 */
void cleanup_route_table()
{
	struct route *r, *next;

	for (r = client_route; r; r = next) {
		next = r->next;
		free_route(r);
	}
}

/**
 * Verifica se é um endereço IPv4 válido
 * @return 1 caso seja válido
 * @return 0 caso seja inválido
 */
int ip_check(const char *ip)
{
	unsigned int n1, n2, n3, n4;

	if (sscanf(ip, "%u.%u.%u.%u", &n1, &n2, &n3, &n4) != 4)
		return 0;
	if ((n1 <= 255) && (n2 <= 255) && (n3 <= 255) && (n4 <= 255))
		return 1;
	return 0;
}
/** @} */
