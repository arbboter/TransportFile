#pragma once


// CTextProgressCtrl

class CTextProgressCtrl : public CProgressCtrl
{
	DECLARE_DYNAMIC(CTextProgressCtrl)

public:
    //进度条进度部分颜色
    COLORREF m_prgsColor;
    //进度条后面空余部分颜色
    COLORREF m_freeColor;
    //进度部分字体颜色
    COLORREF m_prgsTextColor;
    //空白部分字体颜色
    COLORREF m_freeTextColor;
    // 显示文本
    // 当前进度文本
    CString  m_strCurText;
    // 总进度文本
    CString  m_strTotalText;
    // 显示文本：当前进度文本+总进度文本
    CString  m_strText;

    //进度条的最小值，通常是0
    int  m_iMin;
    //进度条的最大值，通常是100
    int  m_iMax;
    //当前的进度
    int  m_iPos;
    //进度条宽度
    int  m_nBarWidth;

public:
    // Sets the current position within the set range of the control.
    int SetPos(_In_ int nPos);

    // 设置文本
    int SetText(_In_ CString strText, _In_ int nPos);
    int SetText(_In_ CString strCurText, _In_ CString strTotalText, _In_ int nPos);
    int SetCurText(_In_ CString strCurText, _In_ int nPos);

    // Sets range of values for the control. (16-bit limit)
    void SetRange(_In_ short nLower, _In_ short nUpper);

public:
	CTextProgressCtrl();
	virtual ~CTextProgressCtrl();

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


