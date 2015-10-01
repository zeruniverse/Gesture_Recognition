
// Gesture_Recognition.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CGesture_RecognitionApp:
// See Gesture_Recognition.cpp for the implementation of this class
//

class CGesture_RecognitionApp : public CWinApp
{
public:
	CGesture_RecognitionApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CGesture_RecognitionApp theApp;