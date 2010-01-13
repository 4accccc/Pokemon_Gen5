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
 * @brief	ＳＥプレーヤー設定定義
 */
//------------------------------------------------------------------
typedef struct {
	u32						soundIdx;
	NNSSndHandle	sndHandle;
}PMSND_SEPLAYER_DATA;
//------------------------------------------------------------------
/**
 * @brief	システムフェード構造定義
 */
//------------------------------------------------------------------
typedef struct {
	BOOL	active;
	int		seq;
	u16		volumeCounter;
	u16		nextTrackBit;
	u32		nextSoundIdx;
	int		fadeInFrame;
	int		fadeOutFrame;
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
 * @brief	スレッド制御定義
 */
//------------------------------------------------------------------
#define BGM_BLOCKLOAD_SIZE  (0x2000)

typedef enum {
	THREADLOAD_SEQBANK = 0,
	THREADLOAD_WAVE,
}THREADLOAD_MODE;

// スレッド使用引数構造体
typedef struct {
	u32	soundIdx;
}THREAD_ARG;

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
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE //通常時処理
#define	SOUND_HEAP_SIZE	(0x0a0000 - 30000 - 12288)   //640K - 30000 - 0x3000
#else                   //DL子機時処理
#define	SOUND_HEAP_SIZE	(0x030000)   //非常に適当
#endif //MULTI_BOOT_MAKE
#define CAPTURE_BUFSIZE (0x2000)
#define TRACK_NUM	(16)

static u8												PmSoundHeap[SOUND_HEAP_SIZE];
static u8												captureBuffer[ CAPTURE_BUFSIZE ] ATTRIBUTE_ALIGN(32);
static NNSSndArc								PmSoundArc;
static NNSSndHeapHandle					PmSndHeapHandle;
static u32											bgmFadeCounter;
static PMSND_FADESTATUS					fadeStatus;
static PMSND_REVERB							reverbStatus;
static SOUNDMAN_PRESET_HANDLE*	systemPresetHandle;
static SOUNDMAN_PRESET_HANDLE*	usrPresetHandle1;
static PMSND_SEPLAYER_DATA			sePlayerData[SEPLAYER_MAX];
static SNDTrackInfo							trackInfo[TRACK_NUM];
static u16											trackActiveBit;	
//static OSMutex									sndTreadMutex;		
//
#ifdef PM_DEBUG
static u32 heapRemainsAfterSys;
static u32 heapRemainsAfterPlayer;
static u32 heapRemainsAfterPresetSE;
#endif

#define THREAD_STACKSIZ		(0x1000)
OSThread		soundLoadThread;
static u64	threadStack[ THREAD_STACKSIZ / sizeof(u64) ];
THREAD_ARG	threadArg;

static void PMSND_InitCore( void );

static void	PMSND_InitSEplayer( void );
static void PMSND_InitSystemFadeBGM( void );
static void PMSND_SetSystemFadeBGM( u32 nextSoundIdx, u16 trackBit, u8 outFrame, u8 inFrame );
static void PMSND_ResetSystemFadeBGM( void );
static void PMSND_CancelSystemFadeBGM( void );
static void PMSND_SystemFadeBGM( void );

static void PMSND_InitCaptureReverb( void );

static void createSoundPlayThread( u32 soundIdx, THREADLOAD_MODE mode );
static void deleteSoundPlayThread( void );
static BOOL checkEndSoundPlayThread( void );
//============================================================================================
/**
 *
 * @brief	セットアップ＆初期化
 *
 */
//============================================================================================
void	PMSND_Init( void )
{
	// サウンドシステム初期化
	NNS_SndInit();
	PmSndHeapHandle = NNS_SndHeapCreate(PmSoundHeap, SOUND_HEAP_SIZE);

	// サウンドの設定
	NNS_SndArcInitWithResult( &PmSoundArc, "wb_sound_data.sdat", PmSndHeapHandle, FALSE );

  PMSND_InitCore();
  
	// 常駐サウンドデータ読み込み
	systemPresetHandle = SOUNDMAN_PresetGroup(GROUP_GLOBAL);
	usrPresetHandle1 = NULL;
#ifdef PM_DEBUG
	heapRemainsAfterPresetSE = NNS_SndHeapGetFreeSize(PmSndHeapHandle);
#endif
	debugBGMsetFlag = FALSE;
  
}
void	PMSND_InitMultiBoot( void* sndData )
{
	// サウンドシステム初期化(マルチブート子機
	// 初期化データは32バイトアライメント
	NNS_SndInit();
	PmSndHeapHandle = NNS_SndHeapCreate(PmSoundHeap, SOUND_HEAP_SIZE);

	// サウンドの設定
	NNS_SndArcInitOnMemory( &PmSoundArc, sndData );

  PMSND_InitCore();

	debugBGMsetFlag = FALSE;
}

//---------------------------------------------------
//  初期化コア部分
//---------------------------------------------------
static void PMSND_InitCore( void )
{
#ifdef PM_DEBUG
	heapRemainsAfterSys = NNS_SndHeapGetFreeSize(PmSndHeapHandle);
#endif
	// サウンドプレーヤーの設定（サウンドデータに定義されているものを参照する）
	NNS_SndArcPlayerSetup(PmSndHeapHandle);
#ifdef PM_DEBUG
	heapRemainsAfterPlayer = NNS_SndHeapGetFreeSize(PmSndHeapHandle);
#endif
	// サウンド管理初期化
	SOUNDMAN_Init(&PmSndHeapHandle);

	// 階層プレーヤーリセットのヒープＬＶを更新
	SOUNDMAN_UpdateHierarchyPlayerSoundHeapLv();

	// ＳＥプレーヤーデータ初期化
	PMSND_InitSEplayer();
	// サウンド階層構造初期化
	SOUNDMAN_InitHierarchyPlayer(PLAYER_BGM);

	PMSND_InitSystemFadeBGM();
	PMSND_InitCaptureReverb();

	// 排他制御用Mutex初期化(NNS_SndMainがスレッドセーフ設計ではないため)
	//OS_InitMutex(&sndTreadMutex);		

	trackActiveBit = 0;	
	bgmFadeCounter = 0;
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
	// サウンドドライバ情報更新
	NNS_SndUpdateDriverInfo();
	// 現在のトラック状態を取得
	{
		NNSSndHandle* sndHandle = PMSND_GetBGMhandlePointer();
		int i;

		trackActiveBit = 0;
		for( i=0; i<TRACK_NUM; i++ ){
			if(NNS_SndPlayerReadDriverTrackInfo(sndHandle, i, &trackInfo[i]) == TRUE){
				trackActiveBit |= (0x0001 << i);	// 有効/無効をBitFieldに変換
			}
		}
	}
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
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	SOUNDMAN_ReleasePresetData(systemPresetHandle);
#endif //MULTI_BOOT_MAKE

	NNS_SndHeapDestroy(PmSndHeapHandle);
}

//============================================================================================
/**
 *
 * @brief	情報取得
 *
 */
//============================================================================================
#ifdef PM_DEBUG
u32 PMSND_GetSndHeapSize( void )									{ return SOUND_HEAP_SIZE; }
u32 PMSND_GetSndHeapRemainsAfterSys( void )				{ return heapRemainsAfterSys; }
u32 PMSND_GetSndHeapRemainsAfterPlayer( void )		{ return heapRemainsAfterPlayer; }
u32 PMSND_GetSndHeapRemainsAfterPresetSE( void )	{ return heapRemainsAfterPresetSE; }
u32 PMSND_GetSEPlayerNum( void )									{ return SEPLAYER_MAX; }
#endif

u32 PMSND_GetSndHeapFreeSize( void )
{
	return NNS_SndHeapGetFreeSize(PmSndHeapHandle);
}

NNSSndHandle* PMSND_GetNowSndHandlePointer( void )
{
	int i;
	for( i=0; i<SEPLAYER_MAX; i++ ){
		if( PMSND_CheckPlaySE_byPlayerID((SEPLAYER_ID)i) == TRUE ){
			return &sePlayerData[i].sndHandle;
		}
	}
	return SOUNDMAN_GetHierarchyPlayerSndHandle();
}

NNSSndHandle* PMSND_GetBGMhandlePointer( void )
{
	return SOUNDMAN_GetHierarchyPlayerSndHandle();
}

u32 PMSND_GetBGMsoundNo( void )
{
	return SOUNDMAN_GetHierarchyPlayerSoundIdx();
}

u32 PMSND_GetNextBGMsoundNo( void )
{
	if( fadeStatus.active == FALSE ){
		return SOUNDMAN_GetHierarchyPlayerSoundIdx();
	}
	return fadeStatus.nextSoundIdx;
}

u32 PMSND_GetBGMplayerNoIdx( void )
{
	if( PMSND_CheckPlaySE() == TRUE ){
		return 0;
	}
	return	SOUNDMAN_GetHierarchyPlayerPlayerNoIdx() + 1;
}
 
BOOL PMSND_CheckOnReverb( void )
{
	return reverbStatus.active;
}

u8 PMSND_GetBGMTrackVolume( int trackNo )
{
	if(trackNo >= TRACK_NUM){ return 0; }
	if(trackActiveBit	& (0x0001 << trackNo)){
		return trackInfo[TRACK_NUM].volume;
	}
	return 0;
}

BOOL PMSND_IsLoading( void )
{
	return OS_IsThreadTerminated(&soundLoadThread) == FALSE;
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
 * @brief	サウンド停止
 */
//------------------------------------------------------------------
static void	PMSND_StopBGM_CORE( void )
{
	NNS_SndPlayerStopSeq(SOUNDMAN_GetHierarchyPlayerSndHandle(), 0);
	SOUNDMAN_UnloadHierarchyPlayer();
}
//------------------------------------------------------------------
void	PMSND_StopBGM( void )
{
	PMSND_ResetSystemFadeBGM();
	PMSND_StopBGM_CORE();
}

//------------------------------------------------------------------
/**
 * @brief	サウンド再生
 */
//------------------------------------------------------------------
static BOOL PMSND_PlayBGM_CORE( u32 soundIdx, u16 trackBit )
{
	BOOL result;

  // ロードスレッド動作中は再生しない
  if( PMSND_IsLoading() ) { return FALSE; }

	PMSND_StopBGM_CORE();
	result = SOUNDMAN_LoadHierarchyPlayer(soundIdx);
	if( result == FALSE){ return FALSE; }

	// サウンド再生開始
	result = NNS_SndArcPlayerStartSeqEx
					(SOUNDMAN_GetHierarchyPlayerSndHandle(), PLAYER_BGM, -1, -1, soundIdx);
	if( result == FALSE){ return FALSE; }

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
void	PMSND_PlayNextBGM_EX( u32 soundIdx, u16 trackBit, u8 fadeOutFrame, u8 fadeInFrame )
{
	PMSND_SetSystemFadeBGM(soundIdx, trackBit, fadeOutFrame, fadeInFrame );
}

//------------------------------------------------------------------
/**
 * @brief	サウンド一時停止
 */
//------------------------------------------------------------------
void	PMSND_PauseBGM( BOOL pauseFlag )
{
	PMSND_CancelSystemFadeBGM();
	NNS_SndPlayerPause(SOUNDMAN_GetHierarchyPlayerSndHandle(), pauseFlag);
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
	if( SOUNDMAN_GetHierarchyPlayerPlayerNoIdx() == 0 ){ return; }

	PMSND_StopBGM_CORE();
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
void	PMSND_SetStatusBGM_EX( u16 trackBit, int tempoRatio, int pitch, int pan )
{
	NNSSndHandle* pBgmHandle = SOUNDMAN_GetHierarchyPlayerSndHandle();

	if(tempoRatio != PMSND_NOEFFECT){
		NNS_SndPlayerSetTempoRatio(pBgmHandle, tempoRatio);
	}
	if(pitch != PMSND_NOEFFECT){
		NNS_SndPlayerSetTrackPitch(pBgmHandle, trackBit, pitch);
	}
	if(pan != PMSND_NOEFFECT){
		NNS_SndPlayerSetTrackPan(pBgmHandle, trackBit, pan);
	}
}

//------------------------------------------------------------------
/**
 * @brief   ＢＧＭボリューム変更
 */ 
//------------------------------------------------------------------
void PMSND_ChangeBGMVolume( u16 trackBit, int volume )
{
	NNSSndHandle* pBgmHandle = SOUNDMAN_GetHierarchyPlayerSndHandle();
	NNS_SndPlayerSetTrackVolume( pBgmHandle, trackBit, volume );
}

//------------------------------------------------------------------
/**
 * @brief	ＢＧＭ終了検出
 */
//------------------------------------------------------------------
BOOL	PMSND_CheckPlayBGM( void )
{
	int count = NNS_SndPlayerCountPlayingSeqBySeqNo(SOUNDMAN_GetHierarchyPlayerSoundIdx());

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
	fadeStatus.fadeInFrame = 60;
	fadeStatus.fadeOutFrame = 60;
	PMSND_ResetSystemFadeBGM();
}

static void PMSND_SetSystemFadeBGM( u32 nextSoundIdx, u16 trackBit, u8 outFrame, u8 inFrame )
{
	fadeStatus.nextTrackBit = trackBit;
	fadeStatus.nextSoundIdx = nextSoundIdx;
	fadeStatus.fadeInFrame = inFrame;
	fadeStatus.fadeOutFrame = outFrame;
	fadeStatus.volumeCounter = outFrame;

	deleteSoundPlayThread();

	fadeStatus.seq = 0;
	fadeStatus.active = TRUE;
}

static void PMSND_ResetSystemFadeBGM( void )
{
	fadeStatus.active = FALSE;
	fadeStatus.nextTrackBit = 0xffff;
	fadeStatus.nextSoundIdx = 0;
	fadeStatus.volumeCounter = 0;//fadeStatus.fadeFrames;
	//最大値に更新
	NNS_SndPlayerSetVolume(SOUNDMAN_GetHierarchyPlayerSndHandle(), 127);

	fadeStatus.seq = 0;
	deleteSoundPlayThread();
}

static void PMSND_CancelSystemFadeBGM( void )
{
	if(fadeStatus.active == TRUE){
		u32	nowSoundIdx = SOUNDMAN_GetHierarchyPlayerSoundIdx();
		if( nowSoundIdx != fadeStatus.nextSoundIdx ){
			while(fadeStatus.seq != 0){
				PMSND_SystemFadeBGM();
				OS_Printf("サウンド読み込みが完了していないのに一時停止や階層操作をしようとしている\n");
			}
			PMSND_PlayBGM_EX(fadeStatus.nextSoundIdx, fadeStatus.nextTrackBit);
		}
		PMSND_ResetSystemFadeBGM();
	}
}

//------------------------------------------------------------------
//int debugCounter;

static void PMSND_SystemFadeBGM( void )
{
	NNSSndHandle*	pBgmHandle = SOUNDMAN_GetHierarchyPlayerSndHandle();
	u32				nowSoundIdx = SOUNDMAN_GetHierarchyPlayerSoundIdx();
	BOOL			bgmSetFlag = FALSE;

	if( fadeStatus.active == FALSE ){ return; }

	switch( fadeStatus.seq ){
	case 0:
		if(nowSoundIdx){
			if( nowSoundIdx != fadeStatus.nextSoundIdx ){
				//FADEOUT
				if( fadeStatus.volumeCounter > 0 ){
					fadeStatus.volumeCounter--;
					NNS_SndPlayerSetVolume(pBgmHandle, fadeStatus.volumeCounter*127/fadeStatus.fadeOutFrame);
					return;
				}
				NNS_SndPlayerSetVolume(pBgmHandle, 0);
			} else {									
				//FADEIN
				if( fadeStatus.volumeCounter < fadeStatus.fadeInFrame ){
					fadeStatus.volumeCounter++;
					NNS_SndPlayerSetVolume(pBgmHandle, fadeStatus.volumeCounter*127/fadeStatus.fadeInFrame);
					return;
				}
				PMSND_ResetSystemFadeBGM();
				return;			//終了
			}
		}
		PMSND_StopBGM_CORE();

		if(fadeStatus.nextSoundIdx == 0){
			PMSND_ResetSystemFadeBGM();
			return;			//終了
		}
		NNS_SndArcSetLoadBlockSize(BGM_BLOCKLOAD_SIZE);	//分割ロード指定

		createSoundPlayThread(fadeStatus.nextSoundIdx, THREADLOAD_SEQBANK);
		fadeStatus.volumeCounter = 0;
		fadeStatus.seq = 1;
		break;
	case 1:
		if(checkEndSoundPlayThread() == TRUE){
			SOUNDMAN_LoadHierarchyPlayer_forThread_heapsvSB();// サウンド階層構造用設定

			createSoundPlayThread( fadeStatus.nextSoundIdx, THREADLOAD_WAVE);
			fadeStatus.seq = 2;
		} else {
			OS_Sleep(1);
		}
		break;
	case 2:
		if(checkEndSoundPlayThread() == TRUE){
			NNS_SndArcSetLoadBlockSize(0);	//分割ロードなしに復帰

			SOUNDMAN_LoadHierarchyPlayer_forThread_end(fadeStatus.nextSoundIdx);

			// サウンド再生開始
			NNS_SndArcPlayerStartSeqEx
					(SOUNDMAN_GetHierarchyPlayerSndHandle(), PLAYER_BGM, -1, -1, fadeStatus.nextSoundIdx);
			NNS_SndPlayerSetVolume(pBgmHandle, 0);
			fadeStatus.seq = 0;
		} else {
			OS_Sleep(1);
		}
		break;
	}
}

//------------------------------------------------------------------
/**
 * @brief	システムフェードフレーム設定
 */
//------------------------------------------------------------------
void PMSND_SetSystemFadeFrames( int fadeOutFrame, int fadeInFrame )
{
	fadeStatus.fadeInFrame = fadeInFrame;
	fadeStatus.fadeOutFrame = fadeOutFrame;
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
 * @brief	ＳＥプレーヤーデータ初期化
 */
//------------------------------------------------------------------
static void	PMSND_InitSEplayer( void )
{
	int i;

	for( i=0; i<SEPLAYER_MAX; i++ ){
		sePlayerData[i].soundIdx = 0;
		NNS_SndHandleInit(&sePlayerData[i].sndHandle);
	}
}

//------------------------------------------------------------------
/**
 * @brief	ＳＥプレーヤーハンドル取得
 */
//------------------------------------------------------------------
NNSSndHandle* PMSND_GetSE_SndHandle( SEPLAYER_ID sePlayerID )
{
	return &sePlayerData[sePlayerID].sndHandle;
}

//------------------------------------------------------------------
/**
 * @brief	ＳＥプレーヤー初期設定ＩＤ取得
 */
//------------------------------------------------------------------
SEPLAYER_ID	PMSND_GetSE_DefaultPlayerID( u32 soundIdx )
{
	const NNSSndSeqParam* seqParam;
  if (soundIdx < PMSND_SE_START || soundIdx >= PMSND_SE_END) {
    GF_ASSERT_MSG(0, "ID(%d) is not SE ID!!", soundIdx);
    return SEPLAYER_SE1;
  }
  seqParam = NNS_SndArcGetSeqParam( soundIdx );

	return (SEPLAYER_ID)(seqParam->playerNo - PLAYER_SE_SYS);
}

//--------------------------------------------------------------
/**
 * @brief	シーケンスの初期ボリューム設定
 *
 * @param	p		サウンドハンドルのアドレス
 * @param	vol		ボリューム(0-127)
 *
 * @retval	none
 *
 * サウンドハンドルが無効の場合は、何もしません。 
 *
 * ボリュームのデフォルト値は、最大の127です。
 * この値の影響はシーケンス全体にかかります。
 *
 * 内部で呼び出されるNNS_SndPlayerSetInitialVolume関数は、
 * NNS_SndArcPlayerStartSeq*関数と NNS_SndArcPlayerStartSeqArc*関数内で
 * 呼びだされています。再度、この関数を呼びだすと、設定した値が上書きされます。
 * 上書きしたくない場合は、 NNS_SndPlayerSetVolume関数などを使ってください。
 *
 * 例
 * PMSND_PlaySE( no );
 * PMSND_PlayerSetInitialVolume( handle. 30 );
 * ボリューム30で再生される
 *
 * そのあと、
 * PMSND_PMVoicePlay( no );
 * デフォルトの値127で再生される(元に戻っている)
 *
 * 逆にいうと、常にボリューム30にしたかったら、
 * PMSND_PlayerSetInitialVolume( handle. 30 );
 * を毎回セットする
 */
//--------------------------------------------------------------
void PMSND_PlayerSetInitialVolume( SEPLAYER_ID sePlayerID, u32 vol )
{
	//エラー回避
	if( vol > 127 ){
		vol = 127;
	}

	//この関数は、NNS_SndArcPlayerStartSeq*関数と NNS_SndArcPlayerStartSeqArc*関数内で
	//呼びだされています。再度、この関数を呼びだすと、設定した値が上書きされます。
	//上書きしたくない場合は、 NNS_SndPlayerSetVolume関数などを使ってください。
	NNS_SndPlayerSetInitialVolume( &sePlayerData[sePlayerID].sndHandle, vol );
	return;
}

//------------------------------------------------------------------
/**
 * @brief	ＳＥサウンド再生関数
 */
//------------------------------------------------------------------
void	PMSND_PlaySE_byPlayerID( u32 soundIdx, SEPLAYER_ID sePlayerID )
{
	int playerNo = sePlayerID + PLAYER_SE_SYS;
	BOOL result;

	sePlayerData[sePlayerID].soundIdx = 0;
	result = NNS_SndArcPlayerStartSeqEx
		(&sePlayerData[sePlayerID].sndHandle, playerNo, -1, -1, soundIdx);
	if(result == TRUE){ sePlayerData[sePlayerID].soundIdx = soundIdx; }
}

static void pmsnd_PlaySECore( u32 soundIdx, u32 volume )
{
	BOOL result;
	SEPLAYER_ID	sePlayerID;

  // ロードスレッド動作中は再生しない
  if( PMSND_IsLoading() ) { return; }

	sePlayerID = PMSND_GetSE_DefaultPlayerID(soundIdx);
	sePlayerData[sePlayerID].soundIdx = 0;
	result = NNS_SndArcPlayerStartSeq(&sePlayerData[sePlayerID].sndHandle, soundIdx);
	if(result == TRUE){
    sePlayerData[sePlayerID].soundIdx = soundIdx;
    if(volume < 128){
      PMSND_PlayerSetInitialVolume( sePlayerID, volume );
    }
  }
}

void	PMSND_PlaySE( u32 soundIdx )
{
  pmsnd_PlaySECore( soundIdx, 0xFFFFFFFF );
}
/*
 *  @brief  SEをボリューム指定付きで再生
 *
 *  @param  soundIdx  再生したいSENo
 *  @param  volume    指定ボリューム(有効値 0-127 デフォルトは127)
 */
void	PMSND_PlaySEVolume( u32 soundIdx, u32 volume )
{
  pmsnd_PlaySECore( soundIdx, volume );
}

//------------------------------------------------------------------
/**
 * @brief	ＳＥサウンド停止関数
 */
//------------------------------------------------------------------
void	PMSND_StopSE_byPlayerID( SEPLAYER_ID sePlayerID )
{
	NNS_SndPlayerStopSeq(&sePlayerData[sePlayerID].sndHandle, 0);
}

void	PMSND_StopSE( void )
{
	int i;

	for( i=0; i<SEPLAYER_MAX; i++ ){ PMSND_StopSE_byPlayerID((SEPLAYER_ID)i); }
}

//------------------------------------------------------------------
/**
 * @brief	ＳＥ終了検出
 */
//------------------------------------------------------------------
BOOL	PMSND_CheckPlaySE_byPlayerID( SEPLAYER_ID sePlayerID )
{
	int playerNo = sePlayerID + PLAYER_SE_SYS;
	int count = NNS_SndPlayerCountPlayingSeqByPlayerNo(playerNo);

	if( count )	{ return TRUE; }
	return FALSE;
}

BOOL	PMSND_CheckPlaySE( void )
{
	int i;

	for( i=0; i<SEPLAYER_MAX; i++ ){
		if( PMSND_CheckPlaySE_byPlayerID((SEPLAYER_ID)i) == TRUE ){ return TRUE; }
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	ＳＥステータス変更
 */
//------------------------------------------------------------------
void	PMSND_SetStatusSE_byPlayerID( SEPLAYER_ID sePlayerID, int tempoRatio, int pitch, int pan )
{
	if(tempoRatio != PMSND_NOEFFECT){
		NNS_SndPlayerSetTempoRatio(&sePlayerData[sePlayerID].sndHandle, tempoRatio);
	}
	if(pitch != PMSND_NOEFFECT){
		NNS_SndPlayerSetTrackPitch(&sePlayerData[sePlayerID].sndHandle, 0xffff, pitch);
	}
	if(pan != PMSND_NOEFFECT){
		NNS_SndPlayerSetTrackPan(&sePlayerData[sePlayerID].sndHandle, 0xffff, pan);
	}
}

void	PMSND_SetStatusSE( int tempoRatio, int pitch, int pan )
{
	int i;

	for( i=0; i<SEPLAYER_MAX; i++ ){
		PMSND_SetStatusSE_byPlayerID((SEPLAYER_ID)i, tempoRatio, pitch, pan );
	}
}


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
void	PMDSND_PresetSoundTbl( const u32* soundIdxTbl, u32 tblNum )
{
	PMSND_ReleasePreset();
	usrPresetHandle1 = SOUNDMAN_PresetSoundTbl(soundIdxTbl, tblNum);
}

void	PMSND_PresetGroup( u32 groupIdx )
{
	PMSND_ReleasePreset();
	usrPresetHandle1 = SOUNDMAN_PresetGroup(groupIdx);
}

void PMSND_ReleasePreset( void )
{
	PMSND_StopBGM_CORE();

	if( usrPresetHandle1 != NULL ){
		SOUNDMAN_ReleasePresetData(usrPresetHandle1);
		usrPresetHandle1 = NULL;
	}
}


//============================================================================================
/**
 *
 *
 * @brief	サウンドファイル分割ロードスレッド
 *
 *
 */
//============================================================================================
//スレッド関数
/// BGMの分割読み込みのサイズ  
//--------------------------------------------------------------------------------------------
static void	_loadSeqBankThread( void* arg )
{
	THREAD_ARG* arg1 = (THREAD_ARG*)arg;
	BOOL result;

	// サウンドデータ（seq, bank）読み込み
	result = NNS_SndArcLoadSeqEx
		(arg1->soundIdx, NNS_SND_ARC_LOAD_SEQ | NNS_SND_ARC_LOAD_BANK, PmSndHeapHandle);
	if( result == FALSE){ OS_Printf("sound seqbank load error!\n"); }
}

static void	_loadWaveThread( void* arg )
{
	THREAD_ARG* arg1 = (THREAD_ARG*)arg;
	BOOL result;

	// サウンドデータ（wave）読み込み
	result = NNS_SndArcLoadSeqEx(arg1->soundIdx, NNS_SND_ARC_LOAD_WAVE, PmSndHeapHandle);
	if( result == FALSE){ OS_Printf("sound wave load error!\n"); }
}

static void createSoundPlayThread( u32 soundIdx, THREADLOAD_MODE mode )
{
	threadArg.soundIdx = soundIdx;

	deleteSoundPlayThread();
	switch( mode ){
	case THREADLOAD_SEQBANK:
		OS_CreateThread(&soundLoadThread, 
										_loadSeqBankThread,
										&threadArg,
										threadStack + (THREAD_STACKSIZ/sizeof(u64)),
										THREAD_STACKSIZ,
										17);
		break;
	case THREADLOAD_WAVE:
		OS_CreateThread(&soundLoadThread, 
										_loadWaveThread,
										&threadArg,
										threadStack + (THREAD_STACKSIZ/sizeof(u64)),
										THREAD_STACKSIZ,
										17);
		break;
	}
	OS_WakeupThreadDirect(&soundLoadThread);
}

static void deleteSoundPlayThread( void )
{
	if( OS_IsThreadTerminated(&soundLoadThread) == FALSE ){
		OS_DestroyThread(&soundLoadThread);
	}
}

static BOOL checkEndSoundPlayThread( void )
{
	return OS_IsThreadTerminated(&soundLoadThread);
}


//-------------------------------------------------------------------------------------------- 
/**
 * @brief 分割ロード・再生
 *
 * @param no    再生するBGMナンバー
 * @param seq   シーケンス管理ポインタ
 * @param start TRUE で分割ロード開始, FALSE でシーケンスを進める
 *
 * @return 再生を開始したら TRUE
 */
//--------------------------------------------------------------------------------------------
BOOL PMSND_PlayBGMdiv(u32 no, u32* seq, BOOL start)
{
	if(start == TRUE){
		PMSND_StopBGM_CORE();
		deleteSoundPlayThread();
		(*seq) = 0; 
	} else {
		switch(*seq){
		case 0:
			NNS_SndArcSetLoadBlockSize(BGM_BLOCKLOAD_SIZE);	//分割ロード指定
			createSoundPlayThread(no, THREADLOAD_SEQBANK);
			(*seq)++;
			break;
		case 1:
			if(checkEndSoundPlayThread() == TRUE){
				SOUNDMAN_LoadHierarchyPlayer_forThread_heapsvSB();// サウンド階層構造用設定

				createSoundPlayThread( no, THREADLOAD_WAVE);
				(*seq)++;
			} else {
				OS_Sleep(1);
			}
			break;
		case 2:
			if(checkEndSoundPlayThread() == TRUE){
				NNS_SndArcSetLoadBlockSize(0);	//分割ロードなしに復帰
	
				SOUNDMAN_LoadHierarchyPlayer_forThread_end(no);
	
				// サウンド再生開始
				{
					NNSSndHandle*	pBgmHandle = SOUNDMAN_GetHierarchyPlayerSndHandle();

					NNS_SndArcPlayerStartSeqEx(pBgmHandle, PLAYER_BGM, -1, -1, no);
					//NNS_SndPlayerSetVolume(pBgmHandle, 0);
				}
				(*seq)++;
        return TRUE;
      } else {
        OS_Sleep(1);
      }
      break;
		}
	}
	return FALSE;
} 


//============================================================================================
/**
 *
 *
 * @brief	外部音楽データ再生
 *
 *
 */
//============================================================================================
BOOL  PMDSND_PresetExtraMusic( void* seqAdrs, void* bnkAdrs, u32 waveNo )
{
	int i;

	SOUNDMAN_LoadHierarchyPlayer_forThread_heapsvSB();

	// 事前に波形読み込み
	if(NNS_SndArcLoadWaveArc(waveNo, PmSndHeapHandle) == FALSE ){
		return FALSE;		// 波形読み込み失敗
	}
	// ダミー音楽のFileIDを入力アドレスに置き換える
	{
		u32 bnkFileID = NNS_SndArcGetBankInfo(BANK_MUS_WB_SHINKA)->fileId;
		u32 seqFileID = NNS_SndArcGetSeqInfo(SEQ_BGM_SHINKA)->fileId;

		NNS_SndArcSetFileAddress(bnkFileID, bnkAdrs); 
		NNS_SndArcSetFileAddress(seqFileID, seqAdrs); 
	}
	SOUNDMAN_LoadHierarchyPlayer_forThread_end(SEQ_BGM_SHINKA);
	{
		// 無理やりバンク情報を書き換える
		NNSSndArcBankInfo* bnkInfo = (NNSSndArcBankInfo*)NNS_SndArcGetBankInfo(BANK_MUS_WB_SHINKA);
		bnkInfo->waveArcNo[0] = waveNo;
		bnkInfo->waveArcNo[1] = 0xffff;
		bnkInfo->waveArcNo[2] = 0xffff;
		bnkInfo->waveArcNo[3] = 0xffff;
	}
	return TRUE;
}

BOOL  PMDSND_PlayExtraMusic( void )
{
	return NNS_SndArcPlayerStartSeq( SOUNDMAN_GetHierarchyPlayerSndHandle(), SEQ_BGM_SHINKA );
}

void	PMDSND_StopExtraMusic( void )
{
	NNS_SndPlayerStopSeq(SOUNDMAN_GetHierarchyPlayerSndHandle(), 0);
	SOUNDMAN_UnloadHierarchyPlayer();
}

void  PMDSND_ReleaseExtraMusic( void )
{
	// ダミー音楽のFileIDを元に戻す
	{
		u32 bnkFileID = NNS_SndArcGetBankInfo(BANK_MUS_WB_SHINKA)->fileId;
		u32 seqFileID = NNS_SndArcGetSeqInfo(SEQ_BGM_SHINKA)->fileId;

		NNS_SndArcSetFileAddress(bnkFileID, NULL);
		NNS_SndArcSetFileAddress(seqFileID, NULL);
	}
}


