#include "pch.h"
#include "CanvasCtrl.h"
#include <cmath>
#include <algorithm>

IMPLEMENT_DYNAMIC(CCanvasCtrl, CStatic)

BEGIN_MESSAGE_MAP(CCanvasCtrl, CStatic)
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_PAINT()
END_MESSAGE_MAP()

void CCanvasCtrl::SetPointRadius(int r)
{
    if (r < 1) r = 1;
    if (r > 200) r = 200; // 안전상 상한 바꾸기 가능 
    m_pointR = r;
    Invalidate(FALSE);
}

void CCanvasCtrl::SetGardenThick(int t)
{
    if (t < 1) t = 1;
    if (t > 30) t = 30;   // 두껍기 상한 바꾸기 가능
    m_gardenThick = t;
    Invalidate(FALSE);
}

int CCanvasCtrl::HitTestPointIndex(CPoint pt) const
{
    // 점을 누르기 쉬우라고 범위 설정
    int r = m_pointR + 2;
    long long r2 = 1LL * r * r;

    for (int i = 0; i < m_count; i++)
    {
        long long dx = (long long)pt.x - (long long)m_pts[i].x;
        long long dy = (long long)pt.y - (long long)m_pts[i].y;

        if (dx * dx + dy * dy <= r2)
            return i; // 0~2 중 하나
    }
    return -1;
}


// MoveTo/LineTo로 원을 다각형처럼 근사해서 그렸음
static void DrawCircleByPolyline(CDC& dc, CPoint c, double r, int seg)
{
    if (r <= 0.0) return;
    if (seg < 12) seg = 12;

	const double twoPi=6.283185307179586;//2파이 3.141592653589793*2한값임
    const double step=twoPi/seg;

    for(int i=0;i<=seg;i++){
        double a=step*i;
        int x=(int)lround(c.x+r*cos(a));
        int y=(int)lround(c.y+r*sin(a));

        if(i == 0)dc.MoveTo(x,y);
        else dc.LineTo(x,y);
    }
}

static void FillCircleByScanlines(CDC& dc, CPoint c, int r)
{
    if(r<=0)return;

    //내부체움
    for (int dy = -r; dy <= r; dy++){
        // x = sqrt(r^2 - y^2) 공식 기반 코드임
        double inside=(double)r*r-(double)dy*dy;
        if (inside<0)continue;

        int dx = (int)lround(sqrt(inside));
        dc.MoveTo(c.x-dx,c.y+dy);
        dc.LineTo(c.x+dx,c.y+dy);
    }
}

void CCanvasCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    //3점이 이미 찍혀 있는경우 1, 2, 3점 중 하나 뭘 집었는지 판별
    if (m_count==3){
        int idx = HitTestPointIndex(point);
        if (idx != -1){
            m_dragIndex=idx;   // 1, 2, 3 중 어떤 점을 잡았는지 저장
            m_dragging=true;
            SetCapture();
            return;
        }
    }

    // 점 위 클릭이 아니면 기존처럼 클릭 좌표 전달
    GetParent()->PostMessage(WM_CANVAS_CLICK, (WPARAM)point.x,(LPARAM)point.y);
    CStatic::OnLButtonDown(nFlags, point);
}

void CCanvasCtrl::SetFirstPoint(CPoint p, bool has)
{
    m_p1=p;
    m_hasP1=has;
    Invalidate(FALSE);
}

void CCanvasCtrl::SetPoints3(const CPoint* pts,int count)
{
    m_count = (count < 3 ? count : 3);
    for (int i =0;i<m_count;i++)m_pts[i]=pts[i];

    // 3점 다 모이면 3점 지나는 원 계산
    m_hasGarden = false;

    if (m_count == 3)
    {
        double ax = m_pts[0].x, ay = m_pts[0].y;
        double bx = m_pts[1].x, by = m_pts[1].y;
        double cx = m_pts[2].x, cy = m_pts[2].y;

        double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
        if (fabs(d) > 1e-9) // 세 점이 일직선이면 외접원 불가 점을 가끔 이동하면서 세 점이 일직선으로 왔을 때 정원이 안그려지면 이것 때문임
        {
            double a2 = ax * ax + ay * ay;
            double b2 = bx * bx + by * by;
            double c2 = cx * cx + cy * cy;

            double ux = (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / d;
            double uy = (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / d;

            m_center = CPoint((int)lround(ux), (int)lround(uy));
            double dx = ux - ax, dy = uy - ay;
            m_R = sqrt(dx * dx + dy * dy);

            m_hasGarden = true;
        }
    }

    Invalidate(FALSE);
}

void CCanvasCtrl::OnPaint()
{
    CPaintDC dc(this);

    // 배경
    CRect rc;
    GetClientRect(&rc);

    int saved = dc.SaveDC();
    dc.IntersectClipRect(rc);

    dc.FillSolidRect(rc, RGB(255, 255, 255));

    // 검정 펜(두께 1) - 두께 입력은 다음 단계에서 연결
    CPen penPoint(PS_SOLID, 1, RGB(0, 0, 0)); //점                
    CPen penGarden(PS_SOLID, m_gardenThick, RGB(0, 0, 0)); //3점 지나는 원    
    CPen* oldPen = dc.SelectObject(&penPoint);

    // 클릭점 3개를 작은 원으로 표시
    const int pointR = m_pointR;
    for (int i = 0; i < m_count; i++)
    {
        // 내부를 선들로 채워서 원 내부를 칠함
        FillCircleByScanlines(dc, m_pts[i],pointR);

        //외곽선
        DrawCircleByPolyline(dc, m_pts[i],(double)pointR,120);
    }

    //점 3개를 지나는 원 그리기
    if (m_hasGarden)
    {
        dc.SelectObject(&penGarden);
        DrawCircleByPolyline(dc, m_center, m_R, 240);
        dc.SelectObject(&penPoint); //점 펜으로 변경
    }

    dc.SelectObject(oldPen);

    dc.RestoreDC(saved);
}

void CCanvasCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_dragging)
    {
        m_dragging=false;
        m_dragIndex=-1;
        ReleaseCapture();
    }
    CStatic::OnLButtonUp(nFlags, point);
}

void CCanvasCtrl::OnMouseMove(UINT nFlags,CPoint point)
{
    if (m_dragging && m_dragIndex >= 0 && m_dragIndex < 3)
    {
        // lparam에 x,y를 넣어서 보냄
        GetParent()->SendMessage(
            WM_CANVAS_POINT_MOVED,
            (WPARAM)m_dragIndex,
            (LPARAM)MAKELPARAM(point.x,point.y)
        );
        return;
    }

    CStatic::OnMouseMove(nFlags, point);
}

void CCanvasCtrl::ClearAll()
{
    // 드래그 중인지 상태확인 하고 해제
    if(GetCapture()==this)
        ReleaseCapture();

    m_dragging=false;
    m_dragIndex=-1;

    //점 정원 초기화
    m_count=0;
    m_hasGarden=false;
    m_center=CPoint(0, 0);
    m_R=0.0;

    //남은 변수 정리
    m_hasP1=false;
    m_p1=CPoint(0, 0);

    Invalidate(FALSE);
}


