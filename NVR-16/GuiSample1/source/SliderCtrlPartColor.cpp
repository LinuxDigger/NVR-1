//
//  "$Id: SliderCtrl.cpp 280 2008-12-17 06:04:55Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//
//yaogang modify for playback 20170906


#include "GUI/Ctrls/Page.h"
#include <vector>

Range::Range()
:start(0), end(0)
{
	
}

Range::Range(int s, int e)
{
	if (s < e)
	{
		start = s;
		end = e;
	}
	else
	{
		start = end = 0;
	}
}

Range::~Range()
{
	
}

#if 0
//val �Ƿ���this ��
VD_BOOL Range::isInRange(int val) //[s, e)
{
	if (start <= val && val < end)
	{
		return TRUE;
	}

	return FALSE;
}
#endif

//this �Ƿ���r ��
VD_BOOL Range::isInRange(const Range &r) const
{
	if (start >= r.start && end <= r.end)
	{
		return TRUE;
	}

	return FALSE;
}

VD_BOOL Range::selfCheck() const
{
	if (start < end)
	{
		return TRUE;
	}

	return FALSE;
}

int Range::Width() const
{
	return end-start;
}

VD_BOOL operator ==(const Range &lhs, const Range &rhs)
{
	if (lhs.start == rhs.start 
		&& lhs.end == rhs.end)
	{
		return TRUE;
	}

	return FALSE;
}

VD_BOOL operator !=(const Range &lhs, const Range &rhs)
{
	if (lhs == rhs)
	{
		return FALSE;
	}

	return TRUE;
}

VD_BOOL isInRange(int val, const Range &r)//val �Ƿ���r ��
{
	if (r.start <= val && val <= r.end)
	{
		return TRUE;
	}

	return FALSE;
}

VD_BOOL isInRangeWithoutStart(int val, const Range &r)
{
	if (r.start < val && val <= r.end)
	{
		return TRUE;
	}

	return FALSE;
}

VD_BOOL isInRangeWithoutEnd(int val, const Range &r)
{
	if (r.start <= val && val < r.end)
	{
		return TRUE;
	}

	return FALSE;
}

VD_BOOL isInRangeWithoutBoth(int val, const Range &r)	
{
	if (r.start < val && val < r.end)
	{
		return TRUE;
	}

	return FALSE;
}

/*
	���������λ�ù�ϵ
	pos < r.start			left
	pos = r.start			equal left
	pos (r.start, r.end)		inside
	pos = r.end 			equal right
	pos > r.end 			right
*/
int pointAndRange(int val, const Range &r)
{
	if (val < r.start)
	{
		return Range::em_left;
	}
	else if (val == r.start)
	{
		return Range::em_equal_start;
	}
	else if (val < r.end)
	{
		return Range::em_inside;
	}
	else if (val == r.end)
	{
		return Range::em_equal_end;
	}

	return Range::em_right;
}


//CSliderCtrlPartColor
CSliderCtrlPartColor::CSliderCtrlPartColor(VD_PCRECT pRect, CPage * pParent, int vmin,int vmax, int SplitLineNum, CTRLPROC vproc)
:CItem(pRect, pParent, IT_SLIDERCTRL, styleEditable|styleAutoFocus),
m_Mutex(MUTEX_RECURSIVE),
m_bTracker(TRUE),
m_sRealRange(vmin, vmax),
m_onValueChanged(vproc),
curpos(0),
newpos(0),
track(FALSE),
tracker_offset(0),
tracker_pick(0),
m_iSplitLineNum(SplitLineNum),
m_ColorSplitLine(VD_RGB(102, 102, 102)),
m_ColorBlank(VD_RGB(200, 200, 200)),
m_Color(VD_RGB(0, 174, 255)),
m_sDispRange(m_sRealRange)
{
	m_iSliderWidth = m_Rect.Width();
	tracker_width = m_Rect.Height()/2;

	printf("%s: realRange[%d, %d], dispRange[%d, %d], SplitLineNum: %d\n",
			__func__, m_sRealRange.start, m_sRealRange.end,
			m_sDispRange.start, m_sDispRange.end, SplitLineNum);
/*
	for (int i = 0; i<m_iSplitLineMax; i++)
	{
		CStatic* pSplit = CreateStatic(CRect(0,0,0,0), this, "");
		pSplit->SetBkColor(m_ColorSplitLine);
		pSplit->Show(FALSE);
		
		m_vSplitLine.push_back(pSplit);
	}
*/
}

CSliderCtrlPartColor::~CSliderCtrlPartColor()
{
	#if 0
	std::vector<CStatic*>::iterator iter;
	
	if (!m_vSplitLine.empty())
	{
		for (iter = m_vSplitLine.begin();
			iter != m_vSplitLine.end();
			++iter)
		{
			delete *iter;
			*iter = NULL;
		}

		m_vSplitLine.clear();
	}
	#endif
}

int CSliderCtrlPartColor::SetPos(int pos)
{
	CGuard guard(m_Mutex);
	
	if(curpos == pos)
	{
		return 0;
	}
	
	if (!isInRange(pos, m_sRealRange))
	{
		printf("%s: %d not inside RealRange[%d, %d]\n", __func__, pos, m_sRealRange.start, m_sRealRange.end);
		return 1;
	}

	if (!isInRange(pos, m_sDispRange))
	{
		printf("%s: %d not inside DispRange[%d, %d]\n", __func__, pos, m_sDispRange.start, m_sDispRange.end);
		return 1;
	}
	
	curpos = pos;
	UpdateTracker();
	Draw();

	printf("%s over\n", __func__);
	
	return 0;
}

int CSliderCtrlPartColor::GetPos()
{
	CGuard guard(m_Mutex);
	
	return curpos;
}

int CSliderCtrlPartColor::GetAt(int px, int py)//�Ƿ��ڻ������������
{
	CGuard guard(m_Mutex);
	
	if(PtInRect(m_Rect, px, py))
	{
		//�Ƿ��ڻ������������
		if(px < m_Rect.left + tracker_offset || px > m_Rect.left + tracker_offset + tracker_width)
		{
			return CSliderCtrl::SA_POSITION;
		}
		else
		{
			return CSliderCtrl::SA_TRACK;
		}
	}
	
	return -1;
}

void CSliderCtrlPartColor::SetTrackerEnable(VD_BOOL b)
{
	CGuard guard(m_Mutex);
	
	if (b != m_bTracker)
	{
		m_bTracker = b;
		Draw();
	}
}

void CSliderCtrlPartColor::SetSplitLineNum(int n)
{
	CGuard guard(m_Mutex);
	
	if (n != m_iSplitLineNum)
	{
		m_iSplitLineNum = n;
		Draw();
	}
}

void CSliderCtrlPartColor::SetSplitLineColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ )
{
	CGuard guard(m_Mutex);
	
	if (color != m_ColorSplitLine)
	{
		m_ColorSplitLine = color;
		Draw();
	}
}

void CSliderCtrlPartColor::SetColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ )
{
	CGuard guard(m_Mutex);
	
	if (color != m_Color)
	{
		m_Color = color;
		Draw();
	}
}

void CSliderCtrlPartColor::SetBlankColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ )
{
	CGuard guard(m_Mutex);
	
	if (color != m_ColorBlank)
	{
		m_ColorBlank = color;
		Draw();
	}
}

//��ָ������Ŵ���ʾ
int CSliderCtrlPartColor::SetDisplayRange(const Range &r)
{
	CGuard guard(m_Mutex);

	if (!r.selfCheck())
	{
		printf("%s: Range[%d, %d] selfCheck failed\n", __func__, r.start, r.end);
		return 1;
	}

	if (!r.isInRange(m_sRealRange))
	{
		printf("%s: Range[%d, %d] not inside RealRange[%d, %d]\n", 
			__func__, r.start, r.end, m_sRealRange.start, m_sRealRange.end);
		return 1;
	}

	if (!isInRange(curpos, r))//�α겻����ʾ������
	{
		printf("%s: curpos: %d not inside Range[%d, %d]\n", 
			__func__, curpos, r.start, r.end);
		return 1;
	}
	 
	if (r == m_sDispRange)
	{
		return 0;
	}

	m_sDispRange = r;
	UpdateTracker();//�����α�λ�á���ʾ����ı䣬�����α�ֵ���䣬λ��Ҳ���
	Draw();
	printf("%s over\n", __func__);
	return 0;
}

//ͬʱ��ʾ����ͷָ�������
//eg: playback module 24hr/2hr/1hr/30min
int CSliderCtrlPartColor::SetDisplayRange(const Range &r, int split_line_num)
{
	CGuard guard(m_Mutex);
	BOOL b_draw = FALSE;

	if (!r.selfCheck())
	{
		printf("%s: Range[%d, %d] selfCheck failed\n", __func__, r.start, r.end);
		return 1;
	}
	
	if (!r.isInRange(m_sRealRange))
	{
		printf("%s: Range[%d, %d] not inside RealRange[%d, %d]\n", 
			__func__, r.start, r.end, m_sRealRange.start, m_sRealRange.end);
		return 1;
	}

	if (!isInRange(curpos, r))//�α겻����ʾ������
	{
		printf("%s: curpos: %d not inside Range[%d, %d]\n", 
			__func__, curpos, r.start, r.end);
		return 1;
	}
	
	if (r != m_sDispRange)
	{
		m_sDispRange = r;
		b_draw = TRUE;
		UpdateTracker();//�����α�λ�á���ʾ����ı䣬�����α�ֵ���䣬λ��Ҳ���
	}

	if (split_line_num != m_iSplitLineNum)
	{
		m_iSplitLineNum = split_line_num;
		b_draw = TRUE;		
	}

	if (b_draw)
	{
		Draw();
	}
	printf("%s over\n", __func__);
	return 0;
}

//������ɫ����(��¼���ʱ���)
int CSliderCtrlPartColor::SetColorRange(const std::vector<Range> &vr)
{
	std::vector<Range>::const_iterator c_iter;
	int pre_end = -1;//ǰһ�������λ��

	if (vr.empty())
	{
		printf("%s: vector Range empty\n", __func__);
		return 1;
	}

	for (c_iter = vr.begin(); c_iter != vr.end(); ++c_iter)
	{
		if (!c_iter->selfCheck())
		{
			printf("%s: Range[%d, %d] selfCheck failed\n", __func__, c_iter->start, c_iter->end);
			return 1;
		}

		if (pre_end >= c_iter->start)//����䲻���н���
		{
			printf("%s: pre_end(%d) >= c_iter->start(%d)\n", __func__, pre_end, c_iter->start);
			return 1;
		}

		if (!c_iter->isInRange(m_sRealRange))//����Ҫ�ڽ�������Χ��
		{
			printf("%s: Range[%d, %d] not inside RealRange[%d, %d]\n", 
				__func__, c_iter->start, c_iter->end, m_sRealRange.start, m_sRealRange.end);
			return 1;
		}

		pre_end = c_iter->end;
	}
	
	m_vRange = vr;
	return 0;
}


void CSliderCtrlPartColor::Draw()
{
	DrawBackground();
	DrawColorRange();
	DrawSplitLine();
	DrawTracker();
}

//����˳��
//	DrawBackground
//	DrawRange
//	DrawSplitLine
//	DrawTracker

void CSliderCtrlPartColor::DrawBackground()
{
	m_DC.Lock();

	m_DC.SetBrush(m_ColorBlank);
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));

	//�»���
	m_DC.SetPen(VD_GetSysColor(VD_COLOR_WINDOW), VD_PS_SOLID, 1);
	m_DC.MoveTo(0, m_Rect.Height()-1);
	m_DC.LineTo(m_Rect.Width(), m_Rect.Height()-1);
	
	m_DC.UnLock();
}

//������ɫ����
void CSliderCtrlPartColor::_DrawColorRange(int start, int end)
{
	CGuard guard(m_Mutex);
	int start_offset = 0;
	int end_offset = 0;
	
	m_DC.Lock();

	m_DC.SetBrush(m_Color);
	m_DC.SetRgnStyle(RS_FLAT);
	
	//val --> pix pos
	start_offset = (float)(m_iSliderWidth) / m_sDispRange.Width() * (start - m_sDispRange.start);
	end_offset = (float)(m_iSliderWidth) / m_sDispRange.Width() * (end - m_sDispRange.start);
	m_DC.Rectangle(CRect(start_offset, 0, end_offset, m_Rect.Height()));
	
	m_DC.UnLock();
}

/*
���������λ�ù�ϵ
pos	< r.start			left
pos	= r.start			equal left
pos	(r.start, r.end)	inside
pos	= r.end			equal right
pos	> r.end			right

�ؼ�������ж�m_sDispRange ��start ��end �����m_vRange �и�������Ĺ�ϵ
*/
void CSliderCtrlPartColor::DrawColorRange()
{
	CGuard guard(m_Mutex);
	std::vector<Range>::const_iterator c_iter;
	int start_relative_pos;//m_sDispRange.start �����c_iter �����λ�ù�ϵ
	int end_relative_pos;//m_sDispRange.end �����c_iter �����λ�ù�ϵ
	VD_BOOL b_found_start = FALSE;//�Ƿ�ȷ����m_sDispRange.start
	VD_BOOL b_found_end = FALSE;//�Ƿ�ȷ����m_sDispRange.end

	int start_color_range = 0;//��ɫ�������ʼ
	int end_color_range = 0;//��ɫ����Ľ���

	if (m_vRange.empty())
	{
		return ;
	}	

	//printf("%s: dispRange[%d, %d]\n", __func__, m_sDispRange.start, m_sDispRange.end);
	for (c_iter = m_vRange.begin(); c_iter != m_vRange.end(); ++c_iter)
	{
		if (b_found_end)//ȷ����m_sDispRange.end �ͽ�����
		{
			break;
		}
		
		start_relative_pos = pointAndRange(m_sDispRange.start, *c_iter);
		end_relative_pos = pointAndRange(m_sDispRange.end, *c_iter);

		printf("%s: colorRange[%d, %d], dispRange[%d, %d], start_relative_pos: %d, end_relative_pos: %d\n",
			__func__, c_iter->start, c_iter->end, 
			m_sDispRange.start, m_sDispRange.end, 
			start_relative_pos, end_relative_pos);				

		if (!b_found_start)
		{
			switch (start_relative_pos)
			{
				case Range::em_left ://m_sDispRange.start  left  range
				{
					switch (end_relative_pos)
					{
						case Range::em_left :
						case Range::em_equal_start ://m_sDispRange.end  left  range
						{
							b_found_start = TRUE;
							b_found_end	= TRUE;							
						} break;
						case Range::em_inside :		//m_sDispRange.end  in  range
						case Range::em_equal_end :
							b_found_start = TRUE;
							b_found_end	= TRUE;
						// break;
						case Range::em_right :		//m_sDispRange.end  right  range
						{
							b_found_start = TRUE;

							start_color_range = c_iter->start;
							end_color_range = MIN(c_iter->end, m_sDispRange.end);
							_DrawColorRange(start_color_range, end_color_range);
						} break;	
						default:
							printf("Error: %s: colorRange[%d, %d], dispRange[%d, %d], start_relative_pos: %d, end_relative_pos: %d, default\n",
								__func__, c_iter->start, c_iter->end, 
								m_sDispRange.start, m_sDispRange.end,
								start_relative_pos, end_relative_pos);	
					}
				} break;
				
				case Range::em_equal_start ://m_sDispRange.start  equal_start  range
				case Range::em_inside :		//m_sDispRange.start  inside  range
				{
					switch (end_relative_pos)
					{
						case Range::em_left :
						case Range::em_equal_start ://m_sDispRange.end  left  range  �����ϲ����ܳ��ָ����
						{
							printf("Error: %s: colorRange[%d, %d], dispRange[%d, %d], start_relative_pos: %d, end_relative_pos: %d, invalid\n",
								__func__, c_iter->start, c_iter->end,
								m_sDispRange.start, m_sDispRange.end,
								start_relative_pos, end_relative_pos);
							
							return;
						} break;
						case Range::em_inside :		//m_sDispRange.end  in  range
						case Range::em_equal_end :
							b_found_start = TRUE;
							b_found_end	= TRUE;
						// break;
						case Range::em_right :		//m_sDispRange.end  right  range
						{
							b_found_start = TRUE;

							start_color_range = MAX(c_iter->start, m_sDispRange.start);
							end_color_range = MIN(c_iter->end, m_sDispRange.end);
							_DrawColorRange(start_color_range, end_color_range);
						} break;	
						default:
							printf("Error: %s: colorRange[%d, %d], dispRange[%d, %d], start_relative_pos: %d, end_relative_pos: %d, default\n",
								__func__, c_iter->start, c_iter->end, 
								m_sDispRange.start, m_sDispRange.end,
								start_relative_pos, end_relative_pos);		
					}
				}break;
					
				case Range::em_equal_end ://m_sDispRange.start  right  range
				case Range::em_right :
				{
					//do nothing, continue;
				} break;
				default:
					printf("Error: %s: colorRange[%d, %d], start_relative_pos: %d, end_relative_pos: %d, default\n",
						__func__, c_iter->start, c_iter->end, start_relative_pos, end_relative_pos);
			}
		}
		else //�Ѿ�ȷ����m_sDispRange.start
		{
			switch (end_relative_pos)
			{
				case Range::em_left :		//m_sDispRange.end  left  range		
				case Range::em_equal_start ://m_sDispRange.end equal_start  range
				{
					b_found_end	= TRUE;
				} break;
				
				case Range::em_inside :		//m_sDispRange.end  inside  range
				case Range::em_equal_end :
				{
					b_found_end	= TRUE;
					start_color_range = c_iter->start;
					end_color_range = MIN(c_iter->end, m_sDispRange.end);
					_DrawColorRange(start_color_range, end_color_range);
				} break;
				
				case Range::em_right :		//m_sDispRange.end  right  range
				{
					start_color_range = c_iter->start;
					end_color_range = c_iter->end;
					_DrawColorRange(start_color_range, end_color_range);
				} break;
				
				default:
					printf("Error: %s: colorRange[%d, %d], dispRange[%d, %d], start_relative_pos: %d, end_relative_pos: %d, default\n",
						__func__, c_iter->start, c_iter->end, 
						m_sDispRange.start, m_sDispRange.end,
						start_relative_pos, end_relative_pos);	
			}
		}
	}
}

//private:
//������(m_iSliderWidth)�ȷ�10�ݣ���ȥ��ͷ������9���ָ���
void CSliderCtrlPartColor::DrawSplitLine()
{
	float f_interval = (float)m_iSliderWidth/m_iSplitLineNum;
	int i_pos = 0;
	int i = 0;
	CGuard guard(m_Mutex);
	
	m_DC.Lock();
	
#if 0
	m_DC.SetBrush(m_ColorSplitLine);
	m_DC.SetRgnStyle(RS_FLAT);
	
	for (i=1; i<m_iSplitLineNum; ++i)
	{
		i_pos = f_interval * i;//��f_intervalΪС��ʱ�������ۼ����

		m_DC.Rectangle(CRect(i_pos, 0, i_pos+1, m_Rect.Height()));
	}
#else
	m_DC.SetPen(m_ColorSplitLine, VD_PS_SOLID, 1);

	for (i=1; i<m_iSplitLineNum; ++i)
	{
		m_DC.MoveTo(i*f_interval, 0);
		m_DC.LineTo(i*f_interval, m_Rect.Height());
	}
#endif

	m_DC.UnLock();
}

void CSliderCtrlPartColor::UpdateTracker()
{
	CGuard guard(m_Mutex);
	newpos = curpos;
	
	tracker_offset = (double)(m_iSliderWidth - tracker_width) * (curpos- m_sDispRange.start) / m_sDispRange.Width()+0.5;
}

void CSliderCtrlPartColor::DrawTracker()
{
	CGuard guard(m_Mutex);
	m_DC.Lock();
	
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(tracker_offset, 0, tracker_offset + tracker_width, m_Rect.Height()-1));
	
	m_DC.UnLock();
}


VD_BOOL CSliderCtrlPartColor::MsgProc(uint msg, uint wpa, uint lpa)
{
	int px, py;
	int vtracker_offset;
	int temp;
	static int press_tracker_offset = 0;

	newpos = curpos;
	switch (msg)
	{
		case XM_LBUTTONDOWN:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		temp = GetAt(px, py);
		if(temp == CSliderCtrl::SA_POSITION){
			newpos = m_sDispRange.start + (double)(m_sDispRange.Width()+ 1) * (px - tracker_width/2 - m_Rect.left) / MAX(1, (m_iSliderWidth - tracker_width))+0.5;
			if(newpos < m_sDispRange.start){
				newpos = m_sDispRange.start;
			}else if(newpos > m_sDispRange.end){
				newpos = m_sDispRange.end;
			}
		}else if(temp == CSliderCtrl::SA_TRACK){
			track = TRUE;
			SetFlag(IF_CAPTURED, TRUE);
			tracker_pick = px - m_Rect.left - tracker_offset;
			press_tracker_offset = tracker_offset;
		}else{
			return FALSE;
		}
		break;

	case XM_LBUTTONUP://�϶��α����ʱ
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(track)
		{			
			newpos = m_sDispRange.start + (double)m_sDispRange.Width() * tracker_offset / MAX(1, (m_iSliderWidth - tracker_width))+0.5;
			
			track = FALSE;
			SetFlag(IF_CAPTURED, FALSE);

			if ((tracker_offset != press_tracker_offset)
				&&(newpos == curpos))//�α�ֵû�䣬����������϶�����ƫ��ʱ����һ��λ��
			{
				UpdateTracker();
				Draw();
			}
		}
		break;
		
	case XM_MOUSEMOVE://�϶��α�ʱ, curpos ���ı�, tracker_offset �ı�
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(track){
			if(px - tracker_pick < m_Rect.left){
				px = tracker_pick + m_Rect.left;
			}else if(px - tracker_pick > m_Rect.left + m_iSliderWidth - tracker_width){
				px = m_Rect.left + m_iSliderWidth - tracker_width + tracker_pick;
			}
			
			vtracker_offset = px - tracker_pick - m_Rect.left;
			
			if(vtracker_offset != tracker_offset)
			{
				tracker_offset = vtracker_offset;
				
				newpos = m_sDispRange.start+ (double)m_sDispRange.Width() * tracker_offset / MAX(1, (m_iSliderWidth - tracker_width))+0.5;
				
				Draw();
			}
			return TRUE;
		}
		
		return FALSE;

	default:
		return FALSE;
	}

	//�����϶�
	if(newpos != curpos)
	{
		curpos = newpos;
		ItemProc(m_onValueChanged);           //zmx 04.7.27
		vtracker_offset = tracker_offset;
		UpdateTracker();
		Draw();
	}

	return TRUE;
}

