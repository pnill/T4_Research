// ProjFile.h: interface for the CProjFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROJFILE_H__B889FD21_5873_11D2_ABF7_00A024569875__INCLUDED_)
#define AFX_PROJFILE_H__B889FD21_5873_11D2_ABF7_00A024569875__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

enum
{
	IDTYPE_EOF,

	IDTYPE_ANIM,
	IDTYPE_EVENT,
	IDTYPE_FLAG,
	IDTYPE_PROPS,
	IDTYPE_POINTTRACKING,
	IDTYPE_STREAMS,
};

class CProjFile	: public CObject
{
protected:
	CString	m_FileName;
	FILE *m_pFile;
	bool m_Processed;
	int m_CurIDType;

public:
	int IsOpen();
	void Close();
	void Open();
	void Serialize(CArchive& ar);
	CString GetFileName()
		{return m_FileName;};
	bool IsProcessed() 
		{return m_Processed;};
	void SetProcessed(bool Processed)
		{m_Processed = Processed;};
	int GetNextIdentifier(char * lpszIDBuffer, unsigned int nBuffSize);
	bool Reset();
	CProjFile(const char *lpszFileName, bool Processed = FALSE);
	CProjFile();
	virtual ~CProjFile();

};

#endif // !defined(AFX_PROJFILE_H__B889FD21_5873_11D2_ABF7_00A024569875__INCLUDED_)
