//============================================================================================
/**
 * @file	pm_wb_voice.h
 * @brief	ポケモンＷＢ用鳴き声再生コールバック関数
 * @author	
 * @date	
 */
//============================================================================================
#ifndef __PMWB_VOICE_H__
#define __PMWB_VOICE_H__

#include "sound/pm_voice.h"
#include "savedata/perapvoice.h"
//------------------------------------------------------------------
/**
 * @brief	データＩＮＤＥＸ有効範囲
 */
//------------------------------------------------------------------
#define PMVOICE_START	(BANK_PV001)
#define PMVOICE_END		(BANK_PV516_SKY)
#define PMVOICE_POKE001	(PMVOICE_START)

//------------------------------------------------------------------
/**
 * @brief	鳴き声ユーザーパラメータ構造体
 */
//------------------------------------------------------------------
typedef struct {
	PERAPVOICE*	perapVoice;
}PMV_REF;

//============================================================================================
/**
 * @brief	波形ＩＤＸ取得コールバック
 */
//============================================================================================
extern void	PMV_GetVoiceWaveIdx( u32, u32, u32* );
//============================================================================================
/**
 * @brief	波形データカスタマイズコールバック
 */
//============================================================================================
extern BOOL	PMV_CustomVoiceWave( u32, u32, u32, void**, u32*, int*, int* );

//============================================================================================
/**
 * @brief	参照データ作成（自分のぺラップ用データより）
 */
//============================================================================================
extern void PMV_MakeRefData( PMV_REF* pmvRef );

//============================================================================================
/**
 * @brief	鳴き声インライン関数
 */
//============================================================================================
// 通常（既存waveのみ使用）
inline void PMV_PlayVoice( u32 pokeNum, u32 formNum ){
		PMVOICE_Play(pokeNum, formNum, 64, FALSE, 0, 0, FALSE, 0);
}

// 手持ちポケモン用再生（参照データは自分のセーブデータ）
inline void PMV_PlayVoiceMine( u32 pokeNum, u32 formNum ){
		PMV_REF pmvRef;

		PMV_MakeRefData(&pmvRef);
		PMVOICE_Play(pokeNum, formNum, 64, FALSE, 0, 0, FALSE, (u32)&pmvRef);
}

// マルチプレイ用再生（参照データを外部より受け取る）
inline void PMV_PlayVoiceMulti( u32 pokeNum, u32 formNum, PMV_REF* pmvRef ){
		PMVOICE_Play(pokeNum, formNum, 64, FALSE, 0, 0, FALSE, (u32)&pmvRef);
}

//--------------------------------------------------------------------------------------------
inline void PMV_PlayVoiceChorus( u32 pokeNum, u32 formNum ){
		PMV_REF pmvRef;

		PMV_MakeRefData(&pmvRef);
		PMVOICE_Play(pokeNum, formNum, 64, TRUE, -10, 20, FALSE, 0);
}

inline void PMV_PlayVoiceRev( u32 pokeNum, u32 formNum ){
		PMV_REF pmvRef;

		PMV_MakeRefData(&pmvRef);
		PMVOICE_Play(pokeNum, formNum, 64, FALSE, 0, 0, TRUE, 0);
}

#endif


