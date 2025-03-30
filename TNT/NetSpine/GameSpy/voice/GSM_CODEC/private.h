/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/*$Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/private.h,v 1.6 1996/07/02 10:15:26 jutta Exp $*/

#ifndef	PRIVATE_H
#define	PRIVATE_H

#ifdef __GNUC__
  #define INLINE __inline__
#else
  #ifdef _MSC_VER
    #define INLINE __inline
    #pragma warning (disable:4514)
    #pragma warning (disable:4711)
  #else 
    #define INLINE
  #endif
#endif

#ifndef __mips64
	#include <memory.h>
#else
	#include <string.h>
#endif 
#include "gsm.h"

typedef long			gsmword;	/* 16 or 32 bit signed int	*/
typedef long			longword;	/* 32 bit signed int	*/


struct gsm_state {

	gsmword		dp0[ 280 ];

	gsmword		z1;		/* preprocessing.c, Offset_com. */
	longword	L_z2;	/*                  Offset_com. */
	int		    mp;		/*                  Preemphasis	*/

	gsmword		u[8];		/* short_term_aly_filter.c	*/
	gsmword		LARpp[2][8];/*                          */
	gsmword		j;		    /*                          */

	gsmword     ltp_cut;        /* long_term.c, LTP crosscorr.  */
	gsmword		nrp; /* 40 */	/* long_term.c, synthesis	*/
	gsmword		v[9];		/* short_term.c, synthesis	*/
	gsmword		msr;		/* decoder.c,	Postprocessing	*/

	char		verbose;	/* only used if !NDEBUG		*/
	char		fast;		/* only used if FAST		*/

	char		wav_fmt;	/* only used if WAV49 defined	*/
	unsigned char	frame_index;	/*            odd/even chaining	*/
	unsigned char	frame_chain;	/*   half-byte to carry forward	*/
};


#define	MIN_WORD	(-32767 - 1)
#define	MAX_WORD	  32767

#define	SASR(x, by)	((x) >> (by))
#define SASL(x, by)	((x) << (by))

/*
 *	Prototypes from add.c
 */
extern gsmword	gsm_norm ( longword a );

/*
 *  Inlined functions from add.h 
 */

/*
# define GSM_MULT_R(a, b)	\
	(SASR( ((longword)(a) * (longword)(b) + 16384), 15 ))
*/
# define GSM_MULT_R(a, b)  \
	(SASR( ((longword)(a) * (longword)(b)), 15 ))

# define GSM_MULT(a,b)	 \
	(SASR( ((longword)(a) * (longword)(b)), 15 ))

# define GSM_ADD(a, b)	\
	(gsmword)((ltmp = (longword)(a) + (longword)(b)) >= MAX_WORD \
	? MAX_WORD : ltmp <= MIN_WORD ? MIN_WORD : ltmp)


# define GSM_SUB(a, b)	\
	(gsmword)((ltmp = (longword)(a) - (longword)(b)) >= MAX_WORD \
	? MAX_WORD : ltmp <= MIN_WORD ? MIN_WORD : ltmp)

# define GSM_ABS(a)	(gsmword)((a) < 0 ?  -(a) : (a))

extern gsmword gsm_div (gsmword num, gsmword denum);

/*
 *  More prototypes from implementations..
 */
extern void Gsm_Long_Term_Predictor (		/* 4x for 160 samples */
		struct gsm_state	* S,
		gsmword	* d,	/* [0..39]   residual signal	IN	*/
		gsmword	* dp,	/* [-120..-1] d'		IN	*/
		gsmword	* e,	/* [0..40] 			OUT	*/
		gsmword	* dpp,	/* [0..40] 			OUT	*/
		gsmword	* Nc,	/* correlation lag		OUT	*/
		gsmword	* bc	/* gain factor			OUT	*/);

extern void Gsm_LPC_Analysis (
		gsmword * s,	 /* 0..159 signals	IN/OUT	*/
	        gsmword * LARc);   /* 0..7   LARc's	OUT	*/

extern void Gsm_Short_Term_Analysis_Filter (
		struct gsm_state * S,
		gsmword	* LARc,	/* coded log area ratio [0..7]  IN	*/
		gsmword	* d	/* st res. signal [0..159]	IN/OUT	*/);

extern void Gsm_Long_Term_Synthesis_Filtering (
		struct gsm_state* S,
		gsmword	Ncr,
		gsmword	bcr,
		gsmword	* erp,		/* [0..39]		  IN 	*/
		gsmword	* drp); 	/* [-120..-1] IN, [0..40] OUT 	*/

void Gsm_RPE_Decoding (
		gsmword xmaxcr,
		gsmword Mcr,
		gsmword * xMcr,  /* [0..12], 3 bits             IN      */
		gsmword * erp); /* [0..39]                     OUT     */

void Gsm_RPE_Encoding (
		gsmword    * e,            /* -5..-1][0..39][40..44     IN/OUT  */
		gsmword    * xmaxc,        /*                              OUT */
		gsmword    * Mc,           /*                              OUT */
		gsmword    * xMc);        /* [0..12]                      OUT */

extern void Gsm_Short_Term_Synthesis_Filter (
		struct gsm_state * S,
		gsmword	* LARcr, 	/* log area ratios [0..7]  IN	*/
		gsmword	* drp,		/* received d [0...39]	   IN	*/
		gsm_signal	* s);		/* signal   s [0..159]	  OUT	*/

extern void Gsm_Update_of_reconstructed_short_time_residual_signal (
		gsmword	* dpp,		/* [0...39]	IN	*/
		gsmword	* ep,		/* [0...39]	IN	*/
		gsmword	* dp);		/* [-120...-1]  IN/OUT 	*/


#endif	/* PRIVATE_H */
