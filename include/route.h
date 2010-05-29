#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>


struct route
{
	struct in_addr dest;
	struct in_addr genmask;
	struct in_addr gateway;
	char *iface;
	struct route *next;
	struct route *prev;
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
