/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/gsm_destroy.c,v 1.3 1994/11/28 19:52:25 jutta Exp $ */

#include 	<malloc.h>

#include "gsm.h"


void gsm_destroy ( gsm S)
{
	if (S) gsifree((char *)S);
}
