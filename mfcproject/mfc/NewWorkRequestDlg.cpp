#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "NewWorkRequestDlg.h"

IMPLEMENT_DYNAMIC(CNewWorkRequestDlg, CDialogEx)

CNewWorkRequestDlg::CNewWorkRequestDlg(int currentUserId, CWnd* pParent )
    : CDialogEx(IDD_NEW_WORK_REQUEST_DIALOG, pParent)
    , m_nCurrentUserId(currentUserId)
{

}

CNewWorkRequestDlg::~CNewWorkRequestDlg()
{
}

void CNewWorkRequestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_TITLE, m_editTitle);
    DDX_Control(pDX, IDC_EDIT_CONTENT_2, m_editContent);
    DDX_Control(pDX, IDC_DATETIMEPICKER_DUE_DATE, m_datePicker);
    DDX_Control(pDX, IDC_BUTTON_ADD_FILE, m_btnAddFile);
    DDX_Control(pDX, IDC_LIST_ATTACHMENTS, m_listAttachments);
    // ✨ CCheckListBox로 연결
    DDX_Control(pDX, IDC_LIST_EMPLOYEES, m_listEmployees);
}


BEGIN_MESSAGE_MAP(CNewWorkRequestDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_ADD_FILE, &CNewWorkRequestDlg::OnBnClickedButtonAddFile)
    ON_BN_CLICKED(IDOK, &CNewWorkRequestDlg::OnBnClickedOk)
END_MESSAGE_MAP()


BOOL CNewWorkRequestDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    LoadEmployeeList(); // 다이얼로그가 뜰 때 직원 목록을 불러옵니다.

    return TRUE;
}

void CNewWorkRequestDlg::LoadEmployeeList()
{
    if (!ConnectDatabase()) return;

    try
    {
        // 관리자가 아닌 일반 직원 목록만 불러옵니다.
        CString strSQL = _T("SELECT u.user_id, e.full_name ")
            _T("FROM users u JOIN employees e ON u.employee_id = e.employee_id ")
            _T("WHERE e.is_admin = 0 ORDER BY e.full_name");

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            CString userId, fullName;
            rs.GetFieldValue(_T("user_id"), userId);
            rs.GetFieldValue(_T("full_name"), fullName);

            int nIndex = m_listEmployees.AddString(fullName);
            // 각 항목에 user_id를 데이터로 저장해둡니다.
            m_listEmployees.SetItemData(nIndex, _ttoi(userId));

            rs.MoveNext();
        }
        rs.Close();
    }
    catch (CDBException* e) { e->Delete(); }
}



void CNewWorkRequestDlg::OnBnClickedButtonAddFile()
{
    // CFileDialog를 이용해 파일을 선택합니다.
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("All Files (*.*)|*.*||"), this);

    if (dlg.DoModal() == IDOK)
    {
        CString filePath = dlg.GetPathName(); // 선택한 파일의 전체 경로
        CString fileName = dlg.GetFileName(); // 파일 이름만

        m_listAttachments.AddString(fileName);
        m_vecFilePaths.push_back(filePath); // 전체 경로는 별도로 저장
    }
}

void CNewWorkRequestDlg::OnBnClickedOk()
{
    // --- 1. 입력 값 유효성 검사 ---
    CString title, content;
    m_editTitle.GetWindowText(title);
    m_editContent.GetWindowText(content);
    title.Trim();
    content.Trim();

    if (title.IsEmpty()) {
        AfxMessageBox(_T("제목을 입력하세요."));
        return;
    }

    std::vector<int> vecTargetUserIDs;
    for (int i = 0; i < m_listEmployees.GetCount(); i++) {
        if (m_listEmployees.GetCheck(i)) {
            vecTargetUserIDs.push_back((int)m_listEmployees.GetItemData(i));
        }
    }
    if (vecTargetUserIDs.empty()) {
        AfxMessageBox(_T("업무를 받을 직원을 한 명 이상 선택하세요."));
        return;
    }

    CTime dueDate;
    m_datePicker.GetTime(dueDate);
    CString strDueDate = dueDate.Format(_T("%Y-%m-%d"));
    if (!ConnectDatabase()) return;

    try
    {
        m_db.BeginTrans(); // 트랜잭션 시작

        // 2-1. work_requests 테이블에 업무 정보 INSERT
        CString safeTitle = title, safeContent = content;
        safeTitle.Replace(_T("'"), _T("''"));
        safeContent.Replace(_T("'"), _T("''"));

        CString strSQL;
        strSQL.Format(
            _T("INSERT INTO work_requests (user_id, title, content, due_date) VALUES (%d, '%s', '%s', '%s')"),
            m_nCurrentUserId, safeTitle, safeContent, strDueDate);
        m_db.ExecuteSQL(strSQL);

        // 2-2. 방금 생성된 업무의 request_id 가져오기
        int newRequestId = -1;
        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, _T("SELECT LAST_INSERT_ID() as id"));
        if (!rs.IsEOF()) {
            CString strId;
            rs.GetFieldValue(_T("id"), strId);
            newRequestId = _ttoi(strId);
        }
        rs.Close();

        if (newRequestId == -1) {
            throw new CDBException(); 
        }


        for (int targetId : vecTargetUserIDs) {
            strSQL.Format(_T("INSERT INTO work_assignees (request_id, assignee_id) VALUES (%d, %d)"), newRequestId, targetId);
            m_db.ExecuteSQL(strSQL);
        }


        for (size_t i = 0; i < m_vecFilePaths.size(); ++i) {
            CString fileName;
            m_listAttachments.GetText(i, fileName);


            CString safeFileName = fileName;
            safeFileName.Replace(_T("'"), _T("''"));

            strSQL.Format(
                _T("INSERT INTO attachments (related_type, related_id, original_filename, stored_filename, file_path, file_size) ")
                _T("VALUES ('work_request', %d, '%s', '%s', '%s', 0)"),
                newRequestId, safeFileName, safeFileName, _T("C:/server_uploads/")); // file_size는 예시로 0
            m_db.ExecuteSQL(strSQL);
        }

        m_db.CommitTrans(); // 모든 쿼리가 성공하면 최종 저장
        AfxMessageBox(_T("새 업무가 성공적으로 등록되었습니다."));

        CDialogEx::OnOK(); // 다이얼로그 닫기

    }
    catch (CDBException* e)
    {
        m_db.Rollback(); // 하나라도 실패하면 모두 취소
        AfxMessageBox(_T("업무 등록 중 오류가 발생하여 모든 작업이 취소되었습니다."));
        e->Delete();
    }
}
BOOL CNewWorkRequestDlg::ConnectDatabase()
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

