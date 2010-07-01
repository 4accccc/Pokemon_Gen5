//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		namein_local.h
 *	@brief  ローカルヘッダ
 *	@author	Toru=Nagihashi
 *	@date		2010.03.31
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
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
#include "system/bmp_oam.h"

//  module
#include "app/app_menu_common.h"
#include "pokeicon/pokeicon.h"
#include "system/prof_word.h"

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
/// マクロ
//=====================================
//回り込み
#define MATH_ROUND(x,min,max)   (x < min? max: x > max ? min: x)

//-------------------------------------
/// ヒープサイズ
//=====================================
#define NAMEIN_HEAP_HIGH_SIZE (0x70000)
#define NAMEIN_HEAP_LOW_SIZE  (0x40000)

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
  PLT_OBJ_SEX_M      = PLT_OBJ_ICON_M+POKEICON_PAL_MAX,  //性別文字色

  //サブBG
  PLT_BG_BACK_S       = 0,  //背景
  PLT_BG_FRAME_S      = 14,  //WINDOWフレーム
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
//カーソルの数
enum
{
  OBJ_CURSOR_L,
  OBJ_CURSOR_R,
  OBJ_CURSOR_MAX,
};

//-------------------------------------
/// CLWK
//=====================================
typedef enum
{
  CLWKID_BAR_TOP,
  CLWKID_BAR_END  = CLWKID_BAR_TOP + OBJ_BAR_NUM,
  CLWKID_PC,
  CLWKID_CURSOR_TOP,
  CLWKID_CURSOR_END = CLWKID_CURSOR_TOP + OBJ_CURSOR_MAX,
  CLWKID_MAX,
} CLWKID;

//-------------------------------------
/// STRINPUT
//=====================================
//文字列長
#define STRINPUT_STR_LEN      (10)     //最大入力文字列長
#define STRINPUT_CHANGE_LEN   (3)      //変換文字列長
//BMPWINサイズ
#define STRINPUT_BMPWIN_X     (8)
#define STRINPUT_BMPWIN_Y     (2)
#define STRINPUT_BMPWIN_W     (24)
#define STRINPUT_BMPWIN_H     (2)
//文字描画位置
#define STRINPUT_STR_START_X  (0)
#define STRINPUT_STR_OFFSET_X (14)
#define STRINPUT_STR_Y        (0)
//BARの位置
#define STRINPUT_BAR_START_X  (STRINPUT_BMPWIN_X*8)
#define STRINPUT_BAR_OFFSET_X (STRINPUT_STR_OFFSET_X)
#define STRINPUT_BAR_Y        (36)
//特殊文字変換
typedef enum
{ 
  STRINPUT_SP_CHANGE_DAKUTEN,     //濁点変換
  STRINPUT_SP_CHANGE_HANDAKUTEN,  //半濁点変換
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
  KEYBOARD_INPUT_AUTOCHANGE,   //Rボタン用自動変換
  KEYBOARD_INPUT_LAST,        //最後まで入力した
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
#define ICON_POS_X  (34)
#define ICON_POS_Y  (20)
#define ICON_POS_SEX_X  (46)
#define ICON_WIDTH  (32)
#define ICON_HEIGHT (32)
#define ICON_POS_POKE_X  (46-32/2)

//アイコンの動作
#define ICON_MOVE_SYNC  (8)
#define ICON_MOVE_Y     (12)
#define ICON_MOVE_START ((0*0xffff) / 360)
#define ICON_MOVE_END   ((180*0xffff) / 360)
#define ICON_MOVE_DIF   ( ICON_MOVE_END - ICON_MOVE_START )

//アイコンの種類
typedef enum
{
  ICON_TYPE_HERO,
  ICON_TYPE_RIVAL,
  ICON_TYPE_PERSON,
  ICON_TYPE_POKE,
  ICON_TYPE_BOX,
  ICON_TYPE_HATENA,
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
  GFL_CLWK    *p_cursor[OBJ_CURSOR_MAX];  //カーソルOBJ
  GFL_CLACTPOS  cursor_pos[OBJ_CURSOR_MAX];//OBJ用座標
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
  NAMEIN_STRCHANGE_EX *p_changedata_ex;

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

  //以下性別文字用OBJ
  u32               font_plt;
  STRBUF            *p_strbuf;  //文字バッファ
  BMPOAM_ACT_PTR		p_bmpoam_wk;  //性別用
  GFL_BMP_DATA      *p_bmp;     //文字用BMP
  BOOL  is_trans_req;
  PRINT_QUE *p_que;
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

  //共通で使うBMPOAM
  BMPOAM_SYS_PTR p_bmpoam_sys;

  //引数
  NAMEIN_PARAM  *p_param;

  //不許可文字の場合のメッセージ
  BOOL  is_illegal_msg;

  //不正文字チェックルーチン
  PROF_WORD_WORK  *p_prof;

  //初期表示文字列（キャンセル時に使うデフォルト文字とは別）
  //海外文字等入力できない文字があったときは、デフォルト文字は存在していても
  //ここは無しになる
  STRBUF  *p_initdraw_str;

} NAMEIN_WORK;
