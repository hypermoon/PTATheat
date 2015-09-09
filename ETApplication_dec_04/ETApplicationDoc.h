//-----------------------------------------------------------------------------
// File: ETApplicationDoc.h
//
// Desc: interface of the CETApplicationDoc class
//
// Copyright (c) 2011 Eartronic Ltd. All rights reserved.
//-----------------------------------------------------------------------------


#pragma once

/*typedef struct linenode
{
	float x;
	float y;
	//float bandval;
}LNNODE;*/
/*
typedef struct freznode
{
	float band;
	float dbhl;
}FREZNODE;
typedef CList<FREZNODE,FREZNODE&> CDocList;
*/

class CETApplicationDoc : public CDocument
{
protected: // create from serialization only
	CETApplicationDoc();
	DECLARE_DYNCREATE(CETApplicationDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CETApplicationDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
public:
	  
	 // CNewList     m_postL_list[3],m_postR_list[3];

	//CList<LNNODE,LNNODE&> m_postR_list[3];

	// CDocList    m_Lf_list[3], m_Rt_list[3];

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
