#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "WritePostDlg.h"
#include <afxdb.h>

IMPLEMENT_DYNAMIC(CWritePostDlg, CDialogEx)
CWritePostDlg::CWritePostDlg(int currentUserId, CWnd* pParent )
    : CDialogEx(IDD_WRITE_POST_DIALOG, pParent), m_nCurrentUserId(currentUserId)
{
}

CWritePostDlg::~CWritePostDlg()
{
}

void CWritePostDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_TITLE, m_editTitle);
    DDX_Control(pDX, IDC_EDIT_CONTENT_1, m_editContent);
}

BEGIN_MESSAGE_MAP(CWritePostDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &CWritePostDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CWritePostDlg::ConnectDatabase()
{
    if (m_db.IsOpen()) return TRUE;
    CString strConnect;
    strConnect.Format(_T("DSN=%s;UID=%s;PWD=%s;"), _T("Mysql_test"), _T("root"), _T("1234"));
    try {
        if (!m_db.OpenEx(strConnect, CDatabase::noOdbcDialog)) {
            AfxMessageBox(_T("MySQL DB 연결 실패"));
            return FALSE;
        }
        return TRUE;
    }
    catch (CDBException* e) {
        TCHAR szError[1024];
        e->GetErrorMessage(szError, 1024);
        AfxMessageBox(szError);
        e->Delete();
        return FALSE;
    }
}

void CWritePostDlg::OnBnClickedOk()
{
    CString title, content;
    m_editTitle.GetWindowText(title);
    m_editContent.GetWindowText(content);
    title.Trim();
    content.Trim();

    if (title.IsEmpty() || content.IsEmpty()) {
        AfxMessageBox(_T("제목과 내용을 모두 입력해주세요."));
        return;
    }

    if (ConnectDatabase()) {
        try {
            CString safeTitle = title, safeContent = content;
            safeTitle.Replace(_T("'"), _T("''"));
            safeContent.Replace(_T("'"), _T("''"));

            CString strSQL;
            strSQL.Format(
                _T("INSERT INTO posts (user_id, title, content) VALUES (%d, '%s', '%s')"),
                m_nCurrentUserId, safeTitle, safeContent);

            m_db.ExecuteSQL(strSQL);
            EndDialog(IDOK);
        }
        catch (CDBException* e) {
            AfxMessageBox(_T("게시글 등록 중 오류가 발생했습니다."));
            e->Delete();
        }
    }
}