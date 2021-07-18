#include <NonDetConsumer.h>

NonDetConsumer::NonDetConsumer(Rewriter &Rewrite,MyHolder& H) : rv(Rewrite,H),TheHolder(H) 
{
}

bool NonDetConsumer::HandleTopLevelDecl(DeclGroupRef d)
{
	for (DeclGroupRef::iterator b = d.begin(), e = d.end(); b != e; ++b)
	{
		rv.TraverseDecl(*b);
	}

  return true; // keep going
}

NonDetConsumer::~NonDetConsumer()
{
	
}
