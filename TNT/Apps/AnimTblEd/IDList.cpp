// IDList.cpp: implementation of the CIDList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "AnimID1.h"
#include "IDList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIDList::CIDList()
{
	m_nAnims = 0;
	m_EnableFilter = TRUE;
}

CIDList::~CIDList()
{
	Delete();
}

void CIDList::Delete()
{
	CAnimID	*pAnimID;

	//---	Remove the ID list
	this->Clear();

	//---	Remove the Filter list
	while (!m_FilterList.IsEmpty())
	{
		pAnimID = m_FilterList.RemoveTail();
		delete pAnimID;
	}
}

CAnimID * CIDList::AddTail(const char *IDName)
{
	CAnimID	*pNewAnimID = new CAnimID( IDName );

	m_IDList.AddTail( pNewAnimID );

	m_nAnims++;

	return pNewAnimID;
}

int CIDList::IsEmpty()
{
	return m_IDList.IsEmpty();
}

CAnimID * CIDList::RemoveTail()
{
	return m_IDList.RemoveTail();
}

CAnimID * CIDList::GetHead()
{
#if 1
	CAnimID		*pAnimID;
	CAnimID		*pFilter;

	//---	Start at the top of the list.
	m_ListPos = m_IDList.GetHeadPosition();

	if (m_ListPos)
	{
		//---	Get a pointer to the current AnimID.
		pAnimID = m_IDList.GetAt( m_ListPos );

		//---	If filtering is enabled then check for skipping this ID.
		if (m_EnableFilter)
		{
			//---	Loop through all filter prefixes and ignore the IDs that match any filter prefix.
			pFilter = GetFirstFilter();
			while (pFilter)
			{
				//---	If we find the filter prefix at the start of the ID then ignore this ID.
				if (pAnimID->m_AnimID.Find( pFilter->m_AnimID ) == 0)
					return this->GetNext();

				//---	Get the next filter prefix to check the ID for.
				pFilter = GetNextFilter();
			}
		}

		//---	If we make it through the filters then return this ID. (It could be NULL)
		return pAnimID;
	}
	else
		return NULL;
#else
	m_ListPos = m_IDList.GetHeadPosition();

	if (m_ListPos)
		return m_IDList.GetAt( m_ListPos );
	else
		return NULL;
#endif
}

CAnimID * CIDList::GetNext()
{
#if 1
	CAnimID		*pAnimID;
	CAnimID		*pFilter;

	if (m_ListPos)
	{
		m_IDList.GetNext( m_ListPos );

		if (m_ListPos)
		{
			//---	Get a pointer to the current AnimID.
			pAnimID = m_IDList.GetAt( m_ListPos );

			//---	If filtering is enabled then check for skipping this ID.
			if (m_EnableFilter)
			{
				//---	Loop through all filter prefixes and ignore the IDs that match any filter prefix.
				pFilter = GetFirstFilter();
				while (pFilter)
				{
					//---	If we find the filter prefix at the start of the ID then ignore this ID.
					if (pAnimID->m_AnimID.Find( pFilter->m_AnimID ) == 0)
						return this->GetNext();

					//---	Get the next filter prefix to check the ID for.
					pFilter = GetNextFilter();
				}
			}

			//---	If we make it through the filters then return this ID. (It could be NULL)
			return pAnimID;
		}
	}

	return NULL;
#else
	if (m_ListPos)
	{
		m_IDList.GetNext( m_ListPos );

		if (m_ListPos)
			return m_IDList.GetAt( m_ListPos );
	}

	return NULL;
#endif
}

int CIDList::IncrementUses(const char *lpszIDName)
{
	CAnimID *pFoundID;

	pFoundID = FindID( lpszIDName );

	if (pFoundID)
		return pFoundID->IncrementUses();

	return -1;
}

int CIDList::DecrementUses(const char *lpszIDName)
{
	CAnimID *pFoundID;

	if( lpszIDName[0] == '\0' )
		return -1;

	pFoundID = FindID( lpszIDName );

	if (pFoundID)
		return pFoundID->DecrementUses();

	return -1;
}

CAnimID * CIDList::FindID(const char *lpszIDName)
{
	POSITION	listPos;
	CAnimID		*pAnimID;
	int			compare = -1;

	listPos = m_IDList.GetHeadPosition();

	while(listPos && (compare < 0))
	{
		pAnimID = m_IDList.GetNext( listPos );

		compare = pAnimID->m_AnimID.Compare( lpszIDName );
	}

	if (compare == 0)
		return pAnimID;
	else
		return NULL;
}

POSITION CIDList::FindIDPos( const char *lpszIDName )
{
	POSITION	nextPos, thisPos = NULL, lastPos;
	CAnimID		*pAnimID;
	int			compare = 0;

	nextPos = m_IDList.GetHeadPosition();

	while(nextPos)
	{
		lastPos = thisPos;
		thisPos = nextPos;
		pAnimID = m_IDList.GetNext( nextPos );

		compare = pAnimID->m_AnimID.Compare( lpszIDName );
		if ((compare == 0) || ((compare < 0) && (nextPos == NULL)))
			return thisPos;
		else if (compare > 0) // search string comes before this ID
			return lastPos;
	}

	return NULL;
}

void CIDList::Serialize(CArchive& ar)
{
	short int	version;
	int			nNum;
	CAnimID		*pFilter;
	CString		Prefix;
 
	if (ar.IsStoring())
	{
		//---	Write Version
		version = 1;
		ar.Write( &version, 2 );

		//---	Write the filter enabled.
		ar << m_EnableFilter;

		//---	Write the filter count.
		nNum = this->GetFilterCount();
		ar << nNum;

		//---	Write the filter strings.
		pFilter = this->GetFirstFilter();
		while (pFilter)
		{
			ar.WriteString( pFilter->m_AnimID );
			ar.WriteString( "\xd\xa" );

			pFilter = this->GetNextFilter();
		}
	}
	else
	{
		//---	Read Version
		ar.Read( &version, 2 );

		switch (version)
		{
		case 1:
			//---	Read the filter enabled.
			ar >> m_EnableFilter;

			//---	Read the filter count.
			ar >> nNum;

			//---	Read the filter strings.
			for (; nNum; nNum--)
			{
				ar.ReadString( Prefix );
				this->AddFilter( Prefix );
			}
			break;
		}
	}
}

CAnimID * CIDList::Add(const char * lpszIDName)
{
	POSITION	newPos;
	CAnimID		*pPosID;
	CAnimID		*pNewAnimID;

	if (!lpszIDName)
		return NULL;

	pNewAnimID = new CAnimID( lpszIDName );

	newPos = FindIDPos( lpszIDName );

	//---	If we do not have a position value then the new item goes at the top of the list.
	if (!newPos)
	{
		m_IDList.AddHead( pNewAnimID );
		m_nAnims++;
	}
	else // otherwise it goes after the position returned.
	{
		pPosID = m_IDList.GetAt( newPos );

		//---	If the item is already in the list then free the new item.
		if (pPosID->m_AnimID.Compare( lpszIDName ) == 0)
		{
			delete pNewAnimID;

			pNewAnimID = pPosID;
		}
		else
		{
			m_IDList.InsertAfter( newPos, pNewAnimID );
			m_nAnims++;
		}
	}

	return pNewAnimID;
}

CAnimID * CIDList::Remove(const char *lpszIDName)
{
	CAnimID		*pAnimID = NULL;
	int			compare;
	POSITION	foundPos;

	foundPos = FindIDPos( lpszIDName );
	
	if (foundPos)
	{
		pAnimID = m_IDList.GetAt( foundPos );
	
		compare = pAnimID->m_AnimID.Compare( lpszIDName );

		if (compare == 0)
		{
			m_IDList.RemoveAt( foundPos );
			m_nAnims--;
			return pAnimID;
		}
	}

	return NULL;
}

//---	If the ID is already in the list it increments the uses.  Otherwise it adds it to the list and increments the uses.
CAnimID * CIDList::AddInc(const char *lpszIDName)
{
	CAnimID *pAnimID;

	pAnimID = this->Add( lpszIDName );

	if (pAnimID)
		pAnimID->IncrementUses();

	return pAnimID;
}

//---	Decrements the uses and if the ID is not used any more it is removed from the list.
CAnimID * CIDList::RemoveDec(const char *lpszIDName)
{
	CAnimID	*pAnimID;

	pAnimID = this->FindID( lpszIDName );
	if (pAnimID && (pAnimID->DecrementUses() == 0))
		this->Remove( lpszIDName );

	return pAnimID;
}

void CIDList::AddFilter(const char *lpszPrefix)
{
	POSITION	newPos;
	CAnimID		*pPosFilter;
	CAnimID		*pNewFilter;

	if (!lpszPrefix)
		return;

	pNewFilter = new CAnimID( lpszPrefix );

	newPos = FindFilter( lpszPrefix );

	//---	If we do not have a position value then the new item goes at the top of the list.
	if (!newPos)
		m_FilterList.AddHead( pNewFilter );
	else // otherwise it goes after the position returned.
	{
		pPosFilter = m_FilterList.GetAt( newPos );

		//---	If the item is already in the list then free the new item.
		if (pPosFilter->m_AnimID.Compare( lpszPrefix ) == 0)
		{
			delete pNewFilter;

			pNewFilter = pPosFilter;
		}
		else
			m_FilterList.InsertAfter( newPos, pNewFilter );
	}
}

void CIDList::RemoveFilter(const char *lpszPrefix)
{
	CAnimID		*pFilter = NULL;
	int			compare;
	POSITION	foundPos;

	foundPos = FindFilter( lpszPrefix );
	
	if (foundPos)
	{
		pFilter = m_FilterList.GetAt( foundPos );
	
		compare = pFilter->m_AnimID.Compare( lpszPrefix );

		if (compare == 0)
			m_FilterList.RemoveAt( foundPos );
	}
}

CAnimID * CIDList::GetFirstFilter()
{
	CAnimID	*pFilter;

	m_FilterListPos = m_FilterList.GetHeadPosition();

	if (m_FilterListPos)
	{
		pFilter = m_FilterList.GetAt( m_FilterListPos );

		if (pFilter)
			return pFilter;
	}

	return NULL;
}

CAnimID * CIDList::GetNextFilter()
{
	CAnimID	*pFilter;

	if (m_FilterListPos)
	{
		m_FilterList.GetNext( m_FilterListPos );

		if (m_FilterListPos)
		{
			pFilter = m_FilterList.GetAt( m_FilterListPos );

			if (pFilter)
				return pFilter;
		}
	}

	return NULL;
}

POSITION CIDList::FindFilter(const char *lpszPrefix)
{
	POSITION	nextPos, thisPos = NULL, lastPos;
	CAnimID		*pAnimID;
	int			compare = 0;

	nextPos = m_FilterList.GetHeadPosition();

	while(nextPos)
	{
		lastPos = thisPos;
		thisPos = nextPos;
		pAnimID = m_FilterList.GetNext( nextPos );

		compare = pAnimID->m_AnimID.Compare( lpszPrefix );
		if ((compare == 0) || ((compare < 0) && (nextPos == NULL)))
			return thisPos;
		else if (compare > 0) // search string comes before this ID
			return lastPos;
	}

	return NULL;
}


int CIDList::EnableFiltering(int Set)
{
	int	current;

	current = m_EnableFilter;

	m_EnableFilter = Set;

	return current;
}

int CIDList::GetFilterCount()
{
	int count = 0;
	CAnimID *pFilter;

	pFilter = this->GetFirstFilter();
	while (pFilter)
	{
		count++;
		pFilter = this->GetNextFilter();
	}

	return count;
}

int CIDList::GetFilteringEnabled()
{
	return m_EnableFilter;
}

void CIDList::Clear()
{
	CAnimID	*pAnimID;

	//---	Remove the ID list
	while (!m_IDList.IsEmpty())
	{
		pAnimID = m_IDList.RemoveTail();
		delete pAnimID;
	}
}
