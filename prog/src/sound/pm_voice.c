//============================================================================================
/**
 * @file	pm_voice.c
 * @brief	ポケモン鳴き声波形再生基本システム	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "sound/pm_voice.h"
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
#define PMVOICE_WAVESIZE_MAX	(26000)

#define VOICE_RATE_DEFAULT		(13379)
#define VOICE_SPEED_DEFAULT		(25825)	//seqデータ(pm_voice.sseq)内((u16)0x1c)を抜粋

#define VOICE_VOLUME			(120)	//seq設定(sound_data.sarc)内(PV_VOL)を参照
//------------------------------------------------------------------
/**
 * @brief	波形プレーヤー定義
 */
//------------------------------------------------------------------
typedef struct {
	BOOL				active;
	void*				waveData;
	u32					waveSize;
	int					waveRate;

	NNSSndWaveOutHandle waveHandle;	
	NNSSndWaveOutHandle waveHandleSub;	
	s8					volume;
	s8					volumeSubDiff;
	u8					channel:4;
	u8					channelSub:4;
	int					speed;
	int					speedSubDiff;
	u8					pan;
	BOOL				subWaveUse;
	BOOL				heapReserveFlag;
}PMVOICE_PLAYER;

static  u8 staticWaveData[PMVOICE_WAVESIZE_MAX];
//------------------------------------------------------------------
/**
 * @brief	システム定義
 */
//------------------------------------------------------------------
#define VOICE_PLAYER_NUM (3)

typedef struct {
	HEAPID			heapID;
	PMVOICE_PLAYER	voicePlayer[VOICE_PLAYER_NUM];
	u16				voicePlayerRef;
	u16				voicePlayerEnableNum;
	PMVOICE_CB_GET_WVIDX*	CallBackGetWaveIdx;
	PMVOICE_CB_GET_WVDAT*	CallBackCustomWave;
}PMVOICE_SYS;

//------------------------------------------------------------------
/**
 * @brief	チャンネル使用データ
 *				※確保するチャンネルの使用順
 */
//------------------------------------------------------------------
static const u16 useChannnelPriority[VOICE_PLAYER_NUM][2] = 
	{ {15, 14}, {13, 12}, {11, 10} };

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	ポケモン鳴き声波形プレーヤー
 *
 *
 *
 *
 *
 */
//============================================================================================
static PMVOICE_SYS pmvSys;

static u16	getPlayerIdx( void );
static void initPlayerWork( PMVOICE_PLAYER* voicePlayer );
static BOOL loadWave( PMVOICE_PLAYER* voicePlayer, int waveNo );
static BOOL playWave( PMVOICE_PLAYER* voicePlayer );
static void stopWave( PMVOICE_PLAYER* voicePlayer );

static void reverseBuf( u8* buf, u32 size );
//============================================================================================
/**
 *
 *
 * @brief	システム（main.c等から呼び出される）
 *
 *
 */
//============================================================================================

//============================================================================================
/**
 * @brief	初期化
 */
//============================================================================================
void	PMVOICE_Init
		( HEAPID heapID,							// 使用heapID 
		  PMVOICE_CB_GET_WVIDX* CallBackGetWaveIdx,	// waveIdx取得用コールバック
		  PMVOICE_CB_GET_WVDAT* CallBackCustomWave 	// 波形カスタマイズコールバック
		)
{
	PMVOICE_PLAYER* voicePlayer;
	int i;

	pmvSys.heapID = heapID;

	for( i=0; i<VOICE_PLAYER_NUM; i++ ){
		voicePlayer = &pmvSys.voicePlayer[i];

		voicePlayer->active = FALSE;
		voicePlayer->channel = useChannnelPriority[i][0];
		voicePlayer->channelSub = useChannnelPriority[i][1];
		initPlayerWork(voicePlayer);
		// player0 だけは静的確保する
		if(i == 0){
			voicePlayer->waveData = staticWaveData;
		}
	}
	pmvSys.voicePlayerEnableNum = 1;
	pmvSys.voicePlayerRef = 0;

	pmvSys.CallBackGetWaveIdx = CallBackGetWaveIdx;
	pmvSys.CallBackCustomWave = CallBackCustomWave;
}

//============================================================================================
/**
 * @brief	リセット
 */
//============================================================================================
void	PMVOICE_Reset( void )
{
	PMVOICE_PLAYER* voicePlayer;
	int i;

	for( i=0; i<VOICE_PLAYER_NUM; i++ ){
		voicePlayer = &pmvSys.voicePlayer[i];
		if(voicePlayer->active == TRUE){ stopWave(voicePlayer); }
	}
}

//============================================================================================
/**
 * @brief	フレームワーク
 */
//============================================================================================
void	PMVOICE_Main()
{
	PMVOICE_PLAYER* voicePlayer;
	BOOL result, resultSub;
	int i;

	for( i=0; i<pmvSys.voicePlayerEnableNum; i++ ){
		voicePlayer = &pmvSys.voicePlayer[i];

		if(voicePlayer->active == TRUE){
			result = NNS_SndWaveOutIsPlaying(voicePlayer->waveHandle);

			if( voicePlayer->subWaveUse == TRUE ){
				resultSub = NNS_SndWaveOutIsPlaying(voicePlayer->waveHandleSub);
			} else {
				resultSub = FALSE;
			}

			if( (result == FALSE)&&(resultSub == FALSE) ){
				stopWave(voicePlayer); 
			}
		}
	}
}

//============================================================================================
/**
 * @brief	終了処理＆破棄
 */
//============================================================================================
void	PMVOICE_Exit( void )
{
	PMVOICE_PLAYER* voicePlayer;
	int i;

	for( i=0; i<VOICE_PLAYER_NUM; i++ ){ 
		voicePlayer = &pmvSys.voicePlayer[i];

		if(voicePlayer->active == TRUE){ stopWave(voicePlayer); }
		// 未開放バッファがあれば開放
		if((i != 0)&&(voicePlayer->waveData != NULL)){
			GFL_HEAP_FreeMemory(voicePlayer->waveData);
		}
		voicePlayer->active = FALSE;
		initPlayerWork(voicePlayer);
	}
}

//============================================================================================
/**
 * @brief	プレーヤー稼動状態チェック
 */
//============================================================================================
BOOL	PMVOICE_CheckBusy( void )
{
	PMVOICE_PLAYER* voicePlayer;
	int i;

	for( i=0; i<VOICE_PLAYER_NUM; i++ ){
		voicePlayer = &pmvSys.voicePlayer[i];
		if(voicePlayer->active == TRUE){ return TRUE; }
	}
	return FALSE;
}

//============================================================================================
/**
 * @brief	プレーヤー用waveバッファ事前確保
 */
//============================================================================================
void	PMVOICE_PlayerHeapReserve( int num, HEAPID heapID )
{
	PMVOICE_PLAYER* voicePlayer;
	int i;

	PMVOICE_Reset();

	if( num > (VOICE_PLAYER_NUM-1) ){ num = VOICE_PLAYER_NUM-1; }

	for( i=1; i<num+1; i++ ){ 
		voicePlayer = &pmvSys.voicePlayer[i];
		if(voicePlayer->waveData == NULL){
			voicePlayer->waveData = GFL_HEAP_AllocClearMemory(heapID, PMVOICE_WAVESIZE_MAX);
			voicePlayer->heapReserveFlag = TRUE;
		}
	}
	pmvSys.voicePlayerEnableNum = num + 1;
	pmvSys.voicePlayerRef = 0;
}

//============================================================================================
/**
 * @brief	プレーヤー用waveバッファ開放
 */
//============================================================================================
void	PMVOICE_PlayerHeapRelease( void )
{
	PMVOICE_PLAYER* voicePlayer;
	int i;

	PMVOICE_Reset();

	for( i=0; i<pmvSys.voicePlayerEnableNum; i++ ){ 
		voicePlayer = &pmvSys.voicePlayer[i];
		if((voicePlayer->waveData != NULL)&&(voicePlayer->heapReserveFlag == TRUE)){
			GFL_HEAP_FreeMemory(voicePlayer->waveData);
			initPlayerWork(voicePlayer);
		}
	}
	pmvSys.voicePlayerEnableNum = 1;
	pmvSys.voicePlayerRef = 0;
}





//============================================================================================
/**
 *
 *
 * @brief	プレーヤー関数
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	プレーヤーＩＤＸ取得
 */
//------------------------------------------------------------------
static u16 getPlayerIdx( void )
{
	PMVOICE_PLAYER* voicePlayer;
	u16 voicePlayerIdx = pmvSys.voicePlayerRef;

	voicePlayer = &pmvSys.voicePlayer[voicePlayerIdx];
	if(voicePlayer->active == TRUE){ stopWave(voicePlayer); }

	if( pmvSys.voicePlayerRef < pmvSys.voicePlayerEnableNum-1){ 
		pmvSys.voicePlayerRef++;
	} else {
		pmvSys.voicePlayerRef = 0;
	}
	return voicePlayerIdx;
}

//------------------------------------------------------------------
/**
 * @brief	プレーヤーワーク初期化
 */
//------------------------------------------------------------------
static void resetPlayerWork( PMVOICE_PLAYER* voicePlayer )
{
	voicePlayer->waveSize = 0;
	voicePlayer->waveRate = VOICE_RATE_DEFAULT;
	voicePlayer->volume = 0;
	voicePlayer->volumeSubDiff = 0;
	voicePlayer->speed = VOICE_SPEED_DEFAULT;
	voicePlayer->speedSubDiff = 0;
	voicePlayer->pan = 64;
	voicePlayer->subWaveUse = FALSE;
}

static void initPlayerWork( PMVOICE_PLAYER* voicePlayer )
{
	resetPlayerWork(voicePlayer);
	voicePlayer->waveData = NULL;
	voicePlayer->heapReserveFlag = FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	波形ロード
 */
//------------------------------------------------------------------
static BOOL loadWave( PMVOICE_PLAYER* voicePlayer, int waveNo )
{
	const NNSSndArcWaveArcInfo* waveArcInfo;
	u32 waveFileSize;
	BOOL result;

	// 波形アーカイブ情報構造体ポインタを取得
	waveArcInfo = NNS_SndArcGetWaveArcInfo(waveNo);
	if( waveArcInfo == NULL ){ return FALSE; }	// 波形データ取得失敗

	// ファイルサイズ取得
	waveFileSize = NNS_SndArcGetFileSize(waveArcInfo->fileId);
	if( waveFileSize == 0 ){ return FALSE; }	// ファイルＩＤ無効

	// 波形データ読み込み
	result = NNS_SndArcReadFile(waveArcInfo->fileId, voicePlayer->waveData, waveFileSize, 0);
	if( result == -1 ){ return FALSE; }				// 読み込み失敗
	if( result != waveFileSize ){ return FALSE; }	// 読み込みサイズ不整合

	voicePlayer->waveSize = waveFileSize;
	voicePlayer->waveRate = VOICE_RATE_DEFAULT;
	voicePlayer->speed = VOICE_SPEED_DEFAULT;

	return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	波形再生
 */
//------------------------------------------------------------------
static BOOL playWave( PMVOICE_PLAYER* voicePlayer )
{
	BOOL result;

	if(voicePlayer->waveData == NULL){ return FALSE; } 
	if(voicePlayer->waveSize == 0){ return FALSE; } 

	// 使用チャンネル確保
	voicePlayer->waveHandle = NNS_SndWaveOutAllocChannel(voicePlayer->channel);		
	if( voicePlayer->waveHandle == NNS_SND_WAVEOUT_INVALID_HANDLE ){ return FALSE; } 

	result = NNS_SndWaveOutStart(	voicePlayer->waveHandle,	// 波形再生ハンドル
									NNS_SND_WAVE_FORMAT_PCM8,	// 波形データフォーマット
									voicePlayer->waveData,		// 波形データアドレス
									FALSE, 0,									// ループフラグ,開始位置
									voicePlayer->waveSize,		// 波形データサンプル数
									voicePlayer->waveRate,		// 波形データサンプリングレート
									voicePlayer->volume,			// 再生volume
									voicePlayer->speed,				// 再生speed
									voicePlayer->pan );				// 再生pan
	if( result == FALSE ){ return FALSE; }

	if( voicePlayer->subWaveUse == TRUE ){
		int	volumeSub;
		int	speedSub;
		// 使用チャンネル確保
		voicePlayer->waveHandleSub = NNS_SndWaveOutAllocChannel(voicePlayer->channelSub);		
		if( voicePlayer->waveHandleSub == NNS_SND_WAVEOUT_INVALID_HANDLE ){ return FALSE; } 

		volumeSub = voicePlayer->volume + voicePlayer->volumeSubDiff;
		if( volumeSub < 0 ){ volumeSub = 0; }
		if( volumeSub > 127 ){ volumeSub = 127; }
		speedSub = voicePlayer->speed + voicePlayer->speedSubDiff;

		result = NNS_SndWaveOutStart(	voicePlayer->waveHandleSub,	// 波形再生ハンドル
										NNS_SND_WAVE_FORMAT_PCM8,	// 波形データフォーマット
										voicePlayer->waveData,		// 波形データアドレス
										FALSE, 0,									// ループフラグ,開始位置
										voicePlayer->waveSize,		// 波形データサンプル数
										voicePlayer->waveRate,		// 波形データサンプリングレート
										(s8)volumeSub,						// 再生volume
										speedSub,									// 再生speed
										voicePlayer->pan );				// 再生pan
		if( result == FALSE ){ return FALSE; }
	}
	voicePlayer->active = TRUE;

	return TRUE;
}
	
//------------------------------------------------------------------
/**
 * @brief	波形停止
 */
//------------------------------------------------------------------
static void stopWave( PMVOICE_PLAYER* voicePlayer )
{
	NNS_SndWaveOutStop(voicePlayer->waveHandle);
	NNS_SndWaveOutWaitForChannelStop(voicePlayer->waveHandle);
	// 使用チャンネル開放
	NNS_SndWaveOutFreeChannel(voicePlayer->waveHandle);

	if( voicePlayer->subWaveUse == TRUE ){
		NNS_SndWaveOutStop(voicePlayer->waveHandleSub);
		NNS_SndWaveOutWaitForChannelStop(voicePlayer->waveHandleSub);
		// 使用チャンネル開放
		NNS_SndWaveOutFreeChannel(voicePlayer->waveHandleSub);
	}
#if 0
	// 波形データバッファ開放
	if((voicePlayer->waveData != NULL)&&(voicePlayer->heapReserveFlag == FALSE)){
		GFL_HEAP_FreeMemory(voicePlayer->waveData); 
	}
#endif
	voicePlayer->active = FALSE;
	resetPlayerWork(voicePlayer);
}





//============================================================================================
/**
 *
 *
 * @brief	鳴き声サウンド関数（各appから呼び出される）
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	鳴き声再生関数
 */
//------------------------------------------------------------------
u32		PMVOICE_Play
		(	u32			pokeNo,			// ポケモンナンバー
			u32			pokeFormNo,		// ポケモンフォームナンバー
			u8			pan,			// 定位(L:0 - 64 - 127:R)
			BOOL		chorus,			// コーラス使用フラグ
			int			chorusVolOfs,	// コーラスボリューム差
			int			chorusSpOfs,	// 再生速度差
			BOOL		reverse,		// 逆再生フラグ
			u32			userParam		// ユーザーパラメーター	
		)		
{
	PMVOICE_PLAYER* voicePlayer;
	u16		voicePlayerIdx;
	u32		waveIdx;
	BOOL	waveLoadFlag;

	GF_ASSERT(pmvSys.CallBackGetWaveIdx);
	GF_ASSERT(pmvSys.CallBackCustomWave);

	// 波形IDX取得
	pmvSys.CallBackGetWaveIdx(pokeNo, pokeFormNo, &waveIdx);

	// 再生プレーヤー取得
	voicePlayerIdx = getPlayerIdx();
	voicePlayer = &pmvSys.voicePlayer[voicePlayerIdx];

	if(voicePlayer->active == TRUE){ stopWave(voicePlayer); };
#if 0
	// 波形データバッファ確保
	if( voicePlayer->heapReserveFlag == FALSE ){
		// 常に最大値で確保（種別によって異なると潜在バグを生む可能性があるため）
		voicePlayer->waveData = GFL_HEAP_AllocClearMemory(pmvSys.heapID, PMVOICE_WAVESIZE_MAX);
	}
#endif	
	// 波形データカスタマイズ(TRUE: コールバック内で生成された)
	waveLoadFlag = pmvSys.CallBackCustomWave(	pokeNo, pokeFormNo, userParam,
													&voicePlayer->waveData, 
													&voicePlayer->waveSize, 
													&voicePlayer->waveRate, 
													&voicePlayer->speed);
	// コールバック内で生成されなかった場合、波形データ取得
	if( waveLoadFlag == FALSE ){ loadWave(voicePlayer, waveIdx); }

	// 逆再生用データ加工
	if( reverse ){ reverseBuf(voicePlayer->waveData, voicePlayer->waveSize); }

	// 各種設定
	voicePlayer->volume = VOICE_VOLUME;
	voicePlayer->pan = pan;
	voicePlayer->subWaveUse = chorus;
	voicePlayer->volumeSubDiff = chorusVolOfs;
	voicePlayer->speedSubDiff = chorusSpOfs;

	// 波形再生
	if( playWave(voicePlayer) == FALSE ){ stopWave(voicePlayer); };
	
	//OS_Printf("voicePlayerIdx = %x\n",voicePlayerIdx);
	return voicePlayerIdx;
}

//------------------------------------------------------------------
/**
 * @brief	鳴き声ステータス変更関数
 */
//------------------------------------------------------------------
void	PMVOICE_SetPan( u32 voicePlayerIdx, u8 pan)
{
	PMVOICE_PLAYER* voicePlayer;

	voicePlayer = &pmvSys.voicePlayer[voicePlayerIdx];
	if(voicePlayer->active == FALSE){ return; }

	voicePlayer->pan = pan;
	NNS_SndWaveOutSetPan(voicePlayer->waveHandle, pan);
	if( voicePlayer->subWaveUse == TRUE ){
		NNS_SndWaveOutSetPan(voicePlayer->waveHandleSub, pan);
	}
}

void	PMVOICE_SetVolume( u32 voicePlayerIdx, s8 volume)
{
	PMVOICE_PLAYER* voicePlayer;

	voicePlayer = &pmvSys.voicePlayer[voicePlayerIdx];
	if(voicePlayer->active == FALSE){ return; }

	voicePlayer->volume = volume;
	NNS_SndWaveOutSetVolume(voicePlayer->waveHandle, voicePlayer->volume);

	if( voicePlayer->subWaveUse == TRUE ){
		int	volumeSub = voicePlayer->volume + voicePlayer->volumeSubDiff;
		if( volumeSub < 0 ){ volumeSub = 0; }
		if( volumeSub > 127 ){ volumeSub = 127; }
		NNS_SndWaveOutSetVolume(voicePlayer->waveHandleSub, (s8)volumeSub);
	}
}

void	PMVOICE_SetSpeed( u32 voicePlayerIdx, int speed)
{
	PMVOICE_PLAYER* voicePlayer;

	voicePlayer = &pmvSys.voicePlayer[voicePlayerIdx];
	if(voicePlayer->active == FALSE){ return; }

	voicePlayer->speed = speed;
	NNS_SndWaveOutSetSpeed(voicePlayer->waveHandle, voicePlayer->speed);

	if( voicePlayer->subWaveUse == TRUE ){
		int speedSub = voicePlayer->speed + voicePlayer->speedSubDiff;
		NNS_SndWaveOutSetSpeed(voicePlayer->waveHandleSub, speedSub);
	}
}

u8	PMVOICE_GetPan( u32 voicePlayerIdx)
{	
	PMVOICE_PLAYER* voicePlayer;

	voicePlayer = &pmvSys.voicePlayer[voicePlayerIdx];
	if(voicePlayer->active == FALSE){ return 64; }

	return voicePlayer->pan;
}

s8	PMVOICE_GetVolume( u32 voicePlayerIdx)
{	
	PMVOICE_PLAYER* voicePlayer;

	voicePlayer = &pmvSys.voicePlayer[voicePlayerIdx];
	if(voicePlayer->active == FALSE){ return 0; }

	return voicePlayer->volume;
}

int	PMVOICE_GetSpeed( u32 voicePlayerIdx)
{	
	PMVOICE_PLAYER* voicePlayer;

	voicePlayer = &pmvSys.voicePlayer[voicePlayerIdx];
	if(voicePlayer->active == FALSE){ return VOICE_SPEED_DEFAULT; }

	return voicePlayer->speed;
}

//------------------------------------------------------------------
/**
 * @brief	鳴き声停止関数
 */
//------------------------------------------------------------------
void	PMVOICE_Stop( u32 voicePlayerIdx )
{
	PMVOICE_PLAYER* voicePlayer;

	voicePlayer = &pmvSys.voicePlayer[voicePlayerIdx];
	if(voicePlayer->active == TRUE){ stopWave(voicePlayer); }
}

//------------------------------------------------------------------
/**
 * @brief	鳴き声終了検出
 */
//------------------------------------------------------------------
BOOL	PMVOICE_CheckPlay( u32 voicePlayerIdx )
{
	PMVOICE_PLAYER* voicePlayer;

	voicePlayer = &pmvSys.voicePlayer[voicePlayerIdx];
	return voicePlayer->active;
}





//============================================================================================
/**
 *
 *
 * @brief	
 *
 *
 */
//============================================================================================
static void reverseBuf( u8* buf, u32 size )
{
	u8	data;
	int i;

	//逆再生用にデータを逆順に入れ替える( [0]-[MAX]、[1]-[MAX-1]... )
	for( i=0; i<(size/2); i++ ){
		data = buf[i];
		buf[i] = buf[size-1-i];
		buf[size-1-i] = data;
	}
}


