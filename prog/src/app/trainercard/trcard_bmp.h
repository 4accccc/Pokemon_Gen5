//============================================================================================
/**
 * @file  trcard_bmp.h
 * @bfief トレーナーカード画面BMP関連ヘッダ
 * @author  Akito Mori / Nozomu Saito (WB移植は有泉くん)
 * @date  05.11.15
 */
//============================================================================================
#ifndef _TRCARD_BMP_H_
#define _TRCARD_BMP_H_

#include "trcard_sys.h"


//=====================================
// 数値の桁数最大数
//=====================================
typedef enum {
  TR_ID_DIGIT = 5,    // トレーナーＩＤの桁数
  MONEY_DIGIT = 7,
  SCORE_DIGIT = 9,
  TIME_H_DIGIT = 3,
  TIME_M_DIGIT = 2,
  YEAR_DIGIT = 4,
  MONTH_DIGIT = 2,
  DAY_DIGIT = 2,
  COMM_DIGIT = 9,
  BATTLE_DIGIT = 9,
  SURETIGAI_DIGIT = 5,
  FEELING_DIGIT = 4,
  GPOWER_DIGIT = 2,
  MUSICAL_DIGIT = 4,
  POKESHIFTER_DIGIT = 3,
  TRIALHOUSE_DIGIT = 4,
}TR_DIGIT;

#define TR_DIGIT_MAX  (32)




//説明ウィンドウ描画パターン定義
enum{
 EXPWIN_PAT_SIGN,
 EXPWIN_PAT_TOUCH,
};

#define TR_STRING_LEN (32)        //ローカライズを考え多めに。
#define TR_DIGIT_LEN  (TR_DIGIT_MAX+1)

extern void TRCBmp_AddTrCardBmp(TR_CARD_WORK* wk);
extern void TRCBmp_ExitTrCardBmpWin(TR_CARD_WORK* wk);
extern void TRCBmp_TransTrWinInfo(TR_CARD_WORK* wk,GFL_BMPWIN *win[]);
extern void TRCBmp_WriteTrWinInfo(TR_CARD_WORK* wk, GFL_BMPWIN *win[], const TR_CARD_DATA* inTrCardData );
extern void TRCBmp_WriteTrWinInfoRev(TR_CARD_WORK* wk, GFL_BMPWIN *win[], const TR_CARD_DATA* inTrCardData );
extern void TRCBmp_NonDispWinInfo(GFL_BMPWIN *win[], const u8 start, const u8 end);
extern void TRCBmp_WritePlayTime(TR_CARD_WORK* wk, GFL_BMPWIN *win, int y, const TR_CARD_DATA *inTrCardData, STRBUF *str, int col);
extern void TRCBmp_WriteSec(TR_CARD_WORK* wk,GFL_BMPWIN *win, const BOOL inDisp, STRBUF *inSecBuf);
extern void TRCBmp_WriteBackButton(TR_CARD_WORK* wk,const u8 on_f);
extern void TRCBmp_WriteExpWin(TR_CARD_WORK* wk,u8 pat);
extern void TRCBmp_SignDrawMsgPut(TR_CARD_WORK* wk,const u8 pat);
extern void TRCBmp_SignDrawYesNoCall(TR_CARD_WORK* wk,const u8 pat);
extern int  TRCBmp_SignDrawYesNoWait(TR_CARD_WORK* wk,const u8 pat);
extern void TRCBmp_PrintTrainerType( TR_CARD_WORK *wk, int trtype, int trans_sw );
extern void TRCBmp_PrintPersonality( TR_CARD_WORK *wk, int personality, int trans_sw );
extern void TRCBmp_WriteScoreListWin( TR_CARD_WORK *wk, int scrol_point, int trans_sw, int col );
extern void TRCBmp_MakeScoreList( TR_CARD_WORK *wk, TR_CARD_DATA *TrCardData );


#endif //_TRCARD_BMP_H_

