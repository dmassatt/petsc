#include "petscvec.h"
#include "petscmat.h"
#include "taosubset.h"

#undef __FUNCT__
#define __FUNCT__ "TaoCreateSubset"
PetscErrorCode TaoCreateSubset(TaoSolver tao, PetscInt subsettype, IndexSet *is) 
{
    PetscErrorCode ierr;

    PetscFunctionBegin;
    PetscValidHeaderSpecific(tao,TAOSOLVER_CLASSID,1);
    
    
    PetscFunctionReturn(0);
}

