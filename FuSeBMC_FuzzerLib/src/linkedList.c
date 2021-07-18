// https://www.learn-c.org/en/Linked_lists
// https://www.tutorialspoint.com/data_structures_algorithms/linked_list_program_in_c.htm
#include <linkedList.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <inputTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void fuSeBMC_push_in_goal_list(FuSeBMC_goal_node_t ** head, unsigned long int goal, char is_new)
{
    FuSeBMC_goal_node_t * new_node;
    new_node = malloc(sizeof(FuSeBMC_goal_node_t));
    new_node->val = goal;
	new_node->is_new = is_new;
	new_node->next = *head;
    *head = new_node;
}

/*void fuSeBMC_print_goal_list(FuSeBMC_goal_node_t * head)
{
    FuSeBMC_goal_node_t * current = head;
    while (current != NULL)
    {
        printf("%lu\n", current->val);
        current = current->next;
    }
}*/


FuSeBMC_goal_node_t * fuSeBMC_find_in_goal_list(FuSeBMC_goal_node_t * head, unsigned long int goal)
{
   //start from the first link
   FuSeBMC_goal_node_t * current = head;

   //if list is empty
   if(head == NULL)
   {
      return NULL;
   }

   //navigate through list
   while(current->val != goal)
   {
      //if it is last node
      if(current->next == NULL)
      {
         return NULL;
      }
      else
      {
         //go to next link
         current = current->next;
      }
   }      
	
   //if data found, return the current Link
   return current;
}
/********* LinkedList FOR INPUTS *********/
int fuSeBMC_get_sizeof_inputType(fuSeBMC_InputType_t input_type)
{
	if(input_type == _char) return sizeof(signed char);
	if(input_type == _uchar) return sizeof(unsigned char);
	if(input_type == _short) return sizeof(signed short);
	if(input_type == _ushort) return sizeof(unsigned short);
	if(input_type == _int) return sizeof(signed int);
	if(input_type == _uint) return sizeof(unsigned int);
	if(input_type == _long) return sizeof(long);
	if(input_type == _ulong) return sizeof(unsigned long); 
	if(input_type == _longlong ) return sizeof(long long);
	if(input_type == _ulonglong ) return sizeof(unsigned long long);
	if(input_type == _float ) return sizeof(float);
	if(input_type == _double) return sizeof(double);
	if(input_type == _bool ) return sizeof(_Bool);
	if(input_type == _string ) return sizeof(10); // TODO:
	return -1 ;
}
void fuSeBMC_push_in_input_list(FuSeBMC_input_node_t ** head, fuSeBMC_InputType_t input_type , void * val_ptr)
{
	int input_size;
	if(input_type == _string)
		input_size = strlen(val_ptr);
	else
		input_size = fuSeBMC_get_sizeof_inputType(input_type);
	//printf("PUSH size%d\n", input_size);fflush(stdout);
	FuSeBMC_input_node_t * new_node;
    new_node = malloc(sizeof(FuSeBMC_input_node_t));
    new_node->input_type = input_type;
	new_node->val_ptr = malloc(input_size);
	memcpy(new_node->val_ptr, val_ptr, input_size);
	new_node->next = *head;
    *head = new_node;
	//printf("DONE PUSH size%d\n", input_size);fflush(stdout);
}
void fuSeBMC_reverse_input_list(FuSeBMC_input_node_t ** head)
{
	// Initialize current, previous and
	// next pointers
	FuSeBMC_input_node_t* current = *head;
	FuSeBMC_input_node_t *prev = NULL, *next = NULL;
	while (current != NULL)
	{
		// Store next
		next = current->next;
		
		// Reverse current node's pointer
        current->next = prev;
		
		
		// Move pointers one position ahead.
		prev = current;
		current = next;
	}
	*head = prev;
}
#ifdef __cplusplus
}
#endif

