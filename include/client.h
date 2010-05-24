#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

struct clientnet_info
{
	char *iface;
	struct in_addr addr;
	struct in_addr netmask;
	struct in_addr netaddr;
	struct in_addr bcastaddr;
	struct clientnet_info *next;
};
