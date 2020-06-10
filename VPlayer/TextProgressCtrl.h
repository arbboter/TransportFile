#pragma once


// CTextProgressCtrl

class CTextProgressCtrl : public CProgressCtrl
{
	DECLARE_DYNAMIC(CTextProgressCtrl)

public:
    //���������Ȳ�����ɫ
    COLORREF m_prgsColor;
    //������������ಿ����ɫ
    COLORREF m_freeColor;
    //���Ȳ���������ɫ
    COLORREF m_prgsTextColor;
    //�հײ���������ɫ
    COLORREF m_freeTextColor;
    // ��ʾ�ı�
    // ��ǰ�����ı�
    CString  m_strCurText;
    // �ܽ����ı�
    CString  m_strTotalText;
    // ��ʾ�ı�����ǰ�����ı�+�ܽ����ı�
    CString  m_strText;

    //����������Сֵ��ͨ����0
    int  m_iMin;
    //�����������ֵ��ͨ����100
    int  m_iMax;
    //��ǰ�Ľ���
    int  m_iPos;
    //���������
    int  m_nBarWidth;

public:
    // Sets the current position within the set range of the control.
    int SetPos(_In_ int nPos);

    // �����ı�
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


