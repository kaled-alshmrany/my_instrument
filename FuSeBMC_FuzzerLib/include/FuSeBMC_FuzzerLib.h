#ifndef FUSEBMC_FUZZER_LIB_H
#define FUSEBMC_FUZZER_LIB_H

#include "inputTypes.h"

#ifdef __cplusplus
extern "C" {
#endif
void fuSeBMC_readPrevTotalCoveredGoals();
void fuSeBMC_reach_error();
char * fuSeBMC_generate_random_text(int length);
void fuSeBMC_Write_InputList_in_Testcase(char * file_name);
void fuSeBMC_abort_prog();
void fuSeBMC_exit(int code);
void fuseGoalCalled(unsigned long int goal);


#ifdef __cplusplus
}
#endif
#endif

