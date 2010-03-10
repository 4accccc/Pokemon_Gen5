//=============================================================================
/**
 *
 *	@file		snd_mic.c
 *	@brief  マイク関連モジュール
 *	@author	hosaka genya (GSから移植)
 *	@data		2009.09.10
 *
 */
//=============================================================================
#include "gflib.h"

#include "sound/snd_mic.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
#define SOUND_MIC_WAIT (3*30) ///< AMPをONにしてからマイクが使用可能になるまで

#define SWAVE_BUFFER_SIZE		(2000)						//2k * 8bit

// ペラップ
#define PERAP_SAMPLING_RATE		(2000)									//サンプリングレート
#define PERAP_SAMPLING_TIME		(1)										//サンプリングする時間
#define PERAP_SAMPLING_SIZE		(PERAP_SAMPLING_RATE * PERAP_SAMPLING_TIME)	//必要なデータ量


//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
/// マイク用ワーク
//==============================================================
typedef struct {
  // オートサンプリング用
  MICAutoParam  AutoParamBuff;
  BOOL          isAutoSampStart;
	u32           amp_init_flag:1;		// Amp設定フラグ
  u32           amp_wait_cnt:31;
  // ペラップ波形保存ワーク
  void*         p_PerapBuff;
} SND_MIC_WORK;

// シングルトン
static SND_MIC_WORK* sp_SndMic = NULL;

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static void MicCallback( MICResult /*result*/, void* /*arg*/ );
static void* Snd_GetWaveBufAdrs( void );

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  マイクモジュール 初期化
 *
 *	@param	HEAPID heap_id  ヒープID
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void SND_MIC_Init( HEAPID heap_id )
{
  SND_MIC_WORK* wk;

  wk = GFL_HEAP_AllocMemory( heap_id, sizeof( SND_MIC_WORK ) );
  GFL_STD_MemClear( wk, sizeof(SND_MIC_WORK) );
  
  //	ペラップバッファ作成
	{	
		u32 size  = sizeof(u8)*SWAVE_BUFFER_SIZE + 32;	//	後にバッファアドレスを32バイトアラインするため+32
		u8 *p_buf	= GFL_HEAP_AllocMemory( heap_id, size );
		MI_CpuClear8( p_buf, size );
		wk->p_PerapBuff = p_buf;
  }

  {
    //MIC_Init関数の初期化は、2回目以降の呼び出しは無効になるように、
    //内部フラグでチェックされています
    MIC_Init();								//MIC_API初期化
      
    //OS_Init()を呼ぶ場合は、その中から呼ばれますので必要ありません。
    //PM_Init();							//PMICマネージャ初期化(OS_INIT内でも呼ばれている)
    
    (void)PM_SetAmp( PM_AMP_ON );			//プログラマブルゲインアンプのスイッチON
    (void)PM_SetAmpGain( PM_AMPGAIN_80 );	//プログラマブルゲインアンプのゲインを設定
    
    //AMP初期化
    wk->amp_init_flag	= 1;
  }

  sp_SndMic = wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  マイクモジュール 開放
 *
 *	@param	void 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void SND_MIC_Exit( void )
{
  GF_ASSERT( sp_SndMic );

  GFL_HEAP_FreeMemory( sp_SndMic->p_PerapBuff );
  GFL_HEAP_FreeMemory( sp_SndMic );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  マイクモジュール 主処理
 *
 *	@param	void 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void SND_MIC_Main( void )
{
  SND_MIC_WORK* wk = sp_SndMic;

  GF_ASSERT( sp_SndMic );

	//3秒調べるためのカウント
	if( wk->amp_init_flag ) {
		if( !SND_MIC_IsAmpOnWaitFlag() ) {	
			wk->amp_wait_cnt++;
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	AMPをONにしてからのカウントをクリアーします
 *					基本的にこの関数は一部からしか使われないはずです
 *
 */
//-----------------------------------------------------------------------------
void SND_MIC_ClearAmpOnWait( void )
{	
  GF_ASSERT( sp_SndMic );

	sp_SndMic->amp_wait_cnt	= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	AMPをONにしてから3秒待ったかどうかのフラグ取得
 *					マイクはこれがONにならないと使用してはいけない（任天堂規約）
 *
 *	@retval	TRUEならば使用許可
 *	@retval	FALSEならば使用不許可
 */
//-----------------------------------------------------------------------------
BOOL SND_MIC_IsAmpOnWaitFlag( void )
{	
  GF_ASSERT( sp_SndMic );

	//3秒以上たったかどうか
  return sp_SndMic->amp_wait_cnt > SOUND_MIC_WAIT;
}

//--------------------------------------------------------------
/**
 * @brief	録音開始
 *
 * @param	p		MICAutoParam型のアドレス
 *
 * @retval	"MIC_RESULT_SUCCESS		処理が正常に完了"
 * @retval	"それ以外				何らかの原因で失敗"
 */
//--------------------------------------------------------------
MICResult SND_MIC_StartAutoSampling( MICAutoParam* p )
{
	MICResult ret;
  
  GF_ASSERT( sp_SndMic );

#ifdef SDK_TWL
	ret = MIC_StartLimitedSampling( p );
#else
	ret = MIC_StartAutoSampling( p );
#endif	// SDK_TWL

	if( ret != MIC_RESULT_SUCCESS ){
		OS_Printf( "StartMicAutoSamoling Failed ret=%d \n", ret );
	}

  // オートサンプリング開始
  sp_SndMic->isAutoSampStart = TRUE;
  sp_SndMic->AutoParamBuff  = *p;

	return ret;
}

//--------------------------------------------------------------
/**
 * @brief	録音停止
 *
 * @param	none
 *
 * @retval	"MIC_RESULT_SUCCESS		処理が正常に完了"
 * @retval	"それ以外				何らかの原因で失敗"
 */
//--------------------------------------------------------------
MICResult SND_MIC_StopAutoSampling(void)
{
	MICResult ret;
  
  GF_ASSERT( sp_SndMic );

  // オートサンプリング停止
  sp_SndMic->isAutoSampStart = FALSE;

#ifdef SDK_TWL
	ret = MIC_StopLimitedSampling();
#else
	ret = MIC_StopAutoSampling();
#endif	// SDK_TWL

	if( ret != MIC_RESULT_SUCCESS ){
		OS_Printf( "MicStopAutoSampling Failed ret=%d \n", ret );
	}

	return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief    マイクスリープ起動処理
 */
//-----------------------------------------------------------------------------
void SND_MIC_StopSleep(void)
{
  GF_ASSERT( sp_SndMic );
  
  if( sp_SndMic->isAutoSampStart )
  {
	  MICResult ret;
#ifdef SDK_TWL
		ret = MIC_StopLimitedSampling();
#else
	  ret = MIC_StopAutoSampling();
#endif	// SDK_TWL
    GF_ASSERT( ret == MIC_RESULT_SUCCESS );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief    マイクスリープ復帰処理
 */
//-----------------------------------------------------------------------------
void SND_MIC_ReStartSleep(void)
{
  GF_ASSERT( sp_SndMic );

  if( sp_SndMic->isAutoSampStart )
  {
	  MICResult ret;
#ifdef SDK_TWL
		ret = MIC_StartLimitedSampling( &sp_SndMic->AutoParamBuff );
#else
	  ret = MIC_StartAutoSampling( &sp_SndMic->AutoParamBuff );
#endif	// SDK_TWL
    GF_ASSERT( ret == MIC_RESULT_SUCCESS );
  }

  SND_MIC_ClearAmpOnWait();
}

//--------------------------------------------------------------
/**
 * @brief	手動録音
 * @param	type      サンプリングレートのタイプ
 * @param	heap      記録領域
 * @param	callback  記録した際のコールバック
 * @param	arg       コールバックに渡す引数ポインタ
 * @retval	"MIC_RESULT_SUCCESS		処理が正常に完了"
 * @retval	"それ以外				何らかの原因で失敗"
 */
//--------------------------------------------------------------
MICResult SND_MIC_ManualSampling(MICSamplingType type ,void* heap,MICCallback callback,void* arg)
{
	MICResult ret;

  ret = MIC_DoSamplingAsync( type, heap, callback, arg);

	if( ret != MIC_RESULT_SUCCESS ){
        OS_Printf( "マイク手動サンプリングが失敗\n" );
	}
	return ret;
}

//--------------------------------------------------------------
/**
 * @brief	ペラップデータ録音開始(サウンドシステムの領域に一時保存する)
 *
 * @param	none
 *
 * @retval	"MIC_RESULT_SUCCESS		処理が正常に完了"
 * @retval	"それ以外				何らかの原因で失敗"
 */
//--------------------------------------------------------------
MICResult SND_PERAP_VoiceRecStart( void * cbFunc, void * cbWork )
{
	MICAutoParam mic;	//マイクパラメータ

	//波形再生用チャンネルを確保する
	//Snd_WaveOutAllocChannel( WAVEOUT_CH_NORMAL );

	mic.type			= MIC_SAMPLING_TYPE_SIGNED_8BIT;	//サンプリング種別

	//バッファは32バイトアラインされたアドレスでないとダメ！
	mic.buffer		= Snd_GetWaveBufAdrs();
	mic.size			= PERAP_SAMPLING_SIZE;

	if( (mic.size&0x1f) != 0 ){
		mic.size &= 0xffffffe0;
	}

	//代表的なサンプリングレートをARM7のタイマー周期に換算した値の定義
#ifdef SDK_TWL
	mic.rate = MIC_SAMPLING_RATE_8180;
#else
	mic.rate			= MIC_SAMPLING_RATE_8K;
//	mic.rate			= HW_CPU_CLOCK_ARM7 / PERAP_SAMPLING_RATE;
#endif	// SDK_TWL

	//連続サンプリング時にバッファをループさせるフラグ
	mic.loop_enable		= FALSE;

	//バッファが飽和した際に呼び出すコールバック関数へのポインタ
	mic.full_callback	= cbFunc;

	//バッファが飽和した際に呼び出すコールバック関数へ渡す引数
	mic.full_arg		= cbWork;

	return SND_MIC_StartAutoSampling( &mic );		//録音開始
}

//--------------------------------------------------------------
/**
 * @brief	ペラップデータ録音停止
 *
 * @param	none
 *
 * @retval	"MIC_RESULT_SUCCESS		処理が正常に完了"
 * @retval	"それ以外				何らかの原因で失敗"
 */
//--------------------------------------------------------------
MICResult SND_PERAP_VoiceRecStop( void )
{
	return SND_MIC_StopAutoSampling();
}

//--------------------------------------------------------------
/**
 * @brief	録音したデータをセーブデータにセット
 *
 * @param	none
 *
 * @retval	none
 */
//--------------------------------------------------------------
void SND_PERAP_VoiceDataSave( PERAPVOICE* perap )
{
	PERAPVOICE_SetVoiceData( perap, (const s8*)Snd_GetWaveBufAdrs() );
}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================

//--------------------------------------------------------------
/**
 * @brief	マイクコールバック関数
 *
 * @param	none
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void MicCallback( MICResult /*result*/, void* /*arg*/ )
{
    OS_Printf( "Mic Callback Done\n" );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  波形保存バッファへのアクセサ
 *
 *	@param	none
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void* Snd_GetWaveBufAdrs( void )
{
  u8* p_buf = sp_SndMic->p_PerapBuff;

  GF_ASSERT( sp_SndMic );

  return (void*)MATH_ROUNDUP32( (int)(p_buf) );
}
