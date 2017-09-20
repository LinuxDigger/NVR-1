#ifndef _PAGE_TEST_COLOR_SLIDER_H_
#define _PAGE_TEST_COLOR_SLIDER_H_

#include "PageFloat.h"
class CPageSearch;
class CPage;

class CPageTestColorSlider:public CPageFloat
{
public:
	CPageTestColorSlider(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPageTestColorSlider();
	void SetSearchPage(CPageSearch* pPage);

	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );

private:
	CSliderCtrlPartColor *m_pSlider;
	CButton* pBt[10];
	void OnClickBt();
	void OnSlider();


	CPage* m_pDesktop;
	CPageSearch* m_pPageSearch;
};

#endif

