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
  u8 SignData[SIGHN_W*SIGHN_H*64/8]; ///<サイン面データ
  u16 personarity;                   ///<性格（トレーナーカードで参照）
  u16 trainer_view_change;           ///<トレーナーカードで見た目を変更した(0:していない 1:した）
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