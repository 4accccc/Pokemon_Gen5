//============================================================================================
/**
 * @file	sound_manager.c
 * @brief	サウンド管理
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "sound/sound_manager.h"

static NNSSndHeapHandle*	pSndHeapHandle = NULL;
//============================================================================================
/**
 *
 *
 *
 * @brief	サウンド管理システム初期化
 *
 *
 *
 */
//============================================================================================
void	SOUNDMAN_Init(NNSSndHeapHandle* pHeapHandle)
{
	// サウンドヒープハンドル設定
	pSndHeapHandle		= pHeapHandle;
}





//============================================================================================
/**
 *
 *
 *
 * @brief	プレーヤー階層構造関数
 *
 *
 *
 */
//============================================================================================
typedef struct _PLAYER_HIERARCHY	PLAYER_HIERARCHY;

struct _PLAYER_HIERARCHY{
	BOOL				active;
	u16					soundIdx;
	u16					playerNo;
	NNSSndHandle		sndHandle;
//	int					heapLvDelete;
	int					heapLvReset;
	int					heapLvPush;
};

//--------------------------------------------------------------------------------------------
#define PLAYER_HIERARCHY_NUM			(8)
#define PLAYER_HIERARCHY_EMPTY			(0xffffffff)
#define PLAYER_HIERARCHY_HEAPLV_NULL	(0xffffffff)
static PLAYER_HIERARCHY	sndHierarchyArray[PLAYER_HIERARCHY_NUM];
static int				sndHierarchyArrayPos;

static u16	hierarchyPlayerSize;
static u16	hierarchyPlayerStartNo;
static u16	playerPlayableChannel;

//--------------------------------------------------------------------------------------------
/**
 * @brief	階層ワーク初期化orリセット
 */
//--------------------------------------------------------------------------------------------
static void initHierarchyPlayerWork( int arrayIdx )
{
	sndHierarchyArray[arrayIdx].active = FALSE;

	sndHierarchyArray[arrayIdx].soundIdx = PLAYER_HIERARCHY_EMPTY;
	NNS_SndHandleReleaseSeq(&sndHierarchyArray[arrayIdx].sndHandle);
//	sndHierarchyArray[arrayIdx].heapLvDelete = PLAYER_HIERARCHY_HEAPLV_NULL;
	sndHierarchyArray[arrayIdx].heapLvReset = PLAYER_HIERARCHY_HEAPLV_NULL;
	sndHierarchyArray[arrayIdx].heapLvPush = PLAYER_HIERARCHY_HEAPLV_NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	階層プレーヤー作成
 */
//--------------------------------------------------------------------------------------------
static void createHierarchyPlayer( void )
{
	PLAYER_HIERARCHY* player = &sndHierarchyArray[sndHierarchyArrayPos];
	BOOL result;

	if( player->active == TRUE ){ return; }

#if 0	
	// プレーヤー削除の際に復帰するヒープ状態ＬＶを保存
	player->heapLvDelete = NNS_SndHeapSaveState(sndHeapHandle);
	// プレーヤー作成
	result = NNS_SndPlayerCreateHeap(player->playerNo, sndHeapHandle, hierarchyPlayerSize );
	if( result == FALSE ){
		GF_ASSERT(0);
	}
#endif
	// チャンネル設定
	NNS_SndPlayerSetAllocatableChannel(player->playerNo, playerPlayableChannel);

	// プレーヤーリセットの際に復帰するヒープ状態ＬＶを保存
	player->heapLvReset = NNS_SndHeapSaveState(*pSndHeapHandle);

	player->active = TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	階層プレーヤー削除
 */
//--------------------------------------------------------------------------------------------
static void deleteHierarchyPlayer( void )
{
	PLAYER_HIERARCHY* player = &sndHierarchyArray[sndHierarchyArrayPos];

	if( player->active == FALSE ){ return; }

	// サウンドハンドルからシーケンスを切り離す
	NNS_SndHandleReleaseSeq(&player->sndHandle);
	// プレーヤー作成前の状態にサウンドヒープを復元
#if 0
	NNS_SndHeapLoadState(*pSndHeapHandle, player->heapLvDelete);
#else
	NNS_SndHeapLoadState(*pSndHeapHandle, player->heapLvReset);
#endif
	// プレーヤー階層ワークリセット
	initHierarchyPlayerWork( sndHierarchyArrayPos );
}


//--------------------------------------------------------------------------------------------
/**
 * 
 * @brief	階層構造初期化
 *
 */
//--------------------------------------------------------------------------------------------
void	SOUNDMAN_InitHierarchyPlayer( const SOUNDMAN_HIERARCHY_PLAYER_DATA* playerData )
{
	int i;

	GF_ASSERT(pSndHeapHandle);

	// 階層プレーヤー構造初期化
	sndHierarchyArrayPos = 0;
	for( i=0; i<PLAYER_HIERARCHY_NUM; i++ ){
		initHierarchyPlayerWork(i);
		// プレーヤーナンバー設定
		sndHierarchyArray[i].playerNo = playerData->hierarchyPlayerStartNo + i;
		// サウンドハンドルは一度だけ初期化
		NNS_SndHandleInit(&sndHierarchyArray[i].sndHandle);
	}
	// 階層プレーヤーステータス設定
	hierarchyPlayerSize		= playerData->hierarchyPlayerSize;
	hierarchyPlayerStartNo	= playerData->hierarchyPlayerStartNo;
	playerPlayableChannel	= playerData->playerPlayableChannel;
}

//--------------------------------------------------------------------------------------------
/**
 *
 * @brief	階層プレーヤー状態取得
 *
 */
//--------------------------------------------------------------------------------------------
u16		SOUNDMAN_GetHierarchyPlayerSoundIdx( void )
{
	PLAYER_HIERARCHY* player = &sndHierarchyArray[sndHierarchyArrayPos];

	if( player->active == FALSE ){ return 0; }

	return player->soundIdx;
}

u16		SOUNDMAN_GetHierarchyPlayerPlayerNo( void )
{
	PLAYER_HIERARCHY* player = &sndHierarchyArray[sndHierarchyArrayPos];

	return player->playerNo;
}

NNSSndHandle*	SOUNDMAN_GetHierarchyPlayerSndHandle( void )
{
	PLAYER_HIERARCHY* player = &sndHierarchyArray[sndHierarchyArrayPos];

	return &player->sndHandle;
}

//--------------------------------------------------------------------------------------------
/**
 *
 * @brief	サウンド再生
 *
 */
//--------------------------------------------------------------------------------------------
BOOL	SOUNDMAN_PlayHierarchyPlayer( u32 soundIdx )
{
	PLAYER_HIERARCHY* player = &sndHierarchyArray[sndHierarchyArrayPos];
	BOOL result;

	GF_ASSERT(pSndHeapHandle);

	if( player->active == FALSE ){
		createHierarchyPlayer();
		//OS_Printf("create player no = %d, heapRemains = %x resetLv = %d, ", 
		//		player->playerNo, NNS_SndHeapGetFreeSize(*pSndHeapHandle), player->heapLvReset);
	}
	// 現在の階層に関連付けされているサウンドの停止
	SOUNDMAN_StopHierarchyPlayer();

	// サウンドデータ（seq, bank）読み込み
	result = NNS_SndArcLoadSeqEx
				(soundIdx, NNS_SND_ARC_LOAD_SEQ | NNS_SND_ARC_LOAD_BANK, *pSndHeapHandle);
	if( result == FALSE){ return FALSE; }

	player->heapLvPush = NNS_SndHeapSaveState(*pSndHeapHandle);

	// サウンドデータ（wave）読み込み
	result = NNS_SndArcLoadSeqEx(soundIdx, NNS_SND_ARC_LOAD_WAVE, *pSndHeapHandle);
	if( result == FALSE){ return FALSE; }

	// サウンドデータ（seq, bank）をプレーヤーヒープに読み込み、サウンド再生開始
	result = NNS_SndArcPlayerStartSeqEx
			(&player->sndHandle, player->playerNo, -1, -1, soundIdx);
	if( result == FALSE){ return FALSE; }

	//OS_Printf("sound hierarchy start playerno = %d, pushLv = %d\n",
	//			player->playerNo, player->heapLvPush);

	player->soundIdx = soundIdx;
	return result;
}

//--------------------------------------------------------------------------------------------
/**
 *
 * @brief	サウンド停止
 *
 */
//--------------------------------------------------------------------------------------------
void	SOUNDMAN_StopHierarchyPlayer( void )
{
	PLAYER_HIERARCHY* player = &sndHierarchyArray[sndHierarchyArrayPos];

	GF_ASSERT(pSndHeapHandle);

	if( player->active == FALSE ){ return; }

	// 現在のハンドルに関連付けされているサウンドの即時停止
	NNS_SndPlayerStopSeq(&player->sndHandle, 0);

	player->soundIdx = PLAYER_HIERARCHY_EMPTY;

	// ヒープ状態をリセット
	NNS_SndHeapLoadState(*pSndHeapHandle, player->heapLvReset);
}

//--------------------------------------------------------------------------------------------
/**
 *
 * @brief	サウンド階層操作
 *
 */
//--------------------------------------------------------------------------------------------
BOOL	SOUNDMAN_PushHierarchyPlayer( void )
{
	PLAYER_HIERARCHY* player = &sndHierarchyArray[sndHierarchyArrayPos];

	GF_ASSERT(pSndHeapHandle);

	if( player->active == FALSE ){ return TRUE; }	// 階層Upの必要はない
	if( sndHierarchyArrayPos >= PLAYER_HIERARCHY_NUM -1 ){ return FALSE; }	// 階層最大

	// サウンドデータ（wave）解放
#if 0
	NNS_SndHeapLoadState(*pSndHeapHandle, player->heapLvReset);
#else
	NNS_SndHeapLoadState(*pSndHeapHandle, player->heapLvPush);
#endif

	// 階層Up
	sndHierarchyArrayPos++;

	return TRUE;
}

//------------------------------------------------------------------
BOOL	SOUNDMAN_PopHierarchyPlayer( void )
{
	PLAYER_HIERARCHY* player = &sndHierarchyArray[sndHierarchyArrayPos];

	GF_ASSERT(pSndHeapHandle);

	if( sndHierarchyArrayPos <= 0 ){ return FALSE; }	// 階層最少

	// 現在の階層に関連付けされているサウンドの停止
	SOUNDMAN_StopHierarchyPlayer();
	
	// 階層Down
	deleteHierarchyPlayer();
	sndHierarchyArrayPos--;

	// 前層のサウンドデータ（wave）があれば読み込みして復元
	player = &sndHierarchyArray[sndHierarchyArrayPos];
	if( player->soundIdx != PLAYER_HIERARCHY_EMPTY ){
		BOOL result;
		result = NNS_SndArcLoadSeqEx(player->soundIdx, NNS_SND_ARC_LOAD_WAVE, *pSndHeapHandle);
	}

	return TRUE;
}





//============================================================================================
/**
 *
 *
 *
 * @brief	サウンドプリセット関数
 *
 *
 *
 */
//============================================================================================
struct _SOUNDMAN_PRESET_HANDLE{
	u32					soundNum;
	int					heapLvRelease;
};

//--------------------------------------------------------------------------------------------
/**
 *
 * @brief	サウンドデータプリセット
 *
 */
//--------------------------------------------------------------------------------------------
SOUNDMAN_PRESET_HANDLE* SOUNDMAN_PresetSoundTbl( const u32* soundIdxTbl, u32 tblNum )
{
	SOUNDMAN_PRESET_HANDLE* handle;
	int heapLvRelease;
	int i;

	GF_ASSERT(pSndHeapHandle);

	// リリースの際に復帰するヒープ状態ＬＶを保存
	heapLvRelease = NNS_SndHeapSaveState(*pSndHeapHandle);

	handle = NNS_SndHeapAlloc(*pSndHeapHandle, sizeof(SOUNDMAN_PRESET_HANDLE), NULL, 0, 0);

	for( i=0; i<tblNum; i++ ){ NNS_SndArcLoadSeq(soundIdxTbl[i], *pSndHeapHandle); }

	handle->soundNum = tblNum;
	handle->heapLvRelease = heapLvRelease;

	return handle;
}

//--------------------------------------------------------------------------------------------
SOUNDMAN_PRESET_HANDLE* SOUNDMAN_PresetGroup( u32 groupIdx )
{
	SOUNDMAN_PRESET_HANDLE* handle;
	const NNSSndArcGroupInfo* groupInfo;
	int heapLvRelease;

	GF_ASSERT(pSndHeapHandle);

	// リリースの際に復帰するヒープ状態ＬＶを保存
	heapLvRelease = NNS_SndHeapSaveState(*pSndHeapHandle);

	handle = NNS_SndHeapAlloc(*pSndHeapHandle, sizeof(SOUNDMAN_PRESET_HANDLE), NULL, 0, 0);

	groupInfo = NNS_SndArcGetGroupInfo(groupIdx);
	NNS_SndArcLoadGroup(groupIdx, *pSndHeapHandle);

	handle->soundNum = groupInfo->count;
	handle->heapLvRelease = heapLvRelease;

	return handle;
}

//--------------------------------------------------------------------------------------------
/**
 *
 * @brief	サウンドデータリリース
 *
 */
//--------------------------------------------------------------------------------------------
void SOUNDMAN_ReleasePresetData( SOUNDMAN_PRESET_HANDLE* handle )
{
	GF_ASSERT(pSndHeapHandle);

	NNS_SndHeapLoadState(*pSndHeapHandle, handle->heapLvRelease);
}


