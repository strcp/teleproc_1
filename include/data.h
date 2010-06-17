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
	int name_size;			/**< Tamanho do nome do arquivo sem /0 */
	long int data_size;		/**< Tamanho do dado carregado */
	long long int tot_len;	/**< Tamanho total do pacote */
	unsigned char fragmented;	/* 0 = No fragments, 1 = Is a fragment, 2 = Last Fragment */
	int id;					/**< Número de identificação do pacote */
	int seq;				/**< Número de sequência do pacote */
} __attribute__((__packed__));	/* Evitando padding do processador */


long int file_size(FILE *fp);
struct data_info *load_data(char *file_path);
void dump_data(struct data_info *dinfo);
void free_data_info(struct data_info *dinfo);
int save_data(struct data_info *data);
/** @} */
