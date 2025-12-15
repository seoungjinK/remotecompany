#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "HomeDlg.h"
#include "NewWorkRequestDlg.h" 
#include "WorkDetailDlg.h" 



IMPLEMENT_DYNAMIC(CHomeDlg, CDialogEx)

CHomeDlg::CHomeDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_HOME_DIALOG, pParent)
    , m_nCurrentUserId(-1)
    , m_bIsAdmin(false)
{
}

CHomeDlg::~CHomeDlg()
{
}

void CHomeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CHomeDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CHomeDlg 메시지 처리기
BOOL CHomeDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    return TRUE;
}

void CHomeDlg::InitializeAndLoadData(int userId, bool isAdmin)
{
    m_nCurrentUserId = userId;
    m_bIsAdmin = isAdmin;
    LoadWorkRequestsFromDB();
}

BOOL CHomeDlg::ConnectDatabase()
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

void CHomeDlg::LoadWorkRequestsFromDB()
{
    if (!ConnectDatabase()) return;

    m_workRequests.clear();
    CString strSQL;

    if (m_bIsAdmin)
    {
        strSQL.Format(
            _T("SELECT wr.request_id, wr.status, wr.title, e.full_name, DATE_FORMAT(wr.due_date, '%%Y-%%m-%%d') as due_date ")
            _T("FROM work_requests wr JOIN users u ON wr.user_id = u.user_id JOIN employees e ON u.employee_id = e.employee_id ")
            _T("WHERE wr.user_id = %d ORDER BY wr.created_at DESC"), m_nCurrentUserId);
    }
    else
    {

        strSQL.Format(
            _T("SELECT wr.request_id, wr.status, wr.title, e.full_name, DATE_FORMAT(wr.due_date, '%%Y-%%m-%%d') as due_date ")
            _T("FROM work_requests wr ")
            _T("JOIN work_assignees wa ON wr.request_id = wa.request_id ")
            _T("JOIN users u ON wr.user_id = u.user_id ")
            _T("JOIN employees e ON u.employee_id = e.employee_id ")
            _T("WHERE wa.assignee_id = %d ORDER BY wr.due_date ASC"), m_nCurrentUserId);
    }

 
    int nRowsFound = 0;

    try
    {
        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            nRowsFound++; 

            WorkRequestInfo info;
            CString strRequestId;
            rs.GetFieldValue(_T("request_id"), strRequestId);
            info.requestId = _ttoi(strRequestId);
            rs.GetFieldValue(_T("status"), info.status);
            rs.GetFieldValue(_T("title"), info.title);
            rs.GetFieldValue(_T("full_name"), info.requesterName);
            rs.GetFieldValue(_T("due_date"), info.dueDate);
            m_workRequests.push_back(info);
            rs.MoveNext();
        }
        rs.Close();
    }
    catch (CDBException* e)
    {
        AfxMessageBox(_T("업무 목록 로딩 중 DB 오류 발생:\n") + e->m_strError);
        e->Delete();
    }

   

    Invalidate();
}

void CHomeDlg::OnPaint()
{
    

    CPaintDC dc(this);
    Graphics graphics(dc.GetSafeHdc());
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    CRect clientRect;
    GetClientRect(&clientRect);
    SolidBrush backBrush(Color::White);
    graphics.FillRectangle(&backBrush, 0, 0, clientRect.Width(), clientRect.Height());

    Gdiplus::Font headerFont(L"맑은 고딕", 12, FontStyleBold);
    Gdiplus::Font titleFont(L"맑은 고딕", 10, FontStyleRegular);
    Gdiplus::Font infoFont(L"맑은 고딕", 9, FontStyleRegular);
    SolidBrush blackBrush(Color::Black);
    SolidBrush grayBrush(Color(120, 120, 120));
    Pen linePen(Color(220, 220, 220));

    int margin = 30;
    float currentY = 20.0f;


    graphics.DrawString(m_bIsAdmin ? L"등록한 업무 목록" : L"할당된 업무 목록", -1, &headerFont, PointF((float)margin, currentY), &blackBrush);

    if (m_bIsAdmin)
    {
        // 관리자일 경우에만 버튼을 그립니다.
        Gdiplus::Font btnFont(L"맑은 고딕", 9, FontStyleRegular);
        CString btnText = _T("+ 새 업무 등록");
        RectF textBounds;
        graphics.MeasureString(btnText, -1, &btnFont, PointF(0, 0), &textBounds);
        float btnWidth = textBounds.Width + 20, btnHeight = textBounds.Height + 10;
        float btnX = clientRect.Width() - margin - btnWidth;
        float btnY = currentY;
        m_rectNewRequestButton.SetRect((int)btnX, (int)btnY, (int)(btnX + btnWidth), (int)(btnY + btnHeight));

        SolidBrush btnBackBrush(Color(245, 245, 245));
        graphics.FillRectangle(&btnBackBrush, m_rectNewRequestButton.left, m_rectNewRequestButton.top, m_rectNewRequestButton.Width(), m_rectNewRequestButton.Height());
        StringFormat strFormat; strFormat.SetAlignment(StringAlignmentCenter); strFormat.SetLineAlignment(StringAlignmentCenter);
        graphics.DrawString(btnText, -1, &btnFont, RectF((float)m_rectNewRequestButton.left, (float)m_rectNewRequestButton.top, (float)m_rectNewRequestButton.Width(), (float)m_rectNewRequestButton.Height()), &strFormat, &blackBrush);
    }
    currentY += 40;

    // --- 2. 목록 컬럼 헤더 그리기 ---
    graphics.DrawString(L"상태", -1, &infoFont, PointF((float)margin, currentY), &grayBrush);
    graphics.DrawString(L"제목", -1, &infoFont, PointF((float)margin + 100, currentY), &grayBrush);
    graphics.DrawString(L"요청자", -1, &infoFont, PointF((float)margin + 400, currentY), &grayBrush);
    graphics.DrawString(L"기한", -1, &infoFont, PointF((float)margin + 500, currentY), &grayBrush);
    currentY += 20;
    graphics.DrawLine(&linePen, (float)margin, currentY, (float)(clientRect.Width() - margin), currentY);
    currentY += 10;

    // --- 3. 업무 목록 그리기 ---
    for (WorkRequestInfo& info : m_workRequests)
    {
        float itemHeight = 40.0f;
        info.rectItem.SetRect(margin, (int)currentY, clientRect.Width() - margin, (int)(currentY + itemHeight));

        // 상태 표시 (색상 태그)
        SolidBrush statusBrush(Color(200, 200, 200)); // 기본 회색
        if (info.status == _T("처리중")) statusBrush.SetColor(Color(204, 230, 255)); // 파란색
        else if (info.status == _T("완료")) statusBrush.SetColor(Color(214, 245, 214)); // 녹색
        else if (info.status == _T("반려")) statusBrush.SetColor(Color(255, 214, 214)); // 빨간색
        graphics.FillRectangle(&statusBrush, (float)margin, currentY + 5, 70.0f, itemHeight - 15);
        StringFormat strFormat; strFormat.SetAlignment(StringAlignmentCenter); strFormat.SetLineAlignment(StringAlignmentCenter);
        graphics.DrawString(info.status, -1, &infoFont, RectF((float)margin, currentY + 5, 70.0f, itemHeight - 15), &strFormat, &grayBrush);

        graphics.DrawString(info.title, -1, &titleFont, PointF((float)margin + 100, currentY + 10), &blackBrush);
        graphics.DrawString(info.requesterName, -1, &infoFont, PointF((float)margin + 400, currentY + 12), &grayBrush);
        graphics.DrawString(info.dueDate, -1, &infoFont, PointF((float)margin + 500, currentY + 12), &grayBrush);

        currentY += itemHeight;
        graphics.DrawLine(&linePen, (float)margin, currentY, (float)(clientRect.Width() - margin), currentY);
    }
}

void CHomeDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_bIsAdmin && m_rectNewRequestButton.PtInRect(point))
    {
        CNewWorkRequestDlg dlg(m_nCurrentUserId, this);
        if (dlg.DoModal() == IDOK)
        {
            LoadWorkRequestsFromDB(); // 등록 성공 시 목록 새로고침
        }
        return;
    }

    // 목록 아이템 클릭 확인
    for (const auto& info : m_workRequests)
    {
        if (info.rectItem.PtInRect(point))
        {
            CWorkDetailDlg dlg(info.requestId, m_nCurrentUserId, m_bIsAdmin, this);

            // 상세 보기 창에서 '상태 변경'이 성공하면 IDOK가 반환됨
            if (dlg.DoModal() == IDOK)
            {
                LoadWorkRequestsFromDB(); // 목록을 새로고침하여 변경된 상태를 반영
            }
            return;
        }
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}