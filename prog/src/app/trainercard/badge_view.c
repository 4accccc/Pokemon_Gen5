//============================================================================================
/**
 * @file    badge_view.c
 * @brief   バッジ閲覧画面
 * @author  Akito Mori
 * @date    10.03.02
 *
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"            // HEAPID_TR_CARD
#include "system/cursor_move.h"     // CURSOR_MOVE
#include "system/gfl_use.h"         // GFUser_VIntr_CreateTCB
#include "system/wipe.h"            // WIPE_SYS_Start,
#include "system/bmp_winframe.h"            // WIPE_SYS_Start,
#include "sound/pm_sndsys.h"        // PMSND_PlaySE
#include "savedata/shortcut.h"      // SHORTCUT_SetRegister
#include "savedata/misc.h"          // MISC_GetBadgeFlag
#include "system/shortcut_data.h"   // 
#include "print/wordset.h"          // WORDSET
#include "print/printsys.h"         // PRINT_UTIL,PRINT_QUE...
#include "app/leader_board.h"       // proc
#include "app/app_menu_common.h"
#include "arc/arc_def.h"            // ARCID_LEADER_BOARD
#include "arc/trainer_case.naix"    // leaderboard graphic
#include "app/trainer_card.h"

#include "font/font.naix"           // NARC_font_large_gftr
#include "arc/message.naix"         // NARC_message_leader_board_dat
#include "msg/msg_trainercard.h"

#include "trcard_sys.h"
#include "trcard_snd_def.h"
//
#include "arc_def.h"
#include "system/bmp_oam.h"
#include "field/enc_cutin_no.h"

#include "badge_view_def.h"       // 各種定義等

#ifdef PM_DEBUG
#define SPECIAL_FEBRUARY
#endif

//--------------------------------------------------------------------------------------------
// 定数定義
//--------------------------------------------------------------------------------------------

// ３Ｄカメラ設定
#define BADGE3D_CAMERA_POS_Y     ( FX32_ONE*1.4  )  ///< ジムリーダーリングの倒れ具合
#define BADGE3D_CAMERA_POS_Z     ( FX32_ONE*5.49 )  ///< ジムリーダーとカメラの距離（ドットのゆがみ調整）
#define BADGE3D_CAMERA_TARGET_Y  ( FX32_ONE*-1.49 )  ///< 座標(0,0,0)のモデリングデータ画面上の位置

// 上画面文字面ウインドウオフセット
#define SYSTEMWIN_FRM_CGX_OFFSET  (  1 )
#define SYSTEMWIN_FRM_PAL_OFFSET  ( 14 )

// 情報ウインドウ文字列長
#define BADGEVIEW_MSG_LEN   ( 26*2*2 )    // 会話ウインドウ分ｘ２

#define BADGE_NUM   ( 8 )

// 48時間で磨き下限が１段階落ちるのでその倍数で管理
//#define BADGE_POLISH_RATE   ( 48*3 )  

// シーケンス制御
enum{
  SUBSEQ_FADEIN_WAIT=0,
  SUBSEQ_MAIN,
  SUBSEQ_ICON_WAIT,
  SUBSEQ_FADEOUT,
  SUBSEQ_FADEOUT_WAIT,
};

// BG指定
#define BV_BGFRAME_U_CUTIN  ( GFL_BG_FRAME1_M )
#define BV_BGFRAME_U_MARK   ( GFL_BG_FRAME2_M )
#define BV_BGFRAME_U_BG     ( GFL_BG_FRAME3_M )
#define BV_BGFRAME_D_MSG    ( GFL_BG_FRAME0_S )
#define BV_BGFRAME_D_BUTTON ( GFL_BG_FRAME1_S )
#define BV_BGFRAME_D_BG     ( GFL_BG_FRAME2_S )
// 以下、追加
#define BV_BGFRAME_U_NOUSE	( GFL_BG_FRAME3_S )
#define BV_BGFRAME_D_WIN	( GFL_BG_FRAME3_S )
//
#define OBJ_DRAW_UP			( CLSYS_DRAW_MAIN )
#define OBJ_DRAW_DOWN		( CLSYS_DRAW_SUB )
#define OBJ_DEFREND_UP		( CLSYS_DEFREND_MAIN )
#define OBJ_DEFREND_DOWN	( CLSYS_DEFREND_SUB )
//
#define PALTYPE_BG_DOWN		( PALTYPE_SUB_BG )
#define PALTYPE_BG_UP		( PALTYPE_MAIN_BG )
//
#define PAL_TOUCH_BG		( 13 )			// 下画面共通パネル用offset(ウインドウと重なるため変更)
#define PAL_BADGE_BG		(  9 )			// 下画面バッジ用offset
#define PAL_CUTIN_BG		( 10 )			// 上画面カットイン用offset

// 情報ウインドウ用定義
enum{
  INFO_TYPE_BADGE =0,
  INFO_TYPE_GYMLEADER,
};

// バッジのパレットを汚す為のパレットデータ数
#define BADGE_POLISH_PAL_NUM    ( 3 )
#define LEADER_PAL_NUM          ( 3 )
#define BADGE_KIRAKIRA_NUM      ( 3 )

//---------------
// 以下に追加
//---------------
// モード
enum{
	MODE_NORMAL = 0,				// 通常(なし)
	MODE_BADGE,						// バッジ
	MODE_LEADER,					// ジムリーダー
};

// 移動のフラグ(MWork.Flag)
enum{
	FLG_MOVE_INIT	= (1<<0),		// 初期化
	FLG_MOVE_END	= (1<<1),		// 終了
};

// 遷移(Step)
enum{
	STEP_INIT		= 0,
	STEP_LV1		= 100,
	STEP_LV2		= 200,
	STEP_LV3		= 300,
	//
	STEP_END		= 10000,
};

// Flag
enum{
	FLG_SET_BGOFFSET = (1<<0),		// BGセット
	FLG_END_BGOFFSET = (1<<1),		// 終了
	//
	FLG_DECIDE_OFF   = (1<<4),		// 決定ボタン(カードに戻る)表示off
	FLG_CANCEL_OFF   = (1<<5),		// cancelボタン(リターン)表示off
	FLG_XBUTTON_OFF  = (1<<6),		// Xボタン(やめる)表示off
	FLG_YBUTTON_OFF  = (1<<7),		// Yボタン(ブックマーク)表示off
};

// 表示on/offで使用
// (clwkと対応)
enum{
	BUTTON_CARD = 0,				// カードに戻る
	BUTTON_RETURN,					// リターン
	BUTTON_END,						// ×やめる
	BUTTON_BOOKMARK,				// ブックマーク
};

// スクロール
enum{
	MODE_UPDOWN = 0,				// 上から下
	MODE_DOWNUP,					// 下から上
	MODE_UPDOWN_REV,				// 上から下(逆)
	MODE_DOWNUP_REV,				// 下から上(逆)
};

// 移動のワーク
enum{
	MWORK_WIN = 0,					// WIN移動用
	MWORK_CUTIN,					// カットイン用
	
	MWORK_MAX,
};

// タイマのワーク
enum{
	TIMER_MES = 0,					// メッセージ用
	TIMER_PAD,
	
	TIMER_MAX,
};

// スクロールスピード
enum{
	SPD_WIN_IN	= 32,				// WIN入
	SPD_WIN_OUT	= 32,				// WIN出
	SPD_CUTIN 	= 32,				// カットイン
	SPD_CUTOUT	= 32,				// カットアウト
};

// いろいろ
enum{
	CUTIN_CHR_SIZE		= 0x2000,	// (カットイン)256キャラ分のキャラデータ
	CUTIN_CHR_OFFSET	= 256,		// (カットイン)256キャラ分オフセット
	CUTIN_PAL_SIZE		= 2,		// (カットイン)パレットサイズ(上下合計)
	//
	CUTIN_SCR_X0		= 128/8,	// (カットイン)左上の座標(キャラ単位)(上半身)
	CUTIN_SCR_Y0		= 0,		// (カットイン)
	CUTIN_SCR_W0		= 128/8,	// (カットイン)横
	CUTIN_SCR_H0		= 128/8,	// (カットイン)縦
    //
	CUTIN_SCR_X1		= 128/8,	// (カットイン)左上の座標(キャラ単位)(下半身)
	CUTIN_SCR_Y1		= 128/8,	// (カットイン)
	CUTIN_SCR_W1		= 128/8,	// (カットイン)横
	CUTIN_SCR_H1		=  64/8,	// (カットイン)縦
};

//--------------------------------------------------------------------------------------------
// 構造体定義定義
//--------------------------------------------------------------------------------------------

// ジムリーダースライド情報
typedef struct{
  u16 on;        ///< 現在のタッチ状況
  u16 old_on;    ///< １フレーム前のタッチ状況
  int x, old_x, old2_x;  ///< X座標、１フレーム前のX座標
  int start_x;   ///< 回転開始時座標 
  int enable;    ///< 
}SLIDE_WORK;

// バッジ磨き情報
typedef struct{
  int polish[BADGE_NUM];     ///< 磨き情報
  u8  grade[BADGE_NUM];      ///< 磨きフラグ
  u8  old_grade[BADGE_NUM];  ///< 磨きフラグ過去
}BADGE_POLISH;

// バッジ磨き判定用ワーク
typedef struct {
  s8 OldDirX;   
  s8 OldDirY;   
  s8 DirX;      
  s8 DirY;
  u8 Snd;
  u8 wait;
  u8 etc;
  u8 etc2;
  int BeforeX;
  int BeforeY;
}SCRATCH_WORK;

// 移動用のワーク(追加)
typedef struct
{
	s16			sx;			///< 開始
	s16			ex;			///< 終了
	s16			flag;		///< フラグ
	s16			pad;
	//
	s32			rate;		///< 加算値(x100)
	s32			calcnum;	///< 計算結果(x100)
} BV_MOVE_WORK;

  

typedef struct {
  ARCHANDLE     *g_handle;  ///< グラフィックデータファイルハンドル
  
  GFL_FONT      *font;      ///< フォントデータ
  GFL_BMPWIN    *InfoWin;
  PRINT_UTIL    printUtil;
  PRINT_QUE     *printQue;
  PRINT_STREAM  *printStream;
  GFL_TCBLSYS   *pMsgTcblSys;

  GFL_MSGDATA   *mman;    ///< メッセージデータマネージャ
  WORDSET       *wset;    ///< 単語セット
  STRBUF        *expbuf;  ///< メッセージ展開領域
  STRBUF        *strbuf[BV_STR_MAX];

  GFL_CLUNIT    *clUnit;            ///< セルアクターユニット
  GFL_CLWK      *clwk[BV_OBJ_MAX];  ///< セルアクターワーク
  u32           clres[BV_RES_MAX];  ///< セルアクターリソース用インデックス

  GFL_TCB       *VblankTcb;         ///< 登録したVblankFuncのポインタ

  // 3D全体
  GFL_G3D_UTIL  *g3dUtil;
  u16           unit_idx;           ///< GFL_G3D_UTILが割り振る番号
  GFL_G3D_LIGHTSET *light3d;
  GFL_G3D_CAMERA   *camera;
  
  TRCARD_CALL_PARAM *param;         ///< 呼び出しパラメータ
  int seq,next_seq;                 ///< サブシーケンス制御用ワーク
  u16 page,page_max;                ///< 現在の表示ページ

  int trainer_num;                  ///< バトルサブウェイデータに存在していた人数
  int scrol_bg;                     ///< BGスクロール用のワーク

  SLIDE_WORK  slide;          ///< ジムリーダー操作用のワーク
  int         old_animeFrame; ///< ジムリーダーアニメフレーム位置
  int         animeFrame;     ///< ジムリーダーアニメフレーム位置
  int         animeSpeed;     ///< ジムリーダーパネルの足し込むスピード
  int         targetSet;      ///< 移動先を指定しているか？
  int         targetFrame;    ///< 移動先フレーム
  // 以下に追加
  s32           Mode;				///< モード(0:なし、1:バッジ、2:リーダー)
  s32           Step;				///< ステップ
  u32           Flag;				///< フラグ
  s32           PageNum;			///< ページ番号(0-7)
  s32           TypeNum;			///< 情報ウインドウタイプ番号
  s16           BgOffY;				///< bgのスクロールオフセット(ウインドウ)
  s16           BgOffX;				///< bgのスクロールオフセット(カットイン)
  s16           LeaderNum;			///< リーダーメッセージ番号(0 or 1)
  s32           Timer[TIMER_MAX];	///< タイマ
  BV_MOVE_WORK  MWork[MWORK_MAX];	///< 移動のワーク
  void          *FacePalBuf;		///< 顔グラフィックパレット
  NNSG2dPaletteData *FacePal;		///< 顔グラフィック

  void              *badgePalBuf[BADGE_POLISH_PAL_NUM]; ///< バッジOBJパレットデータ保存用ワーク
  NNSG2dPaletteData *badgePal[BADGE_POLISH_PAL_NUM];    ///< バッジ用パレットデータ
  void              *leaderPalBuf[LEADER_PAL_NUM];      ///< ジムリーダー顔グラパレットデータ保存用ワーク
  NNSG2dPaletteData *leaderPal[LEADER_PAL_NUM];          ///< ジムリーダー顔グラパレットデータ

  u8          badgeflag[BADGE_NUM];

  TR_CARD_SV_PTR trCard;
  BADGE_POLISH   badge;
  SCRATCH_WORK   scratch;
  
#ifdef PM_DEBUG
  GFL_MSGDATA   *debugname;
#endif
} BADGEVIEW_WORK;

GFL_PROC_RESULT BadgeViewProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
GFL_PROC_RESULT BadgeViewProc_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
GFL_PROC_RESULT BadgeViewProc_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk );


//--------------------------------------------------------------------------------------------
// 実データ定義
//--------------------------------------------------------------------------------------------
// プロセス定義データ
const GFL_PROC_DATA BadgeViewProcData = {
  BadgeViewProc_Init,
  BadgeViewProc_Main,
  BadgeViewProc_End,
};

//-----------------
// 以下に追加
//-----------------
static const s16 bv_timer_tbl[] = {
	5*60,		// リーダーのwaitタイマ1
	8*60,		// リーダーのwaitタイマ2
};

//--------------------------------------------------------------------------------------------
// 関数プロトタイプ宣言
//--------------------------------------------------------------------------------------------
/*** 関数プロトタイプ ***/
static void VBlankFunc( GFL_TCB *tcb, void * work );
static void InitWork( BADGEVIEW_WORK *wk, void *pwk );
static void FreeWork( BADGEVIEW_WORK *wk );
static void VramBankSet(void);
static void BgInit(BADGEVIEW_WORK *wk);
static void BgExit(BADGEVIEW_WORK *wk) ;
static void BgGraphicInit(BADGEVIEW_WORK *wk);
static void BgGraphicExit( BADGEVIEW_WORK *wk );
static void ClActInit(BADGEVIEW_WORK *wk);
static void ClActSet(BADGEVIEW_WORK *wk) ;
static void ClActExit(BADGEVIEW_WORK *wk);
static void Graphic3DInit( BADGEVIEW_WORK *wk );
static void Graphic3DExit( BADGEVIEW_WORK *wk );
static void Resource3DInit( BADGEVIEW_WORK *wk );
static void Resource3DExit( BADGEVIEW_WORK *wk );
static void BmpWinInit(BADGEVIEW_WORK *wk);
static void BmpWinExit(BADGEVIEW_WORK *wk);
static void bmpwin_visible(BADGEVIEW_WORK *wk, const BOOL bflg);
static void PrintSystemInit(BADGEVIEW_WORK *wk);
static void PrintSystemDelete(BADGEVIEW_WORK *wk);
static void PrintSystem_Main( BADGEVIEW_WORK *wk );
static void BgFramePrint( BADGEVIEW_WORK *wk, int id, STRBUF *str, STRBUF *str2, STRBUF *str3 );
static void CursorMoveCallBack_On( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Off( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos );
static BOOL SEQ_Main( BADGEVIEW_WORK *wk );
static void InfoWinPrint( BADGEVIEW_WORK *wk, int type, int no, const s32 smode );
static void InfoWinPut( BADGEVIEW_WORK *wk );
static  int TouchBar_KeyControl( BADGEVIEW_WORK *wk );
static void ExecFunc( BADGEVIEW_WORK *wk, int trg, BOOL keyflag );
static void _page_max_init( BADGEVIEW_WORK *wk );
static void _page_clact_set( BADGEVIEW_WORK *wk, int page, int max );
static  int _page_move_check( BADGEVIEW_WORK *wk, int touch );
static void SetupPage( BADGEVIEW_WORK *wk, int page );
static void CellActorPosSet( BADGEVIEW_WORK *wk, int id, int x, int y );
static void Draw3D( BADGEVIEW_WORK* wk );
static void SlideFunc( BADGEVIEW_WORK *wk, int trg, int flag, int x );
static void SetRtcData( BADGEVIEW_WORK *wk );
static void RefreshPolishData( BADGEVIEW_WORK *wk );
static void Trans_BadgePalette( BADGEVIEW_WORK *wk );
static void BrushBadge( BADGEVIEW_WORK *wk, u32 touch );
static void CheckBookmark( BADGEVIEW_WORK *wk );
//
//-------------
// 以下に追加
//-------------
// 移動関連
static void bv_init_movework( BADGEVIEW_WORK *wk, const s32 wnum, const s16 sx, const s16 ex, const s16 frame );
static s16  bv_main_movework( BADGEVIEW_WORK *wk, const s32 wnum );
static BOOL bv_is_movework(   BADGEVIEW_WORK *wk, const s32 wnum );
// スクロール関連
static void bv_init_bgscroll( BADGEVIEW_WORK* wk, const s32 mode );
static void bv_main_loop(     BADGEVIEW_WORK* wk );
static void bv_main_scroll(   BADGEVIEW_WORK* wk );
static BOOL bv_is_bgscroll(   BADGEVIEW_WORK* wk );
// obj関連
static void bv_set_obj_visible( BADGEVIEW_WORK* wk, const s32 num, const BOOL bflg );
static void bv_set_btn_visible( BADGEVIEW_WORK* wk, const s32 mode, const BOOL bflg );
// タイマ関連
static void bv_init_timer( BADGEVIEW_WORK* wk, const s32 mode, const s32 timer );
static s32  bv_chk_timer(  BADGEVIEW_WORK* wk, const s32 mode );
static void bv_chk_leader( BADGEVIEW_WORK* wk );
// サウンド
static void bv_snd_badge( BADGEVIEW_WORK* wk, const s32 num );
// パレット変更
static void bv_chg_facepal( BADGEVIEW_WORK* wk, const s32 num );
static void bv_chk_facepal( BADGEVIEW_WORK* wk );
// カットイン関連
static void bv_init_cutin( BADGEVIEW_WORK* wk, const s32 cnum );
static void bv_chg_cutin(  BADGEVIEW_WORK* wk, const s32 num );
static void bv_set_cutin_visible( BADGEVIEW_WORK *wk, const BOOL bflg );


#ifdef PM_DEBUG
static void _debug_data_set( BADGEVIEW_WORK *wk );
#endif

// 追加
FS_EXTERN_OVERLAY(enc_cutin_no);

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：初期化
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT BadgeViewProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  BADGEVIEW_WORK * wk;

  OS_Printf( "↓↓↓↓↓　バッジ画面処理開始　↓↓↓↓↓\n" );

  //オーバーレイロード(カットイン情報用)
  GFL_OVERLAY_Load( FS_OVERLAY_ID(enc_cutin_no) );

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TR_CARD, 0x20000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(BADGEVIEW_WORK), HEAPID_TR_CARD );
  MI_CpuClearFast( wk, sizeof(BADGEVIEW_WORK) );

//  WIPE_SetBrightness( WIPE_DISP_MAIN,WIPE_FADE_BLACK );
//  WIPE_SetBrightness( WIPE_DISP_SUB,WIPE_FADE_BLACK );

  InitWork( wk, pwk );  // ワーク初期化
  VramBankSet();        // VRAM設定
  BgInit(wk);           // BGシステム初期化
  BgGraphicInit(wk);    // BGグラフィック転送
  ClActInit(wk);        // セルアクターシステム初期化
  ClActSet(wk);         // セルアクター登録
  Graphic3DInit(wk);    // 3D初期化
  Resource3DInit(wk);   // 3Dリソース読み込み
  BmpWinInit(wk);       // BMPWIN初期化
  PrintSystemInit(wk);  // 描画システム初期化

  // 初期ページ表示
  SetupPage( wk, wk->page );

  // 通信アイコン表示
  GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_TR_CARD );


  // フェードイン開始
  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
                  WIPE_FADE_BLACK,  WIPE_DEF_DIV, 1, HEAPID_TR_CARD );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：メイン
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT BadgeViewProc_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  BADGEVIEW_WORK * wk = mywk;

  if( SEQ_Main( wk ) == FALSE ){
    return GFL_PROC_RES_FINISH;
  }
  
  Draw3D(wk);
  PrintSystem_Main( wk );         // 文字列描画メイン
  GFL_CLACT_SYS_Main();           // セルアクターメイン

  if(++wk->scrol_bg >= 256){
    wk->scrol_bg=0;
  }
  
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：終了
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT BadgeViewProc_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  BADGEVIEW_WORK * wk = mywk;
  
  PrintSystemDelete(wk);  // 描画システム解放
  BmpWinExit(wk);         // BMPWIN解放
  Resource3DExit(wk);     // 3Dリソース解放
  Graphic3DExit(wk);      // 3D終了
  ClActExit(wk);          // セルアクターシステム解放
  BgGraphicExit(wk);      // BGグラフィック関連終了
  BgExit(wk);             // BGシステム終了
  FreeWork(wk);           // ワーク解放
  SetRtcData(wk);           // 日付を保存

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_CheckHeapSafe( HEAPID_TR_CARD );
  GFL_HEAP_DeleteHeap( HEAPID_TR_CARD );

  //オーバーレイアンロード(カットイン情報用)
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(enc_cutin_no) );

  OS_Printf( "↑↑↑↑↑　バッジ画面処理終了　↑↑↑↑↑\n" );

  return GFL_PROC_RES_FINISH;
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
/**
 * 関数
 *
 * @param none
 *
 * @return  none
 */
//----------------------------------------------------------------------------------
static void VBlankFunc( GFL_TCB *tcb, void * work )
{
  BADGEVIEW_WORK * wk = (BADGEVIEW_WORK*)work;
  int scr;
  
  scr = -(wk->scrol_bg/2);
  GFL_BG_SetScroll( BV_BGFRAME_U_BG, GFL_BG_SCROLL_X_SET, scr );
  GFL_BG_SetScroll( BV_BGFRAME_U_BG, GFL_BG_SCROLL_Y_SET, scr );
//  GFL_BG_SetScroll( BV_BGFRAME_D_BG, GFL_BG_SCROLL_X_SET, scr);
//  GFL_BG_SetScroll( BV_BGFRAME_D_BG, GFL_BG_SCROLL_Y_SET, scr);

  // BGスクロール
  if( wk->Flag & FLG_SET_BGOFFSET ){
    // win
    GFL_BG_SetScroll( BV_BGFRAME_D_WIN, GFL_BG_SCROLL_Y_SET, wk->BgOffY );
    // カットイン(test)
    GFL_BG_SetScroll( BV_BGFRAME_U_CUTIN, GFL_BG_SCROLL_X_SET, wk->BgOffX );
    // 最後の1フレームをセットするため
    if(wk->Flag & FLG_END_BGOFFSET){
      wk->Flag &= ~FLG_SET_BGOFFSET;
      wk->Flag &= ~FLG_END_BGOFFSET;
	}
  }
  
  // セルアクター
  GFL_CLACT_SYS_VBlankFunc();

  // BG転送  
  GFL_BG_VBlankFunc();
  
}


//----------------------------------------------------------------------------------
/**
 * @brief ワーク初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void InitWork( BADGEVIEW_WORK *wk, void *pwk )
{
  int i;
  wk->param     = pwk;
  wk->next_seq  = SUBSEQ_FADEIN_WAIT;

  // グラフィックデータハンドルオープン
  wk->g_handle  = GFL_ARC_OpenDataHandle( ARCID_TRAINERCARD, HEAPID_TR_CARD);

  // メッセージマネージャー確保  
  wk->mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                             NARC_message_trainercard_dat, HEAPID_TR_CARD );


  // leader_board.gmmの文字列を全て読み込む
  {
    int i;
    const STRCODE *buf;
    for(i=0;i<BV_STR_MAX;i++){
      wk->strbuf[i] = GFL_MSG_CreateString( wk->mman, i );
      buf = GFL_STR_GetStringCodePointer( wk->strbuf[i] );
      OS_Printf("str%d=%02d, %02d, %02d, %02d, %02d,\n", i,buf[0],buf[1],buf[2],buf[3],buf[4]);
    }
  }
  // 展開用文字列確保
  wk->expbuf = GFL_STR_CreateBuffer( BADGEVIEW_MSG_LEN, HEAPID_TR_CARD );

  // ワードセットワーク確保
  wk->wset = WORDSET_CreateEx( 8, WORDSET_COUNTRY_BUFLEN, HEAPID_TR_CARD );
  
  
  // VBlank関数セット
  wk->VblankTcb = GFUser_VIntr_CreateTCB( VBlankFunc, wk, 0 );  
  


#ifdef PM_DEBUG
  wk->debugname = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                             NARC_message_trname_dat, HEAPID_TR_CARD );

  // デバッグ用データセット
  _debug_data_set( wk );

  GFL_MSG_Delete( wk->debugname );

#endif
  
  // トレーナー数数え上げ
  wk->trainer_num = 8;

  // ジムリーダー回転ワーク初期化
  wk->targetSet    = -1;
  // 最大ページ数取得
  _page_max_init(wk);

  // トレーナーカードセーブデータ
  wk->trCard = GAMEDATA_GetTrainerCardPtr( wk->param->gameData);

  // 時間取得を行いバッジ磨きデータの更新を行う
  RefreshPolishData( wk );
  
  // 追加(初期化)
  wk->Mode      = MODE_NORMAL;
  wk->Step      = STEP_INIT;
  wk->Flag      = 0;
  wk->PageNum   = 0;
  wk->TypeNum   = 0;
  wk->BgOffY    = 0;
  wk->BgOffX    = 0;
  wk->LeaderNum = 0;
  wk->FacePalBuf = 0;
  wk->FacePal	= 0;
  //
  for(i=0;i<TIMER_MAX;i++){
    wk->Timer[i] = 0;
  }
  for(i=0;i<MWORK_MAX;i++){
    wk->MWork[i].flag = 0;
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief ワーク解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void FreeWork( BADGEVIEW_WORK *wk )
{
  int i;
  
  
  // 文字列解放
  for(i=0;i<BV_STR_MAX;i++){
    GFL_STR_DeleteBuffer( wk->strbuf[i] );
  }
  GFL_STR_DeleteBuffer( wk->expbuf );

  // ワードセット解放
  WORDSET_Delete( wk->wset );

  // メッセージデータ解放
  GFL_MSG_Delete( wk->mman );


  // Vblank期間中のBG転送終了
  GFL_TCB_DeleteTask( wk->VblankTcb );

  GFL_ARC_CloseDataHandle( wk->g_handle );
}

//============================
// VRAM割り振り
//============================
static const GFL_DISP_VRAM VramTbl = {
  GX_VRAM_BG_128_B,           // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット

  GX_VRAM_OBJ_64_E,           // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_128_D,      // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ拡張パレット

  GX_VRAM_TEX_0_A,            // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_01_FG,      // テクスチャパレットスロット

  GX_OBJVRAMMODE_CHAR_1D_32K, // メインＯＢＪ領域
  GX_OBJVRAMMODE_CHAR_1D_32K, // サブＯＢＪ領域
};

//----------------------------------------------------------------------------------
/**
 * @brief VRAM設定
 *
 * @param   none    
 */
//----------------------------------------------------------------------------------
static void VramBankSet(void)
{
  GFL_DISP_SetBank( &VramTbl );

  // メイン画面を上に表示
  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
}


// 初期化を行うBG面
static const int bgframe_init_tbl[]={
  BV_BGFRAME_U_CUTIN,
  BV_BGFRAME_U_MARK,
  BV_BGFRAME_U_BG,
  BV_BGFRAME_D_MSG,
  BV_BGFRAME_D_BUTTON,
  BV_BGFRAME_D_BG,
  //
  BV_BGFRAME_D_WIN,
};

//----------------------------------------------------------------------------------
/**
 * @brief BGシステム初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BgInit(BADGEVIEW_WORK *wk)
{
  int i;

  // BG SYSTEM初期化
  GFL_BG_Init( HEAPID_TR_CARD );

  // BGモード設定
  { 
    GFL_BG_SYS_HEADER sysh = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGMode( &sysh );
  }

  {
    // 各BG初期化設定
    const GFL_BG_BGCNT_HEADER header[]={
      { // 上画面：カットイン
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01,      0, 0, 0, FALSE
      },
      { // 上画面：リーグマーク背景
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x128,
        GX_BG_EXTPLTT_01,    2, 0, 0, FALSE
      },
      { // 上画面：背景
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x128,
        GX_BG_EXTPLTT_01,    3, 0, 0, FALSE
      },
      //
      //
      { // 下画面：プレート枠
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x128,
        GX_BG_EXTPLTT_01,      2, 0, 0, FALSE
      },
      { // 下画面：タッチバー共通
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x128,
        GX_BG_EXTPLTT_01,      0, 0, 0, FALSE
      },
      { // 下画面：顔入り背景
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x128,
        GX_BG_EXTPLTT_01,      3, 0, 0, FALSE
      },
      //
      { // 下画面：WIN(追加)
        0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x18000, GFL_BG_CHRSIZ_256x128,
        GX_BG_EXTPLTT_01,      1, 0, 0, FALSE
      },
    };

    GF_ASSERT( NELEMS(header)==NELEMS(bgframe_init_tbl) );

    // BG設定・スクリーンクリア・０キャラ目クリア
    for(i=0;i<NELEMS(bgframe_init_tbl);i++)
    {
      GFL_BG_SetBGControl( bgframe_init_tbl[i], &header[i], GFL_BG_MODE_TEXT );
      GFL_BG_ClearScreen(  bgframe_init_tbl[i] );
      GFL_BG_SetClearCharacter( bgframe_init_tbl[i], 0x20, 0, HEAPID_TR_CARD );
      GFL_BG_SetVisible(  bgframe_init_tbl[i], VISIBLE_ON     );
    }
    // 残りのBG面をOFFに
//    GFL_BG_SetVisible( BV_BGFRAME_U_NOUSE, VISIBLE_OFF );
  }
  
  // 半透明設定
  G2S_BlendNone();
  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3,7,16);
}
//----------------------------------------------------------------------------------
/**
 * @brief  BGシステム終了
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BgExit(BADGEVIEW_WORK *wk) 
{
  int i;
  for(i=0;i<NELEMS(bgframe_init_tbl);i++){
    GFL_BG_FreeBGControl( bgframe_init_tbl[i] );
  }
  GFL_BG_Exit();

  G2_BlendNone();
  G2S_BlendNone();
}


//----------------------------------------------------------------------------------
/**
 * @brief BGグラフィック転送
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BgGraphicInit(BADGEVIEW_WORK *wk)
{
  int i;
  ARCHANDLE * handle = wk->g_handle;

  // 上画面背景転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_trainer_case_badge_bg01_NCGR,
                                        BV_BGFRAME_U_BG, 0, 0, FALSE, HEAPID_TR_CARD );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_trainer_case_badge_bg02_NSCR,
                                        BV_BGFRAME_U_BG, 0, 0, FALSE, HEAPID_TR_CARD );
  GFL_ARCHDL_UTIL_TransVramPalette(     handle, NARC_trainer_case_badge_bg02_NCLR, 
                                        PALTYPE_BG_UP, 0, 0, HEAPID_TR_CARD );

  // 上画面背景(リーグマーク）転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_trainer_case_badge_bg_02_NCGR,
                                        BV_BGFRAME_U_MARK, 0, 0, FALSE, HEAPID_TR_CARD );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_trainer_case_badge_bg03_NSCR,
                                        BV_BGFRAME_U_MARK, 0, 0, FALSE, HEAPID_TR_CARD );

  //
  // 下画面背景転送(顔入り背景)
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_trainer_case_mytest00_NCGR,
                                        BV_BGFRAME_D_BG, 0, 0, FALSE, HEAPID_TR_CARD );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_trainer_case_mytest00_NSCR,
                                        BV_BGFRAME_D_BG, 0, 0, FALSE, HEAPID_TR_CARD );
  GFL_ARCHDL_UTIL_TransVramPalette(     handle, NARC_trainer_case_mytest01_NCLR, 			// グレー
                                        PALTYPE_BG_DOWN, 0, 0, HEAPID_TR_CARD );
//  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_trainer_case_badge_bg01_NCGR,
//                                        BV_BGFRAME_D_BG, 0, 0, FALSE, HEAPID_TR_CARD );
//  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_trainer_case_badge_bg02_NSCR,
//                                        BV_BGFRAME_D_BG, 0, 0, FALSE, HEAPID_TR_CARD );
//  GFL_ARCHDL_UTIL_TransVramPalette(     handle, NARC_trainer_case_badge_bg01_NCLR, 
//                                        PALTYPE_BG_DOWN, 0, 0, HEAPID_TR_CARD );

  // 下画面背景転送(プレート枠)
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_trainer_case_badge_bg01_NCGR,
                                        BV_BGFRAME_D_MSG, 0, 0, FALSE, HEAPID_TR_CARD );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_trainer_case_badge_bg01_NSCR,
                                        BV_BGFRAME_D_MSG, 0, 0, FALSE, HEAPID_TR_CARD );
  GFL_ARCHDL_UTIL_TransVramPalette(     handle, NARC_trainer_case_badge_bg01_NCLR, 
                                        PALTYPE_BG_DOWN, PAL_BADGE_BG*0x20, 0x20, HEAPID_TR_CARD );
  // プレート枠のパレット指定を変更(test)
  GFL_BG_ChangeScreenPalette( BV_BGFRAME_D_MSG, 0, 11, 32, 10, PAL_BADGE_BG );
  GFL_BG_LoadScreenReq( BV_BGFRAME_D_MSG );
  
  
  // タッチバーBG転送
  {
    ARCHANDLE *c_handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_TR_CARD);

    GFL_ARCHDL_UTIL_TransVramBgCharacter( c_handle, APP_COMMON_GetBarCharArcIdx(),
                                          BV_BGFRAME_D_BUTTON, 0, 0, FALSE, HEAPID_TR_CARD );
    GFL_ARCHDL_UTIL_TransVramScreen(      c_handle, APP_COMMON_GetBarScrnArcIdx(),
                                          BV_BGFRAME_D_BUTTON, 0, 0, FALSE, HEAPID_TR_CARD );
    GFL_ARCHDL_UTIL_TransVramPaletteEx(   c_handle, APP_COMMON_GetBarPltArcIdx(), PALTYPE_BG_DOWN,
                                          0, PAL_TOUCH_BG*32, 32, HEAPID_TR_CARD );
    
    // スクリーンのパレット指定を変更
    GFL_BG_ChangeScreenPalette( BV_BGFRAME_D_BUTTON, TOUCHBAR_X, TOUCHBAR_Y,
                                                     TOUCHBAR_W, TOUCHBAR_H, PAL_TOUCH_BG );
    GFL_BG_LoadScreenReq( BV_BGFRAME_D_BUTTON );

    GFL_ARC_CloseDataHandle( c_handle );
  }
  
  // 会話フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_BG_DOWN, 
                                 TALKFONT_PAL_OFFSET, 32, HEAPID_TR_CARD );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_BG_UP, 
                                 TALKFONT_PAL_OFFSET, 32, HEAPID_TR_CARD );

  // 会話ウインドウ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_trainer_case_mytest10_NCGR,
                                        BV_BGFRAME_D_WIN, 0, 0, FALSE, HEAPID_TR_CARD );
  GFL_ARCHDL_UTIL_TransVramScreen(      handle, NARC_trainer_case_mytest10_NSCR,
                                        BV_BGFRAME_D_WIN, 0, 0, FALSE, HEAPID_TR_CARD );
  GFL_ARCHDL_UTIL_TransVramPalette(     handle, NARC_trainer_case_mytest10_NCLR, 
                                        PALTYPE_BG_DOWN, SYSTEMWIN_FRM_PAL_OFFSET*0x20, 0x20, HEAPID_TR_CARD );
  GFL_BG_ChangeScreenPalette( BV_BGFRAME_D_WIN, 0, 0, 32, 12, SYSTEMWIN_FRM_PAL_OFFSET );
  GFL_BG_LoadScreenReq( BV_BGFRAME_D_WIN );

  // システムウインドウリソース転送
//  BmpWinFrame_PalSet( BV_BGFRAME_D_WIN, SYSTEMWIN_FRM_PAL_OFFSET, MENU_TYPE_SYSTEM, HEAPID_TR_CARD );
//  BmpWinFrame_CgxSet( BV_BGFRAME_D_WIN, SYSTEMWIN_FRM_CGX_OFFSET, MENU_TYPE_SYSTEM, HEAPID_TR_CARD);


  // バッジOBJ転送用パレット保持
  for(i=0;i<BADGE_POLISH_PAL_NUM;i++){
    wk->badgePalBuf[i] = GFL_ARCHDL_UTIL_LoadPalette( handle, NARC_trainer_case_badge_obj03_NCLR-i, 
                                                      &wk->badgePal[i], HEAPID_TR_CARD );
  }

  for(i=0;i<LEADER_PAL_NUM;i++){
    wk->leaderPalBuf[i] = GFL_ARCHDL_UTIL_LoadPalette( handle, NARC_trainer_case_gym_leaders2_NCLR-i, 
                                                       &wk->leaderPal[i], HEAPID_TR_CARD );
  }

  // 顔グラフィック用パレット(色つき)
  wk->FacePalBuf = GFL_ARCHDL_UTIL_LoadPalette( handle, NARC_trainer_case_mytest00_NCLR, &wk->FacePal, HEAPID_TR_CARD );
}



//----------------------------------------------------------------------------------
/**
 * @brief BG関連システム終了処理
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BgGraphicExit( BADGEVIEW_WORK *wk )
{
  int i;
  
  // リーダー顔グラ用パレットデータ解放
  for(i=0;i<LEADER_PAL_NUM;i++){
    GFL_HEAP_FreeMemory(  wk->leaderPalBuf[i] );
  }
  // OBJパレット解放
  for(i=0;i<BADGE_POLISH_PAL_NUM;i++){
   GFL_HEAP_FreeMemory( wk->badgePalBuf[i] );
  }
  
  // パレット開放
  GFL_HEAP_FreeMemory( wk->FacePalBuf );
}

//----------------------------------------------------------------------------------
/**
 * @brief セルアクターシステム初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void ClActInit(BADGEVIEW_WORK *wk)
{
  ARCHANDLE *c_handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_TR_CARD);

  // セルアクター初期化
  GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN, &VramTbl, HEAPID_TR_CARD );
  wk->clUnit  = GFL_CLACT_UNIT_Create( BV_CLACT_NUM, 1,  HEAPID_TR_CARD );

  // OBJ面表示ON
  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_OBJ, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

  // セルアクターリソース読み込み
  
  // ---バッジ画面用下画面---
  wk->clres[BV_RES_CHR]  = GFL_CLGRP_CGR_Register(      wk->g_handle, 
                                                        NARC_trainer_case_badge_obj01_NCGR, 0, 
                                                        OBJ_DRAW_DOWN, HEAPID_TR_CARD );
  wk->clres[BV_RES_PLTT] = GFL_CLGRP_PLTT_Register(     wk->g_handle, 
                                                        NARC_trainer_case_badge_obj01_NCLR, 
                                                        OBJ_DRAW_DOWN, 0, HEAPID_TR_CARD );
  wk->clres[BV_RES_CELL] = GFL_CLGRP_CELLANIM_Register( wk->g_handle, 
                                                        NARC_trainer_case_badge_obj01_NCER, 
                                                        NARC_trainer_case_badge_obj01_NANR, 
                                                        HEAPID_TR_CARD );

  // 共通メニュー素材
  wk->clres[BV_RES_COMMON_CHR] = GFL_CLGRP_CGR_Register(       c_handle, 
                                                               APP_COMMON_GetBarIconCharArcIdx(), 0, 
                                                               OBJ_DRAW_DOWN, 
                                                               HEAPID_TR_CARD );

  wk->clres[BV_RES_COMMON_PLTT] = GFL_CLGRP_PLTT_RegisterEx(   c_handle, 
                                                               APP_COMMON_GetBarIconPltArcIdx(), 
                                                               OBJ_DRAW_DOWN, 
                                                               11*32, 0, 3, HEAPID_TR_CARD );

  wk->clres[BV_RES_COMMON_CELL] = GFL_CLGRP_CELLANIM_Register( c_handle, 
                                                               APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_32K), 
                                                               APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_32K), 
                                                               HEAPID_TR_CARD );
#if 0
  // ---上画面(追加test)---
  wk->clres[BV_RES_CHR_UP]  = GFL_CLGRP_CGR_Register(      wk->g_handle, 
                                                           NARC_trainer_case_badge_obj01_NCGR, 0, 
                                                           OBJ_DRAW_UP, HEAPID_TR_CARD );
  wk->clres[BV_RES_PLTT_UP] = GFL_CLGRP_PLTT_Register(     wk->g_handle, 
                                                           NARC_trainer_case_badge_obj01_NCLR, 
                                                           OBJ_DRAW_UP, 0, HEAPID_TR_CARD );
  wk->clres[BV_RES_CELL_UP] = GFL_CLGRP_CELLANIM_Register( wk->g_handle, 
                                                           NARC_trainer_case_badge_obj01_NCER, 
                                                           NARC_trainer_case_badge_obj01_NANR, 
                                                           HEAPID_TR_CARD );
#endif
  GFL_ARC_CloseDataHandle( c_handle );

}

//-------------------------------------------
// セルアクター表示用テーブル
//-------------------------------------------

static const int clact_dat[][8]={
  //   X      Y  anm, chr,               pltt,               cell,             bgprio, up/down
  {  1*8,  21*8,  11, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,         0, OBJ_DEFREND_DOWN, },// カードに戻る
  { 28*8,  21*8,   1, BV_RES_COMMON_CHR, BV_RES_COMMON_CHR,  BV_RES_COMMON_CHR,   0, OBJ_DEFREND_DOWN, },// もどる
  { 24*8,  21*8,   0, BV_RES_COMMON_CHR, BV_RES_COMMON_CHR,  BV_RES_COMMON_CHR,   0, OBJ_DEFREND_DOWN, },// ×
  { 20*8,  21*8+4,   7, BV_RES_COMMON_CHR, BV_RES_COMMON_CHR,  BV_RES_COMMON_CHR, 0, OBJ_DEFREND_DOWN, },// Ｙチェック
  {  2*8,  16*8,   0, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,         2, OBJ_DEFREND_DOWN, },// バッジ0
  {  6*8,  16*8,   1, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,         2, OBJ_DEFREND_DOWN, },// バッジ1
  { 10*8,  16*8,   2, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,         2, OBJ_DEFREND_DOWN, },// バッジ2
  { 14*8,  16*8,   3, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,         2, OBJ_DEFREND_DOWN, },// バッジ3
  { 18*8,  16*8,   4, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,         2, OBJ_DEFREND_DOWN, },// バッジ4
  { 22*8,  16*8,   5, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,         2, OBJ_DEFREND_DOWN, },// バッジ5
  { 26*8,  16*8,   6, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,         2, OBJ_DEFREND_DOWN, },// バッジ6
  { 30*8,  16*8,   7, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,         2, OBJ_DEFREND_DOWN, },// バッジ7
  {  (4*0+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ0-１
  {  (4*0+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ0-２
  {  (4*0+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ0-３
  {  (4*1+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ1-１
  {  (4*1+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ1-２
  {  (4*1+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ1-３
  {  (4*2+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ2-１
  {  (4*2+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ2-２
  {  (4*2+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ2-３
  {  (4*3+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ3-１
  {  (4*3+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ3-２
  {  (4*3+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ3-３
  {  (4*4+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ4-１
  {  (4*4+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ4-２
  {  (4*4+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ4-３
  {  (4*5+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ5-１
  {  (4*5+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ5-２
  {  (4*5+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ5-３
  {  (4*6+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ6-１
  {  (4*6+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ6-２
  {  (4*6+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ6-３
  {  (4*7+2)*8,  14*8,   9, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ7-１
  {  (4*7+4)*8,  17*8,  10, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ7-２
  {  (4*7+3)*8,  19*8,   8, BV_RES_CHR,        BV_RES_PLTT,        BV_RES_CELL,   2, OBJ_DEFREND_DOWN, },// キラキラ7-３
  //
//  {  264,  128,  0, BV_RES_CHR_UP,        BV_RES_PLTT_UP,        BV_RES_CELL_UP,  1, OBJ_DEFREND_UP,   },// カットイン
};

//----------------------------------------------------------------------------------
/**
 * @brief セルアクター登録
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void ClActSet(BADGEVIEW_WORK *wk) 
{

  GFL_CLWK_DATA add;
  int i;

  // セルアクター登録
  for(i=0;i<BV_OBJ_MAX;i++){
    add.pos_x  = clact_dat[i][0];
    add.pos_y  = clact_dat[i][1];
    add.anmseq = clact_dat[i][2];
    add.bgpri    = clact_dat[i][6];	// bgprio指定
    add.softpri  = BV_OBJ_MAX-i;

    wk->clwk[i] = GFL_CLACT_WK_Create( wk->clUnit,
                                       wk->clres[clact_dat[i][3]],
                                       wk->clres[clact_dat[i][4]],
                                       wk->clres[clact_dat[i][5]],
                                       &add, clact_dat[i][7], HEAPID_TR_CARD );		// up/down指定
    GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[i], TRUE );
  }
  
}



//----------------------------------------------------------------------------------
/**
 * @brief セルアクターシステム解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void ClActExit( BADGEVIEW_WORK *wk )
{

  GFL_CLGRP_CGR_Release(      wk->clres[BV_RES_COMMON_CHR] );
  GFL_CLGRP_PLTT_Release(     wk->clres[BV_RES_COMMON_PLTT] );
  GFL_CLGRP_CELLANIM_Release( wk->clres[BV_RES_COMMON_CELL] );
  GFL_CLGRP_CGR_Release(      wk->clres[BV_RES_CHR] );
  GFL_CLGRP_PLTT_Release(     wk->clres[BV_RES_PLTT] );
  GFL_CLGRP_CELLANIM_Release( wk->clres[BV_RES_CELL] );
#if 0
  GFL_CLGRP_CGR_Release(      wk->clres[BV_RES_CHR_UP] );
  GFL_CLGRP_PLTT_Release(     wk->clres[BV_RES_PLTT_UP] );
  GFL_CLGRP_CELLANIM_Release( wk->clres[BV_RES_CELL_UP] );
#endif

  // セルアクターユニット破棄
  GFL_CLACT_UNIT_Delete( wk->clUnit );

  //OAMレンダラー破棄
  GFL_CLACT_SYS_Delete();

}


//ライト初期設定データ
static const GFL_G3D_LIGHT_DATA light_data[] = {
  { 0, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 1, {{  (FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 2, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 3, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
};
static const GFL_G3D_LIGHTSET_SETUP light3d_setup = { light_data, NELEMS(light_data) };

// カメラ設定関連
#define cameraPerspway  ( 0x0b60 )
#define cameraAspect    ( FX32_ONE * 4 / 3 )
#define cameraNear      ( 1 << FX32_SHIFT )
#define cameraFar       ( 1024 << FX32_SHIFT )

//-------------------------------------------------------------------------------------------
/**
 * @brief 3D初期化関数
 */
//-------------------------------------------------------------------------------------------
static void Graphic3DInit( BADGEVIEW_WORK *wk )
{

  // 3Dシステムを初期化
  GFL_G3D_Init( 
      GFL_G3D_VMANLNK, GFL_G3D_TEX128K, 
      GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0x1000, HEAPID_TR_CARD, NULL );

  // ライト作成
  wk->light3d = GFL_G3D_LIGHT_Create( &light3d_setup, HEAPID_TR_CARD );
  GFL_G3D_LIGHT_Switching( wk->light3d );

  // カメラ初期設定
  {
    GFL_G3D_PROJECTION proj = { GFL_G3D_PRJPERS, 0, 0, cameraAspect, 0, cameraNear, cameraFar, 0 }; 
    proj.param1 = FX_SinIdx( cameraPerspway ); 
    proj.param2 = FX_CosIdx( cameraPerspway ); 
    GFL_G3D_SetSystemProjection( &proj ); 
  }
  
  // 3D面をON
  GFL_BG_SetVisible(  GFL_BG_FRAME0_M, VISIBLE_ON );
  // 3D面のプライオリティをセット(1)
  G2_SetBG0Priority( 1 );
  
  // 3D半透明を有効に
  G3X_AlphaBlend(TRUE);
}

//-------------------------------------------------------------------------------------------
/**
 * @brief 終了処理
 */
//-------------------------------------------------------------------------------------------
static void Graphic3DExit( BADGEVIEW_WORK *wk )
{
  // ライトセット破棄
  GFL_G3D_LIGHT_Delete( wk->light3d );

  // 3Dシステム終了
  GFL_G3D_Exit();

}



//============================================================================================
/**
 * @brief 3Dデータ
 */
//============================================================================================

// ジムリーダーパネル
static const GFL_G3D_UTIL_RES res_table_gymleader[] = 
{
#if PM_VERSION == VERSION_B2  
  { ARCID_TRAINERCARD, NARC_trainer_case_gym_leader_b_NSBMD, GFL_G3D_UTIL_RESARC },
  { ARCID_TRAINERCARD, NARC_trainer_case_gym_leader_b_NSBCA, GFL_G3D_UTIL_RESARC },
#else
  { ARCID_TRAINERCARD, NARC_trainer_case_gym_leader_NSBMD, GFL_G3D_UTIL_RESARC },
  { ARCID_TRAINERCARD, NARC_trainer_case_gym_leader_NSBCA, GFL_G3D_UTIL_RESARC },
#endif
};


static const GFL_G3D_UTIL_ANM anm_table_gymleader[] = 
{
  { 1, 0 },
};
static const GFL_G3D_UTIL_OBJ obj_table_gymleader[] = 
{
  {
    0,                           // モデルリソースID
    0,                           // モデルデータID(リソース内部INDEX)
    0,                           // テクスチャリソースID
    anm_table_gymleader,         // アニメテーブル(複数指定のため)
    NELEMS(anm_table_gymleader), // アニメリソース数
  },
}; 

// セットアップデータ
static const GFL_G3D_UTIL_SETUP setup[] =
{
  { res_table_gymleader, NELEMS(res_table_gymleader), obj_table_gymleader, NELEMS(obj_table_gymleader) },
};

#define SETUP_INDEX_MAX          ( 1 )


//--------------------------------------------------------------------------------------------
/**
 * @breif 3Dリソース読み込み
 */
//-------------------------------------------------------------------------------------------- 
static void Resource3DInit( BADGEVIEW_WORK *wk )
{
  // 3D管理ユーティリティーのセットアップ
  wk->g3dUtil = GFL_G3D_UTIL_Create( 10, 10, HEAPID_TR_CARD );

  // ユニット追加
  wk->unit_idx = GFL_G3D_UTIL_AddUnit( wk->g3dUtil, &setup[0] );

  // アニメーション有効化
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      int j;
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( wk->g3dUtil, wk->unit_idx );
      int anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      for( j=0; j<anime_count; j++ )
      {
        GFL_G3D_OBJECT_EnableAnime( obj, j );
      }
    }
  }

  // カメラ作成
  {
    VecFx32    pos = { 0, BADGE3D_CAMERA_POS_Y,    BADGE3D_CAMERA_POS_Z };
    VecFx32 target = { 0, BADGE3D_CAMERA_TARGET_Y, 0                    };
    wk->camera   = GFL_G3D_CAMERA_CreateDefault( &pos, &target, HEAPID_TR_CARD );
  }


}

//--------------------------------------------------------------------------------------------
/**
 * @breif 3Dリソース解放
 */
//-------------------------------------------------------------------------------------------- 
static void Resource3DExit( BADGEVIEW_WORK *wk )
{ 
  // カメラ破棄
  GFL_G3D_CAMERA_Delete( wk->camera );

  // ユニット破棄
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      GFL_G3D_UTIL_DelUnit( wk->g3dUtil, wk->unit_idx );
    }
  }

  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( wk->g3dUtil );
}


#ifdef PM_DEBUG
static VecFx32 test_pos    = { 0, BADGE3D_CAMERA_POS_Y, BADGE3D_CAMERA_POS_Z };
static VecFx32 test_target = { 0, BADGE3D_CAMERA_TARGET_Y, 0 };
static int     moveflag;
#endif
//--------------------------------------------------------------------------------------------
/**
 * @breif 描画
 */
//-------------------------------------------------------------------------------------------- 
static void Draw3D( BADGEVIEW_WORK* wk )
{
  static fx32 frame = 0;
//  static fx32 anime_speed = FX32_ONE;
  static fx32 anime_speed = 0;
  GFL_G3D_OBJSTATUS status;

  VEC_Set( &status.trans, 0, 0, 0 );
  VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &status.rotate );

  // カメラ更新
  GFL_G3D_CAMERA_Switching( wk->camera );

#ifdef PM_DEBUG
  // TEMP: カメラ設定
  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L)
  {
    
    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_X){
      test_pos.z -= FX32_ONE /10;
    }else if(GFL_UI_KEY_GetCont()&PAD_BUTTON_Y){
      test_pos.z += FX32_ONE /10;
    }else if(GFL_UI_KEY_GetCont()&PAD_KEY_UP){
      test_pos.y -= FX32_ONE /10;
    }else if(GFL_UI_KEY_GetCont()&PAD_KEY_DOWN){
      test_pos.y += FX32_ONE /10;
    }else if(GFL_UI_KEY_GetCont()&PAD_KEY_LEFT){
      test_target.y -= FX32_ONE /10;
    }else if(GFL_UI_KEY_GetCont()&PAD_KEY_RIGHT){
      test_target.y += FX32_ONE /10;
    }
    GFL_G3D_CAMERA_SetPos( wk->camera, &test_pos );
    GFL_G3D_CAMERA_SetTarget( wk->camera, &test_target );
    OS_Printf("pos.y=%d pos.z=%d, target.y=%d \n", test_pos.y, test_pos.z, test_target.y);
  }
//  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_R){
//    anime_speed = FX32_ONE;
//  }else{
//    anime_speed = 0;
//  }

#endif

  
  // アニメーション更新
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      int j,frame;
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( wk->g3dUtil, wk->unit_idx );
      int anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      GFL_G3D_OBJECT_GetAnimeFrame( obj, 0, &frame );
//      OS_Printf("anime_frame=%d\n", frame/FX32_ONE);

      for( j=0; j<anime_count; j++ )
      {
//        GFL_G3D_OBJECT_LoopAnimeFrame( obj, j, wk->anime_speed );
        GFL_G3D_OBJECT_SetAnimeFrame( obj, j, &wk->animeFrame );
      }
    }
  }

  // 描画
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( wk->g3dUtil, wk->unit_idx );
      GFL_G3D_DRAW_DrawObject( obj, &status );
    }
  }
  GFL_G3D_DRAW_End();

  frame += anime_speed;
}



// BMPWIN初期化用構造体
typedef struct{
  u32 frame;
  u8  x,y,w,h;
  u16 pal, chr;
}BMPWIN_DAT;


static const BMPWIN_DAT bmpwin_info_dat = {
  BV_BGFRAME_D_WIN, 1, 1, 30, 9, 15, 0,		// 上
};

//----------------------------------------------------------------------------------
/**
 * @brief BMPWIN初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BmpWinInit(BADGEVIEW_WORK *wk)
{
  int i;
  const BMPWIN_DAT *windat;

  GFL_BMPWIN_Init( HEAPID_TR_CARD );
 
  windat = &bmpwin_info_dat;
  wk->InfoWin = GFL_BMPWIN_Create( windat->frame,
                                   windat->x, windat->y,
                                   windat->w, windat->h,
                                   windat->pal, GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin), 0x0f0f );

}

//----------------------------------------------------------------------------------
/**
 * @brief BMPWIN解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void BmpWinExit(BADGEVIEW_WORK *wk)
{
  
  // サブ画面用BMPWINを解放
  GFL_BMPWIN_Delete( wk->InfoWin );

  // BMPWINシステム終了
  GFL_BMPWIN_Exit();

}

//----------------------------------------------------------------------------------
/**
 * @brief bmpwin_visible
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void bmpwin_visible( BADGEVIEW_WORK *wk, const BOOL bflg )
{
	// サブ画面のWIN非表示
	GFL_BG_SetVisible( BV_BGFRAME_D_WIN, bflg );
}

//----------------------------------------------------------------------------------
/**
 * @brief 情報ウインドウの表示ON（一度表示したらOFFしない）
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void InfoWinPut( BADGEVIEW_WORK *wk )
{
  // ウインドウ枠描画
//  BmpWinFrame_Write( wk->InfoWin, 0, SYSTEMWIN_FRM_CGX_OFFSET, SYSTEMWIN_FRM_PAL_OFFSET );
  // 情報ウインドウON
  GFL_BMPWIN_MakeTransWindow( wk->InfoWin );

}


#define BV_COL_BLACK      ( PRINTSYS_LSB_Make(  1,  2, 15) )    // 黒

//----------------------------------------------------------------------------------
/**
 * @brief 文字描画システム初期化
 *

 */
//----------------------------------------------------------------------------------
static void PrintSystemInit(BADGEVIEW_WORK *wk)
{
  int i;
  // フォント確保
  wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr ,
                              GFL_FONT_LOADTYPE_FILE , FALSE , HEAPID_TR_CARD );

  // 描画待機システム初期化
  wk->printQue = PRINTSYS_QUE_Create( HEAPID_TR_CARD );

  // BMPWINとPRINT_UTILを関連付け
  PRINT_UTIL_Setup( &wk->printUtil, wk->InfoWin );
  
}

//----------------------------------------------------------------------------------
/**
 * @brief 文字描画システム解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void PrintSystemDelete(BADGEVIEW_WORK *wk)
{
  
  // メッセージ表示用システム解放
  PRINTSYS_QUE_Delete( wk->printQue );

  // フォント解放
  GFL_FONT_Delete( wk->font );
  
}

//----------------------------------------------------------------------------------
/**
 * @brief 文字描画システムメイン
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void PrintSystem_Main( BADGEVIEW_WORK *wk )
{
  int i;

  // 描画待ち
  PRINTSYS_QUE_Main( wk->printQue );

  // 描画終了済であれば転送
  PRINT_UTIL_Trans( &wk->printUtil, wk->printQue );

}











//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// サブシーケンス
//----------------------------------------------------------------------------------

static BOOL SubSuq_FadeinWait( BADGEVIEW_WORK *wk );
static BOOL SubSuq_Main( BADGEVIEW_WORK *wk );
static BOOL SubSeq_IconWait( BADGEVIEW_WORK *wk );
static BOOL SubSuq_FadeOut( BADGEVIEW_WORK *wk );
static BOOL SubSuq_FadeOutWait( BADGEVIEW_WORK *wk );


static BOOL (*functable[])(BADGEVIEW_WORK *wk) = {
  SubSuq_FadeinWait,      // SUBSEQ_FADEIN_WAIT;
  SubSuq_Main,            // SUBSEQ_MAIN 
  SubSeq_IconWait,        // SUBSEQ_ICON_WAIT,
  SubSuq_FadeOut,         // SUBSEQ_FADEOUT
  SubSuq_FadeOutWait,     // SUBSEQ_FADEOUT_WAIT
};

//----------------------------------------------------------------------------------
/**
 * @brief サブシーケンス制御メイン
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL SEQ_Main( BADGEVIEW_WORK *wk )
{
  // サブシーケンス実行
  return functable[wk->seq](wk);
}


//----------------------------------------------------------------------------------
/**
 * @brief 【SUBSEQ】フェードイン待ち
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL SubSuq_FadeinWait( BADGEVIEW_WORK *wk )
{
  if( WIPE_SYS_EndCheck() ){
    wk->seq = SUBSEQ_MAIN;
  }
  return TRUE;
}


//----------------------------------------------------------------------------------
/**
 * @brief 【SUBSEQ】メイン処理
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL SubSuq_Main( BADGEVIEW_WORK *wk )
{
  u32 ret;
  
  // printQueが空くまでは操作禁止
  if(PRINTSYS_QUE_IsFinished( wk->printQue )==FALSE){
    return TRUE;
  }
  
  // タッチバー処理
  if(TouchBar_KeyControl(wk)==GFL_UI_TP_HIT_NONE){

  }
  Trans_BadgePalette(wk);
  return TRUE;
}


//----------------------------------------------------------------------------------
/**
 * @brief 戻るアイコンのアニメ待ち
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL SubSeq_IconWait( BADGEVIEW_WORK *wk )
{
  switch(wk->next_seq)
  {
  case TOUCH_END:
    if( GFL_CLACT_WK_CheckAnmActive( wk->clwk[BV_OBJ_END] )==FALSE){
      wk->seq = SUBSEQ_FADEOUT;
    } 
  case TOUCH_RETURN:
    if( GFL_CLACT_WK_CheckAnmActive( wk->clwk[BV_OBJ_RETURN] )==FALSE){
      wk->seq = SUBSEQ_FADEOUT;
    } 
  case TOUCH_CHANGE_CARD:
    if( GFL_CLACT_WK_CheckAnmActive( wk->clwk[BV_OBJ_CARD] )==FALSE){
      wk->seq = SUBSEQ_FADEOUT;
    } 
  }
  return TRUE;
}

//----------------------------------------------------------------------------------
/**
 * @brief 【SUBSEQ】フェードアウト
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL SubSuq_FadeOut( BADGEVIEW_WORK *wk )
{
  // フェードイン開始
  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
                  WIPE_FADE_BLACK,  WIPE_DEF_DIV, 1, HEAPID_TR_CARD );
  wk->seq = SUBSEQ_FADEOUT_WAIT;

  return TRUE;
}


//----------------------------------------------------------------------------------
/**
 * @brief 【SUBSEQ】フェードアウト待ち
 *
 * @param   wk    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL SubSuq_FadeOutWait( BADGEVIEW_WORK *wk )
{
  if( WIPE_SYS_EndCheck() ){
    if(wk->next_seq==TOUCH_END){
      wk->param->value     = CALL_NONE;
      wk->param->next_proc = TRAINERCARD_NEXTPROC_EXIT;
      wk->seq = 0;
    }else if(wk->next_seq==TOUCH_RETURN){
      wk->param->value = CALL_NONE;
      wk->param->next_proc = TRAINERCARD_NEXTPROC_RETURN;
      wk->seq = 0;
    }else if(wk->next_seq==TOUCH_CHANGE_CARD){
      wk->param->value = CALL_CARD;
      wk->seq = 0;
    }
    return FALSE;
  }
  return TRUE;
}














//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// インターフェース処理
//----------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
/**
 * セルタクター座標位置セット
 *
 * @param work    
 * @param id    セルアクターインデックス
 * @param x     位置X
 * @param y     位置Y
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CellActorPosSet( BADGEVIEW_WORK *wk, int id, int x, int y )
{
  GFL_CLACTPOS clpos;
  clpos.x = x;
  clpos.y = y;

  GFL_CLACT_WK_SetPos( wk->clwk[id], &clpos, OBJ_DEFREND_DOWN );

}





//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// 機能関数



// ジムリーダーのテキストをWBで対応させるためのテーブル
#if PM_VERSION==VERSION_B2
static const u32 gymleader_text_tbl[]={
  MSG_BADGE_LEADER1_01,
  MSG_BADGE_LEADER2_01,
  MSG_BADGE_LEADER3_01,
  MSG_BADGE_LEADER4_01,
  MSG_BADGE_LEADER5_01,
  MSG_BADGE_LEADER6_01,
  MSG_BADGE_LEADER7_01,
  MSG_BADGE_LEADER8B_01,
};

#else
static const u32 gymleader_text_tbl[]={
  MSG_BADGE_LEADER1_01,
  MSG_BADGE_LEADER2_01,
  MSG_BADGE_LEADER3_01,
  MSG_BADGE_LEADER4_01,
  MSG_BADGE_LEADER5_01,
  MSG_BADGE_LEADER6_01,
  MSG_BADGE_LEADER7_01,
  MSG_BADGE_LEADER8W_01,
};
#endif

//----------------------------------------------------------------------------------
/**
 * @brief ジムリーダー情報を描画する
 *
 * @param   wk    BAGDEVIEW_WORK
 * @param   line  行指定（0 or 1 or 2)
 * @param   ypos  表示の行指定
 * @param   no    ジムインデックス（0-7)
 * @param   strbuf  文字列展開のためのバッファ
 */
//----------------------------------------------------------------------------------
static void _print_gym_leader_info( BADGEVIEW_WORK *wk, int line, const s32 ypos, int no, STRBUF *strbuf )
{
  GFL_MSG_GetString( wk->mman, gymleader_text_tbl[no]+line, strbuf );
  PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, 
                         0, 32*ypos, strbuf, wk->font, BV_COL_BLACK );

}


//----------------------------------------------------------------------------------
/**
 * @brief 情報ウインドウにテキスト書き込み
 *
 * @param   wk    
 * @param   type  INFO_TYPE_BADGE or INFO_TYPE_GYMLEADER
 * @param   no    0-7 (バッジでもジムリーダーでも）
 * @param   smode (0:リーダー1行モード、1:リーダー2行モード)(リーダーのみ有効)
 */
//----------------------------------------------------------------------------------
static void InfoWinPrint( BADGEVIEW_WORK *wk, int type, int no, const s32 smode )
{
  int i;
  STRBUF *strbuf = GFL_STR_CreateBuffer( BADGEVIEW_MSG_LEN, HEAPID_TR_CARD );
  GF_ASSERT( type==INFO_TYPE_BADGE || type==INFO_TYPE_GYMLEADER );
  GF_ASSERT( no>=0 && no<8 );

  // 描画領域クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin), 0x0f0f );

  switch(type){
  // バッジ情報
  case INFO_TYPE_BADGE:   
    {
      // 取得日時
      SHORT_DATE date = TRCSave_GetBadgeDate( wk->trCard, no );
    
      WORDSET_RegisterNumber( wk->wset, 0, date.year+2000,  4, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU);
      WORDSET_RegisterNumber( wk->wset, 1, date.month, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU);
      WORDSET_RegisterNumber( wk->wset, 2, date.day,   2, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU);
    }
    // 描画（１行目）
    GFL_MSG_GetString( wk->mman, MSG_LEAGUE_BADGE1_01+3*no, strbuf );
    WORDSET_ExpandStr( wk->wset, wk->expbuf, strbuf );
    PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, 
                           0, 0, wk->expbuf, wk->font, BV_COL_BLACK );
    
    // 描画（２行目・３行目）
    for(i=1;i<3;i++){
      GFL_MSG_GetString( wk->mman, MSG_LEAGUE_BADGE1_01+3*no+i, strbuf );
      PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, 
                             0, 32*i, strbuf, wk->font, BV_COL_BLACK );
    }
    break;

  // ジムリーダー情報
  case INFO_TYPE_GYMLEADER:
#if 1
	// リーダー情報
    _print_gym_leader_info( wk, 0, 0, no, strbuf );
    // 追加行(メッセージ切り替え)
    _print_gym_leader_info( wk, 1+smode, 1, no, strbuf );
#else
    for(i=0;i<2;i++){
      _print_gym_leader_info( wk, i, i, no, strbuf );
    }
    // 3行目以降はバッジ取得後に表示される
    if(wk->badgeflag[no]){
      _print_gym_leader_info( wk, 2, 2, no, strbuf );
    }
#endif
    break;
  }
  GFL_STR_DeleteBuffer( strbuf );

}

#define GYMREADER_FRAME_MAX ( 256*FX32_ONE)

//----------------------------------------------------------------------------------
/**
 * @brief アニメーフレームをmaxで回り込ませる処理（＋にも－にも）
 *
 * @param   frame   
 * @param   max   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _round_frame( int frame, int max )
{
  int w;
  if(frame >= max){
    w = frame%max;
    frame = w;
  }else if(frame<0){
    w = frame;
    frame = GYMREADER_FRAME_MAX+frame;
  }
  return frame;
}


#define FRAME_SPEED_MIN     ( 4096 )
#define FRAME_SPEED_MINUS   ( 400 )


static void _speed_down( BADGEVIEW_WORK *wk )
{
  // 回転スピードが下がるように計算(でも０にはしない）
  if(wk->animeSpeed>FRAME_SPEED_MIN){
    wk->animeSpeed -= FRAME_SPEED_MINUS;
  }else if(wk->animeSpeed<(-1*FRAME_SPEED_MIN)){
    wk->animeSpeed += FRAME_SPEED_MINUS;
  }else if( wk->animeSpeed == 0 ){
    wk->animeSpeed = FRAME_SPEED_MIN;
  }
  
}

static const int target_frame_tbl[]={
    0*FX32_ONE,  32*FX32_ONE,  64*FX32_ONE,  96*FX32_ONE,
  128*FX32_ONE, 160*FX32_ONE, 192*FX32_ONE, 224*FX32_ONE,
};

//----------------------------------------------------------------------------------
/**
 * @brief ジムリーダーのスクラッチ移動処理
 *
 * @param   wk    
 * @param   flag    
 * @param   x   
 */
//----------------------------------------------------------------------------------
static void SlideFunc( BADGEVIEW_WORK *wk, int trg, int flag, int x )
{
  int i;

  // タッチ開始
  if(wk->slide.old_on==0 && flag){
    wk->old_animeFrame = wk->animeFrame;
    wk->slide.start_x        = x;
  //タッチ継続  
  }else if(wk->slide.old_on==1 && flag==1){
    wk->animeFrame = wk->old_animeFrame + (x - wk->slide.start_x)*1024;
    wk->animeFrame = _round_frame( wk->animeFrame, GYMREADER_FRAME_MAX );
    
  // タッチ終了（慣性取得）
  }else if(wk->slide.old_on==1 && flag==0){
    if(wk->slide.old2_x!=0){
      wk->animeSpeed = (wk->slide.old_x - wk->slide.old2_x)*2048;
    }
    if(wk->animeSpeed > 40000){
      wk->animeSpeed = 40000;
    }else if(wk->animeSpeed<-40000){
      wk->animeSpeed = -40000;
    }
  }

  // タッチしてない
  if(wk->slide.old_on==0 && flag==0){
    // アニメフレーム計算（回り込みつき）
    wk->animeFrame += wk->animeSpeed;
    wk->animeFrame = _round_frame( wk->animeFrame, GYMREADER_FRAME_MAX );

    // 移動先が指定されている時の挙動
    if(wk->targetSet>=0){
      if(wk->targetFrame > (wk->animeFrame-50000) && wk->targetFrame < (wk->animeFrame+50000)){
        wk->animeSpeed = 0;
        wk->animeFrame = wk->targetFrame;
        wk->targetSet = -1;
      }
    // 移動先は指定されていない
    }else{
      _speed_down(wk);

      // 移動スピードが下がってきたときに一番近いリーダーで止まる
      for(i=0;i<BADGE_NUM;i++){
        if(target_frame_tbl[i] > (wk->animeFrame-10000) && target_frame_tbl[i] < (wk->animeFrame+10000)){
          if(MATH_IAbs(wk->animeSpeed) < 10000){
            wk->animeFrame = target_frame_tbl[i];
            wk->animeSpeed = 0;
          }
        }
      }
    }
  }

//  OS_Printf(" flag=%d, x=%d, old_x=%d, old2_x=%d, Frame=%d, speed=%d startx=%d\n", flag, x, wk->slide.old_x, wk->slide.old2_x,wk->animeFrame, wk->animeSpeed, wk->slide.start_x);
  
  wk->slide.old2_x  = wk->slide.old_x;
  wk->slide.old_x  = x;
  wk->slide.old_on = flag;

  

}

// -----------------------------------------
// タッチ座標定義
// -----------------------------------------
#define BADGE_TP_X    (  0 )
#define BADGE_TP_Y    ( 88 )
#define BADGE_TP_Y2   (  0 )	// ジムリーダー用
#define BADGE_TP_W    ( 32 )
#define BADGE_TP_H    ( 80 )
#define GR0_SX   (  95 )
#define GR0_SY   (  33 )
#define GR0_EX   ( 159 )
#define GR0_EY   (  87 )
#define GR1_SX   (  40 )
#define GR1_SY   (  23 )
#define GR1_EX   (  88 )
#define GR1_EY   (  71 )
#define GR2_SX   (  32 )
#define GR2_SY   (  16 )
#define GR2_EX   (  80 )
#define GR2_EY   (  23 )
#define GR3_SX   (  80 )
#define GR3_SY   (   0 )
#define GR3_EX   ( 103 )
#define GR3_EY   (  23 )
#define GR4_SX   ( 112 )
#define GR4_SY   (   0 )
#define GR4_EX   ( 143 )
#define GR4_EY   (  31 )
#define GR5_SX   ( 152 )
#define GR5_SY   (   0 )
#define GR5_EX   ( 176 )
#define GR5_EY   (  22 )
#define GR6_SX   ( 176 )
#define GR6_SY   (  15 )
#define GR6_EX   ( 223 )
#define GR6_EY   (  23 )
#define GR7_SX   ( 168 )
#define GR7_SY   (  23 )
#define GR7_EX   ( 215 )
#define GR7_EY   (  71 )

static const GFL_UI_TP_HITTBL TouchButtonHitTbl[] =
{
  { BADGE_TP_Y, BADGE_TP_Y+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*0,  BADGE_TP_X+BADGE_TP_W*1 },   // バッジ０
  { BADGE_TP_Y, BADGE_TP_Y+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*1,  BADGE_TP_X+BADGE_TP_W*2 },   // 
  { BADGE_TP_Y, BADGE_TP_Y+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*2,  BADGE_TP_X+BADGE_TP_W*3 },   // 
  { BADGE_TP_Y, BADGE_TP_Y+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*3,  BADGE_TP_X+BADGE_TP_W*4 },   // 
  { BADGE_TP_Y, BADGE_TP_Y+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*4,  BADGE_TP_X+BADGE_TP_W*5 },   // 
  { BADGE_TP_Y, BADGE_TP_Y+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*5,  BADGE_TP_X+BADGE_TP_W*6 },   // 
  { BADGE_TP_Y, BADGE_TP_Y+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*6,  BADGE_TP_X+BADGE_TP_W*7 },   // 
  { BADGE_TP_Y, BADGE_TP_Y+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*7,  BADGE_TP_X+BADGE_TP_W*8 },   // 
#if 1	// 領域は要調整
  { BADGE_TP_Y2, BADGE_TP_Y2+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*0,  BADGE_TP_X+BADGE_TP_W*1 },   // ジムリーダー
  { BADGE_TP_Y2, BADGE_TP_Y2+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*1,  BADGE_TP_X+BADGE_TP_W*2 },   // 
  { BADGE_TP_Y2, BADGE_TP_Y2+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*2,  BADGE_TP_X+BADGE_TP_W*3 },   // 
  { BADGE_TP_Y2, BADGE_TP_Y2+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*3,  BADGE_TP_X+BADGE_TP_W*4 },   // 
  { BADGE_TP_Y2, BADGE_TP_Y2+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*4,  BADGE_TP_X+BADGE_TP_W*5 },   // 
  { BADGE_TP_Y2, BADGE_TP_Y2+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*5,  BADGE_TP_X+BADGE_TP_W*6 },   // 
  { BADGE_TP_Y2, BADGE_TP_Y2+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*6,  BADGE_TP_X+BADGE_TP_W*7 },   // 
  { BADGE_TP_Y2, BADGE_TP_Y2+BADGE_TP_H,   BADGE_TP_X+BADGE_TP_W*7,  BADGE_TP_X+BADGE_TP_W*8 },   // 
#else
  { GR0_SY, GR0_EY, GR0_SX, GR0_EX },   // ジムリーダー
  { GR1_SY, GR1_EY, GR1_SX, GR1_EX },   // 
  { GR2_SY, GR2_EY, GR2_SX, GR2_EX },   // 
  { GR3_SY, GR3_EY, GR3_SX, GR3_EX },   // 
  { GR4_SY, GR4_EY, GR4_SX, GR4_EX },   // 
  { GR5_SY, GR5_EY, GR5_SX, GR5_EX },   // 
  { GR6_SY, GR6_EY, GR6_SX, GR6_EX },   // 
  { GR7_SY, GR7_EY, GR7_SX, GR7_EX },   // 
#endif
  { 168, 191,    8,  8+32  },   // TOUCH_CHANGE_CARD
  { 168, 191, 20*8, 24*8-1 },   // TOUCH_SHORTCUT
  { 168, 191, 24*8, 28*8-1 },   // TOUCH_END
  { 168, 191, 28*8, 32*8-1 },   // TOUCH_RETURN
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};

// ジムリーダーパネルをドラッグできる領域チェック用
static const GFL_UI_TP_HITTBL SlideTbl[]={
  { 32, 87, 40, 216, },
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};

// ウインドウ領域 => タッチするとトップ画面へ移行
static const GFL_UI_TP_HITTBL leader_hit_tbl[] = {
  { BADGE_TP_Y, BADGE_TP_Y+BADGE_TP_H,   BADGE_TP_X,  BADGE_TP_X+BADGE_TP_W*8-1 },   // バッジ領域
  //
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};

//=============================================================================================
/**
 * @brief タッチバー制御
 *
 * @param   wk    
 *
 * @retval  staitc BOOL   
 */
//=============================================================================================
static int TouchBar_KeyControl( BADGEVIEW_WORK *wk )
{
  int  flg=0;	// 追加
  int  trg, trg2, cont,cont2;
  u32  tp_x=0, tp_y=0;
  BOOL keyflag=FALSE;
  trg    = GFL_UI_TP_HitTrg( TouchButtonHitTbl );   // タッチトリガー情報
  cont2  = GFL_UI_TP_HitCont( TouchButtonHitTbl ); // タッチトリガー情報
  trg2   = GFL_UI_TP_HitTrg( SlideTbl );          // タッチ情報
  cont   = GFL_UI_TP_HitCont( SlideTbl );         // タッチ情報
  GFL_UI_TP_GetPointCont( &tp_x, &tp_y );         // タッチ座標
  
  // 処理を追加
  bv_main_loop( wk );
  
  // スクロール中 => 入力禁止
  flg = 0;
  if( bv_is_bgscroll( wk ) == TRUE ){
    flg = 1;
    // スクロール中はtrgを無効(初期化)
    trg  = GFL_UI_TP_HIT_NONE;
    trg2 = GFL_UI_TP_HIT_NONE;
  }
  
  // キー処理
  if(flg == 0){
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
      // キャンセル
      if(!(wk->Flag & FLG_CANCEL_OFF)){
        PMSND_PlaySE( SEQ_SE_CANCEL1 );
        keyflag = TRUE;
        trg = TOUCH_RETURN;
      }
    }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X){
      // Xボタン
      if(!(wk->Flag & FLG_XBUTTON_OFF)){
        PMSND_PlaySE( SEQ_SE_CANCEL1 );
        trg = TOUCH_END;
      }
    }else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y){
      // Yボタン
      if(!(wk->Flag & FLG_YBUTTON_OFF)){
        trg = TOUCH_BOOKMARK;
      }
    }else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
      // 決定ボタン
      if(!(wk->Flag & FLG_DECIDE_OFF)){
        trg = TOUCH_CHANGE_CARD;
      }
    }
  }
  
  // 左ページ・右ページ・戻る機能の呼び出し
  ExecFunc(wk, trg, keyflag);

  // ジムリーダーをスライドさせる処理
  SlideFunc( wk, trg2, cont+1, tp_x );

  // バッジを磨く処理(保留)
//  BrushBadge( wk, cont2 );
  
  return trg;
}

//----------------------------------------------------------------------------------
/**
 * @brief どのジムリーダーが一番まえにいるか？
 *
 * @param   frame   
 *
 * @retval  int   0-7
 */
//----------------------------------------------------------------------------------
static int _get_front_gymreader( int frame )
{
  int i;
  for(i=0;i<BADGE_NUM;i++){
    if(target_frame_tbl[i] > (frame-10000) && target_frame_tbl[i] < (frame+10000)){
      break;
    }
  }
  if(i==8){
    i = 0;
  }
  
  return i;
}


//----------------------------------------------------------------------------------
/**
 * @brief 各機能（左ページ・右ページ・戻る）の実行
 *
 * @param   wk    
 * @param   trg 
 */
//----------------------------------------------------------------------------------
static void ExecFunc( BADGEVIEW_WORK *wk, int trg, BOOL keyflag )
{
  s32	num=0;
  s32	flg=0;
  
  if(trg==GFL_UI_TP_HIT_NONE){
    return;
  }
  
//  OS_Printf("trg=%d\n",trg);
  switch(trg){
  case TOUCH_BADGE_0: // バッジにタッチ
  case TOUCH_BADGE_1:
  case TOUCH_BADGE_2:
  case TOUCH_BADGE_3:
  case TOUCH_BADGE_4:
  case TOUCH_BADGE_5:
  case TOUCH_BADGE_6:
  case TOUCH_BADGE_7:
    if(wk->badgeflag[trg-TOUCH_BADGE_0]){
      // 通常とバッジの時のみ
      if((wk->Mode == MODE_NORMAL)||(wk->Mode == MODE_BADGE)){
        // 追加
        flg = 0;
        if(wk->Mode != MODE_BADGE){
          flg=1;
        }
        // 初めてのときはスクロール
        if(flg == 1){
          bv_init_bgscroll( wk, MODE_UPDOWN );
          bv_set_cutin_visible( wk, TRUE );
        }
        bmpwin_visible( wk, TRUE );
//        PMSND_PlaySE( SEQ_SE_SYS_32 );
        // ページ番号を保存
        wk->PageNum = trg-TOUCH_BADGE_0;
        wk->TypeNum = INFO_TYPE_BADGE;
        // 続きのときだけ
        if(flg == 0){
          InfoWinPrint( wk, INFO_TYPE_BADGE, trg-TOUCH_BADGE_0, 0 );
          InfoWinPut( wk );
        }
        wk->targetSet   = TOUCH_BADGE_0;   // ジムリーダー移動先指定
        wk->targetFrame = target_frame_tbl[trg-TOUCH_BADGE_0];
        wk->animeSpeed  = 40000;
        //
        // カットインキャラの切り替え
        bv_chg_cutin( wk, trg-TOUCH_BADGE_0 );
        // 音階
        bv_snd_badge( wk, wk->PageNum );
        // バッジ
        wk->Mode = MODE_BADGE;
      }
    }
    break;
  case TOUCH_GYM_READER_0:  // ジムリーダーにタッチ
  case TOUCH_GYM_READER_1:
  case TOUCH_GYM_READER_2:
  case TOUCH_GYM_READER_3:
  case TOUCH_GYM_READER_4:
  case TOUCH_GYM_READER_5:
  case TOUCH_GYM_READER_6:
  case TOUCH_GYM_READER_7:
    // 通常とジムリーダーの時のみ
    if((wk->Mode == MODE_NORMAL)||(wk->Mode == MODE_LEADER)){
      //回転が終わるまでタッチできなくする
      if( wk->animeSpeed == 0 )
      {
        int pos = _get_front_gymreader(wk->animeFrame);
        PMSND_PlaySE( SEQ_SE_SYS_31 );
        // 追加
        flg = 0;
        if(wk->Mode != MODE_LEADER){
          flg = 1;
        }
        // 初めてのときだけスクロール
        if(flg == 1){
          bv_init_bgscroll( wk, MODE_DOWNUP );
            bv_set_cutin_visible( wk, TRUE );
        }
        bmpwin_visible( wk, TRUE );
        //
        // ページ番号を保存
        wk->PageNum = (trg-TOUCH_GYM_READER_0+pos)%BADGE_NUM;
        wk->TypeNum = INFO_TYPE_GYMLEADER;
        // 続きのときだけ
        if(flg == 0){
          InfoWinPrint( wk, INFO_TYPE_GYMLEADER, (trg-TOUCH_GYM_READER_0+pos)%BADGE_NUM, 0 );
          InfoWinPut( wk );
        }
        // リーダー情報初期化
        wk->LeaderNum = 0;
        bv_init_timer( wk, TIMER_MES, bv_timer_tbl[wk->LeaderNum]);
        //
        // カットインキャラの切り替え
        bv_chg_cutin( wk, trg-TOUCH_GYM_READER_0 );
        //
        // ジムリーダー
        wk->Mode = MODE_LEADER;
      }
    }
    break;
  case TOUCH_CHANGE_CARD:   // カード画面に移動
    if(!(wk->Flag & FLG_DECIDE_OFF)){
      wk->seq      = SUBSEQ_ICON_WAIT;
      wk->next_seq = trg;
      PMSND_PlaySE( SND_TRCARD_DECIDE );
      GFL_CLACT_WK_SetAnmSeq( wk->clwk[BV_OBJ_CARD], 12 );
    }
    break;
  case TOUCH_BOOKMARK:
    if(!(wk->Flag & FLG_YBUTTON_OFF)){
      CheckBookmark(wk);
    }
    break;
  case TOUCH_END:           // トレーナーカード終了→直接フィールドへ
    if(!(wk->Flag & FLG_XBUTTON_OFF)){
      PMSND_PlaySE( SND_TRCARD_END );
      wk->seq = SUBSEQ_ICON_WAIT;
      wk->next_seq = trg;
      GFL_CLACT_WK_SetAnmSeq( wk->clwk[BV_OBJ_END], 8 );
    }
    break;
  case TOUCH_RETURN:        // トレーナーカード終了→メニューへ
    // 通常=>終了
    if(wk->Mode == MODE_NORMAL){
      PMSND_PlaySE( SND_TRCARD_CANCEL );
      wk->seq = SUBSEQ_ICON_WAIT;
      wk->next_seq = trg;
      GFL_CLACT_WK_SetAnmSeq( wk->clwk[BV_OBJ_RETURN], 9 );
      if(keyflag){
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      }else{
        GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
      }
    }
    else{
      // トップに戻る
      if(!(wk->Flag & FLG_CANCEL_OFF)){
        PMSND_PlaySE( SND_TRCARD_CANCEL );
        GFL_CLACT_WK_SetAnmSeq( wk->clwk[BV_OBJ_RETURN], 9 );
        // BGのスクロール処理
        num = MODE_UPDOWN_REV;
        if(wk->Mode == MODE_LEADER){
          num = MODE_DOWNUP_REV;
        }
        bv_init_bgscroll( wk, num );
        // ウインドウを消す
        wk->Mode = MODE_NORMAL;
      }
    }
    break;
  }

}

//----------------------------------------------------------------------------------
/**
 * @brief Yボタンメニュー登録・解除
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void CheckBookmark( BADGEVIEW_WORK *wk )
{
  int flag = GAMEDATA_GetShortCut( wk->param->gameData, SHORTCUT_ID_TRCARD_BADGE );
  flag ^= 1;
  GFL_CLACT_WK_SetAnmSeq( wk->clwk[BV_OBJ_BOOKMARK], 6+flag );
  GAMEDATA_SetShortCut( wk->param->gameData, SHORTCUT_ID_TRCARD_BADGE, flag );
  PMSND_PlaySE( SND_TRCARD_BOOKMARK );
}



//----------------------------------------------------------------------------------
/**
 * @brief ページ総数算出
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void _page_max_init( BADGEVIEW_WORK *wk )
{
  wk->page = 0;
  wk->page_max = wk->trainer_num /BV_1PAGE_NUM;
  if(wk->trainer_num%BV_1PAGE_NUM){
    wk->page_max++;
  }
  OS_Printf("page=%d, trainer_num=%d, max=%d\n", wk->page, wk->trainer_num, wk->page_max);
}

#define BADGE_EFFECT_NUM  ( 3 )

//----------------------------------------------------------------------------------
/**
 * @brief バッジのキラキラの表示・非表示設定
 *
 * @param   wk    
 * @param   badge   
 * @param   enable    
 */
//----------------------------------------------------------------------------------
static void _badge_effect_enable( BADGEVIEW_WORK *wk, int badge, BOOL enable )
{
  int i;
  for(i=0;i<BADGE_EFFECT_NUM;i++){
    if(enable){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BV_OBJ_KIRAKIRA0_0+badge*BADGE_EFFECT_NUM+i], enable );
    }else{
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BV_OBJ_KIRAKIRA0_0+badge*BADGE_EFFECT_NUM+i], enable );
    }
  }
  
}

#define ISSHU_BADGE_NUM   ( 8 )

static int test_badge=0;

//----------------------------------------------------------------------------------
/**
 * @brief   バッジの表示・非常時設定
 *
 *
 * @param   wk    
 * @param   page  
 * @param   max   
 */
//----------------------------------------------------------------------------------
static void _page_clact_set( BADGEVIEW_WORK *wk, int page, int max )
{
  int i;
  MISC *misc = GAMEDATA_GetMiscWork( wk->param->gameData );

#ifdef PM_DEBUG
  // Lボタンをおしているとひとつずつバッジフラグが立って行く
  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L){
    MISC_SetBadgeFlag(misc,test_badge);
    test_badge = (test_badge+1)%8;
  }
#endif

  for(i=0;i<ISSHU_BADGE_NUM;i++){
    if(  MISC_GetBadgeFlag(misc,i)){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BV_OBJ_BADGE0+i],  TRUE );
      _badge_effect_enable( wk, i, TRUE );
      wk->badgeflag[i] = 1;
    }else{
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BV_OBJ_BADGE0+i],  FALSE );
      _badge_effect_enable( wk, i, FALSE );
      wk->badgeflag[i] = 0;
    }
  }
  // Yボタンメニューの登録状態を反映
  {
    int flag = GAMEDATA_GetShortCut( wk->param->gameData, SHORTCUT_ID_TRCARD_BADGE );
    GFL_CLACT_WK_SetAnmSeq( wk->clwk[BV_OBJ_BOOKMARK], 6+flag );
  }

}

//----------------------------------------------------------------------------------
/**
 * @brief 移動可能かチェック
 *
 * @param   wk    
 * @param   touch   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _page_move_check( BADGEVIEW_WORK *wk, int touch )
{
  int result=FALSE;

  // 左移動の時
  if(touch==TOUCH_BADGE_0){
    if( wk->page>0 ){   // 0ページでなければ－１
      wk->page--;
      result=TRUE;
    }

  // 右移動の時
  }else if(touch==TOUCH_BADGE_1){
    if(wk->page < wk->page_max-1){  // MAXページよりも小さければ＋１
      wk->page++;
      result=TRUE;
    }
  }

  return result;
}


//----------------------------------------------------------------------------------
/**
 * @brief 初期ページ表示
 *
 * @param   wk    
 * @param   page    
 */
//----------------------------------------------------------------------------------
static void SetupPage( BADGEVIEW_WORK *wk, int page )
{
  wk->page = page;

  // タッチバー表示設定
  _page_clact_set( wk, page, wk->page_max );

  // バッジ＆ジムリーダーパレット転送
  Trans_BadgePalette( wk );

  // 初期転送し非表示
  InfoWinPut( wk );
  bmpwin_visible( wk, FALSE );
  
  // カットイン非表示
  bv_set_cutin_visible( wk, FALSE );
  
  // 顔グラフィックチェック
  bv_chk_facepal( wk );
}
//----------------------------------------------------------------------------------
/**
 * @brief   表示するトレーナー数を決める
 *
 * @param   page          現在のページ数
 * @param   max           最大ページ数
 * @param   trainer_num   トレーナー全員の数
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _get_print_num( int page, int max, int trainer_num )
{

  int num;
  num = trainer_num-page*BV_1PAGE_NUM;

  if(num > BV_1PAGE_NUM){
    num = BV_1PAGE_NUM;
  }

  OS_Printf("1ページ表示総数=%d\n", num);
  return num;
}




//----------------------------------------------------------------------------------
/**
 * @brief バッジ画面を見た日付を保存
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void SetRtcData( BADGEVIEW_WORK *wk )
{
  RTCDate date;
  RTCTime time;
  s64     digit;
  int     i;

  // 日付・時間取得
  GFL_RTC_GetDateTime( &date, &time);

  digit = RTC_ConvertDateTimeToSecond( &date, &time );

  // 最終バッジ確認時間をセーブデータにセット
  TRCSave_SetLastTime( wk->trCard, digit );

  // 磨きデータもセット
  for(i=0;i<BADGE_NUM;i++){
    TRCSave_SetBadgePolish( wk->trCard, i, wk->badge.polish[i] );
  }
}



//----------------------------------------------------------------------------------
/**
 * @brief バッジの磨きデータを更新する
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void RefreshPolishData( BADGEVIEW_WORK *wk )
{
  RTCDate now_date, last_date;
  RTCTime now_time, last_time;
  s64 digit = TRCSave_GetLastTime( wk->trCard );
  s64 now_digit;
  
  RTC_ConvertSecondToDateTime( &last_date , &last_time, digit );

  OS_Printf("最後の閲覧：%02d/%02d/%02d %02d:%02d.%02d\n", last_date.year, last_date.month,  last_date.day, 
                                                           last_time.hour, last_time.minute, last_time.second);

  // 日付・時間取得
  GFL_RTC_GetDateTime( &now_date, &now_time);

  OS_Printf("今：%02d/%02d/%02d %02d:%02d.%02d\n", now_date.year, now_date.month,  now_date.day, 
                                                   now_time.hour, now_time.minute, now_time.second);

  // 経過時間取得
  now_digit  = RTC_ConvertDateTimeToSecond( &now_date, &now_time );

  OS_Printf("今-過去 %ld秒\n", now_digit-digit);
  
  now_digit -= digit;   // 差分取得
  now_digit /= 60*60;   // 時間に変換
  if(now_digit<0){      // マイナス値にはしないようにする
    now_digit = 0;
  }

  // 磨きデータの取得・時間の差分から更新( 2日で１段階落ちる）
  {
    int i;
    for(i=0;i<BADGE_NUM;i++){
      wk->badge.polish[i] = TRCSave_GetBadgePolish( wk->trCard, i );

// BTS7747:バッジがくすむスピードが２日ではなく0.75日になってしまっている（２日で３下がる）
#ifdef BUGFIX_BTS7747_20100712
      wk->badge.polish[i] -= now_digit*2;
#else
      wk->badge.polish[i] -= now_digit*3;
#endif
      if(wk->badge.polish[i]<0){
        wk->badge.polish[i] = 0;
      }
    }

    // 開いた時に必ず反映されるようにする
    for(i=0;i<BADGE_NUM;i++){
      wk->badge.old_grade[i] = -1;
    }

  }
  

}

//----------------------------------------------------------------------------------
/**
 * @brief バッジの磨き情報でパレットがくすむ
 *
 * @param   wk    
 * @param   no    
 * @param   grade   
 */
//----------------------------------------------------------------------------------
static void _trans_badge_pal( BADGEVIEW_WORK *wk, int no, int grade )
{
  int index;
  NNSG2dPaletteData *pltt;
  u16 *buf;

  if(grade>2){
    grade=2;
  }
  
  pltt  = wk->badgePal[grade];
  buf   = (u16*)pltt->pRawData;
  index = no*16;
  
  GX_LoadOBJPltt( &buf[index], 32*no, 32);
  
}


// ジムリーダーのパレットテーブルは0-7行の他にもう１行追加してある。
// ８番目のジムリーダーB/Wで違うトレーナーになるので、
// BLACKバージョンの場合は９列のパレットを適用する。
static const pal_leader_table[] = {
#if PM_VERSION == VERSION_B2
  0,1,2,3,4,5,6,8,
#else
  0,1,2,3,4,5,6,7,
#endif

};


//----------------------------------------------------------------------------------
/**
 * @brief ジムリーダーの顔のパレットを書き換える
 *
 * @param   wk    
 * @param   no    
 * @param   grade   
 */
//----------------------------------------------------------------------------------
static void _trans_gymleader_pal( BADGEVIEW_WORK *wk, int no, int grade )
{
  const GFL_G3D_RES  *mdl_res;
  NNSG3dResTex *tex;
  NNSG3dResDictPlttData *texpal;
  u16 *pal;
  
  // 2以上のグレードは2にする
  if( grade > 2 ){
    grade = 2;
  }
  pal = wk->leaderPal[grade]->pRawData; //パレットデータ取得
  
  // モデルリソース取得
  mdl_res = GFL_G3D_UTIL_GetResHandle( wk->g3dUtil, 0 );
  // テクスチャリソース取得
  tex     = GFL_G3D_GetResTex( mdl_res );       
  // テクスチャからパレット位置取得
  texpal  = NNS_G3dGetPlttDataByIdx( tex, no );

  // パレット転送(転送位置はテクスチャパレット＋オフセットで確定させる）
  NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT, 
                                      tex->plttInfo.vramKey+(texpal->offset<<3), 
                                      &pal[pal_leader_table[no]*16], 32);

}

//----------------------------------------------------------------------------------
/**
 * @brief バッジの磨き情報に合わせてキラキラが出現する
 *
 * @param   wk    
 * @param   no    
 * @param   grade 
 */
//----------------------------------------------------------------------------------
static void _set_clact_visible( BADGEVIEW_WORK *wk, int no, int grade )
{
  int num = grade-BADGE_POLISH_PAL_NUM;
  int i;
  
  for(i=0;i<BADGE_KIRAKIRA_NUM;i++){
    if(grade<3){
      GFL_CLACT_WK_SetDrawEnable( 
        wk->clwk[BV_OBJ_KIRAKIRA0_0+no*BADGE_KIRAKIRA_NUM+i],  FALSE );
    }else{
      if( i<=num ){
        GFL_CLACT_WK_SetDrawEnable( 
          wk->clwk[BV_OBJ_KIRAKIRA0_0+no*BADGE_KIRAKIRA_NUM+i],  TRUE );
      }else{
        GFL_CLACT_WK_SetDrawEnable( 
          wk->clwk[BV_OBJ_KIRAKIRA0_0+no*BADGE_KIRAKIRA_NUM+i],  FALSE );
      }
    }
  }
}



//----------------------------------------------------------------------------------
/**
 * @brief バッジの磨き状態でパレット転送とOBJ表示制御を行う
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void Trans_BadgePalette( BADGEVIEW_WORK *wk )
{
  int i;
  for(i=0;i<BADGE_NUM;i++){
#ifdef PM_DEBUG
    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_START){
      if(wk->badge.polish[i]!=0){
        wk->badge.polish[i]--;
      }
    }
#endif
    // 磨き値からグレード算出
    wk->badge.grade[i] = wk->badge.polish[i]/BADGE_POLISH_RATE;
    if(wk->badge.grade[i]>5){
      wk->badge.grade[i] = 5;
    }
    
    // 変更が掛かっていた場合はパレット転送キラキラOBJ表示制御を行う
    if(wk->badge.grade[i]!=wk->badge.old_grade[i]){
      _trans_badge_pal( wk, i, wk->badge.grade[i] );
      _trans_gymleader_pal( wk, i, wk->badge.grade[i] );
      _set_clact_visible( wk, i, wk->badge.grade[i] );
    }
    wk->badge.old_grade[i] = wk->badge.grade[i];
  }
#ifdef PM_DEBUG
  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_START){
    OS_Printf("badge:");
    for(i=0;i<BADGE_NUM;i++){
      OS_Printf("%02d,",wk->badge.polish[i]);
    }
    OS_Printf("\n,");
  }
#endif
}


#define MIN_SCRUCH  (3)
#define MAX_SCRUCH  (40)
#define BADGE_SE_CHANGE_LINE  (BADGE_POLISH_RATE*5+BADGE_POLISH_RATE/2)
#define REV_SPEED (FX32_SHIFT - wk->RevSpeed)
#define SCRATCH_WAIT  ( 5 )

//----------------------------------------------------------------------------------
/**
 * @brief 磨いたSEを鳴らす（磨き度合に合わせて変わる）
 *
 * @param   sc    
 * @param   polish    
 */
//----------------------------------------------------------------------------------
static void _play_scratch_se( SCRATCH_WORK *sc, int polish )
{
  if(sc->wait==0){
    if(polish > BADGE_SE_CHANGE_LINE){
      PMSND_PlaySE( SEQ_SE_SYS_34 );
    }else{
      PMSND_PlaySE( SEQ_SE_SYS_33 );
    }
    sc->wait = 5;
  }else{
    sc->wait--;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * バッジこすり音制御するための現在方向をクリア
 *
 * @param *outScruchSnd バッジこすり音構造体へのポインタ
 *
 * @return  none  
 */
//--------------------------------------------------------------------------------------------
static void ClearScratchSndNow( SCRATCH_WORK *sc )
{
  sc->DirX = 0;
  sc->DirY = 0;
}

//--------------------------------------------------------------------------------------------
/**
 * バッジこすり音制御するためのデータをクリア
 *
 * @param *outScruchSnd バッジこすり音構造体へのポインタ
 *
 * @return  none  
 */
//--------------------------------------------------------------------------------------------
static void ClearScratchSnd( SCRATCH_WORK *sc )
{
  sc->OldDirX = 0;
  sc->OldDirY = 0;
  sc->DirX = 0;
  sc->DirY = 0;
  sc->Snd = 0;
}


static void _badge_polish_up( BADGEVIEW_WORK *wk, int no )
{
  if(wk->badge.polish[no]<(BADGE_POLISH_RATE*6-1)){
    wk->badge.polish[no]++;
  }
  
}

//--------------------------------------------------------------------------------------------
/**
 * バッジ磨き
 *
 * @param wk    画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BrushBadge( BADGEVIEW_WORK *wk, u32 touch )
{
  BOOL scruch=FALSE;
  int sub;
  u32 tp_x = 0xffff;
  u32 tp_y = 0xffff;
  
  GFL_UI_TP_GetPointCont( &tp_x, &tp_y );         // タッチ座標


  //保持座標とタッチ座標の差分を取る
  if( (tp_x!=0xffff)&&(tp_y!=0xffff)&&
      (wk->scratch.BeforeX!=0xffff)&&(wk->scratch.BeforeY!=0xffff) ){ //値有効か？
    //保持座標とタッチ座標が同じバッジ何にあるかを調べる
    if ( (touch!=GFL_UI_TP_HIT_NONE)&&(touch<BADGE_NUM)){
      //バッジ所有判定
      if (wk->badgeflag[touch]){
        //差分が規定値以上の場合は磨いたことにする
        if ( wk->scratch.BeforeX > tp_x ){    //前回のほうが値が大きいか？
          sub = wk->scratch.BeforeX - tp_x;
          wk->scratch.DirX = -1;
        }else{
          sub = tp_x - wk->scratch.BeforeX;
          wk->scratch.DirX = 1;
        }
        if ( (sub>=MIN_SCRUCH)&&(sub<=MAX_SCRUCH) ){
          if ( wk->scratch.BeforeY > tp_y ){  //前回のほうが値が大きいか？
            sub = wk->scratch.BeforeY - tp_y;
            wk->scratch.DirY = -1;
          }else{
            sub = tp_y - wk->scratch.BeforeY;
            wk->scratch.DirY = 1;
          }
          if (sub<=MAX_SCRUCH){
            scruch = TRUE;      //こすった
            _play_scratch_se( &wk->scratch, wk->badge.polish[touch] );
          }else{
            ClearScratchSndNow(&wk->scratch);
          }
        }else if (sub<=MAX_SCRUCH){
          if ( wk->scratch.BeforeY > tp_y ){    //前回のほうが値が大きいか？
            sub = wk->scratch.BeforeY - tp_y;
            wk->scratch.DirY = -1;
          }else{
            sub = tp_y - wk->scratch.BeforeY;
            wk->scratch.DirY = 1;
          }
          if ((sub>=MIN_SCRUCH)&&(sub<=MAX_SCRUCH)){
            scruch = TRUE;    //こすった
            _play_scratch_se( &wk->scratch, wk->badge.polish[touch] );
          }else{
            ClearScratchSndNow(&wk->scratch);
          }
        }

        if (scruch){
          OS_Printf("scruch_%d\n",touch);
          OS_Printf("sc=%d,%d-%d,%d\n",wk->scratch.BeforeX,wk->scratch.BeforeY,tp_x,tp_y);
          //磨いた時間更新
          //レベル更新
          _badge_polish_up(wk, touch);
        }
      }
    }
  }
    //保持座標更新
  wk->scratch.BeforeX = tp_x;
  wk->scratch.BeforeY = tp_y;
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * 移動初期化
 *
 * @param wk    画面のワーク
 * @param	const s32			ワーク番号
 * @param	const s16			始点、終点
 * @param	const s16			フレーム数
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bv_init_movework( BADGEVIEW_WORK *wk, const s32 wnum, const s16 sx, const s16 ex, const s16 frame )
{
	// 念のため
	if(frame == 0){
		OS_Printf("frame error\n");
		return;
	}
	wk->MWork[wnum].sx		= sx;
	wk->MWork[wnum].ex		= ex;
	//
	wk->MWork[wnum].rate	= (ex-sx)*100/frame;
	wk->MWork[wnum].calcnum = sx*100;
	wk->MWork[wnum].flag	= FLG_MOVE_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * 移動処理
 *
 * @param wk    画面のワーク
 * @param s32	ワーク番号
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static s16 bv_main_movework( BADGEVIEW_WORK *wk, const s32 wnum )
{
	s16		flg;
	s16		tmp;
	s16		rate=0;
	
	// 移動中
	if(wk->MWork[wnum].flag & FLG_MOVE_INIT){
		if(!(wk->MWork[wnum].flag & FLG_MOVE_END)){
			// 移動
			wk->MWork[wnum].calcnum += wk->MWork[wnum].rate;
			// 1/100にして判定
			tmp = wk->MWork[wnum].calcnum/100;
			// 終了判定
			flg = 0;
			if(wk->MWork[wnum].rate < 0){
				flg = 1;
			}
			if(flg == 0){
				// 加算
				if(tmp >= wk->MWork[wnum].ex){
					rate = wk->MWork[wnum].ex;
					wk->MWork[wnum].flag |= FLG_MOVE_END;
				}
			}else{
				// 減算
				if(tmp <= wk->MWork[wnum].ex){
					rate = wk->MWork[wnum].ex;
					wk->MWork[wnum].flag |= FLG_MOVE_END;
				}
			}
			rate = tmp;
			
		}else{
			rate = wk->MWork[wnum].ex;
		}
	}
	return( rate );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  移動判定
 *
 *	@param	wk
 *	@param	const s32			ワーク番号
 *
 *	@retval BOOL 				(移動中:TRUE、停止、未使用:FALSE)
 */
//-----------------------------------------------------------------------------
static BOOL bv_is_movework( BADGEVIEW_WORK *wk, const s32 wnum )
{
	BOOL	bflg=FALSE;
	//
	// 移動中
	if(wk->MWork[wnum].flag & FLG_MOVE_INIT){
		if(!(wk->MWork[wnum].flag & FLG_MOVE_END)){
			bflg = TRUE;
		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理まとめ
 *
 *	@param	wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bv_main_loop( BADGEVIEW_WORK* wk )
{
	// リーダー関連(タイマ処理)など
	bv_chk_leader( wk );
	
	// スクロール関連
	bv_main_scroll( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG移動初期化(調整)
 *
 *	@param	wk
 *	@param	const s32 			(0:上から下、1:下から上)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
// 移動のテーブル(開始、終了、step、spd)
static const s16 bv_scr_tbl[] = {
    128,   0, STEP_LV1, SPD_WIN_IN,		// 上から下
   -208, -80, STEP_LV1, SPD_WIN_IN,		// 下から上
      0, 128, STEP_LV2, SPD_WIN_OUT,	// 上から下(戻)
    -80,-208, STEP_LV2, SPD_WIN_OUT,	// 下から上(戻)
};

// 移動のテーブル(開始、終了、0、spd)
static const s16 bv_scr_cutin_tbl[] = {
   -128,   0, 0, SPD_CUTIN,				// 外から中
	  0,-128, 0, SPD_CUTOUT,			// 中から外
};

static void bv_init_bgscroll( BADGEVIEW_WORK* wk, const s32 mode )
{
	s16		tmp;
	s32		buf[4];
	//
	// スクロール
	buf[0] = bv_scr_tbl[4*mode];
	buf[1] = bv_scr_tbl[4*mode+1];
	buf[2] = bv_scr_tbl[4*mode+3];
	bv_init_movework( wk, MWORK_WIN, buf[0], buf[1], buf[2] );
	// 開始座標をセットし、すぐ反映
	wk->BgOffY = buf[0];
	GFL_BG_SetScroll( BV_BGFRAME_D_WIN, GFL_BG_SCROLL_Y_SET, wk->BgOffY );
	// step
	wk->Step = bv_scr_tbl[4*mode+2];
	//
	// カットインもここに記述
	tmp = mode/2;
	buf[0] = bv_scr_cutin_tbl[4*tmp];
	buf[1] = bv_scr_cutin_tbl[4*tmp+1];
	buf[2] = bv_scr_cutin_tbl[4*tmp+3];
	bv_init_movework( wk, MWORK_CUTIN, buf[0], buf[1], buf[2] );
	// 開始座標をセットし、すぐ反映
	wk->BgOffX = buf[0];
	GFL_BG_SetScroll( BV_BGFRAME_U_CUTIN, GFL_BG_SCROLL_X_SET, wk->BgOffX );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理まとめ
 *
 *	@param	wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bv_main_scroll( BADGEVIEW_WORK* wk )
{
	s16		tmp;
	
	// 処理
	switch( wk->Step ){
		// 初期化
		case STEP_INIT:
			// 待ち
			break;
		//
		//-----------------
		// スクロールイン
		//-----------------
		// BGスクロール開始
		case STEP_LV1:
			wk->Step = STEP_LV1+10;
			wk->Flag |= FLG_SET_BGOFFSET;
			// 非表示
			bv_set_btn_visible( wk, BUTTON_CARD, FALSE );
			bv_set_btn_visible( wk, BUTTON_END,  FALSE );
			bv_set_btn_visible( wk, BUTTON_BOOKMARK,  FALSE );
			break;
		//
		// BGスクロール処理
		case STEP_LV1+10:
			wk->BgOffY = bv_main_movework( wk, MWORK_WIN );
			if( bv_is_movework( wk, MWORK_WIN ) == FALSE ){
				wk->Step = STEP_LV1+20;
			}
			// カットインも流用
			wk->BgOffX = bv_main_movework( wk, MWORK_CUTIN );
			//
			break;
		//
		// BGスクロール終了
		case STEP_LV1+20:
			// 転送
			InfoWinPrint( wk, wk->TypeNum, wk->PageNum, 0 );
			InfoWinPut( wk );
			wk->Flag |= FLG_END_BGOFFSET;
			wk->Step = STEP_END;
			break;
		
		//-----------------
		// スクロールアウト
		//-----------------
		// BGスクロール開始
		case STEP_LV2:
			wk->Step = STEP_LV2+10;
			wk->Flag |= FLG_SET_BGOFFSET;
			break;
		//
		// BGスクロール処理
		case STEP_LV2+10:
			wk->BgOffY = bv_main_movework( wk, MWORK_WIN );
			if( bv_is_movework( wk, MWORK_WIN ) == FALSE ){
				wk->Step = STEP_LV2+20;
			}
			// カットインも流用
			wk->BgOffX = bv_main_movework( wk, MWORK_CUTIN );
			break;
		//
		// BGスクロール終了
		case STEP_LV2+20:
			// 描画領域クリア
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin), 0x0f0f );
			InfoWinPut( wk );
			bmpwin_visible( wk, FALSE );
			//
			// カットイン非表示
			bv_set_cutin_visible( wk, FALSE );
			//
			// 表示
			bv_set_btn_visible( wk, BUTTON_CARD, TRUE );
			bv_set_btn_visible( wk, BUTTON_END,  TRUE );
			bv_set_btn_visible( wk, BUTTON_BOOKMARK,  TRUE );
			//
			wk->Flag |= FLG_END_BGOFFSET;
			wk->Step = STEP_END;
			break;
		//
		// 終了
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  スクロール中か
 *
 *	@param	wk
 *
 *	@retval BOOL (FALSE:まだ、停止、TRUE:再生中)
 */
//-----------------------------------------------------------------------------
static BOOL bv_is_bgscroll( BADGEVIEW_WORK* wk )
{
	BOOL	bflg=FALSE;
	
	// 処理中
	if(wk->Flag & FLG_SET_BGOFFSET){
		bflg = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objの表示/非表示
 *
 *	@param	wk
 *	@param	s32         番号
 *	@param  const BOOL  (TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bv_set_obj_visible( BADGEVIEW_WORK* wk, const s32 num, const BOOL bflg )
{
	GFL_CLACT_WK_SetDrawEnable( wk->clwk[num], bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ボタン表示(on/off)
 *
 *	@param	wk
 *	@param	s32         (ボタンの番号)
 *	@param  const BOOL  (TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
// ボタンの番号 => clwkのワーク番号に変換
static const s16 btn_clwk_tbl[] = {
	BV_OBJ_CARD,
	BV_OBJ_RETURN,
	BV_OBJ_END,
	BV_OBJ_BOOKMARK,
};

static void bv_set_btn_visible( BADGEVIEW_WORK* wk, const s32 mode, const BOOL bflg )
{
	s16	num = btn_clwk_tbl[mode];
	//
	bv_set_obj_visible( wk, num, bflg );
	//
	// (非表示)フラグもセット
	if(bflg == TRUE){
		wk->Flag &= ~(1<<(mode+4));
	}else{
		wk->Flag |= (1<<(mode+4));
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タイマ処理初期化
 *
 *	@param	BADGEVIEW_WORK* wk
 *	@param	const s32 mode		タイマの種類
 *	@param	const s32 timer		フレーム数
 *
 *	@retval 
 */
//-----------------------------------------------------------------------------
static void bv_init_timer( BADGEVIEW_WORK* wk, const s32 mode, const s32 timer )
{
	wk->Timer[mode] = timer;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タイマ処理
 *
 *	@param	BADGEVIEW_WORK* wk
 *	@param	const s32 mode		タイマの種類
 *
 *	@retval s32					時間
 */
//-----------------------------------------------------------------------------
static s32 bv_chk_timer( BADGEVIEW_WORK* wk, const s32 mode )
{
	wk->Timer[mode]--;
	if(wk->Timer[mode] <= 0){
		wk->Timer[mode] = 0;
	}
	return( wk->Timer[mode] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  リーダー関連の処理
 *
 *	@param	BADGEVIEW_WORK* wk
 *
 *	@retval s32					時間
 */
//-----------------------------------------------------------------------------
static void bv_chk_leader( BADGEVIEW_WORK* wk )
{
	int		trg;
	s32		timer;
	s32		flg=0;
	
	// リーダー情報表示
	if(wk->Mode == MODE_LEADER){
		// タイマー処理
		timer = bv_chk_timer( wk, TIMER_MES );
		if(timer <= 0){
			// 番号切り替え
			wk->LeaderNum = 1-wk->LeaderNum;
			// タイマ初期化
			bv_init_timer( wk, TIMER_MES, bv_timer_tbl[wk->LeaderNum]);
			// 文字転送
			InfoWinPrint( wk, wk->TypeNum, wk->PageNum, wk->LeaderNum );
			InfoWinPut( wk );
			flg = 1;
		}
		// とりあえずここに入れておく(要検討)
		// タッチ判定
		if(flg == 0){
			// タッチ判定
			trg = GFL_UI_TP_HitTrg( leader_hit_tbl );
			if(trg == 0){
				// メッセージ変更
				if(wk->LeaderNum == 0){
					wk->LeaderNum = 1;
					// タイマ初期化
					bv_init_timer( wk, TIMER_MES, bv_timer_tbl[wk->LeaderNum]);
					// 文字転送
					InfoWinPrint( wk, wk->TypeNum, wk->PageNum, wk->LeaderNum );
					InfoWinPut( wk );
				}
				// 終了処理
				else{
					bv_init_bgscroll( wk, MODE_DOWNUP_REV );
					// ウインドウを消す
					wk->Mode = MODE_NORMAL;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  バッジの音
 *
 *	@param	BADGEVIEW_WORK* wk
 *	@param	const s32		(0-7)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const u16 bv_snd_tbl[] = {
	SEQ_SE_SYS_32,		// 0
	SEQ_SE_SYS_32,		// 1
	SEQ_SE_SYS_32,		// 2
	SEQ_SE_SYS_32,		// 3
	SEQ_SE_SYS_32,		// 4
	SEQ_SE_SYS_32,		// 5
	SEQ_SE_SYS_32,		// 6
	SEQ_SE_SYS_32,		// 7
};
//
static void bv_snd_badge( BADGEVIEW_WORK* wk, const s32 num )
{
	PMSND_PlaySE( bv_snd_tbl[num] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パレット変更(グレー=>色つき)
 *
 *	@param	BADGEVIEW_WORK* wk
 *	@param	const s32		(0-7)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bv_chg_facepal( BADGEVIEW_WORK* wk, const s32 num )
{
	NNSG2dPaletteData *pltt;
	u16		*adr;		// u16なので注意
	
	// 下画面の指定したキャラ番号
	pltt = wk->FacePal;
	adr  = (u16*)pltt->pRawData;
	// 正しく動く
	NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB, (0x20*num), &adr[num*0x10], 0x20 );
	// これでもよい
//	GXS_LoadBGPltt( &adr[num*0x10], 0x20*num, 0x20 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  顔グラフィック判定
 *
 *	@param	BADGEVIEW_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bv_chk_facepal( BADGEVIEW_WORK* wk )
{
	s32		i;
	
	for(i=0;i<BADGE_NUM;i++){
		// バッジを取得済み=>変更
		if(wk->badgeflag[i] == 1){
			bv_chg_facepal( wk, i );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  上画面カットインキャラ表示
 *
 *	@param	BADGEVIEW_WORK* wk
 *	@param	const s32		番号(field/enc_cutin_no.h)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void bv_init_cutin( BADGEVIEW_WORK* wk, const s32 cnum )
{
	const ENC_CUTIN_DAT *dat;
	ARCHANDLE			*handle;
	//
	// カットインキャラの情報取得
	// オーバーレイ注意
	// (最初と最後で対応した)
	dat = ENC_CUTIN_NO_GetDat( cnum );
	
	// 転送
	handle = GFL_ARC_OpenDataHandle( ARCID_ENC_CI_FACE, HEAPID_TR_CARD );
	//
	// 上半身
	GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, dat->ChrArc01Idx,
										  BV_BGFRAME_U_CUTIN, 0, CUTIN_CHR_SIZE, TRUE, HEAPID_TR_CARD );
	// 下半身(256キャラ分ずらして転送)
	GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, dat->ChrArc02Idx,
										  BV_BGFRAME_U_CUTIN, CUTIN_CHR_OFFSET, CUTIN_CHR_SIZE, TRUE, HEAPID_TR_CARD );
	// スクリーンはデータ(scn)で持つようにした
	// 計算で処理してもよいが、複雑になりそうなのでこの方法とした
	// handleに注意(ARCID_ENC_CI_FACEではない)
	GFL_ARCHDL_UTIL_TransVramScreen(      wk->g_handle, NARC_trainer_case_cutin00_NSCR,
										  BV_BGFRAME_U_CUTIN, 0, 0, FALSE, HEAPID_TR_CARD );
	// パレット
	GFL_ARCHDL_UTIL_TransVramPalette(     handle, dat->PltArcIdx,
										  PALTYPE_BG_UP, PAL_CUTIN_BG*0x20, CUTIN_PAL_SIZE*0x20, HEAPID_TR_CARD );
	GFL_ARC_CloseDataHandle( handle );
	
	// スクリーンのパレットを合わせる
	// 上半身(128x128)
	GFL_BG_ChangeScreenPalette( BV_BGFRAME_U_CUTIN, CUTIN_SCR_X0, CUTIN_SCR_Y0, CUTIN_SCR_W0, CUTIN_SCR_H0, PAL_CUTIN_BG   );
	// 下半身(128x64)(pal+1)
	GFL_BG_ChangeScreenPalette( BV_BGFRAME_U_CUTIN, CUTIN_SCR_X1, CUTIN_SCR_Y1, CUTIN_SCR_W1, CUTIN_SCR_H1, PAL_CUTIN_BG+1 );
	GFL_BG_LoadScreenReq( BV_BGFRAME_U_CUTIN );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  上画面カットインまとめ
 *
 *	@param	BADGEVIEW_WORK* wk
 *	@param	const s32		番号(0-7)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const s16 bv_cutin_tbl[] = {
	ENC_CUTIN_GYM1A,		// 0
	ENC_CUTIN_GYM2,			// 1
	ENC_CUTIN_GYM3,			// 2
	ENC_CUTIN_GYM4,			// 3
	ENC_CUTIN_GYM5,			// 4
	ENC_CUTIN_GYM6,			// 5
	ENC_CUTIN_GYM7,			// 6
	ENC_CUTIN_GYM8A,		// 7
};

static void bv_chg_cutin( BADGEVIEW_WORK* wk, const s32 num )
{
	// カットイン
	bv_init_cutin( wk, bv_cutin_tbl[num] );
}

//----------------------------------------------------------------------------------
/**
 * @brief	上画面カットイン表示/非表示
 *
 * @param	BADGEVIEW_WORK*	wk
 * @param	const BOOL		(TRUE:表示、FALSE:非表示)
 */
//----------------------------------------------------------------------------------
static void bv_set_cutin_visible( BADGEVIEW_WORK *wk, const BOOL bflg )
{
	// MAIN画面のCUTIN
	GFL_BG_SetVisible( BV_BGFRAME_U_CUTIN, bflg );
}



//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// デバッグ用
//----------------------------------------------------------------------------------
#ifdef PM_DEBUG


  // デバッグ用データセット
static void _debug_data_set( BADGEVIEW_WORK *wk )
{
  int i, num=0;
  STRBUF *str;

#ifdef SPECIAL_FEBRUARY
  num = 23;
#endif

  if(GFL_UI_KEY_GetCont() & PAD_KEY_UP){
    num = 5;
  }else if(GFL_UI_KEY_GetCont() & PAD_BUTTON_X){
    num = 13;
  }else if(GFL_UI_KEY_GetCont() & PAD_KEY_LEFT){
    num = 20;
  }else if(GFL_UI_KEY_GetCont() & PAD_BUTTON_R){
    num = 25;
  }else if(GFL_UI_KEY_GetCont() & PAD_BUTTON_Y){
    num = 30;
  }

  
  for(i=0;i<num;i++){
    str = GFL_MSG_CreateString( wk->debugname, 20+i);
    GFL_STR_DeleteBuffer( str );
  }
}

#endif
