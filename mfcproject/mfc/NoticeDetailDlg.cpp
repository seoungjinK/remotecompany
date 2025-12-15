#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "NoticeDetailDlg.h"

IMPLEMENT_DYNAMIC(CNoticeDetailDlg, CDialogEx)

CNoticeDetailDlg::CNoticeDetailDlg(int noticeId, CWnd* pParent )
	: CDialogEx(IDD_NOTICE_DETAIL_DIALOG, pParent), m_nNoticeId(noticeId)
{
}

CNoticeDetailDlg::~CNoticeDetailDlg()
{
}

void CNoticeDetailDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    DDX_Text(pDX, IDC_STATIC_TITLE, m_strTitle);
    DDX_Text(pDX, IDC_STATIC_AUTHOR, m_strAuthor);
    DDX_Text(pDX, IDC_STATIC_DATE, m_strDate);
    DDX_Text(pDX, IDC_EDIT_CONTENT, m_strContent);
}

BEGIN_MESSAGE_MAP(CNoticeDetailDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CNoticeDetailDlg::ConnectDatabase()
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

BOOL CNoticeDetailDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    if (m_nNoticeId == -1) {
        AfxMessageBox(_T("공지사항 정보를 가져올 수 없습니다."));
        EndDialog(IDCANCEL);
        return TRUE;
    }

    if (ConnectDatabase())
    {
        try
        {
            CString strSQL;
            strSQL.Format(
                _T("SELECT n.title, n.content, e.full_name, DATE_FORMAT(n.created_at, '%%Y-%%m-%%d %%H:%%i') as created_at ")
                _T("FROM notices n JOIN users u ON n.user_id = u.user_id ")
                _T("JOIN employees e ON u.employee_id = e.employee_id ")
                _T("WHERE n.notice_id = %d"), m_nNoticeId);

            CRecordset rs(&m_db);
            rs.Open(CRecordset::forwardOnly, strSQL);

            if (!rs.IsEOF())
            {

                rs.GetFieldValue(_T("title"), m_strTitle);
                rs.GetFieldValue(_T("content"), m_strContent);
                rs.GetFieldValue(_T("full_name"), m_strAuthor);
                rs.GetFieldValue(_T("created_at"), m_strDate);
            }
            rs.Close();
        }
        catch (CDBException* e)
        {
            AfxMessageBox(_T("데이터를 불러오는 중 오류가 발생했습니다."));
            e->Delete();
        }
    }
    UpdateData(FALSE);

    return TRUE;
}



