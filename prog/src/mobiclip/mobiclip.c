//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mobiclip.c
 *	@brief  Mobiclip再生アプリ
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.04.27
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//lib
#include <gflib.h>

// system
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/file_util.h"   // FS_File

// archive
#include "arc_def.h"

// mobliclip library
#include <Mobiclip.h>

// mine
#include "mobiclip/mobiclip.h"
#include "../../resource/mobiclip/mobiclip_local_def.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

// Transform MicroSeconds to Ticks
#define ALARM_COUNT( x )	(u32)(OS_MicroSecondsToTicks( x ))

// Skipボタン定義
#define PAD_BUTTON_SKIP       ( PAD_BUTTON_START | PAD_BUTTON_B )

//==============================================================================
//	ローカル変数
//==============================================================================

// volatile修飾子が必須
// ⇒別スレッドで操作しているため、最適化されてしまうと
// 無限ループではまる可能性がある為。

// Blit転送可能フラグ
static vu32       g_BlitImageFlag;

// サウンドアラームが呼ばれた回数
static vu64 			g_SoundPacketStreamed;	

// イメージ転送VRAM INDEX
static u32	      g_DispBuffer;	

// Flip状態を表す
// Flip待ちだと0
// Flipすると1になる。
static vu32	      g_FlipStatus;	

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	メインワーク
//=====================================
typedef struct {
  HEAPID          heapID;

  MOHandle        handle;           // Mobiclipムービーハンドル
  FILEHANDLE      *file;            // FSハンドル

  BOOL            is_open;          // 初期化出来たか
  BOOL            coverClosed;      // 蓋が折り畳まれている時にTRUE
  
  // image
  int             nbFrameBuffered;  // 解凍フレームカウンタ
  BOOL	          skipDone;         // 処理落ち時に毎フレームSkipするの防止用
  GFL_TCB         *VblankTcb;       // 登録したVblankFuncのポインタ
  OSAlarm         alarm;            // タイミング用


  // sound
  u64 				    nbSoundPacketPushed;        // パケット転送した回数
	int             nbSoundPacketPerFrame;      // 1フレームのパケット数
	BOOL		        soundStarted;               // サウンド再生開始フラグ
  s16*				    audioSampleBufferLeft;      // サウンドバッファ(左音)
  s16*				    audioSampleBufferRight;     // サウンドバッファ(右音)
  u32				      audioSampleBufferIndex;     // サウンドバッファアドレス計算用
  u32				      soundBufferNbPackets;       // サウンドバッファの容量 1フレームのパケット×キューにためておけるフレーム数

  // 【仮】戻り値がある訳ではないので、外部で持たせなくてもいいかもしれない
  MOBICLIP_PARAM  *p_param;     // パラメータ

} MO_WORK;

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT MOBICLIP_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT MOBICLIP_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT MOBICLIP_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );


//-------------------------------------
///	開く/閉じる
//=====================================
static BOOL MO_Open( MO_WORK *p_wk, ARCID arcID );
static void MO_Close( MO_WORK *p_wk );

//-------------------------------------
///	MO_IMAGE
//=====================================
static BOOL MO_IMAGE_Init( MO_WORK *p_wk );
static BOOL MO_IMAGE_Main( MO_WORK *p_wk, MOHandle handle );
static void MO_IMAGE_Exit( MO_WORK *p_wk );
static u16* MO_IMAGE_GetBackBuffer( void );
static void MO_IMAGE_Flip( void );
static void MO_IMAGE_BlitAndFlip( MOHandle handle );

static void VBlankFunc( GFL_TCB *tcb, void * work );
static void AlarmIntr( void* arg );

//-------------------------------------
///	MO_SOUND
//=====================================
static BOOL MO_SOUND_Init( MO_WORK *p_wk, HEAPID heapID );
static BOOL MO_SOUND_Main( MO_WORK *p_wk, MOHandle handle );
static void MO_SOUND_Exit( MO_WORK *p_wk );
static void MO_SOUND_CallBackFunc( void *dummy );
static void MO_SOUND_StartSound( MO_WORK *p_wk, u32 frequency, u32 nbSamplePerAudioPacket );
static void MO_SOUND_StopSound( void );

//-------------------------------------
///	DS蓋開閉時コールバック
//=====================================
// スリープ起動時に呼ばれる関数
static void MO_SleepGo_Func( void *p_wk );
// スリープ復帰時に呼ばれる関数
static void MO_SleepRelease_Func( void *p_wk );
static void MO_Sleep_SetBackLight( PMBackLightSwitch sw );

//=============================================================================
/**
 *						GLOBAL
 */
//=============================================================================
//-------------------------------------
///	PROCデータ外部公開
//=====================================
const GFL_PROC_DATA	Mobiclip_ProcData	=
{
	MOBICLIP_PROC_Init,
	MOBICLIP_PROC_Main,
	MOBICLIP_PROC_Exit,
};

//=============================================================================
// Mobiclipライブラリで
// 割り当て/割り当て解除の関数は
// 自分で作らなければならない
//
// ※必須
//=============================================================================
void* MO_Malloc ( u32 size )
{
  ICHI_Printf("MO_Malloc( %x ) \n", size);
	return GFL_HEAP_AllocMemory( HEAPID_MOBICLIP, size );
}
void MO_Free ( void* mem_p )
{
	GFL_HEAP_FreeMemory(mem_p);
}

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  MOBICLIPパラメータ作成
 *
 *	@param	heapID        ヒープID
 *	@param	arcID         再生するarcID
 *	@param	mode          再生モード
 *	@param	skip_enable   SKIP可能か
 *
 *	@return 作成したパラメータ
 */
//-----------------------------------------------------------------------------
MOBICLIP_PARAM * MOBICLIP_PARAM_Alloc( HEAPID heapID, ARCID arcID, MOBICLIP_MODE mode, BOOL skip_enable )
{
  MOBICLIP_PARAM  *p_param;
  p_param = GFL_HEAP_AllocMemory( heapID, sizeof(MOBICLIP_PARAM) );
  GFL_STD_MemClear( p_param, sizeof(MOBICLIP_PARAM) );

  p_param->arcID  = arcID;
  p_param->mode   = mode;
  p_param->skip_enable = skip_enable;

  return p_param;
}

//----------------------------------------------------------------------------
/**
 *	@brief  MOBICLIPパラメータ解放
 *
 *	@param	MOBICLIP_PARAM  
 */
//-----------------------------------------------------------------------------
void MOBICLIP_PARAM_Free( MOBICLIP_PARAM *p_param )
{
  GFL_HEAP_FreeMemory( p_param );
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================

//=============================================================================
/**
 *　　PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  PROC初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param_adrs			引数
 *	@param	*p_wk_adrs				ワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MOBICLIP_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{

  MO_WORK  *p_wk;

  ICHI_Printf("GFL_HEAPID_APP HeapSize 0x%x \n", GFL_HEAP_GetHeapFreeSize(GFL_HEAPID_APP));
  // ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MOBICLIP, 0x100000 );

  // プロセスワーク作成
  p_wk = GFL_PROC_AllocWork( p_proc, sizeof( MO_WORK ), HEAPID_MOBICLIP );
  MI_CpuClear8( p_wk, sizeof( MO_WORK ) );

  p_wk->heapID  = HEAPID_MOBICLIP;
  p_wk->p_param = p_param_adrs;

  // スリープ コールバック関数設定
  GFL_UI_SleepGoSetFunc( &MO_SleepGo_Func, p_wk );
  GFL_UI_SleepReleaseSetFunc( &MO_SleepRelease_Func, p_wk );
  p_wk->coverClosed = FALSE;

  // ファイル開く
  p_wk->is_open = MO_Open( p_wk, p_wk->p_param->arcID );

  // 初期化
  if( p_wk->is_open == TRUE )
  {
    BOOL ret;

    // イメージ構造体初期化
    ret = MO_IMAGE_Init( p_wk );
    GF_ASSERT_MSG( ret, "MO_IMAGE_Init初期化失敗\n");

    // サウンド初期化
    ret = MO_SOUND_Init( p_wk, p_wk->heapID );
    GF_ASSERT_MSG( ret, "MO_SOUND_Init初期化失敗\n");
  }

  OS_Printf("MOBICLIP_PROC_Init\n");
  OS_Printf("Free HeapSize 0x%x \n", GFL_HEAP_GetHeapFreeSize(p_wk->heapID));
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  PROC破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param_adrs			引数
 *	@param	*p_wk_adrs				ワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MOBICLIP_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  MO_WORK *p_wk = p_wk_adrs;

  // 破棄
  if( p_wk->is_open == TRUE )
  {
    MO_IMAGE_Exit( p_wk );
    MO_SOUND_Exit( p_wk );
  }

  // ファイル閉じる
  MO_Close( p_wk );

  // スリープ
  GFL_UI_SleepGoSetFunc( NULL, NULL );
  GFL_UI_SleepReleaseSetFunc( NULL, NULL );

  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );
  //ヒープ破棄
  GFL_HEAP_DeleteHeap( HEAPID_MOBICLIP );

  OS_Printf("MOBICLIP_PROC_Exit\n");
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PROCメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param_adrs			引数
 *	@param	*p_wk_adrs				ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MOBICLIP_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  MO_WORK *p_wk = p_wk_adrs;

  BOOL  is_end = FALSE;     // 終了

  // ファイルオープンに失敗していたら、終了させる
  if( p_wk->is_open == FALSE )
  {
    return GFL_PROC_RES_FINISH;
  }

  // 蓋閉じてるなら処理しない
  if( p_wk->coverClosed == TRUE )
  {
    return GFL_PROC_RES_CONTINUE;
  }

  // イメージメイン
  if( MO_IMAGE_Main( p_wk, p_wk->handle ) )
  {
    is_end = TRUE;
  }
  // サウンドメイン
  if( MO_SOUND_Main( p_wk, p_wk->handle ) )
  {
    is_end = TRUE;
  }

  // ムービーSkip
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SKIP )
  {
    is_end = TRUE;
  }
  
  // 終了
  if( is_end ){ return GFL_PROC_RES_FINISH; }
  else        { return GFL_PROC_RES_CONTINUE; }

  return GFL_PROC_RES_CONTINUE;
}


//=============================================================================
/**
 *        開く/閉じる
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ムービーファイルを開く
 *
 *	@param	*p_wk   メインワーク
 *	@param	arcID   開きたいムービーファイル
 *
 *	@return 開けたらTRUE
 */
//-----------------------------------------------------------------------------
static BOOL MO_Open( MO_WORK *p_wk, ARCID arcID )
{
  GF_ASSERT_MSG( BUFFERED_FRAME_MAX >= MO_NB_BUFFERED_FRAME_MIN, "キュー定義 6以下には出来ない。\n" );

  //-------------------------------------
  // Mobiclipが必要とする命令の読み込み
  //-------------------------------------
  {
    u32 sizeITCM;
    u32 sizeDTCM;

    sizeITCM = (u32)OS_GetITCMArenaHi() - (u32)OS_GetITCMArenaLo();
    if( sizeITCM < MO_GetOptimalITCMSize() )
    {
      GF_ASSERT_MSG( 0, "WARNING: ITCMを読み出せません\n");
    }
    else
    {
      // 最適なﾊﾟﾌｫｰﾏﾝｽを実行するのに必要な命令TCMのバイト
      sizeITCM = MO_GetOptimalITCMSize();
    }
    OS_Printf("Mobiclip 命令TCM:%d byte\n", sizeITCM);

    sizeDTCM = (u32)OS_GetDTCMArenaHi() - (u32)OS_GetDTCMArenaLo();
    if( sizeDTCM < MO_GetOptimalDTCMSize() )
    {
      GF_ASSERT_MSG( 0, "WARNING: DTCMを読み出せません\n");
    }
    else
    {
      // 最適なﾊﾟﾌｫｰﾏﾝｽを実行するのに必要なデータTCMのバイト
      sizeDTCM = MO_GetOptimalDTCMSize();
    }
    OS_Printf("Mobiclip データTCM:%d byte\n", sizeDTCM);

    //MO_InitITCMArea( OS_GetITCMArenaLo(), sizeITCM );
    //MO_InitDTCMArea( OS_GetDTCMArenaLo(), sizeDTCM );
  }

  //-------------------------------------
  // ムービーファイル開く
  //-------------------------------------
  {
    // FileSysteでファイル開く
    p_wk->file = GFL_FILE_OpenHandle( arcID, p_wk->heapID );
    OS_Printf("ムービーデーファイル %s \n", GFUser_GetFileNameByArcID( arcID ));
    OS_Printf("ムービーデータサイズ %d byte \n", GFL_FILE_GetDataSize( arcID ));

    // メモリにムービーファイル展開
    p_wk->handle = MO_OpenMovie( GFL_FILE_GetFSFile(p_wk->file), BUFFERED_FRAME_MAX );
    if( p_wk->handle == MO_INVALID_HANDLE )
    {
      GF_ASSERT_MSG( 0,"ERROR: Cannot open movie file \n");
      return FALSE;
    }

    // デバッグチェック
    if( OS_IsThreadAvailable() == TRUE)
    {
      OS_Printf( "作成されているスレッドの数 %d \n", OS_GetNumberOfThread());
      OS_DumpThreadList();

      // スタック漏れチェック
      // デバッグ用なので、FINALROMでは動作しない
      OS_CheckStack(OS_GetCurrentThread());
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ムービーファイルを閉じる
 *
 *	@param	*p_wk   メインワーク
 */
//-----------------------------------------------------------------------------
static void MO_Close( MO_WORK *p_wk )
{
  if( p_wk->is_open == TRUE )
  {
    // ムービー閉じる
    MO_CloseMovie( p_wk->handle );
  }

  // ファイルクロース
  GFL_FILE_CloseHandle( p_wk->file );
}

//=============================================================================
/**
 *        MO_IMAGE
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ムービーイメージ  初期化
 *
 *	@param	*p_wk  メインワーク
 *	@param	*p_wk     ワーク
 *
 *	@return 初期化完了ならTRUE
 */
//-----------------------------------------------------------------------------
static BOOL MO_IMAGE_Init( MO_WORK *p_wk )
{

  // グラフィック初期化
  {
    // Disable display of main LCD sreen
    GX_DispOff();

    // Disable display of sub LCD sreen
    GXS_DispOff();

    //レジスタ初期化
    G2_BlendNone();
    G2S_BlendNone();	
    GX_SetVisibleWnd( GX_WNDMASK_NONE );
    GXS_SetVisibleWnd( GX_WNDMASK_NONE );

    // All VRAM banks to LCDC
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

    // Clear all LCDC space
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    // Disable the banks on LCDC
    (void)GX_DisableBankForLCDC();

    // Clear OAM
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);

    // Clear the standard palette
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);

    // Clear OAM
    MI_CpuFillFast((void*)HW_DB_OAM, 192, HW_DB_OAM_SIZE);

    // Clear the standard palette
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);  

    // LCD出力先
    GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);

    // Enables all the banks on LCDC
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

    // Enable display of main LCD sreen
    GX_DispOn();
    //GXS_DispOn();

    GX_SetMasterBrightness(0);
    //GXS_SetMasterBrightness(-16);
    GFL_DISP_GX_SetVisibleControlDirect(0);   //全BG&OBJの表示OFF
    //GFL_DISP_GXS_SetVisibleControlDirect(0);

  }

  // 値初期化
  p_wk->nbFrameBuffered = 0;
  p_wk->skipDone        = FALSE;
  g_BlitImageFlag       = 0;
  g_DispBuffer          = 0;
	g_FlipStatus          = 0;

  // VBlank関数セット
  p_wk->VblankTcb = GFUser_VIntr_CreateTCB( VBlankFunc, p_wk, 1 );

	// Initialize the alarm structure
	OS_CreateAlarm( &p_wk->alarm );

	// Set periodic alarm to call HandlerAlarmIntr() every (1 / framerate) sec after 10 usec.
	OS_SetPeriodicAlarm( &p_wk->alarm, OS_GetTick()+ALARM_COUNT( 10 ), ALARM_COUNT( (u64)16777216000000/(u64)MO_GetVideoFps( p_wk->handle ) ), &AlarmIntr, (void *)0 );

  ICHI_Printf("MO_GetVideoFps %d \n", (u64)MO_GetVideoFps(p_wk->handle) );
  ICHI_Printf("ALARM_COUNT %d \n", ALARM_COUNT( (u64)16777216000000/(u64)MO_GetVideoFps( p_wk->handle )));

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ムービーイメージ  メイン
 *
 *	@param	*p_wk   ワーク
 *	@param	handle  Mobiclipハンドル
 *
 *	@return 再生終了ならTRUE
 */
//-----------------------------------------------------------------------------
static BOOL MO_IMAGE_Main( MO_WORK *p_wk, MOHandle handle )
{

  // ムービーフレームを読み込む
  // 読み込んだフレームはMO_UnpackFrameImageで解凍する
  if( MO_ReadFrame( handle ) == 0 )
  {
    return TRUE;
  }

  // ムービーフレームのイメージを解凍して、
  // イメージフレームFIFOに格納する。
  MO_UnpackFrameImage( handle );

  // フレームを解凍した回数
  p_wk->nbFrameBuffered++;


  // フレームを解凍した回数が指定した回数に達したら、イメージ転送をする
  if (p_wk->nbFrameBuffered >= BUFFERED_FRAME_MAX || MO_GetAudioFrequency(handle) == 0)
  {
    // パケット転送した回数 - サウンドアラームが呼ばれた回数
    // = 転送していないサウンドパケット数
    s32 nbRemainingSoundPacketsToStream = (int)(p_wk->nbSoundPacketPushed - g_SoundPacketStreamed);


    // 閉じてない
    // 転送していないパケットが1フレームに処理できるパケット数*3以内
    // サウンド有り
    // フレームを解凍した回数が指定した回数以上
    if (p_wk->coverClosed || 
        (nbRemainingSoundPacketsToStream <= p_wk->nbSoundPacketPerFrame * 3 && 
         p_wk->skipDone == FALSE && 
         MO_GetAudioFrequency( handle ) > 0 &&
         p_wk->nbFrameBuffered >= BUFFERED_FRAME_MAX))
    {
      // 解凍されたイメージをスキップする。内部FIFOからイメージを取り除く
      // ※サウンドバッファアンダーフローが起きたときなどに発生する
      MO_SkipFrameImage( handle );
      p_wk->skipDone = TRUE;
      ICHI_Printf("SkipFrameImage %d \n", p_wk->nbFrameBuffered);
    }
    else
    {
      // イメージをBlit転送し、バッファをFlipする
      MO_IMAGE_BlitAndFlip( handle );

      p_wk->skipDone = FALSE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ムービーイメージ  終了
 *
 *	@param	*p_wk   ワーク
 */
//-----------------------------------------------------------------------------
static void MO_IMAGE_Exit( MO_WORK *p_wk )
{
  // Flipするまで待ち
  while (g_FlipStatus == 0){};

  // Cancel the alarm 
  OS_CancelAlarm( &p_wk->alarm );

  // Vblank関数破棄
  GFL_TCB_DeleteTask( p_wk->VblankTcb );
}
//----------------------------------------------------------------------------
/**
 *	@brief  表示VRAM取得
 *
 *	@return イメージ転送するVRAM
 */
//-----------------------------------------------------------------------------
static u16* MO_IMAGE_GetBackBuffer( void )
{
	if (g_DispBuffer == 0)
		return (u16*)HW_LCDC_VRAM_B;
	else
		return (u16*)HW_LCDC_VRAM_A;
}
//----------------------------------------------------------------------------
/**
 *	@brief  Flip:イメージを変更
 */
//-----------------------------------------------------------------------------
static void MO_IMAGE_Flip( void )
{
	if (g_DispBuffer == 0)
		GX_SetGraphicsMode(GX_DISPMODE_VRAM_B, (GXBGMode)0, (GXBG0As)0 );
	else
		GX_SetGraphicsMode(GX_DISPMODE_VRAM_A, (GXBGMode)0, (GXBG0As)0 );
	
	g_DispBuffer ^= 1;
}

//----------------------------------------------------------------------------
/**
 *	@brief  Blit:メモリからイメージ転送
 *	        後にFlip待ちに変更
 *
 *	@param	MOHandle handle 
 */
//-----------------------------------------------------------------------------
static void MO_IMAGE_BlitAndFlip( MOHandle handle )
{
  // VBlankでFlipするまで待つ
	while (g_FlipStatus == 0){};

  // アラームで、Blit転送可能待ち
	while( g_BlitImageFlag != 1 ) ;
	g_BlitImageFlag = 0;
	
  // イメージを、内部FIFOからVRAMにBlit転送する。
  // その後FIFOから取り除く。
  // 内部でMO_Mallocが呼び出される。
	MO_BlitFrameImage(handle,MO_IMAGE_GetBackBuffer(),256,MO_QUALITY_ENHANCEMENT_NONE);
	
  // Flip待ちに変更
	g_FlipStatus = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  VBlank関数
 */
//-----------------------------------------------------------------------------
static void VBlankFunc( GFL_TCB *tcb, void * work )
{
  MO_WORK *p_wk = work;

	if (g_FlipStatus == 0)
	{
		g_FlipStatus = 1;
		MO_IMAGE_Flip();
	}
}

static void AlarmIntr( void* arg )
{
	#pragma unused (arg)

  // イメージをBlit転送可能フラグON
	g_BlitImageFlag = 1;
}

//=============================================================================
/**
 *        MO_SOUND
 */
//=============================================================================


//----------------------------------------------------------------------------
/**
 *	@brief  ムービーサウンド
 *
 *	@param	*p_wk  メインワーク
 *	@param	*p_wk     ワーク
 *
 *	@return 初期化完了ならTRUE
 */
//-----------------------------------------------------------------------------
static BOOL MO_SOUND_Init( MO_WORK *p_wk, HEAPID heapID )
{
	u32 nbSamplePerAudioPacket;

  MOHandle handle = p_wk->handle;

  nbSamplePerAudioPacket = MO_GetNbSamplePerAudioPacket(handle);

  // FPS = MO_GetVideoFps() / 16777216
  // 今回の動画は15FPS

  p_wk->soundStarted = FALSE;
  p_wk->nbSoundPacketPerFrame = (int)(((u64)MO_GetAudioFrequency(handle) * (u64)16777216) / ((u64)MO_GetVideoFps(handle) * (u64)nbSamplePerAudioPacket)); 
  p_wk->nbSoundPacketPerFrame += 1;
  ICHI_Printf("nbSouncPacketPerFrame %d \n", p_wk->nbSoundPacketPerFrame );

  p_wk->soundBufferNbPackets = p_wk->nbSoundPacketPerFrame * (BUFFERED_FRAME_MAX+1);

  // sound buffer アロケーション
  p_wk->audioSampleBufferLeft = (s16*)GFL_HEAP_AllocMemory( heapID, p_wk->soundBufferNbPackets * nbSamplePerAudioPacket * 2);
  p_wk->audioSampleBufferRight = (s16*)GFL_HEAP_AllocMemory( heapID,p_wk->soundBufferNbPackets * nbSamplePerAudioPacket * 2);
  ICHI_Printf("Sound Buffer Size ( %x ) * 2\n", p_wk->soundBufferNbPackets * nbSamplePerAudioPacket * 2);

  // sound buffers 0 クリア
  GFL_STD_MemClear(p_wk->audioSampleBufferLeft,p_wk->soundBufferNbPackets*nbSamplePerAudioPacket*2);
  GFL_STD_MemClear(p_wk->audioSampleBufferRight,p_wk->soundBufferNbPackets*nbSamplePerAudioPacket*2);
  DC_StoreAll();

  // No sound packets are streamed by the sound hardware for the moment
  g_SoundPacketStreamed = 0;

  // Initialization the index in the sound buffer where we have to put the new audio samples
  p_wk->audioSampleBufferIndex = 0;

  // No sound packets are pushed by the user for the moment
  p_wk->nbSoundPacketPushed = 0;

  ICHI_Printf("Sound 初期化 \n");
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ムービーサウンド
 *
 *	@param	*p_wk     ワーク
 *	@param	handle    mobiclipハンドル
 *
 *	@return 再生終了ならTRUE
 */
//-----------------------------------------------------------------------------
static BOOL MO_SOUND_Main( MO_WORK *p_wk, MOHandle handle )
{
  u8 i;

  // OS_IsRunOnEmulator → プログラムが"ensata NITRO Software Emulator"上で
  //  実行されていればTRUEを返す。通常ならDEBUGGER実行なのでFALSEを返す。
  //
  // MO_GetAudioFrequency → ムービーのオーディオ周波数を取得
  //
  if (OS_IsRunOnEmulator() == FALSE && MO_GetAudioFrequency(handle) > 0) 
  {
    // カレントフレームの、
    // トラックごとのオーディオパケット数取得
    u32 nbUnpackedSoundPackets = MO_GetFrameNbAudioPacket(handle);

    //-----------------------------------------------------------------------

    // 解凍したカレントフレームのオーディオパケット数分ループ
    for (i=0;i<nbUnpackedSoundPackets;i++)
    {
      // サウンド再生フラグがONなら通る
      // ONになる条件は経過フレームと
      // 解凍出来るフレーム数最大(BUFFERED_FRAME_MAX)が同じになったら
      if (p_wk->soundStarted)
      {
        if (g_SoundPacketStreamed >= p_wk->nbSoundPacketPushed)
          OS_Printf("Sound underflow !!!! \n" );
        else
        {
          // サウンドバッファを新しく転送できるサイズ
          u64 nbSoundPacketsAvailableInBuffer;

          // サウンドバッファに空の容量がない場合は、ループさせる
          do
          {
            // サウンドのパケットを転送した回数 -  サウンドアラームが呼ばれた回数
            nbSoundPacketsAvailableInBuffer = p_wk->nbSoundPacketPushed - g_SoundPacketStreamed;
            // サウンドバッファの容量 - 上記計算結果
            nbSoundPacketsAvailableInBuffer = p_wk->soundBufferNbPackets - nbSoundPacketsAvailableInBuffer;
          }
          while (nbSoundPacketsAvailableInBuffer == 0);
        }
      }

      {
        u32 nbSamplePerAudioPacket = MO_GetNbSamplePerAudioPacket(handle);

        // カレントサウンドのパケットをユーザーバッファへ転送する
        // 第2引数はサウンドフレームの転送先のポインタ
        MO_BlitFrameSoundOnePacket(handle,p_wk->audioSampleBufferLeft+p_wk->audioSampleBufferIndex);
        MO_BlitFrameSoundOnePacket(handle,p_wk->audioSampleBufferRight+p_wk->audioSampleBufferIndex);

        // データをメモリへ展開
        // サンプルパケット分
        DC_StoreRange(p_wk->audioSampleBufferLeft+p_wk->audioSampleBufferIndex,nbSamplePerAudioPacket * 2);
        DC_StoreRange(p_wk->audioSampleBufferRight+p_wk->audioSampleBufferIndex,nbSamplePerAudioPacket * 2);

        // アドレスをパケット分加算
        p_wk->audioSampleBufferIndex+=nbSamplePerAudioPacket;
        if (p_wk->audioSampleBufferIndex == p_wk->soundBufferNbPackets * nbSamplePerAudioPacket)
          p_wk->audioSampleBufferIndex = 0;
      }

      // パケット転送した回数インクリメント
      p_wk->nbSoundPacketPushed++;
    }

    //-----------------------------------------------------------------------

    // フレームを解凍した回数が指定した回数に達したら、サウンド再生開始
    if (p_wk->soundStarted == FALSE && 
        ( p_wk->nbFrameBuffered >= BUFFERED_FRAME_MAX ))
    {
      MO_SOUND_StartSound( p_wk, MO_GetAudioFrequency(handle),MO_GetNbSamplePerAudioPacket(handle));
      p_wk->soundStarted = TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ムービーサウンド
 *
 *	@param	*p_wk     ワーク
 */
//-----------------------------------------------------------------------------
static void MO_SOUND_Exit( MO_WORK *p_wk )
{
  // We stop the sound playback if necessary
  if (p_wk->soundStarted == TRUE)
    MO_SOUND_StopSound();

  // Release of the sound buffer
  GFL_HEAP_FreeMemory(p_wk->audioSampleBufferLeft);
  GFL_HEAP_FreeMemory(p_wk->audioSampleBufferRight);

}

static void MO_SOUND_CallBackFunc( void *dummy )
{
	#pragma unused (dummy)
	g_SoundPacketStreamed++;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ステレオサウンド再生開始
 *
 *	@param	frequency   サウンド周波数 Hz
 *	@param	nbSamplePerAudioPacket  オーディオパケットのサンプル数 主に256
 */
//-----------------------------------------------------------------------------
static void MO_SOUND_StartSound( MO_WORK *p_wk, u32 frequency, u32 nbSamplePerAudioPacket )
{
	u32 soundClockTick;
	u32 alarmClockTick;
	
	// アラーム周期の1単位
	soundClockTick = SND_TIMER_CLOCK/frequency;
	
  // 周期的にアラームを発動させる間隔を求める計算式
	alarmClockTick = (soundClockTick * 2) * (nbSamplePerAudioPacket / 64);
		
	// データをメモリへコピー
	DC_StoreRange(p_wk->audioSampleBufferLeft,p_wk->soundBufferNbPackets*nbSamplePerAudioPacket*2);
	DC_StoreRange(p_wk->audioSampleBufferRight,p_wk->soundBufferNbPackets*nbSamplePerAudioPacket*2);
		
	//----------------------------------------------------------------------------

  // チャンネルを直接操作できるように、ロックをかける
  // 3 → 操作するチャンネルビット ( チャンネル1,2を操作する ）
  // 0 → 使用しない
	SND_LockChannel(3,0);

  // チャンネル0 セットアップ
	SND_SetupChannelPcm(
        0,                                    // チャンネル番号
        SND_WAVE_FORMAT_PCM16,                // 波形データフォーマット
        (void*)p_wk->audioSampleBufferLeft, // 波形データ先頭アドレス
        SND_CHANNEL_LOOP_REPEAT,              // ループ再生設定
        0,                                    // ループ開始位置
        (int)(p_wk->soundBufferNbPackets*nbSamplePerAudioPacket/2),// ループ長
        127,                                  // ボリューム
        SND_CHANNEL_DATASHIFT_NONE,           // データシフト設定
        (int)soundClockTick,                  // タイマー値。SND_TIMER_CLOCK/rate[hZ]
        0);                                   // パン値。0(左),127(右)
        
  // チャンネル1 セットアップ
  SND_SetupChannelPcm(
        1,
        SND_WAVE_FORMAT_PCM16,
        (void*)p_wk->audioSampleBufferRight,
        SND_CHANNEL_LOOP_REPEAT,
        0,
        (int)(p_wk->soundBufferNbPackets*nbSamplePerAudioPacket/2),
        127,
        SND_CHANNEL_DATASHIFT_NONE,
        (int)soundClockTick,
        127);   

  // サウンドアラームをセットアップ
  SND_SetupAlarm(
		0,                            // アラーム番号
		alarmClockTick,               // 最初にアラームが発動するまでのチックカウント
		alarmClockTick,               // 周期的にアラームを発動させる間隔
		MO_SOUND_CallBackFunc,        // アラーム発動時コールバック
    0);                           // コールバックに渡される引数

  // セットアップしたチャンネルとアラームのスタート
  SND_StartTimer(
      3,    // チャンネルビットマスク
      0,    // キャプチャビットマスク
      1,    // アラームビットマスク
      0);   // 使用しない

  
	// この関数で予約された、コマンド発行
	SND_FlushCommand(SND_COMMAND_BLOCK);
}
//----------------------------------------------------------------------------
/**
 *	@brief  サウンド停止
 */
//-----------------------------------------------------------------------------
static void MO_SOUND_StopSound( void )
{
  // タイマーをストップする
	SND_StopTimer(
      3,    // チャンネルビットマスク
      0,    // キャプチャビットマスク
      1,    // アラームビットマスク
      0);   // 使用しない


  // チャンネルのロックを解除する チャンネル0,1
	SND_UnlockChannel(3,0);		
	
	// この関数で予約された、コマンド発行
	SND_FlushCommand(SND_COMMAND_BLOCK);
}


//=============================================================================
/**
 *        DS蓋開閉時コールバック関数
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  スリープ起動時に呼ばれるコールバック関数
 *
 *	@param	*p_wk 呼ばれる際に渡すワーク
 */
//-----------------------------------------------------------------------------
static void MO_SleepGo_Func( void *p_wk )
{
  MO_WORK  *p_mo_wk = p_wk;

  // Sound OFF
  if( p_mo_wk->soundStarted == TRUE )
  {
    MO_SOUND_StopSound();
  }

  // flag ON
  p_mo_wk->coverClosed = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スリープ復帰時に呼ばれるコールバック関数
 *
 *	@param	*p_wk 呼ばれる際に渡すワーク
 */
//-----------------------------------------------------------------------------
static void MO_SleepRelease_Func( void *p_wk )
{
  MO_WORK  *p_mo_wk = p_wk;

  // SOUNDをON
  if( p_mo_wk->soundStarted == TRUE )
  {
    u32 nbSamplePerAudioPacket = MO_GetNbSamplePerAudioPacket(p_mo_wk->handle);
    // sound buffers 0 クリア
    GFL_STD_MemClear(p_mo_wk->audioSampleBufferLeft,p_mo_wk->soundBufferNbPackets*nbSamplePerAudioPacket*2);
    GFL_STD_MemClear(p_mo_wk->audioSampleBufferRight,p_mo_wk->soundBufferNbPackets*nbSamplePerAudioPacket*2);
    DC_StoreAll();


    if (p_mo_wk->nbSoundPacketPushed > g_SoundPacketStreamed)
    {
      p_mo_wk->nbSoundPacketPushed = p_mo_wk->nbSoundPacketPushed - g_SoundPacketStreamed;
    }else{
      p_mo_wk->nbSoundPacketPushed = 0;
    }

    g_SoundPacketStreamed = 0;
    p_mo_wk->audioSampleBufferIndex = (u32)p_mo_wk->nbSoundPacketPushed * nbSamplePerAudioPacket; 	
    if (p_mo_wk->audioSampleBufferIndex == p_mo_wk->soundBufferNbPackets * nbSamplePerAudioPacket)
    {
      p_mo_wk->audioSampleBufferIndex = 0;
    }

    MO_SOUND_StartSound( p_mo_wk, MO_GetAudioFrequency(p_mo_wk->handle),MO_GetNbSamplePerAudioPacket(p_mo_wk->handle));
    ICHI_Printf("Sound ReStart \n");
  }

  // flag OFF
  p_mo_wk->coverClosed = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  バックライトの操作
 *
 *	@param	sw  ON/OFF指定
 */
//-----------------------------------------------------------------------------
static void MO_Sleep_SetBackLight( PMBackLightSwitch sw )
{
  enum {
    SPI_RETRY_MAX = 1000,
  };
  int i;

  //SPIがBUSYの場合を考えてループ
  for ( i = 0; i < SPI_RETRY_MAX; i ++ )
  {
    if ( PM_SetBackLight( PM_LCD_ALL, sw ) == PM_RESULT_SUCCESS )
    {
      return;
    }
  }
}
