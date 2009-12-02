//=============================================================================
/**
 * @file	encount_sv.h
 * @brief	エンカウント関連セーブデータアクセス関連
 * @author	Miyuki Iwasawa
 */
//=============================================================================
#pragma once

#include "savedata/save_control.h"

#define ENC_RND_SEED_GENERATE	(1)
#define ENC_RND_SEED_UNKNOWN	(0)		//使わないけど一応

//移動ポケモンパラメータＩＤ
#define MP_PARAM_ZONE_ID	(1)		//ゾーンＩＤ
#define MP_PARAM_POW_RND	(2)		//パワー乱数
#define MP_PARAM_PER_RND	(3)		//個性乱数
#define MP_PARAM_MONSNO		(4)		//モンスターナンバー
#define MP_PARAM_HP			(5)		//ＨＰ
#define MP_PARAM_LV			(6)		//ＬＶ
#define MP_PARAM_COND		(7)		//状態異常
#define MP_PARAM_ENC		(8)		//エンカウントフラグ（現在移動中かどうか）

typedef struct ENC_SV_DATA_tag * ENC_SV_PTR;
typedef struct MV_POKE_DATA_tag * MPD_PTR;

extern int EncDataSave_GetWorkSize(void);
extern ENC_SV_PTR EncDataSave_GetSaveDataPtr(SAVE_CONTROL_WORK * sv);
extern void EncDataSave_Init(ENC_SV_PTR outEncData);

extern void EncDataSave_UpdateGenerate( SAVE_CONTROL_WORK * sv );
extern u8 EncDataSave_GetGenerateZoneIdx( ENC_SV_PTR inEncData );

extern void EncDataSave_UpdatePlayerZoneHist(ENC_SV_PTR ioEncData, const int inZoneID);
extern int EncDataSave_GetPlayerOldZone(ENC_SV_PTR inEncData);
extern u8 EncDataSave_GetMovePokeZoneIdx(ENC_SV_PTR inEncData, const u8 inTargetPoke);
extern void EncDataSave_SetMovePokeZoneIdx(	ENC_SV_PTR outEncData,
											const u8 inTargetPoke,
											const u8 inZoneIndex	);
extern u8 EncDataSave_IsMovePokeValid(ENC_SV_PTR inEncData, const u8 inTargetPoke);
extern void EncDataSave_ClearMovePokeData(MPD_PTR *outMPData);
extern MPD_PTR	EncDataSave_GetMovePokeDataPtr(ENC_SV_PTR inEncData, const u8 inTargetPoke);
extern u32 EncDataSave_GetMovePokeDataParam(const MPD_PTR inMPData,
											const u8 inParamID);
extern void EncDataSave_SetMovePokeDataParam(	MPD_PTR outMPData,
												const u8 inParamID,
												const u32 inVal	);
extern void EncDataSave_SetSprayCnt(ENC_SV_PTR inEncData,u8 count);
extern BOOL EncDataSave_DecSprayCnt(ENC_SV_PTR inEncData);
extern BOOL EncDataSave_CanUseSpray(ENC_SV_PTR inEncData);

