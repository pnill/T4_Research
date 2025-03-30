#ifndef _MeshList_h_
#define	_MeshList_h_
// MeshList.h : header file
//

#include "FileTags.h"
#include "ieList.h"
#include "ieArchive.h"

class CMesh ;

/////////////////////////////////////////////////////////////////////////////
//---	LOCAL ARCHIVE TAGS
enum
{
	IE_TAG_MESHLIST_END = IE_TAG_MESHLIST+1,
} ;

/////////////////////////////////////////////////////////////////////////////
// CMeshList

class CMeshList : public CieList
{
	DECLARE_DYNCREATE(CMeshList)

// Attributes
public:

// Constructors
public:
	CMeshList();
	~CMeshList();

// Operations
public:
	virtual void Serialize( CieArchive& a ) ;

	CMesh	*GetNext (POSITION &rPosition) { return (CMesh*)CieList::GetNext(rPosition) ; }
	CMesh	*FindMatchingMesh( CMesh *pMesh );
	void	DeleteAllMeshs() ;
	bool	SearchReplaceFilePaths( CString Search, CString Replace, s32 Method );

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
} ;

/////////////////////////////////////////////////////////////////////////////
#endif	//_MeshList_h_
