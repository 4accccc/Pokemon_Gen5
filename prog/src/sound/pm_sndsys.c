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

#include "sound/wb_sound_data.sadl"		//サウンドラベルファイル
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
//------------------------------------------------------------------
/**
 * @brief	プレーヤー定義
 */
//------------------------------------------------------------------
typedef struct {
	u32 size;
	u16 channelBit;
	u8	playableNum;
}PMSNDSYS_PLSETUP;

#define PMSNDSYS_PLSETUP_TBLEND (0xffffffff)

typedef struct {
	NNSSndHandle	sndHandle;
	u16				playerNo;
}PMSNDSYS_PLAYER_DATA;

typedef struct {
	PMSNDSYS_PLAYER_DATA*	playerDataArray;
	u16						playerNum;
	int						heapLvDelete;
	int						heapLvReset;
}PMSNDSYS_PL_UNIT;

//------------------------------------------------------------------
/**
 * @brief	プレーヤー設定定義
 */
//------------------------------------------------------------------
#define PLAYER_PMVOICE_CH	(0xc000)
#define PLAYER_SE_CH		(0xd800)

enum {
	PLAYER_SYSSE = 0,
	PLAYER_SE,
//	PLAYER_BASEBGM,
//	PLAYER_PMVOICE,

	PLAYER_DEFAULT_MAX,
};

static const PMSNDSYS_PLSETUP systemPlayer[] = {
	{ 0x6000, 0x0000, 1 },
	{ 0x6000, 0x0000, 1 },
//	{ 0x6000, PLAYER_MUSIC_CH, 1 },
//	{ 0x6000, PLAYER_PMVOICE_CH, 2 },

	{ PMSNDSYS_PLSETUP_TBLEND, 0x0000, 0 },	// TABLE END
};

#define PLAYER_MUSIC_HEAPSIZ	(0x4000)
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

static u8				PmSoundHeap[SOUND_HEAP_SIZE];
static NNSSndArc		PmSoundArc;

static NNSSndHeapHandle		PmSndHeapHandle;
static u32					playerNumber;

static PMSNDSYS_PL_UNIT systemPlayerUnit;
SOUNDMAN_PRESET_HANDLE* systemPresetHandle;

static const u32 systemPresetSoundIdxTbl[] = {
	1500, 1501, 1504, 1505, 1508, 1509, 1512, 1513,
};

static void PMSNDSYS_CreatePlayerUnit
		( const PMSNDSYS_PLSETUP* setupTbl, PMSNDSYS_PL_UNIT* playerUnit );
static void PMSNDSYS_DeletePlayerUnit
		( PMSNDSYS_PL_UNIT* playerUnit );
//============================================================================================
/**
 *
 * @brief	セットアップ＆初期化
 *
 */
//============================================================================================
void	PMSNDSYS_Init( void )
{
	// サウンドストリーミング再生システム
	//SND_STRM_Init(GFL_HEAPID_SYSTEM);
    // サウンドシステム初期化
	//SOUNDSYS_Init("wb_sound_data.sdat", &pmHierarchyPlayerData );

    // サウンドシステム初期化
 	NNS_SndInit();
	PmSndHeapHandle = NNS_SndHeapCreate(PmSoundHeap, SOUND_HEAP_SIZE);
    // サウンドの設定
	NNS_SndArcInitWithResult( &PmSoundArc, "wb_sound_data.sdat", PmSndHeapHandle, FALSE );
    // サウンド管理初期化
	SOUNDMAN_Init(&PmSndHeapHandle);
    // サウンド階層構造初期化
	SOUNDMAN_InitHierarchyPlayer(&pmHierarchyPlayerData);

//	PmSndHeapHandle = SOUNDSYS_GetSndHeapHandle();

    // サウンドの設定
	playerNumber = 0;

	PMSNDSYS_CreatePlayerUnit(systemPlayer, &systemPlayerUnit);
	// 常駐サウンドデータ読み込み
	systemPresetHandle = SOUNDMAN_PresetSoundTbl
							(systemPresetSoundIdxTbl, NELEMS(systemPresetSoundIdxTbl));
}

//============================================================================================
/**
 *
 * @brief	フレームワーク
 *
 */
//============================================================================================
void	PMSNDSYS_Main( void )
{
	//SND_STRM_Main();
	//SOUNDSYS_Main();
	NNS_SndMain();
}

//============================================================================================
/**
 *
 * @brief	終了処理＆破棄
 *
 */
//============================================================================================
void	PMSNDSYS_Exit( void )
{
	SOUNDMAN_ReleasePresetData(systemPresetHandle);

	PMSNDSYS_DeletePlayerUnit(&systemPlayerUnit);
	NNS_SndHeapDestroy(PmSndHeapHandle);
	//SOUNDSYS_Exit();
	//SND_STRM_Exit();
}

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	可変プレーヤーシステム
 *
 *
 *
 *
 *
 */
//============================================================================================
//============================================================================================
/**
 *
 * @brief	プレーヤーユニット作成
 *
 */
//============================================================================================
static void PMSNDSYS_CreatePlayerUnit
			( const PMSNDSYS_PLSETUP* setupTbl, PMSNDSYS_PL_UNIT* playerUnit )
{
	BOOL result;
	int playerNo;
	int i;

	// プレーヤーユニット削除の際に復帰するヒープ状態ＬＶを保存
	playerUnit->heapLvDelete = NNS_SndHeapSaveState(PmSndHeapHandle);

	// プレーヤー数カウント
	i = 0;
	while(setupTbl[i].size != PMSNDSYS_PLSETUP_TBLEND){ i++; }
	playerUnit->playerNum = i;

	// プレーヤー配列作成
	playerUnit->playerDataArray = (PMSNDSYS_PLAYER_DATA*)NNS_SndHeapAlloc
			(PmSndHeapHandle, sizeof(PMSNDSYS_PLAYER_DATA) * playerUnit->playerNum, NULL, 0, 0 );

	for( i=0; i<playerUnit->playerNum; i++ ){
		playerNo = i + playerNumber;
		// プレーヤー作成
#if 0
		result = NNS_SndPlayerCreateHeap(playerNo, PmSndHeapHandle, setupTbl[i].size);
		if( result == FALSE ){
			GF_ASSERT(0);
		}
#endif
		// チャンネル設定
		NNS_SndPlayerSetAllocatableChannel(playerNo, setupTbl[i].channelBit);
		NNS_SndPlayerSetPlayableSeqCount(playerNo, setupTbl[i].playableNum);

		NNS_SndHandleInit(&playerUnit->playerDataArray[i].sndHandle);
		playerUnit->playerDataArray[i].playerNo = playerNo;
		OS_Printf("player %d handle = %x\n", playerNo, &playerUnit->playerDataArray[i].sndHandle);
	}
	playerNumber += playerUnit->playerNum; 

	// プレーヤーリセットの際に復帰するヒープ状態ＬＶを保存
	playerUnit->heapLvReset = NNS_SndHeapSaveState(PmSndHeapHandle);
}

//============================================================================================
/**
 *
 * @brief	プレーヤーユニット削除
 *
 */
//============================================================================================
static void PMSNDSYS_DeletePlayerUnit( PMSNDSYS_PL_UNIT* playerUnit )
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
 *
 *
 *
 *
 * @brief	サウンド関数（各appから呼び出される）
 *
 *
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	サウンドハンドル取得
 */
//------------------------------------------------------------------
NNSSndHandle* PMSNDSYS_GetBGMhandlePointer( void )
{
	return SOUNDMAN_GetHierarchyPlayerSndHandle();
}

//------------------------------------------------------------------
/**
 * @brief	サウンド再生
 */
//------------------------------------------------------------------
BOOL	PMSNDSYS_PlayBGM( u32 soundIdx )
{
	BOOL result;

	SOUNDMAN_StopHierarchyPlayer();
	result = SOUNDMAN_PlayHierarchyPlayer(soundIdx);

	return result;
}

//------------------------------------------------------------------
/**
 * @brief	サウンド停止
 */
//------------------------------------------------------------------
void	PMSNDSYS_StopBGM( void )
{
	SOUNDMAN_StopHierarchyPlayer();
}

//------------------------------------------------------------------
/**
 * @brief	サウンド一時停止
 */
//------------------------------------------------------------------
void	PMSNDSYS_PauseBGM( BOOL pauseFlag )
{
	SOUNDMAN_PauseHierarchyPlayer(pauseFlag);
}

//------------------------------------------------------------------
/**
 * @brief	サウンド一時停止→上位ＢＧＭ
 */
//------------------------------------------------------------------
void	PMSNDSYS_PushBGM( void )
{
	SOUNDMAN_PauseHierarchyPlayer(TRUE);
	SOUNDMAN_PushHierarchyPlayer();
}

//------------------------------------------------------------------
/**
 * @brief	サウンド停止→下位ＢＧＭ
 */
//------------------------------------------------------------------
void	PMSNDSYS_PopBGM( void )
{
	SOUNDMAN_PopHierarchyPlayer();
	SOUNDMAN_PauseHierarchyPlayer(FALSE);
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
BOOL	PMSNDSYS_PlaySE( u32 soundIdx )
{
	PMSNDSYS_PLAYER_DATA* playerData = &systemPlayerUnit.playerDataArray[PLAYER_SE];
	BOOL result;

	NNS_SndPlayerStopSeq(&playerData->sndHandle, 0);
	// プレイヤー使用チャンネル設定
	NNS_SndPlayerSetAllocatableChannel(playerData->playerNo, PLAYER_SE_CH);

	result = NNS_SndArcPlayerStartSeqEx
				(&playerData->sndHandle, playerData->playerNo, -1, 126, soundIdx);

	if(result == FALSE){
		OS_Printf("sound_Effect start Error... soundIdx = %d\n", soundIdx);
		return FALSE;
	}
	return TRUE;
}

//============================================================================================
/**
 *
 *
 *
 * @brief	鳴き声サウンド関数（各appから呼び出される）
 *
 *
 *
 */
//============================================================================================
BOOL	PMSNDSYS_PlayVoice( u32 pokeNum )
{
	PMSNDSYS_PLAYER_DATA* playerData = &systemPlayerUnit.playerDataArray[PLAYER_SE];
	BOOL result;

	NNS_SndPlayerStopSeq(&playerData->sndHandle, 0);
	// プレイヤー使用チャンネル設定
	NNS_SndPlayerSetAllocatableChannel(playerData->playerNo, PLAYER_PMVOICE_CH);

	result = NNS_SndArcPlayerStartSeqEx
				(&playerData->sndHandle, playerData->playerNo, pokeNum, 127, SEQ_PV);

	if(result == FALSE){
		OS_Printf("voice start Error... pokeNum = %d\n", pokeNum);
		return FALSE;
	}
	return TRUE;
}

