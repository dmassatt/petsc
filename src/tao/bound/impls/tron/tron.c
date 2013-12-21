#include "tron.h"
#include "petsc-private/kspimpl.h"
#include "petsc-private/matimpl.h"
#include "../src/tao/matrix/submatfree.h"


/* TRON Routines */
static PetscErrorCode TronGradientProjections(TaoSolver,TAO_TRON*);
/*------------------------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "TaoDestroy_TRON"
static PetscErrorCode TaoDestroy_TRON(TaoSolver tao)
{
  TAO_TRON *tron = (TAO_TRON *)tao->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;

  ierr = VecDestroy(&tron->X_New);CHKERRQ(ierr);
  ierr = VecDestroy(&tron->G_New);CHKERRQ(ierr);
  ierr = VecDestroy(&tron->Work);CHKERRQ(ierr);
  ierr = VecDestroy(&tron->DXFree);CHKERRQ(ierr);
  ierr = VecDestroy(&tron->R); CHKERRQ(ierr);
  ierr = VecDestroy(&tron->diag); CHKERRQ(ierr);
  ierr = VecScatterDestroy(&tron->scatter); CHKERRQ(ierr);
  ierr = ISDestroy(&tron->Free_Local); CHKERRQ(ierr);
  ierr = MatDestroy(&tron->H_sub); CHKERRQ(ierr);
  ierr = MatDestroy(&tron->Hpre_sub); CHKERRQ(ierr);
  ierr = PetscFree(tao->data);
  tao->data = PETSC_NULL;

  PetscFunctionReturn(0);
}

/*------------------------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "TaoSetFromOptions_TRON"
static PetscErrorCode TaoSetFromOptions_TRON(TaoSolver tao)
{
  TAO_TRON  *tron = (TAO_TRON *)tao->data;
  PetscErrorCode        ierr;
  PetscBool flg;

  PetscFunctionBegin;

  ierr = PetscOptionsHead("Newton Trust Region Method for bound constrained optimization");CHKERRQ(ierr);
  
  ierr = PetscOptionsInt("-tron_maxgpits","maximum number of gradient projections per TRON iterate","TaoSetMaxGPIts",tron->maxgpits,&tron->maxgpits,&flg);
  CHKERRQ(ierr);

  ierr = PetscOptionsTail();CHKERRQ(ierr);
  ierr = TaoLineSearchSetFromOptions(tao->linesearch);CHKERRQ(ierr);
  ierr = KSPSetFromOptions(tao->ksp); CHKERRQ(ierr);

  PetscFunctionReturn(0);
}

/*------------------------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "TaoView_TRON"
static PetscErrorCode TaoView_TRON(TaoSolver tao, PetscViewer viewer)
{
  TAO_TRON  *tron = (TAO_TRON *)tao->data;
  PetscBool isascii;
  PetscErrorCode   ierr;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)viewer,PETSCVIEWERASCII,&isascii);CHKERRQ(ierr);
  if (isascii) {
    ierr = PetscViewerASCIIPushTab(viewer); CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(viewer,"Total PG its: %D,",tron->total_gp_its);CHKERRQ(ierr);
    ierr = PetscViewerASCIIPrintf(viewer,"PG tolerance: %G \n",tron->pg_ftol);CHKERRQ(ierr);
    ierr = PetscViewerASCIIPopTab(viewer); CHKERRQ(ierr);
  } else {
    SETERRQ1(((PetscObject)tao)->comm,PETSC_ERR_SUP,"Viewer type %s not supported for TAO TRON",((PetscObject)viewer)->type_name);
  }    
  PetscFunctionReturn(0);
}


/* ---------------------------------------------------------- */
#undef __FUNCT__  
#define __FUNCT__ "TaoSetup_TRON"
static PetscErrorCode TaoSetup_TRON(TaoSolver tao)
{
  PetscErrorCode ierr;
  TAO_TRON *tron = (TAO_TRON *)tao->data;

  PetscFunctionBegin;

  /* Allocate some arrays */
  ierr = VecDuplicate(tao->solution, &tron->diag); CHKERRQ(ierr);
  ierr = VecDuplicate(tao->solution, &tron->X_New); CHKERRQ(ierr);
  ierr = VecDuplicate(tao->solution, &tron->G_New); CHKERRQ(ierr);
  ierr = VecDuplicate(tao->solution, &tron->Work); CHKERRQ(ierr);
  ierr = VecDuplicate(tao->solution, &tao->gradient); CHKERRQ(ierr);
  ierr = VecDuplicate(tao->solution, &tao->stepdirection); CHKERRQ(ierr);
  if (!tao->XL) {
      ierr = VecDuplicate(tao->solution, &tao->XL); CHKERRQ(ierr);
      ierr = VecSet(tao->XL, TAO_NINFINITY); CHKERRQ(ierr);
  }
  if (!tao->XU) {
      ierr = VecDuplicate(tao->solution, &tao->XU); CHKERRQ(ierr);
      ierr = VecSet(tao->XU, TAO_INFINITY); CHKERRQ(ierr);
  }

  PetscFunctionReturn(0);
}



#undef __FUNCT__  
#define __FUNCT__ "TaoSolve_TRON"
static PetscErrorCode TaoSolve_TRON(TaoSolver tao){

  TAO_TRON *tron = (TAO_TRON *)tao->data;
  PetscErrorCode ierr;
  PetscInt iter=0,its;

  TaoSolverTerminationReason reason = TAO_CONTINUE_ITERATING;
  TaoLineSearchTerminationReason ls_reason = TAOLINESEARCH_CONTINUE_ITERATING;
  PetscReal prered,actred,delta,f,f_new,rhok,gdx,xdiff,stepsize;
  PetscFunctionBegin;

  tron->pgstepsize=1.0;

  tao->trust = tao->trust0;
  /*   Project the current point onto the feasible set */
  ierr = TaoComputeVariableBounds(tao); CHKERRQ(ierr);
  ierr = VecMedian(tao->XL,tao->solution,tao->XU,tao->solution); CHKERRQ(ierr);
  ierr = TaoLineSearchSetVariableBounds(tao->linesearch,tao->XL,tao->XU); CHKERRQ(ierr);

  
  ierr = TaoComputeObjectiveAndGradient(tao,tao->solution,&tron->f,tao->gradient);CHKERRQ(ierr);
  if (tron->Free_Local) {
    ierr = ISDestroy(&tron->Free_Local); CHKERRQ(ierr);
    tron->Free_Local = PETSC_NULL;
  }
  ierr = VecWhichBetween(tao->XL,tao->solution,tao->XU,&tron->Free_Local); CHKERRQ(ierr);
  
  /* Project the gradient and calculate the norm */
  ierr = VecBoundGradientProjection(tao->gradient,tao->solution, tao->XL, tao->XU, tao->gradient); CHKERRQ(ierr);
  ierr = VecNorm(tao->gradient,NORM_2,&tron->gnorm); CHKERRQ(ierr);

  if (PetscIsInfOrNanReal(tron->f) || PetscIsInfOrNanReal(tron->gnorm)) {
    SETERRQ(PETSC_COMM_SELF,1, "User provided compute function generated Inf pr NaN");
  }

  if (tao->trust <= 0) {
    tao->trust=PetscMax(tron->gnorm*tron->gnorm,1.0);
  }

  tron->stepsize=tao->trust;
  ierr = TaoMonitor(tao, iter, tron->f, tron->gnorm, 0.0, tron->stepsize, &reason); CHKERRQ(ierr);
  while (reason==TAO_CONTINUE_ITERATING){

    ierr = TronGradientProjections(tao,tron); CHKERRQ(ierr);
    f=tron->f; delta=tao->trust; 
    tron->n_free_last = tron->n_free;
    ierr = TaoComputeHessian(tao,tao->solution,&tao->hessian, &tao->hessian_pre, &tron->matflag);CHKERRQ(ierr);

    ierr = ISGetSize(tron->Free_Local, &tron->n_free);  CHKERRQ(ierr);

    /* If no free variables */
    if (tron->n_free == 0) {
      actred=0;
      PetscInfo(tao,"No free variables in tron iteration.");
      break;

    }
    /* use free_local to mask/submat gradient, hessian, stepdirection */
    ierr = VecGetSubVec(tao->gradient,tron->Free_Local,tao->subset_type,0.0,&tron->R);
    CHKERRQ(ierr);
    ierr = VecGetSubVec(tao->gradient,tron->Free_Local,tao->subset_type,0.0,&tron->DXFree);
    CHKERRQ(ierr);
    ierr = VecSet(tron->DXFree,0.0); CHKERRQ(ierr);
    ierr = VecScale(tron->R, -1.0); CHKERRQ(ierr);
    ierr = MatGetSubMat(tao->hessian, tron->Free_Local, tron->diag, tao->subset_type, &tron->H_sub); CHKERRQ(ierr);
    if (tao->hessian == tao->hessian_pre) {
      ierr = MatDestroy(&tron->Hpre_sub); CHKERRQ(ierr);
      ierr = PetscObjectReference((PetscObject)(tron->H_sub)); CHKERRQ(ierr);
      tron->Hpre_sub = tron->H_sub;
    } else {
      ierr = MatGetSubMat(tao->hessian_pre, tron->Free_Local, tron->diag, tao->subset_type,&tron->Hpre_sub);
      CHKERRQ(ierr);
    }
    ierr = KSPReset(tao->ksp); CHKERRQ(ierr);
    ierr = KSPSetOperators(tao->ksp, tron->H_sub, tron->Hpre_sub, tron->matflag); CHKERRQ(ierr);
    while (1) {

      /* Approximately solve the reduced linear system */
      ierr = KSPSTCGSetRadius(tao->ksp,delta); CHKERRQ(ierr);
      ierr = KSPSolve(tao->ksp, tron->R, tron->DXFree); CHKERRQ(ierr);
      ierr = KSPGetIterationNumber(tao->ksp,&its); CHKERRQ(ierr);
      tao->ksp_its+=its;
      ierr = VecSet(tao->stepdirection,0.0); CHKERRQ(ierr);
      
      /* Add dxfree matrix to compute step direction vector */
      ierr = VecReducedXPY(tao->stepdirection,tron->DXFree,tron->Free_Local); CHKERRQ(ierr);
      if (0) { 
	PetscReal rhs,stepnorm;
	ierr = VecNorm(tron->R,NORM_2,&rhs); CHKERRQ(ierr);
	ierr = VecNorm(tron->DXFree,NORM_2,&stepnorm); CHKERRQ(ierr);
	ierr = PetscPrintf(PETSC_COMM_WORLD,"|rhs|=%g\t|s|=%g\n",rhs,stepnorm); CHKERRQ(ierr);
      }
      
      
      ierr = VecDot(tao->gradient, tao->stepdirection, &gdx); CHKERRQ(ierr);
      ierr = PetscInfo1(tao,"Expected decrease in function value: %14.12e\n",gdx); 
      CHKERRQ(ierr);
      
      ierr = VecCopy(tao->solution, tron->X_New); CHKERRQ(ierr);
      ierr = VecCopy(tao->gradient, tron->G_New); CHKERRQ(ierr);
      
      stepsize=1.0;f_new=f;

      ierr = TaoLineSearchSetInitialStepLength(tao->linesearch,1.0); CHKERRQ(ierr);
      ierr = TaoLineSearchApply(tao->linesearch, tron->X_New, &f_new, tron->G_New, tao->stepdirection,
				&stepsize,&ls_reason); CHKERRQ(ierr); CHKERRQ(ierr);
      ierr = TaoAddLineSearchCounts(tao); CHKERRQ(ierr);
      
      ierr = MatMult(tao->hessian, tao->stepdirection, tron->Work); CHKERRQ(ierr);
      ierr = VecAYPX(tron->Work, 0.5, tao->gradient); CHKERRQ(ierr);
      ierr = VecDot(tao->stepdirection, tron->Work, &prered); CHKERRQ(ierr);
      actred = f_new - f;
      if (actred<0) {
	rhok=PetscAbs(-actred/prered);
      } else {
	rhok=0.0;
      }
      
      /* Compare actual improvement to the quadratic model */
      if (rhok > tron->eta1) { /* Accept the point */
	/* d = x_new - x */
	ierr = VecCopy(tron->X_New, tao->stepdirection); CHKERRQ(ierr);
	ierr = VecAXPY(tao->stepdirection, -1.0, tao->solution); CHKERRQ(ierr);
	
	ierr = VecNorm(tao->stepdirection, NORM_2, &xdiff); CHKERRQ(ierr);
	xdiff *= stepsize;

	/* Adjust trust region size */
	if (rhok < tron->eta2 ){
	  delta = PetscMin(xdiff,delta)*tron->sigma1;
	} else if (rhok > tron->eta4 ){
	  delta= PetscMin(xdiff,delta)*tron->sigma3;
	} else if (rhok > tron->eta3 ){
	  delta=PetscMin(xdiff,delta)*tron->sigma2;
	}
	ierr = VecBoundGradientProjection(tron->G_New,tron->X_New, tao->XL, tao->XU, tao->gradient); CHKERRQ(ierr);
	if (tron->Free_Local) {
	  ierr = ISDestroy(&tron->Free_Local); CHKERRQ(ierr);
	  tron->Free_Local=PETSC_NULL;
	}
	ierr = VecWhichBetween(tao->XL, tron->X_New, tao->XU, &tron->Free_Local); CHKERRQ(ierr);
	f=f_new;
	ierr = VecNorm(tao->gradient,NORM_2,&tron->gnorm); CHKERRQ(ierr);
	ierr = VecCopy(tron->X_New, tao->solution); CHKERRQ(ierr);
	ierr = VecCopy(tron->G_New, tao->gradient); CHKERRQ(ierr);
	break;
      } 
      else if (delta <= 1e-30) {
	break;
      }
      else {
	delta /= 4.0;
      }
    } /* end linear solve loop */


    tron->f=f; tron->actred=actred; tao->trust=delta;
    iter++;
    ierr = TaoMonitor(tao, iter, tron->f, tron->gnorm, 0.0, delta, &reason); CHKERRQ(ierr);
  }  /* END MAIN LOOP  */

  PetscFunctionReturn(0);
}


#undef __FUNCT__  
#define __FUNCT__ "TronGradientProjections"
static PetscErrorCode TronGradientProjections(TaoSolver tao,TAO_TRON *tron)
{
  PetscErrorCode ierr;
  PetscInt i;
  TaoLineSearchTerminationReason ls_reason;
  PetscReal actred=-1.0,actred_max=0.0;
  PetscReal f_new;
  /*
     The gradient and function value passed into and out of this
     routine should be current and correct.
     
     The free, active, and binding variables should be already identified
  */
  
  PetscFunctionBegin;
  if (tron->Free_Local) {
    ierr = ISDestroy(&tron->Free_Local); CHKERRQ(ierr);
    tron->Free_Local = PETSC_NULL;
  }
  ierr = VecWhichBetween(tao->XL,tao->solution,tao->XU,&tron->Free_Local); CHKERRQ(ierr);

  for (i=0;i<tron->maxgpits;i++){

    if ( -actred <= (tron->pg_ftol)*actred_max) break;
  
    tron->gp_iterates++; tron->total_gp_its++;      
    f_new=tron->f;

    ierr = VecCopy(tao->gradient, tao->stepdirection); CHKERRQ(ierr);
    ierr = VecScale(tao->stepdirection, -1.0); CHKERRQ(ierr);
    ierr = TaoLineSearchSetInitialStepLength(tao->linesearch,tron->pgstepsize); CHKERRQ(ierr);
    ierr = TaoLineSearchApply(tao->linesearch, tao->solution, &f_new, tao->gradient, tao->stepdirection,
			      &tron->pgstepsize, &ls_reason); CHKERRQ(ierr);
    ierr = TaoAddLineSearchCounts(tao); CHKERRQ(ierr);


    /* Update the iterate */
    actred = f_new - tron->f;
    actred_max = PetscMax(actred_max,-(f_new - tron->f));
    tron->f = f_new;
    if (tron->Free_Local) {
      ierr = ISDestroy(&tron->Free_Local); CHKERRQ(ierr);
      tron->Free_Local = PETSC_NULL;
    }
    ierr = VecWhichBetween(tao->XL,tao->solution,tao->XU,&tron->Free_Local); CHKERRQ(ierr);
  }
  
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "TaoComputeDual_TRON" 
static PetscErrorCode TaoComputeDual_TRON(TaoSolver tao, Vec DXL, Vec DXU) {

  TAO_TRON *tron = (TAO_TRON *)tao->data;
  PetscErrorCode       ierr;

  PetscFunctionBegin;

  PetscValidHeaderSpecific(tao,TAOSOLVER_CLASSID,1);
  PetscValidHeaderSpecific(DXL,VEC_CLASSID,2);
  PetscValidHeaderSpecific(DXU,VEC_CLASSID,3);

  if (!tron->Work || !tao->gradient) {
      SETERRQ(PETSC_COMM_SELF,PETSC_ERR_ORDER,"Dual variables don't exist yet or no longer exist.\n");
  }

  ierr = VecBoundGradientProjection(tao->gradient,tao->solution,tao->XL,tao->XU,tron->Work); CHKERRQ(ierr);
  ierr = VecCopy(tron->Work,DXL); CHKERRQ(ierr);
  ierr = VecAXPY(DXL,-1.0,tao->gradient); CHKERRQ(ierr);
  ierr = VecSet(DXU,0.0); CHKERRQ(ierr);
  ierr = VecPointwiseMax(DXL,DXL,DXU); CHKERRQ(ierr);

  ierr = VecCopy(tao->gradient,DXU); CHKERRQ(ierr);
  ierr = VecAXPY(DXU,-1.0,tron->Work); CHKERRQ(ierr);
  ierr = VecSet(tron->Work,0.0); CHKERRQ(ierr);
  ierr = VecPointwiseMin(DXU,tron->Work,DXU); CHKERRQ(ierr);

  PetscFunctionReturn(0);
}

/*------------------------------------------------------------*/
EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "TaoCreate_TRON"
PetscErrorCode TaoCreate_TRON(TaoSolver tao)
{
  TAO_TRON *tron;
  PetscErrorCode   ierr;
  const char *morethuente_type = TAOLINESEARCH_MT;
  PetscFunctionBegin;

  tao->ops->setup = TaoSetup_TRON;
  tao->ops->solve = TaoSolve_TRON;
  tao->ops->view = TaoView_TRON;
  tao->ops->setfromoptions = TaoSetFromOptions_TRON;
  tao->ops->destroy = TaoDestroy_TRON;
  tao->ops->computedual = TaoComputeDual_TRON;

  ierr = PetscNewLog(tao,&tron); CHKERRQ(ierr);

  tao->max_it = 50;
  tao->fatol = 1e-10;
  tao->frtol = 1e-10;
  tao->data = (void*)tron;
  tao->steptol = 1e-12;
  tao->trust0       = 1.0;

  /* Initialize pointers and variables */
  tron->n            = 0;
  tron->maxgpits     = 3;
  tron->pg_ftol      = 0.001;

  tron->eta1         = 1.0e-4;
  tron->eta2         = 0.25;
  tron->eta3         = 0.50;
  tron->eta4         = 0.90;

  tron->sigma1       = 0.5;
  tron->sigma2       = 2.0;
  tron->sigma3       = 4.0;

  tron->gp_iterates  = 0; /* Cumulative number */
  tron->total_gp_its = 0;
 
  tron->n_free       = 0;

  tron->DXFree=PETSC_NULL;
  tron->R=PETSC_NULL;
  tron->X_New=PETSC_NULL;
  tron->G_New=PETSC_NULL;
  tron->Work=PETSC_NULL;
  tron->Free_Local=PETSC_NULL;
  tron->H_sub=PETSC_NULL;
  tron->Hpre_sub=PETSC_NULL;
  tao->subset_type = TAO_SUBSET_SUBVEC;

  ierr = TaoLineSearchCreate(((PetscObject)tao)->comm, &tao->linesearch); CHKERRQ(ierr);
  ierr = TaoLineSearchSetType(tao->linesearch,morethuente_type); CHKERRQ(ierr);
  ierr = TaoLineSearchUseTaoSolverRoutines(tao->linesearch,tao); CHKERRQ(ierr);

  ierr = KSPCreate(((PetscObject)tao)->comm, &tao->ksp); CHKERRQ(ierr);
  ierr = KSPSetType(tao->ksp,KSPSTCG); CHKERRQ(ierr);

  PetscFunctionReturn(0);
}
EXTERN_C_END
