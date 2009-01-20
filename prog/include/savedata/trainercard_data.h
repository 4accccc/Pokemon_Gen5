//=============================================================================
/**
 * @file	trainerbadge_data.h
 * @bfief	トレーナーカード用セーブデータアクセス関連
 * @author	Nozomu Saito
 *
 *
 */
//=============================================================================
#ifndef __TRAINERCARD_DATA_H__
#define __TRAINERCARD_DATA_H__

#include "savedata/save_control.h"
typedef struct TR_CARD_SV_DATA_tag * TR_CARD_SV_PTR;

extern int TRCSave_GetSaveDataSize(void);
extern void TRCSave_InitSaveData(TR_CARD_SV_PTR outTrCard);
extern TR_CARD_SV_PTR TRCSave_GetSaveDataPtr(SAVE_CONTROL_WORK * sv);
extern u8 *TRCSave_GetSignDataPtr(TR_CARD_SV_PTR inTrCard);
extern int TRCSave_GetSigned( TR_CARD_SV_PTR inTrCard );

#endif //__TRAINERCARD_DATA_H__

