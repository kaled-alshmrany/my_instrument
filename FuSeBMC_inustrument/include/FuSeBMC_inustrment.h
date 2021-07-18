#ifndef FUSEBMC_INSTRUMENT_H
#define FUSEBMC_INSTRUMENT_H

#ifdef MYDEBUG
    extern int V_counter ;
    #define GET_LINE() "\n/*B_"+std::to_string(__LINE__)+"_("+ std::to_string(++V_counter) +")"+"*/\n"
    #define GET_LINE_E() "\n/*E_"+std::to_string(__LINE__)+"_("+ std::to_string(V_counter) +")"+"*/\n"
    #define D(x) std::string(x)
#else
    #define GET_LINE() std::string("")
    #define GET_LINE_E() std::string("")
    #define D(x) std::string("")
#endif
#define LOG(x) std::cout <<  #x" = "  << x << "   LINE=" << __LINE__ << std::endl;

enum InstrumentOption
{
    STMT_OPTION_NONE , 
    MUST_INSERT_ELSE ,
    //PARENT_IS_LOOP,
    STMT_IS_IF,
    STMT_IS_ELSE,
	
	PARENT_IS_FOR,
	PARENT_IS_CXX_FOR_RANGE,
	PARENT_IS_DO_WHILE,
	PARENT_IS_WHILE
	 
};

enum GoalType
{
    IF , 
    LOOP,
    AFTER_LOOP,
    ELSE,
    EMPTY_ELSE,
    COMPOUND,
    END_OF_FUNCTION,
	
	FOR,
	CXX_FOR_RANGE,
	DO_WHILE,
	WHILE	 
    //NONE
};

#endif

