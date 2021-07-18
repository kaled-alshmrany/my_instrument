#include <MyOptions.h>
#include <vector>
#include <algorithm>

string MyOptions::GetLabelForFunc(string func)
{
    if(!this->addLabelInFunc)
        return "";
    map<string,string>::iterator it = this->funcLabelMap.find(func);
    if(it != this->funcLabelMap.end())
    {
        return  it->second;
    }
    return "";
}
string MyOptions::GetCallForFunc(string func)
{
    if(!this->addFuncCallInFunc)
        return "";
    map<string,string>::iterator it = this->funcCallMap.find(func);
    if(it != this->funcCallMap.end())
    {
        return  it->second;
    }
    return "";
}
vector<string> * MyOptions::GetUniqueCalls()
{
	vector<string> * vect = new vector<string>();
	for(map<string, string>::iterator it = funcCallMap.begin();
			it != funcCallMap.end();
			it++)
	{
		//it->first :func
		//it->second :call
		if (std::find(vect->begin(), vect->end(), it->second) == vect->end())
		{
			// Element NOT in vector.
			vect->push_back(it->second);
		}

	}
	return vect;
}
