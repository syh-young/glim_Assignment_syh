#pragma once
#include <afxwin.h>
#define WM_CANVAS_CLICK (WM_APP + 1)
#define WM_CANVAS_POINT_MOVED (WM_APP + 2)

class CCanvasCtrl : public CStatic
{
private:
    CPoint m_pts[3];
    int m_count=0;
    bool m_hasGarden=false;
    CPoint m_center;
    double m_R=0.0;
    int m_pointR=10;
    int m_gardenThick=1;
    int  m_dragIndex=-1;//드레그 조건
    int  HitTestPointIndex(CPoint pt) const;
    bool m_dragging=false;
public:
    DECLARE_DYNAMIC(CCanvasCtrl)
    CCanvasCtrl(){}
    void SetFirstPoint(CPoint p,bool has);
    void SetPoints3(const CPoint* pts,int count);
    void SetPointRadius(int r);
    void SetGardenThick(int t);
    int GetDragIndex()const{ return m_dragIndex; }
    void ClearAll();
protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnLButtonDown(UINT nFlags,CPoint point);
    afx_msg void OnMouseMove(UINT nFlags,CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags,CPoint point);
    afx_msg void OnPaint();
    CPoint m_p1;
    bool m_hasP1 = false;
};