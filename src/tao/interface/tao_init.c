#include "taosolver.h"   /*I "taosolver.h" I*/

PetscBool TaoInitializeCalled = PETSC_FALSE;
PetscBool TaoBeganPetsc = PETSC_FALSE;

static int tao_one=1;
static char* tao_executable = (char*)"tao";
static char** tao_executablePtr = &tao_executable;
#undef __FUNCT__
#define __FUNCT__ "TaoInitialize"
/*@C 
  TaoInitialize - Initializes the TAO component and many of the packages associated with it.

   Collective on MPI_COMM_WORLD

   Input Parameters:
+  argc - [optional] count of number of command line arguments
.  args - [optional] the command line arguments
.  file - [optional] PETSc database file, defaults to ~username/.petscrc
          (use PETSC_NULL for default)
-  help - [optional] Help message to print, use PETSC_NULL for no message

   Note:
   TaoInitialize() should always be called near the beginning of your 
   program.  This routine will call PetscInitialize() if it has not yet been
   called.

   Level: beginner

.seealso: TaoFinalize(), PetscInitialize()
@*/
PetscErrorCode TaoInitialize(int *argc, char ***args, const char file[], 
			     const char help[])
{
  PetscErrorCode ierr;

  if (TaoInitializeCalled) {
    return(0);
  }
  if (PetscInitializeCalled) {
    ierr=PetscInfo(0,"TAO successfully initialized.\n"); CHKERRQ(ierr);
  } else {
    if (argc&&args) {
      PetscInitialize(argc,args,file,help); 
    } else {
      PetscInitialize(&tao_one,&tao_executablePtr,0,0); 
    }
    TaoBeganPetsc=PETSC_TRUE;
  }
  if (!PetscInitializeCalled) {
    printf("Error initializing PETSc -- aborting.\n");
    exit(1);
  }
  TaoInitializeCalled = PETSC_TRUE;
  return 0;
}

#undef __FUNCT__
#define __FUNCT__ "TaoFinalize"
/*@
   TaoFinalize - Finalizes interfaces with other packages.

   Collective on MPI_COMM_WORLD

   Level: beginner

.seealso: TaoInitialize(), PetscFinalize()
@*/
PetscErrorCode TaoFinalize(void)
{
  if (TaoBeganPetsc) {
    PetscFinalize();
  } 
  return(0);
}

