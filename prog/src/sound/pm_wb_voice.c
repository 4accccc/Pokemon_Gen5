//============================================================================================
/**
 * @file	pm_wb_voice.c
 * @brief	ポケモンＷＢ用鳴き声再生コールバック関数
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "system/gfl_use.h"

#include "savedata/save_control.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
#include "sound/wb_sound_data.sadl"		//サウンドラベルファイル

#include "sound/pm_wb_voice.h"

//==============================================================================================
//
//	サンプリングデータ用定義
//
//==============================================================================================
//通常
#define VOICE_VOLUME_DEFAULT	(120)			//seq設定(sound_data.sarc)内(PV_VOL)を参照
#define VOICE_RATE_DEFAULT		(13379)		//サンプリングレート

// ぺラップ
#define PERAP_SAMPLING_RATE		(2000)		//サンプリングレート
#define PERAP_SAMPLING_TIME		(1)				//サンプリング時間
#define PERAP_SAMPLING_SIZE		(PERAP_SAMPLING_RATE * PERAP_SAMPLING_TIME)	//データ量

#define PERAP_WAVE_SPD				(32768)		//再生スピード(１倍速)
#define PERAP_WAVE_SPDRAND		(8192)		//再生スピードのランダム
extern PERAPVOICE * SaveData_GetPerapVoice(SAVE_CONTROL_WORK * sv);
//============================================================================================
/**
 *
 *
 * @brief	波形ＩＤＸ取得
 *
 *
 */
//============================================================================================
void PMV_GetVoiceWaveIdx(	u32 pokeNo, 		// [in]ポケモンナンバー
							u32 pokeFormNo,		// [in]ポケモンフォームナンバー
							u32* waveIdx )		// [out]波形IDX
{
	if(( pokeNo < MONSNO_HUSIGIDANE)&&( pokeNo > MONSNO_END) ){
		//指定範囲外
		*waveIdx = PMVOICE_POKE001;
		return;
	}
	if( pokeNo == MONSNO_SHEIMI ){					//シェイミの時のみ、フォルムチェック
		if( pokeFormNo == FORMNO_SHEIMI_SKY ){	//スカイフォルム
			*waveIdx = WAVE_ARC_PV492_SKY;
			return;
		}
	}
	// 波形ＩＤＸ取得
	*waveIdx = (pokeNo-1) + PMVOICE_POKE001;	// pokeNo(1origin)を変換。PMVOICE_POKE001～
}

//============================================================================================
/**
 *
 *
 * @brief	波形データカスタマイズ
 *
 *
 */
//============================================================================================
BOOL PMV_CustomVoiceWave(	u32			pokeNo,			// [in]ポケモンナンバー
													u32			pokeFormNo,	// [in]ポケモンフォームナンバー
													u32			userParam,	// [in]ユーザーパラメーター
													void**	wave,				// [out]波形データ
													u32*		size,				// [out]波形サイズ(MAX 26000)
													int*		rate,				// [out]波形再生レート
													int*		speed,			// [out]波形再生スピード
													s8*			volume)			// [out]再生ボリューム
{
	PMV_REF* pmvRef;

	//ボリューム＆サンプリングレート設定
	*volume = VOICE_VOLUME_DEFAULT;
	*rate		= VOICE_RATE_DEFAULT;

	if( userParam == NULL ){ return FALSE; }	// 既存waveのみ使用

	//参照ユーザーパラメータ設定
	pmvRef = (PMV_REF*)userParam;

	if(( pokeNo == MONSNO_PERAPPU )&&( pokeFormNo == 0 )){	//ぺラップ判定
		//録音データ存在判定
		if( PERAPVOICE_GetExistFlag(pmvRef->perapVoice) == TRUE ){
			//録音データ展開
			PERAPVOICE_ExpandVoiceData( *wave, PERAPVOICE_GetVoiceData(pmvRef->perapVoice) );
			*size = PERAP_SAMPLING_SIZE;
			*rate = PERAP_SAMPLING_RATE;
			//ランダムに音程を変える仕様らしい
			*speed = PERAP_WAVE_SPD + GFUser_GetPublicRand( PERAP_WAVE_SPDRAND );
			return TRUE;
		}
	}
	return FALSE;
}

//============================================================================================
/**
 *
 *
 * @brief	参照データ作成（自分のぺラップ用データより）
 *
 *
 */
//============================================================================================
void PMV_MakeRefDataMine( PMV_REF* pmvRef )
{
	//ぺラップの録音データステータスポインタをセーブデータより取得
	pmvRef->perapVoice = SaveData_GetPerapVoice(SaveControl_GetPointer());
}







