/**** BSIM3v3.2.4, Released by Xuemei Xi 12/21/2001 ****/

/**********
 * Copyright 2001 Regents of the University of California. All rights reserved.
 * File: b3noi.c of BSIM3v3.2.4
 * Author: 1995 Gary W. Ng and Min-Chie Jeng.
 * Author: 1997-1999 Weidong Liu.
 * Author: 2001 Xuemei Xi
 * Modified by Xuemei Xi, 10/05, 12/21, 2001.
 * Modified bt Paolo Nenzi 2002 and Dietmar Warning 2003
 **********/

#include "ngspice.h"
#include "bsim3v32def.h"
#include "cktdefs.h"
#include "iferrmsg.h"
#include "noisedef.h"
#include "suffix.h"
#include "const.h"  /* jwan */

/*
 * BSIM3v32noise (mode, operation, firstModel, ckt, data, OnDens)
 *    This routine names and evaluates all of the noise sources
 *    associated with MOSFET's.  It starts with the model *firstModel and
 *    traverses all of its insts.  It then proceeds to any other models
 *    on the linked list.  The total output noise density generated by
 *    all of the MOSFET's is summed with the variable "OnDens".
 */

/*
 Channel thermal and flicker noises are calculated based on the value
 of model->BSIM3v32noiMod.
 If model->BSIM3v32noiMod = 1,
    Channel thermal noise = SPICE2 model
    Flicker noise         = SPICE2 model
 If model->BSIM3v32noiMod = 2,
    Channel thermal noise = BSIM3v32 model
    Flicker noise         = BSIM3v32 model
 If model->BSIM3v32noiMod = 3,
    Channel thermal noise = SPICE2 model
    Flicker noise         = BSIM3v32 model
 If model->BSIM3v32noiMod = 4,
    Channel thermal noise = BSIM3v32 model
    Flicker noise         = SPICE2 model
 */


/* 
 * The StrongInversionNoiseEval function has been modified in
 * the release 3.2.4 of BSIM3v32 model. To accomodate both the old
 * and the new code, I have renamed according to the following:
 *
 *
 * BSIM3v3.2.4 -> StrongInversionNoiseEvalNew
 * Previous    -> StrongInversionNoiseEvalOld
 *
 * 2002 Paolo Nenzi
 */

/*
 * JX: 1/f noise model is smoothed out 12/18/01.
 */

double
StrongInversionNoiseEvalNew(double Vds, BSIM3v32model *model,
			    BSIM3v32instance *here, double freq, double temp)
{
struct bsim3SizeDependParam *pParam;
double cd, esat, DelClm, EffFreq, N0, Nl;
double T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, Ssi;

    pParam = here->pParam;
    cd = fabs(here->BSIM3v32cd);
    esat = 2.0 * pParam->BSIM3v32vsattemp / here->BSIM3v32ueff;
    if(model->BSIM3v32em<=0.0) DelClm = 0.0; 
    else { 
	    T0 = ((((Vds - here->BSIM3v32Vdseff) / pParam->BSIM3v32litl) 
	    	+ model->BSIM3v32em) / esat);
            DelClm = pParam->BSIM3v32litl * log (MAX(T0, N_MINLOG));
    }
    EffFreq = pow(freq, model->BSIM3v32ef);
    T1 = CHARGE * CHARGE * 8.62e-5 * cd * temp * here->BSIM3v32ueff;
    T2 = 1.0e8 * EffFreq * here->BSIM3v32Abulk * model->BSIM3v32cox
       * pParam->BSIM3v32leff * pParam->BSIM3v32leff;
    N0 = model->BSIM3v32cox * here->BSIM3v32Vgsteff / CHARGE;
    Nl = model->BSIM3v32cox * here->BSIM3v32Vgsteff
    	  * (1.0 - here->BSIM3v32AbovVgst2Vtm * here->BSIM3v32Vdseff) / CHARGE;

    T3 = model->BSIM3v32oxideTrapDensityA
       * log(MAX(((N0 + 2.0e14) / (Nl + 2.0e14)), N_MINLOG));
    T4 = model->BSIM3v32oxideTrapDensityB * (N0 - Nl);
    T5 = model->BSIM3v32oxideTrapDensityC * 0.5 * (N0 * N0 - Nl * Nl);

    T6 = 8.62e-5 * temp * cd * cd;
    T7 = 1.0e8 * EffFreq * pParam->BSIM3v32leff
       * pParam->BSIM3v32leff * pParam->BSIM3v32weff;
    T8 = model->BSIM3v32oxideTrapDensityA + model->BSIM3v32oxideTrapDensityB * Nl
       + model->BSIM3v32oxideTrapDensityC * Nl * Nl;
    T9 = (Nl + 2.0e14) * (Nl + 2.0e14);

    Ssi = T1 / T2 * (T3 + T4 + T5) + T6 / T7 * DelClm * T8 / T9;
    return Ssi;
}

/*
 * The code for releases: BSIM3V32, BSIM3V322, BSIM3V323
 * follows
 */

double
StrongInversionNoiseEvalOld(double vgs, double vds, BSIM3v32model *model,
			    BSIM3v32instance *here, double freq, double temp)
{
  struct bsim3SizeDependParam *pParam;
  double cd, esat, DelClm, EffFreq, N0, Nl, Vgst;
  double T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, Ssi;

  pParam = here->pParam;
  cd = fabs (here->BSIM3v32cd);
  /* Added revision dependent code */
  if (model->BSIM3v32intVersion < BSIM3v32V323)
  {
    if (vds > here->BSIM3v32vdsat)
    {
      esat = 2.0 * pParam->BSIM3v32vsattemp / here->BSIM3v32ueff;
      T0 = ((((vds - here->BSIM3v32vdsat) / pParam->BSIM3v32litl) +
  	   model->BSIM3v32em) / esat);
      DelClm = pParam->BSIM3v32litl * log (MAX (T0, N_MINLOG));
    }
    else
      DelClm = 0.0;
  }
  else
  {
    if (model->BSIM3v32em <= 0.0)	/* flicker noise modified -JX */
      DelClm = 0.0;
    else if (vds > here->BSIM3v32vdsat)
    {
      esat = 2.0 * pParam->BSIM3v32vsattemp / here->BSIM3v32ueff;
      T0 = ((((vds - here->BSIM3v32vdsat) / pParam->BSIM3v32litl) +
  	   model->BSIM3v32em) / esat);
      DelClm = pParam->BSIM3v32litl * log (MAX (T0, N_MINLOG));
    }
    else
      DelClm = 0.0;
  }
  EffFreq = pow (freq, model->BSIM3v32ef);
  T1 = CHARGE * CHARGE * 8.62e-5 * cd * temp * here->BSIM3v32ueff;
  T2 = 1.0e8 * EffFreq * model->BSIM3v32cox
     * pParam->BSIM3v32leff * pParam->BSIM3v32leff;
  Vgst = vgs - here->BSIM3v32von;
  N0 = model->BSIM3v32cox * Vgst / CHARGE;
  if (N0 < 0.0)
      N0 = 0.0;
  Nl = model->BSIM3v32cox * (Vgst - MIN (vds, here->BSIM3v32vdsat)) / CHARGE;
  if (Nl < 0.0)
      Nl = 0.0;

  T3 = model->BSIM3v32oxideTrapDensityA
     * log (MAX (((N0 + 2.0e14) / (Nl + 2.0e14)), N_MINLOG));
  T4 = model->BSIM3v32oxideTrapDensityB * (N0 - Nl);
  T5 = model->BSIM3v32oxideTrapDensityC * 0.5 * (N0 * N0 - Nl * Nl);

  T6 = 8.62e-5 * temp * cd * cd;
  T7 = 1.0e8 * EffFreq * pParam->BSIM3v32leff
     * pParam->BSIM3v32leff * pParam->BSIM3v32weff;
  T8 = model->BSIM3v32oxideTrapDensityA + model->BSIM3v32oxideTrapDensityB * Nl
     + model->BSIM3v32oxideTrapDensityC * Nl * Nl;
  T9 = (Nl + 2.0e14) * (Nl + 2.0e14);

  Ssi = T1 / T2 * (T3 + T4 + T5) + T6 / T7 * DelClm * T8 / T9;
  return Ssi;
}



int
BSIM3v32noise (int mode, int operation, GENmodel *inModel, CKTcircuit *ckt,
	    Ndata *data, double *OnDens)
{
BSIM3v32model *model = (BSIM3v32model *)inModel;
BSIM3v32instance *here;
struct bsim3SizeDependParam *pParam;
char name[N_MXVLNTH];
double tempOnoise;
double tempInoise;
double noizDens[BSIM3v32NSRCS];
double lnNdens[BSIM3v32NSRCS];

double vgs, vds, Slimit;
double T1, T10, T11;
double Ssi, Swi;

double m;

int i;

    /* define the names of the noise sources */
    static char *BSIM3v32nNames[BSIM3v32NSRCS] =
    {   /* Note that we have to keep the order */
	".rd",              /* noise due to rd */
			    /* consistent with the index definitions */
	".rs",              /* noise due to rs */
			    /* in BSIM3v32defs.h */
	".id",              /* noise due to id */
	".1overf",          /* flicker (1/f) noise */
	""                  /* total transistor noise */
    };

    for (; model != NULL; model = model->BSIM3v32nextModel)
    {    for (here = model->BSIM3v32instances; here != NULL;
	      here = here->BSIM3v32nextInstance)
	 {    pParam = here->pParam;
	      switch (operation)
	      {  case N_OPEN:
		     /* see if we have to to produce a summary report */
		     /* if so, name all the noise generators */

		      if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0)
		      {   switch (mode)
			  {  case N_DENS:
			          for (i = 0; i < BSIM3v32NSRCS; i++)
				  {    (void) sprintf(name, "onoise.%s%s",
					              here->BSIM3v32name,
						      BSIM3v32nNames[i]);
                                       data->namelist = (IFuid *) trealloc(
					     (char *) data->namelist,
					     (data->numPlots + 1)
					     * sizeof(IFuid));
                                       if (!data->namelist)
					   return(E_NOMEM);
		                       (*(SPfrontEnd->IFnewUid)) (ckt,
			                  &(data->namelist[data->numPlots++]),
			                  (IFuid) NULL, name, UID_OTHER,
					  NULL);
				       /* we've added one more plot */
			          }
			          break;
		             case INT_NOIZ:
			          for (i = 0; i < BSIM3v32NSRCS; i++)
				  {    (void) sprintf(name, "onoise_total.%s%s",
						      here->BSIM3v32name,
						      BSIM3v32nNames[i]);
                                       data->namelist = (IFuid *) trealloc(
					     (char *) data->namelist,
					     (data->numPlots + 1)
					     * sizeof(IFuid));
                                       if (!data->namelist)
					   return(E_NOMEM);
		                       (*(SPfrontEnd->IFnewUid)) (ckt,
			                  &(data->namelist[data->numPlots++]),
			                  (IFuid) NULL, name, UID_OTHER,
					  NULL);
				       /* we've added one more plot */

			               (void) sprintf(name, "inoise_total.%s%s",
						      here->BSIM3v32name,
						      BSIM3v32nNames[i]);
                                       data->namelist = (IFuid *) trealloc(
					     (char *) data->namelist,
					     (data->numPlots + 1)
					     * sizeof(IFuid));
                                       if (!data->namelist)
					   return(E_NOMEM);
		                       (*(SPfrontEnd->IFnewUid)) (ckt,
			                  &(data->namelist[data->numPlots++]),
			                  (IFuid) NULL, name, UID_OTHER,
					  NULL);
				       /* we've added one more plot */
			          }
			          break;
		          }
		      }
		      break;
	         case N_CALC:
		      m = here->BSIM3v32m;
		      switch (mode)
		      {  case N_DENS:
		              NevalSrc(&noizDens[BSIM3v32RDNOIZ],
				       &lnNdens[BSIM3v32RDNOIZ], ckt, THERMNOISE,
				       here->BSIM3v32dNodePrime, here->BSIM3v32dNode,
				       here->BSIM3v32drainConductance * m);

		              NevalSrc(&noizDens[BSIM3v32RSNOIZ],
				       &lnNdens[BSIM3v32RSNOIZ], ckt, THERMNOISE,
				       here->BSIM3v32sNodePrime, here->BSIM3v32sNode,
				       here->BSIM3v32sourceConductance * m);

                              switch( model->BSIM3v32noiMod )
			      {  case 1:
			         case 3:
			              NevalSrc(&noizDens[BSIM3v32IDNOIZ],
				               &lnNdens[BSIM3v32IDNOIZ], ckt, 
					       THERMNOISE, here->BSIM3v32dNodePrime,
				               here->BSIM3v32sNodePrime,
                                               (2.0 / 3.0 * fabs(here->BSIM3v32gm
				               + here->BSIM3v32gds
					       + here->BSIM3v32gmbs)) * m);
				      break;
			         case 2:
			         case 4:
		      		      /* Added revision dependent code */
		      		      if (model->BSIM3v32intVersion == BSIM3v32V324)
				      {
		                        NevalSrc(&noizDens[BSIM3v32IDNOIZ],
				               &lnNdens[BSIM3v32IDNOIZ], ckt,
					       THERMNOISE, here->BSIM3v32dNodePrime,
                                               here->BSIM3v32sNodePrime,
					       (m * here->BSIM3v32ueff
					       * fabs(here->BSIM3v32qinv)
					       / (pParam->BSIM3v32leff * pParam->BSIM3v32leff
						+ here->BSIM3v32ueff * fabs(here->BSIM3v32qinv)
						* here->BSIM3v32rds)));    /* bugfix */
				      }
		      		      else
				      {	/* for all versions lower then 3.2.4 */
		                        NevalSrc(&noizDens[BSIM3v32IDNOIZ],
				               &lnNdens[BSIM3v32IDNOIZ], ckt,
					       THERMNOISE, here->BSIM3v32dNodePrime,
                                               here->BSIM3v32sNodePrime,
					       (m * here->BSIM3v32ueff
					       * fabs(here->BSIM3v32qinv
					       / (pParam->BSIM3v32leff
					       * pParam->BSIM3v32leff))));
				      }
				      break;
			      }
		              NevalSrc(&noizDens[BSIM3v32FLNOIZ], (double*) NULL,
				       ckt, N_GAIN, here->BSIM3v32dNodePrime,
				       here->BSIM3v32sNodePrime, (double) 0.0);

                              switch( model->BSIM3v32noiMod )
			      {  case 1:
			         case 4:
			              noizDens[BSIM3v32FLNOIZ] *= m * model->BSIM3v32kf
					    * exp(model->BSIM3v32af
					    * log(MAX(fabs(here->BSIM3v32cd),
					    N_MINLOG)))
					    / (pow(data->freq, model->BSIM3v32ef)
					    * pParam->BSIM3v32leff
				            * pParam->BSIM3v32leff
					    * model->BSIM3v32cox);
				      break;
			         case 2:
			         case 3:
		                      vgs = *(ckt->CKTstates[0] + here->BSIM3v32vgs);
		      		      vds = *(ckt->CKTstates[0] + here->BSIM3v32vds);
			              if (vds < 0.0)
			              {   vds = -vds;
			                  vgs = vgs + vds;
			              }
		      		      /* Added revision dependent code */
		      		      if (model->BSIM3v32intVersion == BSIM3v32V324)
				      {
			                Ssi = StrongInversionNoiseEvalNew(vds, model, 
			              		here, data->freq, ckt->CKTtemp);
				        T10 = model->BSIM3v32oxideTrapDensityA
					    * 8.62e-5 * ckt->CKTtemp;
		                        T11 = pParam->BSIM3v32weff
					    * pParam->BSIM3v32leff
				            * pow(data->freq, model->BSIM3v32ef)
				            * 4.0e36;
		                        Swi = T10 / T11 * here->BSIM3v32cd
				            * here->BSIM3v32cd;
				        T1 = Swi + Ssi;
				        if (T1 > 0.0)
                                            noizDens[BSIM3v32FLNOIZ] *= m * (Ssi * Swi) / T1; 
				        else
                                            noizDens[BSIM3v32FLNOIZ] *= 0.0;
				      }
		      		      else
				      {	/* for all versions lower then 3.2.4 */
					if (vgs >= here->BSIM3v32von + 0.1)
					  {
					    Ssi = StrongInversionNoiseEvalOld(vgs, vds, model,
							here, data->freq, ckt->CKTtemp);
					    noizDens[BSIM3v32FLNOIZ] *= m * Ssi;
					  }
					else
					  {
					    pParam = here->pParam;
					    T10 = model->BSIM3v32oxideTrapDensityA
					  	* 8.62e-5 * ckt->CKTtemp;
					    T11 = pParam->BSIM3v32weff
					  	* pParam-> BSIM3v32leff
					  	* pow (data->freq, model->BSIM3v32ef) 
					  	* 4.0e36;
					    Swi = T10 / T11 * here->BSIM3v32cd * here->BSIM3v32cd;

					    Slimit = StrongInversionNoiseEvalOld(
					         here->BSIM3v32von + 0.1, vds, model,
					         here, data->freq, ckt->CKTtemp);
					    T1 = Swi + Slimit;
					    if (T1 > 0.0)
					        	noizDens[BSIM3v32FLNOIZ] *= m * (Slimit * Swi) / T1;
					    else
					        	noizDens[BSIM3v32FLNOIZ] *= 0.0;
					  }
					}
					break;
			              }
                        	
		              lnNdens[BSIM3v32FLNOIZ] =
			        	     log(MAX(noizDens[BSIM3v32FLNOIZ], N_MINLOG));
                                
		              noizDens[BSIM3v32TOTNOIZ] = noizDens[BSIM3v32RDNOIZ]
						     + noizDens[BSIM3v32RSNOIZ]
						     + noizDens[BSIM3v32IDNOIZ]
						     + noizDens[BSIM3v32FLNOIZ];
		              lnNdens[BSIM3v32TOTNOIZ] = 
				     log(MAX(noizDens[BSIM3v32TOTNOIZ], N_MINLOG));

		              *OnDens += noizDens[BSIM3v32TOTNOIZ];

		              if (data->delFreq == 0.0)
			      {   /* if we haven't done any previous 
				     integration, we need to initialize our
				     "history" variables.
				    */

			          for (i = 0; i < BSIM3v32NSRCS; i++)
				  {    here->BSIM3v32nVar[LNLSTDENS][i] =
					     lnNdens[i];
			          }

			          /* clear out our integration variables
				     if it's the first pass
				   */
			          if (data->freq ==
				      ((NOISEAN*) ckt->CKTcurJob)->NstartFreq)
				  {   for (i = 0; i < BSIM3v32NSRCS; i++)
				      {    here->BSIM3v32nVar[OUTNOIZ][i] = 0.0;
				           here->BSIM3v32nVar[INNOIZ][i] = 0.0;
			              }
			          }
		              }
			      else
			      {   /* data->delFreq != 0.0,
				     we have to integrate.
				   */
			          for (i = 0; i < BSIM3v32NSRCS; i++)
				  {    if (i != BSIM3v32TOTNOIZ)
				       {   tempOnoise = Nintegrate(noizDens[i],
						lnNdens[i],
				                here->BSIM3v32nVar[LNLSTDENS][i],
						data);
				           tempInoise = Nintegrate(noizDens[i]
						* data->GainSqInv, lnNdens[i]
						+ data->lnGainInv,
				                here->BSIM3v32nVar[LNLSTDENS][i]
						+ data->lnGainInv, data);
				           here->BSIM3v32nVar[LNLSTDENS][i] =
						lnNdens[i];
				           data->outNoiz += tempOnoise;
				           data->inNoise += tempInoise;
				           if (((NOISEAN*)
					       ckt->CKTcurJob)->NStpsSm != 0)
					   {   here->BSIM3v32nVar[OUTNOIZ][i]
						     += tempOnoise;
				               here->BSIM3v32nVar[OUTNOIZ][BSIM3v32TOTNOIZ]
						     += tempOnoise;
				               here->BSIM3v32nVar[INNOIZ][i]
						     += tempInoise;
				               here->BSIM3v32nVar[INNOIZ][BSIM3v32TOTNOIZ]
						     += tempInoise;
                                           }
			               }
			          }
		              }
		              if (data->prtSummary)
			      {   for (i = 0; i < BSIM3v32NSRCS; i++)
				  {    /* print a summary report */
			               data->outpVector[data->outNumber++]
					     = noizDens[i];
			          }
		              }
		              break;
		         case INT_NOIZ:
			      /* already calculated, just output */
		              if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0)
			      {   for (i = 0; i < BSIM3v32NSRCS; i++)
				  {    data->outpVector[data->outNumber++]
					     = here->BSIM3v32nVar[OUTNOIZ][i];
			               data->outpVector[data->outNumber++]
					     = here->BSIM3v32nVar[INNOIZ][i];
			          }
		              }
		              break;
		      }
		      break;
	         case N_CLOSE:
		      /* do nothing, the main calling routine will close */
		      return (OK);
		      break;   /* the plots */
	      }       /* switch (operation) */
	 }    /* for here */
    }    /* for model */

    return(OK);
}



