/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

#include <malloc.h>


#include "private.h"

gsm gsm_create ()
{
	gsm  r;

	r = (gsm)gsimalloc(sizeof(struct gsm_state));
	if (!r) return r;

	memset((char *)r, 0, sizeof(*r));
	r->nrp = 40;

	return r;
}
