#ifndef MYHOLDER_H
#define MYHOLDER_H

#include <clang/AST/ASTContext.h>

class MyHolder
{
public:
    clang::ASTContext * ASTContext;
    clang::SourceManager * SourceManager;
	//clang::LangOptions * LangOpts;
    MyHolder()
    {
        
    }
};


#endif 

