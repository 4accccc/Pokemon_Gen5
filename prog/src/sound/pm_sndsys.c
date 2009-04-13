//============================================================================================
/**
 * @file	pm_sndsys.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "sound/sound_manager.h"

#include "sound/pm_sndsys.h"
//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	
 *
 *
 *
 *
 *
 */
//============================================================================================
//仮
BOOL debugBGMsetFlag;

//------------------------------------------------------------------
/**
 * @brief	プレーヤー設定定義
 */
//------------------------------------------------------------------
typedef struct {
	u32 size;
	u16 channelBit;
	u8	playableNum;
}PMSND_PLSETUP;

#define PMSND_PLSETUP_TBLEND (0xffffffff)

typedef struct {
	u32				soundIdx;
	NNSSndHandle	sndHandle;
	u16				playerNo;
}PMSND_PLAYER_DATA;

typedef struct {
	PMSND_PLAYER_DATA*	playerDataArray;
	u16					playerNum;
	int					heapLvDelete;
}PMSND_PL_UNIT;

//------------------------------------------------------------------
/**
 * @brief	プレーヤー情報定義
 */
//------------------------------------------------------------------
typedef struct {
	SNDPlayerInfo			playerInfo;
	SNDTrackInfo			trackInfo[16];
}PMSND_PLAYERSTATUS;

//------------------------------------------------------------------
/**
 * @brief	システムフェード構造定義
 */
//------------------------------------------------------------------
typedef struct {
	BOOL	active;
	u16		volumeCounter;
	u16		nextTrackBit;
	u32		nextSoundIdx;
	int		fadeFrames;
}PMSND_FADESTATUS;

//------------------------------------------------------------------
/**
 * @brief	リバーブ構造定義
 */
//------------------------------------------------------------------
typedef struct {
	BOOL	active;
	u32		samplingRate;
	u16		volume;
	u16		depth;
	int		stopFrames;
}PMSND_REVERB;

//------------------------------------------------------------------
/**
 * @brief	プレーヤー設定データ
 */
//------------------------------------------------------------------
#define PLAYER_SE_CH		(0xd800)

static const PMSND_PLSETUP systemPlayer[] = {
	{ 0x0000, PLAYER_SE_CH, 1 },	// システムＳＥ(プリセット)
	{ 0x6000, PLAYER_SE_CH, 1 },	// ＳＥ＆鳴き声(再生時にプレーヤーヒープへ読み込み）

	{ PMSND_PLSETUP_TBLEND, 0x0000, 0 },	// TABLE END
};

#define PLAYER_MUSIC_HEAPSIZ	(0x0000)
#define PLAYER_MUSIC_CH			(0xa7fe)
// ＢＧＭプレーヤー定義
static const SOUNDMAN_HIERARCHY_PLAYER_DATA pmHierarchyPlayerData = {
	PLAYER_MUSIC_HEAPSIZ, PLAYER_DEFAULT_MAX, PLAYER_MUSIC_CH,
};

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	サウンドシステム（main.c等から呼び出される）
 *
 *
 *
 *
 *
 */
//============================================================================================
#define	SOUND_HEAP_SIZE	(0x0b0000)
#define CAPTURE_BUFSIZE (0x2000)

static u8				PmSoundHeap[SOUND_HEAP_SIZE];
static u8				captureBuffer[ CAPTURE_BUFSIZE ] ATTRIBUTE_ALIGN(32);
static NNSSndArc		PmSoundArc;
static NNSSndHeapHandle	PmSndHeapHandle;
static u32				playerNumber;
static u32				bgmFadeCounter;
static PMSND_FADESTATUS	fadeStatus;
static PMSND_REVERB		reverbStatus;
static PMSND_PL_UNIT	systemPlayerUnit;
static PMSND_PLAYERSTATUS bgmPlayerInfo;
static SOUNDMAN_PRESET_HANDLE* systemPresetHandle;

static void	PMSND_WatchDog( void );
static void PMSND_CreatePlayerUnit( const PMSND_PLSETUP* setupTbl, PMSND_PL_UNIT* playerUnit );
static void PMSND_DeletePlayerUnit( PMSND_PL_UNIT* playerUnit );
static void PMSND_InitSystemFadeBGM( void );
static void PMSND_ResetSystemFadeBGM( void );
static void PMSND_CancelSystemFadeBGM( void );
static void PMSND_SystemFadeBGM( void );

static void PMSND_InitCaptureReverb( void );
//============================================================================================
/**
 *
 * @brief	セットアップ＆初期化
 *
 */
//============================================================================================
void	PMSND_Init( void )
{
	u32 size1, size2;

    // サウンドシステム初期化
 	NNS_SndInit();
	PmSndHeapHandle = NNS_SndHeapCreate(PmSoundHeap, SOUND_HEAP_SIZE);
	size1 = NNS_SndHeapGetFreeSize(PmSndHeapHandle);

    // サウンドの設定
	NNS_SndArcInitWithResult( &PmSoundArc, "wb_sound_data.sdat", PmSndHeapHandle, FALSE );

    // サウンド管理初期化
	SOUNDMAN_Init(&PmSndHeapHandle);
    // サウンド階層構造初期化
	SOUNDMAN_InitHierarchyPlayer(&pmHierarchyPlayerData);

    // サウンドの設定
	playerNumber = 0;
	PMSND_CreatePlayerUnit(systemPlayer, &systemPlayerUnit);

	PMSND_InitSystemFadeBGM();
	PMSND_InitCaptureReverb();

	bgmFadeCounter = 0;

	// 常駐サウンドデータ読み込み
	systemPresetHandle = SOUNDMAN_PresetGroup(GROUP_GLOBAL);
#if 0
	{
		u32 presetTbl[] = {1501, 1502};
		systemPresetHandle = SOUNDMAN_PresetSoundTbl(presetTbl,2);
	}
#endif
	size2 = NNS_SndHeapGetFreeSize(PmSndHeapHandle);

	OS_Printf("setup Sound... size(%x) heapRemains(%x)\n", size1 - size2, size2);

	debugBGMsetFlag = FALSE;
}

//============================================================================================
/**
 *
 * @brief	フレームワーク
 *
 */
//============================================================================================
void	PMSND_Main( void )
{
	PMSND_SystemFadeBGM();

	NNS_SndMain();

#if 0	// 現状、常に呼び出す必要もなさそうなので削除
	// サウンドドライバ情報更新
	NNS_SndUpdateDriverInfo();
	{
		NNSSndHandle* pBgmHandle = SOUNDMAN_GetHierarchyPlayerSndHandle();
		int i;

		NNS_SndPlayerReadDriverPlayerInfo(pBgmHandle, &bgmPlayerInfo.playerInfo);
		for( i=0; i<16; i++ ){
			NNS_SndPlayerReadDriverTrackInfo( pBgmHandle, i, &bgmPlayerInfo.trackInfo[i]);
		}
	}
#endif
	if( bgmFadeCounter ){ bgmFadeCounter--; }
}

//============================================================================================
/**
 *
 * @brief	終了処理＆破棄
 *
 */
//============================================================================================
void	PMSND_Exit( void )
{
	SOUNDMAN_ReleasePresetData(systemPresetHandle);

	PMSND_DeletePlayerUnit(&systemPlayerUnit);
	NNS_SndHeapDestroy(PmSndHeapHandle);
}

//============================================================================================
/**
 *
 * @brief	情報取得
 *
 */
//============================================================================================
u32 PMSND_GetSndHeapFreeSize( void )
{
	return NNS_SndHeapGetFreeSize(PmSndHeapHandle);
}

NNSSndHandle* PMSND_GetBGMhandlePointer( void )
{
	return SOUNDMAN_GetHierarchyPlayerSndHandle();
}

u32 PMSND_GetBGMplayerNoIdx( void )
{
	return	SOUNDMAN_GetHierarchyPlayerPlayerNoIdx();
}
 
BOOL PMSND_CheckOnReverb( void )
{
	return reverbStatus.active;
}

//============================================================================================
/**
 *
 * @brief	可変プレーヤーセットアップ
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	プレーヤーユニット作成
 */
//------------------------------------------------------------------
static void PMSND_CreatePlayerUnit
			( const PMSND_PLSETUP* setupTbl, PMSND_PL_UNIT* playerUnit )
{
	BOOL result;
	int playerNo;
	u32 playerSize;
	int i;

	// 現状再生されているＢＧＭは強制停止
	SOUNDMAN_StopHierarchyPlayer();

	// プレーヤーユニット削除の際に復帰するヒープ状態ＬＶを取得
	playerUnit->heapLvDelete = SOUNDMAN_GetHierarchyPlayerSoundHeapLv();

	// プレーヤー数カウント
	i = 0;
	while(setupTbl[i].size != PMSND_PLSETUP_TBLEND){ i++; }
	playerUnit->playerNum = i;

	// プレーヤー配列作成
	playerUnit->playerDataArray = (PMSND_PLAYER_DATA*)NNS_SndHeapAlloc
			(PmSndHeapHandle, sizeof(PMSND_PLAYER_DATA) * playerUnit->playerNum, NULL, 0, 0 );

	for( i=0; i<playerUnit->playerNum; i++ ){
		playerNo = i + playerNumber;
		playerSize = setupTbl[i].size;
		// チャンネル設定
		NNS_SndPlayerSetAllocatableChannel(playerNo, setupTbl[i].channelBit);
		NNS_SndPlayerSetPlayableSeqCount(playerNo, setupTbl[i].playableNum);
		// プレーヤーヒープ作成
		if(playerSize){
			int j;
			for( j=0; j<setupTbl[i].playableNum; j++ ){
				result = NNS_SndPlayerCreateHeap(playerNo, PmSndHeapHandle, playerSize);
				GF_ASSERT( result == TRUE );
			}
		}

		playerUnit->playerDataArray[i].soundIdx = 0;
		NNS_SndHandleInit(&playerUnit->playerDataArray[i].sndHandle);
		playerUnit->playerDataArray[i].playerNo = playerNo;
	}
	playerNumber += playerUnit->playerNum; 

	// 階層プレーヤーリセットのヒープＬＶを更新
	SOUNDMAN_UpdateHierarchyPlayerSoundHeapLv();
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーユニット削除
 */
//------------------------------------------------------------------
static void PMSND_DeletePlayerUnit( PMSND_PL_UNIT* playerUnit )
{
	int i;

	for( i=0; i<playerUnit->playerNum; i++ ){
		NNS_SndHandleReleaseSeq(&playerUnit->playerDataArray[i].sndHandle);
	}
	NNS_SndHeapLoadState(PmSndHeapHandle, playerUnit->heapLvDelete);
}

//============================================================================================
/**
 *
 * @brief	キャプチャー関数
 *
 */
//============================================================================================
static void PMSND_InitCaptureReverb( void )
{
	reverbStatus.active = FALSE;
	reverbStatus.samplingRate = 16000;
	reverbStatus.volume = 0;
	reverbStatus.depth = CAPTURE_BUFSIZE;
	reverbStatus.stopFrames = 0;
}

//------------------------------------------------------------------
/**
 * @brief	リバーブ設定
 */
//------------------------------------------------------------------
void PMSND_EnableCaptureReverb( u32 depth, u32 samplingRate, int volume, int stopFrames )
{
	BOOL result;

	if(reverbStatus.active == TRUE){ return; }

	reverbStatus.samplingRate = samplingRate;
	if(volume > 63){ volume = 63; }
	reverbStatus.volume = volume;
	if(depth > CAPTURE_BUFSIZE){ depth = CAPTURE_BUFSIZE; }
	reverbStatus.depth = depth;
	reverbStatus.stopFrames = stopFrames;

	result = NNS_SndCaptureStartReverb(	captureBuffer, 
										reverbStatus.depth,
										NNS_SND_CAPTURE_FORMAT_PCM16, 
										reverbStatus.samplingRate,
										reverbStatus.volume);
	if(result == TRUE){ reverbStatus.active = TRUE; }
}

void PMSND_DisableCaptureReverb( void )
{
	if(reverbStatus.active == FALSE){ return; }

	NNS_SndCaptureStopReverb(reverbStatus.stopFrames);
	reverbStatus.active = FALSE;
}

void PMSND_ChangeCaptureReverb( u32 depth, u32 samplingRate, int volume, int stopFrames )
{
	BOOL restartFlag = FALSE;

	if(depth != PMSND_NOEFFECT){
		reverbStatus.depth = depth;
		restartFlag = TRUE;
	}
	if(samplingRate != PMSND_NOEFFECT){
		reverbStatus.samplingRate = samplingRate;
		restartFlag = TRUE;
	}
	if(volume != PMSND_NOEFFECT){
		reverbStatus.volume = volume;
		restartFlag = TRUE;
	}
	if(stopFrames != PMSND_NOEFFECT){
		reverbStatus.stopFrames = stopFrames;
	}
	if(restartFlag == TRUE){
		NNS_SndCaptureStopReverb(0);
		reverbStatus.active = FALSE;
		PMSND_EnableCaptureReverb(	reverbStatus.depth, reverbStatus.samplingRate, 
									reverbStatus.volume, reverbStatus.stopFrames );
	}
}





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	ＢＧＭサウンド関数（各appから呼び出される）
 *
 *
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	サウンド再生
 */
//------------------------------------------------------------------
static BOOL PMSND_PlayBGM_CORE( u32 soundIdx, u16 trackBit )
{
	BOOL result;

	SOUNDMAN_StopHierarchyPlayer();
	result = SOUNDMAN_PlayHierarchyPlayer(soundIdx);
	if( trackBit != 0xffff ){ PMSND_ChangeBGMtrack( trackBit ); }

	return result;
}

//------------------------------------------------------------------
void	PMSND_PlayBGM_EX( u32 soundIdx, u16 trackBit )
{
	PMSND_ResetSystemFadeBGM();

	PMSND_PlayBGM_CORE( soundIdx, trackBit );
}

//------------------------------------------------------------------
void	PMSND_PlayNextBGM_EX( u32 soundIdx, u16 trackBit )
{
	fadeStatus.nextSoundIdx = soundIdx;
	fadeStatus.nextTrackBit = trackBit;
	fadeStatus.active = TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	サウンド停止
 */
//------------------------------------------------------------------
void	PMSND_StopBGM( void )
{
	PMSND_ResetSystemFadeBGM();
	SOUNDMAN_StopHierarchyPlayer();
}

//------------------------------------------------------------------
/**
 * @brief	サウンド一時停止
 */
//------------------------------------------------------------------
void	PMSND_PauseBGM( BOOL pauseFlag )
{
	PMSND_CancelSystemFadeBGM();
	SOUNDMAN_PauseHierarchyPlayer(pauseFlag);
}

//------------------------------------------------------------------
/**
 * @brief	サウンドフェードイン
 */
//------------------------------------------------------------------
void	PMSND_FadeInBGM( u16 frames )
{
	PMSND_CancelSystemFadeBGM();
	// 現在のvolumeを即時更新
	NNS_SndPlayerMoveVolume(SOUNDMAN_GetHierarchyPlayerSndHandle(), 0, 0);
	// 再度目標値を設定
	NNS_SndPlayerMoveVolume(SOUNDMAN_GetHierarchyPlayerSndHandle(), 127, frames);

	bgmFadeCounter = frames;
}

//------------------------------------------------------------------
/**
 * @brief	サウンドフェードアウト
 */
//------------------------------------------------------------------
void	PMSND_FadeOutBGM( u16 frames )
{
	PMSND_CancelSystemFadeBGM();
	NNS_SndPlayerMoveVolume(SOUNDMAN_GetHierarchyPlayerSndHandle(), 0, frames);

	bgmFadeCounter = frames;
}

//------------------------------------------------------------------
/**
 * @brief	サウンドフェードアウト検出
 */
//------------------------------------------------------------------
BOOL	PMSND_CheckFadeOnBGM( void )
{
	if( bgmFadeCounter ){
		return TRUE;
	} else {
		return FALSE;
	}
}

//------------------------------------------------------------------
/**
 * @brief	サウンド状態保管
 */
//------------------------------------------------------------------
void	PMSND_PushBGM( void )
{
	PMSND_CancelSystemFadeBGM();

	// 保管の際はトラックパラメータはリセットしておく。
	// ※そのままの状態で保管出来るのだが、各パラメータ値の取得が出来なく
	//	sound_manager側に管理させるとトラック全ての情報を持つことになってしまうので
	// 現状では設定管理はユーザー側で行うようにする。
	NNS_SndPlayerSetTrackMute(SOUNDMAN_GetHierarchyPlayerSndHandle(), 0xffff, FALSE );
	NNS_SndPlayerSetTrackModDepth(SOUNDMAN_GetHierarchyPlayerSndHandle(), 0xffff, 0 );
	NNS_SndPlayerSetTrackModSpeed(SOUNDMAN_GetHierarchyPlayerSndHandle(), 0xffff, 16 );

	SOUNDMAN_PushHierarchyPlayer();
}

//------------------------------------------------------------------
/**
 * @brief	サウンド状態取り出し
 */
//------------------------------------------------------------------
void	PMSND_PopBGM( void )
{
	PMSND_CancelSystemFadeBGM();
	SOUNDMAN_PopHierarchyPlayer();
}

//------------------------------------------------------------------
/**
 * @brief	ＢＧＭ再生トラック変更
 */
//------------------------------------------------------------------
void	PMSND_ChangeBGMtrack( u16 trackBit )
{
	u16		bitMask = trackBit^0xffff;

	NNS_SndPlayerSetTrackMute(SOUNDMAN_GetHierarchyPlayerSndHandle(), 0xffff, FALSE );
	if(bitMask){
		NNS_SndPlayerSetTrackMuteEx
			(SOUNDMAN_GetHierarchyPlayerSndHandle(), bitMask, NNS_SND_SEQ_MUTE_NO_STOP );
	}
}

//------------------------------------------------------------------
/**
 * @brief	ＢＧＭステータス変更
 */
//------------------------------------------------------------------
void	PMSND_SetStatusBGM( int tempoRatio, int pitch, int pan )
{
	NNSSndHandle* pBgmHandle = SOUNDMAN_GetHierarchyPlayerSndHandle();

	if(tempoRatio != PMSND_NOEFFECT){
		NNS_SndPlayerSetTempoRatio(pBgmHandle, tempoRatio);
	}
	if(pitch != PMSND_NOEFFECT){
		NNS_SndPlayerSetTrackPitch(pBgmHandle, 0xffff, pitch);
	}
	if(pan != PMSND_NOEFFECT){
		NNS_SndPlayerSetTrackPan(pBgmHandle, 0xffff, pan);
	}
}

//------------------------------------------------------------------
/**
 * @brief	ＢＧＭ終了検出
 */
//------------------------------------------------------------------
BOOL	PMSND_CheckPlayBGM( void )
{
	u16 playerNo = SOUNDMAN_GetHierarchyPlayerPlayerNoIdx() + PLAYER_DEFAULT_MAX;
	int count = NNS_SndPlayerCountPlayingSeqByPlayerNo(playerNo);

	if( count ){
		return TRUE;
	} else {
		return FALSE;
	}
}

//------------------------------------------------------------------
/**
 *
 * @brief	システムフェードＢＧＭ
 *
 */
//------------------------------------------------------------------
static void PMSND_InitSystemFadeBGM( void )
{
	fadeStatus.fadeFrames = 180;
	PMSND_ResetSystemFadeBGM();
}

static void PMSND_ResetSystemFadeBGM( void )
{
	fadeStatus.active = FALSE;
	fadeStatus.nextTrackBit = 0xffff;
	fadeStatus.nextSoundIdx = 0;
	fadeStatus.volumeCounter = fadeStatus.fadeFrames;
	//最大値に更新
	NNS_SndPlayerSetVolume(SOUNDMAN_GetHierarchyPlayerSndHandle(), 127);
}

static void PMSND_CancelSystemFadeBGM( void )
{
	if(fadeStatus.active == TRUE){
		u32	nowSoundIdx = SOUNDMAN_GetHierarchyPlayerSoundIdx();
		if( nowSoundIdx != fadeStatus.nextSoundIdx ){
			PMSND_PlayBGM_EX(fadeStatus.nextSoundIdx, fadeStatus.nextTrackBit);
		} else {
			PMSND_ResetSystemFadeBGM();
		}
	}
}

//------------------------------------------------------------------
static void PMSND_SystemFadeBGM( void )
{
	NNSSndHandle*	pBgmHandle = SOUNDMAN_GetHierarchyPlayerSndHandle();
	u32				nowSoundIdx = SOUNDMAN_GetHierarchyPlayerSoundIdx();
	BOOL			bgmSetFlag = FALSE;

	if( fadeStatus.active == FALSE ){ return; }

	if(nowSoundIdx){
		if( nowSoundIdx != fadeStatus.nextSoundIdx ){
			//FADEOUT
			if( fadeStatus.volumeCounter > 0 ){
				fadeStatus.volumeCounter--;
			} else {
				bgmSetFlag = TRUE;
			}
		} else {									
			//FADEIN
			if( fadeStatus.volumeCounter < fadeStatus.fadeFrames ){
				fadeStatus.volumeCounter++;
			} else {
				PMSND_ResetSystemFadeBGM();
				return;			//終了
			}
		}
	} else {
		bgmSetFlag = TRUE;	// 最初に鳴り出す場合
	}

	if(bgmSetFlag == TRUE){
		if(fadeStatus.nextSoundIdx){
			PMSND_PlayBGM_CORE( fadeStatus.nextSoundIdx, fadeStatus.nextTrackBit);
			NNS_SndPlayerSetVolume(pBgmHandle, 0);
			fadeStatus.volumeCounter = 0;
		} else {
			PMSND_StopBGM();
		}
	} else {
		int volume = fadeStatus.volumeCounter * 127 / fadeStatus.fadeFrames;

		NNS_SndPlayerSetVolume(pBgmHandle, volume);
	}
}

//------------------------------------------------------------------
/**
 * @brief	システムフェードフレーム設定
 */
//------------------------------------------------------------------
void PMSND_SetSystemFadeFrames( int frames )
{
	fadeStatus.fadeFrames = frames;
}





//============================================================================================
/**
 *
 *
 *
 * @brief	ＳＥサウンド関数（各appから呼び出される）
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	システムＳＥサウンド関数
 */
//------------------------------------------------------------------
void	PMSND_PlaySystemSE( u32 soundIdx )
{
	PMSND_PLAYER_DATA* ppd = &systemPlayerUnit.playerDataArray[PLAYER_SYSSE];

	NNS_SndPlayerStopSeq(&ppd->sndHandle, 0);
	ppd->soundIdx = soundIdx;

	NNS_SndArcPlayerStartSeqEx(&ppd->sndHandle, ppd->playerNo, -1, 126, soundIdx);
}

//------------------------------------------------------------------
/**
 * @brief	ＳＥサウンド関数
 */
//------------------------------------------------------------------
void	PMSND_PlaySE( u32 soundIdx )
{
	PMSND_PLAYER_DATA* ppd = &systemPlayerUnit.playerDataArray[PLAYER_SEVOICE];

	NNS_SndPlayerStopSeq(&ppd->sndHandle, 0);
	// プレイヤー使用チャンネル設定
	NNS_SndPlayerSetAllocatableChannel(ppd->playerNo, PLAYER_SE_CH);
	ppd->soundIdx = soundIdx;

	NNS_SndArcPlayerStartSeqEx(&ppd->sndHandle, ppd->playerNo, -1, 126, soundIdx);
}

//------------------------------------------------------------------
/**
 * @brief	ＳＥ終了検出
 */
//------------------------------------------------------------------
BOOL	PMSND_CheckPlaySE( void )
{
	PMSND_PLAYER_DATA* ppd = &systemPlayerUnit.playerDataArray[PLAYER_SEVOICE];
	int count = NNS_SndPlayerCountPlayingSeqByPlayerNo(ppd->playerNo);

	if( count ){
		return TRUE;
	} else {
		return FALSE;
	}
}

//------------------------------------------------------------------
/**
 * @brief	ＳＥステータス変更
 */
//------------------------------------------------------------------
void	PMSND_SetStatusSE( int tempoRatio, int pitch, int pan )
{
	PMSND_PLAYER_DATA* ppd = &systemPlayerUnit.playerDataArray[PLAYER_SEVOICE];

	if(tempoRatio != PMSND_NOEFFECT){
		NNS_SndPlayerSetTempoRatio(&ppd->sndHandle, tempoRatio);
	}
	if(pitch != PMSND_NOEFFECT){
		NNS_SndPlayerSetTrackPitch(&ppd->sndHandle, 0xffff, pitch);
	}
	if(pan != PMSND_NOEFFECT){
		NNS_SndPlayerSetTrackPan(&ppd->sndHandle, 0xffff, pan);
	}
}



#if 0
//============================================================================================
/**
 *
 *
 * @brief	システムプレーヤーを使用した鳴き声サウンド関数（各appから呼び出される）
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	鳴き声再生関数
 */
//------------------------------------------------------------------
void	PMSND_PlayVoiceByPlayer( u32 pokeNum )
{
	PMSND_PLAYER_DATA* ppd = &systemPlayerUnit.playerDataArray[PLAYER_SEVOICE];

	NNS_SndPlayerStopSeq(&ppd->sndHandle, 0);
	// プレイヤー使用チャンネル設定
	NNS_SndPlayerSetAllocatableChannel(ppd->playerNo, PLAYER_PMVOICE_CH);
	ppd->soundIdx = pokeNum;

	NNS_SndArcPlayerStartSeqEx(&ppd->sndHandle, ppd->playerNo, pokeNum, 127, SEQ_PV);
}

//------------------------------------------------------------------
/**
 * @brief	鳴き声終了検出
 */
//------------------------------------------------------------------
BOOL	PMSND_CheckPlayByPlayer( void )
{
	return PMSND_CheckPlaySE();	//現状はプレーヤーが同じに設定してあるので同じ動作
}
#endif






