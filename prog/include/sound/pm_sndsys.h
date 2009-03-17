//============================================================================================
/**
 * @file	pm_sndsys.h
 * @brief	ポケモンサウンドシステム
 * @author	
 * @date	
 */
//============================================================================================
#ifndef __PMSNDSYS_H__
#define __PMSNDSYS_H__

#include "sound/wb_sound_data.sadl"		//サウンドラベルファイル
//------------------------------------------------------------------
/**
 * @brief	データＩＮＤＥＸ有効範囲
 */
//------------------------------------------------------------------
#define PMSND_BGM_START		(SEQ_MUS_GS_BICYCLE)
#define PMSND_BGM_END		(SEQ_MUS_WB_WIN4)
#define PMSND_SE_START		(SEQ_SE_DP_000)
#define PMSND_SE_END		(SEQ_TEST_TITLE)
#define PMSND_VOICE_START	(BANK_PV001)
#define PMSND_VOICE_END		(BANK_PV516_SKY)
#define PMSND_POKEVOICE_001	(BANK_PV001)

//------------------------------------------------------------------
/**
 * @brief	関数引数用定義
 */
//------------------------------------------------------------------
#define PMSND_NOEFFECT (-1)

//============================================================================================
/**
 *
 * @brief	サウンドシステム（main.c等から呼び出される）
 *
 */
//============================================================================================
extern void	PMSND_Init( void );
extern void	PMSND_Main( void );
extern void	PMSND_Exit( void );

//============================================================================================
/**
 *
 * @brief	サウンド関数（各appから呼び出される）
 *
 */
//============================================================================================
extern NNSSndHandle* PMSND_GetBGMhandlePointer( void );

extern BOOL	PMSND_PlayBGM( u32 soundIdx );					//ＢＧＭを再生
extern BOOL	PMSND_PlayBGM_EX( u32 soundIdx, u16 trackBit );	//上記拡張
extern BOOL	PMSND_PlayNextBGM( u32 soundIdx );				//ＢＧＭフェードインアウト有りで再生
extern BOOL	PMSND_PlayNextBGM_EX( u32 soundIdx, u16 trackBit );//上記拡張
extern void	PMSND_ChangeBGMtrack( u16 trackBit );			//ＢＧＭの再生トラック変更
extern BOOL	PMSND_CheckPlayBGM( void );						//ＢＧＭ終了検出
extern void	PMSND_SetStatusBGM
				( int tempoRatio, int pitch, int pan );//ＢＧＭステータス変更

extern void	PMSND_StopBGM( void );				//現在のＢＧＭを停止
extern void	PMSND_PauseBGM( BOOL pauseFlag );	//現在のＢＧＭを一時停止(TRUE停止,FALSE再開)
extern void	PMSND_FadeInBGM( u16 frames );		//現在のＢＧＭをフェードイン
extern void	PMSND_FadeOutBGM( u16 frames );		//現在のＢＧＭをフェードアウト
extern BOOL	PMSND_CheckFadeOnBGM( void );		//フェード実行チェック(TRUE実行中)

extern void	PMSND_PushBGM( void );				//現在のＢＧＭを退避
extern void	PMSND_PopBGM( void );				//現在のＢＧＭを復元

extern BOOL	PMSND_PlaySystemSE( u32 soundNum );	//システムＳＥを再生
extern BOOL	PMSND_PlaySE( u32 soundNum );		//ＳＥを再生
extern BOOL	PMSND_PlayVoice( u32 pokeNum );		//鳴き声を再生
extern BOOL	PMSND_CheckPlaySEVoice( void );		//ＳＥ終了検出
extern void	PMSND_SetStatusSEVoice
				( int tempoRatio, int pitch, int pan );//ＳＥ＆鳴き声ステータス変更
extern BOOL	PMSND_SetEchoChorus( u32 wait, int pitch );

//============================================================================================
/**
 *
 * @brief	システムフェードフレーム設定（各appから呼び出される）
 *
 */
//============================================================================================
extern void PMSND_SetSystemFadeFrames( int frames );

//============================================================================================
/**
 *
 * @brief	システムエコー＆コーラス設定（各appから呼び出される）
 *
 */
//============================================================================================
extern void PMSND_EnableSystemEchoChorus( int volume, int pitch, int waitFrames );
extern void PMSND_DisableSystemEchoChorus( void );

//サウンドテスト用
//※現在マップ切り替わり時のデータ取得と戦闘などSubProcから帰ってくる
//　場所が切り分けられていないので仮でフラグ管理する
extern BOOL debugBGMsetFlag;

#endif
