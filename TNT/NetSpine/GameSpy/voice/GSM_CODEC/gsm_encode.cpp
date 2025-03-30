/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/gsm_encode.c,v 1.2 1996/07/02 09:59:05 jutta Exp $ */

#include "private.h"

/*	4.2.0 .. 4.2.3	PREPROCESSING SECTION
 *  
 *  	After A-law to linear conversion (or directly from the
 *   	Ato D converter) the following scaling is assumed for
 * 	input to the RPE-LTP algorithm:
 *
 *      in:  0.1.....................12
 *	     S.v.v.v.v.v.v.v.v.v.v.v.v.*.*.*
 *
 *	Where S is the sign bit, v a valid bit, and * a "don't care" bit.
 * 	The original signal is called sop[..]
 *
 *      out:   0.1................... 12 
 *	     S.S.v.v.v.v.v.v.v.v.v.v.v.v.0.0
 */


static void Gsm_Preprocess (
	struct gsm_state * S,
	gsm_signal		 * s,
	gsmword 		 * so )		/* [0..159] 	IN/OUT	*/
{

	gsmword     z1 = S->z1;
	longword    L_z2 = S->L_z2;
	gsmword 	mp = (gsmword)S->mp;

	gsmword 	s1;
	longword    L_s2;

	longword    L_temp;

	gsmword		msp;
	gsmword		SO;

	register int		k = 160;

	while (k--) {

	/*  4.2.1   Downscaling of the input signal
	 */
		SO = (gsmword)(SASL( SASR( *s, 3 ), 2 ));
		s++;

	/*  4.2.2   Offset compensation
	 * 
	 *  This part implements a high-pass filter and requires extended
	 *  arithmetic precision for the recursive part of this filter.
	 *  The input of this procedure is the array so[0...159] and the
	 *  output the array sof[ 0...159 ].
	 */
		/*   Compute the non-recursive part
		 */

		s1 = (gsmword)(SO - z1);
		z1 = SO;

		/*   Compute the recursive part
		 */
		L_s2 = s1;
		L_s2 = SASL( L_s2, 15 );

		/*   Execution of a 31 bv 16 bits multiplication
		 */

/*		msp = (gsmword)SASR( L_z2, 15 );
		lsp = (gsmword)(L_z2 - SASL( msp, 15 ));

		L_s2  += GSM_MULT_R( lsp, 32768 );
		L_z2   = (longword)msp * 32768 + L_s2;
*/
        L_z2 += L_s2;
		/*    Compute sof[k] with rounding
		 */
		L_temp = L_z2 + 16384; /* GSM_L_ADD( L_z2, 16384 ); */

	/*   4.2.3  Preemphasis
	 */

		msp   = (gsmword)GSM_MULT_R( mp, -28672 );
		mp    = (gsmword)SASR( L_temp, 15 );
		*so++ = (gsmword)(mp + msp);
	}

	S->z1   = z1;
	S->L_z2 = L_z2;
	S->mp   = mp;
}

/* 
 *  4.2 FIXED POINT IMPLEMENTATION OF THE RPE-LTP CODER 
 */

static void Gsm_Coder (

	struct gsm_state	* S,

	gsm_signal	* s,	/* [0..159] samples		  	IN	*/

/*
 * The RPE-LTD coder works on a frame by frame basis.  The length of
 * the frame is equal to 160 samples.  Some computations are done
 * once per frame to produce at the output of the coder the
 * LARc[1..8] parameters which are the coded LAR coefficients and 
 * also to realize the inverse filtering operation for the entire
 * frame (160 samples of signal d[0..159]).  These parts produce at
 * the output of the coder:
 */

	gsmword	* LARc,	/* [0..7] LAR coefficients		OUT	*/

/*
 * Procedure 4.2.11 to 4.2.18 are to be executed four times per
 * frame.  That means once for each sub-segment RPE-LTP analysis of
 * 40 samples.  These parts produce at the output of the coder:
 */

	gsmword	* Nc,	/* [0..3] LTP lag			OUT 	*/
	gsmword	* bc,	/* [0..3] coded LTP gain		OUT 	*/
	gsmword	* Mc,	/* [0..3] RPE grid selection		OUT     */
	gsmword	* xmaxc,/* [0..3] Coded maximum amplitude	OUT	*/
	gsmword	* xMc	/* [13*4] normalized RPE samples	OUT	*/
)
{
	int	k;
	gsmword	* dp  = S->dp0 + 120;	/* [ -120...-1 ] */
	gsmword e[50];
    gsmword         * pe = &e[5];
	gsmword	so[160];

    memset(e, 0, sizeof(e));
	Gsm_Preprocess			(S, s, so);
	Gsm_LPC_Analysis		(so, LARc);
	Gsm_Short_Term_Analysis_Filter	(S, LARc, so);

	for (k = 0; k <= 3; k++, xMc += 13) {
        register int i;

		Gsm_Long_Term_Predictor	(S,
                    so+k*40, /* d      [0..39] IN	*/
					dp,	  /* dp  [-120..-1] IN	*/
					pe,	  /* e      [0..39] OUT	*/
					dp,	  /* dpp    [0..39] OUT */
					Nc++,
					bc++);

		Gsm_RPE_Encoding (pe, xmaxc++, Mc++, xMc );
		/*
		 * Gsm_Update_of_reconstructed_short_time_residual_signal
		 *			( dpp, e + 5, dp );
		 */

		for (i = 0; i <= 39; i++, dp++)
			*dp +=  (gsmword)pe[i];
	}
	(void)memcpy( (char *)S->dp0, (char *)(S->dp0 + 160),
		120 * sizeof(*S->dp0) );
}

int gsm_encode ( gsm s, gsm_signal * source, gsm_byte * c)
{
	gsmword	 	LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13*4];

	Gsm_Coder(s, source, LARc, Nc, bc, Mc, xmaxc, xmc);


	/*	variable	size

		GSM_MAGIC	4

		LARc[0]		6
		LARc[1]		6
		LARc[2]		5
		LARc[3]		5
		LARc[4]		4
		LARc[5]		4
		LARc[6]		3
		LARc[7]		3

		Nc[0]		7
		bc[0]		2
		Mc[0]		2
		xmaxc[0]	6
		xmc[0]		3
		xmc[1]		3
		xmc[2]		3
		xmc[3]		3
		xmc[4]		3
		xmc[5]		3
		xmc[6]		3
		xmc[7]		3
		xmc[8]		3
		xmc[9]		3
		xmc[10]		3
		xmc[11]		3
		xmc[12]		3

		Nc[1]		7
		bc[1]		2
		Mc[1]		2
		xmaxc[1]	6
		xmc[13]		3
		xmc[14]		3
		xmc[15]		3
		xmc[16]		3
		xmc[17]		3
		xmc[18]		3
		xmc[19]		3
		xmc[20]		3
		xmc[21]		3
		xmc[22]		3
		xmc[23]		3
		xmc[24]		3
		xmc[25]		3

		Nc[2]		7
		bc[2]		2
		Mc[2]		2
		xmaxc[2]	6
		xmc[26]		3
		xmc[27]		3
		xmc[28]		3
		xmc[29]		3
		xmc[30]		3
		xmc[31]		3
		xmc[32]		3
		xmc[33]		3
		xmc[34]		3
		xmc[35]		3
		xmc[36]		3
		xmc[37]		3
		xmc[38]		3

		Nc[3]		7
		bc[3]		2
		Mc[3]		2
		xmaxc[3]	6
		xmc[39]		3
		xmc[40]		3
		xmc[41]		3
		xmc[42]		3
		xmc[43]		3
		xmc[44]		3
		xmc[45]		3
		xmc[46]		3
		xmc[47]		3
		xmc[48]		3
		xmc[49]		3
		xmc[50]		3
		xmc[51]		3
	*/

	{

		*c++ =	 (gsm_byte)(((GSM_MAGIC & 0xF) << 4)				/* 1 */
			   | ((LARc[0] >> 2) & 0xF));
		*c++ =	 (gsm_byte)(((LARc[0] & 0x3) << 6)
			   | (LARc[1] & 0x3F));
		*c++ =	 (gsm_byte)(((LARc[2] & 0x1F) << 3)
			   | ((LARc[3] >> 2) & 0x7));
		*c++ =	 (gsm_byte)(((LARc[3] & 0x3) << 6)
			   | ((LARc[4] & 0xF) << 2)
			   | ((LARc[5] >> 2) & 0x3));
		*c++ =	 (gsm_byte)(((LARc[5] & 0x3) << 6)
			   | ((LARc[6] & 0x7) << 3)
			   | (LARc[7] & 0x7));
		*c++ =	 (gsm_byte)(((Nc[0] & 0x7F) << 1)
			   | ((bc[0] >> 1) & 0x1));
		*c++ =	 (gsm_byte)(((bc[0] & 0x1) << 7)
			   | ((Mc[0] & 0x3) << 5)
			   | ((xmaxc[0] >> 1) & 0x1F));
		*c++ =	 (gsm_byte)(((xmaxc[0] & 0x1) << 7)
			   | ((xmc[0] & 0x7) << 4)
			   | ((xmc[1] & 0x7) << 1)
			   | ((xmc[2] >> 2) & 0x1));
		*c++ =	 (gsm_byte)(((xmc[2] & 0x3) << 6)
			   | ((xmc[3] & 0x7) << 3)
			   | (xmc[4] & 0x7));
		*c++ =	 (gsm_byte)(((xmc[5] & 0x7) << 5)					/* 10 */
			   | ((xmc[6] & 0x7) << 2)
			   | ((xmc[7] >> 1) & 0x3));
		*c++ =	 (gsm_byte)(((xmc[7] & 0x1) << 7)
			   | ((xmc[8] & 0x7) << 4)
			   | ((xmc[9] & 0x7) << 1)
			   | ((xmc[10] >> 2) & 0x1));
		*c++ =	 (gsm_byte)(((xmc[10] & 0x3) << 6)
			   | ((xmc[11] & 0x7) << 3)
			   | (xmc[12] & 0x7));
		*c++ =	 (gsm_byte)(((Nc[1] & 0x7F) << 1)
			   | ((bc[1] >> 1) & 0x1));
		*c++ =	 (gsm_byte)(((bc[1] & 0x1) << 7)
			   | ((Mc[1] & 0x3) << 5)
			   | ((xmaxc[1] >> 1) & 0x1F));
		*c++ =	 (gsm_byte)(((xmaxc[1] & 0x1) << 7)
			   | ((xmc[13] & 0x7) << 4)
			   | ((xmc[14] & 0x7) << 1)
			   | ((xmc[15] >> 2) & 0x1));
		*c++ =	 (gsm_byte)(((xmc[15] & 0x3) << 6)
			   | ((xmc[16] & 0x7) << 3)
			   | (xmc[17] & 0x7));
		*c++ =	 (gsm_byte)(((xmc[18] & 0x7) << 5)
			   | ((xmc[19] & 0x7) << 2)
			   | ((xmc[20] >> 1) & 0x3));
		*c++ =	 (gsm_byte)(((xmc[20] & 0x1) << 7)
			   | ((xmc[21] & 0x7) << 4)
			   | ((xmc[22] & 0x7) << 1)
			   | ((xmc[23] >> 2) & 0x1));
		*c++ =	 (gsm_byte)(((xmc[23] & 0x3) << 6)
			   | ((xmc[24] & 0x7) << 3)
			   | (xmc[25] & 0x7));
		*c++ =	 (gsm_byte)(((Nc[2] & 0x7F) << 1)					/* 20 */
			   | ((bc[2] >> 1) & 0x1));
		*c++ =	 (gsm_byte)(((bc[2] & 0x1) << 7)
			   | ((Mc[2] & 0x3) << 5)
			   | ((xmaxc[2] >> 1) & 0x1F));
		*c++ =	 (gsm_byte)(((xmaxc[2] & 0x1) << 7)
			   | ((xmc[26] & 0x7) << 4)
			   | ((xmc[27] & 0x7) << 1)
			   | ((xmc[28] >> 2) & 0x1));
		*c++ =	 (gsm_byte)(((xmc[28] & 0x3) << 6)
			   | ((xmc[29] & 0x7) << 3)
			   | (xmc[30] & 0x7));
		*c++ =	 (gsm_byte)(((xmc[31] & 0x7) << 5)
			   | ((xmc[32] & 0x7) << 2)
			   | ((xmc[33] >> 1) & 0x3));
		*c++ =	 (gsm_byte)(((xmc[33] & 0x1) << 7)
			   | ((xmc[34] & 0x7) << 4)
			   | ((xmc[35] & 0x7) << 1)
			   | ((xmc[36] >> 2) & 0x1));
		*c++ =	 (gsm_byte)(((xmc[36] & 0x3) << 6)
			   | ((xmc[37] & 0x7) << 3)
			   | (xmc[38] & 0x7));
		*c++ =	 (gsm_byte)(((Nc[3] & 0x7F) << 1)
			   | ((bc[3] >> 1) & 0x1));
		*c++ =	 (gsm_byte)(((bc[3] & 0x1) << 7)
			   | ((Mc[3] & 0x3) << 5)
			   | ((xmaxc[3] >> 1) & 0x1F));
		*c++ =	 (gsm_byte)(((xmaxc[3] & 0x1) << 7)
			   | ((xmc[39] & 0x7) << 4)
			   | ((xmc[40] & 0x7) << 1)
			   | ((xmc[41] >> 2) & 0x1));
		*c++ =	 (gsm_byte)(((xmc[41] & 0x3) << 6) 				/* 30 */
			   | ((xmc[42] & 0x7) << 3)
			   | (xmc[43] & 0x7));
		*c++ =	 (gsm_byte)(((xmc[44] & 0x7) << 5)
			   | ((xmc[45] & 0x7) << 2)
			   | ((xmc[46] >> 1) & 0x3));
		*c++ =	 (gsm_byte)(((xmc[46] & 0x1) << 7)
			   | ((xmc[47] & 0x7) << 4)
			   | ((xmc[48] & 0x7) << 1)
			   | ((xmc[49] >> 2) & 0x1));
		*c++ =	 (gsm_byte)(((xmc[49] & 0x3) << 6)
			   | ((xmc[50] & 0x7) << 3)
			   | (xmc[51] & 0x7));

	}
    return (int)sizeof(gsm_frame);
}
