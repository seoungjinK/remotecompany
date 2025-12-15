#pragma once
#include "afxdialogex.h"
#include "afxdb.h" 

// CPostDetailDlg 대화 상자

class CPostDetailDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CPostDetailDlg)

public:
    CPostDetailDlg(int postId, int currentUserId, CWnd* pParent = nullptr);
    virtual ~CPostDetailDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_POST_DETAIL_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog(); 

    afx_msg void OnBnClickedSubmitComment();

    int m_nPostId;
    int m_nCurrentUserId;
    CDatabase m_db;

    CStatic m_staticPostTitle;
    CEdit m_editPostContent;
    CListBox m_listComments;
    CEdit m_editNewComment;

    BOOL ConnectDatabase();
    void LoadPostDetails();
	DECLARE_MESSAGE_MAP()
};
