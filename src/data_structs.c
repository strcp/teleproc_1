/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

/**
 * @defgroup data_struct Manipulação de estrutura de dados
 * @brief Manipulação das listas de dados fragmentados.
 * @{
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <data_structs.h>
#include <data.h>
#include <connection.h>

/** Lista que guarda os fragmentos dos dados recebidos */
static struct fragment_list *frag_list = NULL;


int fragment_list_length(struct fragment_list *flist)
{
	struct fragment_list *f;
	int n = 0;

	for (f = flist; f; f = f->next) {
		n++;
	}

	return n;
}

static void free_frag_list(struct fragment_list *list)
{
	struct fragment_list *flist, *f;

	if (!list)
		return;

	for (flist = list; flist; f = flist, flist = flist->next) {
		if (f->frag)
			free_data_info(f->frag);
		free(f);
		f = NULL;
	}
}

#if 0
static void dump_frag_list(struct fragment_list *flist) {
	struct fragment_list *f;

	for (f = flist; f; f = f->next) {
		printf("%d\t", f->frag->seq);
	}
	printf("\n");

}
#endif

/**
 * Ordena fragmentos pelo número de sequência do pacote.
 * @param flist Lista com os fragmentos de um determinado pacote.
 * @return Lista com os fragmentos ordenados.
 */
struct fragment_list *sort_fragments(struct fragment_list *flist)
{
	struct fragment_list *f, **ret;
	int n, i;

	f = flist;
	n = fragment_list_length(flist);
	ret = malloc(n * sizeof(struct fragment_list));
	memset(ret, 0, n * sizeof(struct fragment_list));

	for (f = flist; f; f = f->next, i++) {
		ret[f->frag->seq] = f;
	}

	if (n != i) {
		printf("Packet was not complete. Droping..\n");
		free(ret);
		return NULL;
	}

	for (i = 0; i < n; i++) {
		ret[i]->prev = NULL;
		ret[i]->next = NULL;
		if (ret[i + 1]) {
			ret[i]->next = ret[i + 1];
			ret[i + 1]->prev = ret[i];
		}
	}
	f = ret[0];
	free(ret);

	return f;
}

struct fragment_list *list_steal(struct fragment_list *node)
{
	if (node->next)
		node->next->prev = node->prev;
	if (node->prev)
		node->prev->next = node->next;

	node->next = NULL;
	node->prev = NULL;
	return node;
}

struct fragment_list *list_prepend(struct fragment_list *flist, struct data_info *dinfo)
{
	struct fragment_list *list;

	list = malloc(sizeof(struct fragment_list));
	list->frag = dinfo;
	list->prev = NULL;
	list->next = NULL;
	if (flist) {
		list->next = flist;
		flist->prev = list;
	}

	return list;
}

/**
 * Fragmenta um dado para caber no MAX_DATA_SIZE.
 * @param data Dado para ser fragmentado.
 * @return Lista com os fragmentos do dado.
 */
struct fragment_list *fragment_packet(void *data)
{
	struct data_info *frags;
	struct fragment_list *flist;
	long int i, seq, pkt_size, data_size;
	char *offset;

	struct data_info *dinfo = (struct data_info *)data;


	if (!data)
		return NULL;

	offset = (char *)data + sizeof(struct data_info);
	data_size = MAX_DATA_SIZE - sizeof(struct data_info);

	flist = NULL;

	i = (dinfo->tot_len - sizeof(struct data_info));
	for (seq = 0; i > 0; i -= data_size, seq++) {

		pkt_size = (i - data_size) >= 0 ? data_size : i;
		pkt_size += sizeof(struct data_info);

		frags = malloc(pkt_size);

		frags->tot_len = pkt_size;
		frags->seq = seq;
		frags->name_size = 0;
		frags->id = dinfo->id;
		if (!seq)
			frags->name_size = dinfo->name_size;

		frags->data_size = frags->tot_len - (sizeof(struct data_info) + frags->name_size + 1);

		if ((i - data_size) <= 0)
			/* Último fragmento */
			frags->fragmented = 2;
		else
			frags->fragmented = 1;

		memcpy((char *)frags + sizeof(struct data_info), offset, frags->tot_len - sizeof(struct data_info));
		offset += (frags->tot_len - sizeof(struct data_info));
		flist = list_prepend(flist, frags);
	}
	return flist;
}

/**
 * Defragmenta um dado existente no buffer e retorna como uma única estrutura.
 * @param id Identificador do dado que deve ser restaurado.
 * @return Estrutura com os dados restaurados.
 */
struct data_info *get_defragmented_data(int id)
{
	struct data_info *dinfo;
	struct fragment_list *f, *frags;
	long long int size;
	char *data;

	size = 0;
	frags = get_frag_id_list(id);
	frags = sort_fragments(frags);

	for (f = frags; f; f = f->next) {
		size += f->frag->tot_len - sizeof(struct data_info);
	}
	dinfo = malloc(size + sizeof(struct data_info));
	memset(dinfo, 0, size + sizeof(struct data_info));
	dinfo->tot_len = size + sizeof(struct data_info);

	data = (char *)dinfo + sizeof(struct data_info);
	for (f = frags; f; f = f->next) {
		memcpy(data, (char *)f->frag + sizeof(struct data_info), f->frag->tot_len - sizeof(struct data_info));
		data = data + (f->frag->tot_len - sizeof(struct data_info));
		dinfo->data_size += f->frag->tot_len - sizeof(struct data_info);
		if (f->frag->seq == 0) {
			dinfo->name_size = f->frag->name_size;
			dinfo->id = f->frag->id;
		}
	}
	dinfo->data_size -= (dinfo->name_size + 1);	/* Não esquecendo do \0 :-) */
	dinfo->seq = 0;
	dinfo->fragmented = 0;

	return dinfo;
}

/**
 * Salva o fragmento no buffer global
 * @param dinfo fragmento de um dado.
 * @return 1 em sucesso 0 em erro.
 */
int save_packet_fragment(struct data_info *dinfo)
{
	if ((frag_list = list_prepend(frag_list, dinfo)))
		return 1;
	return 0;
}

/**
 * Retorna uma lista de fragmentos de um específico id existente na buffer global.
 * @param id do dado.
 * @return Lista dos fragmentos com o id especificado.
 */
struct fragment_list *get_frag_id_list(int id)
{
	struct fragment_list *f, *seq_list, *aux;

	seq_list = NULL;
	aux = NULL;
	for (f = frag_list; f; f = aux) {
		aux = f->next;
		if (f->frag->id == id) {
			f = list_steal(f);
			if (f == frag_list)
				frag_list = frag_list->next ? frag_list->next : NULL;
			seq_list = list_prepend(seq_list, f->frag);
			free(f);
			f = NULL;
		}
	}

	return seq_list;
}

/**
 * Verifica se o pacote está completo no buffer global.
 * @param dinfo um fragmento.
 * @return 1 se completo e 0 se não completo.
 */
int is_packet_complete(struct data_info *dinfo)
{
	struct fragment_list *f;
	int last, n;

	for (n = 0, last = -2, f = frag_list; f; f = f->next) {
		if (f->frag->id == dinfo->id) {
			n++;
			if (f->frag->fragmented == 2)
				last = f->frag->seq;
		}
	}
	/* n - 1 == last? já que os números de seq começam em 0 */
	return  (n - 1) == last ? 1 : 0;
}

void cleanup_frag_list(void)
{
	free_frag_list(frag_list);
}

/** @} */
