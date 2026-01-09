#pragma once
#include "CanvasCtrl.h"


// 랜덤 쓰레드 tick은 충돌 방지로 +3 사용 +4는 한번 랜덤이동 하고나서 랜덤이동 다시 하게할려고 넣음
#define WM_RANDOM_TICK (WM_APP + 3)
#define WM_RANDOM_DONE (WM_APP + 4)
// 쓰레드 에서 UI로 전달할 점 3개 정보
struct RandomTickPayload
{
    CPoint pts[3];
};

// 쓰레드 파라미터
struct RandomThreadParam
{
    HWND hDlg;
    int w;
    int h;
    int margin;
    volatile bool*pStop;
};

class CglimsyhDlg : public CDialogEx
{
public:
    CglimsyhDlg(CWnd*pParent=nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_GLIM_SYH_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange*pDX);

protected:
    HICON m_hIcon;
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID,LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();

    // Canvas 메시지
    afx_msg LRESULT OnCanvasClick(WPARAM wParam,LPARAM lParam);
    afx_msg LRESULT OnCanvasPointMoved(WPARAM wParam,LPARAM lParam);

    // 버튼
    afx_msg void OnBnClickedBtnReset();
    afx_msg void OnBnClickedBtnRandomMove();   // IDC_BTN_RANDOM에 이미 매핑된 그 함수명 유지

    // 랜덤 쓰레드 tick 메시지
    afx_msg LRESULT OnRandomTick(WPARAM wParam,LPARAM lParam);
    afx_msg LRESULT OnRandomDone(WPARAM wParam,LPARAM lParam);
    // 랜덤 쓰레드 엔트리
    static UINT RandomThreadProc(LPVOID pParam);

    DECLARE_MESSAGE_MAP()

private:
    CCanvasCtrl m_canvas;
    int m_clickCount=0;
    CPoint m_pts[3];

    // 랜덤 이동 쓰레드 상태
    CWinThread* m_pRandomThread=nullptr;
    volatile bool m_randomRunning=false; //실행 중이면 랜덤 이동 버튼 무시
    volatile bool m_stopRandom=false;    //초기화 누르면 true로 만들어 즉시 중단
public:
    afx_msg void OnEnChangeEditPointR();
};
