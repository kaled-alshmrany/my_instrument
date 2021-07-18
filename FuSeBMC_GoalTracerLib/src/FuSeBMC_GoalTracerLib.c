#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <FuSeBMC_GoalTracerLib.h>
#include <linkedList.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct _IO_FILE * stdin;
extern struct _IO_FILE * stderr;
extern char * fgets (char *__restrict __s, int __n, FILE *__restrict __stream);
extern void * memcpy (void *__restrict __dest, const void *__restrict __src, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern char * strcat (char *__restrict __dest, const char *__restrict __src) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
char * fuSeBMC_get_input();

// This file is part of TestCov,
// a robust test executor with reliable coverage measurement:
// https://gitlab.com/sosy-lab/software/test-suite-validator/
//
// Copyright (C) 2018 - 2020  Dirk Beyer
// SPDX-FileCopyrightText: 2019 Dirk Beyer <https://www.sosy-lab.org>
//
// SPDX-License-Identifier: Apache-2.0

#define FUSEBMC_MAX_INPUT_SIZE 3000

void fuSeBMC_abort_prog()
{
	abort();
}

void __VERIFIER_assume(int cond)
{
  if (!cond)
  {
    fuSeBMC_abort_prog();
  }
}

void __VERIFIER_error()
{
	// You can Implement it to check if '__VERIFIER_error()' is called.

}



// taken from https://stackoverflow.com/a/32496721
void fuSeBMC_replace_char(char *str, char find, char replace)
{
	char *current_pos = strchr(str, find);
	while (current_pos)
	{
		*current_pos = replace;
		current_pos = strchr(current_pos, find);
	}
}

void fuSeBMC_parse_input_from(char *inp_var, char *format, void *destination)
{
	char format_with_fallback[13];
	strcpy(format_with_fallback, format);
	strcat(format_with_fallback, "%c%c%c%c");
	if (inp_var[0] == '0' && inp_var[1] == 'x')
	{
		fuSeBMC_replace_char(format_with_fallback, 'd', 'x');
		fuSeBMC_replace_char(format_with_fallback, 'u', 'x');
	}
	else
	{
		if (inp_var[0] == '\'' || inp_var[0] == '\"')
		{
			int inp_length = strlen(inp_var);
			// Remove ' at the end
			inp_var[inp_length - 1] = '\0';
			// Remove ' in the beginning
			inp_var++;
		}
	}
	char leftover[4];
	int filled = sscanf(inp_var, format_with_fallback, destination, &leftover[0],
			&leftover[1], &leftover[2], &leftover[3]);
	_Bool is_valid = 1;
	if (filled == 5 || filled == 0)
	{
		is_valid = 0;
	}
	while (filled > 1) 
	{
		filled--;
		char literal = leftover[filled - 1];
		switch (literal)
		{
			case 'l':
			case 'L':
			case 'u':
			case 'U':
			case 'f':
			case 'F':
				break;
			default:
				is_valid = 0;
		}
	}
	if (!is_valid)
	{
		fprintf(stderr, "Can't parse input: '%s'\n", inp_var);
		fuSeBMC_abort_prog();
	}
}

void fuSeBMC_parse_input(char *format, void *destination)
{
	char * inp_var = fuSeBMC_get_input();
	fuSeBMC_parse_input_from(inp_var, format, destination);
	
	// NEW
	free(inp_var);
}

char __VERIFIER_nondet_char()
{
	char val;
	char *inp_var = fuSeBMC_get_input();
	if (inp_var[0] == '\'')
	{
		fuSeBMC_parse_input_from(inp_var, "%c", &val);
	} 
	else 
	{
		fuSeBMC_parse_input_from(inp_var, "%hhd", &val);
	}
	free(inp_var);
	return val;
}

unsigned char __VERIFIER_nondet_uchar()
{
	unsigned char val;
	fuSeBMC_parse_input("%hhu", &val);
	return val;
}

short __VERIFIER_nondet_short()
{
	short val;
	fuSeBMC_parse_input("%hd", &val);
	return val;
}

unsigned short __VERIFIER_nondet_ushort()
{
	unsigned short val;
	fuSeBMC_parse_input("%hu", &val);
	return val;
}

int __VERIFIER_nondet_int()
{
	int val;
	fuSeBMC_parse_input("%d", &val);
	return val;
}

unsigned int __VERIFIER_nondet_uint()
{
	unsigned int val;
	fuSeBMC_parse_input("%u", &val);
	return val;
}

long __VERIFIER_nondet_long()
{
	long val;
	fuSeBMC_parse_input("%ld", &val);
	return val;
}

unsigned long __VERIFIER_nondet_ulong()
{
	unsigned long val;
	fuSeBMC_parse_input("%lu", &val);
	return val;
}

long long __VERIFIER_nondet_longlong()
{
	long long val;
	fuSeBMC_parse_input("%lld", &val);
	return val;
}

unsigned long long __VERIFIER_nondet_ulonglong()
{
	unsigned long long val;
	fuSeBMC_parse_input("%llu", &val);
	return val;
}

float __VERIFIER_nondet_float()
{
	float val;
	fuSeBMC_parse_input("%f", &val);
	return val;
}

double __VERIFIER_nondet_double()
{
	double val;
	fuSeBMC_parse_input("%lf", &val);
	return val;
}

_Bool __VERIFIER_nondet_bool()
{
	return (_Bool)__VERIFIER_nondet_int();
}

void *__VERIFIER_nondet_pointer()
{
	return (void *)__VERIFIER_nondet_ulong(); 
}

unsigned int __VERIFIER_nondet_size_t()
{
	return __VERIFIER_nondet_uint();
}

unsigned char __VERIFIER_nondet_u8()
{
    return __VERIFIER_nondet_uchar();
}

unsigned short __VERIFIER_nondet_u16()
{
    return __VERIFIER_nondet_ushort();
}

unsigned int __VERIFIER_nondet_u32()
{
    return __VERIFIER_nondet_uint();
}

unsigned int __VERIFIER_nondet_U32()
{
    return __VERIFIER_nondet_u32();
}

unsigned char __VERIFIER_nondet_unsigned_char()
{
  return __VERIFIER_nondet_uchar();
}

unsigned int __VERIFIER_nondet_unsigned()
{
    return __VERIFIER_nondet_uint();
}

// SEE: https://stackoverflow.com/q/39431924

const char *__VERIFIER_nondet_string()
{
	char *val = malloc(FUSEBMC_MAX_INPUT_SIZE + 1);
	// Read to end of line
	fuSeBMC_parse_input("%[^\n]", val);
	return val;
}

char * fuSeBMC_get_input()
{
    char * inp_var = malloc(FUSEBMC_MAX_INPUT_SIZE);
    char * result = fgets(inp_var, FUSEBMC_MAX_INPUT_SIZE, stdin);
    if (result == 0)
    {
        fprintf(stderr, "No more test inputs available, exiting\n");
        exit(1);
    }
    unsigned int input_length = strlen(inp_var)-1;
    /* Remove '\n' at end of input */
    if (inp_var[input_length] == '\n')
    {
        inp_var[input_length] = '\0';
    }
    return inp_var;
}

FuSeBMC_node_t * fuSeBMC_list_head = NULL;
pthread_mutex_t * fuSeBMC_lock = NULL;
void fuSeBMC_return(int code){exit(code);}
void fuseGoalCalled(unsigned long int goal)
{
    if(fuSeBMC_lock == NULL)
    {
        fuSeBMC_lock = malloc(sizeof(pthread_mutex_t));
        if (pthread_mutex_init(fuSeBMC_lock, NULL) != 0)
        {
            printf("\n mutex init failed\n");
            #ifdef MYDEBUG
            return;
            #endif
        }
    }
    pthread_mutex_lock(fuSeBMC_lock);
    if(fuSeBMC_find_in_list(fuSeBMC_list_head,goal) == NULL)
    {
        fuSeBMC_push_in_list(&fuSeBMC_list_head , goal);
        FILE *fptr;
        fptr = fopen("./goals_covered.txt","a");
        if(fptr == NULL)
        {
           printf("Error: cannot write to file:goals_covered.txt!"); 
#ifdef MYDEBUG
           pthread_mutex_unlock(fuSeBMC_lock);
           exit(1);
#endif
        }
        fprintf(fptr,"%lu\n",goal);
        fclose(fptr);
    }
    pthread_mutex_unlock(fuSeBMC_lock);
}
#ifdef __cplusplus
}
#endif
