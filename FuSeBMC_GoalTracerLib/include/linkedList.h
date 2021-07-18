#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _FuSeBMC_node
{
    unsigned long int  val;
    struct _FuSeBMC_node * next;
} FuSeBMC_node_t;

void fuSeBMC_push_in_list(FuSeBMC_node_t ** head, unsigned long int val);
void fuSeBMC_print_list(FuSeBMC_node_t * head);
FuSeBMC_node_t * fuSeBMC_find_in_list(FuSeBMC_node_t * head, unsigned long int key);


#ifdef __cplusplus
}
#endif

#endif /* LINKEDLIST_H */

