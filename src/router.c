#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <route.h>
#include <listener.h>
#include <connection.h>
#include <cmd_parser.h>
#include <data.h>

int main()
{
	pthread_t th;
	char *cmd, *hist;
	char prompt[] = "router> ";

	init_default_routes();

	pthread_create(&th, NULL, listener, (void *)ROUTER_USAGE);

	while (1) {
		cmd = readline(prompt);
		if (!strcmp(cmd, "exit") || !strcmp(cmd, "quit")) {
			pthread_kill(th, SIGKILL);
			break;
		}
		hist = strdup(cmd);
		parse_cmds(cmd);
		free(cmd);
		if (hist && *hist)
			add_history(hist);
	}
	clear_history();
	cleanup_route_table();

	return 0;
}
