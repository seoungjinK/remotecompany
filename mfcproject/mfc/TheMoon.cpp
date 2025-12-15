#include "pch.h"
#include "framework.h"
#include "TheMoon.h"
#include "TheMoonDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




BEGIN_MESSAGE_MAP(CTheMoonApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()




CTheMoonApp::CTheMoonApp()
{

}




CTheMoonApp theApp;


// CTheMoonApp 초기화

BOOL CTheMoonApp::InitInstance()
{

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);

	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();


	CShellManager* pShellManager = new CShellManager;


	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	SetRegistryKey(_T("로컬 애플리케이션 마법사에서 생성된 애플리케이션"));

	CTheMoonDlg dlg;
	m_pMainWnd = &dlg;


	CString commandLine(m_lpCmdLine);
	CString token;
	int tokenPos = commandLine.Find(_T("token="));
	if (tokenPos != -1)
	{
		token = commandLine.Mid(tokenPos + 6);
		token.Trim(L"\"/");
	}

	INT_PTR nResponse;
	if (!token.IsEmpty())
	{

		if (dlg.VerifyTokenAndLogin(token))
		{
			nResponse = dlg.DoModal();
		}
		else
		{

			AfxMessageBox(_T("자동 로그인에 실패했습니다."));
			nResponse = IDCANCEL;
		}
	}
	else
	{
		nResponse = dlg.DoModal();
	}


	if (nResponse == IDOK)
	{

	}
	else if (nResponse == IDCANCEL)
	{

	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
		TRACE(traceAppMsg, 0, "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 없습니다.\n");
	}
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	return FALSE;
}