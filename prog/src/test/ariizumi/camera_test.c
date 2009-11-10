//======================================================================
/**
 * @file  camera_test.c
 * @brief TWLのカメラテストパート
 * @author  ariizumi
 * @data    09/05/18
 *
 * モジュール名：CAM_TEST
 */
//======================================================================
#include <gflib.h>

#include <twl/ssp/ARM9/jpegenc.h>
#include <twl/ssp/ARM9/jpegdec.h>

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/net_err.h"
#include "system/palanm.h"
#include "net/network_define.h"

#include "arc_def.h"
#include "font/font.naix"

#include "debug/debugwin_sys.h"

#include "test/debug_pause.h"
#include "test/ariizumi/camera_test.h"
#include "test/ariizumi/camera_system.h"
#include "test/ariizumi/jpeg_encoder.h"


//======================================================================
#include "poke_tool/monsno_def.h"

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"

#ifdef SDK_TWL

//======================================================================
//  define
//======================================================================
#pragma mark [> define

#define CAM_TEST_CAP_SIZE_X (128)
#define CAM_TEST_CAP_SIZE_Y (192)
#define CAM_TEST_CAP_PIXEL (CAM_TEST_CAP_SIZE_X*CAM_TEST_CAP_SIZE_Y)

//JPG変換係
#define JPG_OUT_TYPE (SSP_JPEG_OUTPUT_YUV444)
#define JPG_OUT_OPT  (SSP_JPEG_RGB555)

//#define CAMERA_COMM_Print(...) (void)((OS_TPrintf(__VA_ARGS__)))
#define CAMERA_COMM_Print(...)  ((void)0)

#define CAMERA_TEST_AVE_COUNT (10)
//======================================================================
//  enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  CTCS_NONE,
  CTCS_PARENT,
  CTCS_CHILD,
  CTCS_CONNECT,
  CTCS_WAIT_NEGOTIATION,
  CTCS_WAIT_TIMMING,
  
  CTCS_ENCODE_DATA,
  CTCS_WAIT_ENCODE_DATA,
  CTCS_SEND_DATA,
  CTCS_WAIT_POST,

  CTCS_WAIT_BATTLE,
  
}CAMERA_TEST_COMM_STATE;

//通信の送信タイプ
typedef enum
{
  CTSM_SEND_PHOTO = GFL_NET_CMD_CAMERA_TEST,
  CTSM_SEND_FLG,
  
  CTSM_MAX,
}CAMERA_TEST_SEND_MODE;

//フラグ送信の種類
typedef enum
{
  CTCF_POST_DATA,
  CTCF_TEST_FLG,

  CTCF_MAX,
}CAMERA_TEST_COMM_FLG;

//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct

typedef struct _COMM_TVT_MIC_WORK COMM_TVT_MIC_WORK;

typedef struct
{
  u32 dummy;
}CAMERA_TEST_BEACON;

typedef struct
{
  HEAPID heapId;
  CAMERA_SYSTEM_WORK *camSys;
  
  u32 wramAdrB;
  u32 wramAdrC;
  u32 picBuffAdr;
  u32 jpgConvBuffAdr;
  
  u8  capReq;
  void *capBuff[2];
  void *encBuff;
  void *sendBuff;
  void *postBuff;
  BOOL  isUpdateUpper[2];
  
  //通信用
  CAMERA_TEST_COMM_STATE commState;
  CAMERA_TEST_BEACON beacon;
  u16   selfId;
  BOOL  isInitComm;
  BOOL  isConnect;
  BOOL  isPost;
  u16   sendSize;
  u16   postSize;
  
  //デバグ用
  u8    jpgQuality;
  GFL_FONT *fontHandle;
  u16       trimSizeX;
  u16       trimSizeY;
  
  BOOL      isDouble;
  
  u32       sizeAve[CAMERA_TEST_AVE_COUNT];
  u32       sendSecAve[CAMERA_TEST_AVE_COUNT];
  u32       encTimeAve[CAMERA_TEST_AVE_COUNT];
  u8        sizeCntIdx;
  u8        sendSecCntIdx;
  u8        encTimeCntIdx;
  u32       sizeMax;
  u32       sendSecMax;
  u32       encTimeMax;  

  COMM_TVT_MIC_WORK *micWork;
}CAM_TEST_WORK;


//======================================================================
//  proto
//======================================================================
#pragma mark [> proto
static GFL_PROC_RESULT CAM_TEST_InitProc(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT CAM_TEST_ExitProc(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT CAM_TEST_MainProc(GFL_PROC * proc, int * seq, void * pwk, void * mywk);

static void CAM_TEST_InitGraphic( CAM_TEST_WORK *work );
static void CAM_TEST_CapCallBack( void* captureArea , void* userWork );
static void CAM_TEST_EncodeFinishCallBack( void *userWork , u8* jpgData , u32 jpgSize);

static void CAM_TEST_InitComm( CAM_TEST_WORK *work );
static void CAM_TEST_ExitComm( CAM_TEST_WORK *work );
static void CAM_TEST_UpdateComm( CAM_TEST_WORK *work );

extern COMM_TVT_MIC_WORK* COMM_TVT_MIC_Init( CAM_TEST_WORK *work );
extern void COMM_TVT_MIC_Term( CAM_TEST_WORK *work , COMM_TVT_MIC_WORK *micWork );
extern void COMM_TVT_MIC_Main( CAM_TEST_WORK *work , COMM_TVT_MIC_WORK *micWork );
extern const BOOL COMM_TVT_MIC_StartRecord( CAM_TEST_WORK *work , COMM_TVT_MIC_WORK *micWork );
extern const BOOL COMM_TVT_MIC_StopRecord( CAM_TEST_WORK *work , COMM_TVT_MIC_WORK *micWork );
extern const BOOL COMM_TVT_MIC_PlayVoice( CAM_TEST_WORK *work , COMM_TVT_MIC_WORK *micWork , void *buffer , u32 buffSize );


#pragma mark [> comm proc

static const BOOL CAM_TEST_Send_PhotoData( CAM_TEST_WORK *work );
static void CAM_TEST_Post_PhotoData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );
static u8*    CAM_TEST_Post_PhotoData_Buff( int netID, void* pWork , int size );

static void CAM_TEST_SendFlg( CAM_TEST_WORK *work , const u8 flg , const u8 value );
static void CAM_TEST_PostFlg( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle );

static void CAM_TEST_FinishInitCallback( void* pWork );
static void CAM_TEST_FinishTermCallback( void* pWork );
static void*  CAM_TEST_GetBeaconData(void* pWork);
static int CAM_TEST_GetBeaconSize(void *pWork);
static BOOL CAM_TEST_BeacomCompare(GameServiceID GameServiceID1, GameServiceID GameServiceID2);
static void CAM_TEST_ConnectCallBack(void* pWork);

#pragma mark [> debug prot

static u32 CAM_TEST_CalcAve( u32 *dataArr , u8 *nowIdx , u32 newVal );

static void CAM_TEST_InitDebugMenu( CAM_TEST_WORK *work );
static void CAM_TEST_ExitDebugMenu( CAM_TEST_WORK *work );

extern const NetRecvFuncTable BtlRecvFuncTable[];

const GFL_PROC_DATA CameraTastMainProcData = {
  CAM_TEST_InitProc,
  CAM_TEST_MainProc,
  CAM_TEST_ExitProc,
};

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_256_AB,             // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,       // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,         // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_16_F,            // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,      // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_128_D,        // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_NONE,             // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,         // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_32K,
  GX_OBJVRAMMODE_CHAR_1D_32K
};

//--------------------------------------------------------------
//  
//--------------------------------------------------------------

static CAM_TEST_WORK *camWork = NULL;
//------------------------------------------------------------------
//  デバッグ用初期化関数
//------------------------------------------------------------------
static GFL_PROC_RESULT CAM_TEST_InitProc(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  CAM_TEST_WORK *work;
  if( OS_IsRunOnTwl() == FALSE )
  {
    OS_TPrintf("Camera test is TWLmode only!!\n");
    return GFL_PROC_RES_FINISH;
  }
//  SCFG_SetDmacFixed( TRUE );
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ARIIZUMI_DEBUG, 0x50000 );
  
  work = GFL_PROC_AllocWork( proc , sizeof( CAM_TEST_WORK ) , HEAPID_ARIIZUMI_DEBUG);
  camWork = work;
  work->heapId = HEAPID_ARIIZUMI_DEBUG;
	(void)MI_FreeWram_B( MI_WRAM_ARM9 );
	(void)MI_CancelWram_B( MI_WRAM_ARM9 );
  work->wramAdrB = MI_AllocWram( MI_WRAM_B , MI_WRAM_SIZE_256KB , MI_WRAM_ARM9 );
	(void)MI_FreeWram_C( MI_WRAM_ARM9 );
	(void)MI_CancelWram_C( MI_WRAM_ARM9 );
  work->wramAdrC = MI_AllocWram( MI_WRAM_C , MI_WRAM_SIZE_256KB , MI_WRAM_ARM9 );
  GF_ASSERT( work->wramAdrC != 0 );
   
  GFL_BG_Init( work->heapId );
  CAM_TEST_InitGraphic( work );
  
  work->camSys = CAMERA_SYS_InitSystem( work->heapId );
  
  CAMERA_SYS_CreateReadBuffer( work->camSys , 2 , (HEAPID_ARIIZUMI_DEBUG | HEAPDIR_MASK) );
  {
    const u32 bufSize = CAMERA_SYS_GetBufferSize( work->camSys , 2 );
//    CAMERA_SYS_SetReadBuffer( work->camSys , (void*)work->wramAdrB , 2 );
    work->picBuffAdr = work->wramAdrB+bufSize;
  }
  CAMERA_SYS_SetCaptureCallBack( work->camSys , CAM_TEST_CapCallBack , work );
  
  GFL_STD_MemClear( G2S_GetBG3ScrPtr() , 256*192*sizeof(u16) );
  GFL_STD_MemClear( G2_GetBG3ScrPtr() , 256*192*sizeof(u16) );
  
  work->capReq = 0;
  work->encBuff    = (void*)(work->picBuffAdr);
  work->capBuff[0] = (void*)(work->wramAdrC);
  work->capBuff[1] = (void*)(work->wramAdrC + CAM_TEST_CAP_PIXEL*sizeof(u16));
  work->sendBuff   = (void*)(work->wramAdrC + CAM_TEST_CAP_PIXEL*sizeof(u16)*2);
  work->postBuff   = (void*)(work->wramAdrC + CAM_TEST_CAP_PIXEL*sizeof(u16)*3);
  work->jpgConvBuffAdr = work->wramAdrC + CAM_TEST_CAP_PIXEL*sizeof(u16)*4;
  GFL_STD_MemClear( (void*)work->wramAdrB , 0x40000 );
  GFL_STD_MemClear( (void*)work->wramAdrC , 0x40000 );
  CAMERA_COMM_Print("[%x][leastSize:%x]\n",work->wramAdrB,((u32)work->encBuff+CAM_TEST_CAP_PIXEL*sizeof(u16))-(u32)work->wramAdrB );
  CAMERA_COMM_Print("[%x][leastSize:%x]\n",work->wramAdrC,((u32)work->jpgConvBuffAdr+CAM_TEST_CAP_PIXEL*sizeof(u16))-(u32)work->wramAdrC );
  CAMERA_COMM_Print("[%x]\n",work->wramAdrC);
/*
  work->capBuff[0] = GFL_HEAP_AllocMemory( work->heapId , CAM_TEST_CAP_PIXEL*sizeof(u16) );
  work->capBuff[1] = GFL_HEAP_AllocMemory( work->heapId , CAM_TEST_CAP_PIXEL*sizeof(u16) );
  work->sendBuff   = GFL_HEAP_AllocMemory( work->heapId , CAM_TEST_CAP_PIXEL*sizeof(u16) );
  work->postBuff   = GFL_HEAP_AllocMemory( work->heapId , CAM_TEST_CAP_PIXEL*sizeof(u16) );
  GFL_STD_MemClear( work->capBuff[0] , CAM_TEST_CAP_PIXEL*sizeof(u16) );
  GFL_STD_MemClear( work->capBuff[1] , CAM_TEST_CAP_PIXEL*sizeof(u16) );
  GFL_STD_MemClear( work->sendBuff   , CAM_TEST_CAP_PIXEL*sizeof(u16) );
  GFL_STD_MemClear( work->postBuff   , CAM_TEST_CAP_PIXEL*sizeof(u16) );
*/
  work->isUpdateUpper[0] = FALSE;
  work->isUpdateUpper[1] = FALSE;
  
  CAM_TEST_InitComm( work );
  
  CAM_TEST_InitDebugMenu( work );
  
  CAMERA_SYS_StartCapture( work->camSys );
  work->capReq = 1;
  work->isDouble = FALSE;
  CAMERA_COMM_Print("[%x]\n",GFL_HEAP_GetHeapFreeSize( HEAPID_ARIIZUMI_DEBUG ));
  
  {
    u8 i;
    for( i=0;i<CAMERA_TEST_AVE_COUNT;i++ )
    {
      work->sizeAve[i] = 0;
      work->sendSecAve[i] = 0;
      work->encTimeAve[i] = 0;
    }
    work->sizeMax = 0;
    work->sendSecMax = 0;
    work->encTimeMax = 0;
    work->sizeCntIdx = 0;
    work->sendSecCntIdx = 0;
    work->encTimeCntIdx = 0;
  }
  
  work->micWork = COMM_TVT_MIC_Init( work ); 
  
  return GFL_PROC_RES_FINISH;
}


//------------------------------------------------------------------
/**  デバッグ用Exit
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT CAM_TEST_ExitProc(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  CAM_TEST_WORK *work = mywk;
  if( OS_IsRunOnTwl() == FALSE )
  {
    OS_TPrintf("Camera test is TWLmode only!!\n");
    return GFL_PROC_RES_FINISH;
  }
  
  if( work->isInitComm != FALSE )
  {
    CAM_TEST_ExitComm( work );
    return GFL_PROC_RES_CONTINUE;
  }
  CAM_TEST_ExitDebugMenu( work );

  COMM_TVT_MIC_Term( work , work->micWork );
  CAMERA_SYS_ExitSystem( work->camSys );

  GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
  GFL_BG_Exit();

//  GFL_HEAP_FreeMemory( work->postBuff );
//  GFL_HEAP_FreeMemory( work->sendBuff );
//  GFL_HEAP_FreeMemory( work->capBuff[0] );
//  GFL_HEAP_FreeMemory( work->capBuff[1] );
  GFL_PROC_FreeWork( proc );

  MI_FreeWram( MI_WRAM_A , MI_WRAM_SIZE_256KB );

  camWork = NULL;
  GFL_HEAP_DeleteHeap( HEAPID_ARIIZUMI_DEBUG );
  
  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**   デバッグ用メイン
 */  
//------------------------------------------------------------------
static GFL_PROC_RESULT CAM_TEST_MainProc(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  CAM_TEST_WORK *work = mywk;
  if( OS_IsRunOnTwl() == FALSE )
  {
    OS_TPrintf("Camera test is TWLmode only!!\n");
    return GFL_PROC_RES_FINISH;
  }
  
  CAM_TEST_UpdateComm( work );
/*
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    CAMERA_SYS_StartCapture( work->camSys );
  }
  else
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
  {
    CAMERA_SYS_StopCapture( work->camSys );
  }

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
  {
    if( work->isInitComm == TRUE &&
        work->commState == CTCS_NONE )
    {
      GFL_NET_InitServer();
      work->commState = CTCS_PARENT;
    }
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
  {
    if( work->isInitComm == TRUE &&
        work->commState == CTCS_NONE )
    {
      GFL_NET_StartBeaconScan();
      work->commState = CTCS_CHILD;
    }
  }
  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
  {
    MtxFx22 mtx;
    work->isDouble = !work->isDouble;
    GFL_STD_MemClear( G2_GetBG3ScrPtr() , 256*192*2 );
    
    if( work->isDouble == TRUE )
    {
      mtx._00 = FX_Inv(FX32_ONE*2);
      mtx._11 = FX_Inv(FX32_ONE*2);
    }
    else
    {
      mtx._00 = FX_Inv(FX32_ONE);
      mtx._11 = FX_Inv(FX32_ONE);
    }
    mtx._01 = 0;
    mtx._10 = 0;
    G2_SetBG3Affine(&mtx,0,0,0,0);
  }
*/
  
  //本当はVBlankで
  if( work->isUpdateUpper[0] == TRUE )
  {
    u8 i;
    const u32 sizeX = (work->isDouble?CAM_TEST_CAP_SIZE_X/2:CAM_TEST_CAP_SIZE_X);
    const u32 sizeY = (work->isDouble?CAM_TEST_CAP_SIZE_Y/2:CAM_TEST_CAP_SIZE_Y);
    const u32 scrHeight = (work->isDouble?192/2:192);
    const u32 scrWidth  = (work->isDouble?128/2:128);
    
    const u32 topAdr = (scrHeight-sizeY)/2*256*sizeof(u16);
    const u32 leftAdr1 = (scrWidth-sizeX)/2*sizeof(u16);
    const u32 leftAdr2 = (scrWidth+(scrWidth-sizeX)/2)*sizeof(u16);
    for( i=0;i<sizeY;i++ )
    {
      GX_LoadBG3Bmp( (void*)((u32)work->capBuff[0] + (i*sizeX*sizeof(u16))) ,
                     leftAdr1 + topAdr + i*256*sizeof(u16) ,
                     sizeX*sizeof(u16) );
    }
    work->isUpdateUpper[0] = FALSE;
    work->capReq = 1;
  }

  if( work->isUpdateUpper[1] == TRUE )
  {
    const u32 sizeX = (work->isDouble?CAM_TEST_CAP_SIZE_X/2:CAM_TEST_CAP_SIZE_X);
    const u32 sizeY = (work->isDouble?CAM_TEST_CAP_SIZE_Y/2:CAM_TEST_CAP_SIZE_Y);
    {
      s16 convWidth = sizeX;
      s16 convHeight= sizeY;
      SSP_StartJpegDecoder( work->postBuff ,
                            work->postSize ,
                            work->capBuff[1] ,
                            &convWidth ,
                            &convHeight ,
                            0 );
    }
    {
      u8 i;
      const u32 scrHeight = (work->isDouble?192/2:192);
      const u32 scrWidth  = (work->isDouble?128/2:128);
      const u32 topAdr = (scrHeight-sizeY)/2*256*sizeof(u16);
      const u32 leftAdr1 = (scrWidth-sizeX)/2*sizeof(u16);
      const u32 leftAdr2 = (scrWidth+(scrWidth-sizeX)/2)*sizeof(u16);
      for( i=0;i<sizeY;i++ )
      {
        GX_LoadBG3Bmp( (void*)((u32)work->capBuff[1] + (i*sizeX*sizeof(u16))) ,
                       leftAdr2 + topAdr + i*256*sizeof(u16) ,
                       sizeX*sizeof(u16) );
      }
      work->isUpdateUpper[1] = FALSE;
      CAM_TEST_SendFlg( work , CTCF_POST_DATA , 0 );
    }
  }
  
  COMM_TVT_MIC_Main( work , work->micWork );

  if( (GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT) &&
      (GFL_UI_KEY_GetTrg() & PAD_BUTTON_START) )
  {
    return GFL_PROC_RES_FINISH;
  }


  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
//  グラフィック系初期化
//--------------------------------------------------------------

static void CAM_TEST_InitGraphic( CAM_TEST_WORK *work )
{

  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();
  GX_SetVisiblePlane( 0 );
  GXS_SetVisiblePlane( 0 );

  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
  GFL_DISP_SetBank( &vramBank );


  //デバッグウィンドウ用
  if( TRUE )
  {
    // BG1 MAIN (Infoバー
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000,0x6000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &header_main1, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( GFL_BG_FRAME1_M, TRUE );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M, TRUE );
    GFL_BG_ClearFrame( GFL_BG_FRAME1_M );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
    
  }
  //Vram割り当ての設定
  {
    //ダイレクトBMPにGFLが対応してないのでべた書き
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,GX_BGMODE_3,GX_BG0_AS_2D);
    GXS_SetGraphicsMode(GX_BGMODE_3);
    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x08000);
    G2S_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x08000);

    G2_SetBG3Priority(0);
    G2S_SetBG3Priority(0);

    GX_SetVisiblePlane(GX_PLANEMASK_BG1|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ);

  }

}

static void CAM_TEST_CapCallBack( void *captureArea , void *userWork )
{
  CAM_TEST_WORK *work = userWork;
  u16 i;
  const u16 width = CAMERA_SYS_CaptureSizeToWidth( work->camSys );
  const u16 height= CAMERA_SYS_CaptureSizeToHeight( work->camSys );
  for( i=0 ;i<height;i++ )
  {
    const u8 line = i+(96-(height/2));
    GXS_LoadBG3Bmp( (void*)((u32)captureArea + (i*width*sizeof(u16))), 
                    line*256*sizeof(u16) + sizeof(u16)*(128-(width/2)), 
                    width*sizeof(u16));
  }
  
  if( work->capReq != 0 )
  {
    const u32 sizeX = (work->isDouble?CAM_TEST_CAP_SIZE_X/2:CAM_TEST_CAP_SIZE_X);
    const u32 sizeY = (work->isDouble?CAM_TEST_CAP_SIZE_Y/2:CAM_TEST_CAP_SIZE_Y);
    const u8 trgBuf = work->capReq-1;
    const u16 top = (height-sizeY)/2;
    const u32 leftAdr = ((width-sizeX)/2)*sizeof(u16);
    
    for( i=0;i<sizeY;i++ )
    {
      const u32 lineAdr = (top+i)*width*sizeof(u16);
      GFL_STD_MemCopy16( (void*)((u32)captureArea + leftAdr + lineAdr),
                         (void*)((u32)work->capBuff[trgBuf] + (i*sizeX*sizeof(u16))),
                         sizeX*sizeof(u16) );
    }
    work->isUpdateUpper[0] = TRUE;
    work->capReq = 0;
  }
}

static void CAM_TEST_EncodeFinishCallBack( void *userWork , u8* jpgData , u32 jpgSize)
{
  CAM_TEST_WORK *work = userWork;
  if( jpgSize != 0 )
  {
    work->sendSize = jpgSize;
    work->commState = CTCS_SEND_DATA;
  }
  else
  {
    work->commState = CTCS_ENCODE_DATA;
  }
}

#pragma mark [>CommFunc

static const NetRecvFuncTable CamTestRecvTable[] = 
{
  { CAM_TEST_Post_PhotoData   ,CAM_TEST_Post_PhotoData_Buff  },
  { CAM_TEST_PostFlg    ,NULL  },
};

static void CAM_TEST_InitComm( CAM_TEST_WORK *work )
{
  GFLNetInitializeStruct aGFLNetInit = 
  {
    CamTestRecvTable, //NetSamplePacketTbl,  // 受信関数テーブル
    NELEMS(CamTestRecvTable), // 受信テーブル要素数
    NULL,    ///< ハードで接続した時に呼ばれる
    NULL, ///< ネゴシエーション完了時にコール
    NULL, // ユーザー同士が交換するデータのポインタ取得関数
    NULL, // ユーザー同士が交換するデータのサイズ取得関数
    CAM_TEST_GetBeaconData,   // ビーコンデータ取得関数  
    CAM_TEST_GetBeaconSize,   // ビーコンデータサイズ取得関数 
    CAM_TEST_BeacomCompare, // ビーコンのサービスを比較して繋いで良いかどうか判断する
    NULL, // 通信不能なエラーが起こった場合呼ばれる
    NULL, //FatalError
    NULL, // 通信切断時に呼ばれる関数(終了時
    NULL,                   // オート接続で親になった場合
#if GFL_NET_WIFI
    NULL,                   ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL,                   ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,                   ///< wifiフレンドリスト削除コールバック
    NULL,                   ///< DWC形式の友達リスト  
    NULL,                   ///< DWCのユーザデータ（自分のデータ）
    0,                      ///< DWCへのHEAPサイズ
    TRUE,                   ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
    0x333,                  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,         //元になるheapid
    HEAPID_NETWORK,         //通信用にcreateされるHEAPID
    HEAPID_WIFI,            //wifi用にcreateされるHEAPID
    HEAPID_NETWORK,         //
    GFL_WICON_POSX,GFL_WICON_POSY,  // 通信アイコンXY位置
    4,                      //_MAXNUM,  //最大接続人数
    110,                    //_MAXSIZE, //最大送信バイト数
    4,                      //_BCON_GET_NUM,  // 最大ビーコン収集数
    TRUE,                   // CRC計算
    FALSE,                  // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_WIRELESS,  //通信タイプの指定
    TRUE,                   // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_CAMERA_TEST,     //GameServiceID
#if GFL_NET_IRC
    IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
    0,    ///< MP通信時に親の送信量を増やしたい場合サイズ指定 そうしない場合０
    0,    ///< dummy
  };
  work->commState = CTCS_NONE;
  work->isInitComm = FALSE;
  work->isConnect = FALSE;
  work->isPost = FALSE;
  work->jpgQuality = 70;
  
  GFL_NET_Init( &aGFLNetInit , CAM_TEST_FinishInitCallback , (void*)work );
 
}

static void CAM_TEST_ExitComm( CAM_TEST_WORK *work )
{
  GFL_NET_Exit(CAM_TEST_FinishTermCallback);
}


static void CAM_TEST_UpdateComm( CAM_TEST_WORK *work )
{
  static OSTick sendTime;
  switch( work->commState )
  {
  case CTCS_NONE:
    break;
    
  case CTCS_PARENT:
    if( GFL_NET_GetConnectNum() > 0 )
    {
      work->commState = CTCS_CONNECT;
    }
    break;
    
  case CTCS_CHILD:
    {
      u16 i=0;
      
      void* beacon;
      do
      {
        beacon = GFL_NET_GetBeaconData( i );
        i++;
      }while( beacon == NULL && i < 8 );
      
      if( beacon != NULL )
      {
        u8 *macAdd = GFL_NET_GetBeaconMacAddress( i-1 );
        GFL_NET_ConnectToParent( macAdd );
        CAMERA_COMM_Print("Connect!!\n");
        work->commState = CTCS_CONNECT;
      }
    }
    break;
  case CTCS_CONNECT:
    if( GFL_NET_HANDLE_RequestNegotiation() == TRUE )
    {
      work->commState = CTCS_WAIT_NEGOTIATION;
    }
    break;
  case CTCS_WAIT_NEGOTIATION:
    {
      GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsNegotiation( selfHandle ) == TRUE )
      {
        GFL_NET_TimingSyncStart( selfHandle , 0x123 );
        work->commState = CTCS_WAIT_TIMMING;
      }
    }
    break;
  case CTCS_WAIT_TIMMING:
    {
      GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_IsTimingSync( selfHandle , 0x123 ) == TRUE )
      {
        work->selfId = GFL_NET_SystemGetCurrentID();
        CAMERA_COMM_Print("Sync[%d]\n",work->selfId);
        work->commState = CTCS_ENCODE_DATA;
      }
    }
    break; 
  case CTCS_ENCODE_DATA:
    {
      const u32 dataSize = CAM_TEST_CAP_PIXEL*sizeof(u16);

      GFL_STD_MemCopy16( work->capBuff[0],work->encBuff,dataSize );
      
      work->commState = CTCS_WAIT_ENCODE_DATA;
    }                      
    break;
  case CTCS_WAIT_ENCODE_DATA:
    {
      u32 dataSize;
      const u32 sizeX = (work->isDouble?CAM_TEST_CAP_SIZE_X/2:CAM_TEST_CAP_SIZE_X);
      const u32 sizeY = (work->isDouble?CAM_TEST_CAP_SIZE_Y/2:CAM_TEST_CAP_SIZE_Y);
      const u32 convBufSize = SSP_GetJpegEncoderBufferSize( sizeX , sizeY , JPG_OUT_TYPE , JPG_OUT_OPT );
      OSTick startTime = OS_GetTick();
      dataSize = SSP_StartJpegEncoder(  work->encBuff , 
                                        work->sendBuff , 
                                        CAM_TEST_CAP_PIXEL*sizeof(u16) ,
                                        (void*)work->jpgConvBuffAdr , //WRAMから諸々のバッファ取った余り
                                        sizeX ,
                                        sizeY ,
                                        work->jpgQuality ,
                                        JPG_OUT_TYPE ,
                                        JPG_OUT_OPT );
      if( dataSize == 0 )
      {
        OS_TPrintf("JpgConvError!!\n");
      }
      else
      {
        OSTick endTime = OS_GetTick();
        const OSTick time = OS_TicksToMilliSeconds(endTime-startTime);
        CAMERA_COMM_Print("Returu Post[%x][%dms]\n",dataSize,time);
        CAM_TEST_CalcAve( work->sizeAve , &work->sizeCntIdx , dataSize );
        CAM_TEST_CalcAve( work->encTimeAve , &work->encTimeCntIdx , time );
        if( work->sizeMax < convBufSize )
        {
          work->sizeMax = convBufSize;
        }
        if( work->encTimeMax < time )
        {
          work->encTimeMax = time;
        }
        
        work->sendSize = dataSize;
        work->commState = CTCS_SEND_DATA;
      }
    }
    break;
    
  case CTCS_SEND_DATA:
    sendTime = OS_GetTick();
    if( CAM_TEST_Send_PhotoData( work ) == TRUE )
    {
      work->commState = CTCS_WAIT_POST;
    }
    break;

  case CTCS_WAIT_POST:
    if( work->isPost == TRUE )
    {
      OSTick postTime = OS_GetTick();
      const OSTick time = OS_TicksToMilliSeconds(postTime-sendTime);
      CAMERA_COMM_Print("Returu Post[%dms]\n",time);
      CAM_TEST_CalcAve( work->sendSecAve , &work->sendSecCntIdx , time );
      if( work->sendSecMax < time )
      {
        work->sendSecMax = time;
      }
      work->isPost = FALSE;
      work->commState = CTCS_ENCODE_DATA;
    }
    break;
  }
}


//--------------------------------------------------------------
// 通信ライブラリ　初期化・開放用  
//--------------------------------------------------------------
static void CAM_TEST_FinishInitCallback( void* pWork )
{
  CAM_TEST_WORK *work = (CAM_TEST_WORK*)pWork;
  work->isInitComm = TRUE;
}
static void CAM_TEST_FinishTermCallback( void* pWork )
{
  CAM_TEST_WORK *work = (CAM_TEST_WORK*)pWork;
  work->isInitComm = FALSE;
}

//--------------------------------------------------------------
// 写真データ送受信
//--------------------------------------------------------------
static const BOOL CAM_TEST_Send_PhotoData( CAM_TEST_WORK *work )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  {
    const BOOL ret = GFL_NET_SendDataEx( selfHandle , (work->selfId==0?1:0) , 
        CTSM_SEND_PHOTO , work->sendSize , work->sendBuff , FALSE , FALSE , TRUE );
    CAMERA_COMM_Print("Send PhotoData[0x%x]->[0x%x].\n",CAM_TEST_CAP_PIXEL*sizeof(u16),work->sendSize);
    if( ret == FALSE ){
      OS_TPrintf("SendPhoto is failue!\n");
    }
    return ret;
  }
}
static void CAM_TEST_Post_PhotoData( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  CAM_TEST_WORK *work = (CAM_TEST_WORK*)pWork;
  CAMERA_COMM_Print("Post PhotoData.\n");

  work->isUpdateUpper[1] = TRUE;
  work->postSize = size;
}
static u8*    CAM_TEST_Post_PhotoData_Buff( int netID, void* pWork , int size )
{
  CAM_TEST_WORK *work = (CAM_TEST_WORK*)pWork;

  return work->postBuff;
}

typedef struct
{
  u8 flg;
  u8 value;
}CAMERA_TEST_FLG_PACKET;
static void CAM_TEST_SendFlg( CAM_TEST_WORK *work , const u8 flg , const u8 value )
{
  GFL_NETHANDLE *selfHandle = GFL_NET_HANDLE_GetCurrentHandle();
  CAMERA_TEST_FLG_PACKET pkt;
  pkt.flg = flg;
  pkt.value = value;
  CAMERA_COMM_Print("Send Flg[%d:%d].\n",pkt.flg,pkt.value);

  GFL_NET_SendDataEx( selfHandle , (work->selfId==0?1:0) , 
    CTSM_SEND_FLG , sizeof( CAMERA_TEST_FLG_PACKET ) , 
    (void*)&pkt , TRUE , FALSE , FALSE );
}

static void CAM_TEST_PostFlg( const int netID, const int size , const void* pData , void* pWork , GFL_NETHANDLE *pNetHandle )
{
  CAM_TEST_WORK *work = (CAM_TEST_WORK*)pWork;
  CAMERA_TEST_FLG_PACKET *pkt = (CAMERA_TEST_FLG_PACKET*)pData;

  CAMERA_COMM_Print("Post Flg[%d:%d].\n",pkt->flg,pkt->value);
  switch( pkt->flg )
  {
  case CTCF_POST_DATA:
    work->isPost = TRUE;
    break;
  }
}


//--------------------------------------------------------------
// ビーコンデータ取得関数  
//--------------------------------------------------------------
static void*  CAM_TEST_GetBeaconData(void* pWork)
{
  CAM_TEST_WORK *work = (CAM_TEST_WORK*)pWork;
  return (void*)&work->beacon;
}

//--------------------------------------------------------------
// ビーコンデータサイズ取得関数 
//--------------------------------------------------------------
static int CAM_TEST_GetBeaconSize(void *pWork)
{
  return sizeof( CAMERA_TEST_BEACON );
}

static BOOL CAM_TEST_BeacomCompare(GameServiceID GameServiceID1, GameServiceID GameServiceID2)
{
  if( GameServiceID1 == GameServiceID2 )
  {
    return TRUE;
  }
  return FALSE;
}

static void CAM_TEST_ConnectCallBack(void* pWork)
{
  CAM_TEST_WORK *work = (CAM_TEST_WORK*)pWork;
  CAMERA_COMM_Print("Connect!!!");
  work->isConnect = TRUE;
}


#pragma mark [>debugMenu
static void CAM_TEST_DebFunc_StartParent( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebFunc_StartChild( void* userWork , DEBUGWIN_ITEM* item );

static void CAM_TEST_DebFunc_JpgQua( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebDraw_JpgQua( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebFunc_DoubleMode( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebDraw_DoubleMode( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebFunc_SendTime( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebDraw_SendTime( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebFunc_SendSize( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebDraw_SendSize( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebFunc_EncTime( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebDraw_EncTime( void* userWork , DEBUGWIN_ITEM* item );

static void CAM_TEST_DebFunc_CameraActive( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebDraw_CameraActive( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebFunc_CameraPos( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebDraw_CameraPos( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebFunc_CameraResolution( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebDraw_CameraResolution( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebFunc_CameraTrimX( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebDraw_CameraTrimX( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebFunc_CameraTrimY( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebDraw_CameraTrimY( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebFunc_CameraTrimFlag( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebDraw_CameraTrimFlag( void* userWork , DEBUGWIN_ITEM* item );
static void CAM_TEST_DebFunc_CameraTrimSet( void* userWork , DEBUGWIN_ITEM* item );
#define DEB_GROUP_NO (1)
#define DEB_GROUP_CAMERA (2)
#define DEB_GROUP_TRIM (3)

static void CAM_TEST_InitDebugMenu( CAM_TEST_WORK *work )
{
  DEBUG_PAUSE_SetEnable( FALSE );

  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  DEBUGWIN_InitProc( GFL_BG_FRAME1_M , work->fontHandle );
  DEBUGWIN_ChangeLetterColor( 31,31,31 );
  DEBUGWIN_ChangeSelectColor(  0,31,31 );

  DEBUGWIN_AddGroupToTop( DEB_GROUP_NO , "CameraTest" , work->heapId );
  DEBUGWIN_AddGroupToGroup( DEB_GROUP_CAMERA , "Camera" , DEB_GROUP_NO , work->heapId );

  DEBUGWIN_AddItemToGroup( "親機通信",CAM_TEST_DebFunc_StartParent , 
                             (void*)work , DEB_GROUP_NO , work->heapId );
  DEBUGWIN_AddItemToGroup( "子機通信",CAM_TEST_DebFunc_StartChild , 
                             (void*)work , DEB_GROUP_NO , work->heapId );
  DEBUGWIN_AddItemToGroupEx( CAM_TEST_DebFunc_JpgQua , CAM_TEST_DebDraw_JpgQua ,
                             (void*)work , DEB_GROUP_NO , work->heapId );
  DEBUGWIN_AddItemToGroupEx( CAM_TEST_DebFunc_DoubleMode , CAM_TEST_DebDraw_DoubleMode ,
                             (void*)work , DEB_GROUP_NO , work->heapId );
  DEBUGWIN_AddItemToGroupEx( CAM_TEST_DebFunc_SendTime , CAM_TEST_DebDraw_SendTime ,
                             (void*)work , DEB_GROUP_NO , work->heapId );
  DEBUGWIN_AddItemToGroupEx( CAM_TEST_DebFunc_SendSize , CAM_TEST_DebDraw_SendSize ,
                             (void*)work , DEB_GROUP_NO , work->heapId );
  DEBUGWIN_AddItemToGroupEx( CAM_TEST_DebFunc_EncTime , CAM_TEST_DebDraw_EncTime ,
                             (void*)work , DEB_GROUP_NO , work->heapId );

  DEBUGWIN_AddItemToGroupEx( CAM_TEST_DebFunc_CameraActive , CAM_TEST_DebDraw_CameraActive ,
                             (void*)work , DEB_GROUP_CAMERA , work->heapId );
  DEBUGWIN_AddItemToGroupEx( CAM_TEST_DebFunc_CameraPos , CAM_TEST_DebDraw_CameraPos ,
                             (void*)work , DEB_GROUP_CAMERA , work->heapId );
  DEBUGWIN_AddItemToGroup( "↓カメラ停止時のみ有効↓",NULL , NULL , DEB_GROUP_CAMERA , work->heapId );
  DEBUGWIN_AddGroupToGroup( DEB_GROUP_TRIM , "Trimming" , DEB_GROUP_CAMERA , work->heapId );
  DEBUGWIN_AddItemToGroupEx( CAM_TEST_DebFunc_CameraResolution , CAM_TEST_DebDraw_CameraResolution ,
                             (void*)work , DEB_GROUP_CAMERA , work->heapId );
  
  DEBUGWIN_AddItemToGroup( "↓カメラ停止時のみ有効↓",NULL , NULL , DEB_GROUP_TRIM , work->heapId );
  DEBUGWIN_AddItemToGroupEx( CAM_TEST_DebFunc_CameraTrimFlag , CAM_TEST_DebDraw_CameraTrimFlag ,
                             (void*)work , DEB_GROUP_TRIM , work->heapId );
  DEBUGWIN_AddItemToGroupEx( CAM_TEST_DebFunc_CameraTrimX , CAM_TEST_DebDraw_CameraTrimX ,
                             (void*)work , DEB_GROUP_TRIM , work->heapId );
  DEBUGWIN_AddItemToGroupEx( CAM_TEST_DebFunc_CameraTrimY , CAM_TEST_DebDraw_CameraTrimY ,
                             (void*)work , DEB_GROUP_TRIM , work->heapId );
  DEBUGWIN_AddItemToGroup( "サイズ再設定",CAM_TEST_DebFunc_CameraTrimSet , 
                             (void*)work , DEB_GROUP_TRIM , work->heapId );

}
static void CAM_TEST_ExitDebugMenu( CAM_TEST_WORK *work )
{
  DEBUGWIN_RemoveGroup( DEB_GROUP_NO );
  DEBUGWIN_ExitProc();
  GFL_FONT_Delete( work->fontHandle );
}

static u32 CAM_TEST_CalcAve( u32 *dataArr , u8 *nowIdx , u32 newVal )
{
  u8 i;
  u32 sum = 0;
  if( nowIdx != NULL )
  {
    dataArr[*nowIdx] = newVal;
    *nowIdx = *nowIdx+1;
    if( *nowIdx >= CAMERA_TEST_AVE_COUNT )
    {
      *nowIdx = 0;
    }
  }
  for( i=0;i<CAMERA_TEST_AVE_COUNT;i++ )
  {
    sum += dataArr[i];
  }
  return sum/CAMERA_TEST_AVE_COUNT;
}

#pragma mark [>mic func
#include "sound/snd_strm.h"

// マイクゲインを設定
#define COMM_TVT_MIC_Print(...) (void)((OS_TPrintf(__VA_ARGS__)))
//#define COMM_TVT_MIC_Print(...)  ((void)0)

#define COMM_TVT_MIC_AMPGAIN (PM_AMPGAIN_160)
#define COMM_TVT_MIC_SAMPLING_SIZE (0xb000)
#ifdef SDK_TWL
#define COMM_TVT_MIC_SAMPLING_RATE (MIC_SAMPLING_RATE_8180)
#else
#define COMM_TVT_MIC_SAMPLING_RATE (MIC_SAMPLING_RATE_8K)
#endif
#define COMM_TVT_MIC_WAVEOUT_CH	(14)							//波形で使用するチャンネルNO
#define COMM_TVT_MIC_WAVEOUT_PLAY_SPD (32768)

struct _COMM_TVT_MIC_WORK
{
  void *buffer;
  u32  recSize;
  
  
  BOOL isRecord;
  BOOL isPlayWave;
  
  NNSSndWaveOutHandle waveHandle;
};

static void COMM_TVT_MIC_BufferEndCallBack(MICResult	result, void*	arg );

COMM_TVT_MIC_WORK* COMM_TVT_MIC_Init( CAM_TEST_WORK *work )
{
  COMM_TVT_MIC_WORK *micWork = GFL_HEAP_AllocClearMemory( work->heapId , sizeof(COMM_TVT_MIC_WORK) );
  
  MIC_Init();
  PM_Init();
  {
    const u32 ret = PM_SetAmp(PM_AMP_ON);
    if( ret == PM_RESULT_SUCCESS )
    {
      COMM_TVT_MIC_Print("AMPをオンにしました。\n");
    }
    else
    {
      COMM_TVT_MIC_Print("AMPの初期化に失敗（%d）", ret);
    }
  }
  {
    const u32 ret = PM_SetAmpGain(COMM_TVT_MIC_AMPGAIN);
    if( ret == PM_RESULT_SUCCESS )
    {
      COMM_TVT_MIC_Print("AMPのゲインを設定しました。\n");
    }
    else
    {
      COMM_TVT_MIC_Print("AMPのゲイン設定に失敗（%d）", ret);
    }
  }
  micWork->buffer = GFL_NET_Align32Alloc(GFL_HEAPID_APP ,COMM_TVT_MIC_SAMPLING_SIZE);
  
  micWork->isRecord = FALSE;
  micWork->isPlayWave = FALSE;
  micWork->recSize = 0;
  return micWork;
}

void COMM_TVT_MIC_Term( CAM_TEST_WORK *work , COMM_TVT_MIC_WORK *micWork )
{
  GFL_HEAP_FreeMemory( micWork->buffer );
  GFL_HEAP_FreeMemory( micWork );
}

void COMM_TVT_MIC_Main( CAM_TEST_WORK *work , COMM_TVT_MIC_WORK *micWork )
{
  if( micWork->isPlayWave == TRUE )
  {
    if( SND_STRM_CheckFinish() == TRUE )
    {
      SND_STRM_Stop();
      COMM_TVT_MIC_Print( "CommTVTMic StopWave\n" );
      micWork->isPlayWave = FALSE;
    }
  }
  else
  {
    if( micWork->isRecord == FALSE )
    {
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        COMM_TVT_MIC_StartRecord( work , micWork );
      }
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
      {
        if( micWork->recSize != 0 )
        {
          COMM_TVT_MIC_PlayVoice( work , micWork , micWork->buffer , micWork->recSize );
        }
      }
    }
    else
    {
      if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_A) == 0  )
      {
        COMM_TVT_MIC_StopRecord( work , micWork );
      }
      
    }
  }
}

const BOOL COMM_TVT_MIC_StartRecord( CAM_TEST_WORK *work , COMM_TVT_MIC_WORK *micWork )
{
  MICResult ret;
  
  MICAutoParam mic;	//マイクパラメータ
  mic.type			= MIC_SAMPLING_TYPE_SIGNED_8BIT;	//サンプリング種別
  //バッファは32バイトアラインされたアドレスでないとダメ！
  mic.size = COMM_TVT_MIC_SAMPLING_SIZE;
  mic.buffer = micWork->buffer;
  GFL_STD_MemClear32( (void*)micWork->buffer , COMM_TVT_MIC_SAMPLING_SIZE );
  if( (mic.size&0x1f) != 0 ){
  	mic.size &= 0xffffffe0;
  }

  //連続サンプリング時にバッファをループさせるフラグ
  mic.loop_enable		= FALSE;
  //バッファが飽和した際に呼び出すコールバック関数へのポインタ
  mic.full_callback	= COMM_TVT_MIC_BufferEndCallBack;
  //バッファが飽和した際に呼び出すコールバック関数へ渡す引数
  mic.full_arg		= micWork;

  //代表的なサンプリングレートをARM7のタイマー周期に換算した値の定義
  mic.rate = COMM_TVT_MIC_SAMPLING_RATE;
#ifdef SDK_TWL
  ret = MIC_StartLimitedSampling( &mic );
#else
  ret = MIC_StartAutoSampling( &mic );
#endif
    
  
  if( ret == MIC_RESULT_SUCCESS )
  {
#ifdef SDK_TWL
    COMM_TVT_MIC_Print( "CommTVTMic StartRecording(TWL)\n" );
#else
    COMM_TVT_MIC_Print( "CommTVTMic StartRecording(NTR)\n" );
#endif
    micWork->isRecord = TRUE;
    return TRUE;
  }
  else
  {
    COMM_TVT_MIC_Print( "CommTVTMic StartRecording faile[%d]!!!\n",ret );
    
    return FALSE;
  }
}


static void COMM_TVT_MIC_BufferEndCallBack(MICResult	result, void*	arg )
{
  COMM_TVT_MIC_WORK *micWork = arg;
  micWork->isRecord = FALSE;
  micWork->recSize = COMM_TVT_MIC_SAMPLING_SIZE;
  COMM_TVT_MIC_Print( "CommTVTMic StopRecording(Buff is full)\n" );
}

const BOOL COMM_TVT_MIC_StopRecord( CAM_TEST_WORK *work , COMM_TVT_MIC_WORK *micWork )
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
      micWork->recSize = (u32)MIC_GetLastSamplingAddress()-(u32)micWork->buffer+32;
      if( micWork->recSize > COMM_TVT_MIC_SAMPLING_SIZE )
      {
        micWork->recSize = COMM_TVT_MIC_SAMPLING_SIZE;
      }
      
      COMM_TVT_MIC_Print( "CommTVTMic StopRecording buffSize[%x]\n",micWork->recSize );
      micWork->isRecord = FALSE;
      return TRUE;
    }
    else
    {
      COMM_TVT_MIC_Print( "CommTVTMic StopRecording faile[%d]!!!\n",ret );
      return FALSE;
    }
  }
}

const BOOL COMM_TVT_MIC_PlayVoice( CAM_TEST_WORK *work , COMM_TVT_MIC_WORK *micWork , void *buffer , u32 buffSize )
{
  u32 size = buffSize;
	if( (size&0x1f) != 0 ){
		size &= 0xffffffe0;
	}
  SND_STRM_SetUpStraightData( SND_STRM_PCM8 , SND_STRM_8KHZ , GFL_HEAPID_APP, buffer,size);

  SND_STRM_Play();

  micWork->isPlayWave = TRUE;
  COMM_TVT_MIC_Print( "CommTVTMic PlayWave\n" );

  return TRUE;
}

#pragma mark [>debugMenu Comm
static void CAM_TEST_DebFunc_StartParent( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    if( work->isInitComm == TRUE)
    {
      GFL_NET_InitServer();
      work->commState = CTCS_PARENT;
    }
  }
}

static void CAM_TEST_DebFunc_StartChild( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    if( work->isInitComm == TRUE)
    {
      GFL_NET_StartBeaconScan();
      work->commState = CTCS_CHILD;
    }
  }
}

static void CAM_TEST_DebFunc_JpgQua( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  u8 moveValue = 1;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    moveValue = 5;
  }
  
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT &&
      work->jpgQuality >= moveValue )
  {
    work->jpgQuality-=moveValue;
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT &&
      work->jpgQuality <= 100-moveValue )
  {
    work->jpgQuality+=moveValue;
    DEBUGWIN_RefreshScreen();
  }
  
}

static void CAM_TEST_DebDraw_JpgQua( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  DEBUGWIN_ITEM_SetNameV( item , "JPG画質：[%d]",work->jpgQuality );  
}
static void CAM_TEST_DebFunc_DoubleMode( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    MtxFx22 mtx;
    work->isDouble = !work->isDouble;
    GFL_STD_MemClear( G2_GetBG3ScrPtr() , 256*192*2 );
    
    if( work->isDouble == TRUE )
    {
      mtx._00 = FX_Inv(FX32_ONE*2);
      mtx._11 = FX_Inv(FX32_ONE*2);
    }
    else
    {
      mtx._00 = FX_Inv(FX32_ONE);
      mtx._11 = FX_Inv(FX32_ONE);
    }
    mtx._01 = 0;
    mtx._10 = 0;
    G2_SetBG3Affine(&mtx,0,0,0,0);

    work->sizeMax = 0;
    work->sendSecMax = 0;
    work->encTimeMax = 0;

    DEBUGWIN_RefreshScreen();
  }  
}

static void CAM_TEST_DebDraw_DoubleMode( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  if( work->isDouble == TRUE )
  {
    DEBUGWIN_ITEM_SetNameV( item , "倍モード：[ON]" );  
  }
  else
  {
    DEBUGWIN_ITEM_SetNameV( item , "倍モード：[OFF]" );  
  }
  
}

static void CAM_TEST_DebFunc_SendTime( void* userWork , DEBUGWIN_ITEM* item )
{
  //NoFunc
}

static void CAM_TEST_DebDraw_SendTime( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const u32 ave = CAM_TEST_CalcAve( work->sendSecAve , NULL , 0 );

  DEBUGWIN_ITEM_SetNameV( item , "SendTime：[%d][%d]",work->sendSecMax , ave );  
}

static void CAM_TEST_DebFunc_SendSize( void* userWork , DEBUGWIN_ITEM* item )
{
  //NoFunc
}

static void CAM_TEST_DebDraw_SendSize( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const u32 ave = CAM_TEST_CalcAve( work->sizeAve , NULL , 0 )/1024;

  DEBUGWIN_ITEM_SetNameV( item , "SendSize：[%d][%d]",work->sizeMax/1024 , ave );  
}

static void CAM_TEST_DebFunc_EncTime( void* userWork , DEBUGWIN_ITEM* item )
{
  //NoFunc
}

static void CAM_TEST_DebDraw_EncTime( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const u32 ave = CAM_TEST_CalcAve( work->encTimeAve , NULL , 0 );

  DEBUGWIN_ITEM_SetNameV( item , "EncodeTime：[%d][%d]",work->encTimeMax , ave );  
}


#pragma mark [>debugMenu Camera


static void CAM_TEST_DebFunc_CameraActive( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const BOOL flg =CAMERA_SYS_IsCapture( work->camSys );
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    if( flg == TRUE )
    {
      CAMERA_SYS_StopCapture( work->camSys );
    }
    else
    {
      CAMERA_SYS_StartCapture( work->camSys );
    }
    DEBUGWIN_RefreshScreen();
    //DEBUGWIN_UpdateFrame();
  }
}

static void CAM_TEST_DebDraw_CameraActive( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const BOOL flg =CAMERA_SYS_IsCapture( work->camSys );

  if( flg == TRUE )
  {
    DEBUGWIN_ITEM_SetName( item , "カメラ：[ON]" );
  }
  else
  {
    DEBUGWIN_ITEM_SetName( item , "カメラ：[OFF]" );
  }
}

static void CAM_TEST_DebFunc_CameraPos( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    CAMERA_SYS_SwapCameraPos( work->camSys );
    DEBUGWIN_RefreshScreen();
    //DEBUGWIN_UpdateFrame();
  }
}

static void CAM_TEST_DebDraw_CameraPos( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const CAMERASelect pos = CAMERA_SYS_GetCameraPos( work->camSys );

  if( pos == CAMERA_SELECT_IN )
  {
    DEBUGWIN_ITEM_SetName( item , "カメラ位置：[内側]" );
  }
  else
  {
    DEBUGWIN_ITEM_SetName( item , "カメラ位置：[外側]" );
  }
}

static void CAM_TEST_DebFunc_CameraResolution( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const CAMERASize size = CAMERA_SYS_GetResolution( work->camSys );
  if( CAMERA_SYS_IsCapture( work->camSys ) == FALSE )
  {
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      if( size == CAMERA_SIZE_256x192 )
      {
        CAMERA_SYS_SetResolution( work->camSys,CAMERA_SIZE_160x120 );
      }
      else
      {
        CAMERA_SYS_SetResolution( work->camSys,CAMERA_SIZE_256x192 );
      }
      GFL_STD_MemClear( G2S_GetBG3ScrPtr() , 256*192*sizeof(u16) );
      CAMERA_SYS_DeleteReadBuffer( work->camSys );
      CAMERA_SYS_CreateReadBuffer( work->camSys , 2 , (HEAPID_ARIIZUMI_DEBUG | HEAPDIR_MASK) );

      DEBUGWIN_RefreshScreen();
    }
  }
}

static void CAM_TEST_DebDraw_CameraResolution( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const CAMERASize size = CAMERA_SYS_GetResolution( work->camSys );
  if( size == CAMERA_SIZE_256x192 )
  {
    DEBUGWIN_ITEM_SetName( item , "カメラ解像度：[256x192]" );
  }
  else
  {
    DEBUGWIN_ITEM_SetName( item , "カメラ解像度：[160x120]" );
  }
}

static void CAM_TEST_DebFunc_CameraTrimX( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  {
    work->trimSizeX--;
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    work->trimSizeX++;
    DEBUGWIN_RefreshScreen();
  }
}

static void CAM_TEST_DebDraw_CameraTrimX( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const u16 width = CAMERA_SYS_ResolutionSizeToWidth( work->camSys );
  const u16 height= CAMERA_SYS_ResolutionSizeToHeight( work->camSys );
  const u16 nowWidth = CAMERA_SYS_CaptureSizeToWidth( work->camSys );
  if( work->trimSizeX < CAM_TEST_CAP_SIZE_X )
  {
    work->trimSizeX = CAM_TEST_CAP_SIZE_X;
  }
  if( work->trimSizeX > width )
  {
    work->trimSizeX = width;
  }
  
  DEBUGWIN_ITEM_SetNameV( item , "はば：[%d]->[%d]",nowWidth,work->trimSizeX );
}


static void CAM_TEST_DebFunc_CameraTrimY( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  {
    work->trimSizeY-=4;
    DEBUGWIN_RefreshScreen();
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  {
    work->trimSizeY+=4;
    DEBUGWIN_RefreshScreen();
  }
}

static void CAM_TEST_DebDraw_CameraTrimY( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const u16 width = CAMERA_SYS_ResolutionSizeToWidth( work->camSys );
  const u16 height= CAMERA_SYS_ResolutionSizeToHeight( work->camSys );
  const u16 nowHeight = CAMERA_SYS_CaptureSizeToHeight( work->camSys );
  if( work->trimSizeY < CAM_TEST_CAP_SIZE_Y )
  {
    work->trimSizeY = CAM_TEST_CAP_SIZE_Y;
  }
  if( work->trimSizeY > height )
  {
    work->trimSizeY = height;
  }
  
  DEBUGWIN_ITEM_SetNameV( item , "高さ：[%d]->[%d]",nowHeight,work->trimSizeY );
}

static void CAM_TEST_DebFunc_CameraTrimFlag( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const BOOL isTrim = CAMERA_SYS_GetIsTrimming( work->camSys );
  const u16 width = CAMERA_SYS_ResolutionSizeToWidth( work->camSys );
  const u16 height= CAMERA_SYS_ResolutionSizeToHeight( work->camSys );
  if( CAMERA_SYS_IsCapture( work->camSys ) == FALSE )
  {
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      GFL_STD_MemClear( G2S_GetBG3ScrPtr() , 256*192*sizeof(u16) );
      CAMERA_SYS_DeleteReadBuffer( work->camSys );
      if( isTrim == TRUE )
      {
        CAMERA_SYS_ResetTrimming( work->camSys );
      }
      else
      {
        CAMERA_SYS_SetTrimming( work->camSys , 
                                (width/2) -(work->trimSizeX/2) , 
                                (height/2)-(work->trimSizeY/2) , 
                                (width/2) +(work->trimSizeX/2) , 
                                (height/2)+(work->trimSizeY/2) );
      }
      CAMERA_SYS_CreateReadBuffer( work->camSys , 2 , (HEAPID_ARIIZUMI_DEBUG | HEAPDIR_MASK) );
      DEBUGWIN_RefreshScreen();
    }
  }
}

static void CAM_TEST_DebDraw_CameraTrimFlag( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const BOOL isTrim = CAMERA_SYS_GetIsTrimming( work->camSys );
  if( isTrim == TRUE )
  {
    DEBUGWIN_ITEM_SetName( item , "トリミング：[ON]" );
  }
  else
  {
    DEBUGWIN_ITEM_SetName( item , "トリミング：[OFF]" );
  }
}

static void CAM_TEST_DebFunc_CameraTrimSet( void* userWork , DEBUGWIN_ITEM* item )
{
  CAM_TEST_WORK *work = userWork;
  const BOOL isTrim = CAMERA_SYS_GetIsTrimming( work->camSys );
  const u16 width = CAMERA_SYS_ResolutionSizeToWidth( work->camSys );
  const u16 height= CAMERA_SYS_ResolutionSizeToHeight( work->camSys );
  if( CAMERA_SYS_IsCapture( work->camSys ) == FALSE )
  {
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      GFL_STD_MemClear( G2S_GetBG3ScrPtr() , 256*192*sizeof(u16) );
      CAMERA_SYS_DeleteReadBuffer( work->camSys );
      CAMERA_SYS_SetTrimming( work->camSys , 
                              (width/2) -(work->trimSizeX/2) , 
                              (height/2)-(work->trimSizeY/2) , 
                              (width/2) +(work->trimSizeX/2) , 
                              (height/2)+(work->trimSizeY/2) );
      CAMERA_SYS_CreateReadBuffer( work->camSys , 2 , (HEAPID_ARIIZUMI_DEBUG | HEAPDIR_MASK) );
      DEBUGWIN_RefreshScreen();
    }
  }
}

#endif  //def SDK_TWL
