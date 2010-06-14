//=============================================================================
/**
 * @file  trainercard_data.c
 * @bfief トレーナーカード用セーブデータアクセス関連
 * @author  Nozomu Saito
 *
 *
 */
//=============================================================================
#include <gflib.h>
#include "savedata/save_tbl.h"
#include "savedata/trainercard_data.h"

#define BADGE_MAX (8)
#define SIGHN_W (24)
#define SIGHN_H (8)

#define DEFAULT_BADGE_SCRUCH  (140)

typedef struct TR_CARD_SV_DATA_tag
{
  u8         SignData[SIGHN_W*SIGHN_H*64/8];   ///< サイン面データ
  u16        personarity;                      ///< 性格（トレーナーカードで参照）
  u8         trainer_view_change;              ///< トレーナーカードで見た目を変更した(0:していない 1:した）
  u8         card_anime;                       ///< サイン面をアニメさせている
  SHORT_DATE badgeGetDate[TRAINERCARD_BADGE_NUM]; ///< バッジ取得日
  s64        lastTime;                         ///< 最後にトレーナーカードを開いた日付・時刻の数値
  u8         dummy[TRAINERCARD_BADGE_NUM];     ///< ダミー１
  u32        openflag;                         ///< 各イベントをこなしているかフラグ
  u16        polish[TRAINERCARD_BADGE_NUM];    ///< バッジ磨きのパラメータ
  u16        dummy2[TRAINERCARD_BADGE_NUM];    ///< ダミー２
}TR_CARD_SV_DATA;

//==============================================================================
/**
 * セーブデータサイズ取得
 *
 * @param none
 *
 * @return  int   サイズ
 */
//==============================================================================
int TRCSave_GetSaveDataSize(void)
{
  return sizeof(TR_CARD_SV_DATA);
}

//==============================================================================
/**
 * セーブデータ初期化
 *
 * @param outTrCard トレーナーカードデータポインタ
 *
 * @return  none
 */
//==============================================================================
void TRCSave_InitSaveData(TR_CARD_SV_PTR outTrCard)
{
  GFL_STD_MemFill( outTrCard, 0, sizeof(TR_CARD_SV_DATA) );

#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_TRCARD)
  SVLD_SetCrc(GMDATA_ID_TRCARD);
#endif //CRC_LOADCHECK
}

//==============================================================================
/**
 * セーブデータ先頭ポインタ取得
 *
 * @param sv    セーブポインタ
 *
 * @return  TR_CARD_SV_PTR    トレーナーカードデータポインタ
 */
//==============================================================================
TR_CARD_SV_PTR TRCSave_GetSaveDataPtr(SAVE_CONTROL_WORK * sv)
{
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_TRCARD)
  SVLD_CheckCrc(GMDATA_ID_TRCARD);
#endif //CRC_LOADCHECK

  return SaveControl_DataPtrGet(sv, GMDATA_ID_TRCARD);
}



//==============================================================================
/**
 * サインデータ先頭ポインタ取得
 *
 * @param inTrCard    トレーナーカードデータポインタ
 *
 * @return  u8*       サインデータポインタ
 */
//==============================================================================
u8 *TRCSave_GetSignDataPtr(TR_CARD_SV_PTR inTrCard)
{
  return inTrCard->SignData;
}

//==============================================================================
/**
 * @brief   サインが書き込まれているか確認する
 *
 * @param   inTrCard    
 *
 * @retval  int   TRUEなら書き込まれている。FALSEなら全て空白
 */
//==============================================================================
int TRCSave_GetSigned( TR_CARD_SV_PTR inTrCard )
{
  int search=0;
  int i;
  u32 *ptr =  (u32*)inTrCard->SignData; //サイン面データ

  
  for(i=0;i<SIGHN_W*SIGHN_H*64/(8*4);i++){
    search += ptr[i];
  }

  if(search){
    return TRUE;
  }
  
  return FALSE;
}

//=============================================================================================
/**
 * @brief トレーナーカードでユニオン見た目を変更したか？
 *
 * @param   misc    
 *
 * @retval  int   トレーナーカードでユニオン見た目を変更したか？（0:してない 1:した）
 */
//=============================================================================================
int TRCSave_GetTrainerViewChange( TR_CARD_SV_PTR outTrCard )
{
  return outTrCard->trainer_view_change;
}

//=============================================================================================
/**
 * @brief トレーナーカードでユニオン見た目を変更したフラグを立てる
 */
//=============================================================================================
void TRCSave_SetTrainerViewChange( TR_CARD_SV_PTR inTrCard )
{
  inTrCard->trainer_view_change = 1;
}


//=============================================================================================
/**
 * @brief 性格の取得
 *
 * @param   misc    
 *
 * @retval  int   性格値
 */
//=============================================================================================
int TRCSave_GetPersonarity( TR_CARD_SV_PTR outTrCard )
{
  return outTrCard->personarity;

}

//=============================================================================================
/**
 * @brief トレーナーカードで変更した性格を格納
 */
//=============================================================================================
void TRCSave_SetPersonarity( TR_CARD_SV_PTR inTrCard, int personarity )
{
  GF_ASSERT_MSG( (personarity < TRCARD_PERSONARITY_MAX),"性格の最大値を超えている\n");

  inTrCard->personarity = personarity;
}

//=============================================================================================
/**
 * @brief サインアニメ状態をセット
 *
 * @param   inTrCard    
 * @param   flag      TRUE:アニメ FALSE:アニメしない
 */
//=============================================================================================
void TRCSave_SetSignAnime( TR_CARD_SV_PTR inTrCard, BOOL flag )
{
  inTrCard->card_anime = flag;
}

//=============================================================================================
/**
 * @brief   サインアニメ状態を取得
 *
 * @param   outTrCard   
 *
 * @retval  int   TRUE:アニメ FALSE:アニメしない
 */
//=============================================================================================
int TRCSave_GetSignAnime( TR_CARD_SV_PTR outTrCard )
{
  return outTrCard->card_anime;
}


//=============================================================================================
/**
 * @brief 最後にカードを開いた日を設定
 *
 * @param   inTrCard    
 * @param   DateTime    
 */
//=============================================================================================
void TRCSave_SetLastTime( TR_CARD_SV_PTR inTrCard, s64 DateTime )
{
  inTrCard->lastTime = DateTime;
}

//=============================================================================================
/**
 * @brief 最後にカードを開いた日を取得
 *
 * @param   outTrCard   
 *
 * @retval  s64   
 */
//=============================================================================================
s64 TRCSave_GetLastTime( TR_CARD_SV_PTR outTrCard )
{
  return outTrCard->lastTime;
}

//=============================================================================================
/**
 * @brief バッジ取得日時を取得
 *
 * @param   outTrCard   
 * @param   no    
 *
 * @retval  SHORT_DATE    
 */
//=============================================================================================
SHORT_DATE TRCSave_GetBadgeDate( TR_CARD_SV_PTR outTrCard, int no )
{
  GF_ASSERT(no<TRAINERCARD_BADGE_NUM);

  return outTrCard->badgeGetDate[no];
}

//=============================================================================================
/**
 * @brief バッジ取得日時を設定
 *
 * @param   inTrCard    
 * @param   no    
 * @param   date    
 */
//=============================================================================================
void TRCSave_SetBadgeDate( TR_CARD_SV_PTR inTrCard, int no, int year, int month, int day )
{
  RTCDate date;
  RTCTime time;
  s64     digit;

  GF_ASSERT(no<TRAINERCARD_BADGE_NUM);

  // 日付・時間取得
  GFL_RTC_GetDateTime( &date, &time);
  digit = RTC_ConvertDateTimeToSecond( &date, &time );


  // バッジ取得の年月日設定
  inTrCard->badgeGetDate[no].year  = year;
  inTrCard->badgeGetDate[no].month = month;
  inTrCard->badgeGetDate[no].day   = day;

  // 一緒にバッジ磨き情報も入れる
  inTrCard->polish[no] = BADGE_POLISH_MAX/2;

  // 最後にトレーナーカードを開いたのを今にする
  TRCSave_SetLastTime( inTrCard, digit );
}


//=============================================================================================
/**
 * @brief バッジの磨き具合を取得
 *
 * @param   outTrCard   
 * @param   no    
 *
 * @retval  SHORT_DATE    
 */
//=============================================================================================
int TRCSave_GetBadgePolish( TR_CARD_SV_PTR outTrCard, int no )
{
  GF_ASSERT(no<TRAINERCARD_BADGE_NUM);

  return outTrCard->polish[no];
}

//=============================================================================================
/**
 * @brief バッジの磨き具合を設定
 *
 * @param   inTrCard    
 * @param   no    
 * @param   polish    
 */
//=============================================================================================
void TRCSave_SetBadgePolish( TR_CARD_SV_PTR inTrCard, int no, int polish )
{
  inTrCard->polish[no] = polish;
}

//=============================================================================================
/**
 * @brief 各種イベントを終了済か取得
 *
 * @param   outTrCard   
 * @param   event   
 *
 * @retval  BOOL    
 */
//=============================================================================================
BOOL TRCSave_GetEventOpenFlag( TR_CARD_SV_PTR outTrCard, int event )
{
  return 1 & (outTrCard->openflag>>event);
}

//=============================================================================================
/**
 * @brief 各種イベントの終了条件を設定
 *
 * @param   outTrCard   
 * @param   event   
 */
//=============================================================================================
void TRCSave_SetEventOpenFlag( TR_CARD_SV_PTR inTrCard, int event )
{
  inTrCard->openflag |= (1<<event);
}
