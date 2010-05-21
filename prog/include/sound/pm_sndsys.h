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

//マルチブート時は2種類あるのでこのヘッダの前に呼んでおくこと。

#endif //MULTI_BOOT_MAKE

// リバーブ使用定義
//#define REVERB_USE

#ifdef PM_DEBUG
//------------------------------------------------------------------
/*+
 * @brief BGM分割ロードサイズ
 */
//------------------------------------------------------------------
extern int BGM_BLOCKLOAD_SIZE;
#endif // PM_DEBUG


//------------------------------------------------------------------
/**
 * @brief	データＩＮＤＥＸ有効範囲
 */
//------------------------------------------------------------------
#define PMSND_BGM_START		(BGM_START)
#define PMSND_BGM_END			(SEQ_BGM_END)
#define PMSND_SE_START		(SE_START)
#define PMSND_SE_END			(SEQ_SE_END)
#define PMSND_ME_START		(ME_START)
#define PMSND_ME_END			(SEQ_ME_END)

//------------------------------------------------------------------
/**
 * @brief	プレーヤー設定拡張定義
 */
//------------------------------------------------------------------
#define PMSND_PLAYER_MAX (SEPLAYER_SE3 + 1)

#define PMSND_MASKPL_BGM	(1<<PLAYER_BGM)
#define PMSND_MASKPL_SYS	(1<<PLAYER_SE_SYS)
#define PMSND_MASKPL_SE1	(1<<PLAYER_SE_1)
#define PMSND_MASKPL_SE2	(1<<PLAYER_SE_2)
#define PMSND_MASKPL_PSG	(1<<PLAYER_SE_PSG)
#define PMSND_MASKPL_SE3	(1<<PLAYER_SE_3)

#define PMSND_MASKPL_EFFSE	(PMSND_MASKPL_SE1|PMSND_MASKPL_SE2|PMSND_MASKPL_PSG|PMSND_MASKPL_SE3)
#define PMSND_MASKPL_ALLSE	(PMSND_MASKPL_SYS|PMSND_MASKPL_EFFSE)
#define PMSND_MASKPL_ALL	(PMSND_MASKPL_BGM|PMSND_MASKPL_ALLSE)

//------------------------------------------------------------------
/**
 * @brief	BGM フェードフレーム数
 */
//------------------------------------------------------------------
#define PMSND_FADE_FAST    (6)  // 短 
#define PMSND_FADE_SHORT  (30)  // やや短
#define PMSND_FADE_NORMAL (60)  // 中
#define PMSND_FADE_LONG   (90)  // 長

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
	SEPLAYER_SE3,

}SEPLAYER_ID;

#define SEPLAYER_MAX (5)

//------------------------------------------------------------------
/**
 * @brief	関数引数用定義
 */
//------------------------------------------------------------------
#define PMSND_NOEFFECT (-1)

//------------------------------------------------------------------
/**
 * @brief	再生可否判定コールバック設定
 */
//------------------------------------------------------------------
typedef BOOL (*PMSND_PLAYABLE_CALLBACK)( u32 soundIdx );

//再生可否判定コールバック関数の登録
extern void PMSND_SetPlayableCallBack( PMSND_PLAYABLE_CALLBACK func );
extern void PMSND_ResetPlayableCallBack( void );

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


//	ステレオ/モノラル設定
extern void	PMSND_SetStereo( BOOL flag );

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


//----------------------------------------------------------------------------
/**
 *	@brief  サウンド分割読み込みスレッド　CARDカードアクセス中チェック
 *
 *	サウンド分割読み込みスレッド内でのCARDアクセス中にメインスレッドに戻ってきた状態をチェックする関数
 *	注意点は２点
 *	１．処理タイミングによっては、VBlank期間中にカードアクセス完了の割り込みが発生し、
 *	　　本来VBlank期間中に行わなければならない処理がVBlank期間中に行われない可能性がある。
 *	
 *	２・サウンドデータがカードにアクセスしているため、その状態でメインスレッドに戻ってきた際には
 *	　　メインスレッドからカードにアクセスすることが出来ない。
 *
 *	１，２に当てはまる処理では、この関数を使用し、１フレーム処理をスキップする必要があります。
 *
 *	＊さらに、スレッドに関しての知識不足なため、上記内容に誤まったものがある可能性もあります。
 *	　2010.05.21の状態では本関数を使用することで、不具合の対処は出来ています。　tomoya takahashi
 *  
 *	@retval TRUE    CARDアクセス中
 *	@retval FALSE   通常動作
 */
//-----------------------------------------------------------------------------
extern BOOL           PMSND_IsAccessCARD( void );

#ifdef REVERB_USE
//	リバーブ設定
extern void PMSND_EnableCaptureReverb( u32 depth, u32 samplingRate, int volume, int stopFrames );
extern void PMSND_DisableCaptureReverb( void );
extern void PMSND_ChangeCaptureReverb( u32 depth, u32 samplingRate, int volume, int stopFrames );
#endif

//============================================================================================
/**
 *
 * @brief	プレーヤーの音量コントロール(ON/OFF)
 *					※現状32bit = 32プレーヤーまで対応
 *					in: bitmask : 最下位bitから0～のプレーヤー指定
 *												上記プレーヤー設定拡張定義を参照
 */
//============================================================================================
extern void PMSND_AllPlayerVolumeEnable( BOOL playerON, u32 bitmask );
extern void PMSND_AllPlayerVolumeSet( u8 volume, u32 bitmask );

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
extern void  PMSND_FadeInBGMEx( u16 frames, u8 vol );
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

// 拡張チャンネル再生
extern void	PMSND_PlayBGM_WideChannel( u32 soundIdx );
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

/***
 * ボリューム操作 
 *  SEのボリューム
 *    SEのボリュームは、シーケンスボリューム＋イニシャルボリューム　
 *    2つのボリュームで操作することが出来ます。
 *
 * シーケンスボリューム操作
 *    PMSND_PlayerSetVolume
 *    シーケンスのボリュームを設定します。
 * 
 * イニシャルボリューム操作
 *    PMSND_PlayerSetInitialVolume
 *    サウンド製作者が設定したボリュームをプログラム側で上書きすることが出来ます。
 * 
 * 
 * "イニシャルボリュームを操作すると、サウンド製作者の意図したボリュームバランスが変更されて
 * しまいます。イニシャルボリューム使用時には、サウンド製作者の確認を取るようにしてください。"
 *  
 ****/
//ＳＥプレーヤーボリュームセット
extern void PMSND_PlayerSetVolume( SEPLAYER_ID sePlayerID, u32 vol );
//SEイニシャルボリュームセット
extern void PMSND_PlayerSetInitialVolume( SEPLAYER_ID sePlayerID, u32 vol );
#if 0
//============================================================================================
/**
 *
 *
 * @brief	プリセット関数ラッパー
 *					システムでハンドルを１つだけ用意し簡易登録可能にする
 *
 *
 */
//============================================================================================
extern void	PMDSND_PresetSoundTbl( const u32* soundIdxTbl, u32 tblNum );
extern void	PMSND_PresetGroup( u32 groupIdx );
extern void	PMSND_ReleasePreset( void );
#endif

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
extern BOOL  PMDSND_ChangeWaveData
							( u32 waveArcDstID, u32 waveDstIdx, void* waveArcSrcAdrs, u32 waveSrcIdx);
extern BOOL PMDSND_PlayExtraMusic( u32 dummyNo );
extern void PMDSND_StopExtraMusic( void );
extern void PMDSND_ReleaseExtraMusic( void );

#endif
