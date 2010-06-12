/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

/**
 * @defgroup data Manipulação de dados
 * @brief Manipulação dos dados enviados e recebidos.
 * @{
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <data.h>

/**
 * Calcula o tamanho do arquivo.
 * @param fp Ponteiro para o descritor do arquivo.
 * @return O tamanho do arquivo em bytes.
 */
long int file_size(FILE *fp)
{
	long size;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);

	return size;
}

/**
 * Carrega os dados do arquivo em uma estrutura de data_info.
 * @param file_path Caminho para o arquivo que será carregado.
 * @return Estrutura de data_info com os dados do arquivo.
 */
struct data_info *load_data(char *file_path)
{
	FILE *fp;
	long len;
	struct data_info *dinfo;
	char *tmp, *name;

	if (!(fp = fopen(file_path, "rb"))) {
		printf("Error opening file.\n");
		return NULL;
	}
	if ((len = file_size(fp)) < 0) {
		printf("Error calculating file size.\n");
		return NULL;
	}
	if ((tmp = strrchr(file_path, '/')))
		name = ++(tmp);
	else
		name = file_path;

	dinfo = malloc(sizeof(struct data_info) + len + strlen(name) + 1);
	memset(dinfo, 0, sizeof(struct data_info) + len + strlen(name) + 1);

	sprintf(((char *)dinfo + sizeof(struct data_info)), name);
	dinfo->name_size = strlen(name);
	dinfo->data_size = len;
	dinfo->tot_len = dinfo->name_size + 1 + dinfo->data_size + sizeof(struct data_info);
	fread(((char *)dinfo + sizeof(struct data_info) + dinfo->name_size + 1), len, 1, fp);
	fclose(fp);
/*
	char tm[1024];
	memset(tm, 0, 1024);
	snprintf(tm, dinfo->data_size, (char *)dinfo + sizeof(struct data_info) + dinfo->name_size + 1);
*/
	return dinfo;
}

/**
 * Salva os dados de uma estrutura de data_info em um arquivo.
 * @param data Estrutura com os dados do arquivo.
 * @return < 0 em erro, número de itens escritos em sucesso.
 */
int save_data(struct data_info *data)
{
	FILE *fp;
	int ret;
	char *name, *dp;

	name = ((char *)data + sizeof(struct data_info));

	if (!(fp = fopen(name, "wb"))) {
		printf("Error opening %s\n", name);
		return -1;
	}

	dp = ((char *)data + sizeof(struct data_info) + data->name_size + 1);
	if (!(ret = fwrite(dp, data->data_size, 1, fp))) {
		printf("Error writing file %s\n", (char *)data + sizeof(struct data_info));
		return -1;
	}
	fclose(fp);

	return ret;
}

/**
 * Libera a memória usada por uma estrutura de data_info.
 * @param dinfo Ponteiro para a estrutura.
 * @return void
 */
void free_data_info(struct data_info *dinfo)
{
	if (!dinfo)
		return;

	free(dinfo);
}

/**
 * Exibe os dados de uma estrutura de data_info.
 * @param dinfo Ponteiro para a estrutura.
 * @return void
 */
void dump_data(struct data_info *dinfo)
{
	printf("Dump data info\n");
	printf("Name: %s\n", (char *)dinfo + sizeof(struct data_info));
	printf("Size: %ld\n\n", dinfo->data_size);
}
/** @} */
