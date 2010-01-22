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

#include "sound/sound_data.h"

//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE //通常時処理
#include "sound/wb_sound_data.sadl"		//サウンドラベルファイル
#else
#include "multiboot/wb_sound_palpark.sadl"
#define GROUP_GLOBAL (0)
#define SEQ_SE_END (1400)

// とりあえずエラー回避用に定義
#define BANK_MUS_WB_SHINKA	(1010)
#define SEQ_BGM_SHINKA	(1010)

#endif //MULTI_BOOT_MAKE
//------------------------------------------------------------------
/**
 * @brief	データＩＮＤＥＸ有効範囲
 */
//------------------------------------------------------------------
#define PMSND_BGM_START		(BGM_START)
#define PMSND_BGM_END			(SEQ_BGM_END)
#define PMSND_SE_START		(SE_START)
#define PMSND_SE_END			(SEQ_SE_END)

//------------------------------------------------------------------
/**
 * @brief	ＳＥプレーヤー設定定義
 */
//------------------------------------------------------------------
typedef enum {
	SEPLAYER_SYS = 0,
	SEPLAYER_SE1,
	SEPLAYER_SE2,
	SEPLAYER_SE_PSG,

}SEPLAYER_ID;

#define SEPLAYER_MAX (4)
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
extern void	PMSND_InitMultiBoot( void* sndData );
extern void	PMSND_Main( void );
extern void	PMSND_Exit( void );

//	情報取得
#ifdef PM_DEBUG
extern u32 PMSND_GetSndHeapSize( void );
extern u32 PMSND_GetSndHeapRemainsAfterSys( void );
extern u32 PMSND_GetSndHeapRemainsAfterPlayer( void );
extern u32 PMSND_GetSndHeapRemainsAfterPresetSE( void );
extern u32 PMSND_GetSEPlayerNum( void );
#endif
extern u32						PMSND_GetSndHeapSize( void );
extern u32						PMSND_GetSndHeapFreeSize( void );
extern NNSSndHandle*	PMSND_GetNowSndHandlePointer( void );
extern NNSSndHandle*	PMSND_GetBGMhandlePointer( void );
extern u32						PMSND_GetBGMsoundNo( void );
extern u32						PMSND_GetNextBGMsoundNo( void );	//再生予定or再生中のIDX取得
extern u32						PMSND_GetBGMplayerNoIdx( void );
extern BOOL						PMSND_CheckOnReverb( void );
extern u8							PMSND_GetBGMTrackVolume( int trackNo );
extern BOOL           PMSND_IsLoading( void );  // ロード中かどうか

//	リバーブ設定
extern void PMSND_EnableCaptureReverb( u32 depth, u32 samplingRate, int volume, int stopFrames );
extern void PMSND_DisableCaptureReverb( void );
extern void PMSND_ChangeCaptureReverb( u32 depth, u32 samplingRate, int volume, int stopFrames );

//============================================================================================
/**
 *
 * @brief	ＢＧＭサウンド関数（各appから呼び出される）
 *
 */
//============================================================================================
//ＢＧＭを再生
extern void	PMSND_PlayBGM_EX( u32 soundIdx, u16 trackBit );
#define PMSND_PlayBGM( soundIdx ) PMSND_PlayBGM_EX( soundIdx, 0xffff )	//上記簡易版
//ＢＧＭ自動フェード再生
extern void	PMSND_PlayNextBGM_EX
		( u32 soundIdx, u16 trackBit, u8 fadeOutFrame, u8 fadeInFrame );	
#define PMSND_PlayNextBGM( soundIdx, fadeOutFrame, fadeInFrame ) \
					PMSND_PlayNextBGM_EX( soundIdx, 0xffff, fadeOutFrame, fadeInFrame );	//上記簡易版
//ＢＧＭ終了検出(TRUE実行中)
extern BOOL	PMSND_CheckPlayBGM( void );
//ＢＧＭの再生トラック変更
extern void	PMSND_ChangeBGMtrack( u16 trackBit );
//ＢＧＭステータス変更
extern void	PMSND_SetStatusBGM_EX( u16 trackBit, int tempoRatio, int pitch, int pan );
#define PMSND_SetStatusBGM( tempoRatio, pitch, pan ) \
          PMSND_SetStatusBGM_EX( 0xffff, tempoRatio, pitch, pan )   // 上記簡易版
//ＢＧＭボリューム変更
extern void PMSND_ChangeBGMVolume( u16 trackBit, int volume );

//現在のＢＧＭを停止
extern void	PMSND_StopBGM( void );
//現在のＢＧＭを一時停止(TRUE停止,FALSE再開)
extern void	PMSND_PauseBGM( BOOL pauseFlag );
//現在のＢＧＭをフェードイン
extern void	PMSND_FadeInBGM( u16 frames );
//現在のＢＧＭをフェードアウト
extern void	PMSND_FadeOutBGM( u16 frames );
//フェード実行チェック(TRUE実行中)
extern BOOL	PMSND_CheckFadeOnBGM( void );

//現在のＢＧＭを退避
extern void	PMSND_PushBGM( void );
//現在のＢＧＭを復元
extern void	PMSND_PopBGM( void );

//システムフェード（自動）フレーム設定
extern void PMSND_SetSystemFadeFrames( int fadeOutFrame, int fadeInFrame );

// 分割ロード・再生
extern BOOL PMSND_PlayBGMdiv(u32 no, u32* seq, BOOL start);

//============================================================================================
/**
 *
 * @brief	ＳＥサウンド関数（各appから呼び出される）
 *
 */
//============================================================================================
//ＳＥ初期設定SEPLAYER_ID取得
extern SEPLAYER_ID	PMSND_GetSE_DefaultPlayerID( u32 soundIdx );
//ＳＥプレーヤーハンドル取得
extern NNSSndHandle* PMSND_GetSE_SndHandle( SEPLAYER_ID sePlayerID );
//ＳＥプレーヤーボリュームセット
extern void PMSND_PlayerSetInitialVolume( SEPLAYER_ID sePlayerID, u32 vol );

extern void	PMSND_PlaySE_byPlayerID( u32 soundIdx, SEPLAYER_ID sePlayerID );
#define PMSND_PlaySystemSE( soundNum ) PMSND_PlaySE( soundNum )
//ＳＥを再生
extern void	PMSND_PlaySE( u32 soundIdx );
//ＳＥをボリューム指定付きで再生(有効値 0-127)
extern void	PMSND_PlaySEVolume( u32 soundIdx, u32 volume );
//ＳＥをプレーヤーを指定して再生
extern void	PMSND_PlaySE_byPlayerID( u32 soundIdx, SEPLAYER_ID sePlayerID );
//全ＳＥを停止
extern void	PMSND_StopSE( void );
//ＳＥをプレーヤーを指定して停止
extern void	PMSND_StopSE_byPlayerID( SEPLAYER_ID sePlayerID );
//全ＳＥの終了を検出(TRUE実行中)
extern BOOL	PMSND_CheckPlaySE( void );
//ＳＥの終了をプレーヤーを指定して終了検出(TRUE実行中)
extern BOOL	PMSND_CheckPlaySE_byPlayerID( SEPLAYER_ID sePlayerID );
//ＳＥのステータスを変更
extern void	PMSND_SetStatusSE( int tempoRatio, int pitch, int pan );
//ＳＥのステータスをプレーヤーを指定して変更
extern void	PMSND_SetStatusSE_byPlayerID(SEPLAYER_ID sePlayerID,int tempoRatio,int pitch,int pan);
//再生中のＳＥが入力されたＩＤＸと一致するかチェック
extern BOOL	PMSND_CheckPlayingSEIdx( u32 soundIdx );
//============================================================================================
/**
 *
 *
 * @brief	プリセット関数ラッパー
 *					とりあえずシステムでハンドルを１つだけ用意し簡易登録可能にする
 *
 *
 */
//============================================================================================
extern void	PMDSND_PresetSoundTbl( const u32* soundIdxTbl, u32 tblNum );
extern void	PMSND_PresetGroup( u32 groupIdx );
extern void	PMSND_ReleasePreset( void );

//============================================================================================
/**
 *
 *
 * @brief	外部音楽データ再生
 *
 *
 */
//============================================================================================
extern BOOL PMDSND_PresetExtraMusic( void* seqAdrs, void* bnkAdrs, u32 dummyNo );
extern BOOL PMDSND_ChangeWaveData( u32 waveNo, u32 waveIdx, void* waveAdrs); 
extern BOOL PMDSND_PlayExtraMusic( u32 dummyNo );
extern void PMDSND_StopExtraMusic( void );
extern void PMDSND_ReleaseExtraMusic( void );


//サウンドテスト用
//※現在マップ切り替わり時のデータ取得と戦闘などSubProcから帰ってくる
//　場所が切り分けられていないので仮でフラグ管理する
extern BOOL debugBGMsetFlag;
#endif
