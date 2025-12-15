#pragma once
#include "afxdialogex.h"
#include <afxdb.h>
#include <vector> 

class CNewWorkRequestDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CNewWorkRequestDlg)

public:
    CNewWorkRequestDlg(int currentUserId, CWnd* pParent = nullptr);
    virtual ~CNewWorkRequestDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_NEW_WORK_REQUEST_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    // 멤버 변수
    int m_nCurrentUserId;
    CDatabase m_db;

    // 컨트롤 변수
    CEdit m_editTitle;
    CEdit m_editContent;
    CDateTimeCtrl m_datePicker;
    CButton m_btnAddFile;
    CListBox m_listAttachments;
    CCheckListBox m_listEmployees; 

    // 첨부 파일 경로 목록
    std::vector<CString> m_vecFilePaths;

    // 헬퍼 함수
    BOOL ConnectDatabase();
    void LoadEmployeeList();

    // 메시지 핸들러
    afx_msg void OnBnClickedButtonAddFile();
    afx_msg void OnBnClickedOk();
    DECLARE_MESSAGE_MAP()
};