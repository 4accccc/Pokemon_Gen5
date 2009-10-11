//======================================================================
/**
 * @file	jpeg_encoder.c
 * @brief	RGB555形式の画像をJpegにエンコード・デコードする
 * @author	ariizumi
 * @data	09/05/22
 *
 * モジュール名：JPG_ENC
 */
//======================================================================
#include <twl/ssp/ARM9/jpegenc.h>
#include <twl/ssp/ARM9/jpegdec.h>
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "test/ariizumi/jpeg_encoder.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define JPG_ENC_STACK_SIZE (1024)
#define JPG_ENC_THREAD_PRIO (20)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

struct _JPG_ENC_WORK
{
  HEAPID heapId;
  OSThread thread;
  
  void *threadStack;
  
  BOOL isSleep;
  //エンコード用
  BOOL  reqEncode;
  BOOL  isEncode;
  JPG_ENC_EncodeCallBack encCallBack;
  void  *encUserWork;
  void  *encodeSrc;
  u8    *encodeDst;
  u32   encodeDstSize;
  void  *encodeWorkBuff;
  u16   encodeSizeX; 
  u16   encodeSizeY;
  u32   encodeQuality;
  u32   encodeSampling;
  u32   encodeOption;
  
  //デコード用
  

};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void JPG_ENC_ThreadFunc( void *arg );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------

JPG_ENC_WORK* JPG_ENC_InitSystem( HEAPID heapId )
{
  JPG_ENC_WORK *work = GFL_HEAP_AllocMemory( heapId , sizeof( JPG_ENC_WORK ));
  
  work->isSleep = TRUE;
  //4バイトアライメントだけど無いから32で
  work->threadStack = GFL_NET_Align32Alloc( heapId , JPG_ENC_STACK_SIZE );
  OS_CreateThread( &work->thread ,
                   JPG_ENC_ThreadFunc ,
                   work ,
                   (void*)((u32)work->threadStack + JPG_ENC_STACK_SIZE) ,
                   JPG_ENC_STACK_SIZE ,
                   JPG_ENC_THREAD_PRIO );
  return work;
}

void JPG_ENC_ExitSystem( JPG_ENC_WORK* work )
{
  GF_ASSERT_MSG( work->isSleep == FALSE , "JpegEncoder Thread is active!!\n");

  OS_KillThread( &work->thread , NULL );
  
  GFL_NET_Align32Free( work->threadStack );
  GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
//	エンコード系
//--------------------------------------------------------------

const u32 JPG_ENC_GetEncodeWorkSize( const u16 sizeX , const u16 sizeY , u32 sampling , u32 option )
{
  return SSP_GetJpegEncoderBufferSize( sizeX , sizeY , sampling , option );
}

void JPG_ENC_StartEncode( JPG_ENC_WORK* work , void *src , u8 *dst , u32 dstSize , void *workBuff , 
                          const u16 sizeX , const u16 sizeY , u32 quality, 
                          u32 sampling , u32 option , JPG_ENC_EncodeCallBack callBack , void* userWork )
{
  work->encCallBack = callBack;
  work->encUserWork = userWork;
  
  work->encodeSrc       = src;
  work->encodeDst       = dst;
  work->encodeDstSize   = dstSize;
  work->encodeWorkBuff  = workBuff;
  work->encodeSizeX     = sizeX;
  work->encodeSizeY     = sizeY;
  work->encodeQuality   = quality;
  work->encodeSampling  = sampling;
  work->encodeOption    = option;
  
  work->isEncode  = TRUE;
  OS_TPrintf("JPEG ENCODER:req encord\n");
  if( work->isSleep == TRUE )
  {
    //寝てたら起こす
    OS_WakeupThreadDirect( &work->thread );
  }
}

const BOOL  JPG_ENC_IsEncode( const JPG_ENC_WORK* work )
{
  return work->isEncode;
}


//--------------------------------------------------------------
//	スレッド関数
//--------------------------------------------------------------

static void JPG_ENC_ThreadFunc( void *arg )
{
  JPG_ENC_WORK *work = (JPG_ENC_WORK*)arg;
  while( TRUE )
  {
    work->isSleep = FALSE;
    
    if( work->isEncode == TRUE )
    {
      u32 retSize;
      OS_TPrintf("JPEG ENCODER:start encord\n");
      retSize = SSP_StartJpegEncoder( work->encodeSrc ,
                                      work->encodeDst ,
                                      work->encodeDstSize ,
                                      work->encodeWorkBuff ,
                                      work->encodeSizeX ,
                                      work->encodeSizeY ,
                                      work->encodeQuality ,
                                      work->encodeSampling ,
                                      work->encodeOption );
      if( work->encCallBack != NULL )
      {
        work->encCallBack( work->encUserWork , work->encodeDst , retSize );
      }
      OS_TPrintf("JPEG ENCODER:finish encord\n");
      work->isEncode = FALSE;
    }
    
    if( work->isEncode == FALSE )
    {
      //リクエストが無ければ寝かす
      work->isSleep = TRUE;
      OS_SleepThread( NULL );
    }
  }
}

