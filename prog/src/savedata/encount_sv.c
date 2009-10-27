//============================================================================================
/**
 * @file	encount.c
 * @brief	エンカウント関連アクセス用ソース
 * @author	iwasawa GAME FREAK inc.
 * @date	2009.10.27
 */
//============================================================================================

#include "gflib.h"
#include "savedata/save_tbl.h"
#include "savedata/encount_sv.h"
#include "system/gfl_use.h"
//#include "field/move_pokemon_def.h"

#define MOVE_POKE_MAX (1) ///@todo

//自機のゾーン足跡
typedef struct PLAYER_ZONE_HIST_tag
{
	int beforeZone;	//前回ゾーン
	int oldZone;	//前々回ゾーン
}PLAYER_ZONE_HIST;

//移動ポケモンデータ
typedef struct MV_POKE_DATA_tag{

	int zoneID;			//ゾーンＩＤ
	u32	pow_rnd;		// パワー乱数
	u32	personal_rnd;	// 個性乱数

	u16	mons_no;				// ポケモンナンバー
	u16	hp;				// HP

	u8	level;				// level
	u8	cond;			// 状態異常

	u8	encount_f;		// エンカウントフラグ
	u8	dummy;

}MV_POKE_DATA;

typedef struct ENC_SV_DATA_tag
{
	int GenerateRandSeed;		//大量発生ランダムの種
	PLAYER_ZONE_HIST	PlayerZoneHist;	//ゾーン履歴
	MV_POKE_DATA	MovePokeData[MOVE_POKE_MAX];	//移動ポケモンデータ
	u8 MovePokeZoneIdx[MOVE_POKE_MAX];	//移動ポケモン現在ゾーンインデックス
	u8 GenerateValid;					//大量発生発動フラグ
	u8 SprayCount;						//スプレー有効歩数(最大250歩)
	
}ENC_SV_DATA;

//----------------------------------------------------------
/**
 * @brief	エンカウント関連セーブデータサイズ取得
 * 
 * @return	int		ENC_SV_DATAのサイズ
 */
//----------------------------------------------------------
int EncDataSave_GetWorkSize(void)
{
	return sizeof(ENC_SV_DATA);
}

//==============================================================================
/**
 * セーブデータ先頭ポインタ取得
 *
 * @param	sv		セーブポインタ
 *
 * @return	ENC_SV_PTR		トレーナーカードデータポインタ
 */
//==============================================================================
ENC_SV_PTR EncDataSave_GetSaveDataPtr(SAVE_CONTROL_WORK * sv)
{
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_ENCOUNT)
	SVLD_SetCrc(GMDATA_ID_ENCOUNT);
#endif //CRC_LOADCHECK

	return SaveControl_DataPtrGet(sv, GMDATA_ID_ENCOUNT);
}

//----------------------------------------------------------
/**
 * @brief		エンカウント関連セーブデータの初期化
 * 
 * @param	outEncData		エンカウント関連セーブデータへのポインタ
 *
 * @return	none
 */
//----------------------------------------------------------
void EncDataSave_Init(ENC_SV_PTR outEncData)
{
	MI_CpuClear8( outEncData, sizeof(ENC_SV_DATA) );

	//ランダムの種セット
	outEncData->GenerateRandSeed = GFUser_GetPublicRand(GFL_STD_RAND_MAX);
	
	//大量発生発動フラグを落とす
	outEncData->GenerateValid = 0;
	//スプレー
	outEncData->SprayCount = 0;

#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_ENCOUNT)
	SVLD_SetCrc(GMDATA_ID_ENCOUNT);
#endif //CRC_LOADCHECK

}

//----------------------------------------------------------
/**
 * @brief		エンカウント関連ランダムの種更新
 * 
 * @param	outEncData		エンカウント関連セーブデータへのポインタ
 *
 * @return	none
 */
//----------------------------------------------------------
void EncDataSave_UpdateRandSeed(ENC_SV_PTR ioEncData, const u32 inRandSeed)
{
	ioEncData->GenerateRandSeed = inRandSeed;

#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_ENCOUNT)
	SVLD_SetCrc(GMDATA_ID_ENCOUNT);
#endif //CRC_LOADCHECK

}

//----------------------------------------------------------
/**
 * @brief	エンカウント関連ランダムの種取得
 * 
 * @param	inEncData		エンカウント関連セーブデータへのポインタ
 * @param	inSeedType		ランダムの種の種類（encount.hに意義）
 *
 * @return	u32				サファリランダムの種
 */
//----------------------------------------------------------
u32 EncDataSave_GetRandSeed(ENC_SV_PTR inEncData, const u8 inSeedType)
{
	switch(inSeedType){
	case ENC_RND_SEED_GENERATE:
		return inEncData->GenerateRandSeed;
	default:
		GF_ASSERT(0);
		return 0;
	}
}

//==============================================================================
/**
 * 大量発生開始
 *
 * @param	sv				セーブポインタ
 *
 * @return	none
 */
//==============================================================================
void EncDataSave_StartGenerate( SAVE_CONTROL_WORK * sv )
{
	ENC_SV_PTR enc_data;
	enc_data = EncDataSave_GetSaveDataPtr(sv);
	enc_data->GenerateValid = 1;

#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_ENCOUNT)
	SVLD_SetCrc(GMDATA_ID_ENCOUNT);
#endif //CRC_LOADCHECK

}

//==============================================================================
/**
 * 大量発生中かを返す
 *
 * @param	inEncData			エンカウント関連セーブポインタ
 *
 * @return	u8 0以外：大量発生中
 */
//==============================================================================
u8 EncDataSave_IsGenerate( ENC_SV_PTR inEncData )
{
	return inEncData->GenerateValid;
}

//==============================================================================
/**
 * 主人公のゾーン履歴の更新
 *
 * @param	outEncData			エンカウント関連セーブポインタ
 * @param	inzoneID			ゾーンＩＤ
 */
//==============================================================================
void EncDataSave_UpdatePlayerZoneHist(ENC_SV_PTR ioEncData, const int inzoneID)
{
	if (ioEncData->PlayerZoneHist.beforeZone != inzoneID){
		ioEncData->PlayerZoneHist.oldZone = ioEncData->PlayerZoneHist.beforeZone;
		ioEncData->PlayerZoneHist.beforeZone = inzoneID;
	}
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_ENCOUNT)
	SVLD_SetCrc(GMDATA_ID_ENCOUNT);
#endif //CRC_LOADCHECK
}

//==============================================================================
/**
 * 主人公の前々回いたゾーンを取得
 *
 * @param	inEncData		エンカウント関連セーブポインタ
 *
 * @return	int				ゾーンＩＤ
 */
//==============================================================================
int EncDataSave_GetPlayerOldZone(ENC_SV_PTR inEncData)
{
	return inEncData->PlayerZoneHist.oldZone;
}

//==============================================================================
/**
 * 指定移動ポケモンのゾーンインデックスを取得
 *
 * @param	inEncData		エンカウント関連セーブポインタ
 * @param	inTargetPoke	移動ポケモン(0～2)
 *
 * @return	u8				ゾーンインデックス
 */
//==============================================================================
u8 EncDataSave_GetMovePokeZoneIdx(ENC_SV_PTR inEncData, const u8 inTargetPoke)
{
	GF_ASSERT(inTargetPoke<MOVE_POKE_MAX);
	
	return inEncData->MovePokeZoneIdx[inTargetPoke];
}

//==============================================================================
/**
 * 指定移動ポケモンのゾーンインデックスをセット
 *
 * @param	outEncData		エンカウント関連セーブポインタ
 * @param	inTargetPoke	移動ポケモン(0～2)
 * @param	inZoneIndex		ゾーンインデックス
 *
 * @return	none
 */
//==============================================================================
void EncDataSave_SetMovePokeZoneIdx(ENC_SV_PTR outEncData, const u8 inTargetPoke, const u8 inZoneIndex)
{
	GF_ASSERT(inTargetPoke<MOVE_POKE_MAX);
	outEncData->MovePokeZoneIdx[inTargetPoke] = inZoneIndex;

#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_ENCOUNT)
	SVLD_SetCrc(GMDATA_ID_ENCOUNT);
#endif //CRC_LOADCHECK

}

//==============================================================================
/**
 * 指定移動ポケモンのエンカウントフラグみて、移動しているかどうかをチェック
 *
 * @param	inEncData		エンカウント関連セーブポインタ
 * @param	inTargetPoke	移動ポケモン(0～2)
 *
 * @return	u8			0以外:移動してる	0:移動してない
 */
//==============================================================================
u8 EncDataSave_IsMovePokeValid(ENC_SV_PTR inEncData, const u8 inTargetPoke)
{
	GF_ASSERT(inTargetPoke<MOVE_POKE_MAX);
	return inEncData->MovePokeData[inTargetPoke].encount_f;
}

//==============================================================================
/**
 * 指定移動ポケモンデータのクリア
 *
 * @param	inEncData		エンカウント関連セーブポインタ
 * @param	inTargetPoke	移動ポケモン(0～2)
 *
 * @return	none
 */
//==============================================================================
void EncDataSave_ClearMovePokeData(MPD_PTR *outMPData)
{
	MI_CpuClear8((*outMPData), sizeof(MV_POKE_DATA));

#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_ENCOUNT)
	SVLD_SetCrc(GMDATA_ID_ENCOUNT);
#endif //CRC_LOADCHECK

}

//==============================================================================
/**
 * 指定移動ポケモンデータポインタの取得
 *
 * @param	inEncData		エンカウント関連セーブポインタ
 * @param	inTargetPoke	移動ポケモン(0～2)
 *
 * @return	MPD_PTR			移動ポケモンデータ
 */
//==============================================================================
MPD_PTR	EncDataSave_GetMovePokeDataPtr(ENC_SV_PTR inEncData, const u8 inTargetPoke)
{
	GF_ASSERT(inTargetPoke<MOVE_POKE_MAX);
	return &(inEncData->MovePokeData[inTargetPoke]);
}

//==============================================================================
/**
 * 移動ポケモンデータパラメータ取得
 *
 * @param	inMPData		移動尾ポケモンデータポインタ
 * @param	inParamID		パラメータＩＤ
 *
 * @return	u32
 */
//==============================================================================
u32 EncDataSave_GetMovePokeDataParam(const MPD_PTR inMPData, const u8 inParamID)
{
	u32 val;
	switch(inParamID){
	case MP_PARAM_ZONE_ID:			//出現ゾーン
		val = inMPData->zoneID;
		break;
	case MP_PARAM_POW_RND:			//パワー乱数
		val = inMPData->pow_rnd;
		break;
	case MP_PARAM_PER_RND:			//個性乱数
		val = inMPData->personal_rnd;
		break;
	case MP_PARAM_MONSNO:			//モンスターナンバー
		val = inMPData->mons_no;
		break;
	case MP_PARAM_HP:				//ＨＰ
		val = inMPData->hp;
		break;
	case MP_PARAM_LV:				//ＬＶ
		val = inMPData->level;
		break;
	case MP_PARAM_COND:				//状態異常
		val = inMPData->cond;
		break;
	case MP_PARAM_ENC:				//エンカウントフラグ
		val = inMPData->encount_f;
		break;
	default:
		GF_ASSERT(0);
		return 0;
	}
	return val;
}

//==============================================================================
/**
 * 移動ポケモンデータパラメータセット
 *
 * @param	EncData		エンカウント関連セーブポインタ
 * @param	inParamID	パラメータＩＤ
 * @param	inVal		セットする値
 *
 * @return	none
 */
//==============================================================================
void EncDataSave_SetMovePokeDataParam(MPD_PTR outMPData, const u8 inParamID, const u32 inVal)
{
	u32 val;
	switch(inParamID){
	case MP_PARAM_ZONE_ID:			//出現ゾーン
		outMPData->zoneID = inVal;
		break;
	case MP_PARAM_POW_RND:			//パワー乱数
		outMPData->pow_rnd = inVal;
		break;
	case MP_PARAM_PER_RND:			//個性乱数
		outMPData->personal_rnd = inVal;
		break;
	case MP_PARAM_MONSNO:			//モンスターナンバー
		outMPData->mons_no = inVal;
		break;
	case MP_PARAM_HP:				//ＨＰ
		outMPData->hp = inVal;
		break;
	case MP_PARAM_LV:				//ＬＶ
		outMPData->level = inVal;
		break;
	case MP_PARAM_COND:				//状態異常
		outMPData->cond = inVal;
		break;
	case MP_PARAM_ENC:				//エンカウントフラグ
		outMPData->encount_f = inVal;
		break;
	default:
		GF_ASSERT(0);
	}

#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_ENCOUNT)
	SVLD_SetCrc(GMDATA_ID_ENCOUNT);
#endif //CRC_LOADCHECK

}


//==============================================================================
/**
 * スプレー有効歩数を格納したメンバのポインタを取得
 *
 * @param	inEncData		エンカウント関連セーブポインタ
 *
 * @return	u8*				スプレー有効歩数ポインタ
 */
//==============================================================================
u8 *EncDataSave_GetSprayCnt(ENC_SV_PTR inEncData)
{
	return &(inEncData->SprayCount);
}

//==============================================================================
/**
 * スプレーを使用できるかどうかのチェック
 * 
 * @param	inEncData		エンカウント関連セーブポインタ
 *
 * @return	BOOL	TRUE:使用可		FALSE:使用不可
 */
//==============================================================================
BOOL EncDataSave_CanUseSpray(ENC_SV_PTR inEncData)
{
	if (!inEncData->SprayCount){
		return TRUE;
	}else{
		return FALSE;
	}
}

