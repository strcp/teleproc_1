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

static struct route *client_route = NULL;

int init_default_routes(void)
{
	/* TODO */
	struct clientnet_info *cinfo, *ptr;

	cinfo = get_ifaces_info();

	for (ptr = cinfo; ptr; ptr = ptr->next) {
		add_client_route(inet_ntoa(ptr->addr), "0.0.0.0", "255.255.255.255", ptr->iface);
	}

	return 1;
}

static struct route *find_route(const char *dest)
{
	struct route *ptr;

	if (!dest)
		return NULL;

	for (ptr = client_route; ptr; ptr = ptr->next) {
		if (!(ptr->dest.s_addr ^ (inet_addr(dest)))) {
			return ptr;
		}
	}

	return NULL;
}

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
	/* Verificando se o device está disponível para possuir roteamento */
	if (!(cinfo = get_iface_info(iface))) {
		printf("Device unknown or not connected to a IPv4 network.\n");
		free_clientnet_info(cinfo);
		return NULL;
	}
	free_clientnet_info(cinfo);

	if ((croute = find_route(dest))) {
		printf("Route already exists.\n");
		return croute;
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

int del_client_route(const char *dest)
{
	struct route *ptr;

	if (!dest) {
		printf("Error removing route.\n");
		return 0;
	}

	if ((ptr = find_route(dest))) {
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

void cleanup_route_table()
{
	struct route *r, *next;

	for (r = client_route; r; r = next) {
		next = r->next;
		free_route(r);
	}
}
