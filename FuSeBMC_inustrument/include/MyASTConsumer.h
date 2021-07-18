#ifndef MYASTCONSUMER_H
#define MYASTCONSUMER_H

#include <clang/AST/ASTConsumer.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Rewrite/Frontend/Rewriters.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include <MyHolder.h>
#include <MyVisitor.h>
using namespace clang;

class MyASTConsumer:public ASTConsumer
{
 public:
     MyASTConsumer(Rewriter &Rewrite,MyHolder& H);
     virtual bool HandleTopLevelDecl(DeclGroupRef d);
	 virtual ~MyASTConsumer();
     MyVisitor rv;
     MyHolder& TheHolder;
};


#endif /* MYASTCONSUMER_H */

