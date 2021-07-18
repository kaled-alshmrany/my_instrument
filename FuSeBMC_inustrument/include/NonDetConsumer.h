#ifndef NONDETCONSUMER_H
#define NONDETCONSUMER_H

#include <clang/AST/ASTConsumer.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Rewrite/Frontend/Rewriters.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include <MyHolder.h>
#include <NonDetVisitor.h>

using namespace clang;

class NonDetConsumer : public ASTConsumer
{
 public:
     NonDetConsumer(Rewriter &Rewrite,MyHolder& H);
     virtual bool HandleTopLevelDecl(DeclGroupRef d);
	 virtual ~NonDetConsumer();
     NonDetVisitor  rv;
     MyHolder& TheHolder;
};



#endif /* NONDETCONSUMER_H */

