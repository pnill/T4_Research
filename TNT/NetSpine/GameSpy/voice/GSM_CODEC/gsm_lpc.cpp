/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/lpc.c,v 1.5 1994/12/30 23:14:54 jutta Exp $ */

#include "private.h"


/*
 *  4.2.4 .. 4.2.7 LPC ANALYSIS SECTION
 */

/* 4.2.4 */


static void Autocorrelation (
	gsmword     * s,		/* [0..159]	IN/OUT  */
 	longword * L_ACF)	/* [0..8]	OUT     */
/*
 *  The goal is to compute the array L_ACF[k].  The signal s[i] must
 *  be scaled in order to avoid an overflow situation.
 */
{
	register int	k, i;
	gsmword         * sp = s;
	gsmword         sl;
	gsmword		    temp, smax, scalauto;
    gsmword         ss[160];
    gsmword         * ssp = ss;

	/*  Dynamic scaling of the array  s[0..159]
	 */

	/*  Search for the maximum.
	 */
	smax = 0;
	for (k = 160; k--; sp++) {
		temp = (gsmword)GSM_ABS( *sp );
		if (temp > smax) smax = temp;
	}

	/*  Computation of the scaling factor.
	 */
	if (smax == 0) scalauto = 0;
	else {
		scalauto = (gsmword)(4 - gsm_norm( SASL( (longword)smax, 16 ) ));/* sub(4,..) */
	}

	/*  Scaling to the array ss[0...159]
	 */

    sp = s;
	if (scalauto > 0) {
	    for (k = 160; k--; sp++, ssp++) 
			*ssp = (gsmword)SASR( *sp, scalauto );
	}
    else {
        memcpy(ssp, sp, sizeof(ss));
    }

	/*  Compute the L_ACF[..].
	 */

#	define STEP(k)	 L_ACF[k] += ((longword)sl * ssp[ -(k) ]);

#	define NEXTI	 sl = *++ssp

    ssp = ss;
    sl = *ssp;
	for (k = 9; k--; L_ACF[k] = 0) ;

	STEP (0);
	NEXTI;
	STEP(0); STEP(1);
	NEXTI;
	STEP(0); STEP(1); STEP(2);
	NEXTI;
	STEP(0); STEP(1); STEP(2); STEP(3);
	NEXTI;
	STEP(0); STEP(1); STEP(2); STEP(3); STEP(4);
	NEXTI;
	STEP(0); STEP(1); STEP(2); STEP(3); STEP(4); STEP(5);
	NEXTI;
	STEP(0); STEP(1); STEP(2); STEP(3); STEP(4); STEP(5); STEP(6);
	NEXTI;
	STEP(0); STEP(1); STEP(2); STEP(3); STEP(4); STEP(5); STEP(6); STEP(7);

	for (i = 8; i < 160; i++) {

		NEXTI;

		STEP(0);
		STEP(1); STEP(2); STEP(3); STEP(4);
		STEP(5); STEP(6); STEP(7); STEP(8);
	}

	for (k = 9; k--; L_ACF[k] *= 2) ; 

}

/* 4.2.5 */

static void Reflection_coefficients (
	longword	        * L_ACF,	/* 0...8	IN	*/
	register gsmword	* r			/* 0...7	OUT 	*/
)
{
	register int	i, m, n;
	register gsmword	temp;
	gsmword		ACF[9];	/* 0..8 */
	gsmword		P[  9];	/* 0..8 */
	gsmword		K[  9]; /* 2..8 */

	/*  Schur recursion with 16 bits arithmetic.
	 */

	if (L_ACF[0] == 0) {
		for (i = 8; i--; *r++ = 0) ;
		return;
	}

	temp = gsm_norm( L_ACF[0] );

	/* ? overflow ? */
	for (i = 0; i <= 8; i++) ACF[i] = (gsmword)SASR( SASL (L_ACF[i], temp ), 16 );

	/*   Initialize array P[..] and K[..] for the recursion.
	 */

	for (i = 1; i <= 7; i++) K[ i ] = ACF[ i ];
	for (i = 0; i <= 8; i++) P[ i ] = ACF[ i ];

	/*   Compute reflection coefficients
	 */
	for (n = 1; n <= 8; n++, r++) {

		temp = P[1];
		temp = (gsmword)GSM_ABS(temp);
		if (P[0] < temp) {
			for (i = n; i <= 8; i++) *r++ = 0;
			return;
		}

		*r = gsm_div( temp, P[0] );

		if (P[1] > 0) *r = (gsmword)-*r;		/* r[n] = sub(0, r[n]) */
		if (n == 8) return; 

		/*  Schur recursion
		 */
		temp = (gsmword)GSM_MULT_R( P[1], *r );
		P[0] += temp;

		for (m = 1; m <= 8 - n; m++) {
			P[m] = (gsmword)P[ m+1 ] + (gsmword)GSM_MULT_R( K[m], *r );
			K[m] = (gsmword)K[ m ] + (gsmword)GSM_MULT_R( P[ m+1 ], *r );
		}
	}
}

/* 4.2.6 */

static void Transformation_to_Log_Area_Ratios (
	register gsmword	* r 			/* 0..7	   IN/OUT */
)
/*
 *  The following scaling for r[..] and LAR[..] has been used:
 *
 *  r[..]   = integer( real_r[..]*32768. ); -1 <= real_r < 1.
 *  LAR[..] = integer( real_LAR[..] * 16384 );
 *  with -1.625 <= real_LAR <= 1.625
 */
{
	register gsmword	temp;
	register int	i;


	/* Computation of the LAR[0..7] from the r[0..7]
	 */
	for (i = 1; i <= 8; i++, r++) {

		temp = *r;
		temp = (gsmword)GSM_ABS(temp);

		if (temp < 22118) {
			temp = (gsmword)SASR( temp, 1 );
		} else if (temp < 31130) {
			temp -= 11059;
		} else {
			temp -= 26112;
			temp = (gsmword)SASL( temp, 2 );
		}

		*r = (gsmword)(*r < 0 ? -temp : temp);
	}
}

/* 4.2.7 */

static void Quantization_and_coding (
	register gsmword * LAR    	/* [0..7]	IN/OUT	*/
)
{
	register gsmword	temp;


	/*  This procedure needs four tables; the following equations
	 *  give the optimum scaling for the constants:
	 *  
	 *  A[0..7] = integer( real_A[0..7] * 1024 )
	 *  B[0..7] = integer( real_B[0..7] *  512 )
	 *  MAC[0..7] = maximum of the LARc[0..7]
	 *  MIC[0..7] = minimum of the LARc[0..7]
	 */

#	undef STEP
#	define	STEP( A, B, MAC, MIC )		\
		temp = (gsmword)GSM_MULT( A,   *LAR );	\
		temp += B;	\
		temp += 256;	\
		temp = (gsmword)SASR( temp, 9 );	\
		*LAR  =  (gsmword)(temp>MAC ? MAC - MIC : (temp<MIC ? 0 : temp - MIC)); \
		LAR++;

	STEP(  20480,     0,  31, -32 );
	STEP(  20480,     0,  31, -32 );
	STEP(  20480,  2048,  15, -16 );
	STEP(  20480, -2560,  15, -16 );

	STEP(  13964,    94,   7,  -8 );
	STEP(  15360, -1792,   7,  -8 );
	STEP(   8534,  -341,   3,  -4 );
	STEP(   9036, -1144,   3,  -4 );

#	undef	STEP
}

void Gsm_LPC_Analysis (
	gsmword 		 * s,		/* 0..159 signals	IN/OUT	*/
    gsmword 		 * LARc)	/* 0..7   LARc's	OUT	*/
{
	longword	L_ACF[9];

	Autocorrelation			  (s,	  L_ACF	);
	Reflection_coefficients		  (L_ACF, LARc	);
	Transformation_to_Log_Area_Ratios (LARc);
	Quantization_and_coding		  (LARc);
}
