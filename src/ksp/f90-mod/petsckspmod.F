#define PETSC_USE_FORTRAN_MODULES

        module petscpcdef
        use petscdmdef
#include <finclude/petscpc.h>
#include <finclude/ftn-custom/petscpcdef.h90>
        end module

        module petsckspdef
        use petscpcdef
#include <finclude/petscksp.h>
#include <finclude/ftn-custom/petsckspdef.h90>
        end module

        module petscpc
        use petsckspdef
        use petscdm
#include <finclude/petscpc.h90>
        end module

        module petscksp
        use petsckspdef
        use petscpc
#include <finclude/petscksp.h90>
        end module


