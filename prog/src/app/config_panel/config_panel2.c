//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file     config_panel2.c
 *  @brief    コンフィグ画面
 *  @author   Toru=Nagihashi
 *  @data     2009.09.15
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//library
#include <gflib.h>

//system
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID
#include "system/bmp_winframe.h"
#include "sound/pm_sndsys.h"
#include "gamesystem/msgspeed.h"

//module
#include "app/app_menu_common.h"
#include "app/app_taskmenu.h"

//ui_common
#include "ui/touchbar.h"

//archive
#include "arc_def.h"
#include "config_gra.naix"

//print
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_config.h"
#include "print/gf_font.h"
#include "print/printsys.h"

//mine
#include "app/config_panel.h"
#include "config_snd.h"
#include "savedata/config.h"

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *          定数宣言
*/
//=============================================================================
//-------------------------------------
/// デバッグ
//=====================================
#ifdef PM_DEBUG
#define GAMESYS_NONE_MOVE //ゲームシステムがなくても動く
#endif //PM_DEBUG

//-------------------------------------
/// 切り替えマクロ
//=====================================
#define CONFIG_KEY_TOUCH  //キーとタッチの読み替えON

//-------------------------------------
/// パレット
//=====================================
enum
{
  // メイン画面BG                           0~0xD                   0xE 0xF
  CONFIG_BG_PAL_M_00 = 0,// BG用パレット先頭  バー明るい色    フォント未選択OFF色
  CONFIG_BG_PAL_M_01,   //                    バー普通の色
  CONFIG_BG_PAL_M_02,   //
  CONFIG_BG_PAL_M_03,   //                    バー暗い色      フォント未選択OFF色
  CONFIG_BG_PAL_M_04,   //
  CONFIG_BG_PAL_M_05,   //                    バー一番くらい
  CONFIG_BG_PAL_M_06,   //
  CONFIG_BG_PAL_M_07,   //  選択色バーパレットフェード        フォント選択中ON色
  CONFIG_BG_PAL_M_08,   //  TITLE
  CONFIG_BG_PAL_M_09,   //  CONFIRMWND
  CONFIG_BG_PAL_M_10,   //
  CONFIG_BG_PAL_M_11,   //
  CONFIG_BG_PAL_M_12,   //                                    フォント選択中OFF色
  CONFIG_BG_PAL_M_13,   //  TASKWND
  CONFIG_BG_PAL_M_14,   //
  CONFIG_BG_PAL_M_15,   //  CONFIRMTEXT


  // サブ画面BG
  CONFIG_BG_PAL_S_00 = 0,// 背景
  CONFIG_BG_PAL_S_01,   //
  CONFIG_BG_PAL_S_02,   //
  CONFIG_BG_PAL_S_03,   //
  CONFIG_BG_PAL_S_04,   //  FRM
  CONFIG_BG_PAL_S_05,   //
  CONFIG_BG_PAL_S_06,   //
  CONFIG_BG_PAL_S_07,   //
  CONFIG_BG_PAL_S_08,   //
  CONFIG_BG_PAL_S_09,   //
  CONFIG_BG_PAL_S_10,   //
  CONFIG_BG_PAL_S_11,   //
  CONFIG_BG_PAL_S_12,   //
  CONFIG_BG_PAL_S_13,   //
  CONFIG_BG_PAL_S_14,   //
  CONFIG_BG_PAL_S_15,   //

  // メイン画面OBJ
  CONFIG_OBJ_PAL_M_00 = 0,//  OBJ用パレット先頭 明るい色
  CONFIG_OBJ_PAL_M_01,    //                    普通の色
  CONFIG_OBJ_PAL_M_02,    //
  CONFIG_OBJ_PAL_M_03,    //                    暗い色
  CONFIG_OBJ_PAL_M_04,    //
  CONFIG_OBJ_PAL_M_05,    //
  CONFIG_OBJ_PAL_M_06,    //  したばー
  CONFIG_OBJ_PAL_M_07,    //  選択色
  CONFIG_OBJ_PAL_M_08,    //  したバー共通素材
  CONFIG_OBJ_PAL_M_09,    //
  CONFIG_OBJ_PAL_M_10,    //
  CONFIG_OBJ_PAL_M_11,    //
  CONFIG_OBJ_PAL_M_12,    //
  CONFIG_OBJ_PAL_M_13,    //
  CONFIG_OBJ_PAL_M_14,    //
  CONFIG_OBJ_PAL_M_15,    //


};

//-------------------------------------
/// OBJリソース登録ID
//=====================================
enum
{
  OBJRESID_ITEM_PLT,
  OBJRESID_ITEM_CHR,
  OBJRESID_ITEM_CEL,
  OBJRESID_BAR_CHR,
  OBJRESID_BAR_CEL,
  OBJRESID_MAX
};
//-------------------------------------
/// コンフィグのメニュー
//=====================================
typedef enum
{
  CONFIG_LIST_MSGSPEED,
  CONFIG_LIST_BTLEFF,
  CONFIG_LIST_BTLRULE,
  CONFIG_LIST_SND,
  CONFIG_LIST_STR,
  CONFIG_LIST_WIRELESS,
  CONFIG_LIST_REPORT,


  CONFIG_LIST_MAX,  //最大

  CONFIG_LIST_INIT    = CONFIG_LIST_MSGSPEED-1, //初期
  CONFIG_LIST_DECIDE  = CONFIG_LIST_MAX,  //下
  CONFIG_LIST_CANCEL, //下
} CONFIG_LIST;

//-------------------------------------
/// コンフィグアイテムの数
//=====================================
typedef enum
{
  CONFIG_ITEM_MSG_SLOW,
  CONFIG_ITEM_MSG_NORMAL,
  CONFIG_ITEM_MSG_FAST,
  CONFIG_ITEM_BTLEFF_YES,
  CONFIG_ITEM_BTLEFF_NO,
  CONFIG_ITEM_BTLRULE_IREKAE,
  CONFIG_ITEM_BTLRULE_KACHINUKI,
  CONFIG_ITEM_SND_STEREO,
  CONFIG_ITEM_SND_MONO,
  CONFIG_ITEM_STR_HIRAGANA,
  CONFIG_ITEM_STR_KANJI,
  CONFIG_ITEM_WIRELESS_YES,
  CONFIG_ITEM_WIRELESS_NO,
  CONFIG_ITEM_REPORT_YES,
  CONFIG_ITEM_REPORT_NO,

  CONFIG_ITEM_MAX,
  CONFIG_ITEM_NONE  = CONFIG_ITEM_MAX,
}CONFIG_ITEM;

//-------------------------------------
/// CLWKのID
//=====================================
typedef enum
{
  CLWKID_CHECK,
  CLWKID_DOWNBAR,

  CLWKID_ITEM_TOP,
  CLWKID_ITEM_END = CLWKID_ITEM_TOP + CONFIG_ITEM_MAX,

  CLWKID_MAX
}CLWKID;
//-------------------------------------
/// BMPWINID
//=====================================
typedef enum
{
  BMPWINID_ITEM_TOP,
  BMPWINID_ITEM_END = BMPWINID_ITEM_TOP + CONFIG_ITEM_MAX - 1,
  BMPWINID_ITEM_MAX,
  BMPWINID_LIST_TOP = BMPWINID_ITEM_MAX,
  BMPWINID_LIST_END = BMPWINID_LIST_TOP + CONFIG_LIST_MAX - 1,
  BMPWINID_LIST_MAX,
  BMPWINID_CONFIG_MENU_MAX  = BMPWINID_LIST_MAX,
  BMPWINID_TITLE  = BMPWINID_CONFIG_MENU_MAX,
  BMPWINID_TEXT,
  BMPWINID_CONFIRM,

  BMPWINID_MAX
}BMPWINID;

//-------------------------------------
/// SCROLL
//=====================================
#define SCROLL_WINDOW_OFS_CHR   (2)       //ウィンドウBGの開始オフセット
#define SCROLL_WINDOW_OFS_DOT   (SCROLL_WINDOW_OFS_CHR*GFL_BG_1CHRDOTSIZ)       //ウィンドウBGの開始オフセット
#define SCROLL_FONT_OFS_CHR ((4+SCROLL_WINDOW_OFS_DOT))       //ウィンドウとフォントの差オフセット
#define SCROLL_START_OFS_Y  (-(36-SCROLL_WINDOW_OFS_DOT))   //開始オフセット
#define SCROLL_WINDOW_H_CHR (3)                         //１つのウィンドウの幅（キャラ単位）
#define SCROLL_WINDOW_H_DOT (SCROLL_WINDOW_H_CHR*GFL_BG_1CHRDOTSIZ)   //１つのウィンドウの幅（ドット単位）
#define SCROLL_DISTANCE     (SCROLL_WINDOW_H_DOT*CONFIG_LIST_MAX)     //スクロールする距離
#define SCROLL_START        (SCROLL_START_OFS_Y)                                //開始
#define SCROLL_END          (-(156-(SCROLL_WINDOW_OFS_DOT+SCROLL_DISTANCE)))    //終了
#define SCROLL_DISPLAY      (4) //画面から見えるウィンドウの数
#define SCROLL_START_OBJ    (SCROLL_WINDOW_OFS_DOT-SCROLL_START_OFS_Y)  //開始
#define SCROLL_END_OBJ      (156-(SCROLL_START_OBJ+SCROLL_DISTANCE)+36)   //終了
#define SCROLL_CONT_SYNC_MAX  (10)  //押しっぱなしの移動間隔
#define SCROLL_TOP_BAR_Y    (3*GFL_BG_1CHRDOTSIZ)
#define SCROLL_APP_BAR_Y    (192-3*GFL_BG_1CHRDOTSIZ)

#define SCROLL_CHAGEPLT_SAFE_SYNC (6)   //何シンク同じならばOKか

//-------------------------------------
/// UIの種類
//=====================================
typedef enum
{
  UI_INPUT_NONE,      //入力なし
  UI_INPUT_SLIDE,     //タッチスライド
  UI_INPUT_FLICK,     //タッチはじき
  UI_INPUT_TOUCH,     //タッチトリガー
  UI_INPUT_TRG_UP,    //キー上
  UI_INPUT_TRG_DOWN,  //キー下
  UI_INPUT_TRG_RIGHT, //キー右
  UI_INPUT_TRG_LEFT,  //キー左
  UI_INPUT_TRG_DECIDE,//キー決定
  UI_INPUT_TRG_CANCEL,//キーキャンセル
  UI_INPUT_CONT_UP,   //キー上
  UI_INPUT_CONT_DOWN, //キー下
  UI_INPUT_TRG_Y, //キーY
  UI_INPUT_TRG_X, //キーX
} UI_INPUT;

//-------------------------------------
/// UIの情報
//=====================================
typedef enum
{
  UI_INPUT_PARAM_TRGPOS,
  UI_INPUT_PARAM_SLIDEPOS,
  UI_INPUT_PARAM_SLIDENOW,
} UI_INPUT_PARAM;


//-------------------------------------
/// UI
//=====================================
#define UI_SLIDE_DISTANCE_START (1) //この距離以上ならばスライドする

//-------------------------------------
/// 位置
//=====================================
//アプリケーションバー
#define APPBAR_MENUBAR_X  (0)
#define APPBAR_MENUBAR_Y  (21)
#define APPBAR_MENUBAR_W  (32)
#define APPBAR_MENUBAR_H  (3)

#define APPBAR_WIN_X  (14)
#define APPBAR_WIN_W  (9)

//タイトルバー
#define TITLEBAR_MENUBAR_X  (0)
#define TITLEBAR_MENUBAR_Y  (0)
#define TITLEBAR_MENUBAR_W  (32)
#define TITLEBAR_MENUBAR_H  (3)

//解説ウィンドウ
#define INFOWND_X   (1)
#define INFOWND_Y   (19)
#define INFOWND_W   (30)
#define INFOWND_H   (4)

//タイトル文字
#define TITLEWND_X  (0)
#define TITLEWND_Y  (0)
#define TITLEWND_W  (16)
#define TITLEWND_H  (3)

//確認
#define CONFIRMWND_X  (1)
#define CONFIRMWND_Y  (1)
#define CONFIRMWND_W  (30)
#define CONFIRMWND_H  (4)

//チェックボタン
#define CLWK_CHECK_X  (88)
#define CLWK_CHECK_Y  (172)

//-------------------------------------
/// PARETTLE_FADE
//=====================================
#define PLTFADE_SELECT_ADD  (0x400)
#define PLTFADE_SELECT_STARTCOLOR GX_RGB( 7, 13, 20 )
#define PLTFADE_SELECT_ENDCOLOR GX_RGB( 12, 25, 30 )

//-------------------------------------
/// APP
//=====================================
typedef enum
{
  APPBAR_WIN_DECIDE,
  APPBAR_WIN_CANCEL,
  APPBAR_WIN_EXIT,
  APPBAR_WIN_MAX,
  APPBAR_WIN_NULL = APPBAR_WIN_MAX,
}APPBAR_WIN_LIST;

//-------------------------------------
/// CONFIRM
//=====================================
typedef enum
{
  CONFIRM_SELECT_NULL,
  CONFIRM_SELECT_YES,
  CONFIRM_SELECT_NO,
} CONFIRM_SELECT;

enum
{
  CONFIRM_WIN_YES,
  CONFIRM_WIN_NO,
  CONFIRM_WIN_MAX,
};

//-------------------------------------
/// ETC
//=====================================
#define APPBAR_BG_CHARAAREA_SIZE  (4*GFL_BG_1CHRDATASIZ)
#define TITLEBAR_BG_CHARAAREA_SIZE  (32*4*GFL_BG_1CHRDATASIZ)


//=============================================================================
/**
 *          構造体宣言
*/
//=============================================================================
//-------------------------------------
/// コンフィグパラメータ
//=====================================
typedef struct
{
  u8 param[CONFIG_LIST_MAX];
  u8 change;
  CONFIG *p_savedata;
}CONFIG_PARAM;
//-------------------------------------
/// BGワーク
//=====================================
typedef struct
{
  GFL_BMPWIN  *p_bmpwin[BMPWINID_MAX];
  u16                   pltfade_cnt[2];
  GXRgb                 trans_color[2];
  void                  *ncg_buf;
  NNSG2dCharacterData   *ncg_data;

  GXRgb                 plt_color[0x10];

} GRAPHIC_BG_WORK;
//-------------------------------------
/// OBJワーク
//=====================================
typedef struct
{
  GFL_CLUNIT *p_clunit;
  u32         reg_id[OBJRESID_MAX];
  GFL_CLWK   *p_clwk[CLWKID_MAX];
  u16         anm_idx;
  u16         dummy;
} GRAPHIC_OBJ_WORK;
//-------------------------------------
/// GRAPHICワーク
//=====================================
typedef struct
{
  GRAPHIC_BG_WORK   bg;
  GRAPHIC_OBJ_WORK  obj;
  GFL_TCB           *p_vblank_task;
} GRAPHIC_WORK;
//-------------------------------------
/// シーケンス管理
//=====================================
typedef struct _SEQ_WORK SEQ_WORK;  //関数型作るため前方宣言
typedef void (*SEQ_FUNCTION)( SEQ_WORK *p_wk, int *p_seq, void *p_param );
struct _SEQ_WORK
{
  SEQ_FUNCTION  seq_function;
  BOOL is_end;
  int seq;
  void *p_param;
};
//-------------------------------------
/// UI
//=====================================
typedef struct
{
  UI_INPUT  input;
  GFL_POINT tp_pos;
  GFL_POINT slide;
  GFL_POINT slide_start;
  BOOL      is_start_slide;
} UI_WORK;
//-------------------------------------
/// メッセージウィンドウ
//=====================================
typedef struct
{
  GFL_MSGDATA       *p_msg;
  GFL_FONT          *p_font;
  PRINT_STREAM      *p_stream;
  GFL_TCBLSYS       *p_tcbl;
  GFL_BMPWIN*       p_bmpwin;
  STRBUF*           p_strbuf;
  u16               clear_chr;
  u16               heapID;
  PRINT_UTIL        util;
  PRINT_QUE         *p_que;
  u32               print_update;
} MSGWND_WORK;
//-------------------------------------
/// 最終確認画面 CONFIRM
//=====================================
typedef struct
{
  MSGWND_WORK msg;
  APP_TASKMENU_WORK     *p_menu;
  APP_TASKMENU_ITEMWORK item[ CONFIRM_WIN_MAX ];
  u16 select;
  u16 seq;
  APP_TASKMENU_INITWORK init;
  APP_TASKMENU_RES      *p_res;
} CONFIRM_WORK;

//-------------------------------------
/// APPBAR
//=====================================
typedef struct
{
  GFL_FONT        *p_font;
  PRINT_QUE       *p_que;
  TOUCHBAR_WORK   *p_touch;
  APPBAR_WIN_LIST select;
  BOOL            is_decide;
  APP_TASKMENU_RES *p_res;
  GFL_MSGDATA     *p_msg;
  APP_TASKMENU_ITEMWORK item[ APPBAR_WIN_MAX-1 ];
  APP_TASKMENU_WIN_WORK *p_win[ APPBAR_WIN_MAX-1 ];
  GAMEDATA        *p_gdata;
} APPBAR_WORK;
//-------------------------------------
/// SCROLL
//=====================================
typedef struct
{
  u16 font_frm;
  u16 back_frm;
  CONFIG_LIST select;
  GFL_CLWK *p_item[CONFIG_ITEM_MAX];
  int obj_y_ofs;
  u32 cont_sync;
  u32 dir_bit;
  int pre_y;
  CONFIG_PARAM  now;
  BOOL is_info_update;
} SCROLL_WORK;
//-------------------------------------
/// コンフィグメインワーク
//=====================================
typedef struct
{
  //グラフィックシステム
  GRAPHIC_WORK  graphic;

  //シーケンスシステム
  SEQ_WORK      seq;

  //UIワーク
  UI_WORK       ui;

  //タッチバー
  APPBAR_WORK   appbar;

  //スクロール
  SCROLL_WORK   scroll;

  //上画面解説文ウィンドウ
  MSGWND_WORK   info;

  //最終確認選択
  CONFIRM_WORK  confirm;

  //以前の設定情報
  CONFIG_PARAM  pre;

  //共通で使うフォント
  GFL_FONT      *p_font;

  //共通で使うキュー
  PRINT_QUE     *p_que;

  //共通で使うメッセージデータ
  GFL_MSGDATA   *p_msg;

  //共通タスクメニュー用リソース
  APP_TASKMENU_RES  *p_menures;
  APP_TASKMENU_RES  *p_confirmres;

  //共通で使うタスクシステム
  GFL_TCBLSYS       *p_tcbl;

  //引数
  CONFIG_PANEL_PARAM  *p_param;
} CONFIG_WORK;

//=============================================================================
/**
 *          プロトタイプ宣言
 *              モジュール名が大文字  XXXX_Init
 *                →ソース内グローバル関数（パブリック）
 *              モジュール名が小文字  Xxxx_Init
 *                →モジュール内関数（プライベート）
*/
//=============================================================================
//-------------------------------------
/// プロセス
//=====================================
static GFL_PROC_RESULT CONFIG_PROC_Init
  ( GFL_PROC *p_proc,int *p_seq, void *p_param_adrs, void *p_wk_adrs);
static GFL_PROC_RESULT CONFIG_PROC_Main
  ( GFL_PROC *p_proc,int *p_seq, void *p_param_adrs, void *p_wk_adrs);
static GFL_PROC_RESULT CONFIG_PROC_Exit
  ( GFL_PROC *p_proc,int *p_seq, void *p_param_adrs, void *p_wk_adrs);
//-------------------------------------
/// グラフィック
//=====================================
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk );
static void GRAPHIC_Main( GRAPHIC_WORK *p_wk );
static GFL_CLWK * GRAPHIC_GetClwk( const GRAPHIC_WORK *cp_wk, CLWKID id );
static GFL_CLUNIT *GRAPHIC_GetUnit( const GRAPHIC_WORK *cp_wk );
static GFL_BMPWIN * GRAPHIC_GetBmpwin( const GRAPHIC_WORK *cp_wk, BMPWINID id );
static void GRAPHIC_PrintBmpwin( GRAPHIC_WORK *p_wk );
static void GRAPHIC_StartPalleteFade( GRAPHIC_WORK *p_wk );
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work );
//-------------------------------------
/// BG
//=====================================
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_Main( GRAPHIC_BG_WORK *p_wk );
static GFL_BMPWIN *GRAPHIC_BG_GetBmpwin( const GRAPHIC_BG_WORK *cp_wk, BMPWINID id );
static void GRAPHIC_BG_PrintBmpwin( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_StartPalletFade( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk );
static void Graphic_Bg_PalletFadeMain( u16 *p_buff, u16 *p_cnt, u16 add, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end );
//-------------------------------------
/// OBJ
//=====================================
static void GRAPHIC_OBJ_Init
  ( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID );
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk );
static GFL_CLWK* GRAPHIC_OBJ_GetClwk( const GRAPHIC_OBJ_WORK *cp_wk, CLWKID id );
static GFL_CLUNIT *GRAPHIC_OBJ_GetUnit( const GRAPHIC_OBJ_WORK *cp_wk );
//-------------------------------------
/// UI
//=====================================
static void UI_Init( UI_WORK *p_wk, HEAPID heapID );
static void UI_Exit( UI_WORK *p_wk );
static void UI_Main( UI_WORK *p_wk );
static UI_INPUT UI_GetInput( const UI_WORK *cp_wk );
static void UI_GetParam( const UI_WORK *cp_wk, UI_INPUT_PARAM param, GFL_POINT *p_data );
static BOOL UI_IsKey( UI_INPUT input );
//-------------------------------------
/// MSGWND
//=====================================
static void MSGWND_Init( MSGWND_WORK* p_wk, GFL_BMPWIN *p_bmpwin, GFL_FONT *p_font, GFL_MSGDATA *p_msg, GFL_TCBLSYS *p_tcbl, PRINT_QUE *p_que, HEAPID heapID );
static void MSGWND_Exit( MSGWND_WORK* p_wk );
static void MSGWND_Print( MSGWND_WORK* p_wk, u32 strID, int wait );
static BOOL MSGWND_IsEndMsg( MSGWND_WORK* p_wk );
//-------------------------------------
/// APPBAR
//=====================================
static void APPBAR_Init( APPBAR_WORK *p_wk, GFL_CLUNIT *p_clunit, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, APP_TASKMENU_RES *p_res, GAMEDATA *p_gdata,HEAPID heapID );
static void APPBAR_Exit( APPBAR_WORK *p_wk );
static void APPBAR_Main( APPBAR_WORK *p_wk, const UI_WORK *cp_ui, const SCROLL_WORK *cp_scroll );
static void APPBAR_MoveMain( APPBAR_WORK *p_wk );
static BOOL APPBAR_IsDecide( const APPBAR_WORK *cp_wk, APPBAR_WIN_LIST *p_select );
static BOOL APPBAR_IsWaitEffect( const APPBAR_WORK *cp_wk );
static void APPBAR_StopEffect( APPBAR_WORK *p_wk );
static void APPBAR_ReWrite( APPBAR_WORK *p_wk, HEAPID heapID );
//-------------------------------------
/// SCROLL
//=====================================
static void SCROLL_Init( SCROLL_WORK *p_wk, u8 font_frm, u8 back_frm, const GRAPHIC_WORK *cp_grp, const CONFIG_PARAM *cp_param, HEAPID heapID );
static void SCROLL_Exit( SCROLL_WORK *p_wk );
static void SCROLL_Main( SCROLL_WORK *p_wk, const UI_WORK *cp_ui, MSGWND_WORK *p_msg, GRAPHIC_WORK *p_graphic, APPBAR_WORK *p_appbar );
static CONFIG_LIST SCROLL_GetSelect( const SCROLL_WORK *cp_wk );
static void SCROLL_GetConfigParam( const SCROLL_WORK *cp_wk, CONFIG_PARAM *p_param );
static void Scroll_ChangePlt( SCROLL_WORK *p_wk, BOOL is_decide_draw );
static void Scroll_Move( SCROLL_WORK *p_wk, int y_add );
static void Scroll_TouchItem( SCROLL_WORK *p_wk, const GFL_POINT *cp_pos );
static void Scroll_SelectItem( SCROLL_WORK *p_wk, s8 dir );
static CONFIG_LIST Scroll_PosToList( const SCROLL_WORK *cp_wk, const GFL_POINT *cp_pos );
static void Scroll_ChangePlt_Safe_Main( SCROLL_WORK *p_wk );
static void Scroll_ChangePlt_Safe_Check( SCROLL_WORK *p_wk, int y );
static BOOL Scroll_ChangePlt_Safe_IsOk( const SCROLL_WORK *cp_wk );
//-------------------------------------
/// CONFIRM
//=====================================
static void CONFIRM_Init( CONFIRM_WORK *p_wk, GFL_BMPWIN *p_bmpwin, GFL_FONT *p_font, GFL_MSGDATA *p_msg, GFL_TCBLSYS *p_tcbl, APP_TASKMENU_RES *p_res, HEAPID heapID );
static void CONFIRM_Exit( CONFIRM_WORK *p_wk );
static void CONFIRM_Main( CONFIRM_WORK *p_wk );
static void CONFIRM_Start( CONFIRM_WORK *p_wk, int wait );
static CONFIRM_SELECT CONFIRM_Select( const CONFIRM_WORK *cp_wk );
//-------------------------------------
/// CONFIG_PARAM
//=====================================
static void CONFIGPARAM_Init( CONFIG_PARAM *p_wk, CONFIG *p_savedata );
static void CONFIGPARAM_Exit( CONFIG_PARAM *p_wk );
static void CONFIGPARAM_Save( const CONFIG_PARAM *cp_wk );
static void CONFIGPARAM_SetItem( CONFIG_PARAM *p_wk, CONFIG_ITEM item );
static void CONFIGPARAM_AddItem( CONFIG_PARAM *p_wk, CONFIG_LIST list, s8 dir );
static BOOL CONFIGPARAM_IsItemSetting( const CONFIG_PARAM *cp_wk, CONFIG_ITEM item );
static int CONFIGPARAM_GetMsgSpeed( const CONFIG_PARAM *cp_wk );
static CONFIG_ITEM CONFIGPARAM_GetChangeTiming( CONFIG_PARAM *p_wk );
static BOOL CONFIGPARAM_Compare( const CONFIG_PARAM *cp_wk1, const CONFIG_PARAM *cp_wk2 );
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
/// SEQFUNC
//=====================================
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_Decide( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_SetPreConfig( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_SetNowConfig( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
//-------------------------------------
/// ETC
//=====================================
static void ARCHDL_UTIL_TransVramScreenEx( ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 chr_ofs, u8 src_x, u8 src_y, u8 src_w, u8 src_h, u8 dst_x, u8 dst_y, u8 dst_w, u8 dst_h, u8 plt, BOOL compressedFlag, HEAPID heapID );
static BOOL COLLISION_IsRectXPos( const GFL_RECT *cp_rect, const GFL_POINT *cp_pos );

//=============================================================================
/**
 *          外部公開
 */
//=============================================================================
//-------------------------------------
/// コンフィグプロセス
//=====================================
const GFL_PROC_DATA ConfigPanelProcData =
{
  CONFIG_PROC_Init,
  CONFIG_PROC_Main,
  CONFIG_PROC_Exit
};

//=============================================================================
/**
 *          データ
 */
//=============================================================================
//-------------------------------------
/// BG設定
//=====================================
enum
{
  GRAPHIC_BG_FRAME_BAR_M,       //バーやタイトル
  GRAPHIC_BG_FRAME_FONT_M,      //メインフォント
  GRAPHIC_BG_FRAME_WND_M,       //ウィンドウ
  GRAPHIC_BG_FRAME_DECIDE_M,    //最終確認ウィンドウ

  GRAPHIC_BG_FRAME_TEXT_S,      //ウィンドウ
  GRAPHIC_BG_FRAME_BACK_S,      //背景

  GRAPHIC_BG_FRAME_MAX
};
static const struct
{
  u32                 frame;
  GFL_BG_BGCNT_HEADER bgcnt_header;
  u32                 mode;
} sc_bgsetup[GRAPHIC_BG_FRAME_MAX]  =
{
  //MAIN
  //GRAPHIC_BG_FRAME_BAR_M
  {
    GFL_BG_FRAME0_M,
    {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
    },
    GFL_BG_MODE_TEXT
  },
  //GRAPHIC_BG_FRAME_FONT_M
  {
    GFL_BG_FRAME1_M,
    {
      0, 0, 0x1000, 0,
      GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
    },
    GFL_BG_MODE_TEXT
  },
  //GRAPHIC_BG_FRAME_WND_M
  {
    GFL_BG_FRAME2_M,
    {
      0, 0, 0x1000, 0,
      GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x1800, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
    },
    GFL_BG_MODE_TEXT
  },
  //GRAPHIC_BG_FRAME_DECIDE_M
  {
    GFL_BG_FRAME3_M,
    {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x2800, GX_BG_CHARBASE_0x14000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    },
    GFL_BG_MODE_TEXT
  },

  //SUB
  {
    GFL_BG_FRAME0_S,
    {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    },
    GFL_BG_MODE_TEXT
  },
  {
    GFL_BG_FRAME1_S,
    {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    },
    GFL_BG_MODE_TEXT
  },

};
//フレーム取得用マクロ
#define GRAPHIC_BG_GetFrame( x )  (sc_bgsetup[ x ].frame)

//-------------------------------------
/// 文字用のBMPWIN作成データ
//    一括で作成すると足りないので個別に作って
//    1024キャラ以内にしています
//=====================================
static const struct
{
  u32     strID;  //文字番号
  u8      x;      //BMPWIN座標X
  u8      y;      //BMPWIN座標Y
  u8      w;      //BMPWIN座標幅
  u8      h;      //BMPWIN座標高さ
} sc_config_menu_bmpwin_data[BMPWINID_CONFIG_MENU_MAX]  =
{

  //おそい
  {
    mes_config_sm01_00,
    //15,5,5,2
    14,5,6,2
  },
  //普通
  {
    mes_config_sm01_01,
    //21,5,5,2
    20,5,6,2
  },
  //早い
  {
    mes_config_sm01_02,
    26,5,6,2
    //27,5,5,2  //本来の値※１
  },
  //みる
  {
    mes_config_sm02_00,
    15,8,4,2
  },
  //みない
  {
    mes_config_sm02_01,
    24,8,6,2
  },
  //いれかえ
  {
    mes_config_sm03_00,
    15,11,8,2,
  },
  //かちぬき
  {
    mes_config_sm03_01,
    24,11,8,2
  },

  //ステレオ
  {
    mes_config_sm04_00,
    15,14,6,2
  },
  //モノラル
  {
    mes_config_sm04_01,
    24,14,6,2
  },
  //ひらがな
  {
    mes_config_sm05_00,
    15,17,8,2
  },
  //漢字
  {
    mes_config_sm05_01,
    24,17,8,2
  },
  //オン
  {
    mes_config_sm06_00,
    15,20,8,2
  },
  //オフ
  {
    mes_config_sm06_01,
    24,20,8,2
  },
  //かく
  {
    mes_config_sm07_00,
    15,23,6,2
  },
  //かかない
  {
    mes_config_sm07_01,
    24,23,7,2
  },
  //○話の早さ
  {
    mes_config_menu01,
    1,5,12,2
  },
  //○戦闘アニメ
  {
    mes_config_menu02,
    1,8,10,2
  },
  //○試合のルール
  {
    mes_config_menu03,
    1,11,12,2
  },
  //○サウンド
  {
    mes_config_menu04,
    1,14,6,2
  },
  //○文字モード
  {
    mes_config_menu05,
    1,17,8,2
  },
  //○無線
  {
    mes_config_menu06,
    1,20,10,2
  },
  //○ワイヤレス接続前レポート
  {
    mes_config_menu07,
    1,23,10,2
  },
};

//-------------------------------------
/// メニューの情報
//=====================================
static const struct
{
  u16 infoID;   //対応する説明メッセージ
  u16 max;      //項目の最大数
  u8 item[3]; //項目
} sc_list_info[CONFIG_LIST_MAX] =
{
  //CONFIG_LIST_MSGSPEED,
  {
    mes_config_comment01,
    MSGSPEED_CONFIG_MAX,
    {
      CONFIG_ITEM_MSG_SLOW,
      CONFIG_ITEM_MSG_NORMAL,
      CONFIG_ITEM_MSG_FAST,
    },
  },
  //CONFIG_LIST_BTLEFF,
  {
    mes_config_comment02,
    WAZAEFF_MODE_MAX,
    {
      CONFIG_ITEM_BTLEFF_YES,
      CONFIG_ITEM_BTLEFF_NO,
    },
  },
  //CONFIG_LIST_BTLRULE,
  {
    mes_config_comment03,
    BATTLERULE_MAX,
    {
      CONFIG_ITEM_BTLRULE_IREKAE,
      CONFIG_ITEM_BTLRULE_KACHINUKI,
    },
  },
  //CONFIG_LIST_SND,
  {
    mes_config_comment04,
    SOUNDMODE_MAX,
    {
      CONFIG_ITEM_SND_STEREO,
      CONFIG_ITEM_SND_MONO,
    },
  },
  //CONFIG_LIST_STR,
  {
    mes_config_comment05,
    MOJIMODE_MAX,
    {
      CONFIG_ITEM_STR_HIRAGANA,
      CONFIG_ITEM_STR_KANJI,
    },
  },
  //CONFIG_LIST_WIRELESS,
  {
    mes_config_comment06,
    NETWORK_SEARCH_MAX,
    {
      CONFIG_ITEM_WIRELESS_YES,
      CONFIG_ITEM_WIRELESS_NO,
    },
  },
  //CONFIG_LIST_REPORT,
  {
    mes_config_comment07,
    WIRELESSSAVE_MAX,
    {
      CONFIG_ITEM_REPORT_YES,
      CONFIG_ITEM_REPORT_NO,
    }
  },
};

//-------------------------------------
/// アイテムの情報
//=====================================
static const struct
{
  CONFIG_LIST list;     //対応するリスト
  u32         idx;      //リストのインデックス
  GFL_POINT   obj_pos;  //OBJの位置
  struct
  {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
  } touch_pos;//タッチ座標
}sc_item_info[CONFIG_ITEM_MAX]  =
{
  //  CONFIG_ITEM_MSG_SLOW,
  {
    CONFIG_LIST_MSGSPEED,
    MSGSPEED_SLOW,
    {
      15*8,0
    },
    {
      14*8,0,6*8,24
    },
  },
  //  CONFIG_ITEM_MSG_NORMAL,
  {
    CONFIG_LIST_MSGSPEED,
    MSGSPEED_NORMAL,
    {
      21*8-3,0
    },
    {
      20*8,0,6*8,24,
    }
  },
  //  CONFIG_ITEM_MSG_FAST,
  {
    CONFIG_LIST_MSGSPEED,
    MSGSPEED_FAST,
    {
      27*8-5,0
    },
    {
      26*8,0,6*8,24,
    }
  },
  //  CONFIG_ITEM_BTLEFF_YES,
  {
    CONFIG_LIST_BTLEFF,
    WAZAEFF_MODE_ON,
    {
      15*8,3*8
    },
    {
      14*8,3*8,9*8,24
    }
  },
  //  CONFIG_ITEM_BTLEFF_NO,
  {
    CONFIG_LIST_BTLEFF,
    WAZAEFF_MODE_OFF,
    {
      24*8,3*8
    },
    {
      23*8,3*8,9*8,24
    }
  },
  //  CONFIG_ITEM_BTLRULE_IREKAE,
  {
    CONFIG_LIST_BTLRULE,
    BATTLERULE_IREKAE,
    {
      15*8,6*8
    },
    {
      14*8,6*8,9*8,24
    }
  },
  //  CONFIG_ITEM_BTLRULE_KACHINUKI,
  {
    CONFIG_LIST_BTLRULE,
    BATTLERULE_KACHINUKI,
    {
      24*8,6*8
    },
    {
      23*8,6*8,9*8,24
    }
  },
  //  CONFIG_ITEM_SND_STEREO,
  {
    CONFIG_LIST_SND,
    SOUNDMODE_STEREO,
    {
      15*8,9*8
    },
    {
      14*8,9*8,9*8,24
    }
  },
  //  CONFIG_ITEM_SND_MONO,
  {
    CONFIG_LIST_SND,
    SOUNDMODE_MONO,
    {
      24*8,9*8
    },
    {
      23*8,9*8,9*8,24
    }
  },
  //  CONFIG_ITEM_STR_HIRAGANA,
  {
    CONFIG_LIST_STR,
    MOJIMODE_HIRAGANA,
    {
      15*8,12*8
    },
    {
      14*8,12*8,9*8,24
    }
  },
  //  CONFIG_ITEM_STR_KANJI,
  {
    CONFIG_LIST_STR,
    MOJIMODE_KANJI,
    {
      24*8,12*8
    },
    {
      23*8,12*8,9*8,24
    }
  },
  //  CONFIG_ITEM_WIRELESS_YES,
  {
    CONFIG_LIST_WIRELESS,
    NETWORK_SEARCH_ON,
    {
      15*8,15*8
    },
    {
      14*8,15*8,9*8,24
    }
  },
  //  CONFIG_ITEM_WIRELESS_NO,
  {
    CONFIG_LIST_WIRELESS,
    NETWORK_SEARCH_OFF,
    {
      24*8,15*8
    },
    {
      23*8,15*8,9*8,24
    }
  },
  //  CONFIG_ITEM_REPORT_YES,
  {
    CONFIG_LIST_REPORT,
    WIRELESSSAVE_ON,
    {
      15*8,18*8
    },
    {
      14*8,18*8,9*8,24
    }
  },
  //  CONFIG_ITEM_REPORT_NO,
  {
    CONFIG_LIST_REPORT,
    WIRELESSSAVE_OFF,
    {
      24*8,18*8
    },
    {
      23*8,18*8,9*8,24
    }
  },
} ;

//-------------------------------------
/// APPBAR上の決定、やめるボタン
//=====================================
static const GFL_RECT sc_appbar_rect[APPBAR_WIN_MAX-1]  =
{
  {
    (APPBAR_WIN_X)*8, (APPBAR_MENUBAR_Y)*8,
    (APPBAR_WIN_X+APPBAR_WIN_W)*8, (APPBAR_MENUBAR_Y+APP_TASKMENU_PLATE_HEIGHT)*8
  },
  {
    (APPBAR_WIN_X+APPBAR_WIN_W)*8, (APPBAR_MENUBAR_Y)*8,
    (APPBAR_WIN_X+APPBAR_WIN_W*2)*8, (APPBAR_MENUBAR_Y+APP_TASKMENU_PLATE_HEIGHT)*8
  }
};


//=============================================================================
/**
 *          プロセス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  コンフィグプロセス初期化
 *
 *  @param  GFL_PROC *p_procプロセス
 *  @param  *p_seq          シーケンス
 *  @param  *p_param_adrs   引数ワーク
 *  @param  *p_wk_adrs      ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT CONFIG_PROC_Init( GFL_PROC *p_proc,int *p_seq, void *p_param_adrs, void *p_wk_adrs)
{
  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));

  //ヒープ作成
  GFL_HEAP_CreateHeap(GFL_HEAPID_APP,HEAPID_CONFIG,0x21000);

  //プロセスワーク作成
  {
    CONFIG_WORK *p_wk;
    p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(CONFIG_WORK), HEAPID_CONFIG);
    GFL_STD_MemClear( p_wk, sizeof(CONFIG_WORK) );
    p_wk->p_param = p_param_adrs;
    p_wk->p_param->is_exit  = FALSE;

    //最初にGRAPHIC初期化(パレット読み込みで、下記のものが上書きされるため)
    GRAPHIC_Init( &p_wk->graphic, HEAPID_CONFIG );

    //共通モジュール初期化
    p_wk->p_font    = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
                        GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_CONFIG );
    p_wk->p_que     = PRINTSYS_QUE_Create( HEAPID_CONFIG );
    p_wk->p_menures = APP_TASKMENU_RES_Create( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_M),
                        CONFIG_BG_PAL_M_13, p_wk->p_font, p_wk->p_que, HEAPID_CONFIG );
    p_wk->p_confirmres  = APP_TASKMENU_RES_Create( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_DECIDE_M),
                        CONFIG_BG_PAL_M_09, p_wk->p_font, p_wk->p_que, HEAPID_CONFIG );
    p_wk->p_msg   = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
                        NARC_message_config_dat, HEAPID_CONFIG );

    p_wk->p_tcbl    = GFL_TCBL_Init( HEAPID_CONFIG, HEAPID_CONFIG, 32, 32 );

    //モジュール初期化
    {
      CONFIG * p_sv = SaveData_GetConfig(SaveControl_GetPointer());
      CONFIGPARAM_Init( &p_wk->pre, p_sv );
    }
    SEQ_Init( &p_wk->seq, p_wk, SEQFUNC_FadeOut );  //最初はFadeOutシーケンス


    {
      GAMEDATA *p_gdata;
#ifdef GAMESYS_NONE_MOVE
      if( p_wk->p_param->p_gamesys == NULL )
      {
        OS_Printf( "\n!!!! GameSysPtr == NULL  !!!!\n" );
        p_gdata = NULL;
      }
      else
      {
        p_gdata = GAMESYSTEM_GetGameData(p_wk->p_param->p_gamesys);
      }
#else
      p_gdata = GAMESYSTEM_GetGameData(p_wk->p_param->p_gamesys);
#endif //GAMESYS_NONE_MOVE

      APPBAR_Init( &p_wk->appbar,
        GRAPHIC_GetUnit(&p_wk->graphic),
        p_wk->p_font,
        p_wk->p_que,
        p_wk->p_msg,
        p_wk->p_menures,
        p_gdata,
        HEAPID_CONFIG );
    }
    SCROLL_Init( &p_wk->scroll,
        GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_FONT_M),
        GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_WND_M),
        &p_wk->graphic,
        &p_wk->pre,
        HEAPID_CONFIG);
    UI_Init( &p_wk->ui, HEAPID_CONFIG );
    MSGWND_Init( &p_wk->info,
        GRAPHIC_GetBmpwin( &p_wk->graphic, BMPWINID_TEXT ),
        p_wk->p_font,
        p_wk->p_msg,
        p_wk->p_tcbl,
        p_wk->p_que,
        HEAPID_CONFIG );
    CONFIRM_Init( &p_wk->confirm,
        GRAPHIC_GetBmpwin( &p_wk->graphic, BMPWINID_CONFIRM ),
        p_wk->p_font,
        p_wk->p_msg,
        p_wk->p_tcbl,
        p_wk->p_confirmres,
        HEAPID_CONFIG );
  }

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  コンフィグプロセス破棄
 *
 *  @param  GFL_PROC *p_procプロセス
 *  @param  *p_seq          シーケンス
 *  @param  *p_param_adrs   引数ワーク
 *  @param  *p_wk_adrs      ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT CONFIG_PROC_Exit( GFL_PROC *p_proc,int *p_seq, void *p_param_adrs, void *p_wk_adrs)
{
  {
    CONFIG_WORK *p_wk = p_wk_adrs;

    //モジュール破棄
    CONFIRM_Exit( &p_wk->confirm );
    MSGWND_Exit( &p_wk->info );
    UI_Exit( &p_wk->ui );
    SCROLL_Exit( &p_wk->scroll );
    APPBAR_Exit( &p_wk->appbar );
    SEQ_Exit( &p_wk->seq );
    CONFIGPARAM_Exit( &p_wk->pre );

    //共通モジュール破棄
    GFL_TCBL_Exit( p_wk->p_tcbl );
    GFL_MSG_Delete( p_wk->p_msg );
    APP_TASKMENU_RES_Delete( p_wk->p_confirmres );
    APP_TASKMENU_RES_Delete( p_wk->p_menures );
    PRINTSYS_QUE_Delete( p_wk->p_que );
    GFL_FONT_Delete( p_wk->p_font );

    //GRAPHIC破棄
    GRAPHIC_Exit( &p_wk->graphic );
  }

  //ワークエリア解放
  GFL_PROC_FreeWork( p_proc );

  //ヒープ解放
  GFL_HEAP_DeleteHeap( HEAPID_CONFIG );

  //オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  コンフィグプロセスメイン処理
 *
 *  @param  GFL_PROC *p_procプロセス
 *  @param  *p_seq          シーケンス
 *  @param  *p_param_adrs   引数ワーク
 *  @param  *p_wk_adrs      ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT CONFIG_PROC_Main( GFL_PROC *p_proc,int *p_seq, void *p_param_adrs, void *p_wk_adrs)
{
  CONFIG_WORK *p_wk = p_wk_adrs;

  //シーケンス
  SEQ_Main( &p_wk->seq );

  //描画
  GRAPHIC_Main( &p_wk->graphic );

  //プリント
  PRINTSYS_QUE_Main( p_wk->p_que );

  //タスク
  GFL_TCBL_Main( p_wk->p_tcbl );

  //アプリバー
  APPBAR_MoveMain( &p_wk->appbar );

  //終了
  if( SEQ_IsEnd( &p_wk->seq ) )
  {
    return GFL_PROC_RES_FINISH;
  }
  else
  {
    return GFL_PROC_RES_CONTINUE;
  }
}

//=============================================================================
/**
 *            GRAPHIC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  グラフィック設定
 *
 *  @param  GRAPHIC_WORK *p_wk  ワーク
 *  @param  heapID              ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, HEAPID heapID )
{
  static const GFL_DISP_VRAM sc_vramSetTable =
  {
    GX_VRAM_BG_128_A,           // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_128_B,          // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_NONE,           // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_NONE,       // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_128K,
    GX_OBJVRAMMODE_CHAR_1D_128K,
  };

  //クリア
  GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );

  //レジスタ初期化
  G2_BlendNone();
  G2S_BlendNone();
  GX_SetVisibleWnd( GX_WNDMASK_NONE );
  GXS_SetVisibleWnd( GX_WNDMASK_NONE );

  //VRAMクリアー
  GFL_DISP_ClearVRAM( 0 );

  //VRAMバンク設定
  GFL_DISP_SetBank( &sc_vramSetTable );

  //ディスプレイON
  GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
  GFL_DISP_SetDispOn();

  //表示
  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();

  //フォント初期化
  GFL_FONTSYS_Init();
  GFL_FONTSYS_SetColor( 0xF, 0xE, 0 );

  //モジュール初期化
  GRAPHIC_BG_Init( &p_wk->bg, heapID );
  GRAPHIC_OBJ_Init( &p_wk->obj, &sc_vramSetTable, heapID );

  //VBlankTask登録
  p_wk->p_vblank_task = GFUser_VIntr_CreateTCB(Graphic_VBlankTask, p_wk, 0 );
}
//----------------------------------------------------------------------------
/**
 *  @brief  グラフィック破棄
 *
 *  @param  GRAPHIC_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk )
{
  //VBLANKTask消去
  GFL_TCB_DeleteTask( p_wk->p_vblank_task );

  //モジュール破棄
  GRAPHIC_OBJ_Exit( &p_wk->obj );
  GRAPHIC_BG_Exit( &p_wk->bg );

  //デフォルト色へ戻す
  GFL_FONTSYS_SetDefaultColor();

  //レジスタ初期化
  G2_BlendNone();
  G2S_BlendNone();
  GX_SetVisibleWnd( GX_WNDMASK_NONE );
  GXS_SetVisibleWnd( GX_WNDMASK_NONE );

  //クリア
  GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  グラフィック描画
 *
 *  @param  GRAPHIC_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Main( GRAPHIC_WORK *p_wk )
{
  GRAPHIC_OBJ_Main( &p_wk->obj );
  GRAPHIC_BG_Main( &p_wk->bg );
}
//----------------------------------------------------------------------------
/**
 *  @brief  CLWK取得
 *
 *  @param  const GRAPHIC_WORK *cp_wk ワーク
 *  @param  id                        CLWKID
 *
 *  @return CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK * GRAPHIC_GetClwk( const GRAPHIC_WORK *cp_wk, CLWKID id )
{
  return GRAPHIC_OBJ_GetClwk( &cp_wk->obj, id );
}

//----------------------------------------------------------------------------
/**
 *  @brief  CLUNIT取得
 *
 *  @param  const GRAPHIC_WORK *cp_wk   ワーク
 *
 *  @return GFL_CLUNIT
 */
//-----------------------------------------------------------------------------
static GFL_CLUNIT *GRAPHIC_GetUnit( const GRAPHIC_WORK *cp_wk )
{
  return GRAPHIC_OBJ_GetUnit( &cp_wk->obj );
}
//----------------------------------------------------------------------------
/**
 *  @brief  BMPWIN取得
 *
 *  @param  const GRAPHIC_WORK *cp_wk ワーク
 *  @param  id                        BMPWINのID
 *
 *  @return BMPWIN
 */
//-----------------------------------------------------------------------------
static GFL_BMPWIN * GRAPHIC_GetBmpwin( const GRAPHIC_WORK *cp_wk, BMPWINID id )
{
  return GRAPHIC_BG_GetBmpwin( &cp_wk->bg, id );
}
//----------------------------------------------------------------------------
/**
 *  @brief  BMPWINに文字描画
 *
 *  @param  GRAPHIC_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_PrintBmpwin( GRAPHIC_WORK *p_wk )
{
  GRAPHIC_BG_PrintBmpwin( &p_wk->bg );
}
//----------------------------------------------------------------------------
/**
 *  @brief  フェード開始
 *
 *  @param  GRAPHIC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_StartPalleteFade( GRAPHIC_WORK *p_wk )
{
  GRAPHIC_BG_StartPalletFade( &p_wk->bg );
}
//----------------------------------------------------------------------------
/**
 *  @brief  グラフィックVBLANK関数
 *
 *  @param  GRAPHIC_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work )
{
  GRAPHIC_WORK *p_wk  = p_work;
  GRAPHIC_BG_VBlankFunction( &p_wk->bg );
  GRAPHIC_OBJ_VBlankFunction( &p_wk->obj );
}

//=============================================================================
/**
 *            BG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  BG  初期化
 *
 *  @param  GRAPHIC_BG_WORK *p_wk ワーク
 *  @param  heapID                ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID )
{
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_BG_WORK) );

  //初期化
  GFL_BG_Init( heapID );
  GFL_BMPWIN_Init( heapID );

  //グラフィックモード設定
  {
    static const GFL_BG_SYS_HEADER sc_bgsys_header  =
    {
      GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_2D
    };
    GFL_BG_SetBGMode( &sc_bgsys_header );
  }

  //BG面設定
  {
    int i;
    for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
    {
      GFL_BG_SetBGControl( sc_bgsetup[i].frame, &sc_bgsetup[i].bgcnt_header, sc_bgsetup[i].mode );
      GFL_BG_ClearFrame( sc_bgsetup[i].frame );
      GFL_BG_SetVisible( sc_bgsetup[i].frame, VISIBLE_ON );
    }
  }
  GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_DECIDE_M), VISIBLE_OFF );

  GFL_BG_FillCharacter( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TEXT_S), 0, 1,  0 );
  GFL_BG_FillCharacter( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_DECIDE_M), 0, 1,  0 );


  //素材読み込み
  {
    ARCHANDLE * p_handle  = GFL_ARC_OpenDataHandle( ARCID_CONFIG_GRA, heapID );

    //メイン画面-----------------------------------
    //PLT
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_config_gra_est_bar_01_NCLR,
        PALTYPE_MAIN_BG, CONFIG_BG_PAL_M_00*0x20, 0, heapID );

    //titlebar--------------
    //CHR
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_config_gra_ue_bar_01_NCGR,
        GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_M), 0, 0, FALSE, heapID );

    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_config_gra_ue_bar_01_NSCR,
        GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_M), 0, 0, FALSE, heapID );

    //WND------------------
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_config_gra_est_bar_01_NCGR,
        GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_WND_M), 0, 0, FALSE, heapID );

    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_config_gra_est_bar_01_NSCR,
        GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_WND_M), 0, 0, FALSE, heapID );

    //CONFIRM-------------
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_config_gra_ue_flame_01_NCGR,
        GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_DECIDE_M), 1, 0, 0, heapID );

    GFL_ARCHDL_UTIL_TransVramPaletteEx( p_handle, NARC_config_gra_est_uegamen_01_NCLR,
        PALTYPE_MAIN_BG, 4*0x20, CONFIG_BG_PAL_M_15*0x20, 0x20, heapID );

    //TITLE
    p_wk->ncg_buf = GFL_ARCHDL_UTIL_LoadBGCharacter( p_handle ,
      NARC_config_gra_ue_bar_window_NCGR, FALSE, &p_wk->ncg_data, heapID );


    //サブ画面-----------------------------------
    //PLT
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_config_gra_est_uegamen_01_NCLR,
        PALTYPE_SUB_BG, CONFIG_BG_PAL_S_00*0x20, 0, heapID );

    //BACK---------
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_config_gra_est_uegamen_01_NCGR,
        GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BACK_S), 0, 0, FALSE, heapID );

    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_config_gra_est_uegamen_01_NSCR,
        GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BACK_S), 0, 0, FALSE, heapID );

    //TEXT-----------
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_config_gra_ue_flame_01_NCGR,
        GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TEXT_S), 1, 0, 0, heapID );


    GFL_ARC_CloseDataHandle( p_handle );
  }

  //パレット読みこみ保存
  {
    void *p_buff;
    u16 *p_raw_data;
    NNSG2dPaletteData* p_plt_data;

    p_buff  = GFL_ARC_UTIL_LoadPalette( ARCID_CONFIG_GRA, NARC_config_gra_est_bar_01_NCLR, &p_plt_data, heapID );
    p_raw_data  = p_plt_data->pRawData;
    GFL_STD_MemCopy( &p_raw_data[ 0x10 * CONFIG_BG_PAL_M_07] , p_wk->plt_color, sizeof(GXRgb)*0x10 );
    GFL_HEAP_FreeMemory( p_buff );
  }

  //BMPWIN作成
  {
    int i;
    for( i = 0; i < BMPWINID_CONFIG_MENU_MAX; i++ )
    {
      p_wk->p_bmpwin[i] = GFL_BMPWIN_Create( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_FONT_M),
          sc_config_menu_bmpwin_data[i].x, sc_config_menu_bmpwin_data[i].y,
          sc_config_menu_bmpwin_data[i].w, sc_config_menu_bmpwin_data[i].h,
          0, GFL_BMP_CHRAREA_GET_B );
    }
    p_wk->p_bmpwin[BMPWINID_TEXT] = GFL_BMPWIN_Create( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TEXT_S),
        INFOWND_X, INFOWND_Y, INFOWND_W, INFOWND_H,
        CONFIG_BG_PAL_S_04, GFL_BMP_CHRAREA_GET_B );

    p_wk->p_bmpwin[BMPWINID_CONFIRM]  = GFL_BMPWIN_Create( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_DECIDE_M),
        CONFIRMWND_X, CONFIRMWND_Y, CONFIRMWND_W, CONFIRMWND_H,
        CONFIG_BG_PAL_M_15, GFL_BMP_CHRAREA_GET_B );

    p_wk->p_bmpwin[BMPWINID_TITLE]  = GFL_BMPWIN_Create( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_M),
        TITLEWND_X, TITLEWND_Y, TITLEWND_W, TITLEWND_H,
        CONFIG_BG_PAL_M_08, GFL_BMP_CHRAREA_GET_B );

  }
  GRAPHIC_BG_PrintBmpwin( p_wk );

  GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TEXT_S), FALSE );
}
//----------------------------------------------------------------------------
/**
 *  @brief  BG  破棄
 *
 *  @param  GRAPHIC_BG_WORK *p_wk   ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk )
{

  //BMPWIN破棄
  {
    int i;
    for( i = 0; i < BMPWINID_MAX; i++ )
    {
      if( p_wk->p_bmpwin[i] )
      {
        GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
      }
    }
  }

  //リソース破棄
  {
    GFL_HEAP_FreeMemory( p_wk->ncg_buf );
    GFL_BG_FillCharacterRelease( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TEXT_S), 1, 0 );
    GFL_BG_FillCharacterRelease( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_DECIDE_M), 1, 0 );
  }

  //BG面破棄
  {
    int i;
    for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
    {
      GFL_BG_FreeBGControl( sc_bgsetup[i].frame );
    }
  }

  //終了
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();

  //クリア
  GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_BG_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  BG  メイン処理
 *
 *  @param  GRAPHIC_BG_WORK *p_wk   ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Main( GRAPHIC_BG_WORK *p_wk )
{
  //選択色のパレットフェード
  Graphic_Bg_PalletFadeMain( &p_wk->trans_color[0], &p_wk->pltfade_cnt[0],
      PLTFADE_SELECT_ADD, CONFIG_BG_PAL_M_07, 1, p_wk->plt_color[0xb], p_wk->plt_color[0xc] );
}
//----------------------------------------------------------------------------
/**
 *  @brief  BG  BMPWIN取得
 *
 *  @param  const GRAPHIC_BG_WORK *cp_wk  ワーク
 *  @param  id  BMPWINのID
 *
 *  @return BMPWIN
 */
//-----------------------------------------------------------------------------
static GFL_BMPWIN *GRAPHIC_BG_GetBmpwin( const GRAPHIC_BG_WORK *cp_wk, BMPWINID id )
{
  return cp_wk->p_bmpwin[ id ];
}
//----------------------------------------------------------------------------
/**
 *  @brief  BMPWINに文字を貼り付ける
 *
 *  @param  GRAPHIC_BG_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_PrintBmpwin( GRAPHIC_BG_WORK *p_wk )
{
  //BMPWIN作成し文字を貼り付ける
  {
    int i;
    GFL_FONT  *p_font;
    STRBUF    *p_strbuf;
    GFL_MSGDATA *p_msg;
    s32 x;

    p_font  = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
        GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_CONFIG );

    p_msg   = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
        NARC_message_config_dat, HEAPID_CONFIG );


    for( i = 0; i < BMPWINID_CONFIG_MENU_MAX; i++ )
    {
      x = 0;
      //離しの速さだけぎゅうぎゅうなので、右のらんから外れているので
      //左１キャラ多く作り※１、右に修正しています
      switch(i )
      {
      case CONFIG_ITEM_MSG_SLOW:
        x = 7;
        break;
      case CONFIG_ITEM_MSG_NORMAL:
        x = 4;
        break;
      case CONFIG_ITEM_MSG_FAST:
        x = 2;
        break;
      }

      p_strbuf  = GFL_MSG_CreateString( p_msg, sc_config_menu_bmpwin_data[i].strID );

      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0 );

      PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), x, 0, p_strbuf, p_font );

      GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_bmpwin[i] );

      GFL_STR_DeleteBuffer(p_strbuf);
    }

    //タイトル
    //BMPWINに絵のデータ転送
    {
      u32 srcAdr, dstAdr;
      srcAdr  = (u32)(p_wk->ncg_data->pRawData);
      dstAdr  = (u32)(GFL_BMP_GetCharacterAdrs(GFL_BMPWIN_GetBmp( p_wk->p_bmpwin[BMPWINID_TITLE] )));
      GFL_STD_MemCopy32( (void*)srcAdr, (void*)dstAdr, TITLEWND_W*TITLEWND_H*0x20 );
    }

    //文字表示
    p_strbuf  = GFL_MSG_CreateString( p_msg, mes_config_title );
    {
      //センター処理
      int x;
      x = GFL_BMPWIN_GetSizeX( p_wk->p_bmpwin[BMPWINID_TITLE] )*4;
      x -= PRINTSYS_GetStrWidth( p_strbuf, p_font, 0 )/2;
      PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[BMPWINID_TITLE]), x, 4, p_strbuf, p_font, PRINTSYS_LSB_Make(0xf,0xE,1) );
    }
    GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_bmpwin[BMPWINID_TITLE] );
    GFL_STR_DeleteBuffer(p_strbuf);


    GFL_MSG_Delete( p_msg );
    GFL_FONT_Delete( p_font );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  Palletフェード開始
 *
 *  @param  GRAPHIC_BG_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_StartPalletFade( GRAPHIC_BG_WORK *p_wk )
{
  p_wk->pltfade_cnt[0]  = 0;
}
//----------------------------------------------------------------------------
/**
 *  @brief  BG  VBlank関数
 *
 *  @param  GRAPHIC_BG_WORK *p_wk   ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk )
{
  GFL_BG_VBlankFunc();
}
//----------------------------------------------------------------------------
/**
 *  @brief  BG　パレットフェード
 *
 *  @param  u16 *p_buff 色保存バッファ（VBlank転送のため）
 *  @param  *p_cnt      カウンタバッファ
 *  @param  add         カウンタ加算値
 *  @param  plt_num     パレット縦番号
 *  @param  plt_col     パレット横番号
 *  @param  start       開始色
 *  @param  end         終了色
 */
//-----------------------------------------------------------------------------
static void Graphic_Bg_PalletFadeMain( u16 *p_buff, u16 *p_cnt, u16 add, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end )
{
  //パレットアニメ
  if( *p_cnt + add >= 0x10000 )
  {
    *p_cnt = *p_cnt+add-0x10000;
  }
  else
  {
    *p_cnt += add;
  }
  {
    //1～0に変換
    const fx16 cos = (FX_CosIdx(*p_cnt)+FX16_ONE)/2;
    const u8 start_r  = (start & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
    const u8 start_g  = (start & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
    const u8 start_b  = (start & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;
    const u8 end_r  = (end & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
    const u8 end_g  = (end & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
    const u8 end_b  = (end & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;

    const u8 r = start_r + (((end_r-start_r)*cos)>>FX16_SHIFT);
    const u8 g = start_g + (((end_g-start_g)*cos)>>FX16_SHIFT);
    const u8 b = start_b + (((end_b-start_b)*cos)>>FX16_SHIFT);

    *p_buff = GX_RGB(r, g, b);

    NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_MAIN ,
                                        plt_num * 32 + plt_col *2 ,
                                        p_buff, 2 );
  }

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

  //読み込み
  {
    ARCHANDLE *p_handle;

    p_handle  = GFL_ARC_OpenDataHandle( ARCID_CONFIG_GRA, heapID );

    p_wk->reg_id[OBJRESID_ITEM_PLT] = GFL_CLGRP_PLTT_Register( p_handle,
        NARC_config_gra_est_obj_01_NCLR, CLSYS_DRAW_MAIN, CONFIG_OBJ_PAL_M_00*0x20, heapID );

    p_wk->reg_id[OBJRESID_ITEM_CHR] = GFL_CLGRP_CGR_Register( p_handle,
        NARC_config_gra_est_obj_01_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

    p_wk->reg_id[OBJRESID_ITEM_CEL] = GFL_CLGRP_CELLANIM_Register( p_handle,
        NARC_config_gra_est_obj_01_NCER, NARC_config_gra_est_obj_01_NANR, heapID );

    p_wk->reg_id[OBJRESID_BAR_CHR]  = GFL_CLGRP_CGR_Register( p_handle,
        NARC_config_gra_shita_bar_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

    p_wk->reg_id[OBJRESID_BAR_CEL]  = GFL_CLGRP_CELLANIM_Register( p_handle,
        NARC_config_gra_shita_bar_NCER, NARC_config_gra_shita_bar_NANR, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }

  //CLWK作成
  {
    int i;
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
    cldata.bgpri    = 2;
    cldata.softpri  = 2;
    for( i = CLWKID_ITEM_TOP; i < CLWKID_ITEM_END; i++ )
    {
      cldata.pos_x  = sc_item_info[i-CLWKID_ITEM_TOP].obj_pos.x;
      cldata.pos_y  = sc_item_info[i-CLWKID_ITEM_TOP].obj_pos.y;
      p_wk->p_clwk[i] =   GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->reg_id[OBJRESID_ITEM_CHR],
          p_wk->reg_id[OBJRESID_ITEM_PLT],
          p_wk->reg_id[OBJRESID_ITEM_CEL],
          &cldata,
          CLSYS_DEFREND_MAIN,
          heapID );
      GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[i], FALSE );
    }

    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
    cldata.pos_x    = 128;
    cldata.pos_y    = 96;
    cldata.bgpri    = 2;
    cldata.softpri  = 1;
    p_wk->p_clwk[CLWKID_DOWNBAR]  =   GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->reg_id[OBJRESID_BAR_CHR],
          p_wk->reg_id[OBJRESID_ITEM_PLT],
          p_wk->reg_id[OBJRESID_BAR_CEL],
          &cldata,
          CLSYS_DEFREND_MAIN,
          heapID );
    GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk[CLWKID_DOWNBAR], CONFIG_OBJ_PAL_M_06, CLWK_PLTTOFFS_MODE_PLTT_TOP );
  }

  if( GFL_NET_IsInit() )
  { 
    GFL_NET_ReloadIcon();
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
  //CLWK破棄
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
    GFL_CLGRP_PLTT_Release( p_wk->reg_id[OBJRESID_ITEM_PLT] );
    GFL_CLGRP_CGR_Release( p_wk->reg_id[OBJRESID_ITEM_CHR] );
    GFL_CLGRP_CELLANIM_Release( p_wk->reg_id[OBJRESID_ITEM_CEL] );
    GFL_CLGRP_CGR_Release( p_wk->reg_id[OBJRESID_BAR_CHR] );
    GFL_CLGRP_CELLANIM_Release( p_wk->reg_id[OBJRESID_BAR_CEL] );
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
  if( p_wk->anm_idx++ > 8*12 )
  {
    p_wk->anm_idx = 0;
  }

  //OBJのアニメを同期させる
  {
    int i;
    for( i = CLWKID_ITEM_TOP; i < CLWKID_ITEM_END; i++ )
    {
      GFL_CLACT_WK_SetAnmIndex( p_wk->p_clwk[ i ], p_wk->anm_idx/12 );
    }
  }


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
 *  @brief  OBJ描画 CLUNITの取得
 *
 *  @param  const GRAPHIC_OBJ_WORK *cp_wk   ワーク
 *
 *  @return CLUNIT
 */
//-----------------------------------------------------------------------------
static GFL_CLUNIT *GRAPHIC_OBJ_GetUnit( const GRAPHIC_OBJ_WORK *cp_wk )
{
  return cp_wk->p_clunit;
}
//=============================================================================
/**
 *          UI
 *            UIは入力を抽象化して返す
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  ユーザー入力初期化
 *
 *  @param  UI_WORK *p_wk ワーク
 *  @param  heapID        ヒープID
 */
//-----------------------------------------------------------------------------
static void UI_Init( UI_WORK *p_wk, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(UI_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  ユーザー入力破棄
 *
 *  @param  UI_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UI_Exit( UI_WORK *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(UI_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  ユーザー入力メイン処理
 *
 *  @param  UI_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UI_Main( UI_WORK *p_wk )
{
  u32 x, y;

  //一端リセット
  p_wk->input = UI_INPUT_NONE;

  //キー入力
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
  {
    p_wk->input = UI_INPUT_TRG_UP;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
  {
    p_wk->input = UI_INPUT_TRG_DOWN;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
  {
    p_wk->input = UI_INPUT_TRG_LEFT;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
  {
    p_wk->input = UI_INPUT_TRG_RIGHT;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
  {
    p_wk->input = UI_INPUT_CONT_UP;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )
  {
    p_wk->input = UI_INPUT_CONT_DOWN;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_A )
  {
    p_wk->input = UI_INPUT_TRG_DECIDE;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_B )
  {
    p_wk->input = UI_INPUT_TRG_CANCEL;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
  {
    p_wk->input = UI_INPUT_TRG_Y;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
  {
    p_wk->input = UI_INPUT_TRG_X;
  }


  //タッチ入力
  if( GFL_UI_TP_GetPointTrg( &x, &y ) )
  {
    p_wk->input = UI_INPUT_TOUCH;
    p_wk->tp_pos.x  = x;
    p_wk->tp_pos.y  = y;
    p_wk->slide_start.x = x;
    p_wk->slide_start.y = y;
    p_wk->is_start_slide  = TRUE;
  }
  else if( GFL_UI_TP_GetPointCont( &x, &y ) && p_wk->is_start_slide )
  {
    if( MATH_IAbs( y - p_wk->tp_pos.y ) >= UI_SLIDE_DISTANCE_START )
    {
      p_wk->input = UI_INPUT_SLIDE;
      p_wk->slide.x   = x - p_wk->slide_start.x;
      p_wk->slide.y   = y - p_wk->slide_start.y;
      p_wk->slide_start.x = x;
      p_wk->slide_start.y = y;
    }
  }
  else
  {
    p_wk->is_start_slide  = FALSE;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  入力を取得
 *
 *  @param  const UI_WORK *cp_wk  ワーク
 *  @param  *p_data   種類別の受け取りデータ
 *    UI_INPUT_TOUCHならば TRG座標
 *    UI_INPUT_SLIDEならば 移動オフセット
 *
 *  @return 入力の種類
 */
//-----------------------------------------------------------------------------
static UI_INPUT UI_GetInput( const UI_WORK *cp_wk )
{
  return cp_wk->input;
}
//----------------------------------------------------------------------------
/**
 *  @brief  情報を取得
 *
 *  @param  const UI_WORK *cp_wk  ワーク
 *  @param  param                 情報の種類
 *  @param  *p_data               情報受け取り
 */
//-----------------------------------------------------------------------------
static void UI_GetParam( const UI_WORK *cp_wk, UI_INPUT_PARAM param, GFL_POINT *p_data )
{
  if( p_data )
  {
    switch( param )
    {
    case UI_INPUT_PARAM_TRGPOS:
      *p_data = cp_wk->tp_pos;
      break;

    case UI_INPUT_PARAM_SLIDEPOS:
      *p_data = cp_wk->slide;
      break;

    case UI_INPUT_PARAM_SLIDENOW:
      *p_data = cp_wk->slide_start;
      break;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  キー入力かチェック
 *
 *	@param	UI_INPUT input  入力情報
 *
 *	@return キー入力ならTRUE　さもなくばFALSE
 */
//-----------------------------------------------------------------------------
static BOOL UI_IsKey( UI_INPUT input )
{ 
  switch(input )
  { 
  case UI_INPUT_NONE:      //入力なし
    return FALSE;

  case UI_INPUT_SLIDE:     //タッチスライド
  case UI_INPUT_FLICK:     //タッチはじき
  case UI_INPUT_TOUCH:     //タッチトリガー
    return FALSE;

  case UI_INPUT_TRG_UP:    //キー上
  case UI_INPUT_TRG_DOWN:  //キー下
  case UI_INPUT_TRG_RIGHT: //キー右
  case UI_INPUT_TRG_LEFT:  //キー左
  case UI_INPUT_TRG_DECIDE://キー決定
  case UI_INPUT_TRG_CANCEL://キーキャンセル
  case UI_INPUT_CONT_UP:   //キー上
  case UI_INPUT_CONT_DOWN: //キー下
  case UI_INPUT_TRG_Y: //キーY
  case UI_INPUT_TRG_X: //キーX
    return TRUE;
  }

  return FALSE;
}
//=============================================================================
/**
 *          MSGWND
 */
//=============================================================================
//-------------------------------------
///	
//=====================================
typedef enum
{
  MSGWND_PRINT_TYPE_NONE,
  MSGWND_PRINT_TYPE_STREAM, //プリントストリーム
  MSGWND_PRINT_TYPE_FILL,   //一括プリント
} MSGWND_PRINT_TYPE;

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージウィンドウ  初期化
 *
 *  @param  MSGWND_WORK* p_wk ワーク
 *  @param  bgframe     BGフレーム
 *  @param  x           X
 *  @param  y           Y
 *  @param  w           幅
 *  @param  h           高さ
 *  @param  plt         パレット番号
 *  @param  heapID      ヒープID
 */
//-----------------------------------------------------------------------------
static void MSGWND_Init( MSGWND_WORK* p_wk, GFL_BMPWIN *p_bmpwin, GFL_FONT *p_font, GFL_MSGDATA *p_msg, GFL_TCBLSYS *p_tcbl, PRINT_QUE *p_que, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
  p_wk->clear_chr = 0x4;
  p_wk->p_bmpwin  = p_bmpwin;
  p_wk->heapID    = heapID;

  p_wk->p_font    = p_font;
  p_wk->p_msg     = p_msg;
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( 255, heapID );
  p_wk->p_tcbl    = p_tcbl;
  p_wk->p_stream  = NULL;
  p_wk->p_que     = p_que;
  p_wk->print_update  = MSGWND_PRINT_TYPE_NONE;

  BmpWinFrame_Write( p_wk->p_bmpwin, WINDOW_TRANS_OFF,1, GFL_BMPWIN_GetPalette(p_bmpwin) );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );

  GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );

  PRINT_UTIL_Setup( &p_wk->util, p_wk->p_bmpwin );
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージウィンドウ  破棄
 *
 *  @param  MSGWND_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MSGWND_Exit( MSGWND_WORK* p_wk )
{
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
    p_wk->p_stream  = NULL;
  }

  GFL_STR_DeleteBuffer( p_wk->p_strbuf );
  GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージウィンドウ  プリント開始
 *
 *  @param  MSGWND_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MSGWND_Print( MSGWND_WORK* p_wk, u32 strID, int wait )
{
  //一端消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );

  //文字列作成
  GFL_MSG_GetString( p_wk->p_msg, strID, p_wk->p_strbuf );

  //ストリーム破棄
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
    p_wk->p_stream  = NULL;
  }

  //文字描画が違う
  if( wait == 0 )
  { 
    PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strbuf, p_wk->p_font );

    p_wk->print_update  = MSGWND_PRINT_TYPE_FILL;
  }
  else
  { 

    //ストリーム開始
    p_wk->p_stream  = PRINTSYS_PrintStream( p_wk->p_bmpwin, 0, 0, p_wk->p_strbuf,
        p_wk->p_font, wait, p_wk->p_tcbl, 0, p_wk->heapID, p_wk->clear_chr );

    p_wk->print_update  = MSGWND_PRINT_TYPE_STREAM;
  }

}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージウィンドウストリーム終了待ち
 *
 *  @param  const MSGWND_WORK* p_wk  ワーク
 *
 *  @return TRUEでストリーム終了  FALSEえ処理中
 */
//-----------------------------------------------------------------------------
static BOOL MSGWND_IsEndMsg( MSGWND_WORK* p_wk )
{
  switch( p_wk->print_update )
  { 
  default:
  case MSGWND_PRINT_TYPE_NONE:
    return TRUE;

  case MSGWND_PRINT_TYPE_FILL:
    return PRINT_UTIL_Trans( &p_wk->util, p_wk->p_que );

  case MSGWND_PRINT_TYPE_STREAM:
    return PRINTSTREAM_STATE_DONE == PRINTSYS_PrintStreamGetState( p_wk->p_stream );
  }
}
//=============================================================================
/**
 *          APPBAR
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  APPBAR初期化
 *
 *  @param  APPBAR_WORK *p_wk ワーク
 *  @param  *p_clunit ユニット
 *  @param  *p_font   フォント
 *  @param  p_que     プリントキュー
 *  @param  p_msg     メッセージ
 *  @param  p_res     タスクメニュー用素材
 *  @param  p_gdata   Yボタン登録のためにゲームデータ保持
 *  @param  heapID    ヒープ
 *
 */
//-----------------------------------------------------------------------------
static void APPBAR_Init( APPBAR_WORK *p_wk, GFL_CLUNIT *p_clunit, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, APP_TASKMENU_RES *p_res, GAMEDATA *p_gdata, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(APPBAR_WORK) );
  p_wk->select  = APPBAR_WIN_NULL;
  p_wk->p_font  = p_font;
  p_wk->p_que   = p_que;
  p_wk->p_res   = p_res;
  p_wk->p_msg   = p_msg;
  p_wk->p_gdata = p_gdata;

  {
    TOUCHBAR_ITEM_ICON  item[]  =
    {
      {
        TOUCHBAR_ICON_CLOSE,
        {
          TOUCHBAR_ICON_X_02,
          TOUCHBAR_ICON_Y
        }
      },
      {
        TOUCHBAR_ICON_CHECK,
        {
          TOUCHBAR_ICON_X_01,
          TOUCHBAR_ICON_Y_CHECK
        }
      }
    };
    TOUCHBAR_SETUP setup;

    GFL_STD_MemClear( &setup, sizeof(TOUCHBAR_SETUP) );
    setup.p_item    = item;
    setup.item_num  = NELEMS(item);
    setup.p_unit    = p_clunit;
    setup.obj_plt   = CONFIG_OBJ_PAL_M_08;
    setup.mapping   = APP_COMMON_MAPPING_128K;
    setup.is_notload_bg = TRUE;

    p_wk->p_touch = TOUCHBAR_Init( &setup, heapID );
  }

  APPBAR_ReWrite( p_wk, heapID );

  //Yボタン登録されていれば、アイコンをON

#ifdef GAMESYS_NONE_MOVE
  if( p_wk->p_gdata == NULL )
  {
    OS_Printf( "\n!!!! GameSysPtr == NULL  !!!!\n" );
    return ;
  }
#endif //GAMESYS_NONE_MOVE

  if( GAMEDATA_GetShortCut( p_wk->p_gdata, SHORTCUT_ID_CONFIG ))
  {
    TOUCHBAR_SetFlip( p_wk->p_touch, TOUCHBAR_ICON_CHECK, TRUE );
  }
  else
  {
    TOUCHBAR_SetFlip( p_wk->p_touch, TOUCHBAR_ICON_CHECK, FALSE );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  APPBAR破棄
 *
 *  @param  APPBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void APPBAR_Exit( APPBAR_WORK *p_wk )
{

  TOUCHBAR_Exit( p_wk->p_touch );

  {
    int i;
    for( i = 0; i < APPBAR_WIN_MAX-1; i++ )
    {
      GFL_STR_DeleteBuffer( p_wk->item[i].str );
      APP_TASKMENU_WIN_Delete( p_wk->p_win[i] );
    }
  }

  GFL_STD_MemClear( p_wk, sizeof(APPBAR_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  APPBARメイン処理
 *
 *  @param  APPBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void APPBAR_Main( APPBAR_WORK *p_wk, const UI_WORK *cp_ui, const SCROLL_WORK *cp_scroll )
{
  GFL_POINT   pos;
  UI_INPUT    input;
  CONFIG_LIST select;
  BOOL is_update  = FALSE;


  //決定時は入力できない
  if( p_wk->is_decide )
  { 
    input = UI_INPUT_NONE;
  }
  else
  { 
    //入力取得
    input = UI_GetInput( cp_ui );
  }

  //スクロールの選択取得
  select  = SCROLL_GetSelect( cp_scroll );

  switch( input )
  {
  case UI_INPUT_TOUCH:
    UI_GetParam( cp_ui, UI_INPUT_PARAM_TRGPOS, &pos );
    GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
    //タッチしたならば、
    //入っていれば、選択
    if( COLLISION_IsRectXPos( &sc_appbar_rect[APPBAR_WIN_DECIDE], &pos )
        && p_wk->select != APPBAR_WIN_DECIDE )
    {
      PMSND_PlaySE( CONFIG_SE_DECIDE );
      p_wk->select = APPBAR_WIN_DECIDE;
      p_wk->is_decide = TRUE;
    }
    else if(  COLLISION_IsRectXPos( &sc_appbar_rect[APPBAR_WIN_CANCEL], &pos )
        && p_wk->select != APPBAR_WIN_CANCEL )
    {
      PMSND_PlaySE( CONFIG_SE_DECIDE );
      p_wk->select  = APPBAR_WIN_CANCEL;
      p_wk->is_decide = TRUE;
    }
    else if( p_wk->select != APPBAR_WIN_NULL )
    {
      p_wk->select  = APPBAR_WIN_NULL;
    }

    is_update = TRUE;
    break;

  case UI_INPUT_TRG_UP:
    /* fallthrough */
  case UI_INPUT_CONT_UP:
    /* fallthrough */
  case UI_INPUT_TRG_LEFT:
    /* fallthrough */
  case UI_INPUT_TRG_DOWN:
    /* fallthrough */
  case UI_INPUT_CONT_DOWN:
    /* fallthrough */
  case UI_INPUT_TRG_RIGHT:
    switch( select )
    {
    case CONFIG_LIST_DECIDE:
      p_wk->select  = APPBAR_WIN_DECIDE;
      break;
    case CONFIG_LIST_CANCEL:
      p_wk->select  = APPBAR_WIN_CANCEL;
      break;
    default:
      p_wk->select  = APPBAR_WIN_NULL;
    }
    is_update = TRUE;
    break;

  case UI_INPUT_TRG_DECIDE:
    if( p_wk->select != APPBAR_WIN_NULL )
    {
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      PMSND_PlaySE( CONFIG_SE_DECIDE );
      p_wk->is_decide = TRUE;
      is_update = TRUE;
    }
    break;

  case UI_INPUT_TRG_CANCEL:
    GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
    PMSND_PlaySE( CONFIG_SE_CANCEL );
    p_wk->select    = APPBAR_WIN_CANCEL;
    p_wk->is_decide = TRUE;
    is_update = TRUE;
    break;
  }

  //選択時
  if( is_update )
  {
    if( p_wk->select == APPBAR_WIN_NULL )
    {
      APP_TASKMENU_WIN_SetActive( p_wk->p_win[APPBAR_WIN_DECIDE], FALSE );
      APP_TASKMENU_WIN_SetActive( p_wk->p_win[APPBAR_WIN_CANCEL], FALSE );
    }
    else
    {
      if( p_wk->is_decide )
      {
        APP_TASKMENU_WIN_SetActive( p_wk->p_win[p_wk->select], FALSE );
        APP_TASKMENU_WIN_SetDecide( p_wk->p_win[p_wk->select], TRUE );
        APP_TASKMENU_WIN_SetActive( p_wk->p_win[p_wk->select^1], FALSE );
      }
      else
      {
        APP_TASKMENU_WIN_SetActive( p_wk->p_win[p_wk->select], TRUE );
        APP_TASKMENU_WIN_SetActive( p_wk->p_win[p_wk->select^1], FALSE );
      }
    }
  }

  //タッチバーメイン
  TOUCHBAR_Main( p_wk->p_touch );

  switch( TOUCHBAR_GetTrg( p_wk->p_touch ) )
  {
  case TOUCHBAR_ICON_CHECK :

#ifdef GAMESYS_NONE_MOVE
    if( p_wk->p_gdata == NULL )
    {
      OS_Printf( "\n!!!! GameSysPtr == NULL  !!!!\n" );
      break;
    }
#endif //GAMESYS_NONE_MOVE

    if( TOUCHBAR_GetFlip( p_wk->p_touch, TOUCHBAR_ICON_CHECK ) )
    {
      GAMEDATA_SetShortCut( p_wk->p_gdata, SHORTCUT_ID_CONFIG, TRUE );
    }
    else
    {
      GAMEDATA_SetShortCut( p_wk->p_gdata, SHORTCUT_ID_CONFIG, FALSE );
    }
    p_wk->select = APPBAR_WIN_NULL;
    break;

  case TOUCHBAR_ICON_CLOSE:
    p_wk->select  = APPBAR_WIN_EXIT;
    p_wk->is_decide = TRUE;
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  動作メイン
 *
 *	@param	APPBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void APPBAR_MoveMain( APPBAR_WORK *p_wk )
{ 
  //モジュールメイン実行
  {
    int i;
    for( i = 0; i < APPBAR_WIN_MAX-1; i++ )
    {
      APP_TASKMENU_WIN_Update( p_wk->p_win[i] );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  APPBARの選択決定取得
 *
 *  @param  const APPBAR_WORK *cp_wk  ワーク
 *  @param  select                    何を選んだか
 *
 *  @return TRUEで決定  FALSEでまだ
 */
//-----------------------------------------------------------------------------
static BOOL APPBAR_IsDecide( const APPBAR_WORK *cp_wk, APPBAR_WIN_LIST *p_select )
{
  if( p_select )
  {
    *p_select = cp_wk->select;
  }
  return cp_wk->is_decide;
}

//----------------------------------------------------------------------------
/**
 *	@brief  演出待ち
 *
 *	@param	const APPBAR_WORK *cp_wkt   ワーク
 *
 *	@return TRUEで演出終了  FALSEで演出中
 */
//-----------------------------------------------------------------------------
static BOOL APPBAR_IsWaitEffect( const APPBAR_WORK *cp_wk )
{ 
  BOOL ret  = TRUE;
  if( cp_wk->is_decide && cp_wk->select != APPBAR_WIN_NULL )
  { 
    ret = APP_TASKMENU_WIN_IsFinish( cp_wk->p_win[cp_wk->select] );
  }
  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出を消す
 *
 *	@param	APPBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void APPBAR_StopEffect( APPBAR_WORK *p_wk )
{ 
  {
    int i;
    for( i = 0; i < APPBAR_WIN_MAX-1; i++ )
    {
      APP_TASKMENU_WIN_SetDecide( p_wk->p_win[i], FALSE );
      APP_TASKMENU_WIN_SetActive( p_wk->p_win[i], FALSE );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  再描画
 *
 *  @param  APPBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void APPBAR_ReWrite( APPBAR_WORK *p_wk, HEAPID heapID )
{
  //決定、やめるボタン作成
  {
    int i;

    for( i = 0; i < APPBAR_WIN_MAX-1; i++ )
    {
      if( p_wk->item[i].str != NULL )
      {
        GFL_STR_DeleteBuffer( p_wk->item[i].str );
      }
      p_wk->item[i].str       = GFL_MSG_CreateString( p_wk->p_msg, mes_config_menu08 + i );
      p_wk->item[i].msgColor  = APP_TASKMENU_ITEM_MSGCOLOR;
      p_wk->item[i].type      = APP_TASKMENU_WIN_TYPE_NORMAL+i;
      if( p_wk->p_win[i] )
      {
          APP_TASKMENU_WIN_Delete( p_wk->p_win[i] );
      }
      p_wk->p_win[i] = APP_TASKMENU_WIN_Create( p_wk->p_res, &p_wk->item[i],
                            APPBAR_WIN_X + APPBAR_WIN_W*i, APPBAR_MENUBAR_Y, APPBAR_WIN_W, heapID );
    }
  }
}
//=============================================================================
/**
 *          SCROLL
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  スクロール管理初期化
 *
 *  @param  SCROLL_WORK *p_wk ワーク
 *  @param  font_frm          文字BG
 *  @param  back_frm          背景BG
 *  @param  heapID            ヒープID
 */
//-----------------------------------------------------------------------------
static void SCROLL_Init( SCROLL_WORK *p_wk, u8 font_frm, u8 back_frm, const GRAPHIC_WORK *cp_grp, const CONFIG_PARAM *cp_param, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk,sizeof(SCROLL_WORK) );
  p_wk->font_frm  = font_frm;
  p_wk->back_frm  = back_frm;
  p_wk->obj_y_ofs = SCROLL_WINDOW_OFS_DOT;
  p_wk->now       = *cp_param;

  {
    int i;
    for( i = 0; i < CONFIG_ITEM_MAX; i++ )
    {
      p_wk->p_item[i] = GRAPHIC_GetClwk( cp_grp, CLWKID_ITEM_TOP + i );
    }
  }

  //開始オフセットまでずらす
  Scroll_Move( p_wk, SCROLL_START_OFS_Y );

  //キーモードがキーならばカーソル位置を押す
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_TOUCH )
  {
    p_wk->select    = CONFIG_LIST_INIT;
    Scroll_ChangePlt( p_wk, FALSE );
    p_wk->is_info_update = FALSE;
  }
  else
  {
    p_wk->select    = CONFIG_LIST_MSGSPEED;
    Scroll_ChangePlt( p_wk, TRUE );
    p_wk->is_info_update = TRUE;
  }

}
//----------------------------------------------------------------------------
/**
 *  @brief  スクロール管理破棄
 *
 *  @param  SCROLL_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SCROLL_Exit( SCROLL_WORK *p_wk )
{
  GFL_STD_MemClear( p_wk,sizeof(SCROLL_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  スクロール管理メイン処理
 *
 *  @param  SCROLL_WORK *p_wk ワーク
 *  @param  UI_WORK *cp_ui    UIワーク
 *  @param  MSGWND_WORK       MSGワーク
 *
 */
//-----------------------------------------------------------------------------
static void SCROLL_Main( SCROLL_WORK *p_wk, const UI_WORK *cp_ui, MSGWND_WORK *p_msg, GRAPHIC_WORK *p_graphic, APPBAR_WORK *p_appbar )
{
  UI_INPUT input;
  GFL_POINT trg_pos;
  GFL_POINT slide_pos;
  GFL_POINT slide_now;
  int y, bar_top;
  BOOL is_bmpprint_decide;
  BOOL  is_decide = TRUE;

  //入力取得
  input = UI_GetInput( cp_ui );

  //先頭バー計算
  y = GFL_BG_GetScrollY( p_wk->back_frm );
  bar_top = (y-SCROLL_START)/SCROLL_WINDOW_H_DOT;

  //入力がキーかタッチかチェック
#ifdef CONFIG_KEY_TOUCH
  if( input != UI_INPUT_NONE )
  { 
    if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_TOUCH )
    {
      if( UI_IsKey( input ) )
      { 
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
        Scroll_ChangePlt( p_wk, TRUE );
        if( CONFIG_LIST_INIT < p_wk->select && p_wk->select < CONFIG_LIST_MAX )
        { 
          int speed;
          if( p_wk->select == CONFIG_LIST_MSGSPEED )
          { 
            speed = CONFIGPARAM_GetMsgSpeed(&p_wk->now);
          }
          else
          { 
            speed = 0;
          }
          GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TEXT_S), TRUE );
          MSGWND_Print( p_msg, sc_list_info[ p_wk->select ].infoID, speed );
        }
        return;
      }
      else
      { 
        is_decide = FALSE;
      }
    }
    else
    { 
      if( !UI_IsKey( input ) )
      { 
        GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
        is_decide = FALSE;
      }
      else
      { 
        is_decide = TRUE;
      }
   }
  }
#endif

  //入力による操作
  switch( input )
  {
  case UI_INPUT_SLIDE:
    UI_GetParam( cp_ui, UI_INPUT_PARAM_SLIDEPOS, &slide_pos );
    UI_GetParam( cp_ui, UI_INPUT_PARAM_SLIDENOW, &slide_now );
    UI_GetParam( cp_ui, UI_INPUT_PARAM_TRGPOS, &trg_pos );
    if( SCROLL_TOP_BAR_Y < trg_pos.y && trg_pos.y < SCROLL_APP_BAR_Y
       && SCROLL_TOP_BAR_Y < slide_now.y && slide_now.y < SCROLL_APP_BAR_Y )
    {
      Scroll_Move( p_wk, -slide_pos.y );
      Scroll_ChangePlt_Safe_Check( p_wk, -slide_pos.y );
      //上下に移動させすぎるとちかちかしてしまうので、チェック
      if( Scroll_ChangePlt_Safe_IsOk(p_wk) )
      {
        Scroll_ChangePlt( p_wk, is_decide );
        GRAPHIC_StartPalleteFade( p_graphic );
      }
    }
    break;

  case UI_INPUT_CONT_UP:
    //一定間隔ごとに移動するため、一定間隔に達していなければbreak
    //達していれば、下のcaseの処理を実行
    if( p_wk->cont_sync++ < SCROLL_CONT_SYNC_MAX )
    {
      break;
    }
    /* fallthrough */

  case UI_INPUT_TRG_UP:
    p_wk->cont_sync = 0;
    p_wk->select--;
    p_wk->select  = MATH_CLAMP( p_wk->select, CONFIG_LIST_MSGSPEED, CONFIG_LIST_CANCEL );
    //スクロールする距離になったら動く
    if( p_wk->select < bar_top )
    {
      Scroll_Move( p_wk, -SCROLL_WINDOW_H_DOT );
    }
    if( p_wk->select < CONFIG_LIST_MAX )
    {
      p_wk->is_info_update  = TRUE;
    }
    PMSND_PlaySE( CONFIG_SE_MOVE );
    Scroll_ChangePlt( p_wk, is_decide );
    GRAPHIC_StartPalleteFade( p_graphic );
    break;

  case UI_INPUT_CONT_DOWN:
    //一定間隔ごとに移動するため、一定間隔に達していなければbreak
    //達していれば、下のcaseの処理を実行
    if( p_wk->cont_sync++ < SCROLL_CONT_SYNC_MAX )
    {
      break;
    }
    /* fallthrough */

  case UI_INPUT_TRG_DOWN:
    p_wk->cont_sync = 0;
    p_wk->select++;
    p_wk->select  = MATH_CLAMP( p_wk->select, CONFIG_LIST_MSGSPEED, CONFIG_LIST_CANCEL );
    //スクロールする距離になったら動く
    if( p_wk->select > bar_top+SCROLL_DISPLAY )
    {
      Scroll_Move( p_wk, SCROLL_WINDOW_H_DOT );
    }
    if( p_wk->select < CONFIG_LIST_MAX )
    {
      p_wk->is_info_update  = TRUE;
    }
    PMSND_PlaySE( CONFIG_SE_MOVE );
    Scroll_ChangePlt( p_wk, is_decide );
    GRAPHIC_StartPalleteFade( p_graphic );
    break;

  case UI_INPUT_TOUCH:
    UI_GetParam( cp_ui, UI_INPUT_PARAM_TRGPOS, &trg_pos );
    if( SCROLL_TOP_BAR_Y < trg_pos.y && trg_pos.y < SCROLL_APP_BAR_Y )
    {
      //座標から選択への変換
      p_wk->select  = Scroll_PosToList( p_wk, &trg_pos );
      //項目のタッチ
      Scroll_TouchItem( p_wk, &trg_pos );
      Scroll_ChangePlt( p_wk, is_decide );
      GRAPHIC_StartPalleteFade( p_graphic );
      p_wk->is_info_update  = TRUE;
      is_bmpprint_decide  = is_decide;
    }
    break;

  case UI_INPUT_TRG_RIGHT:
    if( p_wk->select < CONFIG_LIST_MAX )
    {
      PMSND_PlaySE( CONFIG_SE_MOVE );
      Scroll_SelectItem( p_wk, 1 );
      Scroll_ChangePlt( p_wk, is_decide );
      GRAPHIC_StartPalleteFade( p_graphic );
      is_bmpprint_decide  = is_decide;
    }
    else
    {
      p_wk->select++;
      PMSND_PlaySE( CONFIG_SE_MOVE );
      p_wk->select  = MATH_CLAMP( p_wk->select, CONFIG_LIST_MSGSPEED, CONFIG_LIST_CANCEL );
    }
    break;

  case UI_INPUT_TRG_LEFT:
    if( p_wk->select < CONFIG_LIST_MAX )
    {
      PMSND_PlaySE( CONFIG_SE_MOVE );
      Scroll_SelectItem( p_wk, -1 );
      Scroll_ChangePlt( p_wk, is_decide );
      GRAPHIC_StartPalleteFade( p_graphic );
      is_bmpprint_decide  = is_decide;
    }
    else if( p_wk->select == CONFIG_LIST_CANCEL )
    {
      p_wk->select--;
      PMSND_PlaySE( CONFIG_SE_MOVE );
      p_wk->select  = MATH_CLAMP( p_wk->select, CONFIG_LIST_MSGSPEED, CONFIG_LIST_CANCEL );
    }
    break;

  case UI_INPUT_TRG_CANCEL:
    Scroll_ChangePlt( p_wk, FALSE );
    break;
  }

  //項目に変更があった場合の更新
  {
    CONFIG_ITEM item  = CONFIGPARAM_GetChangeTiming( &p_wk->now );
    switch( item )
    {
    case CONFIG_ITEM_STR_HIRAGANA:
    case CONFIG_ITEM_STR_KANJI:
      //漢字変更のためPRINTしなおし
      GRAPHIC_PrintBmpwin( p_graphic );
      APPBAR_ReWrite( p_appbar, HEAPID_CONFIG );
      Scroll_ChangePlt( p_wk, is_bmpprint_decide );
      p_wk->is_info_update  = TRUE;
      break;
    case CONFIG_ITEM_MSG_SLOW:
    case CONFIG_ITEM_MSG_NORMAL:
    case CONFIG_ITEM_MSG_FAST:
      //メッセージ速度を変更したときは分かりやすいように
      //上画面更新
      p_wk->is_info_update  = TRUE;
      break;
    }
  }

  //上画面メッセージ更新
  if( p_wk->is_info_update )
  {
    if( CONFIG_LIST_INIT < p_wk->select && p_wk->select < CONFIG_LIST_MAX )
    {
      int speed;
      if( p_wk->select == CONFIG_LIST_MSGSPEED )
      { 
        speed = CONFIGPARAM_GetMsgSpeed(&p_wk->now);
      }
      else
      { 
        speed = 0;
      }
      GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TEXT_S), TRUE );
      MSGWND_Print( p_msg, sc_list_info[ p_wk->select ].infoID, speed );
      p_wk->is_info_update  = FALSE;
    }
  }


  MSGWND_IsEndMsg( p_msg );

  //パレット切り替えによるちかちか防止メイン
  Scroll_ChangePlt_Safe_Main( p_wk );
}
//----------------------------------------------------------------------------
/**
 *  @brief  現在何を選択しているか取得
 *
 *  @param  const SCROLL_WORK *cp_wk  ワーク
 *
 *  @return 現在選択しているもの
 */
//-----------------------------------------------------------------------------
static CONFIG_LIST SCROLL_GetSelect( const SCROLL_WORK *cp_wk )
{
  return cp_wk->select;
}

//----------------------------------------------------------------------------
/**
 *  @brief  コンフィグパラメータ受け取り
 *
 *  @param  const SCROLL_WORK *cp_wk  ワーク
 *  @param  *p_param                  受け取り
 */
//-----------------------------------------------------------------------------
static void SCROLL_GetConfigParam( const SCROLL_WORK *cp_wk, CONFIG_PARAM *p_param )
{
  *p_param  = cp_wk->now;
}
//----------------------------------------------------------------------------
/**
 *  @brief  パレットで塗る
 *
 *  @param  SCROLL_WORK *p_wk ワーク
 *  @param  is_decide_draw    決定色を塗るかどうか
 */
//-----------------------------------------------------------------------------
static void Scroll_ChangePlt( SCROLL_WORK *p_wk, BOOL is_decide_draw )
{
  enum
  {
    SCROLL_SELECT_PLT = CONFIG_BG_PAL_M_07,
  };

  static const u8 sc_bright_plt[] =
  {
    CONFIG_BG_PAL_M_03,CONFIG_BG_PAL_M_01,CONFIG_BG_PAL_M_00,CONFIG_BG_PAL_M_01, CONFIG_BG_PAL_M_03,
  };

  int i;
  int bar_top;
  int start;
  int y;

  //決定時でリストないのとき
  if( p_wk->select > CONFIG_LIST_MAX )
  {
    is_decide_draw  = FALSE;
  }

  //座標取得
  y = GFL_BG_GetScrollY( p_wk->back_frm );

  //先頭バー
  bar_top = (y-SCROLL_START)/SCROLL_WINDOW_H_DOT;

  //一端全部ぬる
  GFL_BG_ChangeScreenPalette( p_wk->back_frm, 0, 0,
      32, 32, CONFIG_BG_PAL_M_05 );

  //通常のリストを塗る
  for( i = 0; i < NELEMS(sc_bright_plt); i++ )
  {
    //バーをぬる
    start = SCROLL_WINDOW_OFS_CHR +  ( bar_top +i ) * SCROLL_WINDOW_H_CHR;
    GFL_BG_ChangeScreenPalette( p_wk->back_frm, 0, start,
        32, SCROLL_WINDOW_H_CHR, sc_bright_plt[i] );
  }

  //フォントを塗る
  //リスト項目の文字
  for( i = BMPWINID_ITEM_TOP; i < BMPWINID_ITEM_MAX; i++ )
  {
    //選択中であれば
    if( sc_item_info[i].list == p_wk->select && is_decide_draw )
    {
      //ONかOFFか
      if( CONFIGPARAM_IsItemSetting( &p_wk->now, i ) )
      {
        GFL_BG_ChangeScreenPalette( p_wk->font_frm,
            sc_config_menu_bmpwin_data[i].x,
            sc_config_menu_bmpwin_data[i].y,
            sc_config_menu_bmpwin_data[i].w,
            sc_config_menu_bmpwin_data[i].h,
            CONFIG_BG_PAL_M_07
            );
      }
      else
      {
        GFL_BG_ChangeScreenPalette( p_wk->font_frm,
            sc_config_menu_bmpwin_data[i].x,
            sc_config_menu_bmpwin_data[i].y,
            sc_config_menu_bmpwin_data[i].w,
            sc_config_menu_bmpwin_data[i].h,
            CONFIG_BG_PAL_M_12
            );
      }
    }
    else
    {
      //ONかOFFか
      if( CONFIGPARAM_IsItemSetting( &p_wk->now, i ) )
      {
        GFL_BG_ChangeScreenPalette( p_wk->font_frm,
            sc_config_menu_bmpwin_data[i].x,
            sc_config_menu_bmpwin_data[i].y,
            sc_config_menu_bmpwin_data[i].w,
            sc_config_menu_bmpwin_data[i].h,
            CONFIG_BG_PAL_M_00
            );
      }
      else
      {
        GFL_BG_ChangeScreenPalette( p_wk->font_frm,
            sc_config_menu_bmpwin_data[i].x,
            sc_config_menu_bmpwin_data[i].y,
            sc_config_menu_bmpwin_data[i].w,
            sc_config_menu_bmpwin_data[i].h,
            CONFIG_BG_PAL_M_03
            );
      }
    }
  }
  //リストタイトルの文字
  for( i = BMPWINID_LIST_TOP; i < BMPWINID_LIST_MAX; i++ )
  {
  //選択中であれば
    if( ((i - BMPWINID_LIST_TOP)== p_wk->select) && is_decide_draw )
    {
      GFL_BG_ChangeScreenPalette( p_wk->font_frm,
          sc_config_menu_bmpwin_data[i].x,
          sc_config_menu_bmpwin_data[i].y,
          sc_config_menu_bmpwin_data[i].w,
          sc_config_menu_bmpwin_data[i].h,
          SCROLL_SELECT_PLT
          );
    }
    else
    {
      GFL_BG_ChangeScreenPalette( p_wk->font_frm,
          sc_config_menu_bmpwin_data[i].x,
          sc_config_menu_bmpwin_data[i].y,
          sc_config_menu_bmpwin_data[i].w,
          sc_config_menu_bmpwin_data[i].h,
          CONFIG_BG_PAL_M_00
          );
    }
  }


  //決定のリストを塗る
  if( is_decide_draw )
  {
    start = SCROLL_WINDOW_OFS_CHR + ( p_wk->select ) * SCROLL_WINDOW_H_CHR;
    GFL_BG_ChangeScreenPalette( p_wk->back_frm, 0, start,
          32, SCROLL_WINDOW_H_CHR, SCROLL_SELECT_PLT );
  }

  //転送
  GFL_BG_LoadScreenV_Req( p_wk->back_frm );
  GFL_BG_LoadScreenV_Req( p_wk->font_frm );

  //OBJのパレット変更
  {

    enum
    {
      SCROLL_SELECT_PLT = CONFIG_OBJ_PAL_M_07,
    };

    static const u8 sc_bright_obj_plt[] =
    {
      CONFIG_OBJ_PAL_M_03,CONFIG_OBJ_PAL_M_01,CONFIG_OBJ_PAL_M_00,CONFIG_OBJ_PAL_M_01,CONFIG_OBJ_PAL_M_03,
    };

    int i, j;

    for( j = bar_top; j < NELEMS(sc_bright_obj_plt) + bar_top; j++ )
    {
      for( i = 0; i < CONFIG_ITEM_MAX; i++ )
      {
        //選択中であれば
        if( sc_item_info[i].list == p_wk->select && is_decide_draw )
        {
          //ONかOFFか
          if( CONFIGPARAM_IsItemSetting( &p_wk->now, i ) )
          {
            GFL_CLACT_WK_SetAnmSeqDiff( p_wk->p_item[i], 3 );
          }
          else
          {
            GFL_CLACT_WK_SetAnmSeqDiff( p_wk->p_item[i], 2 );
          }
        }
        else if( sc_item_info[i].list == j )
        {
          //ONかOFFか
          if( CONFIGPARAM_IsItemSetting( &p_wk->now, i ) )
          {
            GFL_CLACT_WK_SetAnmSeqDiff( p_wk->p_item[i], 0 );
          }
          else
          {
            GFL_CLACT_WK_SetAnmSeqDiff( p_wk->p_item[i], 1 );
          }
        }
      }
    }

  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  スクロール移動
 *
 *  @param  SCROLL_WORK *p_wk   ワーク
 *  @param  y_add               Y移動値
 */
//-----------------------------------------------------------------------------
static void Scroll_Move( SCROLL_WORK *p_wk, int y_add )
{
  int y;


  //BGの移動

  //制限
  y = GFL_BG_GetScrollY( p_wk->back_frm );
  y += y_add;
  y = MATH_CLAMP( y, SCROLL_START, SCROLL_END );

  //移動
  GFL_BG_SetScrollReq( p_wk->font_frm, GFL_BG_SCROLL_Y_SET, y + SCROLL_FONT_OFS_CHR );
  GFL_BG_SetScrollReq( p_wk->back_frm, GFL_BG_SCROLL_Y_SET, y );

  //OBJの移動
  {
    int i;
    GFL_CLACTPOS  pos;

    //移動
    p_wk->obj_y_ofs -= y_add;
    p_wk->obj_y_ofs = MATH_CLAMP( p_wk->obj_y_ofs, SCROLL_END_OBJ, SCROLL_START_OBJ );

    //設定
    for( i = 0; i < CONFIG_ITEM_MAX; i++ )
    {
      pos.x = sc_item_info[i].obj_pos.x;
      pos.y = sc_item_info[i].obj_pos.y + p_wk->obj_y_ofs;
      GFL_CLACT_WK_SetPos( p_wk->p_item[i], &pos, CLSYS_DEFREND_MAIN );
    }
  }

}

//----------------------------------------------------------------------------
/**
 *  @brief  タッチで項目を選択する処理
 *
 *  @param  SCROLL_WORK *p_wk ワーク
 *  @param  GFL_POINT *cp_pos タッチ座標
 */
//-----------------------------------------------------------------------------
static void Scroll_TouchItem( SCROLL_WORK *p_wk, const GFL_POINT *cp_pos )
{
  int i;
  GFL_RECT  rect;

  //タッチ範囲内ならば
    //項目をチェック
    for( i = 0; i < CONFIG_ITEM_MAX; i++ )
    {
      rect.top    = sc_item_info[i].touch_pos.y + p_wk->obj_y_ofs;
      rect.left   = sc_item_info[i].touch_pos.x;
      rect.bottom = rect.top + sc_item_info[i].touch_pos.h;
      rect.right  = sc_item_info[i].touch_pos.x + sc_item_info[i].touch_pos.w;

      //押せる箇所もタッチ範囲に入っているかチェック
      if( SCROLL_TOP_BAR_Y < rect.top && rect.top < SCROLL_APP_BAR_Y
          && SCROLL_TOP_BAR_Y < rect.bottom && rect.bottom < SCROLL_APP_BAR_Y )
      {

        //矩形
        if( COLLISION_IsRectXPos( &rect, cp_pos ))
        {
          PMSND_PlaySE( CONFIG_SE_MOVE );
          CONFIGPARAM_SetItem( &p_wk->now, i );
          break;
        }
      }
    }
}

//----------------------------------------------------------------------------
/**
 *  @brief  項目を選択（右か左に１つずらす）
 *
 *  @param  SCROLL_WORK *p_wk ワーク
 *  @param  s8 dir            正ならば右  負ならば左
 */
//-----------------------------------------------------------------------------
static void Scroll_SelectItem( SCROLL_WORK *p_wk, s8 dir )
{
  //選択しているとき
  if( p_wk->select < CONFIG_LIST_MAX )
  {
    CONFIGPARAM_AddItem( &p_wk->now, p_wk->select, dir );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  座標からリストへの変換
 *
 *  @param  const SCROLL_WORK *cp_wk  ワーク
 *  @param  GFL_POINT *cp_pos         タッチ座標
 *
 *  @return 選択しているリスト
 */
//-----------------------------------------------------------------------------
static CONFIG_LIST Scroll_PosToList( const SCROLL_WORK *cp_wk, const GFL_POINT *cp_pos )
{
  int y;
  CONFIG_LIST list;

  y = GFL_BG_GetScrollY( cp_wk->back_frm );
  list  = (y + cp_pos->y - SCROLL_WINDOW_OFS_DOT) / SCROLL_WINDOW_H_DOT;


  //タッチ範囲内なら計算
  if( CONFIG_LIST_MSGSPEED <= list && list < CONFIG_LIST_MAX )
  {
    return list;
  }
  else
  {
    //範囲外ならば現在のものを返す
    return cp_wk->select;
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  スライド時のパレットきりかえちかちかチェック  毎フレーム処理
 *
 *  @param  SCROLL_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Scroll_ChangePlt_Safe_Main( SCROLL_WORK *p_wk )
{
  p_wk->dir_bit <<= 1;
}

//----------------------------------------------------------------------------
/**
 *  @brief  スライド時のパレットきりかえちかちかチェック  チェックメイン処理
 *
 *  @param  SCROLL_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Scroll_ChangePlt_Safe_Check( SCROLL_WORK *p_wk, int y )
{
  //前回の方向と同じならば
  if( p_wk->pre_y / MATH_IAbs(p_wk->pre_y) == y / MATH_IAbs(y) )
  {
    p_wk->dir_bit |= 0x1;
  }
  p_wk->pre_y = y;
}
//----------------------------------------------------------------------------
/**
 *  @brief  スライド時のパレットきりかえちかちかチェック  大丈夫かどうか
 *
 *  @param  SCROLL_WORK *p_wk ワーク
 *  @return TRUEはりかえてもOK  FALSEちかちかする場合
 */
//-----------------------------------------------------------------------------
static BOOL Scroll_ChangePlt_Safe_IsOk( const SCROLL_WORK *cp_wk )
{
  int i;
  int cnt = 0;

  int now_bit = cp_wk->dir_bit & 0x1;

  for( i = 1; i < SCROLL_CHAGEPLT_SAFE_SYNC; i++ )
  {
    if( (cp_wk->dir_bit >> i) & 0x1 == now_bit )
    {
      cnt++;
    }
  }

  return cnt == SCROLL_CHAGEPLT_SAFE_SYNC-1;

}
//=============================================================================
/**
 *          CONFIRM
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  最終確認選択処理
 *
 *  @param  CONFIRM_WORK *p_wk  ワーク
 *  @param  *p_bmpwin           文字用BMPWIN
 *  @param  *p_font             フォント
 *  @param  *p_msg              メッセージ
 *  @param  *p_tcbl             ストリーム用タスク
 *  @param  *p_res              選択ウィンドウ用リソース
 *  @param  heapID              ヒープID
 */
//-----------------------------------------------------------------------------
static void CONFIRM_Init( CONFIRM_WORK *p_wk, GFL_BMPWIN *p_bmpwin, GFL_FONT *p_font, GFL_MSGDATA *p_msg, GFL_TCBLSYS *p_tcbl, APP_TASKMENU_RES *p_res, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(CONFIRM_WORK) );
  p_wk->select  = CONFIRM_SELECT_NULL;
  p_wk->p_res   = p_res;

  //最終確認メッセージ作成
  MSGWND_Init( &p_wk->msg,
      p_bmpwin,
      p_font,
      p_msg,
      p_tcbl,
      NULL,
      heapID );

  //はい、いいえウィンドウ作成
  {
    int i;
    for( i = 0; i < CONFIRM_WIN_MAX; i++ )
    {
      p_wk->item[i].str       = GFL_MSG_CreateString( p_msg, mes_config_comment21 + i );
      p_wk->item[i].msgColor  = APP_TASKMENU_ITEM_MSGCOLOR;
      p_wk->item[i].type      = APP_TASKMENU_WIN_TYPE_NORMAL;
    }
  }

  //タスクメニュー初期化構造体設定
  {
    p_wk->init.heapId   = heapID;
    p_wk->init.itemNum  = CONFIRM_WIN_MAX;
    p_wk->init.itemWork = p_wk->item;
    p_wk->init.posType   = ATPT_RIGHT_DOWN;
    p_wk->init.charPosX = 32;
    p_wk->init.charPosY = 12;
    p_wk->init.w        = APP_TASKMENU_PLATE_WIDTH;
    p_wk->init.h        = APP_TASKMENU_PLATE_HEIGHT;
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  最終確認選択  破棄
 *
 *  @param  CONFIRM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CONFIRM_Exit( CONFIRM_WORK *p_wk )
{
  //はい、いいえウィンドウ解放
  {
    int i;
    for( i = 0; i < CONFIRM_WIN_MAX; i++ )
    {
      GFL_STR_DeleteBuffer( p_wk->item[i].str );
    }
  }

  //メッセージ破棄
  MSGWND_Exit( &p_wk->msg );

  GFL_STD_MemClear( p_wk, sizeof(CONFIRM_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  最終確認選択メイン処理
 *
 *  @param  CONFIRM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CONFIRM_Main( CONFIRM_WORK *p_wk )
{
  enum
  {
    SEQ_PRINT_WAIT,
    SEQ_SELECT_INIT,
    SEQ_SELECT_WAIT,
    SEQ_SELECT_EXIT,
  };

  switch( p_wk->seq )
  {
  case SEQ_PRINT_WAIT:
    if( MSGWND_IsEndMsg( &p_wk->msg ) )
    {
      p_wk->seq = SEQ_SELECT_INIT;
    }
    break;

  case SEQ_SELECT_INIT:
    p_wk->p_menu  = APP_TASKMENU_OpenMenu( &p_wk->init, p_wk->p_res );
    p_wk->seq = SEQ_SELECT_WAIT;
    break;

  case SEQ_SELECT_WAIT:
    APP_TASKMENU_UpdateMenu( p_wk->p_menu );
    if( APP_TASKMENU_IsFinish( p_wk->p_menu ) )
    {
      //タスクメニュー破棄
      APP_TASKMENU_CloseMenu( p_wk->p_menu );
      p_wk->select  = CONFIRM_SELECT_YES + APP_TASKMENU_GetCursorPos( p_wk->p_menu );
      p_wk->seq     = SEQ_SELECT_EXIT;
    }
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *  @brief  開始
 *
 *  @param  CONFIRM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CONFIRM_Start( CONFIRM_WORK *p_wk, int wait )
{
  G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_OBJ , 8 );
  GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_DECIDE_M), TRUE );

  MSGWND_Print( &p_wk->msg, mes_config_comment20, wait );

  if( GFL_NET_IsInit() )
  { 
    //通信アイコンがBGにかぶるので位置を変える
    GFL_NET_ChangeIconPosition(-16,-16);
    GFL_NET_ReloadIcon();
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  選択取得
 *
 *  @param  const CONFIRM_WORK *cp_wk   ワーク
 *
 *  @return CONFIRM_SELECT列挙
 */
//-----------------------------------------------------------------------------
static CONFIRM_SELECT CONFIRM_Select( const CONFIRM_WORK *cp_wk )
{
  return cp_wk->select;
}
//=============================================================================
/**
 *          CONFIG_PARAM
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  設定情報  初期化
 *
 *  @param  CONFIG_PARAM *p_wk    ワーク
 *  @param  CONFIG *cp_savedata   コンフィグセーブデータ
 */
//-----------------------------------------------------------------------------
static void CONFIGPARAM_Init( CONFIG_PARAM *p_wk, CONFIG *p_savedata )
{
  GFL_STD_MemClear( p_wk, sizeof(CONFIG_PARAM) );
  p_wk->p_savedata  = p_savedata;
  p_wk->change      = CONFIG_ITEM_NONE;

  p_wk->param[CONFIG_LIST_MSGSPEED] = CONFIG_GetMsgSpeed(p_savedata);
  p_wk->param[CONFIG_LIST_BTLEFF]   = CONFIG_GetWazaEffectMode(p_savedata);
  p_wk->param[CONFIG_LIST_BTLRULE]  = CONFIG_GetBattleRule(p_savedata);
  p_wk->param[CONFIG_LIST_SND]      = CONFIG_GetSoundMode(p_savedata);
  p_wk->param[CONFIG_LIST_STR]      = CONFIG_GetMojiMode(p_savedata);
  p_wk->param[CONFIG_LIST_WIRELESS] = CONFIG_GetNetworkSearchMode(p_savedata);
  p_wk->param[CONFIG_LIST_REPORT]   = CONFIG_GetWirelessSaveMode(p_savedata);

  { 
    int i;
    for( i  = 0; i < CONFIG_LIST_MAX; i++ )
    { 
      OS_TFPrintf( 3, "config[%d] %d\n", i, p_wk->param[i] );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  設定情報  破棄
 *
 *  @param  CONFIG_PARAM *p_wk    ワーク
 */
//-----------------------------------------------------------------------------
static void CONFIGPARAM_Exit( CONFIG_PARAM *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(CONFIG_PARAM) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  設定情報  セーブデータへ反映
 *
 *  @param  const CONFIG_PARAM *cp_wk ワーク
 *  @param  *p_savedata               セーブデータ
 */
//-----------------------------------------------------------------------------
static void CONFIGPARAM_Save( const CONFIG_PARAM *cp_wk )
{
  CONFIG_SetMsgSpeed( cp_wk->p_savedata, cp_wk->param[CONFIG_LIST_MSGSPEED] );
  CONFIG_SetWazaEffectMode( cp_wk->p_savedata, cp_wk->param[CONFIG_LIST_BTLEFF] );
  CONFIG_SetBattleRule( cp_wk->p_savedata, cp_wk->param[CONFIG_LIST_BTLRULE] );
  CONFIG_SetSoundMode( cp_wk->p_savedata, cp_wk->param[CONFIG_LIST_SND] );
  CONFIG_SetMojiMode( cp_wk->p_savedata, cp_wk->param[CONFIG_LIST_STR] );
  CONFIG_SetNetworkSearchMode( cp_wk->p_savedata, cp_wk->param[CONFIG_LIST_WIRELESS] );
  CONFIG_SetWirelessSaveMode( cp_wk->p_savedata, cp_wk->param[CONFIG_LIST_REPORT] );

 { 
    int i;
    OS_TFPrintf( 3, "セーブ\n" );
    for( i  = 0; i < CONFIG_LIST_MAX; i++ )
    { 
      OS_TFPrintf( 3, "config[%d] %d\n", i, cp_wk->param[i] );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  設定情報  項目を設定
 *
 *  @param  CONFIG_PARAM *p_wk  ワーク
 *  @param  item                項目
 */
//-----------------------------------------------------------------------------
static void CONFIGPARAM_SetItem( CONFIG_PARAM *p_wk, CONFIG_ITEM item )
{
  if( p_wk->param[ sc_item_info[ item ].list ] != sc_item_info[ item ].idx )
  {
    p_wk->param[ sc_item_info[ item ].list ]  = sc_item_info[ item ].idx;

    //漢字、メッセージスピードのコンフィグ内変更により設定
    p_wk->change  = item;
    CONFIGPARAM_Save( p_wk );
  }
 { 
    int i;
    OS_TFPrintf( 3, "SetItem\n" );
    for( i  = 0; i < CONFIG_LIST_MAX; i++ )
    { 
      OS_TFPrintf( 3, "config[%d] %d\n", i, p_wk->param[i] );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  設定情報  項目を１つずらす
 *
 *  @param  CONFIG_PARAM *p_wk  ワーク
 *  @param  list                メニュー
 *  @param  dir                 ずらすオフセット
 */
//-----------------------------------------------------------------------------
static void CONFIGPARAM_AddItem( CONFIG_PARAM *p_wk, CONFIG_LIST list, s8 dir )
{
  s8 now_idx;
  CONFIG_ITEM now, next;

  //現在のアイテムを取得
  {
    int i;
    now = -1;
    for( i = 0; i < sc_list_info[list].max; i++ )
    {
      now = sc_list_info[list].item[i];
      if( sc_item_info[ now ].idx == p_wk->param[list] )
      {
        break;
      }
    }
    GF_ASSERT( now != -1 );
  }

  //差分を求めて、最大最小チェックする
  now_idx = now - sc_list_info[ list ].item[0];
  now_idx += dir;
  now_idx = MATH_CLAMP( now_idx, 0, sc_list_info[list].max-1 );

  //差分からITEMへ変換
  next    = sc_list_info[ list ].item[ now_idx ];

  //CONFIG用へ変換
  if( p_wk->param[ list ] != sc_item_info[ next ].idx )
  {
    p_wk->param[ list ] = sc_item_info[ next ].idx;

    //漢字、メッセージスピードのコンフィグ内変更により設定
    p_wk->change  = next;
    CONFIGPARAM_Save( p_wk );
  }
 { 
    int i;
    OS_TFPrintf( 3, "AddItem\n" );
    for( i  = 0; i < CONFIG_LIST_MAX; i++ )
    { 
      OS_TFPrintf( 3, "config[%d] %d\n", i, p_wk->param[i] );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  設定情報  指定した項目が設定されているかチェック
 *
 *  @param  const CONFIG_PARAM *cp_wk ワーク
 *  @param  item                      調べる項目
 *
 *  @return TRUEならば設定されている  FALSEならばされていない
 */
//-----------------------------------------------------------------------------
static BOOL CONFIGPARAM_IsItemSetting( const CONFIG_PARAM *cp_wk, CONFIG_ITEM item )
{
  return cp_wk->param[ sc_item_info[item].list ] == sc_item_info[item].idx;
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージスピード取得
 *
 *  @param  const CONFIG_PARAM *cp_wk   ワーク
 *
 *  @return メッセージスピード
 */
//-----------------------------------------------------------------------------
static int CONFIGPARAM_GetMsgSpeed( const CONFIG_PARAM *cp_wk )
{
  return MSGSPEED_GetWait();
}
//----------------------------------------------------------------------------
/**
 *  @brief  項目を変えたとき、１シンクだけ変えた項目を返す
 *
 *  @param  CONFIG_PARAM *p_wk  ワーク
 *
 *  @return 変えた項目
 */
//-----------------------------------------------------------------------------
static CONFIG_ITEM CONFIGPARAM_GetChangeTiming( CONFIG_PARAM *p_wk )
{
  CONFIG_ITEM change  = p_wk->change;
  p_wk->change  = CONFIG_ITEM_NONE;
  return change;
}
//----------------------------------------------------------------------------
/**
 *  @brief  パラメータ同士を比較する
 *
 *  @param  const CONFIG_PARAM *cp_wk1  比較１
 *  @param  CONFIG_PARAM *cp_wk2        比較２
 *
 *  @return TRUEならば同じ
 */
//-----------------------------------------------------------------------------
static BOOL CONFIGPARAM_Compare( const CONFIG_PARAM *cp_wk1, const CONFIG_PARAM *cp_wk2 )
{
  return GFL_STD_MemComp( cp_wk1->param, cp_wk2->param, sizeof(u8) * CONFIG_LIST_MAX ) == 0;
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
 *  @brief  フェードOUT
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_END,
  };

  CONFIG_WORK *p_wk = p_param;

  switch( *p_seq )
  {
  case SEQ_FADEOUT_START:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;

  case SEQ_FADEOUT_WAIT:
    if( !GFL_FADE_CheckFade() )
    {
      *p_seq  = SEQ_END;
    }
    break;

  case SEQ_END:
    SEQ_SetNext( p_seqwk, SEQFUNC_Main );
    break;

  default:
    GF_ASSERT(0);
  }

}
//----------------------------------------------------------------------------
/**
 *  @brief  フェードIN
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_EXIT,
  };

  CONFIG_WORK *p_wk = p_param;

  switch( *p_seq )
  {
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
    SEQ_End( p_seqwk );
    break;

  default:
    GF_ASSERT(0);
  }

}
//----------------------------------------------------------------------------
/**
 *  @brief  設定画面メイン処理
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  CONFIG_WORK *p_wk = p_param;

  //モジュールメイン

  //決定していないときに動く
  if( !APPBAR_IsDecide( &p_wk->appbar, NULL ) )
  {
    SCROLL_Main( &p_wk->scroll, &p_wk->ui, &p_wk->info, &p_wk->graphic, &p_wk->appbar );
  }
  APPBAR_Main(  &p_wk->appbar, &p_wk->ui, &p_wk->scroll );
  UI_Main( &p_wk->ui );

  //状態変移
  {
    APPBAR_WIN_LIST select;
    if( APPBAR_IsDecide( &p_wk->appbar, &select ) )
    {
        switch( select )
        {
        case APPBAR_WIN_DECIDE:
          if( APPBAR_IsWaitEffect( &p_wk->appbar ) )
          { 
            SEQ_SetNext( p_seqwk, SEQFUNC_SetNowConfig );
          }
          break;

        case APPBAR_WIN_EXIT:
          { 
            CONFIG_PARAM  now;
            p_wk->p_param->is_exit  = TRUE;
            SCROLL_GetConfigParam( &p_wk->scroll, &now );
            //今の設定が前の設定と違うならば、最終確認画面を出す
            if( CONFIGPARAM_Compare( &p_wk->pre, &now ) )
            {
              SEQ_SetNext( p_seqwk, SEQFUNC_SetPreConfig );
            }
            else
            {
              APPBAR_StopEffect( &p_wk->appbar );
              SEQ_SetNext( p_seqwk, SEQFUNC_Decide );
            }
          }
          break;
        case APPBAR_WIN_CANCEL:
          if( APPBAR_IsWaitEffect( &p_wk->appbar ) )
          {
            CONFIG_PARAM  now;
            SCROLL_GetConfigParam( &p_wk->scroll, &now );
            //今の設定が前の設定と違うならば、最終確認画面を出す
            if( CONFIGPARAM_Compare( &p_wk->pre, &now ) )
            {
              SEQ_SetNext( p_seqwk, SEQFUNC_SetPreConfig );
            }
            else
            {
              APPBAR_StopEffect( &p_wk->appbar );
              SEQ_SetNext( p_seqwk, SEQFUNC_Decide );
            }
          }
          break;
        }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  最終確認処理
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Decide( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_YES,
    SEQ_NO,
  };

  CONFIG_WORK *p_wk = p_param;

  switch( *p_seq )
  {
  case SEQ_INIT:
    {
      int speed;
      CONFIG_PARAM config;

      SCROLL_GetConfigParam( &p_wk->scroll, &config );
      speed = CONFIGPARAM_GetMsgSpeed(&config);
      CONFIRM_Start( &p_wk->confirm, speed );
      GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TEXT_S), FALSE );
    }
    *p_seq  = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    switch( CONFIRM_Select( &p_wk->confirm ) )
    {
    case CONFIRM_SELECT_YES:
      *p_seq  = SEQ_YES;
      break;
    case CONFIRM_SELECT_NO:
      *p_seq  = SEQ_NO;
      break;
    }
    break;

  case SEQ_YES:
    SEQ_SetNext( p_seqwk, SEQFUNC_SetNowConfig );
    break;

  case SEQ_NO:
    SEQ_SetNext( p_seqwk, SEQFUNC_SetPreConfig );
    break;
  }

  CONFIRM_Main( &p_wk->confirm );
}
//----------------------------------------------------------------------------
/**
 *  @brief  変更前の設定反映処理
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_SetPreConfig( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  CONFIG_WORK *p_wk = p_param;

  //設定変更
  CONFIGPARAM_Save( &p_wk->pre );

  SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
}
//----------------------------------------------------------------------------
/**
 *  @brief  変更後の設定反映処理
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_SetNowConfig( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  CONFIG_WORK *p_wk = p_param;

  //設定変更
  {
    CONFIG_PARAM config;
    SCROLL_GetConfigParam( &p_wk->scroll, &config );
    CONFIGPARAM_Save( &config );
  }

  PMSND_PlaySE( CONFIG_SE_SAVE );

  SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
}
//=============================================================================
/**
 *    ETC
 */
//=============================================================================
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
 *  @brief  矩形と点の衝突検知
 *
 *  @param  const GFL_RECT *cp_rect   矩形
 *  @param  GFL_POINT *cp_pos         点
 *
 *  @return TRUEでヒット  FALSEでヒットしていない
 */
//-----------------------------------------------------------------------------
static BOOL COLLISION_IsRectXPos( const GFL_RECT *cp_rect, const GFL_POINT *cp_pos )
{
  return ( ((u32)( cp_pos->x - cp_rect->left) <= (u32)(cp_rect->right - cp_rect->left))
            & ((u32)( cp_pos->y - cp_rect->top) <= (u32)(cp_rect->bottom - cp_rect->top)));
}
