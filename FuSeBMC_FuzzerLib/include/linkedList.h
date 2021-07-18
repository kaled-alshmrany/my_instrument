#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "inputTypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/********* LinkedList FOR GOALS *********/
typedef struct _FuSeBMC_goal_node
{
    unsigned long int  val;
	char is_new; // '1': Is New Goal; '0' is old goal.
	struct _FuSeBMC_goal_node * next;
} FuSeBMC_goal_node_t;

void fuSeBMC_push_in_goal_list(FuSeBMC_goal_node_t ** head, unsigned long int goal, char is_new);
//void fuSeBMC_print_goal_list(FuSeBMC_goal_node_t * head);
FuSeBMC_goal_node_t * fuSeBMC_find_in_goal_list(FuSeBMC_goal_node_t * head, unsigned long int goal);

/********* LinkedList FOR INPUTS *********/
typedef struct _FuSeBMC_input_node
{
    fuSeBMC_InputType_t input_type;
	void * val_ptr;
	struct _FuSeBMC_input_node * next;
} FuSeBMC_input_node_t;

int fuSeBMC_get_sizeof_inputType(fuSeBMC_InputType_t input_type);
void fuSeBMC_push_in_input_list(FuSeBMC_input_node_t ** head, fuSeBMC_InputType_t input_type , void * val_ptr);
void fuSeBMC_reverse_input_list(FuSeBMC_input_node_t ** head);
#ifdef __cplusplus
}
#endif

#endif /* LINKEDLIST_H */

