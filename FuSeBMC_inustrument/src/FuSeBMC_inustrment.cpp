#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <system_error>
//#include <unistd.h>

#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>

//#include <clang/Basic/DiagnosticOptions.h>
//#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/Lexer.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Rewrite/Frontend/Rewriters.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/Builtins.h>
#include <llvm/Support/Path.h>
#include <clang/Tooling/Tooling.h>

#include <MyASTConsumer.h>
#include <NonDetConsumer.h>
#include <FuSeBMC_inustrment.h>
#include <MyVisitor.h>
#include <NonDetVisitor.h>
#include <MyHolder.h>
#include <GoalCounter.h>
#include <MyOptions.h>
#include <iostream>
#include <fstream>
#include <regex>




#define FuSeBMC_inustrment_VERSION "Version 1.0.0"

using namespace clang;

bool mustPrintGeneratedCode = false;
MyOptions * myOptions;
bool isOutFileWritten = false;
unsigned mainFileID = 0;
#ifdef MYDEBUG
    int V_counter =0 ;
#endif

void usage(char * prog)
{
    std::cout << prog <<" --input inp --output outpt [-h|--help] [-v|--version] [--goal-output-file file] [--info-file file] [--show-parse-tree] "<<std::endl;
    std::cout <<"\t\t\t\t [--goal-pro-func-output-dir dir] [--add-else] [--add-labels] " << std::endl;
    std::cout <<"\t\t\t\t [--add-label-after-loop] [--add-goal-at-end-of-func] " << std::endl;
    std::cout<<"\t\t\t\t[--add-label-in-func lbl_func pairs] [--add-FuSeBMC-func] [--compiler-args][..][..]"<<std::endl;
    std::cout << "--help|-h\t\tShow the usage."<<std::endl;
    std::cout << "--version|-v\t\tPrints the version of this Software."<<std::endl;
    std::cout << "--input inp\t\tThe C input source file to be instrumented"<<std::endl;
    std::cout << "--output outpt\t\t The output file that will be created, includes the instrumented source code. "<<std::endl;
    std::cout << "--goal-output-file file\t\tThe Number of Generated Goals will be stored in this file."<<std::endl;
    std::cout << "--info-file file\t\t XML Output file contains info about goals."<<std::endl;
    std::cout << "--show-parse-tree\t\t Show the parse tree of the source code."<<std::endl;
    std::cout << "--goal-pro-func-output-dir dir\t\tFor each function in the source,It will be generated a file(same name of function)"<< std::endl;
    std::cout<<"\t\t\t\tthat includes the goals in  this function, these generated files will be stored in this directory."<<std::endl;
    std::cout << "--add-else\t\tAdd 'Else' statement if not exits."<<std::endl;
    std::cout << "--add-labels\t\tAdd labels in statements(if,else,while,for...)."<<std::endl;
    std::cout << "--add-label-after-loop\t\tAdd label after (while,for,do_while,for c++)."<<std::endl;
    std::cout << "--add-goal-at-end-of-func\t\tA Goal will be added at the end of each function (or before 'return')"<<std::endl;    
    std::cout << "--add-label-in-func lbl_func pairs\t\tAdd one label in the corresponding function: lbl1=func1,lbl2=func2"<<std::endl;
	std::cout << "--add-func-call-in-func func_func pairs\t\tAdd functions call in the corresponding function: newCall1=main,newCall2=func"<<std::endl;
    std::cout << "--add-FuSeBMC-func add 'void FuSeBMC_custom_func(void){}' to the instrumented file." << std::endl;
	std::cout << "--handle-return-in-main add 'fuSeBMC_return()' before 'return' in 'main' function." << std::endl;
	std::cout << "--export-line-number-for-NonDetCalls export line number (in info file) where NonDet methods are called." << std::endl;
    std::cout << "--export-goal-info export the depth of the goals in info.xml file." << std::endl;
	std::cout << "--compiler-args\t\tThe next arguments will be passed to the compiler."<<std::endl;
    std::cout << "Example 1:" << prog << " --input mysource.c --output myout.c --goal-output-file theGoals.txt "<<std::endl;
    std::cout << "Example 2:" << prog << " --input mysource.c --output myout.c --goal-pro-func-output-dir /home/myout "<<std::endl;
    std::cout << "Example 3:" << prog << " --input mysource.c --output myout.c --compiler-args -I/user/include -I/home/dir1"<<std::endl;
}

std::vector<std::string> * split(std::string s,std::string del)
{
    std::vector<std::string> * vect = new std::vector<std::string>();
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(del)) != std::string::npos)
    {
        token = s.substr(0, pos);
        vect->push_back(token);
        s.erase(0, pos + del.length());
    }
    vect->push_back(s);
    return vect;
}
/**./a/b/c is file or dir ?? **/
bool createDirs(std::string & path,bool isDir = true)
{
    llvm::SmallString<1024> OutputDirectory(path);
    if(!isDir)
        llvm::sys::path::remove_filename(OutputDirectory);
    //const auto abs_path = clang::tooling::getAbsolutePath(path);
    //std::cout << "OutputDirectory :"<< OutputDirectory.c_str() << std::endl;
    //const auto directory = llvm::sys::path::parent_path(abs_path);
    //const auto directory = llvm::sys::path::root_path(path);
    //std::cout << "Create :"<< directory.data() << std::endl;
    if (!OutputDirectory.empty())
    {
        if (auto ec = llvm::sys::fs::create_directories(OutputDirectory)) 
        {
            llvm::errs() <<"failed to create output directory: " << OutputDirectory << "\n" << ec.message().c_str();
            return false;
        }
    }
    
    return true;      
}

//https://stackoverflow.com/questions/4643512/replace-substring-with-another-substring-c
/*void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos)
    {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}*/


int main(int argc, char **argv)
{

  myOptions = new MyOptions();
  GoalCounter& goalCounter = GoalCounter::getInstance();
  int compilerArgsIndex = argc;
  for(int i=0;i<argc;i++)
  {
      if(strcmp(argv[i],"--compiler-args")==0)
      {
          if(compilerArgsIndex!=argc)
          {
              std::cerr << "duplicated argument: --compiler-args" << std::endl;
              usage(argv[0]);
              exit(-1);
              
          }
          else
              compilerArgsIndex=i;
      }
  }
  
  for(int i=0;i<compilerArgsIndex;i++)
  {
      if(strcmp(argv[i],"--help")==0 || strcmp(argv[i],"-h")==0)
      {
          usage(argv[0]);
          exit(0);
      }
      else if(strcmp(argv[i],"--version")==0 || strcmp(argv[i],"-v")==0)
      {
          std::cout << FuSeBMC_inustrment_VERSION << std::endl;
          exit(0);
      }
  }
  int arg_index=1;  
  while(true)  
  {
      if(arg_index==compilerArgsIndex)break;
      char * the_arg=argv[arg_index];
      if(strcmp(the_arg,"--input")==0)
      {
          arg_index++;
          if(arg_index==compilerArgsIndex || argv[arg_index][0]=='-')
          {
              std::cerr << "No Input" << std::endl;
              usage(argv[0]);
              exit(-1);
          }          
          myOptions->inputFile =  argv[arg_index];
      }
      else if(strcmp(the_arg,"--output")==0)
      {
          arg_index++;
          if(arg_index==compilerArgsIndex || argv[arg_index][0]=='-')
          {
              std::cerr << "No Output" << std::endl;
              usage(argv[0]);
              exit(-1);
          }        
          myOptions->outputFile =  argv[arg_index];
          if(!createDirs(myOptions->outputFile,false))
              exit(-1);
      }
      else if(strcmp(the_arg,"--goal-output-file")==0)
      {
          arg_index++;
          if(arg_index==compilerArgsIndex || argv[arg_index][0]=='-')
          {
              std::cerr << "No goal-output-file" << std::endl;
              usage(argv[0]);
              exit(-1);
          }          
          myOptions->goalOutputFile =  argv[arg_index];
          if(!createDirs(myOptions->goalOutputFile,false))
              exit(-1);
      }
      else if(strcmp(the_arg,"--info-file")==0)
      {
          arg_index++;
          if(arg_index==compilerArgsIndex || argv[arg_index][0]=='-')
          {
              std::cerr << "No info-file" << std::endl;
              usage(argv[0]);
              exit(-1);
          }          
          myOptions->infoFile =  argv[arg_index];
          if(!createDirs(myOptions->infoFile,false))
              exit(-1);
      }
      else if(strcmp(the_arg,"--show-parse-tree")==0)
      {
        myOptions->showParseTree=true;
      }
      else if(strcmp(the_arg,"--goal-pro-func-output-dir")==0)
      {
          arg_index++;
          if(arg_index==compilerArgsIndex || argv[arg_index][0]=='-')
          {
              std::cerr << "No goal-pro-func-output-dir" << std::endl;
              usage(argv[0]);
              exit(-1);
          }          
          myOptions->goalProFuncOutputDir =  argv[arg_index];
          if(!createDirs(myOptions->goalProFuncOutputDir,true ))
              exit(-1);
      }
      else if(strcmp(the_arg,"--add-else")==0)
      {
          myOptions->addElse = true;
      }
      else if(strcmp(the_arg,"--add-labels")==0)
      {
          myOptions->addLabels = true;
      }
      else if(strcmp(the_arg,"--add-label-after-loop")==0)
      {
          myOptions->addLabelAfterLoop = true;
      }
      else if(strcmp(the_arg,"--add-goal-at-end-of-func")==0)
      {
          myOptions->addGoalAtEndOfFunc = true;
      }
      else if(strcmp(the_arg,"--add-FuSeBMC-func")==0)
      {
          myOptions->addFuSeBMCFunc = true;
      } 
	  else if(strcmp(the_arg,"--handle-return-in-main")==0)
      {
          myOptions->handleReturnInMain = true;
      }
	  else if(strcmp(the_arg,"--export-line-number-for-NonDetCalls")==0)
      {
          myOptions->exportLineNumberForNonDetCalls = true;
      }
      else if(strcmp(the_arg,"--add-label-in-func")==0)
      {
          myOptions->addLabelInFunc = true;
          arg_index++;
          if(arg_index==compilerArgsIndex || argv[arg_index][0]=='-')
          {
              std::cerr << "No value for :add-label-in-func" << std::endl;
              usage(argv[0]);
              exit(-1);
          }          
          
          std::vector<std::string> * lbl_func_pairs=split(argv[arg_index],",");
          for(std::string lbl_func_pair : *lbl_func_pairs)
          {
              std::vector<std::string> * lbl_func=split(lbl_func_pair,"=");
              assert(lbl_func->size()==2);
              myOptions->funcLabelMap[(*lbl_func)[1]]=(*lbl_func)[0];                      
              delete lbl_func;
          }
          delete lbl_func_pairs;
      }
	  else if(strcmp(the_arg,"--add-func-call-in-func")==0)
      {
          myOptions->addFuncCallInFunc = true;
          arg_index++;
          if(arg_index==compilerArgsIndex || argv[arg_index][0]=='-')
          {
              std::cerr << "No value for :add-func-call-in-func" << std::endl;
              usage(argv[0]);
              exit(-1);
          }          
          //--add-func-call-in-func
          std::vector<std::string> * call_func_pairs=split(argv[arg_index],",");
          for(std::string call_func_pair : *call_func_pairs)
          {
              std::vector<std::string> * call_func=split(call_func_pair,"=");
              assert(call_func->size()==2);
              myOptions->funcCallMap[(*call_func)[1]]=(*call_func)[0];                      
              delete call_func;
          }
          delete call_func_pairs;
      }
	  else if(strcmp(the_arg,"--export-goal-info")==0)
      {
          myOptions->exportGoalInfo = true;
      }
      else 
      {
          std::cerr << "unknown option:" << the_arg << std::endl;
          usage(argv[0]);
          exit(-1);
      }
      arg_index++;
      
  }
#ifdef MYDEBUG
  std::cout <<"myOptions->inputFile=" << myOptions->inputFile << std::endl;
  std::cout <<"myOptions->outputFile=" << myOptions->outputFile << std::endl;
  std::cout <<"myOptions->goalOutputFile=" << myOptions->goalOutputFile << std::endl;
  std::cout <<"myOptions->infoFile=" << myOptions->infoFile << std::endl;
  std::cout <<"myOptions->goalProFuncOutputDir=" << myOptions->goalProFuncOutputDir << std::endl;
  std::cout <<"myOptions->addElse=" << myOptions->addElse << std::endl;
  std::cout <<"myOptions->addGoalAtEndOfFunc=" << myOptions->addGoalAtEndOfFunc << std::endl;
  std::cout <<"myOptions->showParseTree=" << myOptions->showParseTree << std::endl;
  std::cout <<"myOptions->addLabelInFunc=" << myOptions->addLabelInFunc << std::endl;
  std::cout <<"myOptions->addLabels=" << myOptions->addLabels << std::endl;
  std::cout <<"myOptions->addLabelAfterLoop=" << myOptions->addLabelAfterLoop << std::endl;
  std::cout <<"myOptions->addFuSeBMCFunc=" << myOptions->addFuSeBMCFunc << std::endl;
  std::cout <<"myOptions->exportLineNumberForNonDetCalls=" << myOptions->exportLineNumberForNonDetCalls << std::endl;
  for (const auto& map_elem : myOptions->funcLabelMap)
  {
      std::cout << map_elem.first  << ':' << map_elem.second << std::endl ;
  }
#endif
  if(myOptions->inputFile.empty())
  {
	  std::cerr <<"No Input File "<< std::endl;
      usage(argv[0]);
      exit(-1);
  }
  
  // Make sure it exists
  struct stat sb;
  if (stat(myOptions->inputFile.c_str(), &sb) == -1)
  {
    std::cerr << "Cannot open file:" << myOptions->inputFile << std::endl;
    exit(EXIT_FAILURE);
  }
  if(myOptions->outputFile.empty())
  {
      std::cerr << "No output File" <<std::endl;
      usage(argv[0]);
      exit(-1);
  }
  int new_args_count = argc - compilerArgsIndex + 1;
  char ** new_argv =(char**) malloc(sizeof(char *) * new_args_count);
  int j=0;
  for(int i = compilerArgsIndex+1; i<new_args_count;i++)
  {
      new_argv[j] = argv[i];
      j++;
  }
  new_argv[new_args_count-1] =const_cast<char *>(myOptions->inputFile.c_str()); 
  
  SmallVector<char,1024> path_vect;
  llvm::sys::fs::real_path(argv[0],path_vect,true);
  std::string current_folder = llvm::sys::path::parent_path(path_vect.data()).str();
  //std::cout << path_vect.data() << std::endl;
  //std::cout << llvm::sys::path::parent_path(path_vect.data()).str() << std::endl;
  
  /*argc=6;
  char *argvv[]={"./my_instrument", "./examples/a.cpp", "./examples/uftp_out.c" , "./examples/uftp_goals.txt",
  "./examples/goals_out", "-I/home/kaled/sdb1/uFTP/library", "-DOPENSSL_ENABLE"};
  argv=argvv;
  */
  
  
   /*std::string inputFile(argv[1]);
   std::string outputFile(argv[2]);
   std::string goalOutputFile(argv[3]);
   std::string goalProFuncDir(argv[4]);
   if(goalProFuncDir != "-nogoalProFunc") */
   GoalCounter::getInstance().mustGenerateFuncLabelMap = !myOptions->goalProFuncOutputDir.empty();
  //std::string fileName(argv[argc - 1]);

  CompilerInstance compiler;
  DiagnosticOptions diagnosticOptions;
  compiler.createDiagnostics();
  //compiler.createDiagnostics(argc, argv);
  
  /*for(int i=0;i<new_args_count;i++)
  {
      std::cout << i << "=" << new_argv[i] << std::endl;
  }*/
  
  // Create an invocation that passes any flags to preprocessor
  CompilerInvocation *Invocation = new CompilerInvocation;
  //bool compile_result = CompilerInvocation::CreateFromArgs(*Invocation, new_argv, new_argv + new_args_count - 1, compiler.getDiagnostics());
  ArrayRef<const char *> arr(new_argv,new_args_count);
  bool compile_result = CompilerInvocation::CreateFromArgs(*Invocation, arr, compiler.getDiagnostics());
  if(!compile_result)
  {
      llvm::errs() << "Cannot create CompilerInvocation.\n";
      exit(-1);
  }
  compiler.setInvocation(std::shared_ptr<CompilerInvocation>(Invocation));

  // Set default target triple
  std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
  pto->Triple = llvm::sys::getDefaultTargetTriple();
  TargetInfo *pti = TargetInfo::CreateTargetInfo(compiler.getDiagnostics(), pto);
  compiler.setTarget(pti);

  compiler.createFileManager();
  compiler.createSourceManager(compiler.getFileManager());

  HeaderSearchOptions &headerSearchOptions = compiler.getHeaderSearchOpts();
  std::ifstream includesFile;
  includesFile.open(current_folder + "/myincludes.txt");
  if (includesFile)
  { 
      std::string line;
      while (std::getline(includesFile,line))
      {
          //std::cout << line << std::endl;   
          if (line.rfind("#",0) == 0)
              continue;
          if (line.rfind("./",0) == 0)
          {
              headerSearchOptions.AddPath(current_folder + "/" + line,clang::frontend::Angled, false, false);
#ifdef MYDEBUG
              std::cout << current_folder + "/" + line << std::endl;
#endif
          }
          else 
          {
              headerSearchOptions.AddPath(line,clang::frontend::Angled, false, false);
          }
      }
      includesFile.close();      
  }
  //return 0;
   
 // Allow C++ code to get rewritten
  LangOptions langOpts;
  langOpts.GNUMode = 1; 
  langOpts.CXXExceptions = 1; 
  langOpts.RTTI = 1; 
  langOpts.Bool = 1; 
  langOpts.CPlusPlus = 1;
  
  langOpts.WChar = true;
  langOpts.Blocks = true;
  langOpts.DebuggerSupport = true;
  //langOpts.SpellChecking = false;
  //langOpts.ThreadsafeStatics = false;
  //langOpts.AccessControl = false;
  langOpts.DollarIdents = true;
  langOpts.Exceptions = true;
  
  
  //langOpts.NoBuiltin=false;  
  
 
  compiler.createPreprocessor(clang::TU_Complete);
  compiler.getPreprocessorOpts().UsePredefines = true;
 
  compiler.createASTContext();
  Preprocessor & preprocessor = compiler.getPreprocessor();
  
  //Invocation->setLangDefaults(langOpts, clang::InputKind::CXX,compiler.getTarget().getTriple(),compiler.getPreprocessorOpts(), clang::LangStandard::lang_c11);
#if (LIBCLANG_MAJOR == 3) && (LIBCLANG_MINOR < 9)
    Invocation->setLangDefaults(langOpts, clang::InputKind::CXX,compiler.getTarget().getTriple(),compiler.getPreprocessorOpts(), clang::LangStandard::lang_c11);
#endif
  
  //clang::InputKind ik(Language::CXX, InputKind::Format::Source, false);
  //Invocation->setLangDefaults(langOpts, ik ,compiler.getTarget().getTriple(),compiler.getPreprocessorOpts(), clang::LangStandard::lang_c11);
  
  //Invocation->getLangOpts()->GNUMode = 1; 
  //Invocation->getLangOpts()->CXXExceptions = 1; 
  //Invocation->getLangOpts()->RTTI = 1; 
  //Invocation->getLangOpts()->Bool = 1; 
  //Invocation->getLangOpts()->CPlusPlus = 1;
  
  //Invocation->getLangOpts()->WChar = true;
  //Invocation->getLangOpts()->Blocks = true;
  //Invocation->getLangOpts()->DebuggerSupport = true;
  //Invocation->getLangOpts()->SpellChecking = false;
  //Invocation->getLangOpts()->ThreadsafeStatics = false;
  //Invocation->getLangOpts()->AccessControl = false;
  //Invocation->getLangOpts()->DollarIdents = true;
  //Invocation->getLangOpts()->Exceptions = true;
  
  Invocation->getLangOpts()->BracketDepth = 200000;
  preprocessor.SetSuppressIncludeNotFoundError(false);
  preprocessor.getBuiltinInfo().initializeBuiltins(preprocessor.getIdentifierTable(),preprocessor.getLangOpts());
  
  // Initialize rewriter
  Rewriter Rewrite;
  Rewrite.setSourceMgr(compiler.getSourceManager(),compiler.getLangOpts());

  //const FileEntry *pFile = compiler.getFileManager().getFile(myOptions->inputFile);
  llvm::ErrorOr<const FileEntry *> pFile = compiler.getFileManager().getFile(myOptions->inputFile);
  if(pFile.getError())
  {
      std::cout << "Error in reading:" << myOptions->inputFile <<  std::endl;
      exit(-1);
  }
  mainFileID  = pFile.get()->getUID();
  
  compiler.getSourceManager().setMainFileID(compiler.getSourceManager().createFileID(pFile.get(), clang::SourceLocation(), clang::SrcMgr::C_User));
  compiler.getDiagnosticClient().BeginSourceFile(compiler.getLangOpts(),&compiler.getPreprocessor());
  MyHolder myHolder;
  myHolder.SourceManager = &compiler.getSourceManager();
  myHolder.ASTContext = &compiler.getASTContext();
  //myHolder.LangOpts = &langOpts;
  if(preprocessor.hadModuleLoaderFatalFailure())
  {
      std::cout << "Error in hadModuleLoaderFatalFailure." << std::endl;
      //exit(-1);
  }
  MyASTConsumer astConsumer(Rewrite,myHolder);
  //NonDetConsumer nonDetConsumer(Rewrite,myHolder);

  // Convert <file>.c to <file_out>.c
  /*std::string outName (fileName);
  size_t ext = outName.rfind(".");
  if (ext == std::string::npos)
     ext = outName.length();
  outName.insert(ext, "_out");
   */
  /*if(compiler.getDiagnostics().hasFatalErrorOccurred())
  {
       llvm::errs() << "HASSSS FATAL ERORRRR\n";
      //exit(-1);
  }*/
  std::error_code OutErrorInfo;
  std::error_code ok;
  llvm::raw_fd_ostream outFile(llvm::StringRef(myOptions->outputFile), OutErrorInfo, llvm::sys::fs::F_None);
  

  if (OutErrorInfo == ok)
  {
     // Parse the AST
    clang::ParseAST(compiler.getPreprocessor(), &astConsumer, compiler.getASTContext());
    compiler.getDiagnosticClient().EndSourceFile();

    // Now output rewritten source code
    clang::FileID fileID = compiler.getSourceManager().getMainFileID();
    if(fileID.isInvalid())
    {
        llvm::errs() << "File ID is not valid.\n";
        exit(-1);
    }
    
    if(myOptions->addFuSeBMCFunc)
    {
        outFile << "void FuSeBMC_custom_func(void){}\n";        
    }
	/*if(myOptions->handleReturnInMain)
    {
        outFile << "void fuSeBMC_return(int code);\n";        
    }*/
	if(myOptions->addFuncCallInFunc)
	{
		vector<string> * vect_unique_calls =  myOptions->GetUniqueCalls();
		for(vector<string>::iterator it = vect_unique_calls->begin();
				it != vect_unique_calls->end();
				it++)
		{
			outFile << "void " << *it << "();\n";
		}

		delete vect_unique_calls;
	}
    const RewriteBuffer *RewriteBuf = Rewrite.getRewriteBufferFor(fileID);
    if(RewriteBuf)
    {
        std::cout << "Output to: " << myOptions->outputFile << std::endl;
        std::string theOut = std::string(RewriteBuf->begin(), RewriteBuf->end());
        //ReplaceStringInPlace(theOut,"__VERIFIER_nondet_", "\n\n__VERIFIER_nondet_");
        //llvm::outs() << theOut;
       
        outFile << theOut;
        if(mustPrintGeneratedCode)
            llvm::outs() << theOut;
		isOutFileWritten = true;
    }
    else
    {
        llvm::errs() << "No changes can be made.\n";
        llvm::errs() << "The input file will be copied to the output file.\n";
        // copy the input file to the output
        std::ifstream inputFilefs;
        inputFilefs.open(myOptions->inputFile);
        if (inputFilefs)
        {
            std::string line;
            while (std::getline(inputFilefs,line))
            {
                //ReplaceStringInPlace(line,"__VERIFIER_nondet_", "\n\n__VERIFIER_nondet_");
                outFile << line << "\n";
            }
            inputFilefs.close();
			isOutFileWritten = true;
        }
    }
  }
  else
  {
    llvm::errs() << "Cannot open " << myOptions->outputFile << " for writing\n";
  }

  outFile.close();
 #ifdef MYDEBUG
  //system(("sed -i '/^$/d' "+myOptions->outputFile).c_str());
#endif
  /***********/
  std::cout << "We have: " << goalCounter.counter << " Goals." << std::endl;
  if(!myOptions->goalOutputFile.empty())
  {
    std::cout << "Output Goals to: " << myOptions->goalOutputFile << std::endl;
    llvm::raw_fd_ostream goalOutputFile_fs(llvm::StringRef(myOptions->goalOutputFile), OutErrorInfo, llvm::sys::fs::F_None);
    if (OutErrorInfo == ok)
    {
      goalOutputFile_fs << std::to_string(goalCounter.counter);    
    }
    else
    {
      llvm::errs() << "Cannot open " << myOptions->goalOutputFile << " for writing\n";
    }

    goalOutputFile_fs.close();
  }
  
  
  
  if(GoalCounter::getInstance().mustGenerateFuncLabelMap)
  {
      std::cout << "GoalProFunc Dir: " << myOptions->goalProFuncOutputDir << std::endl;
      for(map<string,vector<string> >::const_iterator ptr= GoalCounter::getInstance().funcLabelMap->begin();
              ptr != GoalCounter::getInstance().funcLabelMap->end();
              ptr++)
      {
          llvm::raw_fd_ostream goalProFunc_fs(llvm::StringRef(myOptions->goalProFuncOutputDir+"/"+ptr->first), OutErrorInfo, llvm::sys::fs::F_None);
          if (OutErrorInfo == ok)
          {
               for(vector<string>::const_iterator goal_name = ptr->second.begin();
                       goal_name != ptr->second.end(); goal_name++)
               {
                   goalProFunc_fs << *goal_name;
                   goalProFunc_fs<< "\n";                  
               }                   
          }
          else
          {
              llvm::errs() << "Cannot open create file for function :  " << ptr->first << " \n";
          }
          goalProFunc_fs.close();      
          
      }
  }
  
  /* NEW CODE BEGIN **/
  bool isOutFileOpenOK = true;
  if(isOutFileWritten &&  myOptions->exportLineNumberForNonDetCalls)
  {
	  
	//pFile.get()->closeFile();
	//compiler.getSourceManager().setFileIsTransient(pFile.get());

	FileManager * nonDetFileManager = compiler.createFileManager();
	compiler.createSourceManager(*nonDetFileManager);
	llvm::ErrorOr<const FileEntry *> pFileDet = nonDetFileManager->getFile(myOptions->outputFile);
	if(pFileDet.getError())
	{
		std::cout << "Cannot open:" << myOptions->outputFile  << std::endl;
		isOutFileOpenOK = false;
		//exit(-1);
	}

	//compiler.getSourceManager().clearIDTables();
	//compiler.getDiagnosticClient().clear();
	compiler.getSourceManager().setMainFileID(compiler.getSourceManager().createFileID(pFileDet.get(), clang::SourceLocation(), clang::SrcMgr::C_User));

	compiler.getDiagnosticClient().BeginSourceFile(compiler.getLangOpts(),&compiler.getPreprocessor());
	Rewriter nonDetRewrite;
	nonDetRewrite.setSourceMgr(compiler.getSourceManager(),compiler.getLangOpts());

	compiler.createPreprocessor(clang::TU_Complete);
	compiler.getPreprocessorOpts().UsePredefines = true;
	compiler.createASTContext();

	MyHolder nonDetHolder;
	nonDetHolder.SourceManager = &compiler.getSourceManager();
	nonDetHolder.ASTContext = &compiler.getASTContext();
	NonDetConsumer nonDetConsumer(nonDetRewrite,nonDetHolder);

	clang::ParseAST(compiler.getPreprocessor(), &nonDetConsumer, compiler.getASTContext());
	compiler.getDiagnosticClient().EndSourceFile();
	
	/*clang::FileID fileID = compiler.getSourceManager().getMainFileID();
    if(fileID.isInvalid())
    {
        llvm::errs() << "File ID is not valid.\n";
        exit(-1);
    }
    
	
	const RewriteBuffer * RewriteBuf = nonDetRewrite.getRewriteBufferFor(fileID);
	llvm::raw_fd_ostream nonDet_outFile(llvm::StringRef(myOptions->outputFile), OutErrorInfo, llvm::sys::fs::F_None);
    if(RewriteBuf)
    {
        //std::cout << "Output to: " << myOptions->outputFile << std::endl;
        std::string theOut = std::string(RewriteBuf->begin(), RewriteBuf->end());
        //ReplaceStringInPlace(theOut,"__VERIFIER_nondet_", "\n\n__VERIFIER_nondet_");
        //llvm::outs() << theOut;
       
        nonDet_outFile << theOut;
    }*/
  }
 
  /* NEW CODE END **/
  
  // Write Info File:
   if(!myOptions->infoFile.empty())
  {
      std::cout << "info File: " << myOptions->infoFile << std::endl;
      llvm::raw_fd_ostream infoFile_fs(llvm::StringRef(myOptions->infoFile), OutErrorInfo, llvm::sys::fs::F_None);
      if (OutErrorInfo == ok)
      { 
          infoFile_fs << "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
          infoFile_fs << "<info>";
          infoFile_fs << "<goalsCount>" << std::to_string(goalCounter.counter) << "</goalsCount>";
          
          infoFile_fs << "<if>";
          for(vector<unsigned long int>::iterator ptr = goalCounter.vctIFGoals->begin();
                  ptr != goalCounter.vctIFGoals->end(); ptr++)
              infoFile_fs << *ptr << ",";          
          infoFile_fs << "</if>";
          
          infoFile_fs << "<loop>";
          for(vector<unsigned long int>::iterator ptr = goalCounter.vectLOOPGoals->begin();
                  ptr != goalCounter.vectLOOPGoals->end(); ptr++)
              infoFile_fs << *ptr << ",";          
          infoFile_fs << "</loop>";
          
          infoFile_fs << "<afterLoop>";
          for(vector<unsigned long int>::iterator ptr = goalCounter.vectAFTER_LOOPGoals->begin();
                  ptr != goalCounter.vectAFTER_LOOPGoals->end(); ptr++)
              infoFile_fs << *ptr << ",";          
          infoFile_fs << "</afterLoop>";
          
          infoFile_fs << "<else>";
          for(vector<unsigned long int>::iterator ptr = goalCounter.vectELSEGoals->begin();
                  ptr != goalCounter.vectELSEGoals->end(); ptr++)
              infoFile_fs << *ptr << ",";          
          infoFile_fs << "</else>";
          
          infoFile_fs << "<emptyElse>";
          for(vector<unsigned long int>::iterator ptr = goalCounter.vectEMPTY_ELSEGoals->begin();
                  ptr != goalCounter.vectEMPTY_ELSEGoals->end(); ptr++)
              infoFile_fs << *ptr << ",";          
          infoFile_fs << "</emptyElse>";
          
          infoFile_fs << "<compound>";
          for(vector<unsigned long int>::iterator ptr = goalCounter.vectCOMPOUNDGoals->begin();
                  ptr != goalCounter.vectCOMPOUNDGoals->end(); ptr++)
              infoFile_fs << *ptr << ",";          
          infoFile_fs << "</compound>";
          
          infoFile_fs << "<endOfFunc>";
          for(vector<unsigned long int>::iterator ptr = goalCounter.vectEND_OF_FUNCTIONGoals->begin();
                  ptr != goalCounter.vectEND_OF_FUNCTIONGoals->end(); ptr++)
              infoFile_fs << *ptr << ",";          
          infoFile_fs << "</endOfFunc>";

		  /******/
		  infoFile_fs << "<For>";
          for(vector<unsigned long int>::iterator ptr = goalCounter.vctForGoals->begin();
                  ptr != goalCounter.vctForGoals->end(); ptr++)
              infoFile_fs << *ptr << ",";          
          infoFile_fs << "</For>";
		  
		  infoFile_fs << "<CXXForRange>";
          for(vector<unsigned long int>::iterator ptr = goalCounter.vctCXXForRangeGoals->begin();
                  ptr != goalCounter.vctCXXForRangeGoals->end(); ptr++)
              infoFile_fs << *ptr << ",";          
          infoFile_fs << "</CXXForRange>";
		  
		  infoFile_fs << "<DoWhile>";
          for(vector<unsigned long int>::iterator ptr = goalCounter.vctDoWhileGoals->begin();
                  ptr != goalCounter.vctDoWhileGoals->end(); ptr++)
              infoFile_fs << *ptr << ",";          
          infoFile_fs << "</DoWhile>";
		  
		  infoFile_fs << "<While>";
          for(vector<unsigned long int>::iterator ptr = goalCounter.vctWhileGoals->begin();
                  ptr != goalCounter.vctWhileGoals->end(); ptr++)
              infoFile_fs << *ptr << ",";          
          infoFile_fs << "</While>";
		  
		  
		  /*****/
		  
		  
		  
		  infoFile_fs << "<nonDetCalls>";
		  if(isOutFileOpenOK)
		  {
			for(vector<NonDetCallInfo*>::iterator ptr = goalCounter.vectLineNumberForNonDetCalls->begin();
					ptr != goalCounter.vectLineNumberForNonDetCalls->end(); ptr++)
			{
				//infoFile_fs << *ptr << ","; 
				NonDetCallInfo * nonDetCallInfo_ptr  = *ptr ;
				infoFile_fs << "<nonDetCall><line>" << nonDetCallInfo_ptr->line << "</line>";
				infoFile_fs << "<funcName>" << nonDetCallInfo_ptr->funcName << "</funcName></nonDetCall>";
			}
		  }
		  else
			  infoFile_fs << "ERROR";
          infoFile_fs << "</nonDetCalls>";
		  
		  infoFile_fs << "<goalInfos>";
		  if(isOutFileOpenOK)
		  {
			for(vector<GoalInfo*>::iterator ptr = goalCounter.vectGoalInfo->begin();
					ptr != goalCounter.vectGoalInfo->end(); ptr++)
			{
				GoalInfo * goalInfo_ptr  = *ptr ;
				infoFile_fs << "<goalInfo><goal>" << goalInfo_ptr->goal << "</goal>";
				infoFile_fs << "<depth>" << goalInfo_ptr->depth << "</depth></goalInfo>";
			}
		  }
		  else
			  infoFile_fs << "ERROR";
          infoFile_fs << "</goalInfos>";
		  
          
          infoFile_fs << "</info>";
      }
      else
      {
          llvm::errs() << "Cannot open " << myOptions->infoFile << " for writing\n";
      }

        infoFile_fs.close();
 
    }
   std::cout << "DONE !!!" << std::endl;
  return 0;
}

