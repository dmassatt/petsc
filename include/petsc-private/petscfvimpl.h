#if !defined(_PETSCFVIMPL_H)
#define _PETSCFVIMPL_H

#include <petscfv.h>
#include <petsc-private/petscimpl.h>

typedef struct _PetscLimiterOps *PetscLimiterOps;
struct _PetscLimiterOps {
  PetscErrorCode (*setfromoptions)(PetscLimiter);
  PetscErrorCode (*setup)(PetscLimiter);
  PetscErrorCode (*view)(PetscLimiter,PetscViewer);
  PetscErrorCode (*destroy)(PetscLimiter);
  PetscErrorCode (*limit)(PetscLimiter, PetscScalar, PetscScalar *);
};

struct _p_PetscLimiter {
  PETSCHEADER(struct _PetscLimiterOps);
  void           *data;             /* Implementation object */
};

typedef struct {
  PetscInt dummy;
} PetscLimiter_Sin;

typedef struct _PetscFVOps *PetscFVOps;
struct _PetscFVOps {
  PetscErrorCode (*setfromoptions)(PetscFV);
  PetscErrorCode (*setup)(PetscFV);
  PetscErrorCode (*view)(PetscFV,PetscViewer);
  PetscErrorCode (*destroy)(PetscFV);
  PetscErrorCode (*integraterhsfunction)(PetscFV, PetscInt, PetscInt, PetscFV[], PetscInt, PetscCellGeometry, PetscCellGeometry, PetscScalar[], PetscScalar[],
                                         void (*)(const PetscReal[], const PetscReal[], const PetscScalar[], const PetscScalar[], PetscScalar[], void *),
                                         PetscScalar[], PetscScalar[], void *);
};

struct _p_PetscFV {
  PETSCHEADER(struct _PetscFVOps);
  void           *data;             /* Implementation object */
  PetscLimiter    limiter;          /* The slope limiter */
  PetscInt        numComponents;    /* The number of field components */
  PetscInt        dim;              /* The spatial dimension */
  PetscBool       computeGradients; /* Flag for gradient computation */
  PetscScalar    *fluxWork;         /* The work array for flux calculation */
};

typedef struct {
  PetscInt cellType;
} PetscFV_Upwind;

typedef struct {
  PetscInt cellType;
} PetscFV_LeastSquares;

#endif
