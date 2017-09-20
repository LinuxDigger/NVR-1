#ifndef __PAGE_SEARCH_REC_H__
#define __PAGE_SEARCH_REC_H__


#include "PageFloat.h"
#include "biz.h"


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
	void OnClkBtn();
	void OnSlider();
	void UpdateCalendar(SYSTEM_TIME* pTime);
	void AdjustChn();
	
private:
	CStatic* pTitle;
	CStatic* pFrameLines[6];
	CCalendar* pCalendar;

	//录像类型
	CCheckBox* pChkRecType[5];
	CStatic* pTextRecType[5];

	//单画面、四画面
	std::vector<CItem*>	items[2];
	int playChnNum;
	CButton* pBtnSingle;
	CButton* pBtnFour;
	CComboBox *pComboBoxChn[4];
	CStatic* pTextChn[4];

	CButton* pBtnPlay;

	//进度条
	CStatic* pTextHour[25];//小时刻度
	CSliderCtrlPartColor *m_pSlider[4];	

	int m_ChnMax;
};

#endif


