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

    //�����ȼ���Ҫ��ʾ�Ľ�������ȣ������ͬһ���ȶ�λ��ƴ���
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
    //���������Ȳ�����ɫ
    m_prgsColor = RGB(0, 128, 0);
    //������������ಿ����ɫ
    m_freeColor = RGB(192, 192, 192);;
    //���Ȳ���������ɫ
    m_prgsTextColor = RGB(255, 69, 0);
    //�հײ���������ɫ
    m_freeTextColor = RGB(255, 69, 0);

    //����������Сֵ��ͨ����0
    m_iMin = 0;
    //�����������ֵ��ͨ����100
    m_iMax = 100;
    //��ǰ�Ľ���
    m_iPos = 0;
    //���������
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
    //�����ж������Ƿ���Ч
    if (m_iMin >= m_iMax) 
    {
        return;
    }

    CPaintDC dc(this); // device context for painting
    // ��Ϊ��ͼ��Ϣ���� CProgressCtrl::OnPaint()


    //��ȡ��Ч�Ľ�������λ�úʹ�С
    CRect LeftRect, RightRect, ClientRect;
    GetClientRect(ClientRect);
    LeftRect = RightRect = ClientRect;

    //������ʾ���ȵı���
    double Fraction = (double)(m_iPos - m_iMin) / ((double)(m_iMax - m_iMin));

    //���������������е���Ч����
    LeftRect.right = LeftRect.left + (int)((LeftRect.right - LeftRect.left) * Fraction);
    dc.FillSolidRect(LeftRect, m_prgsColor);

    //����ʣ�����
    RightRect.left = LeftRect.right;
    dc.FillSolidRect(RightRect, m_freeColor);

    // �����ı�
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

    //�������ֱ�����ɫΪ͸��
    dc.SetBkMode(TRANSPARENT);

    //Ϊ���ܹ��ڽ��Ⱥ�ʣ���������ʾ��ͬ��ɫ�����壬��Ҫ�ֱ��������ߵ�������ɫ����ͼ
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
    //�����������������ֻ��ʾһ��������ɫ����ô����ֱ�ӵ���
    dc.SetTextColor(m_freeTextColor);
    dc.DrawText(strPercent, ClientRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
#endif
}