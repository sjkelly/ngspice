/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
Author: 1987 Kartikeya Mayaram, U. C. Berkeley CAD Group
**********/

#include "ngspice/ngspice.h"
#include "numd2def.h"
#include "ngspice/sperror.h"
#include "ngspice/suffix.h"


int
NUMD2mDelete(GENmodel *gen_model)
{
    NG_IGNORE(gen_model);
    return OK;
}
