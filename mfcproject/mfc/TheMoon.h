#pragma once
#define WM_USER_START_CHAT (WM_USER + 1)

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"




class CTheMoonApp : public CWinApp
{
public:
	CTheMoonApp();


public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CTheMoonApp theApp;
