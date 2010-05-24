#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>

#include <cbproto.h>
#include <connection.h>
#include <route.h>

static struct route *client_route = NULL;

int init_default_routes(void)
{
	/* TODO */
	struct clientnet_info *cinfo, *ptr;

	cinfo = get_ifaces_info();

	for (ptr = cinfo; ptr; ptr = ptr->next) {
		add_client_route(inet_ntoa(ptr->addr), "10.0.2.1", "0.0.0.0", "eth1");
	}

	return 1;
}

struct route *add_client_route(const char *dest,
					 const char *gateway,
					 const char *genmask,
					 char *iface)
{
	struct route *croute, *ptr, *tmp;

	if (!iface) {
		printf("Error adding route.\n");

		return NULL;
	}

	croute = malloc(sizeof(struct route));
	memset(croute, 0, sizeof(struct route));

	croute->dest.s_addr = inet_addr(dest);
	croute->genmask.s_addr = inet_addr(genmask);
	croute->gateway.s_addr = inet_addr(gateway);
	croute->iface = strdup(iface);

	if (!client_route) {
		client_route = croute;

		return croute;
	}

	for (ptr = client_route; ptr; ptr = ptr->next)
		tmp = ptr;

	tmp->next = croute;

	return croute;
}

int del_client_route(const char *dest,
					 const char *genmask,
					 const char *gateway,
					 char *iface)
{
	struct route *ptr, *gf;

	if (!iface) {
		printf("Error adding route.\n");

		return -1;
	}

	for (ptr = client_route; ptr; gf = ptr, ptr = ptr->next) {
		if ((ptr->dest.s_addr == inet_addr(dest)) &&
			(ptr->genmask.s_addr == inet_addr(genmask)) &&
			(ptr->gateway.s_addr == inet_addr(gateway)) &&
			(!strcmp(ptr->iface, iface))) {
			gf->next = ptr->next;
			free_route(ptr);
		}
	}

	return 0;
}

void show_route_table(void)
{
	/*TODO: Acertar a identação da tabela na exibição */
	struct route *ptr;

	printf("Client IP routing table\n");
	printf("Destination \tGateway \tGenmask \t\tIface\n");
	for (ptr = client_route; ptr; ptr = ptr->next) {
		printf("%s \t", inet_ntoa(ptr->dest));
		printf("%s \t", inet_ntoa(ptr->gateway));
		printf("%s \t", inet_ntoa(ptr->genmask));
		printf("\t%s\n", ptr->iface);
	}
}

struct route *get_route_by_daddr(const char *daddr)
{
	struct route *ptr, *def;
	
	def = NULL;
	for (ptr = client_route; ptr; ptr = ptr->next) {
		if (ptr->dest.s_addr == 0)
			def = ptr;
		else if ((inet_addr(daddr) & ptr->genmask.s_addr) == ptr->dest.s_addr)
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
