#ifndef _TRCARD_BMP_H_
#define _TRCARD_BMP_H_

#include "trcard_sys.h"

typedef enum {
	TR_ID_DIGIT = 5,
	MONEY_DIGIT = 6,
	MONS_NUM_DIGIT = 3,
	SCORE_DIGIT = 9,
	TIME_H_DIGIT = 3,
	TIME_M_DIGIT = 2,
	YEAR_DIGIT = 2,
	MONTH_DIGIT = 2,
	DAY_DIGIT = 2,
	COMM_DIGIT = 6,
	BATTLE_DIGIT = 4,
	TRADE_DIGIT = 6,
}TR_DIGIT;

#define TR_DIGIT_MAX	(9)

typedef enum{
	TRC_BMPWIN_TR_ID,
	TRC_BMPWIN_TR_NAME,
	TRC_BMPWIN_MONEY,
	TRC_BMPWIN_BOOK,
	TRC_BMPWIN_SCORE,
	TRC_BMPWIN_PLAY_TIME,
	TRC_BMPWIN_START_TIME,
	TRC_BMPWIN_CLEAR_TIME,
	TRC_BMPWIN_COMM_INFO,
	TRC_BMPWIN_BATTLE_INFO,
	TRC_BMPWIN_TRADE_INFO,
	
	TRC_BMPWIN_MSG,
	TRC_BMPWIN_EXP,
	TRC_BMPWIN_BACK,
}TRC_BMPWIN;

//説明ウィンドウ描画パターン定義
enum{
 EXPWIN_PAT_SIGN,
 EXPWIN_PAT_TOUCH,
};

#define TR_STRING_LEN	(32)				//ローカライズを考え多めに。
#define TR_DIGIT_LEN	(TR_DIGIT_MAX+1)

extern void TRCBmp_AddTrCardBmp(TR_CARD_WORK* wk);
extern void TRCBmp_ExitTrCardBmpWin(TR_CARD_WORK* wk);
extern void TRCBmp_TransTrWinInfo(TR_CARD_WORK* wk,GFL_BMPWIN	*win[]);
extern void TRCBmp_WriteTrWinInfo(TR_CARD_WORK* wk, GFL_BMPWIN *win[], const TR_CARD_DATA* inTrCardData );
extern void TRCBmp_WriteTrWinInfoRev(TR_CARD_WORK* wk, GFL_BMPWIN *win[], const TR_CARD_DATA* inTrCardData );
extern void TRCBmp_NonDispWinInfo(GFL_BMPWIN *win[], const u8 start, const u8 end);
extern void TRCBmp_WritePlayTime(TR_CARD_WORK* wk, GFL_BMPWIN	*win[], const TR_CARD_DATA *inTrCardData, STRBUF *str);
extern void TRCBmp_WriteSec(TR_CARD_WORK* wk,GFL_BMPWIN	*win, const BOOL inDisp, STRBUF *inSecBuf);
extern void TRCBmp_WriteBackButton(TR_CARD_WORK* wk,const u8 on_f);
extern void TRCBmp_WriteExpWin(TR_CARD_WORK* wk,u8 pat);
extern void TRCBmp_SignDrawMsgPut(TR_CARD_WORK* wk,const u8 pat);
extern void TRCBmp_SignDrawYesNoCall(TR_CARD_WORK* wk,const u8 pat);
extern int TRCBmp_SignDrawYesNoWait(TR_CARD_WORK* wk,const u8 pat);

#endif //_TRCARD_BMP_H_

