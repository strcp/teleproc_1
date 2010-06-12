/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

struct fragment_list
{
	struct data_info *frag;
	struct fragment_list *next;
	struct fragment_list *prev;
};

struct fragment_list *frag_list;

struct fragment_list *fragment_packet(void *data);
int save_packet_fragment(struct data_info *dinfo);
struct data_info *get_defregmented_data(struct fragment_list *frags);
void free_fragment_list(struct fragment_list *flist);