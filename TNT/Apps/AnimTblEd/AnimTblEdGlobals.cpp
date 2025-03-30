#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimTblEdDoc.h"

int CurrentExportTransIndex;
int CurrentExportCount;
int CurrentExportType;
int LastTokenType;

int GLB_NumAnims;

CAnimTblEdDoc	*GBL_pGlobalDoc;

void GBL_SetModifiedFlag(int Set)
{
	GBL_pGlobalDoc->SetModifiedFlag( Set );
} /* GBL_SetModifiedFlag */

void UpdateScrollBar(CScrollBar *pScrollBar, int nSBCode)
{
	int Min, Max, i;

	//---	get the range of the scroll bar
	pScrollBar->GetScrollRange(&Min, &Max);

	//---	check for the up arrow to have been hit
	if (nSBCode == 0)
	{
		if (pScrollBar->GetScrollPos() > 0)
			pScrollBar->SetScrollPos(pScrollBar->GetScrollPos() - 1);
		return;
	}

	//---	check for down arrow to have been hit
	if (nSBCode == 1)
	{
		if (pScrollBar->GetScrollPos() < Max)
			pScrollBar->SetScrollPos(pScrollBar->GetScrollPos() + 1);
		return;
	}

	//---	check for page up to have been hit (scroll bar above the nob)
	if (nSBCode == 2)
	{
		if (pScrollBar->GetScrollPos() > 0)
		{
			i = pScrollBar->GetScrollPos();
			i -= 8;
			if (i < 0)
				i = 0;
			pScrollBar->SetScrollPos(i);
		}
		return;
	}

	//---	check for page down to have been hit (scroll bar below the nob)
	if (nSBCode == 3)
	{
		if (pScrollBar->GetScrollPos() < Max)
		{
			i = pScrollBar->GetScrollPos();
			i += 8;
			if (i > Max)
				i = Max;
			pScrollBar->SetScrollPos(i);
		}
		return;
	}

	//---	check for the nob to have been moved
	if (nSBCode == 5)
	{
		SCROLLINFO inf;

		inf.fMask = SIF_POS;
		pScrollBar->GetScrollInfo(&inf);
		if (inf.nTrackPos != pScrollBar->GetScrollPos())
			pScrollBar->SetScrollPos(inf.nTrackPos);
	}
}