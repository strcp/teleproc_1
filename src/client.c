#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <route.h>
#include <connection.h>
#include <cmd_parser.h>
#include <listener.h>

int main (int argc, char **argv)
{
	pthread_t th;
	void *status;
	char *cmd, *hist;
	char prompt[] = "client> ";
	int opt, opts = 0;


	opterr = 0;
	client_port = 0;

	while ((opt = getopt(argc, argv, "s:")) != -1) {
		opts++;
		switch (opt) {
			case 's':
				client_port = atoi(optarg);
				break;
			case '?':
			default:
				printf("Usage: %s -s <port to listen>\n", argv[0]);
				exit(0);
		}
	}
	if (!opts) {
		printf("Usage: %s -s <port to listen>\n", argv[0]);
		exit(0);
	}

	/* A ordem da tabela afeta o roteamento, a prioridade é sempre da regra mais
	 * antiga. */
	init_default_routes();


	pthread_create(&th, NULL, listener, (void *)CLIENT_USAGE);

	if (fork()) {
		pthread_join(th, &status);
		printf("Show Statistics\n");
	} else {
		while (1) {
			cmd = readline(prompt);
			if (!strcmp(cmd, "exit") || !strcmp(cmd, "quit"))
				break;
			hist = strdup(cmd);
			parse_cmds(cmd);
			free(cmd);
			if (hist && *hist)
				add_history(hist);
		}
		clear_history();
	}
	cleanup_route_table();

	return 0;
}
