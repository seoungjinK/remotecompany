#pragma once
#include "afxdialogex.h"
#include <vector>
#include <gdiplus.h>
#include "afxdb.h" 

struct NoticeInfo
{
    int noticeId;
    CString title;
    CString authorName; 
    CString createdAt;  

    CRect rectItem; 
};

struct PostInfo
{
    int postId;
    CString title;
    CString createdAt;
    int commentCount; 

    CRect rectItem;
};



class CCommunityDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCommunityDlg)

protected:
    CDatabase m_db; 
    int m_nCurrentUserId;

    std::vector<NoticeInfo> m_notices;
    std::vector<PostInfo> m_posts;
    CScrollBar m_scrollNotices;
    CScrollBar m_scrollPosts;

    CRect m_rectWritePostButton;
    int m_nNoticeScrollPos;
    int m_nNoticeTotalHeight;
    int m_nPostScrollPos;
    int m_nPostTotalHeight;


public:
	CCommunityDlg(CWnd* pParent = nullptr);   
	virtual ~CCommunityDlg();
    void InitializeAndLoadData(int userId);


// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMMUNITY_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
    BOOL ConnectDatabase();
    void LoadDataFromDB();
    void OnPaint();
    void OnLButtonDown(UINT nFlags, CPoint point);
    BOOL OnInitDialog();
    void UpdateNoticeScrollbar();
    void UpdatePostScrollbar();


	DECLARE_MESSAGE_MAP()

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};
