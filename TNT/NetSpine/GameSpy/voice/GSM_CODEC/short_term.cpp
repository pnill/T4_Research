/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/short_term.c,v 1.2 1994/05/10 20:18:47 jutta Exp $ */

#include "private.h"

/*
 *  SHORT TERM ANALYSIS FILTERING SECTION
 */

/* 4.2.8 */

static void Decoding_of_the_coded_Log_Area_Ratios (
	gsmword 	* LARc,		/* coded log area ratio	[0..7] 	IN	*/
	gsmword	* LARpp)	/* out: decoded ..			*/
{
	register gsmword	temp1 /* , temp2 */;
	register long	ltmp;	/* for GSM_ADD */

	/*  This procedure requires for efficient implementation
	 *  two tables.
 	 *
	 *  INVA[1..8] = integer( (32768 * 8) / real_A[1..8])
	 *  MIC[1..8]  = minimum value of the LARc[1..8]
	 */

	/*  Compute the LARpp[1..8]
	 */

	/* 	for (i = 1; i <= 8; i++, B++, MIC++, INVA++, LARc++, LARpp++) {
	 *
	 *		temp1  = GSM_ADD( *LARc, *MIC ) << 10;
	 *		temp2  = *B << 1;
	 *		temp1  = GSM_SUB( temp1, temp2 );
	 *
	 *		assert(*INVA != MIN_WORD);
	 *
	 *		temp1  = GSM_MULT_R( *INVA, temp1 );
	 *		*LARpp = GSM_ADD( temp1, temp1 );
	 *	}
	 */

#undef	STEP
#define	STEP( B, MIC, INVA )	\
		temp1    = (gsmword)(SASL( *LARc++ + MIC , 10));	\
		temp1    -= SASL( B, 1 );		\
		temp1    = (gsmword)GSM_MULT_R( INVA, temp1 );		\
		*LARpp++ = (gsmword)GSM_ADD( temp1, temp1 );

	STEP(      0,  -32,  13107 );
	STEP(      0,  -32,  13107 );
	STEP(   2048,  -16,  13107 );
	STEP(  -2560,  -16,  13107 );

	STEP(     94,   -8,  19223 );
	STEP(  -1792,   -8,  17476 );
	STEP(   -341,   -4,  31454 );
	STEP(  -1144,   -4,  29708 );

	/* NOTE: the addition of *MIC is used to restore
	 * 	 the sign of *LARc.
	 */
}

/* 4.2.9 */
/* Computation of the quantized reflection coefficients 
 */

/* 4.2.9.1  Interpolation of the LARpp[1..8] to get the LARp[1..8]
 */

/*
 *  Within each frame of 160 analyzed speech samples the short term
 *  analysis and synthesis filters operate with four different sets of
 *  coefficients, derived from the previous set of decoded LARs(LARpp(j-1))
 *  and the actual set of decoded LARs (LARpp(j))
 *
 * (Initial value: LARpp(j-1)[1..8] = 0.)
 */

static INLINE void Coefficients_0_12 (
	register gsmword * LARpp_j_1,
	register gsmword * LARpp_j,
	register gsmword * LARp)
{
	register int 	i;

	for (i = 1; i <= 8; i++, LARp++, LARpp_j_1++, LARpp_j++) {
		*LARp = (gsmword)(SASR( *LARpp_j_1, 2 ) + SASR( *LARpp_j, 2 ));
		*LARp += (gsmword)SASR( *LARpp_j_1, 1);
	}
}

static INLINE void Coefficients_13_26 (
	register gsmword * LARpp_j_1,
	register gsmword * LARpp_j,
	register gsmword * LARp)
{
	register int i;
	for (i = 1; i <= 8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
		*LARp = SASR( *LARpp_j_1, 1) + SASR( *LARpp_j, 1 );
	}
}

static INLINE void Coefficients_27_39 (
	register gsmword * LARpp_j_1,
	register gsmword * LARpp_j,
	register gsmword * LARp)
{
	register int i;

	for (i = 1; i <= 8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
		*LARp = (gsmword)(SASR( *LARpp_j_1, 2 ) + SASR( *LARpp_j, 2 ));
		*LARp += (gsmword)SASR( *LARpp_j, 1 );
	}
}


static INLINE void Coefficients_40_159 (
	register gsmword * LARpp_j,
	register gsmword * LARp)
{
	register int i;

	for (i = 1; i <= 8; i++, LARp++, LARpp_j++)
		*LARp = *LARpp_j;
}

/* 4.2.9.2 */

static void LARp_to_rp (
	register gsmword * LARp)	/* [0..7] IN/OUT  */
/*
 *  The input of this procedure is the interpolated LARp[0..7] array.
 *  The reflection coefficients, rp[i], are used in the analysis
 *  filter and in the synthesis filter.
 */
{
	register int 		i;
	register gsmword	temp;

	for (i = 1; i <= 8; i++, LARp++) {

		/* temp = GSM_ABS( *LARp );
	         *
		 * if (temp < 11059) temp <<= 1;
		 * else if (temp < 20070) temp += 11059;
		 * else temp = GSM_ADD( temp >> 2, 26112 );
		 *
		 * *LARp = *LARp < 0 ? -temp : temp;
		 */

		if (*LARp < 0) {
			temp = (gsmword)GSM_ABS( *LARp );
			*LARp = (gsmword)(- ((temp < 11059) ? SASL( temp, 1 )
				: ((temp < 20070) ? temp + 11059
				:  ( SASR( temp, 2 ) + 26112 ))));
		} else {
			temp  = *LARp;
			*LARp =    (gsmword)((temp < 11059) ? SASL( temp, 1 )
				: ((temp < 20070) ? temp + 11059
				:  ( SASR( temp, 2 ) + 26112 )));
		}
	}
}

static void Short_term_analysis_filtering (
	struct gsm_state    * S,
	register gsmword	* rp,	/* [0..7]	IN	*/
	register int 	    k_n, 	/*   k_end - k_start	*/
	register gsmword	* s	    /* [0..n-1]	IN/OUT	*/
)
{
    register gsmword	* u = S->u;
    register gsmword	di, ui, sav;
    register longword 	ltmp;
    gsmword             u0=u[0], u1=u[1], u2=u[2], u3=u[3],
                        u4=u[4], u5=u[5], u6=u[6], u7=u[7];
    gsmword             rp0, rp1, rp2, rp3,
                        rp4, rp5, rp6, rp7;

    rp0 = rp[0]; rp1 = rp[1];
    rp2 = rp[2]; rp3 = rp[3];
    rp4 = rp[4]; rp5 = rp[5];
    rp6 = rp[6]; rp7 = rp[7];
    
    while ( k_n-- != 0) {
        
        di = sav = *s;

        /* rp[0] */
        ui  = u0;
        u0  = sav;
        
        sav   = ui + (gsmword)GSM_MULT_R(rp0, di);
        di    += (gsmword)GSM_MULT_R(rp0, ui);
        
        /* rp[1] */
        ui  = u1;
        u1  = sav;
        
        sav   = ui + (gsmword)GSM_MULT_R(rp1, di);
        di    += (gsmword)GSM_MULT_R(rp1, ui);
        
        /* rp[2] */
        ui  = u2;
        u2  = sav;
        
        sav   = ui + (gsmword)GSM_MULT_R(rp2, di);
        di    += (gsmword)GSM_MULT_R(rp2, ui);
        
        /* rp[3] */
        ui  = u3;
        u3  = sav;
        
        sav   = ui + (gsmword)GSM_MULT_R(rp3, di);
        di    += (gsmword)GSM_MULT_R(rp3, ui);
        
        /* rp[4] */
        ui  = u4;
        u4  = sav;
        
        sav   = ui + (gsmword)GSM_MULT_R(rp4, di);
        di    += (gsmword)GSM_MULT_R(rp4, ui);
        
        /* rp[5] */
        ui  = u5;
        u5  = sav;
        
        sav   = ui + (gsmword)GSM_MULT_R(rp5,di);
        di    += (gsmword)GSM_MULT_R(rp5,ui);
        
        /* rp[6] */
        ui  = u6;
        u6  = sav;
        
        sav   = ui + (gsmword)GSM_MULT_R(rp6,di);
        di    += (gsmword)GSM_MULT_R(rp6,ui);
        
        /* rp[7] */
        ui    = u7;
        u7  = sav;
        
        *s++ = (gsmword)GSM_ADD( di, (gsmword)GSM_MULT_R(rp7,ui) ); /* This GSM_ADD is needed for over/under flow */
    }
    u[0]=GSM_ADD(u0, 0);
    u[1]=GSM_ADD(u1, 0);
    u[2]=GSM_ADD(u2, 0);
    u[3]=GSM_ADD(u3, 0);
    u[4]=GSM_ADD(u4, 0);
    u[5]=GSM_ADD(u5, 0);
    u[6]=GSM_ADD(u6, 0);
    u[7]=GSM_ADD(u7, 0);
}

static void Short_term_synthesis_filtering (
	struct gsm_state    * S,
	register gsmword	* rrp,	/* [0..7]	IN	*/
	register int	    k,	    /* k_end - k_start	*/
	register gsmword	* wt,	/* [0..k-1]	IN	*/
	register gsm_signal	* sr	/* [0..k-1]	OUT	*/
)
{
	register gsmword	* v = S->v;
	register gsmword	sri;
	register longword	ltmp;	/* for GSM_ADD  & GSM_SUB */
    gsmword             v0=v[0], v1=v[1], v2=v[2], v3=v[3],
                        v4=v[4], v5=v[5], v6=v[6], v7=v[7];
    gsmword             rrp0, rrp1, rrp2, rrp3,
                        rrp4, rrp5, rrp6, rrp7;

    rrp0 = rrp[0]; rrp1 = rrp[1];
    rrp2 = rrp[2]; rrp3 = rrp[3];
    rrp4 = rrp[4]; rrp5 = rrp[5];
    rrp6 = rrp[6]; rrp7 = rrp[7];
    while (k-- != 0) {
        sri = *wt++;

        /* rrp[7] */
        sri  -= (gsmword)GSM_MULT_R(rrp7, v7);
 
        /* rrp[6] */
        sri  -= (gsmword)GSM_MULT_R(rrp6, v6);
        v7 = v6 + (gsmword)GSM_MULT_R(rrp6, sri);
        
        /* rrp[5] */
        sri  -= (gsmword)GSM_MULT_R(rrp5, v5);
        v6 = v5 + (gsmword)GSM_MULT_R(rrp5, sri);
        
        /* rrp[4] */
        sri  -= (gsmword)GSM_MULT_R(rrp4, v4);
        v5 = v4 + (gsmword)GSM_MULT_R(rrp4, sri);
        
        /* rrp[3] */
        sri  -= (gsmword)GSM_MULT_R(rrp3, v3);
        v4 = v3 + (gsmword)GSM_MULT_R(rrp3, sri);
        
        /* rrp[2] */
        sri  -= (gsmword)GSM_MULT_R(rrp2, v2);
        v3 = v2 + (gsmword)GSM_MULT_R(rrp2, sri);
        
        /* rrp[1] */
        sri  -= (gsmword)GSM_MULT_R(rrp1, v1);
        v2 = v1 + (gsmword)GSM_MULT_R(rrp1, sri);
        
        /* rrp[0] */
        sri  = (gsmword)GSM_SUB( sri, (gsmword)GSM_MULT_R(rrp0, v0) );
        v1 = v0 + (gsmword)GSM_MULT_R(rrp0, sri);

        v0 = sri;
        *sr++ = (gsm_signal)sri;
    }
    v[0]=GSM_ADD(v0, 0);
    v[1]=GSM_ADD(v1, 0);
    v[2]=GSM_ADD(v2, 0);
    v[3]=GSM_ADD(v3, 0);
    v[4]=GSM_ADD(v4, 0);
    v[5]=GSM_ADD(v5, 0);
    v[6]=GSM_ADD(v6, 0);
    v[7]=GSM_ADD(v7, 0);
}

void Gsm_Short_Term_Analysis_Filter (

	struct gsm_state * S,

	gsmword	* LARc,		/* coded log area ratio [0..7]  IN	*/
	gsmword	* s		/* signal [0..159]		IN/OUT	*/
)
{
	gsmword		* LARpp_j	= S->LARpp[ S->j      ];
	gsmword		* LARpp_j_1	= S->LARpp[ S->j ^= 1 ];

	gsmword		LARp[8];

	Decoding_of_the_coded_Log_Area_Ratios( LARc, LARpp_j );

	Coefficients_0_12(  LARpp_j_1, LARpp_j, LARp );
	LARp_to_rp( LARp );
	Short_term_analysis_filtering( S, LARp, 13, s);

	Coefficients_13_26( LARpp_j_1, LARpp_j, LARp);
	LARp_to_rp( LARp );
	Short_term_analysis_filtering( S, LARp, 14, s + 13);

	Coefficients_27_39( LARpp_j_1, LARpp_j, LARp);
	LARp_to_rp( LARp );
	Short_term_analysis_filtering( S, LARp, 13, s + 27);

	Coefficients_40_159( LARpp_j, LARp);
	LARp_to_rp( LARp );
	Short_term_analysis_filtering( S, LARp, 120, s + 40);
}

void Gsm_Short_Term_Synthesis_Filter (
	struct gsm_state * S,

	gsmword	    * LARcr,	/* received log area ratios [0..7] IN  */
	gsmword	    * wt,		/* received d [0..159]		   IN  */

	gsm_signal	* s		    /* signal   s [0..159]		  OUT  */
)
{
	gsmword		* LARpp_j	= S->LARpp[ S->j     ];
	gsmword		* LARpp_j_1	= S->LARpp[ S->j ^=1 ];

	gsmword		LARp[8];

	Decoding_of_the_coded_Log_Area_Ratios( LARcr, LARpp_j );

	Coefficients_0_12( LARpp_j_1, LARpp_j, LARp );
	LARp_to_rp( LARp );
	Short_term_synthesis_filtering( S, LARp, 13, wt, s );

	Coefficients_13_26( LARpp_j_1, LARpp_j, LARp);
	LARp_to_rp( LARp );
	Short_term_synthesis_filtering( S, LARp, 14, wt + 13, s + 13 );

	Coefficients_27_39( LARpp_j_1, LARpp_j, LARp);
	LARp_to_rp( LARp );
	Short_term_synthesis_filtering( S, LARp, 13, wt + 27, s + 27 );

	Coefficients_40_159( LARpp_j, LARp );
	LARp_to_rp( LARp );
	Short_term_synthesis_filtering(S, LARp, 120, wt + 40, s + 40);
}
