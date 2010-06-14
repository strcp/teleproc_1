/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <data_structs.h>
#include <data.h>
#include <connection.h>


struct fragment_list *frag_list;

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

struct fragment_list *fragment_packet(void *data)
{
	struct data_info *dinfo = (struct data_info *)data;
	struct data_info *frags;
	struct fragment_list *flist;
	long int i, seq, pkt_size, data_size;
	char *offset;


	if (!data)
		return NULL;

	offset = (char *)data + sizeof(struct data_info);
	data_size = MAX_DATA_SIZE - sizeof(struct data_info);

	flist = NULL;

	i = (dinfo->tot_len - sizeof(struct data_info));
	for (seq = 0; i > 0; i -= data_size, seq++) {

		pkt_size = i >= 0 ? data_size : data_size + i;
		pkt_size += sizeof(struct data_info);

		frags = malloc(pkt_size);

		frags->tot_len = pkt_size;
		frags->seq = seq;
		frags->name_size = 0;
		if (!seq)
			frags->name_size = dinfo->name_size;

		frags->data_size = frags->tot_len - (sizeof(struct data_info) + frags->name_size + 1);

		if (i <= 0)
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

#if 0
struct data_info *get_defregmented_data(struct fragment_list *frags)
{
	struct data_info *dinfo;
	struct fragment_list *f;

	for (f = frags; f; f = f->next) {
		f->data->
	}
}

int save_packet_fragment(struct iphdr *ip)
{
	struct fragment_list *flist, *fl;
	struct hash_frags *ht;
	struct data_info *tmp;
	int size;

	if (!ip)
		return 0;

	ht = malloc(sizeof(struct hash_frags));

	ht->index = ip->frag_off;
	tmp = get_packet_data((char *)ip);
	size = (sizeof(struct data_info) + tmp->name_size + 1 + tmp->data_size);
	ht->data = (struct data_info *)malloc(size);
	memcpy(ht->data, tmp, size);

	flist = malloc(sizeof(struct fragment_list));
	flist->pkt_id = ip->id;
	flist->frag = ht;
	flist->prev = NULL;
	flist->next = NULL;

	for (fl = frag_list; fl; fl = fl->next) {
		if (!fl->next) {
			fl->next = flist;
			flist->prev = fl;
		}
	}
	if (!frag_list)
		frag_list = flist;

	return 1;
}

struct data_info *get_defregmented_data(int ip_id)
{
}

void free_hash(struct hash_table * ht)
{
}

void free_fragments(int id)
{
}

void free_fragment_list(struct fragment_list *flist)
{
}
#endif
