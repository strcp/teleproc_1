/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

/**
 * @defgroup cmd_parser Parser de Comandos
 * @brief Funções de parsing para os comandos chamados pelo terminar tanto do
 * roteador quanto do cliente.
 * @{
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <route.h>
#include <connection.h>
#include <data.h>

/**
 * Verificação dos parâmetros passados ao comando route.
 * @param params Ponteiro para os parâmetros passados para o comando de route.
 * @return void
 */
void route_cmd(char *params)
{
	char *p1, *p2, *tmp;
	char *net, *gw, *nm, *iface;


	if (!(p1 = strtok_r(params, " ", &p2))) {
		show_route_table();
		return;
	}

	if (!(strcmp(p1, "show"))) {
		show_route_table();
		return;
	} else if (!strcmp(p1, "del")) {
		while ((tmp = strtok_r(p2, " ", &p2))) {
			if (!strcmp(tmp, "-net")) {
				net = strtok_r(p2, " ", &p2);
			} else {
				goto route_usage;
			}
		}
		del_client_route(net);
		return;
	} else if ((!strcmp(p1, "add"))) {
		while ((tmp = strtok_r(p2, " ", &p2))) {
			if (!strcmp(tmp, "-net")) {
				net = strtok_r(p2, " ", &p2);
			} else if (!strcmp(tmp, "gw")) {
				gw = strtok_r(p2, " ", &p2);
			} else if (!strcmp(tmp, "netmask")) {
				nm = strtok_r(p2, " ", &p2);
			} else if (!strcmp(tmp, "dev")) {
				iface = strtok_r(p2, " ", &p2);
			} else {
				goto route_usage;
				break;
			}
		}
		add_client_route(net, gw, nm, iface);
		return;
	} else if (!strcmp(p1, "flush")) {
		cleanup_route_table();
		return;
	}

route_usage:
	printf("Usage: route {add|del|flush|show} -net <ip> gw <ip> netmask <ip> dev <devname>\n");
}

/**
 * Verificação dos parâmetros passados ao comando send.
 * @param params Ponteiro para os parâmetros passados para o comando de send.
 * @return void
 */
void send_cmd(char *params)
{
	char *p1, *p2;
	char *daddr, *dport, *data;
	struct data_info *dinfo;

	if (!(p1 = strtok_r(params, " ", &p2)))
		goto send_usage;

	if ((strcmp(p1, "-file")) || (!(data = strtok_r(p2, " ", &p2))))
		goto send_usage;
	if (!(daddr = strtok_r(p2, " ", &p2)))
		goto send_usage;
	if (!(dport = strtok_r(p2, " ", &p2)))
		goto send_usage;

	if (!(dinfo = load_data(data))) {
		printf("Error to load file.\n");
		return;
	}
	//dump_data(dinfo);
	send_udp_data(daddr, atoi(dport), dinfo,  dinfo->size + sizeof(struct data_info));
	free_data_info(dinfo);

	return;

send_usage:
	printf("Usage: send -file </path/to/file> <dest_ip> <dest_port>\n");
}

/**
 * Verificação dos parâmetros passados ao comando inserção de erro.
 * @param params Ponteiro para os parâmetros passados para o comando de error.
 * @return void
 */
void error_cmd(char *params)
{
	char *p1, *p2;

	if (!(p1 = strtok_r(params, " ", &p2)))
		goto send_usage;

	if (!(strcmp(p1, "enable"))) {
		printf("Random error enabled.\n");
		enable_error();
		return;
	} else if (!(strcmp(p1, "disable"))) {
		printf("Random error disabled.\n");
		disable_error();
		return;
	}

send_usage:
	printf("Usage: error {enable|disable}\n");
}

/**
 * Verificação dos parâmetros passados ao comando de ifconfig.
 * @param params Ponteiro para os parâmetros passados para o comando ifconfig.
 * @return void
 */
void ifconfig_cmd(char *params)
{
	char *p1, *p2;

	if ((p1 = strtok_r(params, " ", &p2))) {
		ifconfig(p1);
	} else {
		ifconfig(NULL);
	}
}

/**
 * Verifica qual o comando requisitado, pegando seus parâmetros e chamando as
 * funções específicas de parsing de parâmetros do comando.
 * @param full_cmd Ponteiro para os comando com os parâmetros passados pelo cliente.
 * @return void
 */
void parse_cmds(char *full_cmd)
{
	char *cmd, *param;

	param = NULL;
	if (!(cmd = strtok_r(full_cmd, " ", &param)))
		return;

	if (!(strcmp(cmd, "route")))
		route_cmd(param);
	else if (!(strcmp(cmd, "ifconfig")))
		ifconfig_cmd(param);
	else if (!(strcmp(cmd, "send")))
		send_cmd(param);
	else if (!(strcmp(cmd, "stats")))
		dump_statistics();
	else if (!(strcmp(cmd, "error")))
		error_cmd(param);
	else
		printf("cmd unknown.\n");
}
/** @} */
