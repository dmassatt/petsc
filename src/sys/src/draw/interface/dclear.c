/*$Id: dclear.c,v 1.28 2000/05/05 22:13:25 balay Exp bsmith $*/
/*
       Provides the calling sequences for all the basic Draw routines.
*/
#include "src/sys/src/draw/drawimpl.h"  /*I "petscdraw.h" I*/

#undef __FUNC__  
#define __FUNC__ /*<a name="DrawClear"></a>*/"DrawClear" 
/*@
   DrawClear - Clears graphical output.

   Not collective (Use DrawSynchronizedClear() for collective)

   Input Parameter:
.  draw - the drawing context

   Level: beginner

.keywords: draw, clear

.seealso: DrawBOP(), DrawEOP(), DrawSynchronizedClear()
@*/
int DrawClear(Draw draw)
{
  int ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(draw,DRAW_COOKIE);
  if (draw->ops->clear) {
    ierr = (*draw->ops->clear)(draw);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name="DrawBOP"></a>*/"DrawBOP" 
/*@
   DrawBOP - Begins a new page or frame on the selected graphical device.

   Collective on Draw

   Input Parameter:
.  draw - the drawing context

   Level: advanced

.keywords: draw, page, frame

.seealso: DrawEOP(), DrawClear()
@*/
int DrawBOP(Draw draw)
{
  int ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(draw,DRAW_COOKIE);
  if (draw->ops->beginpage) {
    ierr = (*draw->ops->beginpage)(draw);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}
#undef __FUNC__  
#define __FUNC__ /*<a name="DrawEOP"></a>*/"DrawEOP" 
/*@
   DrawEOP - Ends a page or frame on the selected graphical device.

   Collective on Draw

   Input Parameter:
.  draw - the drawing context

   Level: advanced

.keywords: draw, page, frame

.seealso: DrawBOP(), DrawClear()
@*/
int DrawEOP(Draw draw)
{
  int ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(draw,DRAW_COOKIE);
  if (draw->ops->endpage) {
    ierr =  (*draw->ops->endpage)(draw);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

