/*$Id: ex2.c,v 1.14 2000/05/05 22:14:45 balay Exp bsmith $*/

/*
       Formatted test for ISStride routines.
*/

static char help[] = "Tests IS stride routines\n\n";

#include "petscis.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  int        i,n,ierr,*ii,start,stride;
  IS         is;
  PetscTruth flg;

  PetscInitialize(&argc,&argv,(char*)0,help);

  /*
     Test IS of size 0 
  */
  ierr = ISCreateStride(PETSC_COMM_SELF,0,0,2,&is);CHKERRA(ierr);
  ierr = ISGetSize(is,&n);CHKERRA(ierr);
  if (n != 0) SETERRA(1,0,0);
  ierr = ISStrideGetInfo(is,&start,&stride);CHKERRA(ierr);
  if (start != 0) SETERRA(1,0,0);
  if (stride != 2) SETERRA(1,0,0);
  ierr = ISStride(is,&flg);CHKERRA(ierr);
  if (flg != PETSC_TRUE) SETERRA(1,0,0);
  ierr = ISGetIndices(is,&ii);CHKERRA(ierr);
  ierr = ISRestoreIndices(is,&ii);CHKERRA(ierr);
  ierr = ISDestroy(is);CHKERRA(ierr);

  /*
     Test ISGetIndices()
  */
  ierr = ISCreateStride(PETSC_COMM_SELF,10000,-8,3,&is);CHKERRA(ierr);
  ierr = ISGetLocalSize(is,&n);CHKERRA(ierr);
  ierr = ISGetIndices(is,&ii);CHKERRA(ierr);
  for (i=0; i<10000; i++) {
    if (ii[i] != -8 + 3*i) SETERRA(1,0,0);
  }
  ierr = ISRestoreIndices(is,&ii);CHKERRA(ierr);
  ierr = ISDestroy(is);CHKERRA(ierr);

  PetscFinalize();
  return 0;
}
 






