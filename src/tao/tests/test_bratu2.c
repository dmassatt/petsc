/* ------------------------------------------------------------------------

    Solid Fuel Ignition (SFI) problem.  This problem is modeled by
    the partial differential equation
  
            -Laplacian u - lambda*exp(u) = 0,  0 < x,y < 1 ,
  
    with boundary conditions
   
             u = 0  for  x = 0, x = 1, y = 0, y = 1.
  
    A finite difference approximation with the usual 5-point stencil
    is used to discretize the boundary value problem to obtain a nonlinear 
    system of equations.


  ------------------------------------------------------------------------- */

/* 
   Include "petscda.h" so that we can use distributed arrays (DAs).
   Include "tao.h" so that we can use TAO solvers.  Note that this
   file automatically includes libraries such as:
     petsc.h       - base PETSc routines   petscvec.h - vectors
     petscsys.h    - sysem routines        petscmat.h - matrices
     petscis.h     - index sets            petscksp.h - Krylov subspace methods
     petscviewer.h - viewers               petscpc.h  - preconditioners

*/

#include "taosolver.h"
#include "petscda.h"

static char help[] = 
"Solves a nonlinear system in parallel with OT.\n\
We solve the  Bratu (SFI - solid fuel ignition) problem in a 2D rectangular\n\
domain, using distributed arrays (DAs) to partition the parallel grid.\n\
The command line options include:\n\
  -par <parameter>, where <parameter> indicates the problem's nonlinearity\n\
     problem SFI:  <parameter> = Bratu parameter (0 <= par <= 6.81)\n\
  -mx <xg>, where <xg> = number of grid points in the x-direction\n\
  -my <yg>, where <yg> = number of grid points in the y-direction.\n\n";




/* 
   User-defined application context - contains data needed by the 
   application-provided call-back routines, FormJacobian() and
   FormFunction().
*/
typedef struct {
  /* problem parameters */
  PetscReal   param;          /* test problem parameter */
  PetscInt    mx,my;          /* discretization in x, y directions */

  /* working space */
  Vec         localX, localF; /* ghosted local vector */
  DA          da;             /* distributed array data structure */
} AppCtx;

/* 
   User-defined routines
*/
PetscErrorCode FormInitialGuess(AppCtx*,Vec);
PetscErrorCode FormFunction(TaoSolver,Vec,Vec,void*);
PetscErrorCode FormJacobian(TaoSolver,Vec,Mat*,Mat*,MatStructure*,void*);

#undef __FUNCT__
#define __FUNCT__ "main"
int main( int argc, char **argv )
{
  PetscErrorCode      ierr;                 /* used to check for functions returning nonzeros */
  PetscInt      Nx, Ny;              /* number of preocessors in x- and y- directions */
  PetscInt      m, N;                /* number of local and global elements in vectors */
  Vec      x,f;                 /* solution, residual vectors */
  Mat      J;                   /* Jacobian matrix */
  PetscBool flg;               /* flag - 1 indicates matrix-free version */
  TaoSolver tao;               /* TaoSolver solver context */
  ISLocalToGlobalMapping isltog;
  PetscInt      nloc, *ltog;         /* indexing variables */
  PetscReal   bratu_lambda_max = 6.81, bratu_lambda_min = 0.; /* parameter bound */
  AppCtx   user;                /* user-defined work context */

  /* Initialize PETSc and TAO */
  PetscInitialize( &argc, &argv,(char *)0,help );
  TaoInitialize( &argc, &argv,(char *)0,help );

  /* Initialize problem parameters  */
  user.mx = 4; user.my = 4; user.param = 6.0;

  /* check for any command line arguments that override defaults */
  ierr = PetscOptionsGetInt(NULL,"-mx",&user.mx,&flg); CHKERRQ(ierr);
  ierr = PetscOptionsGetInt(NULL,"-my",&user.my,&flg); CHKERRQ(ierr);
  ierr = PetscOptionsGetReal(NULL,"-par",&user.param,&flg); CHKERRQ(ierr);
  if (user.param >= bratu_lambda_max || user.param <= bratu_lambda_min) {
    SETERRQ(PETSC_COMM_SELF,1,"Lambda is out of range");
  }

  /* Calculate and derived values from parameters */
  N = user.mx*user.my;


  /* Let PETSc determine the grid division among processes */
  Nx = Ny = m = PETSC_DECIDE;


  /* Create distributed array (DA) to manage parallel grid and vectors  */
  ierr = DACreate2d(PETSC_COMM_WORLD,DA_NONPERIODIC,DA_STENCIL_STAR,user.mx,
                    user.my,Nx,Ny,1,1,NULL,NULL,&user.da); CHKERRQ(ierr);

  /*
     Extract global and local vectors from DA; then duplicate for remaining
     vectors that are the same types
  */
  ierr = DACreateGlobalVector(user.da,&x); CHKERRQ(ierr);
  ierr = DACreateLocalVector(user.da,&user.localX); CHKERRQ(ierr);
  ierr = VecDuplicate(x,&f); CHKERRQ(ierr);
  ierr = VecDuplicate(user.localX,&user.localF); CHKERRQ(ierr);

  ierr = VecGetLocalSize(x,&m); CHKERRQ(ierr);
  ierr = MatCreateAIJ(PETSC_COMM_WORLD,m,m,N,N,5,NULL,3,NULL,&J); CHKERRQ(ierr);

  /*
    Get the global node numbers for all local nodes, including ghost points.
    Associate this mapping with the matrix for later use in setting matrix
    entries via MatSetValuesLocal().
  */
  ierr = DAGetGlobalIndices(user.da,&nloc,&ltog); CHKERRQ(ierr);
  ierr = ISLocalToGlobalMappingCreate(PETSC_COMM_SELF,nloc,ltog,&isltog); CHKERRQ(ierr);
  ierr = MatSetLocalToGlobalMapping(J,isltog); CHKERRQ(ierr);
  ierr = ISLocalToGlobalMappingDestroy(isltog); CHKERRQ(ierr);

  /* The Tao code begins here */

  /* Create the optimization solver, Petsc application   */
  ierr = TaoCreate(PETSC_COMM_WORLD,&tao);CHKERRQ(ierr);
  ierr = TaoSetType(tao,"tao_pounders"); CHKERRQ(ierr);

  /* Set the initial vector */
  ierr = FormInitialGuess(&user,x); CHKERRQ(ierr);
  ierr = TaoSetInitialVector(tao,x); CHKERRQ(ierr);

  /* Set the user function, constraints, jacobian evaluation routines */
  ierr = TaoSetSeparableObjectiveRoutine(tao,f,FormFunction,(void*)&user); CHKERRQ(ierr);
  ierr = TaoSetJacobianRoutine(tao,J,J,FormJacobian,(void*)&user); CHKERRQ(ierr);


  /* Check for any TAO command line options */ 
  ierr = TaoSetFromOptions(tao); CHKERRQ(ierr);

  /* SOLVE THE LEAST-SQUARES APPLICATION */
  ierr = TaoSolve(tao); CHKERRQ(ierr);

  /*
    To view TAO solver information,
     ierr = TaoView(tao); CHKERRQ(ierr);
  */

  /* Free TAO data structures */
  ierr = TaoDestroy(&tao); CHKERRQ(ierr);  

  /* Free PETSc data structures */
  ierr = VecDestroy(&x); CHKERRQ(ierr);
  ierr = VecDestroy(&f); CHKERRQ(ierr);      
  ierr = VecDestroy(&user.localX); CHKERRQ(ierr); 
  ierr = VecDestroy(&user.localF); CHKERRQ(ierr); 
  ierr = MatDestroy(&J); CHKERRQ(ierr);
  ierr = DADestroy(user.da); CHKERRQ(ierr);


  /* Finalize TAO and PETSc */
  TaoFinalize();
  PetscFinalize();

  return 0;
}

#undef __FUNCT__
#define __FUNCT__ "FormInitialGuess"
/* ------------------------------------------------------------------- */
/* 
   FormInitialGuess - Forms initial approximation.

   Input Parameters:
   user - user-defined application context
   X - vector

   Output Parameter:
   X - vector
 */
PetscErrorCode FormInitialGuess(AppCtx *user,Vec X)
{
  PetscErrorCode ierr;
  PetscInt    i, j, row, mx, my, xs, ys, xm, ym, gxm, gym, gxs, gys;
  PetscReal  lambda, temp1, temp, hx, hy;
  PetscReal  *x;

  mx = user->mx;            my = user->my;            lambda = user->param;
  hx = 1.0/(mx-1);  hy = 1.0/(my-1);
  temp1 = lambda/(lambda + 1.0);

  /*
     Get a pointer to vector data.
       - For default PETSc vectors, VecGetArray() returns a pointer to
         the data array.  Otherwise, the routine is implementation dependent.
       - You MUST call VecRestoreArray() when you no longer need access to
         the array.
  */

  ierr = VecGetArray(user->localX,&x); CHKERRQ(ierr);
  /* 
     Since we don't need the data from ghost points, we do not need
     to call DAGlobalToLocal functions 
  */


  /*
     Get local grid boundaries (for 2-dimensional DA):
       xs, ys   - starting grid indices (no ghost points)
       xm, ym   - widths of local grid (no ghost points)
       gxs, gys - starting grid indices (including ghost points)
       gxm, gym - widths of local grid (including ghost points)
  */
  ierr = DAGetCorners(user->da,&xs,&ys,NULL,&xm,&ym,NULL); CHKERRQ(ierr);
  ierr = DAGetGhostCorners(user->da,&gxs,&gys,NULL,&gxm,&gym,NULL); CHKERRQ(ierr);

  /*
     Compute initial guess over the locally owned part of the grid
  */
  for (j=ys; j<ys+ym; j++) {
    temp = PetscMin(j,my-j-1)*hy;
    for (i=xs; i<xs+xm; i++) {
      row = i - gxs + (j - gys)*gxm; 
      
      if (i == 0 || j == 0 || i == mx-1 || j == my-1 ) {
        x[row] = 0.0; 
        continue;
      }
      x[row] = temp1*PetscSqrtScalar( PetscMin(PetscMin(i,mx-i-1)*hx,temp) ); 
    }
  }

  /*
     Restore vector
  */
  ierr = VecRestoreArray(user->localX,&x); CHKERRQ(ierr);

  /*
     Insert values into global vector
  */
  
  ierr = DALocalToGlobal(user->da,user->localX,INSERT_VALUES,X); CHKERRQ(ierr);
  return 0;
} 

#undef __FUNCT__
#define __FUNCT__ "FormFunction"
/* ------------------------------------------------------------------- */
/* 
   FormFunction - Evaluates nonlinear function, F(x).

   Input Parameters:
.  tao - the OT context
.  X - input vector
.  ptr - optional user-defined context, as set by OTSetFunction()

   Output Parameter:
.  F - function vector
 */
PetscErrorCode FormFunction(TaoSolver tao,Vec X,Vec F,void *ptr)
{
  AppCtx  *user = (AppCtx *) ptr;
  PetscErrorCode     ierr;
  PetscInt i, j, row, mx, my, xs, ys, xm, ym, gxs, gys, gxm, gym;
  PetscReal  two = 2.0, lambda,hx, hy, hxdhy, hydhx,sc;
  PetscReal  u, uxx, uyy, *x,*f;
  Vec localX=user->localX,localF=user->localF;

  mx = user->mx;            my = user->my;            lambda = user->param;
  hx = 1.0/(mx-1);  hy = 1.0/(my-1);
  sc = hx*hy*lambda;        hxdhy = hx/hy;            hydhx = hy/hx;

  /*
     Scatter ghost points to local vector, using the 2-step process
        DAGlobalToLocalBegin(), DAGlobalToLocalEnd().
     By placing code between these two statements, computations can be
     done while messages are in transition.
  */

  ierr = DAGlobalToLocalBegin(user->da,X,INSERT_VALUES,localX); CHKERRQ(ierr);
  ierr = DAGlobalToLocalEnd(user->da,X,INSERT_VALUES,localX); CHKERRQ(ierr);

  /*
     Get pointers to vector data
  */
  ierr = VecGetArray(localX,&x); CHKERRQ(ierr);
  ierr = VecGetArray(localF,&f); CHKERRQ(ierr);

  /*
     Get local grid boundaries
  */
  ierr = DAGetCorners(user->da,&xs,&ys,NULL,&xm,&ym,NULL); CHKERRQ(ierr);
  ierr = DAGetGhostCorners(user->da,&gxs,&gys,NULL,&gxm,&gym,NULL); CHKERRQ(ierr);

  /*
     Compute function over the locally owned part of the grid
  */
  for (j=ys; j<ys+ym; j++) {
    row = (j - gys)*gxm + xs - gxs - 1; 
    for (i=xs; i<xs+xm; i++) {
      row++;
      if (i == 0 || j == 0 || i == mx-1 || j == my-1 ) {
        f[row] = x[row];
        continue;
      }
      u = x[row];
      uxx = (two*u - x[row-1] - x[row+1])*hydhx;
      uyy = (two*u - x[row-gxm] - x[row+gxm])*hxdhy;
      f[row] = uxx + uyy - sc*PetscExpScalar(u);
    }
  }

  /*
     Restore vectors
  */
  ierr = VecRestoreArray(localX,&x); CHKERRQ(ierr);
  ierr = VecRestoreArray(localF,&f); CHKERRQ(ierr);

  /*
     Insert values into global vector
  */
  ierr = DALocalToGlobal(user->da,localF,INSERT_VALUES,F); CHKERRQ(ierr);
  PetscLogFlops(11*ym*xm);
  return 0; 
} 


#undef __FUNCT__
#define __FUNCT__ "FormJacobian"
/* ------------------------------------------------------------------- */
/*
   FormJacobian - Evaluates Jacobian matrix.

   Input Parameters:
.  tao - the OT context
.  x - input vector
.  ptr - optional user-defined context, as set by OTSetJacobian()

   Output Parameters:
.  A - Jacobian matrix
.  B - optionally different preconditioning matrix
.  flag - flag indicating matrix structure

   Notes:
   Due to grid point reordering with DAs, we must always work
   with the local grid points, and then transform them to the new
   global numbering with the "ltog" mapping (via DAGetGlobalIndices()).
   We cannot work directly with the global numbers for the original
   uniprocessor grid!  

   Two methods are available for imposing this transformation
   when setting matrix entries:
     (A) MatSetValuesLocal(), using the local ordering (including
         ghost points!)
         - Use DAGetGlobalIndices() to extract the local-to-global map
         - Associate this map with the matrix by calling
           MatSetLocalToGlobalMapping() once
         - Set matrix entries using the local ordering
           by calling MatSetValuesLocal()
     (B) MatSetValues(), using the global ordering 
         - Use DAGetGlobalIndices() to extract the local-to-global map
         - Then apply this map explicitly yourself
         - Set matrix entries using the global ordering by calling
           MatSetValues()
   Option (A) seems cleaner/easier in many cases, and is the procedure
   used in this example.
*/
PetscErrorCode FormJacobian(TaoSolver tao,Vec X,Mat *JJ,Mat *Jpre,MatStructure *structure, void *ptr)
{
  AppCtx  *user = (AppCtx *) ptr;  /* user-defined application context */
  Mat     jac=*JJ;
  Vec     localX=user->localX; /* local vector */
  PetscErrorCode     ierr;
  PetscInt i, j, row, mx, my, col[5];
  PetscInt     xs, ys, xm, ym, gxs, gys, gxm, gym;
  PetscReal  two = 2.0, one = 1.0, lambda, v[5], hx, hy, hxdhy, hydhx, sc, *x;
  

  mx = user->mx;            my = user->my;            lambda = user->param;
  hx = 1.0/(mx-1);  hy = 1.0/(my-1);
  sc = hx*hy;               hxdhy = hx/hy;            hydhx = hy/hx;

  /*
     Scatter ghost points to local vector, using the 2-step process
        DAGlobalToLocalBegin(), DAGlobalToLocalEnd().
     By placing code between these two statements, computations can be
     done while messages are in transition.
  */

  ierr = DAGlobalToLocalBegin(user->da,X,INSERT_VALUES,localX); CHKERRQ(ierr);
  ierr = DAGlobalToLocalEnd(user->da,X,INSERT_VALUES,localX); CHKERRQ(ierr);

  /*
     Get pointer to vector data
  */
  ierr = VecGetArray(localX,&x); CHKERRQ(ierr);


  /*
     Get local grid boundaries
  */
  ierr = DAGetCorners(user->da,&xs,&ys,NULL,&xm,&ym,NULL); CHKERRQ(ierr);
  ierr = DAGetGhostCorners(user->da,&gxs,&gys,NULL,&gxm,&gym,NULL); CHKERRQ(ierr);

  /* 
     Compute entries for the locally owned part of the Jacobian.
      - Currently, all PETSc parallel matrix formats are partitioned by
        contiguous chunks of rows across the processors. 
      - Each processor needs to insert only elements that it owns
        locally (but any non-local elements will be sent to the
        appropriate processor during matrix assembly). 
      - Here, we set all entries for a particular row at once.
      - We can set matrix entries either using either
        MatSetValuesLocal() or MatSetValues(), as discussed above.
  */
  for (j=ys; j<ys+ym; j++) {
    row = (j - gys)*gxm + xs - gxs - 1; 
    for (i=xs; i<xs+xm; i++) {
      row++;
      /* boundary points */
      if (i == 0 || j == 0 || i == mx-1 || j == my-1 ) {
        ierr = MatSetValuesLocal(jac,1,&row,1,&row,&one,INSERT_VALUES); CHKERRQ(ierr);
        continue;
      }
      /* interior grid points */
      v[0] = -hxdhy; col[0] = row - gxm;
      v[1] = -hydhx; col[1] = row - 1;
      v[2] = two*(hydhx + hxdhy) - sc*lambda*PetscExpScalar(x[row]); col[2] = row;
      v[3] = -hydhx; col[3] = row + 1;
      v[4] = -hxdhy; col[4] = row + gxm;
      ierr = MatSetValuesLocal(jac,1,&row,5,col,v,INSERT_VALUES); CHKERRQ(ierr);
    }
  }

  /* 
     Assemble matrix, using the 2-step process:
       MatAssemblyBegin(), MatAssemblyEnd().
     By placing code between these two statements, computations can be
     done while messages are in transition.
  */
  ierr = MatAssemblyBegin(jac,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  ierr = VecRestoreArray(localX,&x); CHKERRQ(ierr);
  ierr = MatAssemblyEnd(jac,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);

  /*
     Set flag to indicate that the Jacobian matrix retains an identical
     nonzero structure throughout all nonlinear iterations (although the
     values of the entries change). Thus, we can save some work in setting
     up the preconditioner (e.g., no need to redo symbolic factorization for
     ILU/ICC preconditioners).
      - If the nonzero structure of the matrix is different during
        successive linear solves, then the flag DIFFERENT_NONZERO_PATTERN
        must be used instead.  If you are unsure whether the matrix
        structure has changed or not, use the flag DIFFERENT_NONZERO_PATTERN.
      - Caution:  If you specify SAME_NONZERO_PATTERN, PETSc
        believes your assertion and does not check the structure
        of the matrix.  If you erroneously claim that the structure
        is the same when it actually is not, the new preconditioner
        will not function correctly.  Thus, use this optimization
        feature with caution!
  */

  /*
      Tell the matrix we will never add a new nonzero location to the
    matrix. If we do it will generate an error.
  */
  ierr = MatSetOption(jac,MAT_NEW_NONZERO_LOCATION_ERR,PETSC_TRUE);CHKERRQ(ierr);

  return 0;
}

