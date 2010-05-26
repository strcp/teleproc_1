#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <cbproto.h>
#include <route.h>
#include <connection.h>

int main()
{
	ifconfig("eth1");

	/* A ordem da tabela afeta o roteamento, a prioridade é sempre da regra mais
	 * antiga. */
	add_client_route("192.168.6.11", "0.0.0.0", "255.255.255.255", "eth0");
	add_client_route("0.0.0.0", "192.168.6.11", "0.0.0.0", "eth0");
//	add_client_route("10.0.2.0", "10.0.2.5", "255.255.255.0", "eth0");

	show_route_table();
	
	del_client_route("192.168.6.11", "0.0.0.0", "255.255.255.255", "eth0");
	show_route_table();
	//send_cbp_data("192.168.6.11", 5555, 5556, NULL, 0);
	send_cbp_data("192.168.6.66", 5555, 5556, NULL, 0);

	cleanup_route_table();

	return 0;
}
