#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "CommunityDlg.h"
#include <vector>
#include <gdiplus.h>
#include "afxdb.h"

#include "NoticeDetailDlg.h"
#include "PostDetailDlg.h"     
#include "WritePostDlg.h" 


using namespace Gdiplus;


IMPLEMENT_DYNAMIC(CCommunityDlg, CDialogEx)

CCommunityDlg::CCommunityDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COMMUNITY_DIALOG, pParent)
{

}

CCommunityDlg::~CCommunityDlg()
{
}

void CCommunityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCommunityDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_SIZE()         
    ON_WM_VSCROLL()      
    ON_WM_MOUSEWHEEL()   
END_MESSAGE_MAP()

BOOL CCommunityDlg::ConnectDatabase()
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

void CCommunityDlg::LoadDataFromDB()
{
    if (!ConnectDatabase()) return;

    m_notices.clear();
    m_posts.clear();


    try
    {

        CString strSQL = _T("SELECT n.notice_id, n.title, e.full_name, DATE_FORMAT(n.created_at, '%Y-%m-%d') as created_at ")
            _T("FROM notices n JOIN users u ON n.user_id = u.user_id ")
            _T("JOIN employees e ON u.employee_id = e.employee_id ")
            _T("ORDER BY n.created_at DESC LIMIT 10"); 

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            NoticeInfo info;
            CString strNoticeId;
            rs.GetFieldValue(_T("notice_id"), strNoticeId);
            info.noticeId = _ttoi(strNoticeId);
            rs.GetFieldValue(_T("title"), info.title);
            rs.GetFieldValue(_T("full_name"), info.authorName);
            rs.GetFieldValue(_T("created_at"), info.createdAt);
            m_notices.push_back(info);
            rs.MoveNext();
        }
        rs.Close();
    }
    catch (CDBException* e) { e->Delete(); }

    try
    {

        CString strSQL = _T("SELECT p.post_id, p.title, DATE_FORMAT(p.created_at, '%Y-%m-%d') as created_at, COUNT(c.comment_id) as comment_count ")
            _T("FROM posts p LEFT JOIN comments c ON p.post_id = c.post_id ")
            _T("GROUP BY p.post_id ")
            _T("ORDER BY p.created_at DESC LIMIT 10");

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            PostInfo info;
            CString strPostId, strCommentCount;
            rs.GetFieldValue(_T("post_id"), strPostId);
            info.postId = _ttoi(strPostId);
            rs.GetFieldValue(_T("title"), info.title);
            rs.GetFieldValue(_T("created_at"), info.createdAt);
            rs.GetFieldValue(_T("comment_count"), strCommentCount);
            info.commentCount = _ttoi(strCommentCount);
            m_posts.push_back(info);
            rs.MoveNext();
        }
        rs.Close();
    }
    catch (CDBException* e) { e->Delete(); }


    UpdateNoticeScrollbar();
    UpdatePostScrollbar();
    Invalidate();
}


void CCommunityDlg::OnPaint()
{

    CPaintDC dc(this);
    Graphics graphics(dc.GetSafeHdc());
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

    CRect clientRect;
    GetClientRect(&clientRect);
    int scrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);

    Gdiplus::Font headerFont(L"맑은 고딕", 14, FontStyleBold);
    Gdiplus::Font titleFont(L"맑은 고딕", 10, FontStyleRegular);
    Gdiplus::Font infoFont(L"맑은 고딕", 8, FontStyleRegular);
    SolidBrush backBrush(Color(255, 255, 255));
    SolidBrush headerBrush(Color(0, 0, 0));
    SolidBrush titleBrush(Color(50, 50, 50));
    SolidBrush infoBrush(Color(150, 150, 150));
    Pen linePen(Color(230, 230, 230));

    RectF topHalfRect(0.0f, 0.0f, (float)(clientRect.Width() - scrollBarWidth), (float)clientRect.Height() / 2.0f);
    RectF bottomHalfRect(0.0f, topHalfRect.Height, (float)(clientRect.Width() - scrollBarWidth), (float)clientRect.Height() / 2.0f);

    int margin = 20;
    float itemHeight = 50.0f;




    {
        graphics.SetClip(topHalfRect);
        graphics.FillRectangle(&backBrush, topHalfRect);


        RectF noticeHeaderRect((float)margin, 20.0f, 200.0f, 30.0f);
        graphics.DrawString(L"📢 공지사항", -1, &headerFont, noticeHeaderRect, NULL, &headerBrush);


        float currentY = noticeHeaderRect.GetBottom() + 10.0f;
        currentY -= m_nNoticeScrollPos;

        for (NoticeInfo& info : m_notices)
        {
            info.rectItem.SetRect(margin, (int)currentY, (int)topHalfRect.Width - margin, (int)(currentY + itemHeight));


            if (currentY + itemHeight > 0 && currentY < topHalfRect.Height)
            {
                graphics.DrawString(info.title, -1, &titleFont, PointF((float)margin, currentY + 8.0f), &titleBrush);
                CString noticeDetails;
                noticeDetails.Format(_T("%s  |  %s"), info.authorName, info.createdAt);
                graphics.DrawString(noticeDetails, -1, &infoFont, PointF((float)margin, currentY + 30.0f), &infoBrush);
                graphics.DrawLine(&linePen, (float)margin, currentY + itemHeight - 1.0f, topHalfRect.Width - margin, currentY + itemHeight - 1.0f);
            }
            currentY += itemHeight;
        }

        graphics.ResetClip();
    }
    // 2. 하단: 익명 게시판 그리기

    {
        graphics.SetClip(bottomHalfRect);

        graphics.FillRectangle(&backBrush, bottomHalfRect);

        RectF postHeaderRect((float)margin, bottomHalfRect.Y + 20.0f, 200.0f, 30.0f);
        graphics.DrawString(L"😎 익명 게시판", -1, &headerFont, postHeaderRect, NULL, &headerBrush);

        Gdiplus::Font writeBtnFont(L"맑은 고딕", 9, FontStyleRegular);
        CString writeBtnText = _T("+ 글쓰기");
        RectF textBounds;
        graphics.MeasureString(writeBtnText, -1, &writeBtnFont, PointF(0, 0), &textBounds);
        float btnWidth = textBounds.Width + 20.0f;
        float btnHeight = textBounds.Height + 10.0f;
        float btnX = bottomHalfRect.Width - margin - btnWidth;
        float btnY = postHeaderRect.Y + (postHeaderRect.Height - btnHeight) / 2;
        m_rectWritePostButton.SetRect((int)btnX, (int)btnY, (int)(btnX + btnWidth), (int)(btnY + btnHeight));

        SolidBrush btnBackBrush(Color(240, 240, 240));
        Pen btnBorderPen(Color(200, 200, 200));
        graphics.FillRectangle(&btnBackBrush, btnX, btnY, btnWidth, btnHeight);
        graphics.DrawRectangle(&btnBorderPen, btnX, btnY, btnWidth, btnHeight);
        StringFormat strFormat;
        strFormat.SetAlignment(StringAlignmentCenter);
        strFormat.SetLineAlignment(StringAlignmentCenter);
        RectF textRect(btnX, btnY, btnWidth, btnHeight);
        graphics.DrawString(writeBtnText, -1, &writeBtnFont, textRect, &strFormat, &headerBrush);


        float currentY = postHeaderRect.GetBottom() + 10.0f;
        currentY -= m_nPostScrollPos;

        for (PostInfo& info : m_posts)
        {
            info.rectItem.SetRect(margin, (int)currentY, (int)bottomHalfRect.Width - margin, (int)(currentY + itemHeight));

            if (currentY + itemHeight > bottomHalfRect.Y && currentY < bottomHalfRect.GetBottom())
            {
                CString titleWithComments;
                titleWithComments.Format(_T("%s  [%d]"), info.title, info.commentCount);
                graphics.DrawString(titleWithComments, -1, &titleFont, PointF((float)margin, currentY + 8.0f), &titleBrush);
                graphics.DrawString(info.createdAt, -1, &infoFont, PointF((float)margin, currentY + 30.0f), &infoBrush);
                graphics.DrawLine(&linePen, (float)margin, currentY + itemHeight - 1.0f, bottomHalfRect.Width - margin, currentY + itemHeight - 1.0f);
            }
            currentY += itemHeight;
        }

        graphics.ResetClip();
    }
}



void CCommunityDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRect clientRect;
    GetClientRect(&clientRect);
    CRect topHalfRect(0, 0, clientRect.Width(), clientRect.Height() / 2);
    CRect bottomHalfRect(0, topHalfRect.bottom, clientRect.Width(), clientRect.Height());

    if (m_rectWritePostButton.PtInRect(point))
    {
        CWritePostDlg dlg(m_nCurrentUserId, this);
        if (dlg.DoModal() == IDOK)
        {
            LoadDataFromDB();
        }
        return;
    }

    if (topHalfRect.PtInRect(point))
    {
        for (const auto& info : m_notices)
        {
            if (info.rectItem.PtInRect(point))
            {
                CNoticeDetailDlg dlg(info.noticeId, this);
                dlg.DoModal();
                return;
            }
        }
    }

    else if (bottomHalfRect.PtInRect(point))
    {
        for (const auto& info : m_posts)
        {
            if (info.rectItem.PtInRect(point))
            {
                
                CPostDetailDlg dlg(info.postId, m_nCurrentUserId, this);
                dlg.DoModal();
                return; 
            }
        }
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}

void CCommunityDlg::InitializeAndLoadData(int userId)
{
    m_nCurrentUserId = userId;
    LoadDataFromDB();
}

BOOL CCommunityDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 스크롤바 컨트롤 생성
    CRect rectDummy(0, 0, 0, 0); // 처음에는 보이지 않게 생성
    m_scrollNotices.Create(SBS_VERT | WS_CHILD, rectDummy, this, 1001);
    m_scrollPosts.Create(SBS_VERT | WS_CHILD, rectDummy, this, 1002);

    return TRUE;
}

void CCommunityDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (nType == SIZE_MINIMIZED || cx == 0 || cy == 0) return;

    CRect clientRect;
    GetClientRect(&clientRect);
    int scrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);

    int headerAreaHeight = 60;

    // --- 공지사항 스크롤바 ---
    if (m_scrollNotices.GetSafeHwnd()) {
        m_scrollNotices.MoveWindow(
            clientRect.right - scrollBarWidth,
            headerAreaHeight,
            scrollBarWidth,
            clientRect.Height() / 2 - headerAreaHeight
        );
        m_scrollNotices.ShowWindow(SW_SHOW);
    }

    // --- 게시판 스크롤바 ---
    if (m_scrollPosts.GetSafeHwnd()) {
        m_scrollPosts.MoveWindow(
            clientRect.right - scrollBarWidth,
            clientRect.Height() / 2 + headerAreaHeight,
            scrollBarWidth,
            clientRect.Height() / 2 - headerAreaHeight
        );
        m_scrollPosts.ShowWindow(SW_SHOW);
    }

    // 스크롤바 정보 업데이트
    if (!m_notices.empty() || !m_posts.empty()) {
        UpdateNoticeScrollbar();
        UpdatePostScrollbar();
    }

    Invalidate();
}
void CCommunityDlg::UpdateNoticeScrollbar()
{
    if (!m_scrollNotices.GetSafeHwnd()) return;

    CRect clientRect;
    GetClientRect(&clientRect);

    int headerAreaHeight = 60;

    CRect noticeAreaRect(0, 0, clientRect.Width(), clientRect.Height() / 2);

    // 1. 공지사항 콘텐츠의 총 높이 계산
    float totalHeight = 20.0f + 30.0f + 10.0f; 
    totalHeight += m_notices.size() * 50.0f; 
    m_nNoticeTotalHeight = static_cast<int>(totalHeight);

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = m_nNoticeTotalHeight;

    si.nPage = (clientRect.Height() / 2) - headerAreaHeight;
    m_scrollNotices.SetScrollInfo(&si, TRUE);
}

void CCommunityDlg::UpdatePostScrollbar()
{
    if (!m_scrollPosts.GetSafeHwnd()) return;

    CRect clientRect;
    GetClientRect(&clientRect);

    int headerAreaHeight = 60;

    CRect postAreaRect(0, clientRect.Height() / 2, clientRect.Width(), clientRect.Height());

    // 1. 게시판 콘텐츠의 총 높이 계산
    float totalHeight = 20.0f + 30.0f + 10.0f;
    totalHeight += m_posts.size() * 50.0f; 
    m_nPostTotalHeight = static_cast<int>(totalHeight);

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = m_nPostTotalHeight;

    si.nPage = (clientRect.Height() / 2) - headerAreaHeight;
    m_scrollPosts.SetScrollInfo(&si, TRUE);
}


void CCommunityDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int* pScrollPos = nullptr;
    int totalHeight = 0;
    int pageHeight = 0;
    CScrollBar* pTargetScrollBar = nullptr;


    CRect clientRect;

    GetClientRect(&clientRect);

    if (pScrollBar == &m_scrollNotices) {
        pScrollPos = &m_nNoticeScrollPos;
        totalHeight = m_nNoticeTotalHeight;
        pageHeight = clientRect.Height() / 2; 
        pTargetScrollBar = &m_scrollNotices;
    }
    else if (pScrollBar == &m_scrollPosts) {
        pScrollPos = &m_nPostScrollPos;
        totalHeight = m_nPostTotalHeight;
        pageHeight = clientRect.Height() / 2; 
        pTargetScrollBar = &m_scrollPosts;
    }
    else {
        CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
        return;
    }

    int oldPos = *pScrollPos;

    switch (nSBCode) {
        case SB_LINEUP:    *pScrollPos -= 20; break;
        case SB_LINEDOWN:  *pScrollPos += 20; break;
        case SB_PAGEUP:    *pScrollPos -= pageHeight; break;
        case SB_PAGEDOWN:  *pScrollPos += pageHeight; break;
        case SB_THUMBTRACK: *pScrollPos = nPos; break;
    }

    int maxScrollPos = totalHeight - pageHeight;
    if (maxScrollPos < 0) maxScrollPos = 0;
    *pScrollPos = max(0, min(*pScrollPos, maxScrollPos));

    if (oldPos != *pScrollPos) {
        pTargetScrollBar->SetScrollPos(*pScrollPos);
        Invalidate();
    }
}

BOOL CCommunityDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    CRect clientRect;
    GetClientRect(&clientRect);
    CRect topHalf(0, 0, clientRect.Width(), clientRect.Height() / 2);


    if (topHalf.PtInRect(pt)) {

        int oldPos = m_nNoticeScrollPos;
        m_nNoticeScrollPos -= zDelta / 2; 

        int maxScrollPos = m_nNoticeTotalHeight - topHalf.Height();
        if (maxScrollPos < 0) maxScrollPos = 0;
        m_nNoticeScrollPos = max(0, min(m_nNoticeScrollPos, maxScrollPos));
        
        if (oldPos != m_nNoticeScrollPos) {
            m_scrollNotices.SetScrollPos(m_nNoticeScrollPos);
            Invalidate();
        }
    }
    else {
        // 게시판 스크롤
        int oldPos = m_nPostScrollPos;
        m_nPostScrollPos -= zDelta / 2;

        CRect bottomHalf(0, topHalf.bottom, clientRect.Width(), clientRect.Height());
        int maxScrollPos = m_nPostTotalHeight - bottomHalf.Height();
        if (maxScrollPos < 0) maxScrollPos = 0;
        m_nPostScrollPos = max(0, min(m_nPostScrollPos, maxScrollPos));

        if (oldPos != m_nPostScrollPos) {
            m_scrollPosts.SetScrollPos(m_nPostScrollPos);
            Invalidate();
        }
    }

    return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}