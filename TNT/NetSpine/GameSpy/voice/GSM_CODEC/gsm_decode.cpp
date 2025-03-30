/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/gsm_decode.c,v 1.2 1996/07/02 09:59:05 jutta Exp $ */

#include "private.h"

//#ifndef GSM_DIV
//#define GSM_DIV
//INLINE 
gsmword gsm_div (gsmword num, gsmword denum)
{
	longword	L_num   = SASL( num, 15 );
	longword	L_denum = denum;

    if (L_num == 0)
	    return 0;
    return (gsmword)(L_num/L_denum);
}
//#else
//extern gsmword gsm_div (gsmword num, gsmword denum);
//#endif // GSM_DIV


/*
 *  4.3 FIXED POINT IMPLEMENTATION OF THE RPE-LTP DECODER
 */

static INLINE void Postprocessing (
	struct gsm_state	* S,
	register gsm_signal * s)
{
	register int		k;
	register gsmword	msr = S->msr;
	register longword	ltmp;	/* for GSM_ADD */

	for (k = 160; k--; s++) {
		msr = *s + (gsmword)GSM_MULT_R( msr, 28672 );  	   /* Deemphasis 	     */
		*s  = (gsm_signal)(GSM_ADD(msr, msr) & 0xFFF8);  /* Truncation & Upscaling */
	}
	S->msr = msr;
}

static void Gsm_Decoder (
	struct gsm_state	* S,

	gsmword		* LARcr,	/* [0..7]		IN	*/

	gsmword		* Ncr,		/* [0..3] 		IN 	*/
	gsmword		* bcr,		/* [0..3]		IN	*/
	gsmword		* Mcr,		/* [0..3] 		IN 	*/
	gsmword		* xmaxcr,	/* [0..3]		IN 	*/
	gsmword		* xMcr,		/* [0..13*4]		IN	*/

	gsm_signal		* s)		/* [0..159]		OUT 	*/
{
	int		    j;
	gsmword		erp[40], wt[160];
	gsmword		* drp = S->dp0 + 120;

	for (j=0; j <= 3; j++, xmaxcr++, bcr++, Ncr++, Mcr++, xMcr += 13) {

		Gsm_RPE_Decoding( *xmaxcr, *Mcr, xMcr, erp );
		Gsm_Long_Term_Synthesis_Filtering( S, *Ncr, *bcr, erp, drp );

        memcpy(&wt[ j * 40 ], drp, sizeof(drp[0]) * 40);
	}

	Gsm_Short_Term_Synthesis_Filter( S, LARcr, wt, s );
	Postprocessing(S, s);
}

int gsm_decode ( gsm s, gsm_byte * c, gsm_signal * target)
{
	gsmword  	LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13*4];

	{
		/* GSM_MAGIC  = (*c >> 4) & 0xF; */

		if (((*c >> 4) & 0x0F) != GSM_MAGIC) return 0;

		LARc[0]  = (gsmword)((*c++ & 0xF) << 2);			/* 1 */
		LARc[0] |= (gsmword)((*c >> 6) & 0x3);
		LARc[1]  = (gsmword)(*c++ & 0x3F);
		LARc[2]  = (gsmword)((*c >> 3) & 0x1F);
		LARc[3]  = (gsmword)((*c++ & 0x7) << 2);
		LARc[3] |= (gsmword)((*c >> 6) & 0x3);
		LARc[4]  = (gsmword)((*c >> 2) & 0xF);
		LARc[5]  = (gsmword)((*c++ & 0x3) << 2);
		LARc[5] |= (gsmword)((*c >> 6) & 0x3);
		LARc[6]  = (gsmword)((*c >> 3) & 0x7);
		LARc[7]  = (gsmword)(*c++ & 0x7);
		Nc[0]  = (gsmword)((*c >> 1) & 0x7F);
		bc[0]  = (gsmword)((*c++ & 0x1) << 1);
		bc[0] |= (gsmword)((*c >> 7) & 0x1);
		Mc[0]  = (gsmword)((*c >> 5) & 0x3);
		xmaxc[0]  = (gsmword)((*c++ & 0x1F) << 1);
		xmaxc[0] |= (gsmword)((*c >> 7) & 0x1);
		xmc[0]	= (gsmword)((*c >> 4) & 0x7);
		xmc[1]	= (gsmword)((*c >> 1) & 0x7);
		xmc[2]	= (gsmword)((*c++ & 0x1) << 2);
		xmc[2] |= (gsmword)((*c >> 6) & 0x3);
		xmc[3]	= (gsmword)((*c >> 3) & 0x7);
		xmc[4]	= (gsmword)(*c++ & 0x7);
		xmc[5]	= (gsmword)((*c >> 5) & 0x7);
		xmc[6]	= (gsmword)((*c >> 2) & 0x7);
		xmc[7]	= (gsmword)((*c++ & 0x3) << 1);			/* 10 */
		xmc[7] |= (gsmword)((*c >> 7) & 0x1);
		xmc[8]	= (gsmword)((*c >> 4) & 0x7);
		xmc[9]	= (gsmword)((*c >> 1) & 0x7);
		xmc[10]  = (gsmword)((*c++ & 0x1) << 2);
		xmc[10] |= (gsmword)((*c >> 6) & 0x3);
		xmc[11]  = (gsmword)((*c >> 3) & 0x7);
		xmc[12]  = (gsmword)(*c++ & 0x7);
		Nc[1]  = (gsmword)((*c >> 1) & 0x7F);
		bc[1]  = (gsmword)((*c++ & 0x1) << 1);
		bc[1] |= (gsmword)((*c >> 7) & 0x1);
		Mc[1]  = (gsmword)((*c >> 5) & 0x3);
		xmaxc[1]  = (gsmword)((*c++ & 0x1F) << 1);
		xmaxc[1] |= (gsmword)((*c >> 7) & 0x1);
		xmc[13]  = (gsmword)((*c >> 4) & 0x7);
		xmc[14]  = (gsmword)((*c >> 1) & 0x7);
		xmc[15]  = (gsmword)((*c++ & 0x1) << 2);
		xmc[15] |= (gsmword)((*c >> 6) & 0x3);
		xmc[16]  = (gsmword)((*c >> 3) & 0x7);
		xmc[17]  = (gsmword)(*c++ & 0x7);
		xmc[18]  = (gsmword)((*c >> 5) & 0x7);
		xmc[19]  = (gsmword)((*c >> 2) & 0x7);
		xmc[20]  = (gsmword)((*c++ & 0x3) << 1);
		xmc[20] |= (gsmword)((*c >> 7) & 0x1);
		xmc[21]  = (gsmword)((*c >> 4) & 0x7);
		xmc[22]  = (gsmword)((*c >> 1) & 0x7);
		xmc[23]  = (gsmword)((*c++ & 0x1) << 2);
		xmc[23] |= (gsmword)((*c >> 6) & 0x3);
		xmc[24]  = (gsmword)((*c >> 3) & 0x7);
		xmc[25]  = (gsmword)(*c++ & 0x7);
		Nc[2]  = (gsmword)((*c >> 1) & 0x7F);
		bc[2]  = (gsmword)((*c++ & 0x1) << 1); 			/* 20 */
		bc[2] |= (gsmword)((*c >> 7) & 0x1);
		Mc[2]  = (gsmword)((*c >> 5) & 0x3);
		xmaxc[2]  = (gsmword)((*c++ & 0x1F) << 1);
		xmaxc[2] |= (gsmword)((*c >> 7) & 0x1);
		xmc[26]  = (gsmword)((*c >> 4) & 0x7);
		xmc[27]  = (gsmword)((*c >> 1) & 0x7);
		xmc[28]  = (gsmword)((*c++ & 0x1) << 2);
		xmc[28] |= (gsmword)((*c >> 6) & 0x3);
		xmc[29]  = (gsmword)((*c >> 3) & 0x7);
		xmc[30]  = (gsmword)(*c++ & 0x7);
		xmc[31]  = (gsmword)((*c >> 5) & 0x7);
		xmc[32]  = (gsmword)((*c >> 2) & 0x7);
		xmc[33]  = (gsmword)((*c++ & 0x3) << 1);
		xmc[33] |= (gsmword)((*c >> 7) & 0x1);
		xmc[34]  = (gsmword)((*c >> 4) & 0x7);
		xmc[35]  = (gsmword)((*c >> 1) & 0x7);
		xmc[36]  = (gsmword)((*c++ & 0x1) << 2);
		xmc[36] |= (gsmword)((*c >> 6) & 0x3);
		xmc[37]  = (gsmword)((*c >> 3) & 0x7);
		xmc[38]  = (gsmword)(*c++ & 0x7);
		Nc[3]  = (gsmword)((*c >> 1) & 0x7F);
		bc[3]  = (gsmword)((*c++ & 0x1) << 1);
		bc[3] |= (gsmword)((*c >> 7) & 0x1);
		Mc[3]  = (gsmword)((*c >> 5) & 0x3);
		xmaxc[3]  = (gsmword)((*c++ & 0x1F) << 1);
		xmaxc[3] |= (gsmword)((*c >> 7) & 0x1);
		xmc[39]  = (gsmword)((*c >> 4) & 0x7);
		xmc[40]  = (gsmword)((*c >> 1) & 0x7);
		xmc[41]  = (gsmword)((*c++ & 0x1) << 2);
		xmc[41] |= (gsmword)((*c >> 6) & 0x3);
		xmc[42]  = (gsmword)((*c >> 3) & 0x7);
		xmc[43]  = (gsmword)(*c++ & 0x7);					/* 30  */
		xmc[44]  = (gsmword)((*c >> 5) & 0x7);
		xmc[45]  = (gsmword)((*c >> 2) & 0x7);
		xmc[46]  = (gsmword)((*c++ & 0x3) << 1);
		xmc[46] |= (gsmword)((*c >> 7) & 0x1);
		xmc[47]  = (gsmword)((*c >> 4) & 0x7);
		xmc[48]  = (gsmword)((*c >> 1) & 0x7);
		xmc[49]  = (gsmword)((*c++ & 0x1) << 2);
		xmc[49] |= (gsmword)((*c >> 6) & 0x3);
		xmc[50]  = (gsmword)((*c >> 3) & 0x7);
		xmc[51]  = (gsmword)(*c & 0x7);					/* 33 */
	}

	Gsm_Decoder(s, LARc, Nc, bc, Mc, xmaxc, xmc, target);

	return 160;
}
