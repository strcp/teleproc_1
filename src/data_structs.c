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

struct fragment_list *fragment_packet(void *data)
{
	struct data_info *dinfo = (struct data_info *)data;
	struct data_info *frags;
	struct fragment_list *flist;
	int i, seq, pkt_size, offset;

	if (!data)
		return NULL;
	/* TODO: Finish this! Correct sizes, etc.. */
	for (seq = 0, i = dinfo->tot_len; i > 0; i -= MAX_DATA_SIZE, seq++) {
		pkt_size = i >= 0 ? MAX_DATA_SIZE : MAX_DATA_SIZE + i;

		frags = malloc(pkt_size);
		if ((i - MAX_DATA_SIZE) <= 0)
			frags->fragmented = 2;
		else
			frags->fragmented = 1;

		frags->tot_len = pkt_size;
		frags->seq = seq;
		frags->data_size = pkt_size - sizeof(struct data_info);
		if (!seq)
			frags->name_size = dinfo->name_size;
//		flist = list_prepend(flist, frags);
	}
	return flist;
}
#if 0
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
