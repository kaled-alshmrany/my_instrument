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
#include <MyVisitor.h>
#include <GoalCounter.h>
#include <MyOptions.h>
#include <FuSeBMC_inustrment.h>
//#include <clang/ASTMatchers/ASTMatchFinder.h>
//#include <clang/ASTMatchers/ASTMatchers.h>

using namespace clang;
extern MyOptions * myOptions;
extern unsigned mainFileID;
GoalCounter & goalCounter = GoalCounter::getInstance();

bool MyVisitor::TraverseDecl(Decl* decl)
  {
      //llvm::outs() << "TTTTTTTT";
      ///decl->dumpColor();
      //if(decl->isFunctionOrFunctionTemplate()) 
      {
          //FunctionDecl *func = decl->getAsFunction();
          //const SourceManager &SM = *TheHolder.SourceManager;
          //const ASTContext *Context = TheHolder.ASTContext;
          //if(func->hasBody())
          //{
           //   Stmt * body =func->getBody();
            //  for (Stmt::child_iterator i = body->child_begin(), e = body->child_end(); i != e; ++i)
             // {
             //     Stmt *currStmt = *i;
              //    if(isa<ReturnStmt>(currStmt))
               //   {
                //      ReturnStmt * retStmt =cast<ReturnStmt>(currStmt);
                      //std::cout << "BEGIN STMMT" << std::endl;
                      //retStmt->dumpColor();
                      //std::cout << "END STMMT" << std::endl;
                //      TheRewriter.InsertTextBefore(retStmt->getLocStart().getLocWithOffset(0) ,"/*my return goal*/"+GoalCounter::getInstance().GetNewGoalForFunc(current_func->getNameAsString())+"");
                //  }
              //}
              //TheRewriter.InsertTextAfter(body->getLocStart().getLocWithOffset(1),"\r\n" + GoalCounter::getInstance().GetNewGoalForFunc(func->getNameAsString()));
          //}
         
          /*if(myOptions->replace_reach_error)
          {
            IdentifierInfo * identifierInfo = func->getIdentifier();          
            if (identifierInfo->getName()=="reach_error")
            {
                this->has_reach_error = true;
                //this->has_reach_error_body = func->hasBody();
                //if(func->hasBody())
                //    TheRewriter.ReplaceText(func->getNameInfo().getBeginLoc(), identifierInfo->getName().size(), " __VERIFIER_NEW_error "); 
            }
          }*/
      }
      //return true;
      return Base::TraverseDecl(decl);
  }
  
tok::TokenKind MyVisitor::getTokenKindInLocation(SourceLocation location)
{
    Token tmpToken;
    SourceLocation beginningOfToken = Lexer::GetBeginningOfToken(location, *this->sourceManager, this->aSTContext->getLangOpts());
    const bool invalid = Lexer::getRawToken(beginningOfToken, tmpToken, *this->sourceManager, this->aSTContext->getLangOpts());
    //assert(!invalid && "Expected a valid token.");
    if (invalid)
        return tok::NUM_TOKENS;
    return tmpToken.getKind();
}
  
bool MyVisitor::checkStmt(Stmt *S,SourceLocation initialLoc, SourceLocation endLocHint,
                            InstrumentOption instrumentOption,bool isIfIfOrElseIf)
{
    // 1) If there's a corresponding "else" or "while", the check inserts "} "
    // right before that token.
    // 2) If there's a multi-line block comment starting on the same line after
    // the location we're inserting the closing brace at, or there's a non-comment
    // token, the check inserts "\n}" right before that token.
    // 3) Otherwise the check finds the end of line (possibly after some block or
    // line comments) and inserts "\n}" right before that EOL.
    if (!S) return false;  
    //llvm::outs()<< "SSSSSSSSSSSSSS\r\n" << S->getStmtClassName() ;
    //S->dumpColor();
    
    const SourceManager &sourceManager = *TheHolder.SourceManager;
    ASTContext *astContext = TheHolder.ASTContext;

     // Already inside braces.
    if(isa<CompoundStmt>(S))
    {
        //while(x){compound_statement} ; if(x){compound_statement}
        CompoundStmt * compoundStmt = cast<CompoundStmt>(S);
        if(myOptions->addLabels)
        {
            GoalType tmpGoalType = GoalType::COMPOUND;
			
            if(instrumentOption == InstrumentOption::MUST_INSERT_ELSE ||
                    instrumentOption == InstrumentOption::STMT_IS_IF)
                tmpGoalType = GoalType::IF;
            else if(instrumentOption == InstrumentOption::STMT_IS_ELSE)
                tmpGoalType = GoalType::ELSE;
			else if(instrumentOption == InstrumentOption::PARENT_IS_FOR)
                tmpGoalType = GoalType::FOR;
			else if(instrumentOption == InstrumentOption::PARENT_IS_CXX_FOR_RANGE)
                tmpGoalType = GoalType::CXX_FOR_RANGE;
			else if(instrumentOption == InstrumentOption::PARENT_IS_DO_WHILE)
                tmpGoalType = GoalType::DO_WHILE;
			else if(instrumentOption == InstrumentOption::PARENT_IS_WHILE)
                tmpGoalType = GoalType::WHILE;

            TheRewriter.InsertTextAfter(compoundStmt->getLBracLoc().getLocWithOffset(1),
                (isIfIfOrElseIf? "" : GET_LINE() + goalCounter.GetNewGoalForFunc(current_func->getNameAsString(),tmpGoalType) +
					GET_LINE_E()));
        }

        if(instrumentOption == InstrumentOption::MUST_INSERT_ELSE && myOptions->addElse)
            TheRewriter.InsertTextBefore(compoundStmt->getRBracLoc().getLocWithOffset(1),
                    GET_LINE() + "\nelse\n{" +
                    goalCounter.GetNewGoalForFunc(current_func->getNameAsString(),GoalType::EMPTY_ELSE) + 
                    "}" + GET_LINE_E());
		
		bool is_for_CxxFor_while = (instrumentOption == InstrumentOption::PARENT_IS_FOR ||
									instrumentOption == InstrumentOption::PARENT_IS_CXX_FOR_RANGE ||
									instrumentOption == InstrumentOption::PARENT_IS_WHILE //||
									//instrumentOption == InstrumentOption::PARENT_IS_DO_WHILE 
		);
		
		//TODO: handle label after Loop When instrumentOption == InstrumentOption::PARENT_IS_DO_WHILE .


        //if(instrumentOption == InstrumentOption::PARENT_IS_LOOP && myOptions->addLabelAfterLoop)
		if(is_for_CxxFor_while  && myOptions->addLabelAfterLoop)
          TheRewriter.InsertTextBefore(compoundStmt->getRBracLoc().getLocWithOffset(1),
                        GET_LINE() + goalCounter.GetNewGoalForFunc(current_func->getNameAsString(),GoalType::AFTER_LOOP) + 
                        GET_LINE_E());
        return false;

    } // END If Compound

    // Treat macros.
    CharSourceRange fileRange = Lexer::makeFileCharRange(CharSourceRange::getTokenRange(S->getSourceRange()), sourceManager,astContext->getLangOpts());
    if (fileRange.isInvalid())
      return false;
  #ifdef MYDEBUG
    // InitialLoc points at the last token before opening brace to be inserted.
    assert(initialLoc.isValid());
  #endif
    // Convert InitialLoc to file location, if it's on the same macro expansion
    // level as the start of the statement. We also need file locations for
    // Lexer::getLocForEndOfToken working properly.
    initialLoc = Lexer::makeFileCharRange(
                     CharSourceRange::getCharRange(initialLoc, S->getBeginLoc()),
                     sourceManager, astContext->getLangOpts()).getBegin();
    if (initialLoc.isInvalid())
      return false;
    SourceLocation startLoc = Lexer::getLocForEndOfToken(initialLoc, 0, sourceManager, astContext->getLangOpts());

    // StartLoc points at the location of the opening brace to be inserted.
    SourceLocation endLoc;
    std::string closing;
    if (endLocHint.isValid())
    {
        endLoc = endLocHint;
        closing = "\n" + D("/*A*/") + "}\n";
        //TheRewriter.InsertTextAfter(endLoc,closing);
    } 
    else 
    {
        const auto fileRangeEndLoc = fileRange.getEnd().getLocWithOffset(-1);
        endLoc = findEndLocation(fileRangeEndLoc);
        closing = "\n" + D("/*B*/") + "}\n";
        //TheRewriter.InsertTextAfter(endLoc,closing);
    }
  #ifdef MYDEBUG
    assert(startLoc.isValid());
    assert(endLoc.isValid());
  #endif
   
    if(myOptions->addLabels)
    {
        GoalType tmpGoalType = GoalType::COMPOUND;
		
        if(instrumentOption == InstrumentOption::MUST_INSERT_ELSE ||
                    instrumentOption == InstrumentOption::STMT_IS_IF)
            tmpGoalType = GoalType::IF;
        else if(instrumentOption == InstrumentOption::STMT_IS_ELSE)
            tmpGoalType = GoalType::ELSE;
		
		else if(instrumentOption == InstrumentOption::PARENT_IS_FOR)
                tmpGoalType = GoalType::FOR;
		else if(instrumentOption == InstrumentOption::PARENT_IS_CXX_FOR_RANGE)
			tmpGoalType = GoalType::CXX_FOR_RANGE;
		else if(instrumentOption == InstrumentOption::PARENT_IS_DO_WHILE)
			tmpGoalType = GoalType::DO_WHILE;
		else if(instrumentOption == InstrumentOption::PARENT_IS_WHILE)
			tmpGoalType = GoalType::WHILE;

        TheRewriter.InsertTextAfter(startLoc,GET_LINE() + "\n{\n" +
                ((isIfIfOrElseIf?"" : goalCounter.GetNewGoalForFunc(current_func->getNameAsString(), tmpGoalType))) +
                GET_LINE_E());
    }
    if(instrumentOption == InstrumentOption::MUST_INSERT_ELSE && myOptions->addElse)
    {
        TheRewriter.InsertTextBefore(endLoc,GET_LINE() + closing + "else \n{" + 
                            goalCounter.GetNewGoalForFunc(current_func->getNameAsString(),GoalType::EMPTY_ELSE) + 
                            "}" + GET_LINE_E());
    }
    else 
    {
  #ifdef MYDEBUG
        //std::cout << "LINE:"<< endLoc.printToString(sourceManager) << " V_counter:" << V_counter << std::endl;
  #endif
        if(myOptions->addLabels)
        {
			bool is_for_CxxFor_while = (instrumentOption == InstrumentOption::PARENT_IS_FOR ||
							instrumentOption == InstrumentOption::PARENT_IS_CXX_FOR_RANGE ||
							instrumentOption == InstrumentOption::PARENT_IS_WHILE //||
							//instrumentOption == InstrumentOption::PARENT_IS_DO_WHILE 
							);
		
		//TODO: handle label after Loop When instrumentOption == InstrumentOption::PARENT_IS_DO_WHILE 
					
             //if(instrumentOption == InstrumentOption::PARENT_IS_LOOP && myOptions->addLabelAfterLoop)
			if(is_for_CxxFor_while && myOptions->addLabelAfterLoop)	 
                 closing += D("/*C*/") + goalCounter.GetNewGoalForFunc(current_func->getNameAsString(),GoalType::AFTER_LOOP);
            TheRewriter.InsertTextBefore(endLoc,GET_LINE() + closing + GET_LINE_E());
        }
    }

    return true;
}
 void MyVisitor::check(Stmt * S)
 {
	 
    //const SourceManager &SM = *(TheHolder.SourceManager);
    //const ASTContext *Context = TheHolder.ASTContext;
    /*if(myOptions->replace_reach_error)
    {
       if(isa<CallExpr>(S))
       {
          //std::cout << "IT IS CallExpr !!" << std::endl;
          CallExpr * call =cast<CallExpr>(S);
          FunctionDecl * func_decl = call->getDirectCallee(); 
          IdentifierInfo * identifierInfo = func_decl->getIdentifier();
          //unsigned StartLine = SM.getSpellingLineNumber(call->getLocStart());
          //std::cout << "StartLine=" << StartLine << std::endl;
          if (identifierInfo->getName()=="reach_error")
              TheRewriter.ReplaceText(call->getLocStart(), identifierInfo->getName().size(), " reach_original_error");
       }
    }*/

    /*if(isa<DeclRefExpr>(S))
    {
       std::cout << "IT IS" << std::endl;
       DeclRefExpr * decl =cast<DeclRefExpr>(S);
       std::cout <<  decl->getNameInfo().getAsString() << std::endl;
       FunctionDecl *func_decl = decl->getDirectCallee(); 
       //DeclarationName declarationName();
       //decl->getNameInfo().setName()

    }*/

    /*if (isa<DeclStmt>(S))
    {
        DeclStmt *declStmt = cast<DeclStmt>(S);
        declStmt->dumpColor();
        if(!declStmt->isSingleDecl())
        {
            for (DeclStmt::decl_iterator i = declStmt->decl_begin(), e = declStmt->decl_end(); i != e; ++i)
            {
                Decl * decl = *i;
                if(isa<VarDecl>(decl))
                {
                    VarDecl * varDecl = cast<VarDecl>(decl);
                    IdentifierInfo * identifierInfo = varDecl->getIdentifier();
                    std::cout << "VARNAME:" << identifierInfo->getName().str() << std::endl;
                    //varDecl->getNameAsString();

                   if(varDecl->hasInit())
                   {
                       Expr* varInit = varDecl->getInit();
                       if(varInit->isRValue())
                       {
                         // Works
             #if 0       
                         SourceRange varSourceRange = varInit->getSourceRange();
                         if(!varSourceRange.isValid())
                           return true;
                         CharSourceRange charSourceRange(varSourceRange, true);
                         StringRef sourceText = Lexer::getSourceText(charSourceRange, astContext->getSourceManager(), astContext->getLangOpts(), 0);
                         //llvm::outs() << ", initialization value: " << sourceText.str();
             #endif
                         if (isa<CallExpr>(varInit))
                         {
                           // Works
                           CallExpr *Call = dyn_cast<CallExpr>(varInit);
                           Decl *D = Call->getCalleeDecl();
                           FunctionDecl *FD = Call->getDirectCallee();
                           std::string fname = FD->getNameInfo().getAsString();
                           //if (FD->isExternC() && (fname == "malloc" || fname == "calloc"))
                           if(fname == "get")
                           {
                               std::cout << "JAAA" << std::endl;
                             //declarations.insert(varDecl);
                             //std::string str = "\ns2e_concretize_fork(" + name + ", " + "sizeof(" + name + "), " + "0" + ");\n";
                             //llvm::outs() << str;
                             //InstrumentStmtAfter(varDecl, str);
                           }
                         }
                       }
                 }


                }
            }
        }
        //for (Stmt::child_iterator i = declStmt->child_begin(), e = declStmt->child_end(); i != e; ++i)
           {
               //Stmt *currStmt = *i;
               //if(isa<VarDecl>(currStmt))
               //    std::cout << " VarDecl "<< std::endl;
           }
    }*/
     
    // Get location of closing parenthesis or 'do' to insert opening brace.
    if (isa<ForStmt>(S))
    {
        ForStmt *forStmt = cast<ForStmt>(S);
        checkStmt(forStmt->getBody(),forStmt->getRParenLoc(),SourceLocation(),InstrumentOption::PARENT_IS_FOR,false);
    }
    else if (isa<CXXForRangeStmt>(S))
    {
        CXXForRangeStmt * cXXForRangeStmt=cast<CXXForRangeStmt>(S);
        checkStmt(cXXForRangeStmt->getBody(), cXXForRangeStmt->getRParenLoc(),SourceLocation(),InstrumentOption::PARENT_IS_CXX_FOR_RANGE,false);
    }
    else if (isa<DoStmt>(S))
    {
        /**do {} while(x);*/
        DoStmt * doStmt = cast<DoStmt>(S);
        if(myOptions->addLabelAfterLoop)
        {
            SourceLocation locSemi = skipCommentAndWhitespaceUntilSemi(doStmt->getWhileLoc());
            if(locSemi.isValid())
                TheRewriter.InsertTextBefore(locSemi.getLocWithOffset(1),GET_LINE() + 
                            goalCounter.GetNewGoalForFunc(current_func->getNameAsString(),GoalType::AFTER_LOOP) +
                            GET_LINE_E());
        }
        checkStmt(doStmt->getBody(), doStmt->getDoLoc(), doStmt->getWhileLoc(),InstrumentOption::PARENT_IS_DO_WHILE,false);
    } 
    else if(isa<WhileStmt>(S))
    {
        WhileStmt * whileStmt = cast<WhileStmt>(S);
        SourceLocation startLoc = findRParenLocInIfOrWhile(whileStmt);
        if (startLoc.isInvalid())
            return;
      checkStmt(whileStmt->getBody(), startLoc, SourceLocation(), InstrumentOption::PARENT_IS_WHILE, false);
    } 
    else if(isa<IfStmt>(S))
    {
       
        IfStmt * ifStmt = cast<IfStmt>(S);
        SourceLocation startLoc = findRParenLocInIfOrWhile(ifStmt);
        if (startLoc.isInvalid())
            return;
        
        Stmt *Else = ifStmt->getElse();
        Stmt *Then = ifStmt->getThen();
        bool isIfIf = isa<IfStmt>(Then) || isCompoundWithOneIf(Then);
        //if(Else && !isa<IfStmt>(Else))
        if (Else) 
        {
            // We must check here (can without Else)
            bool isElseIf =isa<IfStmt>(Else) || isCompoundWithOneIf(Else);
            // Omit 'else if' statements here, they will be handled directly.
            checkStmt(Then, startLoc, ifStmt->getElseLoc(), InstrumentOption::STMT_IS_IF, isIfIf);
            checkStmt(Else, ifStmt->getElseLoc(), SourceLocation(),InstrumentOption::STMT_IS_ELSE, isElseIf);
        }
        else
        {
            checkStmt(Then, startLoc, ifStmt->getElseLoc(), InstrumentOption::MUST_INSERT_ELSE, isIfIf);            
        }
    }
    /*else if(isa<CompoundStmt>(S))
    {
        CompoundStmt * compoundStmt = cast<CompoundStmt>(S);
        TheRewriter.InsertTextAfter(compoundStmt->getRBracLoc().getLocWithOffset(-1),"\n;GOAL_XX:;\n");

    }*/
    /*else if(isa<DeclStmt>(S))
    {
        llvm::outs()<< "\nSSSSSSSSSSSSSS\r\n" << S->getStmtClassName() ;
        S->dumpColor();
    }*/
    else if(isa<SwitchStmt>(S))
    {
        //llvm::outs()<<  current_func->getNameAsString() << "\n";
         SwitchStmt * switchStmt = cast<SwitchStmt>(S);
         const SwitchCase * switchCase = switchStmt->getSwitchCaseList();
         while(switchCase != nullptr)
         {
            if(myOptions->addLabels)
                TheRewriter.InsertTextAfter(switchCase->getColonLoc().getLocWithOffset(1),
                            goalCounter.GetNewGoalForFunc(current_func->getNameAsString(),GoalType::IF));
            switchCase = switchCase->getNextSwitchCase();
         }
         
        //llvm::outs()<< "\nXXXXXXXXXX\r\n" << S->getStmtClassName() ;
        //S->dumpColor();
    }
	else if(isa<ReturnStmt>(S))
    {
		if(myOptions->handleReturnInMain && this->current_func != NULL && this->current_func->getNameAsString() == "main")
		{
			ReturnStmt * returnStmt = cast<ReturnStmt>(S);
			//returnStmt-> getRetValue()->dumpColor();
			//llvm::outs() << "ret_b:" << returnStmt->getBeginLoc().getRawEncoding() << "\n";
			//llvm::outs() << "ret_e:" << returnStmt->getEndLoc().getRawEncoding() << "\n";
			//TheRewriter.InsertTextAfter(returnStmt->getBeginLoc().getLocWithOffset(0) ,GET_LINE() +  D("/*ret_B*/") + GET_LINE_E());
			clang::Expr *expr = returnStmt->getRetValue();
			if (expr)
			{
				//TheRewriter.InsertTextAfter(expr->getBeginLoc() ,GET_LINE() +  "/*exp_b*/" + "" + GET_LINE_E());
				//TheRewriter.InsertTextAfter(expr->getEndLoc() ,GET_LINE() +  "/*exp_e*/" + "" + GET_LINE_E());
				SourceLocation expr_b = expr->getBeginLoc();
				bool isSemiFound = false;
				Token tmpToken;
				while(true)
				{
					while (isWhitespace(*FullSourceLoc(expr_b, *this->sourceManager).getCharacterData()))
						expr_b = expr_b.getLocWithOffset(1);				
					const bool invalid = Lexer::getRawToken(expr_b, tmpToken, *this->sourceManager, this->aSTContext->getLangOpts());
					//llvm::outs() << "expr_e_raw_loc:" << expr_b.getRawEncoding() << "\n";
					//llvm::outs() << "NAME:(" << tmpToken.getName() << ")\n";
					//llvm::outs() << "CharDta:(" << *FullSourceLoc(expr_b, *this->sourceManager).getCharacterData() << ")\n";
					if(!invalid)
					{
						//continue;
						if(tmpToken.getKind() == tok::semi)
						{
							//llvm::outs() << "SEMI  "  << "\n";
							isSemiFound = true;
							break;
						}
					}
					//expr_b = expr_b.getLocWithOffset(1);
					expr_b = Lexer::getLocForEndOfToken(expr_b, 0, *this->sourceManager, this->aSTContext->getLangOpts());
				} // End WHILE

				if(isSemiFound)
				{
					CharSourceRange charSourceRange = CharSourceRange::getCharRange(expr->getBeginLoc(), expr_b);
					if(charSourceRange.isValid())
					{
						const StringRef return_val_exp = Lexer::getSourceText(charSourceRange, *this->sourceManager, this->aSTContext->getLangOpts());
						TheRewriter.InsertTextAfter(returnStmt->getBeginLoc() ,GET_LINE() +  "fuSeBMC_return(" + return_val_exp.str() + ");" + GET_LINE_E());
						//llvm::outs() << "EXPRRRR:(" << return_val_exp.str() << ")\n";
					}
				}				
				
			}//if (expr)
			else // No expr : return;
				TheRewriter.InsertTextAfter(returnStmt->getBeginLoc() ,GET_LINE() +  "fuSeBMC_return(0);" + GET_LINE_E());
		}		 
	}
    else
    {
        //llvm_unreachable("Invalid match");
        //llvm::outs()<< "\nSSSSSSSSSSSSSS\r\n" << S->getStmtClassName() ;
        //llvm::outs()<< S->getLocStart().printToString(SM) << "\n";
        //S->dumpColor();
    }
}

 template <typename IfOrWhileStmt> 
 SourceLocation MyVisitor::findRParenLocInIfOrWhile(const IfOrWhileStmt * stmt)
 {
    // Skip macros.
    if (stmt->getBeginLoc().isMacroID()) return SourceLocation();
    
    SourceLocation condEndLoc = stmt->getCond()->getEndLoc();
    if(const DeclStmt * conditionVariable = stmt->getConditionVariableDeclStmt())
        condEndLoc = conditionVariable->getEndLoc();
#ifdef MYDEBUG
    assert(condEndLoc.isValid());
#endif
    /*if(!condEndLoc.isValid())
    {
        stmt->dump();
        stmt->getLocStart().dump(sourceManager);
        exit(0);
    }*/
    SourceLocation pastCondEndLoc = Lexer::getLocForEndOfToken(condEndLoc, 0, *this->sourceManager, this->aSTContext->getLangOpts());
    if (pastCondEndLoc.isInvalid())
        return SourceLocation();

    SourceLocation rParenLoc = skipCommentAndWhitespace(pastCondEndLoc);
    if (rParenLoc.isInvalid())
        return SourceLocation();

    tok::TokenKind TokKind = getTokenKindInLocation(rParenLoc);
    if (TokKind != tok::r_paren)
        return SourceLocation();

    return rParenLoc;
}
 
SourceLocation MyVisitor::skipCommentAndWhitespace(SourceLocation sourceLocation)
 {
#if MYDEBUG
    assert(sourceLocation.isValid());
#endif
    while (true) 
    {
        while (isWhitespace(*FullSourceLoc(sourceLocation, *this->sourceManager).getCharacterData()))
            sourceLocation = sourceLocation.getLocWithOffset(1);
        tok::TokenKind tokenKind = getTokenKindInLocation(sourceLocation);
        if (tokenKind == tok::NUM_TOKENS || tokenKind != tok::comment)
            return sourceLocation;
        // Fast-forward current token.
        sourceLocation = Lexer::getLocForEndOfToken(sourceLocation, 0, *this->sourceManager, this->aSTContext->getLangOpts());
    }
}

 SourceLocation MyVisitor::skipCommentAndWhitespaceUntilSemi(SourceLocation sourceLocation)
 {
#if MYDEBUG
    assert(sourceLocation.isValid());
#endif
    while(true)
    {
        while (isWhitespace(*FullSourceLoc(sourceLocation, *this->sourceManager).getCharacterData()))
            sourceLocation = sourceLocation.getLocWithOffset(1);
        tok::TokenKind tokenKind = getTokenKindInLocation(sourceLocation);
        if(tokenKind == tok::NUM_TOKENS || tokenKind != tok::comment)
            if(tokenKind == tok::semi)
                return sourceLocation;
        // Fast-forward current token.
        sourceLocation = Lexer::getLocForEndOfToken(sourceLocation, 0, *this->sourceManager, this->aSTContext->getLangOpts());
    }
}
SourceLocation MyVisitor::findEndLocation(SourceLocation lastTokenLoc)
{
    SourceLocation tmpLocation = lastTokenLoc;
    // Loc points to the beginning of the last (non-comment non-ws) token
    // before end or ';'.
#if MYDEBUG
    assert(tmpLocation.isValid());
#endif
    bool skipEndWhitespaceAndComments = true;
    tok::TokenKind tokenKind = getTokenKindInLocation(tmpLocation);
    if (tokenKind == tok::NUM_TOKENS || tokenKind == tok::semi || tokenKind == tok::r_brace) 
    {
        // If we are at ";" or "}", we found the last token. We could use as well
        // `if (isa<NullStmt>(S))`, but it wouldn't work for nested statements.
        skipEndWhitespaceAndComments = false;
    }
    tmpLocation = Lexer::getLocForEndOfToken(tmpLocation, 0,*this-> sourceManager, this->aSTContext->getLangOpts());
    // Loc points past the last token before end or after ';'.
    if (skipEndWhitespaceAndComments)
    {
        tmpLocation = skipCommentAndWhitespace(tmpLocation);
        tok::TokenKind tokenKind = getTokenKindInLocation(tmpLocation);
        if (tokenKind == tok::semi)
            tmpLocation = Lexer::getLocForEndOfToken(tmpLocation, 0, *this->sourceManager, this->aSTContext->getLangOpts());
    }
    while(true)
    {
#if MYDEBUG
        assert(tmpLocation.isValid());
#endif
        while (isHorizontalWhitespace(*FullSourceLoc(tmpLocation, *this->sourceManager).getCharacterData()))
            tmpLocation = tmpLocation.getLocWithOffset(1);
        if (isVerticalWhitespace(*FullSourceLoc(tmpLocation, *this->sourceManager).getCharacterData())) 
        {
            // EOL, insert brace before.
            break;
        }
        tok::TokenKind tokenKind = getTokenKindInLocation(tmpLocation);
        if (tokenKind != tok::comment)
        {
            // Non-comment token, insert brace before.
            break;
        }
        SourceLocation tokenEndLoc =Lexer::getLocForEndOfToken(tmpLocation, 0, *this->sourceManager, this->aSTContext->getLangOpts());
        SourceRange tokenRange(tmpLocation, tokenEndLoc);
        StringRef commentStr = Lexer::getSourceText(CharSourceRange::getTokenRange(tokenRange), *this->sourceManager, this->aSTContext->getLangOpts());
        if (commentStr.startswith("/*") && commentStr.find('\n') != StringRef::npos) 
        {
            // Multi-line block comment, insert brace before.
            break;
        }
        // else: Trailing comment, insert brace after the newline.
        // Fast-forward current token.
        tmpLocation = tokenEndLoc;
    }
    return tmpLocation;
}

/*bool MyVisitor::VisitFunctionDecl(FunctionDecl *func)
{
	string funcName = func->getNameInfo().getName().getAsString();
	string funcType = func->getReturnType().getAsString();
	cout << "Found function declaration: " << funcName << " of type " << funcType << "\n";
	return true;
}*/
bool MyVisitor::VisitDecl(Decl *decl)
{
	if(decl->isFunctionOrFunctionTemplate() && decl->hasBody())
	{
		const clang::FileEntry* f_entry = GetLocFileEntry(decl->getBeginLoc());
		const SourceManager & sourceManager = *TheHolder.SourceManager;
		
		//return sourceManager.getFileEntryForID(sourceManager.getFileID(loc));
		std::cout << "We are in Func: " << decl->getAsFunction()->getNameAsString() << std::endl;
		std::cout << "FILE = " << f_entry->getName().str() << std::endl;
		std::cout << "getUID = " << f_entry->getUID() << std::endl;
		//std::cout << "MID = " << sourceManager.getMainFileID(). << std::endl;
		std::cout << "-------------------------------" << std::endl;
		if(f_entry->getUID() != mainFileID) return true;
	}
	
    if (decl->isFunctionOrFunctionTemplate())
		this->current_func = decl->getAsFunction();
	
	
   
    if(decl->isFunctionOrFunctionTemplate() && decl->hasBody() && myOptions->addGoalAtEndOfFunc)         
    {
		std::cout << "We are in Func: " << decl->getAsFunction()->getNameAsString() << std::endl;
		//decl->PrintStats();
		//decl->dumpColor();
		
		
        Stmt*  body =decl->getBody();
        // Iterate over the top level of statements; 
        //not insert before
        // I don't know if we must check if the sub-statements include 'return'
        bool hasReturn = false;
        for (Stmt::child_iterator i = body->child_begin(), e = body->child_end(); i != e; ++i)
        {
            Stmt *currStmt = *i;
            if(isa<ReturnStmt>(currStmt))
            {
                hasReturn=true;
                ReturnStmt * retStmt =cast<ReturnStmt>(currStmt);
                 //std::cout << "BEGIN STMMT" << std::endl;
                 //retStmt->dumpColor();
                 //std::cout << "END STMMT" << std::endl;
                TheRewriter.InsertTextBefore(retStmt->getBeginLoc().getLocWithOffset(0) ,GET_LINE() + 
                       goalCounter.GetNewGoalForFunc(current_func->getNameAsString(),GoalType::END_OF_FUNCTION) + "" +
                       GET_LINE_E());
            }
        }
        // NOTE : insert Before
        if(!hasReturn)
            TheRewriter.InsertTextBefore(body->getEndLoc().getLocWithOffset(0),GET_LINE() + 
                       goalCounter.GetNewGoalForFunc(current_func->getNameAsString(),GoalType::END_OF_FUNCTION) +
                       GET_LINE_E());

    }

    if(decl->isFunctionOrFunctionTemplate() && decl->hasBody())
    {
        std::string lbl = myOptions->GetLabelForFunc(this->current_func->getNameAsString());
        if(!lbl.empty())
        {
            //std::cout <<"We have Label:" << lbl << std::endl;
            Stmt * body = decl->getBody();
            bool hasLbl = isLabelExistsInStement(body,lbl);
            //std::cout <<"hasLbl:" << hasLbl << std::endl;
            //std::cout <<"DONEEEEE:" << lbl << std::endl;
            if(!hasLbl)
            {
                //std::cout <<"Label:" << lbl << " will be added in function:" << this->current_func->getNameAsString() << std::endl;
                TheRewriter.InsertTextBefore(body->getBeginLoc().getLocWithOffset(1),GET_LINE() + lbl + ":;\n");
            }
        }
		
		// Add calls in Func.
		if(myOptions->addFuncCallInFunc)
		{
			std::string call = myOptions->GetCallForFunc(this->current_func->getNameAsString());
			if(!call.empty())
			{
				Stmt * body = decl->getBody();
				TheRewriter.InsertTextBefore(body->getBeginLoc().getLocWithOffset(1),GET_LINE() + call + "();\n");				
			}
			// Handle 'return' statements in main function.
			//std::cout << "WEEE are in: " << this->current_func->getNameAsString() << std::endl;
		}
		
   
    }
	if(decl->isFunctionOrFunctionTemplate() && decl->hasBody() &&
			myOptions->handleReturnInMain && this->current_func->getNameAsString() == "main")
		{
			Stmt * body = decl->getBody();
			//HandleReturnStmt(body);
			bool hasReturn = false;
			for (Stmt::child_iterator i = body->child_begin(), e = body->child_end(); i != e; ++i)
			{
				Stmt * child = *i;
				if(!child) continue;
				if(isa<ReturnStmt>(child))
				{
					hasReturn = true;
					break;
				}				
			}
			if(!hasReturn)
			{
				TheRewriter.InsertTextBefore(body->getEndLoc().getLocWithOffset(0),GET_LINE() + 
                       "fuSeBMC_return(0);" +
                       GET_LINE_E());
			}
		}
     return true;
  }


/** Defined in super class, will be automatically called*/

bool MyVisitor::VisitStmt(Stmt *s)
{
    if(!s) return true;
    //llvm::outs() << "SSSSSSSSSSSSSSSs\r\n" ;
    const SourceManager & sourceManager = *TheHolder.SourceManager;
    //const ASTContext * aSTContext = TheHolder.ASTContext;

    if(myOptions->showParseTree)
    {
        s->dumpColor();
        s->getBeginLoc().dump(sourceManager);
        //llvm::outs() << "SSSSSSSSSSSSSSSs\r\n" ;
    }
    check(s);
    return true;
}  
bool isLabelExistsInStement(Stmt * s , std::string & lbl)
{
    if(!s || s == nullptr) return false;
    if(isa<LabelStmt>(s))
    {
        LabelStmt * lblStmt =cast<LabelStmt>(s);
        if(lbl==lblStmt->getName())
            return true;
    }
    for (Stmt::child_iterator i = s->child_begin(), e = s->child_end(); i != e; ++i)
    {
        Stmt *currStmt = *i;
        if(isLabelExistsInStement(currStmt,lbl))
            return true;
    }
    return false;
  }
/**
 * Not used ...
 */
bool hasOneCompoundChild(Stmt * s)
{
    
    // Else if child  of If,
    bool isIf = s && isa<IfStmt>(s);
    
    unsigned int count =0;
    Stmt * child = nullptr;
    for (Stmt::child_iterator i = s->child_begin(), e = s->child_end(); i != e; ++i)
    {
        Stmt * ii = *i;
        if(!ii) continue;
        
        if(isa<NullStmt>(ii)) continue;
        if(isa<ImplicitCastExpr>(ii)) continue;
        if(isIf)
        {
            IfStmt * ifStmt =cast<IfStmt>(s);
            Stmt * Else = ifStmt->getElse();
            if(Else && Else == ii) continue;
        }
        count++;
        child = ii;
#ifdef MYDEBUG
        std::cout << "CHILD -->" << count <<std::endl;
        //std::cout << typeid(*i).name() << std::endl;
        i->dumpColor();
        std::cout << "-----------------------" << std::endl;
#endif
    }
    if(count==1)
    {
        //Stmt * child= *(s->child_begin());
        //if(child ) 
        if(isa<IfStmt>(child) || isa<ForStmt>(child) || isa<WhileStmt>(child) || isa<CXXForRangeStmt>(child)
                 || isa<SwitchStmt>(child) || isa<DoStmt>(child))
         {
#ifdef MYDEBUG
             std::cout << "Statement has one child: YES -->" << count <<std::endl;
             s->dumpColor();
             std::cout << "-----------------------" << std::endl;
#endif
             return true;
         }
        
    }

#ifdef MYDEBUG
             std::cout << "Statement has one child: NO -->" << count << std::endl;
             s->dumpColor();
             std::cout << "-----------------------" << std::endl;
#endif
    return false;

    
}
/**
 * if(x){if(x);}  or if(x);else{if(x);}
 */
bool isCompoundWithOneIf(Stmt * s)
{
    if(!isa<CompoundStmt>(s))
        return false;
    int counter=0;
    Stmt * lastNotNullChild = nullptr;
    CompoundStmt * compoundStmt=cast<CompoundStmt>(s);
    for (Stmt::child_iterator i = compoundStmt->child_begin(), e = compoundStmt->child_end(); i != e; ++i)
    {
        Stmt * child = *i;
        if(!child) continue;
        if(isa<NullStmt>(child)) continue;
        //if(isa<IfStmt>(child))
        counter++;
        lastNotNullChild = child;
        
    }
    if(!lastNotNullChild)
        return false;
    if(counter ==1 && isa<IfStmt>(lastNotNullChild)) 
        return true;
    return false;
}
const clang::FileEntry* MyVisitor::GetLocFileEntry(SourceLocation loc) 
{
	const SourceManager & sourceManager = *TheHolder.SourceManager;
	return sourceManager.getFileEntryForID(sourceManager.getFileID(loc));
}