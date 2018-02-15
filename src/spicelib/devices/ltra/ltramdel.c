/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1990 Jaijeet S. Roychowdhury
**********/

#include "ngspice/ngspice.h"
#include "ltradefs.h"
#include "ngspice/sperror.h"
#include "ngspice/suffix.h"


int
LTRAmDelete(GENmodel *gen_model)
{
    NG_IGNORE(gen_model);
    return OK;
}
