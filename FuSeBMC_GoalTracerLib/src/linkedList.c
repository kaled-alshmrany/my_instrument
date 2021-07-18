// https://www.learn-c.org/en/Linked_lists
// https://www.tutorialspoint.com/data_structures_algorithms/linked_list_program_in_c.htm
#include <linkedList.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void fuSeBMC_push_in_list(FuSeBMC_node_t ** head, unsigned long int val)
{
    FuSeBMC_node_t * new_node;
    new_node = malloc(sizeof(FuSeBMC_node_t));
    new_node->val = val;
    new_node->next = *head;
    *head = new_node;
}

void fuSeBMC_print_list(FuSeBMC_node_t * head)
{
    FuSeBMC_node_t * current = head;
    while (current != NULL)
    {
        printf("%lu\n", current->val);
        current = current->next;
    }
}


FuSeBMC_node_t * fuSeBMC_find_in_list(FuSeBMC_node_t * head, unsigned long int key)
{
   //start from the first link
   FuSeBMC_node_t * current = head;

   //if list is empty
   if(head == NULL)
   {
      return NULL;
   }

   //navigate through list
   while(current->val != key)
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
#ifdef __cplusplus
}
#endif

