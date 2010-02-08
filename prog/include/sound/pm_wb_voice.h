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
#define PMVOICE_START			(BANK_PV001)
#define PMVOICE_END				(WAVE_ARC_PV492_SKY)
#define PMVOICE_POKE001		(PMVOICE_START)

//------------------------------------------------------------------
/**
 * @brief	鳴き声ユーザーパラメータ構造体
 */
//------------------------------------------------------------------
typedef struct {
	PERAPVOICE*	perapVoice;
}PMV_REF;

//------------------------------------------------------------------
/**
 * @brief	pan（定位）設定定義(左:0 - 64 - 127:右)
 */
//------------------------------------------------------------------
#define PMV_PAN_L (0)
#define PMV_PAN_C (64)
#define PMV_PAN_R (127)

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
extern BOOL	PMV_CustomVoiceWave( u32, u32, u32, void**, u32*, int*, int*, s8* );

//============================================================================================
/**
 * @brief	参照データ作成（自分のぺラップ用データより）
 */
//============================================================================================
extern void PMV_MakeRefDataMine( PMV_REF* pmvRef );





//============================================================================================
/**
 *
 * @brief	鳴き声操作インライン関数
 *
 */
//============================================================================================
// 停止（再生中の全鳴き声が対象）
inline void PMV_StopVoice( void ){ PMVOICE_Reset(); }

// 終了検出（再生中の全鳴き声が対象）
inline BOOL PMV_CheckPlay( void ){ return PMVOICE_CheckBusy(); }


//============================================================================================
/**
 * @brief	鳴き声再生インライン関数
 *
 *	※インライン関数として３種
 *	　・無印		：すべて波形データを再生
 *	　・_forMine	：セーブデータの内容を参照して再生（手持ちポケモンを想定）
 *	　・_forMulti	：外部から渡された内容を参照して再生（マルチプレイを想定）
 *	があり。
 *
 *	それぞれ
 *	　・無印		：通常再生
 *	　・_Pan		：定位設定をして再生
 *	　・_Chorus		：コーラス設定をして再生
 *	　・_Reverse	：逆再生
 *	　・_Custom		：フルカスタマイズ
 *	となっている。
 *			
 *	引数説明
 *	　・pokeNo		：ポケモンナンバー
 *	　・formNo		：ポケモンフォームナンバー
 *	　・pan			：定位(左:0 - 64 - 127:右)
 *	　・chorus		：コーラス使用フラグ
 *	　・ch_vol		：コーラスボリューム差(0 = 元の波形との差分なし)
 *	　・ch_spd		：再生速度差(0 = 元の波形との差分なし)
 *	　・reverse		：逆再生フラグ
 *	　・pmvRef		：参照データ
 */
//============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * @brief	無印
 */
//--------------------------------------------------------------------------------------------
inline u32 PMV_PlayVoice( u32 pokeNo, u32 formNo )
{
	return PMVOICE_Play(pokeNo, formNo, PMV_PAN_C, FALSE, 0, 0, FALSE, 0);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_Pan( u32 pokeNo, u32 formNo, u8 pan )
{
	return PMVOICE_Play(pokeNo, formNo, pan, FALSE, 0, 0, FALSE, 0);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_Chorus( u32 pokeNo, u32 formNo, int ch_vol, int ch_spd)
{
	return PMVOICE_Play(pokeNo, formNo, PMV_PAN_C, TRUE, ch_vol, ch_spd, FALSE, 0);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_Reverse( u32 pokeNo, u32 formNo)
{
	return PMVOICE_Play(pokeNo, formNo, PMV_PAN_C, FALSE, 0, 0, TRUE, 0);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_Custom
	( u32 pokeNo, u32 formNo, u8 pan, BOOL chorus, int ch_vol, int ch_spd, BOOL reverse )
{
	return PMVOICE_Play(pokeNo, formNo, pan, chorus, ch_vol, ch_spd, reverse, 0);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	_forMine
 */
//--------------------------------------------------------------------------------------------
inline u32 PMV_PlayVoice_forMine( u32 pokeNo, u32 formNo )
{
	PMV_REF pmvRef;
	PMV_MakeRefDataMine(&pmvRef);
	return PMVOICE_Play(pokeNo, formNo, PMV_PAN_C, FALSE, 0, 0, FALSE, (u32)&pmvRef);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_forMine_Pan( u32 pokeNo, u32 formNo, u8 pan )
{
	PMV_REF pmvRef;
	PMV_MakeRefDataMine(&pmvRef);
	return PMVOICE_Play(pokeNo, formNo, pan, FALSE, 0, 0, FALSE, (u32)&pmvRef);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_forMine_Chorus( u32 pokeNo, u32 formNo, int ch_vol, int ch_spd)
{
	PMV_REF pmvRef;
	PMV_MakeRefDataMine(&pmvRef);
	return PMVOICE_Play(pokeNo, formNo, PMV_PAN_C, TRUE, ch_vol, ch_spd, FALSE, (u32)&pmvRef);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_forMine_Reverse( u32 pokeNo, u32 formNo)
{
	PMV_REF pmvRef;
	PMV_MakeRefDataMine(&pmvRef);
	return PMVOICE_Play(pokeNo, formNo, PMV_PAN_C, FALSE, 0, 0, TRUE, (u32)&pmvRef);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_forMine_Custom
	( u32 pokeNo, u32 formNo, u8 pan, BOOL chorus, int ch_vol, int ch_spd, BOOL reverse )
{
	PMV_REF pmvRef;
	PMV_MakeRefDataMine(&pmvRef);
	return PMVOICE_Play(pokeNo, formNo, pan, chorus, ch_vol, ch_spd, reverse, (u32)&pmvRef);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	_forMulti
 */
//--------------------------------------------------------------------------------------------
inline u32 PMV_PlayVoice_forMulti( u32 pokeNo, u32 formNo, PMV_REF* pmvRef )
{
	return PMVOICE_Play(pokeNo, formNo, PMV_PAN_C, FALSE, 0, 0, FALSE, (u32)&pmvRef);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_forMulti_Pan( u32 pokeNo, u32 formNo, u8 pan, PMV_REF* pmvRef )
{
	return PMVOICE_Play(pokeNo, formNo, pan, FALSE, 0, 0, FALSE, (u32)&pmvRef);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_forMulti_Chorus
		( u32 pokeNo, u32 formNo, int ch_vol, int ch_spd, PMV_REF* pmvRef)
{
	return PMVOICE_Play(pokeNo, formNo, PMV_PAN_C, TRUE, ch_vol, ch_spd, FALSE, (u32)&pmvRef);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_forMulti_Reverse( u32 pokeNo, u32 formNo, PMV_REF* pmvRef)
{
	return PMVOICE_Play(pokeNo, formNo, PMV_PAN_C, FALSE, 0, 0, TRUE, (u32)&pmvRef);
}
//--------------------------------------------------------------
inline u32 PMV_PlayVoice_forMulti_Custom( u32 pokeNo, u32 formNo, u8 pan, 
						BOOL chorus, int ch_vol, int ch_spd, BOOL reverse, PMV_REF* pmvRef )
{
	return PMVOICE_Play(pokeNo, formNo, pan, chorus, ch_vol, ch_spd, reverse, (u32)&pmvRef);
}

//------------------------------------------------------------------
/**
 * @brief	鳴き声ステータス変更関数
 */
//------------------------------------------------------------------
inline void	PMV_SetPan(u32 voicePlayerIdx, u8 pan)
{
	PMVOICE_SetPan(voicePlayerIdx, pan);
}
inline void	PMV_SetVolume(u32 voicePlayerIdx, s8 volume)
{
	PMVOICE_SetVolume(voicePlayerIdx, volume);
}
inline void	PMV_SetSpeed(u32 voicePlayerIdx, int speed)
{
	PMVOICE_SetSpeed(voicePlayerIdx, speed);
}
inline u8	PMV_GetPan(u32 voicePlayerIdx)
{
	return PMVOICE_GetPan(voicePlayerIdx);
}
inline s8	PMV_GetVolume(u32 voicePlayerIdx)
{
	return PMVOICE_GetVolume(voicePlayerIdx);
}
inline int	PMV_GetSpeed(u32 voicePlayerIdx)
{
	return PMVOICE_GetSpeed(voicePlayerIdx);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	デモ分割再生用
 */
//--------------------------------------------------------------------------------------------
inline u32 PMV_LoadVoiceDemo( u32 pokeNo, u32 formNo )
{
	return PMVOICE_LoadOnly(pokeNo, formNo, PMV_PAN_C, FALSE, 0, 0, FALSE, 0);
}

inline BOOL PMV_PlayVoiceDemo( u32 voicePlayerIdx)
{
	return PMVOICE_PlayOnly(voicePlayerIdx);
}

#endif


