//============================================================================================
/**
 * @file	pm_sndsys.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "sound/pm_sndsys.h"
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
//------------------------------------------------------------------
/**
 * @brief	鳴き声プレーヤー定義
 */
//------------------------------------------------------------------
typedef struct {
	BOOL				active;
	NNSSndWaveOutHandle waveHandle;	
	u8					volume;
	u8					channel;
	void*				waveData;
	u32					waveSize;
}PMVOICE_PLAYER;

//------------------------------------------------------------------
/**
 * @brief	システム定義
 */
//------------------------------------------------------------------
#define VOICE_PLAYER_NUM (8)

typedef struct {
	HEAPID			heapID;
	PMVOICE_PLAYER	voicePlayer[8];
}PMVOICE_SYS;

//------------------------------------------------------------------
/**
 * @brief	チャンネル使用データ
 *				※確保するチャンネルの使用順
 */
//------------------------------------------------------------------
static const u16 useChannnelPriority[16] = { 15, 14, 13, 12, 11, 10, 9, 8 };

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	ポケモン鳴き声再生（波形再生）
 *
 *
 *
 *
 *
 */
//============================================================================================
static PMVOICE_SYS pmvSys;

static BOOL playWave( PMVOICE_PLAYER* voicePlayer );
static void stopWave( PMVOICE_PLAYER* voicePlayer );
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
void	PMVOICE_Init( HEAPID heapID )
{
	PMVOICE_PLAYER* voicePlayer;
	int i;

	pmvSys.heapID = heapID;

	for( i=0; i<VOICE_PLAYER_NUM; i++ ){
		voicePlayer = &pmvSys.voicePlayer[i];
		voicePlayer->active = FALSE;
		voicePlayer->channel = useChannnelPriority[i];
		voicePlayer->waveData = NULL;
		voicePlayer->waveSize = 0;
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
	int i;

	for( i=0; i<VOICE_PLAYER_NUM; i++ ){
		voicePlayer = &pmvSys.voicePlayer[i];

		if(voicePlayer->active == TRUE){
			if(NNS_SndWaveOutIsPlaying(voicePlayer->waveHandle) == FALSE){ 
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
	}
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
#define VOICE_RATE_DEFAULT	(13379)
#define VOICE_SPEED_DEFAULT (25825)	//seqデータ(pm_voice.sseq)内((u16)0x1c)を抜粋
//------------------------------------------------------------------
/**
 * @brief	波形ロード
 */
//------------------------------------------------------------------
static BOOL loadWave( PMVOICE_PLAYER* voicePlayer, int waveNo )
{
	const NNSSndSeqParam* seqInfo;
	const NNSSndArcWaveArcInfo* waveArcInfo;
	u32 waveFileSize;
	BOOL result;

	if(( waveNo < PMVOICE_START)&&( waveNo > PMVOICE_END) ){ return FALSE; }	//指定範囲外

	// 波形アーカイブ情報構造体ポインタを取得
	waveArcInfo = NNS_SndArcGetWaveArcInfo(waveNo);
	if( waveArcInfo == NULL ){ return FALSE; }	// 波形データ取得失敗

	// ファイルサイズ取得
	waveFileSize = NNS_SndArcGetFileSize(waveArcInfo->fileId);
	if( waveFileSize == 0 ){ return FALSE; }	// ファイルＩＤ無効

	// 波形データバッファ確保
	voicePlayer->waveData = GFL_HEAP_AllocClearMemory(pmvSys.heapID, waveFileSize);

	// 波形データ読み込み
	result = NNS_SndArcReadFile(waveArcInfo->fileId, voicePlayer->waveData, waveFileSize, 0);
	if( result == -1 ){ return FALSE; }				// 読み込み失敗
	if( result != waveFileSize ){ return FALSE; }	// 読み込みサイズ不整合

	// シーケンス情報構造体ポインタを取得
	seqInfo = NNS_SndArcGetSeqParam(SEQ_PV);
	voicePlayer->volume = seqInfo->volume;

	voicePlayer->waveSize = waveFileSize;

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
									FALSE, 0,					// ループフラグ,開始位置
									voicePlayer->waveSize,		// 波形データサンプル数
									VOICE_RATE_DEFAULT,			// 波形データサンプリングレート
									voicePlayer->volume,		// 再生volume
									VOICE_SPEED_DEFAULT,		// 再生speed
									64 );						// 再生pan
	if( result == TRUE ){ voicePlayer->active = TRUE; }

	return result;
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
	// 波形データバッファ開放
	if(voicePlayer->waveData != NULL){ GFL_HEAP_FreeMemory(voicePlayer->waveData); }

	voicePlayer->active = FALSE;
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
int	PMVOICE_Play( u32 pokeNum )
{
	PMVOICE_PLAYER* voicePlayer;
	int i;

	for( i=0; i<VOICE_PLAYER_NUM; i++ ){
		voicePlayer = &pmvSys.voicePlayer[i];

		if(voicePlayer->active == FALSE){
			loadWave(voicePlayer, (pokeNum-1) + PMVOICE_POKE001);
			playWave(voicePlayer);
			return i;
		}
	}
	return PMVOICE_ERROR;
}

//------------------------------------------------------------------
/**
 * @brief	鳴き声停止関数
 */
//------------------------------------------------------------------
void	PMVOICE_Stop( int voicePlayerIdx )
{
	PMVOICE_PLAYER* voicePlayer;

	GF_ASSERT( (voicePlayerIdx >= 0)&&(voicePlayerIdx < VOICE_PLAYER_NUM) );

	voicePlayer = &pmvSys.voicePlayer[voicePlayerIdx];

	if(voicePlayer->active == TRUE){ stopWave(voicePlayer); }
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
void	PMVOICE_PlayVoiceByPlayer( u32 pokeNum )
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
BOOL	PMVOICE_CheckPlayByPlayer( void )
{
	return PMSND_CheckPlaySE();	//現状はプレーヤーが同じに設定してあるので同じ動作
}
#endif





