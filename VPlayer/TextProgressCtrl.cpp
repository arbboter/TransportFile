// TextProgressCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "VPlayer.h"
#include "TextProgressCtrl.h"


int CTextProgressCtrl::SetPos(_In_ int nPos)
{
    if (!::IsWindow(m_hWnd))
    {
        return -1;
    }

    int nOldPos = m_iPos;
    m_iPos = nPos;

    CRect rect;
    GetClientRect(rect);

    //这里先计算要显示的进度条宽度，避免对同一进度多次绘制窗口
    double Fraction = (double)(m_iPos - m_iMin) / ((double)(m_iMax - m_iMin));
    int nBarWidth = (int) (Fraction * rect.Width());
    if (nBarWidth != m_nBarWidth)
    {
        m_nBarWidth = nBarWidth;
    }
    RedrawWindow();

    return nOldPos;
}

int CTextProgressCtrl::SetText(_In_ CString strText, _In_ int nPos)
{
    m_strText = strText;
    return SetPos(nPos);
}

int CTextProgressCtrl::SetText(_In_ CString strCurText, _In_ CString strTotalText, _In_ int nPos)
{
    m_strCurText = strCurText;
    m_strTotalText = strTotalText;
    m_strText = strCurText + L"-" + strTotalText;
    return SetPos(nPos);
}

int CTextProgressCtrl::SetCurText(_In_ CString strCurText, _In_ int nPos)
{
    m_strCurText = strCurText;
    m_strText = strCurText + L"-" + m_strTotalText;
    return SetPos(nPos);
}

void CTextProgressCtrl::SetRange(_In_ short nLower, _In_ short nUpper)
{
    m_iMax = nUpper;
    m_iMin = nLower;
    m_iPos = m_iMin;
    m_nBarWidth = 0;
}

// CTextProgressCtrl

IMPLEMENT_DYNAMIC(CTextProgressCtrl, CWnd)

CTextProgressCtrl::CTextProgressCtrl()
{
    //进度条进度部分颜色
    m_prgsColor = RGB(0, 128, 0);
    //进度条后面空余部分颜色
    m_freeColor = RGB(192, 192, 192);;
    //进度部分字体颜色
    m_prgsTextColor = RGB(255, 69, 0);
    //空白部分字体颜色
    m_freeTextColor = RGB(255, 69, 0);

    //进度条的最小值，通常是0
    m_iMin = 0;
    //进度条的最大值，通常是100
    m_iMax = 100;
    //当前的进度
    m_iPos = 0;
    //进度条宽度
    m_nBarWidth = 1024;
}

CTextProgressCtrl::~CTextProgressCtrl()
{
}


BEGIN_MESSAGE_MAP(CTextProgressCtrl, CWnd)
    ON_WM_PAINT()
END_MESSAGE_MAP()



// CTextProgressCtrl message handlers




void CTextProgressCtrl::OnPaint()
{
    //首先判断设置是否有效
    if (m_iMin >= m_iMax) 
    {
        return;
    }

    CPaintDC dc(this); // device context for painting
    // 不为绘图消息调用 CProgressCtrl::OnPaint()


    //获取有效的进度条的位置和大小
    CRect LeftRect, RightRect, ClientRect;
    GetClientRect(ClientRect);
    LeftRect = RightRect = ClientRect;

    //计算显示进度的比例
    double Fraction = (double)(m_iPos - m_iMin) / ((double)(m_iMax - m_iMin));

    //绘制整个进度条中的有效进度
    LeftRect.right = LeftRect.left + (int)((LeftRect.right - LeftRect.left) * Fraction);
    dc.FillSolidRect(LeftRect, m_prgsColor);

    //绘制剩余进度
    RightRect.left = LeftRect.right;
    dc.FillSolidRect(RightRect, m_freeColor);

    // 设置文本
    CString strPercent;
    strPercent.Format(_T("%d%%"), (int)(Fraction*100.0));
    if(!m_strText.IsEmpty())
    {
        strPercent = m_strText + _T("(") + strPercent + _T(")");
        m_strText.Empty();
    }
    else if(m_iPos == m_iMin)
    {
        strPercent = L"";
    }

    //设置文字背景颜色为透明
    dc.SetBkMode(TRANSPARENT);

    //为了能够在进度和剩余进度中显示不同颜色的字体，需要分别设置两边的字体颜色并绘图
#if 0
    CRgn rgn;
    rgn.CreateRectRgn(LeftRect.left, LeftRect.top, LeftRect.right, LeftRect.bottom);
    dc.SelectClipRgn(&rgn);
    dc.SetTextColor(m_prgsTextColor);
    dc.DrawText(strPercent, ClientRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    rgn.DeleteObject();
    rgn.CreateRectRgn(RightRect.left, RightRect.top, RightRect.right, RightRect.bottom);
    dc.SelectClipRgn(&rgn);
    dc.SetTextColor(m_freeTextColor);
    dc.DrawText(strPercent, ClientRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

#else
    //如果在整个进度条中只显示一种字体颜色，那么可以直接调用
    dc.SetTextColor(m_freeTextColor);
    dc.DrawText(strPercent, ClientRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
#endif
}