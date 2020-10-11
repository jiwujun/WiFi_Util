
// MFC_WIFI.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMFCWIFIApp:
// See MFC_WIFI.cpp for the implementation of this class
//

class CMFCWIFIApp : public CWinApp
{
public:
	CMFCWIFIApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMFCWIFIApp theApp;
