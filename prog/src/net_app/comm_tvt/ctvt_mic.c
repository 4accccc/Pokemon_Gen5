//======================================================================
/**
 * @file	ctvt_mic.c
 * @brief	通信TVTシステム：マイク関係
 * @author	ariizumi
 * @data	09/12/19
 *
 * モジュール名：CTVT_MIC
 */
//======================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/camera_system.h"
#include "net/network_define.h"
#include "app/app_menu_common.h"

#include "ctvt_mic.h"
#include "enc_adpcm.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

// マイクゲインを設定
#define CTVT_MIC_Print(...) (void)((OS_TPrintf(__VA_ARGS__)))
//#define CTVT_MIC_Print(...)  ((void)0)


#define CTVT_MIC_CUT_SIZE (0x1000)  //ノイズが載るから頭を切る分
#define CTVT_MIC_SAMPLING_SIZE (CTVT_SEND_WAVE_SIZE+CTVT_MIC_CUT_SIZE)


#define CTVT_MIC_AMPGAIN (PM_AMPGAIN_160)
#ifdef SDK_TWL
#define CTVT_MIC_SAMPLING_RATE (MIC_SAMPLING_RATE_8180)
#else
#define CTVT_MIC_SAMPLING_RATE (MIC_SAMPLING_RATE_8K)
#endif
#define CTVT_MIC_WAVEOUT_CH	(7)							//波形で使用するチャンネルNO
#define CTVT_MIC_WAVEOUT_PLAY_SPD (32768)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _CTVT_MIC_WORK
{
  u32 recSize;
  void *recBuffer;
  
  BOOL isRecord;
  BOOL isPlayWave;
  
  NNSSndWaveOutHandle waveHandle;
};

//======================================================================
//	proto
//======================================================================
static void CTVT_MIC_PlayWaveInit( CTVT_MIC_WORK *micWork , const HEAPID heapId );
static void CTVT_MIC_PlayWaveTerm( CTVT_MIC_WORK *micWork );
static void CTVT_MIC_PlayWaveMain( CTVT_MIC_WORK *micWork );
static void CTVT_MIC_BufferEndCallBack(MICResult	result, void*	arg );

//--------------------------------------------------------------
// マイク初期化
//--------------------------------------------------------------
CTVT_MIC_WORK* CTVT_MIC_Init( const HEAPID heapId )
{
  CTVT_MIC_WORK *micWork = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_MIC_WORK) );
  
  MIC_Init();
  PM_Init();
  {
    const u32 ret = PM_SetAmp(PM_AMP_ON);
    if( ret == PM_RESULT_SUCCESS )
    {
      CTVT_MIC_Print("AMPをオンにしました。\n");
    }
    else
    {
      CTVT_MIC_Print("AMPの初期化に失敗（%d）", ret);
    }
  }
#if defined(SDK_TWL)
  {
    
    const u32 ret = PM_SetAmpGainLevel(80);
    //const u32 ret = PM_SetAmpGain(CTVT_MIC_AMPGAIN);
    if( ret == PM_RESULT_SUCCESS )
    {
      CTVT_MIC_Print("AMPのゲインを設定しました。\n");
    }
    else
    {
      CTVT_MIC_Print("AMPのゲイン設定に失敗（%d）", ret);
    }
  }
  SNDEX_Init();
  {
    static SNDEXIirFilterParam FilterParam =
      { 0x7E46, 0x81BA, 0x7E46, 0x7E43, 0x836E};     // 低周波成分を減衰させる
    const u32 ret = SNDEX_SetIirFilter(SNDEX_IIR_FILTER_ADC_1, &FilterParam);
    if ( ret == SNDEX_RESULT_SUCCESS)
    {
      CTVT_MIC_Print("マイクのフィルターを設定しました。\n");
    }
    else
    {
      CTVT_MIC_Print("マイクのフィルターの設定に失敗。\n");
    }
    
  }
#endif //defined(SDK_TWL)
  
  CTVT_MIC_PlayWaveInit( micWork , heapId );
  
  micWork->recBuffer = GFL_NET_Align32Alloc(heapId ,CTVT_MIC_SAMPLING_SIZE);
  micWork->recSize = 0;
  
  micWork->isRecord = FALSE;
  return micWork;
}

//--------------------------------------------------------------
// マイク開放
//--------------------------------------------------------------
void CTVT_MIC_Term( CTVT_MIC_WORK *micWork )
{
  CTVT_MIC_PlayWaveTerm( micWork );

  GFL_NET_Align32Free( micWork->recBuffer );
  GFL_HEAP_FreeMemory( micWork );
}

//--------------------------------------------------------------
// マイク更新
//--------------------------------------------------------------
void CTVT_MIC_Main( CTVT_MIC_WORK *micWork )
{
  if( micWork->isRecord == TRUE )
  {
    const u32 lastAdr = (u32)MIC_GetLastSamplingAddress();
    if( lastAdr == 0 )
    {
      micWork->recSize = 0;
    }
    else
    {
      micWork->recSize = lastAdr-(u32)micWork->recBuffer+4;
      if( micWork->recSize > CTVT_MIC_SAMPLING_SIZE )
      {
        micWork->recSize = CTVT_MIC_SAMPLING_SIZE;
      }
    }
  }
  CTVT_MIC_PlayWaveMain( micWork );

}

//--------------------------------------------------------------
// マイク録音開始
//--------------------------------------------------------------
const BOOL CTVT_MIC_StartRecord( CTVT_MIC_WORK *micWork )
{
  MICResult ret;
  
  MICAutoParam mic;	//マイクパラメータ
  mic.type			= MIC_SAMPLING_TYPE_SIGNED_12BIT;	//サンプリング種別
  //バッファは32バイトアラインされたアドレスでないとダメ！
  mic.size = CTVT_MIC_SAMPLING_SIZE;
  mic.buffer = micWork->recBuffer;
  GFL_STD_MemClear32( (void*)micWork->recBuffer , CTVT_MIC_SAMPLING_SIZE );
  if( (mic.size&0x1f) != 0 ){
  	mic.size &= 0xffffffe0;
  }

  //連続サンプリング時にバッファをループさせるフラグ
  mic.loop_enable		= FALSE;
  //バッファが飽和した際に呼び出すコールバック関数へのポインタ
  mic.full_callback	= CTVT_MIC_BufferEndCallBack;
  //バッファが飽和した際に呼び出すコールバック関数へ渡す引数
  mic.full_arg		= micWork;

  //代表的なサンプリングレートをARM7のタイマー周期に換算した値の定義
  mic.rate = CTVT_MIC_SAMPLING_RATE;
#ifdef SDK_TWL
  ret = MIC_StartLimitedSampling( &mic );
#else
  ret = MIC_StartAutoSampling( &mic );
#endif
    
  
  if( ret == MIC_RESULT_SUCCESS )
  {
#ifdef SDK_TWL
    CTVT_MIC_Print( "CommTVTMic StartRecording(TWL)\n" );
#else
    CTVT_MIC_Print( "CommTVTMic StartRecording(NTR)\n" );
#endif
    micWork->isRecord = TRUE;
    micWork->recSize = 0;
    return TRUE;
  }
  else
  {
    CTVT_MIC_Print( "CommTVTMic StartRecording faile[%d]!!!\n",ret );
    
    return FALSE;
  }
}

//--------------------------------------------------------------
// マイクバッファ終了CB
//--------------------------------------------------------------
static void CTVT_MIC_BufferEndCallBack(MICResult	result, void*	arg )
{
  CTVT_MIC_WORK *micWork = arg;
  micWork->isRecord = FALSE;
  micWork->recSize = CTVT_MIC_SAMPLING_SIZE;
  CTVT_MIC_Print( "CommTVTMic StopRecording(Buff is full)\n" );
}

//--------------------------------------------------------------
// マイク録音停止
//--------------------------------------------------------------
const BOOL CTVT_MIC_StopRecord( CTVT_MIC_WORK *micWork )
{
  if( micWork->isRecord == FALSE )
  {
    return TRUE;
  }
  else
  {
    MICResult ret;
#ifdef SDK_TWL
    ret = MIC_StopLimitedSampling();
#else
    ret = MIC_StopAutoSampling();
#endif
    if( ret == MIC_RESULT_SUCCESS )
    {
      micWork->recSize = (u32)MIC_GetLastSamplingAddress()-(u32)micWork->recBuffer+4;
      if( micWork->recSize > CTVT_MIC_SAMPLING_SIZE )
      {
        micWork->recSize = CTVT_MIC_SAMPLING_SIZE;
      }
      
      CTVT_MIC_Print( "CommTVTMic StopRecording buffSize[%x]\n",micWork->recSize );
      micWork->isRecord = FALSE;
      return TRUE;
    }
    else
    {
      CTVT_MIC_Print( "CommTVTMic StopRecording faile[%d]!!!\n",ret );
      return FALSE;
    }
  }
}

//--------------------------------------------------------------
// マイク録音中
//--------------------------------------------------------------
const BOOL CTVT_MIC_IsRecording( CTVT_MIC_WORK *micWork )
{
  return micWork->isRecord;
}

//--------------------------------------------------------------
// マイク録音サイズ取得
//--------------------------------------------------------------
const u32 CTVT_MIC_GetRecSize( CTVT_MIC_WORK *micWork )
{
  if( micWork->recSize > CTVT_MIC_CUT_SIZE )
  {
    return micWork->recSize - CTVT_MIC_CUT_SIZE;
  }
  else
  {
    return 0;
  }
}

//--------------------------------------------------------------
// マイクバッファ取得
//--------------------------------------------------------------
void* CTVT_MIC_GetRecBuffer( CTVT_MIC_WORK *micWork )
{
  return (void*)((u32)micWork->recBuffer+CTVT_MIC_CUT_SIZE);
}

//--------------------------------------------------------------
// エンコード
//--------------------------------------------------------------
const u32 CTVT_MIC_EncodeData( CTVT_MIC_WORK *micWork , void* decData , void *encData , const u32 dataSize )
{
  /*
  int i;
  s8 *decDataWork = decData;
  s8 *encDataWork = encData;
  
  GFL_STD_MemClear32( (void*)encDataWork , dataSize/2 );
  
  for( i=0;i<dataSize/2;i++ )
  {
    encDataWork[i] = (decDataWork[i*2]+decDataWork[(i*2)+1])>>1;

  }
  */
  
  u32 size;
  
  GFL_STD_MemClear( encData , dataSize );
  
  size = ENC_ADPCM_EncodeData( decData , dataSize , encData );
  
  return size;
}

//--------------------------------------------------------------
// デコード
//--------------------------------------------------------------
const u32 CTVT_MIC_DecodeData( CTVT_MIC_WORK *micWork , void* encData , void *decData , const u32 dataSize )
{
  /*
  int i;
  s8 *decDataWork = decData;
  s8 *encDataWork = encData;
  s8 befVal = 0;

  GFL_STD_MemClear32( (void*)decDataWork , dataSize/2 );
  
  for( i=0;i<dataSize/2;i++ )
  {
    const int baseIdx = i*2;
    const s8 ofs = (encDataWork[i] - befVal)>>1;
    decDataWork[baseIdx  ] = encDataWork[i];
    decDataWork[baseIdx+1] = encDataWork[i]+ofs;
    //recBuffer[baseIdx+2] = workData[i]+ofs*2;
    //recBuffer[baseIdx+3] = workData[i]+ofs*3;
  }
  */
  u32 size;
  GFL_STD_MemClear32( decData , dataSize*4 );

  size = ENC_ADPCM_DecodeData( encData , dataSize , decData );
  //GFL_STD_MemCopy( encData , decData , dataSize );

  return size;
}

//--------------------------------------------------------------
// 再生初期化
//--------------------------------------------------------------
static void CTVT_MIC_PlayWaveInit( CTVT_MIC_WORK *micWork , const HEAPID heapId )
{
  micWork->waveHandle = NNS_SndWaveOutAllocChannel( CTVT_MIC_WAVEOUT_CH );
  GF_ASSERT_MSG( micWork->waveHandle != NNS_SND_WAVEOUT_INVALID_HANDLE , "Waveハンドルの確保に失敗！！\n" );
  micWork->isPlayWave = TRUE;

}

//--------------------------------------------------------------
// 再生開放
//--------------------------------------------------------------
static void CTVT_MIC_PlayWaveTerm( CTVT_MIC_WORK *micWork )
{
  NNS_SndWaveOutFreeChannel( micWork->waveHandle );
}

//--------------------------------------------------------------
// 再生更新
//--------------------------------------------------------------
static void CTVT_MIC_PlayWaveMain( CTVT_MIC_WORK *micWork )
{
}

//--------------------------------------------------------------
// 再生
//--------------------------------------------------------------
const BOOL CTVT_MIC_PlayWave( CTVT_MIC_WORK *micWork , void *buffer , u32 size , u8 volume , int speed )
{
  const BOOL ret = NNS_SndWaveOutStart(
                    micWork->waveHandle ,
                    NNS_SND_WAVE_FORMAT_PCM16 ,
                    buffer ,
                    FALSE ,
                    0 ,
                    size /2,
                    (int)( HW_CPU_CLOCK_ARM7 / CTVT_MIC_SAMPLING_RATE ),
                    volume ,
                    speed ,
                    64 );
  micWork->isPlayWave = ret;
  return ret;
}

//--------------------------------------------------------------
// 停止
//--------------------------------------------------------------
void CTVT_MIC_StopWave( CTVT_MIC_WORK *micWork )
{
  micWork->isPlayWave = FALSE;
  NNS_SndWaveOutStop( micWork->waveHandle );
}

//--------------------------------------------------------------
// 再生中確認
//--------------------------------------------------------------
const BOOL CTVT_MIC_IsPlayWave( CTVT_MIC_WORK *micWork )
{
  return micWork->isPlayWave;
}

//--------------------------------------------------------------
// 終了取得
//--------------------------------------------------------------
const BOOL CTVT_MIC_IsFinishWave( CTVT_MIC_WORK *micWork )
{
  if( NNS_SndWaveOutIsPlaying( micWork->waveHandle ) == TRUE )
  {
    return FALSE;
  }
  return TRUE;
}
