//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file   irc_aura_main.c
 *  @brief  赤外線ミニゲーム  オーラチェック
 *  @author Toru=Nagihashi
 *  @data   2009.05.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//  lib
#include <gflib.h>

//  constant
#include "system/main.h"  //HEAPID
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"

//  module
#include "infowin/infowin.h"

#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "system/bmp_winframe.h"

#include "net_app/irc_appbar.h"

//  archive
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_irc_aura.h"
#include "irccompatible_gra.naix"

//  aura
#include "net_app/irc_aura.h"

// proc
#include "net_app/irc_result.h"

//sound
#include "../irc_compatible/irc_compatible_snd.h"

#ifdef PM_DEBUG
//debug用
#include "system/net_err.h" //VRAM退避用アドレスを貰うため
#include <wchar.h>          //wcslen
#endif


FS_EXTERN_OVERLAY(irc_result);

//=============================================================================
/**
 *          定数宣言
*/
//=============================================================================
#ifdef PM_DEBUG
static GFL_POINT s_pos  = {0};
#endif //PM_DEBUG


#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
static BOOL s_is_debug_only_play  = 0;

#define DEBUG_AURA_MSG  //デバッグメッセージを出す
#endif


//-------------------------------------
/// 点数
//=====================================
//左手最初の座標の距離による点数
#define VAL_POS_SCORE_00 (100)
#define VAL_POS_SCORE_01 (80)
#define VAL_POS_SCORE_02 (50)
#define VAL_POS_SCORE_03 (30)
#define VAL_POS_SCORE_04 (10)
#define VAL_POS_SCORE_05 (0)

#define RANGE_POS_SCORE_00 (8*8)
#define RANGE_POS_SCORE_01 (16*16)
#define RANGE_POS_SCORE_02 (24*24)
#define RANGE_POS_SCORE_03 (32*32)
#define RANGE_POS_SCORE_04 (40*40)

//ブレの座標による点数
#define VAL_SHAKE_SCORE_00 (100)
#define VAL_SHAKE_SCORE_01 (80)
#define VAL_SHAKE_SCORE_02 (50)
#define VAL_SHAKE_SCORE_03 (30)
#define VAL_SHAKE_SCORE_04 (0)

#define RANGE_SHAKE_SCORE_00(x) ( x == 0 )
#define RANGE_SHAKE_SCORE_01(x) ( x == 1 )
#define RANGE_SHAKE_SCORE_02(x) ( x == 2 )
#define RANGE_SHAKE_SCORE_03(x) ( 3 <= x && x <= 5 )
#define RANGE_SHAKE_SCORE_04(x) ( x <= 6)

//-------------------------------------
/// パレット
//=====================================
enum{
  // メイン画面BG
  AURA_BG_PAL_M_00 = 0,//背景
  AURA_BG_PAL_M_01,   //
  AURA_BG_PAL_M_02,   //
  AURA_BG_PAL_M_03,   //
  AURA_BG_PAL_M_04,   //
  AURA_BG_PAL_M_05,   //
  AURA_BG_PAL_M_06,   //
  AURA_BG_PAL_M_07,   //
  AURA_BG_PAL_M_08,   // 背景
  AURA_BG_PAL_M_09,   // ガイド
  AURA_BG_PAL_M_10,   // 使用してない
  AURA_BG_PAL_M_11,   // 使用してない
  AURA_BG_PAL_M_12,   // 使用してない
  AURA_BG_PAL_M_13,   // APPBAR
  AURA_BG_PAL_M_14,   // フォント用PLT
  AURA_BG_PAL_M_15,   // INFOWIN


  // サブ画面BG
  AURA_BG_PAL_S_00 = 0, //背景
  AURA_BG_PAL_S_01,   //
  AURA_BG_PAL_S_02,   //
  AURA_BG_PAL_S_03,   //
  AURA_BG_PAL_S_04,   //
  AURA_BG_PAL_S_05,   //
  AURA_BG_PAL_S_06,   //
  AURA_BG_PAL_S_07,   //
  AURA_BG_PAL_S_08,   // 背景
  AURA_BG_PAL_S_09,   // ガイド
  AURA_BG_PAL_S_10,   // 使用してない
  AURA_BG_PAL_S_11,   // 使用してない
  AURA_BG_PAL_S_12,   // 使用してない
  AURA_BG_PAL_S_13,   // 使用してない
  AURA_BG_PAL_S_14,   // フォント用PLT
  AURA_BG_PAL_S_15,   // 使用してない

  // メイン画面OBJ
  AURA_OBJ_PAL_M_00 = 0,//GUIDE用OBJ
  AURA_OBJ_PAL_M_01,    // 使用してない
  AURA_OBJ_PAL_M_02,    // 使用してない
  AURA_OBJ_PAL_M_03,    // 使用してない
  AURA_OBJ_PAL_M_04,    // 使用してない
  AURA_OBJ_PAL_M_05,    // 使用してない
  AURA_OBJ_PAL_M_06,    // 使用してない
  AURA_OBJ_PAL_M_07,    // 使用してない
  AURA_OBJ_PAL_M_08,    // 使用してない
  AURA_OBJ_PAL_M_09,    // 使用してない
  AURA_OBJ_PAL_M_10,    // 使用してない
  AURA_OBJ_PAL_M_11,    // 使用してない
  AURA_OBJ_PAL_M_12,    // 使用してない
  AURA_OBJ_PAL_M_13,    // APPBAR
  AURA_OBJ_PAL_M_14,    // APPBAR
  AURA_OBJ_PAL_M_15,    // APPBAR


  // サブ画面OBJ
  AURA_OBJ_PAL_S_00 = 0,  //使用してない
  AURA_OBJ_PAL_S_01,    // 使用してない
  AURA_OBJ_PAL_S_02,    // 使用してない
  AURA_OBJ_PAL_S_03,    // 使用してない
  AURA_OBJ_PAL_S_04,    // 使用してない
  AURA_OBJ_PAL_S_05,    // 使用してない
  AURA_OBJ_PAL_S_06,    // 使用してない
  AURA_OBJ_PAL_S_07,    // 使用してない
  AURA_OBJ_PAL_S_08,    // 使用してない
  AURA_OBJ_PAL_S_09,    // 使用してない
  AURA_OBJ_PAL_S_10,    // 使用してない
  AURA_OBJ_PAL_S_11,    // 使用してない
  AURA_OBJ_PAL_S_12,    // 使用してない
  AURA_OBJ_PAL_S_13,    // 使用してない
  AURA_OBJ_PAL_S_14,    // 使用してない
  AURA_OBJ_PAL_S_15,    // 使用してない
};

//-------------------------------------
/// 文字
//=====================================
#define TEXTSTR_PLT_NO        (AURA_BG_PAL_M_14)
#define TEXTSTR_BUFFER_LENGTH (255)

//-------------------------------------
/// 位置
//=====================================
#define MSGWND_TEXT_X (1)
#define MSGWND_TEXT_Y (18)
#define MSGWND_TEXT_W (30)
#define MSGWND_TEXT_H (5)

#define MSGWND_RETURN_X (1)
#define MSGWND_RETURN_Y (22)
#define MSGWND_RETURN_W (30)
#define MSGWND_RETURN_H (2)

#define DEBUGMSG_LEFT1  (0)
#define DEBUGMSG_LEFT2  (20)
//#define DEBUGMSG_LEFT3  (40)
//#define DEBUGMSG_LEFT4  (60)

#define DEBUGMSG_RIGHT1 (40)
#define DEBUGMSG_RIGHT2 (60)
//#define DEBUGMSG_RIGHT3 (120)
//#define DEBUGMSG_RIGHT4 (140)

#define DEBUGMSG2_TAB     (170)
#define DEBUGMSG3_TAB     (85)

#define	MSGWND_TITLE_X	(6)
#define	MSGWND_TITLE_Y	(3)
#define	MSGWND_TITLE_W	(20)
#define	MSGWND_TITLE_H	(5)

//-------------------------------------
/// カウント
//=====================================
#define TOUCH_COUNTER_MAX (150)
#define TOUCH_COUNTER_SHAKE_SYNC  (15)  //ブレを観測するための1回のシンク
#define TOUCH_COUNTER_SHAKE_MAX (TOUCH_COUNTER_MAX/TOUCH_COUNTER_SHAKE_SYNC)  //ブレを取得する回数
#define RESULT_SEND_CNT (COMPATIBLE_IRC_SENDATA_CNT)

//-------------------------------------
/// 個数
//=====================================
#define CIRCLE_MAX    (3)

//-------------------------------------
///   MSG_FONT
//=====================================
typedef enum {
  MSG_FONT_TYPE_LARGE,
}MSG_FONT_TYPE;

//-------------------------------------
/// OBJ登録ID
//=====================================
enum {
  OBJREGID_TOUCH_PLT,
  OBJREGID_TOUCH_CHR,
  OBJREGID_TOUCH_CEL,

  OBJREGID_MAX
};

//-------------------------------------
/// メッセージ表示ウィンドウ
//=====================================
enum {
  MSGWNDID_TEXT,

  MSGWNDID_MAX,
};

//-------------------------------------
/// タッチエフェクト
//=====================================
typedef enum {
  TOUCHEFFID_LEFT,
  TOUCHEFFID_RIGHT,


  TOUCHEFFID_MAX,
}TOUCHEFFID;

//-------------------------------------
/// 個人結果画面の定数
//=====================================
enum
{
  AOR_MSGWNDID_TITLE,
  AOR_MSGWNDID_MSG,
  AOR_MSGWNDID_DEBUG,

  AOR_MSGWNDID_MAX,
};

#define AOR_MSGWND_TITLE_X  (2)
#define AOR_MSGWND_TITLE_Y  (3)
#define AOR_MSGWND_TITLE_W  (20)
#define AOR_MSGWND_TITLE_H  (2)
#define AOR_MSGWND_MSG_X    (6)
#define AOR_MSGWND_MSG_Y    (9)
#define AOR_MSGWND_MSG_W    (20)
#define AOR_MSGWND_MSG_H    (6)
#define AOR_MSGWND_DEBUG_X  (2)
#define AOR_MSGWND_DEBUG_Y  (16)
#define AOR_MSGWND_DEBUG_W  (28)
#define AOR_MSGWND_DEBUG_H  (4)

//-------------------------------------
/// モデル数
//=====================================
#define G3D_MDL_MAX         (2)

//-------------------------------------
/// デバッグ用人数セーブ機能
//=====================================
#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
#define DEBUG_GAME_NUM  (1)
#define DEBUG_PLAYER_SAVE_NUM (DEBUG_GAME_NUM*2)
#else
#define DEBUG_PLAYER_SAVE_NUM (1)
#endif //DEBUG_IRC_COMPATIBLE_ONLYPLAY

//-------------------------------------
/// 3Dリソース
//=====================================
typedef enum
{
  G3D_MDL_ANM_ICA,
  G3D_MDL_ANM_IMA,
  G3D_MDL_ANM_MAX
}G3D_MDL_ANM;
typedef struct
{
  u16 imdID;
  u16 anmID[G3D_MDL_ANM_MAX];
} G3D_MDL_SETUP;

//-------------------------------------
///   BACKOBJ
//=====================================
//後ろOBJの数
#define BACKOBJ_CLWK_MAX  (32)
//
typedef enum
{
  BACKOBJ_MOVE_TYPE_RAIN,   //雨のように上から降り注ぐ
  BACKOBJ_MOVE_TYPE_EMITER, //放出
  BACKOBJ_MOVE_TYPE_GATHER, //集中
} BACKOBJ_MOVE_TYPE;

//OBJのパレットと対応している
typedef enum
{
  BACKOBJ_COLOR_RED     = 1,
  BACKOBJ_COLOR_ORANGE  = 5,
  BACKOBJ_COLOR_YELLOW  = 6,
  BACKOBJ_COLOR_YEGREEN = 7,
  BACKOBJ_COLOR_GREEN   = 8,
  BACKOBJ_COLOR_WATER   = 9,
  BACKOBJ_COLOR_BLUE    = 0xA,
} BACKOBJ_COLOR;

//１つのOBJが動くまでのシンク
#define BACKOBJ_MOVE_SYNC (10)

#define BACKOBJ_ONE_MOVE_SYNC_MIN (180)
#define BACKOBJ_ONE_MOVE_SYNC_MAX (190)
#define BACKOBJ_ONE_MOVE_SYNC_DIF (BACKOBJ_ONE_MOVE_SYNC_MAX-BACKOBJ_ONE_MOVE_SYNC_MIN)

//-------------------------------------
/// CLWK取得
//=====================================
typedef enum
{
  CLWKID_BACKOBJ_TOP_S,
  CLWKID_BACKOBJ_END_S = CLWKID_BACKOBJ_TOP_S + BACKOBJ_CLWK_MAX,

  CLWKID_MAX
}CLWKID;


#define IRC_AURA_SCORE_MINUS  (1) //減点

//=============================================================================
/**
 *          構造体宣言
*/
//=============================================================================
//-------------------------------------
/// BG関係
//=====================================
typedef struct
{
  GFL_ARCUTIL_TRANSINFO frame_char;
} GRAPHIC_BG_WORK;
//-------------------------------------
/// 3Dリソース
//=====================================
typedef struct
{
  GFL_G3D_RES *p_res;
  GFL_G3D_RES *p_anm_res[G3D_MDL_ANM_MAX];
  GFL_G3D_RND *p_rnd;
  GFL_G3D_OBJ *p_obj;
  GFL_G3D_ANM *p_anm[G3D_MDL_ANM_MAX];
  GFL_G3D_OBJSTATUS status;
  BOOL  is_visible;
  BOOL  is_end_loop;
} G3D_MDL_WORK;

//-------------------------------------
/// 3D描画環境
//=====================================
typedef struct
{
  GFL_G3D_CAMERA    *p_camera;
  G3D_MDL_WORK      mdl[G3D_MDL_MAX];
  fx32              alpha;
} GRAPHIC_3D_WORK;
#if 0
//-------------------------------------
/// 円
//=====================================
enum {
  CIRCLE_VTX_MAX  = 60, //頂点数
};
typedef struct
{
  VecFx16 *p_vtx;
  u32     vtx_num;
  GXRgb vtx_color;
  u8  alpha;
  u8  dummy[3];
  GFL_G3D_OBJSTATUS status;
  BOOL  is_visible;
} CIRCLE_WORK;
typedef struct{
  CIRCLE_WORK       c[CIRCLE_MAX];
  u16               rot[CIRCLE_MAX];
  u16               rot_add[CIRCLE_MAX];
}TOUCH_EFFECT_WORK;
#endif
typedef struct {
  //TOUCH_EFFECT_WORK wk[TOUCHEFFID_MAX];
  G3D_MDL_WORK      *p_mdl[TOUCHEFFID_MAX];
  GFL_POINT         left;
} TOUCH_EFFECT_SYS;

//-------------------------------------
/// OBJ関係
//=====================================
typedef struct {
  GFL_CLUNIT *p_clunit;
  u32         reg_id[OBJREGID_MAX];
  GFL_CLWK   *p_clwk[CLWKID_MAX];
} GRAPHIC_OBJ_WORK;
//-------------------------------------
/// 描画関係
//=====================================
typedef struct
{
  GRAPHIC_BG_WORK   bg;
  GRAPHIC_3D_WORK   g3d;
  GRAPHIC_OBJ_WORK  obj;
  TOUCH_EFFECT_SYS    touch_eff;

  GFL_TCB           *p_vblank_task;
} GRAPHIC_WORK;
//-------------------------------------
/// 文字管理
//=====================================
typedef struct
{
  GFL_FONT*         p_font;
  GFL_MSGDATA*      p_msg;
  PRINT_QUE*        p_print_que;
  WORDSET*          p_wordset;
} MSG_WORK;

//-------------------------------------
/// メッセージ表示ウィンドウ
//=====================================
typedef struct
{
  GFL_BMPWIN*       p_bmpwin;
  PRINT_UTIL        print_util;
  STRBUF*           p_strbuf;
  u16               clear_chr;
  u16               dummy;
} MSGWND_WORK;

//-------------------------------------
/// ブレ計測
//=====================================
typedef struct
{
  GFL_POINT   shake[TOUCH_COUNTER_SHAKE_MAX];
  u16         shake_idx;
  u32         cnt;        //カウンタ
  u32         shake_cnt;  //ブレ計測用カウンタ
  BOOL        is_end;     //1回取得したフラグ
}SHAKE_SEARCH_WORK;

//-------------------------------------
/// オーラ用ネット
//=====================================
//データ
typedef struct
{
  GFL_POINT trg_left;
  GFL_POINT trg_right;
  SHAKE_SEARCH_WORK shake_left;
  SHAKE_SEARCH_WORK shake_right;
} AURANET_RESULT_DATA;
//ネットメイン
typedef struct
{
  COMPATIBLE_IRC_SYS  *p_irc;
  u32 seq;
  AURANET_RESULT_DATA result_recv;  //受信バッファ
  AURANET_RESULT_DATA result_recv_my; //受信バッファ
  AURANET_RESULT_DATA result_send;  //送信バッファ
  BOOL is_recv;
} AURANET_WORK;
//-------------------------------------
/// BACKOBJ 背面ぴかぴか
//=====================================
typedef struct
{
  GFL_CLWK          *p_clwk;
  int               sf_type;
  BOOL              is_req;
  VecFx32           start;
  VecFx32           end;
  u32               sync_now;
  u32               sync_max;
} BACKOBJ_ONE;
typedef struct
{
  BACKOBJ_ONE       wk[BACKOBJ_CLWK_MAX];
  BACKOBJ_MOVE_TYPE type;
  BACKOBJ_COLOR     color;
  u32               sync_now; //１つのワークを開始するまでのシンク
  u32               sync_max;
} BACKOBJ_WORK;
//-------------------------------------
/// 個人成績画面
//=====================================
typedef struct
{
  u32 seq;
  const MSG_WORK *cp_msg;
  const SHAKE_SEARCH_WORK *cp_search;
  MSGWND_WORK     msgwnd[AOR_MSGWNDID_MAX];
} AURA_ONLYRESULT_WORK;


#ifdef DEBUG_AURA_MSG
//-------------------------------------
/// デバッグプリント用画面
//=====================================
typedef struct
{
  GFL_BMP_DATA *p_bmp;
  GFL_FONT*     p_font;

  BOOL  is_now_save;
  u8  frm;

  u8  *p_char_temp_area;      ///<キャラクタVRAM退避先
  u16 *p_scrn_temp_area;      ///<スクリーンVRAM退避先
  u16 *p_pltt_temp_area;      ///<パレットVRAM退避先

  u8  font_col_bkup[3];
  u8  prioryty_bkup;
  u8  scroll_x_bkup;
  u8  scroll_y_bkup;

  HEAPID heapID;
} DEBUG_PRINT_WORK;

#define DEBUGPRINT_CHAR_TEMP_AREA (0x4000)
#define DEBUGPRINT_SCRN_TEMP_AREA (0x800)
#define DEBUGPRINT_PLTT_TEMP_AREA (0x20)
#define DEBUGPRINT_WIDTH  (32)
#define DEBUGPRINT_HEIGHT (18)

static DEBUG_PRINT_WORK *sp_dp_wk;
#endif //DEBUG_AURA_MSG




//-------------------------------------
/// オーラチェックメインワーク
//=====================================
typedef struct _AURA_MAIN_WORK AURA_MAIN_WORK;
typedef void (*SEQ_FUNCTION)( AURA_MAIN_WORK *p_wk, u16 *p_seq );
struct _AURA_MAIN_WORK
{
  //グラフィックモジュール
  GRAPHIC_WORK    grp;
  MSG_WORK        msg;
  MSGWND_WORK     msgwnd[MSGWNDID_MAX];
  BACKOBJ_WORK    backobj;

  //シーケンス管理
  SEQ_FUNCTION    seq_function;
  u16   seq;
  u16   cnt;
  BOOL is_end;
  BOOL is_next_proc;

#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
  u16             debug_player;     //自分か相手か
  u16             debug_game_cnt; //何ゲーム目か
#endif //DEBUG_IRC_COMPATIBLE_ONLYPLAY

  MSGWND_WORK     msgtitle; //タイトルメッセージ

  //ネット
  AURANET_WORK  net;

  //結果表示
  AURA_ONLYRESULT_WORK  onlyresult;

  //下画面バー
  APPBAR_WORK *p_appbar;

  //結果
  GFL_POINT   trg_left[DEBUG_PLAYER_SAVE_NUM];
  GFL_POINT   trg_right[DEBUG_PLAYER_SAVE_NUM];
  SHAKE_SEARCH_WORK shake_left[DEBUG_PLAYER_SAVE_NUM];
  SHAKE_SEARCH_WORK shake_right[DEBUG_PLAYER_SAVE_NUM];

  //引数
  IRC_AURA_PARAM  *p_param;
};

//=============================================================================
/**
 *          プロトタイプ宣言
*/
//=============================================================================
//proc
static GFL_PROC_RESULT IRC_AURA_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_AURA_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_AURA_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
//grp
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk );
static void GRAPHIC_Draw( GRAPHIC_WORK *p_wk );
static GFL_CLWK* GRAPHIC_GetClwk( const GRAPHIC_WORK *cp_wk, CLWKID id );
static GFL_CLUNIT * GRAPHIC_GetClunit( const GRAPHIC_WORK *cp_wk );
static TOUCH_EFFECT_SYS * GRAPHIC_GetTouchEffWk( GRAPHIC_WORK *p_wk );
static void GRAPHIC_WriteBmpwinFrame( GRAPHIC_WORK *p_wk, GFL_BMPWIN *p_bmpwin );
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work );
//BG
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_WriteBmpwinFrame( GRAPHIC_BG_WORK *p_wk, GFL_BMPWIN *p_bmpwin );
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk );
//obj
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID );
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk );
static GFL_CLWK* GRAPHIC_OBJ_GetClwk( const GRAPHIC_OBJ_WORK *cp_wk, CLWKID id );
static GFL_CLUNIT * GRAPHIC_OBJ_GetClunit( const GRAPHIC_OBJ_WORK *cp_wk );
//3d
static void GRAPHIC_3D_Init( GRAPHIC_3D_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_3D_Exit( GRAPHIC_3D_WORK *p_wk );
static void GRAPHIC_3D_StartDraw( GRAPHIC_3D_WORK *p_wk );
static void GRAPHIC_3D_EndDraw( GRAPHIC_3D_WORK *p_wk );
static G3D_MDL_WORK *GRAPHIC_3D_GetObj( const GRAPHIC_3D_WORK *cp_wk, u16 idx );
static void Graphic_3d_SetUp( void );
//3d_mdl
static void G3DMDL_Load( G3D_MDL_WORK *p_wk, ARCHANDLE *p_handle, const G3D_MDL_SETUP *cp_setup, HEAPID heapID );
static void G3DMDL_UnLoad( G3D_MDL_WORK *p_wk );
static void G3DMDL_Draw( G3D_MDL_WORK *p_wk );
static BOOL G3DMDL_IsLoopEnd( const G3D_MDL_WORK *cp_wk );
static void G3DMDL_SetPos( G3D_MDL_WORK *p_wk, const VecFx32 *cp_trans );
static void G3DMDL_SetScale( G3D_MDL_WORK *p_wk, const VecFx32 *cp_scale );
static void G3DMDL_SetVisible( G3D_MDL_WORK *p_wk, BOOL is_visible );
static BOOL G3DMDL_GetVisible( const G3D_MDL_WORK *cp_wk );
//MSG_WORK
static void MSG_Init( MSG_WORK *p_wk, MSG_FONT_TYPE font, HEAPID heapID );
static void MSG_Exit( MSG_WORK *p_wk );
static BOOL MSG_Main( MSG_WORK *p_wk );
static GFL_FONT*  MSG_GetFont( const MSG_WORK *cp_wk );
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk );
static const GFL_MSGDATA * MSG_GetMsgDataConst( const MSG_WORK *cp_wk );
static WORDSET * MSG_GetWordSet( const MSG_WORK *cp_wk );
//MSG_WINDOW
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe, u8 x, u8 y, u8 w, u8 h, u8 plt, HEAPID heapID );
static void MSGWND_InitEx( MSGWND_WORK* p_wk, u8 bgframe,
    u8 x, u8 y, u8 w, u8 h, u8 plt, u16 clear_chr, u8 dir, HEAPID heapID );
static void MSGWND_Exit( MSGWND_WORK* p_wk );
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, const MSG_WORK *cp_msg );
static void MSGWND_Print( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y );
static void MSGWND_PrintCenter( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID );
static void MSGWND_PrintNumber( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 number, u16 buff_id, u16 x, u16 y );
static void MSGWND_PrintPlayerName( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, const COMPATIBLE_STATUS *cp_status, u16 x, u16 y, HEAPID heapID );
static void MSGWND_Clear( MSGWND_WORK* p_wk );
static GFL_BMPWIN *MSGWND_GetBmpWin( const MSGWND_WORK* cp_wk );
//ONLYRESULT
#if 0
static void AURA_ONLYRESULT_Init( AURA_ONLYRESULT_WORK* p_wk, u8 frm, const MSG_WORK *cp_msg, const SHAKE_SEARCH_WORK *cp_search,  HEAPID heapID );
static void AURA_ONLYRESULT_Exit( AURA_ONLYRESULT_WORK* p_wk );
static BOOL AURA_ONLYRESULT_Main( AURA_ONLYRESULT_WORK* p_wk );
#endif
//SEQ
static void SEQ_Change( AURA_MAIN_WORK *p_wk, SEQ_FUNCTION  seq_function );
static void SEQ_End( AURA_MAIN_WORK *p_wk );
//SEQ_FUNC
static void SEQFUNC_StartGame( AURA_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_TouchLeft( AURA_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_TouchRight( AURA_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_Result( AURA_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_SceneError( AURA_MAIN_WORK *p_wk, u16 *p_seq );
//net
static void AURANET_Init( AURANET_WORK *p_wk, COMPATIBLE_IRC_SYS *p_irc );
static void AURANET_Exit( AURANET_WORK *p_wk );
static BOOL AURANET_SendResultData( AURANET_WORK *p_wk, const AURANET_RESULT_DATA *cp_data );
static void AURANET_GetResultData( const AURANET_WORK *cp_wk, AURANET_RESULT_DATA *p_data );
//net_recv
static void NETRECV_Result( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle );
static u8* NETRECV_GetBufferAddr(int netID, void* pWork, int size);
//円プリミティブ
#if 0
static void CIRCLE_Init( CIRCLE_WORK *p_wk, u32 vtx_num, HEAPID heapID );
static void CIRCLE_Exit( CIRCLE_WORK *p_wk );
static void CIRCLE_Draw( CIRCLE_WORK *p_wk );
static void CIRCLE_SetVtxColor( CIRCLE_WORK *p_wk, GXRgb color );
static void CIRCLE_SetAlpha( CIRCLE_WORK *p_wk, u8 alpha );
static void CIRCLE_SetTrans( CIRCLE_WORK *p_wk, const VecFx32 *cp_trans );
static void CIRCLE_SetScale( CIRCLE_WORK *p_wk, const VecFx32 *cp_scale );
static void CIRCLE_SetRot( CIRCLE_WORK *p_wk, const MtxFx33 *cp_rot );
static void CIRCLE_SetVisible( CIRCLE_WORK *p_wk, BOOL is_visible );
static void CIRCLE_GetTrans( const CIRCLE_WORK *cp_wk, VecFx32 *p_trans );
static void CIRCLE_GetScale( const CIRCLE_WORK *cp_wk, VecFx32 *p_scale );
static void CIRCLE_GetRot( const CIRCLE_WORK *cp_wk, MtxFx33 *p_rot );
static BOOL CIRCLE_GetVisible( const CIRCLE_WORK *cp_wk );
static void Circle_DrawLine( VecFx16 *p_start, VecFx16 *p_end );
#endif
//演出
static void TOUCH_EFFECT_Init( TOUCH_EFFECT_SYS *p_sys, const GRAPHIC_3D_WORK *cp_g3d, HEAPID heapID );
static void TOUCH_EFFECT_Exit( TOUCH_EFFECT_SYS *p_sys );
static void TOUCH_EFFECT_Draw( TOUCH_EFFECT_SYS *p_wk );
static void TOUCH_EFFECT_SetVisible( TOUCH_EFFECT_SYS *p_wk, TOUCHEFFID id, BOOL is_visible );
static void TOUCH_EFFECT_SetPos( TOUCH_EFFECT_SYS *p_wk, TOUCHEFFID id, u32 x, u32 y );

//汎用
static BOOL TP_GetRectTrg( const GFL_RECT *cp_rect, GFL_POINT *p_trg );
static BOOL TP_GetRectCont( const GFL_RECT *cp_rect, GFL_POINT *p_cont );
static u8   CalcScore( AURA_MAIN_WORK *p_wk );
static void ARCHDL_UTIL_TransVramScreenEx( ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 chr_ofs, u8 src_x, u8 src_y, u8 src_w, u8 src_h, u8 dst_x, u8 dst_y, u8 dst_w, u8 dst_h, u8 plt, BOOL compressedFlag, HEAPID heapID );
typedef enum
{
  GUIDE_VISIBLE_LEFT  = 1<<0,
  GUIDE_VISIBLE_RIGHT = 1<<1,
  GUIDE_VISIBLE_BOTH  = GUIDE_VISIBLE_LEFT|GUIDE_VISIBLE_RIGHT,
}GUIDE_VISIBLE_MASK;
static void GUIDE_SetVisible( GUIDE_VISIBLE_MASK msk, HEAPID heapID );

//BACKOBJ
static void BACKOBJ_Init( BACKOBJ_WORK *p_wk, const GRAPHIC_WORK *cp_grp, BACKOBJ_MOVE_TYPE type, BACKOBJ_COLOR color, u32 clwk_ofs, int sf_type );
static void BACKOBJ_Exit( BACKOBJ_WORK *p_wk );
static void BACKOBJ_Main( BACKOBJ_WORK *p_wk );
static void BACKOBJ_StartEmit( BACKOBJ_WORK *p_wk, const GFL_POINT *cp_pos );
//BACKOBJ_ONE
static void BACKOBJ_ONE_Init( BACKOBJ_ONE *p_wk, GFL_CLWK *p_clwk, BACKOBJ_COLOR color, int sf_type );
static void BACKOBJ_ONE_Main( BACKOBJ_ONE *p_wk );
static void BACKOBJ_ONE_Start( BACKOBJ_ONE *p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, u32 sync );
static BOOL BACKOBJ_ONE_IsMove( const BACKOBJ_ONE *cp_wk );

#ifdef DEBUG_AURA_MSG
static void DEBUGAURA_PRINT_UpDate( AURA_MAIN_WORK *p_wk );
#else
#define DEBUGAURA_PRINT_UpDate( ... ) ((void)0)
#endif //DEBUG_AURA_MSG


//ブレ計測
static void SHAKESEARCH_Init( SHAKE_SEARCH_WORK *p_wk );
static void SHAKESEARCH_Exit( SHAKE_SEARCH_WORK *p_wk );
static BOOL SHAKESEARCH_Main( SHAKE_SEARCH_WORK *p_wk, const GFL_RECT *cp_rect );
//DEBUG_PRINT
#ifdef DEBUG_AURA_MSG
static void DEBUGPRINT_Init( u8 frm, BOOL is_now_save, HEAPID heapID );
static void DEBUGPRINT_Exit( void );
static void DEBUGPRINT_Open( void );
static void DEBUGPRINT_Close( void );
static void DEBUGPRINT_Print( const u16 *cp_str, u16 x, u16 y );
static void DEBUGPRINT_PrintNumber( const u16 *cp_str, int number, u16 x, u16 y );
static void DEBUGPRINT_Clear( void );
#else
#define DEBUGPRINT_Init(...)        ((void)0)
#define DEBUGPRINT_Exit(...)        ((void)0)
#define DEBUGPRINT_Open(...)        ((void)0)
#define DEBUGPRINT_Close(...)       ((void)0)
#define DEBUGPRINT_Print(...)       ((void)0)
#define DEBUGPRINT_PrintNumber(...) ((void)0)
#define DEBUGPRINT_Clear(...)       ((void)0)
#endif //DEBUG_AURA_MSG
static STRBUF * DEBUGPRINT_CreateWideChar( const u16 *cp_str, HEAPID heapID );
static STRBUF * DEBUGPRINT_CreateWideCharNumber( const u16 *cp_str, int number, HEAPID heapID );

//=============================================================================
/**
 *          データ
 */
//=============================================================================
//-------------------------------------
/// オーラチェック用プロックデータ
//=====================================
const GFL_PROC_DATA IrcAura_ProcData  =
{
  IRC_AURA_PROC_Init,
  IRC_AURA_PROC_Main,
  IRC_AURA_PROC_Exit,
};
//-------------------------------------
/// BGシステム
//=====================================
typedef enum
{
  GRAPHIC_BG_FRAME_M_INFOWIN,
  GRAPHIC_BG_FRAME_M_GUIDE,
  GRAPHIC_BG_FRAME_M_BACK,
  GRAPHIC_BG_FRAME_S_ROGO,
  GRAPHIC_BG_FRAME_S_TEXT,
  GRAPHIC_BG_FRAME_S_BACK,
  GRAPHIC_BG_FRAME_S_TITLE,
  GRAPHIC_BG_FRAME_MAX
} GRAPHIC_BG_FRAME;
static const u32 sc_bgcnt_frame[ GRAPHIC_BG_FRAME_MAX ] =
{
  GFL_BG_FRAME1_M, GFL_BG_FRAME2_M, GFL_BG_FRAME3_M, GFL_BG_FRAME0_S, GFL_BG_FRAME1_S,GFL_BG_FRAME2_S,GFL_BG_FRAME3_S
};
static const u32 sc_bgcnt_mode[ GRAPHIC_BG_FRAME_MAX ] =
{
  GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_AFFINE,
};
static const GFL_BG_BGCNT_HEADER sc_bgcnt_data[ GRAPHIC_BG_FRAME_MAX ] =
{
  // GRAPHIC_BG_FRAME_M_INFOWIN
  {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  },
  // GRAPHIC_BG_FRAME_M_GUIDE
  {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
    GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
  },
  // GRAPHIC_BG_FRAME_M_BACK
  {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x1800, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
    GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
  },
  // GRAPHIC_BG_FRAME_S_ROGO
  {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
    GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
  },
  // GRAPHIC_BG_FRAME_S_TEXT
  {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
    GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
  },
  // GRAPHIC_BG_FRAME_S_BACK
  {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
    GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
  },
  // GRAPHIC_BG_FRAME_S_TITLE
  {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
    GX_BG_SCRBASE_0x1800, GX_BG_CHARBASE_0x14000, GFL_BG_CHRSIZ256_128x128,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  },
};

//-------------------------------------
/// タッチ範囲
//=====================================
static const GFL_RECT sc_left =
{
  0, 2*8, 11*8, 20*8
};
#if 0
static const GFL_RECT sc_right  =
{
  19, 2, 22, 20
};
#else //2点タッチなので、上の座標と左座標の中点
static const GFL_RECT sc_right  =
{
  11*8+1, 2*8, 22*8, 20*8
};
#endif

//-------------------------------------
/// NET
//=====================================
enum
{
  NETRECV_RESULT  = GFL_NET_CMD_IRCAURA,
};
static const NetRecvFuncTable sc_recv_tbl[] =
{
  {
    NETRECV_Result, NETRECV_GetBufferAddr
  }
};

//=============================================================================
/**
 *      PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  オーラチェック  メインプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_AURA_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{
  AURA_MAIN_WORK  *p_wk;

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCAURA, 0x25000 );
  //プロセスワーク作成
  p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(AURA_MAIN_WORK), HEAPID_IRCAURA );
  GFL_STD_MemClear( p_wk, sizeof(AURA_MAIN_WORK) );
  p_wk->p_param = p_param;
  p_wk->p_param->minus  = 0;

#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
    s_is_debug_only_play  = p_wk->p_param->is_only_play;
#endif// DEBUG_IRC_COMPATIBLE_ONLYPLAY

  //モジュール初期化
  AURANET_Init( &p_wk->net, p_wk->p_param->p_irc );
  GRAPHIC_Init( &p_wk->grp, HEAPID_IRCAURA );
  MSG_Init( &p_wk->msg, MSG_FONT_TYPE_LARGE, HEAPID_IRCAURA );
  {
    GAME_COMM_SYS_PTR comm  = NULL;
    if( p_wk->p_param->p_gamesys )
    {
      comm  = GAMESYSTEM_GetGameCommSysPtr(p_wk->p_param->p_gamesys);
    }
    INFOWIN_Init( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_INFOWIN], AURA_BG_PAL_M_15, comm, HEAPID_IRCAURA );
  }
  MSGWND_Init( &p_wk->msgwnd[MSGWNDID_TEXT], sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TEXT],
      MSGWND_TEXT_X, MSGWND_TEXT_Y, MSGWND_TEXT_W, MSGWND_TEXT_H, AURA_BG_PAL_S_08, HEAPID_IRCAURA );
  BmpWinFrame_Write( p_wk->msgwnd[MSGWNDID_TEXT].p_bmpwin, WINDOW_TRANS_ON,
          GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->grp.bg.frame_char), AURA_BG_PAL_S_06 );

  //タイトル文字列作成
  MSGWND_InitEx( &p_wk->msgtitle, sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE],
      MSGWND_TITLE_X, MSGWND_TITLE_Y, MSGWND_TITLE_W, MSGWND_TITLE_H, AURA_BG_PAL_S_08, 1, GFL_BMP_CHRAREA_GET_B, HEAPID_IRCAURA );
  MSGWND_PrintCenter( &p_wk->msgtitle, &p_wk->msg, AURA_TITLE_000 );
  GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_SCALE_X_SET, TITLE_STR_SCALE_X );
  GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_SCALE_Y_SET, TITLE_STR_SCALE_Y );
  GFL_BG_SetRotateCenterReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_CENTER_X_SET, (MSGWND_TITLE_X + MSGWND_TITLE_W/2)*8 );
  GFL_BG_SetRotateCenterReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_CENTER_Y_SET, (MSGWND_TITLE_Y + MSGWND_TITLE_H/2)*8 );

  //ふれはば検知初期化
  {
    int i;
    for( i = 0; i < DEBUG_PLAYER_SAVE_NUM; i++ )
    {
      SHAKESEARCH_Init( &p_wk->shake_left[ i ] );
      SHAKESEARCH_Init( &p_wk->shake_right[ i ] );
    }
  }

  BACKOBJ_Init( &p_wk->backobj, &p_wk->grp, BACKOBJ_MOVE_TYPE_RAIN, BACKOBJ_COLOR_RED, CLWKID_BACKOBJ_TOP_S, CLSYS_DEFREND_SUB );

  //APPBAR
  {
    GFL_CLUNIT  *p_unit = GRAPHIC_GetClunit( &p_wk->grp );
    p_wk->p_appbar  = APPBAR_Init( APPBAR_OPTION_MASK_RETURN, p_unit, sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_INFOWIN], AURA_BG_PAL_M_13, AURA_OBJ_PAL_M_13, APP_COMMON_MAPPING_128K, MSG_GetFont(&p_wk->msg ), MSG_GetPrintQue(&p_wk->msg ), HEAPID_IRCAURA );
  }

  //オーラシーンセット
  COMPATIBLE_IRC_SetScene( p_wk->p_param->p_irc, COMPATIBLE_SCENE_AURA );

  SEQ_Change( p_wk, SEQFUNC_StartGame );

  //オーラがかかっては不味いのでウィンドウで隠す
  GX_SetVisibleWnd(GX_WNDMASK_W0 | GX_WNDMASK_W1);
  G2_SetWnd0InsidePlane(
      GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 |
      GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ,
      FALSE );
  G2_SetWnd1InsidePlane(
      GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 |
      GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ,
      FALSE );
  G2_SetWndOutsidePlane( GX_WND_PLANEMASK_BG0 |
      GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 |
      GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ,
      FALSE );
  G2_SetWnd0Position(0, 0, 255, 16);
  G2_SetWnd1Position(0, 192-24, 255, 192);

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  オーラチェック  メインプロセス破棄処理
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_AURA_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{
  AURA_MAIN_WORK  *p_wk;

  p_wk  = p_work;

  //ウィンドウ解除
  GX_SetVisibleWnd(GX_WNDMASK_NONE);


  //APPBAR
  APPBAR_Exit( p_wk->p_appbar );

  BACKOBJ_Exit( &p_wk->backobj );

  //モジュール破棄
  MSGWND_Exit( &p_wk->msgtitle );
  {
    int i;
    for( i = 0; i < MSGWNDID_MAX; i++ )
    {
      MSGWND_Exit( &p_wk->msgwnd[i] );
    }
  }
  INFOWIN_Exit();
  GRAPHIC_Exit( &p_wk->grp );
  MSG_Exit( &p_wk->msg );
  AURANET_Exit( &p_wk->net );

  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );
  //ヒープ破棄
  GFL_HEAP_DeleteHeap( HEAPID_IRCAURA );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  オーラチェック  メインプロセスメイン処理
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_AURA_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{
  enum
  {
    SEQ_INIT,
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_MAIN,
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_EXIT,
  };

  AURA_MAIN_WORK  *p_wk;
  p_wk  = p_work;

  switch( *p_seq )
  {
  case SEQ_INIT:
    *p_seq  = SEQ_FADEOUT_START;
    break;

  case SEQ_FADEOUT_START:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;

  case SEQ_FADEOUT_WAIT:
    if( !GFL_FADE_CheckFade() )
    {
      *p_seq  = SEQ_MAIN;
    }
    break;

  case SEQ_MAIN:

    p_wk->seq_function( p_wk, &p_wk->seq );
    if( p_wk->is_end )
    {
      *p_seq  = SEQ_FADEIN_START;
    }

    break;

  case SEQ_FADEIN_START:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    *p_seq  = SEQ_FADEIN_WAIT;
    break;

  case SEQ_FADEIN_WAIT:
    if( !GFL_FADE_CheckFade() )
    {
      *p_seq  = SEQ_EXIT;
    }
    break;

  case SEQ_EXIT:
    return GFL_PROC_RES_FINISH;

  default:
    GF_ASSERT_MSG( 0, "IRC_AURA_PROC_MainのSEQエラー %d", *p_seq );
  }

  INFOWIN_Update();
  if( MSG_Main( &p_wk->msg ) )
  {
    int i;
    for( i = 0; i < MSGWNDID_MAX; i++ )
    {
      MSGWND_Main( &p_wk->msgwnd[i], &p_wk->msg );
      MSGWND_Main( &p_wk->msgtitle, &p_wk->msg );
    }
  }
  //1たん停止
  //BACKOBJ_Main( &p_wk->backobj );
  GRAPHIC_Draw( &p_wk->grp );

  //シーンを継続的に送る
  COMPATIBLE_IRC_SendSceneContinue( p_wk->p_param->p_irc );

  APPBAR_Main( p_wk->p_appbar );

#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
  {
    s_pos.y   += 1;
    NAGI_Printf( "Dpos X%d Y%d\n", s_pos.x, s_pos.y );
  }
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
  {
    s_pos.y   -= 1;
    NAGI_Printf( "Dpos X%d Y%d\n", s_pos.x, s_pos.y );
  }
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
  {
    s_pos.x   -= 1;
    NAGI_Printf( "Dpos X%d Y%d\n", s_pos.x, s_pos.y );
  }
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
  {
    s_pos.x   += 1;
    NAGI_Printf( "Dpos X%d Y%d\n", s_pos.x, s_pos.y );
  }

#endif


  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *        GRAPHIC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  描画関係初期化
 *
 *  @param  GRAPHIC_WORK* p_wk  ワーク
 *  @param  heapID          ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Init( GRAPHIC_WORK* p_wk, HEAPID heapID )
{
  static const GFL_DISP_VRAM sc_vramSetTable =
  {
    GX_VRAM_BG_128_A,           // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_64_E,           // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_128_D,       // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_NONE,           // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_NONE,       // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_128K,
    GX_OBJVRAMMODE_CHAR_1D_128K,
  };

  //ワーククリア
  GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );

  //VRAMクリアー
  GFL_DISP_ClearVRAM( 0 );

  // VRAMバンク設定
  GFL_DISP_SetBank( &sc_vramSetTable );

  // ディスプレイON
  GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
  GFL_DISP_SetDispOn();

  //表示
  GFL_DISP_GX_InitVisibleControl();

  //描画モジュール
  GRAPHIC_OBJ_Init( &p_wk->obj, &sc_vramSetTable, heapID );
  GRAPHIC_BG_Init( &p_wk->bg, heapID );
  GRAPHIC_3D_Init( &p_wk->g3d, heapID );

  TOUCH_EFFECT_Init( &p_wk->touch_eff, &p_wk->g3d, heapID );

  //VBlackTask登録
  p_wk->p_vblank_task = GFUser_VIntr_CreateTCB(Graphic_VBlankTask, p_wk, 0 );
}


//----------------------------------------------------------------------------
/**
 *  @brief  描画関係破棄
 *
 *  @param  GRAPHIC_WORK* p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Exit( GRAPHIC_WORK* p_wk )
{

  GFL_TCB_DeleteTask( p_wk->p_vblank_task );

  TOUCH_EFFECT_Exit( &p_wk->touch_eff );

  GRAPHIC_3D_Exit( &p_wk->g3d );
  GRAPHIC_BG_Exit( &p_wk->bg );
  GRAPHIC_OBJ_Exit( &p_wk->obj );
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画関係描画処理
 *
 *  @param  GRAPHIC_WORK* p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Draw( GRAPHIC_WORK* p_wk )
{
  GRAPHIC_OBJ_Main( &p_wk->obj );

  GRAPHIC_3D_StartDraw( &p_wk->g3d );
/*  //NNS系の3D描画
  //なし
  NNS_G3dGeFlushBuffer();
  //SDK系の3D描画
*/
  TOUCH_EFFECT_Draw( &p_wk->touch_eff );

  GRAPHIC_3D_EndDraw( &p_wk->g3d );

}
//----------------------------------------------------------------------------
/**
 *  @brief  CLWK取得
 *
 *  @param  const GRAPHIC_WORK *cp_wk ワーク
 *  @param  id                        CLWKのID
 *
 *  @return CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK* GRAPHIC_GetClwk( const GRAPHIC_WORK *cp_wk, CLWKID id )
{
  return GRAPHIC_OBJ_GetClwk( &cp_wk->obj, id );
}

//----------------------------------------------------------------------------
/**
 *  @brief  CLUNIT取得
 *
 *  @param  const GRAPHIC_WORK *cp_wk   ワーク
 *
 *  @return CLUNIT
 */
//-----------------------------------------------------------------------------
static GFL_CLUNIT * GRAPHIC_GetClunit( const GRAPHIC_WORK *cp_wk )
{
  return GRAPHIC_OBJ_GetClunit( &cp_wk->obj );
}
//----------------------------------------------------------------------------
/**
 *  @brief  タッチ演出ワーク取得
 *
 *  @param  const GRAPHIC_WORK *cp_wk ワーク
 *  @param  id                        ID
 *
 *  @return タッチ演出ワーク取得
 */
//-----------------------------------------------------------------------------
static TOUCH_EFFECT_SYS * GRAPHIC_GetTouchEffWk( GRAPHIC_WORK *p_wk )
{
  return &p_wk->touch_eff;
}
//----------------------------------------------------------------------------
/**
 *  @brief  枠描画
 *
 *  @param  const GRAPHIC_WORK *cp_wk ワーク
 *  @param  *p_bmpwin                 BMPWIN
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_WriteBmpwinFrame( GRAPHIC_WORK *p_wk, GFL_BMPWIN *p_bmpwin )
{
  GRAPHIC_BG_WriteBmpwinFrame( &p_wk->bg, p_bmpwin );
}
//----------------------------------------------------------------------------
/**
 *  @brief  VBlank用タスク
 *
 *  @param  GFL_TCB *p_tcb  タスクControlブロック
 *  @param  *p_work         ワークアドレス
 *
 */
//-----------------------------------------------------------------------------
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work )
{
  GRAPHIC_WORK* p_wk  = p_work;
  GRAPHIC_BG_VBlankFunction( &p_wk->bg );
  GRAPHIC_OBJ_VBlankFunction( &p_wk->obj );
}

//=============================================================================
/**
 *          GRAPHIC_BG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  BG描画  初期化
 *
 *  @param  GRAPHIC_BG_WORK* p_wk ワーク
 *  @param  heapID            ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK* p_wk, HEAPID heapID )
{
  int i;

  //ＢＧシステム起動
  GFL_BG_Init( heapID );
  GFL_BMPWIN_Init( heapID );

  //ＢＧモード設定
  {
    static const GFL_BG_SYS_HEADER sc_bg_sys_header =
    {
      GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_1,GX_BG0_AS_3D
    };
    GFL_BG_SetBGMode( &sc_bg_sys_header );
  }

  //ＢＧコントロール設定
  {
    int i;

    for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
    {
      GFL_BG_SetBGControl( sc_bgcnt_frame[i], &sc_bgcnt_data[i], sc_bgcnt_mode[i] );
      GFL_BG_ClearFrame( sc_bgcnt_frame[i] );
      GFL_BG_SetVisible( sc_bgcnt_frame[i], VISIBLE_ON );
    }
  }

  //読み込み設定
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_IRCCOMPATIBLE, heapID );

    //パレット
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_irccompatible_gra_aura_bg_NCLR,
        PALTYPE_MAIN_BG, AURA_BG_PAL_M_00*0x20, 0, heapID );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_irccompatible_gra_aura_bg_NCLR,
        PALTYPE_SUB_BG, AURA_BG_PAL_S_00*0x20, 0, heapID );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_irccompatible_gra_aura_bg_guide_NCLR,
        PALTYPE_MAIN_BG, AURA_BG_PAL_M_09*0x20, 0, heapID );

    //キャラ
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_aura_bg_b_NCGR,
        sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_BACK], 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_aura_bg_NCGR,
        sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_BACK], 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_title_rogo_NCGR,
        sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_ROGO], 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_aura_bg_guide_NCGR,
        sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_GUIDE], 0, 0, FALSE, heapID );

    //スクリーン
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_aura_bg_b_NSCR,
        sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_BACK], 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_aura_bg_block_b_NSCR,
        sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_BACK], 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_title_aura_NSCR,
        sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_ROGO], 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_aura_bg_guide_l_NSCR,
        sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_GUIDE], 0, 0, FALSE, heapID );

    GUIDE_SetVisible( GUIDE_VISIBLE_LEFT, heapID );

    //ワク
    GFL_BG_FillCharacter( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT], 0, 1, 0 );
    p_wk->frame_char  = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_irccompatible_gra_ue_frame_NCGR, sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT], 0, FALSE, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }

  //3dは半透明
  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG2,GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3, 13, 8 );
}

//----------------------------------------------------------------------------
/**
 *  @brief  BG描画  破棄
 *
 *  @param  GRAPHIC_BG_WORK* p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK* p_wk )
{
  int i;

  //アルファ
  G2_BlendNone();

  //読み込み破棄
  {
    GFL_BG_FreeCharacterArea(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT],
        GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->frame_char),
        GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->frame_char));
    GFL_BG_FillCharacterRelease(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT], 1,0);
  }

  // BGコントロール破棄
  for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
  {
    GFL_BG_FreeBGControl( sc_bgcnt_frame[i] );
  }

  // BGシステム破棄
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

//----------------------------------------------------------------------------
/**
 *  @brief  窓枠を書き込み
 *
 *  @param  GRAPHIC_BG_WORK *p_wk ワーク
 *  @param  *p_bmpwin             書き込むBMPWIN
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_WriteBmpwinFrame( GRAPHIC_BG_WORK *p_wk, GFL_BMPWIN *p_bmpwin )
{
  BmpWinFrame_Write( p_bmpwin, WINDOW_TRANS_ON,
          GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->frame_char), AURA_BG_PAL_M_01 );
}

//----------------------------------------------------------------------------
/**
 *  @brief  BG描画  VBlack関数
 *
 *  @param  GRAPHIC_BG_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk )
{
  GFL_BG_VBlankFunc();
}
//=============================================================================
/**
 *        OBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  OBJ描画 初期化
 *
 *  @param  GRAPHIC_OBJ_WORK *p_wk      ワーク
 *  @param  GFL_DISP_VRAM* cp_vram_bank バンクテーブル
 *  @param  heapID                      ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID )
{
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_OBJ_WORK) );

  //システム作成
  GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN, cp_vram_bank, heapID );
  p_wk->p_clunit  = GFL_CLACT_UNIT_Create( 128, 0, heapID );
  GFL_CLACT_UNIT_SetDefaultRend( p_wk->p_clunit );

  //表示
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

  //リソース読み込み
  {
    ARCHANDLE *p_handle;

    p_handle  = GFL_ARC_OpenDataHandle( ARCID_IRCCOMPATIBLE, heapID );

    p_wk->reg_id[OBJREGID_TOUCH_PLT]  = GFL_CLGRP_PLTT_Register( p_handle,
        NARC_irccompatible_gra_aura_obj_NCLR, CLSYS_DRAW_SUB, AURA_OBJ_PAL_S_00*0x20, heapID );

    p_wk->reg_id[OBJREGID_TOUCH_CHR]  = GFL_CLGRP_CGR_Register( p_handle,
        NARC_irccompatible_gra_aura_obj_NCGR, FALSE, CLSYS_DRAW_SUB, heapID );

    p_wk->reg_id[OBJREGID_TOUCH_CEL]  = GFL_CLGRP_CELLANIM_Register( p_handle,
        NARC_irccompatible_gra_aura_obj_NCER, NARC_irccompatible_gra_aura_obj_NANR, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }

  //CLWK作成
  {
    int i;
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    for( i = CLWKID_BACKOBJ_TOP_S; i < CLWKID_BACKOBJ_END_S; i++ )
    {
      p_wk->p_clwk[i] = GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->reg_id[OBJREGID_TOUCH_CHR],
          p_wk->reg_id[OBJREGID_TOUCH_PLT],
          p_wk->reg_id[OBJREGID_TOUCH_CEL],
          &cldata,
          CLSYS_DEFREND_SUB,
          heapID
          );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
      GFL_CLACT_WK_SetBgPri( p_wk->p_clwk[i], 3 );
      GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[i], TRUE );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i], 0 );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  OBJ描画 破棄
 *
 *  @param  GRAPHIC_OBJ_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk )
{
  //セル破棄
  {
    int i;
    for( i = 0; i < CLWKID_MAX; i++ )
    {
      if( p_wk->p_clwk[i] )
      {
        GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
      }
    }
  }

  //リソース破棄
  {
    GFL_CLGRP_PLTT_Release( p_wk->reg_id[OBJREGID_TOUCH_PLT] );
    GFL_CLGRP_CGR_Release( p_wk->reg_id[OBJREGID_TOUCH_CHR] );
    GFL_CLGRP_CELLANIM_Release( p_wk->reg_id[OBJREGID_TOUCH_CEL] );
  }


  //システム破棄
  GFL_CLACT_UNIT_Delete( p_wk->p_clunit );
  GFL_CLACT_SYS_Delete();
  GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_OBJ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  OBJ描画 メイン処理
 *
 *  @param  GRAPHIC_OBJ_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk )
{
  GFL_CLACT_SYS_Main();

}
//----------------------------------------------------------------------------
/**
 *  @brief  OBJ描画 Vブランク処理
 *
 *  @param  GRAPHIC_OBJ_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk )
{
  GFL_CLACT_SYS_VBlankFunc();
}

//----------------------------------------------------------------------------
/**
 *  @brief  OBJ描画 CLWK取得
 *
 *  @param  const GRAPHIC_OBJ_WORK *cp_wk ワーク
 *  @param  id                            CLWKのID
 *
 *  @return CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK* GRAPHIC_OBJ_GetClwk( const GRAPHIC_OBJ_WORK *cp_wk, CLWKID id )
{
  GF_ASSERT( id < CLWKID_MAX );
  return cp_wk->p_clwk[id];
}

//----------------------------------------------------------------------------
/**
 *  @brief  OBJ描画 CLUNIT取得
 *
 *  @param  const GRAPHIC_OBJ_WORK *cp_wk   ワーク
 *
 *  @return CLUNIT
 */
//-----------------------------------------------------------------------------
static GFL_CLUNIT * GRAPHIC_OBJ_GetClunit( const GRAPHIC_OBJ_WORK *cp_wk )
{
  return cp_wk->p_clunit;
}
//=============================================================================
/**
 *          GRAPHIC_3D
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  ３D環境の初期化
 *
 *  @param  p_wk      ワーク
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_Init( GRAPHIC_3D_WORK *p_wk, HEAPID heapID )
{
  static const VecFx32 sc_CAMERA_PER_POS    = { 0,0,FX32_CONST( 70 ) };
  static const VecFx32 sc_CAMERA_PER_UP     = { 0,FX32_ONE,0 };
  static const VecFx32 sc_CAMERA_PER_TARGET = { 0,0,FX32_CONST( 0 ) };

  enum
  {
    CAMERA_PER_FOVY   = (40),
    CAMERA_PER_ASPECT = (FX32_ONE * 4 / 3),
    CAMERA_PER_NEAR   = (FX32_ONE * 1),
    CAMERA_PER_FER    = (FX32_ONE * 1000),
    CAMERA_PER_SCALEW =(0),
  };

  GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K,
      GFL_G3D_VMANLNK, GFL_G3D_PLT32K, 0, heapID, Graphic_3d_SetUp );

#if 0
  //射影
  p_wk->p_camera = GFL_G3D_CAMERA_CreatePerspective( CAMERA_PER_FOVY, CAMERA_PER_ASPECT,
        CAMERA_PER_NEAR, CAMERA_PER_FER, CAMERA_PER_SCALEW,
        &sc_CAMERA_PER_POS, &sc_CAMERA_PER_UP, &sc_CAMERA_PER_TARGET, heapID );
#else
  //正射影
  p_wk->p_camera = GFL_G3D_CAMERA_CreateOrtho(
    // const fx32 top, const fx32 bottom, const fx32 left, const fx32 right,
      FX32_CONST(24), -FX32_CONST(24), -FX32_CONST(32), FX32_CONST(32),
        CAMERA_PER_NEAR, CAMERA_PER_FER, CAMERA_PER_SCALEW,
      &sc_CAMERA_PER_POS, &sc_CAMERA_PER_UP, &sc_CAMERA_PER_TARGET, heapID );
#endif

  //読み込み
  {
    static const G3D_MDL_SETUP  sc_aura_setup =
    {
      NARC_irccompatible_gra_aura_nsbmd,
      {
        NARC_irccompatible_gra_aura_nsbca,
        NARC_irccompatible_gra_aura_nsbma,
      }
    };
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_IRCCOMPATIBLE, heapID );

    G3DMDL_Load( &p_wk->mdl[0], p_handle, &sc_aura_setup, heapID );
    G3DMDL_Load( &p_wk->mdl[1], p_handle, &sc_aura_setup, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ３D環境の破棄
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_Exit( GRAPHIC_3D_WORK *p_wk )
{
  //解放
  {
    int i;
    for( i = 0; i < G3D_MDL_MAX; i++ )
    {
      G3DMDL_UnLoad( &p_wk->mdl[i] );
    }
  }

  GFL_G3D_CAMERA_Delete( p_wk->p_camera );
  GFL_G3D_Exit();
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画開始
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_StartDraw( GRAPHIC_3D_WORK *p_wk )
{

  GFL_G3D_DRAW_Start();
  GFL_G3D_CAMERA_Switching( p_wk->p_camera );
  GFL_G3D_DRAW_SetLookAt();

}

//----------------------------------------------------------------------------
/**
 *  @brief  描画終了
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_EndDraw( GRAPHIC_3D_WORK *p_wk )
{
  GFL_G3D_DRAW_End();
}

//----------------------------------------------------------------------------
/**
 *  @brief  モデル取得
 *
 *  @param  const GRAPHIC_3D_WORK *cp_wk  ワーク
 *
 *  @return モデル
 */
//-----------------------------------------------------------------------------
static G3D_MDL_WORK *GRAPHIC_3D_GetObj( const GRAPHIC_3D_WORK *cp_wk, u16 idx )
{
  return (G3D_MDL_WORK*)(&cp_wk->mdl[ idx ]);
}
//----------------------------------------------------------------------------
/**
 *  @brief  環境セットアップコールバック関数
 */
//-----------------------------------------------------------------------------
static void Graphic_3d_SetUp( void )
{
  // ３Ｄ使用面の設定(表示＆プライオリティー)
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
  G2_SetBG0Priority(0);

  // 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
  G3X_SetShading( GX_SHADING_HIGHLIGHT );
  G3X_AntiAlias( FALSE );
  G3X_AlphaTest( FALSE, 0 );  // アルファテスト　　オフ
  G3X_AlphaBlend( TRUE );   // アルファブレンド　オン
  G3X_EdgeMarking( FALSE );
  G3X_SetFog( FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );

  // クリアカラーの設定
  G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE); //color,alpha,depth,polygonID,fog
  // ビューポートの設定
  G3_ViewPort(0, 0, 255, 191);

  // ライト設定
  {
    static const GFL_G3D_LIGHT sc_GFL_G3D_LIGHT[] =
    {
      {
        { 0, -FX16_ONE, 0 },
        GX_RGB( 16,16,16),
      },
      {
        { 0, FX16_ONE, 0 },
        GX_RGB( 16,16,16),
      },
      {
        { 0, -FX16_ONE, 0 },
        GX_RGB( 16,16,16),
      },
      {
        { 0, -FX16_ONE, 0 },
        GX_RGB( 16,16,16),
      },
    };
    int i;

    for( i=0; i<NELEMS(sc_GFL_G3D_LIGHT); i++ ){
      GFL_G3D_SetSystemLight( i, &sc_GFL_G3D_LIGHT[i] );
    }
  }

  //レンダリングスワップバッファ
  GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
}
//=============================================================================
/**
 *        3DMDL
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  3Dモデル読み込み
 *
 *  @param  G3D_MDL_WORK *p_wk  ワーク
 *  @param  *p_handle           ハンドル
 *  @param  dataID              データID
 *  @param  heapID              ヒープID
 */
//-----------------------------------------------------------------------------
static void G3DMDL_Load( G3D_MDL_WORK *p_wk, ARCHANDLE *p_handle, const G3D_MDL_SETUP *cp_setup, HEAPID heapID )
{
  //ワーククリア
  GFL_STD_MemClear(p_wk, sizeof(G3D_MDL_WORK));

  //モデル読み込み
  {
    BOOL      result;

    GFL_G3D_RES *p_tex  = NULL;
    GFL_G3D_RES *p_mdl  = NULL;

    p_wk->p_res = GFL_G3D_CreateResourceHandle( p_handle, cp_setup->imdID );
    GF_ASSERT( p_wk->p_res != NULL );

    if( GFL_G3D_CheckResourceType( p_wk->p_res, GFL_G3D_RES_CHKTYPE_TEX ) ){
      p_tex = p_wk->p_res;
      result  = GFL_G3D_TransVramTexture( p_tex );
      GF_ASSERT( result );
    }

    if( GFL_G3D_CheckResourceType( p_wk->p_res, GFL_G3D_RES_CHKTYPE_MDL ) )
    {
      int i;

      p_mdl = p_wk->p_res;
      p_wk->p_rnd = GFL_G3D_RENDER_Create( p_mdl, 0, p_tex );

      // アニメーション作成
      for( i = 0; i < G3D_MDL_ANM_MAX; i++ )
      {
        p_wk->p_anm_res[i]  = GFL_G3D_CreateResourceHandle( p_handle, cp_setup->anmID[i] );

        p_wk->p_anm[i]      = GFL_G3D_ANIME_Create( p_wk->p_rnd, p_wk->p_anm_res[i], 0 );
      }

      //  OBJ作成
      p_wk->p_obj = GFL_G3D_OBJECT_Create( p_wk->p_rnd, p_wk->p_anm, G3D_MDL_ANM_MAX );

      // アニメーション反映
      for( i=0; i<G3D_MDL_ANM_MAX; i++ )
      {
        GFL_G3D_OBJECT_EnableAnime( p_wk->p_obj, i );
      }
    }

    //アルファを保存しておく
    //p_wk->alpha = NNS_G3dMdlGetMdlAlpha( G3D_OBJECT_GetMdl( p_wk->p_obj ), 0 );
  }

  //座標
  {
    MTX_Identity33( &p_wk->status.rotate );
    VEC_Set( &p_wk->status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  解放
 *
 *  @param  G3D_MDL_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void G3DMDL_UnLoad( G3D_MDL_WORK *p_wk )
{
  int i;

  //OBJ破棄
  GFL_G3D_OBJECT_Delete( p_wk->p_obj );
  for( i = 0; i<G3D_MDL_ANM_MAX; i++ )
  {
    if( p_wk->p_anm[i] )
    {
      GFL_G3D_ANIME_Delete( p_wk->p_anm[i] );
    }
  }
  GFL_G3D_RENDER_Delete( p_wk->p_rnd );

  //リソース破棄
  for( i = 0; i<G3D_MDL_ANM_MAX; i++ )
  {
    GFL_G3D_DeleteResource( p_wk->p_anm_res[i] );
  }
  GFL_G3D_DeleteResource( p_wk->p_res );

  //クリア
  GFL_STD_MemClear(p_wk, sizeof(G3D_MDL_WORK));
}
//----------------------------------------------------------------------------
/**
 *  @brief  描画
 *
 *  @param  G3D_MDL_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void G3DMDL_Draw( G3D_MDL_WORK *p_wk )
{
  if( p_wk->is_visible )
  {
    int i;
    int frame;
    for( i = 0; i<G3D_MDL_ANM_MAX; i++ )
    {
      GFL_G3D_OBJECT_LoopAnimeFrame( p_wk->p_obj, i, FX32_ONE);
    }
    GFL_G3D_OBJECT_GetAnimeFrame( p_wk->p_obj, 0, &frame );
    p_wk->is_end_loop = frame == 0;

    GFL_G3D_DRAW_DrawObject( p_wk->p_obj, &p_wk->status );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  アニメエンド
 *
 *  @param  const G3D_MDL_WORK *cp_wk   ワーク
 *
 *  @return TRUEループ終了  FALSEループ中
 */
//-----------------------------------------------------------------------------
static BOOL G3DMDL_IsLoopEnd( const G3D_MDL_WORK *cp_wk )
{
  return cp_wk->is_end_loop;
}
//----------------------------------------------------------------------------
/**
 *  @brief  座標設定
 *
 *  @param  G3D_MDL_WORK *p_wk  ワーク
 *  @param  VecFx32 *cp_trans   座標
 */
//-----------------------------------------------------------------------------
static void G3DMDL_SetPos( G3D_MDL_WORK *p_wk, const VecFx32 *cp_trans )
{
  p_wk->status.trans  = *cp_trans;
}
//----------------------------------------------------------------------------
/**
 *  @brief  スケール設定
 *
 *  @param  G3D_MDL_WORK *p_wk  ワーク
 *  @param  VecFx32 *cp_scale   スケール
 */
//-----------------------------------------------------------------------------
static void G3DMDL_SetScale( G3D_MDL_WORK *p_wk, const VecFx32 *cp_scale )
{
  p_wk->status.scale  = *cp_scale;
}
//----------------------------------------------------------------------------
/**
 *  @brief  表示フラグ設定
 *
 *  @param  G3D_MDL_WORK *p_wk  ワーク
 *  @param  is_visible        TRUEで描画FALSEで非表示
 */
//-----------------------------------------------------------------------------
static void G3DMDL_SetVisible( G3D_MDL_WORK *p_wk, BOOL is_visible )
{
  if( p_wk->is_visible != is_visible )
  {
    GFL_G3D_OBJECT_ResetAnimeFrame( p_wk->p_obj, 0 );
    p_wk->is_visible  = is_visible;
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  表示フラグ取得
 *
 *  @param  const G3D_MDL_WORK *cp_wk ワーク
 *
 *  @return TRUEで表示  FALSEで非表示
 */
//-----------------------------------------------------------------------------
static BOOL G3DMDL_GetVisible( const G3D_MDL_WORK *cp_wk )
{
  return cp_wk->is_visible;
}
//=============================================================================
/**
 *          MSG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  MSG関係を設定
 *
 *  @param  MSG_WORK *p_wk  ワーク
 *  @param  font            フォントタイプ
 *  @param  heapID          ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSG_Init( MSG_WORK *p_wk, MSG_FONT_TYPE font, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(MSG_WORK) );

  GFL_FONTSYS_Init();

  switch( font )
  {
  case MSG_FONT_TYPE_LARGE:
    p_wk->p_font  = GFL_FONT_Create( ARCID_FONT,
        NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
    break;
  default:
    GF_ASSERT_MSG( 0, "MSGFONT_ERRO %d", font );
  };

  p_wk->p_print_que = PRINTSYS_QUE_Create( heapID );

  p_wk->p_msg = GFL_MSG_Create(
    GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_irc_aura_dat, heapID );

  p_wk->p_wordset = WORDSET_Create( heapID );

  {
/*    GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
    GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
*/  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  MSG関係を破棄
 *
 *  @param  MSG_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MSG_Exit( MSG_WORK *p_wk )
{
  WORDSET_Delete( p_wk->p_wordset );

  GFL_MSG_Delete( p_wk->p_msg );

  PRINTSYS_QUE_Delete( p_wk->p_print_que );

  GFL_FONT_Delete( p_wk->p_font );
  GFL_STD_MemClear( p_wk, sizeof(MSG_WORK) );
}

//----------------------------------------------------------------------------
/**
 *  @brief  MSG関係 メイン処理
 *
 *  @param  MSG_WORK *p_wk  ワーク
 *
 * @retval  BOOL  処理が終了していればTRUE／それ以外はFALSE
 *
 */
//-----------------------------------------------------------------------------
static BOOL MSG_Main( MSG_WORK *p_wk )
{
  return PRINTSYS_QUE_Main( p_wk->p_print_que );
}

//----------------------------------------------------------------------------
/**
 *  @brief  FONTを取得
 *
 *  @param  const MSG_WORK *cp_wk   ワーク
 *
 *  @return FONT
 */
//-----------------------------------------------------------------------------
static GFL_FONT*  MSG_GetFont( const MSG_WORK *cp_wk )
{
  return cp_wk->p_font;
}

//----------------------------------------------------------------------------
/**
 *  @brief  PRINTQUEを取得
 *
 *  @param  const MSG_WORK *cp_wk   ワーク
 *
 *  @return PRINTQUE
 */
//-----------------------------------------------------------------------------
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk )
{
  return cp_wk->p_print_que;
}
//----------------------------------------------------------------------------
/**
 *  @brief  MSGDATAを取得
 *
 *  @param  const MSG_WORK *cp_wk   ワーク
 *
 *  @return MSGDATA
 */
//-----------------------------------------------------------------------------
static const GFL_MSGDATA * MSG_GetMsgDataConst( const MSG_WORK *cp_wk )
{
  return cp_wk->p_msg;
}

//----------------------------------------------------------------------------
/**
 *  @brief  WORDSETを取得
 *
 *  @param  const MSG_WORK *cp_wk ワーク
 *
 *  @return WORDSET
 */
//-----------------------------------------------------------------------------
static WORDSET * MSG_GetWordSet( const MSG_WORK *cp_wk )
{
  return cp_wk->p_wordset;
}

//=============================================================================
/**
 *      MSGWND
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ表示面  初期化
 *
 *  @param  MSGWND_WORK* p_wk ワーク
 *  @param  bgframe           表示するBG面
 *  @param  x                 開始X位置（キャラ単位）
 *  @param  y                 開始Y位置（キャラ単位）
 *  @param  w                 幅（キャラ単位）
 *  @param  h                 高さ（キャラ単位）
 *  @param  heapID            ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe, u8 x, u8 y, u8 w, u8 h, u8 plt, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
  p_wk->clear_chr = 0xF;
  p_wk->p_bmpwin  = GFL_BMPWIN_Create( bgframe, x, y, w, h, plt, GFL_BMP_CHRAREA_GET_B );
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( TEXTSTR_BUFFER_LENGTH, heapID );
  PRINT_UTIL_Setup( &p_wk->print_util, p_wk->p_bmpwin );
  MSGWND_Clear( p_wk );
  GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ表示面  初期化
 *
 *  @param  MSGWND_WORK* p_wk ワーク
 *  @param  bgframe           表示するBG面
 *  @param  x                 開始X位置（キャラ単位）
 *  @param  y                 開始Y位置（キャラ単位）
 *  @param  w                 幅（キャラ単位）
 *  @param  h                 高さ（キャラ単位）
 *  @param  plt               パレット番号
 *  @param  heapID            ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_InitEx( MSGWND_WORK* p_wk, u8 bgframe,
    u8 x, u8 y, u8 w, u8 h, u8 plt, u16 clear_chr, u8 dir, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
  p_wk->clear_chr = clear_chr;
  p_wk->p_bmpwin  = GFL_BMPWIN_Create( bgframe, x, y, w, h, plt, dir );
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( TEXTSTR_BUFFER_LENGTH, heapID );
  PRINT_UTIL_Setup( &p_wk->print_util, p_wk->p_bmpwin );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );
  GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ表示面  破棄
 *
 *  @param  MSGWND_WORK* p_wk   ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Exit( MSGWND_WORK* p_wk )
{
  GFL_STR_DeleteBuffer( p_wk->p_strbuf );
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );
  GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ表示面  メイン処理
 *
 *  @param  MSGWND_WORK *p_wk   ワーク
 *  @param  MSG_WORK  *p_msg    メッセージ
 *
 *  @retval  BOOL 転送が終わっている場合はTRUE／終わっていない場合はFALSE
 *
 */
//-----------------------------------------------------------------------------
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, const MSG_WORK *cp_msg )
{
  return PRINT_UTIL_Trans( &p_wk->print_util, cp_msg->p_print_que );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ表示面に文字を表示
 *
 *  @param  MSGWND_WORK* p_wk ワーク
 *  @param  MSG_WORK *cp_msg  文字管理
 *  @param  strID             文字ID
 *  @param  x                 開始位置X
 *  @param  y                 開始位置Y
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Print( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y )
{
  const GFL_MSGDATA* cp_msgdata;
  PRINT_QUE*  p_que;
  GFL_FONT*   p_font;

  cp_msgdata  = MSG_GetMsgDataConst( cp_msg );
  p_que   = MSG_GetPrintQue( cp_msg );
  p_font  = MSG_GetFont( cp_msg );

  //一端消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );

  //文字列作成
  GFL_MSG_GetString( cp_msgdata, strID, p_wk->p_strbuf );

  //キューをクリア
  PRINTSYS_QUE_Clear( p_que );

  //表示
  PRINT_UTIL_Print( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ表示面の中央に文字を表示
 *
 *  @param  MSGWND_WORK* p_wk ワーク
 *  @param  MSG_WORK *cp_msg  文字管理
 *  @param  strID             文字ID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_PrintCenter( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID )
{
  const GFL_MSGDATA* cp_msgdata;
  PRINT_QUE*  p_que;
  GFL_FONT*   p_font;
  u16 x, y;

  cp_msgdata  = MSG_GetMsgDataConst( cp_msg );
  p_que   = MSG_GetPrintQue( cp_msg );
  p_font  = MSG_GetFont( cp_msg );

  //一端消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );

  //文字列作成
  GFL_MSG_GetString( cp_msgdata, strID, p_wk->p_strbuf );

  //センター位置計算
  x = GFL_BMPWIN_GetSizeX( p_wk->p_bmpwin )*4;
  y = GFL_BMPWIN_GetSizeY( p_wk->p_bmpwin )*4;
  x -= PRINTSYS_GetStrWidth( p_wk->p_strbuf, p_font, 0 )/2;
  y -= PRINTSYS_GetStrHeight( p_wk->p_strbuf, p_font )/2;

  //キューをクリア
  PRINTSYS_QUE_Clear( p_que );

  //表示
  PRINT_UTIL_PrintColor( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font,PRINTSYS_LSB_Make(0xf,0xe,4) );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ表示面に数値つき文字を表示
 *
 *  @param  MSGWND_WORK* p_wk ワーク
 *  @param  MSG_WORK *cp_msg  文字管理
 *  @param  strID             文字ID
 *  @param  number            数値
 *  @param  buff_id           ワードセット登録バッファ
 *  @param  x                 開始位置X
 *  @param  y                 開始位置Y
 */
//-----------------------------------------------------------------------------
static void MSGWND_PrintNumber( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 number, u16 buff_id, u16 x, u16 y )
{
  const GFL_MSGDATA* cp_msgdata;
  WORDSET *p_wordset;

  //一端消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );

  //モジュール取得
  p_wordset   = MSG_GetWordSet( cp_msg );
  cp_msgdata  = MSG_GetMsgDataConst( cp_msg );

  //数値をワードセットに登録
  WORDSET_RegisterNumber( p_wordset, buff_id, number, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

  //元の文字列に数値を適用
  {
    STRBUF  *p_strbuf;
    p_strbuf  = GFL_MSG_CreateString( cp_msgdata, strID );
    WORDSET_ExpandStr( p_wordset, p_wk->p_strbuf, p_strbuf );
    GFL_STR_DeleteBuffer( p_strbuf );
  }

  //表示
  {
    PRINT_QUE*  p_que;
    GFL_FONT*   p_font;
    p_que   = MSG_GetPrintQue( cp_msg );
    p_font  = MSG_GetFont( cp_msg );

    //キューをクリア
    PRINTSYS_QUE_Clear( p_que );

    PRINT_UTIL_Print( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ表示面に数値つき文字を表示
 *
 *  @param  MSGWND_WORK* p_wk ワーク
 *  @param  MSG_WORK *cp_msg  文字管理
 *  @param  strID             文字ID
 *  @param  cp_status         プレイヤーの状態
 *  @param  x                 開始位置X
 *  @param  y                 開始位置Y
 */
//-----------------------------------------------------------------------------
static void MSGWND_PrintPlayerName( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, const COMPATIBLE_STATUS *cp_status, u16 x, u16 y, HEAPID heapID )
{
  const GFL_MSGDATA* cp_msgdata;
  WORDSET *p_wordset;

  //一端消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin),p_wk->clear_chr );

  //モジュール取得
  p_wordset   = MSG_GetWordSet( cp_msg );
  cp_msgdata  = MSG_GetMsgDataConst( cp_msg );

  //ワードセットに登録
  { 
    STRBUF  *p_name = GFL_STR_CreateBuffer( IRC_COMPATIBLE_SV_DATA_NAME_LEN, heapID );
    GFL_STR_SetStringCodeOrderLength( p_name, cp_status->name, IRC_COMPATIBLE_SV_DATA_NAME_LEN );

    WORDSET_RegisterWord( p_wordset, 0, p_name, cp_status->sex == 1 ? PTL_SEX_MALE : PTL_SEX_FEMALE, TRUE, PM_LANG );
    GFL_STR_DeleteBuffer( p_name );
  }

  //元の文字列に数値を適用
  {
    STRBUF  *p_strbuf;
    p_strbuf  = GFL_MSG_CreateString( cp_msgdata, strID );
    WORDSET_ExpandStr( p_wordset, p_wk->p_strbuf, p_strbuf );
    GFL_STR_DeleteBuffer( p_strbuf );
  }

  //表示
  {
    PRINT_QUE*  p_que;
    GFL_FONT*   p_font;
    p_que   = MSG_GetPrintQue( cp_msg );
    p_font  = MSG_GetFont( cp_msg );
    PRINT_UTIL_Print( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  画面クリア
 *
 *  @param  MSGWND_WORK* p_wk   ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Clear( MSGWND_WORK* p_wk )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );
  GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin );
}

//----------------------------------------------------------------------------
/**
 *  @brief  BMPWIN取得
 *
 *  @param  const MSGWND_WORK* cp_wk  ワーク
 *
 *  @return BMPWIN
 */
//-----------------------------------------------------------------------------
static GFL_BMPWIN *MSGWND_GetBmpWin( const MSGWND_WORK* cp_wk )
{
  return cp_wk->p_bmpwin;
}
//=============================================================================
/**
 *        個人結果画面
 */
//=============================================================================
//=============================================================================
/**
 *        SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  シーケンス変更
 *
 *  @param  AURA_MAIN_WORK *p_wk  ワーク
 *  @param  seq_function          シーケンス関数
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Change( AURA_MAIN_WORK *p_wk, SEQ_FUNCTION  seq_function )
{
  p_wk->seq_function  = seq_function;
  p_wk->seq = 0;
}

//----------------------------------------------------------------------------
/**
 *  @brief  PROC終了
 *
 *  @param  AURA_MAIN_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( AURA_MAIN_WORK *p_wk )
{
  p_wk->is_end  = TRUE;
}

//=============================================================================
/**
 *      SEQ_FUNCTION
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  ゲーム開始直後
 *
 *  @param  AURA_MAIN_WORK *p_wk  メインワーク
 *  @param  *p_seq                シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_StartGame( AURA_MAIN_WORK *p_wk, u16 *p_seq )
{
  enum{
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  GFL_POINT   *p_trg_left;

  p_trg_left  = &p_wk->trg_left[0];

  switch( *p_seq )
  {
  case SEQ_INIT:

    if( p_wk->p_param->p_gamesys )
    {
      MSGWND_PrintPlayerName( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg,
          AURA_STR_000, p_wk->p_param->p_you_status,  0, 0, HEAPID_IRCAURA );
    }
    else
    {
      MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg,
          AURA_STR_000, 0, 0 );
    }

    //タッチ演出OFF
    {
      int i;
      TOUCH_EFFECT_SYS *p_touch;
      p_touch = GRAPHIC_GetTouchEffWk( &p_wk->grp );
      for( i = 0; i < TOUCHEFFID_MAX; i++ )
      {
        TOUCH_EFFECT_SetVisible( p_touch, i, FALSE );
      }
      PMSND_StopSE();
    }

    GUIDE_SetVisible( GUIDE_VISIBLE_LEFT, HEAPID_IRCAURA );

    *p_seq  = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    if( TP_GetRectTrg( &sc_left, p_trg_left ) )
    {
      GUIDE_SetVisible( 0, HEAPID_IRCAURA );

      MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_001, 0, 0 );

      //左タッチ演出ON
      {
        TOUCH_EFFECT_SYS *p_touch;
        p_touch = GRAPHIC_GetTouchEffWk( &p_wk->grp );
        TOUCH_EFFECT_SetVisible( p_touch, TOUCHEFFID_LEFT, TRUE );
        TOUCH_EFFECT_SetPos( p_touch, TOUCHEFFID_LEFT, p_trg_left->x, p_trg_left->y );
        PMSND_PlaySE( IRCAURA_SE_AURA_L );
      }


      *p_seq  = SEQ_END;
    }

    break;

  case SEQ_END:
    SEQ_Change( p_wk, SEQFUNC_TouchLeft );
    break;
  }

  if( APPBAR_GetTrg(p_wk->p_appbar) == APPBAR_ICON_RETURN )
  {
    PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
    p_wk->p_param->result = IRCAURA_RESULT_RETURN;
    SEQ_End( p_wk );
  }



  //シーンが異なるチェック
  if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) != 0 )
  {
    SEQ_Change( p_wk, SEQFUNC_SceneError );
  }

}
//----------------------------------------------------------------------------
/**
 *  @brief  左手をタッチ中
 *
 *  @param  AURA_MAIN_WORK *p_wk  メインワーク
 *  @param  *p_seq                シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_TouchLeft( AURA_MAIN_WORK *p_wk, u16 *p_seq )
{
  enum{
    SEQ_MAIN,
    SEQ_WAIT_RIGHT,
    SEQ_RET,
  };
  GFL_POINT   *p_trg_right;
  SHAKE_SEARCH_WORK *p_shake_left;
  GFL_POINT pos;

  p_trg_right   = &p_wk->trg_right[0];
  p_shake_left  = &p_wk->shake_left[0];


  switch( *p_seq )
  {
  case SEQ_MAIN:
    if( TP_GetRectCont( &sc_left, &pos ) )
    {
      //計測終了待ち
      if( SHAKESEARCH_Main( p_shake_left, &sc_left ) )
      {
        MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_002, 0, 0 );
        GUIDE_SetVisible( GUIDE_VISIBLE_BOTH, HEAPID_IRCAURA );

        *p_seq  = SEQ_WAIT_RIGHT;
      }


      //左タッチ演出ON
      {
        TOUCH_EFFECT_SYS *p_touch;
        p_touch = GRAPHIC_GetTouchEffWk( &p_wk->grp );
        TOUCH_EFFECT_SetVisible( p_touch, TOUCHEFFID_LEFT, TRUE );
        TOUCH_EFFECT_SetPos( p_touch, TOUCHEFFID_LEFT, pos.x, pos.y );
      }

    }
    else
    {
      SHAKESEARCH_Init( p_shake_left);
      *p_seq  = SEQ_RET;
    }
    break;

  case SEQ_WAIT_RIGHT:
    if( TP_GetRectCont( &sc_right, p_trg_right ) )
    {
      GUIDE_SetVisible( 0, HEAPID_IRCAURA );


      MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_001, 0, 0 );
      SEQ_Change( p_wk, SEQFUNC_TouchRight );

      //右タッチ演出ON
      {
        TOUCH_EFFECT_SYS *p_touch;
        p_touch = GRAPHIC_GetTouchEffWk( &p_wk->grp );
        TOUCH_EFFECT_SetVisible( p_touch, TOUCHEFFID_RIGHT, TRUE );
        TOUCH_EFFECT_SetPos( p_touch, TOUCHEFFID_RIGHT, p_trg_right->x, p_trg_right->y );
        PMSND_PlaySE( IRCAURA_SE_AURA_R );
      }
    }
    else if( TP_GetRectCont( &sc_left, &pos ) )
    {
    }
    else
    {
      *p_seq  = SEQ_RET;
    }
    break;

  case SEQ_RET:
    p_wk->p_param->minus  += IRC_AURA_SCORE_MINUS;
    SEQ_Change( p_wk, SEQFUNC_StartGame );
    break;
  }

  if( APPBAR_GetTrg(p_wk->p_appbar) == APPBAR_ICON_RETURN )
  {
    PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
    p_wk->p_param->result = IRCAURA_RESULT_RETURN;
    SEQ_End( p_wk );
  }


  //シーンが異なるチェック
  if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) != 0 )
  {
    SEQ_Change( p_wk, SEQFUNC_SceneError );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  右手タッチ中
 *
 *  @param  AURA_MAIN_WORK *p_wk  メインワーク
 *  @param  *p_seq                シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_TouchRight( AURA_MAIN_WORK *p_wk, u16 *p_seq )
{
  enum{
    SEQ_MAIN,
    SEQ_RET,
  };


  GFL_POINT pos;
  SHAKE_SEARCH_WORK *p_shake_right;
  p_shake_right = &p_wk->shake_right[0];

  switch( *p_seq )
  {
  case SEQ_MAIN:
    if( TP_GetRectCont( &sc_right, &pos ) )
    {
      if( SHAKESEARCH_Main( p_shake_right, &sc_right ) )
      {
        int i;

        MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_003, 0, 0 );
        PMSND_PlaySE( IRCCOMMON_SE_IRC );
        SEQ_Change( p_wk, SEQFUNC_Result );
      }

      //右タッチ演出ON
      {
        TOUCH_EFFECT_SYS *p_touch;
        p_touch = GRAPHIC_GetTouchEffWk( &p_wk->grp );
        TOUCH_EFFECT_SetVisible( p_touch, TOUCHEFFID_RIGHT, TRUE );
        TOUCH_EFFECT_SetPos( p_touch, TOUCHEFFID_RIGHT, pos.x, pos.y );
      }

    }
    else
    {
      //右タッチ演出ON
      {
        TOUCH_EFFECT_SYS *p_touch;
        p_touch = GRAPHIC_GetTouchEffWk( &p_wk->grp );
        TOUCH_EFFECT_SetVisible( p_touch, TOUCHEFFID_RIGHT, FALSE );
      }
      SHAKESEARCH_Init( p_shake_right );
      *p_seq  = SEQ_RET;
    }
    break;

  case SEQ_RET:
    p_wk->p_param->minus  += IRC_AURA_SCORE_MINUS;
    SEQ_Change( p_wk, SEQFUNC_StartGame );
    break;
  }


  if( APPBAR_GetTrg(p_wk->p_appbar) == APPBAR_ICON_RETURN )
  {
    PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
    p_wk->p_param->result = IRCAURA_RESULT_RETURN;
    SEQ_End( p_wk );
  }

  //シーンが異なるチェック
  if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) != 0 )
  {
    SEQ_Change( p_wk, SEQFUNC_SceneError );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  結果
 *
 *  @param  AURA_MAIN_WORK *p_wk  メインワーク
 *  @param  *p_seq                シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Result( AURA_MAIN_WORK *p_wk, u16 *p_seq )
{

  enum{
    SEQ_RESULT,
    SEQ_MSG_PRINT,
    SEQ_SENDRESULT,
    SEQ_SCENE,
    SEQ_TIMING,
    SEQ_CALC,
    SEQ_MSG_PRINT_END,
    SEQ_END,

  };
  AURANET_RESULT_DATA result;

  switch( *p_seq )
  {
  case SEQ_RESULT:
    if( COMPATIBLE_IRC_TimingSyncWait( p_wk->p_param->p_irc, COMPATIBLE_TIMING_NO_AURA_RESULT ) )
    {
      *p_seq  = SEQ_MSG_PRINT;
    }
    break;

  case SEQ_MSG_PRINT:
    MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_004, 0, 0 );
    *p_seq  = SEQ_SENDRESULT;
    break;

  case SEQ_SENDRESULT:
    result.trg_left     = p_wk->trg_left[0];
    result.trg_right    = p_wk->trg_right[0];
    result.shake_left   = p_wk->shake_left[0];
    result.shake_right  = p_wk->shake_right[0];
    if( AURANET_SendResultData( &p_wk->net, &result ) )
    {
      *p_seq  = SEQ_SCENE;
    }

    break;

  case SEQ_SCENE:
    //次のシーンを設定
    COMPATIBLE_IRC_SetScene(  p_wk->p_param->p_irc, COMPATIBLE_SCENE_RESULT );
    if( COMPATIBLE_IRC_SendScene( p_wk->p_param->p_irc ) )
    {
      *p_seq  = SEQ_TIMING;
    }
    break;

  case SEQ_TIMING:
    if( COMPATIBLE_IRC_TimingSyncWait( p_wk->p_param->p_irc, COMPATIBLE_TIMING_NO_AURA_END ) )
    {
      *p_seq  = SEQ_MSG_PRINT_END;
    }
    break;

  case SEQ_MSG_PRINT_END:
    if( p_wk->cnt >= RESULT_SEND_CNT )
    {
      PMSND_PlaySE( IRCMENU_SE_IRC_ON );
      MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_005, 0, 0 );
      *p_seq  = SEQ_CALC;
    }
    break;

  case SEQ_CALC:
    p_wk->p_param->score  = CalcScore( p_wk );
    p_wk->p_param->result = IRCAURA_RESULT_CLEAR;
    *p_seq  = SEQ_END;
    break;

  case SEQ_END:
    SEQ_End( p_wk );
    return; //↓のアサートを通過しないため
  }


  //シーンが異なるチェック
  if( *p_seq < SEQ_SCENE && COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) != 0 )
  {
    SEQ_Change( p_wk, SEQFUNC_SceneError );
  }

  //戻るボタン
  if( APPBAR_GetTrg(p_wk->p_appbar) == APPBAR_ICON_RETURN )
  {
    PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
    p_wk->p_param->result = IRCAURA_RESULT_RETURN;
    SEQ_End( p_wk );
  }

  //赤外線開始待ち
  if( SEQ_MSG_PRINT <= *p_seq && *p_seq <=  SEQ_MSG_PRINT_END )
  {
    if( p_wk->cnt <= RESULT_SEND_CNT )
    {
      p_wk->cnt++;
    }
  }

}

//----------------------------------------------------------------------------
/**
 *  @brief  シーンエラーシーケンス
 *
 *  @param  AURA_MAIN_WORK *p_wk  メインワーク
 *  @param  *p_seq                シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_SceneError( AURA_MAIN_WORK *p_wk, u16 *p_seq )
{
  enum
  {
    SEQ_JUNCTION,
    SEQ_INIT,
    SEQ_MSG_PRINT,
    SEQ_TIMING,
    SEQ_END,
  };

  switch( *p_seq )
  {
  case SEQ_JUNCTION:
    NAGI_Printf( "ErrorScene!\n" );
    //自分が先ならば
    if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) > 0 )
    {
      *p_seq  = SEQ_INIT;
    }
    //相手が先ならば
    else if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) < 0 )
    {
      p_wk->p_param->score  = 0;
      p_wk->p_param->result = IRCAURA_RESULT_RESULT;
      SEQ_End( p_wk );
    }
    else
    {
      GF_ASSERT( 0 );
    }
    break;
  case SEQ_INIT:
    p_wk->p_param->score  = 0;
    p_wk->p_param->result = IRCAURA_RESULT_RESULT;
    *p_seq  = SEQ_MSG_PRINT;
    break;
  case SEQ_MSG_PRINT:
    MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_006, 0, 0 );
    *p_seq  = SEQ_TIMING;
    break;
  case SEQ_TIMING:
    if( COMPATIBLE_IRC_TimingSyncWait( p_wk->p_param->p_irc, COMPATIBLE_TIMING_NO_SCENE_ERROR ) )
    {
      *p_seq  = SEQ_END;
    }
    break;
  case SEQ_END:
    SEQ_End( p_wk );
    break;
  }
}

//=============================================================================
/**
 *          NET
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  NET初期化
 *
 *  @param  AURANET_WORK *p_wk  ワーク
 *  @param  COMPATIBLE_IRC_SYS  赤外線
 *
 */
//-----------------------------------------------------------------------------
static void AURANET_Init( AURANET_WORK *p_wk, COMPATIBLE_IRC_SYS *p_irc )
{
  GFL_STD_MemClear( p_wk, sizeof(AURANET_WORK) );
  p_wk->p_irc = p_irc;

  COMPATIBLE_IRC_AddCommandTable( p_irc, GFL_NET_CMD_IRCAURA, sc_recv_tbl, NELEMS(sc_recv_tbl), p_wk );
}
//----------------------------------------------------------------------------
/**
 *  @brief  NET破棄
 *
 *  @param  AURANET_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void AURANET_Exit( AURANET_WORK *p_wk )
{

  COMPATIBLE_IRC_DelCommandTable( p_wk->p_irc, GFL_NET_CMD_IRCAURA );
  GFL_STD_MemClear( p_wk, sizeof(AURANET_WORK) );
}

//----------------------------------------------------------------------------
/**
 *  @brief  結果情報を相手へ送受信
 *
 *  @param  AURANET_WORK *p_wk  ワーク
 *  @param  AURANET_RESULT_DATA *cp_data  データ
 *
 *  @retval TRUEならば、相手から受信
 *  @retval FALSEならば受信待ち
 */
//-----------------------------------------------------------------------------
static BOOL AURANET_SendResultData( AURANET_WORK *p_wk, const AURANET_RESULT_DATA *cp_data )
{
  enum
  {
    SEQ_BUFF_SAVE,
    SEQ_SEND_DATA,
    SEQ_RECV_WAIT,
  };


#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
  if( s_is_debug_only_play )
  {
    return TRUE;
  }
#endif// DEBUG_IRC_COMPATIBLE_ONLYPLAY

  switch( p_wk->seq )
  {
  case SEQ_BUFF_SAVE:
    p_wk->result_send = *cp_data;
    p_wk->seq = SEQ_SEND_DATA;
    NAGI_Printf( "結果データ送信開始\n" );
    break;

  case SEQ_SEND_DATA:
    {
      NetID netID;
      netID = GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );
      if( netID == 0 )
      {
        netID = 1;
      }else{
        netID = 0;
      }
      if( COMPATIBLE_IRC_SendDataEx( p_wk->p_irc, NETRECV_RESULT,
          sizeof(AURANET_RESULT_DATA), &p_wk->result_send, netID, FALSE, FALSE, TRUE ) )
      {
        NAGI_Printf( "結果データ送信完了、相手待ち\n" );
        p_wk->seq = SEQ_RECV_WAIT;
      }
    }
    break;

  case SEQ_RECV_WAIT:
    if( p_wk->is_recv )
    {
      NAGI_Printf( "結果データ受信完了\n" );
      p_wk->is_recv = FALSE;
      p_wk->seq = 0;
      return TRUE;
    }
    break;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  結果データ受け取り
 *
 *  @param  AURANET_WORK *p_wk  ワーク
 *  @param  *p_data             データ受け取り
 *
 */
//-----------------------------------------------------------------------------
static void AURANET_GetResultData( const AURANET_WORK *cp_wk, AURANET_RESULT_DATA *p_data )
{
  *p_data = cp_wk->result_recv;
}
//=============================================================================
/**
 *        NETRECV
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  結果データ受け取り受信コールバック
 *
 *  @param  const int netID ネットID
 *  @param  int size        サイズ
 *  @param  void* p_data    データ
 *  @param  p_work          ワーク
 *  @param  p_net_handle    ハンドル
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void NETRECV_Result( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle )
{
  AURANET_WORK *p_wk  = p_work;

  if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
  {
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if( netID == GFL_NET_SystemGetCurrentID() )
  {
    return; //自分のデータは無視
  }

//  GFL_STD_MemCopy( cp_data, &p_wk->result_recv, sizeof(AURANET_RESULT_DATA) );
  NAGI_Printf("結果データ受け取り完了\n" );
  p_wk->is_recv   = TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  結果データ受け取りバッファポインタ
 *
 *  @param  netID
 *  @param  pWork
 *  @param  size
 *
 *  @return バッファポインタ
 */
//-----------------------------------------------------------------------------
static u8* NETRECV_GetBufferAddr(int netID, void* p_work, int size)
{
  AURANET_WORK *p_wk  = p_work;

  if( netID == GFL_NET_SystemGetCurrentID() )
  {
    return (u8*)&p_wk->result_recv_my;
  }
  else
  {
    return (u8*)&p_wk->result_recv;
  }

}

//=============================================================================
/**
 *        円プリミティブ
 */
//=============================================================================
#if 0
//----------------------------------------------------------------------------
/**
 *  @brief  円プリミティブを作成
 *
 *  @param  CIRCLE_WORK *p_wk   ワーク
 *  @param  vtx_num             頂点数
 *  @param  HEAPID              メモリ確保用ID
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_Init( CIRCLE_WORK *p_wk, u32 vtx_num, HEAPID heapID )
{
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(CIRCLE_WORK) );

  //初期化
  p_wk->vtx_num = vtx_num;
  CIRCLE_SetAlpha( p_wk, 31 );
  {
    MtxFx33 rot;
    MTX_Identity33( &rot );
    CIRCLE_SetRot( p_wk, &rot );
  }

  //頂点作成
  p_wk->p_vtx = GFL_HEAP_AllocMemory( heapID, sizeof(VecFx16)*p_wk->vtx_num );
  GFL_STD_MemClear( p_wk->p_vtx, sizeof(VecFx16)*p_wk->vtx_num );

  //円の頂点座標作成
  {
    int i;
    for( i = 0; i < p_wk->vtx_num; i++ )
    {
      p_wk->p_vtx[i].x  = FX_CosIdx( ( i * 360 / p_wk->vtx_num) * 0xFFFF / 360 );
      p_wk->p_vtx[i].y  = FX_SinIdx( ( i * 360 / p_wk->vtx_num) * 0xFFFF / 360 );
      p_wk->p_vtx[i].z  = 0;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  円プリミティブを破棄
 *
 *  @param  CIRCLE_WORK *p_wk   ワーク
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_Exit( CIRCLE_WORK *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_vtx );
  GFL_STD_MemClear( p_wk, sizeof(CIRCLE_WORK) );
}

//----------------------------------------------------------------------------
/**
 *  @brief  円を描画
 *
 *  @param  CIRCLE_WORK *p_wk   ワーク
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_Draw( CIRCLE_WORK *p_wk )
{
  if( p_wk->is_visible )
  {
    G3_PushMtx();

    //回転設定
    G3_MultMtx33( &p_wk->status.rotate );

    //移動設定
    G3_Translate( p_wk->status.trans.x, p_wk->status.trans.y, p_wk->status.trans.z );

    //サイズ設定
    G3_Scale( p_wk->status.scale.x, p_wk->status.scale.y, p_wk->status.scale.z);

    //マテリアル設定
    G3_MaterialColorDiffAmb(GX_RGB(16, 16, 16), GX_RGB(16, 16, 16), FALSE );
    G3_MaterialColorSpecEmi(GX_RGB( 16, 16, 16 ), GX_RGB( 31,31,31 ), FALSE );
    G3_PolygonAttr( GX_LIGHTMASK_0123,GX_POLYGONMODE_MODULATE,GX_CULL_BACK,0,p_wk->alpha,0 );

    //描画
    G3_Begin( GX_BEGIN_TRIANGLES );
    {
      int i;
      G3_Color(p_wk->vtx_color);
      for( i = 0; i < p_wk->vtx_num - 1; i++ )
      {
        Circle_DrawLine( &p_wk->p_vtx[i], &p_wk->p_vtx[i+1] );
      }
      Circle_DrawLine( &p_wk->p_vtx[i], &p_wk->p_vtx[0] );
    }
    G3_End();

    G3_PopMtx(1);
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  頂点カラー設定
 *
 *  @param  CIRCLE_WORK *p_wk ワーク
 *  @param  color             カラー
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_SetVtxColor( CIRCLE_WORK *p_wk, GXRgb color )
{
  p_wk->vtx_color = color;
}

//----------------------------------------------------------------------------
/**
 *  @brief  アルファせってい
 *
 *  @param  CIRCLE_WORK *p_wk ワーク
 *  @param  alpha             アルファ  （0 ~ 31）
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_SetAlpha( CIRCLE_WORK *p_wk, u8 alpha )
{
  p_wk->alpha = alpha;
}

//----------------------------------------------------------------------------
/**
 *  @brief  トランスレーション設定
 *
 *  @param  CIRCLE_WORK *p_wk   ワーク
 *  @param  VecFx32 *cp_trans   移動
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_SetTrans( CIRCLE_WORK *p_wk, const VecFx32 *cp_trans )
{
  p_wk->status.trans  = *cp_trans;
}
//----------------------------------------------------------------------------
/**
 *  @brief  スケール値設定
 *
 *  @param  CIRCLE_WORK *p_wk   ワーク
 *  @param  VecFx32 *cp_scale   スケール
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_SetScale( CIRCLE_WORK *p_wk, const VecFx32 *cp_scale )
{
  p_wk->status.scale  = *cp_scale;
}
//----------------------------------------------------------------------------
/**
 *  @brief  回転行列設定
 *
 *  @param  CIRCLE_WORK *p_wk ワーク
 *  @param  MtxFx33 *cp_rot   回転行列
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_SetRot( CIRCLE_WORK *p_wk, const MtxFx33 *cp_rot )
{
  p_wk->status.rotate = *cp_rot;
}

//----------------------------------------------------------------------------
/**
 *  @brief  表示フラグ設定
 *
 *  @param  CIRCLE_WORK *p_wk ワーク
 *  @param  is_visible        TRUEならば表示　FALSEならば非表示
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_SetVisible( CIRCLE_WORK *p_wk, BOOL is_visible )
{
  p_wk->is_visible  = is_visible;
}
//----------------------------------------------------------------------------
/**
 *  @brief  移動取得
 *
 *  @param  const CIRCLE_WORK *cp_wk  ワーク
 *  @param  *p_trans                  移動受け取り
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_GetTrans( const CIRCLE_WORK *cp_wk, VecFx32 *p_trans )
{
  *p_trans  = cp_wk->status.trans;
}
//----------------------------------------------------------------------------
/**
 *  @brief  拡大縮小取得
 *
 *  @param  const CIRCLE_WORK *cp_wk  ワーク
 *  @param  *p_scale                  拡縮受け取り
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_GetScale( const CIRCLE_WORK *cp_wk, VecFx32 *p_scale )
{
  *p_scale  = cp_wk->status.scale;
}
//----------------------------------------------------------------------------
/**
 *  @brief  回転行列取得
 *
 *  @param  const CIRCLE_WORK *cp_wk  ワーク
 *  @param  *p_rot                    回転行列受け取り
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_GetRot( const CIRCLE_WORK *cp_wk, MtxFx33 *p_rot )
{
  *p_rot  = cp_wk->status.rotate;
}
//----------------------------------------------------------------------------
/**
 *  @brief  表示フラグ取得
 *
 *  @param  const CIRCLE_WORK *cp_wk  ワーク
 *
 *  @return TRUEならば表示中  FALSEならば非表示
 */
//-----------------------------------------------------------------------------
static BOOL CIRCLE_GetVisible( const CIRCLE_WORK *cp_wk )
{
  return cp_wk->is_visible;
}
//----------------------------------------------------------------------------
/**
 *  @brief  線を描画
 *
 *  @param  VecFx16 *p_start    開始座標
 *  @param  *p_end              終了座標
 *
 */
//-----------------------------------------------------------------------------
static void Circle_DrawLine( VecFx16 *p_start, VecFx16 *p_end )
{
#if 1
  G3_Vtx( p_start->x, p_start->y, p_start->z);
  G3_Vtx( p_end->x, p_end->y, p_end->z);
  G3_Vtx( p_start->x, p_start->y, p_start->z);
#else
  G3_Vtx( 0, 0, 0);
  G3_Vtx( p_start->x, p_start->y, p_start->z);
  G3_Vtx( p_end->x, p_end->y, p_end->z);
#endif
}
#endif
//=============================================================================
/**
 *        演出
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  タッチ演出、初期化
 *
 *  @param  TOUCH_EFFECT_SYS *p_wk    ワーク
 *  @param  heapID                    ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void TOUCH_EFFECT_Init( TOUCH_EFFECT_SYS *p_sys, const GRAPHIC_3D_WORK *cp_g3d, HEAPID heapID )
{
  GFL_STD_MemClear( p_sys, sizeof(TOUCH_EFFECT_SYS) );

  {
    int i, j;
    VecFx32 scale;
    for( j = 0; j < TOUCHEFFID_MAX; j++ )
    {
      p_sys->p_mdl[j] = GRAPHIC_3D_GetObj( cp_g3d, j );
      G3DMDL_SetVisible( p_sys->p_mdl[j], FALSE );
#if 0
      TOUCH_EFFECT_WORK *p_wk = &p_sys->wk[j];
      for( i = 0; i < CIRCLE_MAX; i++ )
      {
        CIRCLE_Init( &p_wk->c[i], CIRCLE_VTX_MAX, heapID );
        CIRCLE_SetVtxColor( &p_wk->c[i], GX_RGB(0,31,31) );
        p_wk->rot[i]  = 0;
        p_wk->rot_add[i]  = 0x1F;
        VEC_Set( &scale, FX_SinIdx(p_wk->rot[i])/4,
              FX_SinIdx(p_wk->rot[i])/4, 0 );
        CIRCLE_SetScale( &p_wk->c[i], &scale );
        CIRCLE_SetVisible( &p_wk->c[i], FALSE );
      }
#endif
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  破棄
 *
 *  @param  TOUCH_EFFECT_SYS *p_sys   ワーク
 *
 */
//-----------------------------------------------------------------------------
static void TOUCH_EFFECT_Exit( TOUCH_EFFECT_SYS *p_sys )
{
  {
    int i,j;
    for( j = 0; j < TOUCHEFFID_MAX; j++ )
    {

#if 0
      TOUCH_EFFECT_WORK *p_wk = &p_sys->wk[j];
      for( i = 0; i < CIRCLE_MAX; i++ )
      {
        CIRCLE_Exit( &p_wk->c[i] );
      }
#endif
    }
  }
  GFL_STD_MemClear( p_sys, sizeof(TOUCH_EFFECT_SYS) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  タッチ演出  描画
 *
 *  @param  TOUCH_EFFECT_SYS *p_wk    ワーク
 *
 */
//-----------------------------------------------------------------------------
static void TOUCH_EFFECT_Draw( TOUCH_EFFECT_SYS *p_sys )
{
  int i;

  for( i = 0; i < TOUCHEFFID_MAX; i++ )
  {

    G3DMDL_Draw( p_sys->p_mdl[i] );
#if 0
    TOUCH_EFFECT_WORK *p_wk = &p_sys->wk[j];

    for( i = 0; i < CIRCLE_MAX; i++ )
    {
      if( CIRCLE_GetVisible( &p_wk->c[i] ) )
      {
        p_wk->rot[i]  += p_wk->rot_add[i];
      }
      {
        VEC_Set( &scale, FX_SinIdx(p_wk->rot[i])/4,
              FX_SinIdx(p_wk->rot[i])/4, 0 );
        CIRCLE_SetScale( &p_wk->c[i], &scale );
      }

      CIRCLE_Draw( &p_wk->c[i] );
    }
#endif
  }
  {
    if( G3DMDL_GetVisible( p_sys->p_mdl[TOUCHEFFID_LEFT] ) &&
        G3DMDL_IsLoopEnd( p_sys->p_mdl[TOUCHEFFID_LEFT] ) )
    {
      PMSND_PlaySE( IRCAURA_SE_AURA_L );
    }
    if( G3DMDL_GetVisible( p_sys->p_mdl[TOUCHEFFID_RIGHT] ) &&
     G3DMDL_IsLoopEnd( p_sys->p_mdl[TOUCHEFFID_RIGHT] ) )
    {
      PMSND_PlaySE( IRCAURA_SE_AURA_R );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  タッチ演出  表示設定
 *
 *  @param  TOUCH_EFFECT_SYS *p_wk  ワーク
 *  @param  is_visible              表示ON,OFF
 *
 */
//-----------------------------------------------------------------------------
static void TOUCH_EFFECT_SetVisible( TOUCH_EFFECT_SYS *p_sys, TOUCHEFFID id, BOOL is_visible )
{
  int i;

  G3DMDL_SetVisible( p_sys->p_mdl[id], is_visible );
#if 0
  TOUCH_EFFECT_WORK *p_wk = &p_sys->wk[id];

  for( i = 0; i < CIRCLE_MAX; i++ )
  {
    CIRCLE_SetVisible( &p_wk->c[i], is_visible );
    if( !is_visible )
    {
      p_wk->rot[i]  = 0;
    }
  }
#endif
}

//----------------------------------------------------------------------------
/**
 *  @brief  座標設定
 *
 *  @param  TOUCH_EFFECT_SYS *p_wk    ワーク
 *  @param  x                         LCD座標X
 *  @param  y                         LCD座標Y
 *
 */
//-----------------------------------------------------------------------------
static void TOUCH_EFFECT_SetPos( TOUCH_EFFECT_SYS *p_sys, TOUCHEFFID id, u32 x, u32 y )
{
  enum
  {
    AURA_DISTANCE_OFS = 64,
  };

//  TOUCH_EFFECT_WORK *p_wk = &p_sys->wk[id];
  VecFx32 near;
  VecFx32 far;
  VecFx32 v;
  int i;

  switch( id )
  {
  case TOUCHEFFID_LEFT:
    NNS_G3dScrPosToWorldLine( x, y, &near, &far );
    VEC_Subtract( &far, &near, &v );
    VEC_Normalize( &v, &v );
    VEC_MultAdd( FX32_CONST(AURA_DISTANCE_OFS), &v, &near, &near );
    G3DMDL_SetPos( p_sys->p_mdl[id], &near );

#if 0
    for( i = 0; i < CIRCLE_MAX; i++ )
    {
      CIRCLE_SetTrans( &p_wk->c[i], &near );
    }
#endif
    p_sys->left.x = x;
    p_sys->left.y = y;
    break;

  case TOUCHEFFID_RIGHT:
    {
      GFL_POINT pos;
      pos.x   = p_sys->left.x + (x - p_sys->left.x) * 2;
      pos.y   = p_sys->left.y + (y - p_sys->left.y) * 2;
  //    NAGI_Printf( "X 左%d 中%d 右%d", p_wk->left.x, x, pos.x );
    //  NAGI_Printf( "Y 左%d 中%d 右%d", p_wk->left.y, y, pos.y );
      NNS_G3dScrPosToWorldLine( pos.x, pos.y, &near, &far );
      VEC_Subtract( &far, &near, &v );
      VEC_Normalize( &v, &v );
      VEC_MultAdd( FX32_CONST(AURA_DISTANCE_OFS), &v, &near, &near );
      G3DMDL_SetPos( p_sys->p_mdl[id], &near );

#if 0
      for( i = 0; i < CIRCLE_MAX; i++ )
      {
        CIRCLE_SetTrans( &p_wk->c[i], &near );
      }
#endif
    }
    break;
  }

}
//=============================================================================
/**
 *        汎用
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  矩形内にTrgしたかどうか
 *
 *  @param  const GFL_RECT *cp_rect 矩形
 *  @param  *p_trg                  座標受け取り
 *
 *  @retval TRUEタッチした
 *  @retval FALSEタッチしていない
 */
//-----------------------------------------------------------------------------
static BOOL TP_GetRectTrg( const GFL_RECT *cp_rect, GFL_POINT *p_trg )
{
  u32 x, y;
  BOOL ret;

  //Cont中で、矩形内のとき
  if( GFL_UI_TP_GetPointTrg( &x, &y ) )
  {
    if( ((u32)( x - cp_rect->left) < (u32)(cp_rect->right - cp_rect->left))
        & ((u32)( y - cp_rect->top) < (u32)(cp_rect->bottom - cp_rect->top))
      )
    {
      //受け取りが存在したら代入して返す
      if( p_trg )
      {
        p_trg->x  = x;
        p_trg->y  = y;
      }
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  矩形内にタッチしたかどうか
 *
 *  @param  const GFL_RECT *cp_rect 矩形
 *  @param  *p_cont                 座標受け取り
 *
 *  @retval TRUEタッチした
 *  @retval FALSEタッチしていない
 */
//-----------------------------------------------------------------------------
static BOOL TP_GetRectCont( const GFL_RECT *cp_rect, GFL_POINT *p_cont )
{
  u32 x, y;
  BOOL ret;

  //Cont中で、矩形内のとき
  if( GFL_UI_TP_GetPointCont( &x, &y ) )
  {
    if( ((u32)( x - cp_rect->left) < (u32)(cp_rect->right - cp_rect->left))
        & ((u32)( y - cp_rect->top) < (u32)(cp_rect->bottom - cp_rect->top))
      )
    {
      //受け取りが存在したら代入して返す
      if( p_cont )
      {
        p_cont->x = x;
        p_cont->y = y;
      }
      return TRUE;
    }
  }

  return FALSE;
}

#ifdef DEBUG_AURA_MSG
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグプリントを更新
 *
 *  @param  AURA_MAIN_WORK *p_wk  ワーク
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void DEBUGAURA_PRINT_UpDate( AURA_MAIN_WORK *p_wk )
{
  int i, j;
  int now_idx;

  DEBUGPRINT_Clear();

  DEBUGPRINT_PrintNumber( L"%d番目のゲーム", p_wk->debug_game_cnt, 0,  0 );
  DEBUGPRINT_PrintNumber( L"%d人目の番です", p_wk->debug_player, 100,  0 );


  //プリント
  OS_Printf( "↓○%d番目のゲーム　プリント開始↓↓↓↓↓↓↓↓\n",p_wk->debug_game_cnt );
  for( j = 0; j < 2; j++ )
  {
    now_idx = p_wk->debug_game_cnt + (j*DEBUG_GAME_NUM);

    DEBUGPRINT_Print( L"左指の座標", DEBUGMSG_LEFT1+(DEBUGMSG2_TAB*j), 10 );
    DEBUGPRINT_Print( L"右指の座標", DEBUGMSG_RIGHT1+(DEBUGMSG2_TAB*j),10 );

    DEBUGPRINT_PrintNumber( L"X %d", p_wk->trg_left[now_idx].x, DEBUGMSG_LEFT1+(DEBUGMSG2_TAB*j), 20 );
    DEBUGPRINT_PrintNumber( L"Y %d", p_wk->trg_left[now_idx].y, DEBUGMSG_LEFT2+(DEBUGMSG2_TAB*j), 20 );

    DEBUGPRINT_PrintNumber( L"X %d", p_wk->trg_right[now_idx].x, DEBUGMSG_RIGHT1+(DEBUGMSG2_TAB*j), 20 );
    DEBUGPRINT_PrintNumber( L"Y %d", p_wk->trg_right[now_idx].y, DEBUGMSG_RIGHT2+(DEBUGMSG2_TAB*j), 20 );

    DEBUGPRINT_Print( L"左指のブレ", DEBUGMSG_LEFT1+(DEBUGMSG2_TAB*j), 30 );
    DEBUGPRINT_Print( L"右指のブレ", DEBUGMSG_RIGHT1+(DEBUGMSG2_TAB*j),30 );

    for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
    {
      //DEBUGPRINT_PrintNumber( L"X %d", p_wk->shake_left.shake[i].x, DEBUGMSG_LEFT1, i*10+40 );
      DEBUGPRINT_PrintNumber( L"X %d ", p_wk->shake_left[now_idx].shake[i].x- p_wk->shake_left[now_idx].shake[0].x, DEBUGMSG_LEFT1+(DEBUGMSG2_TAB*j), i*10+40 );
      //DEBUGPRINT_PrintNumber( L"Y %d", p_wk->shake_left.shake[i].y, DEBUGMSG_LEFT3, i*10+40 );
      DEBUGPRINT_PrintNumber( L"Y %d ", p_wk->shake_left[now_idx].shake[i].y- p_wk->shake_left[now_idx].shake[0].y, DEBUGMSG_LEFT2+(DEBUGMSG2_TAB*j), i*10+40 );

      //DEBUGPRINT_PrintNumber( L"X %d", p_wk->shake_right.shake[i].x, DEBUGMSG_RIGHT1, i*10+40 );
      DEBUGPRINT_PrintNumber( L"X %d ", p_wk->shake_right[now_idx].shake[i].x - p_wk->shake_right[now_idx].shake[0].x, DEBUGMSG_RIGHT1+(DEBUGMSG2_TAB*j), i*10+40 );
      //DEBUGPRINT_PrintNumber( L"Y %d", p_wk->shake_right.shake[i].y, DEBUGMSG_RIGHT3, i*10+40 );
      DEBUGPRINT_PrintNumber( L"Y %d ", p_wk->shake_right[now_idx].shake[i].y - p_wk->shake_right[now_idx].shake[0].y,DEBUGMSG_RIGHT2+(DEBUGMSG2_TAB*j), i*10+40 );
    }


    OS_Printf( "◇%d人目のデータ\n", j );

    OS_Printf("左手座標\n");
    OS_Printf("X %d Y %d\n", p_wk->trg_left[now_idx].x, p_wk->trg_left[now_idx].y);

    OS_Printf("右手座標\n");
    OS_Printf("X %d Y %d\n", p_wk->trg_right[now_idx].x, p_wk->trg_right[now_idx].y);

    OS_Printf("左手ブレ幅\n");
    for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
    {
      OS_Printf( "X %d (%d) Y %d (%d)\n",
          p_wk->shake_left[now_idx].shake[i].x,
          p_wk->shake_left[now_idx].shake[i].x - p_wk->shake_left[now_idx].shake[0].x,
          p_wk->shake_left[now_idx].shake[i].y,
          p_wk->shake_left[now_idx].shake[i].y - p_wk->shake_left[now_idx].shake[0].y );
    }

    OS_Printf("右手ブレ幅\n");
    for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
    {
      OS_Printf( "X %d (%d) Y %d (%d)\n",
          p_wk->shake_right[now_idx].shake[i].x,
          p_wk->shake_right[now_idx].shake[i].x - p_wk->shake_right[now_idx].shake[0].x,
          p_wk->shake_right[now_idx].shake[i].y,
          p_wk->shake_right[now_idx].shake[i].y - p_wk->shake_right[now_idx].shake[0].y );
    }
  }

  {
    u16 idx1, idx2;
    s32 ofs1[4];
    s32 ofs2[4];

    idx1  = p_wk->debug_game_cnt + (0*DEBUG_GAME_NUM);;
    idx2  = p_wk->debug_game_cnt + (1*DEBUG_GAME_NUM);;

    //差---------------------
    DEBUGPRINT_PrintNumber( L"X %d", p_wk->trg_left[idx1].x - p_wk->trg_left[idx2].x,
        DEBUGMSG3_TAB, 20 );
    DEBUGPRINT_PrintNumber( L"Y %d", p_wk->trg_left[idx1].y - p_wk->trg_left[idx2].y,
        DEBUGMSG3_TAB+20, 20 );
    DEBUGPRINT_PrintNumber( L"X %d", p_wk->trg_right[idx1].x - p_wk->trg_right[idx2].x,
        DEBUGMSG3_TAB+40, 20 );
    DEBUGPRINT_PrintNumber( L"Y %d", p_wk->trg_right[idx1].y - p_wk->trg_right[idx2].y,
        DEBUGMSG3_TAB+60, 20 );

    for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
    {
      ofs1[0] = p_wk->shake_left[idx1].shake[i].x- p_wk->shake_left[idx1].shake[0].x;
      ofs1[1] = p_wk->shake_left[idx1].shake[i].y- p_wk->shake_left[idx1].shake[0].y;
      ofs1[2] = p_wk->shake_right[idx1].shake[i].x- p_wk->shake_right[idx1].shake[0].x;
      ofs1[3] = p_wk->shake_right[idx1].shake[i].y- p_wk->shake_right[idx1].shake[0].y;
      ofs2[0] = p_wk->shake_left[idx2].shake[i].x- p_wk->shake_left[idx2].shake[0].x;
      ofs2[1] = p_wk->shake_left[idx2].shake[i].y- p_wk->shake_left[idx2].shake[0].y;
      ofs2[2] = p_wk->shake_right[idx2].shake[i].x- p_wk->shake_right[idx2].shake[0].x;
      ofs2[3] = p_wk->shake_right[idx2].shake[i].y- p_wk->shake_right[idx2].shake[0].y;

      DEBUGPRINT_PrintNumber( L"X %d ", ofs1[0]-ofs2[0], DEBUGMSG3_TAB, i*10+40 );
      DEBUGPRINT_PrintNumber( L"Y %d ", ofs1[1]-ofs2[1], DEBUGMSG3_TAB+20, i*10+40 );
      DEBUGPRINT_PrintNumber( L"X %d ", ofs1[2]-ofs2[2], DEBUGMSG3_TAB+40, i*10+40 );
      DEBUGPRINT_PrintNumber( L"Y %d ", ofs1[3]-ofs2[3], DEBUGMSG3_TAB+60, i*10+40 );
    }

    OS_Printf( "◇差\n" );
    OS_Printf("左手座標の差\n");
    OS_Printf("X %d Y %d\n",
        p_wk->trg_left[idx1].x - p_wk->trg_left[idx2].x,
        p_wk->trg_left[idx1].y - p_wk->trg_left[idx2].y);

    OS_Printf("右手座標の差\n");
    OS_Printf("X %d Y %d\n",
        p_wk->trg_right[idx1].x - p_wk->trg_right[idx2].x,
        p_wk->trg_right[idx1].y - p_wk->trg_right[idx2].y);

    OS_Printf( "左手ブレの差\n" );
    for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
    {
      ofs1[0] = p_wk->shake_left[idx1].shake[i].x- p_wk->shake_left[idx1].shake[0].x;
      ofs1[1] = p_wk->shake_left[idx1].shake[i].y- p_wk->shake_left[idx1].shake[0].y;
      ofs2[0] = p_wk->shake_left[idx2].shake[i].x- p_wk->shake_left[idx2].shake[0].x;
      ofs2[1] = p_wk->shake_left[idx2].shake[i].y- p_wk->shake_left[idx2].shake[0].y;
      OS_Printf( "X (%d) Y (%d)\n",ofs1[0]-ofs2[0], ofs1[1]-ofs2[1] );
    }

    OS_Printf( "右手ブレの差\n" );
    for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
    {
      ofs1[2] = p_wk->shake_right[idx1].shake[i].x- p_wk->shake_right[idx1].shake[0].x;
      ofs1[3] = p_wk->shake_right[idx1].shake[i].y- p_wk->shake_right[idx1].shake[0].y;
      ofs2[2] = p_wk->shake_right[idx2].shake[i].x- p_wk->shake_right[idx2].shake[0].x;
      ofs2[3] = p_wk->shake_right[idx2].shake[i].y- p_wk->shake_right[idx2].shake[0].y;
      OS_Printf( "X (%d) Y (%d)\n",ofs1[2]-ofs2[2], ofs1[3]-ofs2[3] );
    }

  }

  {
    GFL_POINT temp;
    u32 checklist[9];   //左上、右上、左下、右下、左、上、右、下、中心



    for( j = 0; j < 2; j++ )
    {
      OS_Printf( "△%d人目の左手ブレ計測\n", j );
      now_idx = p_wk->debug_game_cnt + (j*DEBUG_GAME_NUM);
      GFL_STD_MemClear( checklist, sizeof(u32)*9 );

      for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
      {
        temp.x  = p_wk->shake_left[now_idx].shake[i].x - p_wk->shake_left[now_idx].shake[0].x;
        temp.y  = p_wk->shake_left[now_idx].shake[i].y - p_wk->shake_left[now_idx].shake[0].y;
        if( temp.x < 0 && temp.y > 0 )        //左上
        {
          checklist[0]++;
        }
        else if( temp.x > 0 && temp.y > 0 )   //右上
        {
          checklist[1]++;
        }
        else if( temp.x < 0 && temp.y < 0 )   //左下
        {
          checklist[2]++;
        }
        else if( temp.x > 0 && temp.y < 0 )   //右下
        {
          checklist[3]++;
        }
        else if( temp.x < 0 && temp.y == 0 )    //左
        {
          checklist[4]++;
        }
        else if( temp.x == 0 && temp.y > 0 )    //上
        {
          checklist[5]++;
        }
        else if( temp.x > 0 && temp.y == 0 )    //右
        {
          checklist[6]++;
        }
        else if( temp.x == 0 && temp.y < 0 )  //下
        {
          checklist[7]++;
        }
        else if( temp.x == 0 && temp.y == 0 ) //中心
        {
          checklist[8]++;
        }

      }

      OS_Printf( "左上　%d個\n", checklist[0] );
      OS_Printf( "右上　%d個\n", checklist[1] );
      OS_Printf( "左下　%d個\n", checklist[2] );
      OS_Printf( "右下　%d個\n", checklist[3] );
      OS_Printf( "左　　%d個\n", checklist[4] );
      OS_Printf( "上　　%d個\n", checklist[5] );
      OS_Printf( "右　　%d個\n", checklist[6] );
      OS_Printf( "下　　%d個\n", checklist[7] );
      OS_Printf( "中心　%d個\n", checklist[8] );


      OS_Printf( "△%d人目の右手ブレ計測\n", j );
      GFL_STD_MemClear( checklist, sizeof(u32)*9 );
      for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
      {
        temp.x  = p_wk->shake_right[now_idx].shake[i].x - p_wk->shake_right[now_idx].shake[0].x;
        temp.y  = p_wk->shake_right[now_idx].shake[i].y - p_wk->shake_right[now_idx].shake[0].y;
        if( temp.x < 0 && temp.y > 0 )        //左上
        {
          checklist[0]++;
        }
        else if( temp.x > 0 && temp.y > 0 )   //右上
        {
          checklist[1]++;
        }
        else if( temp.x < 0 && temp.y < 0 )   //左下
        {
          checklist[2]++;
        }
        else if( temp.x > 0 && temp.y < 0 )   //右下
        {
          checklist[3]++;
        }
        else if( temp.x < 0 && temp.y == 0 )    //左
        {
          checklist[4]++;
        }
        else if( temp.x == 0 && temp.y > 0 )    //上
        {
          checklist[5]++;
        }
        else if( temp.x > 0 && temp.y == 0 )    //右
        {
          checklist[6]++;
        }
        else if( temp.x == 0 && temp.y < 0 )  //下
        {
          checklist[7]++;
        }
        else if( temp.x == 0 && temp.y == 0 ) //中心
        {
          checklist[8]++;
        }

      }

      OS_Printf( "左上　%d個\n", checklist[0] );
      OS_Printf( "右上　%d個\n", checklist[1] );
      OS_Printf( "左下　%d個\n", checklist[2] );
      OS_Printf( "右下　%d個\n", checklist[3] );
      OS_Printf( "左　　%d個\n", checklist[4] );
      OS_Printf( "上　　%d個\n", checklist[5] );
      OS_Printf( "右　　%d個\n", checklist[6] );
      OS_Printf( "下　　%d個\n", checklist[7] );
      OS_Printf( "中心　%d個\n", checklist[8] );

    }
  }

  {
    GFL_POINT pos1, pos2;
    u32 ret;
    u32 pos_score;
    OS_Printf( "△お互いの座標計測\n" );
    pos1  = p_wk->trg_left[p_wk->debug_game_cnt + (0*DEBUG_GAME_NUM)];
    pos2  = p_wk->trg_left[p_wk->debug_game_cnt + (1*DEBUG_GAME_NUM)];
    ret = ((u32)(pos1.x-pos2.x)*(u32)(pos1.x-pos2.x))+((u32)(pos1.y-pos2.y)*(u32)(pos1.y-pos2.y));
    if( 0 < ret & ret <= RANGE_POS_SCORE_00 )
    {
      pos_score = VAL_POS_SCORE_00;
    }
    else if( RANGE_POS_SCORE_00 < ret & ret <= RANGE_POS_SCORE_01 )
    {
      pos_score = VAL_POS_SCORE_01;
    }
    else if( RANGE_POS_SCORE_01 < ret & ret <= RANGE_POS_SCORE_02 )
    {
      pos_score = VAL_POS_SCORE_02;
    }
    else if( RANGE_POS_SCORE_02 < ret & ret <= RANGE_POS_SCORE_03 )
    {
      pos_score = VAL_POS_SCORE_03;
    }
    else if( RANGE_POS_SCORE_03 < ret & ret <= RANGE_POS_SCORE_04 )
    {
      pos_score = VAL_POS_SCORE_04;
    }
    else{
      pos_score = VAL_POS_SCORE_05;
    }
    OS_Printf( "距離　%d点\n", pos_score ); }

  {
    GFL_POINT temp1, temp2;
    u32 shake1, shake2;
    u8 idx1, idx2;
    u16 ret;
    u16 score;
    OS_Printf( "△お互いのブレ計算結果\n" );

    idx1  = p_wk->debug_game_cnt + (0*DEBUG_GAME_NUM);
    idx2  = p_wk->debug_game_cnt + (1*DEBUG_GAME_NUM);

    score = 0;
    for( i = 1; i <TOUCH_COUNTER_SHAKE_MAX; i++ )
    {
      temp1.x = p_wk->shake_left[idx1].shake[i].x - p_wk->shake_left[idx1].shake[0].x;
      temp1.y = p_wk->shake_left[idx1].shake[i].y - p_wk->shake_left[idx1].shake[0].y;
      temp2.x = p_wk->shake_left[idx2].shake[i].x - p_wk->shake_left[idx2].shake[0].x;
      temp2.y = p_wk->shake_left[idx2].shake[i].y - p_wk->shake_left[idx2].shake[0].y;

      shake1  = MATH_IAbs( temp1.x ) + MATH_IAbs( temp1.y );
      shake2  = MATH_IAbs( temp2.x ) + MATH_IAbs( temp2.y );

      if( shake1 > shake2 )
      {
        ret = shake1 - shake2;
      }
      else
      {
        ret = shake2 - shake1;
      }

      if( RANGE_SHAKE_SCORE_00(ret) )
      {
        score += VAL_SHAKE_SCORE_00;
        OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_00 );
      }
      else if( RANGE_SHAKE_SCORE_01(ret) )
      {
        score += VAL_SHAKE_SCORE_01;
        OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_01 );
      }
      else if( RANGE_SHAKE_SCORE_02(ret) )
      {
        score += VAL_SHAKE_SCORE_02;
        OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_02 );
      }
      else if( RANGE_SHAKE_SCORE_03(ret) )
      {
        score += VAL_SHAKE_SCORE_03;
        OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_03 );
      }
      else if( RANGE_SHAKE_SCORE_04(ret) )
      {
#if VAL_SHAKE_SCORE_04
          score += VAL_SHAKE_SCORE_04;
#endif //VAL_SHAKE_SCORE_04
          OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_04 );
      }
    }

    OS_Printf( "ブレ結果 %d点\n", score/9 );
  }


  OS_Printf( "↑○%d番目のゲーム　プリント終了↑↑↑↑↑↑↑↑\n", p_wk->debug_game_cnt );
}
#endif //DEBUG_AURA_MSG
//----------------------------------------------------------------------------
/**
 *  @brief  スコア計算
 *
 *  @param  AURA_MAIN_WORK *p_wk  ワーク
 *
 *  @return スコア
 */
//-----------------------------------------------------------------------------
static u8  CalcScore( AURA_MAIN_WORK *p_wk )
{
  AURANET_RESULT_DATA my;
  AURANET_RESULT_DATA you;

  u32 pos_score;
  u32 shake_score;
  my.trg_left   = p_wk->trg_left[0];
  my.trg_right  = p_wk->trg_right[0];
  my.shake_left = p_wk->shake_left[0];
  my.shake_right= p_wk->shake_right[0];

  AURANET_GetResultData( &p_wk->net, &you );

  //座標スコア
  {
    GFL_POINT pos1, pos2;
    u32 ret;
    OS_Printf( "△お互いの座標計測\n" );
    pos1  = my.trg_left;
    pos2  = you.trg_left;
    ret = ((u32)(pos1.x-pos2.x)*(u32)(pos1.x-pos2.x))+((u32)(pos1.y-pos2.y)*(u32)(pos1.y-pos2.y));
    if( 0 < ret & ret <= RANGE_POS_SCORE_00 )
    {
      pos_score = VAL_POS_SCORE_00;
    }
    else if( RANGE_POS_SCORE_00 < ret & ret <= RANGE_POS_SCORE_01 )
    {
      pos_score = VAL_POS_SCORE_01;
    }
    else if( RANGE_POS_SCORE_01 < ret & ret <= RANGE_POS_SCORE_02 )
    {
      pos_score = VAL_POS_SCORE_02;
    }
    else if( RANGE_POS_SCORE_02 < ret & ret <= RANGE_POS_SCORE_03 )
    {
      pos_score = VAL_POS_SCORE_03;
    }
    else if( RANGE_POS_SCORE_03 < ret & ret <= RANGE_POS_SCORE_04 )
    {
      pos_score = VAL_POS_SCORE_04;
    }
    else{
      pos_score = VAL_POS_SCORE_05;
    }
    OS_Printf("自分の左手座標\n");
    OS_Printf("X %d Y %d\n",
        pos1.x,
        pos1.y);

    OS_Printf("相手の左手座標\n");
    OS_Printf("X %d Y %d\n",
        pos2.x,
        pos2.y);
    OS_Printf( "距離　%d点\n", pos_score );
  }

  //ブレスコア
  {
    int i;
    GFL_POINT temp1, temp2;
    u32 shake1, shake2;
    u16 ret;
    shake_score = 0;
    for( i = 1; i <TOUCH_COUNTER_SHAKE_MAX; i++ )
    {
      temp1.x = my.shake_left.shake[i].x - my.shake_left.shake[0].x;
      temp1.y = my.shake_left.shake[i].y - my.shake_left.shake[0].y;
      temp2.x = you.shake_left.shake[i].x - you.shake_left.shake[0].x;
      temp2.y = you.shake_left.shake[i].y - you.shake_left.shake[0].y;

      OS_Printf( "自分のブレ[%d] X %d Y %d\n", i, temp1.x, temp1.y );
      OS_Printf( "相手のブレ[%d] X %d Y %d\n", i, temp2.x, temp2.y );



      shake1  = MATH_IAbs( temp1.x ) + MATH_IAbs( temp1.y );
      shake2  = MATH_IAbs( temp2.x ) + MATH_IAbs( temp2.y );

      if( shake1 > shake2 )
      {
        ret = shake1 - shake2;
      }
      else
      {
        ret = shake2 - shake1;
      }

      if( RANGE_SHAKE_SCORE_00(ret) )
      {
        shake_score += VAL_SHAKE_SCORE_00;
        OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_00 );
      }
      else if( RANGE_SHAKE_SCORE_01(ret) )
      {
        shake_score += VAL_SHAKE_SCORE_01;
        OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_01 );
      }
      else if( RANGE_SHAKE_SCORE_02(ret) )
      {
        shake_score += VAL_SHAKE_SCORE_02;
        OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_02 );
      }
      else if( RANGE_SHAKE_SCORE_03(ret) )
      {
        shake_score += VAL_SHAKE_SCORE_03;
        OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_03 );
      }
      else if( RANGE_SHAKE_SCORE_04(ret) )
      {
#if VAL_SHAKE_SCORE_04
          shake_score += VAL_SHAKE_SCORE_04;
#endif //VAL_SHAKE_SCORE_04
          OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_04 );
      }
    }
    shake_score /= 9;
    OS_Printf( "ブレ　%d点\n", shake_score );
  }

  return (pos_score + shake_score)/2;
}
//----------------------------------------------------------------------------
/**
 *  @brief  Screenデータの　VRAM転送拡張版（読み込んだスクリーンの一部範囲をバッファに張りつける）
 *
 *  @param  ARCHANDLE *handle ハンドル
 *  @param  datID             データID
 *  @param  frm               フレーム
 *  @param  chr_ofs           キャラオフセット
 *  @param  src_x             転送元ｘ座標
 *  @param  src_y             転送元Y座標
 *  @param  src_w             転送元幅      //データの全体の大きさ
 *  @param  src_h             転送元高さ    //データの全体の大きさ
 *  @param  dst_x             転送先X座標
 *  @param  dst_y             転送先Y座標
 *  @param  dst_w             転送先幅
 *  @param  dst_h             転送先高さ
 *  @param  plt               パレット番号
 *  @param  compressedFlag    圧縮フラグ
 *  @param  heapID            一時バッファ用ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void ARCHDL_UTIL_TransVramScreenEx( ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 chr_ofs, u8 src_x, u8 src_y, u8 src_w, u8 src_h, u8 dst_x, u8 dst_y, u8 dst_w, u8 dst_h, u8 plt, BOOL compressedFlag, HEAPID heapID )
{
  void *p_src_arc;
  NNSG2dScreenData* p_scr_data;

  //読み込み
  p_src_arc = GFL_ARCHDL_UTIL_Load( handle, datID, compressedFlag, GetHeapLowID(heapID) );

  //アンパック
  if(!NNS_G2dGetUnpackedScreenData( p_src_arc, &p_scr_data ) )
  {
    GF_ASSERT(0); //スクリーンデータじゃないよ
  }

  //エラー
  GF_ASSERT( src_w * src_h * 2 <= p_scr_data->szByte );

  //キャラオフセット計算
  if( chr_ofs )
  {
    int i;
    if( GFL_BG_GetScreenColorMode( frm ) == GX_BG_COLORMODE_16 )
    {
      u16 *p_scr16;

      p_scr16 = (u16 *)&p_scr_data->rawData;
      for( i = 0; i < src_w * src_h ; i++ )
      {
        p_scr16[i]  += chr_ofs;
      }
    }
    else
    {
      GF_ASSERT(0); //256版は作ってない
    }
  }

  //書き込み
  if( GFL_BG_GetScreenBufferAdrs( frm ) != NULL )
  {
    GFL_BG_WriteScreenExpand( frm, dst_x, dst_y, dst_w, dst_h,
        &p_scr_data->rawData, src_x, src_y, src_w, src_h );
    GFL_BG_ChangeScreenPalette( frm, dst_x, dst_y, dst_w, dst_h, plt );
    GFL_BG_LoadScreenReq( frm );
  }
  else
  {
    GF_ASSERT(0); //バッファがない
  }

  GFL_HEAP_FreeMemory( p_src_arc );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ガイドBGの表示設定
 *
 *  @param  GUIDE_VISIBLE_MASK msk  マスク値
 *  @param  heapID                  読み込みテンポラリ用
 */
//-----------------------------------------------------------------------------
static void GUIDE_SetVisible( GUIDE_VISIBLE_MASK msk, HEAPID heapID )
{
  enum
  {
    GUIDE_DST_L_X = 0,
    GUIDE_DST_L_Y = 0,
    GUIDE_DST_L_W = 16,
    GUIDE_DST_L_H = 24,

    GUIDE_DST_R_X = 16,
    GUIDE_DST_R_Y = 0,
    GUIDE_DST_R_W = 16,
    GUIDE_DST_R_H = 24,
  };

  ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_IRCCOMPATIBLE, heapID );

  if( msk & GUIDE_VISIBLE_LEFT )
  {
    ARCHDL_UTIL_TransVramScreenEx( p_handle, NARC_irccompatible_gra_aura_bg_guide_l_NSCR,
        sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_GUIDE],
        0, 0, 0, 16, 24,
        GUIDE_DST_L_X, GUIDE_DST_L_Y, GUIDE_DST_L_W, GUIDE_DST_L_H,
        AURA_BG_PAL_M_09, FALSE, heapID );
  }
  else
  {
    GFL_BG_FillScreen( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_GUIDE], 0,
        GUIDE_DST_L_X,GUIDE_DST_L_Y,GUIDE_DST_L_W,GUIDE_DST_L_H,
        AURA_BG_PAL_M_09 );
  }

  if( msk & GUIDE_VISIBLE_RIGHT )
  {
    ARCHDL_UTIL_TransVramScreenEx( p_handle, NARC_irccompatible_gra_aura_bg_guide_r_NSCR,
        sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_GUIDE],
        0, 0, 0, 16, 24,
        GUIDE_DST_R_X, GUIDE_DST_R_Y, GUIDE_DST_R_W, GUIDE_DST_R_H,
        AURA_BG_PAL_M_09, FALSE, heapID );
  }
  else
  {
    GFL_BG_FillScreen( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_GUIDE], 0,
        GUIDE_DST_R_X,GUIDE_DST_R_Y,GUIDE_DST_R_W,GUIDE_DST_R_H,
        AURA_BG_PAL_M_09 );
  }

  GFL_BG_LoadScreenReq(sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_GUIDE]);

  GFL_ARC_CloseDataHandle( p_handle );
}

//=============================================================================
/**
 *      BACKOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  BACKOBJ_WORK *p_wk    ワーク
 *  @param  GRAPHIC_WORK *cp_grp  CLWK受け取り用グラフィック
 *  @param  type                  動作タイプ
 *  @param  color                 色
 *  @param  clwk_ofs              CLWKオフセット何番から使い始めるか
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_Init( BACKOBJ_WORK *p_wk, const GRAPHIC_WORK *cp_grp, BACKOBJ_MOVE_TYPE type, BACKOBJ_COLOR color, u32 clwk_ofs, int sf_type )
{
  GFL_STD_MemClear( p_wk, sizeof(BACKOBJ_WORK) );
  p_wk->type  = type;
  p_wk->color = color;
  p_wk->sync_now  = 0;
  p_wk->sync_max  = BACKOBJ_MOVE_SYNC;
  {
    int i;
    GFL_CLWK *p_clwk;
    for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
    {
      p_clwk  = GRAPHIC_GetClwk( cp_grp, clwk_ofs + i );
      BACKOBJ_ONE_Init( &p_wk->wk[i], p_clwk, color, sf_type );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  破棄
 *
 *  @param  BACKOBJ_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_Exit( BACKOBJ_WORK *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(BACKOBJ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  メイン処理
 *
 *  @param  BACKOBJ_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_Main( BACKOBJ_WORK *p_wk )
{
  int i;
  u32 sync;
  u16 rot;
  GFL_POINT start;
  GFL_POINT end;

  //タイプ別移動
  switch( p_wk->type )
  {
  case BACKOBJ_MOVE_TYPE_RAIN:    //雨
    if( p_wk->sync_now++ > p_wk->sync_max )
    {
      p_wk->sync_now  = 0;
      for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
      {
        if( !BACKOBJ_ONE_IsMove( &p_wk->wk[i] ) )
        {
          u8  up_or_down  = GFUser_GetPublicRand(2);

          start.x = GFUser_GetPublicRand(256);
          start.y = -36;
          end.x   = start.x;
          end.y   = 192 + 36;
          sync  = BACKOBJ_ONE_MOVE_SYNC_MIN + GFUser_GetPublicRand(BACKOBJ_ONE_MOVE_SYNC_DIF);
          if( up_or_down )
          {
            BACKOBJ_ONE_Start( &p_wk->wk[i], &end, &start, sync );
          }
          else
          {
            BACKOBJ_ONE_Start( &p_wk->wk[i], &start, &end, sync );
          }
          break;
        }
      }
    }
    break;

  case BACKOBJ_MOVE_TYPE_EMITER:  //放出
    /* BACKOBJ_StartEmitで行う  */
    break;

  case BACKOBJ_MOVE_TYPE_GATHER:  //集中
    if( p_wk->sync_now++ > p_wk->sync_max )
    {
      p_wk->sync_now  = 0;
      for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
      {
        if( !BACKOBJ_ONE_IsMove( &p_wk->wk[i] ) )
        {
          u16 rot = GFUser_GetPublicRand(0xFFFF);
          start.x = (FX_SinIdx(rot) * 256) >> FX32_SHIFT;
          start.y = (FX_CosIdx(rot) * 256) >> FX32_SHIFT;
          end.x   = 128;
          end.y   = 96;
          sync  = BACKOBJ_ONE_MOVE_SYNC_MIN + GFUser_GetPublicRand(BACKOBJ_ONE_MOVE_SYNC_DIF);
          BACKOBJ_ONE_Start( &p_wk->wk[i], &start, &end, sync );
          break;
        }
      }
    }
    break;
  }


  //ワーク動作
  {
    for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
    {
      BACKOBJ_ONE_Main( &p_wk->wk[i] );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  放出開始
 *
 *  @param  BACKOBJ_WORK *p_wk  ワーク
 *  @param  GFL_POINT *cp_pos   放出開始座標
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_StartEmit( BACKOBJ_WORK *p_wk, const GFL_POINT *cp_pos )
{
  int i;
  u32 sync;
  u16 rot;
  GFL_POINT end;
  for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
  {
    if( !BACKOBJ_ONE_IsMove( &p_wk->wk[i] ) )
    {
      rot = GFUser_GetPublicRand(0xFFFF);
      end.x = (FX_SinIdx(rot) * 256) >> FX32_SHIFT;
      end.y = (FX_CosIdx(rot) * 256) >> FX32_SHIFT;
      sync  = BACKOBJ_ONE_MOVE_SYNC_MIN + GFUser_GetPublicRand(BACKOBJ_ONE_MOVE_SYNC_DIF);
      BACKOBJ_ONE_Start( &p_wk->wk[i], cp_pos, &end, sync );
    }
  }
}
//=============================================================================
/**
 *    背面OBJ1個
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  初期化
 *
 *  @param  BACKOBJ_ONE *p_wk ワーク
 *  @param  *p_clwk           アクター
 *  @param  color             色
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_ONE_Init( BACKOBJ_ONE *p_wk, GFL_CLWK *p_clwk, BACKOBJ_COLOR color, int sf_type )
{
  GFL_STD_MemClear( p_wk, sizeof(BACKOBJ_ONE) );
  p_wk->p_clwk  = p_clwk;
  p_wk->sf_type = sf_type;

  GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk, color, CLWK_PLTTOFFS_MODE_PLTT_TOP );
}
//----------------------------------------------------------------------------
/**
 *  @brief  メイン処理
 *
 *  @param  BACKOBJ_ONE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_ONE_Main( BACKOBJ_ONE *p_wk )
{
  if( p_wk->is_req )
  {
    GFL_CLACTPOS pos;
    VecFx32 v;
    VecFx32 v2;
    fx32 mag;
    VEC_Subtract( &p_wk->end, &p_wk->start, &v );
    mag = VEC_Mag( &v );
    VEC_Normalize( &v, &v );
    mag = mag * p_wk->sync_now / p_wk->sync_max;
    VEC_MultAdd( mag, &v, &p_wk->start, &v2 );

    pos.x = v2.x >> FX32_SHIFT;
    pos.y = v2.y >> FX32_SHIFT;

    if( p_wk->sync_now++ > p_wk->sync_max )
    {
      pos.x = p_wk->end.x;
      pos.y = p_wk->end.y;
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, FALSE );

      p_wk->is_req  = FALSE;
    }

    GFL_CLACT_WK_SetPos( p_wk->p_clwk, &pos, p_wk->sf_type );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  移動開始
 *
 *  @param  BACKOBJ_ONE *p_wk   ワーク
 *  @param  GFL_POINT *cp_start 開始座標
 *  @param  GFL_POINT *cp_end   終了座標
 *  @param  sync                移動シンク
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_ONE_Start( BACKOBJ_ONE *p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, u32 sync )
{
  p_wk->is_req    = TRUE;
  VEC_Set( &p_wk->start, FX32_CONST(cp_start->x), FX32_CONST(cp_start->y), 0 );
  VEC_Set( &p_wk->end, FX32_CONST(cp_end->x), FX32_CONST(cp_end->y), 0 );
  p_wk->sync_now  = 0;
  p_wk->sync_max  = sync;
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  動作中フラグ取得
 *
 *  @param  const BACKOBJ_ONE *cp_wk  ワーク
 *
 *  @return TRUEならば動作中  FALSEならば動いていない
 */
//-----------------------------------------------------------------------------
static BOOL BACKOBJ_ONE_IsMove( const BACKOBJ_ONE *cp_wk )
{
  return cp_wk->is_req;
}
//=============================================================================
/**
 *      ブレ計測
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  ブレ計測  初期化
 *
 *  @param  SHAKE_SEARCH_WORK *p_wk   ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SHAKESEARCH_Init( SHAKE_SEARCH_WORK *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(SHAKE_SEARCH_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  ブレ計測  破棄
 *
 *  @param  SHAKE_SEARCH_WORK *p_wk   ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SHAKESEARCH_Exit( SHAKE_SEARCH_WORK *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(SHAKE_SEARCH_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  ブレ計測  計測メイン
 *
 *  @param  SHAKE_SEARCH_WORK *p_wk
 *  @param  cp_rect　計測する範囲
 *
 *  @retval TRUE  計測終了
 *  @retval FALSE 計測中
 */
//-----------------------------------------------------------------------------
static BOOL SHAKESEARCH_Main( SHAKE_SEARCH_WORK *p_wk, const GFL_RECT *cp_rect )
{

  //終了していたらTRUE
  if( p_wk->is_end )
  {
    return TRUE;
  }

  //計測終了待ち
  if( p_wk->cnt++ > TOUCH_COUNTER_MAX )
  {
    TP_GetRectCont( cp_rect, &p_wk->shake[TOUCH_COUNTER_SHAKE_MAX-1] );
    p_wk->is_end  = TRUE;
    return TRUE;
  }
  else
  {

    if( p_wk->shake_cnt == 0 )
    {
      TP_GetRectCont( cp_rect, &p_wk->shake[p_wk->shake_idx++] );
      GF_ASSERT_MSG( p_wk->shake_idx < TOUCH_COUNTER_SHAKE_MAX,
          "ブレ計測インデックスエラーです %d\n" , p_wk->shake_idx );
    }

    //計測
    if( p_wk->shake_cnt++ > TOUCH_COUNTER_SHAKE_SYNC )
    {
      p_wk->shake_cnt = 0;
    }
  }

  return FALSE;
}

#ifdef DEBUG_AURA_MSG
//=============================================================================
/**
 *        デバッグプリント用画面
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用プリント領域  初期化
 *
 *  @param  frm                     フレーム
 *  @param  heapID                  ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Init( u8 frm, BOOL is_now_save, HEAPID heapID )
{
  DEBUG_PRINT_WORK *p_wk;

  sp_dp_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(DEBUG_PRINT_WORK) );
  p_wk = sp_dp_wk;
  GFL_STD_MemClear( p_wk, sizeof(DEBUG_PRINT_WORK) );
  p_wk->heapID            = heapID;
  p_wk->is_now_save       = is_now_save;
  p_wk->frm               = frm;

  //デバッグプリント用フォント
  p_wk->p_font  = GFL_FONT_Create( ARCID_FONT,
        NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

  //退避エリアをNetEffから取得
  NetErr_GetTempArea( &p_wk->p_char_temp_area, &p_wk->p_scrn_temp_area, &p_wk->p_pltt_temp_area );
}
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用プリント領域  破棄
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Exit( void )
{
  DEBUG_PRINT_WORK *p_wk  = sp_dp_wk;

  GFL_FONT_Delete( p_wk->p_font );
  GFL_STD_MemClear( p_wk, sizeof(DEBUG_PRINT_WORK) );

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用プリント領域オープン
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Open( void )
{
  DEBUG_PRINT_WORK *p_wk  = sp_dp_wk;

  GF_ASSERT( p_wk );

  if( p_wk->is_now_save )
  {
    //VRAMのデータを退避
    GFL_STD_MemCopy16(GFL_DISPUT_GetCgxPtr(p_wk->frm), p_wk->p_char_temp_area, DEBUGPRINT_CHAR_TEMP_AREA);
    GFL_STD_MemCopy16(GFL_DISPUT_GetScrPtr(p_wk->frm), p_wk->p_scrn_temp_area, DEBUGPRINT_SCRN_TEMP_AREA);
    GFL_STD_MemCopy16(GFL_DISPUT_GetPltPtr(p_wk->frm), p_wk->p_pltt_temp_area, DEBUGPRINT_PLTT_TEMP_AREA);
    //Fontカラーの退避
    GFL_FONTSYS_GetColor( &p_wk->font_col_bkup[0] ,
        &p_wk->font_col_bkup[1] ,
        &p_wk->font_col_bkup[2] );

    //もろもろ退避
    p_wk->prioryty_bkup = GFL_BG_GetPriority(p_wk->frm);
    p_wk->scroll_x_bkup = GFL_BG_GetScrollX(p_wk->frm);
    p_wk->scroll_y_bkup = GFL_BG_GetScrollY(p_wk->frm);
  }

  //上で退避させたものの設定
  GFL_BG_SetPriority( p_wk->frm , 0 );
  GFL_BG_SetScroll( p_wk->frm , GFL_BG_SCROLL_X_SET , 0 );
  GFL_BG_SetScroll( p_wk->frm , GFL_BG_SCROLL_Y_SET , 0 );

  //デバッグプリント用設定
  //スクリーンの作成
  {
    u8 x,y;
    u16 buf;
    for( y = 0;y<DEBUGPRINT_HEIGHT;y++ )
    {
      for( x=0;x<DEBUGPRINT_WIDTH;x++ )
      {
        buf = x+y*DEBUGPRINT_WIDTH;
        GFL_BG_WriteScreen( p_wk->frm, &buf, x,y,1,1 );
      }
      for( x=DEBUGPRINT_WIDTH;x<32;x++ )
      {
        buf = DEBUGPRINT_HEIGHT*DEBUGPRINT_WIDTH;
        GFL_BG_WriteScreen( p_wk->frm, &buf, x,y,1,1 );
      }
    }
    for( y = DEBUGPRINT_HEIGHT;y<24;y++ )
    {
      buf = DEBUGPRINT_HEIGHT*DEBUGPRINT_WIDTH;
      GFL_BG_WriteScreen( p_wk->frm, &buf, x,y,1,1 );
      for( x=0;x<32;x++ )
      {
        buf = DEBUGPRINT_HEIGHT*DEBUGPRINT_WIDTH;
        GFL_BG_WriteScreen( p_wk->frm, &buf, x,y,1,1 );
      }
    }
    GFL_BG_LoadScreenReq( p_wk->frm );
  }

  //パレットの作成
  {
    u16 col[4]={ 0xFFFF , 0x0000 , 0x7fff , 0x001f };
    GFL_BG_LoadPalette( p_wk->frm, col, sizeof(u16)*4, 0 );
  }

  //書き込むためのBMP作成
  p_wk->p_bmp = GFL_BMP_CreateInVRAM( GFL_DISPUT_GetCgxPtr(p_wk->frm), DEBUGPRINT_WIDTH, DEBUGPRINT_HEIGHT, GFL_BMP_16_COLOR, p_wk->heapID );
  GFL_STD_MemClear16( GFL_DISPUT_GetCgxPtr(p_wk->frm) , DEBUGPRINT_CHAR_TEMP_AREA );
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用プリント領域  終了
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Close( void )
{
  DEBUG_PRINT_WORK *p_wk  = sp_dp_wk;

  GF_ASSERT( p_wk );

  GFL_BMP_Delete( p_wk->p_bmp );
  if( p_wk->is_now_save )
  {
    //もろもろ復帰
    GFL_BG_SetScroll( p_wk->frm , GFL_BG_SCROLL_X_SET , p_wk->scroll_x_bkup );
    GFL_BG_SetScroll( p_wk->frm , GFL_BG_SCROLL_Y_SET , p_wk->scroll_y_bkup );
    GFL_BG_SetPriority( p_wk->frm , p_wk->prioryty_bkup );
    //Fontカラーの復帰
    GFL_FONTSYS_SetColor( p_wk->font_col_bkup[0] ,
        p_wk->font_col_bkup[1] ,
        p_wk->font_col_bkup[2] );
    GFL_STD_MemCopy16(p_wk->p_char_temp_area, GFL_DISPUT_GetCgxPtr(p_wk->frm), DEBUGPRINT_CHAR_TEMP_AREA);
    GFL_STD_MemCopy16(p_wk->p_scrn_temp_area, GFL_DISPUT_GetScrPtr(p_wk->frm), DEBUGPRINT_SCRN_TEMP_AREA);
    GFL_STD_MemCopy16(p_wk->p_pltt_temp_area, GFL_DISPUT_GetPltPtr(p_wk->frm), DEBUGPRINT_PLTT_TEMP_AREA);
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用プリント領域に書き込み
 *
 *  @param  u16 *cp_str             ワイド文字列
 *  @param  x                       座標X
 *  @param  y                       座標Y
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Print( const u16 *cp_str, u16 x, u16 y )
{
  STRBUF  *p_strbuf;
  STRCODE str[128];
  u16 strlen;
  DEBUG_PRINT_WORK *p_wk  = sp_dp_wk;

  GF_ASSERT(p_wk);

  //ワイド文字をSTRBUFに変換
  p_strbuf  = DEBUGPRINT_CreateWideChar( cp_str, p_wk->heapID );

  //書き込み
  PRINTSYS_Print( p_wk->p_bmp, x, y, p_strbuf, p_wk->p_font );

  //破棄
  GFL_STR_DeleteBuffer( p_strbuf );
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用プリント領域に数値つき文字書き込み
 *
 *  @param  u16 *cp_str             ワイド文字列（%dや%fを使ってください）
 *  @param  number                  数字
 *  @param  x                       座標X
 *  @param  y                       座標Y
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_PrintNumber( const u16 *cp_str, int number, u16 x, u16 y )
{
  u16 str[128];
  swprintf( str, 128, cp_str, number );
  DEBUGPRINT_Print( str, x, y );
}
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ用プリント領域をクリアー
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Clear( void )
{
  DEBUG_PRINT_WORK *p_wk  = sp_dp_wk;
  GF_ASSERT(p_wk);
  GFL_BMP_Clear( p_wk->p_bmp, 0 );
}
#endif //DEBUG_AURA_MSG

//----------------------------------------------------------------------------
/**
 *  @brief  Wide文字列をSTRBUFに格納
 *
 *  @param  const u16 *cp_char  ワイド文字列
 *  @param  heapID              ヒープID
 *
 *  @return STRBUF
 */
//-----------------------------------------------------------------------------
static STRBUF * DEBUGPRINT_CreateWideChar( const u16 *cp_str, HEAPID heapID )
{
  STRBUF  *p_strbuf;
  STRCODE str[128];
  u16 strlen;

  //STRBUF用に変換
  strlen  = wcslen(cp_str);
  GFL_STD_MemCopy(cp_str, str, strlen*2);
  str[strlen] = GFL_STR_GetEOMCode();

  //STRBUFに転送
  p_strbuf  = GFL_STR_CreateBuffer( strlen*2, heapID );
  GFL_STR_SetStringCode( p_strbuf, str);

  return p_strbuf;
}

//----------------------------------------------------------------------------
/**
 *  @brief  数値つきWide文字列をSTRBUFに格納
 *
 *  @param  const u16 *cp_char  ワイド文字列
 *  @param  number              数値
 *  @param  heapID              ヒープID
 *
 *  @return STRBUF
 */
//-----------------------------------------------------------------------------
static STRBUF * DEBUGPRINT_CreateWideCharNumber( const u16 *cp_str, int number, HEAPID heapID )
{
  u16 str[128];
  swprintf( str, 128, cp_str, number );
  return DEBUGPRINT_CreateWideChar( str, heapID );
}
//#endif //DEBUG_AURA_MSG

