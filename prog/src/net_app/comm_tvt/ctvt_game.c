//======================================================================
/**
 * @file	ctvt_game.c
 * @brief	通信TVTシステム：ミニゲーム画面
 * @author	ichiraku
 * @data	2010/11/25
 *
 * モジュール名：CTVT_GAME
 */
//======================================================================
#include <gflib.h>
#include <wchar.h>					//wcslen

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/camera_system.h"
#include "system/bmp_winframe.h"
#include "net/network_define.h"
#include "app/app_menu_common.h"

#include "msg/msg_comm_tvt.h"

#include "arc_def.h"
#include "comm_tvt.naix"
#include "tvt_balloon.naix"
#include "font/font.naix"

#include "ctvt_game.h"
#include "ctvt_game_balloon.h"
#include "ctvt_comm.h"
#include "ctvt_camera.h"
#include "ctvt_snd_def.h"
#include "ctvt_game_balloon_tbl.cdat"

//debug
#include "debug/debug_str_conv.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define CTVT_GAME_TIME_LIMIT    (60)    // 制限時間 (秒)
#define CTVT_TEX_SIZE           (128)   // テクスチャの縦横サイズ
#define CTVT_CAPTURE_SIZE ( CTVT_TEX_SIZE * CTVT_TEX_SIZE * 2 ) // 128*128*u16

#define OUTPUT_WIDTH            (64)    // DSPで変換後の幅
#define OUTPUT_HEIGHT           (64)    // DSPで変換後の高さ

#define BALLOON_RND_W_MAX       (4)     // 出現位置乱数のMAX値
#define BALLOON_RND_V_MAX       (3)     // 出現位置乱数のMAX値

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//ステート
typedef enum
{
  CGS_FADEIN,
  CGS_FADEIN_WAIT,

  CGS_FADEOUT, //両方フェード
  CGS_FADEOUT_WAIT,

  CGS_CAMERA, // 顔を撮るモード
  CGS_GAME,   // メイン
  
  //親がいなくなったことによる終了
  CGS_END_PARENT_NONE_INIT,
  CGS_END_PARENT_NONE,

  //一人になったので終了
  CGS_END_MEMBER_NONE_INIT,
  CGS_END_MEMBER_NONE,  
  
}CTVT_GAME_STATE;


//-------------------------------------
///	セルリソースインデックス
//=====================================
typedef enum {
  CGOR_CAMERA_PLT,      // カメラ撮影モード
  CGOR_COMMON_PLT,      // ゲーム共通
  CGOR_PLAY_PLT,        // ゲームプレイ中

  CGOR_CAMERA_NCG,      // カメラ撮影モード
  CGOR_COMMON_NCG,      // ゲーム共通
  CGOR_PLAY_NCG,        // ゲームプレイ中

  CGOR_CAMERA_ANM,      // カメラ撮影モード
  CGOR_COMMON_ANM,      // ゲーム共通
  CGOR_PLAY_ANM,        // ゲームプレイ中
  
  CGOR_MAX,   // 最大値
} CTVT_GAME_OBJ_RES;
#define CGOR_PLT_TOP  ( CGOR_CAMERA_PLT )   // パレットTOP
#define CGOR_NCG_TOP  ( CGOR_CAMERA_NCG )   // キャラクタTOP
#define CGOR_ANM_TOP  ( CGOR_CAMERA_ANM )   // セルアニメTOP

//-------------------------------------
///	セルアニメインデックス 共通
//=====================================
typedef enum {
  CCOA_TITLE,     // タイトル
  CCOA_START,     // 文字 はじめる
  CCOA_SETSUMEI,  // 文字 説明
  CCOA_END,       // 文字 おわり

  CCOA_MAX,
} CTVT_COMMON_OBJ_ANM;

//-------------------------------------
///	セルアニメインデックス  プレイ中
//=====================================
typedef enum {
  CPOA_NUM_0,    // 数字 0
  CPOA_NUM_1,    // 数字 1
  CPOA_NUM_2,    // 数字 2
  CPOA_NUM_3,    // 数字 3
  CPOA_NUM_4,    // 数字 4
  CPOA_NUM_5,    // 数字 5
  CPOA_NUM_6,    // 数字 6
  CPOA_NUM_7,    // 数字 7
  CPOA_NUM_8,    // 数字 8
  CPOA_NUM_9,    // 数字 9
  CPOA_NUM_P,    // 数字 +
  CPOA_NUM_M,    // 数字 -
  CPOA_TIME,     // 文字 TIME
  CPOA_SCORE,    // 文字 SCORE
} CTVT_PLAY_OBJ_ANM;



//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//-------------------------------------
///	メインワーク
//=====================================
struct _CTVT_GAME_WORK
{
  HEAPID          heapId;

  CTVT_GAME_STATE state;
  BOOL            isCamCapture; // カメラキャプチャ中か？              
  
  // 各ステート内でのシーケンス管理用
  // ・カメラ撮る
  // ・ミニゲーム
  // ・結果(仮)
  u8    seq;      
  

  BOOL isFinish;
  BOOL reqStopCamera;

  //=====================================
  // カメラ関係
  //-------------------------------------
  void  *scrBuf;   // 1サイズ 0x18000
  BOOL  isUpdateCapture; // カメラキャプチャが更新されたか？
  void  *capBuf;   // 128x128*2
  // メンバーの管理用
  void  *postCaptureBuf[CTVT_GAME_MEMBER_NUM-1];  // 128x128*2
  u8    CaptureBufUseBit;    // バッファ管理用


  //=====================================
  // ゲームプレイ関係
  //-------------------------------------
  GFL_TCBSYS    *task;          // タスクシステム
  void          *taskMem;       // タスク用メモリ
  BALLOON_WORK  balloon_wk[ BALLOON_MAX ];    // 風船構造体
  u8            balloon_rnd_w;                // 風船出現調整
  u8            balloon_rnd_v;                // 風船出現調整
  u8            balloon_z_cnt;                // 風船出現調整
  void          *direct_tex[CTVT_GAME_MEMBER_NUM];    // 128x128を64x64に変換した移動先
  u8            balloon_tbl_idx;              // 風船出現にどのテーブルを使用するか
  u8            balloon_tbl_cnt;              // テーブルの要素カウンタ
  u16           bonus_tbl[CTVT_GAME_MEMBER_NUM][5]; // ボーナス風船の出現フレームのテーブル
  u8            bonus_tbl_idx[CTVT_GAME_MEMBER_NUM];

  // メッセージ系
  GFL_BMPWIN    *msgWinMain;        // 下画面
  BOOL          isUpdateMsgMain;    // メッセージ更新
  GFL_BMPWIN    *msgWinSub;         // 上画面
  BOOL          isUpdateMsgSub;     // メッセージ更新
  // 制限時間管理
  u16           timeLimit;      // 制限時間 (秒)
  u32           befFrameCnt;    // フレームカウンタ保存用

  //===============================
  // 通信
  u32   rnd_seed;         // 乱数シード値
  u16   frame;            // 通信で受信したフレーム
  BOOL  isRecvFrame;      // フレーム受信したか
  // 以下親のみ
  u16   oya_frame;        // 親のフレーム経過数
  s16   balloon_wait;     // 風船出現待ち時間
  u8    balloon_flg;      // 風船出現フラグ
  s32   score[CTVT_GAME_MEMBER_NUM];    // スコア


  //=====================================
  // GRAPHIC関係
  //-------------------------------------
  //↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
  BOOL              is_init;

  // 2D
  // セル関係
  u32         cellResIdx[ CGOR_MAX ];   // セルリソースインデックス
  GFL_CLUNIT  *cellUnit;                // セルアクターシステム
  // カメラモード
  GFL_CLWK    *clwkCam;                 // カメラモードでのセル関係
  // 共通（ゲームプレイ中以外）
  GFL_CLWK    *clwkTitle;               // タイトル文字
  GFL_CLWK    *clwkTitleItem[3];        // タイトル項目
  // ゲームプレイ中
  GFL_CLWK    *clwkTime[2];             // 制限時間
  GFL_CLWK    *clwkText;                // 文字 TIMEやRANK
  GFL_CLWK    *clwkScoreText;           // SCORE 文字
  GFL_CLWK    *clwkScore[4];            // SCORE 数字

  // 3D
  GFL_G3D_UTIL    *g3dUtil;
  u16             unit_idx[CTVT_GAME_MEMBER_NUM];           ///< GFL_G3D_UTILが割り振る番号
  GFL_G3D_CAMERA  *p_camera;
  u16             scrData[CTVT_TEX_SIZE][CTVT_TEX_SIZE]; // RGB 5:5:5 フォーマット

  GFL_TCB *vBlankTcb;


#ifdef PM_DEBUG
  //=====================================
  // DEBUG　主にゲーム調整用
  //-------------------------------------
  fx32  debug_r;    // 風船:半径特定用(当たり判定)
  fx32  debug_size; // 風船:サイズ変更
  fx32  debug_v;    // 風船:速さ変更
  u16   debug_wait; // 風船:個数変更(出現タイミング)
  u16   debug_b_cnt[4]; // どれだけ風船がでたか数える
#endif
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

// モード別メイン関数
static BOOL CTVT_GAME_Camera( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );
static void CTVT_GAME_Update( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );

// ↑のシーケンス内での関数 本当はシーケンス関数をつくりたかったが、複雑になりそうなので個別で関数を定義した
static void game_update_seq_init( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static BOOL game_update_seq_title( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static BOOL game_update_seq_main( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static void game_update_seq_score( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );
static void game_update_seq_exit( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork );

// 強制終了チェック
static const BOOL CTVT_GAME_CheckFinishReq( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork );

// カメラキャプチャコールバック
static void CTVT_GAME_CapCallBack( void *captureArea , void *userWork );

// GRAPHIC
static void GRAPHIC_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID );
static void GRAPHIC_Exit( CTVT_GAME_WORK* p_wk );
static void GRAPHIC_Draw( CTVT_GAME_WORK* p_wk );
static void VBlankFunc( GFL_TCB *p_tcb, void *p_work );

// BG
static void GRAPHIC_BG_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( CTVT_GAME_WORK* p_wk );
static void GRAPHIC_BG_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );

// 3d
static void GRAPHIC_3D_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID );
static void GRAPHIC_3D_Exit( CTVT_GAME_WORK* p_wk );

// resource
static void RESOURCE_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID, ARCHANDLE* arcHandle );
static void RESOURCE_Exit( CTVT_GAME_WORK* p_wk );

// message
static void disp_message_main( COMM_TVT_WORK *work, CTVT_GAME_WORK* p_wk, const u16 msgId );
static void disp_message_sub( COMM_TVT_WORK *work, CTVT_GAME_WORK* p_wk, const u16 msgId );



// balloon
// 風船設定
static void balloon_set( CTVT_GAME_WORK *p_game_wk, BALLOON_WORK*  p_wk, u16  unit_idx );
// 全ての風船が終了しているか？
static BOOL balloon_end_check( CTVT_GAME_WORK *p_wk );
// 誰の風船を出すか計算して求める
static u8   balloon_find_idx( CTVT_GAME_WORK *p_wk );

// タスクに設定するタスク関数(風船用)
static void TaskFunc( GFL_TCB* p_tcb, void* p_work );

// タスクに登録されている風船とタッチの当たり判定
static BOOL balloon_hit_check( CTVT_GAME_WORK* p_wk, u32 tpx, u32 tpy, u16 *ret_idx );
// タスクに登録されている風船同士の距離判定
static void balloon_collision_check( CTVT_GAME_WORK *p_wk );
// 2点の距離の壁判定処理(移動も自動)
static void collision_vec( fx32* x1, fx32* y1, fx32* x2, fx32* y2, fx32 dir, u8 avoid );
// レイと球の交差判定
static BOOL collision3d_sphere_checkRay( const VecFx32 *cp_rayPos, const VecFx32 *cp_rayWay, const GFL_COLLISION3D_CIRCLE *cp_circle );


// タイトルでの項目
static const GFL_UI_TP_HITTBL BALLOON_HitTitleButton[4] = 
{
  { 60,  84 , 92, 164 },  // はじめる
  { 84, 108 , 92, 164 },  // せつめい
  { 108,132 , 92, 164 },  // おわる
  {GFL_UI_TP_HIT_END,0,0,0}
};

//------------------------------------------------------------------
//  処理時間AVERAGE
//------------------------------------------------------------------
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

#define DEBUG_MAIN_TIME_AVERAGE (1) // 時間AVERAGE　表示 １：ON　０：OFF
#define DEBUG_AVERAGE_MAX (10)

static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start[DEBUG_AVERAGE_MAX];    // 1フレーム開始
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_End[DEBUG_AVERAGE_MAX];    // 1フレーム開始
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave[DEBUG_AVERAGE_MAX];      // AVERAGE
static int DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[DEBUG_AVERAGE_MAX]; // カウンタ
static int DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount[DEBUG_AVERAGE_MAX];    // 16000Over
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max[DEBUG_AVERAGE_MAX];    // 16000Over
#define DEBUG_DEBUG_MAIN_TIME_AVERAGE_AVE_COUNT_TIME  ( 120 )

#define DEBUG_DEBUG_MAIN_TIME_AVERAGE_NOW_TIME_DRAW_KEY  ( PAD_BUTTON_R )

static void DEBUG_MAIN_TIME_AVERAGE_StartFunc( u8 id );
static void DEBUG_MAIN_TIME_AVERAGE_EndFunc( u8 id );
#endif

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
CTVT_GAME_WORK* CTVT_GAME_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId )
{
  CTVT_GAME_WORK* gameWork = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_GAME_WORK) );
  gameWork->heapId = heapId;

  gameWork->scrBuf = GFL_HEAP_AllocClearMemory( heapId, CTVT_BUFFER_SCR_SIZE);

  //バージョンで白黒を変える
  if( GET_VERSION() == VERSION_B2 )
  {
  }
  else
  {
  }
  return gameWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void CTVT_GAME_TermSystem( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  ICHI_Printf("----CTVT_GAME_TermSystem----\n");

  GFL_HEAP_FreeMemory( gameWork->scrBuf );
  GFL_HEAP_FreeMemory( gameWork );
}
//--------------------------------------------------------------
//	モード切替時初期化
//--------------------------------------------------------------
void CTVT_GAME_InitMode( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  const HEAPID heapID = gameWork->heapId;
  u8 i;

  ICHI_Printf("----CTVT_GAME_InitMode----\n");

  // GRAPHIC初期化
  GRAPHIC_Init( gameWork, heapID );

  // リソース
  {
    ARCHANDLE* arcHandle = GFL_ARC_OpenDataHandle( ARCID_TVT_BALLOON , heapID );
    RESOURCE_Init( gameWork, heapID, arcHandle );
    GFL_ARC_CloseDataHandle( arcHandle );
  }

  // メッセージ系初期化
  {
    GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
        PALTYPE_MAIN_BG, CTVT_GAME_PAL_BG_M_FONT*0x20, 16*2, heapID );
    GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
        PALTYPE_SUB_BG, CTVT_GAME_PAL_BG_S_FONT*0x20, 16*2, heapID );

    //gameWork->msgWinMain = GFL_BMPWIN_Create( CTVT_GAME_FRAME_M_MSG , 
    //    1, 1, 30, 4, CTVT_GAME_PAL_BG_M_FONT, GFL_BMP_CHRAREA_GET_B );
    gameWork->msgWinSub = GFL_BMPWIN_Create( CTVT_GAME_FRAME_S_MSG , 
        1, 15, 30, 4, CTVT_GAME_PAL_BG_S_FONT, GFL_BMP_CHRAREA_GET_B );

    //BmpWinFrame_GraphicSet( CTVT_GAME_FRAME_M_MSG, CTVT_GAME_BMPWIN_CGX_MAIN,
    //    CTVT_GAME_PAL_BG_M_WINFRAME, 0, heapID );
    BmpWinFrame_GraphicSet( CTVT_GAME_FRAME_S_MSG, CTVT_GAME_BMPWIN_CGX,
        CTVT_GAME_PAL_BG_S_WINFRAME, 0, heapID );

  }

  // OBJ作成
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 96;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;

    gameWork->clwkCam = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_CAMERA_PLT],
        gameWork->cellResIdx[CGOR_CAMERA_NCG],
        gameWork->cellResIdx[CGOR_CAMERA_ANM],
        &cellInitData ,CLSYS_DRAW_MAIN, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkCam , FALSE );
    GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkCam , TRUE );
  }

  // カメラ キャプチャコールバック関数置き換え
  {
    CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork(work);
    CAMERA_SYSTEM_WORK *camSys = CTVT_CAMERA_GetCamSys( work, camWork );

    CAMERA_SYS_SetCaptureCallBack( camSys, CTVT_GAME_CapCallBack, gameWork );
    CAMERA_SYS_StartCapture( camSys );
    ICHI_Printf("------カメラ再設定--------\n");
  }


  // 会話モードの低優先度システムは破棄
  GFL_NET_LDATA_ExitSystem();

  // 構造体初期化
  gameWork->state = CGS_FADEIN;
  gameWork->isCamCapture = FALSE;
  gameWork->isUpdateCapture = FALSE;
  gameWork->CaptureBufUseBit = 0;
  gameWork->isFinish = FALSE;
  gameWork->reqStopCamera = FALSE;

  gameWork->capBuf = GFL_NET_Align32Alloc( heapID, CTVT_CAPTURE_SIZE);

  for( i=0;i<CTVT_GAME_MEMBER_NUM-1;i++ )
  {
    gameWork->postCaptureBuf[i] = GFL_NET_Align32Alloc( heapID, CTVT_CAPTURE_SIZE);
  }


  GFL_NET_ReloadIconTopOrBottom(TRUE , heapID );

  //上画面の会話アイコンを消す
  COMM_TVT_EraseTalkIcon( work );


  ICHI_Printf("----CTVT_GAME_InitMode END----\n");
}

//--------------------------------------------------------------
//	モード切替時開放
//--------------------------------------------------------------
void CTVT_GAME_TermMode( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  u8 i;
  const HEAPID heapId = gameWork->heapId;

  ICHI_Printf("----CTVT_GAME_TermMode----\n");

  GFL_NET_ReloadIconTopOrBottom(FALSE , heapId );


  // 会話モードの低優先度システム作成
  GFL_NET_LDATA_InitSystem( heapId , FALSE );
  // ↑の後に通信のリフレッシュを行う必要あり
  //

  // OBJ破棄
  GFL_CLACT_WK_Remove( gameWork->clwkCam );

  // メッセージ系破棄
  GFL_BMPWIN_ClearTransWindow( gameWork->msgWinMain );
  GFL_BMPWIN_ClearTransWindow( gameWork->msgWinSub );
  GFL_BMPWIN_Delete( gameWork->msgWinMain );
  GFL_BMPWIN_Delete( gameWork->msgWinSub );

  // リソース破棄
  RESOURCE_Exit( gameWork );

  // GRAPHIC破棄
  GRAPHIC_Exit( gameWork );

  // カメラ コールバック関数戻す
  {

  }
  GFL_NET_Align32Free( gameWork->capBuf );

  for( i=0;i<CTVT_GAME_MEMBER_NUM-1;i++ )
  {
    GFL_NET_Align32Free( gameWork->postCaptureBuf[i] );
  }

#if 0
  GFL_BG_ClearScreen( CTVT_FRAME_MAIN_MSG );
  GFL_BG_LoadScreenReq( CTVT_FRAME_MAIN_MSG );
  GFL_BG_SetScroll( CTVT_FRAME_MAIN_MSG , GFL_BG_SCROLL_Y_SET , 0 );

  GFL_BG_ClearScreen( CTVT_FRAME_SUB_MISC );
  GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_MISC );
  GFL_BG_SetVisible( CTVT_FRAME_SUB_BAR , TRUE );
#endif
  //GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);

  //COMM_TVT_RedrawName( work );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
const COMM_TVT_MODE CTVT_GAME_Main( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  const HEAPID heapId = gameWork->heapId;

  int i;

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc(0);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON


  switch( gameWork->state )
  {
  case CGS_FADEIN:
    if( COMM_TVT_GetUpperFade( work ) == TRUE )
    {
      WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
          WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
    }
    else
    {
      WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
          WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
    }
    gameWork->state = CGS_FADEIN_WAIT;
    break;
  case CGS_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      gameWork->state = CGS_CAMERA;
    }
    break;

  case CGS_FADEOUT:
    {
      CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork( work );
      if( gameWork->reqStopCamera == FALSE ||
          CTVT_CAMERA_IsStopCapture( work , camWork ) == TRUE )
      {
        WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
            WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
        gameWork->state = CGS_FADEOUT_WAIT;
      }
    }
    break;

  case CGS_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      if( gameWork->isFinish == FALSE )
      {
        return CTM_TALK;
      }
      else
      {
        return CTM_END;
      }
    }
    break;

  case CGS_CAMERA:
    if( CTVT_GAME_Camera( work, gameWork ) == TRUE )
    {
      gameWork->state = CGS_GAME;
    }
    break;

  case CGS_GAME:
    if( CTVT_GAME_CheckFinishReq( work , gameWork ) == FALSE )
    {
      CTVT_GAME_Update( work , gameWork );
    }
    break;

    //親のリクエストによる終了
  case CGS_END_PARENT_NONE_INIT:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      CTVT_COMM_ExitComm( work , commWork );

      COMM_TVT_SetSusspend( work , TRUE );
      gameWork->state = CGS_END_PARENT_NONE;
      {
        CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork( work );
        CTVT_CAMERA_PlayStopSe(work,camWork);
        gameWork->reqStopCamera = TRUE;
      }
    }
    break;
  case CGS_END_PARENT_NONE:
    if( GFL_UI_TP_GetTrg() == TRUE ||
        GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) )
    {
      gameWork->isFinish = TRUE;
      gameWork->state = CGS_FADEOUT;
      COMM_TVT_SetSusspend( work , TRUE );
      {
        CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork( work );
        CTVT_CAMERA_StopCapture( work , camWork );
        gameWork->reqStopCamera = TRUE;
      }
    }
    break;

    //一人になったので終了
  case CGS_END_MEMBER_NONE_INIT:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      CTVT_COMM_ExitComm( work , commWork );

      COMM_TVT_SetSusspend( work , TRUE );
      gameWork->state = CGS_END_MEMBER_NONE;
      {
        CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork( work );
        CTVT_CAMERA_PlayStopSe(work,camWork);
        gameWork->reqStopCamera = TRUE;
      }
    }
    break;
  case CGS_END_MEMBER_NONE:
    if( GFL_UI_TP_GetTrg() == TRUE ||
        GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) )
    {
      gameWork->isFinish = TRUE;
      gameWork->state = CGS_FADEOUT;
      COMM_TVT_SetSusspend( work , TRUE );
      {
        CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork( work );
        CTVT_CAMERA_StopCapture( work , camWork );
        gameWork->reqStopCamera = TRUE;
      }
    }
    break;
  }

  // COMM_TVT_Mainからコピーしてきた
  GFL_CLACT_SYS_Main();
  PRINTSYS_QUE_Main( COMM_TVT_GetPrintQue(work) );

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc(0);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

  return CTM_GAME;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラキャプチャ中フラグ取得
 *
 *	@param	CTVT_GAME_WORK *gameWork 
 *
 *	@return TRUE:カメラキャプチャ中
 */
//-----------------------------------------------------------------------------
BOOL CTVT_GAME_GetIsCamCapture( CTVT_GAME_WORK *gameWork )
{
  return gameWork->isCamCapture;
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船出現
 *
 *	@param	CTVT_GAME_WORK  *gameWork
 *
 *	@retval TRUE  出現した
 *	@retval FALSE 出現できなかった
 */
//-----------------------------------------------------------------------------
BOOL CTVT_GAME_BalloonAppear( CTVT_GAME_WORK *p_wk )
{
  u8  i;

  // 空いてるタスクを探す
  for( i=0; i<BALLOON_MAX; i++ )
  {
    if( p_wk->balloon_wk[i].tcb == NULL )
    {
      p_wk->balloon_wk[i].tcb = GFL_TCB_AddTask( p_wk->task, TaskFunc, &p_wk->balloon_wk[i], 0 );
      GF_ASSERT( p_wk->balloon_wk[i].tcb != NULL );

      if( p_wk->balloon_wk[i].tcb != NULL )
      {
        // 誰の風船か
        u8  idx = balloon_find_idx( p_wk );

        // 風船出現位置調整
        {
          u8  rnd;
          u8  i;
          // 前の乱数と同じにならないようにする
          // 無限ループになるのが怖いから最大10回にする
          for( i=0; i<10; i++ )
          {
            rnd = GFL_STD_MtRand(BALLOON_RND_W_MAX);
            if( rnd != p_wk->balloon_rnd_w )
            {
              break;
            }
          }
          p_wk->balloon_rnd_w = rnd;

          for( i=0; i<10; i++ )
          {
            rnd = GFL_STD_MtRand(BALLOON_RND_V_MAX);
            if( rnd != p_wk->balloon_rnd_v )
            {
              break;
            }
          }
          p_wk->balloon_rnd_v = rnd;

          // Zをずらす為の調整用
          p_wk->balloon_z_cnt++;
          p_wk->balloon_z_cnt %= 3;
        }

        // 風船出現
        balloon_set( p_wk, &p_wk->balloon_wk[i], p_wk->unit_idx[idx] );
        p_wk->balloon_wk[i].parent_g3dUtil = p_wk->g3dUtil;
        // ボーナス風船判定
        {
          if( p_wk->bonus_tbl[idx][ p_wk->bonus_tbl_idx[idx] ] > p_wk->timeLimit )
          {
            p_wk->balloon_wk[i].type = BALLOON_BONUS;
            p_wk->bonus_tbl[idx][ p_wk->bonus_tbl_idx[idx] ] = 0;
            p_wk->bonus_tbl_idx[idx]++;
            p_wk->bonus_tbl_idx[idx] %= 5;
            //ICHI_Printf("ボーナス風船出現 %d \n", idx);
          }
          else
          {
            p_wk->balloon_wk[i].type = BALLOON_NORMAL;
          }
        }

        // 出現した風船が他の風船と重なっているかチェック
        {
          u8  j;
          for( j=0; j<BALLOON_MAX; j++ )
          {
            // 違う風船 かつ サイズが同じ かつ 表示している
            // ならチェックする
            if( ( j != i ) && ( p_wk->balloon_wk[i].size == p_wk->balloon_wk[j].size ) && ( p_wk->balloon_wk[j].visible == TRUE ) )
            {
              // 壁判定
              collision_vec( 
                  &p_wk->balloon_wk[i].status.trans.x, &p_wk->balloon_wk[i].status.trans.y,
                  &p_wk->balloon_wk[j].status.trans.x, &p_wk->balloon_wk[j].status.trans.y,
                  p_wk->balloon_wk[i].status.scale.x * 140, 1 );

            }
          }
        }

        return TRUE;
      }
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信から送られてくる乱数シード値を格納
 *
 *	@param	p_wk    ゲーム管理構造体
 *	        data    設定するゲーム情報
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CommRndSeedSet( CTVT_GAME_WORK *p_wk, const u32 rnd_seed )
{
  // 受信データを更新
  p_wk->rnd_seed = rnd_seed;
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信から送られてくるフレーム経過数を格納
 *
 *	@param	p_wk    ゲーム管理構造体
 *	        frame   受信したフレーム
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CommFrameSet( CTVT_GAME_WORK *p_wk, const u16 frame )
{
  p_wk->frame = frame;
  p_wk->isRecvFrame = TRUE;   // 受信したフラグを立てる
}

//----------------------------------------------------------------------------
/**
 *	@brief  経過フレーム取得
 *
 *	@param	CTVT_GAME_WORK  *gameWork 
 *
 *	@return u16 経過フレーム
 */
//-----------------------------------------------------------------------------
u16 CTVT_GAME_GetFrame( CTVT_GAME_WORK *gameWork )
{
  return gameWork->frame;
}

//----------------------------------------------------------------------------
/**
 *	@brief  親のフレーム取得
 *
 *	@param	CTVT_GAME_WORK  *gameWork 
 *
 *	@return u16 経過フレーム
 */
//-----------------------------------------------------------------------------
u16 CTVT_GAME_GetOyaFrame( CTVT_GAME_WORK *gameWork )
{
  return gameWork->oya_frame;
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信からTP座標が送られてきた場合の設定関数
 *	        内部で当たり判定する
 *
 *	@param	commWork    通信構造体
 *	        p_wk        ゲーム管理構造体
 *	        data        設定するゲームデータ
 *	        netID　     送信者のNetID
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CommDataSet_TP( CTVT_COMM_WORK *commWork, CTVT_GAME_WORK *p_wk, const CTVT_GAME_SEND_DATA *data, int netID )
{
  // 親しかここは通らないはず
  BOOL  ret;
  u16   ret_idx;    // どの風船が当たったかIDXを格納

  ret = balloon_hit_check( p_wk, data->tpx, data->tpy, &ret_idx );
  // Hit Check
  // 当たりがある → 送信
  if( ret == TRUE )
  {
    CTVT_GAME_SEND_DATA send_data;
    send_data.burst_idx = ret_idx;
    send_data.burst_netID = netID;
    CTVT_COMM_SendGameReq( commWork, send_data, CCGDT_BURST );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  指定したIDXの風船を割る（タスクから消す）
 *	        スコアの計算も行う
 *
 *	@param	CTVT_GAME_WORK *p_wk
 *	@param	idx   割りたい風船のIDX(受信したものをそのまま指定してOK)
 *	@param  netID 割った人のnetID(受信したものをそのまま指定してOK)
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_BalloonDestroy( CTVT_GAME_WORK *p_wk, u16 idx, u8 netId )
{
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  // 得点計算

  // ポイント加算
  if( p_wk->balloon_wk[ idx ].unit_idx == netId )
  {
    if( p_wk->balloon_wk[ idx ].type == BALLOON_BONUS )
    {
      // ボーナス風船
      p_wk->score[ netId ] += 4;
      //ICHI_Printf("NetID %d score +4\n", netId);
    }
    else
    {
      // 通常風船
      p_wk->score[ netId ] += 2;
      //ICHI_Printf("NetID %d score +2\n", netId);
    }
    // 自分だけ鳴らす
    if( selfId == netId )
    {
      // SE再生
      PMSND_PlaySystemSE( SEQ_SE_DECIDE4 );
      //PMSND_PlaySystemSE( SEQ_SE_LCG_01 );
    }
  }
  // ポイント減算
  else
  {
    if( p_wk->balloon_wk[ idx ].type == BALLOON_BONUS )
    {
      // ボーナス風船
      p_wk->score[ netId ] -= 4;
      //ICHI_Printf("NetID %d score -4\n", netId);
    }
    else
    {
      // 通常風船
      p_wk->score[ netId ] -= 2;
      //ICHI_Printf("NetID %d score -2\n", netId);
    }

    // 自分だけ鳴らす
    if( selfId == netId )
    {
      // SE再生
      PMSND_PlaySystemSE( SEQ_SE_BEEP );
      //PMSND_PlaySystemSE( SEQ_SE_LCG_01 );
    }
  }

  // 得点表示更新(自分のみ)
  if( netId == selfId )
  {
    int score = p_wk->score[ selfId ];

    // + or -
    if( score < 0 )
    {
      GFL_CLACT_WK_SetAnmSeq( p_wk->clwkScore[0], CPOA_NUM_M );
    }
    else
    {
      GFL_CLACT_WK_SetAnmSeq( p_wk->clwkScore[0], CPOA_NUM_P );
    }

    // 得点表示
    score = GFL_STD_Abs(score); // 絶対値
    // 百の位
    GFL_CLACT_WK_SetAnmSeq( p_wk->clwkScore[1], CPOA_NUM_0 + (score/100));
    // 十の位
    GFL_CLACT_WK_SetAnmSeq( p_wk->clwkScore[2], CPOA_NUM_0 + (score/10));
    // 一の位
    GFL_CLACT_WK_SetAnmSeq( p_wk->clwkScore[3], CPOA_NUM_0 + (score%10));

  }

  // タスク消去
  //CTVT_BALLOON_Destroy( &p_wk->balloon_wk[ idx ] );
  // 風船をHit時に変える -> 消去まで自動で行う
  p_wk->balloon_wk[ idx ].type = BALLOON_HIT;

}

//----------------------------------------------------------------------------
/**
 *	@brief  通信から送られてくるゲーム情報を格納
 *
 *	@param	p_wk    ゲーム管理構造体
 *	        score   設定するゲーム情報
 */
//-----------------------------------------------------------------------------
void CTVT_GAME_CommScoreSet( CTVT_GAME_WORK *p_wk, const CTVT_GAME_SEND_SCORE *score )
{
  u8 i;

  for( i=0; i<CTVT_GAME_MEMBER_NUM; i++ )
  {
    // 受信データを更新
    p_wk->score[i] = score->score[i];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラで顔を撮るモード
 *
 *	@param	COMM_TVT_WORK *work 
 *	@param	*gameWork 
 *
 *	@return 終了時TRUEを返す
 */
//-----------------------------------------------------------------------------
static BOOL CTVT_GAME_Camera( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  CTVT_CAMERA_WORK *camWork = COMM_TVT_GetCameraWork(work);
  CAMERA_SYSTEM_WORK *camSys = CTVT_CAMERA_GetCamSys( work, camWork );


  // シーケンス管理
  enum{
    SEQ_INIT_SYNC_INIT,  // カメラ同期開始
    SEQ_INIT_SYNC_WAIT,  // カメラ同期待ち
    SEQ_INIT,       // 初期化
    SEQ_MAIN,       // カメラ撮る
    SEQ_RECV_WAIT,  // カメラキャプチャ受信待ち
    SEQ_EXIT_SYNC_INIT,  // 終了前同期
    SEQ_EXIT_SYNC_WAIT,  // 終了前同期待ち
    SEQ_EXIT,       // 終了
    SEQ_NEXT,       // 次のモードへ
  };

  switch( gameWork->seq ){
  case SEQ_INIT_SYNC_INIT:
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_HANDLE_TimeSyncStart( pNet, CTVT_COMM_GAME_CAMERA_INIT_SYNC, WB_NET_COMM_TVT );
      gameWork->seq = SEQ_INIT_SYNC_WAIT;
    }
    break;
  case SEQ_INIT_SYNC_WAIT:
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CTVT_COMM_GAME_CAMERA_INIT_SYNC , WB_NET_COMM_TVT ) == TRUE )
      {
        gameWork->seq = SEQ_INIT;
      }
    }
    break;
  case SEQ_INIT:
    {
      GFL_BG_SetVisible( CTVT_GAME_FRAME_M_CAMERA, VISIBLE_ON );
      GFL_BG_SetVisible( CTVT_GAME_FRAME_S_BG,     VISIBLE_ON );
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkCam, TRUE );

      // カメラ撮影用のリソース
      // BGロード
      {
        // 上画面
        GFL_ARC_UTIL_TransVramPalette( ARCID_COMM_TVT_GRA, NARC_comm_tvt_tv_camera_bg_NCLR , 
            PALTYPE_SUB_BG, CTVT_GAME_PAL_BG_S_BG*32, 32*5, gameWork->heapId );
        GFL_ARC_UTIL_TransVramBgCharacter( ARCID_COMM_TVT_GRA, NARC_comm_tvt_tv_camera_bg_NCGR,
            CTVT_GAME_FRAME_S_BG, 0, 0, FALSE, gameWork->heapId );
        GFL_ARC_UTIL_TransVramScreen( ARCID_COMM_TVT_GRA , NARC_comm_tvt_tv_camera_bg_NSCR, 
            CTVT_GAME_FRAME_S_BG, 0, 0, FALSE, gameWork->heapId );

        GFL_BG_LoadScreenReq( CTVT_GAME_FRAME_S_BG );
      }

      CAMERA_SYS_StartCapture( camSys );
      gameWork->isCamCapture = TRUE;

      // カメラキャプチャ送信用に低優先度通信のシステムを作成
      GFL_NET_LDATA_InitSystem( gameWork->heapId, FALSE );

      // 低優先度送信システムにカメラ受信バッファを渡す
      {
        const u8 selfIdx = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
        const u8 connectNum = COMM_TVT_GetConnectNum( work );
        u8 netID;       // netID
        u8 bufIdx =0;    // bufIdx
        for( netID=0;netID<connectNum;netID++ )
        {
          ICHI_Printf("create post buf selfID %d netID %d \n", selfIdx, netID);

          // 自分のNetIDは受信の設定にしない
          if( netID != selfIdx )
          {
            // バッファ設定
            GFL_NET_LDATA_CreatePostBuffer_SetAddres( CTVT_CAPTURE_SIZE, netID, gameWork->heapId, gameWork->postCaptureBuf[bufIdx] );
            bufIdx++;
            gameWork->CaptureBufUseBit |= 1<<netID;

            ICHI_Printf("バッファ設定 netID %d \n", netID);
          }
        }
        ICHI_Printf("camera capture 通信設定完了 ");
        ICHI_Printf("UserBit %x \n", gameWork->CaptureBufUseBit);
      }

      gameWork->seq = SEQ_MAIN; // 次のシーケンスへ
    }
    break;
  case SEQ_MAIN:
    {

      // カメラ更新
      if( COMM_TVT_CanUseCamera() == TRUE )
      {
        CAMERA_SYS_UpdateCameta( camSys );
      }

      // Aボタンでカメラ撮る
      if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_A)
      {
        // キャプチャデータを128x128でコピー
        {
          int   iy;
          const u16 capPosX = 64;   // トリミングX 256-128/2
          const u16 capPosY = 32;   // トリミングY 192-128/2
          const u16 capSizeX = CTVT_TEX_SIZE; // 128
          const u16 capSizeY = CTVT_TEX_SIZE; // 128

          for( iy=0; iy< CTVT_TEX_SIZE; iy++ )
          {
            const u32 capTopAdr = (u32)gameWork->scrBuf + (capPosY+iy)*256*2 + capPosX*2;
            const u32 bufTopAdr = (u32)gameWork->capBuf + capSizeX*iy*2;

            GFL_STD_MemCopy32( (void*)capTopAdr, (void*)bufTopAdr, capSizeX*2 );
          }

          // カメラキャプチャ送信リクエスト
          {
            // 通常送信
            //CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
            //CTVT_COMM_SendGameReq_Camera( work, commWork, gameWork->capBuf );

            // 低優先度送信で一気に送る
            GFL_NET_LDATA_SetSendData( gameWork->capBuf, CTVT_CAPTURE_SIZE, gameWork->CaptureBufUseBit, FALSE );
            ICHI_Printf("Send Data CamCapture\n");
          }
        }

        // カメラのキャプチャはSTOP
        CAMERA_SYS_StopCapture( camSys );
        gameWork->isCamCapture = FALSE;

        gameWork->seq = SEQ_RECV_WAIT; // 次のシーケンスへ
      }
    }
    break;
  case SEQ_RECV_WAIT:
    {
      if( GFL_NET_LDATA_IsFinishSend() == FALSE )
      {
        // 送信完了まで
        break;
      }

      // 受信完了待ち
      {
        u8    i;
        BOOL  ret = TRUE;   // 次シーケンスへいってよいか判定用
        const u8 connectNum = COMM_TVT_GetConnectNum( work );

        // ゲーム人数分回す
        for( i=0;i<connectNum;i++ )
        {
          // 送信したNetIDのみチェック
          if( gameWork->CaptureBufUseBit & (1<<i) )
          {
            if( GFL_NET_LDATA_IsFinishPost(i) == TRUE )
            {
              ICHI_Printf("NetId %d から受信完了 ", i);
              ICHI_Printf("size %x \n", GFL_NET_LDATA_GetPostDataSize(i));
            }
            else
            {
              ret = FALSE;
            }
          }
        }

        // 全員分の受信が完了した
        if( ret == TRUE )
        {
          // データコピーする？

          gameWork->seq = SEQ_EXIT_SYNC_INIT; // 次のシーケンスへ
        }
      }
    }
    break;
  case SEQ_EXIT_SYNC_INIT:  // 終了前同期
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_HANDLE_TimeSyncStart( pNet, CTVT_COMM_GAME_CAMERA_END_SYNC, WB_NET_COMM_TVT );
      gameWork->seq = SEQ_EXIT_SYNC_WAIT;

      ICHI_Printf("カメラモード終了同期待ち\n");
    }
    break;
  case SEQ_EXIT_SYNC_WAIT:  // 終了前同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CTVT_COMM_GAME_CAMERA_END_SYNC , WB_NET_COMM_TVT ) == TRUE )
      {
        gameWork->seq = SEQ_EXIT;
        ICHI_Printf("カメラモード終了同期完了\n");
      }
    }
    break;
  case SEQ_EXIT:
    {
      // 低優先度通信終わり
      GFL_NET_LDATA_ExitSystem();

      GFL_BG_SetVisible( CTVT_GAME_FRAME_M_CAMERA, VISIBLE_OFF );
      GFL_BG_ClearFrame( CTVT_GAME_FRAME_M_CAMERA );
      GFL_BG_ClearFrame( CTVT_GAME_FRAME_S_BG );
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkCam, FALSE );

      gameWork->seq = SEQ_NEXT; // 次のシーケンスへ
    }
    break;
  case SEQ_NEXT:
#if (defined(SDK_TWL))
    {
      BOOL prev = OS_GetPrintBlockingMode();
      OS_SetPrintBlockingMode( TRUE );
      // WRAMの状態を表示
      MI_DumpWramList_B();
      MI_DumpWramList_C();
      OS_SetPrintBlockingMode( prev );
      // 他のコンポーネントと共存できない
      DSP_UnloadG711();
    }
#endif
    gameWork->seq = 0;    // 次モードの為、初期化
    return  TRUE;
  }

  return FALSE;
}

//--------------------------------------------------------------
//	更新
//  キー操作・モード変更周り
//--------------------------------------------------------------
static void CTVT_GAME_Update( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  CTVT_CAMERA_WORK  *camWork  = COMM_TVT_GetCameraWork(work);
  CTVT_COMM_WORK    *commWork = COMM_TVT_GetCommWork( work );
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  // シーケンス管理
  enum{
    SEQ_INIT_RND,         // 乱数Seed値送る(親のみ)
    SEQ_INIT_SYNC_INIT,   // 初期化開始同期
    SEQ_INIT_SYNC_WAIT,   // 初期化開始同期待ち
    SEQ_INIT,             // 初期化
    SEQ_TITLE,            // タイトル
    SEQ_MAIN_SYNC_INIT,   // ゲーム開始同期
    SEQ_MAIN_SYNC_WAIT,   // ゲーム開始同期待ち
    SEQ_MAIN,             // ゲームメイン
    SEQ_SCORE_SYNC_INIT,  // スコア同期
    SEQ_SCORE_SYNC_WAIT,  // スコア同期待ち
    SEQ_SCORE,            // スコア
    SEQ_EXIT_SYNC_INIT,   // ゲーム終了同期
    SEQ_EXIT_SYNC_WAIT,   // ゲーム終了同期待ち
    SEQ_EXIT,             // 終了
    SEQ_NEXT,       // 次のモードへ
  };

  // シーケンス別処理
  switch( gameWork->seq ){
  case SEQ_INIT_RND:
    // 親なら乱数Seed値送信
    if( selfId == 0 )
    {
      CTVT_GAME_SEND_DATA data;

      RTCDate	date;
      RTCTime	time;
      u32		seed;

      RTC_GetDateTime(&date,&time);

      seed = date.year+date.month*0x100+date.day*0x10000+time.hour*0x100000+(time.minute+time.second)*0x1000000;

      // 通信データ送信
      data.rnd_seed = seed;
      CTVT_COMM_SendGameReq( commWork, data, CCGDT_RND );
    }
    gameWork->seq = SEQ_INIT_SYNC_INIT;
    break;
  case SEQ_INIT_SYNC_INIT:   // ゲーム開始同期
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_HANDLE_TimeSyncStart( pNet, CTVT_COMM_GAME_TITLE_SYNC, WB_NET_COMM_TVT );
      gameWork->seq = SEQ_INIT_SYNC_WAIT;

      ICHI_Printf("初期化開始同期待ち\n");
    }
    break;
  case SEQ_INIT_SYNC_WAIT:   // ゲーム開始同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CTVT_COMM_GAME_TITLE_SYNC , WB_NET_COMM_TVT ) == TRUE )
      {
        gameWork->seq = SEQ_INIT;
        ICHI_Printf("初期化開始同期完了\n");
      }
    }
    break;
  case SEQ_INIT:             // 初期化
    game_update_seq_init( work, gameWork );
    gameWork->seq = SEQ_TITLE;
    break;
  case SEQ_TITLE:
    if( game_update_seq_title( work, gameWork ) )
    {
      gameWork->seq = SEQ_MAIN_SYNC_INIT;
    }
    break;
  case SEQ_MAIN_SYNC_INIT:   // ゲーム開始同期
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_HANDLE_TimeSyncStart( pNet, CTVT_COMM_GAME_BEFORE_SYNC, WB_NET_COMM_TVT );
      gameWork->seq = SEQ_MAIN_SYNC_WAIT;

      ICHI_Printf("ゲーム開始同期待ち\n");
    }
    break;
  case SEQ_MAIN_SYNC_WAIT:   // ゲーム開始同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CTVT_COMM_GAME_BEFORE_SYNC , WB_NET_COMM_TVT ) == TRUE )
      {
        gameWork->seq = SEQ_MAIN;
        PMSND_PlaySystemSE( SEQ_SE_LCG_03 );    // スタートのホイッスル音
        ICHI_Printf("ゲーム開始同期完了\n");
      }
    }
    break;
  case SEQ_MAIN:             // ゲームメイン
    if( game_update_seq_main( work, gameWork ) )
    {
      gameWork->seq = SEQ_SCORE_SYNC_INIT;
    }
    break;
  case SEQ_SCORE_SYNC_INIT:  // スコア同期
    {
      // 風船だけ更新
      {
        // 経過フレーム数をインクリメント
        // 親、子関係なくしないと、子が止まる可能性がある
        gameWork->frame++;

        GRAPHIC_Draw( gameWork );
      }
      // 全ての風船の終了まで待つ
      if( balloon_end_check( gameWork ) == TRUE )
      {
        GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
        GFL_NET_HANDLE_TimeSyncStart( pNet, CTVT_COMM_GAME_SCORE_SYNC, WB_NET_COMM_TVT );
        gameWork->oya_frame = 0;    // 0以上だと通信メインで自動送信してしまう
        gameWork->seq = SEQ_SCORE_SYNC_WAIT;

        ICHI_Printf("スコア同期待ち\n");
      }
    }
    break;
  case SEQ_SCORE_SYNC_WAIT:  // スコア同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CTVT_COMM_GAME_SCORE_SYNC , WB_NET_COMM_TVT ) == TRUE )
      {
        gameWork->seq = SEQ_SCORE;
        ICHI_Printf("スコア同期完了\n");
      }
    }
    break;
  case SEQ_SCORE:            // スコア
    game_update_seq_score( work, gameWork );
    gameWork->seq = SEQ_EXIT_SYNC_INIT;
    break;
  case SEQ_EXIT_SYNC_INIT:   // ゲーム終了同期
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_HANDLE_TimeSyncStart( pNet, CTVT_COMM_GAME_END_SYNC, WB_NET_COMM_TVT );
      gameWork->seq = SEQ_EXIT_SYNC_WAIT;

      ICHI_Printf("ゲーム終了同期待ち\n");
    }
    break;
  case SEQ_EXIT_SYNC_WAIT:   // ゲーム終了同期待ち
    {
      GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
      if( GFL_NET_HANDLE_IsTimeSync( pNet, CTVT_COMM_GAME_END_SYNC , WB_NET_COMM_TVT ) == TRUE )
      {
        gameWork->seq = SEQ_EXIT;
        ICHI_Printf("ゲーム終了同期完了\n");
      }
    }
    break;
  case SEQ_EXIT:             // 終了
    if( GFL_UI_KEY_GetTrg() & PAD_ALL_MASK )
    {
      game_update_seq_exit( work, gameWork );
      gameWork->seq = SEQ_NEXT;
    }
    break;
  case SEQ_NEXT:
    gameWork->seq = 0;    // 次モードの為、初期化
    //gameWork->state = CGS_FADEOUT;
    //PMSND_PlaySystemSE( CTVT_SND_CANCEL );
    return;
  }

  // =========================
  // メッセージ
#if 0
  if( gameWork->isUpdateMsgMain == TRUE )
  {
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( gameWork->msgWinMain )) == FALSE )
    {
      GFL_BMPWIN_TransVramCharacter( gameWork->msgWinMain );
      GFL_BMPWIN_MakeScreen( gameWork->msgWinMain );
      GFL_BG_LoadScreenReq(CTVT_GAME_FRAME_M_MSG);
      gameWork->isUpdateMsgMain = FALSE;
    }
  }
#endif

  if( gameWork->isUpdateMsgSub == TRUE )
  {
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( gameWork->msgWinSub )) == FALSE )
    {
      GFL_BMPWIN_TransVramCharacter( gameWork->msgWinSub );
      GFL_BMPWIN_MakeScreen( gameWork->msgWinSub );
      GFL_BG_LoadScreenReq(CTVT_GAME_FRAME_S_MSG);
      gameWork->isUpdateMsgSub = FALSE;
    }
  }

#if 0
  // デバッグカメラ
  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L)
  {
    VecFx32 test_pos;

    GFL_G3D_CAMERA_GetPos( gameWork->p_camera, &test_pos );

    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_X){
      test_pos.z -= FX32_ONE /2;
    }else if(GFL_UI_KEY_GetCont()&PAD_BUTTON_Y){
      test_pos.z += FX32_ONE /2;
    }else if(GFL_UI_KEY_GetCont()&PAD_KEY_UP){
      test_pos.y -= FX32_ONE /2;
    }else if(GFL_UI_KEY_GetCont()&PAD_KEY_DOWN){
      test_pos.y += FX32_ONE /2;
    }else if(GFL_UI_KEY_GetCont()&PAD_KEY_LEFT){
      test_pos.x -= FX32_ONE /2;
    }else if(GFL_UI_KEY_GetCont()&PAD_KEY_RIGHT){
      test_pos.x += FX32_ONE /2;
    }
    GFL_G3D_CAMERA_SetPos( gameWork->p_camera, &test_pos );
    OS_Printf("pos.y=%x pos.z=%x \n", test_pos.y, test_pos.z);
  }
#endif


#if 0
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    //gameWork->state = CGS_FADEOUT;
    //PMSND_PlaySystemSE( CTVT_SND_CANCEL );
    gameWork->state = CGS_CAMERA;
    gameWork->seq = 0;
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    gameWork->state = CGS_FADEOUT;
    //PMSND_PlaySystemSE( CTVT_SND_CANCEL );
  }

#endif
}


//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:初期化
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_init( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8  i;
  const HEAPID heapID = gameWork->heapId;
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  const u8 connectNum = COMM_TVT_GetConnectNum( work );

  // 乱数初期化
  GFL_STD_MtRandInit( gameWork->rnd_seed );
  ICHI_Printf("乱数Seed値 %d \n", gameWork->rnd_seed);
  ICHI_Printf("送信データサイズ:%d \n", sizeof(CTVT_GAME_SEND_DATA));

  // カメラキャプチャ128x128を
  // 64x64に変化し、それをテクスチャとして張る
  for(i=0; i<CTVT_GAME_MEMBER_NUM; i++ )
  {
    gameWork->direct_tex[i] = GFL_NET_Align32Alloc( gameWork->heapId, 64*64*2);
  }

#if (defined(SDK_TWL))
  // DSPグラフィックを使用して、画像サイズを変更する
  // ※DSPを使うとプログラムの常駐サイズが増える
  {
    u8  cap_cnt = 0;

    FSFile  file[1];

    DSP_OpenStaticComponentGraphics(file);
    if (!DSP_LoadGraphics(file, 0xFF, 0xFF))
    {
      GF_PANIC("can't allocate WRAM Slot");
    }

    for(i=0; i<connectNum; i++ )
    {
      void  *src;   // 変換元
      void  *dst;   // 変換先
      BOOL  ret;
      f32  factx, facty;
      factx = DSP_CalcScalingFactorF32(CTVT_TEX_SIZE, OUTPUT_WIDTH);
      facty = DSP_CalcScalingFactorF32(CTVT_TEX_SIZE, OUTPUT_HEIGHT);

      ICHI_Printf("変換率 %d\n", factx);

      // NetIDが自分だと自分のキャプチャを使用する
      if( i == selfId )
      {
        src = gameWork->capBuf;
      }
      else
      {
        src = gameWork->postCaptureBuf[ cap_cnt++ ];
      }

      dst = gameWork->direct_tex[ i ];

      // 変換
      ret = DSP_Scaling(src, dst, CTVT_TEX_SIZE, CTVT_TEX_SIZE, factx, facty, DSP_GRAPHICS_SCALING_MODE_BILINEAR);
      if( ret == FALSE )
      {
        OS_Printf("DSPグラフィック 正常に変換が行われませんでした。\n");
      }
    }

    // DSP アンロード
    DSP_UnloadGraphics( );
  }
#endif

#if (defined(SDK_TWL))
  // 3Dにカメラキャプチャを貼り付ける
  {
    BOOL  rc;
    u32   dst;    // 転送先の実アドレス
    void* src;    // 転送元データ
    u32   size;   // 転送サイズ
    u8    type;   // 貼り付ける写真 3枚ある

    for(i=0; i<connectNum; i++ )
    {
      // ほんとはループ3回だが、今はボーナス風船をわかりやすくするため2回
      for( type=0; type<1; type++ )
      {
        u16 unit_res_cnt = GFL_G3D_UTIL_GetUnitResCount( gameWork->g3dUtil, gameWork->unit_idx[i] );
        GFL_G3D_RES* res = GFL_G3D_UTIL_GetResHandle( gameWork->g3dUtil, unit_res_cnt*i + type );
        NNSG3dTexKey texkey = GFL_G3D_GetTextureDataVramKey( res );
        u32 tex_adr = NNS_GfdGetTexKeyAddr(texkey);

        ICHI_Printf("テクスチャ貼り付け\n");
        // 転送
        {
          dst = tex_adr;
          src = gameWork->direct_tex[i];
          size = (OUTPUT_WIDTH*OUTPUT_HEIGHT*2);
          rc = NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_VRAM, dst, src, size);
          GF_ASSERT(rc);
          ICHI_Printf("dst %d src %d size %d \n", dst, src, size);
        }
      }
    }
  }
#endif

  // ゲーム用のリソース
  // BGロード
  {

    // 下画面
    GFL_ARC_UTIL_TransVramPalette( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_l_NCLR , 
        PALTYPE_MAIN_BG, CTVT_GAME_PAL_BG_M_BG*32, 32*5, heapID );
    GFL_ARC_UTIL_TransVramBgCharacter( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_l_NCGR,
        CTVT_GAME_FRAME_M_BG, 0, 0, FALSE, heapID );
    GFL_ARC_UTIL_TransVramScreen( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_l_NSCR, 
        CTVT_GAME_FRAME_M_BG, 0, 0, FALSE, heapID );

    // 上画面
    GFL_ARC_UTIL_TransVramPalette( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_u_NCLR , 
        PALTYPE_SUB_BG, CTVT_GAME_PAL_BG_S_BG*32, 32*5, heapID );
    GFL_ARC_UTIL_TransVramBgCharacter( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_u_NCGR,
        CTVT_GAME_FRAME_S_BG, 0, 0, FALSE, heapID );
    GFL_ARC_UTIL_TransVramScreen( ARCID_TVT_BALLOON, NARC_tvt_balloon_tv_game_bg_u_NSCR, 
        CTVT_GAME_FRAME_S_BG,  0, 0, FALSE, heapID );

    GFL_BG_SetVisible( CTVT_GAME_FRAME_M_3D,    VISIBLE_ON );
    //GFL_BG_SetVisible( CTVT_GAME_FRAME_M_MSG,   VISIBLE_ON );
    GFL_BG_SetVisible( CTVT_GAME_FRAME_M_BG,    VISIBLE_ON );
    GFL_BG_SetVisible( CTVT_GAME_FRAME_S_MSG,   VISIBLE_ON );
    GFL_BG_SetVisible( CTVT_GAME_FRAME_S_BG,    VISIBLE_ON );

    GFL_BG_LoadScreenReq( CTVT_GAME_FRAME_M_BG );
    GFL_BG_LoadScreenReq( CTVT_GAME_FRAME_S_BG );
  }
  // OBJ作成
  {
    GFL_CLWK_DATA cellInitData;

    // タイトル
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 96;
    cellInitData.anmseq = CCOA_TITLE;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;

    gameWork->clwkTitle = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_COMMON_PLT],
        gameWork->cellResIdx[CGOR_COMMON_NCG],
        gameWork->cellResIdx[CGOR_COMMON_ANM],
        &cellInitData ,CLSYS_DRAW_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitle , TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkTitle , TRUE );

    // タイトル項目
    for( i=0; i<3; i++ )
    {
      cellInitData.pos_x = 128;
      cellInitData.pos_y = 80 + (i*16);
      cellInitData.anmseq = CCOA_START + i;
      gameWork->clwkTitleItem[i] = GFL_CLACT_WK_Create( gameWork->cellUnit,
          gameWork->cellResIdx[CGOR_COMMON_PLT],
          gameWork->cellResIdx[CGOR_COMMON_NCG],
          gameWork->cellResIdx[CGOR_COMMON_ANM],
          &cellInitData ,CLSYS_DRAW_MAIN, heapID );
      GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitleItem[i] , TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( gameWork->clwkTitleItem[i] , TRUE );
    }


    // 制限時間の数字
    cellInitData.pos_x = 126;
    cellInitData.pos_y = 96;
    cellInitData.anmseq = CPOA_NUM_6;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;

    gameWork->clwkTime[0] = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_PLAY_PLT],
        gameWork->cellResIdx[CGOR_PLAY_NCG],
        gameWork->cellResIdx[CGOR_PLAY_ANM],
        &cellInitData ,CLSYS_DRAW_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTime[0], FALSE );

    cellInitData.pos_x = 142;
    cellInitData.anmseq = CPOA_NUM_0;
    gameWork->clwkTime[1] = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_PLAY_PLT],
        gameWork->cellResIdx[CGOR_PLAY_NCG],
        gameWork->cellResIdx[CGOR_PLAY_ANM],
        &cellInitData ,CLSYS_DRAW_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTime[1], FALSE );

    // TIME文字
    cellInitData.pos_x = 72;
    cellInitData.pos_y = 96;
    cellInitData.anmseq = CPOA_TIME;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;

    gameWork->clwkText = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_PLAY_PLT],
        gameWork->cellResIdx[CGOR_PLAY_NCG],
        gameWork->cellResIdx[CGOR_PLAY_ANM],
        &cellInitData ,CLSYS_DRAW_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkText, FALSE );

    // SCORE
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 158;
    cellInitData.anmseq = CPOA_SCORE;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;

    gameWork->clwkScoreText = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_PLAY_PLT],
        gameWork->cellResIdx[CGOR_PLAY_NCG],
        gameWork->cellResIdx[CGOR_PLAY_ANM],
        &cellInitData ,CLSYS_DRAW_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkScoreText, FALSE );

    // 得点
    cellInitData.pos_x = 176;
    cellInitData.anmseq = CPOA_NUM_P;
    gameWork->clwkScore[0] = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_PLAY_PLT],
        gameWork->cellResIdx[CGOR_PLAY_NCG],
        gameWork->cellResIdx[CGOR_PLAY_ANM],
        &cellInitData ,CLSYS_DRAW_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkScore[0], FALSE );

    cellInitData.pos_x = 192;
    cellInitData.anmseq = CPOA_NUM_0;
    gameWork->clwkScore[1] = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_PLAY_PLT],
        gameWork->cellResIdx[CGOR_PLAY_NCG],
        gameWork->cellResIdx[CGOR_PLAY_ANM],
        &cellInitData ,CLSYS_DRAW_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkScore[1], FALSE );
    cellInitData.pos_x = 208;
    gameWork->clwkScore[2] = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_PLAY_PLT],
        gameWork->cellResIdx[CGOR_PLAY_NCG],
        gameWork->cellResIdx[CGOR_PLAY_ANM],
        &cellInitData ,CLSYS_DRAW_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkScore[2], FALSE );
    cellInitData.pos_x = 224;
    gameWork->clwkScore[3] = GFL_CLACT_WK_Create( gameWork->cellUnit,
        gameWork->cellResIdx[CGOR_PLAY_PLT],
        gameWork->cellResIdx[CGOR_PLAY_NCG],
        gameWork->cellResIdx[CGOR_PLAY_ANM],
        &cellInitData ,CLSYS_DRAW_SUB, heapID );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkScore[3], FALSE );
  }

  // 風船用タスク作成
  // タスクシステムを作るのに必要なメモリサイズを計算
  gameWork->taskMem = GFL_HEAP_AllocMemory( gameWork->heapId, GFL_TCB_CalcSystemWorkSize(BALLOON_MAX) );
  // タスクシステムを作成
  gameWork->task = GFL_TCB_Init( BALLOON_MAX, gameWork->taskMem );

  // 風船初期化
  for( i=0; i<BALLOON_MAX; i++ )
  {
    CTVT_BALLOON_Init( &gameWork->balloon_wk[i] );
  }

  // 値初期化
  for( i=0; i<CTVT_GAME_MEMBER_NUM; i++ )
  {
    gameWork->score[i] = 0;
  }
  gameWork->balloon_rnd_w = BALLOON_RND_W_MAX + 1;  // +1は最初の乱数がどれでも通るように
  gameWork->balloon_rnd_v = BALLOON_RND_V_MAX + 1;  // +1は最初の乱数がどれでも通るように
  gameWork->balloon_z_cnt = 0;
  gameWork->balloon_flg   = 0;
  gameWork->balloon_wait  = 0;
  gameWork->balloon_tbl_idx = 0;
  gameWork->balloon_tbl_cnt = 0;
  gameWork->timeLimit     = CTVT_GAME_TIME_LIMIT;
  gameWork->befFrameCnt   = 0;
  gameWork->rnd_seed      = 0;
  gameWork->frame         = 0;
  gameWork->oya_frame     = 0;
  gameWork->isRecvFrame   = FALSE;

  // ボーナス風船をいつ出すかきめる
  for( i=0; i<CTVT_GAME_MEMBER_NUM; i++ )
  {
    ICHI_Printf("player %d ");

    gameWork->bonus_tbl_idx[i] = 0;
    gameWork->bonus_tbl[i][0] = ( GFL_STD_MtRand(10) + 40 ); // 50～40
    gameWork->bonus_tbl[i][1] = ( GFL_STD_MtRand(10) + 30 ); // 40～30
    gameWork->bonus_tbl[i][2] = ( GFL_STD_MtRand(5) + 15 );  // 20～10
    gameWork->bonus_tbl[i][3] = ( GFL_STD_MtRand(5) + 10 );  // 10～5
    gameWork->bonus_tbl[i][4] = ( GFL_STD_MtRand(5) + 5 );   // 5～0

    ICHI_Printf("%d ", gameWork->bonus_tbl[i][0]);
    ICHI_Printf("%d ", gameWork->bonus_tbl[i][1]);
    ICHI_Printf("%d ", gameWork->bonus_tbl[i][2]);
    ICHI_Printf("%d ", gameWork->bonus_tbl[i][3]);
    ICHI_Printf("%d \n", gameWork->bonus_tbl[i][4]);
  }

#ifdef PM_DEBUG
  gameWork->debug_r    = 0x45;    // 半径計算用 サイズに掛ける割合 0x45は目安
  gameWork->debug_size = 0x200;   // サイズに掛ける値
  gameWork->debug_v    = 0x2000;  // ベクトルに掛ける値 
  gameWork->debug_wait = 20;      // 風船出現間隔(フレーム)
  GFL_STD_MemFill( gameWork->debug_b_cnt, 0, sizeof(u16)*4 );
#endif

  // BGM
  PMSND_PlayBGM( SEQ_BGM_LCG_01 );

  ICHI_Printf("ゲーム初期化完了\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:タイトル
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 *	@return TRUE で終了
 */
//-----------------------------------------------------------------------------
static BOOL game_update_seq_title( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  int ret = GFL_UI_TP_HitTrg( BALLOON_HitTitleButton );

  if( ret != GFL_UI_TP_HIT_NONE )
  {
    // タイトル非表示
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitle , FALSE );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitleItem[0] , FALSE );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitleItem[1] , FALSE );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTitleItem[2] , FALSE );

    // 制限時間表示
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTime[0], TRUE );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTime[1], TRUE );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkText, TRUE );
    // SCORE表示
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkScore[0], TRUE );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkScore[1], TRUE );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkScore[2], TRUE );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkScore[3], TRUE );
    GFL_CLACT_WK_SetDrawEnable( gameWork->clwkScoreText, TRUE );
    return TRUE;
  }

#if 1
  if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_A)
  {
    //disp_message_main( work, gameWork, COMM_TVT_SYS_05 );
  }
  if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_B)
  {
    disp_message_sub( work, gameWork, COMM_TVT_SYS_05 );
  }
#endif

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:メイン
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 *	@return TRUE で終了
 */
//-----------------------------------------------------------------------------
static BOOL game_update_seq_main( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

#ifdef PM_DEBUG
  // Rボタン押しながらでデバッグ
  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_R)
  {
    // 当たり判定
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_UP)
    {
      gameWork->debug_r += 0x5;    // 半径特定用
      OS_Printf("debug_r %x \n", gameWork->debug_r );
    }
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_DOWN)
    {
      gameWork->debug_r -= 0x5;    // 半径特定用
      OS_Printf("debug_r %x \n", gameWork->debug_r );
    }

    // サイズ
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_RIGHT)
    {
      gameWork->debug_size += 0x50;
      OS_Printf("debug_size %x \n", gameWork->debug_size);
    }
    if(GFL_UI_KEY_GetTrg()&PAD_KEY_LEFT)
    {
      gameWork->debug_size -= 0x50;
      OS_Printf("debug_size %x \n", gameWork->debug_size);
    }

    // 速さ
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_A)
    {
      gameWork->debug_v += 0x400;
      OS_Printf("debug_v %x \n", gameWork->debug_v);
    }
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_B)
    {
      gameWork->debug_v -= 0x400;
      OS_Printf("debug_v %x \n", gameWork->debug_v);
    }

    // 風船と風船の間隔
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_X)
    {
      gameWork->debug_wait += 5;
      OS_Printf("debug_wait %d \n", gameWork->debug_wait);
    }
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_Y)
    {
      gameWork->debug_wait -= 5;
      OS_Printf("debug_wait %d \n", gameWork->debug_wait);
    }
  }

  // 制限時間早める処理
  // デバッグ 制限時間早める
  if( selfId == 0 && GFL_UI_KEY_GetCont()&PAD_BUTTON_L)
  {
    gameWork->oya_frame += 5;
  }
  // ↑までデバッグ
#endif


    // 風船出現管理(親だけ)
  if( selfId == 0 )
  {
    // 待ち時間が終わると出現フラグON
    if( gameWork->balloon_wait <= 0 )
    {
      gameWork->balloon_flg = 1;
    }

    // 出現フラグがON
    if( gameWork->balloon_flg == 1 )
    {
      // 風船出現させる
      {
        // 通信データ送信
        CTVT_GAME_SEND_DATA data = {0,};
        CTVT_COMM_SendGameReq( commWork, data, CCGDT_BALOON_APPEAR );
      }
      
      gameWork->balloon_flg = 0;
#ifdef PM_DEBUG
      gameWork->balloon_wait = gameWork->debug_wait;
#else
      gameWork->balloon_wait = 20;
#endif
    }
    else
    {
      gameWork->balloon_wait--;
    }
  }

  // タッチペンの座標を送信する
  {
    u32 tpx,tpy;
    const BOOL isTrg = GFL_UI_TP_GetPointTrg( &tpx,&tpy );

    if( isTrg == TRUE )
    {
      // 通信データ送信
      CTVT_GAME_SEND_DATA data;
      data.tpx = tpx;
      data.tpy = tpy;
      CTVT_COMM_SendGameReq( commWork, data, CCGDT_TP );
      //ICHI_Printf("touch!\n");
    }
  }

  // 受信していない場合
  // TRUEで受信していると、frameの値は更新されている
  if( gameWork->isRecvFrame == FALSE )
  {
    gameWork->frame++;
  }
  gameWork->isRecvFrame = FALSE;    // 受信していないに戻す

  if( selfId == 0 )
  {
    gameWork->oya_frame++;
  }
  //ICHI_Printf("フレーム %d\n", gameWork->frame);

  // 制限時間処理
  if( gameWork->timeLimit > 0 )
  {
    if( ( gameWork->frame - gameWork->befFrameCnt ) > 60 )
    {
      gameWork->befFrameCnt = gameWork->frame;
      gameWork->timeLimit--;

      // OBJ変更
      {
        // 十秒
        GFL_CLACT_WK_SetAnmSeq( gameWork->clwkTime[0], CPOA_NUM_0 + (gameWork->timeLimit/10));
        // 秒
        GFL_CLACT_WK_SetAnmSeq( gameWork->clwkTime[1], CPOA_NUM_0 + (gameWork->timeLimit%10));
      }
    }

    if( gameWork->timeLimit == 0 )
    {
      // 終了のホイッスル音
      PMSND_PlaySystemSE( SEQ_SE_LCG_04 );    // スタートのホイッスル音
    }
  }
  else if( gameWork->timeLimit == 0 )
  {
    // 終了だけど風船の数を一定にするまで出現させる
    if( gameWork->balloon_tbl_cnt == 0 )
    {
      // 終了時に親ならスコア送信
      if( selfId == 0 )
      {
        CTVT_GAME_SEND_SCORE score;
        u8  i;

        ICHI_Printf("-----送信スコア-------------\n");
        for(i=0; i<CTVT_GAME_MEMBER_NUM; i++ )
        {
          score.score[i] = gameWork->score[i];
          ICHI_Printf("%d -> %d\n",i, score.score[i]);
        }

        CTVT_COMM_SendGameReq_Score( commWork, score );
      }
      gameWork->oya_frame = 0;    // 0以上だと通信メインで自動送信してしまう

      return TRUE; // 終了
    }
  }

  if( gameWork->timeLimit == 20 )
  {
    // 元々のテンポをratio/256倍する
    PMSND_SetStatusBGM( 285, 85, PMSND_NOEFFECT );
    //PMSND_SetStatusBGM( 285, PMSND_NOEFFECT, PMSND_NOEFFECT );
  }
  
  // 風船の更新 & 描画
  GRAPHIC_Draw( gameWork );

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:スコア
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_score( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8  i;
  const u8 selfId = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  const u8 connectNum = COMM_TVT_GetConnectNum( work );
  u8  rank = 1; // 順位

  ICHI_Printf("--SCORE------------------\n");
  for( i=0; i<connectNum; i++ )
  {
    ICHI_Printf("NetID %d SCORE %d \n", i, gameWork->score[i]);

    // 得点が自分より高い人がいると順位をさげる
    if( gameWork->score[i] > gameWork->score[selfId] )
    {
      rank++;
    }
  }

  // 再度得点更新
  {
    int score = gameWork->score[ selfId ];

    // + or -
    if( score < 0 )
    {
      GFL_CLACT_WK_SetAnmSeq( gameWork->clwkScore[0], CPOA_NUM_M );
    }
    else
    {
      GFL_CLACT_WK_SetAnmSeq( gameWork->clwkScore[0], CPOA_NUM_P );
    }

    // 得点表示
    score = GFL_STD_Abs(score); // 絶対値
    // 百の位
    GFL_CLACT_WK_SetAnmSeq( gameWork->clwkScore[1], CPOA_NUM_0 + (score/100));
    // 十の位
    GFL_CLACT_WK_SetAnmSeq( gameWork->clwkScore[2], CPOA_NUM_0 + (score/10));
    // 一の位
    GFL_CLACT_WK_SetAnmSeq( gameWork->clwkScore[3], CPOA_NUM_0 + (score%10));
  }

#ifdef PM_DEBUG
  ICHI_Printf("風船個数 プレイヤー0 %d \n", gameWork->debug_b_cnt[ 0 ]);
  ICHI_Printf("風船個数 プレイヤー1 %d \n", gameWork->debug_b_cnt[ 1 ]);
  ICHI_Printf("風船個数 プレイヤー2 %d \n", gameWork->debug_b_cnt[ 2 ]);
  ICHI_Printf("風船個数 プレイヤー3 %d \n", gameWork->debug_b_cnt[ 3 ]);
#endif

  // ランク表示
  GFL_CLACT_WK_SetDrawEnable( gameWork->clwkTime[0], FALSE );
  GFL_CLACT_WK_SetAnmSeq( gameWork->clwkTime[1], CPOA_NUM_0 + rank );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲーム状態 シーケンス:終了
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	*gameWork 
 */
//-----------------------------------------------------------------------------
static void game_update_seq_exit( COMM_TVT_WORK *work, CTVT_GAME_WORK *gameWork )
{
  u8  i;

  // 風船破棄
  for( i=0; i<BALLOON_MAX; i++ )
  {
    CTVT_BALLOON_Destroy( &gameWork->balloon_wk[i] );
  }

  // タスク
  GFL_TCB_Exit( gameWork->task );
  if( gameWork->task != NULL )
  {
    GFL_HEAP_FreeMemory( gameWork->task );
    gameWork->task = NULL;
  }
  if( gameWork->taskMem != NULL )
  {
    GFL_HEAP_FreeMemory( gameWork->taskMem );
    gameWork->taskMem = NULL;
  }

  // OBJ破棄
  GFL_CLACT_WK_Remove( gameWork->clwkTitle );
  GFL_CLACT_WK_Remove( gameWork->clwkTitleItem[0] );
  GFL_CLACT_WK_Remove( gameWork->clwkTitleItem[1] );
  GFL_CLACT_WK_Remove( gameWork->clwkTitleItem[2] );
  GFL_CLACT_WK_Remove( gameWork->clwkTime[0] );
  GFL_CLACT_WK_Remove( gameWork->clwkTime[1] );
  GFL_CLACT_WK_Remove( gameWork->clwkText );
  GFL_CLACT_WK_Remove( gameWork->clwkScoreText );
  GFL_CLACT_WK_Remove( gameWork->clwkScore[0] );
  GFL_CLACT_WK_Remove( gameWork->clwkScore[1] );
  GFL_CLACT_WK_Remove( gameWork->clwkScore[2] );
  GFL_CLACT_WK_Remove( gameWork->clwkScore[3] );

  // テクスチャ用メモリ破棄
  for( i=0;i<CTVT_GAME_MEMBER_NUM;i++ )
  {
    GFL_NET_Align32Free( gameWork->direct_tex[i] );
  }
}


//--------------------------------------------------------------
//	強制終了チェック
//--------------------------------------------------------------
static const BOOL CTVT_GAME_CheckFinishReq( COMM_TVT_WORK *work , CTVT_GAME_WORK *gameWork )
{
  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );

  // 子で、親がいなくなった
  if( COMM_TVT_GetSelfIdx( work ) != 0 && CTVT_COMM_IsEnableMember( work, commWork, 0 ) == FALSE )
  {
    gameWork->state = CGS_END_PARENT_NONE_INIT;
    OS_Printf("強制終了：親がいなくなった\n");
    return TRUE;
  }

  if( COMM_TVT_GetConnectNum( work ) <= 1 )
  {
    //誰も居なくなった。
    gameWork->state = CGS_END_MEMBER_NONE_INIT;
    OS_Printf("強制終了：誰もいなくなった\n");
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  キャプチャ完了時に呼ばれるコールバック関数
 *
 *	@param	void *captureArea   キャプチャデータ
 *	@param	*userWork 
 */
//-----------------------------------------------------------------------------
static void CTVT_GAME_CapCallBack( void *captureArea , void *userWork )
{
  CTVT_GAME_WORK* p_wk = userWork;

  // LCDのDMA転送との競合解消
  if( p_wk->isUpdateCapture == TRUE )
  {
    return;
  }

  // キャプチャデータコピー
  GFL_STD_MemCopy32( (void*)captureArea, (void*)p_wk->scrBuf, CTVT_BUFFER_SCR_SIZE );

  p_wk->isUpdateCapture = TRUE;
}

//=============================================================================
/**
 *      GRAPHIC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  描画関係初期化
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID )
{
  static const GFL_DISP_VRAM sc_vramSetTable =
  {
    GX_VRAM_BG_128_A,           // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_64_E,           // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_01_BD,          // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_0_G,      // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_128K, // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_128K, // サブOBJマッピングモード
  };


#if 1
  //GX_SetVisibleWnd( GX_WNDMASK_NONE );
  // sysの開放処理をコピー
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
#endif
  //GFL_BMPWIN_Exit();
  //GFL_BG_Exit();

  //GFL_BMPWIN_Init( heapID );

  // ディスプレイ
  GFL_DISP_SetDispOn();
  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();
  G2_BlendNone();
  G2S_BlendNone();

  //表示
  GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );

  //VRAMクリアー
  GFL_DISP_ClearVRAM( 0 );

  // VRAMバンク設定
  GFL_DISP_SetBank( &sc_vramSetTable );

  //描画モジュール
  GRAPHIC_BG_Init( p_wk, heapID );
  GRAPHIC_3D_Init( p_wk, heapID );

  //OBJ系の初期化
  GFL_CLACT_SYS_Delete();
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    GFL_CLACT_SYS_Create( &cellSysInitData , &sc_vramSetTable ,heapID );

    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );

    p_wk->cellUnit = GFL_CLACT_UNIT_Create( 16, 0, heapID );
    GFL_CLACT_UNIT_SetDefaultRend( p_wk->cellUnit );
  }
  // ライブｷｬｽﾀｰのセルアクターを作り直さないといけない

  // VBlank
  p_wk->vBlankTcb = GFUser_VIntr_CreateTCB( VBlankFunc , p_wk , 1 );

  p_wk->is_init = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  描画関係破棄
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Exit( CTVT_GAME_WORK* p_wk )
{
  GF_ASSERT( p_wk->is_init );

  GFL_TCB_DeleteTask( p_wk->vBlankTcb );

  GFL_CLACT_UNIT_Delete( p_wk->cellUnit );
  GFL_CLACT_SYS_Delete();

  GRAPHIC_3D_Exit( p_wk );
  GRAPHIC_BG_Exit( p_wk );

  p_wk->is_init = FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  描画関係描画処理
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Draw( CTVT_GAME_WORK* p_wk )
{
  u8 i;

  if( p_wk->is_init )
  {
    // 3D描画
    GFL_G3D_DRAW_Start();


    // カメラ更新
    GFL_G3D_CAMERA_Switching( p_wk->p_camera );
    GFL_G3D_DRAW_SetLookAt();

    //ライト設定
    NNS_G3dGlbLightVector( 0, 0, 0, FX32_ONE );
    NNS_G3dGlbLightColor( 0, GX_RGB(31,31,31) );
    NNS_G3dGlbMaterialColorDiffAmb(
        GX_RGB(31,31,31), GX_RGB(31,31,31), FALSE );
    NNS_G3dGlbMaterialColorSpecEmi(
        GX_RGB(31,31,31), GX_RGB(31,31,31), FALSE );

    // アニメーション更新
    for( i=0; i<CTVT_GAME_MEMBER_NUM; i++ )
    {
      int j;
      u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->g3dUtil, p_wk->unit_idx[i] );
      for( j= 0; j<3; j++ )
      {
        u16 obj_idx = head_obj_idx + j;
        GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( p_wk->g3dUtil, obj_idx );
        GFL_G3D_OBJECT_LoopAnimeFrame( obj, 0, FX32_ONE );
      }
    }


#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc(1);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    // 風船タスクメイン
    GFL_TCB_Main( p_wk->task );
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc(1);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

    // 風船同士の当たり判定
    balloon_collision_check( p_wk );

    // クリアーカラー(水色)
    G3X_SetClearColor( GX_RGB(18, 29, 31 ), 0, 0x7fff, 63, FALSE);

    GFL_G3D_DRAW_End();
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
  CTVT_GAME_WORK* p_wk = p_work;

  // isUpdateCapture は カメラDMAとLCDDMAの競合解決の為のフラグ
  // isCamCapture は 単にカメラ撮影モードかのフラグ
  if( p_wk->isUpdateCapture == TRUE && p_wk->isCamCapture == TRUE )
  {
    // カメラキャプチャ転送
    DC_FlushRange( p_wk->scrBuf , CTVT_BUFFER_SCR_SIZE );
    GX_LoadBG3Bmp( p_wk->scrBuf , 0, CTVT_BUFFER_SCR_SIZE );
    p_wk->isUpdateCapture = FALSE;
  }

  // BG転送  
  GFL_BG_VBlankFunc();
  // OBJの更新
  GFL_CLACT_SYS_VBlankFunc();

  //machine_useでやってた
  //  NNS_GfdDoVramTransfer();
}

//=============================================================================
/**
 *      GRAPHIC_BG
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  BG描画  初期化
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID )
{
  //GFL_BG_Init( heapID );

  // BGモード設定
  { 
    GFL_BG_SYS_HEADER sysh = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_3, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGMode( &sysh );
  }

  // 個別フレーム設定
  {
    //Main
    // BG1 MSG (文字
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000,GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_23, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 BG (背景
    static const GFL_BG_BGCNT_HEADER header_main2 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x08000,GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // Sub
    // BG0 MSG (文字
    static const GFL_BG_BGCNT_HEADER header_sub0 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000,GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_23, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x08000,GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    //GRAPHIC_BG_SetupBgFunc( &header_main1, CTVT_GAME_FRAME_M_MSG, GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_main1, CTVT_GAME_FRAME_M_BG, GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_sub0, CTVT_GAME_FRAME_S_MSG, GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_sub3, CTVT_GAME_FRAME_S_BG, GFL_BG_MODE_TEXT );

    // カメラキャプチャのBG
    // gflibが対応していないので直書き
    //GX_SetBGCharOffset(GX_BGCHAROFFSET_0x00000);
    //GX_SetBGScrOffset ( GX_BGSCROFFSET_0x00000);

    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
    GFL_STD_MemClear( G2_GetBG3ScrPtr(), CTVT_BUFFER_SCR_SIZE );

  }

  // ブレンド
  G2_SetBlendAlpha(
      GX_BLEND_PLANEMASK_BG0,
      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
      0, 16 );
  // 3Dのアルファをきれいに出すため、第1対象面にBG0を指定している
  // ev1とev2は使われない  // TWLプログラミングマニュアル「2D面とのαブレンディング」参考


  // BGプライオリティを設定
  GFL_BG_SetPriority( CTVT_GAME_FRAME_M_3D, 2 ); 
  //GFL_BG_SetPriority( CTVT_GAME_FRAME_M_MSG, 0 ); 
  GFL_BG_SetPriority( CTVT_GAME_FRAME_M_BG, 3 ); 
  GFL_BG_SetPriority( CTVT_GAME_FRAME_M_CAMERA, 3 ); 

  GFL_BG_SetVisible( CTVT_GAME_FRAME_M_3D,    VISIBLE_OFF );
  //GFL_BG_SetVisible( CTVT_GAME_FRAME_M_MSG,   VISIBLE_OFF );
  GFL_BG_SetVisible( CTVT_GAME_FRAME_M_BG,    VISIBLE_OFF );
  GFL_BG_SetVisible( CTVT_GAME_FRAME_M_CAMERA,VISIBLE_OFF );
  GFL_BG_SetVisible( CTVT_GAME_FRAME_S_MSG,   VISIBLE_OFF );
  GFL_BG_SetVisible( CTVT_GAME_FRAME_S_BG,    VISIBLE_OFF );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG描画  破棄
 *
 *	@param	CTVT_GAME_WORK* p_wk 
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Exit( CTVT_GAME_WORK* p_wk )
{
  // ミニゲームの設定から会話の設定に戻す？
  // ↓COMM_TVT_InitGraphicの中から必要なものをコピーしてきた

#if 0
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
    GX_OBJVRAMMODE_CHAR_1D_128K,
    GX_OBJVRAMMODE_CHAR_1D_128K
  };


  // 表示
  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
  // VRAMクリアー
  GFL_DISP_ClearVRAM( 0 );
  // VRAMバンク設定
  GFL_DISP_SetBank( &vramBank );

  //Vram割り当ての設定
  {
    static const GFL_BG_SYS_HEADER sys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_5, GX_BGMODE_0, GX_BG0_AS_2D,
    };

    // BG0 MSG (文字
    static const GFL_BG_BGCNT_HEADER header_sub0 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x5000, GX_BG_CHARBASE_0x18000,0x08000,
      GX_BG_EXTPLTT_23, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG1 BAR (バー
    static const GFL_BG_BGCNT_HEADER header_sub1 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000,0x05000,
      GX_BG_EXTPLTT_23, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 SUB (その他(画面別BG
    static const GFL_BG_BGCNT_HEADER header_sub2 = {
      0, 0, 0x1000, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x10000,0x08000,
      GX_BG_EXTPLTT_23, 2, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 SUB (背景
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x08000,0x08000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    //Main
    // BG0 MSG (文字
    static const GFL_BG_BGCNT_HEADER header_main0 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000,0x07000,
      GX_BG_EXTPLTT_23, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG1 BG (BGノイズ
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x0800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x08000,0x08000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };

    GFL_BG_SetBGMode( &sys_data );

    GRAPHIC_BG_SetupBgFunc( &header_main0 , CTVT_FRAME_MAIN_MSG  , GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_main1 , CTVT_FRAME_MAIN_BG  , GFL_BG_MODE_TEXT );

    GRAPHIC_BG_SetupBgFunc( &header_sub0 , CTVT_FRAME_SUB_MSG  , GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_sub1 , CTVT_FRAME_SUB_BAR  , GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_sub2 , CTVT_FRAME_SUB_MISC , GFL_BG_MODE_TEXT );
    GRAPHIC_BG_SetupBgFunc( &header_sub3 , CTVT_FRAME_SUB_BG   , GFL_BG_MODE_TEXT );

    //ダイレクトBMPにGFLが対応してないのでべた書き

    G2_SetBG2ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x10000);
    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x28000);

    GFL_STD_MemClear( G2_GetBG2ScrPtr() , 256*192*sizeof(u16) );
    GFL_STD_MemClear( G2_GetBG3ScrPtr() , 256*192*sizeof(u16) );

    G2_SetBG2Priority(1);
    G2_SetBG3Priority(2);
    GFL_BG_SetVisible( CTVT_FRAME_MAIN_DRAW, VISIBLE_ON );
    GFL_BG_SetVisible( CTVT_FRAME_MAIN_CAMERA, VISIBLE_ON );

  }
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG設定
 *	        comm_tvt_sys.cのGRAPHIC_BG_SetupBgFuncをそのままコピーしてきた
 *
 *	@param	const GFL_BG_BGCNT_HEADER *bgCont 
 *	@param	bgPlane 
 *	@param	mode 
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

//=============================================================================
/**
 *      GRAPHIC_3D
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D環境  初期化
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID )
{
  // 3Dシステム初期化
  GFL_G3D_Init(
      GFL_G3D_VMANLNK, GFL_G3D_TEX256K,
      GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0, heapID, NULL );

  GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );

  G3X_SetClearColor(GX_RGB(0x1f,0x1f,0x1f),0,0x7fff,63,FALSE); //color,alpha,depth,polygonID,fog
  G3X_AlphaBlend( TRUE );
  G3X_EdgeMarking( FALSE );
  G3X_AntiAlias( TRUE );
  G3X_SetFog( FALSE, 0, 0, 0 );

  //カメラ設定
#if 0
  // 正射影
  {
    static const VecFx32 cam_pos = { FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(400.0f) };
    static const VecFx32 cam_target = { FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(0.0f) };
    static const VecFx32 defaultCameraUp = { 0, FX32_ONE, 0 };

    p_wk->p_camera = GFL_G3D_CAMERA_CreateOrtho(
        FX32_CONST(96),
        -FX32_CONST(96),
        -FX32_CONST(128),
        FX32_CONST(128),
        defaultCameraNear, defaultCameraFar, 0,
        &cam_pos, &defaultCameraUp, &cam_target,
        heapID );

    GFL_G3D_CAMERA_Switching( p_wk->p_camera );
  }
#endif
  // 透視射影
  { 
    static const VecFx32 cam_pos    = { 0, 0 , 250*FX32_ONE};
    static const VecFx32 cam_target = { 0,0,0};
    p_wk->p_camera = GFL_G3D_CAMERA_CreateDefault( &cam_pos, &cam_target, heapID );

    GFL_G3D_CAMERA_Switching( p_wk->p_camera );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  3D環境  破棄
 *
 *	@param	CTVT_GAME_WORK* p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_Exit( CTVT_GAME_WORK* p_wk )
{
  GFL_G3D_CAMERA_Delete( p_wk->p_camera );
  GFL_G3D_Exit();
}

//=============================================================================
/**
 *      RESOURCE
 */
//=============================================================================
// RES
// プレイヤー1
static const GFL_G3D_UTIL_RES res_3d_table1[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon1_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
};
// プレイヤー2
static const GFL_G3D_UTIL_RES res_3d_table2[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon2_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
};
// プレイヤー3
static const GFL_G3D_UTIL_RES res_3d_table3[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon3_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
};
// プレイヤー4
static const GFL_G3D_UTIL_RES res_3d_table4[] =
{
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbmd, GFL_G3D_UTIL_RESARC },  // 通常
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbmd, GFL_G3D_UTIL_RESARC },  // 驚き
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbmd, GFL_G3D_UTIL_RESARC },  // ボーナス
  { ARCID_TVT_BALLOON, NARC_tvt_balloon_balloon4_nsbca, GFL_G3D_UTIL_RESARC },  // アニメ
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_3d_table[] =
{
  { 3, 0 },
};

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] = 
{
  {
    0,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    0,      // テクスチャリソースID
    anm_3d_table,   // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table),      // アニメリソース数
  },
  {
    1,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    1,      // テクスチャリソースID
    anm_3d_table,   // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table),      // アニメリソース数
  },
  {
    2,      // モデルリソースID
    0,      // モデルデータID(リソース内部INDEX)
    2,      // テクスチャリソースID
    anm_3d_table,   // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table),      // アニメリソース数
  },
};

#define CTVT_UNIT_RES_MAX (16)    // 3種 * 4人  + アニメ*4

// セットアップデータ
// 覚書
// ユニットのOBJとリソースの数は違うので、
// ただ単にunit_idx を使用しても、そこからOBJを取得するか、
// リソースを取得するかで、加算の値が違う
// ここで言うと、OBJは３つだが、リソースは4つなので、
// ユニットの0番目は0
// ユニットの1番目は4 となる ←モデルが3つなので、＋３と間違えやすい
static const GFL_G3D_UTIL_SETUP G3DUtilSetup[] =
{
  { res_3d_table1, NELEMS(res_3d_table1), obj_3d_table, NELEMS(obj_3d_table) },
  { res_3d_table2, NELEMS(res_3d_table2), obj_3d_table, NELEMS(obj_3d_table) },
  { res_3d_table3, NELEMS(res_3d_table3), obj_3d_table, NELEMS(obj_3d_table) },
  { res_3d_table4, NELEMS(res_3d_table4), obj_3d_table, NELEMS(obj_3d_table) },
};

//----------------------------------------------------------------------------
/**
 *	@brief  リソース読み込み
 *
 *	@param	CTVT_GAME_WORK* p_wk 
 */
//-----------------------------------------------------------------------------
static void RESOURCE_Init( CTVT_GAME_WORK* p_wk, HEAPID heapID, ARCHANDLE* arcHandle )
{
  u8  i;

  // OBJ
  // カメラ撮影用
  {
    ARCHANDLE* arch = GFL_ARC_OpenDataHandle( ARCID_COMM_TVT_GRA, heapID );
    p_wk->cellResIdx[CGOR_CAMERA_PLT] = GFL_CLGRP_PLTT_RegisterComp( arch , 
        NARC_comm_tvt_tv_camera_obj_NCLR , CLSYS_DRAW_MAIN , 
        0 , heapID  );
    p_wk->cellResIdx[CGOR_CAMERA_NCG] = GFL_CLGRP_CGR_Register( arch , 
        NARC_comm_tvt_tv_camera_obj_NCGR , FALSE , CLSYS_DRAW_MAIN , heapID  );
    p_wk->cellResIdx[CGOR_CAMERA_ANM] = GFL_CLGRP_CELLANIM_Register( arch , 
        NARC_comm_tvt_tv_camera_obj_NCER , NARC_comm_tvt_tv_camera_obj_NANR, heapID  );
    GFL_ARC_CloseDataHandle( arch );
  }


  // OBJ
  // 共通
  p_wk->cellResIdx[CGOR_COMMON_PLT] = GFL_CLGRP_PLTT_RegisterComp( arcHandle , 
      NARC_tvt_balloon_game_common_obj_NCLR, CLSYS_DRAW_MAX, 
      CTVT_GAME_PAL_OBJ_COMMON*32, heapID  );
  p_wk->cellResIdx[CGOR_COMMON_NCG] = GFL_CLGRP_CGR_Register( arcHandle, 
      NARC_tvt_balloon_game_common_obj_NCGR, FALSE , CLSYS_DRAW_MAX, heapID  );
  p_wk->cellResIdx[CGOR_COMMON_ANM] = GFL_CLGRP_CELLANIM_Register( arcHandle, 
      NARC_tvt_balloon_game_common_obj_NCER, NARC_tvt_balloon_game_common_obj_NANR, heapID  );

  // プレイ中
  p_wk->cellResIdx[CGOR_PLAY_PLT] = GFL_CLGRP_PLTT_RegisterComp( arcHandle , 
      NARC_tvt_balloon_game_play_obj_NCLR, CLSYS_DRAW_MAX, 
      CTVT_GAME_PAL_OBJ_PLAY*32, heapID  );
  p_wk->cellResIdx[CGOR_PLAY_NCG] = GFL_CLGRP_CGR_Register( arcHandle, 
      NARC_tvt_balloon_game_play_obj_NCGR, FALSE , CLSYS_DRAW_MAX, heapID  );
  p_wk->cellResIdx[CGOR_PLAY_ANM] = GFL_CLGRP_CELLANIM_Register( arcHandle, 
      NARC_tvt_balloon_game_play_obj_NCER, NARC_tvt_balloon_game_play_obj_NANR, heapID  );

  // 3Dリソース
  {
    // 3D管理ユーティリティーのセットアップ
    p_wk->g3dUtil = GFL_G3D_UTIL_Create( CTVT_UNIT_RES_MAX, BALLOON_MAX, heapID );

    // ユニット追加
    for( i=0; i<CTVT_GAME_MEMBER_NUM; i++ )
    {
      p_wk->unit_idx[i] = GFL_G3D_UTIL_AddUnit( p_wk->g3dUtil, &G3DUtilSetup[i] );
      ICHI_Printf("ユニット追加 プレイヤー%d = %d\n", i, p_wk->unit_idx[i]);
    }

    // アニメーション有効化
    {
      for( i=0; i<CTVT_GAME_MEMBER_NUM; i++ )
      {
        int j;
        u16 unit_res_cnt = GFL_G3D_UTIL_GetUnitResCount( p_wk->g3dUtil, p_wk->unit_idx[i] );
        u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->g3dUtil, p_wk->unit_idx[i] );
        for( j= 0; j<3; j++ )
        {
          u16 res_idx = unit_res_cnt*p_wk->unit_idx[i] + j;
          u16 obj_idx = head_obj_idx + j;
          GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( p_wk->g3dUtil, obj_idx );
          GFL_G3D_RES* res = GFL_G3D_UTIL_GetResHandle( p_wk->g3dUtil, res_idx );
          GFL_G3D_OBJECT_EnableAnime( obj, 0 );
          ICHI_Printf("obj_idx %d res_idx %d res_addrs %x obj_addrs %x\n", obj_idx, res_idx, res, obj);
          if( GFL_G3D_CheckResourceType( res, GFL_G3D_RES_CHKTYPE_TEX ) )
          {
            ICHI_Printf("テクスチャデータが存在するリソース\n");
          }
        }
      }
    }


    ICHI_Printf("総オブジェクト数 %d\n", GFL_G3D_UTIL_GetObjCount(p_wk->g3dUtil));
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  リソース破棄
 *
 *	@param	CTVT_GAME_WORK* p_wk 
 */
//-----------------------------------------------------------------------------
static void RESOURCE_Exit( CTVT_GAME_WORK* p_wk )
{
  u8  i;


  // リソースの破棄
  for( i=CGOR_PLT_TOP; i<CGOR_NCG_TOP; i++ )
  {
    GFL_CLGRP_PLTT_Release( p_wk->cellResIdx[i] );
  }
  for( i=CGOR_NCG_TOP;i<CGOR_ANM_TOP;i++ )
  {
    GFL_CLGRP_CGR_Release( p_wk->cellResIdx[i] );
  }
  for( i=CGOR_ANM_TOP;i<CGOR_MAX;i++ )
  {
    GFL_CLGRP_CELLANIM_Release( p_wk->cellResIdx[i] );
  }

  // ユニット破棄
  for( i=0; i<CTVT_GAME_MEMBER_NUM; i++ )
  {
    GFL_G3D_UTIL_DelUnit( p_wk->g3dUtil, p_wk->unit_idx[i] );
  }

  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( p_wk->g3dUtil );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ表示 下画面
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	p_wk
 *	@param	u16 msgId 
 */
//-----------------------------------------------------------------------------
static void disp_message_main( COMM_TVT_WORK *work, CTVT_GAME_WORK* p_wk, const u16 msgId )
{
  GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
  GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
  PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
  STRBUF *str;

  PRINTSYS_QUE_Clear( printQue );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->msgWinMain), 0xF );
  str = GFL_MSG_CreateString( msgHandle, msgId );
  PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp( p_wk->msgWinMain ) , 
      0, 0, str, fontHandle,CTVT_FONT_COLOR_BLACK );
  GFL_STR_DeleteBuffer( str );
  GFL_BMPWIN_TransVramCharacter( p_wk->msgWinMain );
  GFL_BMPWIN_MakeScreen( p_wk->msgWinMain );

  BmpWinFrame_Write( p_wk->msgWinMain, WINDOW_TRANS_ON_V, 
      CTVT_GAME_BMPWIN_CGX_MAIN, CTVT_GAME_PAL_BG_M_WINFRAME );
  p_wk->isUpdateMsgMain = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ表示 上画面
 *
 *	@param	COMM_TVT_WORK *work
 *	@param	p_wk
 *	@param	u16 msgId 
 */
//-----------------------------------------------------------------------------
static void disp_message_sub( COMM_TVT_WORK *work, CTVT_GAME_WORK* p_wk, const u16 msgId )
{
  GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
  GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
  PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
  STRBUF *str;

  PRINTSYS_QUE_Clear( printQue );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->msgWinSub), 0xF );
  str = GFL_MSG_CreateString( msgHandle, msgId );
  PRINTSYS_PrintQueColor( printQue, GFL_BMPWIN_GetBmp( p_wk->msgWinSub ) , 
      0, 0, str, fontHandle, CTVT_FONT_COLOR_BLACK );
  GFL_STR_DeleteBuffer( str );
  GFL_BMPWIN_TransVramCharacter( p_wk->msgWinSub );
  GFL_BMPWIN_MakeScreen( p_wk->msgWinSub );

  BmpWinFrame_Write( p_wk->msgWinSub, WINDOW_TRANS_ON_V, 
      CTVT_GAME_BMPWIN_CGX, CTVT_GAME_PAL_BG_S_WINFRAME );
  p_wk->isUpdateMsgSub = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船出現の初期化
 *	        初期位置や方向など
 *
 *	@param	CTVT_GAME_WORK *p_game_wk
 *	@param	p_wk        設定する風船の構造体
 *	@param	unit_idx    ユニットID 　どのモデルか？
 */
//-----------------------------------------------------------------------------
static void balloon_set( CTVT_GAME_WORK *p_game_wk, BALLOON_WORK*  p_wk, u16  unit_idx )
{
  p_wk->p_game_wk = p_game_wk;
  // サイズ決定 
  //p_wk->size  = GFL_STD_MtRand(3) + 1;  // 1～3;
  p_wk->size  = p_game_wk->balloon_z_cnt + 1;

  p_wk->unit_idx    = unit_idx;   // モデル指定
  p_wk->visible     = TRUE;       // フラグON
  p_wk->color = unit_idx;    // unit_idx と 対になっている
  p_wk->alpha = 31;
  p_wk->alpha_cnt = 5;

  p_wk->save_frame = p_game_wk->frame;


  //ICHI_Printf("風船出現 %d unit %d ", p_wk->tcb, unit_idx);

  {
    // 風船の初期位置
    fx32  x;
    fx32  y;
    fx32  z;

    // X軸乱数( X軸をBALLOON_RND_W_MAXに分けて、その中での乱数 )
    {
      u16 rate = 256 / BALLOON_RND_W_MAX;
      u16 tmp;

      // 最初に0からrateの乱数をだして、調整用の乱数分足す
      tmp = GFL_STD_MtRand( rate ) + (rate * p_game_wk->balloon_rnd_w);
      x = FX32_CONST(-128) + FX32_ONE * tmp;
    }

    // Y軸乱数
    y = FX32_ONE * ( -240 );

    // Z軸乱数( -100,-200,-300 )
    //z = 0;
    //z = FX32_CONST(-60) * (4 - rnd);
    z = FX32_CONST(-80) * (4 - p_wk->size);

    VEC_Set( &p_wk->status.trans, x, y, z );
    //ICHI_Printf("pos%d:%d:%d ", FX_Whole(x), FX_Whole(y), FX_Whole(z));
  }

  {
    // 風船のサイズ(大・中・小)
#ifdef PM_DEBUG
    fx32 size = p_game_wk->debug_size * p_wk->size;
#else
    fx32 size = 0x200 * p_wk->size;
#endif

    size += 0x700;  // デフォルト値を足す
    VEC_Set( &p_wk->status.scale, size, size, size );
    //ICHI_Printf("size%x ", size);
  }
  // 風船の初期ベクトル
  {
    VecFx32 vec;
    MtxFx33 rot;
    u16     ang;
    u16     rate = 60 / BALLOON_RND_V_MAX;   // 60はてきとう
    fx32    vy =  ( GFL_STD_MtRand(2) + 1) * FX32_ONE;

    VEC_Set( &vec, 0, FX32_ONE, 0 );    // 上へのベクトル

    ang = GFL_STD_MtRand( rate ) + ( rate * p_game_wk->balloon_rnd_v ) + 330; // +330=360-30 unsigned型だから-できなかった
    ang %= 360;
    ang *= 182;  // 65536÷360 = 182.044...
    GFL_CALC3D_MTX_CreateRot( 0, 0, ang, &rot );

    MTX_MultVec33( &vec, &rot, &p_wk->vec );    // 角度ベクトル
    VEC_Normalize( &p_wk->vec, &p_wk->vec );

    // 残り20秒ではやくなる
    if( p_game_wk->timeLimit <= 20 )
    {
#ifdef PM_DEBUG
      vy += p_game_wk->debug_v;
#else
      vy += 0x2000;
#endif
    }

    // 速さをきめる
    GFL_CALC3D_VEC_MulScalar( &p_wk->vec, vy, &p_wk->vec );
    //OS_Printf("ang %d vec %f:%f \n",ang, (float)((float)p_wk->vec.x/FX32_ONE), (float)((float)p_wk->vec.y/FX32_ONE));
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  全ての風船が終了しているかチェック
 *
 *	@param	CTVT_GAME_WORK *p_wk 
 *
 *	@return TRUE  終了している
 */
//-----------------------------------------------------------------------------
static BOOL balloon_end_check( CTVT_GAME_WORK *p_wk )
{
  u8    i;
  for( i=0; i<BALLOON_MAX; i++ )
  {
    if( p_wk->balloon_wk[i].visible == TRUE )
    {
      // 1つでも表示しているものがあるとFALSE
      return FALSE;
    }
  }

  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  誰の風船を出すか計算して返す
 *
 *	@param	CTVT_GAME_WORK *p_wk 
 *
 *	@return u8  出すプレイヤー番号 0からの連番
 */
//-----------------------------------------------------------------------------
static u8 balloon_find_idx( CTVT_GAME_WORK *p_wk )
{
  u8  idx = p_wk->balloon_tbl_idx;
  u8  cnt = p_wk->balloon_tbl_cnt;
  u8  ret_idx;

  GF_ASSERT( idx<BALLOON_TBL_MAX );
  GF_ASSERT( cnt<CTVT_GAME_MEMBER_NUM );
  ret_idx = balloon_tbl[ idx ][ cnt ];

  // 次へ
  p_wk->balloon_tbl_cnt++;
  if( p_wk->balloon_tbl_cnt >= CTVT_GAME_MEMBER_NUM )
  {
    p_wk->balloon_tbl_cnt = 0;

    // 次のテーブルを求める
    p_wk->balloon_tbl_idx = GFL_STD_MtRand( BALLOON_TBL_MAX );
  }

#ifdef PM_DEBUG
  p_wk->debug_b_cnt[ ret_idx ]++;
#endif

  return ret_idx;
}

// タスクに設定するタスク関数(風船用)
static void TaskFunc( GFL_TCB* p_tcb, void* p_work )
{
  BALLOON_WORK  *p_wk = (BALLOON_WORK*)p_work;

  if( p_wk->visible == TRUE )
  {

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc(2);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    CTVT_BALLOON_Update( p_wk );
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc(2);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_StartFunc(3);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    CTVT_BALLOON_Draw( p_wk );
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
    DEBUG_MAIN_TIME_AVERAGE_EndFunc(3);
#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  タスクに登録されている風船とタッチの当たり判定
 *
 *	@param	CTVT_GAME_WORK* p_wk    ゲーム管理構造体
 *	@param	tpx       タッチX座標
 *	@param	tpy       タッチY座標
 *	@param	*ret_idx  割れた風船のIDXを格納（当たったときのみ）　
 *
 *	@retval TRUE    当たり有り
 *	@retval FALSE   当たり無し
 */
//-----------------------------------------------------------------------------
static BOOL balloon_hit_check( CTVT_GAME_WORK* p_wk, u32 tpx, u32 tpy, u16 *ret_idx )
{
  u8  i;

  GFL_COLLISION3D_CIRCLE    circle;
  VecFx32   center;   // 中心座標
  fx32      r;        // 半径

  VecFx32   rayPos;             // レイ開始座標
  VecFx32   rayWay;             // レイ方向
  BOOL      ret = FALSE;        // 返り値用
  u8        save_size = 0;      // 当たったやつのサイズ保存しておく

  // スクリーン座標からレイ取得
  GFL_CALC3D_VEC_Get2DPosTo3DRay( &rayPos, &rayWay, tpx, tpy );
  //OS_Printf("レイ座標%f:%f:%f レイ方向%f:%f:%f \n",(float)((float)rayPos.x/FX32_ONE),(float)((float)rayPos.y/FX32_ONE),(float)((float)rayPos.z/FX32_ONE),(float)((float)rayWay.x/FX32_ONE), (float)((float)rayWay.y/FX32_ONE), (float)((float)rayWay.z/FX32_ONE));


  for( i=0; i<BALLOON_MAX; i++ )
  {
    BOOL  b_hit = FALSE;

    if( p_wk->balloon_wk[i].visible == FALSE || p_wk->balloon_wk[i].type == BALLOON_HIT )
    {
      // 表示されていないならスキップ
      continue;
    }

    //OS_Printf("モデル座標 %.2f:%.2f:%.2f\n", (float)((float)p_wk->balloon_wk[i].status.trans.x/FX32_ONE), (float)((float)p_wk->balloon_wk[i].status.trans.y/FX32_ONE),(float)((float)p_wk->balloon_wk[i].status.trans.z/FX32_ONE) );


    // 球設定
    // 半径はサイズによって変わるので、割合を掛けている FX_MULは使用しない
    r = p_wk->balloon_wk[i].status.scale.x * 0x45;
#ifdef PM_DEBUG
    r = p_wk->balloon_wk[i].status.scale.x * p_wk->debug_r;
#endif
    VEC_Set( &center, p_wk->balloon_wk[i].status.trans.x, p_wk->balloon_wk[i].status.trans.y, p_wk->balloon_wk[i].status.trans.z );

    //OS_Printf("w %.2f PosScale %.2f 半径 %.2f size %x \n", (float)((float)modelInfo->boxW/FX32_ONE),(float)((float)modelInfo->posScale/FX32_ONE),(float)((float)r/FX32_ONE), p_wk->balloon_wk[i].status.scale.x );

    GFL_COLLISION3D_CIRCLE_SetData( &circle, &center, r );

    // 当たり判定
    b_hit = collision3d_sphere_checkRay( &rayPos, &rayWay, &circle );
    if( b_hit == TRUE )
    {
      //ICHI_Printf("当たった！\n");

      // Hitデータ保存
      ret = TRUE;

      // サイズ大だと手前にあるはずなので、それ以降の処理はしない
      if( p_wk->balloon_wk[i].size == BALLOON_SIZE_L )
      {
        *ret_idx = i;   // 何番目か保存
        return TRUE;
      }

      // サイズ比較 (手前にあるか奥にあるか)
      if( p_wk->balloon_wk[i].size > save_size )
      {
        *ret_idx = i;   // 何番目か保存
        save_size = p_wk->balloon_wk[i].size;   // サイズを保存しておく
      }
    }
  }

  return ret;
}


// タスクに登録されている風船同士の距離判定
//----------------------------------------------------------------------------
/**
 *	@brief  タスクに登録されている風船同士の距離判定
 *
 *	@param	CTVT_GAME_WORK *p_wk 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void balloon_collision_check( CTVT_GAME_WORK *p_wk )
{
  u8 i, j;

  // 19*19回
  for( i=0; i<BALLOON_MAX - 1; i++ )
  {

    // 表示していない風船はスキップ
    // 割れている風船もスキップ
    if( p_wk->balloon_wk[i].visible == FALSE || p_wk->balloon_wk[i].type == BALLOON_HIT )
    {
      continue;
    }

    for( j=i+1; j< BALLOON_MAX; j++ )
    {
      // 風船同士のサイズが同じ かつ 対象の風船も表示している
      if( p_wk->balloon_wk[i].size == p_wk->balloon_wk[j].size
          && p_wk->balloon_wk[j].visible == TRUE )
      {
        // 壁判定
        collision_vec( 
            &p_wk->balloon_wk[i].status.trans.x, &p_wk->balloon_wk[i].status.trans.y,
            &p_wk->balloon_wk[j].status.trans.x, &p_wk->balloon_wk[j].status.trans.y,
            p_wk->balloon_wk[i].status.scale.x * 140, 0 );
      }
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  2点間の距離判定を行い 衝突していると移動する
 *	        壁判定の処理
 *	        ※避けるのはX座標だけ
 *
 *	@param	fx32* x1  点1のX座標
 *	@param	fx32* y1  点1のY座標
 *	@param	fx32* x2  点2のX座標
 *	@param	fx32* y2  点2のY座標
 *	@param	fx32  dir 衝突の距離 
 *	@param  u8    avoid 避け方 0:両方避ける 1:点1が避ける 2:点2が避ける
 */
//-----------------------------------------------------------------------------
static void collision_vec( fx32* x1, fx32* y1, fx32* x2, fx32* y2, fx32 dir, u8 avoid )
{
  VecFx32 v1;   // ベクトル1
  VecFx32 v2;   // ベクトル2
  fx32    dist; // ベクトル間の距離

  VEC_Set( &v1, *x1, *y1, 0 );
  VEC_Set( &v2, *x2, *y2, 0 );

  // 距離取得
  dist = VEC_Distance( &v1, &v2 );

  // 衝突している
  if( dist < dir )
  {
    fx32  sub;
    fx32  *right, *left;

    sub = dir - dist;   // 避ける長さ
    //ICHI_Printf("直径 %x ", dir);
    //ICHI_Printf("避ける長さ %x \n", sub);

    // 右にある点と左にある点判別
    if( *x1 < *x2 )
    {
      left  = x1;
      right = x2;
    }
    else
    {
      left  = x2;
      right = x1;
    }

    // 避け方判別
    switch( avoid ){
    case 0:   // 両方避ける
      *right  += FX_Div(sub, 0x4800);
      *left   -= FX_Div(sub, 0x4800);
      break;
    case 1:   // 点1が避ける
      if( right == x1 )
      {
        *x1 += sub;
      }
      else if( left == x1 )
      {
        *x1 -= sub;
      }
      *y1 -= sub;   // 関数の意味とはことなるが風船の見た目のために入れた
      break;
    case 2:   // 点2が避ける
      if( right == x2 )
      {
        *x2 += sub;
      }
      else if( left == x2 )
      {
        *x2 -= sub;
      }
      break;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  レイと球の交差判定
 *
 *	@param	*cp_rayPos  レイ開始座標
 *	@param	*cp_rayWay  レイ方向(正規化されていること！)
 *	@param	*cp_circle  円構造体
 *
 *	@retval	TRUE		当たる	
 *	@retval	FALSE		当たらない
 */
//-----------------------------------------------------------------------------
static BOOL collision3d_sphere_checkRay( const VecFx32 *cp_rayPos, const VecFx32 *cp_rayWay, const GFL_COLLISION3D_CIRCLE *cp_circle )
{
  // メモ覚書
  // ゲームプログラミングの為のリアルタイム衝突 .P177
  // 光線の式  R(t) = P + td, t>=0
  //           P:光線の原点
  //           d:光線の方向 (正規化されている)
  //
  //  球の式   (X-C)･(X-C) = r*r
  //           C:球の中心
  //           r:半径
  //
  //  上記2つの式でR(t) = Xと置き換えると
  //   (P + td - C)･(P + td - C) = r*r
  //  となる
  //  

  VecFx32   m;      // レイ->円へのベクトル
  fx32      c;      // 
  fx32      b;
  fx32      disc;


  // レイ->円ベクトル算出
  VEC_Subtract( cp_rayPos, &cp_circle->center, &m );
  // 
  c = VEC_DotProduct( &m, &m ) - FX_MUL( cp_circle->r, cp_circle->r);
  //OS_Printf("c %.2f \n", (float)((float)c/FX32_ONE));

  // 少なくとも1つの実数解が存在している場合、交差している
  if( c <= 0 ) return TRUE;

  b = VEC_DotProduct( &m, cp_rayWay );
  //OS_Printf("b %.2f \n", (float)((float)b/FX32_ONE));
  // レイの原点が球の外側にあり、レイが球から離れた方向を指している場合には早期に終了
  if( b > 0 ) return FALSE;

  disc = FX_Mul( b, b ) - c;
  //OS_Printf("d %.2f \n", (float)((float)disc/FX32_ONE));

  // 負の判別式はレイが球を外れていることに一致
  if( disc < 0 ) return FALSE;

  return TRUE;
}

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON

static void DEBUG_MAIN_TIME_AVERAGE_StartFunc( u8 id )
{
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start[id] = OS_GetTick(); 
}

static void DEBUG_MAIN_TIME_AVERAGE_EndFunc( u8 id )
{
  OSTick now_time;
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_End[id] = OS_GetTick(); 

  now_time = OS_TicksToMicroSeconds(DEBUG_DEBUG_MAIN_TIME_AVERAGE_End[id] - DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start[id]);

  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave[id] += now_time;
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[id] ++;
  if( now_time > 16000 ){
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount[id] ++;
#if DEBUG_MAIN_TIME_AVERAGE
    OS_TPrintf( " id %d : ", id );
    OS_TPrintf( " over tick %ld", now_time );
    OS_TPrintf( " ave tick %d\n", (DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave[id] / DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[id]) );
#endif
  }
  if( DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max[id] < now_time ){
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max[id] = now_time;
  }
  if( DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[id] >= DEBUG_DEBUG_MAIN_TIME_AVERAGE_AVE_COUNT_TIME ){

#if DEBUG_MAIN_TIME_AVERAGE
    OS_TPrintf( " id %d : ", id );
    OS_TPrintf( " ave tick %d", 
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave[id] / DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[id] );
    OS_TPrintf( " Over 120::%d",
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount[id] );
    OS_TPrintf( " Max %d\n",
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max[id] );
#endif

    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave[id] = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count[id] = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount[id] = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max[id]       = 0;
  }

}

#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
