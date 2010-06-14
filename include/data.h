/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

/**
 * @ingroup data
 * @{
 */
#include <stdio.h>

/** Estrutura com as informações do dado */
struct data_info
{
	int name_size;			/**< Tamanho do nome do arquivo */
	long int data_size;		/**< Tamanho do dado carregado */
	long int tot_len;
	unsigned char fragmented;	/* 0 = No fragments, 1 = Is a fragment, 2 = Last Fragment */
	int id;
	int seq;
};// __attribute__((__packed__));


long int file_size(FILE *fp);
struct data_info *load_data(char *file_path);
void dump_data(struct data_info *dinfo);
void free_data_info(struct data_info *dinfo);
int save_data(struct data_info *data);
/** @} */
