/* $Id: matimpl.h,v 1.111 2000/06/02 16:58:45 bsmith Exp bsmith $ */

#if !defined(__MATIMPL)
#define __MATIMPL
#include "petscmat.h"

/*
  This file defines the parts of the matrix data structure that are 
  shared by all matrix types.
*/

/*
    If you add entries here also add them to the MATOP enum
    in include/petscmat.h and include/finclude/petscmat.h
*/
typedef struct _MatOps *MatOps;
struct _MatOps {
  int       (*setvalues)(Mat,int,int *,int,int *,Scalar *,InsertMode),
            (*getrow)(Mat,int,int *,int **,Scalar **),
            (*restorerow)(Mat,int,int *,int **,Scalar **),
            (*mult)(Mat,Vec,Vec),
/* 4*/      (*multadd)(Mat,Vec,Vec,Vec),
            (*multtranspose)(Mat,Vec,Vec),
            (*multtransposeadd)(Mat,Vec,Vec,Vec),
            (*solve)(Mat,Vec,Vec),
            (*solveadd)(Mat,Vec,Vec,Vec),
            (*solvetranspose)(Mat,Vec,Vec),
/*10*/      (*solvetransposeadd)(Mat,Vec,Vec,Vec),
            (*lufactor)(Mat,IS,IS,MatLUInfo*),
            (*choleskyfactor)(Mat,IS,double),
            (*relax)(Mat,Vec,double,MatSORType,double,int,Vec),
            (*transpose)(Mat,Mat *),
/*15*/      (*getinfo)(Mat,MatInfoType,MatInfo*),
            (*equal)(Mat,Mat,PetscTruth *),
            (*getdiagonal)(Mat,Vec),
            (*diagonalscale)(Mat,Vec,Vec),
            (*norm)(Mat,NormType,double *),
/*20*/      (*assemblybegin)(Mat,MatAssemblyType),
            (*assemblyend)(Mat,MatAssemblyType),
            (*compress)(Mat),
            (*setoption)(Mat,MatOption),
            (*zeroentries)(Mat),
/*25*/      (*zerorows)(Mat,IS,Scalar *),
            (*lufactorsymbolic)(Mat,IS,IS,MatLUInfo*,Mat *),
            (*lufactornumeric)(Mat,Mat *),
            (*choleskyfactorsymbolic)(Mat,IS,double,Mat *),
            (*choleskyfactornumeric)(Mat,Mat *),
/*30*/      (*getsize)(Mat,int *,int *),
            (*getlocalsize)(Mat,int *,int *),
            (*getownershiprange)(Mat,int *,int *),
            (*ilufactorsymbolic)(Mat,IS,IS,MatILUInfo*,Mat *),
            (*incompletecholeskyfactorsymbolic)(Mat,IS,double,int,Mat *),
/*35*/      (*getarray)(Mat,Scalar **),
            (*restorearray)(Mat,Scalar **),
            (*duplicate)(Mat,MatDuplicateOption,Mat *),
            (*forwardsolve)(Mat,Vec,Vec),
            (*backwardsolve)(Mat,Vec,Vec),
/*40*/      (*ilufactor)(Mat,IS,IS,MatILUInfo*),
            (*incompletecholeskyfactor)(Mat,IS,double),
            (*axpy)(Scalar *,Mat,Mat),
            (*getsubmatrices)(Mat,int,IS *,IS *,MatReuse,Mat **),
            (*increaseoverlap)(Mat,int,IS *,int),
/*45*/      (*getvalues)(Mat,int,int *,int,int *,Scalar *),
            (*copy)(Mat,Mat,MatStructure),
            (*printhelp)(Mat),
            (*scale)(Scalar *,Mat),
            (*shift)(Scalar *,Mat),
/*50*/      (*diagonalset)(Mat,Vec,InsertMode),
            (*iludtfactor)(Mat,MatILUInfo*,IS,IS,Mat *),
            (*getblocksize)(Mat,int *),
            (*getrowij)(Mat,int,PetscTruth,int*,int **,int **,PetscTruth *),
            (*restorerowij)(Mat,int,PetscTruth,int *,int **,int **,PetscTruth *),
/*55*/      (*getcolumnij)(Mat,int,PetscTruth,int*,int **,int **,PetscTruth *),
            (*restorecolumnij)(Mat,int,PetscTruth,int*,int **,int **,PetscTruth *),
            (*fdcoloringcreate)(Mat,ISColoring,MatFDColoring),
            (*coloringpatch)(Mat,int,int *,ISColoring*),
            (*setunfactored)(Mat),
/*60*/      (*permute)(Mat,IS,IS,Mat*),
            (*setvaluesblocked)(Mat,int,int *,int,int *,Scalar *,InsertMode),
            (*getsubmatrix)(Mat,IS,IS,int,MatReuse,Mat*),
            (*destroy)(Mat),
            (*view)(Mat,Viewer),
            (*getmaps)(Mat,Map*,Map*),
            (*usescaledform)(Mat,PetscTruth),
            (*scalesystem)(Mat,Vec,Vec),
            (*unscalesystem)(Mat,Vec,Vec),
            (*setlocaltoglobalmapping)(Mat,ISLocalToGlobalMapping),
            (*setvalueslocal)(Mat,int,int *,int,int *,Scalar *,InsertMode),
            (*zerorowslocal)(Mat,IS,Scalar *);
};

/*
   Utility private matrix routines
*/
EXTERN int MatConvert_Basic(Mat,MatType,Mat*);
EXTERN int MatCopy_Basic(Mat,Mat,MatStructure);
EXTERN int MatView_Private(Mat);
EXTERN int MatGetMaps_Petsc(Mat,Map *,Map *);

/* 
  The stash is used to temporarily store inserted matrix values that 
  belong to another processor. During the assembly phase the stashed 
  values are moved to the correct processor and 
*/

typedef struct {
  int           nmax;                   /* maximum stash size */
  int           umax;                   /* user specified max-size */
  int           oldnmax;                /* the nmax value used previously */
  int           n;                      /* stash size */
  int           bs;                     /* block size of the stash */
  int           reallocs;               /* preserve the no of mallocs invoked */           
  int           *idx;                   /* global row numbers in stash */
  int           *idy;                   /* global column numbers in stash */
  MatScalar     *array;                 /* array to hold stashed values */
  /* The following variables are used for communication */
  MPI_Comm      comm;
  int           size,rank;
  int           tag1,tag2;
  MPI_Request   *send_waits;            /* array of send requests */
  MPI_Request   *recv_waits;            /* array of receive requests */
  MPI_Status    *send_status;           /* array of send status */
  int           nsends,nrecvs;         /* numbers of sends and receives */
  MatScalar     *svalues,*rvalues;     /* sending and receiving data */
  int           rmax;                   /* maximum message length */
  int           *nprocs;                /* tmp data used both duiring scatterbegin and end */
  int           nprocessed;             /* number of messages already processed */
} MatStash;

EXTERN int MatStashCreate_Private(MPI_Comm,int,MatStash*);
EXTERN int MatStashDestroy_Private(MatStash*);
EXTERN int MatStashScatterEnd_Private(MatStash*);
EXTERN int MatStashSetInitialSize_Private(MatStash*,int);
EXTERN int MatStashGetInfo_Private(MatStash*,int*,int*);
EXTERN int MatStashValuesRow_Private(MatStash*,int,int,int*,MatScalar*);
EXTERN int MatStashValuesCol_Private(MatStash*,int,int,int*,MatScalar*,int);
EXTERN int MatStashValuesRowBlocked_Private(MatStash*,int,int,int*,MatScalar*,int,int,int);
EXTERN int MatStashValuesColBlocked_Private(MatStash*,int,int,int*,MatScalar*,int,int,int);
EXTERN int MatStashScatterBegin_Private(MatStash*,int*);
EXTERN int MatStashScatterGetMesg_Private(MatStash*,int*,int**,int**,MatScalar**,int*);

#define FACTOR_LU       1
#define FACTOR_CHOLESKY 2

struct _p_Mat {
  PETSCHEADER(struct _MatOps)
  Map                    rmap,cmap;
  void                   *data;            /* implementation-specific data */
  int                    factor;           /* 0, FACTOR_LU, or FACTOR_CHOLESKY */
  double                 lupivotthreshold; /* threshold for pivoting */
  PetscTruth             assembled;        /* is the matrix assembled? */
  PetscTruth             was_assembled;    /* new values inserted into assembled mat */
  int                    num_ass;          /* number of times matrix has been assembled */
  PetscTruth             same_nonzero;     /* matrix has same nonzero pattern as previous */
  int                    M,N;             /* global numbers of rows, columns */
  int                    m,n;             /* local numbers of rows, columns */
  MatInfo                info;             /* matrix information */
  ISLocalToGlobalMapping mapping;          /* mapping used in MatSetValuesLocal() */
  ISLocalToGlobalMapping bmapping;         /* mapping used in MatSetValuesBlockedLocal() */
  InsertMode             insertmode;       /* have values been inserted in matrix or added? */
  MatStash               stash,bstash;     /* used for assembling off-proc mat emements */
};


/*
    Object for partitioning graphs
*/

typedef struct _MatPartitioningOps *MatPartitioningOps;
struct _MatPartitioningOps {
  int         (*apply)(MatPartitioning,IS*);
  int         (*setfromoptions)(MatPartitioning);
  int         (*printhelp)(MatPartitioning);
  int         (*destroy)(MatPartitioning);
  int         (*view)(MatPartitioning,Viewer);
};

struct _p_MatPartitioning {
  PETSCHEADER(struct _MatPartitioningOps)
  Mat         adj;
  int         *vertex_weights;
  int         n;                                 /* number of partitions */
  void        *data;
  int         setupcalled;
};

/*
    MatFDColoring is used to compute Jacobian matrices efficiently
  via coloring. The data structure is explained below in an example.

   Color =   0    1     0    2   |   2      3       0 
   ---------------------------------------------------
            00   01              |          05
            10   11              |   14     15               Processor  0
                       22    23  |          25
                       32    33  | 
   ===================================================
                                 |   44     45     46
            50                   |          55               Processor 1
                                 |   64            66
   ---------------------------------------------------

    ncolors = 4;

    ncolumns      = {2,1,1,0}
    columns       = {{0,2},{1},{3},{}}
    nrows         = {4,2,3,3}
    rows          = {{0,1,2,3},{0,1},{1,2,3},{0,1,2}}
    columnsforrow = {{0,0,2,2},{1,1},{4,3,3},{5,5,5}}
    vscaleforrow  = {{,,,},{,},{,,},{,,}}
    vwscale       = {dx(0),dx(1),dx(2),dx(3)}               MPI Vec
    vscale        = {dx(0),dx(1),dx(2),dx(3),dx(4),dx(5)}   Seq Vec

    ncolumns      = {1,0,1,1}
    columns       = {{6},{},{4},{5}}
    nrows         = {3,0,2,2}
    rows          = {{0,1,2},{},{1,2},{1,2}}
    columnsforrow = {{6,0,6},{},{4,4},{5,5}}
    vscaleforrow =  {{,,},{},{,},{,}}
    vwscale       = {dx(4),dx(5),dx(6)}              MPI Vec
    vscale        = {dx(0),dx(4),dx(5),dx(6)}        Seq Vec

    See the routine MatFDColoringApply() for how this data is used
    to compute the Jacobian.

*/

struct  _p_MatFDColoring{
  PETSCHEADER(int)
  int    M,N,m;            /* total rows, columns; local rows */
  int    rstart;           /* first row owned by local processor */
  int    ncolors;          /* number of colors */
  int    *ncolumns;        /* number of local columns for a color */ 
  int    **columns;        /* lists the local columns of each color (using global column numbering) */
  int    *nrows;           /* number of local rows for each color */
  int    **rows;           /* lists the local rows for each color (using the local row numbering) */
  int    **columnsforrow;  /* lists the corresponding columns for those rows (using the global column) */ 
  double error_rel;        /* square root of relative error in computing function */
  double umin;             /* minimum allowable u'dx value */
  int    freq;             /* frequency at which new Jacobian is computed */
  Vec    w1,w2,w3;         /* work vectors used in computing Jacobian */
  int    (*f)(void);       /* function that defines Jacobian */
  void   *fctx;            /* optional user-defined context for use by the function f */
  int    **vscaleforrow;   /* location in vscale for each columnsforrow[] entry */
  Vec    vscale;   /* holds FD scaling, i.e. 1/dx for each perturbed column */
};



#endif






