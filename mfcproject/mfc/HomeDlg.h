#pragma once
#include <afxdb.h>
#include <vector>
#include <gdiplus.h>

struct WorkRequestInfo
{
    int requestId;
    CString status;
    CString title;
    CString requesterName; // 요청한 사람 이름
    CString dueDate;

    CRect rectItem; // 클릭 감지를 위한 좌표
};

class CHomeDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CHomeDlg)

public:
    CHomeDlg(CWnd* pParent = nullptr);
    virtual ~CHomeDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_HOME_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();


    CDatabase m_db;
    int m_nCurrentUserId;
    bool m_bIsAdmin;
    std::vector<WorkRequestInfo> m_workRequests;
    CRect m_rectNewRequestButton;

    BOOL ConnectDatabase();
    void LoadWorkRequestsFromDB();

    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

public:
    void InitializeAndLoadData(int userId, bool isAdmin);
    DECLARE_MESSAGE_MAP()
};