//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file   namein.c
 *  @brief  名前入力画面
 *  @author Toru=Nagihashi
 *  @data   2009.10.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//  lib
#include <gflib.h>

//  SYSTEM
#include "system/main.h"  //HEAPID
#include "system/gfl_use.h"
#include "gamesystem/gamesystem.h"
#include "system/bmp_winframe.h"
#include "system/nsbtx_to_clwk.h"
#include "sound/pm_sndsys.h"
#include "gamesystem/msgspeed.h"

//  module
#include "app/app_menu_common.h"
#include "pokeicon/pokeicon.h"

//  archive
#include "arc_def.h"
#include "message.naix"
#include "namein_gra.naix"
#include "font/font.naix"
#include "fieldmap/fldmmdl_mdlres.naix" //フィールド人物OBJ
#include "msg/msg_namein.h"
#include "wifi_unionobj_plt.cdat" //ユニオンOBJのパレット位置
#include "wifi_unionobj.naix"

//  print
#include "print/str_tool.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_msg.h"

//  savedata
#include "savedata/misc.h"

//  se
#include "sound/sound_manager.h"

//  mine
#include "namein_snd.h"     //サウンド定義
#include "namein_graphic.h" //グラフィック設定
#include "namein_data.h"    //名前入力データ（キー配列と変換）
#include "app/name_input.h" //外部参照

#include "debug/debug_str_conv.h"

//=============================================================================
/**
 *          定数宣言
*/
//=============================================================================
//-------------------------------------
/// デバッグ関係
//=====================================
#ifdef PM_DEBUG
//#define DEBUG_PRINT_ON
#endif //PM_DEBUG

#ifdef DEBUG_PRINT_ON
//ユニコードデバッグプリント
static inline void DEBUG_NAMEIN_Print( STRCODE *x , int len )
{ 
  int i;  
  char str[3] = {0};
  for( i = 0; i < len; i++ )
  {
    DEB_STR_CONV_StrcodeToSjis( &cp_code[i] , str , 1 );
    NAGI_Printf( "%s ", str );
  }
  NAGI_Printf("\n");
}
#else
#define DEBUG_NAMEIN_Print(x,len) /*  */
#endif //DEBUG_PRINT_ON


#define NAMEIN_KEY_TOUCH  //キーとタッチの操作わけ処理ON


//-------------------------------------
/// マクロ
//=====================================
//回り込み
#define MATH_ROUND(x,min,max)   (x < min? max: x > max ? min: x)

//-------------------------------------
/// ヒープサイズ
//=====================================
#define NAMEIN_HEAP_HIGH_SIZE (0x70000)
#define NAMEIN_HEAP_LOW_SIZE (0x40000)

//-------------------------------------
/// BGフレーム
//=====================================
enum
{
  //メイン
  BG_FRAME_STR_M  = GFL_BG_FRAME0_M,    //入力中の文字列欄
  BG_FRAME_KEY_M  = GFL_BG_FRAME1_M,    //キー配列
  BG_FRAME_BTN_M  = GFL_BG_FRAME2_M,    //共通ボタン
  BG_FRAME_FONT_M = GFL_BG_FRAME3_M,    //フォント

  //サブ
  BG_FRAME_TEXT_S = GFL_BG_FRAME0_S,    //テキスト欄
  BG_FRAME_BACK_S = GFL_BG_FRAME1_S,    //背景
} ;

//-------------------------------------
/// パレット
//=====================================
enum
{
  //メインBG
  PLT_BG_STR_M        = 0,  //入力文字列
  PLT_BG_KEY_NORMAL_M = 1,  //背景基本色パレット
  PLT_BG_KEY_MOVE_M   = 2,  //背景移動時パレット
  PLT_BG_KEY_DECIDE_M = 3,  //背景決定用パレット
  PLT_BG_KEY_PRESS_M  = 4,  //押し続け用パレット

  PLT_BG_FONT_M       = 15, //フォント色

  //メインOBJ
  PLT_OBJ_CURSOR_M    = 0,  //文字列の下のバーOBJ
  PLT_OBJ_ICON_M      = 2,  //アイコン

  //サブBG
  PLT_BG_FRAME_S      = 0,  //WINDOWフレーム
  PLT_BG_FONT_S       = 15, //フォント色

  //サブOBJ
  PLT_OBJ_S           = 0,
} ;

//-------------------------------------
/// リソースインデックス
//=====================================
typedef enum
{
  RESID_OBJ_COMMON_PLT,
  RESID_OBJ_COMMON_CHR,
  RESID_OBJ_COMMON_CEL,
  RESID_OBJ_MAX,
} RESID;

//-------------------------------------
/// OBJ
//=====================================
//バーの数
#define OBJ_BAR_NUM (16)

//-------------------------------------
/// CLWK
//=====================================
typedef enum
{
  CLWKID_BAR_TOP,
  CLWKID_BAR_END  = CLWKID_BAR_TOP + OBJ_BAR_NUM,
  CLWKID_PC,
  CLWKID_MAX,
} CLWKID;

//-------------------------------------
/// STRINPUT
//=====================================
//文字列長
#define STRINPUT_STR_LEN      (NAMEIN_BOX_LENGTH)     //入力文字列長
#define STRINPUT_CHANGE_LEN   (3)                     //変換文字列長
//BMPWINサイズ
#define STRINPUT_BMPWIN_X     (8)
#define STRINPUT_BMPWIN_Y     (2)
#define STRINPUT_BMPWIN_W     (16)
#define STRINPUT_BMPWIN_H     (2)
//文字描画位置
#define STRINPUT_STR_START_X  (0)
#define STRINPUT_STR_OFFSET_X (14)
#define STRINPUT_STR_Y        (0)
//BARの位置
#define STRINPUT_BAR_START_X  (64)
#define STRINPUT_BAR_OFFSET_X (STRINPUT_STR_OFFSET_X)
#define STRINPUT_BAR_Y        (36)
//特殊文字変換
typedef enum
{ 
  STRINPUT_SP_CHANGE_DAKUTEN,     //濁点
  STRINPUT_SP_CHANGE_HANDAKUTEN,  //半濁点
} STRINPUT_SP_CHANGE;

//-------------------------------------
/// KEYMAP
//=====================================
//幅、高さ
#define KEYMAP_KANA_WIDTH     (13)
#define KEYMAP_KANA_HEIGHT    (5)
#define KEYMAP_QWERTY_WIDTH   (11)
#define KEYMAP_QWERTY_HEIGHT  (4)

//モード
typedef enum
{ 
  NAMEIN_INPUTTYPE_KANA = NAMEIN_KEYMAPTYPE_KANA,   //かな
  NAMEIN_INPUTTYPE_KATA = NAMEIN_KEYMAPTYPE_KATA,   //カナ
  NAMEIN_INPUTTYPE_ABC  = NAMEIN_KEYMAPTYPE_ABC,      //ABC
  NAMEIN_INPUTTYPE_KIGOU= NAMEIN_KEYMAPTYPE_KIGOU,    //1/♪
  NAMEIN_INPUTTYPE_QWERTY = NAMEIN_KEYMAPTYPE_QWERTY, //ﾛｰﾏじ

  NAMEIN_INPUTTYPE_MAX  =NAMEIN_KEYMAPTYPE_MAX,
}NAMEIN_INPUTTYPE;

//キー情報
typedef enum
{ 
  KEYMAP_KEYTYPE_NONE,    //なにもなし
  KEYMAP_KEYTYPE_STR,     //文字入力
  KEYMAP_KEYTYPE_KANA,    //かなモード
  KEYMAP_KEYTYPE_KATA,    //カナモード
  KEYMAP_KEYTYPE_ABC,     //アルファベットモード
  KEYMAP_KEYTYPE_KIGOU,   //記号モード
  KEYMAP_KEYTYPE_QWERTY,  //ローマ字モード
  KEYMAP_KEYTYPE_DELETE,  //けす
  KEYMAP_KEYTYPE_DECIDE,  //やめる
  KEYMAP_KEYTYPE_SHIFT,   //シフト
  KEYMAP_KEYTYPE_SPACE,   //スペース
  KEYMAP_KEYTYPE_DAKUTEN,   //濁点
  KEYMAP_KEYTYPE_HANDAKUTEN,    //半濁点
}KEYMAP_KEYTYPE;

//キー移動保存バッファ
enum
{
  //kana
  KEYMAP_KEYMOVE_BUFF_DELETE, //消すから出るとき（上下共通）
  KEYMAP_KEYMOVE_BUFF_DICEDE, //おわりから出るとき（上下共通）

  //qwerty
  KEYMAP_KEYMOVE_BUFF_Q_U,  //Qから上にいくとき
  KEYMAP_KEYMOVE_BUFF_A_D,  //Aから下にいくとき
  KEYMAP_KEYMOVE_BUFF_L_U,  //Lから上にいくとき
  KEYMAP_KEYMOVE_BUFF_BOU_U,//ーから上にいくとき
  KEYMAP_KEYMOVE_BUFF_LSHIFT_D, //左シフトから下にいくとき
  KEYMAP_KEYMOVE_BUFF_SPACE_U,  //スペースから上にいくとき
  KEYMAP_KEYMOVE_BUFF_SPACE_D,  //スペースから下にいくとき

  KEYMAP_KEYMOVE_BUFF_MAX,
};
//キー移動デフォルト値
static const u8 sc_keymove_default[KEYMAP_KEYMOVE_BUFF_MAX] =
{ 
  5,9,  0,1,9,8,0,4,4
};

//-------------------------------------
/// KEYBOARD
//=====================================
//キーボードBMPWINサイズ
#define KEYBOARD_BMPWIN_X   (3)
#define KEYBOARD_BMPWIN_Y   (5)
#define KEYBOARD_BMPWIN_W   (26)
#define KEYBOARD_BMPWIN_H   (15)
//モード切替移動
#define KEYBOARD_CHANGEMOVE_START_Y     (0)
#define KEYBOARD_CHANGEMOVE_END_Y       (-48)
#define KEYBOARD_CHANGEMOVE_SYNC        (7)
#define KEYBOARD_CHANGEMOVE_START_ALPHA (16)
#define KEYBOARD_CHANGEMOVE_END_ALPHA   (0)
//状態
typedef enum
{ 
  KEYBOARD_STATE_WAIT,  //処理待ち中
  KEYBOARD_STATE_INPUT, //入力
  KEYBOARD_STATE_MOVE,  //移動中
}KEYBOARD_STATE;
//入力情報
typedef enum
{ 
  KEYBOARD_INPUT_NONE,        //入力していない
  KEYBOARD_INPUT_STR,         //文字入力
  KEYBOARD_INPUT_CHANGESTR,   //変換文字入力
  KEYBOARD_INPUT_DAKUTEN,     //濁点
  KEYBOARD_INPUT_HANDAKUTEN,  //半濁点
  KEYBOARD_INPUT_BACKSPACE,   //一つ前に戻る
  KEYBOARD_INPUT_CHAGETYPE,   //入力タイプ変更
  KEYBOARD_INPUT_EXIT,        //終了  
  KEYBOARD_INPUT_SHIFT,       //シフト
  KEYBOARD_INPUT_SPACE,       //スペース
}KEYBOARD_INPUT;

//-------------------------------------
/// KEYANM
//=====================================
//カウント
#define KEYANM_DECIDE_CNT (10)
//何のアニメか
typedef enum
{
  KEYANM_TYPE_NONE,
  KEYANM_TYPE_MOVE,
  KEYANM_TYPE_DECIDE
} KEYANM_TYPE;

//-------------------------------------
/// MSGWND
//=====================================
//フレームを読み込んだキャラ位置
#define MSGWND_FRAME_CHR  (1)
//BMPWINの幅
#define MSGWND_BMPWIN_X (1)
#define MSGWND_BMPWIN_Y (19)
#define MSGWND_BMPWIN_W (30)
#define MSGWND_BMPWIN_H (4)

//-------------------------------------
/// ICON
//=====================================
//アイコンの座標
#define ICON_POS_X  (46)
#define ICON_POS_Y  (20)
#define ICON_WIDTH  (32)
#define ICON_HEIGHT (32)

//アイコンの動作
#define ICON_MOVE_SYNC  (8)
#define ICON_MOVE_Y     (12)
#define ICON_MOVE_START ((0*0xffff) / 360)
#define ICON_MOVE_END   ((180*0xffff) / 360)
#define ICON_MOVE_DIF   ( ICON_MOVE_END - ICON_MOVE_START )

//アイコンの種類
typedef enum
{
  ICON_TYPE_HERO    = NAMEIN_MYNAME,
  ICON_TYPE_RIVAL   = NAMEIN_RIVALNAME,
  ICON_TYPE_PERSON  = NAMEIN_FRIENDNAME,
  ICON_TYPE_POKE    = NAMEIN_POKEMON,
  ICON_TYPE_BOX     = NAMEIN_BOX,
} ICON_TYPE;

//-------------------------------------
/// その他
//=====================================
#define NAMEIN_DEFAULT_NAME_MAX (2)

//=============================================================================
/**
 *          構造体宣言
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
/// BGリソース管理
//=====================================
typedef struct 
{
  int dummy;
} BG_WORK;
//-------------------------------------
/// OBJリソース管理
//=====================================
typedef struct 
{
  u32         res[RESID_OBJ_MAX];   //リソース
  GFL_CLWK    *p_clwk[CLWKID_MAX];  //CLWK
} OBJ_WORK;
//-------------------------------------
/// KEYANM
//=====================================
typedef struct 
{
  u16   plt_normal[0x10]; //通常時の元のパレット
  u16   plt_flash[0x10];  //決定時の元のパレット
  u16   color[0x10];      //色バッファ
  u16   cnt[0x10];        //カウント
  u16   decide_cnt;       //決定アニメ状態カウント
  

  GFL_RECT    range;      //ボタンの矩形
  KEYANM_TYPE type;       //アニメの種類
  BOOL        is_start;   //開始フラグ
  BOOL        is_shift;   //シフト押下フラグ
  NAMEIN_INPUTTYPE mode;  //モード
} KEYANM_WORK;

//-------------------------------------
/// STRINPUT 入力した文字欄
//=====================================
typedef struct 
{
  STRCODE           input_str[STRINPUT_STR_LEN+1];    //確定文字列
  STRCODE           change_str[STRINPUT_CHANGE_LEN+1];//変換文字列
  u16               input_idx;    //確定文字列文字数（EOMのぞく）
  u16               change_idx;   //変換文字列文字数（EOMのぞく）
  u16               strlen;       //EOMを除く最大文字列長
  u16               dummy;
  BOOL              is_update;    //文字描画アップデート
  PRINT_UTIL        util;         //文字描画UTIL
  GFL_FONT          *p_font;      //フォント
  PRINT_QUE         *p_que;       //文字キュー
  STRBUF            *p_strbuf;    //文字バッファ
  GFL_BMPWIN        *p_bmpwin;    //BMPWIN
  GFL_CLWK          *p_clwk[OBJ_BAR_NUM];   //バーのOBJ

  NAMEIN_STRCHANGE  *p_changedata[NAMEIN_STRCHANGETYPE_MAX];  //変換データ
} STRINPUT_WORK;
//-------------------------------------
/// キー配列
//=====================================
typedef struct 
{
  u8 data[KEYMAP_KEYMOVE_BUFF_MAX];
} KEYMOVE_BUFF;
typedef BOOL (*GET_KEYRECT_FUNC)( const GFL_POINT *cp_cursor, GFL_RECT *p_rect );
typedef void (*GET_KEYMOVE_FUNC)( GFL_POINT *p_now, const GFL_POINT *cp_add, KEYMOVE_BUFF *p_buff );
typedef KEYMAP_KEYTYPE (*GET_KEYTYPE_FUNC)( const GFL_POINT *cp_cursor, GFL_POINT *p_strpos );
typedef struct 
{
  NAMEIN_KEYMAP     *p_key; //モードによってことなるキー配列情報

  KEYMOVE_BUFF      buff;   //キー移動バッファ

  //以下関数型は、モードによって、
  //キー配列が異なる情報（キーの位置、カーソルの移動方法、キーの種類判別）を
  //関数切り替えによって実現させるための型
  GET_KEYRECT_FUNC  get_keyrect;    //キーの位置（矩形）
  GET_KEYMOVE_FUNC  get_movecursor; //カーソルの移動
  GET_KEYTYPE_FUNC  get_keytype;    //キーの種類

  u16               w;  //配列最大WIDTH
  u16               h;  //配列最大HEIGHT
} KEYMAP_WORK;
//-------------------------------------
/// メッセージウィンドウ
//=====================================
typedef struct 
{
  GFL_MSGDATA       *p_msg;     //メッセージデータ
  GFL_FONT          *p_font;    //フォント
  PRINT_UTIL        util;       //文字描画
  PRINT_QUE         *p_que;     //文字キュー
  GFL_BMPWIN*       p_bmpwin;   //BMPWIN
  WORDSET           *p_word;    //単語
  STRBUF*           p_strbuf;   //文字バッファ
  u16               clear_chr;  //クリアキャラ
  u16               dummy;
} MSGWND_WORK;
//-------------------------------------
/// プリントストリーム
//=====================================
typedef struct 
{
  PRINT_STREAM      *p_stream;
  GFL_TCBLSYS       *p_tcblsys;
  SEQ_FUNCTION      next_seq_function;
  u16               wait_max;
  u16               wait_count;
} PS_WORK;
//-------------------------------------
/// キーボード
//=====================================
typedef struct 
{
  KEYMAP_WORK     keymap;     //キー情報
  KEYANM_WORK     keyanm;     //キーアニメ
  GFL_POINT       cursor;     //カーソル位置
  GFL_BMPWIN      *p_bmpwin;  //BMPWIN
  KEYBOARD_STATE  state;      //状態
  KEYBOARD_INPUT  input;      //入力種類
  PRINT_UTIL      util;       //文字描画
  GFL_FONT        *p_font;    //フォント
  PRINT_QUE       *p_que;     //文字キュー
  STRCODE         code;       //入力したコード
  HEAPID          heapID;     //ヒープID(Main内で読みこみがあるため)
  NAMEIN_INPUTTYPE mode;      //入力モード（かな・ABCとか）
  BOOL            is_shift;   //シフトを押しているか
  //以下、モード切替演出用
  u16             change_move_cnt;  //モード切替のときのアニメカウント
  u16             change_move_seq;  //モード切替のときのシーケンス
  BOOL            is_change_anm;    //モード切替のアニメリクエスト
  NAMEIN_INPUTTYPE change_mode;     //モード切替する入力モード（かな・ABCとか）
  BOOL            is_btn_move;      //ボタンも動作するか

  void              *p_scr_adrs[2];
  NNSG2dScreenData  *p_scr[2];

  NAMEIN_KEYMAP_HANDLE *p_keymap_handle;

} KEYBOARD_WORK;
//-------------------------------------
/// アイコン
//=====================================
typedef struct 
{
  GFL_CLWK  *p_clwk;  //アイコンのCLWK
  ICON_TYPE type;     //アイコンの種類
  u32       plt;      //パレット登録ID
  u32       ncg;      //キャラ登録ID
  u32       cel;      //セル登録ID
  BOOL      is_trg;   //トリガー入力
  u32       seq;      //メインシーケンス
  BOOL      is_move_start;//移動開始フラグ
  u16       sync;         //移動シンク
} ICON_WORK;
//-------------------------------------
/// メインワーク
//=====================================
typedef struct 
{
  //グラフィック設定
  NAMEIN_GRAPHIC_WORK *p_graphic;

  //OBJリソース
  OBJ_WORK      obj;

  //BGリソース
  BG_WORK       bg;

  //入力文字欄
  STRINPUT_WORK strinput;

  //キーボード
  KEYBOARD_WORK keyboard;

  //キーマップデータ
  NAMEIN_KEYMAP_HANDLE *p_keymap_handle;

  //上画面ウィンドウ
  MSGWND_WORK   msgwnd;

  //上画面ウィンドウのプリントストリーム
  PS_WORK       ps;

  //文字欄横のアイコン
  ICON_WORK     icon;

  //シーケンス
  SEQ_WORK      seq;

  //共通で使うフォント
  GFL_FONT      *p_font;

  //共通で使うキュー
  PRINT_QUE     *p_que;

  //共通で使うメッセージ
  GFL_MSGDATA   *p_msg;

  //共通で使う単語
  WORDSET       *p_word;

  //引数
  NAMEIN_PARAM  *p_param;
} NAMEIN_WORK;
//=============================================================================
/**
 *          プロトタイプ
*/
//=============================================================================
//-------------------------------------
/// プロセス
//=====================================
static GFL_PROC_RESULT NAMEIN_PROC_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT NAMEIN_PROC_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT NAMEIN_PROC_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
/// BGリソースなど
//=====================================
static void BG_Init( BG_WORK *p_wk, HEAPID heapID );
static void BG_Exit( BG_WORK *p_wk );
//-------------------------------------
/// OBJリソースなど
//=====================================
static void OBJ_Init( OBJ_WORK *p_wk, GFL_CLUNIT *p_clunit, HEAPID heapID );
static void OBJ_Exit( OBJ_WORK *p_wk );
static GFL_CLWK *OBJ_GetClwk( const OBJ_WORK *cp_wk, CLWKID clwkID );
//-------------------------------------
/// パレットアニメ
//=====================================
static void KEYANM_Init( KEYANM_WORK *p_wk, NAMEIN_INPUTTYPE mode, HEAPID heapID );
static void KEYANM_Exit( KEYANM_WORK *p_wk );
static void KEYANM_Start( KEYANM_WORK *p_wk, KEYANM_TYPE type, const GFL_RECT *cp_rect, BOOL is_shift, NAMEIN_INPUTTYPE mode );
static void KEYANM_Main( KEYANM_WORK *p_wk, NAMEIN_INPUTTYPE mode, const GFL_RECT *cp_rect );
static void KEYANM_Reset( KEYANM_WORK *p_wk );
static BOOL KeyAnm_PltFade( u16 *p_buff, u16 *p_cnt, u16 add, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end );
//-------------------------------------
/// STRINPUT
//=====================================
static void STRINPUT_Init( STRINPUT_WORK *p_wk, const STRBUF * cp_default_str, u8 strlen, GFL_FONT *p_font, PRINT_QUE *p_que, const OBJ_WORK *cp_obj, HEAPID heapID );
static void STRINPUT_Exit( STRINPUT_WORK *p_wk );
static void STRINPUT_Main( STRINPUT_WORK *p_wk );
static void STRINPUT_BackSpace( STRINPUT_WORK *p_wk );
static BOOL STRINPUT_SetStr( STRINPUT_WORK *p_wk, STRCODE code );
static BOOL STRINPUT_SetChangeStr( STRINPUT_WORK *p_wk, STRCODE code, BOOL is_shift );
static BOOL STRINPUT_SetChangeSP( STRINPUT_WORK *p_wk, STRINPUT_SP_CHANGE type );
static BOOL STRINPUT_PrintMain( STRINPUT_WORK *p_wk );
static void STRINPUT_CopyStr( const STRINPUT_WORK *cp_wk, STRBUF *p_strbuf );
static void STRINPUT_DeleteChangeStr( STRINPUT_WORK *p_wk );
static void STRINPUT_DecideChangeStr( STRINPUT_WORK *p_wk );
static void STRINPUT_Delete( STRINPUT_WORK *p_wk );
static void STRINPUT_SetLongStr( STRINPUT_WORK *p_wk, const STRCODE *cp_code );
static BOOL STRINPUT_IsInputEnd( const STRINPUT_WORK *cp_wk );
static u32  STRINPUT_GetStrLength( const STRINPUT_WORK *cp_wk );
static BOOL StrInput_ChangeStrToStr( STRINPUT_WORK *p_wk, BOOL is_shift );
//-------------------------------------
/// KEYMAP
//=====================================
static void KEYMAP_Create( KEYMAP_WORK *p_wk, NAMEIN_INPUTTYPE mode, NAMEIN_KEYMAP_HANDLE *p_keymap_handle, HEAPID heapID );
static void KEYMAP_Delete( KEYMAP_WORK *p_wk );
static KEYMAP_KEYTYPE KEYMAP_GetKeyInfo( const KEYMAP_WORK *cp_wk, const GFL_POINT *cp_cursor, STRCODE *p_code );
static BOOL KEYMAP_GetTouchCursor( const KEYMAP_WORK *cp_wk, const GFL_POINT *cp_trg, GFL_POINT *p_cursor );
static void KEYMAP_MoveCursor( KEYMAP_WORK *p_wk, GFL_POINT *p_now, const GFL_POINT *cp_add );
static void KEYMAP_Print( const KEYMAP_WORK *cp_wk, PRINT_UTIL *p_util, PRINT_QUE *p_que, GFL_FONT *p_font, HEAPID heapID );
static BOOL KEYMAP_GetRange( const KEYMAP_WORK *cp_wk, const GFL_POINT *cp_cursor, GFL_RECT *p_rect );

static BOOL KEYMAP_GetCursorByKeyType( const KEYMAP_WORK *cp_wk, KEYMAP_KEYTYPE key, GFL_POINT *p_cursor );
static void KEYMAP_TOOL_ChangeCursorPos( GFL_POINT *p_cursor, NAMEIN_INPUTTYPE pre, NAMEIN_INPUTTYPE next );
static BOOL KeyMap_KANA_GetRect( const GFL_POINT *cp_cursor, GFL_RECT *p_rect );
static BOOL KeyMap_QWERTY_GetRect( const GFL_POINT *cp_cursor, GFL_RECT *p_rect );
static void KeyMap_KANA_GetMoveCursor( GFL_POINT *p_now, const GFL_POINT *cp_add, KEYMOVE_BUFF *p_buff );
static void KeyMap_QWERTY_GetMoveCursor( GFL_POINT *p_now, const GFL_POINT *cp_add, KEYMOVE_BUFF *p_buff );
static KEYMAP_KEYTYPE KeyMap_KANA_GetKeyType( const GFL_POINT *cp_cursor, GFL_POINT *p_strpos );
static KEYMAP_KEYTYPE KeyMap_KIGOU_GetKeyType( const GFL_POINT *cp_cursor, GFL_POINT *p_strpos );
static KEYMAP_KEYTYPE KeyMap_QWERTY_GetKeyType( const GFL_POINT *cp_cursor, GFL_POINT *p_strpos );
//-------------------------------------
/// KEYBOARD_WORK
//=====================================
static void KEYBOARD_Init( KEYBOARD_WORK *p_wk, NAMEIN_INPUTTYPE mode, GFL_FONT *p_font, PRINT_QUE *p_que, NAMEIN_KEYMAP_HANDLE *p_keymap_handle, HEAPID heapID );
static void KEYBOARD_Exit( KEYBOARD_WORK *p_wk );
static void KEYBOARD_Main( KEYBOARD_WORK *p_wk, const STRINPUT_WORK *cp_strinput );
static KEYBOARD_STATE KEYBOARD_GetState( const KEYBOARD_WORK *cp_wk );
static KEYBOARD_INPUT KEYBOARD_GetInput( const KEYBOARD_WORK *cp_wk, STRCODE *p_str );
static BOOL KEYBOARD_IsShift( const KEYBOARD_WORK *cp_wk );
static BOOL KEYBOARD_PrintMain( KEYBOARD_WORK *p_wk );
static NAMEIN_INPUTTYPE KEYBOARD_GetInputType( const KEYBOARD_WORK *cp_wk );
static BOOL Keyboard_StartMove( KEYBOARD_WORK *p_wk, NAMEIN_INPUTTYPE mode );
static BOOL Keyboard_MainMove( KEYBOARD_WORK *p_wk );
static void Keyboard_ChangeMode( KEYBOARD_WORK *p_wk, NAMEIN_INPUTTYPE mode );

typedef struct
{ 
  KEYMAP_KEYTYPE  type;
  BOOL            is_push;    //ボタンを押した
  GFL_POINT       anm_pos;
}KEYBOARD_INPUT_REQUEST;
//通常時
static BOOL Keyboard_KeyReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req );
static BOOL Keyboard_BtnReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req );
static BOOL Keyboard_TouchReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req );
static void Keyboard_Decide( KEYBOARD_WORK *p_wk, const KEYBOARD_INPUT_REQUEST *cp_req );
//モード切替時
static BOOL Keyboard_Move_KeyReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req );
static BOOL Keyboard_Move_BtnReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req );
static BOOL Keyboard_Move_TouchReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req );
static void Keyboard_Move_Decide( KEYBOARD_WORK *p_wk, const KEYBOARD_INPUT_REQUEST *cp_req );

//-------------------------------------
/// MSGWND
//=====================================
static void MSGWND_Init( MSGWND_WORK* p_wk, GFL_FONT *p_font, GFL_MSGDATA *p_msg, PRINT_QUE *p_que, WORDSET *p_word, HEAPID heapID );
static void MSGWND_Exit( MSGWND_WORK* p_wk );
static void MSGWND_Print( MSGWND_WORK* p_wk, u32 strID );
static void MSGWND_ExpandPrintPoke( MSGWND_WORK *p_wk, u32 strID, u16 mons_no, u16 form, HEAPID heapID );
static void MSGWND_ExpandPrintPP( MSGWND_WORK *p_wk, u32 strID, const POKEMON_PARAM *cp_pp );
static BOOL MSGWND_PrintMain( MSGWND_WORK* p_wk );

//-------------------------------------
/// PS(PrintStream)
//=====================================
static void PS_Init( PS_WORK* p_wk, HEAPID heapID );
static void PS_Exit( PS_WORK* p_wk );
static void PS_SetupBox( PS_WORK* p_wk, MSGWND_WORK* p_msgwnd_wk, NAMEIN_PARAM* p_param, HEAPID heapID );

//-------------------------------------
/// ICON
//=====================================
static void ICON_Init( ICON_WORK *p_wk, ICON_TYPE type, u32 param1, u32 param2, GFL_CLUNIT *p_unit, HEAPID heapID );
static void ICON_Exit( ICON_WORK *p_wk );
static void ICON_Main( ICON_WORK *p_wk );
static BOOL ICON_IsTrg( const ICON_WORK *cp_wk );
static void Icon_StartMove( ICON_WORK *p_wk );
static BOOL Icon_MainMove( ICON_WORK *p_wk );
//-------------------------------------
/// その他
//=====================================
static BOOL COLLISION_IsRectXPos( const GFL_RECT *cp_rect, const GFL_POINT *cp_pos );
static STRBUF* DEFAULTNAME_CreateStr( const NAMEIN_WORK *cp_wk, NAMEIN_MODE mode, HEAPID heapID );
static void FinishInput( NAMEIN_WORK *p_wk );
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
static void SEQFUNC_WaitPrint( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_PrintStream( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_End( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_SaveEnd( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
//=============================================================================
/**
 *          外部参照
*/
//=============================================================================
//-------------------------------------
/// プロセステーブル
//=====================================
const GFL_PROC_DATA NameInputProcData =
{ 
  NAMEIN_PROC_Init,
  NAMEIN_PROC_Main,
  NAMEIN_PROC_Exit,
};

//-------------------------------------
/// SE
//=====================================
const u32 NAMEIN_SE_PresetData[]  =
{ 
  NAMEIN_SE_MOVE_CURSOR,
  NAMEIN_SE_DELETE_STR,
  NAMEIN_SE_DECIDE_STR,
  NAMEIN_SE_CHANGE_MODE,
  NAMEIN_SE_DECIDE,
};

const u32 NAMEIN_SE_PresetNum = 5;
//=============================================================================
/**
 *    外部参照
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINに渡すPARAM構造体作成
 *
 *  @param  HEAPID heapId         ヒープID
 *  @param  mode                  入力の種類
 *  @param  param1                種類ごとに違う引数１
 *  @param  param2                種類ごとに違う引数２
 *  @param  wordmax               入力文字最大数
 *  @param  STRBUF *default_str   デフォルトで入力されている文字列
 *
 *  @retura NAMEIN_PARAM
 */
//-----------------------------------------------------------------------------
NAMEIN_PARAM *NAMEIN_AllocParam( HEAPID heapId, NAMEIN_MODE mode, int param1, int param2, int wordmax, const STRBUF *default_str )
{ 
  NAMEIN_PARAM *p_param;
  p_param = GFL_HEAP_AllocMemory( heapId, sizeof(NAMEIN_PARAM) );
  GFL_STD_MemClear( p_param, sizeof(NAMEIN_PARAM) );
  p_param->mode     = mode;
  p_param->wordmax  = wordmax;
  p_param->param1 = param1;
  p_param->param2 = param2;

  //バッファ作成
  p_param->strbuf = GFL_STR_CreateBuffer( wordmax + 1, heapId );

  //デフォルト入力文字コピー
  if( default_str )
  { 
    GFL_STR_CopyBuffer( p_param->strbuf, default_str );
  }

  return p_param;
}
//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINに渡すPARAM構造体作成 ポケモンモード固定＆PP指定版
 *
 *  @param  HEAPID heapId         ヒープID
 *  @param  POKEMON_PARAM *pp     ポケモンパラム
 *  @param  wordmax               入力文字最大数
 *  @param  STRBUF *default_str   デフォルトで入力されている文字列
 *
 *  @return NAMEIN_PARAM
 */
//-----------------------------------------------------------------------------
NAMEIN_PARAM *NAMEIN_AllocParamPokemonByPP( HEAPID heapId, const POKEMON_PARAM *pp, int wordmax, const STRBUF *default_str )
{ 
  NAMEIN_PARAM *p_param;
  p_param = GFL_HEAP_AllocMemory( heapId, sizeof(NAMEIN_PARAM) );
  GFL_STD_MemClear( p_param, sizeof(NAMEIN_PARAM) );
  p_param->mode     = NAMEIN_POKEMON;
  p_param->wordmax  = wordmax;
  p_param->pp       = pp;

  //PPの場合引数を設定
  if( p_param->pp )
  { 
    p_param->mons_no  = PP_Get( pp, ID_PARA_monsno, NULL );
    p_param->form     = PP_Get( pp, ID_PARA_form_no, NULL );
  }

  //バッファ作成
  p_param->strbuf = GFL_STR_CreateBuffer( wordmax + 1, heapId );

  //デフォルト入力文字コピー
  if( default_str )
  { 
    GFL_STR_CopyBuffer( p_param->strbuf, default_str );
  }

  return p_param;
}

//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINに渡すPARAM構造体作成 ポケモンモード固定＆PP指定版
 *          ポケモン捕獲時はこちらを使用してください
 *
 *  @param  HEAPID heapId         ヒープID
 *  @param  POKEMON_PARAM *pp     ポケモンパラム
 *  @param  wordmax               入力文字最大数
 *  @param  STRBUF *default_str   デフォルトで入力されている文字列
 *  @param  box_strbuf   [ポケモンニックネーム]は○○のパソコンの[ボックス名]に転送された！
 *  @param  box_manager  ボックスマネージャ(box_strbuf!=NULLのときしか使われない) 
 *  @param  box_tray     ボックストレイナンバー(box_strbuf!=NULLのときしか使われない) 
 *
 *  @return NAMEIN_PARAM
 */
//-----------------------------------------------------------------------------
NAMEIN_PARAM *NAMEIN_AllocParamPokemonCapture( HEAPID heapId, const POKEMON_PARAM *pp, int wordmax, const STRBUF *default_str,
                                               const STRBUF *box_strbuf, const BOX_MANAGER *box_manager, u32 box_tray )
{
  NAMEIN_PARAM *p_param = NAMEIN_AllocParamPokemonByPP( heapId, pp, wordmax, default_str );
  p_param->box_strbuf = box_strbuf;
  p_param->box_manager = box_manager;
  p_param->box_tray = box_tray;
  return p_param;
}

//----------------------------------------------------------------------------
/**
 *  @brief  NAMEIN_PARAMを破棄
 *
 *  @param  *param  ワーク
 */
//-----------------------------------------------------------------------------
void NAMEIN_FreeParam(NAMEIN_PARAM *param)
{ 
  GFL_STR_DeleteBuffer( param->strbuf );
  GFL_HEAP_FreeMemory( param );
}
//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINPARAMから、入力された文字をコピー
 *
 *  @param  const NAMEIN_PARAM *param   ワーク
 *  @param  *strbuf   コピー受け取りバッファ
 */
//-----------------------------------------------------------------------------
void NAMEIN_CopyStr( const NAMEIN_PARAM *param, STRBUF *strbuf )
{ 
  GFL_STR_CopyBuffer( strbuf, param->strbuf );
}
//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINPARAMから、入力された文字をAllocかつコピーして返す
 *
 *  @param  const NAMEIN_PARAM *param ワーク
 *  @param  heapID                    ヒープID
 *
 *  @return コピーされた文字列
 */
//-----------------------------------------------------------------------------
STRBUF* NAMEIN_CreateCopyStr( const NAMEIN_PARAM *param, HEAPID heapID )
{ 
  return GFL_STR_CreateCopyBuffer( param->strbuf, heapID );
}
//----------------------------------------------------------------------------
/**
 *  @brief  NAMEINPARAMから、キャンセルされたかのフラグを取得
 *
 *  @param  const NAMEIN_PARAM *param   ワーク
 *
 *  @retval TRUEならば0文字でOKをおされたorデフォルト文字と同じ
 *  @retval FALSEならばキャンセルされていない
 */
//-----------------------------------------------------------------------------
BOOL NAMEIN_IsCancel( const NAMEIN_PARAM *param )
{ 
  return param->cancel;
}

//=============================================================================
/**
 *          proc
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  名前入力  メインプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT NAMEIN_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  NAMEIN_WORK   *p_wk;
  NAMEIN_PARAM  *p_param;
  NAMEIN_INPUTTYPE  mode;

  u32 heap_size;
  GFL_FONT_LOADTYPE font_load;
  GflMsgLoadType    msg_load;

  //引数受け取り
  p_param = p_param_adrs;

  //セーブデータ受け取り
  { 
    const MISC *cp_misc;
    cp_misc = SaveData_GetMisc( SaveControl_GetPointer() );
    mode    = MISC_GetNameInMode( cp_misc, p_param->mode );
  }
  
  if( 1 )
  { 
    heap_size = NAMEIN_HEAP_HIGH_SIZE;
    font_load = GFL_FONT_LOADTYPE_MEMORY;
    msg_load  = GFL_MSG_LOAD_FAST;
  }
  else
  { 
    heap_size = NAMEIN_HEAP_LOW_SIZE;
    font_load = GFL_FONT_LOADTYPE_FILE;
    msg_load  = GFL_MSG_LOAD_NORMAL;
  }


  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_NAME_INPUT, heap_size );

  //プロセスワーク作成
  p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(NAMEIN_WORK), HEAPID_NAME_INPUT );
  GFL_STD_MemClear( p_wk, sizeof(NAMEIN_WORK) );

  //初期化
  p_wk->p_param = p_param;

  //共通モジュールの作成
  p_wk->p_font    = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
      font_load, FALSE, HEAPID_NAME_INPUT );
  p_wk->p_que     = PRINTSYS_QUE_Create( HEAPID_NAME_INPUT );
  p_wk->p_msg   = GFL_MSG_Create( msg_load, ARCID_MESSAGE, 
                        NARC_message_namein_dat, HEAPID_NAME_INPUT );
  p_wk->p_word  = WORDSET_Create( HEAPID_NAME_INPUT );

  // イントロからセーブデータを引渡された場合
  if( p_wk->p_param->p_intr_sv )
  { 
    //p_wk->p_intr_sv = IntrSave_Init(HEAPID_NAME_INPUT, SaveControl_GetPointer());
  }


  //グラフィック設定
  p_wk->p_graphic = NAMEIN_GRAPHIC_Init( 0, HEAPID_NAME_INPUT );

  //読みこみ
  BG_Init( &p_wk->bg, HEAPID_NAME_INPUT );
  { 
    GFL_CLUNIT  *p_clunit = NAMEIN_GRAPHIC_GetClunit( p_wk->p_graphic );
    OBJ_Init( &p_wk->obj, p_clunit, HEAPID_NAME_INPUT );
    ICON_Init( &p_wk->icon, p_param->mode, p_param->param1, p_param->param2, p_clunit, HEAPID_NAME_INPUT );
  }
  p_wk->p_keymap_handle = NAMEIN_KEYMAP_HANDLE_Alloc( HEAPID_NAME_INPUT );

  //モジュール作成
  SEQ_Init( &p_wk->seq, p_wk, SEQFUNC_WaitPrint );
  STRINPUT_Init( &p_wk->strinput, p_param->strbuf, p_param->wordmax, p_wk->p_font, p_wk->p_que, &p_wk->obj, HEAPID_NAME_INPUT );
  KEYBOARD_Init( &p_wk->keyboard, mode, p_wk->p_font, p_wk->p_que, p_wk->p_keymap_handle, HEAPID_NAME_INPUT );
  MSGWND_Init( &p_wk->msgwnd, p_wk->p_font, p_wk->p_msg, p_wk->p_que, p_wk->p_word, HEAPID_NAME_INPUT );
  PS_Init( &p_wk->ps, HEAPID_NAME_INPUT );
  //文字描画
  if( p_param->mode == NAMEIN_POKEMON )
  { 
    //ポケモンの場合は、種族名を単語登録
    if( p_param->pp == NULL )
    { 
      MSGWND_ExpandPrintPoke( &p_wk->msgwnd, NAMEIN_MSG_INFO_000 + p_param->mode, p_param->param1, p_param->param2, HEAPID_NAME_INPUT );
    }
    else
    { 
      MSGWND_ExpandPrintPP( &p_wk->msgwnd, NAMEIN_MSG_INFO_000 + p_param->mode, p_param->pp );
    }
  }
  else
  { 
    //他は通常
    MSGWND_Print( &p_wk->msgwnd, NAMEIN_MSG_INFO_000 + p_param->mode );
  }


  if( p_wk->p_param->p_intr_sv )
  { 
    IntrSave_Resume( p_wk->p_param->p_intr_sv );
  }

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  名前入力  メインプロセス破棄
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT NAMEIN_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  NAMEIN_WORK *p_wk = p_wk_adrs;

  //モジュール破棄
  PS_Exit( &p_wk->ps );
  MSGWND_Exit( &p_wk->msgwnd );
  KEYBOARD_Exit( &p_wk->keyboard );
  NAMEIN_KEYMAP_HANDLE_Free( p_wk->p_keymap_handle );
  STRINPUT_Exit( &p_wk->strinput );
  SEQ_Exit( &p_wk->seq );

  //グラフィック破棄
  ICON_Exit( &p_wk->icon );
  OBJ_Exit( &p_wk->obj );
  BG_Exit( &p_wk->bg );
  NAMEIN_GRAPHIC_Exit( p_wk->p_graphic );

  //共通モジュールの破棄
  WORDSET_Delete( p_wk->p_word );
  GFL_MSG_Delete( p_wk->p_msg );
  PRINTSYS_QUE_Delete( p_wk->p_que );
  GFL_FONT_Delete( p_wk->p_font );

  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );
  //ヒープ破棄
  GFL_HEAP_DeleteHeap( HEAPID_NAME_INPUT );

  
  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  名前入力  メインプロセスメイン処理
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT NAMEIN_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{ 
  NAMEIN_WORK *p_wk = p_wk_adrs;

  //シーケンス
  SEQ_Main( &p_wk->seq );

  //描画
  NAMEIN_GRAPHIC_2D_Draw( p_wk->p_graphic );

  //プリント
  PRINTSYS_QUE_Main( p_wk->p_que );

  //セーブ
  if( p_wk->p_param->p_intr_sv )
  { 
    //外部でよんでいる
    //IntrSave_Main( p_wk->p_intr_sv );
  }

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
 *    BGリソース読みこみ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  BGリソース読みこみ
 *
 *  @param  BG_WORK *p_wk ワーク
 *  @param  heapID        ヒープID
 */
//-----------------------------------------------------------------------------
static void BG_Init( BG_WORK *p_wk, HEAPID heapID )
{ 
  { 
    ARCHANDLE * p_handle  = GFL_ARC_OpenDataHandle( ARCID_NAMEIN_GRA, heapID );

    //PLT
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_namein_gra_name_bg_NCLR,
        PALTYPE_MAIN_BG, PLT_BG_STR_M*0x20, 0, heapID );
    GFL_ARCHDL_UTIL_TransVramPaletteEx( p_handle, NARC_namein_gra_name_bg_NCLR,
        PALTYPE_MAIN_BG, PLT_BG_KEY_MOVE_M*0x20, PLT_BG_KEY_DECIDE_M*0x20, 0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramPaletteEx( p_handle, NARC_namein_gra_name_bg_NCLR,
        PALTYPE_MAIN_BG, PLT_BG_KEY_MOVE_M*0x20, PLT_BG_KEY_PRESS_M*0x20, 0x20, heapID );

    //CHR
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_namein_gra_name_bg_NCGR, 
        BG_FRAME_STR_M, 0, 0, FALSE, heapID );

    //SCR
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_namein_gra_name_base_NSCR,
        BG_FRAME_STR_M, 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_namein_gra_name_btn_NSCR,
        BG_FRAME_BTN_M, 0, 0, FALSE, heapID );
  
    GFL_ARC_CloseDataHandle( p_handle );
  }

  { 
    ARCHANDLE * p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, heapID );


    //上画面フォントパレット
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr,
        PALTYPE_SUB_BG, PLT_BG_FONT_S*0x20, 0x20, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }

  { 
    GFL_BG_FillCharacter( BG_FRAME_TEXT_S, 0, 1,  0 );
    BmpWinFrame_GraphicSet(
        BG_FRAME_TEXT_S, MSGWND_FRAME_CHR, PLT_BG_FRAME_S, MENU_TYPE_SYSTEM, heapID );
  }

  GFL_BG_SetBackGroundColor( BG_FRAME_TEXT_S, 0 );
}
//----------------------------------------------------------------------------
/**
 *  @brief  BGリソース破棄
 *
 *  @param  BG_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BG_Exit( BG_WORK *p_wk )
{ 
  GFL_BG_FillCharacterRelease( BG_FRAME_TEXT_S, 1, 0 );
}
//----------------------------------------------------------------------------
/**
 *  @brief  BGパレットフェード
 *
 *  @param  u16 *p_buff 色保存バッファ
 *  @param  *p_cnt      カウンタバッファ
 *  @param  add         カウンタ加算値
 *  @param  plt_num     パレット縦番号
 *  @param  plt_col     パレット横番号
 *  @param  start       開始色
 *  @param  end         終了色
 */
//-----------------------------------------------------------------------------
static void BG_MainPltAnm( u16 *p_buff, u16 *p_cnt, u16 add, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end )
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
    
    NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_MAIN,
                                        plt_num * 32 + plt_col * 2,
                                        p_buff, 2 );
  }
}
//=============================================================================
/**
 *    OBJリソース読みこみ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  OBJリソース読みこみ
 *
 *  @param  OBJ_WORK *p_wk  ワーク
 *  @param  clunit          ユニット
 *  @param  HEAPID heapID   ヒープID
 */
//-----------------------------------------------------------------------------
static void OBJ_Init( OBJ_WORK *p_wk, GFL_CLUNIT *p_clunit, HEAPID heapID )
{ 
  //リソース読みこみ
  { 
    ARCHANDLE * p_handle  = GFL_ARC_OpenDataHandle( ARCID_NAMEIN_GRA, heapID );

    p_wk->res[ RESID_OBJ_COMMON_PLT ] = GFL_CLGRP_PLTT_Register( p_handle, 
        NARC_namein_gra_name_obj_NCLR, CLSYS_DRAW_MAIN, PLT_OBJ_CURSOR_M*0x20, heapID );
    p_wk->res[ RESID_OBJ_COMMON_CEL ] = GFL_CLGRP_CELLANIM_Register( p_handle,
        NARC_namein_gra_name_obj_NCER, NARC_namein_gra_name_obj_NANR, heapID );
    p_wk->res[ RESID_OBJ_COMMON_CHR ] = GFL_CLGRP_CGR_Register( p_handle,
        NARC_namein_gra_name_obj_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }
  //CLWK登録
  {
    int i;
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
    cldata.anmseq = 0;
    for( i = CLWKID_BAR_TOP; i < CLWKID_BAR_END; i++ )
    { 
      p_wk->p_clwk[i] =   GFL_CLACT_WK_Create( p_clunit,
          p_wk->res[RESID_OBJ_COMMON_CHR],
          p_wk->res[RESID_OBJ_COMMON_PLT],
          p_wk->res[RESID_OBJ_COMMON_CEL],
          &cldata,
          CLSYS_DEFREND_MAIN,
          heapID );
    }

    cldata.anmseq = 1;
    p_wk->p_clwk[CLWKID_PC] =   GFL_CLACT_WK_Create( p_clunit,
        p_wk->res[RESID_OBJ_COMMON_CHR],
        p_wk->res[RESID_OBJ_COMMON_PLT],
        p_wk->res[RESID_OBJ_COMMON_CEL],
        &cldata,
        CLSYS_DEFREND_MAIN,
        heapID );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CLWKID_PC], FALSE );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  OBJリソース破棄
 *
 *  @param  BG_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void OBJ_Exit( OBJ_WORK *p_wk )
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
    GFL_CLGRP_PLTT_Release( p_wk->res[RESID_OBJ_COMMON_PLT] );
    GFL_CLGRP_CGR_Release( p_wk->res[RESID_OBJ_COMMON_CHR] );
    GFL_CLGRP_CELLANIM_Release( p_wk->res[RESID_OBJ_COMMON_CEL] );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  CLWK取得
 *
 *  @param  const OBJ_WORK *p_wk    ワーク
 *  @param  clwkID                  CLWK番号
 *
 *  @return CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK *OBJ_GetClwk( const OBJ_WORK *cp_wk, CLWKID clwkID )
{ 
  return cp_wk->p_clwk[ clwkID ];
}
//=============================================================================
/**
 *          KEYANM
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  キーアニメ  初期化
 *
 *  @param  KEYANM_WORK *p_wk ワーク
 *  @param  mode              モード
 *  @param  heapID            ヒープID
 */
//-----------------------------------------------------------------------------
static void KEYANM_Init( KEYANM_WORK *p_wk, NAMEIN_INPUTTYPE mode, HEAPID heapID )
{ 
  void *p_buff;
  NNSG2dPaletteData *p_plt;

  //クリア
  GFL_STD_MemClear( p_wk, sizeof(KEYANM_WORK) );
  p_wk->mode  = mode;

  //もとのパレットから色情報を保存
  p_buff  = GFL_ARC_UTIL_LoadPalette( ARCID_NAMEIN_GRA, NARC_namein_gra_name_bg_NCLR, &p_plt, heapID );

  { 
    int i;
    const u16 *cp_plt_adrs;

    cp_plt_adrs = p_plt->pRawData;
    //１ライン目
    for( i = 0; i < 0x10; i++ )
    { 
      p_wk->plt_normal[i] = cp_plt_adrs[i+0x10];
    }
    //２ライン目
    for( i = 0; i < 0x10; i++ )
    { 
      p_wk->plt_flash[i]  = cp_plt_adrs[i+0x20];
    }
  }

  //パレット破棄
  GFL_HEAP_FreeMemory( p_buff );

}
//----------------------------------------------------------------------------
/**
 *  @brief  キーアニメ  破棄
 *
 *  @param  KEYANM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KEYANM_Exit( KEYANM_WORK *p_wk )
{ 
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(KEYANM_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーアニメ  開始
 *
 *  @param  KEYANM_WORK *p_wk   ワーク
 *  @param  type                移動タイプ
 *  @param  GFL_RECT *cp_rect   矩形
 *  @param  is_shift            シフトを押しているか
 *  @param  mode                モード
 */
//-----------------------------------------------------------------------------
static void KEYANM_Start( KEYANM_WORK *p_wk, KEYANM_TYPE type, const GFL_RECT *cp_rect, BOOL is_shift, NAMEIN_INPUTTYPE mode )
{ 
  p_wk->is_start  = FALSE;
  p_wk->type      = type;
  p_wk->is_shift  = is_shift;
  p_wk->mode      = mode;

  //他のキーを全て元に戻す
  GFL_BG_ChangeScreenPalette( BG_FRAME_BTN_M, 0, 0, 32, 24, PLT_BG_KEY_NORMAL_M );
  GFL_BG_ChangeScreenPalette( BG_FRAME_KEY_M, 0, 0, 32, 24, PLT_BG_KEY_NORMAL_M );

  //SHIFT押していれば、そこを色変え
  if( is_shift && mode == NAMEIN_INPUTTYPE_QWERTY )
  { 
    GFL_BG_ChangeScreenPalette( BG_FRAME_KEY_M, 3, 16,
      4, 3, PLT_BG_KEY_PRESS_M );
    GFL_BG_ChangeScreenPalette( BG_FRAME_KEY_M, 23, 16,
      4, 3, PLT_BG_KEY_PRESS_M );
  }

  //モードのボタンを色変え
  { 
    GFL_BG_ChangeScreenPalette( BG_FRAME_KEY_M, 3 + mode * 2, 20,
      2, 3, PLT_BG_KEY_PRESS_M );
    GFL_BG_ChangeScreenPalette( BG_FRAME_BTN_M, 3 + mode * 2, 20,
      2, 3, PLT_BG_KEY_PRESS_M );
  }

  //キーの色を張替え
  //押している上にいたときわからないので、
  //現在のキーの色変えを優先し後に
  if( cp_rect && type != KEYANM_TYPE_NONE )
  { 
    p_wk->range = *cp_rect;
    
    GFL_BG_ChangeScreenPalette( BG_FRAME_BTN_M, cp_rect->left, cp_rect->top,
        cp_rect->right - cp_rect->left, cp_rect->bottom - cp_rect->top,
        PLT_BG_KEY_NORMAL_M + type );
    GFL_BG_ChangeScreenPalette( BG_FRAME_KEY_M, cp_rect->left, cp_rect->top,
        cp_rect->right - cp_rect->left, cp_rect->bottom - cp_rect->top,
        PLT_BG_KEY_NORMAL_M + type );
  }

  //スクリーンリクエスト
  GFL_BG_LoadScreenReq( BG_FRAME_BTN_M );
  GFL_BG_LoadScreenReq( BG_FRAME_KEY_M );


  if( type != KEYANM_TYPE_NONE )
  { 
    //カウントバッファクリア
    GFL_STD_MemClear( p_wk->cnt, sizeof(u16)*0x10 );
    p_wk->decide_cnt  = 0;

    //アニメ開始
    p_wk->is_start  = TRUE;
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーアニメ  メイン処理
 *
 *  @param  KEYANM_WORK *p_wk ワーク
 *  @param  NAMEIN_INPUTTYPE  現在のモード
 *  @param  RECT  自分の最新のカーソルがいる矩形
 */
//-----------------------------------------------------------------------------
static void KEYANM_Main( KEYANM_WORK *p_wk, NAMEIN_INPUTTYPE mode, const GFL_RECT *cp_rect )
{ 
  u8 plt_num;
  u16 add;
  int i;
  BOOL is_end;

  //モード別おす処理
  if( p_wk->is_start )
  { 
    //モード別処理
    switch( p_wk->type )
    { 
    case KEYANM_TYPE_MOVE:  
      plt_num = PLT_BG_KEY_MOVE_M;
      add     = 0x400;
      break;

    case KEYANM_TYPE_DECIDE:
      plt_num = PLT_BG_KEY_DECIDE_M;
      add     = 0xFFFF/10;
      //終了したら自分でモード切替
      if( p_wk->decide_cnt++ > KEYANM_DECIDE_CNT )
      { 
        //キーモードがキーならばカーソル位置を押す
        if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_KEY )
        { 
          //モードボタン+カーソル位置押下のため
          KEYANM_Start( p_wk, KEYANM_TYPE_MOVE, cp_rect, p_wk->is_shift, mode );
        }
        else
        { 
          //モードボタン押下のため
          KEYANM_Start( p_wk, KEYANM_TYPE_NONE, cp_rect, p_wk->is_shift, mode );
        }
      }
      break;
    case KEYANM_TYPE_NONE:
      GF_ASSERT(0);
      break;
    }

    //それぞれのパレットをアニメ
    for( i = 0; i < 0x10; i++ )
    { 
      KeyAnm_PltFade( &p_wk->color[i], &p_wk->cnt[i], add, plt_num, i, p_wk->plt_normal[i], p_wk->plt_flash[i] );
    }
  }

  //モード切替時のリフレッシュ
  if( p_wk->mode != mode )
  { 
    //キーモードがキーならばカーソル位置を押す
    if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_KEY )
    { 
      //モードボタン+カーソル位置押下のため
      KEYANM_Start( p_wk, KEYANM_TYPE_MOVE, &p_wk->range, p_wk->is_shift, mode );
    }
    else
    { 
      //モードボタンのため
      KEYANM_Start( p_wk, KEYANM_TYPE_NONE, &p_wk->range, p_wk->is_shift, mode );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーアニメ  リセット
 *
 *  @param  KEYANM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KEYANM_Reset( KEYANM_WORK *p_wk )
{ 

}
//----------------------------------------------------------------------------
/**
 *  @brief  キーアニメ  フェード
 *
 *  @param  u16 *p_buff 色保存バッファ（VBlank転送のため）
 *  @param  *p_cnt      カウンタバッファ
 *  @param  add         カウンタ加算値
 *  @param  plt_num     パレット縦番号
 *  @param  plt_col     パレット横番号
 *  @param  start       開始色
 *  @param  end         終了色
 *
 *  @retval TRUEでENDになった。　FALSEでまだ  （ENDで止まらず、ループします）
 */
//-----------------------------------------------------------------------------
static BOOL KeyAnm_PltFade( u16 *p_buff, u16 *p_cnt, u16 add, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end )
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
    BOOL ret;
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
    
    ret = NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_MAIN ,
                                        plt_num * 32 + plt_col *2 ,
                                        p_buff, 2 );

    GF_ASSERT( ret );
  }

  return *p_buff == end;
}
//=============================================================================
/**
 *          STRINPUT
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  初期化
 *
 *  @param  STRINPUT_WORK *p_wk   ワーク
 *  @param  default_str           デフォルト文字列
 *  @param  strlen                文字列長
 *  @param  p_font                フォント
 *  @param  p_que                 プリントキュー
 *  @param  cp_obj                clwk取得用OBJワーク
 *  @param  heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void STRINPUT_Init( STRINPUT_WORK *p_wk, const STRBUF * cp_default_str, u8 strlen, GFL_FONT *p_font, PRINT_QUE *p_que, const OBJ_WORK *cp_obj, HEAPID heapID )
{ 
  GF_ASSERT( strlen <= STRINPUT_STR_LEN );

  //クリア
  GFL_STD_MemClear( p_wk, sizeof(STRINPUT_WORK) );
  p_wk->p_font  = p_font;
  p_wk->strlen  = strlen;
  p_wk->p_que   = p_que;

  //CLWK取得
  {  
    int i;
    for( i = 0; i < OBJ_BAR_NUM; i++ )
    { 
      p_wk->p_clwk[ i ] = OBJ_GetClwk( cp_obj, CLWKID_BAR_TOP + i );
    }
  }

  if( GFL_STR_GetBufferLength( cp_default_str ) == 0  )
  { 
    p_wk->input_str[0]  = GFL_STR_GetEOMCode();
  }
  else
  { 
    GFL_STR_GetStringCode( cp_default_str, p_wk->input_str, strlen + 1 );
    p_wk->input_idx = GFL_STR_GetBufferLength( cp_default_str );
  }

  //BMPWIN作成
  p_wk->p_bmpwin  = GFL_BMPWIN_Create( BG_FRAME_STR_M, STRINPUT_BMPWIN_X, STRINPUT_BMPWIN_Y, STRINPUT_BMPWIN_W, STRINPUT_BMPWIN_H, PLT_BG_FONT_M, GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );

  //プリントキュー設定
  PRINT_UTIL_Setup( &p_wk->util, p_wk->p_bmpwin );

  //文字バッファ
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( STRINPUT_STR_LEN + STRINPUT_CHANGE_LEN + 1, heapID );

  //検索用データ読みこみ
  { 
    int i;
    for( i = 0; i < NAMEIN_STRCHANGETYPE_MAX; i++ )
    { 
      p_wk->p_changedata[i] = NAMEIN_STRCHANGE_Alloc( i, heapID );
    }
  }

  //CLWK設定
  {  
    int i;
    u16 x;
    GFL_CLACTPOS  clpos;
    clpos.y = STRINPUT_BAR_Y;
    x = STRINPUT_BMPWIN_W * 8 / 2 - (p_wk->strlen * STRINPUT_STR_OFFSET_X) /2;  //開始
    for( i = 0; i < OBJ_BAR_NUM; i++ )
    { 
      clpos.x = STRINPUT_BAR_START_X + x + i * STRINPUT_BAR_OFFSET_X;

      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i], 1 );
      GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[i], TRUE );
      GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, CLSYS_DEFREND_MAIN );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
    }
    for( i = p_wk->strlen; i < OBJ_BAR_NUM; i++ )
    { 
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
    }

    GFL_CLACT_WK_StartAnm( p_wk->p_clwk[ p_wk->input_idx ] );
  }


  //文字描画するために1回UPDATE
  p_wk->is_update = TRUE;
  STRINPUT_Main( p_wk );
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  破棄
 *
 *  @param  STRINPUT_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void STRINPUT_Exit( STRINPUT_WORK *p_wk )
{ 
  //検索用データ破棄
  { 
    int i;
    for( i = 0; i < NAMEIN_STRCHANGETYPE_MAX; i++ )
    { 
      NAMEIN_STRCHANGE_Free( p_wk->p_changedata[i] );
    }
  }

  //文字バッファ破棄
  GFL_STR_DeleteBuffer( p_wk->p_strbuf );
  
  //BMPWIN破棄
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );

  //クリア
  GFL_STD_MemClear( p_wk, sizeof(STRINPUT_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  メイン処理
 *
 *  @param  STRINPUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void STRINPUT_Main( STRINPUT_WORK *p_wk )
{ 
  if( p_wk->is_update )
  { 
    STRCODE buff[ STRINPUT_STR_LEN + STRINPUT_CHANGE_LEN + 1 ];

    //確定文字列があれば連結
    if( p_wk->input_idx > 0 )
    { 
      STRTOOL_Copy( p_wk->input_str, buff, p_wk->input_idx + 1 );
    }
    //変換文字列があれば、後尾に連結
    if( p_wk->change_idx > 0 )
    { 
      STRTOOL_Copy( p_wk->change_str, &buff[p_wk->input_idx], p_wk->change_idx + 1 );
    }

    //連結した文字数があれば、文字描画
    //なければクリア
    if( p_wk->input_idx + p_wk->change_idx > 0 )
    { 
      int i;
      STRCODE code[2];
      u16 x;
  
      //一端クリア
      GFL_BMP_Clear(GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );

      //等幅にするために1文字ずつプリント
      for( i = 0; i < p_wk->input_idx + p_wk->change_idx; i++ )
      { 
        code[0] = buff[i];
        code[1] = GFL_STR_GetEOMCode();


        GFL_STR_SetStringCodeOrderLength( p_wk->p_strbuf, code, 2 );
        x = STRINPUT_BMPWIN_W * 8 / 2 - (p_wk->strlen * STRINPUT_STR_OFFSET_X) /2;  //開始
        x += i * STRINPUT_STR_OFFSET_X + STRINPUT_BAR_OFFSET_X/2;               //幅
        x -= PRINTSYS_GetStrWidth( p_wk->p_strbuf, p_wk->p_font, 0 )/2;

        //変換文字入力中はフォントの色を変える
        if( i >= p_wk->input_idx )
        { 
          PRINT_UTIL_PrintColor( &p_wk->util, p_wk->p_que, x, STRINPUT_STR_Y, p_wk->p_strbuf, p_wk->p_font, PRINTSYS_LSB_Make(3,4,0) );
        }
        else
        { 
          PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, x, STRINPUT_STR_Y, p_wk->p_strbuf, p_wk->p_font );
        }
      }
    }
    else
    { 
      GFL_BMP_Clear(GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );
      GFL_BMPWIN_TransVramCharacter(p_wk->p_bmpwin);
    }

    //バーのアニメを変更
    { 
      int i;
      for( i = 0; i < p_wk->strlen; i++ )
      { 
        GFL_CLACT_WK_SetAnmFrame( p_wk->p_clwk[i], 0 );
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[ i ], 1 );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
      for( ; i < OBJ_BAR_NUM; i++ )
      { 
        GFL_CLACT_WK_SetAnmFrame( p_wk->p_clwk[i], 0 );
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[ i ], 1 );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
      }
      for(i = 0; i <= p_wk->input_idx && i < p_wk->strlen; i++ )
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[ p_wk->input_idx ], 0 );
    }

    p_wk->is_update = FALSE;
  }
  
  STRINPUT_PrintMain( p_wk );
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  1文字消去
 *
 *  @param  STRINPUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void STRINPUT_BackSpace( STRINPUT_WORK *p_wk )
{ 
  //変換中ならば、変換中の文字を
  //そうでないならば、普通に消去
  if( p_wk->change_idx > 0 )
  { 
    p_wk->change_idx--;
    p_wk->change_str[ p_wk->change_idx ] = GFL_STR_GetEOMCode();
    p_wk->is_update = TRUE;

    PMSND_PlaySE( NAMEIN_SE_DELETE_STR );
  }
  else
  { 
    if( p_wk->input_idx > 0 )
    { 
      p_wk->input_idx--;
      p_wk->input_str[ p_wk->input_idx ] = GFL_STR_GetEOMCode();
      p_wk->is_update = TRUE;

      PMSND_PlaySE( NAMEIN_SE_DELETE_STR );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  文字を設定
 *
 *  @param  STRINPUT_WORK *p_wk ワーク
 *  @param  STRCODE code
 */
//-----------------------------------------------------------------------------
static BOOL STRINPUT_SetStr( STRINPUT_WORK *p_wk, STRCODE code )
{ 
  //入力できるか
  if( p_wk->input_idx < p_wk->strlen )
  { 
    p_wk->input_str[ p_wk->input_idx ] = code;
    p_wk->input_idx++;
    p_wk->input_str[ p_wk->input_idx ] = GFL_STR_GetEOMCode();

    OS_Printf( "\nSetStr\n " );
    DEBUG_NAMEIN_Print(p_wk->input_str,p_wk->input_idx);

    PMSND_PlaySE( NAMEIN_SE_DECIDE_STR );

    p_wk->is_update = TRUE;

    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  変換文字を設定
 *
 *  @param  STRINPUT_WORK *p_wk ワーク
 *  @param  code                変換文字列にいれる文字
 *  @param  is_shift            シフトが入力されているかどうか
 *
 *  @retval TRUE　文字を変換した　FALSEしていない
 */
//-----------------------------------------------------------------------------
static BOOL STRINPUT_SetChangeStr( STRINPUT_WORK *p_wk, STRCODE code, BOOL is_shift )
{ 
  //入力できるか
  if( p_wk->input_idx < p_wk->strlen
      && p_wk->change_idx < STRINPUT_CHANGE_LEN )
  { 
    p_wk->change_str[ p_wk->change_idx ] = code;
    p_wk->change_idx++;
    p_wk->change_str[ p_wk->change_idx ] = GFL_STR_GetEOMCode();

    //変換バッファから確定バッファへの変換処理
    StrInput_ChangeStrToStr( p_wk, is_shift );

    PMSND_PlaySE( NAMEIN_SE_DECIDE_STR );
    
    p_wk->is_update = TRUE;
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  濁点、半濁点を変換する
 *
 *  @param  STRINPUT_WORK *p_wk ワーク
 *
 *  @retval TRUE　文字を変換した　FALSEしていない
 */
//-----------------------------------------------------------------------------
static BOOL STRINPUT_SetChangeSP( STRINPUT_WORK *p_wk, STRINPUT_SP_CHANGE type )
{ 
  NAMEIN_STRCHANGE  *p_data;

  switch( type )
  { 
  case STRINPUT_SP_CHANGE_DAKUTEN:      //濁点
    //検索文字サーチ
    p_data  =  p_wk->p_changedata[NAMEIN_STRCHANGETYPE_DAKUTEN];
    break;
  case STRINPUT_SP_CHANGE_HANDAKUTEN: //半濁点
    //検索文字サーチ
    p_data  =  p_wk->p_changedata[NAMEIN_STRCHANGETYPE_HANDAKUTEN];
    break;
  default:
    GF_ASSERT(0);
    return FALSE;
  }

  //確定文字列の一番後ろを見て、変換可能だったら、変える
  if( 0 < p_wk->input_idx )
  { 
    u16 idx;
    STRCODE code[NAMEIN_STRCHANGE_CODE_LEN+1];
    u8 len;
    STRCODE delete[NAMEIN_STRCHANGE_CODE_LEN+1];
    u8 delete_len;
    int i;

    idx = NAMEIN_STRCHANGE_GetIndexByInputStr( p_data, &p_wk->input_str[ p_wk->input_idx-1 ] ); 
    if( idx != NAMEIN_STRCHANGE_NONE )
    { 
      if( NAMEIN_STRCHANGE_GetChangeStr( p_data, idx, code, NAMEIN_STRCHANGE_CODE_LEN+1, &len ) )
      { 
        //確定バッファから削除
        NAMEIN_STRCHANGE_GetInputStr( p_data, idx, delete, NAMEIN_STRCHANGE_CODE_LEN+1, &delete_len );
        //確定バッファに追加
        for( i = 0; i < delete_len; i++ )
        { 
          STRINPUT_BackSpace( p_wk );
        }

        //確定バッファに追加
        for( i = 0; i < len; i++ )
        { 
          STRINPUT_SetStr( p_wk, code[i] );
        }

        PMSND_PlaySE( NAMEIN_SE_DECIDE_STR );
        return TRUE;
      }
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  描画メイン
 *
 *  @param  STRINPUT_WORK *p_wk   ワーク
 *
 * @retval  BOOL  転送が終わっている場合はTRUE／終わっていない場合はFALSE
 */
//-----------------------------------------------------------------------------
static BOOL STRINPUT_PrintMain( STRINPUT_WORK *p_wk )
{ 
  return PRINT_UTIL_Trans( &p_wk->util, p_wk->p_que );
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  入力した文字列をコピー
 *
 *  @param  const STRINPUT_WORK *cp_wk  ワーク
 *  @param  *p_strbuf                   STRBUF
 */
//-----------------------------------------------------------------------------
static void STRINPUT_CopyStr( const STRINPUT_WORK *cp_wk, STRBUF *p_strbuf )
{ 
  GFL_STR_SetStringCodeOrderLength( p_strbuf, cp_wk->input_str, cp_wk->input_idx + 1 );
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  変換文字列を消去
 *
 *  @param  STRINPUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void STRINPUT_DeleteChangeStr( STRINPUT_WORK *p_wk )
{ 
  if( p_wk->change_idx > 0 )
  { 
    p_wk->change_idx  = 0;
    p_wk->change_str[ p_wk->change_idx ] = GFL_STR_GetEOMCode();
    p_wk->is_update = TRUE;
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  変換文字列を確定
 *
 *  @param  STRINPUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void STRINPUT_DecideChangeStr( STRINPUT_WORK *p_wk )
{
  int i;
  for( i = 0; i < p_wk->change_idx; i++ )
  { 
    STRINPUT_SetStr( p_wk, p_wk->change_str[ i ] );
  }
  p_wk->change_idx  = 0;
  p_wk->change_str[ p_wk->change_idx ] = GFL_STR_GetEOMCode();
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  入力されている文字を全て消去
 *
 *  @param  STRINPUT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void STRINPUT_Delete( STRINPUT_WORK *p_wk )
{ 
  int i;
  if( p_wk->input_idx > 0 )
  { 
    p_wk->input_idx = 0;
    p_wk->input_str[ p_wk->input_idx ] = GFL_STR_GetEOMCode();
    p_wk->is_update = TRUE;
  } 
  STRINPUT_DeleteChangeStr( p_wk );
  
}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄
 *
 *  @param  STRINPUT_WORK *p_wk   ワーク
 *  @param  STRCODE *cp_code      文字列
 *
 */
//-----------------------------------------------------------------------------
static void STRINPUT_SetLongStr( STRINPUT_WORK *p_wk, const STRCODE *cp_code )
{ 
  while( *cp_code != GFL_STR_GetEOMCode() )
  { 
    if( !STRINPUT_SetStr( p_wk, *cp_code ))
    { 
      break;
    }
    cp_code++;
  }

}
//----------------------------------------------------------------------------
/**
 *  @brief  入力文字欄  入力数が限界かどうか
 *
 *  @param  const STRINPUT_WORK *cp_wk  ワーク
 *
 *  @return TRUEならば入力最大  FALSEならばまだ
 */
//-----------------------------------------------------------------------------
static BOOL STRINPUT_IsInputEnd( const STRINPUT_WORK *cp_wk )
{ 
  return cp_wk->input_idx == cp_wk->strlen;
}
//----------------------------------------------------------------------------
/**
 *  @brief  確定入力文字数を返す
 *
 *  @param  const STRINPUT_WORK *cp_wk ワーク
 *
 *  @return 確定入力文字数
 */
//-----------------------------------------------------------------------------
static u32  STRINPUT_GetStrLength( const STRINPUT_WORK *cp_wk )
{ 
  return cp_wk->input_idx;
}
//----------------------------------------------------------------------------
/**
 *  @brief  変換文字列から確定文字列への変換
 *
 *  @param  STRINPUT_WORK *p_wk   ワーク
 *  @param  is_shift              シフトが入力されているかどうか
 *
 *  @return TRUEで変換した  FALSEで変換しなかった
 */
//-----------------------------------------------------------------------------
static BOOL StrInput_ChangeStrToStr( STRINPUT_WORK *p_wk, BOOL is_shift )
{ 
    //変換処理
  int i,j;
  u16 idx;
  u8  len;
  STRCODE code[NAMEIN_STRCHANGE_CODE_LEN+1];

  BOOL (*GetChangeStr)( const NAMEIN_STRCHANGE *cp_wk, u16 idx, STRCODE *p_code, u8 code_len, u8 *p_len );
  
  //シフト入力によって関数が違う
  if( is_shift )
  { 
    GetChangeStr  = NAMEIN_STRCHANGE_GetChangeShiftStr;
  }
  else
  { 
    GetChangeStr  = NAMEIN_STRCHANGE_GetChangeStr;
  }


  for( i = 0; i < p_wk->change_idx; i++ )
  { 
    //検索文字サーチ
    idx = NAMEIN_STRCHANGE_GetIndexByInputStr( p_wk->p_changedata[NAMEIN_STRCHANGETYPE_QWERTY], &p_wk->change_str[ i ] ); 
    if( idx != NAMEIN_STRCHANGE_NONE )
    { 
      //変換してた文字列の前の文字も確定文字列にする
      NAGI_Printf( "変換せず確定する文字列 長さ%d\n", i );
      DEBUG_NAMEIN_Print(p_wk->change_str,i);
      for( j = 0; j < i; j++ )
      { 
        STRINPUT_SetStr( p_wk, p_wk->change_str[ j ] );
      } 
      for( j = 0; j < i; j++ )
      { 
        STRINPUT_BackSpace( p_wk );
      }

      //置換し確定バッファに送る
      if( GetChangeStr( p_wk->p_changedata[NAMEIN_STRCHANGETYPE_QWERTY], idx, code, NAMEIN_STRCHANGE_CODE_LEN+1, &len ) )
      { 
        NAGI_Printf( "変換後文字列 長さ%d\n", len );
        DEBUG_NAMEIN_Print(code,len);
        for( j = 0; j < len; j++ )
        { 
          STRINPUT_SetStr( p_wk, code[j] );
        }
        //置換したので、検索バッファから削除
        NAMEIN_STRCHANGE_GetInputStr( p_wk->p_changedata[NAMEIN_STRCHANGETYPE_QWERTY], idx, code, NAMEIN_STRCHANGE_CODE_LEN+1, &len );

        NAGI_Printf( "消す文字列 長さ%d\n", len );
        DEBUG_NAMEIN_Print(code,len);
        for( j = 0; j < len; j++ )
        { 
          STRINPUT_BackSpace( p_wk );
        }

        //残る文字を追加
        NAMEIN_STRCHANGE_GetRestStr( p_wk->p_changedata[NAMEIN_STRCHANGETYPE_QWERTY], idx, code, NAMEIN_STRCHANGE_CODE_LEN+1, &len ); 
        NAGI_Printf( "残る文字列 長さ%d\n", len );
        DEBUG_NAMEIN_Print(code,len);
        for( j = 0; j < len; j++ )
        { 
          p_wk->change_str[ p_wk->change_idx ] = code[j];
          p_wk->change_idx++; 
          p_wk->change_str[ p_wk->change_idx ] = GFL_STR_GetEOMCode();
        }

        return TRUE;
      }
    }
  }
  return FALSE;
}
//=============================================================================
/**
 *          KEYMAP
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  キー配列  作成
 *
 *  @param  KEYMAP_WORK *p_wk ワーク
 *  @param  mode              配列モード
 *  @param  heapID            ヒープID
 */
//-----------------------------------------------------------------------------
static void KEYMAP_Create( KEYMAP_WORK *p_wk, NAMEIN_INPUTTYPE mode, NAMEIN_KEYMAP_HANDLE *p_keymap_handle, HEAPID heapID )
{
  GF_ASSERT( mode < NAMEIN_INPUTTYPE_MAX );

  //クリア
  GFL_STD_MemClear( p_wk, sizeof(KEYMAP_WORK) );
  p_wk->p_key = NAMEIN_KEYMAP_HANDLE_GetData( p_keymap_handle, mode );

  switch( mode )
  {
  case NAMEIN_INPUTTYPE_QWERTY:
    p_wk->w     = 11;
    p_wk->h     = 4;
    p_wk->get_keyrect     = KeyMap_QWERTY_GetRect;
    p_wk->get_movecursor  = KeyMap_QWERTY_GetMoveCursor;
    p_wk->get_keytype     = KeyMap_QWERTY_GetKeyType;
    break;
  case NAMEIN_INPUTTYPE_KANA:
    /* fallthrough */
  case NAMEIN_INPUTTYPE_KATA:
    p_wk->w     = 13;
    p_wk->h     = 6;
    p_wk->get_keyrect     = KeyMap_KANA_GetRect;
    p_wk->get_movecursor  = KeyMap_KANA_GetMoveCursor;
    p_wk->get_keytype     = KeyMap_KANA_GetKeyType;
    break;
  case NAMEIN_INPUTTYPE_ABC:
    /* fallthrough */
  case NAMEIN_INPUTTYPE_KIGOU:
    p_wk->w     = 13;
    p_wk->h     = 6;
    p_wk->get_keyrect     = KeyMap_KANA_GetRect;
    p_wk->get_movecursor  = KeyMap_KANA_GetMoveCursor;
    p_wk->get_keytype     = KeyMap_KIGOU_GetKeyType;
    break;
  }

  //デフォルト移動値
  { 
    int i;
    for( i = 0; i < KEYMAP_KEYMOVE_BUFF_MAX; i++ )
    { 
      p_wk->buff.data[i]  = sc_keymove_default[i];
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  KEYMAP_WORK *p_wk 
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static void KEYMAP_Delete( KEYMAP_WORK *p_wk )
{ 
 // 全部メモリにおくようにしたので消すのは大元にした
  //NAMEIN_KEYMAP_Free( p_wk->p_key );
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(KEYMAP_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  キー配列  カーソルの位置の文字を取得する
 *
 *  @param  const KEYMAP_WORK *cp_wk  ワーク
 *  @param  GFL_POINT *cp_cursor      カーソルの位置
 *  @param  文字列
 *
 *  @return キーの種類
 */
//-----------------------------------------------------------------------------
static KEYMAP_KEYTYPE KEYMAP_GetKeyInfo( const KEYMAP_WORK *cp_wk, const GFL_POINT *cp_cursor, STRCODE *p_code )
{ 
  KEYMAP_KEYTYPE  type;
  GFL_POINT strpos;

  //キータイプ取得
  type  = cp_wk->get_keytype( cp_cursor, &strpos );

  if( type == KEYMAP_KEYTYPE_STR ||
      type == KEYMAP_KEYTYPE_DAKUTEN ||
      type == KEYMAP_KEYTYPE_HANDAKUTEN )
  { 
    if( p_code )
    { 
      *p_code = NAMEIN_KEYMAP_GetStr( cp_wk->p_key, strpos.x, strpos.y );
    }
  }
  else if( type == KEYMAP_KEYTYPE_SPACE )
  { 
    if( p_code )
    {
      *p_code = L'　';
    }
  }


  return type;
}
//----------------------------------------------------------------------------
/**
 *  @brief  キー配列  タッチ座標をカーソル座標に変換する
 *
 *  @param  const KEYMAP_WORK *cp_wk  ワーク
 *  @param  GFL_POINT *cp_trg         タッチ座標
 *  @param  *cp_cursor                カーソル座標
 *
 *  @retval TRUE変換できた FALSE範囲外だった
 */
//-----------------------------------------------------------------------------
static BOOL KEYMAP_GetTouchCursor( const KEYMAP_WORK *cp_wk, const GFL_POINT *cp_trg, GFL_POINT *p_cursor )
{ 
  GFL_POINT pos;
  GFL_RECT rect;

  for( pos.y = 0; pos.y < cp_wk->h; pos.y++ )
  { 
    for( pos.x = 0; pos.x < cp_wk->w; pos.x++ )
    { 
      if( KEYMAP_GetRange( cp_wk, &pos, &rect ) )
      {
        //キャラ単位をドット単位に
        rect.left   *= 8;
        rect.top    *= 8;
        rect.right  *= 8;
        rect.bottom *= 8;

        //当たり判定
        if( COLLISION_IsRectXPos( &rect, cp_trg ) )
        { 
          *p_cursor = pos;
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  キー配列  キーの繋がりを考慮して、加算値分動く
 *
 *  @param  KEYMAP_WORK *p_wk   ワーク
 *  @param  *p_now              カーソルの現在座標
 *  @param  GFL_POINT *cp_add   加算値
 */
//-----------------------------------------------------------------------------
static void KEYMAP_MoveCursor( KEYMAP_WORK *p_wk, GFL_POINT *p_now, const GFL_POINT *cp_add )
{ 
  p_wk->get_movecursor( p_now, cp_add, &p_wk->buff );
}
//----------------------------------------------------------------------------
/**
 *  @brief  キー配列  キー配列をBMPWINに書き込む
 *
 *  @param  const KEYMAP_WORK *cp_wk  ワーク
 *  @param  *p_util   プリント便利構造体
 *  @param  *p_que    キュー
 *  @param  *p_font   フォント
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void KEYMAP_Print( const KEYMAP_WORK *cp_wk, PRINT_UTIL *p_util, PRINT_QUE *p_que, GFL_FONT *p_font, HEAPID heapID )
{ 
  STRBUF  *p_strbuf;
  GFL_POINT pos;
  GFL_RECT  rect;
  u16 x, y;
  KEYMAP_KEYTYPE keytype;
  STRCODE get;

  //バッファ作成
  p_strbuf  = GFL_STR_CreateBuffer( 2, heapID );

  //全文字をプリント
  for( pos.y = 0; pos.y < NAMEIN_KEYMAP_GetHeight( cp_wk->p_key ); pos.y++ )
  { 
    for( pos.x = 0; pos.x < NAMEIN_KEYMAP_GetWidth( cp_wk->p_key ); pos.x++ )
    { 
      if( KEYMAP_GetRange( cp_wk, &pos, &rect ) )
      { 
        keytype = KEYMAP_GetKeyInfo( cp_wk, &pos, &get );
        if( keytype == KEYMAP_KEYTYPE_STR ||
            keytype == KEYMAP_KEYTYPE_DAKUTEN ||
            keytype == KEYMAP_KEYTYPE_HANDAKUTEN)
        { 
          //文字取得し、バッファに納める
          STRCODE code[2];
          code[0] = get;
          code[1] = GFL_STR_GetEOMCode();
          GFL_STR_SetStringCodeOrderLength( p_strbuf, code, 2 );

          //ドット単位へ
          rect.left   *=  8;
          rect.top    *=  8;
          rect.right  *=  8;
          rect.bottom *=  8;

          //矩形から、文字を中心に描画するために、位置算出
          x = rect.left + (rect.right - rect.left)/2 - KEYBOARD_BMPWIN_X*8;
          y = rect.top + (rect.bottom - rect.top)/2 - KEYBOARD_BMPWIN_Y*8;
          x -= PRINTSYS_GetStrWidth( p_strbuf, p_font, 0 )/2;
          y -= PRINTSYS_GetStrHeight( p_strbuf, p_font )/2;

          //描画
          //  絵のデータを見ると、わくが左から右へ、15ドットまでしかないため
          //  絵とあわせるために-1
          //  (うけとってくる矩形座標はキャラ単位なので16)
          PRINT_UTIL_Print( p_util, p_que, x-1, y, p_strbuf, p_font );
        }
      }
    }
  }

  //バッファクリア
  GFL_STR_DeleteBuffer( p_strbuf );
}
//----------------------------------------------------------------------------
/**
 *  @brief  キー配列  カーソルからキーのキャラ座標を受け取る
 *
 *  @param  const KEYMAP_WORK *cp_wk  ワーク
 *  @param  GFL_POINT *cp_cursor      カーソル
 *  @param  *p_rect                   キャラ矩形情報
 *  @retval TRUEなら受け取れた  FALSEならば受け取れない
 */
//-----------------------------------------------------------------------------
static BOOL KEYMAP_GetRange( const KEYMAP_WORK *cp_wk, const GFL_POINT *cp_cursor, GFL_RECT *p_rect )
{ 
  return cp_wk->get_keyrect( cp_cursor, p_rect );
}
//----------------------------------------------------------------------------
/**
 *  @brief  キー配列  キーの種類からキーのタイプを捜す
 *
 *  @param  const KEYMAP_WORK *cp_wk  ワーク
 *  @param  key         キーの種類
 *  @param  *p_cursor   カーソル位置受け取り
 *
 *  @return TRUEで存在  FALSEでない
 */
//-----------------------------------------------------------------------------
static BOOL KEYMAP_GetCursorByKeyType( const KEYMAP_WORK *cp_wk, KEYMAP_KEYTYPE key, GFL_POINT *p_cursor )
{ 
  GFL_POINT       pos;
    
  //次のキーの種類を探索してあったらカーソル位置変更
  for( pos.y = 0; pos.y <= cp_wk->h; pos.y++ )
  { 
    for( pos.x = 0; pos.x <= cp_wk->w; pos.x++ )
    { 
      if( key == cp_wk->get_keytype( &pos, NULL ) )
      { 
        *p_cursor = pos;
        return TRUE;
      }
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  キー配列  カーソル位置切り替え
 *
 *  @param  GFL_POINT *p_cursor カーソル
 *  @param  pre                 前のモード
 *  @param  next                次のモード
 */
//-----------------------------------------------------------------------------
static void KEYMAP_TOOL_ChangeCursorPos( GFL_POINT *p_cursor, NAMEIN_INPUTTYPE pre, NAMEIN_INPUTTYPE next )
{ 

  s16 w, h;
  GET_KEYTYPE_FUNC  prev_get_keytype;
  GET_KEYTYPE_FUNC  next_get_keytype;

  //モードで使う関数が違う
  if( pre == NAMEIN_INPUTTYPE_QWERTY )
  { 
    prev_get_keytype  = KeyMap_QWERTY_GetKeyType;
    next_get_keytype  = KeyMap_KANA_GetKeyType;
    w = KEYMAP_KANA_WIDTH;
    h = KEYMAP_KANA_HEIGHT;
  }
  else if( next == NAMEIN_INPUTTYPE_QWERTY )
  { 
    prev_get_keytype  = KeyMap_KANA_GetKeyType;
    next_get_keytype  = KeyMap_QWERTY_GetKeyType;
    w = KEYMAP_QWERTY_WIDTH;
    h = KEYMAP_QWERTY_HEIGHT;
  }

  //キーの種類と同じ場所を探す
  { 
    KEYMAP_KEYTYPE  key;
    GFL_POINT       pos;
    
    //現在のキーの種類
    key = prev_get_keytype( p_cursor, NULL );

    //次のキーの種類を探索してあったらカーソル位置変更
    for( pos.y = 0; pos.y <= h; pos.y++ )
    { 
      for( pos.x = 0; pos.x <= w; pos.x++ )
      { 
        if( key == next_get_keytype( &pos, NULL ) )
        { 
          *p_cursor = pos;
          return;
        }
      }
    }

  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  かなカナなどの碁盤の目上の矩形受け取り
 *
 *  @param  const GFL_POINT *cp_cursor  カーソル
 *  @param  *p_rect                     矩形受け取り(キャラ単位)
 *  @retval TRUEならば矩形が存在する  FALSEならばしない
 */
//-----------------------------------------------------------------------------
static BOOL KeyMap_KANA_GetRect( const GFL_POINT *cp_cursor, GFL_RECT *p_rect )
{ 
  enum
  { 
    KANA_START_X  = 3,
    KANA_START_Y  = 5,
    KANA_KEY_W    = 2,
    KANA_KEY_H    = 3,

    KANA_LONGKEY_W    = 8,
  };

  //キーの種類取得
  KEYMAP_KEYTYPE  key;
  GFL_POINT       strpos;

  key = KeyMap_KANA_GetKeyType( cp_cursor, &strpos );

  switch( key )
  { 
  case KEYMAP_KEYTYPE_STR:    //文字入力
    /*  fallthrough */
  case KEYMAP_KEYTYPE_DAKUTEN:  //濁点
    /*  fallthrough */
  case KEYMAP_KEYTYPE_HANDAKUTEN: //半濁点
    p_rect->left    = KANA_START_X + strpos.x * KANA_KEY_W;
    p_rect->top     = KANA_START_Y + strpos.y * KANA_KEY_H;
    p_rect->right   = p_rect->left + KANA_KEY_W;
    p_rect->bottom  = p_rect->top  + KANA_KEY_H;
    break;
  case KEYMAP_KEYTYPE_KANA:   //かなモード
    /*  fallthrough */
  case KEYMAP_KEYTYPE_KATA:   //カナモード
    /*  fallthrough */
  case KEYMAP_KEYTYPE_ABC:    //アルファベットモード
    /*  fallthrough */
  case KEYMAP_KEYTYPE_KIGOU:  //記号モード
    /*  fallthrough */
  case KEYMAP_KEYTYPE_QWERTY: //ローマ字モード
    p_rect->left    = KANA_START_X + cp_cursor->x * KANA_KEY_W;
    p_rect->top     = KANA_START_Y + cp_cursor->y * KANA_KEY_H;
    p_rect->right   = p_rect->left + KANA_KEY_W;
    p_rect->bottom  = p_rect->top  + KANA_KEY_H;
    break;
  case KEYMAP_KEYTYPE_DELETE: //けす
    p_rect->left    = KANA_START_X + cp_cursor->x * KANA_KEY_W;
    p_rect->top     = KANA_START_Y + cp_cursor->y * KANA_KEY_H;
    p_rect->right   = p_rect->left + KANA_LONGKEY_W;
    p_rect->bottom  = p_rect->top  + KANA_KEY_H;
    break;  
    break;
  case KEYMAP_KEYTYPE_DECIDE: //やめる
    p_rect->left    = KANA_START_X + (cp_cursor->x-1) * KANA_KEY_W + KANA_LONGKEY_W;
    p_rect->top     = KANA_START_Y + (cp_cursor->y) * KANA_KEY_H;
    p_rect->right   = p_rect->left + KANA_LONGKEY_W;
    p_rect->bottom  = p_rect->top  + KANA_KEY_H;
    break;
  case KEYMAP_KEYTYPE_NONE:
    return FALSE;
  }

  switch( cp_cursor->y )
  { 
  case 5:
    if( cp_cursor->x > 6 )
    { 
      return FALSE;
    }
    break;
  }


  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  PCのキーボード状の矩形受け取り
 *
 *  @param  const GFL_POINT *cp_cursor  カーソル
 *  @param  *p_rect                     矩形受け取り
 *  @retval TRUEならば矩形が存在する  FALSEならばしない
 */
//-----------------------------------------------------------------------------
static BOOL KeyMap_QWERTY_GetRect( const GFL_POINT *cp_cursor, GFL_RECT *p_rect )
{ 
  enum
  { 
    KANA_KEY_W        = 2,
    KANA_KEY_H        = 3,

    KANA_LONGKEY_W    = 4,  
    KANA_ENTER_W      = 5,  
    KANA_SPACE_W      = 14, 
  };

  //文字キーの行ごとの開始位置
  static const int sc_start_x[] =
  { 
    5,6,7
  };
  static const int sc_start_y[] =
  { 
    8,12,16
  };


  //キーの種類取得
  KEYMAP_KEYTYPE  key;
  GFL_POINT       strpos;
  
  key = KeyMap_QWERTY_GetKeyType( cp_cursor, &strpos );

  switch( key )
  { 
  case KEYMAP_KEYTYPE_STR:    //文字入力
    p_rect->left    = sc_start_x[ strpos.y ] + strpos.x * KANA_KEY_W;
    p_rect->top     = sc_start_y[ strpos.y ];
    p_rect->right   = p_rect->left + KANA_KEY_W;
    p_rect->bottom  = p_rect->top  + KANA_KEY_H;
    break;
  case KEYMAP_KEYTYPE_KANA:   //かなモード
    /*  fallthrough */
  case KEYMAP_KEYTYPE_KATA:   //カナモード
    /*  fallthrough */
  case KEYMAP_KEYTYPE_ABC:    //アルファベットモード
    /*  fallthrough */
  case KEYMAP_KEYTYPE_KIGOU:  //記号モード
    /*  fallthrough */
  case KEYMAP_KEYTYPE_QWERTY: //ローマ字モード
    p_rect->left    = 3 + cp_cursor->x * KANA_KEY_W;
    p_rect->top     = 20;
    p_rect->right   = p_rect->left + KANA_KEY_W;
    p_rect->bottom  = p_rect->top  + KANA_KEY_H;
    break;
  case KEYMAP_KEYTYPE_DELETE: //けす
    p_rect->left    = 25;
    p_rect->top     = 8;
    p_rect->right   = p_rect->left + KANA_LONGKEY_W;
    p_rect->bottom  = p_rect->top  + KANA_KEY_H;
    break;  
    break;
  case KEYMAP_KEYTYPE_DECIDE: //やめる
    p_rect->left    = 24;
    p_rect->top     = 12;
    p_rect->right   = p_rect->left + KANA_ENTER_W;
    p_rect->bottom  = p_rect->top  + KANA_KEY_H;
    break;
  case KEYMAP_KEYTYPE_SHIFT : //シフト
    if( cp_cursor->x == 0 )
    { 
      p_rect->left    = 3;
      p_rect->top     = 16;
      p_rect->right   = p_rect->left + KANA_LONGKEY_W;
      p_rect->bottom  = p_rect->top  + KANA_KEY_H;
    }
    else
    { 
      p_rect->left    = 23;
      p_rect->top     = 16;
      p_rect->right   = p_rect->left + KANA_LONGKEY_W;
      p_rect->bottom  = p_rect->top  + KANA_KEY_H;
    }
    break;
  case KEYMAP_KEYTYPE_SPACE : //スペース
    p_rect->left    = 13;
    p_rect->top     = 20;
    p_rect->right   = p_rect->left + KANA_SPACE_W;
    p_rect->bottom  = p_rect->top  + KANA_KEY_H;
    break;
  case KEYMAP_KEYTYPE_NONE:
    return FALSE;
  }
  
  switch( cp_cursor->y )
  { 
  case 1:
    if( cp_cursor->x > 9 ) 
    { 
      return FALSE;
    }
    break;
  case 2:
    if( cp_cursor->x > 9 ) 
    { 
      return FALSE;
    }
    break;
  case 3:
    if( cp_cursor->x > 5 ) 
    { 
      return FALSE;
    }
    break;
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  かなカナなどの碁盤の目上のカーソル移動
 *
 *  @param  GFL_POINT *p_now    カーソル
 *  @param  GFL_POINT *cp_add   移動値
 *  @param  p_buff              カーソル移動記憶バッファ
 */
//-----------------------------------------------------------------------------
static void KeyMap_KANA_GetMoveCursor( GFL_POINT *p_now, const GFL_POINT *cp_add, KEYMOVE_BUFF *p_buff )
{
  //位置
  enum
  { 
    ROW_START   = 0,
    ROW_KEY_END = 4,
    ROW_BTN     = 5,
    ROW_END     = ROW_BTN,

    COL_START   = 0,
    COL_KEY_END = 12,
    COL_BTN_END = 6,
  };


  BOOL is_btn_in  = FALSE;
  BOOL is_btn_out = FALSE;

  switch( p_now->y )
  { 
  case ROW_START:
    //キー最上段
    p_now->x  += cp_add->x;
    p_now->x  = MATH_ROUND( p_now->x, COL_START, COL_KEY_END );
    p_now->y  += cp_add->y;
    p_now->y  = MATH_ROUND( p_now->y, ROW_START, ROW_END );

    if( p_now->y == ROW_BTN )
    { 
      is_btn_in = TRUE;
    }
    break;
  case ROW_KEY_END:
    //キー最下段

    p_now->x  += cp_add->x;
    p_now->x  = MATH_ROUND( p_now->x, COL_START, COL_KEY_END );
    p_now->y  += cp_add->y;
    p_now->y  = MATH_ROUND( p_now->y, ROW_START, ROW_END );

    if( p_now->y == ROW_BTN )
    { 
      is_btn_in = TRUE;
    }
    break;
  case ROW_BTN:
    //ボタン

    p_now->x  += cp_add->x;
    p_now->x  = MATH_ROUND( p_now->x, COL_START, COL_BTN_END );
    p_now->y  += cp_add->y;
    p_now->y  = MATH_ROUND( p_now->y, ROW_START, ROW_END );

    if( p_now->y != ROW_BTN )
    { 
      is_btn_out  = TRUE;
    }
    break;
  default:
    //キー中間
    p_now->x  += cp_add->x;
    p_now->x  = MATH_ROUND( p_now->x, COL_START, COL_KEY_END );
    p_now->y  += cp_add->y;
    p_now->y  = MATH_ROUND( p_now->y, ROW_START, ROW_END );
  }

  //ボタンに入ったときの修正
  if( is_btn_in )
  {
    if( 5 <= p_now->x  && p_now->x <= 8 )
    { 
      //けすボタン
      p_buff->data[KEYMAP_KEYMOVE_BUFF_DELETE]  = p_now->x;
      p_now->x  = 5;
    }
    else if( 9 <= p_now->x  && p_now->x <= 12 )
    { 
      //やめるボタン
      p_buff->data[KEYMAP_KEYMOVE_BUFF_DICEDE]  = p_now->x;
      p_now->x  = 6;
    }
  }

  //ボタンから出たときの修正
  if( is_btn_out )
  { 
    if( p_now->x == 5 )
    { 
      //けすボタン
      p_now->x  = p_buff->data[KEYMAP_KEYMOVE_BUFF_DELETE];
    }
    else if( p_now->x == 6 )
    { 
      p_now->x  = p_buff->data[KEYMAP_KEYMOVE_BUFF_DICEDE];
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  PCのキーボード状のカーソル移動
 *
 *  @param  GFL_POINT *p_now    カーソル
 *  @param  GFL_POINT *cp_add   移動値
 *  @param  p_buff              カーソル移動記憶バッファ
 */
//-----------------------------------------------------------------------------
static void KeyMap_QWERTY_GetMoveCursor( GFL_POINT *p_now, const GFL_POINT *cp_add, KEYMOVE_BUFF *p_buff )
{ 
  //位置
  enum
  { 
    ROW_START   = 0,
    ROW_LINE0   = ROW_START,
    ROW_LINE1,
    ROW_LINE2,
    ROW_LINE3,
    ROW_END     = ROW_LINE3,

    COL_START     = 0,
    COL_LINE1_END = 10,
    COL_LINE2_END = 9,
    COL_LINE3_END = 9,
    COL_LINE4_END = 5,
  };

  switch( p_now->y )
  { 
  case ROW_LINE0:
    p_now->x  += cp_add->x;
    p_now->x  = MATH_ROUND( p_now->x, COL_START, COL_LINE1_END );
    p_now->y  += cp_add->y;
    p_now->y  = MATH_ROUND( p_now->y, ROW_START, ROW_END );

    if( p_now->y == ROW_LINE3 )
    { 
      //バッファ
      switch( p_now->x )
      { 
      case 8:
      case 9:
        p_buff->data[KEYMAP_KEYMOVE_BUFF_L_U] = p_now->x;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_D] = p_now->x;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_U] = 8;
        break;
      case 4:
      case 5:
      case 6:
      case 7:
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_D] = p_now->x;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_U] = p_now->x;
        break;
      case 10:
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_D] = 10;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_U] = 9;
        break;
      }

      //ROW_LINE3へ移動
      switch( p_now->x )
      { 
      case 0: //Q
        p_now->x  = p_buff->data[KEYMAP_KEYMOVE_BUFF_Q_U];
        break;
      case 1:
        /* fallthrough */
      case 2:
        /* fallthrough */
      case 3:
        p_now->x  += 1;
        break;
      default:  //4～10
        p_now->x  = 5;
        break;
      }
    }
    else if( p_now->y == ROW_LINE1 )
    { 
      //バッファ
      switch( p_now->x )
      { 
      case 8:
      case 9:
        p_buff->data[KEYMAP_KEYMOVE_BUFF_L_U] = p_now->x;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_D] = p_now->x;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_U] = MATH_CLAMP( p_now->x, 4,9 );
        break;
      case 4:
      case 5:
      case 6:
      case 7:
      case 10:
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_D] = p_now->x;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_U] = MATH_CLAMP( p_now->x, 4,9 );
        break;
      }

      //ROW_LINE1へ移動
      switch( p_now->x )
      { 
      case 8:   //O
        /* fallthrough */
      case 9:   //P
        p_now->x  = 8;
        break;
      case 10:  //×
        p_now->x  = 9;
        break;
      default:  //0～7まではそのまま
        p_now->x  = p_now->x;
        break;
      }
    }
    break;
  case ROW_LINE1:
    p_now->x  += cp_add->x;
    p_now->x  = MATH_ROUND( p_now->x, COL_START, COL_LINE2_END );
    p_now->y  += cp_add->y;
    p_now->y  = MATH_ROUND( p_now->y, ROW_START, ROW_END );
    if( p_now->y == ROW_LINE0 )
    { 
      //バッファ
      switch( p_now->x )
      { 
      case 7:
        /* fallthrough  */
      case 8:
        p_buff->data[KEYMAP_KEYMOVE_BUFF_BOU_U] = p_now->x;
        break;
      }

      //ROW_LINE0へ移動
      switch( p_now->x )
      { 
      case 8: //L 
        p_now->x  = p_buff->data[KEYMAP_KEYMOVE_BUFF_L_U];
        break;
      case 9: //ENTER
        p_now->x  = 10;
        break;
      default:  //0～７
        p_now->x  = p_now->x;
        break;
      }
    }
    else if( p_now->y == ROW_LINE2 )
    { 
      //バッファ
      switch( p_now->x )
      { 
      case 7:
        /* fallthrough  */
      case 8:
        p_buff->data[KEYMAP_KEYMOVE_BUFF_BOU_U] = p_now->x;
        break;
      }

      //ROW_LINE2へ移動
      switch( p_now->x )
      { 
      case 0: //a
        p_now->x  = p_buff->data[KEYMAP_KEYMOVE_BUFF_A_D];
        break;
      case 7:
        /* fallthrough */
      case 8:
        p_now->x  = 8;
        break;
      case 9:
        p_now->x  = 9;
        break;
      default:  //1～6
        p_now->x  += 1;
        break;
      }
    }
    break;
  case ROW_LINE2:
    p_now->x  += cp_add->x;
    p_now->x  = MATH_ROUND( p_now->x, COL_START, COL_LINE3_END );
    p_now->y  += cp_add->y;
    p_now->y  = MATH_ROUND( p_now->y, ROW_START, ROW_END );
    if( p_now->y == ROW_LINE1 )
    { 
      //バッファ
      switch( p_now->x )
      { 
      case 0://SHIFT
      case 1://Z
        p_buff->data[KEYMAP_KEYMOVE_BUFF_A_D] = p_now->x;
        break;

      case 4:
      case 5:
      case 6:
      case 7:
      case 8: //4
      case 9:
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_U] = p_now->x;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_D] = p_now->x;
        break;
      }

      //ROW_LINE1へ移動
      switch( p_now->x )
      { 
      case 0: //SHIFT L
        /* fallthrough */
      case 1: //Z
        p_now->x  = 0;
        break;
      case 8: //-
        p_now->x  = p_buff->data[KEYMAP_KEYMOVE_BUFF_BOU_U];
        break;
      case 9: //SHIFT R
        p_now->x  = 9;
        break;
      default:  //2~7
        p_now->x  -= 1;
      }

    }
    else if( p_now->y == ROW_LINE3 )
    { 
      //バッファ
      switch( p_now->x )
      { 
      case 0://SHIFT
      case 1://Z
        p_buff->data[KEYMAP_KEYMOVE_BUFF_A_D] = p_now->x;
        break;

      case 4:
      case 5:
      case 6:
      case 7:
      case 8: //4～9
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_U] = p_now->x;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_D] = p_now->x;
        break;
      case 9: 
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_U] = 9;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_D] = 10;
        break;
      }

      //ROW_LINE3へ移動
      switch( p_now->x )
      { 
      case 0: //SHIFT L
        p_now->x  = p_buff->data[KEYMAP_KEYMOVE_BUFF_LSHIFT_D];
        break;
      case 1: //Z
        /* fallthrough */
      case 2: //X
        /* fallthrough */
      case 3: //C
        p_now->x  += 1;
        break;
      default: //4～8
        p_now->x  = 5;
      }
    }
    break;
  case ROW_LINE3:
    p_now->x  += cp_add->x;
    p_now->x  = MATH_ROUND( p_now->x, COL_START, COL_LINE4_END );
    p_now->y  += cp_add->y;
    p_now->y  = MATH_ROUND( p_now->y, ROW_START, ROW_END );
    if( p_now->y == ROW_LINE2 )
    { 
      //バッファ処理
      switch( p_now->x )
      { 
      case 0:
      case 1:
        p_buff->data[KEYMAP_KEYMOVE_BUFF_LSHIFT_D]  = p_now->x;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_Q_U] = p_now->x;
        break;
      }

      //ROW_LINE2へ移動
      switch( p_now->x )
      { 
      case 0: //かな
        /* fallthrough */
      case 1: //カナ
        p_now->x  = 0;
        break;
      case 2: //ABC
        /* fallthrough */
      case 3: //記号
        /* fallthrough */
      case 4: //ローマ字
        p_now->x  -= 1;
        break;
      case 5: //SPCASE
        p_now->x  = p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_U];
        break;
      }
    }
    else if( p_now->y == ROW_LINE0 )
    { 
      //バッファ処理
      switch( p_now->x )
      { 
      case 0:
      case 1:
        p_buff->data[KEYMAP_KEYMOVE_BUFF_LSHIFT_D]  = p_now->x;
        p_buff->data[KEYMAP_KEYMOVE_BUFF_Q_U] = p_now->x;
        break;
      }

      //ROW_LINE0へ移動
      switch( p_now->x )
      { 
      case 0: //かな
        /* fallthrough */
      case 1: //カナ
        p_now->x  = 0;
        break;
      case 2: //ABC
        /* fallthrough */
      case 3: //記号
        /* fallthrough */
      case 4: //ローマ字
        p_now->x  -= 1;
        break;
      case 5: //SPCASE
        p_now->x  = p_buff->data[KEYMAP_KEYMOVE_BUFF_SPACE_D];
        break;
      }
    }
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *  @brief  かなカナなどの碁盤の目上のキー種類取得
 *
 *  @param  const GFL_POINT *cp_cursor  カーソル
 *  @param  p_strpos                    文字受け取り座標
 *
 *  @return キー情報
 */
//-----------------------------------------------------------------------------
static KEYMAP_KEYTYPE KeyMap_KANA_GetKeyType( const GFL_POINT *cp_cursor, GFL_POINT *p_strpos )
{ 
  if( cp_cursor->y < KEYMAP_KANA_HEIGHT )
  { 
    if( cp_cursor->y == 0 && cp_cursor->x == 12 )
    { 
      if( p_strpos )
      { 
        p_strpos->x = cp_cursor->x;
        p_strpos->y = cp_cursor->y;
      }
      //濁点
      return KEYMAP_KEYTYPE_DAKUTEN;
    }
    else if( cp_cursor->y == 1 && cp_cursor->x == 12 )
    { 
      if( p_strpos )
      { 
        p_strpos->x = cp_cursor->x;
        p_strpos->y = cp_cursor->y;
      }
      //半濁点
      return KEYMAP_KEYTYPE_HANDAKUTEN;
    }
    else
    { 
      //その他文字
      //碁盤の目上はカーソル位置と同じ
      if( p_strpos )
      { 
        p_strpos->x = cp_cursor->x;
        p_strpos->y = cp_cursor->y;
      }
      return KEYMAP_KEYTYPE_STR;
    }
  }
  else if( cp_cursor->x < 7 )
  { 
    return cp_cursor->x + KEYMAP_KEYTYPE_KANA;
  }

  return KEYMAP_KEYTYPE_NONE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  かなカナなどの碁盤の目上のキー種類取得
 *
 *  @param  const GFL_POINT *cp_cursor  カーソル
 *  @param  p_strpos                    文字受け取り座標
 *
 *  @return キー情報
 */
//-----------------------------------------------------------------------------
static KEYMAP_KEYTYPE KeyMap_KIGOU_GetKeyType( const GFL_POINT *cp_cursor, GFL_POINT *p_strpos )
{ 
  if( cp_cursor->y < KEYMAP_KANA_HEIGHT )
  { 
    //その他文字
    //碁盤の目上はカーソル位置と同じ
    if( p_strpos )
    { 
      p_strpos->x = cp_cursor->x;
      p_strpos->y = cp_cursor->y;
    }

    return KEYMAP_KEYTYPE_STR;
  }
  else if( cp_cursor->x < 7 )
  { 
    return cp_cursor->x + KEYMAP_KEYTYPE_KANA;
  }

  return KEYMAP_KEYTYPE_NONE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  PCのキーボード状のキー種類取得
 *
 *  @param  const GFL_POINT *cp_cursor  カーソル
 *  @param  GFL_POINT *p_strpos         文字受け取り座標
 *
 *  @return キー情報
 */
//-----------------------------------------------------------------------------
static KEYMAP_KEYTYPE KeyMap_QWERTY_GetKeyType( const GFL_POINT *cp_cursor, GFL_POINT *p_strpos )
{ 
  GFL_POINT strpos;
  KEYMAP_KEYTYPE  ret = KEYMAP_KEYTYPE_NONE;

  //それぞれの行からキーの種類を受け取る
  switch( cp_cursor->y )
  {
  case 0:
    if( cp_cursor->x == 10 )
    { 
      ret =  KEYMAP_KEYTYPE_DELETE;
    }
    else
    { 
      strpos  = *cp_cursor;
      ret =  KEYMAP_KEYTYPE_STR;
    }
    break;
  case 1:
    if( cp_cursor->x == 9 )
    { 
      ret =  KEYMAP_KEYTYPE_DECIDE;
    }
    else
    { 
      strpos  = *cp_cursor;
      ret =  KEYMAP_KEYTYPE_STR;
    }
    break;
  case 2:
    if( cp_cursor->x == 0 || cp_cursor->x == 9 )
    { 
      ret =  KEYMAP_KEYTYPE_SHIFT;
    }
    else
    { 
      //左シフトの分減らす
      strpos.x  = cp_cursor->x - 1;
      strpos.y  = cp_cursor->y;
      ret =  KEYMAP_KEYTYPE_STR;
    }
    break;
  case 3:
    if( cp_cursor->x == 5 )
    { 
      ret =  KEYMAP_KEYTYPE_SPACE;
    }
    else
    { 
      ret =  KEYMAP_KEYTYPE_KANA + cp_cursor->x;
    }
    break;
  }

  //文字位置を受け取り
  if( ret == KEYMAP_KEYTYPE_STR )
  { 
    if( p_strpos )
    { 
      *p_strpos = strpos;
    }
  }


  return ret;
}
//=============================================================================
/**
 *          KEYBOARD_WORK
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  キーボード  初期化
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *  @param  mode    入力モード
 *  @param  p_font  フォント
 *  @param  p_que   キュー
 *  @param  heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void KEYBOARD_Init( KEYBOARD_WORK *p_wk, NAMEIN_INPUTTYPE mode, GFL_FONT *p_font, PRINT_QUE *p_que, NAMEIN_KEYMAP_HANDLE *p_keymap_handle, HEAPID heapID )
{ 
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(KEYBOARD_WORK) );
  p_wk->cursor.x  = 0;
  p_wk->cursor.y  = 0;
  p_wk->p_font    = p_font;
  p_wk->p_que     = p_que;
  p_wk->heapID    = heapID;
  p_wk->mode      = mode;
  p_wk->change_mode = mode;
  p_wk->p_keymap_handle = p_keymap_handle;

  //モードによって読み込むスクリーンが違うのでメモリに取っておく
  p_wk->p_scr_adrs[0] = GFL_ARC_UTIL_LoadScreen(ARCID_NAMEIN_GRA, NARC_namein_gra_name_romaji_NSCR, FALSE, &p_wk->p_scr[0], heapID );
  p_wk->p_scr_adrs[1] = GFL_ARC_UTIL_LoadScreen(ARCID_NAMEIN_GRA, NARC_namein_gra_name_kana_NSCR, FALSE, &p_wk->p_scr[1], heapID );

  if( mode == NAMEIN_INPUTTYPE_QWERTY )
  { 
    GFL_BG_LoadScreenBuffer( BG_FRAME_KEY_M, p_wk->p_scr[0]->rawData, 0x800 );
    GFL_BG_SetVisible( BG_FRAME_BTN_M, FALSE );
  }
  else
  { 
    GFL_BG_LoadScreenBuffer( BG_FRAME_KEY_M, p_wk->p_scr[1]->rawData, 0x800 );
    GFL_BG_SetVisible( BG_FRAME_BTN_M, TRUE );
  }
  GFL_BG_LoadScreenReq( BG_FRAME_KEY_M );

  //キーアニメ作成
  KEYANM_Init( &p_wk->keyanm, mode, heapID );

  //キーマップ作成
  KEYMAP_Create( &p_wk->keymap, mode, p_wk->p_keymap_handle, heapID );

  //キー用BMPWIN作成
  p_wk->p_bmpwin  = GFL_BMPWIN_Create( BG_FRAME_FONT_M, KEYBOARD_BMPWIN_X, KEYBOARD_BMPWIN_Y, KEYBOARD_BMPWIN_W, KEYBOARD_BMPWIN_H, PLT_BG_FONT_M, GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );

  //プリントキュー設定
  PRINT_UTIL_Setup( &p_wk->util, p_wk->p_bmpwin );

  //描画
  KEYMAP_Print( &p_wk->keymap, &p_wk->util, p_que, p_font, heapID );


 //キーモードがキーならばカーソル位置を押す
  { 
    GFL_RECT rect;
    if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_KEY )
    { 
      //モードボタン+カーソル位置押下のため
      if( KEYMAP_GetRange( &p_wk->keymap, &p_wk->cursor, &rect ) )
      { 
        KEYANM_Start( &p_wk->keyanm, KEYANM_TYPE_MOVE, &rect, p_wk->is_shift, p_wk->mode );  
      }
    }
    else
    { 
      //モードボタン押下のため
      if( KEYMAP_GetRange( &p_wk->keymap, &p_wk->cursor, &rect ) )
      { 
        KEYANM_Start( &p_wk->keyanm, KEYANM_TYPE_NONE, &rect, p_wk->is_shift, p_wk->mode );  
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーボード  破棄
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KEYBOARD_Exit( KEYBOARD_WORK *p_wk )
{ 
  //BMPWIN破棄
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );

  //キーマップ破棄
  KEYMAP_Delete( &p_wk->keymap );

  //キーアニメ破棄
  KEYANM_Exit( &p_wk->keyanm );

  GFL_HEAP_FreeMemory( p_wk->p_scr_adrs[0] );
  GFL_HEAP_FreeMemory( p_wk->p_scr_adrs[1] );

  //クリア
  GFL_STD_MemClear( p_wk, sizeof(KEYBOARD_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーボード  メイン処理
 *
 *  @param  KEYBOARD_WORK *p_wk   ワーク
 *  @param  STRINPUT_WORK *cp_strinput  文字入力欄
 *
 */
//-----------------------------------------------------------------------------
static void KEYBOARD_Main( KEYBOARD_WORK *p_wk, const STRINPUT_WORK *cp_strinput )
{
  //入力を初期化
  p_wk->input = KEYBOARD_INPUT_NONE;

  //状態ごとの処理
  switch( p_wk->state )
  { 
  case KEYBOARD_STATE_WAIT:   //処理待ち中
    { 
      BOOL  is_update = FALSE;
      KEYBOARD_INPUT_REQUEST req;

      GFL_STD_MemClear( &req, sizeof(KEYBOARD_INPUT_REQUEST) );

      is_update = Keyboard_TouchReq( p_wk, &req );
      if( !is_update )
      { 
        Keyboard_KeyReq( p_wk, &req );
        is_update = Keyboard_BtnReq( p_wk, &req );
      }

      if( is_update )
      { 
        Keyboard_Decide( p_wk, &req );
      }
    }
    break;

  case KEYBOARD_STATE_MOVE:   //移動中
    //移動中にもモード切替のみ可能
    { 
      BOOL  is_update = FALSE;
      KEYBOARD_INPUT_REQUEST req;

      GFL_STD_MemClear( &req, sizeof(KEYBOARD_INPUT_REQUEST) );

      is_update = Keyboard_Move_TouchReq( p_wk, &req );
      if( !is_update )
      { 
        Keyboard_Move_KeyReq( p_wk, &req );
        is_update = Keyboard_Move_BtnReq( p_wk, &req );
      }

      if( is_update )
      { 
        Keyboard_Move_Decide( p_wk, &req );
      }
    }

    //移動
    if( Keyboard_MainMove( p_wk ) )
    { 
      p_wk->state = KEYBOARD_STATE_WAIT;
    }
    break;

  case KEYBOARD_STATE_INPUT:  //入力
    if( STRINPUT_IsInputEnd( cp_strinput ) )
    { 
      KEYMAP_GetCursorByKeyType( &p_wk->keymap, KEYMAP_KEYTYPE_DECIDE, &p_wk->cursor );
    }
    p_wk->state = KEYBOARD_STATE_WAIT;
    break;

  default:
    GF_ASSERT(0);
  }

  { 
    GFL_RECT rect;
    KEYMAP_GetRange( &p_wk->keymap, &p_wk->cursor, &rect );
    KEYANM_Main( &p_wk->keyanm, p_wk->mode, &rect );
  }

  KEYBOARD_PrintMain( p_wk );
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーボード  現在の情報を取得
 *
 *  @param  const KEYBOARD_WORK *cp_wk ワーク
 *
 *  @return 状態
 */
//-----------------------------------------------------------------------------
static KEYBOARD_STATE KEYBOARD_GetState( const KEYBOARD_WORK *cp_wk )
{ 
  return cp_wk->state;
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーボード  現在の入力を取得
 *
 *  @param  const KEYBOARD_WORK *cp_wk  ワーク
 *  @param  *p_str                      入力した文字列
 *
 *  @return 入力状況
 */
//-----------------------------------------------------------------------------
static KEYBOARD_INPUT KEYBOARD_GetInput( const KEYBOARD_WORK *cp_wk, STRCODE *p_str )
{ 

  //入力したときは文字列を返す
  if( cp_wk->input == KEYBOARD_INPUT_STR ||
      cp_wk->input == KEYBOARD_INPUT_CHANGESTR ||
      cp_wk->input == KEYBOARD_INPUT_SPACE
      )
  { 
    if( p_str )
    { 
      *p_str  = cp_wk->code;
    }
  }

  return cp_wk->input;
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーボード  シフト押しているかどうか
 *
 *  @param  const KEYBOARD_WORK *cp_wk  ワーク
 *
 *  @return TRUE シフト押している FALSEシフト押してない
 */
//-----------------------------------------------------------------------------
static BOOL KEYBOARD_IsShift( const KEYBOARD_WORK *cp_wk )
{ 
  return cp_wk->is_shift;
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーボード  描画メイン
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *
 *  @retval  BOOL  転送が終わっている場合はTRUE／終わっていない場合はFALSE
 */
//-----------------------------------------------------------------------------
static BOOL KEYBOARD_PrintMain( KEYBOARD_WORK *p_wk )
{ 
  return PRINT_UTIL_Trans( &p_wk->util, p_wk->p_que );
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーボード  入力モードを返す
 *
 *  @param  const KEYBOARD_WORK *cp_wk  ワーク
 *
 *  @return 入力モードを返す
 */
//-----------------------------------------------------------------------------
static NAMEIN_INPUTTYPE KEYBOARD_GetInputType( const KEYBOARD_WORK *cp_wk )
{ 
  return cp_wk->mode;
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーボード  入力モード変更時の動作アニメ開始
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *  @param  mode                モード
 *  @retval TRUE 開始可能 FALSE不可能
 */
//-----------------------------------------------------------------------------
static BOOL Keyboard_StartMove( KEYBOARD_WORK *p_wk, NAMEIN_INPUTTYPE mode )
{ 
  if( p_wk->change_mode != mode )
  { 
    p_wk->change_mode     = mode;
    p_wk->change_move_cnt = 0;
    p_wk->change_move_seq = 0;
    p_wk->is_change_anm   = TRUE;

    p_wk->is_btn_move  = p_wk->change_mode == NAMEIN_INPUTTYPE_QWERTY
                        || p_wk->mode == NAMEIN_INPUTTYPE_QWERTY;

  
    if( p_wk->is_btn_move )
    { 
      //フォントとフレームとボタンが動く
//      G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 |
//          GX_BLEND_PLANEMASK_BG3, KEYBOARD_CHANGEMOVE_START_ALPHA );
  
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3,
          GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 |
          GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BD,
          KEYBOARD_CHANGEMOVE_START_ALPHA, 16 );
    }
    else
    {
      //フォントとフレームだけ動く
//      G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG3,
//          KEYBOARD_CHANGEMOVE_START_ALPHA );
//
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG3,
          GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 |
          GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BD,
          KEYBOARD_CHANGEMOVE_START_ALPHA, 16 );
    }
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーボード  入力モード変更時の動作アニメ動作
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *
 *  @retval TRUE終了  FALSE処理中or処理していない
 */
//-----------------------------------------------------------------------------
static BOOL Keyboard_MainMove( KEYBOARD_WORK *p_wk )
{
  enum
  { 
    SEQ_DOWN_MAIN,
    SEQ_CHANGE_MODE,
    SEQ_UP_INIT,
    SEQ_UP_MAIN,
    SEQ_END,
  };
  s16 scroll_y;
  s16 alpha;

  if( p_wk->is_change_anm )
  { 
    switch( p_wk->change_move_seq )
    { 
    case SEQ_DOWN_MAIN:
      GFL_BG_GetScrollY( BG_FRAME_KEY_M );
      scroll_y  = KEYBOARD_CHANGEMOVE_START_Y
        + (KEYBOARD_CHANGEMOVE_END_Y-KEYBOARD_CHANGEMOVE_START_Y)
        * p_wk->change_move_cnt / KEYBOARD_CHANGEMOVE_SYNC;

      alpha     = KEYBOARD_CHANGEMOVE_START_ALPHA
        + (KEYBOARD_CHANGEMOVE_END_ALPHA-KEYBOARD_CHANGEMOVE_START_ALPHA)
        * p_wk->change_move_cnt / KEYBOARD_CHANGEMOVE_SYNC;

      G2_ChangeBlendAlpha( alpha, 16 );
      //G2_ChangeBlendBrightness( alpha );
      GFL_BG_SetScroll( BG_FRAME_KEY_M, GFL_BG_SCROLL_Y_SET, scroll_y );
      GFL_BG_SetScroll( BG_FRAME_FONT_M, GFL_BG_SCROLL_Y_SET, scroll_y );
      if( p_wk->is_btn_move )
      { 
        GFL_BG_SetScroll( BG_FRAME_BTN_M, GFL_BG_SCROLL_Y_SET, scroll_y );
      }
      if( p_wk->change_move_cnt++ > KEYBOARD_CHANGEMOVE_SYNC )
      { 
        if( p_wk->is_btn_move )
        { 
          GFL_BG_SetVisible( BG_FRAME_KEY_M, FALSE );
          GFL_BG_SetVisible( BG_FRAME_BTN_M, FALSE );
          GFL_BG_SetVisible( BG_FRAME_FONT_M, FALSE );
        }
        else
        { 
          GFL_BG_SetVisible( BG_FRAME_KEY_M, FALSE );
          GFL_BG_SetVisible( BG_FRAME_FONT_M, FALSE );
        }
        p_wk->change_move_cnt = 0;
        p_wk->change_move_seq     = SEQ_CHANGE_MODE;
      }
      break;

    case SEQ_CHANGE_MODE:
      GFL_BG_SetScroll( BG_FRAME_KEY_M, GFL_BG_SCROLL_Y_SET, KEYBOARD_CHANGEMOVE_END_Y );
      GFL_BG_SetScroll( BG_FRAME_FONT_M, GFL_BG_SCROLL_Y_SET, KEYBOARD_CHANGEMOVE_END_Y );
      if( p_wk->is_btn_move )
      { 
        GFL_BG_SetScroll( BG_FRAME_BTN_M, GFL_BG_SCROLL_Y_SET, KEYBOARD_CHANGEMOVE_END_Y );
      }

      Keyboard_ChangeMode( p_wk, p_wk->change_mode );
      p_wk->change_move_seq = SEQ_UP_INIT;
      break;

    case SEQ_UP_INIT:
      p_wk->change_move_cnt = KEYBOARD_CHANGEMOVE_SYNC;
      p_wk->change_move_seq = SEQ_UP_MAIN;
      GFL_BG_SetVisible( BG_FRAME_KEY_M, TRUE );
      GFL_BG_SetVisible( BG_FRAME_FONT_M, TRUE );
      if( p_wk->mode != NAMEIN_INPUTTYPE_QWERTY )
      { 
        GFL_BG_SetVisible( BG_FRAME_BTN_M, TRUE );
      }
      G2_ChangeBlendAlpha( 0, 16 );
      break;

    case SEQ_UP_MAIN:
      scroll_y  = KEYBOARD_CHANGEMOVE_START_Y
        + (KEYBOARD_CHANGEMOVE_END_Y-KEYBOARD_CHANGEMOVE_START_Y)
        * p_wk->change_move_cnt / KEYBOARD_CHANGEMOVE_SYNC;

      alpha     = KEYBOARD_CHANGEMOVE_START_ALPHA
        + (KEYBOARD_CHANGEMOVE_END_ALPHA-KEYBOARD_CHANGEMOVE_START_ALPHA)
        * p_wk->change_move_cnt / KEYBOARD_CHANGEMOVE_SYNC;

      G2_ChangeBlendAlpha( alpha, 16 );
      //G2_ChangeBlendBrightness( alpha );
      GFL_BG_SetScroll( BG_FRAME_KEY_M, GFL_BG_SCROLL_Y_SET, scroll_y );
      GFL_BG_SetScroll( BG_FRAME_FONT_M, GFL_BG_SCROLL_Y_SET, scroll_y );
      if( p_wk->is_btn_move )   
      { 
        GFL_BG_SetScroll( BG_FRAME_BTN_M, GFL_BG_SCROLL_Y_SET, scroll_y );
      }
      if( p_wk->change_move_cnt-- == 0 )
      { 
        p_wk->change_move_seq = SEQ_END;
      }
      break;

    case SEQ_END:
      GFL_BG_SetScroll( BG_FRAME_KEY_M, GFL_BG_SCROLL_Y_SET, KEYBOARD_CHANGEMOVE_START_Y );
      GFL_BG_SetScroll( BG_FRAME_FONT_M, GFL_BG_SCROLL_Y_SET, KEYBOARD_CHANGEMOVE_START_Y );
      GFL_BG_SetScroll( BG_FRAME_BTN_M, GFL_BG_SCROLL_Y_SET, KEYBOARD_CHANGEMOVE_START_Y );

      p_wk->is_btn_move = FALSE;

      G2_BlendNone();
      p_wk->is_change_anm = FALSE;
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  キーボード  モード切替
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *  @param  mode                モード
 */
//-----------------------------------------------------------------------------
static void Keyboard_ChangeMode( KEYBOARD_WORK *p_wk, NAMEIN_INPUTTYPE mode )
{ 
  if( p_wk->mode != mode )
  { 
    //キー配列作成しなおし
    KEYMAP_Delete( &p_wk->keymap );
    KEYMAP_Create( &p_wk->keymap, mode, p_wk->p_keymap_handle, p_wk->heapID );


    //キー配列描画
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );
    KEYMAP_Print( &p_wk->keymap, &p_wk->util, p_wk->p_que, p_wk->p_font, p_wk->heapID );

    //モードにより、読み込むスクリーンを切り替え
    if( mode == NAMEIN_INPUTTYPE_QWERTY )
    { 
      GFL_BG_LoadScreenBuffer( BG_FRAME_KEY_M, p_wk->p_scr[0]->rawData, 0x800 );
      GFL_BG_SetVisible( BG_FRAME_BTN_M, FALSE );
    }
    else
    { 
      GFL_BG_LoadScreenBuffer( BG_FRAME_KEY_M, p_wk->p_scr[1]->rawData, 0x800 );
    }
    GFL_BG_LoadScreenReq( BG_FRAME_KEY_M );

    //前か後がQWERTYだった場合、カーソル位置変更
    if( mode == NAMEIN_INPUTTYPE_QWERTY || p_wk->mode == NAMEIN_INPUTTYPE_QWERTY )
    { 
      KEYMAP_TOOL_ChangeCursorPos( &p_wk->cursor, p_wk->mode, mode );
    }

    //変更したのでモード代入
    p_wk->mode  = mode;
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  キー入力
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *  @param  *p_req              リクエスト情報
 */
//-----------------------------------------------------------------------------
static BOOL Keyboard_KeyReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req )
{ 
  
  GFL_POINT pos;
  BOOL is_move  = FALSE;

  if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
  { 
    pos.x = 0;
    pos.y = -1;
    is_move = TRUE;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
  { 
    pos.x = 0;
    pos.y = 1;
    is_move = TRUE;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
  { 
    pos.x = -1;
    pos.y = 0;
    is_move = TRUE;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
  { 
    pos.x = 1;
    pos.y = 0;
    is_move = TRUE;
  }

  if( is_move )
  { 
    GFL_RECT rect;

#ifdef NAMEIN_KEY_TOUCH
    //以前がタッチ状態ならば、動かず、キー状態へ切り替わる
    if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_TOUCH )
    {
      if( KEYMAP_GetRange( &p_wk->keymap, &p_wk->cursor, &rect ) )
      { 
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
        KEYANM_Start( &p_wk->keyanm, KEYANM_TYPE_MOVE, &rect, p_wk->is_shift, p_wk->mode );
      }
    }
    else
#endif
    { 
      //以前がキー状態ならば、移動アニメ
      KEYMAP_MoveCursor( &p_wk->keymap, &p_wk->cursor, &pos );
      if( KEYMAP_GetRange( &p_wk->keymap, &p_wk->cursor, &rect ) )
      { 
        PMSND_PlaySE( NAMEIN_SE_MOVE_CURSOR );
        KEYANM_Start( &p_wk->keyanm, KEYANM_TYPE_MOVE, &rect, p_wk->is_shift, p_wk->mode );
      }
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  ボタン入力
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *  @param  *p_req              リクエスト情報
 */
//-----------------------------------------------------------------------------
static BOOL Keyboard_BtnReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req )
{
  BOOL ret  = FALSE;

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  { 
    p_req->anm_pos  = p_wk->cursor;
    p_req->is_push    = TRUE;

    ret = TRUE;
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL )
  { 
    GFL_POINT pos;
    //アニメ
    KEYMAP_GetCursorByKeyType( &p_wk->keymap, KEYMAP_KEYTYPE_DELETE, &p_req->anm_pos );
    //バックスペースになる
    p_req->type = KEYMAP_KEYTYPE_DELETE;
    ret = TRUE;
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  { 
    GFL_POINT pos;
    GFL_RECT  rect;
    //モードを循環移動
    p_req->type = KEYMAP_KEYTYPE_KANA + p_wk->mode + 1;
    if( p_req->type > KEYMAP_KEYTYPE_QWERTY )
    { 
      p_req->type = KEYMAP_KEYTYPE_KANA;
    }
    //アニメ
    KEYMAP_GetCursorByKeyType( &p_wk->keymap, p_req->type, &p_wk->cursor );
    p_req->anm_pos  = p_wk->cursor;
    //キー入力を決定
    ret = TRUE;
  }

#ifdef NAMEIN_KEY_TOUCH
  if( ret && GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_TOUCH )
  {
    GFL_RECT rect;
    if( KEYMAP_GetRange( &p_wk->keymap, &p_wk->cursor, &rect ) )
    { 
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      KEYANM_Start( &p_wk->keyanm, KEYANM_TYPE_MOVE, &rect, p_wk->is_shift, p_wk->mode );
      return FALSE;
    }
  }
#endif

  return ret;
}
//----------------------------------------------------------------------------
/**
 *  @brief  タッチ入力
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *  @param  *p_req              リクエスト情報
 */
//-----------------------------------------------------------------------------
static BOOL Keyboard_TouchReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req )
{ 
  GFL_POINT pos;
  u32 x, y;
  if( GFL_UI_TP_GetPointTrg( &x, &y ) )
  { 
    pos.x = x;
    pos.y = y;
    if( KEYMAP_GetTouchCursor( &p_wk->keymap, &pos, &p_wk->cursor ) )
    { 
      p_req->anm_pos    = p_wk->cursor;
      p_req->is_push    = TRUE;
      GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  決定
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *  @param  *p_req              リクエスト情報
 */
//-----------------------------------------------------------------------------
static void Keyboard_Decide( KEYBOARD_WORK *p_wk, const KEYBOARD_INPUT_REQUEST *cp_req )
{ 
  GFL_RECT rect;
  KEYMAP_KEYTYPE  type;

  if( cp_req->is_push )
  { 
    type  = KEYMAP_GetKeyInfo( &p_wk->keymap, &p_wk->cursor, &p_wk->code );
  }
  else
  { 
    type  = cp_req->type;
  }

  switch( type )
  { 
  case KEYMAP_KEYTYPE_STR:      //文字入力
    //QWERTYのときは変換入力
    //他のときは通常入力
    p_wk->input = p_wk->mode == NAMEIN_INPUTTYPE_QWERTY ?
      KEYBOARD_INPUT_CHANGESTR: KEYBOARD_INPUT_STR;
    p_wk->state = KEYBOARD_STATE_INPUT;
    break;
  case KEYMAP_KEYTYPE_SPACE:    //スペース入力
    p_wk->input = KEYBOARD_INPUT_SPACE;
    p_wk->state = KEYBOARD_STATE_INPUT;
    break;

  case KEYMAP_KEYTYPE_KANA:   //かなモード
    /* fallthrough */
  case KEYMAP_KEYTYPE_KATA:   //カナモード
    /* fallthrough */
  case KEYMAP_KEYTYPE_ABC:      //アルファベットモード
    /* fallthrough */
  case KEYMAP_KEYTYPE_KIGOU:    //記号モード
    /* fallthrough */
  case KEYMAP_KEYTYPE_QWERTY: //ローマ字モード
    if( Keyboard_StartMove( p_wk, type - KEYMAP_KEYTYPE_KANA ) )
    { 
      PMSND_PlaySE( NAMEIN_SE_CHANGE_MODE );

      p_wk->state = KEYBOARD_STATE_MOVE;
      p_wk->input = KEYBOARD_INPUT_CHAGETYPE;
    }
    break;

  case KEYMAP_KEYTYPE_DELETE: //けす
    p_wk->input = KEYBOARD_INPUT_BACKSPACE;
    break;

  case KEYMAP_KEYTYPE_DECIDE: //やめる

    PMSND_PlaySE( NAMEIN_SE_DECIDE );
    p_wk->input = KEYBOARD_INPUT_EXIT;
    break;

  case KEYMAP_KEYTYPE_SHIFT:    //シフト
    p_wk->input = KEYBOARD_INPUT_SHIFT;
    p_wk->is_shift  ^= 1;
    break;

  case KEYMAP_KEYTYPE_DAKUTEN:    //濁点
    p_wk->input = KEYBOARD_INPUT_DAKUTEN;
    break;

  case KEYMAP_KEYTYPE_HANDAKUTEN:   //半濁点
    p_wk->input = KEYBOARD_INPUT_HANDAKUTEN;
    break;
  }

  //決定アニメ
  if( KEYMAP_GetRange( &p_wk->keymap, &cp_req->anm_pos, &rect ) )
  { 
    KEYANM_Start( &p_wk->keyanm, KEYANM_TYPE_DECIDE, &rect, p_wk->is_shift, p_wk->mode );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  キー
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *  @param  *p_req              リクエスト情報
 */
//-----------------------------------------------------------------------------
static BOOL Keyboard_Move_KeyReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req )
{ 
  GFL_POINT pos;
  GFL_POINT next_cursor;
  BOOL is_move  = FALSE;

  if( !p_wk->is_btn_move )
  { 
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
    { 
      pos.x = 0;
      pos.y = -1;
      is_move = TRUE;
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
    { 
      pos.x = 0;
      pos.y = 1;
      is_move = TRUE;
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
    { 
      pos.x = -1;
      pos.y = 0;
      is_move = TRUE;
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
    { 
      pos.x = 1;
      pos.y = 0;
      is_move = TRUE;
    }
  }

  if( is_move )
  { 
    KEYMAP_KEYTYPE  type;
    GFL_RECT rect;

    //未来の座標を作成
    next_cursor = p_wk->cursor;
    KEYMAP_MoveCursor( &p_wk->keymap, &next_cursor, &pos );
    type  = KEYMAP_GetKeyInfo( &p_wk->keymap, &next_cursor, NULL );
    if( type == KEYMAP_KEYTYPE_KANA
        || type == KEYMAP_KEYTYPE_KATA
        || type == KEYMAP_KEYTYPE_ABC
        || type == KEYMAP_KEYTYPE_KIGOU
        || type == KEYMAP_KEYTYPE_QWERTY
      )
    { 

#ifdef NAMEIN_KEY_TOUCH
      //以前がタッチ状態ならば、動かず、キー状態へ切り替わる
      if( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_TOUCH )
      {
        if( KEYMAP_GetRange( &p_wk->keymap, &p_wk->cursor, &rect ) )
        { 
          //移動アニメ
          GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
          KEYANM_Start( &p_wk->keyanm, KEYANM_TYPE_MOVE, &rect, p_wk->is_shift, p_wk->mode );
        }
      }
      else
#endif
      { 
        KEYMAP_MoveCursor( &p_wk->keymap, &p_wk->cursor, &pos );
        //移動アニメ
        if( KEYMAP_GetRange( &p_wk->keymap, &p_wk->cursor, &rect ) )
        { 
          PMSND_PlaySE( NAMEIN_SE_MOVE_CURSOR );
          KEYANM_Start( &p_wk->keyanm, KEYANM_TYPE_MOVE, &rect, p_wk->is_shift, p_wk->mode );
        }
      }
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  ボタン
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *  @param  *p_req              リクエスト情報
 */
//-----------------------------------------------------------------------------
static BOOL Keyboard_Move_BtnReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req )
{ 
  BOOL ret  = FALSE;

  if( !p_wk->is_btn_move )
  { 
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
    { 
      p_req->anm_pos  = p_wk->cursor;
      p_req->is_push    = TRUE;

      ret = TRUE;
    }
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
    { 
      GFL_POINT pos;
      //モードを循環移動
      p_req->type = KEYMAP_KEYTYPE_KANA + p_wk->mode + 1;
      if( p_req->type > KEYMAP_KEYTYPE_QWERTY )
      { 
        p_req->type = KEYMAP_KEYTYPE_KANA;
      }
      //アニメ
      KEYMAP_GetCursorByKeyType( &p_wk->keymap, p_req->type, &p_wk->cursor );
      p_req->anm_pos  = p_wk->cursor;

      ret = TRUE;
    }

#ifdef NAMEIN_KEY_TOUCH
    if( ret && GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_TOUCH )
    {
      GFL_RECT  rect;
      if( KEYMAP_GetRange( &p_wk->keymap, &p_wk->cursor, &rect ) )
      { 
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
        KEYANM_Start( &p_wk->keyanm, KEYANM_TYPE_MOVE, &rect, p_wk->is_shift, p_wk->mode );  
        return FALSE;
      }
    }
#endif
  }

  return ret;
}
//----------------------------------------------------------------------------
/**
 *  @brief  タッチ
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *  @param  *p_req              リクエスト情報
 */
//-----------------------------------------------------------------------------
static BOOL Keyboard_Move_TouchReq( KEYBOARD_WORK *p_wk, KEYBOARD_INPUT_REQUEST *p_req )
{ 
  GFL_POINT pos;
  u32 x, y;

  //QWERTYはタッチできない
  //モード以外タッチできない
  if( !p_wk->is_btn_move )
  { 
    if( GFL_UI_TP_GetPointTrg( &x, &y ) )
    { 
      pos.x = x;
      pos.y = y;
      if( KEYMAP_GetTouchCursor( &p_wk->keymap, &pos, &p_wk->cursor ) )
      { 
        KEYMAP_KEYTYPE  type;
        type  = KEYMAP_GetKeyInfo( &p_wk->keymap, &p_wk->cursor, NULL );
        if( type == KEYMAP_KEYTYPE_KANA
            || type == KEYMAP_KEYTYPE_KATA
            || type == KEYMAP_KEYTYPE_ABC
            || type == KEYMAP_KEYTYPE_KIGOU
            || type == KEYMAP_KEYTYPE_QWERTY
            )
        { 
          p_req->anm_pos    = p_wk->cursor;
          p_req->is_push    = TRUE;
          GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  決定
 *
 *  @param  KEYBOARD_WORK *p_wk ワーク
 *  @param  *p_req              リクエスト情報
 */
//-----------------------------------------------------------------------------
static void Keyboard_Move_Decide( KEYBOARD_WORK *p_wk, const KEYBOARD_INPUT_REQUEST *cp_req )
{ 
  GFL_RECT rect;
  KEYMAP_KEYTYPE  type;

  if( cp_req->is_push )
  { 
    type  = KEYMAP_GetKeyInfo( &p_wk->keymap, &p_wk->cursor, &p_wk->code );
  }
  else
  { 
    type  = cp_req->type;
  }

  switch( type )
  {

  case KEYMAP_KEYTYPE_KANA:   //かなモード
    /* fallthrough */
  case KEYMAP_KEYTYPE_KATA:   //カナモード
    /* fallthrough */
  case KEYMAP_KEYTYPE_ABC:      //アルファベットモード
    /* fallthrough */
  case KEYMAP_KEYTYPE_KIGOU:    //記号モード
    /* fallthrough */
  case KEYMAP_KEYTYPE_QWERTY: //ローマ字モード
    if( Keyboard_StartMove( p_wk, type - KEYMAP_KEYTYPE_KANA ) )
    { 
      PMSND_PlaySE( NAMEIN_SE_CHANGE_MODE );

      p_wk->state = KEYBOARD_STATE_MOVE;
      p_wk->input = KEYBOARD_INPUT_CHAGETYPE;
    }
    break;
  }


  //決定アニメ
  if( KEYMAP_GetRange( &p_wk->keymap, &cp_req->anm_pos, &rect ) )
  { 
    KEYANM_Start( &p_wk->keyanm, KEYANM_TYPE_DECIDE, &rect, p_wk->is_shift, p_wk->mode );
  }
}
//=============================================================================
/**
 *          MSGWND
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージウィンドウ  初期化
 *  @param  MSGWND_WORK* p_wk ワーク
 *  @param  *p_font           フォント
 *  @param  *p_msg            メッセージデータ
 *  @param  *p_que            キュー
 *  @param  *p_word           単語
 *  @param  heapID            ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Init( MSGWND_WORK* p_wk, GFL_FONT *p_font, GFL_MSGDATA *p_msg, PRINT_QUE *p_que, WORDSET *p_word, HEAPID heapID )
{ 
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
  //初期化
  p_wk->clear_chr = 0xF;
  p_wk->p_font    = p_font;
  p_wk->p_msg     = p_msg;
  p_wk->p_que     = p_que;
  p_wk->p_word    = p_word;

  //バッファ作成
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( 255, heapID );

  //BMPWIN作成
  p_wk->p_bmpwin  = GFL_BMPWIN_Create( BG_FRAME_TEXT_S, MSGWND_BMPWIN_X, MSGWND_BMPWIN_Y, MSGWND_BMPWIN_W, MSGWND_BMPWIN_H, PLT_BG_FONT_S, GFL_BMP_CHRAREA_GET_B );

  //フレーム書き込み
  BmpWinFrame_Write( p_wk->p_bmpwin, WINDOW_TRANS_OFF, MSGWND_FRAME_CHR, PLT_BG_FRAME_S );

  //プリントキュー設定
  PRINT_UTIL_Setup( &p_wk->util, p_wk->p_bmpwin );

  //転送
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );
  GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );
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
  //BMPWIN破棄
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );

  //バッファ破棄
  GFL_STR_DeleteBuffer( p_wk->p_strbuf );

  //クリア
  GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージウィンドウ  プリント開始
 *
 *  @param  MSGWND_WORK* p_wk ワーク
 *  @param  strID             文字ID
 */
//-----------------------------------------------------------------------------
static void MSGWND_Print( MSGWND_WORK* p_wk, u32 strID )
{ 
  //一端消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );  

  //文字列作成
  GFL_MSG_GetString( p_wk->p_msg, strID, p_wk->p_strbuf );

  //文字列描画
  PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strbuf, p_wk->p_font );
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージウィンドウ  単語登録してプリント開始
 *
 *  @param  MSGWND_WORK *p_wk ワーク
 *  @param  strID       文字ID
 *  @param  mons_no     モンスター番号
 *  @param  form        フォルム
 *  @param  heapID      バッファ作成用ヒープID
 */
//-----------------------------------------------------------------------------
static void MSGWND_ExpandPrintPoke( MSGWND_WORK *p_wk, u32 strID, u16 mons_no, u16 form, HEAPID heapID )
{ 
  //登録
  { 
    POKEMON_PARAM *p_pp;

    //PP作成
    p_pp  = PP_Create( mons_no, 1, 0, heapID );
    PP_Put( p_pp, ID_PARA_form_no, form );

    //プリント
    MSGWND_ExpandPrintPP( p_wk, strID, p_pp );

    //バッファクリア
    GFL_HEAP_FreeMemory( p_pp );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージウィンドウ  PPの単語登録してプリント開始
 *
 *  @param  MSGWND_WORK *p_wk     ワーク
 *  @param  strID                 文字ID
 *  @param  POKEMON_PARAM *cp_pp  ポケモンパラム
 */
//-----------------------------------------------------------------------------
static void MSGWND_ExpandPrintPP( MSGWND_WORK *p_wk, u32 strID, const POKEMON_PARAM *cp_pp )
{ 
  //単語登録
  WORDSET_RegisterPokeMonsName( p_wk->p_word, 0, cp_pp );

  //プリント
  { 
    STRBUF  *p_strbuf;

    //一端消去
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );  

    //文字列作成
    p_strbuf  = GFL_MSG_CreateString( p_wk->p_msg, strID );

    //単語展開
    WORDSET_ExpandStr( p_wk->p_word, p_wk->p_strbuf, p_strbuf );  

    //文字列描画
    PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strbuf, p_wk->p_font );

    //バッファクリア
    GFL_STR_DeleteBuffer( p_strbuf );
  }

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
 *          PS(PrintStream)
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  プリントストリーム  初期化
 *  @param  PS_WORK* p_wk     ワーク
 *  @param  heapID            ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void PS_Init( PS_WORK* p_wk, HEAPID heapID )
{
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(PS_WORK) );

  // GFL_TCBLシステム
  p_wk->p_tcblsys = GFL_TCBL_Init( heapID, heapID, 1, 0 );
}

//----------------------------------------------------------------------------
/**
 *  @brief  プリントストリーム  破棄
 *
 *  @param  PS_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void PS_Exit( PS_WORK* p_wk )
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
 *  @brief  ポケモン捕獲でボックス転送になった時のプリントストリームの準備
 *
 *  @param  PS_WORK* p_wk            PSワーク
 *  @param  MSGWND_WORK* p_msgwnd_wk MSGWNDワーク
 *  @param  NAMEIN_PARAM* p_param    NAMEIN_PARAMワーク
 *  @param  heapID                   ヒープID
 */
//-----------------------------------------------------------------------------
static void PS_SetupBox( PS_WORK* p_wk, MSGWND_WORK* p_msgwnd_wk, NAMEIN_PARAM* p_param, HEAPID heapID )
{
  const u32 tcbpri = 2;
  
  const u32 buf_id_nickname = 0;
  const u32 buf_id_box = 1;
  WORDSET *wordset;  // ポケモンのニックネーム、ボックス名を入れておく

  // WORDSET生成
  wordset = WORDSET_Create( heapID );
  
  // ポケモンのニックネームを得る
  if( NAMEIN_IsCancel(p_param) )
  {
    WORDSET_RegisterPokeNickName( wordset, buf_id_nickname, p_param->pp );
  }
  else
  {
    const u32  sex           = 0;  // WORDSET_RegisterPokeNickNameでは必要ないので、
    const BOOL singular_flag = 0;  // static void InitParam(WORDSET_PARAM* param)を参考に数値設定。
    const u32  lang          = 0;
    
    STRBUF *strbuf = GFL_STR_CreateCopyBuffer( p_param->strbuf, heapID );
    NAMEIN_CopyStr( p_param, strbuf );
    WORDSET_RegisterWord( wordset, buf_id_nickname, strbuf, sex, singular_flag, lang );
    GFL_STR_DeleteBuffer( strbuf );
  }

  // ボックス名を得る
  WORDSET_RegisterBoxName( wordset, buf_id_box, p_param->box_manager, p_param->box_tray );

  // 文字描画関連の設定をクリアする
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
  }
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_msgwnd_wk->p_bmpwin ), p_msgwnd_wk->clear_chr );
  GFL_STR_ClearBuffer( p_msgwnd_wk->p_strbuf );
  
  // 文字描画関連の設定を行う
  WORDSET_ExpandStr( wordset, p_msgwnd_wk->p_strbuf, p_param->box_strbuf );
  p_wk->p_stream = PRINTSYS_PrintStream( p_msgwnd_wk->p_bmpwin, 0, 0, p_msgwnd_wk->p_strbuf,
                                         p_msgwnd_wk->p_font, MSGSPEED_GetWait(),
                                         p_wk->p_tcblsys, tcbpri, heapID, p_msgwnd_wk->clear_chr );

  // その他の設定を行う
  p_wk->next_seq_function = SEQFUNC_FadeIn;
  p_wk->wait_max = 60;
  p_wk->wait_count = 0;
 
  // WORDSET破棄
  WORDSET_Delete( wordset );
}

//=============================================================================
/**
 *    アイコン
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  ICON_WORK *p_wk ワーク
 *  @param  type    アイコンの種類
 *  @param  param1  引数１  アイコンの種類によって違う以下参照
 *  @param  param2  引数２  アイコンの種類によって違う以下参照
 *  @param  *p_unit OBJ読みこみ用CLUNIT
 *  @param  heapID  ヒープID
 *
 *  @note{  
 *    param1,param2について
 *    ICON_TYPE_HEROの場合
 *      param1　…  性別
 *    ICON_TYPE_PERSONの場合
 *      param1  …  フィールドOBJの種類
 *      param2  …  未使用
 *    ICON_TYPE_POKEの場合
 *      param1  …  モンスター番号
 *      param2  …  フォルム
 *    ICON_TYPE_BOXの場合
 *      param1  …  未使用
 *      param2  …  未使用
 *  }
 */
//-----------------------------------------------------------------------------
static void ICON_Init( ICON_WORK *p_wk, ICON_TYPE type, u32 param1, u32 param2, GFL_CLUNIT *p_unit, HEAPID heapID )
{ 
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(ICON_WORK) );

  //タイプ別の読みこみ
  { 
    ARCID     arcID;
    ARCDATID  plt, chr, cel, anm;
    u8        plt_src_ofs = 0;
    BOOL      is_comp = FALSE;
    ARCHANDLE * p_handle;

    switch( type )
    { 
    case ICON_TYPE_RIVAL:
      /* fallthrough */
    case ICON_TYPE_HERO:
      arcID = APP_COMMON_GetArcId();
      plt   = APP_COMMON_GetNull4x4PltArcIdx();
      chr   = APP_COMMON_GetNull4x4CharArcIdx();
      cel   = APP_COMMON_GetNull4x4CellArcIdx( APP_COMMON_MAPPING_128K );
      anm   = APP_COMMON_GetNull4x4AnimeArcIdx( APP_COMMON_MAPPING_128K );
      break;
    case ICON_TYPE_PERSON:
      arcID = ARCID_WIFIUNIONCHAR;
      plt   = NARC_wifi_unionobj_wf_match_top_trainer_NCLR;
      plt_src_ofs = sc_wifi_unionobj_plt[param1];
      chr   = NARC_wifi_unionobj_front00_NCGR+param1;
      cel   = NARC_wifi_unionobj_front00_NCER;
      anm   = NARC_wifi_unionobj_front00_NANR;
      break;
    case ICON_TYPE_POKE:
      arcID = ARCID_POKEICON;
      plt   = POKEICON_GetPalArcIndex();
      chr   = POKEICON_GetCgxArcIndexByMonsNumber( param1, param2, FALSE );
      cel   = POKEICON_GetCellArcIndex();
      anm   = POKEICON_GetAnmArcIndex();
      is_comp = TRUE;
      break;
    case ICON_TYPE_BOX:
      arcID = ARCID_NAMEIN_GRA;
      plt   = NARC_namein_gra_name_obj_NCLR;
      chr   = NARC_namein_gra_name_obj_NCGR;
      cel   = NARC_namein_gra_name_obj_NCER;
      anm   = NARC_namein_gra_name_obj_NANR;
      break;
    default:
      GF_ASSERT(0);
    }

    //読みこみ
    p_handle  = GFL_ARC_OpenDataHandle( arcID, heapID );
    if( is_comp )
    { 
      p_wk->plt = GFL_CLGRP_PLTT_RegisterComp( p_handle, 
          plt, CLSYS_DRAW_MAIN, PLT_OBJ_ICON_M*0x20, heapID );
    }
    else
    { 
      p_wk->plt = GFL_CLGRP_PLTT_RegisterEx( p_handle, 
          plt, CLSYS_DRAW_MAIN, PLT_OBJ_ICON_M*0x20, plt_src_ofs, 1, heapID );
    }
    p_wk->cel = GFL_CLGRP_CELLANIM_Register( p_handle,
        cel, anm, heapID );
    p_wk->ncg = GFL_CLGRP_CGR_Register( p_handle,
        chr, FALSE, CLSYS_DRAW_MAIN, heapID );
    GFL_ARC_CloseDataHandle( p_handle );
  }

  //CLWK作成
  { 
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
    cldata.pos_x  = ICON_POS_X;
    cldata.pos_y  = ICON_POS_Y;
    p_wk->p_clwk  =   GFL_CLACT_WK_Create( p_unit,
        p_wk->ncg,
        p_wk->plt,
        p_wk->cel,
        &cldata,
        CLSYS_DEFREND_MAIN,
        heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk, TRUE );
  }

  //タイプ別設定
  { 
    switch( type )
    { 
    case ICON_TYPE_HERO:
      if( param1 == PTL_SEX_MALE )
      { 
        CLWK_TransNSBTX( p_wk->p_clwk, ARCID_MMDL_RES, NARC_fldmmdl_mdlres_hero_nsbtx, 3, NSBTX_DEF_SX, NSBTX_DEF_SY, 0, CLSYS_DEFREND_MAIN, heapID );
      }
      else 
      { 
        CLWK_TransNSBTX( p_wk->p_clwk, ARCID_MMDL_RES, NARC_fldmmdl_mdlres_heroine_nsbtx, 3, NSBTX_DEF_SX, NSBTX_DEF_SY, 0, CLSYS_DEFREND_MAIN, heapID );
      }
      break;
    case ICON_TYPE_RIVAL:
      //@todoライバルのリソースとの下向き番号　主人公は３だが…。
      CLWK_TransNSBTX( p_wk->p_clwk, ARCID_MMDL_RES, NARC_fldmmdl_mdlres_hero_nsbtx, 3, NSBTX_DEF_SX, NSBTX_DEF_SY, 0, CLSYS_DEFREND_MAIN, heapID );
    case ICON_TYPE_PERSON:
      GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk, 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );
      break;
    case ICON_TYPE_POKE:
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk, POKEICON_ANM_HPMAX );
      GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk, 
          POKEICON_GetPalNum( param1, param2, FALSE ),
          CLWK_PLTTOFFS_MODE_OAM_COLOR );
      break;
    case ICON_TYPE_BOX:
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk, 2 );
      break;
    default:
      GF_ASSERT(0);
    }
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  アイコン破棄
 *
 *  @param  ICON_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void ICON_Exit( ICON_WORK *p_wk )
{ 
  //リソース破棄
  { 
    GFL_CLGRP_PLTT_Release( p_wk->plt );
    GFL_CLGRP_CGR_Release( p_wk->ncg );
    GFL_CLGRP_CELLANIM_Release( p_wk->cel );
  }
  //CLWK消去
  GFL_CLACT_WK_Remove( p_wk->p_clwk );
  //クリア
  GFL_STD_MemClear( p_wk, sizeof(ICON_WORK) );
}
//----------------------------------------------------------------------------
/**
 *  @brief  アイコン  メイン処理
 *
 *  @param  ICON_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void ICON_Main( ICON_WORK *p_wk )
{ 
  enum
  { 
    SEQ_MAIN, //タッチ可能状態
    SEQ_ANM,  //アイコンアニメ
  };

  switch( p_wk->seq )
  { 
  case SEQ_MAIN:
    { 

      u32 x, y;
      GFL_CLACTPOS  clpos;
      GFL_CLACT_WK_GetPos( p_wk->p_clwk, &clpos, CLSYS_DRAW_MAIN );

      p_wk->is_trg   = FALSE;

      if( GFL_UI_TP_GetPointTrg( &x, &y) )
      { 
        GFL_POINT pos;
        GFL_RECT  rect;

        rect.left   = clpos.x - ICON_WIDTH/2;
        rect.top    = clpos.y - ICON_HEIGHT/2;
        rect.right  = clpos.x + ICON_WIDTH/2;
        rect.bottom = clpos.y + ICON_HEIGHT/2;

        pos.x = x;
        pos.y = y;

        if( COLLISION_IsRectXPos( &rect, &pos ) )
        {
          Icon_StartMove( p_wk );
          p_wk->seq = SEQ_ANM;
        }
      }
    }
    break;

  case SEQ_ANM:
    if( Icon_MainMove( p_wk ) )
    { 
      p_wk->is_trg  = TRUE;
      p_wk->seq = SEQ_MAIN;
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  アイコン入力状態
 *
 *  @param  const ICON_WORK *cp_wk  ワーク
 *
 *  @return TRUE入力  FALSE入力していない
 */
//-----------------------------------------------------------------------------
static BOOL ICON_IsTrg( const ICON_WORK *cp_wk )
{ 
  return cp_wk->is_trg;
}
//----------------------------------------------------------------------------
/**
 *  @brief  アイコン移動開始
 *
 *  @param  ICON_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void Icon_StartMove( ICON_WORK *p_wk )
{ 
  p_wk->sync  = 0;
  p_wk->is_move_start = TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  アイコン移動処理
 *
 *  @param  ICON_WORK *p_wk ワーク
 *
 *  @return TRUEアイコン移動終了  FALSEアイコン移動中
 */
//-----------------------------------------------------------------------------
static BOOL Icon_MainMove( ICON_WORK *p_wk )
{ 
  if( p_wk->is_move_start )
  { 
    u16   now;
    fx16  sin;
    GFL_CLACTPOS  clpos;

    now = ICON_MOVE_START + ICON_MOVE_DIF * p_wk->sync++ / ICON_MOVE_SYNC;
    sin = FX_SinIdx( now );

    clpos.x = ICON_POS_X;
    clpos.y = ICON_POS_Y - ((ICON_MOVE_Y*sin) >> FX16_SHIFT);

    GFL_CLACT_WK_SetPos( p_wk->p_clwk, &clpos, CLSYS_DRAW_MAIN );
  
    NAGI_Printf( "sin 0x%x y %d \n", sin, ((ICON_MOVE_Y*sin) >> FX16_SHIFT) );

    if( p_wk->sync > ICON_MOVE_SYNC )
    { 
      p_wk->is_move_start = FALSE;
    }

    return FALSE;
  }

  return TRUE;
}
//=============================================================================
/**
 *    その他
 */
//=============================================================================
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
//----------------------------------------------------------------------------
/**
 *  @brief  デフォルト名  を作成し返す
 *
 *  @param  p_wk              ワーク
 *  @param  NAMEIN_MODE mode  モード
 *  @param  HEAPID            ヒープ
 *  @retval STRBUF
 */
//-----------------------------------------------------------------------------
static STRBUF* DEFAULTNAME_CreateStr( const NAMEIN_WORK *cp_wk, NAMEIN_MODE mode, HEAPID heapID )
{ 
  STRBUF *p_msg_str;
  switch( mode )
  { 
  case NAMEIN_MYNAME:
    return GFL_MSG_CreateString( cp_wk->p_msg, NAMEIN_DEF_NAME_000 + GFUser_GetPublicRand( NAMEIN_DEFAULT_NAME_MAX ) );

  case NAMEIN_POKEMON:
    return GFL_MSG_CreateString( GlobalMsg_PokeName, cp_wk->p_param->mons_no );

  case NAMEIN_BOX:
    return GFL_STR_CreateCopyBuffer( cp_wk->p_param->strbuf, heapID );

  case NAMEIN_RIVALNAME:
    //@todoライバル名
    return GFL_MSG_CreateString( cp_wk->p_msg, NAMEIN_DEF_NAME_000 + GFUser_GetPublicRand( NAMEIN_DEFAULT_NAME_MAX ) );
  }

  return NULL;
}

//----------------------------------------------------------------------------
/**
 *  @brief  入力が終了したときの 文字列を確定する or キャンセルを確定する 処理
 *
 *  @param  p_wk              ワーク
 */
//-----------------------------------------------------------------------------
static void FinishInput( NAMEIN_WORK *p_wk )
{
  //一端キャンセルフラグをOFF
  p_wk->p_param->cancel = FALSE;


  //元の文字列と入力文字列の一致確認処理
  //一致していればキャンセルとみなす
  { 
    STRBUF *p_src_str;
    p_src_str = GFL_STR_CreateCopyBuffer( p_wk->p_param->strbuf, HEAPID_NAME_INPUT );

    //今回の文字列を返す
    STRINPUT_CopyStr( &p_wk->strinput, p_wk->p_param->strbuf );

    //以前の文字列と一致したもしくはならば、キャンセルとみなす
    { 
      if( GFL_STR_CompareBuffer( p_src_str, p_wk->p_param->strbuf ) )
      { 
        p_wk->p_param->cancel = TRUE;
      }
    }
    GFL_STR_DeleteBuffer( p_src_str );
  }

  //入力数が0ならば
  //主人公入力ならばデフォルト名をいれ、
  //そのほかならキャンセルとみなす
  if( GFL_STR_GetBufferLength( p_wk->p_param->strbuf ) == 0 )
  { 
    if( p_wk->p_param->mode == NAMEIN_MYNAME ||
        p_wk->p_param->mode == NAMEIN_RIVALNAME )
    {
      GF_ASSERT( 0 );
    }
    else
    { 
      p_wk->p_param->cancel = TRUE;
    }
  }
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
 *  @brief  描画完了待ち
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_WaitPrint( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  NAMEIN_WORK *p_wk = p_param;

  BOOL ret;

  ret = KEYBOARD_PrintMain( &p_wk->keyboard );
  ret |= STRINPUT_PrintMain( &p_wk->strinput );
  ret |= MSGWND_PrintMain( &p_wk->msgwnd );

  if( ret )
  { 
    SEQ_SetNext( p_seqwk, SEQFUNC_FadeOut );
  }
}
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
    SEQ_SetNext( p_seqwk, SEQFUNC_End );
    break;

  default:
    GF_ASSERT(0);
  }

}

//----------------------------------------------------------------------------
/**
 *  @brief  プリントストリーム
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_PrintStream( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
  enum
  {
    SEQ_PS_STREAM,
    SEQ_PS_WAIT,
    SEQ_PS_EXIT,
  };

  NAMEIN_WORK *p_wk = p_param;
  PS_WORK *p_ps_wk = &(p_wk->ps);

  switch( *p_seq )
  {
  case SEQ_PS_STREAM:
    {
      PRINTSTREAM_STATE state;
      GFL_TCBL_Main( p_ps_wk->p_tcblsys );
      state = PRINTSYS_PrintStreamGetState( p_ps_wk->p_stream );
      if( state == PRINTSTREAM_STATE_DONE )
      {
        *p_seq = SEQ_PS_WAIT;
      }
    }
    break;
  case SEQ_PS_WAIT:
    {
      if( p_ps_wk->wait_count >= p_ps_wk->wait_max )
      {
        *p_seq = SEQ_PS_EXIT;
      }
      else
      {
        p_ps_wk->wait_count++;
      }
    }
    break;
  case SEQ_PS_EXIT:
    {
      SEQ_SetNext( p_seqwk, p_ps_wk->next_seq_function );
    }
    break;
  default:
    {
      GF_ASSERT(0);
    }
    break;
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
  NAMEIN_WORK *p_wk = p_param;

  //入力処理
  { 
    KEYBOARD_INPUT  input;
    BOOL            is_shift;
    STRCODE         code;
    input     = KEYBOARD_GetInput( &p_wk->keyboard, &code );
    is_shift  = KEYBOARD_IsShift( &p_wk->keyboard );

    switch( input )
    { 
    case KEYBOARD_INPUT_STR:        //文字入力
      STRINPUT_SetStr( &p_wk->strinput, code );
      break;
    case KEYBOARD_INPUT_CHANGESTR:  //変換文字入力
      STRINPUT_SetChangeStr( &p_wk->strinput, code, is_shift );
      break;
    case KEYBOARD_INPUT_DAKUTEN:    //濁点
      STRINPUT_SetChangeSP( &p_wk->strinput, STRINPUT_SP_CHANGE_DAKUTEN );
      break;
    case KEYBOARD_INPUT_HANDAKUTEN: //半濁点
      STRINPUT_SetChangeSP( &p_wk->strinput, STRINPUT_SP_CHANGE_HANDAKUTEN );
      break;
    case KEYBOARD_INPUT_BACKSPACE:  //一つ前に戻る
      STRINPUT_BackSpace( &p_wk->strinput );
      break;
    case KEYBOARD_INPUT_CHAGETYPE:  //入力タイプ変更
      STRINPUT_DeleteChangeStr( &p_wk->strinput );
      break;
    case KEYBOARD_INPUT_EXIT:       //終了  
      //変換文字列を消す
      STRINPUT_DeleteChangeStr( &p_wk->strinput );
      //自分の名前とライバル名入力で、何もいれなかったら、
      //デフォルト名をいれる
      if( p_wk->p_param->mode == NAMEIN_MYNAME ||
          p_wk->p_param->mode == NAMEIN_RIVALNAME )
      { 
        if( STRINPUT_GetStrLength( &p_wk->strinput ) == 0 )
        { 
          STRBUF  *p_default;
          p_default = DEFAULTNAME_CreateStr( p_wk, p_wk->p_param->mode, HEAPID_NAME_INPUT );

          //デフォルト名入力
          if( p_default != NULL )
          { 
            STRINPUT_SetLongStr( &p_wk->strinput, GFL_STR_GetStringCodePointer(p_default) );
            GFL_STR_DeleteBuffer( p_default );
          }
        }
      }

      // 入力が終了したときの 文字列を確定する or キャンセルを確定する 処理
      FinishInput( p_wk );

      if( p_wk->p_param->mode==NAMEIN_POKEMON && p_wk->p_param->box_strbuf!=NULL )  // ポケモン捕獲時でボックス転送
      {
        PS_SetupBox( &p_wk->ps, &p_wk->msgwnd, p_wk->p_param, HEAPID_NAME_INPUT );
        SEQ_SetNext( p_seqwk, SEQFUNC_PrintStream );
      }
      else
      {
        SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
      }
      break;
    case KEYBOARD_INPUT_NONE:       //何もしていない
  
      //アイコンタッチしたら、デフォルト名を入れる
      if( ICON_IsTrg( &p_wk->icon ) )
      { 
        STRBUF  *p_default;
        p_default = DEFAULTNAME_CreateStr( p_wk, p_wk->p_param->mode, HEAPID_NAME_INPUT );

        //一端全てけす
        STRINPUT_Delete( &p_wk->strinput );

        //デフォルト名入力
        if( p_default != NULL )
        { 
          STRINPUT_SetLongStr( &p_wk->strinput, GFL_STR_GetStringCodePointer(p_default) );
          GFL_STR_DeleteBuffer( p_default );
        }

      }
      break;
    case KEYBOARD_INPUT_SHIFT:      //シフト
      /*  何もしない */
      break;
    case KEYBOARD_INPUT_SPACE:      //スペース入力
      STRINPUT_DecideChangeStr( &p_wk->strinput );
      STRINPUT_SetStr( &p_wk->strinput, code );
      break;
    default:
      GF_ASSERT(0);
    }
  }

  //メイン処理
  KEYBOARD_Main( &p_wk->keyboard, &p_wk->strinput );
  STRINPUT_Main( &p_wk->strinput );
  ICON_Main( &p_wk->icon );
}

//----------------------------------------------------------------------------
/**
 *  @brief  設定画面終了処理
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_End( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{ 
  NAMEIN_WORK *p_wk = p_param;

  //SEQFUNC_Mainへ移動FinishInput( p_wk );

  //入力モードを保存する
  { 
    MISC *p_misc;
    NAMEIN_INPUTTYPE input_type;
    p_misc  = SaveData_GetMisc( SaveControl_GetPointer() );
    input_type  = KEYBOARD_GetInputType( &p_wk->keyboard );
    MISC_SetNameInMode( p_misc, p_wk->p_param->mode, input_type );
  }

  //終了
  if( p_wk->p_param->p_intr_sv )
  { 
    SEQ_SetNext( p_seqwk, SEQFUNC_SaveEnd );
  }
  else
  { 
    SEQ_End( p_seqwk );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  セーブ終了待ち
 *
 *  @param  SEQ_WORK *p_seqwk シーケンスワーク
 *  @param  *p_seq          シーケンス
 *  @param  *p_param        ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_SaveEnd( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{ 
  enum
  { 
    SEQ_SAVE_SUSPEND,
    SEQ_SAVE_WAIT,
  };

  NAMEIN_WORK *p_wk = p_param;

  switch( *p_seq )
  { 
  case SEQ_SAVE_SUSPEND:
    IntrSave_ReqSuspend( p_wk->p_param->p_intr_sv );
    *p_seq  = SEQ_SAVE_WAIT;
    break;

  case SEQ_SAVE_WAIT:
    if(IntrSave_CheckSuspend(p_wk->p_param->p_intr_sv) == TRUE){
      SEQ_End( p_seqwk );
    }
    break;
  } 
}
