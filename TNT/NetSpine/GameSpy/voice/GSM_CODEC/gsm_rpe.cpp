/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/rpe.c,v 1.3 1994/05/10 20:18:46 jutta Exp $ */

#include "private.h"

gsmword gsm_FAC[8]	= { 18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767 };
gsmword gsm_NRFAC[8] = { 29128, 26215, 23832, 21846, 20165, 18725, 17476, 16384 };
gsmword gsm_H[11] = {-134, -374, 0, 2054, 5741, 8192, 5741, 2054, 0, -374, -134 };

/*  4.2.13 .. 4.2.17  RPE ENCODING SECTION
 */

/* 4.2.13 */

static void Weighting_filter (
	gsmword	    * e,	/* signal [-5..0.39.44]	IN  */
	gsmword		* x		/* signal [0..39]	OUT */
)
{
	register longword	L_result, ltmp;
    register gsmword	* pe = e;
	register int		k /* , i */ ;

	pe -= 5;

	/*  Compute the signal x[0..39]
	 */ 
	for (k = 40; k--; pe++) {

#undef	STEP
#define	STEP( i, H )	(pe[i] * (longword)H)

		/*  Every one of these multiplications is done twice --
		 *  but I don't see an elegant way to optimize this. 
		 *  Do you?
		 */

#ifdef	STUPID_COMPILER
		L_result += STEP(	0, 	-134 ) ;
		L_result += STEP(	1, 	-374 ) ;
	    L_result += STEP(	2, 	0    ) ;
		L_result += STEP(	3, 	2054 ) ;
		L_result += STEP(	4, 	5741 ) ;
		L_result += STEP(	5, 	8192 ) ;
		L_result += STEP(	6, 	5741 ) ;
		L_result += STEP(	7, 	2054 ) ;
	 	L_result += STEP(	8, 	0    ) ;
		L_result += STEP(	9, 	-374 ) ;
		L_result += STEP(	10, -134 ) ;
/*
*/
#else
#undef	STEP
#define	STEP( i, H )	(SASL( pe[i], (H) ) )

		L_result = 4096 - STEP( 0, 7 ) - (STEP( 1, 8 ))
		+ STEP(	3, 	11 ) + STEP( 4, 12 ) + STEP( 4, 10 )
		+ STEP(	5, 	13 ) + STEP( 6, 12 ) + STEP( 6, 10 )
		+ STEP(	7, 	11 ) - STEP( 9, 8 ) - STEP( 10, 7 );
#endif

		*x++ =  (gsmword)GSM_ADD( SASR( L_result, 13 ), 0 );
	}
}

/* 4.2.14 */

static void RPE_grid_selection (
	gsmword		* x,		/* [0..39]		IN  */ 
	gsmword		* xM,		/* [0..12]		OUT */
	gsmword		* Mc_out	/*			OUT */
)
/*
 *  The signal x[0..39] is used to select the RPE grid which is
 *  represented by Mc.
 */
{
	/* register gsmword	temp1;	*/
	register int		/* m, */  i;
	register longword	L_result, L_temp;
	longword		EM;	/* xxx should be L_EM? */
	gsmword			Mc;

	longword		L_common_0_3;

	EM = 0;
	Mc = 0;

	/* for (m = 0; m <= 3; m++) {
	 *	L_result = 0;
	 *
	 *
	 *	for (i = 0; i <= 12; i++) {
	 *
	 *		temp1    = SASR( x[m + 3*i], 2 );
	 *
	 *		assert(temp1 != MIN_WORD);
	 *
	 *		L_temp   = GSM_L_MULT( temp1, temp1 );
	 *		L_result = GSM_L_ADD( L_temp, L_result );
	 *	}
	 * 
	 *	if (L_result > EM) {
	 *		Mc = m;
	 *		EM = L_result;
	 *	}
	 * }
	 */

#undef	STEP
#define	STEP( m, i )		L_temp = SASR( x[m + 3 * i], 2 );	\
				L_result += L_temp * L_temp;

	/* common part of 0 and 3 */

	L_result = 0;
	STEP( 0, 1 ); STEP( 0, 2 ); STEP( 0, 3 ); STEP( 0, 4 );
	STEP( 0, 5 ); STEP( 0, 6 ); STEP( 0, 7 ); STEP( 0, 8 );
	STEP( 0, 9 ); STEP( 0, 10); STEP( 0, 11); STEP( 0, 12);
	L_common_0_3 = L_result;

	/* i = 0 */

	STEP( 0, 0 );
	L_result = SASL( L_result, 1 );	/* implicit in L_MULT */
	EM = L_result;

	/* i = 1 */

	L_result = 0;
	STEP( 1, 0 );
	STEP( 1, 1 ); STEP( 1, 2 ); STEP( 1, 3 ); STEP( 1, 4 );
	STEP( 1, 5 ); STEP( 1, 6 ); STEP( 1, 7 ); STEP( 1, 8 );
	STEP( 1, 9 ); STEP( 1, 10); STEP( 1, 11); STEP( 1, 12);
	L_result = SASL( L_result, 1 );
	if (L_result > EM) {
		Mc = 1;
	 	EM = L_result;
	}

	/* i = 2 */

	L_result = 0;
	STEP( 2, 0 );
	STEP( 2, 1 ); STEP( 2, 2 ); STEP( 2, 3 ); STEP( 2, 4 );
	STEP( 2, 5 ); STEP( 2, 6 ); STEP( 2, 7 ); STEP( 2, 8 );
	STEP( 2, 9 ); STEP( 2, 10); STEP( 2, 11); STEP( 2, 12);
	L_result = SASL( L_result, 1 );
	if (L_result > EM) {
		Mc = 2;
	 	EM = L_result;
	}

	/* i = 3 */

	L_result = L_common_0_3;
	STEP( 3, 12 );
	L_result = SASL( L_result, 1 );
	if (L_result > EM) {
		Mc = 3;
	 	EM = L_result;
	}

	/**/

	/*  Down-sampling by a factor 3 to get the selected xM[0..12]
	 *  RPE sequence.
	 */
	for (i = 0; i <= 12; i ++) xM[i] = x[Mc + 3*i];
	*Mc_out = Mc;
}

/* 4.12.15 */

static void APCM_quantization_xmaxc_to_exp_mant (
	gsmword		xmaxc,		/* IN 	*/
	gsmword		* exp_out,	/* OUT	*/
	gsmword		* mant_out )	/* OUT  */
{
	gsmword	exp, mant;

	/* Compute exponent and mantissa of the decoded version of xmaxc
	 */

	exp = 0;
	if (xmaxc > 15) exp = (gsmword)(SASR(xmaxc, 3) - 1);
	mant = (gsmword)(xmaxc - SASL( exp, 3 ));

	if (mant == 0) {
		exp  = -4;
		mant = 7;
	}
	else {
		while (mant <= 7) {
			mant = (gsmword)(SASL(mant, 1) | 1);
			exp--;
		}
		mant -= 8;
	}

	*exp_out  = exp;
	*mant_out = mant;
}

static void APCM_quantization (
	gsmword		* xM,		/* [0..12]		IN	*/

	gsmword		* xMc,		/* [0..12]		OUT	*/
	gsmword		* mant_out,	/* 			OUT	*/
	gsmword		* exp_out,	/*			OUT	*/
	gsmword		* xmaxc_out	/*			OUT	*/
)
{
	int	i, itest;

	gsmword	xmax, xmaxc, temp, temp1, temp2;
	gsmword	exp, mant;
    register longword 	ltmp;


	/*  Find the maximum absolute value xmax of xM[0..12].
	 */

	xmax = 0;
	for (i = 0; i <= 12; i++) {
		temp = xM[i];
		temp = (gsmword)GSM_ABS(temp);
		if (temp > xmax) xmax = temp;
	}

	/*  Qantizing and coding of xmax to get xmaxc.
	 */

	exp   = 0;
	temp  = (gsmword)SASR( xmax, 9 );
	itest = 0;

	for (i = 0; i <= 5; i++) {

		itest |= (temp <= 0);
		temp = (gsmword)SASR( temp, 1 );

		if (itest == 0) exp++;		/* exp = add (exp, 1) */
	}

	temp = (gsmword)(exp + 5);

	xmaxc = (gsmword)GSM_ADD( (gsmword)SASR(xmax, temp), (gsmword)SASL(exp, 3) );

	/*   Quantizing and coding of the xM[0..12] RPE sequence
	 *   to get the xMc[0..12]
	 */

	APCM_quantization_xmaxc_to_exp_mant( xmaxc, &exp, &mant );

	/*  This computation uses the fact that the decoded version of xmaxc
	 *  can be calculated by using the exponent and the mantissa part of
	 *  xmaxc (logarithmic table).
	 *  So, this method avoids any division and uses only a scaling
	 *  of the RPE samples by a function of the exponent.  A direct 
	 *  multiplication by the inverse of the mantissa (NRFAC[0..7]
	 *  found in table 4.5) gives the 3 bit coded version xMc[0..12]
	 *  of the RPE samples.
	 */


	/* Direct computation of xMc[0..12] using table 4.5
	 */

	temp1 = (gsmword)(6 - exp);		/* normalization by the exponent */
	temp2 = gsm_NRFAC[ mant ];  	/* inverse mantissa 		 */

	for (i = 0; i <= 12; i++) {

		temp = (gsmword)SASL(xM[i], temp1);
		temp = (gsmword)GSM_MULT( temp, temp2 );
		temp = (gsmword)SASR(temp, 12);
		xMc[i] = (gsmword)(temp + 4);		/* see note below */
	}

	/*  NOTE: This equation is used to make all the xMc[i] positive.
	 */

	*mant_out  = mant;
	*exp_out   = exp;
	*xmaxc_out = xmaxc;
}

/* 4.2.16 */

static void APCM_inverse_quantization (
	register gsmword	* xMc,	/* [0..12]			IN 	*/
	gsmword		mant,
	gsmword		exp,
	register gsmword	* xMp)	/* [0..12]			OUT 	*/
/* 
 *  This part is for decoding the RPE sequence of coded xMc[0..12]
 *  samples to obtain the xMp[0..12] array.  Table 4.6 is used to get
 *  the mantissa of xmaxc (FAC[0..7]).
 */
{
	int	i;
	gsmword	temp, temp1, temp2, temp3;
	longword	ltmp;

	temp1 = gsm_FAC[ mant ];	/* see 4.2-15 for mant */
	temp2 = (gsmword)GSM_SUB( 6, exp );	/* see 4.2-15 for exp  */
	temp3 = (gsmword)SASL( 1, GSM_SUB( temp2, 1 ));

	for (i = 13; i--;) {

		temp = (gsmword)(SASL(*xMc++, 1) - 7);	        /* restore sign   */

		temp = (gsmword)SASL(temp, 12);				/* 16 bit signed  */
		temp = (gsmword)GSM_MULT_R( temp1, temp );
		temp = (gsmword)GSM_ADD( temp, temp3 );
		*xMp++ = (gsmword)SASR( temp, temp2 );
	}
}

/* 4.2.17 */

static void RPE_grid_positioning (
	gsmword		Mc,		/* grid position	IN	*/
	register gsmword	* xMp,		/* [0..12]		IN	*/
	register gsmword	* ep		/* [0..39]		OUT	*/
)
/*
 *  This procedure computes the reconstructed long term residual signal
 *  ep[0..39] for the LTP analysis filter.  The inputs are the Mc
 *  which is the grid position selection and the xMp[0..12] decoded
 *  RPE samples which are upsampled by a factor of 3 by inserting zero
 *  values.
 */
{
	int	i = 13;

        switch (Mc) {
                case 3: *ep++ = 0;
                case 2:  do {
                                *ep++ = 0;
                case 1:         *ep++ = 0;
                case 0:         *ep++ = *xMp++;
                         } while (--i != 0);
        }
        while (++Mc < 4) *ep++ = 0;
}

/* 4.2.18 */

/*  This procedure adds the reconstructed long term residual signal
 *  ep[0..39] to the estimated signal dpp[0..39] from the long term
 *  analysis filter to compute the reconstructed short term residual
 *  signal dp[-40..-1]; also the reconstructed short term residual
 *  array dp[-120..-41] is updated.
 */

void Gsm_RPE_Encoding (

	gsmword	* e,		/* -5..-1][0..39][40..44	IN/OUT  */
	gsmword	* xmaxc,	/* 				OUT */
	gsmword	* Mc,		/* 			  	OUT */
	gsmword	* xMc)		/* [0..12]			OUT */
{
	gsmword	x[40];
	gsmword	xM[13], xMp[13];
	gsmword	mant, exp;

	Weighting_filter(e, x);
	RPE_grid_selection(x, xM, Mc);

	APCM_quantization(	xM, xMc, &mant, &exp, xmaxc);
	APCM_inverse_quantization(  xMc,  mant,  exp, xMp);

	RPE_grid_positioning( *Mc, xMp, e );

}

void Gsm_RPE_Decoding (

	gsmword 		xmaxcr,
	gsmword		Mcr,
	gsmword		* xMcr,  /* [0..12], 3 bits 		IN	*/
	gsmword		* erp	 /* [0..39]			OUT 	*/
)
{
	gsmword	exp, mant;
	gsmword	xMp[ 13 ];

	APCM_quantization_xmaxc_to_exp_mant( xmaxcr, &exp, &mant );
	APCM_inverse_quantization( xMcr, mant, exp, xMp );
	RPE_grid_positioning( Mcr, xMp, erp );

}
