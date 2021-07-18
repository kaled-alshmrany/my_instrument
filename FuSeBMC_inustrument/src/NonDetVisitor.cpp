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
#include <NonDetVisitor.h>
#include <GoalCounter.h>
#include <MyOptions.h>
#include <FuSeBMC_inustrment.h>

using namespace clang;
extern MyOptions * myOptions;

GoalCounter& goalCounterNonDet = GoalCounter::getInstance();
bool NonDetVisitor::TraverseDecl(Decl* decl)
{
	//llvm::outs() << "KKKKKKKKKKK\r\n" ;
	return Base::TraverseDecl(decl);
}
bool NonDetVisitor::VisitDecl(Decl *decl)
{
	if (decl->isFunctionOrFunctionTemplate())
	{
		this->current_func = decl->getAsFunction();
	}
	/*if(decl->isFunctionOrFunctionTemplate() && decl->hasBody())
    {
		
	}*/
	
	
	if(decl->isFunctionOrFunctionTemplate() && decl->hasBody())
    {
		if(myOptions->exportGoalInfo)
		{
			Stmt * stmtBody = decl->getBody();
			searchForGoals(stmtBody,0);
		}
	}
	return true;
}
void NonDetVisitor::searchForGoals(Stmt *s, int depth)
{
	if(!s || s == nullptr) return;
	//s->dumpColor();
	if(isa<LabelStmt>(s))
	{
		LabelStmt * labelStmt = cast<LabelStmt>(s);
		const char * lblName = labelStmt->getName();
		size_t lblName_len = strlen(lblName);
		if(lblName_len > 5) // "GOAL_120"
		{
			if(lblName[0] == 'G' && lblName[1] == 'O' && lblName[2] == 'A' && lblName[3] == 'L' && lblName[4] == '_')
			{
				int lblNumber_len = lblName_len - 5 + 1; //+1 for '\0';
				char * lblNumber =(char *) malloc(lblNumber_len * sizeof(char));
				int loop = 0;
				for(int i=5; i < lblName_len;i++)
				{
					lblNumber[loop] = lblName[i];
					loop++;
				}
				lblNumber[lblNumber_len-1] = '\0';
				//std::cout << "lblNumber=" << lblNumber << " Depth = " << depth << std::endl;
				
				//https://www.cplusplus.com/reference/cstdlib/strtoul/
				char *eptr;
				unsigned long int goal = strtoul(lblNumber, &eptr, 10); //base = 10
				if (goal == 0)
				{
					/* If a conversion error occurred, display a message and exit */
					if (errno == EINVAL)
					{
						printf("Conversion error occurred: %d\n", errno);
						//exit(0);
					}
					/* If the value provided was out of range, display a warning message */
					if (errno == ERANGE)
						printf("The value provided was out of range\n");
				}
				else if(goal > 0)
				{
					GoalInfo * goalInfo = new GoalInfo(goal,depth);
					goalCounterNonDet.vectGoalInfo->push_back(goalInfo);
				}
				free(lblNumber);				
			}
		}
		
	}
	for (Stmt::child_iterator i = s->child_begin(), e = s->child_end(); i != e; ++i)
	{
		Stmt * child = *i;
		if(!child) continue;
		if(isa<NullStmt>(child)) continue;
		searchForGoals(child, depth+1);
	}
}

bool NonDetVisitor::VisitStmt(Stmt *s)
{
    if(!s) return true;
    //llvm::outs() << "SSSSSSSSSSSSSSSs\r\n" ;
    //const SourceManager & sourceManager = *TheHolder.SourceManager;
    //const ASTContext * aSTContext = TheHolder.ASTContext;
	
	   
    //check(s);
	if(myOptions->exportLineNumberForNonDetCalls)
	{
		if(isa<CallExpr>(s))
		  {
			//std::cout << "IT IS CallExpr !!" << std::endl;
			CallExpr * call =cast<CallExpr>(s);
			FunctionDecl * func_decl = call->getDirectCallee(); 
			IdentifierInfo * identifierInfo = func_decl->getIdentifier();
			//unsigned StartLine = SM.getSpellingLineNumber(call->getLocStart());
			//std::cout << "StartLine=" << StartLine << std::endl;
			if (identifierInfo->getName().startswith("__VERIFIER_nondet_"))
			{
				// s starts with prefix
				unsigned startLine = this->sourceManager->getSpellingLineNumber(call->getBeginLoc());
				
				/*if (std::find(goalCounterNonDet.vectLineNumberForNonDetCalls->begin(),
						goalCounterNonDet.vectLineNumberForNonDetCalls->end(),startLine)
						== goalCounterNonDet.vectLineNumberForNonDetCalls->end())*/
				/*for(vector<NonDetCallInfo>::iterator ptr = goalCounter.vectLineNumberForNonDetCalls->begin();
					ptr != goalCounter.vectLineNumberForNonDetCalls->end(); ptr++)*/
				{
					NonDetCallInfo * NonDetCallInfo_ptr = new NonDetCallInfo(startLine,identifierInfo->getName().str());
					goalCounterNonDet.vectLineNumberForNonDetCalls->push_back(NonDetCallInfo_ptr);
				}

	#if MYDEBUG
				std::cout << "NonDetCall in startLine=" << startLine << std::endl;
	#endif
			}
			//if (identifierInfo->getName()=="reach_error")
			//	TheRewriter.ReplaceText(call->getBeginLoc(), identifierInfo->getName().size(), " reach_original_error");
		  }
    
	}
	return true;
} 