#ifndef __PAGE_SEARCH_REC_H__
#define __PAGE_SEARCH_REC_H__


#include "PageFloat.h"
#include "biz.h"

#define RIGHT_PAGE_WIDTH	(204)
#define BOTTOM_PAGE_HEIGHT	(10/*�ϱ���*/+24/*�̶�ֵ*/+12*4/*������*/+24/*�±���*/)	//106

#if 0
typedef struct 
{
	std::vector<int> v_indexs; //�洢sSearchResult0.psRecfileInfo[] �±�
	std::vector<Range> v_ranges;
}
#endif

class CPageSearchRec:public CPageFloat
{
public:
	CPageSearchRec(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPageSearchRec();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );

private:
	void OnDaySelected();
	void OnEventType();
	void OnEventAllType();
	void OnCombox();
	void OnClkBtn();
	void OnSlider();
	void UpdateCalendar(SYSTEM_TIME* pTime);
	void AdjustChn();
	void Search();
	void dealResult();//�������������Խ�������ɫ
	
private:
	CStatic* pTitle;
	CStatic* pFrameLines[6];
	CCalendar* pCalendar;

	//¼������
	CCheckBox* pChkRecType[5];
	CStatic* pTextRecType[5];

	//�����桢�Ļ���
	//std::vector<CItem*>	items[2];
	int playChnNum;
	CButton* pBtnSingle;
	CButton* pBtnFour;
	CComboBox *pComboBoxChn[4];
	CStatic* pTextChn[4];

	CButton* pBtnPlay;

	//������
	CStatic* pTextHour[25];//Сʱ�̶�
	CSliderCtrlPartColor *m_pSlider[4];	

	int m_ChnMax;

	//����
	u32 todayOpenTime;
	u32 todayBeginTime;
	u32 m_startTime;
	u32 m_endTime;
	u8 m_MaskType;
	u8 m_WindowChn[4];

	SBizSearchPara sSearchPara;
	SBizSearchResult sSearchResult0;
	
	
};

#endif


