/*$Id: aijbaij.c,v 1.1 2000/06/06 19:00:44 bsmith Exp bsmith $*/

#include "src/mat/impls/baij/seq/baij.h"

#undef __FUNC__  
#define __FUNC__ /*<a name="MatConvert_SeqBAIJ_SeqAIJ"></a>*/"MatConvert_SeqBAI_SeqAIJ"
int MatConvert_SeqBAIJ_SeqAIJ(Mat A,MatType newtype,Mat *B)
{
  Mat_SeqBAIJ *a = (Mat_SeqBAIJ*)A->data; 
  int         ierr,bs = a->bs,*ai = a->i,*aj = a->j,n = A->M/bs,i,j,k;
  int         *rowlengths,*rows,*cols,maxlen = 0,ncols;
  Scalar      *aa = a->a;

  PetscFunctionBegin;
  rowlengths = (int*)PetscMalloc(n*bs*sizeof(int));CHKPTRQ(rowlengths);
  for (i=0; i<n; i++) {
    maxlen = PetscMax(maxlen,(ai[i+1] - ai[i]));
    for (j=0; j<bs; j++) {
      rowlengths[i*bs+j] = bs*(ai[i+1] - ai[i]);
    }
  }
  ierr = MatCreateSeqAIJ(PETSC_COMM_SELF,A->m,A->n,0,rowlengths,B);CHKERRQ(ierr);
  ierr = MatSetOption(*B,MAT_COLUMN_ORIENTED);CHKERRQ(ierr);
  ierr = MatSetOption(*B,MAT_ROWS_SORTED);CHKERRQ(ierr);
  ierr = MatSetOption(*B,MAT_COLUMNS_SORTED);CHKERRQ(ierr);
  ierr = PetscFree(rowlengths);CHKERRQ(ierr);

  rows = (int*)PetscMalloc(bs*sizeof(int));CHKPTRQ(rows);
  cols = (int*)PetscMalloc(bs*maxlen*sizeof(int));CHKPTRQ(cols);
  for (i=0; i<n; i++) {
    for (j=0; j<bs; j++) {
      rows[j] = i*bs+j;
    }
    ncols = ai[i+1] - ai[i];
    for (k=0; k<ncols; k++) {
      for (j=0; j<bs; j++) {
        cols[k*bs+j] = bs*(*aj) + j;
      }
      aj++;
    }
    ierr  = MatSetValues(*B,bs,rows,bs*ncols,cols,aa,INSERT_VALUES);
    aa   += ncols*bs*bs;
  }
  ierr = PetscFree(cols);CHKERRQ(ierr);
  ierr = PetscFree(rows);CHKERRQ(ierr);
  ierr = MatAssemblyBegin(*B,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(*B,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


