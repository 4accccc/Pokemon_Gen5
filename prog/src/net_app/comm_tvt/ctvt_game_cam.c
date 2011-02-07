//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		ctvt_game_cam.c
 *	@brief  ゲームに使うカメラ画像撮影
 *	@author		Katsuhiko=Ichiraku
 *	@data		2011.01.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// lib
#include <gflib.h>

// constant
#include "system/main.h"	//HEAPID
#include "system/gfl_use.h"
#include "gamesystem/msgspeed.h"

// archive
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "tvt_camera.naix"

// net
#include "net/network_define.h"

// msg
#include "msg/msg_comm_tvt.h"
// print
#include "print/str_tool.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_msg.h"

// Module
#include "system/camera_system.h"
#include "system/bmp_winframe.h"
#include "system/wipe.h"

// mine
#include "ctvt_game_cam.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	パレット
//=====================================
enum
{
  // メインBG
  PLT_BG_BACK_M     = 0,    // 背景
  PLT_BG_WINFRAME_M = 14,   // WINDOWフレーム
  PLT_BG_FONT_M     = 15,   // フォント色

  // メインOBJ
  PLT_OBJ_M         = 0,    // 共通OBJ 3本
  
  // サブBG
  PLT_BG_S          = 0,    //
  // サブOBJ
  PLT_OBJ_S         = 0,    // 共通OBJ 3本
  
};

//-------------------------------------
///	セルリソースインデックス
//=====================================
typedef enum
{
  RESID_OBJ_COMMON_PLT,
  RESID_OBJ_COMMON_CHR,
  RESID_OBJ_COMMON_CEL,

  RESID_OBJ_MAX,    // 最大値
} RESID;


//-------------------------------------
///	セルワークインデックス
//=====================================
typedef enum
{
  CLWKID_FRAME_M,   // 写真フレーム(上画面)
  CLWKID_FRAME_S,   // 写真フレーム(下画面)
  CLWKID_COUNT,     // カウントダウン

  CLWKID_CHARA_TOP, // NITORO時のキャラクタ用フレーム
  CLWKID_CHARA_END = CLWKID_CHARA_TOP + CTVT_CHARACTER_NUM,

  CLWKID_MAX,   // 最大値
} CLWKID;


//-------------------------------------
///	MSGWND
//=====================================
// フレームを読み込んだキャラ位置
#define MSGWND_FRAME_CHR  (1)
// BMPWIN
#define MSGWND_BMPWIN_X (1)
#define MSGWND_BMPWIN_Y (19)
#define MSGWND_BMPWIN_W (30)
#define MSGWND_BMPWIN_H (4)

//-------------------------------------
///	通信用
//=====================================
#define CAMERA_INIT_SYNC  (30)    // カメラ開始同期
#define CAMERA_END_SYNC   (31)    // カメラ終了同期
#define CAMERA_SEND_SYNC  (32)    // 送信同期

//-------------------------------------
///	TWL時の写真表示用
//=====================================
#define INFO_W  (72)  // 1枚目から2枚までの幅
#define INFO_X  (24)  // 最初の1枚のX座標
#define INFO_Y  (20)  // Y座標

//-------------------------------------
///	NITRO時のルーレット用
//=====================================
#define CHAR_NUM (4)  // キャラクターの数
#define ROULETET_WAIT (5)       // 次のフレームに移動するまでの待ちフレーム数
#define ROULETET_WAIT_MAX (60)  // 次のフレームに移動するまでの待ちフレーム数MAX
#define ROULETTE_TIME (720)     // ルーレットの回るフレーム数 12秒
#define ROULETTE_DIV  (25)      // 何フレームごとにウェイトが加算されるか

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
/// シーケンス管理
//=====================================
typedef struct _SEQ_WORK SEQ_WORK;  //関数型作るため前方宣言
typedef void (*SEQ_FUNCTION)( SEQ_WORK *p_wk, int *p_seq, void *p_param );
struct _SEQ_WORK
{
  SEQ_FUNCTION  seq_function;   //実行中のシーケンス関数
  BOOL is_end;                  //シーケンスシステム終了フラグ
  int seq;                      //実行中のシーケンス関数の中のシーケンス
  void *p_param;                //実行中のシーケンス関数に渡すワーク
};


//-------------------------------------
///	OBJリソース管理
//=====================================
typedef struct {
  u32         res[RESID_OBJ_MAX];   // リソース
  GFL_CLWK    *p_clwk[CLWKID_MAX];  // CLWK
} OBJ_WORK;

//-------------------------------------
///	メッセージウィンドウ
//=====================================
typedef struct {
  GFL_MSGDATA       *p_msg;     // メッセージデータ
  GFL_FONT          *p_font;    // フォント
  PRINT_UTIL        util;       // 文字描画
  PRINT_QUE         *p_que;     // 文字キュー
  GFL_BMPWIN*       p_bmpwin;   // BMPWIN
  WORDSET           *p_word;    // 単語
  STRBUF*           p_strbuf;   // 文字バッファ
  STRBUF*           p_strexpand;// 文字バッファ
  u16               clear_chr;  // クリアキャラ
  u16               dummy;
} MSGWND_WORK;

//-------------------------------------
///	プリントストリーム
//=====================================
typedef struct {
  PRINT_STREAM      *p_stream;
  GFL_TCBLSYS       *p_tcblsys;
  SEQ_FUNCTION      next_seq_function;
  u16               wait_max;
  u16               wait_count;
} PS_WORK;

//-------------------------------------
///	カメラ
//=====================================
typedef struct {
  void  *scr_buf;   // 0x18000 -> 256x192x2
  BOOL  is_update;  // カメラキャプチャが更新されたか
  u8    cnt;        // 撮影枚数 3枚
  CAMERA_SYSTEM_WORK  *p_cam_sys;
} CAMERA_WORK;

//-------------------------------------
///	ルーレット
//=====================================
typedef struct {
  void  *pic_buf[CHAR_NUM]; // キャラクターBMP
  u8    select;       // どのフレームが光ってるか0～4
  u16   wait;         // 1回フレームが移動しての待ち時間
  BOOL  move_frame;   // フレーム移動フラグ
  u16   frame_cnt;    // 経過フレーム
} ROULETTE_WORK;


//-------------------------------------
///	通信
//=====================================
typedef struct {
  HEAPID  heapID;

  void*   cap_buf;        // 送信用バッファ 64x64x2
  u8      cap_buf_bit;    // バッファ管理用
  u8      cap_cnt[ CTVT_MEMBER_NUM-1 ]; // 受信枚数カウンタ

  u8      cap_cnt_base;   // 受信枚数カウンタ基準
  BOOL    can_next;       // 次の写真撮影にいってよいか
  u8      seq;
  BOOL    is_end;         // すべて受信完了したか
  BOOL    is_init;        // 初期化が完了したか(カメラ同期をとる)
} COMM_WORK;

//-------------------------------------
///	メインワーク
//=====================================
struct _CTVT_GAME_CAM_WORK{
  // ヒープID
  HEAPID        heapID; 
  // VBlank
  GFL_TCB       *vBlankTcb;

  // グラフィックシステム
  CTVT_GAME_CAM_GRAPHIC_WORK *p_grh;

  // シーケンス
  SEQ_WORK      seq;

  // OBJリソース
  OBJ_WORK      obj;

  // カメラワーク
  CAMERA_WORK   camera;   // TWL起動のみ使用

  // ルーレットワーク
  ROULETTE_WORK roulette; // NITRO起動のみ使用

  // 通信ワーク
  COMM_WORK     comm;     // 低優先度通信

  // 上画面ウィンドウ
  MSGWND_WORK   msgwnd;

  // 上画面ウィンドウ プリントストリーム
  PS_WORK       ps;

  //共通で使うフォント
  GFL_FONT      *p_font;

  //共通で使うキュー
  PRINT_QUE     *p_que;

  //共通で使うメッセージ
  GFL_MSGDATA   *p_msg;

  //共通で使う単語
  WORDSET       *p_word;

  // 引数
  CTVT_GAME_CAM_PARAM   *p_param;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

//-------------------------------------
///	BGリソース
//=====================================
static void BG_Init( HEAPID heapID );
static void BG_Exit( void );

//-------------------------------------
///	OBJリソース
//=====================================
static void OBJ_Init( OBJ_WORK *p_wk, GFL_CLUNIT *p_clunit, HEAPID heapID );
static void OBJ_Exit( OBJ_WORK *p_wk );
static GFL_CLWK *OBJ_GetClwk( const OBJ_WORK *cp_wk, CLWKID clwkID );

//-------------------------------------
///	MSGWND
//=====================================
static void MSGWND_Init( MSGWND_WORK *p_wk, GFL_FONT *p_font, GFL_MSGDATA *p_msg, PRINT_QUE *p_que, WORDSET *p_word, HEAPID heapID );
static void MGSWND_Exit( MSGWND_WORK *p_wk );
static void MSGWND_Print( MSGWND_WORK *p_wk, u32 msgID );
static void MSGWND_PrintNumber( MSGWND_WORK *p_wk, u8 number, u32 msgID );
static BOOL MSGWND_PrintMain( MSGWND_WORK* p_wk );

//-------------------------------------
///	PS(PrintStream)
//=====================================
static void PS_Init( PS_WORK *p_wk, HEAPID heapID );
static void PS_Exit( PS_WORK *p_wk );
static void PS_Print( PS_WORK *p_wk, MSGWND_WORK *p_msgwnd, SEQ_FUNCTION next_seq_function, u32 msgID, HEAPID heapID );
static void PS_PrintNumber( PS_WORK *p_wk, MSGWND_WORK *p_msgwnd, SEQ_FUNCTION next_seq_function, u8 number, u32 msgID, HEAPID heapID );

//-------------------------------------
///	INFO 上画面写真
//=====================================
static void INFO_AddBmp( u16 posx, u16 posy, u16 sizex, u16 sizey, const u16 *p_adrs );

//-------------------------------------
///	CAMERA  TWL専用
//=====================================
static void CAM_Init( CAMERA_WORK *p_wk, CAMERA_SYSTEM_WORK *p_cam_sys, HEAPID heapID );
static void CAM_Exit( CAMERA_WORK *p_wk );
static void CAM_CaptureScaling( CAMERA_WORK *p_wk, void* dst, HEAPID heapID );
static void CAM_CaptureSave( CAMERA_WORK *p_wk, void* dst );
static void CAM_CaptureStart( CAMERA_WORK *p_wk );
static void CAM_CaptureStop( CAMERA_WORK *p_wk );
static void CAM_PlayShutterSe( CAMERA_WORK *p_wk );
static const BOOL CAM_IsPlayShutterSe( CAMERA_WORK *p_wk );

//-------------------------------------
///	ROULETTE NITRO専用
//=====================================
static void ROU_Init( ROULETTE_WORK *p_wk, HEAPID heapID );
static void ROU_Exit( ROULETTE_WORK *p_wk );
static void ROU_Main( ROULETTE_WORK *p_wk, OBJ_WORK *p_obj );
static void ROU_AddBmp( u16 posx, u16 posy, u16 sizex, u16 sizey, const u16 *p_adrs );
static void ROU_WaitSet( ROULETTE_WORK *p_wk, const u16 wait );
static BOOL ROU_GetStopFlg( ROULETTE_WORK *p_wk );

//-------------------------------------
///	COMM
//=====================================
static void COMM_Init( COMM_WORK *p_wk, CTVT_GAME_CAM_MEMBER_WORK member[], BOOL isTwl, HEAPID heapID );
static void COMM_Exit( COMM_WORK *p_wk, BOOL isTwl );
static void COMM_Main( COMM_WORK *p_wk, CTVT_GAME_CAM_MEMBER_WORK member[] );
static void COMM_SendCapture( COMM_WORK *p_wk );
static BOOL COMM_CanNext( const COMM_WORK *p_wk );
static void* COMM_GetSendBuf( const COMM_WORK *p_wk );
static BOOL COMM_IsEnd( const COMM_WORK *p_wk );
static void COMM_CaptureSave( COMM_WORK *p_wk, void* dst );

//-------------------------------------
/// SEQ
//=====================================
static void SEQ_Init( SEQ_WORK *p_wk, void *p_param, SEQ_FUNCTION seq_function );
static void SEQ_Exit( SEQ_WORK *p_wk );
static void SEQ_Main( SEQ_WORK *p_wk );
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk );
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function );
static void SEQ_End( SEQ_WORK *p_wk );

//-------------------------------------
///	SEQFUNC
//=====================================
static void SEQFUNC_End( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );

// 通信同期
static void SEQFUNC_SYNC_Init( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_SYNC_End( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );

// PrintStream
static void SEQFUNC_PrintStream( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );

// TWL
static void SEQFUNC_TWL_Init( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_TWL_Count( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_TWL_MsgWait( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_TWL_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_TWL_Wait( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
// カメラキャプチャコールバック
static void cap_call_back( void *captureArea, void *userWork );

// NITRO
static void SEQFUNC_NITRO_Init( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_NITRO_Trans( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_NITRO_Start( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_NITRO_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_NITRO_Wait( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );


static void VBlankFunc( GFL_TCB *p_tcb, void *p_work );

//=============================================================================
/**
 *    外部参照
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  カメラ撮影  初期化
 *
 *	@param	CTVT_GAME_CAM_WORK *p_wk  ワーク
 *	@param	*p_param                  パラメータ
 *	@param	heapID                    ヒープID
 *
 *	@return 初期化完了でTRUE
 */
//-----------------------------------------------------------------------------
CTVT_GAME_CAM_WORK* CTVT_GAME_CAM_Init( CTVT_GAME_CAM_PARAM *p_param, HEAPID heapID )
{
  CTVT_GAME_CAM_WORK *p_wk;
  // ワークメモリ作成
  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(CTVT_GAME_CAM_WORK) );

  // 引数パラメータ
  p_wk->p_param = p_param;
  p_wk->heapID  = heapID;

  // 共通モジュールの作成
  p_wk->p_font  = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
  p_wk->p_que   = PRINTSYS_QUE_CreateEx( 1024*2, heapID );
  p_wk->p_msg   = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_comm_tvt_dat, heapID );
  p_wk->p_word  = WORDSET_Create( heapID );

  // グラフィック設定
  p_wk->p_grh = CTVT_GAME_CAM_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, heapID );
  
  // リソース
  BG_Init( heapID );
  {
    GFL_CLUNIT *p_clunit = CTVT_GAME_CAM_GRAPHIC_GetClunit( p_wk->p_grh );
    OBJ_Init( &p_wk->obj, p_clunit, heapID );
  }

  // モジュール作成
  SEQ_Init( &p_wk->seq, p_wk, SEQFUNC_SYNC_Init );
  MSGWND_Init( &p_wk->msgwnd, p_wk->p_font, p_wk->p_msg, p_wk->p_que, p_wk->p_word, heapID );
  PS_Init( &p_wk->ps, heapID );
  COMM_Init( &p_wk->comm, p_param->member, p_param->isTwl, heapID );

  // VBlnak
  // 本来はグラフィックの中にあるが、
  // カメラキャプチャを転送する必要があるため外にだした
  p_wk->vBlankTcb = GFUser_VIntr_CreateTCB( VBlankFunc , p_wk , 1 );

  // 
  if( GFL_NET_IsInit() )
  { 
    GFL_NET_ReloadIconTopOrBottom( TRUE, heapID );
  }

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ撮影  メイン
 *
 *	@param	CTVT_GAME_CAM_WORK *p_wk ワーク
 *
 *	@return 終了時:TRUE
 */
//-----------------------------------------------------------------------------
BOOL CTVT_GAME_CAM_Main( CTVT_GAME_CAM_WORK *p_wk )
{
	// シーケンス
	SEQ_Main( &p_wk->seq );

  // 通信
  COMM_Main( &p_wk->comm, p_wk->p_param->member );

  // 描画
  CTVT_GAME_CAM_GRAPHIC_2D_Draw( p_wk->p_grh );

  // プリント
  PRINTSYS_QUE_Main( p_wk->p_que );

  // 終了
  if( SEQ_IsEnd( &p_wk->seq ) && COMM_IsEnd(&p_wk->comm) )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ撮影  終了
 *
 *	@param	CTVT_GAME_CAM_WORK *p_wk ワーク
 *
 *	@return 終了完了でTRUE
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CAM_Exit( CTVT_GAME_CAM_WORK *p_wk )
{
  GFL_TCB_DeleteTask( p_wk->vBlankTcb );

  // モジュール破棄
  COMM_Exit( &p_wk->comm, p_wk->p_param->isTwl );
  PS_Exit( &p_wk->ps );
  MGSWND_Exit( &p_wk->msgwnd );
  SEQ_Exit( &p_wk->seq );

  // グラフィック破棄
  OBJ_Exit( &p_wk->obj );
  BG_Exit();

  CTVT_GAME_CAM_GRAPHIC_Exit( p_wk->p_grh );

  // 共通モジュールの破棄
  WORDSET_Delete( p_wk->p_word );
  GFL_MSG_Delete( p_wk->p_msg );
  PRINTSYS_QUE_Clear( p_wk->p_que );
  PRINTSYS_QUE_Delete( p_wk->p_que );
  GFL_FONT_Delete( p_wk->p_font );

  // メインワーク破棄
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ撮影  パラメータ作成
 *
 *	@param	HEAPID heapID ヒープID
 *	@param	isTwl         TWL起動か
 *	@param	*p_cap_buf    受信したキャプチャ保存アドレス
 *	@param	select_chara  選択するキャラクターID
 *	@param	*p_cam_sys    カメラシステムへのポインタ
 *
 *	@return p_param 作成したパラメータ
 */
//-----------------------------------------------------------------------------
CTVT_GAME_CAM_PARAM *CTVT_GAME_CAM_AllocParam( HEAPID heapID, BOOL isTwl, u8 select_chara, CTVT_GAME_CAM_MEMBER_WORK p_member[], CAMERA_SYSTEM_WORK *p_cam_sys )
{
  u8 i, j;
  CTVT_GAME_CAM_PARAM *p_param;
  ARCDATID  dat_tbl[4] = {
    NARC_tvt_camera_char1_1_ntft,
    NARC_tvt_camera_char2_1_ntft,
    NARC_tvt_camera_char3_1_ntft,
    NARC_tvt_camera_char4_1_ntft,
  };

  p_param = GFL_HEAP_AllocClearMemory( heapID, sizeof(CTVT_GAME_CAM_PARAM) );
  p_param->isTwl        = isTwl;
  p_param->select_chara = select_chara;
  p_param->p_cam_sys    = p_cam_sys;


  // TWLはキャプチャ用のメモリ確保
  // NITROはキャラBMPを読み込む
  //
  // 自分
  if( isTwl == TRUE )
  {
    for( i=0; i<CAPTURE_NUM; i++ )
    {
      p_param->p_cap[i] = GFL_NET_Align32Alloc( heapID, CAPTURE_SIZE);
    }
  }
  else
  {
    for( i=0; i<CAPTURE_NUM; i++ )
    {
      p_param->p_cap[i] = GFL_ARC_UTIL_Load( ARCID_TVT_CAMERA, dat_tbl[select_chara]+i, FALSE, heapID );
    }
  }
  // メンバー
  {
    for( i=0; i<CTVT_MEMBER_NUM-1; i++ )
    {
      p_param->member[i] = p_member[i];

      for( j=0; j<CAPTURE_NUM; j++  )
      {
        p_param->member[i].p_cap_buf[j] = p_member[i].p_cap_buf[j];
      }
    }
  }
#if 0
  {
    u8 i, j;
    for( i=0; i<3; i++)
    {
      ICHI_Printf("i %d netID %d isTWL %d \n", i, p_member[i].netID, p_member[i].isTwl );
      for( j=0; j<3; j++ )
      {
        ICHI_Printf("j %d addrs %x \n ", j, p_member[i].p_cap_buf[j] );
      }
    }
  }
#endif

  return p_param;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ撮影  パラメータ破棄
 *
 *	@param	CTVT_GAME_CAM_PARAM *p_param
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CAM_FreeParam( CTVT_GAME_CAM_PARAM *p_param)
{
  u8 i;
  if( p_param->isTwl == TRUE )
  {
    for( i=0; i<CAPTURE_NUM; i++ )
    {
      GFL_NET_Align32Free( p_param->p_cap[i] );
    }
  }
  else
  {
    for( i=0; i<CAPTURE_NUM; i++ )
    {
      GFL_HEAP_FreeMemory( p_param->p_cap[i] );
    }
  }
  GFL_HEAP_FreeMemory( p_param );
}


//=============================================================================
/**
 *    BGリソース
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  BGリソース読み込み
 *
 *	@param	HEAPID heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void BG_Init( HEAPID heapID )
{
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_TVT_CAMERA, heapID );

    // 上画面
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_tvt_camera_bg_NCLR,
        PALTYPE_MAIN_BG, PLT_BG_BACK_M*0x20, 0, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_tvt_camera_bg_upper_NCGR,
        BG_FRAME_BACK_M, 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_tvt_camera_bg_upper_NSCR,
        BG_FRAME_BACK_M, 0, 0, FALSE, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }

  { 
    ARCHANDLE * p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, heapID );

    //上画面フォントパレット
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_MAIN_BG, PLT_BG_FONT_M*0x20, 0x20, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }

  {
    //上画面テキスト枠
    GFL_BG_FillCharacter( BG_FRAME_TEXT_M, 0, 1, 0 );
    BmpWinFrame_GraphicSet(
        BG_FRAME_TEXT_M, MSGWND_FRAME_CHR, PLT_BG_WINFRAME_M, MENU_TYPE_SYSTEM, heapID );
  }

  // ダイレクトカラーの設定
  {
    // カメラキャプチャのBG or キャラクターのBG
    // gflibが対応していないので直書き
    G2S_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
    GFL_STD_MemClear( G2S_GetBG3ScrPtr(), CTVT_BUFFER_SCR_SIZE );

    // 上画面撮った写真表示用
    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x14000);
    GFL_STD_MemClear( G2_GetBG3ScrPtr(), CTVT_BUFFER_SCR_SIZE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGリソース破棄
 */
//-----------------------------------------------------------------------------
static void BG_Exit( void )
{
  GFL_BG_FillCharacterRelease( BG_FRAME_TEXT_M, 1, 0 );
}

//=============================================================================
/**
 *    OBJリソース
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  OBJリソース読み込み
 *
 *	@param	OBJ_WORK *p_wk  OBJワーク
 *	@param	*p_clunit       ユニット
 *	@param	heapID          ヒープID
 */
//-----------------------------------------------------------------------------
static void OBJ_Init( OBJ_WORK *p_wk, GFL_CLUNIT *p_clunit, HEAPID heapID )
{
  // リソース読み込み
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_TVT_CAMERA, heapID );

    p_wk->res[ RESID_OBJ_COMMON_PLT ] = GFL_CLGRP_PLTT_RegisterComp( p_handle,
        NARC_tvt_camera_obj_NCLR, CLSYS_DRAW_MAX, PLT_OBJ_M*0x20, heapID );
    p_wk->res[ RESID_OBJ_COMMON_CHR ] = GFL_CLGRP_CGR_Register( p_handle, 
        NARC_tvt_camera_obj_NCGR, FALSE, CLSYS_DRAW_MAX, heapID );
    p_wk->res[ RESID_OBJ_COMMON_CEL ] = GFL_CLGRP_CELLANIM_Register( p_handle, 
        NARC_tvt_camera_obj_NCER, NARC_tvt_camera_obj_NANR, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }

  // CLWK登録
  {
    int i;
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    // 写真フレーム(下画面)
    cldata.anmseq = 0;
    cldata.pos_x  = 128; 
    cldata.pos_y  = 96;
    cldata.bgpri  = 0;
    p_wk->p_clwk[ CLWKID_FRAME_S ] = GFL_CLACT_WK_Create( p_clunit,
        p_wk->res[ RESID_OBJ_COMMON_PLT ],
        p_wk->res[ RESID_OBJ_COMMON_CHR ],
        p_wk->res[ RESID_OBJ_COMMON_CEL ],
        &cldata, CLSYS_DEFREND_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[ CLWKID_FRAME_S ], FALSE );
 
    // 写真フレーム(上画面)
    cldata.anmseq = 1;
    cldata.pos_x  = 128; 
    cldata.pos_y  = 52; 
    cldata.bgpri  = 1;
    p_wk->p_clwk[ CLWKID_FRAME_M ] = GFL_CLACT_WK_Create( p_clunit,
        p_wk->res[ RESID_OBJ_COMMON_PLT ],
        p_wk->res[ RESID_OBJ_COMMON_CHR ],
        p_wk->res[ RESID_OBJ_COMMON_CEL ],
        &cldata, CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[ CLWKID_FRAME_M ], TRUE );

    // カウントダウン
    cldata.anmseq = 2;
    cldata.pos_x  = 128; 
    cldata.pos_y  = 96; 
    cldata.bgpri  = 0;
    p_wk->p_clwk[ CLWKID_COUNT ] = GFL_CLACT_WK_Create( p_clunit,
        p_wk->res[ RESID_OBJ_COMMON_PLT ],
        p_wk->res[ RESID_OBJ_COMMON_CHR ],
        p_wk->res[ RESID_OBJ_COMMON_CEL ],
        &cldata, CLSYS_DEFREND_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[ CLWKID_COUNT ], FALSE );
		GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[CLWKID_COUNT], TRUE );

    // キャラクタ用フレーム
    cldata.anmseq = 3;
    cldata.bgpri  = 0;
    {
      GFL_CLACTPOS pos[CHAR_NUM] = 
      {
        { 74, 50 },         // 左上
        { 74+118, 50 },     // 右上
        { 74+118, 50+94 },  // 右下
        { 74, 50+94 },      // 左下
      };

      for( i=CLWKID_CHARA_TOP; i<CLWKID_CHARA_END; i++ )
      {
        cldata.pos_x  = pos[ i-CLWKID_CHARA_TOP ].x;
        cldata.pos_y  = pos[ i-CLWKID_CHARA_TOP ].y;

        p_wk->p_clwk[ i ] = GFL_CLACT_WK_Create( p_clunit,
            p_wk->res[ RESID_OBJ_COMMON_PLT ],
            p_wk->res[ RESID_OBJ_COMMON_CHR ],
            p_wk->res[ RESID_OBJ_COMMON_CEL ],
            &cldata, CLSYS_DEFREND_SUB, heapID );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[ i ], FALSE );
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  OBJリソース破棄
 *
 *	@param	OBJ_WORK *p_wk  OBJワーク
 */
//-----------------------------------------------------------------------------
static void OBJ_Exit( OBJ_WORK *p_wk )
{
  // CLWK破棄
  {
    u8 i;
    for( i=0; i<CLWKID_MAX; i++ )
    {
      if( p_wk->p_clwk[i] )
      {
        GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
      }
    }
  }

  // リソース破棄
  {
    GFL_CLGRP_PLTT_Release( p_wk->res[RESID_OBJ_COMMON_PLT] );
    GFL_CLGRP_CGR_Release( p_wk->res[RESID_OBJ_COMMON_CHR] );
    GFL_CLGRP_CELLANIM_Release( p_wk->res[RESID_OBJ_COMMON_CEL] );
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  CLWK取得
 *
 *	@param	const OBJ_WORK *cp_wk OBJワーク
 *	@param	clwkID                CLWKインデックス
 *
 *	@return CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK *OBJ_GetClwk( const OBJ_WORK *cp_wk, CLWKID clwkID )
{
  return cp_wk->p_clwk[ clwkID ];
}

//=============================================================================
/**
 *    MSGWND
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージウィンドウ  初期化
 *
 *	@param	MSGWND_WORK* p_wk ワーク
 *	@param	*p_font           フォント
 *	@param	*p_msg            メッセージデータ
 *	@param	*p_que            キュー
 *	@param	*p_word           単語
 *	@param	heapID            ヒープID
 */
//-----------------------------------------------------------------------------
static void MSGWND_Init( MSGWND_WORK *p_wk, GFL_FONT *p_font, GFL_MSGDATA *p_msg, PRINT_QUE *p_que, WORDSET *p_word, HEAPID heapID )
{
  // 初期化
  GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );

  p_wk->clear_chr = 0xF;
  p_wk->p_font    = p_font;
  p_wk->p_msg     = p_msg;
  p_wk->p_que     = p_que;
  p_wk->p_word    = p_word;

  // バッファ作成
  p_wk->p_strbuf    = GFL_STR_CreateBuffer( (18*3*2+EOM_SIZE), heapID );
  p_wk->p_strexpand = GFL_STR_CreateBuffer( (18*3*2+EOM_SIZE), heapID );

  // BMPWIN作成
  p_wk->p_bmpwin  = GFL_BMPWIN_Create( BG_FRAME_TEXT_M, MSGWND_BMPWIN_X, MSGWND_BMPWIN_Y, MSGWND_BMPWIN_W, MSGWND_BMPWIN_H, PLT_BG_FONT_M, GFL_BMP_CHRAREA_GET_B );

  // BMPWINFRAME
  BmpWinFrame_Write( p_wk->p_bmpwin, WINDOW_TRANS_OFF, MSGWND_FRAME_CHR, PLT_BG_WINFRAME_M );

  // プリントキュー設定
  PRINT_UTIL_Setup( &p_wk->util, p_wk->p_bmpwin );

  // 転送
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_bmpwin ), p_wk->clear_chr );
  GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );
}


//----------------------------------------------------------------------------
/**
 *	@brief  メッセージウィンドウ  破棄
 *
 *	@param	MSGWND_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MGSWND_Exit( MSGWND_WORK *p_wk )
{
  // BMPWIN破棄
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );

  // バッファ破棄
  GFL_STR_DeleteBuffer( p_wk->p_strexpand );
  GFL_STR_DeleteBuffer( p_wk->p_strbuf );

  // クリア
  GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージウィンドウ  プリント開始
 *
 *	@param	MSGWND_WORK *p_wk ワーク
 *	@param	msgID             メッセージID
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void MSGWND_Print( MSGWND_WORK *p_wk, u32 msgID )
{
  // 描画残り待ち
  while( PRINTSYS_QUE_IsExistTarget( p_wk->p_que, GFL_BMPWIN_GetBmp(p_wk->p_bmpwin) ) )
  {
    PRINTSYS_QUE_Main( p_wk->p_que );
  }

  // クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_bmpwin ), p_wk->clear_chr );

  // 文字列作成
  GFL_MSG_GetString( p_wk->p_msg, msgID, p_wk->p_strbuf );

  // 文字列描画
  PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strbuf, p_wk->p_font );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージウィンドウ  数字入力
 *
 *	@param	MSGWND_WORK *p_wk
 *	@param	number
 *	@param	msgID 
 */
//-----------------------------------------------------------------------------
static void MSGWND_PrintNumber( MSGWND_WORK *p_wk, u8 number, u32 msgID )
{
  // 描画残り待ち
  while( PRINTSYS_QUE_IsExistTarget( p_wk->p_que, GFL_BMPWIN_GetBmp(p_wk->p_bmpwin) ) )
  {
    PRINTSYS_QUE_Main( p_wk->p_que );
  }

  // クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_bmpwin ), p_wk->clear_chr );

  // 文字列作成
  GFL_MSG_GetString( p_wk->p_msg, msgID, p_wk->p_strbuf );

  // 数字入力
  WORDSET_RegisterNumber( p_wk->p_word, 0, number, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  WORDSET_ExpandStr( p_wk->p_word, p_wk->p_strexpand, p_wk->p_strbuf );

  // 文字列描画
  PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strexpand, p_wk->p_font );
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージウィンドウ  QUEの描画待ち
 *
 *  @param  const MSGWND_WORK* cp_wk  ワーク
 *
 *  @return TRUEでストリーム終了  FALSEは処理中
 */
//-----------------------------------------------------------------------------
static BOOL MSGWND_PrintMain( MSGWND_WORK* p_wk )
{ 
  return PRINT_UTIL_Trans( &p_wk->util, p_wk->p_que );
}

//=============================================================================
/**
 *    PS(PrintStream)
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  PS  初期化
 *  @param  PS_WORK* p_wk ワーク
 *  @param  heapID        ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void PS_Init( PS_WORK *p_wk, HEAPID heapID )
{
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(PS_WORK) );

  // GFL_TCBLシステム
  p_wk->p_tcblsys = GFL_TCBL_Init( heapID, heapID, 2, 0 );
}

//----------------------------------------------------------------------------
/**
 *  @brief  PS  破棄
 *
 *  @param  PS_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void PS_Exit( PS_WORK *p_wk )
{
  // プリントストリーム
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
  }

  // GFL_TCBLシステム
  GFL_TCBL_Exit( p_wk->p_tcblsys );

  //クリア
  GFL_STD_MemClear( p_wk, sizeof(PS_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  PS  メッセージをプリントストリーム表示
 *
 *	@param	PS_WORK *p_wk     ワーク
 *	@param	*p_msgwnd         メッセージウィンドウワーク
 *	@param	next_seq_function 次シーケンス関数
 *	@param	msgID             メッセージID
 *	@param	heapID            ヒープID
 */
//-----------------------------------------------------------------------------
static void PS_Print( PS_WORK *p_wk, MSGWND_WORK *p_msgwnd, SEQ_FUNCTION next_seq_function, u32 msgID, HEAPID heapID )
{
  // 文字列作成
  GFL_MSG_GetString( p_msgwnd->p_msg, msgID, p_msgwnd->p_strbuf );

  // 設定をクリア
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
  }
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_msgwnd->p_bmpwin ), p_msgwnd->clear_chr );
	GFL_BMPWIN_TransVramCharacter( p_msgwnd->p_bmpwin );
  //GFL_STR_ClearBuffer( p_msgwnd->p_strbuf );

  // 文字描画関連の設定を行う
  // wait値を固定で0にしている
  p_wk->p_stream = PRINTSYS_PrintStream( p_msgwnd->p_bmpwin, 0, 0, p_msgwnd->p_strbuf,
      p_msgwnd->p_font, MSGSPEED_GetWait(),
      p_wk->p_tcblsys, 2, heapID, p_msgwnd->clear_chr );
	GFL_BMPWIN_MakeScreen( p_msgwnd->p_bmpwin );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(p_msgwnd->p_bmpwin) );


  PRINTSYS_PrintStream_StartTempSpeedMode( p_wk->p_stream, 0 );

  // その他設定
  p_wk->next_seq_function = next_seq_function;
  p_wk->wait_max  = 90;
  p_wk->wait_count= 0;
}


//----------------------------------------------------------------------------
/**
 *	@brief  PS  メッセージをプリントストリーム表示 数字付き
 *
 *	@param	PS_WORK *p_wk     ワーク
 *	@param	*p_msgwnd         メッセージウィンドウワーク
 *	@param	next_seq_function 次シーケンス関数
 *	@param	number            出したい数字
 *	@param	msgID             メッセージID
 *	@param	heapID            ヒープID
 */
//-----------------------------------------------------------------------------
static void PS_PrintNumber( PS_WORK *p_wk, MSGWND_WORK *p_msgwnd, SEQ_FUNCTION next_seq_function, u8 number, u32 msgID, HEAPID heapID )
{
  // 文字列作成
  GFL_MSG_GetString( p_msgwnd->p_msg, msgID, p_msgwnd->p_strbuf );

  // 数字入力
  WORDSET_RegisterNumber( p_msgwnd->p_word, 0, number, 1, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  WORDSET_ExpandStr( p_msgwnd->p_word, p_msgwnd->p_strexpand, p_msgwnd->p_strbuf );

  // 設定をクリア
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
  }
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_msgwnd->p_bmpwin ), p_msgwnd->clear_chr );
	GFL_BMPWIN_TransVramCharacter( p_msgwnd->p_bmpwin );

  // 文字描画関連の設定を行う
  // wait値を固定で0にしている
  p_wk->p_stream = PRINTSYS_PrintStream( p_msgwnd->p_bmpwin, 0, 0, p_msgwnd->p_strexpand,
      p_msgwnd->p_font, MSGSPEED_GetWait(),
      p_wk->p_tcblsys, 2, heapID, p_msgwnd->clear_chr );
	GFL_BMPWIN_MakeScreen( p_msgwnd->p_bmpwin );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(p_msgwnd->p_bmpwin) );

  PRINTSYS_PrintStream_StartTempSpeedMode( p_wk->p_stream, 0 );

  // その他設定
  p_wk->next_seq_function = next_seq_function;
  p_wk->wait_max  = 90;
  p_wk->wait_count= 0;
}

//=============================================================================
/**
 *    INFO  上画面写真
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  INFO  写真追加
 *
 *	@param	posx        X座標
 *	@param	posy        Y座標
 *	@param	sizex       幅
 *	@param	sizey       高さ
 *	@param	u8 *p_adrs  元データ
 */
//-----------------------------------------------------------------------------
static void INFO_AddBmp( u16 posx, u16 posy, u16 sizex, u16 sizey, const u16 *p_adrs )
{
  u8  iy;
  
  DC_FlushRange( p_adrs, (sizex * sizey * 2) );
  ICHI_Printf("Flush Range Size %x\n", (sizex * sizey * 2));

  // sizex * sizey 転送
  for( iy=0; iy<sizey; iy++ )
  {
    const u32 offset = (u32)posx*2 + ((iy+posy)*256*2);
    const u16 *src   = (u16*)( p_adrs + (sizex*iy));
    //ICHI_Printf("[%d] offset %x ", iy, offset);
    //ICHI_Printf("src %x \n", src);
    GX_LoadBG3Bmp( src, offset, sizex*2 );
  }
}

//=============================================================================
/**
 *    CAMERA
 *    TWL起動でしか使われない
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  カメラ  初期化
 *
 *	@param	CAMERA_WORK *p_wk ワーク
 *	@param	heapID            ヒープID
 */
//-----------------------------------------------------------------------------
static void CAM_Init( CAMERA_WORK *p_wk, CAMERA_SYSTEM_WORK *p_cam_sys, HEAPID heapID )
{
  // メモリ作成
  p_wk->scr_buf = GFL_NET_Align32Alloc( heapID, CTVT_BUFFER_SCR_SIZE );
  p_wk->is_update = FALSE;
  p_wk->cnt       = 0;
  p_wk->p_cam_sys = p_cam_sys;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ  終了
 *
 *	@param	CAMERA_WORK *p_wk ワーク
 *	@param	heapID            ヒープID
 */
//----------------------------------------------------------------------------
static void CAM_Exit( CAMERA_WORK *p_wk )
{
  // メモリ破棄
  GFL_NET_Align32Free( p_wk->scr_buf );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ  キャプチャ変換
 *	        流れ
 *	        scr_buf 256x192のキャプチャ画像
 *	        ↓
 *	        dst     128x128にトリミング
 *	        ↓
 *	        dst     64x64に圧縮
 *
 *	@param	CAMERA_WORK *p_wk   ワーク
 *	@param	dst                 変換後のキャプチャ
 */
//-----------------------------------------------------------------------------
static void CAM_CaptureScaling( CAMERA_WORK *p_wk, void* dst, HEAPID heapID )
{
  void *trimming_tmp;   // 128x128
  trimming_tmp = GFL_NET_Align32Alloc( heapID, TRIMMING_SIZE);

#if (defined(SDK_TWL))
  // DSP初期化
  {
    // 他のコンポーネントと共存できない
    // ↓はカメラの音で使用している
    CAMERA_SYS_DspExit( p_wk->p_cam_sys );

    // DSP_Graphicロード
    // DSPグラフィックを使用して、画像サイズを変更する
    // ※DSP_OpenStaticComponent*()を使うとプログラムの常駐サイズが増える
    {
      FSFile  file[1];

      DSP_OpenStaticComponentGraphics(file);
      if (!DSP_LoadGraphics(file, 0xFF, 0xFF))
      {
        GF_PANIC("can't allocate WRAM Slot");
      }
    }
  }
#endif

  // 128x128にトリミング
  {
    int   iy;
    const u16 capPosX = 64;   // トリミングX 256-128/2
    const u16 capPosY = 32;   // トリミングY 192-128/2
    const u16 capSizeX = CAPTURE_TRIMMING_W; // 128
    const u16 capSizeY = CAPTURE_TRIMMING_H; // 128

    for( iy=0; iy< CAPTURE_TRIMMING_H; iy++ )
    {
      const u32 capTopAdr = (u32)p_wk->scr_buf + (capPosY+iy)*256*2 + capPosX*2;
      const u32 bufTopAdr = (u32)trimming_tmp + capSizeX*iy*2;

      GFL_STD_MemCopy32( (void*)capTopAdr, (void*)bufTopAdr, capSizeX*2 );
    }
  }
  ICHI_Printf("scr_buf %x \n", p_wk->scr_buf);
  ICHI_Printf("trim_tmp %x \n", trimming_tmp);

#if (defined(SDK_TWL))
  // DSP を使用して64x64に圧縮
  {
    void  *src;   // 変換元
    BOOL  ret;
    f32  factx, facty;
    factx = DSP_CalcScalingFactorF32(CAPTURE_TRIMMING_W, OUTPUT_WIDTH);
    facty = DSP_CalcScalingFactorF32(CAPTURE_TRIMMING_H, OUTPUT_HEIGHT);
    ICHI_Printf("変換率 %x %x\n", factx, facty);

    src = trimming_tmp;

    // 変換
    ret = DSP_Scaling(src, dst, CAPTURE_TRIMMING_W, CAPTURE_TRIMMING_H, factx, facty, DSP_GRAPHICS_SCALING_MODE_BILINEAR);
    if( ret == FALSE )
    {
      OS_Printf("DSPグラフィック 正常に変換が行われませんでした。\n");
    }
  }
#endif

#if (defined(SDK_TWL))
  // DSP解放
  {
    // DSP Graphic アンロード
    DSP_UnloadGraphics( );

    // DSP G.711をロードし直す
    CAMERA_SYS_DspInit( p_wk->p_cam_sys );
  }
#endif

  GFL_NET_Align32Free( trimming_tmp );
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分のキャプチャを保存する
 *
 *	@param	CAMERA_WORK *p_wk ワーク
 *	@param	dst               保存先
 */
//-----------------------------------------------------------------------------
static void CAM_CaptureSave( CAMERA_WORK *p_wk, void* dst )
{
  GFL_STD_MemCopy32( p_wk->scr_buf, dst, CAPTURE_SIZE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  キャプチャースタート
 *
 *	@param	CAMERA_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CAM_CaptureStart( CAMERA_WORK *p_wk )
{
  CAMERA_SYS_StartCapture( p_wk->p_cam_sys );
}

//----------------------------------------------------------------------------
/**
 *	@brief  キャプチャーストップ
 *
 *	@param	CAMERA_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CAM_CaptureStop( CAMERA_WORK *p_wk )
{
  CAMERA_SYS_StopCapture( p_wk->p_cam_sys );
}

//----------------------------------------------------------------------------
/**
 *	@brief  シャッター音再生
 *
 *	@param	CAMERA_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CAM_PlayShutterSe( CAMERA_WORK *p_wk )
{
  CAMERA_SYS_PlayShutterSe( p_wk->p_cam_sys );
}

//----------------------------------------------------------------------------
/**
 *	@brief  シャッター音再生中かチェック
 *
 *	@param	CAMERA_WORK *p_wk ワーク
 *
 *	@return TRUEで再生中
 */
//-----------------------------------------------------------------------------
static const BOOL CAM_IsPlayShutterSe( CAMERA_WORK *p_wk )
{
  return CAMERA_SYS_IsPlayShutterSe( p_wk->p_cam_sys );
}

//=============================================================================
/**
 *  ROULETTE NITRO専用
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ROULETTE  初期化
 *
 *	@param	ROULETTE_WORK *p_wk   ワーク
 *	@param	heapID        heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void ROU_Init( ROULETTE_WORK *p_wk, HEAPID heapID )
{
  // キャラクター１番目の顔BMPを読み込み
  ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_TVT_CAMERA, heapID );

  p_wk->pic_buf[0] = GFL_ARCHDL_UTIL_Load( arcHandle, NARC_tvt_camera_char1_1_ntft, FALSE, heapID );
  p_wk->pic_buf[1] = GFL_ARCHDL_UTIL_Load( arcHandle, NARC_tvt_camera_char2_1_ntft, FALSE, heapID );
  p_wk->pic_buf[2] = GFL_ARCHDL_UTIL_Load( arcHandle, NARC_tvt_camera_char3_1_ntft, FALSE, heapID );
  p_wk->pic_buf[3] = GFL_ARCHDL_UTIL_Load( arcHandle, NARC_tvt_camera_char4_1_ntft, FALSE, heapID );

  GFL_ARC_CloseDataHandle( arcHandle );

#if 0
  u8 i;
  for( i=0; i<CHAR_NUM; i++ )
  {
    void *tmp_buf;
    u16 pos_x  = (74-32) + ( 118*( i%2 ) );
    u16 pos_y  = (50-32) + ( 94*( i/2 ) );

    tmp_buf = GFL_ARC_UTIL_Load( ARCID_TVT_CAMERA, NARC_tvt_camera_char1_1_ntft+(i*3), FALSE, heapID );
    ROU_AddBmp( pos_x, pos_y, 64, 64, tmp_buf );

    GFL_HEAP_FreeMemory( tmp_buf );
  }
#endif

  p_wk->select = 0;
  p_wk->wait   = 0;
  p_wk->move_frame= FALSE;
  p_wk->frame_cnt = 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ROULETTE  破棄
 *
 *	@param	ROULETTE_WORK *p_wk   ワーク
 */
//-----------------------------------------------------------------------------
static void ROU_Exit( ROULETTE_WORK *p_wk )
{
  u8  i;
  
  for( i=0; i<CHAR_NUM; i++ )
  {
    GFL_HEAP_FreeMemory( p_wk->pic_buf[i] );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  ROULETTE  メイン
 *
 *	@param	ROULETTE_WORK *p_wk ワーク 
 */
//-----------------------------------------------------------------------------
static void ROU_Main( ROULETTE_WORK *p_wk, OBJ_WORK *p_obj )
{
  if( p_wk->move_frame == TRUE )
  {
    // 待ち時間
    if( p_wk->wait > 0 )
    {
      p_wk->wait--;
    }
    if( p_wk->wait == 0)
    {
      u8  clwkID = CLWKID_CHARA_TOP + p_wk->select;
      // 前のフレームは色を戻す
      GFL_CLACT_WK_SetPlttOffs( OBJ_GetClwk( p_obj, clwkID ), 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );
      // 次のフレームは色を変える
      p_wk->select++;
      p_wk->select %= CHAR_NUM;
      clwkID = CLWKID_CHARA_TOP + p_wk->select;
      GFL_CLACT_WK_SetPlttOffs( OBJ_GetClwk( p_obj, clwkID ), 1, CLWK_PLTTOFFS_MODE_PLTT_TOP );

      p_wk->move_frame = FALSE;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ROU  写真追加
 *
 *	@param	posx        X座標
 *	@param	posy        Y座標
 *	@param	sizex       幅
 *	@param	sizey       高さ
 *	@param	u8 *p_adrs  元データ
 */
//-----------------------------------------------------------------------------
static void ROU_AddBmp( u16 posx, u16 posy, u16 sizex, u16 sizey, const u16 *p_adrs )
{
  u8  iy;
  
  DC_FlushRange( p_adrs, (sizex * sizey * 2) );
  ICHI_Printf("Flush Range Size %x\n", (sizex * sizey * 2));

  // sizex * sizey 転送
  for( iy=0; iy<sizey; iy++ )
  {
    const u32 offset = (u32)posx*2 + ((iy+posy)*256*2);
    const u16 *src   = (u16*)( p_adrs + (sizex*iy));
    //ICHI_Printf("[%d] offset %x ", iy, offset);
    //ICHI_Printf("src %x \n", src);
    GXS_LoadBG3Bmp( src, offset, sizex*2 );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ROULETTE  ウェイト設定
 *
 *	@param	ROULETTE_WORK *p_wk
 *	@param	u16 wait ウェイト値 フレーム
 */
//-----------------------------------------------------------------------------
static void ROU_WaitSet( ROULETTE_WORK *p_wk, const u16 wait )
{
  p_wk->wait = wait;
  p_wk->move_frame = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ROULETTE  移動フラグ取得
 *
 *	@param	ROULETTE_WORK *p_wk 
 *
 *	@return TRUEで移動途中
 */
//-----------------------------------------------------------------------------
static BOOL ROU_GetStopFlg( ROULETTE_WORK *p_wk )
{
  return p_wk->move_frame;
}

//=============================================================================
/**
 *    COMM
 *    低優先度通信 メインの裏で自動受信のイメージ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  通信  初期化
 *
 *	@param	COMM_WORK *p_wk ワーク
 *	@param	heapID          ヒープID
 */
//-----------------------------------------------------------------------------
static void COMM_Init( COMM_WORK *p_wk, CTVT_GAME_CAM_MEMBER_WORK member[], BOOL isTwl, HEAPID heapID )
{
  // ワーククリア
  GFL_STD_MemClear( p_wk, sizeof(COMM_WORK) );

  p_wk->heapID = heapID;

  // 低優先度システム作成
  GFL_NET_LDATA_InitSystem( heapID , FALSE );

  // 送信用バッファ作成
  if( isTwl == TRUE )
  {
    p_wk->cap_buf = GFL_HEAP_AllocClearMemory( heapID, CAPTURE_SIZE);
  }

  // 低優先度送信システムにカメラ受信バッファを渡す
  {
    u8  i;
    u8  mem_cnt = 0;
    const u8 connectNum = GFL_NET_GetConnectNum();
    const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
    for( i=0; i<connectNum; i++ )
    {
      if( selfId == i )
      {
        // 自分だとスキップ
        continue;
      }

      // TWLのみ受信設定
      if( member[mem_cnt].isTwl == TRUE )
      {
        u8  netID = member[mem_cnt].netID;

        // バッファ設定
        GFL_NET_LDATA_CreatePostBuffer_SetAddres( CAPTURE_SIZE, netID, heapID, member[mem_cnt].p_cap_buf[0] );
        p_wk->cap_buf_bit |= 1<<netID;

        ICHI_Printf("バッファ設定 i %d netID %d mem %d adrs %x \n", i, netID, mem_cnt, member[mem_cnt].p_cap_buf[0] );

        mem_cnt++;
      }
    }
    ICHI_Printf("camera capture 通信設定完了 ");
    ICHI_Printf("UserBit %x \n", p_wk->cap_buf_bit);
  }

  // カウンタクリア
  {
    u8 i;
    for( i=0; i<CTVT_MEMBER_NUM-1; i++ )
    {
      p_wk->cap_cnt[i] = 0;
    }
  }

  p_wk->is_init = FALSE;    // カメラ開始同期後にTRUEにする
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信  終了
 *
 *	@param	COMM_WORK *p_wk ワーク 
 */
//-----------------------------------------------------------------------------
static void COMM_Exit( COMM_WORK *p_wk, BOOL isTwl )
{
  if( isTwl == TRUE )
  {
    GFL_HEAP_FreeMemory( p_wk->cap_buf );
  }

  // 低優先度送信削除
  GFL_NET_LDATA_ExitSystem();
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信  メイン
 *	        処理の流れとしては、常に受信していて、
 *	        受信が完了したNetIdは次の写真を受信する
 *
 *	@param	COMM_WORK *p_wk ワーク 
 */
//-----------------------------------------------------------------------------
static void COMM_Main( COMM_WORK *p_wk, CTVT_GAME_CAM_MEMBER_WORK member[] )
{
  enum{
    SEQ_POST,       // 受信中
    SEQ_SYNC,       // 同期
    SEQ_SYNC_WAIT,  // 同期待ち
    SEQ_END,        // 終了
  };

  if( p_wk->is_end == TRUE )
  {
    // 終了してたらSkip
    return;
  }

  if( p_wk->is_init == FALSE )
  {
    // 初期化完了していない
    return;
  }

  switch( p_wk->seq ){
  case SEQ_POST:       // 受信中
    {
      u8    i;
      u8    mem_cnt = 0;
      BOOL  ret = TRUE;   // 次シーケンスへいってよいか判定用
      const u8 connectNum = GFL_NET_GetConnectNum();
      const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

      // 送信してくる人数分のみまわす
      for( i=0; i<connectNum; i++ )
      {
        // 送信してくるNetIDのみチェック
        if( p_wk->cap_buf_bit & (1<<i) )
        {
          ICHI_Printf("NetID %d Check \n", i);
          if( p_wk->cap_cnt_base != p_wk->cap_cnt[mem_cnt] )
          {
            // 既に受信した
            break;
          }

          if( GFL_NET_LDATA_IsFinishPost(i) == TRUE )
          {
            // 次の受信のためにバッファを設定し直す
            void *p_tmp;

            ICHI_Printf("%d からデータ来た\n", i );
            ICHI_Printf("member[%d].netID = %d \n", mem_cnt, member[mem_cnt].netID);
            ICHI_Printf("size %x \n", GFL_NET_LDATA_GetPostDataSize(i));

            // バッファ設定
            GFL_NET_LDATA_DeletePostBuffer(i);
            p_wk->cap_cnt[mem_cnt]++;
            if( p_wk->cap_cnt[mem_cnt] < CAPTURE_NUM )
            {
              GFL_NET_LDATA_CreatePostBuffer_SetAddres( CAPTURE_SIZE, i, p_wk->heapID, member[mem_cnt].p_cap_buf[ p_wk->cap_cnt[mem_cnt] ] );
              ICHI_Printf("バッファ設定 netID %d mem %d adrs %x \n", i, mem_cnt, member[mem_cnt].p_cap_buf[ p_wk->cap_cnt[mem_cnt] ] );
            }

            ICHI_Printf("NetId %d から%d枚受信完了 ", i, p_wk->cap_cnt[mem_cnt] );
          }
          else
          {
            ret = FALSE;
          }
        }
        // メンバーカウント
        if( selfId != i )
        {
          mem_cnt++;
        }
      }

      // 全員分の受信が完了した
      if( ret == TRUE )
      {
        p_wk->seq = SEQ_SYNC;
        ICHI_Printf("全員分受信完了\n");
      }
    }
    break;
  case SEQ_SYNC:       // 同期
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_HANDLE_TimeSyncStart( pNet, CAMERA_SEND_SYNC, WB_NET_COMM_TVT );
      p_wk->seq = SEQ_SYNC_WAIT;
      ICHI_Printf("送信同期待ち\n");
    }
    break;
  case SEQ_SYNC_WAIT:  // 同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CAMERA_SEND_SYNC, WB_NET_COMM_TVT ) == TRUE )
      {
        p_wk->seq = SEQ_POST;
        p_wk->can_next = TRUE;

        p_wk->cap_cnt_base++;
        if( p_wk->cap_cnt_base == CAPTURE_NUM )
        {
          p_wk->is_end = TRUE;
          p_wk->seq = SEQ_END;
          ICHI_Printf("全員分受信終わり\n");
        }
        ICHI_Printf("送信同期完了\n");
      }
    }
    break;
  case SEQ_END:
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信  送信
 *          この関数を呼ぶ前に送信バッファにデータを入れておくこと
 *
 *	@param	COMM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void COMM_SendCapture( COMM_WORK *p_wk )
{
  GF_ASSERT( p_wk->cap_buf );
  // カメラキャプチャ送信リクエスト
  {
    // 低優先度送信で一気に送る
    GFL_NET_LDATA_SetSendData( p_wk->cap_buf, CAPTURE_SIZE, p_wk->cap_buf_bit, FALSE );
    ICHI_Printf("Send CamCapture addr %x bit %x \n", p_wk->cap_buf, p_wk->cap_buf_bit);

    {
      u8 i;
      u16 *tmp = p_wk->cap_buf;
      ICHI_Printf("キャプチャのデータ 少しだけ\n");
      for(i=0; i<64; i++ )
      {
        ICHI_Printf("%x ", *(tmp+i));
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信  次の撮影にいってよいか
 *
 *	@param	const COMM_WORK *p_wk ワーク
 *	@param  const u8        cnt   何枚目か
 *
 *	@return TRUEでOK
 */
//-----------------------------------------------------------------------------
static BOOL COMM_CanNext( const COMM_WORK *p_wk )
{
  // 送信完了まで待つ
  if( GFL_NET_LDATA_IsFinishSend() == FALSE )
  {
    return FALSE;
  }
  
  return p_wk->can_next;
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信  送信バッファ取得
 *
 *	@param	const COMM_WORK p_wkワーク
 *
 *	@return void* cap_buf 
 */
//-----------------------------------------------------------------------------
static void* COMM_GetSendBuf( const COMM_WORK *p_wk )
{
  return p_wk->cap_buf;
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信  受信が全て完了したか
 *
 *	@param	const COMM_WORK p_wkワーク
 *
 *	@return TRUE で終了
 */
//-----------------------------------------------------------------------------
static BOOL COMM_IsEnd( const COMM_WORK *p_wk )
{
  return p_wk->is_end;
}


//----------------------------------------------------------------------------
/**
 *	@brief  自分のキャプチャを保存する
 *
 *	@param	COMM_WORK *p_wk ワーク
 *	@param	dst             保存先
 */
//-----------------------------------------------------------------------------
static void COMM_CaptureSave( COMM_WORK *p_wk, void* dst )
{
  GFL_STD_MemCopy32( p_wk->cap_buf, dst, CAPTURE_SIZE );
}

//=============================================================================
/**
 *            SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 初期化
 *
 *  @param  SEQ_WORK *p_wk  ワーク
 *  @param  *p_param        パラメータ
 *  @param  seq_function    シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Init( SEQ_WORK *p_wk, void *p_param, SEQ_FUNCTION seq_function )
{ 
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );

  //初期化
  p_wk->p_param = p_param;

  //セット
  SEQ_SetNext( p_wk, seq_function );
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 破棄
 *
 *  @param  SEQ_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void SEQ_Exit( SEQ_WORK *p_wk )
{ 
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ メイン処理
 *
 *  @param  SEQ_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Main( SEQ_WORK *p_wk )
{ 
  if( !p_wk->is_end )
  { 
    p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_param );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 終了取得
 *
 *  @param  const SEQ_WORK *cp_wk   ワーク
 *
 *  @return TRUEならば終了  FALSEならば処理中
 */ 
//-----------------------------------------------------------------------------
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk )
{ 
  return cp_wk->is_end;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 次のシーケンスを設定
 *
 *  @param  SEQ_WORK *p_wk  ワーク
 *  @param  seq_function    シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function )
{ 
  p_wk->seq_function  = seq_function;
  p_wk->seq = 0;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 終了
 *
 *  @param  SEQ_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( SEQ_WORK *p_wk )
{ 
  p_wk->is_end  = TRUE;
}


//=============================================================================
/**
 *          SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  シーケンス終了
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_End( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{ 
  CTVT_GAME_CAM_WORK *p_wk = p_param;

  // 終了前処理

  //終了
  SEQ_End( p_seqwk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  開始同期
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_SYNC_Init( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_SYNC,       // 同期
    SEQ_SYNC_WAIT,  // 同期待ち
    SEQ_END,
  };

  CTVT_GAME_CAM_WORK *p_wk = p_param;

  switch( *p_seq ){
  case SEQ_SYNC:      // 同期
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_HANDLE_TimeSyncStart( pNet, CAMERA_INIT_SYNC, WB_NET_COMM_TVT );
      *p_seq = SEQ_SYNC_WAIT;
      ICHI_Printf("カメラ開始同期待ち\n");
    }
    break;
  case SEQ_SYNC_WAIT: // 同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CAMERA_INIT_SYNC, WB_NET_COMM_TVT ) == TRUE )
      {
        p_wk->comm.is_init = TRUE;
        *p_seq = SEQ_END;
        ICHI_Printf("カメラ開始同期完了\n");
      }
    }
    break;
  case SEQ_END:
    // TWL起動かNITRO起動か判定
    if( p_wk->p_param->isTwl )
    {
      if( COMM_TVT_CanUseCamera() == TRUE )
      {
        SEQ_SetNext( p_seqwk, SEQFUNC_TWL_Init );
      }
      else
      {
        SEQ_SetNext( p_seqwk, SEQFUNC_NITRO_Init );
      }
    }
    else
    {
      SEQ_SetNext( p_seqwk, SEQFUNC_NITRO_Init );
    }
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  終了同期
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_SYNC_End( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_SYNC,       // 同期
    SEQ_SYNC_WAIT,  // 同期待ち
    SEQ_END,
  };

  CTVT_GAME_CAM_WORK *p_wk = p_param;

  switch( *p_seq ){
  case SEQ_SYNC:      // 同期
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_HANDLE_TimeSyncStart( pNet, CAMERA_END_SYNC, WB_NET_COMM_TVT );
      *p_seq = SEQ_SYNC_WAIT;
      ICHI_Printf("カメラ終了同期待ち\n");
    }
    break;
  case SEQ_SYNC_WAIT: // 同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CAMERA_END_SYNC, WB_NET_COMM_TVT ) == TRUE )
      {
        *p_seq = SEQ_END;
        ICHI_Printf("カメラ終了同期完了\n");
      }
    }
    break;
  case SEQ_END:
    SEQ_SetNext( p_seqwk, SEQFUNC_End );
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  プリントストリーム
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_PrintStream( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_STREAM,   // ストリーム表示中
    SEQ_WAIT,
    SEQ_EXIT,
  };

  CTVT_GAME_CAM_WORK *p_wk = p_param;
  PS_WORK *p_ps = &(p_wk->ps);

  ICHI_Printf("|");
  switch( *p_seq ){
  case SEQ_STREAM:   // ストリーム表示中
    {
      PRINTSTREAM_STATE state = PRINTSYS_PrintStreamGetState( p_ps->p_stream );
      GFL_TCBL_Main( p_ps->p_tcblsys );
      switch( state ) {
      case PRINTSTREAM_STATE_RUNNING:
        {
          if( GFL_UI_TP_GetCont() )
          {
            //PRINTSYS_PrintStreamShortWait( p_ps->p_stream, 0 );
          }
          ICHI_Printf("*");
        }
        break;
      case PRINTSTREAM_STATE_DONE:
        {
          *p_seq = SEQ_WAIT;
        }
        break;
      case PRINTSTREAM_STATE_PAUSE:
        {
        }
        break;
      }
    }
    break;
  case SEQ_WAIT:
    {
      if( p_ps->wait_count >= p_ps->wait_max )
      {
        *p_seq = SEQ_EXIT;
      }
      else
      {
        p_ps->wait_count++;
      }
    }
    break;
  case SEQ_EXIT:
    {
      // waitが設定されていたらデフォルトに戻す
      if( PRINTSYS_IsTempSpeedMode( p_ps->p_stream ) == TRUE )
      {
        PRINTSYS_PrintStream_StopTempSpeedMode( p_ps->p_stream );
      }
      PRINTSYS_PrintStreamDelete( p_ps->p_stream );
      p_ps->p_stream = NULL;

      SEQ_SetNext( p_seqwk, p_ps->next_seq_function );
    }
    break;
  default:
    GF_ASSERT(0);
    break;
  }
}

//=============================================================================
/**
 *    TWL SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  TWL 初期化
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_TWL_Init( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  CTVT_GAME_CAM_WORK *p_wk = p_param;

  // カメラモジュール作成
  CAM_Init( &p_wk->camera, p_wk->p_param->p_cam_sys, p_wk->heapID );

  // TWL時のみ表示するもの
  GFL_CLACT_WK_SetDrawEnable( OBJ_GetClwk( &p_wk->obj, CLWKID_FRAME_S ), TRUE );
  
  // カメラ キャプチャコールバック関数置き換え
  CAMERA_SYS_SetCaptureCallBack( p_wk->p_param->p_cam_sys, cap_call_back, &p_wk->camera );
  CAMERA_SYS_StartCapture( p_wk->p_param->p_cam_sys );
  
  // 最初の説明メッセージ
  // プリントストリームは3つで1セット
  PS_Print( &p_wk->ps, &p_wk->msgwnd, SEQFUNC_TWL_Count, COMM_TVT_GAME_06, p_wk->heapID );
  SEQ_SetNext( p_seqwk, SEQFUNC_PrintStream );
}

//----------------------------------------------------------------------------
/**
 *	@brief  TWL 枚数メッセージ待ち
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_TWL_Count( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  CTVT_GAME_CAM_WORK *p_wk = p_param;

  // X枚目の写真を撮影します メッセージ
  PS_PrintNumber( &p_wk->ps, &p_wk->msgwnd, SEQFUNC_TWL_MsgWait, (p_wk->camera.cnt+1), COMM_TVT_GAME_07, p_wk->heapID );
  SEQ_SetNext( p_seqwk, SEQFUNC_PrintStream );
}

//----------------------------------------------------------------------------
/**
 *	@brief  TWL 撮影メッセージ待ち
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_TWL_MsgWait( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  CTVT_GAME_CAM_WORK *p_wk = p_param;

  // X枚目の写真を撮影します メッセージ
  PS_Print( &p_wk->ps, &p_wk->msgwnd, SEQFUNC_TWL_Main, (COMM_TVT_GAME_08 + p_wk->camera.cnt), p_wk->heapID );
  SEQ_SetNext( p_seqwk, SEQFUNC_PrintStream );
}

//----------------------------------------------------------------------------
/**
 *	@brief  TWL メイン
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_TWL_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_CAPTURE_MSG,  // 写真撮影メッセージ表示
    SEQ_CAPTURE,      // 写真撮影
    SEQ_SE_WAIT,      // シャッター音再生待ち
    SEQ_FADE_WAIT,    // フェードイン待ち
    SEQ_SEND,         // 送信する
    SEQ_SEND_WAIT,    // 送信完了待ち
    SEQ_NEXT,         // 次の写真撮影へ
    SEQ_END,
  };

  CTVT_GAME_CAM_WORK *p_wk = p_param;

  switch( *p_seq ){
  case SEQ_CAPTURE_MSG:
    // カウントダウン開始
    GFL_CLACT_WK_SetDrawEnable( OBJ_GetClwk( &p_wk->obj, CLWKID_COUNT ), TRUE );
    GFL_CLACT_WK_ResetAnm( OBJ_GetClwk( &p_wk->obj, CLWKID_COUNT ) );
    *p_seq = SEQ_CAPTURE;
    break;
  case SEQ_CAPTURE:     // 写真撮影
    if( GFL_CLACT_WK_CheckAnmActive( OBJ_GetClwk( &p_wk->obj, CLWKID_COUNT ) ) == FALSE )
    {
      // キャプチャストップ
      CAM_CaptureStop( &p_wk->camera );

      // 数字非表示
      GFL_CLACT_WK_SetDrawEnable( OBJ_GetClwk( &p_wk->obj, CLWKID_COUNT ), FALSE );

      // ホワイトフェードアウト
      WIPE_SYS_Start( WIPE_PATTERN_S, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
          WIPE_FADE_WHITE, WIPE_DEF_DIV, 2, p_wk->heapID );

#ifndef DEBUG_ONLY_FOR_ichiraku_katsuhiko
      // シャッター音再生
      CAM_PlayShutterSe( &p_wk->camera );
#endif

      *p_seq = SEQ_SE_WAIT;
    }
    break;

  case SEQ_SE_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE 
       && CAM_IsPlayShutterSe( &p_wk->camera ) == FALSE )
    {
      // ホワイトフェードイン
      WIPE_SYS_Start( WIPE_PATTERN_S, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
          WIPE_FADE_WHITE, WIPE_DEF_DIV, 2, p_wk->heapID );
      *p_seq = SEQ_FADE_WAIT;
    }
    break;

  case SEQ_FADE_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      // 画像縮小
      void* dst = COMM_GetSendBuf( &p_wk->comm );
      CAM_CaptureScaling( &p_wk->camera, dst, p_wk->heapID );
      // キャプチャ保存しておく
      // 縮小した後に保存しないとデータがおかしくなる
      COMM_CaptureSave( &p_wk->comm, p_wk->p_param->p_cap[p_wk->camera.cnt] );

      *p_seq = SEQ_SEND;
    }
    break;

  case SEQ_SEND:        // 送信する
    COMM_SendCapture( &p_wk->comm );

    // 上画面にとった写真を表示
    INFO_AddBmp(INFO_X + INFO_W*p_wk->camera.cnt, INFO_Y, OUTPUT_WIDTH, OUTPUT_HEIGHT, COMM_GetSendBuf(&p_wk->comm) );

    // 写真撮ったカウント増やす
    p_wk->camera.cnt++;
    *p_seq = SEQ_NEXT;
    break;
  case SEQ_NEXT:        // 次の写真撮影へ

    if( COMM_CanNext( &p_wk->comm ) == FALSE )
    {
      break;
    }
    if( p_wk->camera.cnt < CAPTURE_NUM )
    {
      p_wk->comm.can_next = FALSE;    // フラグリセット

      // キャプチャスタート
      CAM_CaptureStart( &p_wk->camera );

      SEQ_SetNext( p_seqwk, SEQFUNC_TWL_Count );
    }
    else
    {
      // 3枚撮ったから終了
      *p_seq = SEQ_END;
    }
    break;
  case SEQ_END:
    SEQ_SetNext( p_seqwk, SEQFUNC_TWL_Wait );
    break;
  }

  // メイン処理
  MSGWND_PrintMain( &p_wk->msgwnd );
  // カメラ
  CAMERA_SYS_UpdateCameta( p_wk->p_param->p_cam_sys );

}

//----------------------------------------------------------------------------
/**
 *	@brief  TWL 終了待ち
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_TWL_Wait( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  CTVT_GAME_CAM_WORK *p_wk = p_param;


  // カメラのキャプチャはSTOP
  CAMERA_SYS_StopCapture( p_wk->p_param->p_cam_sys );

  //// StopCaptureは停止音待ち
  //if( CAMERA_SYS_IsStopCapture( p_wk->p_param->p_cam_sys ) == TRUE )
  {
    // カメラモジュール破棄
    CAM_Exit( &p_wk->camera );

    SEQ_SetNext( p_seqwk, SEQFUNC_SYNC_End );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  キャプチャ完了時に呼ばれるコールバック関数
 *
 *	@param	void *captureArea   キャプチャデータ
 *	@param	*userWork 
 */
//-----------------------------------------------------------------------------
static void cap_call_back( void *captureArea, void *userWork )
{
  CAMERA_WORK* p_wk = userWork;

  // LCDのDMA転送との競合解消
  if( p_wk->is_update == TRUE )
  {
    return;
  }

  // キャプチャデータコピー
  GFL_STD_MemCopy32( (void*)captureArea, (void*)p_wk->scr_buf, CTVT_BUFFER_SCR_SIZE );

  p_wk->is_update = TRUE;
}

//=============================================================================
/**
 *    NITRO SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  NITRO 初期化
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_NITRO_Init( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  CTVT_GAME_CAM_WORK *p_wk = p_param;

  // ルーレットモジュール
  ROU_Init( &p_wk->roulette, p_wk->heapID );

  // NITOR時のみ表示するもの
  {
    u8 i;
    for( i=CLWKID_CHARA_TOP; i<CLWKID_CHARA_END; i++ )
    {
      GFL_CLACT_WK_SetDrawEnable( OBJ_GetClwk( &p_wk->obj, i ), TRUE );
    }
  }

  // NITRO用背景を読み込む
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_TVT_CAMERA, p_wk->heapID );

    // 下画面
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_tvt_camera_bg_NCLR,
        PALTYPE_SUB_BG, PLT_BG_S*0x20, 0, p_wk->heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_tvt_camera_bg_lower_NCGR,
        BG_FRAME_BACK_S, 0x0, 0, FALSE, p_wk->heapID );
    // 転送OFFSET 0x300=32*24
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_tvt_camera_bg_lower_NSCR,
        BG_FRAME_BACK_S, 0x0, 0, FALSE, p_wk->heapID );

    GFL_ARC_CloseDataHandle( p_handle );
    GFL_BG_SetVisible( BG_FRAME_BACK_S, TRUE );
  }

  // 下画面にキャラクターの画像を表示する
#if 0
  {
    u8 i;

    for( i=0; i<CHAR_NUM; i++ )
    {
      u16 pos_x  = (74-32) + ( 118*( i%2 ) );
      u16 pos_y  = (50-32) + ( 94*( i/2 ) );

      ROU_AddBmp( pos_x, pos_y, 64, 64, p_wk->roulette.pic_buf[i] );
    }
  }
#endif
#if 0
  {
    u8 i;
    for( i=0; i<CAPTURE_NUM; i++ )
    {
      void *tmp_buf = GFL_ARC_UTIL_Load( ARCID_TVT_CAMERA, NARC_tvt_camera_char1_1_ntft+i, FALSE, p_wk->heapID );
      INFO_AddBmp(INFO_X + INFO_W*i, INFO_Y, OUTPUT_WIDTH, OUTPUT_HEIGHT, tmp_buf );
      GFL_HEAP_FreeMemory( tmp_buf );
    }
  }
#endif
  SEQ_SetNext( p_seqwk, SEQFUNC_NITRO_Trans );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NITRO 画像転送
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_NITRO_Trans( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  CTVT_GAME_CAM_WORK *p_wk = p_param;

  u8  cnt = p_wk->roulette.select;
  u16 pos_x = (74-32) + ( 118*( cnt%2 ) );
  u16 pos_y = (50-32) + ( 94*( cnt/2 ) );

  enum
  {
    SEQ_CHARA1_TRANS,   // キャラ画像転送
    SEQ_CHARA2_TRANS,   // キャラ画像転送
    SEQ_CHARA3_TRANS,   // キャラ画像転送
    SEQ_CHARA4_TRANS,   // キャラ画像転送
    SEQ_END,
  };

  // 転送順番を1->2->4->3にしているのはわざと。
  switch( *p_seq ){
  case SEQ_CHARA1_TRANS:   // キャラ画像転送
    ROU_AddBmp( pos_x, pos_y, 64, 64, p_wk->roulette.pic_buf[ 0 ] );
    *p_seq = SEQ_CHARA2_TRANS;
    break;
  case SEQ_CHARA2_TRANS:   // キャラ画像転送
    ROU_AddBmp( pos_x, pos_y, 64, 64, p_wk->roulette.pic_buf[ 1 ] );
    *p_seq = SEQ_CHARA4_TRANS;
    break;
  case SEQ_CHARA3_TRANS:   // キャラ画像転送
    ROU_AddBmp( pos_x, pos_y, 64, 64, p_wk->roulette.pic_buf[ 2 ] );
    *p_seq = SEQ_END;
    break;
  case SEQ_CHARA4_TRANS:   // キャラ画像転送
    ROU_AddBmp( pos_x, pos_y, 64, 64, p_wk->roulette.pic_buf[ 3 ] );
    *p_seq = SEQ_CHARA3_TRANS;
    break;
  case SEQ_END:
    p_wk->roulette.select = 0;
    // 最初の説明メッセージ
    // プリントストリームは3つで1セット
    PS_Print( &p_wk->ps, &p_wk->msgwnd, SEQFUNC_NITRO_Start, COMM_TVT_GAME_11, p_wk->heapID );
    SEQ_SetNext( p_seqwk, SEQFUNC_PrintStream );
    return;
  }

  p_wk->roulette.select++;
}

//----------------------------------------------------------------------------
/**
 *	@brief  NITRO 最初のメッセージ
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_NITRO_Start( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  CTVT_GAME_CAM_WORK *p_wk = p_param;

  PS_Print( &p_wk->ps, &p_wk->msgwnd, SEQFUNC_NITRO_Main, COMM_TVT_GAME_12, p_wk->heapID );
  SEQ_SetNext( p_seqwk, SEQFUNC_PrintStream );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NITRO メイン
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_NITRO_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_ROULETTE_12_7,  // ルーレット中
    SEQ_ROULETTE_7_1,   // ルーレット中
    SEQ_JUST,           // キャラの位置調整
    SEQ_FLASH,          // 当たったキャラの点滅
    SEQ_END,
  };

  CTVT_GAME_CAM_WORK *p_wk = p_param;

  switch( *p_seq ){
  case SEQ_ROULETTE_12_7:     // ルーレット中
    if( ROU_GetStopFlg( &p_wk->roulette ) == FALSE )
    {
      // ルーレット設定
      ROU_WaitSet( &p_wk->roulette, ROULETET_WAIT );
    }
    // 経過フレームを見て、5秒経つと次に移行
    p_wk->roulette.frame_cnt++;
    if( p_wk->roulette.frame_cnt >= 300 )
    {
      *p_seq = SEQ_ROULETTE_7_1;
    }
    break;

  case SEQ_ROULETTE_7_1:
    {
      u16 wait_frame; 
      // frame_cntを足すことで徐々に遅くしている
      wait_frame = ROULETET_WAIT + (p_wk->roulette.frame_cnt-300)/ROULETTE_DIV;

      if( ROU_GetStopFlg( &p_wk->roulette ) == FALSE )
      {
        // ルーレット設定
        ROU_WaitSet( &p_wk->roulette, wait_frame );
      }
      // 経過フレームを見て、12秒経つと調整に移行
      p_wk->roulette.frame_cnt++;
      if( p_wk->roulette.frame_cnt >= ROULETTE_TIME )
      {
        *p_seq = SEQ_JUST;
      }
    }
    break;

  case SEQ_JUST:
    {
      u16 wait_frame; 
      // frame_cntを足すことで徐々に遅くしている
      wait_frame = ROULETET_WAIT + (p_wk->roulette.frame_cnt-300)/ROULETTE_DIV;
      if( p_wk->roulette.select != p_wk->p_param->select_chara )
      {
        if( ROU_GetStopFlg( &p_wk->roulette ) == FALSE )
        {
          // ルーレット設定
          ROU_WaitSet( &p_wk->roulette, wait_frame );
        }
      }
      else
      {
        // 点滅に使用するため初期化
        p_wk->roulette.frame_cnt = 0;
        p_wk->roulette.wait = 0;
        *p_seq = SEQ_FLASH;
      }
    }
    break;

  case SEQ_FLASH:
    {
      if( !(p_wk->roulette.frame_cnt%6) )
      {
        u8  clwkID = CLWKID_CHARA_TOP + p_wk->roulette.select;
        p_wk->roulette.wait ^= 1;

        GFL_CLACT_WK_SetPlttOffs( OBJ_GetClwk( &p_wk->obj, clwkID ), p_wk->roulette.wait, CLWK_PLTTOFFS_MODE_PLTT_TOP );
      }
      p_wk->roulette.frame_cnt++;
      if( p_wk->roulette.frame_cnt > 80 )
      {
        *p_seq = SEQ_END;
      }
    }
    break;

  case SEQ_END:
    // 上画面にキャラ表示
    {
      u8 i;
      ARCDATID  dat_tbl[4] = {
        NARC_tvt_camera_char1_1_ntft,
        NARC_tvt_camera_char2_1_ntft,
        NARC_tvt_camera_char3_1_ntft,
        NARC_tvt_camera_char4_1_ntft,
      };

      for( i=0; i<CAPTURE_NUM; i++ )
      {
        void *tmp_buf = GFL_ARC_UTIL_Load( ARCID_TVT_CAMERA, dat_tbl[p_wk->roulette.select]+i, FALSE, p_wk->heapID );
        INFO_AddBmp(INFO_X + INFO_W*i, INFO_Y, OUTPUT_WIDTH, OUTPUT_HEIGHT, tmp_buf );
        GFL_HEAP_FreeMemory( tmp_buf );
      }
    }
    PS_Print( &p_wk->ps, &p_wk->msgwnd, SEQFUNC_NITRO_Wait, COMM_TVT_GAME_13, p_wk->heapID );
    SEQ_SetNext( p_seqwk, SEQFUNC_PrintStream );
    break;
  }

  // メイン処理
  MSGWND_PrintMain( &p_wk->msgwnd );
  ROU_Main( &p_wk->roulette, &p_wk->obj );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NITRO 終了待ち
 *
 *	@param	SEQ_WORK *p_seqwk シーケンスワーク
 *	@param	*p_seq            シーケンス
 *	@param	*p_param          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_NITRO_Wait( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  CTVT_GAME_CAM_WORK *p_wk = p_param;

  if( COMM_IsEnd( &p_wk->comm ) == TRUE )
  {
    // ルーレットモジュール
    ROU_Exit( &p_wk->roulette );
    SEQ_SetNext( p_seqwk, SEQFUNC_SYNC_End );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  VBlank
 *
 *	@param	GFL_TCB *p_tcb
 *	@param	*p_work 
 */
//-----------------------------------------------------------------------------
static void VBlankFunc( GFL_TCB *p_tcb, void *p_work )
{
  CTVT_GAME_CAM_WORK* p_wk = p_work;

  // isUpdateCapture は カメラDMAとLCDDMAの競合解決の為のフラグ
  if( p_wk->camera.is_update == TRUE && p_wk->camera.scr_buf != NULL )
  {
    // カメラキャプチャ転送
    DC_FlushRange( p_wk->camera.scr_buf, CTVT_BUFFER_SCR_SIZE );
    GXS_LoadBG3Bmp( p_wk->camera.scr_buf, 0, CTVT_BUFFER_SCR_SIZE );
    p_wk->camera.is_update = FALSE;
  }

  // OBJの更新
  GFL_CLACT_SYS_VBlankFunc();

  //machine_useでやってた
  //  NNS_GfdDoVramTransfer();
}

