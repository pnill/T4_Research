/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/long_term.c,v 1.6 1996/07/02 12:33:19 jutta Exp $ */

#include "private.h"

gsmword gsm_QLB[4] = {  3277,    11469,	21299,	   32767	};
gsmword gsm_DLB[4] = {  6554,    16384,	26214,	   32767	};

/*
 *  4.2.11 .. 4.2.12 LONG TERM PREDICTOR (LTP) SECTION
 */


/*
 * This module computes the LTP gain (bc) and the LTP lag (Nc)
 * for the long term analysis filter.   This is done by calculating a
 * maximum of the cross-correlation function between the current
 * sub-segment short term residual signal d[0..39] (output of
 * the short term analysis filter; for simplification the index
 * of this array begins at 0 and ends at 39 for each sub-segment of the
 * RPE-LTP analysis) and the previous reconstructed short term
 * residual signal dp[ -120 .. -1 ].  A dynamic scaling must be
 * performed to avoid overflow.
 */

 /* The next procedure exists in six versions.  First two integer
  * version (if USE_FLOAT_MUL is not defined); then four floating
  * point versions, twice with proper scaling (USE_FLOAT_MUL defined),
  * once without (USE_FLOAT_MUL and FAST defined, and fast run-time
  * option used).  Every pair has first a Cut version (see the -C
  * option to toast or the LTP_CUT option to gsm_option()), then the
  * uncut one.  (For a detailed explanation of why this is altogether
  * a bad idea, see Henry Spencer and Geoff Collyer, ``#ifdef Considered
  * Harmful''.)
  */

/* Phil Frisbie's loop unrolled version */
static void Calculation_of_the_LTP_parameters (
	register gsmword	* d,		/* [0..39]	IN	*/
	register gsmword	* dp,		/* [-120..-1]	IN	*/
	gsmword		* bc_out,	/* 		OUT	*/
	gsmword		* Nc_out	/* 		OUT	*/
)
{
	register int  	k, lambda;
	gsmword		Nc, bc;
	gsmword		wt[40];
    gsmword        * pwt = wt;
	longword	L_max, L_power;
	gsmword		R, S, dmax, scal;
	register gsmword	temp;
    gsmword        * pd = d;

	/*  Search of the optimum scaling of d[0..39].
	 */
	dmax = 0;

	for (k = 0; k < 40; k+=8) {
# undef STEP
# define STEP temp = (gsmword)GSM_ABS( *pd );\
		if (temp > dmax) dmax = temp;\
        pd++;

        STEP;STEP;STEP;STEP;
        STEP;STEP;STEP;STEP;
    }

	temp = 0;
	if (dmax == 0) scal = 0;
	else {
		temp = gsm_norm( SASL( (longword)dmax, 16 ) );
	}

	if (temp > 6) scal = 0;
	else scal = (gsmword)(6 - temp);

	/*  Initialization of a working array wt
	 */

    pd = d;
    for (k = 0; k < 40; k+=8) {
# undef STEP
# define STEP        *pwt = (gsmword)SASR( *pd, scal ); pd++; pwt++;

        STEP;STEP;STEP;STEP;
        STEP;STEP;STEP;STEP;
    }
 
	/* Search for the maximum cross-correlation and coding of the LTP lag
	 */
	L_max = 0;
	Nc    = 40;	/* index for the maximum cross-correlation */

	for (lambda = 40; lambda <= 120; lambda++) {

# undef STEP
#		define STEP(k) 	(longword)wt[k] * dp[k - lambda];

		register longword L_result;

		L_result  = STEP(0)  ; L_result += STEP(1) ;
		L_result += STEP(2)  ; L_result += STEP(3) ;
		L_result += STEP(4)  ; L_result += STEP(5)  ;
		L_result += STEP(6)  ; L_result += STEP(7)  ;
		L_result += STEP(8)  ; L_result += STEP(9)  ;
		L_result += STEP(10) ; L_result += STEP(11) ;
		L_result += STEP(12) ; L_result += STEP(13) ;
		L_result += STEP(14) ; L_result += STEP(15) ;
		L_result += STEP(16) ; L_result += STEP(17) ;
		L_result += STEP(18) ; L_result += STEP(19) ;
		L_result += STEP(20) ; L_result += STEP(21) ;
		L_result += STEP(22) ; L_result += STEP(23) ;
		L_result += STEP(24) ; L_result += STEP(25) ;
		L_result += STEP(26) ; L_result += STEP(27) ;
		L_result += STEP(28) ; L_result += STEP(29) ;
		L_result += STEP(30) ; L_result += STEP(31) ;
		L_result += STEP(32) ; L_result += STEP(33) ;
		L_result += STEP(34) ; L_result += STEP(35) ;
		L_result += STEP(36) ; L_result += STEP(37) ;
		L_result += STEP(38) ; L_result += STEP(39) ;

		if (L_result > L_max) {

			Nc    = (gsmword)lambda;
			L_max = L_result;
		}
	}

	*Nc_out = Nc;

	L_max = SASL(L_max, 1 );

	/*  Rescaling of L_max
	 */
	L_max = SASR( L_max, (6 - scal) );	/* sub(6, scal) */

	/*   Compute the power of the reconstructed short term residual
	 *   signal dp[..]
	 */
	L_power = 0;
	for (k = 0; k < 40; k+=4) {

		register longword L_temp;

		L_temp   = SASR( dp[k - Nc], 3 );
		L_power += L_temp * L_temp;

		L_temp   = SASR( dp[k + 1 - Nc], 3 );
		L_power += L_temp * L_temp;

		L_temp   = SASR( dp[k + 2 - Nc], 3 );
		L_power += L_temp * L_temp;

		L_temp   = SASR( dp[k + 3 - Nc], 3 );
		L_power += L_temp * L_temp;

    }
	L_power = SASL( L_power, 1 );	/* from L_MULT */

	/*  Normalization of L_max and L_power
	 */

	if (L_max <= 0)  {
		*bc_out = 0;
		return;
	}
	if (L_max >= L_power) {
		*bc_out = 3;
		return;
	}

	temp = gsm_norm( L_power );

	R = (gsmword)SASR( SASL( L_max, temp ), 16 );
	S = (gsmword)SASR( SASL( L_power, temp ), 16 );

	/*  Coding of the LTP gain
	 */

	/*  Table 4.3a must be used to obtain the level DLB[i] for the
	 *  quantization of the LTP gain b to get the coded version bc.
	 */
	for (bc = 0; bc <= 2; bc++) if (R <= GSM_MULT(S, gsm_DLB[bc])) break;
	*bc_out = bc;
}

static void Cut_Calculation_of_the_LTP_parameters (

	register gsmword	* d,		/* [0..39]	IN	*/
	register gsmword	* dp,		/* [-120..-1]	IN	*/
	gsmword		* bc_out,	/* 		OUT	*/
	gsmword		* Nc_out	/* 		OUT	*/
)
{
	register int  	k, lambda;
	gsmword		Nc, bc;

	longword	L_result;
	longword	L_max, L_power;
	gsmword		R, S, dmax, scal, best_k;

	register gsmword	temp, wt_k;

	/*  Search of the optimum scaling of d[0..39].
	 */
	dmax = best_k = 0;
	for (k = 0; k <= 39; k++) {
		temp = (gsmword)GSM_ABS( d[k] );
		if (temp > dmax) {
			dmax = temp;
			best_k = (gsmword)k;
		}
	}
	temp = 0;
	if (dmax == 0) scal = 0;
	else {
		temp = gsm_norm( (longword)dmax << 16 );
	}
	if (temp > 6) scal = 0;
	else scal = (gsmword)(6 - temp);

	/* Search for the maximum cross-correlation and coding of the LTP lag
	 */
	L_max = 0;
	Nc    = 40;	/* index for the maximum cross-correlation */
	wt_k  = (gsmword)SASR(d[best_k], scal);

	for (lambda = 40; lambda <= 120; lambda++) {
		L_result = (longword)wt_k * dp[best_k - lambda];
		if (L_result > L_max) {
			Nc    = (gsmword)lambda;
			L_max = L_result;
		}
	}
	*Nc_out = Nc;
	L_max <<= 1;

	/*  Rescaling of L_max
	 */
	L_max = L_max >> (6 - scal);	/* sub(6, scal) */

	/*   Compute the power of the reconstructed short term residual
	 *   signal dp[..]
	 */
	L_power = 0;
	for (k = 0; k <= 39; k++) {

		register longword L_temp;

		L_temp   = SASR( dp[k - Nc], 3 );
		L_power += L_temp * L_temp;
	}
	L_power <<= 1;	/* from L_MULT */

	/*  Normalization of L_max and L_power
	 */

	if (L_max <= 0)  {
		*bc_out = 0;
		return;
	}
	if (L_max >= L_power) {
		*bc_out = 3;
		return;
	}

	temp = gsm_norm( L_power );

	R = (gsmword)SASR( SASL(L_max, temp), 16 );
	S = (gsmword)SASR( SASL(L_power, temp), 16 );

	/*  Coding of the LTP gain
	 */

	/*  Table 4.3a must be used to obtain the level DLB[i] for the
	 *  quantization of the LTP gain b to get the coded version bc.
	 */
	for (bc = 0; bc <= 2; bc++) if (R <= GSM_MULT(S, gsm_DLB[bc])) break;
	*bc_out = bc;
}

/* 4.2.12 */

static void Long_term_analysis_filtering (
	gsmword		bc,	/* 					IN  */
	gsmword		Nc,	/* 					IN  */
	register gsmword	* dp,	/* previous d	[-120..-1]		IN  */
	register gsmword	* d,	/* d		[0..39]			IN  */
	register gsmword	* dpp,	/* estimate	[0..39]			OUT */
	register gsmword	* e	/* long term res. signal [0..39]	OUT */
)
/*
 *  In this part, we have to decode the bc parameter to compute
 *  the samples of the estimate dpp[0..39].  The decoding of bc needs the
 *  use of table 4.3b.  The long term residual signal e[0..39]
 *  is then calculated to be fed to the RPE encoding section.
 */
{
	register int      k;

    dp -= Nc;

#	undef STEP
#	define STEP(BP)					\
	for (k = 40; k--; e++, dpp++, d++, dp++) {			\
		*dpp = (gsmword)GSM_MULT_R( BP, *dp);	\
		*e = (gsmword)( *d - *dpp );	\
	}

	switch (bc) {
	case 0:	STEP(  3277 ); break;
	case 1:	STEP( 11469 ); break;
	case 2: STEP( 21299 ); break;
	case 3: STEP( 32767 ); break; 
	}
}

void Gsm_Long_Term_Predictor ( 	/* 4x for 160 samples */
	struct gsm_state	* S,
	gsmword	* d,	/* [0..39]   residual signal	IN	*/
	gsmword	* dp,	/* [-120..-1] d'		IN	*/

	gsmword	* e,	/* [0..39] 			OUT	*/
	gsmword	* dpp,	/* [0..39] 			OUT	*/
	gsmword	* Nc,	/* correlation lag		OUT	*/
	gsmword	* bc	/* gain factor			OUT	*/
)
{

    if(S->ltp_cut != 0)
    {
	    Cut_Calculation_of_the_LTP_parameters(d, dp, bc, Nc);
    }
    else
    {
	    Calculation_of_the_LTP_parameters(d, dp, bc, Nc);
    }

	Long_term_analysis_filtering( *bc, *Nc, dp, d, dpp, e );
}

/* 4.3.2 */
void Gsm_Long_Term_Synthesis_Filtering (
	struct gsm_state	* S,

	gsmword			Ncr,
	gsmword			bcr,
	register gsmword		* erp,	   /* [0..39]		  	 IN */
	register gsmword		* drp	   /* [-120..-1] IN, [-120..40] OUT */
)
/*
 *  This procedure uses the bcr and Ncr parameter to realize the
 *  long term synthesis filtering.  The decoding of bcr needs
 *  table 4.3b.
 */
{
	register int 		k;
	gsmword			    brp, Nr;
    gsmword		        * pdrp;

	/*  Check the limits of Nr.
	 */
	Nr = (gsmword)(Ncr < 40 || Ncr > 120 ? S->nrp : Ncr);
	S->nrp = Nr;

	/*  Decoding of the LTP gain bcr
	 */
	brp = gsm_QLB[ bcr ];

	/*  Computation of the reconstructed short term residual 
	 *  signal drp[0..39]
	 */
    pdrp = drp;
	for (k = 40; k--; pdrp++) {
		*pdrp = (gsmword)(*erp++ + GSM_MULT_R( brp, pdrp[ -(Nr) ] ));
	}

	/*
	 *  Update of the reconstructed short term residual signal
	 *  drp[ -1..-120 ]
	 */
    memcpy(&drp[-120], &drp[-80], (sizeof(drp[0]) * 120));
}
