#pragma once
#include "afxdialogex.h"
#include <afxdb.h>

#pragma once

class CWritePostDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CWritePostDlg)

public:
    CWritePostDlg(int currentUserId, CWnd* pParent = nullptr); // 생성자 수정
    virtual ~CWritePostDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_WRITE_POST_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    int m_nCurrentUserId;
    CDatabase m_db;
    CEdit m_editTitle;
    CEdit m_editContent;

    BOOL ConnectDatabase();
    afx_msg void OnBnClickedOk(); // '등록'(IDOK) 버튼 핸들러
    DECLARE_MESSAGE_MAP()
};
