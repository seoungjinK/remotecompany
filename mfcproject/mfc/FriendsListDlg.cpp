#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "FriendsListDlg.h"
#include <gdiplus.h> 
#include "TheMoonDlg.h"
#include "UserInfoDlg.h" 


using namespace Gdiplus;


IMPLEMENT_DYNAMIC(CFriendsListDlg, CDialogEx)

CFriendsListDlg::CFriendsListDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_FRIENDS_LIST_DIALOG, pParent)
    , m_nScrollPos(0)             
    , m_nTotalContentHeight(0)    
{
    
}

CFriendsListDlg::~CFriendsListDlg()
{
}

void CFriendsListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CFriendsListDlg::InitializeAndLoadData(int userId)
{
    m_nCurrentUserId = userId;
    LoadFriendsListFromDB();
}


BEGIN_MESSAGE_MAP(CFriendsListDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_EN_CHANGE(1003, &CFriendsListDlg::OnEnChangeSearch)
    ON_WM_SIZE()         
    ON_WM_VSCROLL()      
    ON_WM_MOUSEWHEEL()   
END_MESSAGE_MAP()



BOOL CFriendsListDlg::ConnectDatabase()
{
    if (m_db.IsOpen())
        return TRUE;

    CString strConnect;
    strConnect.Format(_T("DSN=%s;UID=%s;PWD=%s;"), _T("Mysql_test"), _T("root"), _T("1234"));

    try
    {
        if (!m_db.OpenEx(strConnect, CDatabase::noOdbcDialog)) {
            AfxMessageBox(_T("MySQL 데이터베이스 연결 실패."));
            return FALSE;
        }
        return TRUE;
    }
    catch (CDBException* e)
    {
        TCHAR szError[1024];
        if (e->GetErrorMessage(szError, 1024))
        {
            AfxMessageBox(szError);
        }
        else
        {
            AfxMessageBox(_T("데이터베이스 오류가 발생했으나 상세 메시지를 가져올 수 없습니다."));
        }

        e->Delete();
        return FALSE;
    }
}

//2. 친구 목록 로드 함수 추가
void CFriendsListDlg::LoadFriendsListFromDB()
{
    if (!ConnectDatabase()) return;

    m_friendsByDept.clear(); // 기존 목록 초기화

    CString strSQL;
    strSQL.Format(
        _T("SELECT u.user_id, e.full_name, e.department ")
        _T("FROM users u JOIN employees e ON u.employee_id = e.employee_id ")
        _T("WHERE u.user_id != %d ") // 자기 자신은 목록에서 제외
        _T("ORDER BY e.department, e.full_name"), m_nCurrentUserId);

    CRecordset rs(&m_db);
    try
    {
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            CString strUserId, fullName, department;
            rs.GetFieldValue(_T("user_id"), strUserId);
            rs.GetFieldValue(_T("full_name"), fullName);
            rs.GetFieldValue(_T("department"), department);

            FriendInfo friendInfo;
            friendInfo.userId = _ttoi(strUserId);
            friendInfo.fullName = fullName;
            friendInfo.department = department;

            // map에 부서별로 추가
            m_friendsByDept[department].push_back(friendInfo);

            rs.MoveNext();
        }
    }
    catch (CDBException* e)
    {
        AfxMessageBox(e->m_strError);
        e->Delete();
    }
    rs.Close();

    m_filteredFriendsByDept = m_friendsByDept;

    Invalidate(); // 화면을 다시 그리도록 요청
}
void CFriendsListDlg::OnPaint()
{
    CPaintDC dc(this);
    Graphics graphics(dc.GetSafeHdc());
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

    CRect clientRect;
    GetClientRect(&clientRect);
    int margin = 10;
    int searchAreaHeight = margin + 25 + margin;
    int scrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);

    // 목록이 그려질 영역에 클리핑 설정
    RectF listAreaRect(0.0f, (float)searchAreaHeight, (float)(clientRect.Width() - scrollBarWidth), (float)(clientRect.Height() - searchAreaHeight));
    graphics.SetClip(listAreaRect);

    SolidBrush backBrush(Color::White);
    graphics.FillRectangle(&backBrush, listAreaRect);

    // --- 폰트, 브러시, 펜 정의 ---
    Gdiplus::Font deptFont(L"맑은 고딕", 10, FontStyleBold, UnitPoint);
    Gdiplus::Font nameFont(L"맑은 고딕", 11, FontStyleRegular, UnitPoint);
    Gdiplus::Font buttonFont(L"맑은 고딕", 8, FontStyleRegular);
    SolidBrush deptBackBrush(Color(245, 245, 245));
    SolidBrush textBrush(Color(0, 0, 0));
    SolidBrush btnTextBrush(Color(80, 80, 80));
    Pen linePen(Color(230, 230, 230));
    Pen btnPen(Color(200, 200, 200));

    // 그리기를 시작할 Y 좌표를 검색창 아래로 설정
    float currentY = (float)searchAreaHeight;

    currentY -= m_nScrollPos;

    int buttonWidth = 40;
    int buttonHeight = 22;
    int buttonMargin = 5;

    // 필터링된 목록을 기준으로 그리기
    for (auto& pair : m_filteredFriendsByDept)
    {
        // 부서 헤더 그리기
        float deptHeaderHeight = 30.0f;
        RectF deptRect(0.0f, currentY, (float)listAreaRect.Width, deptHeaderHeight);
        graphics.FillRectangle(&deptBackBrush, deptRect);
        graphics.DrawString(pair.first, -1, &deptFont, PointF((float)margin, currentY + 7.0f), &textBrush);
        currentY += deptHeaderHeight;

        // 친구 목록 그리기
        for (FriendInfo& info : pair.second)
        {
            float itemHeight = 60.0f;
            info.rectItem.SetRect(0, (int)currentY, (int)listAreaRect.Width, (int)(currentY + itemHeight));

            // 이름 그리기
            graphics.DrawString(info.fullName, -1, &nameFont, PointF((float)margin, currentY + 20.0f), &textBrush);

            // 버튼 위치 계산 및 그리기
            int infoBtnX = (int)listAreaRect.Width - margin - buttonWidth;
            int btnY = (int)(currentY + (itemHeight - buttonHeight) / 2.0f);
            info.rectInfoButton.SetRect(infoBtnX, btnY, infoBtnX + buttonWidth, btnY + buttonHeight);
            graphics.DrawRectangle(&btnPen, (float)info.rectInfoButton.left, (float)info.rectInfoButton.top, (float)info.rectInfoButton.Width(), (float)info.rectInfoButton.Height());
            graphics.DrawString(L"정보", -1, &buttonFont, PointF((float)info.rectInfoButton.left + 9.0f, (float)info.rectInfoButton.top + 4.0f), &btnTextBrush);

            int chatBtnX = info.rectInfoButton.left - buttonMargin - buttonWidth;
            info.rectChatButton.SetRect(chatBtnX, btnY, chatBtnX + buttonWidth, btnY + buttonHeight);
            graphics.DrawRectangle(&btnPen, (float)info.rectChatButton.left, (float)info.rectChatButton.top, (float)info.rectChatButton.Width(), (float)info.rectChatButton.Height());
            graphics.DrawString(L"채팅", -1, &buttonFont, PointF((float)info.rectChatButton.left + 9.0f, (float)info.rectChatButton.top + 4.0f), &btnTextBrush);

            // 구분선
            graphics.DrawLine(&linePen, (float)margin, currentY + itemHeight - 1.0f, listAreaRect.Width - margin, currentY + itemHeight - 1.0f);

            currentY += itemHeight;
        }
    }
}

void CFriendsListDlg::OnLButtonDown(UINT nFlags, CPoint point)
{

    for (const auto& pair : m_filteredFriendsByDept)
    {
        for (const FriendInfo& info : pair.second)
        {
            // 채팅 버튼 클릭 부분
            if (info.rectChatButton.PtInRect(point))
            {
                int roomId = GetOrCreateDirectChatRoom(info.userId);
                if (roomId != -1)
                {
                    GetParent()->SendMessage(WM_USER_START_CHAT, static_cast<WPARAM>(roomId));
                }
                return;
            }

            // 정보 버튼 클릭 부분
            if (info.rectInfoButton.PtInRect(point))
            {
                CUserInfoDlg dlg(info.userId, this);
                dlg.DoModal();
                return;
            }
        }
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}


// CFriendsListDlg 메시지 처리기
int CFriendsListDlg::GetOrCreateDirectChatRoom(int targetUserId)
{
    if (!ConnectDatabase()) return -1;

    int roomId = -1;

    CString strSQL;
    strSQL.Format(
        _T("SELECT p1.room_id FROM participants p1 ")
        _T("JOIN participants p2 ON p1.room_id = p2.room_id ")
        _T("JOIN chat_rooms cr ON p1.room_id = cr.room_id ")
        _T("WHERE cr.room_type = 'direct' AND p1.user_id = %d AND p2.user_id = %d"),
        m_nCurrentUserId, targetUserId);

    CRecordset rs(&m_db);
    try
    {
        rs.Open(CRecordset::forwardOnly, strSQL);
        if (!rs.IsEOF())
        {
            CString strRoomId;
            rs.GetFieldValue(_T("room_id"), strRoomId);
            roomId = _ttoi(strRoomId);
        }
        rs.Close();
    }
    catch (CDBException* e)
    {
        e->Delete();
    }

    if (roomId == -1)
    {
        try
        {
            m_db.BeginTrans();

            m_db.ExecuteSQL(_T("INSERT INTO chat_rooms (room_type) VALUES ('direct')"));

            CRecordset rsLastId(&m_db);
            rsLastId.Open(CRecordset::forwardOnly, _T("SELECT LAST_INSERT_ID() as room_id"));
            if (!rsLastId.IsEOF())
            {
                CString strRoomId;
                rsLastId.GetFieldValue(_T("room_id"), strRoomId);
                roomId = _ttoi(strRoomId);
            }
            rsLastId.Close();

            if (roomId != -1)
            {
                CString strInsertParticipants;
                strInsertParticipants.Format(
                    _T("INSERT INTO participants (room_id, user_id) VALUES (%d, %d), (%d, %d)"),
                    roomId, m_nCurrentUserId, roomId, targetUserId);
                m_db.ExecuteSQL(strInsertParticipants);
            }

            m_db.CommitTrans();
        }
        catch (CDBException* e)
        {
            m_db.Rollback(); 
            AfxMessageBox(_T("채팅방 생성 중 오류가 발생했습니다."));
            e->Delete();
            return -1;
        }
    }
    return roomId;
}

BOOL CFriendsListDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_fontSearch.CreatePointFont(100, _T("맑은 고딕"));
    m_editSearch.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        CRect(0, 0, 0, 0), this, 1003); // ID: 1003
    m_editSearch.SetFont(&m_fontSearch);

    
    m_editSearch.SetCueBanner(_T("이름으로 검색"));
    CRect rectDummy(0, 0, 0, 0);
    m_scrollFriends.Create(SBS_VERT | WS_CHILD, rectDummy, this, 1004); 

    
    RepositionLayout();

    return TRUE;
}


void CFriendsListDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);
    if (nType == SIZE_MINIMIZED) return;

    RepositionLayout();
    UpdateScrollbar();
    Invalidate();
}

void CFriendsListDlg::RepositionLayout()
{
    CRect clientRect;
    GetClientRect(&clientRect);
    int margin = 10;
    int searchHeight = 25;
    int searchAreaHeight = margin + searchHeight + margin;
    int scrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);

    if (m_editSearch.GetSafeHwnd())
    {
        m_editSearch.MoveWindow(margin, margin, clientRect.Width() - margin * 2, searchHeight);
    }
    if (m_scrollFriends.GetSafeHwnd())
    {

        m_scrollFriends.MoveWindow(clientRect.right - scrollBarWidth, searchAreaHeight, scrollBarWidth, clientRect.Height() - searchAreaHeight);
        m_scrollFriends.ShowWindow(SW_SHOW);
    }
}

void CFriendsListDlg::OnEnChangeSearch()
{
    CString searchText;
    m_editSearch.GetWindowText(searchText);
    searchText.MakeLower(); 

    m_filteredFriendsByDept.clear(); 

    if (searchText.IsEmpty())
    {

        m_filteredFriendsByDept = m_friendsByDept;
    }
    else
    {
        for (const auto& pair : m_friendsByDept)
        {
            const CString& department = pair.first;
            const std::vector<FriendInfo>& friends = pair.second;

            for (const FriendInfo& info : friends)
            {
                CString lowerFullName = info.fullName;
                lowerFullName.MakeLower();

                if (lowerFullName.Find(searchText) != -1)
                {
                    m_filteredFriendsByDept[department].push_back(info);
                }
            }
        }
    }

    Invalidate(); 
}





void CFriendsListDlg::UpdateScrollbar()
{
    if (!m_scrollFriends.GetSafeHwnd()) return;

    CRect clientRect;
    GetClientRect(&clientRect);
    int searchAreaHeight = 10 + 25 + 10;


    float totalHeight = 0;
    for (const auto& pair : m_filteredFriendsByDept)
    {
        totalHeight += 30.0f; 
        totalHeight += pair.second.size() * 60.0f; 
    }
    m_nTotalContentHeight = static_cast<int>(totalHeight);

    // 스크롤바 정보 설정
    SCROLLINFO si = { 0 };
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = m_nTotalContentHeight;
    si.nPage = clientRect.Height() - searchAreaHeight;
    m_scrollFriends.SetScrollInfo(&si, TRUE);
}


// 3. OnVScroll 함수 (스크롤바 클릭/드래그 처리)
void CFriendsListDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    if (pScrollBar != &m_scrollFriends) {
        CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
        return;
    }

    int oldPos = m_nScrollPos;
    CRect clientRect;
    GetClientRect(&clientRect);
    int searchAreaHeight = 10 + 25 + 10;
    int pageHeight = clientRect.Height() - searchAreaHeight;

    switch (nSBCode) {
    case SB_LINEUP:    m_nScrollPos -= 20; break;
    case SB_LINEDOWN:  m_nScrollPos += 20; break;
    case SB_PAGEUP:    m_nScrollPos -= pageHeight; break;
    case SB_PAGEDOWN:  m_nScrollPos += pageHeight; break;
    case SB_THUMBTRACK: m_nScrollPos = nPos; break;
    }

    int maxScrollPos = m_nTotalContentHeight - pageHeight;
    if (maxScrollPos < 0) maxScrollPos = 0;
    m_nScrollPos = max(0, min(m_nScrollPos, maxScrollPos));

    if (oldPos != m_nScrollPos) {
        m_scrollFriends.SetScrollPos(m_nScrollPos);
        Invalidate();
    }
}


// 4. OnMouseWheel 함수 (마우스 휠 처리)
BOOL CFriendsListDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    int oldPos = m_nScrollPos;
    m_nScrollPos -= zDelta;

    CRect clientRect;
    GetClientRect(&clientRect);
    int searchAreaHeight = 10 + 25 + 10;
    int maxScrollPos = m_nTotalContentHeight - (clientRect.Height() - searchAreaHeight);
    if (maxScrollPos < 0) maxScrollPos = 0;
    m_nScrollPos = max(0, min(m_nScrollPos, maxScrollPos));

    if (oldPos != m_nScrollPos) {
        m_scrollFriends.SetScrollPos(m_nScrollPos);
        Invalidate();
    }

    return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}