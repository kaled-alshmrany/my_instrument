#include <MyASTConsumer.h>

MyASTConsumer::MyASTConsumer(Rewriter &Rewrite,MyHolder& H) : rv(Rewrite,H),TheHolder(H)
{
	
}

bool MyASTConsumer::HandleTopLevelDecl(DeclGroupRef d)
{
  
  for (DeclGroupRef::iterator b = d.begin(), e = d.end(); b != e; ++b)
  {
    rv.TraverseDecl(*b);
  }

  return true; // keep going
}

MyASTConsumer::~MyASTConsumer()
{ 
};
