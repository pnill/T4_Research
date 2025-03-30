/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/*$Header: /home/kbs/jutta/src/gsm/gsm-1.0/inc/RCS/gsm.h,v 1.11 1996/07/05 18:02:56 jutta Exp $*/

#ifndef	GSM_H
#define	GSM_H

#ifdef __cplusplus
extern "C" {
#endif

#define GSI_MEM_ONLY
#include "../../nonport.h"

/*
 *	Interface
 */

typedef struct gsm_state * 	gsm;
typedef short		   	gsm_signal;		/* signed 16 bit */
typedef unsigned char		gsm_byte;
typedef gsm_byte 		gsm_frame[33];		/* 33 * 8 bits	 */

#define	GSM_MAGIC		0xD		  	/* 13 kbit/s RPE-LTP */

#define	GSM_OPT_LTP_CUT		3

gsm  gsm_create(void);
void gsm_destroy(gsm);	

int  gsm_option(gsm, int, int *);

int  gsm_encode(gsm, gsm_signal *, gsm_byte  *);
int  gsm_decode(gsm, gsm_byte   *, gsm_signal *);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif	/* GSM_H */
