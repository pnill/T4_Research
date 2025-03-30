// ProjFile.cpp: implementation of the CProjFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnimTblEd.h"
#include "ProjFile.h"

#include "AnimTblEdDefs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProjFile::CProjFile()
{
	m_Processed = FALSE;
}

CProjFile::~CProjFile()
{
	//---	Close our file.

#if 0
//	TRY
//	{
		m_File.Close();
//	}
//	CATCH( CFileException, e )
//	{
//		#ifdef _DEBUG
//			afxDump << "File could not be closed " << e->m_cause << "\n";
//		#endif
//	}
#endif
}

CProjFile::CProjFile(const char * lpszFileName, bool Processed)
{
	//---	Set the file pointer to NULL.
	m_pFile = NULL;

	//---	Copy the file name into the filename buffer.
	m_FileName.Format( "%s", lpszFileName );

#if 0
	TRY
	{
		m_File.Open( m_FileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText);
	}
	CATCH( CFileException, e )
	{
	   #ifdef _DEBUG
		  afxDump << "File could not be opened " << e->m_cause << "\n";
	   #endif
	}
	END_CATCH
#endif

	//---	Set Flag indicating whether the file has been processed.
	m_Processed = Processed;
}


/////////////////////////////////////////////////////////////////////////
// Resets file pointer to the top.
/////////////////////////////////////////////////////////////////////////

bool CProjFile::Reset()
{
	if (!m_pFile)
		return FALSE;

	//---	Set the file pointer to the start of the file.
	fseek( m_pFile, 0L, SEEK_SET);

	m_CurIDType = IDTYPE_ANIM;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////
// Get next identifier from the project file.  Return FALSE at EOF.
/////////////////////////////////////////////////////////////////////////

int CProjFile::GetNextIdentifier(char * lpszIDBuffer, unsigned int nBuffSize)
{
	char inStream[1024] = "";

	//---	Loop until we have an identifier.
	while (strcmp(inStream, "#define") != 0) 
	{
		//---	get the next string
		if (fscanf( m_pFile, "%s", &inStream[0] ) == EOF)
			return IDTYPE_EOF; // when we reach the end of the file we return FALSE.

		//---	check for a type change
		if (strcmp( inStream, "EVENTS" ) == 0)
			m_CurIDType = IDTYPE_EVENT;
		else if (strcmp( inStream, "FLAGS" ) == 0)
			m_CurIDType = IDTYPE_FLAG;
		else if (strcmp( inStream, "PROPS" ) == 0)
			m_CurIDType = IDTYPE_PROPS;
		else if (strcmp( inStream, "TRACKING" ) == 0)
			m_CurIDType = IDTYPE_POINTTRACKING;
		else if (strcmp( inStream, "STREAMS" ) == 0)
			m_CurIDType = IDTYPE_STREAMS;
	}

	//---	Get the identifier.
	fscanf( m_pFile, "%s", &inStream[0] );
	
	//---	Ensure that we will not write past the end of our buffer.
	if (strlen(inStream) > nBuffSize)
		inStream[nBuffSize] = NULL;

	//---	Copy the string into the output buffer.
	strcpy( lpszIDBuffer, inStream );

	return m_CurIDType;	// when we have a valid ID we return TRUE.
}


void CProjFile::Serialize(CArchive &ar)
{
	if (ar.IsStoring())
	{
		ar.WriteString( m_FileName );
		ar.WriteString( "\xd\xa" );
	}
	else
	{
		ar.ReadString( m_FileName );
	}
}

void CProjFile::Open()
{
	//---	Open the file for reading.
	m_pFile = fopen(m_FileName, "rt");
}

void CProjFile::Close()
{
	if (m_pFile)
		fclose( m_pFile );
}

int CProjFile::IsOpen()
{
	return m_pFile ? TRUE : FALSE;
}
