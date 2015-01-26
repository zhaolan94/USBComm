// MFCDLLTest.h : main header file for the MFCDLLTest DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMFCDLLTestApp
// See MFCDLLTest.cpp for the implementation of this class
//

class CMFCDLLTestApp : public CWinApp
{
public:
	CMFCDLLTestApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
