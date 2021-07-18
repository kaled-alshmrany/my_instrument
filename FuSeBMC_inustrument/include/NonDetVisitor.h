#ifndef NONDETVISITOR_H
#define NONDETVISITOR_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtCXX.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/Decl.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Rewrite/Frontend/Rewriters.h>
#include <clang/Basic/SourceManager.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Rewrite/Frontend/Rewriters.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <clang/Lex/Token.h>
#include <clang/AST/ParentMap.h>

#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <iostream>

#include <MyHolder.h>
#include <GoalCounter.h>
#include <MyOptions.h>
#include <FuSeBMC_inustrment.h>

using namespace clang;
class NonDetVisitor : public RecursiveASTVisitor<NonDetVisitor>
{
    using Base = RecursiveASTVisitor<NonDetVisitor>;
    
public:
    Rewriter& TheRewriter;
    MyHolder& TheHolder;
	SourceManager * sourceManager;
    ASTContext * aSTContext = TheHolder.ASTContext;
	FunctionDecl* current_func;
	
    NonDetVisitor(Rewriter &R, MyHolder& H) : TheRewriter(R),TheHolder(H)
    {
		this->sourceManager = TheHolder.SourceManager;
		this->aSTContext = TheHolder.ASTContext;
    }
   
  bool TraverseDecl(Decl* decl);
  //void HandleReturnStmt(Stmt * s);
  bool VisitDecl(Decl * decl);
  bool VisitStmt(Stmt *s);
void searchForGoals(Stmt *s, int depth);  
};


#endif /* NONDETVISITOR_H */

