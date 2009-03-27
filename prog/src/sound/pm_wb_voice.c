//============================================================================================
/**
 * @file	pm_wb_voice.c
 * @brief	ポケモンＷＢ用鳴き声再生コールバック関数
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
#include "sound/wb_sound_data.sadl"		//サウンドラベルファイル

#include "sound/pm_wb_voice.h"
//============================================================================================
/**
 *
 *
 * @brief	波形ＩＤＸ取得
 *
 *
 */
//============================================================================================
void PMWB_GetVoiceWaveIdx(	u32 pokeNo, 		// [in]ポケモンナンバー
							u32 pokeFormNo,		// [in]ポケモンフォームナンバー
							u32* waveIdx )		// [out]波形IDX
{
	if(( pokeNo < PMVOICE_START)&&( pokeNo > PMVOICE_END) ){
		//指定範囲外
		*waveIdx = PMVOICE_POKE001;
		return;
	}
	if( pokeNo == MONSNO_EURISU ){					//シェイミの時のみ、フォルムチェック
		if( pokeFormNo == FORMNO_SHEIMI_FLOWER ){	//スカイフォルム
			*waveIdx = WAVE_ARC_PV516_SKY;
			return;
		}
	}
	// 波形ＩＤＸ取得
	*waveIdx = (pokeNo-1) + PMVOICE_POKE001;	// 1origin, PMVOICE_POKE001～
}

//============================================================================================
/**
 *
 *
 * @brief	波形データ取得
 *
 *
 */
//============================================================================================
BOOL PMWB_CustomVoiceWave(	u32 pokeNo,			// [in]ポケモンナンバー
							u32 pokeFormNo,		// [in]ポケモンフォームナンバー
							void** wave,		// [out]波形データ
							u32* size,			// [out]波形サイズ
							int* rate,			// [out]波形再生レート
							int* speed )		// [out]波形再生スピード
{
	return FALSE;
}

