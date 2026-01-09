
// glim_syhDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "glim_syh.h"
#include "glim_syhDlg.h"
#include "afxdialogex.h"
#include <random>
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CglimsyhDlg 대화 상자



CglimsyhDlg::CglimsyhDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GLIM_SYH_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CglimsyhDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CglimsyhDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_CANVAS_POINT_MOVED, &CglimsyhDlg::OnCanvasPointMoved)
	ON_MESSAGE(WM_CANVAS_CLICK, &CglimsyhDlg::OnCanvasClick)
	ON_MESSAGE(WM_RANDOM_TICK, &CglimsyhDlg::OnRandomTick)
	ON_BN_CLICKED(IDC_BTN_RESET, &CglimsyhDlg::OnBnClickedBtnReset)
	ON_BN_CLICKED(IDC_BTN_RANDOM, &CglimsyhDlg::OnBnClickedBtnRandomMove)
	ON_MESSAGE(WM_RANDOM_DONE, &CglimsyhDlg::OnRandomDone)
	//ON_STN_CLICKED(IDC_STATIC_P3, &CglimsyhDlg::OnStnClickedStaticP3) //클릭되는거 디버깅 할때 필요해서 일단 놔둠
	ON_EN_CHANGE(IDC_EDIT_POINT_R, &CglimsyhDlg::OnEnChangeEditPointR)
END_MESSAGE_MAP()


// CglimsyhDlg 메시지 처리기

BOOL CglimsyhDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	m_canvas.SubclassDlgItem(IDC_PIC_CANVAS, this);
	SetDlgItemText(IDC_STATIC_P1, L"p1: (-, -)");
	SetDlgItemText(IDC_STATIC_P2, L"p2: (-, -)");
	SetDlgItemText(IDC_STATIC_P3, L"p3: (-, -)");

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

static bool IsCollinear(const CPoint& a, const CPoint& b, const CPoint& c)
{
	// 외접원 계산 0이면 직선
	double ax = a.x, ay = a.y;
	double bx = b.x, by = b.y;
	double cx = c.x, cy = c.y;
	double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
	return std::fabs(d) < 1e-9;
}

void CglimsyhDlg::OnBnClickedBtnRandomMove()
{
	// 3점(정원) 상태 아니면 무시
	if (m_clickCount < 3)
		return;

	// 실행 중이면 무시 (요구사항: 무시)
	if (m_randomRunning)
		return;

	// 캔버스 크기
	CRect rc;
	m_canvas.GetClientRect(&rc);

	// 점 반지름이 캔버스 밖으로 안 나가게 margin
	// (정확히 하려면 Edit에서 반지름 읽어서 margin=r+2 정도로 해도 됨)
	int margin = 20;

	// stop 플래그 초기화 + running 시작
	m_stopRandom = false;
	m_randomRunning = true;

	// 쓰레드 파라미터 준비 (쓰레드에서 unique_ptr로 delete 처리함)
	RandomThreadParam* p = new RandomThreadParam();
	p->hDlg = this->GetSafeHwnd();
	p->w = rc.Width();
	p->h = rc.Height();
	p->margin = margin;
	p->pStop = &m_stopRandom;

	// 쓰레드 시작
	m_pRandomThread = AfxBeginThread(RandomThreadProc, p, THREAD_PRIORITY_NORMAL, 0, 0, nullptr);

	// 안전장치<<쓰레드 실패하면 원상복구하기...
	if (m_pRandomThread == nullptr)
	{
		m_randomRunning = false;
		m_stopRandom = false;
		delete p;
	}
}


void CglimsyhDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CglimsyhDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CglimsyhDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CglimsyhDlg::OnCanvasClick(WPARAM wParam, LPARAM lParam)
{
	int x = (int)wParam;
	int y = (int)lParam;

	if (m_clickCount >= 3)
		return 0; // 4번째부터 무시

	m_pts[m_clickCount] = CPoint(x, y);

	CString s;
	s.Format(L"p%d: (%d, %d)", m_clickCount + 1, x, y);

	if (m_clickCount == 0) {
		SetDlgItemText(IDC_STATIC_P1, s);
		m_canvas.SetFirstPoint(CPoint(x, y), true);
	}
	else if (m_clickCount == 1) {
		SetDlgItemText(IDC_STATIC_P2, s);
	}
	else {
		SetDlgItemText(IDC_STATIC_P3, s);
	}

	//count 증가
	m_clickCount++;

	// Edit에서 점 반지름 읽어서 캔버스에 전달
	CString tmp;
	GetDlgItemText(IDC_EDIT_POINT_R, tmp);
	int r = _wtoi(tmp);
	m_canvas.SetPointRadius(r);
	CString tmp2;
	GetDlgItemText(IDC_EDIT_CIRCLE_THICK, tmp2);
	int thick = _wtoi(tmp2);
	m_canvas.SetGardenThick(thick);
	// 점/정원 상태 전달
	m_canvas.SetPoints3(m_pts, m_clickCount);

	return 0;
}

LRESULT CglimsyhDlg::OnCanvasPointMoved(WPARAM wParam, LPARAM lParam)
{
	int idx = (int)wParam;
	int x = (int)(short)LOWORD(lParam);
	int y = (int)(short)HIWORD(lParam);

	if (idx < 0 || idx > 2) return 0;
	if (m_clickCount < 3)   return 0;   // 3점 다 찍힌 이후에만 드래그 의미 있음

	//  다이얼로그가 좌표 업데이트
	m_pts[idx] = CPoint(x, y);

	//  Static 텍스트 갱신
	CString s;
	s.Format(L"p%d: (%d, %d)", idx + 1, x, y);
	if (idx == 0) SetDlgItemText(IDC_STATIC_P1, s);
	else if (idx == 1) SetDlgItemText(IDC_STATIC_P2, s);
	else SetDlgItemText(IDC_STATIC_P3, s);

	// 캔버스에 다시 전달 → 정원 재계산 + 다시 그리기
	m_canvas.SetPoints3(m_pts, 3);

	return 0;
}

void CglimsyhDlg::OnBnClickedBtnReset()
{
	// 다이얼로그 쪽 상태 초기화
	m_clickCount = 0;
	for (int i = 0; i < 3; i++) m_pts[i] = CPoint(0, 0);

	// 좌표 표시 초기화
	SetDlgItemText(IDC_STATIC_P1, L"p1: (-, -)");
	SetDlgItemText(IDC_STATIC_P2, L"p2: (-, -)");
	SetDlgItemText(IDC_STATIC_P3, L"p3: (-, -)");

	// 캔버스(그림) 초기화
	m_canvas.ClearAll();
}

LRESULT CglimsyhDlg::OnRandomTick(WPARAM wParam, LPARAM lParam)
{
	// 스레드가 넘긴 payload
	RandomTickPayload* p = (RandomTickPayload*)wParam;
	if (!p) return 0;

	// 점 3개 갱신
	m_pts[0] = p->pts[0];
	m_pts[1] = p->pts[1];
	m_pts[2] = p->pts[2];

	// p1,2,3좌표값 갱신
	CString s;
	s.Format(L"p1: (%d, %d)", m_pts[0].x, m_pts[0].y); SetDlgItemText(IDC_STATIC_P1, s);
	s.Format(L"p2: (%d, %d)", m_pts[1].x, m_pts[1].y); SetDlgItemText(IDC_STATIC_P2, s);
	s.Format(L"p3: (%d, %d)", m_pts[2].x, m_pts[2].y); SetDlgItemText(IDC_STATIC_P3, s);

	//점 3개 다시찍어서 정원 만들기
	m_canvas.SetPoints3(m_pts, 3);

	delete p; //payload 안하기
	return 0;
}

UINT CglimsyhDlg::RandomThreadProc(LPVOID pParam)
{
	std::unique_ptr<RandomThreadParam> param((RandomThreadParam*)pParam);
	if (!param) return 0;

	HWND hDlg = param->hDlg;
	int w = param->w;
	int h = param->h;
	int margin = param->margin;
	volatile bool* pStop = param->pStop;

	static std::mt19937 rng((unsigned)std::random_device{}());

	int minX = margin;
	int maxX = (w - 1) - margin;
	int minY = margin;
	int maxY = (h - 1) - margin;

	if (minX >= maxX || minY >= maxY)
		return 0;

	std::uniform_int_distribution<int> dx(minX, maxX);
	std::uniform_int_distribution<int> dy(minY, maxY);

	auto isCollinear = [](const CPoint& a, const CPoint& b, const CPoint& c) -> bool {
		double ax = a.x, ay = a.y;
		double bx = b.x, by = b.y;
		double cx = c.x, cy = c.y;
		double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
		return std::fabs(d) < 1e-9;
		};

	// 0.5초마다 10번
	for (int i = 0; i < 10; i++)
	{
		if (pStop && *pStop) break;

		CPoint p0, p1, p2;
		for (int attempt = 0; attempt < 30; attempt++)
		{
			p0 = CPoint(dx(rng), dy(rng));
			p1 = CPoint(dx(rng), dy(rng));
			p2 = CPoint(dx(rng), dy(rng));
			if (!isCollinear(p0, p1, p2)) break;
		}

		RandomTickPayload* payload = new RandomTickPayload();
		payload->pts[0] = p0;
		payload->pts[1] = p1;
		payload->pts[2] = p2;

		::PostMessage(hDlg, WM_RANDOM_TICK, (WPARAM)payload, 0);

		// 0.5초 마다 바꾸라 해서 대기 임
		for (int t = 0; t < 50; t++)
		{
			if (pStop && *pStop) break;
			::Sleep(10);
		}
	}
	::PostMessage(hDlg, WM_RANDOM_DONE, 0, 0);
	return 0;
}

LRESULT CglimsyhDlg::OnRandomDone(WPARAM wParam, LPARAM lParam)
{
	m_randomRunning = false;
	return 0;
}

void CglimsyhDlg::OnEnChangeEditPointR()
{
	
}
