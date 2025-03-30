// MovieExport.h: interface for the CMovieExport class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOVIEEXPORT_H__8E599862_E5B6_11D1_AEC0_00A024569FF3__INCLUDED_)
#define AFX_MOVIEEXPORT_H__8E599862_E5B6_11D1_AEC0_00A024569FF3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ExportFile.h"
#include "ExportProgress.h"
#include "x_math.hpp"

class CMovie ;
class CCeDDoc ;

class CMovieExport  
{
//---	Attributes
public:
	static CExportProgress	*m_pProgressDialog ;

	CExportFile		movie_Header ;
	CExportFile		movie_ActorTable ;
	CExportFile		movie_Actors ;
	CExportFile		movie_Final ;

	s16				m_DefineID ;					// New Define ID
	CStringArray	m_DefineArray ;					// String Array for Defines

	int				m_Target ;

//---	Operations
public:
	CMovieExport( int Target ) ;
	virtual ~CMovieExport( ) ;

	CString MakeDefine( CString String ) ;
	bool FindDefine( s16 *pID, CString String ) ;

	void Export( const char *pFileName, CMovie *pMovie, CCeDDoc *pDoc ) ;

    class ShortVector3
    {
    public:
        ShortVector3();
        ShortVector3(const vector3 &);

        s16 X, Y, Z;

        void                EnsureNonzero();
    };
};

CExportFile& operator<<(CExportFile&, const CMovieExport::ShortVector3&);

#endif // !defined(AFX_MOVIEEXPORT_H__8E599862_E5B6_11D1_AEC0_00A024569FF3__INCLUDED_)
