#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <route.h>
#include <connection.h>
#include <cmd_parser.h>

int main()
{
	char *cmd;
	char prompt[] = "client> ";

	/* A ordem da tabela afeta o roteamento, a prioridade é sempre da regra mais
	 * antiga. */
	add_client_route("192.168.6.11", "0.0.0.0", "255.255.255.255", "eth0");
	add_client_route("0.0.0.0", "192.168.6.11", "0.0.0.0", "eth0");
//	add_client_route("10.0.2.0", "10.0.2.5", "255.255.255.0", "eth0");

	send_udp_data("192.168.6.66", 5555, 5556, "fuubar\0", 7);

	while (1) {
		cmd = readline(prompt);
		if (!strcmp(cmd, "exit") || !strcmp(cmd, "quit"))
			break;
		parse_cmds(cmd);
		if (cmd && *cmd)
			add_history(cmd);
	}
	clear_history();
	cleanup_route_table();

	return 0;
}
