//=============================================================================
/**
 * @file   trainercard_data.h
 * @bfief  トレーナーカード用セーブデータアクセス関連
 * @author Akito Mori
 *
 *
 */
//=============================================================================
#ifndef __TRAINERCARD_DATA_H__
#define __TRAINERCARD_DATA_H__

// トレーナーカードで設定できる「性格」の最大値
#define TRCARD_PERSONARITY_MAX    ( 25 )

#define TRCARD_FINISH_PALACE      (  0 )
#define TRCARD_FINISH_POKESHIFTER (  1 )
#define TRCARD_FINISH_TRIALHOUSE  (  2 )
#define TRCARD_FINISH_DENDOU      (  3 )

// バッジ磨き情報（１ランク分）
#define BADGE_POLISH_RATE   ( 48*2 )  
#define BADGE_POLISH_MAX    ( BADGE_POLISH_RATE*5 )  

// トレーナーカードで扱うバッジの数
#define TRAINERCARD_BADGE_NUM (  8 )

#include "savedata/save_control.h"
#include "system/pms_data.h"

// サイズがRTCDateより小さい日付構造体
typedef struct{
  u8 year;
  u8 month;
  u8 day;
  u8 etc;
} SHORT_DATE;


typedef struct TR_CARD_SV_DATA_tag * TR_CARD_SV_PTR;

extern int TRCSave_GetSaveDataSize(void);
extern void TRCSave_InitSaveData(TR_CARD_SV_PTR outTrCard);

// トレーナカードデータ取得
extern TR_CARD_SV_PTR TRCSave_GetSaveDataPtr(SAVE_CONTROL_WORK * sv);

// サインデータ周り
extern u8 *TRCSave_GetSignDataPtr(TR_CARD_SV_PTR inTrCard);
extern int TRCSave_GetSigned( TR_CARD_SV_PTR inTrCard );
// ユニオン見た目を変更しているか？
extern void TRCSave_SetTrainerViewChange( TR_CARD_SV_PTR inTrCard );
extern  int TRCSave_GetTrainerViewChange( TR_CARD_SV_PTR outTrCard );
// 性格（トレーナーカードで表示される）
extern void TRCSave_SetPersonarity( TR_CARD_SV_PTR inTrCard, int personarity );
extern  int TRCSave_GetPersonarity( TR_CARD_SV_PTR outTrCard );
// サインアニメ
extern void TRCSave_SetSignAnime( TR_CARD_SV_PTR inTrCard, int flag );
extern  int TRCSave_GetSignAnime( TR_CARD_SV_PTR outTrCard );
// 最後にカードを開いた日
extern void TRCSave_SetLastTime( TR_CARD_SV_PTR inTrCard, s64 DateTime );
extern s64  TRCSave_GetLastTime( TR_CARD_SV_PTR outTrCard );
// バッジ取得日時
extern void TRCSave_SetBadgeDate( TR_CARD_SV_PTR inTrCard, int no, int year, int month, int day );
extern SHORT_DATE TRCSave_GetBadgeDate( TR_CARD_SV_PTR outTrCard, int no );
// バッジの磨き具合
extern int  TRCSave_GetBadgePolish( TR_CARD_SV_PTR outTrCard, int no );
extern void TRCSave_SetBadgePolish( TR_CARD_SV_PTR inTrCard, int no, int polish );
// 各種イベントの終了状態
extern void TRCSave_SetEventOpenFlag( TR_CARD_SV_PTR inTrCard, int event );
extern BOOL TRCSave_GetEventOpenFlag( TR_CARD_SV_PTR outTrCard, int event );


#endif //__TRAINERCARD_DATA_H__

