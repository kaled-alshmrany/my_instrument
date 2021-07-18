#ifndef MYVISITOR_H
#define MYVISITOR_H

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

#include <cstdio>
#include <memory>
#include <sstream>
#include <string>

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
#include <FuSeBMC_inustrment.h>
#include <MyHolder.h>
#include <GoalCounter.h>

//#include <clang/ASTMatchers/ASTMatchFinder.h>
//#include <clang/ASTMatchers/ASTMatchers.h>



using namespace clang;
bool isLabelExistsInStement(Stmt * s , std::string & lbl);
bool hasOneCompoundChild(Stmt * s);
/**
 * if(x){if(x);}  or if(x);else{if(x);}
 * @param s
 * @return 
 */
bool isCompoundWithOneIf(Stmt * s);
class MyVisitor : public RecursiveASTVisitor<MyVisitor>
{
    using Base = RecursiveASTVisitor<MyVisitor>;
    
public:
    Rewriter& TheRewriter;
    MyHolder& TheHolder;
    FunctionDecl* current_func;
    //SourceManager * TheSourceManager;
    //bool has_reach_error= false;
    //bool has_reach_error_body = false;
	
	SourceManager * sourceManager;
    ASTContext * aSTContext = TheHolder.ASTContext;
	
    MyVisitor(Rewriter &R, MyHolder& H) : TheRewriter(R),TheHolder(H)
    {
		this->sourceManager = TheHolder.SourceManager;
		this->aSTContext = TheHolder.ASTContext;
    }
   
  bool TraverseDecl(Decl* decl);
  tok::TokenKind getTokenKindInLocation(SourceLocation location);
  bool checkStmt(Stmt *S,SourceLocation initialLoc, SourceLocation endLocHint = SourceLocation(), 
                InstrumentOption instrumentOption = InstrumentOption::STMT_OPTION_NONE, bool isIfIfOrElseIf = false);
  void check(Stmt * S);
  template <typename IfOrWhileStmt> 
    SourceLocation findRParenLocInIfOrWhile(const IfOrWhileStmt * stmt);
  SourceLocation skipCommentAndWhitespace(SourceLocation sourceLocation);
  SourceLocation skipCommentAndWhitespaceUntilSemi(SourceLocation sourceLocation);
  SourceLocation findEndLocation(SourceLocation lastTokenLoc);
  bool VisitDecl(Decl *decl);
  //bool VisitFunctionDecl(FunctionDecl *func);
  bool VisitStmt(Stmt *s);
  const clang::FileEntry* GetLocFileEntry(SourceLocation loc);
};
#endif

