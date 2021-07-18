#include <GoalCounter.h>
#include <MyOptions.h>
#include <string>

extern MyOptions * myOptions;
GoalCounter GoalCounter::instance {};

GoalCounter& GoalCounter::getInstance()
{
   return instance;
}

GoalCounter::GoalCounter()
{
    this->funcLabelMap = new map<string,vector<string>>();
    this->vctIFGoals = new vector<unsigned long int>();
    this->vectLOOPGoals = new vector<unsigned long int>();
    this->vectAFTER_LOOPGoals = new vector<unsigned long int>();
    this->vectELSEGoals = new vector<unsigned long int>();
    this->vectEMPTY_ELSEGoals = new vector<unsigned long int>();
    this-> vectCOMPOUNDGoals = new vector<unsigned long int>();
    this->vectEND_OF_FUNCTIONGoals = new vector<unsigned long int>();
	this->vectLineNumberForNonDetCalls = new vector<NonDetCallInfo*>();
	
	this->vctForGoals = new vector<unsigned long int>();
	this->vctCXXForRangeGoals = new vector<unsigned long int>();
	this->vctDoWhileGoals = new vector<unsigned long int>();
	this->vctWhileGoals = new vector<unsigned long int>();
	
	this->vectGoalInfo = new vector<GoalInfo *>();
}

//GoalCounter::GoalCounter(GoalCounter const&)     = delete;

//void GoalCounter::operator=(GoalCounter const&)  = delete;

/*std::string GoalCounter::GetNewGoal()
{
    counter++;
    return std::string("\nGOAL_") + std::to_string(counter) + std::string(":;\n");
}*/
std::string GoalCounter::GetNewGoalForFunc(std::string func, GoalType goalType)
{
    counter++;
    std::string someGoal = std::string("\nGOAL_") + std::to_string(counter) + std::string(":;\n");
    if(mustGenerateFuncLabelMap)
        (*funcLabelMap)[func].push_back(std::string("GOAL_") + std::to_string(counter));
    if(!myOptions->infoFile.empty())
    {
        if(goalType == GoalType::IF)
            this->vctIFGoals->push_back(counter);
        else if(goalType == GoalType::LOOP)
            this->vectLOOPGoals->push_back(counter);
        else if(goalType == GoalType::AFTER_LOOP)
            this->vectAFTER_LOOPGoals->push_back(counter);
        else if(goalType == GoalType::ELSE)
            this->vectELSEGoals->push_back(counter);
        else if(goalType == GoalType::EMPTY_ELSE)
            this->vectEMPTY_ELSEGoals->push_back(counter);
        else if(goalType == GoalType::COMPOUND)
            this-> vectCOMPOUNDGoals->push_back(counter);
        else if(goalType == GoalType::END_OF_FUNCTION)
            this->vectEND_OF_FUNCTIONGoals->push_back(counter);
		
		else if(goalType == GoalType::FOR)
            this->vctForGoals->push_back(counter);
		else if(goalType == GoalType::CXX_FOR_RANGE)
            this->vctCXXForRangeGoals->push_back(counter);
		else if(goalType == GoalType::DO_WHILE)
            this->vctDoWhileGoals->push_back(counter);
		else if(goalType == GoalType::WHILE)
            this->vctWhileGoals->push_back(counter);
    }
        
    return someGoal;
}
NonDetCallInfo::NonDetCallInfo(unsigned long int p_line, string p_funcName):
line(p_line) , funcName(p_funcName)
{
	
}

GoalInfo::GoalInfo(unsigned long int p_goal, unsigned long int p_depth):
goal(p_goal), depth(p_depth)
{
	
}