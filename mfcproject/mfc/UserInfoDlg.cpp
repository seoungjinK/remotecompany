#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "UserInfoDlg.h"

IMPLEMENT_DYNAMIC(CUserInfoDlg, CDialogEx)


CUserInfoDlg::CUserInfoDlg(int userId, CWnd* pParent )
    : CDialogEx(IDD_USER_INFO_DIALOG, pParent), m_nUserId(userId)
{
}

CUserInfoDlg::~CUserInfoDlg()
{
}

void CUserInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_STATIC_NAME, m_strName);
    DDX_Text(pDX, IDC_STATIC_DEPT, m_strDept);
    DDX_Text(pDX, IDC_STATIC_EMAIL, m_strEmail);
    DDX_Text(pDX, IDC_STATIC_PHONE, m_strPhone);
}

BEGIN_MESSAGE_MAP(CUserInfoDlg, CDialogEx)
END_MESSAGE_MAP()



BOOL CUserInfoDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    if (m_nUserId == -1) {
        AfxMessageBox(_T("사용자 정보를 가져올 수 없습니다."));
        EndDialog(IDCANCEL);
        return TRUE;
    }

    if (ConnectDatabase())
    {
        try
        {
            CString strSQL;
            strSQL.Format(
                _T("SELECT e.full_name, e.department, u.email, u.phone_number ")
                _T("FROM users u JOIN employees e ON u.employee_id = e.employee_id ")
                _T("WHERE u.user_id = %d"), m_nUserId);

            CRecordset rs(&m_db);
            rs.Open(CRecordset::forwardOnly, strSQL);

            if (!rs.IsEOF())
            {
                // DB에서 읽어온 값들을 멤버 변수에 저장
                rs.GetFieldValue(_T("full_name"), m_strName);
                rs.GetFieldValue(_T("department"), m_strDept);
                rs.GetFieldValue(_T("email"), m_strEmail);
                rs.GetFieldValue(_T("phone_number"), m_strPhone);
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

BOOL CUserInfoDlg::ConnectDatabase()
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