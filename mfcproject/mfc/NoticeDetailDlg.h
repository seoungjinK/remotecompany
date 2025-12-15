#pragma once
#include "afxdialogex.h"
#include "afxdb.h" 

// CNoticeDetailDlg 대화 상자

class CNoticeDetailDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CNoticeDetailDlg)

public:
    CNoticeDetailDlg(int noticeId, CWnd* pParent = nullptr);
    virtual ~CNoticeDetailDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_NOTICE_DETAIL_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog(); 

    int m_nNoticeId; // 이전 창에서 전달받을 공지사항 ID
    CDatabase m_db;

    CString m_strTitle;
    CString m_strAuthor;
    CString m_strDate;
    CString m_strContent;

    BOOL ConnectDatabase();

	DECLARE_MESSAGE_MAP()
};
