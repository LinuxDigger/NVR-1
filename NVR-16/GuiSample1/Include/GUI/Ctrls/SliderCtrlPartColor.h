//
//  "$Id: SliderCtrl.h 280 2008-12-17 06:04:55Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//
//yaogang modify for playback 20170906

#if 0
������������ɫ:
��¼�� {	0,		174,	255,	128	},//COLOR_FRAMESELECTED
��¼�� {	180,	180,	180,	150	},//COLOR_CTRLTEXT
�ָ��� {	102,	102,	102,	128 },//COLOR_FRAME

#endif
#include "MultiTask/Mutex.h"
//#include "GUI/Ctrls/Static.h"


#ifndef __CTRL_SLIDER_PART_COLOR_H__
#define __CTRL_SLIDER_PART_COLOR_H__

class Range
{
friend VD_BOOL isInRange(int val, const Range &r);//val �Ƿ���this ��
friend VD_BOOL isInRangeWithoutStart(int val, const Range &r);
friend VD_BOOL isInRangeWithoutEnd(int val, const Range &r);
friend VD_BOOL isInRangeWithoutBoth(int val, const Range &r);
friend VD_BOOL operator ==(const Range &lhs, const Range &rhs);
friend VD_BOOL operator !=(const Range &lhs, const Range &rhs);

public:
	Range(int s, int e);
	~Range();
	VD_BOOL isInRange(const Range &r) const;//this �Ƿ���r ��
	VD_BOOL selfCheck() const;
	int Width() const;

	int start;
	int end;
};


class CSliderCtrlPartColor : public CItem
{
public:
	CSliderCtrlPartColor(VD_PCRECT pRect, CPage * pParent, int vmin = 0,int vmax = 100, int SplitLineMax=10, CTRLPROC vproc = NULL);
	virtual ~CSliderCtrlPartColor();
	void Draw();
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);

	int SetPos(int pos);
	int GetPos();
	int GetAt(int px, int py);//�Ƿ��ڻ������������
	
	void SetTrackerEnable(VD_BOOL b);
	void SetSplitLineNum(int n);
	void SetSplitLineColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ );
	void SetColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ );
	void SetBlankColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ );
	//��ָ������Ŵ���ʾ
	int SetDisplayRange(const Range &r);
	//ͬʱ�ı�����ߺͷָ�������
	//eg: playback module 24hr/2hr/1hr/30min
	int SetDisplayRange(const Range &r, int split_line_num);
	
	//������ɫ����(��¼���ʱ���)
	void SetColorRange(std::vector<Range> &vr);

	void Select(VD_BOOL flag)
	{
		SetFlag(IF_SELECTED, flag);
	}

private:	
	void DrawBackground();
	void DrawRange();
	void DrawSplitLine();
	void UpdateTracker();
	void DrawTracker();
	
private:
	CMutex m_Mutex;
	VD_BOOL m_bTracker; //����ʹ��, �Ƿ���ʾ���϶�

	int m_iSliderWidth; //����������= m_Rect.Width()
	Range m_sRealRange;//����[min, max]   ʵ������
	CTRLPROC m_onValueChanged;
	
	//�α�, ��ɫ= m_ColorBlank
	int curpos;//[min, max]
	int newpos;//[min, max] �϶��α�ʹ��
	VD_BOOL	track;			//���ڹ���
	int		tracker_width;	//��������
	int		tracker_offset;	//������ƫ��[0, m_iSliderWidth]
	int		tracker_pick;	//���������ʱ�������λ��

	//�ָ���(�̶�)
	//void InitSplitLine(int maxlines = 16, VD_COLORREF color = VD_RGB(232,232,232));
	int m_iSplitLineMax;
	int m_iSplitLineNum;//�ȷֽ�����24hr/24, 2hr/12, 1hr/12, 30min/10
	VD_COLORREF m_ColorSplitLine;
	//std::vector<CStatic*> m_vSplitLine;	

	//��ɫ����
	VD_COLORREF m_ColorBlank;	//�հ�������ɫ   ��¼��
	VD_COLORREF m_Color;		//��ɫ����	��¼��
	std::vector<Range> m_vRange;

	//��ָ������Ŵ���ʾ
	//eg: ��ʼ��ʾһ��24Сʱ, ��Ӧ�û��Ŵ���ʾ8-10��Сʱ����
	Range m_sDispRange;// [min, max] �Ӽ�, �ڲ�ʹ��, ��Ҫ�Ŵ���ʾ��һ������
	//int m_iZoomFactorFactor;/���ǽ����ϲ㴦��, �������Ŵ�ϵ�� , �ⲿ����. 24hr/1  2hr/12   1hr/24   30min/48
};

#endif //__CTRL_SLIDER_PART_COLOR_H__
