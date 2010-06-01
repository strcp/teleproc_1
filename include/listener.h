#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define ROUTER_PORT 6666
#define MAXSIZE 100000

typedef enum usage_type_t {
	ROUTER_USAGE = 0,
	CLIENT_USAGE
} usage_type_t;

int sanity_check(struct iphdr *ip);
void *listener(void *usage_type);
int where_to_send(char *packet, usage_type_t usage_type);
