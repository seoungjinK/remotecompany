#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "PostDetailDlg.h"


// CPostDetailDlg 대화 상자

IMPLEMENT_DYNAMIC(CPostDetailDlg, CDialogEx)

CPostDetailDlg::CPostDetailDlg(int postId, int currentUserId, CWnd* pParent )
    : CDialogEx(IDD_POST_DETAIL_DIALOG, pParent)
    , m_nPostId(postId)
    , m_nCurrentUserId(currentUserId)
{
}
CPostDetailDlg::~CPostDetailDlg()
{
}

void CPostDetailDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_POST_TITLE, m_staticPostTitle);
    DDX_Control(pDX, IDC_EDIT_POST_CONTENT, m_editPostContent);
    DDX_Control(pDX, IDC_LIST_COMMENTS, m_listComments);
    DDX_Control(pDX, IDC_EDIT_NEW_COMMENT, m_editNewComment);
}


BEGIN_MESSAGE_MAP(CPostDetailDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_SUBMIT_COMMENT, &CPostDetailDlg::OnBnClickedSubmitComment)
END_MESSAGE_MAP()

BOOL CPostDetailDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    LoadPostDetails(); // 데이터 로드 함수 호출

    return TRUE;
}


BOOL CPostDetailDlg::ConnectDatabase()
{
    if (m_db.IsOpen())
        return TRUE;

    // 성공한 연결 방식 (DSN, UID, PWD를 문자열에 포함)
    CString strConnect;
    strConnect.Format(_T("DSN=%s;UID=%s;PWD=%s;"),
        _T("Mysql_test"),   // DSN 이름
        _T("root"),         // MySQL 사용자명
        _T("1234"));        // MySQL 비밀번호

    try
    {
        if (!m_db.OpenEx(strConnect, CDatabase::noOdbcDialog))
        {
            AfxMessageBox(_T("MySQL 데이터베이스 연결 실패. DSN 및 인증 정보를 확인하세요."));
            return FALSE;
        }
        return TRUE;
    }
    catch (CDBException* e)
    {
        TCHAR szError[1024];
        e->GetErrorMessage(szError, 1024);
        CString errorMsg;
        errorMsg.Format(_T("DB 연결 중 예외 발생: %s"), szError);
        AfxMessageBox(errorMsg);
        e->Delete();
        return FALSE;
    }
}

void CPostDetailDlg::LoadPostDetails()
{
    if (!ConnectDatabase()) return;

    // --- 1. 게시글 본문 불러오기 ---
    try
    {
        CString strSQL;
        strSQL.Format(_T("SELECT title, content FROM posts WHERE post_id = %d"), m_nPostId);

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        if (!rs.IsEOF())
        {
            CString title, content;
            rs.GetFieldValue(_T("title"), title);
            rs.GetFieldValue(_T("content"), content);

            // 컨트롤에 텍스트 설정
            m_staticPostTitle.SetWindowText(title);
            m_editPostContent.SetWindowText(content);
        }
        rs.Close();
    }
    catch (CDBException* e) { e->Delete(); }

    // --- 2. 댓글 목록 불러오기 ---
    try
    {
        m_listComments.ResetContent(); 

        CString strSQL;
        strSQL.Format(_T("SELECT content, DATE_FORMAT(created_at, '%%Y-%%m-%%d %%H:%%i') as created_at ")
            _T("FROM comments WHERE post_id = %d ORDER BY created_at ASC"), m_nPostId);

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            CString content, createdAt;
            rs.GetFieldValue(_T("content"), content);
            rs.GetFieldValue(_T("created_at"), createdAt);

            CString formattedComment;
            formattedComment.Format(_T("익명 (%s) : %s"), createdAt, content);

            m_listComments.AddString(formattedComment);
            rs.MoveNext();
        }
        rs.Close();

        int nCount = m_listComments.GetCount();
        if (nCount > 0) {
            m_listComments.SetTopIndex(nCount - 1);
        }
    }
    catch (CDBException* e) { e->Delete(); }
}
void CPostDetailDlg::OnBnClickedSubmitComment()
{
    // 1. 입력창에서 댓글 내용 가져오기
    CString commentText;
    m_editNewComment.GetWindowText(commentText);
    commentText.Trim(); // 앞뒤 공백 제거

    // 2. 내용이 비어있는지 확인
    if (commentText.IsEmpty())
    {
        AfxMessageBox(_T("댓글 내용을 입력하세요."));
        return;
    }

    // 3. DB에 댓글 삽입 (INSERT 쿼리)
    if (ConnectDatabase())
    {
        try
        {
            CString safeContent = commentText;
            safeContent.Replace(_T("'"), _T("''"));

            // 3-2. INSERT 쿼리 실행
            CString strSQL;
            strSQL.Format(
                _T("INSERT INTO comments (post_id, user_id, content) VALUES (%d, %d, '%s')"),
                m_nPostId, m_nCurrentUserId, safeContent);

            m_db.ExecuteSQL(strSQL);

            // 4. 댓글 목록 새로고침 및 입력창 비우기
            LoadPostDetails(); // 댓글 목록을 다시 불러와 화면을 갱신
            m_editNewComment.SetWindowText(_T("")); // 입력창 초기화
        }
        catch (CDBException* e)
        {
            AfxMessageBox(_T("댓글 등록 중 오류가 발생했습니다."));
            e->Delete();
        }
    }
}