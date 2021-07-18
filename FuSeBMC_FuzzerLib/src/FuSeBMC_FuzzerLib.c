#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

//#include <errno.h>
//#include <limits.h>

#include <FuSeBMC_FuzzerLib.h>
#include <linkedList.h>
#include <inputTypes.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
extern struct _IO_FILE * stdin;
extern struct _IO_FILE * stderr;
extern char * fgets (char *__restrict __s, int __n, FILE *__restrict __stream);
extern void * memcpy (void *__restrict __dest, const void *__restrict __src, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern char * strcat (char *__restrict __dest, const char *__restrict __src) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
*/
//char * fuSeBMC_get_input();
//int input_count = 0;

//#define BUF_SIZE 1024
//char buffer[BUF_SIZE];

//int inputArr[100];
//int inputIndex = -1;
// 1 for error-call
// 2 cover-branches
extern unsigned int fuSeBMC_category;

//extern const char * fuSeBMC_run_id;

// This file is part of TestCov,
// a robust test executor with reliable coverage measurement:
// https://gitlab.com/sosy-lab/software/test-suite-validator/
//
// Copyright (C) 2018 - 2020  Dirk Beyer
// SPDX-FileCopyrightText: 2019 Dirk Beyer <https://www.sosy-lab.org>
//
// SPDX-License-Identifier: Apache-2.0

#define FUSEBMC_MAX_STRING_SIZE 100
#define FUSEBMC_MIN_STRING_SIZE 1

FuSeBMC_goal_node_t * fuSeBMC_goal_list_head = NULL;
pthread_mutex_t * fuSeBMC_lock = NULL;
unsigned char fuSeBMC_IsPrevTotalCoveredGoalsLoaded = 0;
unsigned char fuSeBMC_IsNewGoalsAdded = 0;
unsigned char fuSeBMC_IsRandomHasSeed = 0;

FuSeBMC_input_node_t * fuSeBMC_input_list_head = NULL;
/*Helper to print values in file.
 * USAGE: fuSeBMC_print_val_in_file("file1.txt","%s%d", "Hello" , 10);
 */
void fuSeBMC_print_val_in_file (char * file_name , char * format, ...)
{
	va_list args;
	va_start (args, format);		
	FILE * fPtr = fopen(file_name, "a");
	if(fPtr == NULL)
	{
	   printf("Error: cannot read file:%s\n" , file_name);
	   return;

	}
	vfprintf (fPtr, format, args);
	va_end (args);
	fclose(fPtr);
	
}
void fuSeBMC_init_mutex()
{
	if(fuSeBMC_lock == NULL)
		{
			fuSeBMC_lock = malloc(sizeof(pthread_mutex_t));
			if (pthread_mutex_init(fuSeBMC_lock, NULL) != 0)
			{
				printf("\n mutex init failed\n");
				#ifdef MYDEBUG
				exit(0);
				#endif
			}
		}
	// cover-branches
	if(fuSeBMC_category == 2 && fuSeBMC_IsPrevTotalCoveredGoalsLoaded != 1)
		fuSeBMC_readPrevTotalCoveredGoals();

	}

void fuSeBMC_readPrevTotalCoveredGoals()
{
	if(fuSeBMC_IsPrevTotalCoveredGoalsLoaded == 1) return;
	pthread_mutex_lock(fuSeBMC_lock);
	
	// Read the goals from file.
	FILE* fPtr = fopen("./FuSeBMC_Fuzzer_goals_covered.txt", "r");
	if(fPtr == NULL)
	{
	   printf("Error: cannot read file:FuSeBMC_Fuzzer_goals_covered.txt!");
	   fuSeBMC_IsPrevTotalCoveredGoalsLoaded = 1;
	   pthread_mutex_unlock(fuSeBMC_lock);
	   return;

	}
	unsigned long int goal = 0;
	while (!feof(fPtr))
	{
		fscanf(fPtr, "%lu", &goal);
		fuSeBMC_push_in_goal_list(&fuSeBMC_goal_list_head , goal, '0'); // '0' is old goal		
	}
	fclose(fPtr);
	
	
	// Read the goals from Environment variable.
	
	/*char* fuSeBMC_run_id_val = getenv(fuSeBMC_run_id);
	if(fuSeBMC_run_id_val)
	{
		char * eptr = NULL;
		char * token = strtok(fuSeBMC_run_id_val, (const char *)"|");
		while (token)
		{
			//printf("token: %s\n", token);
			unsigned long int goal = strtoul(token, &eptr, 10);
			//unsigned long int strtoul (const char * __nptr, char ** __endptr, int __base);
			// endptr :  It is used by the strtoll function to indicate where the conversion stopped.
			// The strtoll function will modify endptr (if endptr is not a null pointer) so that endptr points to 
			// the first character that was not converted.
			 
			//EINVAL		22	 Invalid argument 
			if ((errno == EINVAL ||  goal == ULONG_MAX || eptr != NULL))
			{
				fuSeBMC_print_val_in_file("./err_goals_parse.txt", "error : %s\n" ,fuSeBMC_run_id_val);
				exit(0);
			}
			fuSeBMC_push_in_list(&fuSeBMC_list_head , goal, '0'); // '0' is not new goal.
			token = strtok(NULL, "|");
		}		
	}*/
	
	
	fuSeBMC_IsPrevTotalCoveredGoalsLoaded = 1;
	pthread_mutex_unlock(fuSeBMC_lock);
}
/**function to convert ascii char[] to hex-string (char[])
 *Example "AB" = 0x4142s
 */

char * fuSeBMC_string2hexString(char* input)
{
	int dest_len = (strlen(input)*2)+1+2 ;//*2: ech char converted to 2 chars; +1 for \n ; +2 for 0x
	char* output = malloc(dest_len * sizeof(char));
	memset(output,'\0',dest_len);
	//printf("dest_len=%d\n",dest_len);
	int loop = 0;
    int i = 0;
	sprintf(output,"%s", "0x");
	char* tmp_ptr = output;
	tmp_ptr += 2;
    while(input[loop] != '\0')
    {
		tmp_ptr += i;
        sprintf(tmp_ptr,"%02X", input[loop]);
        loop += 1;
        i += 2;
    }
    //insert NULL at the end of the output string
    tmp_ptr[i++] = '\0';
	return output;
}
/**
 * returns a random text with a given length.
 * @param length : the required length of the random text.
 * @return 
 */
char * fuSeBMC_generate_random_text(int length)
{
	if(!fuSeBMC_IsRandomHasSeed)
	{
		srand(time(0));
		fuSeBMC_IsRandomHasSeed = 1;
	}
    const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuv";
    int stringLength = sizeof(alphanum) - 1;
    char * rnd_text = malloc(sizeof(char) * length + 1); // +1 for '\0'
    char * tmp_ptr = rnd_text;
    //memset(rnd_text, '\0',length);
    for(int i = 0; i < length; ++i)
    {
        *tmp_ptr = alphanum[rand() % stringLength];
        tmp_ptr++;
    }
	*tmp_ptr = '\0';
	return rnd_text;
}
void fuSeBMC_Write_InputList_in_Testcase(char * file_name)
{
	if(fuSeBMC_input_list_head == NULL) return; // No saved Inputs.
	fuSeBMC_reverse_input_list(&fuSeBMC_input_list_head);
	
	FILE *fptr;
	fptr = fopen(file_name,"w");
	if(fptr == NULL)
	{
	   printf("Error: cannot write to file: %s !", file_name); 
#ifdef MYDEBUG
	   exit(1);
#endif
	}
	
	fprintf(fptr,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?><!DOCTYPE testcase PUBLIC \"+//IDN sosy-lab.org//DTD test-format testcase 1.0//EN\" \"https://sosy-lab.org/test-format/testcase-1.0.dtd\"><testcase>");
	FuSeBMC_input_node_t * current = fuSeBMC_input_list_head;
    while (current != NULL)
    {
		if(current->input_type == _char)
		{
			fprintf(fptr,"<input type=\"char\">"); // 
			signed int val = (signed int)(char)(*((char *)current->val_ptr));
			fprintf(fptr,"%d",val);
			//fprintf(fptr,"%c",*((char *)current->val_ptr));
			//fputc(*((char *)current->val_ptr), fptr);
			//fputc('A', fptr);
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _uchar)
		{
			fprintf(fptr,"<input type=\"unsigned char\">");
			unsigned int val = (unsigned int)(unsigned char)(*((unsigned char *)current->val_ptr));
			//fprintf(fptr,"%hhu",*((unsigned char *)current->val_ptr));
			fprintf(fptr,"%u",val);
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _short)
		{
			fprintf(fptr,"<input type=\"short\">");
			fprintf(fptr,"%hd",*((short *)current->val_ptr));
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _ushort)
		{
			fprintf(fptr,"<input type=\"unsigned short\">");
			fprintf(fptr,"%hu",*((unsigned short *)current->val_ptr));
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _int) 
		{
			fprintf(fptr,"<input type=\"int\">"); //
			signed int val = (int)(*((int *)current->val_ptr));
			fprintf(fptr,"%d",val);
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _uint) 
		{
			fprintf(fptr,"<input type=\"unsigned int\">");
			fprintf(fptr,"%u",*((unsigned int *)current->val_ptr));
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _long)
		{
			fprintf(fptr,"<input type=\"long\">");
			fprintf(fptr,"%ld",*((long*)current->val_ptr));
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _ulong) 
		{
			fprintf(fptr,"<input type=\"unsigned long int\">");
			fprintf(fptr,"%lu",*((unsigned long *)current->val_ptr));
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _longlong ) 
		{
			fprintf(fptr,"<input type=\"long long\">");
			fprintf(fptr,"%lld",*((long long *)current->val_ptr));
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _ulonglong ) 
		{
			fprintf(fptr,"<input type=\"unsigned long long\">");
			fprintf(fptr,"%llu",*((unsigned long long *)current->val_ptr));
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _float ) 
		{
			fprintf(fptr,"<input type=\"float\">");
			fprintf(fptr,"%f",*((float *)current->val_ptr));
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _double) 
		{
			fprintf(fptr,"<input type=\"double\">");
			fprintf(fptr,"%lf",*((double *)current->val_ptr));
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _bool)
		{
			fprintf(fptr,"<input type=\"bool\">");
			_Bool val = (_Bool)*((_Bool *)current->val_ptr);
			if(val)
				fprintf(fptr,"1");
			else
				fprintf(fptr,"0");
			fprintf(fptr,"</input>");
		}
		else if(current->input_type == _string ) 
		{ 
			fprintf(fptr,"<input type=\"string\">"); // No derefrence.
			//fprintf(fptr,"%s",((char *)current->val_ptr));
			fprintf(fptr,"%s",fuSeBMC_string2hexString(current->val_ptr));
			fprintf(fptr,"</input>");
		
		}
		
        current = current->next;
    } // End While
	
	fprintf(fptr,"</testcase>");
	fclose(fptr);		
}

void fuSeBMC_reach_error()
{
	fuSeBMC_Write_InputList_in_Testcase("./test-suite/FuSeBMC_Fuzzer_testcase.xml");
	fuSeBMC_print_val_in_file("./error.txt","%s\n","fuSeBMC_reach_error");
	// Kill the Parent Fuzzer.
	kill(getppid(), SIGINT);
		
}
// will inserted after return in main method.
void fuSeBMC_return(int code)
{
	//fuSeBMC_print_val_in_file("./fuSeBMC_return.txt", "code=%d\n" , code);
	fuSeBMC_exit(code);
}
void fuSeBMC_exit(int code)
{
	//fuSeBMC_print_val_in_file("./exit.txt", "code=%d\n" , code);
	if(fuSeBMC_input_list_head == NULL) return; // No saved Inputs.
	
	//cover-branches
	if(fuSeBMC_category == 2)
	{
		if(fuSeBMC_IsNewGoalsAdded == 0) return; // No new goals are covered.
		char outFileName[100];// for Testcase und covered goals.
		char * tmp_random_str;
		while(1)
		{
			memset(outFileName,'\0',100);
			tmp_random_str = fuSeBMC_generate_random_text(30);
			snprintf(outFileName, 100, "./test-suite/FUZ_%s.xml", tmp_random_str);
			
			if(access(outFileName, F_OK ) == 0 )
			{
				// file exists
				free(tmp_random_str);
			}
			else break;				
		}
		
		//1- write the testcase.
		fuSeBMC_Write_InputList_in_Testcase(outFileName);
		
		// Append To Zip Archive.
		if(access(outFileName, F_OK) == 0 )
		{
			// file exists
			char zip_command[1024];
			memset(zip_command,'\0',1024);
			snprintf(zip_command, 1024, "zip -j ./test-suite.zip %s", outFileName); // -j Junk path; don't add path.
			system(zip_command);
		}

		//2- Write covered goals by this input.
		/*memset(outFileName,'\0',100);
		snprintf(outFileName, 100, "./test-suite/%s.txt", tmp_random_str);
		FILE * fptr = fopen(outFileName,"w");
		if(fptr == NULL)
		{
		   printf("Error: cannot write to file:%s!\n", outFileName); 
	#ifdef MYDEBUG
		   exit(1);
	#endif
		}
		pthread_mutex_lock(fuSeBMC_lock);
		FuSeBMC_node_t * current = fuSeBMC_list_head;
		while (current != NULL)
		{
			fprintf(fptr,"%lu\n", current->val);
			current = current->next;
		}
		fclose(fptr);
		*/
		free(tmp_random_str);
		
		//3- Save the Covered goals in file.
		if(fuSeBMC_IsNewGoalsAdded == 1)
		{
			FILE *fptr;
			fptr = fopen("./FuSeBMC_Fuzzer_goals_covered.txt","a");
			if(fptr == NULL)
			{
			   printf("Error: cannot write to file:./FuSeBMC_Fuzzer_goals_covered.txt!");
			   pthread_mutex_unlock(fuSeBMC_lock);
			   return;
			}
			FuSeBMC_goal_node_t * current = fuSeBMC_goal_list_head;
			while (current != NULL)
			{
				if(current->is_new == '0') break;
				fprintf(fptr,"%lu\n", current->val);
				current = current->next;
			}
			
			fclose(fptr);
		}
		
		
		//3 - Save the covered goals in Environment.
		
		/*const int env_var_size = 1024; // 512 MB
		char * buffer = malloc(env_var_size * sizeof(char));
		memset(buffer, '\0' , env_var_size);
		
		FuSeBMC_node_t * current = fuSeBMC_list_head;
		while (current != NULL)
		{
			sprintf(buffer, "%lu",current->val);
			current = current->next;
			if(current != NULL)
				sprintf(buffer, "|");
		}
		int res = setenv(fuSeBMC_run_id, buffer, 1);  // 1: overwrite the old value.
		fuSeBMC_print_val_in_file("./buffer.txt","buffer=%s\n", buffer);
		*/
		pthread_mutex_unlock(fuSeBMC_lock);

		
		//Must Exit.
		exit(code);
	}
}
void fuSeBMC_abort_prog()
{
	//cover-branches
	if(fuSeBMC_category == 2)
	{
		fuSeBMC_exit(0);
	}
	 /*if(fuSeBMC_lock != NULL)
	 {
		 pthread_mutex_destroy(fuSeBMC_lock);
		 free(fuSeBMC_lock);
	 }*/
	/*FILE *fptr;
	fptr = fopen("./abort.txt","a");
	if(fptr == NULL)
	{
	   printf("Error: cannot write to file:abort.txt!"); 
#ifdef MYDEBUG
	   exit(1);
#endif
	}
	fprintf(fptr,"fuSeBMC_category=%u\n",fuSeBMC_category);
	fclose(fptr);
*/
  	
	// code here
	//abort();
}
void fuSeBMC___assert_fail (const char *__assertion, const char *__file,
			   unsigned int __line, const char *__function)
{
	//cover-branches
	if(fuSeBMC_category == 2)
	{
		fuSeBMC_exit(0);
	}
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
/*void fuSeBMC_replace_char(char *str, char find, char replace)
{
	char *current_pos = strchr(str, find);
	while (current_pos)
	{
		*current_pos = replace;
		current_pos = strchr(current_pos, find);
	}
}*/

/*void fuSeBMC_parse_input_from(char *inp_var, char *format, void *destination)
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
}*/

/*void fuSeBMC_parse_input(char *format, void *destination)
{
	char * inp_var = fuSeBMC_get_input();
	fuSeBMC_parse_input_from(inp_var, format, destination);
	
	// NEW
	//free(inp_var);
}*/

char __VERIFIER_nondet_char()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	char val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof(char), stdin)) > 0) 
	{
		//fuSeBMC_print(val);
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _char,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _char,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
}

unsigned char __VERIFIER_nondet_uchar()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	unsigned char val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof(unsigned char), stdin)) > 0) 
	{
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _uchar,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _uchar,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
}

short __VERIFIER_nondet_short()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	short val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof(short), stdin)) > 0) 
	{
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _short,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _short,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
}

unsigned short __VERIFIER_nondet_ushort()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	unsigned short val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof(unsigned short), stdin)) > 0) 
	{
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _ushort,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _ushort,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
}

int __VERIFIER_nondet_int()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	signed int val = 0;
	int c;
	//setvbuf(stdin, (char*)NULL, _IOFBF, 0);// full buffering mode
	// read Int from STDIN.
	if ((c = fread(&val ,1 ,sizeof(signed int), stdin)) > 0) 
	{
		//Store the Value in the input_list to be exported as Testcase if needed.
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _int,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _int,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
			
}

unsigned int __VERIFIER_nondet_uint()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	unsigned int val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof( unsigned int), stdin)) > 0) 
	{
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _uint,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _uint,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
}

long __VERIFIER_nondet_long()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	long val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof(long), stdin)) > 0) 
	{
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _long,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _long,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
}

unsigned long __VERIFIER_nondet_ulong()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	unsigned long val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof(unsigned long), stdin)) > 0) 
	{
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _ulong,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _ulong,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
}

long long __VERIFIER_nondet_longlong()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	long long val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof(long long), stdin)) > 0) 
	{
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _longlong,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _longlong,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
}

unsigned long long __VERIFIER_nondet_ulonglong()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	unsigned long long val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof(unsigned long long), stdin)) > 0) 
	{
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _ulonglong,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _ulonglong,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
}

float __VERIFIER_nondet_float()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	float val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof(float), stdin)) > 0) 
	{
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _float,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _float,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
}

double __VERIFIER_nondet_double()
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	double val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof(double), stdin)) > 0) 
	{
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _double,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _double,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
}

_Bool __VERIFIER_nondet_bool()
{
	//return (_Bool)__VERIFIER_nondet_int();
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	_Bool val = 0;
	int c;
	if ((c = fread(&val ,1 , sizeof(_Bool), stdin)) > 0) 
	{
		fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _bool,&val);
		pthread_mutex_unlock(fuSeBMC_lock);
		return val;		
	}
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _bool,&val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return 0;
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
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);
	
	/*char *val = malloc(FUSEBMC_MAX_INPUT_SIZE + 1);
	// Read to end of line
	fuSeBMC_parse_input("%[^\n]", val);
	*/
	if(fuSeBMC_IsRandomHasSeed == 0)
	{
		//srand(time(0));
		srand(getpid());
		fuSeBMC_IsRandomHasSeed = 1;
	}
	
	for(int i = 0; i < 10; i++)rand();
	
	//int str_length = rand() % FUSEBMC_MAX_STRING_SIZE;
	int str_length = (rand() %
           (FUSEBMC_MAX_STRING_SIZE - FUSEBMC_MIN_STRING_SIZE + 1)) + FUSEBMC_MIN_STRING_SIZE;
	
	//fuSeBMC_print_val_in_file("./vals.txt","str_length=%d\n", str_length);
	char * val = malloc(sizeof(char) * str_length + 1); // +1 for '\0'
	/*if(val == NULL)
	{
		fuSeBMC_print_val_in_file("./exception.txt","%s\n", "Exc");
		exit(0);
	}*/
	memset(val, '\0' , str_length + 1);
	int c = 0;
	while(1)
	{
		if ((c = fread(val, sizeof(char), str_length , stdin)) >= FUSEBMC_MIN_STRING_SIZE)
		{
			//fuSeBMC_print_val_in_file("./vals.txt","numInLoop=%d\n", c);
			//fuSeBMC_print_val_in_file("./vals.txt","%s\n", val);
			fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _string, val);
			pthread_mutex_unlock(fuSeBMC_lock);
			return val;		
		}
	}
	//fuSeBMC_print_val_in_file("./vals.txt","numOutLoop=%d\n", c);
	//fuSeBMC_print_val_in_file("./vals.txt","%s\n", val);
	fuSeBMC_push_in_input_list(&fuSeBMC_input_list_head, _string,val);
	pthread_mutex_unlock(fuSeBMC_lock);
	return val;
}

/*char * fuSeBMC_get_input()
{
    char * inp_var = malloc(FUSEBMC_MAX_INPUT_SIZE);
    char * result = fgets(inp_var, FUSEBMC_MAX_INPUT_SIZE, stdin);
    if (result == 0)
    {
        fprintf(stderr, "No more test inputs available, exiting\n");
        exit(1);
    }
    unsigned int input_length = strlen(inp_var)-1;
    // Remove '\n' at end of input 
    if (inp_var[input_length] == '\n')
    {
        inp_var[input_length] = '\0';
    }
    return inp_var;
}
*/


void fuseGoalCalled(unsigned long int goal)
{
	fuSeBMC_init_mutex();	
    pthread_mutex_lock(fuSeBMC_lock);	
	/*if(inputIndex > 20)
	{
		__assert_fail ("0", "32_1_cilled_ok_nondet_linux-3.4-32_1-drivers--media--dvb--frontends--dvb_dummy_fe.ko-ldv_main0_sequence_infinite_withcheck_stateful.cil.out.c", 3, __extension__ __PRETTY_FUNCTION__); 
		kill(getppid(), SIGINT);
	}*/
	FuSeBMC_goal_node_t * search_node = fuSeBMC_find_in_goal_list(fuSeBMC_goal_list_head,goal);
    if(search_node == NULL)
    {
        fuSeBMC_push_in_goal_list(&fuSeBMC_goal_list_head , goal, '1'); // '1': new goal
		fuSeBMC_IsNewGoalsAdded = 1;
		
    }	
    pthread_mutex_unlock(fuSeBMC_lock);
}
#ifdef __cplusplus
}
#endif
