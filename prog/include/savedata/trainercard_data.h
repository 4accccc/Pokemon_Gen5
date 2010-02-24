//=============================================================================
/**
 * @file  trainerbadge_data.h
 * @bfief トレーナーカード用セーブデータアクセス関連
 * @author  Nozomu Saito
 *
 *
 */
//=============================================================================
#ifndef __TRAINERCARD_DATA_H__
#define __TRAINERCARD_DATA_H__

// トレーナーカードで設定できる「性格」の最大値
#define TRCARD_PERSONARITY_MAX    ( 8 )

#include "savedata/save_control.h"
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



#endif //__TRAINERCARD_DATA_H__

