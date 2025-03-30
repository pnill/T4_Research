/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/gsm_option.c,v 1.3 1996/07/02 09:59:05 jutta Exp $ */

#include "private.h"

int gsm_option ( gsm r, int opt, int * val)
{
	int 	result = -1;

	switch (opt) {
	case GSM_OPT_LTP_CUT:
		result = r->ltp_cut;
		if (val) r->ltp_cut = (gsmword)*val;
		break;

	default:
		break;
	}
	return result;
}
