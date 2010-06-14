/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

/**
 * @defgroup client Terminal do Cliente
 * @brief Terminal de comunicação com o cliente.
 * @{
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <signal.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <route.h>
#include <connection.h>
#include <cmd_parser.h>
#include <listener.h>
#include <data_structs.h>


/**
 * Processo inicial do Cliente. Ao rodar é setada a porta na qual o Cliente irá
 * receber seus dados.
 * @param argv -s \<source port\>
 * @return Retorna 0 se não houver problemas.
 */
int main (int argc, char **argv)
{
	static pthread_t th;
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

	/* Aqui a thread que recebe os dados é iniciada. */
	pthread_create(&th, NULL, listener, (void *)CLIENT_USAGE);
	while (1) {
		cmd = readline(prompt);
		if (!strcmp(cmd, "exit") || !strcmp(cmd, "quit")){
			thread_exit();
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
	cleanup_frag_list();

	return 0;
}
/** @} */
