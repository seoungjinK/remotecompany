#pragma once
#include "afxdialogex.h"
#include <afxdb.h>


class CUserInfoDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CUserInfoDlg)

public:
    CUserInfoDlg(int userId, CWnd* pParent = nullptr);
    virtual ~CUserInfoDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_USER_INFO_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog(); // OnInitDialog 선언

    int m_nUserId; // 이전 창에서 전달받을 사용자 ID
    CDatabase m_db;

    CString m_strName;
    CString m_strDept;
    CString m_strEmail;
    CString m_strPhone;

    BOOL ConnectDatabase(); // DB 연결 함수 선언
    DECLARE_MESSAGE_MAP()
};
