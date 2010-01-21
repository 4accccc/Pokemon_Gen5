//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *  GAME FREAK inc.
 *
 *  @file   pl_wifi_note.c
 *  @brief    友達手帳  プラチナバージョン
 *  @author   tomoya takahshi
 *  @data   2007.07.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"

#include "msg/msg_wifi_note.h"

#include "sound/pm_sndsys.h"
#include "system/bmp_menu.h"
#include "system/bmp_menulist.h"
#include "system/bmp_winframe.h"
#include "system/touch_subwindow.h"
#include "system/selbox.h"
#include "system/wipe.h"
#include "print/wordset.h"
#include "print/global_msg.h"
#include "print/str_tool.h"
#include "gamesystem\msgspeed.h"

#include "savedata/wifilist.h"

#include "app/wifi_note.h"
#include "app/codein.h"
#include "app/name_input.h"
#include "net_app/wifi2dmap/wf2dmap_common.h"
#include "net_app/wifi2dmap/wifi_2dchar.h"
#include "net_app/union/union_beacon_tool.h"
#include "net_app/union/union_gra_tool.h"
#include "net/dwc_rapfriend.h"

#include "system/blink_palanm.h"
#include "app/app_menu_common.h"
#include "app/app_taskmenu.h"
#include "ui/touchbar.h"

#include "wifi_note.naix"
#include "wifi_note_snd.h"


#define USE_SEL_BOX (1)
#define USE_FRONTIER_DATA (0)
#define NOTE_TEMP_COMMENT (0)
#define NOTE_DEBUG (1)

#pragma mark[>define
//-----------------------------------------------------------------------------
/**
 *          定数宣言
*/
//-----------------------------------------------------------------------------


// WIFI2DMAPシステムオーバーレイ
FS_EXTERN_OVERLAY(wifi2dmap);

//-------------------------------------
/// 文字列バッファサイズ
//=====================================
#define WFNOTE_STRBUF_SIZE  (128)

//-------------------------------------
/// CLACT
//=====================================
#define CLACT_RESOURCE_NUM    ( 4 ) // リソースマネージャ数
#define CLACT_WKNUM       ( 64 )  // ワーク数
#define CLACT_RESNUM      ( 64 )  // 読み込むリソース数
#define CLACT_MAIN2_REND_Y_S32  ( 0 ) // スクロールさせないサーフェース面のY座標位置
#define CLACT_MAIN2_REND_Y    ( 0 ) // スクロールさせないサーフェース面のY座標位置
#define CLACT_MAIN_VRAM_SIZ   ( 0x14000 )
#define CLACT_SUB_VRAM_SIZ    ( 0 )

// パレット定義
#define	CLACT_PALNUM_WIFINOTE		( 0 )
#define	CLACT_PALSIZ_WIFINOTE		( 5 )
#define	CLACT_PALNUM_TB					( CLACT_PALNUM_WIFINOTE + CLACT_PALSIZ_WIFINOTE )
#define	CLACT_PALSIZ_TB					( 1 )

//-------------------------------------
/// VRAMTRANFER_MAN
//=====================================
#define VRAMTR_MAN_NUM    (16)

//-------------------------------------
/// BG
//=====================================
#define DFRM_BACK			(GFL_BG_FRAME0_M)
#define DFRM_MSG			(GFL_BG_FRAME1_M)
#define DFRM_SCROLL		(GFL_BG_FRAME2_M)
#define DFRM_SCRMSG		(GFL_BG_FRAME3_M)
//#define UFRM_MSG			(GFL_BG_FRAME0_S)
//#define UFRM_BASE			(GFL_BG_FRAME1_S)
#define UFRM_BACK			(GFL_BG_FRAME0_S)
#define WFNOTE_BG_NUM	(5)

enum{ // パレットテーブル
  // メイン
  BGPLT_M_BACK_0 = 0,			// ベース
  BGPLT_M_BACK_1,					// ボタン（通常時）
  BGPLT_M_BACK_2,					// ボタン（選択時のグラデ用１）
  BGPLT_M_BACK_3,					// ボタン（選択時のグラデ用１）
  BGPLT_M_BACK_4,					// ボタン（項目なし用）
  BGPLT_M_BACK_5,					// タッチバー用
////////////////////////
  BGPLT_M_BACK_6,
  BGPLT_M_BACK_7,
  BGPLT_M_BACK_8,
  BGPLT_M_BACK_9,
//  BGPLT_M_BACK_A,
////////////////////////
  BGPLT_M_MSGFONT = 0x0A,
  BGPLT_M_TALKWIN = 0x0B,
  BGPLT_M_YNWIN = 0x0C,
  BGPLT_M_YNWIN2 = 0x0D,
  BGPLT_M_SBOX = 0x0E,

  // サブ
  BGPLT_S_BACK_0 = 0,
  BGPLT_S_BACK_1,
  BGPLT_S_BACK_2,
  BGPLT_S_BACK_3,
  BGPLT_S_BACK_4,
  BGPLT_S_MSGFONT = 0x0E,
  BGPLT_S_TRGRA = 0x0F,
};
#define BGPLT_M_BACK_NUM  ((BGPLT_M_BACK_5+1) - BGPLT_M_BACK_0)
#define BGPLT_S_BACK_NUM  (BGPLT_M_BACK_NUM)

//アクターアニメ
enum{
  ACTANM_CUR_MSEL,
  ACTANM_CUR_LIST,
  ACTANM_CUR_REC,
  ACTANM_HATENA,
  ACTANM_CLEAR,
  ACTANM_YAZI_LN,
  ACTANM_YAZI_LA,
  ACTANM_YAZI_RN,
  ACTANM_YAZI_RA,
  ACTANM_YAZI_TN,
  ACTANM_YAZI_TA,
  ACTANM_YAZI_BN,
  ACTANM_YAZI_BA,
  ACTANM_PAGE_NUM1,
  ACTANM_PAGE_NUM2,
  ACTANM_PAGE_NUM3,
  ACTANM_PAGE_NUM4,
  ACTANM_NULL = 0xFF,
};


// 基本CGX  MAIN_MSG面に読み込まれます
#define BMPL_WIN_CGX_MENU (1)
//はい・いいえメニュー領域
#define BMPL_YESNO_PX   ( 25 )
#define BMPL_YESNO_PY   ( 9 )
#define BMPL_YESNO_FRM    ( DFRM_MSG )
#define BMPL_YESNO_PAL    ( BGPLT_M_YNWIN )
#define BMPL_YESNO_CGX_SIZ  ( TOUCH_SW_USE_CHAR_NUM )
#define BMPL_YESNO_CGX    ( BMPL_WIN_CGX_MENU+MENU_WIN_CGX_SIZ )

#define BMPL_WIN_CGX_TALK   (BMPL_YESNO_CGX+BMPL_YESNO_CGX_SIZ)
#define BMPL_WIN_CGX_TALKEND  (BMPL_WIN_CGX_TALK+TALK_WIN_CGX_SIZ)

// 基本タイトルビットマップ
#define BMPL_TITLE_X  ( 1 )
#define BMPL_TITLE_Y  ( 0 )
#define BMPL_TITLE_SIZX ( 24 )
#define BMPL_TITLE_SIZY ( 3 )
#define BMPL_TITLE_CGX  ( BMPL_WIN_CGX_TALKEND )
#define BMPL_TITLE_CGX_END  ( BMPL_TITLE_CGX + (BMPL_TITLE_SIZX*BMPL_TITLE_SIZY) )
#define BMPL_TITLE_PL_Y ( 4 )

//選択ボックスウィンドウ領域
#define SBOX_FLIST_SEL_CT (4)
#define SBOX_FLIST_WCGX   (BMPL_TITLE_CGX_END)
#define SBOX_FLIST_W    (17)
#define SBOX_FLIST_FCGX_SIZ (SBOX_FLIST_W*2*SBOX_FLIST_SEL_CT)
#define SBOX_FLIST_FCGX   (SBOX_FLIST_WCGX+SBOX_WINCGX_SIZ)
#define SBOX_FLIST_PX   (16)
#define SBOX_FLIST_PY   (5)


// 基本OAMのデータ
#define WFNOTE_OAM_COMM_CONTID  ( 100 ) // 基本的OAMの管理ID

// FONTカラー
#define WFNOTE_COL_BLACK  ( PRINTSYS_LSB_Make( 1, 2, 0 ) )    // フォントカラー：黒
#define WFNOTE_COL_WHITE  ( PRINTSYS_LSB_Make( 15, 2, 0 ) )   // フォントカラー：白
#define WFNOTE_COL_RED    ( PRINTSYS_LSB_Make( 3, 4, 0 ) )    // フォントカラー：赤
#define WFNOTE_COL_BLUE   ( PRINTSYS_LSB_Make( 5, 6, 0 ) )    // フォントカラー：青
#define WFNOTE_TCOL_BLACK ( PRINTSYS_LSB_Make( 1, 15, 0 ) )   // フォントカラー：縁白の黒

#define WFNOTE_FONT_COL_WHITE (0xF)
// 描画エリア数
enum{
  WFNOTE_DRAWAREA_MAIN,
  WFNOTE_DRAWAREA_RIGHT,
  WFNOTE_DRAWAREA_LEFT,
  WFNOTE_DRAWAREA_NUM,
};

//  基本スクリーンデータ
enum{
  WFNOTE_SCRNDATA_BACK, // 背景用
  WFNOTE_SCRNDATA_KIT,    // 書き込み用
  WFNOTE_SCRNDATA_NUM,    //
};

//-------------------------------------
/// STATUS
//=====================================
enum {
  STATUS_MODE_SELECT, // 最初の選択画面
  STATUS_FRIENDLIST,  // 友達データ表示
  STATUS_CODEIN,    // 友達コード入力
  STATUS_MYCODE,    // 自分の友達コード確認
  STATUS_FRIENDINFO,  // 友達詳細データ
  STATUS_END,     // 終了
  STATUS_NUM
} ;
typedef enum{
  WFNOTE_STRET_CONTINUE,  // つづく
  WFNOTE_STRET_FINISH,  // 作業は終わった
} WFNOTE_STRET;
#define STATUS_CHANGE_WAIT  (4) // 変更時ウエイト

//-------------------------------------
/// PROC SEQ
//=====================================
enum {
  WFNOTE_PROCSEQ_MAIN,
  WFNOTE_PROCSEQ_WAIT,
} ;

//-------------------------------------
/// 友達データ新規設定関数　戻り値
//=====================================
enum{
  RCODE_NEWFRIEND_SET_OK, // 設定できた
  RCODE_NEWFRIEND_SET_FRIENDKEYNG,  // 友達コードが不正
  RCODE_NEWFRIEND_SET_SETING, // もう設定してあった
  RCODE_NEWFRIEND_SET_NUM

};

// 矢印データ
#define WFNOTE_YAZIRUSHINUM   (2) // 矢印の数
#define WFNOTE_YAZIRUSHI_LX   (8)
#define WFNOTE_YAZIRUSHI_RX   (248)
#define WFNOTE_YAZIRUSHI_LRY  (98)
#define WFNOTE_YAZIRUSHI_PRI  ( 8 )

#define WFNOTE_YAZIRUSHI_TBX  (256-12)
#define WFNOTE_YAZIRUSHI_TY   (3*8)
#define WFNOTE_YAZIRUSHI_BY   (17*8)

enum{
  ACT_YAZI_L,
  ACT_YAZI_R,
  ACT_YAZI_T,
  ACT_YAZI_B,
};
enum{
  ACT_YAZIANM_NORMAL,
  ACT_YAZIANM_ACTIVE,
};


//カーソルデータ
#define WFNOTE_CURSOR_PRI   (9)
#define WFNOTE_CURSOR_BGPRI   (1)

// タイトルメッセージスピード
#define WFNOTE_TITLE_MSG_SPEED  ( 2 )



//-------------------------------------
/// MODESELECT
//=====================================
// BMP
#define BMPL_MODESEL_MSG_X    ( 4 ) // 開始位置（キャラ単位）
#define BMPL_MODESEL_MSG_Y    ( 5 ) // 開始位置（キャラ単位）
#define BMPL_MODESEL_MSG_SIZX ( 24 )  // サイズ（キャラ単位）
#define BMPL_MODESEL_MSG_SIZY ( 20 )  // サイズ（キャラ単位）
#define BMPL_MODESEL_MSGCGX   ( 1 ) // キャラクタオフセット

#define BMPL_MODESEL_TALK_X   ( 2 ) // 開始位置（キャラ単位）
#define BMPL_MODESEL_TALK_Y   ( 19 )  // 開始位置（キャラ単位）
#define BMPL_MODESEL_TALK_SIZX    ( 27 )  // サイズ（キャラ単位）
#define BMPL_MODESEL_TALK_SIZY    ( 4 ) // サイズ（キャラ単位）
#define BMPL_MODESEL_TALKCGX    ( BMPL_TITLE_CGX_END )  // キャラクタオフセット

#define BMPL_MODESEL_YESNO_FRM    ( DFRM_MSG )
#define BMPL_MODESEL_YESNO_X    ( 25 )  // 開始位置（キャラ単位）
#define BMPL_MODESEL_YESNO_Y    ( 13 )  // 開始位置（キャラ単位）
#define BMPL_MODESEL_YESNO_SIZX   ( 6 ) // サイズ（キャラ単位）
#define BMPL_MODESEL_YESNO_SIZY   ( 4 ) // サイズ（キャラ単位）
#define BMPL_MODESEL_YESNOCGX   ( BMPL_MODESEL_TALKCGX+(BMPL_MODESEL_TALK_SIZX*BMPL_MODESEL_TALK_SIZY) )  // キャラクタオフセット

// 動作シーケンス
enum{
  SEQ_MODESEL_INIT,
  SEQ_MODESEL_FADEINWAIT,
  SEQ_MODESEL_INIT_NOFADE,
  SEQ_MODESEL_MAIN,
  SEQ_MODESEL_MAIN_ENDWAIT,
  SEQ_MODESEL_FADEOUT,
  SEQ_MODESEL_FADEOUTWAIT,

  // コード入力から戻ってきたとき用
  SEQ_MODESEL_CODECHECK_INIT,
  SEQ_MODESEL_CODECHECK_FADEINWAIT,
  SEQ_MODESEL_CODECHECK_MSGWAIT,    //登録確認メッセージON
  SEQ_MODESEL_CODECHECK_MSGYESNOWAIT, //はいいいえ選択待ち
  SEQ_MODESEL_CODECHECK_RESLUTMSGWAIT,  //選択結果表示
};
// カーソル
enum{
  MODESEL_CUR_FLIST,
  MODESEL_CUR_CODEIN,
  MODESEL_CUR_MYCODE,
  MODESEL_CUR_END,
  MODESEL_CUR_NUM,
};
#define MODESEL_CUR_CSIZX ( 28 )  // カーソル部分の横サイズ（キャラ）
#define MODESEL_CUR_CSIZY ( 4 ) // カーソル部分の縦サイズ（キャラ）
#define MODESEL_CUR_PAL_ON  ( BGPLT_M_BACK_9 )  // 選択時
#define MODESEL_CUR_PAL_ANM ( BGPLT_M_BACK_9 )  // 選択時
#define MODESEL_CUR_PAL_OFF ( BGPLT_M_BACK_2 )  // 非選択時


//-------------------------------------
/// FRIENDLIST
//=====================================
#define FLIST_PAGE_FRIEND_NUM (8) // 1ページにいる人の数
#define FLIST_PAGE_FRIEND_HALF  (FLIST_PAGE_FRIEND_NUM/2) // 1ページにいる人の数
#define FLIST_PAGE_MAX      (4) // ページ総数
#define FLIST_FRIEND_MAX    ( FLIST_PAGE_FRIEND_NUM*FLIST_PAGE_MAX )  // 表示できる友達の最大値
#define FLIST_2DCHAR_NUM    (16)// 2dcharワーク数

enum{
  FLIST_MAIN_EXE_CANCEL,
  FLIST_MAIN_EXE_LEFT,
  FLIST_MAIN_EXE_RIGHT,
  FLIST_MAIN_EXE_UP,
  FLIST_MAIN_EXE_DOWN,
  FLIST_MAIN_EXE_DECIDE,
};

enum{ // カーソルデータすう
  FLIST_CURSORDATA_0,
  FLIST_CURSORDATA_1,
  FLIST_CURSORDATA_2,
  FLIST_CURSORDATA_3,
  FLIST_CURSORDATA_4,
  FLIST_CURSORDATA_5,
  FLIST_CURSORDATA_6,
  FLIST_CURSORDATA_7,
  FLIST_CURSORDATA_BACK,
  FLIST_CURSORDATA_NUM,
  FLIST_CURSORDATA_OFF, // カーソルを消すときのPOS
};

//////////////////////////////////////////
// ビットマップ
//////////////////////////////////////////
#define BMPL_FLIST_TEXT_OFSX  ( 4 ) // 開始位置ｘ
#define BMPL_FLIST_TEXT_OFSY  ( 4 ) // 開始位置ｙ
#define BMPL_FLIST_TEXT_SIZX  ( 26 )  // サイズｘ
#define BMPL_FLIST_TEXT_SIZY  ( 17 )  // サイズｙ
#define BMPL_FLIST_TEXT_CGX0  ( 1 ) // cgx領域1
#define BMPL_FLIST_TEXT_CGX1  ( BMPL_FLIST_TEXT_CGX0+(BMPL_FLIST_TEXT_SIZX*BMPL_FLIST_TEXT_SIZY) )  // cgx領域2

#define BMPL_FLIST_BKMSG_X  ( 21 )  // 開始位置ｘ
#define BMPL_FLIST_BKMSG_Y  ( 21 )  // 開始位置ｙ
#define BMPL_FLIST_BKMSG_SIZX ( 9 ) // サイズｘ
#define BMPL_FLIST_BKMSG_SIZY ( 2 ) // サイズｙ
#define BMPL_FLIST_BKMSG_CGX  (SBOX_FLIST_FCGX+SBOX_FLIST_FCGX_SIZ )

#define BMPL_FLIST_TALK_X   ( 2 ) // 開始位置（キャラ単位）
#define BMPL_FLIST_TALK_Y   ( 1 )  // 開始位置（キャラ単位）
#define BMPL_FLIST_TALK_SIZX  ( 27 )  // サイズ（キャラ単位）
#define BMPL_FLIST_TALK_SIZY  ( 4 ) // サイズ（キャラ単位）
#define BMPL_FLIST_TALK_CGX   (BMPL_FLIST_BKMSG_CGX+(BMPL_FLIST_BKMSG_SIZX*BMPL_FLIST_BKMSG_SIZY))  // キャラクタオフセット

#define BMPL_FLIST_YESNO_FRM    ( GFL_BG_FRAME1_M )
#define BMPL_FLIST_YESNO_X    ( 25 )  // 開始位置（キャラ単位）
#define BMPL_FLIST_YESNO_Y    ( 13 )  // 開始位置（キャラ単位）
#define BMPL_FLIST_YESNO_SIZX   ( 6 ) // サイズ（キャラ単位）
#define BMPL_FLIST_YESNO_SIZY   ( 4 ) // サイズ（キャラ単位）
#define BMPL_FLIST_YESNO_CGX    ( BMPL_FLIST_TALK_CGX+(BMPL_FLIST_TALK_SIZX*BMPL_FLIST_TALK_SIZY) ) // キャラクタオフセット


// 動作シーケンス
enum{
  SEQ_FLIST_INIT_PAGEINIT,
  SEQ_FLIST_INIT,
  SEQ_FLIST_MAIN,
  SEQ_FLIST_SCRLLINIT,
  SEQ_FLIST_SCRLL,

  SEQ_FLIST_MENUINIT, //　メニュー選択
  SEQ_FLIST_MENUWAIT, // メニュー選択待ち
  SEQ_FLIST_INFOINIT,   // 詳しく見るへ
  SEQ_FLIST_INFO,   // 詳しく見るへ
  SEQ_FLIST_INFORET,  // 詳しく見るへ
  SEQ_FLIST_NAMECHG_INIT, // 名前変更
  SEQ_FLIST_NAMECHG_WIPE, // 名前変更
  SEQ_FLIST_NAMECHG_WAIT, // 名前変更
  SEQ_FLIST_NAMECHG_WAITWIPE, // 名前変更
  SEQ_FLIST_NAMECHG_WAITWIPEWAIT, // 名前変更
  SEQ_FLIST_DELETE_INIT,  // 破棄
  SEQ_FLIST_DELETE_YESNODRAW, // 破棄
  SEQ_FLIST_DELETE_WAIT,  // 破棄YESNOをまってから処理
  SEQ_FLIST_DELETE_ANMSTART,  // 破棄YESNOをまってから処理
  SEQ_FLIST_DELETE_ANMWAIT, // 破棄YESNOをまってから処理
  SEQ_FLIST_DELETE_END,   //  破棄YESNOをまってから処理
  SEQ_FLIST_CODE_INIT,  // コード切り替え
  SEQ_FLIST_CODE_WAIT,  // 何かキーを押してもらう
  SEQ_FLIST_END,
};

// しおり
#define FLIST_SHIORI_X    ( 16 )    // 位置（キャラサイズ）
#define FLIST_SHIORI_Y    ( 0 )   // 位置(キャラサイズ)
#define FLIST_SHIORI_SIZX ( 16 )    // サイズ（キャラクタサイズ）
#define FLIST_SHIORI_SIZY ( 4 )   // サイズ（キャラクタサイズ）
#define FLIST_SHIORI_ONEX ( 4 )   // １ページ分のサイズ
#define FLIST_SHIORI_OFSX ( 0 )   // ONEXの中で表示するキャラクタまでの数
#define FLIST_SHIORI_OFSY ( 0 )   // ONEXの中で表示するキャラクタまでの数
#define FLIST_SHIORISCRN_X  ( 0 )   // スクリーン読み込みｘ
#define FLIST_SHIORISCRN_Y  ( 4 )   // スクリーン読み込みｙ
#define FLIST_CL_SHIORISCRN_X ( 16 )    // スクリーン読み込みｘ
#define FLIST_CL_SHIORISCRN_Y ( 4 )   // スクリーン読み込みｙ
#define FLIST_SHIORISCRN_POS_X  ( 0 ) // ポジションスクリーン読み込みｘ
#define FLIST_SHIORISCRN_POS_Y  ( 8 ) // ポジションスクリーン読み込みｘ
#define FLIST_EFF_SHIORISCRN_POS_X  ( 16 )  // エフェクト用ポジションスクリーン読み込みｘ
#define FLIST_EFF_SHIORISCRN_POS_Y  ( 8 ) // エフェクト用ポジションスクリーン読み込みｘ

#define FLIST_SHIORIPOS_SIZ (4)     // ポジションスクリーンの大きさ

// もどる表示位置
#define FLIST_BACK_X    ( 17 )  // 戻る表示位置
#define FLIST_BACK_Y    ( 20 )  // 戻る表示位置
#define FLIST_BACK_SIZX   ( 14 )  // 戻る表示サイズ
#define FLIST_BACK_SIZY   ( 4 ) // 戻る表示サイズ
#define FLIST_SCRBACK_X   ( 3 ) // 戻る読み込み先位置
#define FLIST_SCRBACK_Y   ( 0 ) // 戻る読み込み先位置
#define FLIST_CL_SCRBACK_X    ( 17 )  // 戻る読み込み先位置
#define FLIST_CL_SCRBACK_Y    ( 0 ) // 戻る読み込み先位置

// プレイヤー表示位置（DATA_FListCursorDataないの位置データからのオフセット）
#define FLIST_PLAYER_X    ( 8 )
#define FLIST_PLAYER_Y    ( 6+CLACT_MAIN2_REND_Y_S32 )
#define FLIST_PLAYER_PRI      ( 8 )
#define FLIST_PLAYER_BGPRI      ( 1 )
// ？表示位置（DATA_FListCursorDataないの位置データからのオフセット）
#define FLIST_HATENA_X    ( 16 )
#define FLIST_HATENA_Y    ( 16+CLACT_MAIN2_REND_Y_S32 )
//　カーソル表示プライオリティ
#define FLIST_CURSOR_PRI    (FLIST_PLAYER_PRI+8)
#define FLIST_CURSOR_BGPRI    (FLIST_PLAYER_BGPRI)

// はてなの場合
#define FLIST_NONPLAYER_SIZX  (3)
#define FLIST_NONPLAYER_SIZY  (4)
#define FLIST_NONPLAYER_SCRN_X  (0)
#define FLIST_NONPLAYER_SCRN_Y  (0)

// プレイヤー名表示位置（DATA_FListCursorDataないの位置データからのオフセット）
#define FLIST_PLAYERNAME_X  ( 0 )
#define FLIST_PLAYERNAME_Y  ( -24 )

// SEQ_MAIN関数戻り値
enum{
  RCODE_FLIST_SEQMAIN_NONE,
  RCODE_FLIST_SEQMAIN_PCHG_LEFT,// ページ変更
  RCODE_FLIST_SEQMAIN_PCHG_RIGHT,// ページ変更
  RCODE_FLIST_SEQMAIN_CANCEL, // 戻る
  RCODE_FLIST_SEQMAIN_SELECTPL, // 人を選択した
};


// スクロール
#define FLIST_SCROLL_COUNT  (24)  // スクロールカウント（4の倍数）
#define FLIST_SCROLL_SIZX (256) // スクロールサイズ


// メニューデータ
enum{
  BMPL_FLIST_MENU_NML,    // 通常メニュー
  BMPL_FLIST_MENU_CODE, // 友達番号のみ登録時のメニュー
  BMPL_FLIST_MENU_NUM,
};
#define BMPL_FLIST_MENU_LISTNUM (SBOX_FLIST_SEL_CT) // メニューリストの項目数

// 詳細画面へフェードのウエイト
#define FLIST_INFO_WAIT (8)

//  消しゴムアクター
#define FLIST_CLEARACT_Y  ( -4 + CLACT_MAIN2_REND_Y_S32 )

#define TP_FLIST_BTN_W    (14*8)
#define TP_FLIST_BTN_H    (4*8)
#define TP_FLIST_BTN_X01  (2*8)
#define TP_FLIST_BTN_X02  (16*8)
#define TP_FLIST_BTN_OY   (4*8)
#define TP_FLIST_BTN_Y01  (4*8)
#define TP_FLIST_BTN_Y02  (TP_FLIST_BTN_Y01+TP_FLIST_BTN_OY)
#define TP_FLIST_BTN_Y03  (TP_FLIST_BTN_Y02+TP_FLIST_BTN_OY)
#define TP_FLIST_BTN_Y04  (TP_FLIST_BTN_Y03+TP_FLIST_BTN_OY)

#define TP_FLIST_BACK_X   (17*8)
#define TP_FLIST_BACK_Y   (TP_FLIST_BTN_Y04+TP_FLIST_BTN_OY)

#define TP_FLIST_YAZI_W   (16)
#define TP_FLIST_YAZI_H   (16)
#define TP_FLIST_YAZI_Y   (11*8)
#define TP_FLIST_YAZIL_X  (0)
#define TP_FLIST_YAZIR_X  (30*8)

#define TP_FLIST_BM_W			(24)
#define TP_FLIST_BM_H			(24)
#define TP_FLIST_BM_Y			(168)
#define TP_FLIST_BM01_X   (8)
#define TP_FLIST_BM02_X   (TP_FLIST_BM01_X+TP_FLIST_BM_W)
#define TP_FLIST_BM03_X   (TP_FLIST_BM02_X+TP_FLIST_BM_W)
#define TP_FLIST_BM04_X   (TP_FLIST_BM03_X+TP_FLIST_BM_W)

//-------------------------------------
/// CODEIN
//=====================================
enum{ // シーケンス
  SEQ_CODEIN_NAMEIN,
  SEQ_CODEIN_NAMEIN_WAIT,
  SEQ_CODEIN_CODEIN_WAIT,
  SEQ_CODEIN_END,

  SEQ_CODEIN_NAMEINONLY,  // 名前入力のみ
  SEQ_CODEIN_NAMEINONLY_WAIT, // 名前入力のみ
  SEQ_CODEIN_NAMEINONLY_END,  // 名前入力のみ

};
#define FRIENDCODE_MAXLEN (12)  // 友達コード文字数

//-------------------------------------
/// MYCODE
//=====================================
// BMP  メッセージ
#define BMPL_MYCODE_MSG_X   ( 2 ) // 開始位置（キャラ単位）
#define BMPL_MYCODE_MSG_Y   ( 10 )  // 開始位置（キャラ単位）
#define BMPL_MYCODE_MSG_SIZX  ( 30 )  // サイズ（キャラ単位）
#define BMPL_MYCODE_MSG_SIZY  ( 8 ) // サイズ（キャラ単位）
#define BMPL_MYCODE_MSG_CGX   ( 1 ) // キャラクタオフセット
// BMP  コード
#define BMPL_MYCODE_CODE_X    ( 9 ) // 開始位置（キャラ単位）
#define BMPL_MYCODE_CODE_Y    ( 6 ) // 開始位置（キャラ単位）
#define BMPL_MYCODE_CODE_SIZX ( 14 )  // サイズ（キャラ単位）
#define BMPL_MYCODE_CODE_SIZY ( 2 ) // サイズ（キャラ単位）
#define BMPL_MYCODE_CODE_CGX  ( BMPL_MYCODE_MSG_CGX+(BMPL_MYCODE_MSG_SIZX*BMPL_MYCODE_MSG_SIZY) ) // キャラクタオフセット
// BMP  戻るボタン
#define BMPL_MYCODE_BACK_X    ( 21 )  // 開始位置（キャラ単位）
#define BMPL_MYCODE_BACK_Y    ( 21 )  // 開始位置（キャラ単位）
#define BMPL_MYCODE_BACK_SIZX ( 9 ) // サイズ（キャラ単位）
#define BMPL_MYCODE_BACK_SIZY ( 2 ) // サイズ（キャラ単位）
#define BMPL_MYCODE_BACK_CGX  ( BMPL_MYCODE_CODE_CGX+(BMPL_MYCODE_CODE_SIZX*BMPL_MYCODE_CODE_SIZY) )  // キャラクタオフセット

// シーケンス
enum{
  SEQ_MYCODE_INIT,
  SEQ_MYCODE_MAIN,
  SEQ_MYCODE_BTNANM,
};

//-------------------------------------
/// FRIENDINFO
//=====================================
#define FINFO_PAGE_NUM          ( 8 ) // ページ数
#define FINFO_BFOFF_PAGE_NUM    ( 3 ) // フロンティア無しページ数

//実行コード
enum{
  FINFO_EXE_DECIDE,
  FINFO_EXE_CANCEL,
  FINFO_EXE_LEFT,
  FINFO_EXE_RIGHT,
  FINFO_EXE_TOP,
  FINFO_EXE_BOTTOM,
};

// シーケンス
enum{
  SEQ_FINFO_INIT,
  SEQ_FINFO_INITRP,
  SEQ_FINFO_MAIN,
  SEQ_FINFO_SCRINIT,
  SEQ_FINFO_SCR,
  SEQ_FINFO_SCR_WAIT_MSG,
  SEQ_FINFO_FRIENDCHG,
  SEQ_FINFO_END,
};

//スクリーン
enum{
 FINFO_SCRNDATA_MB,
 FINFO_SCRNDATA_M,
 FINFO_SCRNDATA_MO,
 FINFO_SCRNDATA_S,
 FINFO_SCRNDATA_NUM
};

/////////////////////////////////
// ビットマップ下画面
/////////////////////////////////
#define BMPL_FINFO_TEXT_CGX0  ( 1 ) // サイズｙ
#define BMPL_FINFO_TEXT_CGX1  ( BMPL_FINFO_TEXT_CGX0+(500) )  // サイズｙ

/////////////////////////////////
//ビットマップ上画面
/////////////////////////////////
/*
enum{
 FINFO_WIN_PAGE,
 FINFO_WIN_TITLE,
 FINFO_WIN_NAME,
 FINFO_WIN_GRP02,
 FINFO_WIN_DAY02,
 FINFO_WIN_TRGRA,
 FINFO_WIN_GRP01,
 FINFO_WIN_DAY01,
 FINFO_WIN_NUM,
};
*/

//#define BMPL_FINFO_FRM      ( UFRM_MSG )
#define BMPL_FINFO_PAL      ( BGPLT_S_MSGFONT )
//ページ
#define BMPL_FINFO_PAGE_X   ( 26 )  // 開始位置（キャラ単位）
#define BMPL_FINFO_PAGE_Y   ( 1 ) // 開始位置（キャラ単位）
#define BMPL_FINFO_PAGE_SIZX  ( 5)  // サイズ（キャラ単位）
#define BMPL_FINFO_PAGE_SIZY  ( 2 ) // サイズ（キャラ単位）
#define BMPL_FINFO_PAGE_SIZ   ( BMPL_FINFO_PAGE_SIZX * BMPL_FINFO_PAGE_SIZY ) // サイズ（キャラ単位）
#define BMPL_FINFO_PAGE_CGX   ( 1 ) // キャラクタオフセット
// BMP タイトル
#define BMPL_FINFO_TITLE_X    ( BMPL_TITLE_X )  // 開始位置（キャラ単位）
#define BMPL_FINFO_TITLE_Y    ( BMPL_TITLE_Y )  // 開始位置（キャラ単位）
#define BMPL_FINFO_TITLE_SIZX ( BMPL_TITLE_SIZX ) // サイズ（キャラ単位）
#define BMPL_FINFO_TITLE_SIZY ( BMPL_TITLE_SIZY ) // サイズ（キャラ単位）
#define BMPL_FINFO_TITLE_SIZ  ( BMPL_TITLE_SIZX * BMPL_TITLE_SIZY ) // サイズ（キャラ単位）
#define BMPL_FINFO_TITLE_CGX  ( BMPL_FINFO_PAGE_CGX+BMPL_FINFO_PAGE_SIZ ) // キャラクタオフセット
//名前
#define BMPL_FINFO_NAME_X   ( 22 )  // 開始位置（キャラ単位）
#define BMPL_FINFO_NAME_Y   ( 6 ) // 開始位置（キャラ単位）
#define BMPL_FINFO_NAME_SIZX  ( 9 ) // サイズ（キャラ単位）
#define BMPL_FINFO_NAME_SIZY  ( 2 ) // サイズ（キャラ単位）
#define BMPL_FINFO_NAME_SIZ   ( BMPL_FINFO_NAME_SIZX * BMPL_FINFO_NAME_SIZY ) // サイズ（キャラ単位）
#define BMPL_FINFO_NAME_CGX   ( BMPL_FINFO_TITLE_CGX+BMPL_FINFO_TITLE_SIZ ) // キャラクタオフセット
//グループ
#define BMPL_FINFO_GRP01_X    ( 1 ) // 開始位置（キャラ単位）
#define BMPL_FINFO_GRP01_Y    ( 17 )  // 開始位置（キャラ単位）
#define BMPL_FINFO_GRP01_SIZX ( 6 ) // サイズ（キャラ単位）
#define BMPL_FINFO_GRP01_SIZY ( 2 ) // サイズ（キャラ単位）
#define BMPL_FINFO_GRP01_SIZ  ( BMPL_FINFO_GRP01_SIZX * BMPL_FINFO_GRP01_SIZY ) // サイズ（キャラ単位）
#define BMPL_FINFO_GRP01_CGX  ( BMPL_FINFO_NAME_CGX+BMPL_FINFO_NAME_SIZ ) // キャラクタオフセット

#define BMPL_FINFO_GRP02_X    ( 9 ) // 開始位置（キャラ単位）
#define BMPL_FINFO_GRP02_Y    ( 17 )  // 開始位置（キャラ単位）
#define BMPL_FINFO_GRP02_SIZX ( 8 ) // サイズ（キャラ単位）
#define BMPL_FINFO_GRP02_SIZY ( 2 ) // サイズ（キャラ単位）
#define BMPL_FINFO_GRP02_SIZ  ( BMPL_FINFO_GRP02_SIZX * BMPL_FINFO_GRP02_SIZY ) // サイズ（キャラ単位）
#define BMPL_FINFO_GRP02_CGX  ( BMPL_FINFO_GRP01_CGX+BMPL_FINFO_GRP01_SIZ ) // キャラクタオフセット

//日付
#define BMPL_FINFO_DAY01_X    ( 1 ) // 開始位置（キャラ単位）
#define BMPL_FINFO_DAY01_Y    ( 21 )  // 開始位置（キャラ単位）
#define BMPL_FINFO_DAY01_SIZX ( 17 )  // サイズ（キャラ単位）
#define BMPL_FINFO_DAY01_SIZY ( 2 ) // サイズ（キャラ単位）
#define BMPL_FINFO_DAY01_SIZ  ( BMPL_FINFO_DAY01_SIZX * BMPL_FINFO_DAY01_SIZY ) // サイズ（キャラ単位）
#define BMPL_FINFO_DAY01_CGX  ( BMPL_FINFO_GRP02_CGX+BMPL_FINFO_GRP02_SIZ ) // キャラクタオフセット

#define BMPL_FINFO_DAY02_X    ( 19 )  // 開始位置（キャラ単位）
#define BMPL_FINFO_DAY02_Y    ( 21 )  // 開始位置（キャラ単位）
#define BMPL_FINFO_DAY02_SIZX ( 10 )  // サイズ（キャラ単位）
#define BMPL_FINFO_DAY02_SIZY ( 2 ) // サイズ（キャラ単位）
#define BMPL_FINFO_DAY02_SIZ  ( BMPL_FINFO_DAY02_SIZX * BMPL_FINFO_DAY02_SIZY ) // サイズ（キャラ単位）
#define BMPL_FINFO_DAY02_CGX  ( BMPL_FINFO_DAY01_CGX+BMPL_FINFO_DAY01_SIZ ) // キャラクタオフセット

//トレーナーグラフィック
#define BMPL_FINFO_TRGRA_X    ( 4 ) // 開始位置（キャラ単位）
#define BMPL_FINFO_TRGRA_Y    ( 4 ) // 開始位置（キャラ単位）
#define BMPL_FINFO_TRGRA_SIZX ( 10 )  // サイズ（キャラ単位）
#define BMPL_FINFO_TRGRA_SIZY ( 10 )  // サイズ（キャラ単位）
#define BMPL_FINFO_TRGRA_SIZ  ( BMPL_FINFO_TRGRA_SIZX * BMPL_FINFO_TRGRA_SIZY ) // サイズ（キャラ単位）
#define BMPL_FINFO_TRGRA_CGX  ( BMPL_FINFO_DAY02_CGX+BMPL_FINFO_DAY02_SIZ ) // キャラクタオフセット
#define BMPL_FINFO_TRGRA_PAL  ( BGPLT_S_TRGRA )
#define BMPL_FINFO_TRGRA_CUTX ( 0 )
#define BMPL_FINFO_TRGRA_CUTY ( 0 )

// しおり
#define FINFO_SHIORI_X    ( 0 ) // 位置（キャラサイズ）
#define FINFO_SHIORI_Y    ( 20 )    // 位置(キャラサイズ)
#define FINFO_SHIORI_SIZX ( 32 )    // サイズ（キャラクタサイズ）
#define FINFO_SHIORI_SIZY ( 4 )   // サイズ（キャラクタサイズ）
#define FINFO_SHIORI_ONEX ( 1 )   // １ページ分のサイズ
#define FINFO_SHIORI_OFSX ( 1 )   // ONEXの中で表示するキャラクタまでの数
#define FINFO_SHIORI_OFSY ( 1 )   // ONEXの中で表示するキャラクタまでの数
#define FINFO_SHIORISCRN_X  ( 0 )   // スクリーン読み込みｘ
#define FINFO_SHIORISCRN_Y  ( 12 )    // スクリーン読み込みｙ
#define FINFO_CL_SHIORISCRN_X ( 0 )   // スクリーン読み込みｘ
#define FINFO_CL_SHIORISCRN_Y ( 28 )    // スクリーン読み込みｙ
#define FINFO_SHIORISCRN_POS_X  ( 0 ) // ポジションスクリーン読み込みｘ
#define FINFO_SHIORISCRN_POS_Y  ( 16 )  // ポジションスクリーン読み込みｘ
#define FINFO_EFF_SHIORISCRN_POS_X  ( 5 ) // エフェクト用ポジションスクリーン読み込みｘ
#define FINFO_EFF_SHIORISCRN_POS_Y  ( 1 ) // エフェクト用ポジションスクリーン読み込みｘ
#define FINFO_EFF_SHIORI_POS_SIZ  (4)     // ポジションスクリーンの大きさ

// スクロール
#define FINFO_SCROLL_COUNT  (8) // スクロールカウント
#define FINFO_SCROLL_SIZX (256) // スクロールサイズ


// 最大値
#define FINFO_PAGE_X  ( 2 ) // ビットマップエリア
#define FINFO_PAGE_Y  ( 1 ) // ビットマップエリア
#define FINFO_PAGE_SIZX ( 26 )  // ビットマップエリア
#define FINFO_PAGE_SIZY ( 18 )  // ビットマップエリア

// ページ００
enum{
  FINFO_PAGE00_BA_BTT,
  FINFO_PAGE00_BA_TRT = 0,
  FINFO_PAGE00_BA_BT = 0,
  FINFO_PAGE00_BA_TR = 0,
  FINFO_PAGE00_BA_NUM = 1,
};
#define FINFO_PAGE00_BA_CAP_X   ( 2 ) // ビットマップエリア タイトル
#define FINFO_PAGE00_BA_CAP_SIZX  ( 12 )  // ビットマップエリア タイトル
#define FINFO_PAGE00_BA_CAP_SIZY  ( 2 ) // ビットマップエリア タイトル
#define FINFO_PAGE00_BA_CAP_BTT_Y ( 5 ) // ビットマップエリア 対戦タイトル
#define FINFO_PAGE00_BA_CAP_TRT_Y ( 12 )  // ビットマップエリア 交換タイトル

#define FINFO_PAGE00_BA_BT_X  ( 19 )  // ビットマップエリア 対戦成績
#define FINFO_PAGE00_BA_BT_Y  ( FINFO_PAGE00_BA_CAP_BTT_Y ) // ビットマップエリア 対戦成績
#define FINFO_PAGE00_BA_BT_SIZX ( 8 ) // ビットマップエリア 対戦成績
#define FINFO_PAGE00_BA_BT_SIZY ( 5 ) // ビットマップエリア 対戦成績

#define FINFO_PAGE00_BA_TR_X  ( 23 )  // ビットマップエリア 交換
#define FINFO_PAGE00_BA_TR_Y  ( FINFO_PAGE00_BA_CAP_TRT_Y ) // ビットマップエリア 交換
#define FINFO_PAGE00_BA_TR_SIZX ( 4 ) // ビットマップエリア 交換
#define FINFO_PAGE00_BA_TR_SIZY ( 2 ) // ビットマップエリア 交換

#define FINFO_PAGE00_BTT_X    ( 0 ) // 対戦成績タイトル
#define FINFO_PAGE00_BTT_Y    ( 0 ) // 対戦成績タイトル
#define FINFO_PAGE00_BT_X   ( 0 ) // 対戦成績かち&まけ
#define FINFO_PAGE00_BT_Y   ( 0 ) // 対戦成績かち&まけ
#define FINFO_PAGE00_BT_OFSY  ( 24 )  // 対戦成績Yオフセット

#define FINFO_PAGE00_TRT_X  ( 0 ) // ポケモン交換タイトル
#define FINFO_PAGE00_TRT_Y  ( 0 ) // ポケモン交換タイトル
#define FINFO_PAGE00_TRN_X  ( 0 ) // ポケモン交換値
#define FINFO_PAGE00_TRN_Y  ( 0 ) // ポケモン交換値

// ページ０１
enum{
  FINFO_PAGE01_BA_CAP = 0,
  FINFO_PAGE01_BA_DAT = 0,
  FINFO_PAGE01_BA_NUM = 1,
};
#define FINFO_PAGE01_BA_CAP_X ( 5 ) // ビットマップエリア
#define FINFO_PAGE01_BA_CAP_Y ( 5 ) // ビットマップエリア
#define FINFO_PAGE01_BA_CAP_SIZX ( 9 )  // ビットマップエリア
#define FINFO_PAGE01_BA_CAP_SIZY ( 2 )  // ビットマップエリア
#define FINFO_PAGE01_BA_DAT_X ( FINFO_PAGE01_BA_CAP_X ) // ビットマップエリア
#define FINFO_PAGE01_BA_DAT_Y ( 9 ) // ビットマップエリア
#define FINFO_PAGE01_BA_DAT_SIZX ( 19 ) // ビットマップエリア
#define FINFO_PAGE01_BA_DAT_SIZY ( 5 )  // ビットマップエリア

#define FINFO_PAGE01_BTT_X    ( 0 ) // バトルタワータイトル
#define FINFO_PAGE01_BTT_Y    ( 0 ) // バトルタワータイトル
#define FINFO_PAGE01_DAT_X    ( 0 ) // バトルタワータイトル
#define FINFO_PAGE01_DATN_X   ( 7*8 ) // バトルタワータイトル
#define FINFO_PAGE01_DAT_Y01  ( 0 ) // バトルタワータイトル
#define FINFO_PAGE01_DAT_Y02  ( 24 )  // バトルタワータイトル

// ページ02
enum{
  FINFO_PAGE02_BA_TT = 0,   // タイトル
  FINFO_PAGE02_LV50TT = 0,  // LV50タイトル
  FINFO_PAGE02_LV50NO = 0,  // LV50値
  FINFO_PAGE02_LV100TT = 0, // LV100タイトル
  FINFO_PAGE02_LV100NO = 0, // LV100値
  FINFO_PAGE02_BA_NUM = 1,
};

#define FINFO_PAGE02_BA_TT_X  ( 2 ) // ビットマップエリア タイトル
#define FINFO_PAGE02_BA_TT_Y  ( 1 ) // ビットマップエリア タイトル
#define FINFO_PAGE02_BA_TT_SIZX ( 14 )  // ビットマップエリア タイトル
#define FINFO_PAGE02_BA_TT_SIZY ( 2 ) // ビットマップエリア タイトル
#define FINFO_PAGE02_BA_LV50TT_X  ( 2 ) // ビットマップエリア LV50タイトル
#define FINFO_PAGE02_BA_LV50TT_Y  ( 4 ) // ビットマップエリア LV50タイトル
#define FINFO_PAGE02_BA_LV50TT_SIZX ( 26 )  // ビットマップエリア LV50タイトル
#define FINFO_PAGE02_BA_LV50TT_SIZY ( 2 ) // ビットマップエリア LV50タイトル
#define FINFO_PAGE02_BA_LV50NO_X  ( 2 ) // ビットマップエリア LV50値
#define FINFO_PAGE02_BA_LV50NO_Y  ( 7 ) // ビットマップエリア LV50値
#define FINFO_PAGE02_BA_LV50NO_SIZX ( 26 )  // ビットマップエリア LV50値
#define FINFO_PAGE02_BA_LV50NO_SIZY ( 4 ) // ビットマップエリア LV50値
#define FINFO_PAGE02_BA_LV100TT_X ( 2 ) // ビットマップエリア LV100タイトル
#define FINFO_PAGE02_BA_LV100TT_Y ( 12 )  // ビットマップエリア LV100タイトル
#define FINFO_PAGE02_BA_LV100TT_SIZX ( 26 ) // ビットマップエリア LV100タイトル
#define FINFO_PAGE02_BA_LV100TT_SIZY ( 2 )  // ビットマップエリア LV100タイトル
#define FINFO_PAGE02_BA_LV100NO_X ( 2 ) // ビットマップエリア LV100値
#define FINFO_PAGE02_BA_LV100NO_Y ( 15 )  // ビットマップエリア LV100値
#define FINFO_PAGE02_BA_LV100NO_SIZX ( 26 ) // ビットマップエリア LV100値
#define FINFO_PAGE02_BA_LV100NO_SIZY ( 4 )  // ビットマップエリア LV100値
#define FINFO_PAGE02_TT_X   ( 0 ) // タイトル
#define FINFO_PAGE02_TT_Y   ( 0 ) // タイトル
#define FINFO_PAGE02_LV50TT_X ( 0 ) // LV50タイトル
#define FINFO_PAGE02_LV50TT2_X  ( 120 ) // LV50タイトル
#define FINFO_PAGE02_LV50TT_Y ( 0 ) // LV50タイトル
#define FINFO_PAGE02_LV50LT_X ( 0 ) // LV50前回
#define FINFO_PAGE02_LV50LN0_X  ( 56 )  // LV50前回
#define FINFO_PAGE02_LV50LN1_X  ( 160 ) // LV50前回
#define FINFO_PAGE02_LV50L_Y  ( 0 ) // LV50前回
#define FINFO_PAGE02_LV50MT_X ( 0 ) // LV50最高
#define FINFO_PAGE02_LV50MN0_X  ( 56 )  // LV50最高
#define FINFO_PAGE02_LV50MN1_X  ( 160 ) // LV50最高
#define FINFO_PAGE02_LV50M_Y  ( 16 )  // LV50最高
#define FINFO_PAGE02_LV100TT_X  ( 0 ) // LV100タイトル
#define FINFO_PAGE02_LV100TT2_X ( 120 ) // LV100タイトル
#define FINFO_PAGE02_LV100TT_Y  ( 0 ) // LV100タイトル
#define FINFO_PAGE02_LV100LT_X  ( 0 ) // LV100前回
#define FINFO_PAGE02_LV100LN0_X ( 56 )  // LV100前回
#define FINFO_PAGE02_LV100LN1_X ( 160 ) // LV100前回
#define FINFO_PAGE02_LV100L_Y ( 0 ) // LV100前回
#define FINFO_PAGE02_LV100MT_X  ( 0 ) // LV100最高
#define FINFO_PAGE02_LV100MN0_X ( 56 )  // LV100最高
#define FINFO_PAGE02_LV100MN1_X ( 160 ) // LV100最高
#define FINFO_PAGE02_LV100M_Y ( 16 )  // LV100最高


// ページ03
enum{
  FINFO_PAGE03_BA,
  FINFO_PAGE03_BA_NUM,
};
#define FINFO_PAGE03_BA_X ( 2 ) // ビットマップエリア
#define FINFO_PAGE03_BA_Y ( 5 ) // ビットマップエリア
#define FINFO_PAGE03_BA_SIZX ( 26 ) // ビットマップエリア
#define FINFO_PAGE03_BA_SIZY ( 11 ) // ビットマップエリア

#define FINFO_PAGE03_TT_X ( 0 ) // タイトル
#define FINFO_PAGE03_TT_Y ( 0 ) // タイトル
#define FINFO_PAGE03_CPT_X  ( 104 ) // キャッスルポイントタイトル
#define FINFO_PAGE03_CPT_Y  ( 20 )  // キャッスルポイントタイトル
#define FINFO_PAGE03_CPLT_X ( 0 ) // キャッスルポイント前回
#define FINFO_PAGE03_CPLN0_X  ( 56 )  // キャッスルポイント前回
#define FINFO_PAGE03_CPLN1_X  ( 168 ) // キャッスルポイント前回
#define FINFO_PAGE03_CPL_Y  ( 48 )  // キャッスルポイント前回
#define FINFO_PAGE03_CPMT_X ( 0 ) // キャッスルポイント最大
#define FINFO_PAGE03_CPMN0_X  ( 56 )  // キャッスルポイント最大
#define FINFO_PAGE03_CPMN1_X  ( 168 ) // キャッスルポイント最大
#define FINFO_PAGE03_CPM_Y  ( 72 )  // キャッスルポイント最大


// ページ04
enum{
  FINFO_PAGE04_BA,
  FINFO_PAGE04_BA_NUM,
};
#define FINFO_PAGE04_BA_X ( 2 ) // ビットマップエリア
#define FINFO_PAGE04_BA_Y ( 5 ) // ビットマップエリア
#define FINFO_PAGE04_BA_SIZX ( 20 ) // ビットマップエリア
#define FINFO_PAGE04_BA_SIZY ( 11 ) // ビットマップエリア

#define FINFO_PAGE04_TT_X ( 0 ) // タイトル
#define FINFO_PAGE04_TT_Y ( 0 ) // タイトル
#define FINFO_PAGE04_TT2_Y  ( 24 )  // タイトル
#define FINFO_PAGE04_LT_X ( 0 ) // 前回
#define FINFO_PAGE04_LN_X ( 56 )  // 前回
#define FINFO_PAGE04_L_Y  ( 48 )  // 前回
#define FINFO_PAGE04_MT_X ( 0 ) // 最大
#define FINFO_PAGE04_MN_X ( 56 )  // 最大
#define FINFO_PAGE04_M_Y  ( 72 )  // 最大


// ページ05
enum{
  FINFO_PAGE05_BA,
  FINFO_PAGE05_BA_NUM,
};
#define FINFO_PAGE05_BA_X ( 2 ) // ビットマップエリア
#define FINFO_PAGE05_BA_Y ( 5 ) // ビットマップエリア
#define FINFO_PAGE05_BA_SIZX ( 20 ) // ビットマップエリア
#define FINFO_PAGE05_BA_SIZY ( 11 ) // ビットマップエリア

#define FINFO_PAGE05_TT_X ( 0 ) // タイトル
#define FINFO_PAGE05_TT_Y ( 0 ) // タイトル
#define FINFO_PAGE05_LT_X ( 0 ) // 前回
#define FINFO_PAGE05_LN_X ( 56 )  // 前回
#define FINFO_PAGE05_L_Y  ( 24 )  // 前回
#define FINFO_PAGE05_MT_X ( 0 ) // 最大
#define FINFO_PAGE05_MN_X ( 56 )  // 最大
#define FINFO_PAGE05_M_Y  ( 48 )  // 最大

// ページ06
enum{
  FINFO_PAGE06_BA,
  FINFO_PAGE06_BA_NUM,
};
#define FINFO_PAGE06_BA_X ( 2 ) // ビットマップエリア
#define FINFO_PAGE06_BA_Y ( 5 ) // ビットマップエリア
#define FINFO_PAGE06_BA_SIZX ( 22 ) // ビットマップエリア
#define FINFO_PAGE06_BA_SIZY ( 11 ) // ビットマップエリア

#define FINFO_PAGE06_TT_X ( 0 ) // タイトル
#define FINFO_PAGE06_TT_Y ( 0 ) // タイトル
#define FINFO_PAGE06_BC_X ( 0 ) // たまなげ
#define FINFO_PAGE06_BC_N_X ( 120 ) // 値
#define FINFO_PAGE06_BC_Y ( 24 )  // Y
#define FINFO_PAGE06_BB_X ( 0 ) // たまのり
#define FINFO_PAGE06_BB_N_X ( 120 ) // 値
#define FINFO_PAGE06_BB_Y ( 48 )  // Y
#define FINFO_PAGE06_BL_X ( 0 ) // たまのり
#define FINFO_PAGE06_BL_N_X ( 120 ) // 値
#define FINFO_PAGE06_BL_Y ( 72 )  // Y

// ページ07
enum{
  FINFO_PAGE07_BA,
  FINFO_PAGE07_BA_NUM,
};
#define FINFO_PAGE07_BA_X ( 1 ) // ビットマップエリア
#define FINFO_PAGE07_BA_Y ( 5 ) // ビットマップエリア
#define FINFO_PAGE07_BA_SIZX ( 27 ) // ビットマップエリア
#define FINFO_PAGE07_BA_SIZY ( 2 )  // ビットマップエリア

// ページ変更ウエイト
#define FINFO_PAGECHG_WAIT  ( 4 )
// ボタンアニメウェイト
#define WFNOTE_BTNANM_WAIT  (4)


#define TP_FINFO_BTN_H  (4*8)
#define TP_FINFO_BTN_Y  (20*8)

#define TP_FINFO_BTN01_W  (4*8)
#define TP_FINFO_BTN01_OX (4*8)
#define TP_FINFO_BTN01_X1 (0*0)
#define TP_FINFO_BTN01_X2 (TP_FINFO_BTN01_X1+TP_FINFO_BTN01_OX)
#define TP_FINFO_BTN01_X3 (TP_FINFO_BTN01_X2+TP_FINFO_BTN01_OX)
#define TP_FINFO_BTN01_X4 (TP_FINFO_BTN01_X3+TP_FINFO_BTN01_OX)
#define TP_FINFO_BTN01_X5 (TP_FINFO_BTN01_X4+TP_FINFO_BTN01_OX)
#define TP_FINFO_BTN01_X6 (TP_FINFO_BTN01_X5+TP_FINFO_BTN01_OX)
#define TP_FINFO_BTN01_X7 (TP_FINFO_BTN01_X6+TP_FINFO_BTN01_OX)
#define TP_FINFO_BTN01_X8 (TP_FINFO_BTN01_X7+TP_FINFO_BTN01_OX)

#define TP_FINFO_BTN02_W  (14*8)
#define TP_FINFO_BTN02_OX (TP_FINFO_BTN02_W)
#define TP_FINFO_BTN02_X1 (0*0)
#define TP_FINFO_BTN02_X2 (TP_FINFO_BTN02_X1+TP_FINFO_BTN02_OX)

#define TP_FINFO_YAZI_W   (32)
#define TP_FINFO_YAZI_H   (32)
#define TP_FINFO_YAZI_X   (29*8+4)
#define TP_FINFO_YAZI_Y1  (WFNOTE_YAZIRUSHI_TY-16)
#define TP_FINFO_YAZI_Y2  (WFNOTE_YAZIRUSHI_BY-16)

//////////////////////////////////////////////////////////////////////////////
//タッチパネル座標
//////////////////////////////////////////////////////////////////////////////
#define TP_MODESEL_BTN_X		(2*8+3)
#define TP_MODESEL_BTN_W		(28*8-6-1)
#define TP_MODESEL_BTN_H		(4*8-4-1)
#define TP_MODESEL_BTN_OY		(5*8)
#define TP_MODESEL_BTN_Y01  (5*8+2)
#define TP_MODESEL_BTN_Y02  (TP_MODESEL_BTN_Y01+TP_MODESEL_BTN_OY)
#define TP_MODESEL_BTN_Y03  (TP_MODESEL_BTN_Y02+TP_MODESEL_BTN_OY)
#define TP_MODESEL_BTN_Y04  (TP_MODESEL_BTN_Y03+TP_MODESEL_BTN_OY)

#define TP_MYCODE_BTN_X   (17*8)
#define TP_MYCODE_BTN_Y   (20*8)
#define TP_MYCODE_BTN_W   (14*8)
#define TP_MYCODE_BTN_H   (4*8)

//OBJ用リソースIDX
enum
{
/*
  WOR_OBJ_PLT,
  WOR_OBJ_NCG,
  WOR_OBJ_ANM,
  WNOTE_OBJ_RES_MAX,
*/
	// キャラ
	CHR_RES_WOR = 0,
	CHR_RES_TB,
	CHR_RES_MAX,
	// パレット
	PAL_RES_WOR = 0,
	PAL_RES_TB,
	PAL_RES_MAX,
	// セル
	CEL_RES_WOR = 0,
	CEL_RES_TB,
	CEL_RES_MAX,

}WNOTE_OBJ_RES;

#pragma mark[>struct
//-----------------------------------------------------------------------------
/**
 *          構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
/// メニューデータ構造体
//=====================================
typedef struct {
  u32 strid;
  u32 param;
} WFNOTE_MENU_DATA;


//-------------------------------------
//  スクリーンエリアデータ
//=====================================
typedef struct {
  s16 scrn_x;   // 描画エリア開始スクリーンｘ
  s16 scrn_y;   // 描画エリア開始スクリーンy
  u8 scrn_sizx; // 描画エリアサイズスクリーンｘ
  u8 scrn_sizy; // 描画エリアサイズスクリーンｙ
} WFNOTE_SCRNAREA;


//-------------------------------------
//  スクリーンデータ
//=====================================
typedef struct {
  void* p_scrnbuff[WFNOTE_SCRNDATA_NUM];  // スクリーンデータ
  NNSG2dScreenData* p_scrn[WFNOTE_SCRNDATA_NUM];  // スクリーンデータ
} WFNOTE_SCRNDATA;


//-------------------------------------
/// コード入力結果データ
//=====================================
typedef struct {
  STRBUF* p_name;
  STRBUF* p_code;
} WFNOTE_CODEIN_DATA;

//-------------------------------------
/// 名前入力初期化情報
//=====================================
typedef struct {
  const STRCODE* cp_str;
  BOOL init;
} WFNOTE_NAMEIN_INITDATA;


//-------------------------------------
/// 友達インデックスデータ
//=====================================
typedef struct {
  u32 friendnum;  // 友達の総数
  u32 infonum;  // 詳細情報の総数
  u8 fridx[FLIST_FRIEND_MAX]; // 友達インデックス
  u8 infoidx[FLIST_FRIEND_MAX]; // 詳細情報インデックス
} WFNOTE_IDXDATA;



//-------------------------------------
/// グローバルデータワーク
//=====================================
typedef struct {
  int key_mode;
  //SAVE_CONTROL_WORK* p_save;  // セーブデータ
  GAMEDATA* pGameData;
  u8 subseq;      // サブシーケンス
  u8 reqstatus;   // 次に進みたいステータス
  u8 reqsubseq;   // 次のステータスの開始サブシーケンス
  s8 select_listidx;  // 選択したリストインデックス

  u8  msg_spd;
  u8  win_type;
  u8  lr_key_enable;

  WFNOTE_IDXDATA idx; // 友達データインデックス

  WFNOTE_CODEIN_DATA codein;  // コード入力画面で入力したデータ
  WFNOTE_NAMEIN_INITDATA namein_init; // 名前入力画面初期化データ
} WFNOTE_DATA;

//-------------------------------------
/// 表示ワーク
//=====================================
typedef struct {
//  int key_mode;

//  GF_BGL_INI* p_bgl;    // bgl

//  GFL_CLUNIT *p_clact;  // アクター
//  NNSG2dRenderSurface scrll_surface;  // スクロール用サーフェース

  WORDSET* p_wordset;     // メッセージ展開用ワークマネージャー
  GFL_MSGDATA* p_msgman;  // 名前入力メッセージデータマネージャー

  ARCHANDLE* p_handle;    // グラフィックアーカイブハンドル

/*
#if USE_SEL_BOX
  SELBOX_SYS      *sbox;  // 選択ボックスシステムワーク
#endif
  TOUCH_SW_SYS    *ynbtn_wk;  ///<YesNoボタンワーク
*/

  GFL_BMPWIN *title;    // タイトル用ビットマップ
  STRBUF* p_titlestr;     // タイトル文字列
  STRBUF* p_titletmp;     // タイトル文字列
  STRBUF* p_str;        // 文字列汎用バッファ
  STRBUF* p_tmp;        // 文字列汎用バッファ
  u32 title_stridx;     // タイトル描画インデックス

  GFL_TCBLSYS   *msgTcblSys;
  PRINT_STREAM  *printHandle;   //タイトル用
  PRINT_STREAM  *printHandleMsg;  //メッセージ用

  PRINT_QUE   *printQue;

  //Obj系
  GFL_CLUNIT  *cellUnit;
  GFL_CLUNIT  *cellUnitScroll;
  GFL_CLSYS_REND  *scrollRender;
//  u32     objResIdx[WNOTE_OBJ_RES_MAX];
	u32	chrRes[CHR_RES_MAX];
	u32	palRes[PAL_RES_MAX];
	u32	celRes[CEL_RES_MAX];

  GFL_CLWK *p_yazirushi[WFNOTE_YAZIRUSHINUM]; // 矢印アクター
  GFL_CLWK *p_cur;  // カーソルアクター
  GFL_CLWK *p_tb;		// タッチバー×ボタンアクター

  WFNOTE_SCRNDATA scrn; // 基本スクリーンデータ

  GFL_FONT  *fontHandle;
  HEAPID    heapID;

	BLINKPALANM_WORK * bpawk;

} WFNOTE_DRAW;

//-------------------------------------
/// モード選択画面ワーク
//=====================================
typedef struct {
  // データ
  u32 cursor; // カーソル
  u32 wait;

	// ボタンアニメ
	u16	btn_anmSeq;		// シーケンス
	u16	btn_anmCnt;		// カウンタ

  // グラフィック
  GFL_BMPWIN *msg;  // メッセージ面
  void* p_scrnbuff; // スクリーンデータ
  NNSG2dScreenData* p_scrn; // スクリーンデータ
  GFL_BMPWIN *talk; // 会話ウィンドウ
  BMPMENU_WORK* p_yesno;// YESNOウィンドウ
  u32 talkmsg_idx;  // 会話メッセージidx
  STRBUF* p_str;

	// はい・いいえ
	APP_TASKMENU_ITEMWORK	ynlist[2];
	APP_TASKMENU_RES * listRes;
	APP_TASKMENU_WORK * listWork;

} WFNOTE_MODESELECT;

//-------------------------------------
/// 友達データ表示画面ワーク
//  描画エリアデータ
//=====================================
typedef struct {
  WFNOTE_SCRNAREA scrn_area;  // 描画スクリーンエリア
  GFL_BMPWIN **text;  // メッセージ面
  WF_2DCWK* p_clwk[FLIST_PAGE_FRIEND_NUM];

  GFL_CLWK * p_hate[FLIST_PAGE_FRIEND_NUM];

} WFNOTE_FLIST_DRAWAREA;

//-------------------------------------
/// 友達データカーソルデータ
//=====================================
typedef struct {
  WFNOTE_SCRNAREA scrn_data;
  u8  bg_frame;
  u8  off_pal,on_pal;
} WFNOTE_FLIST_CURSORDATA;


//-------------------------------------
/// 友達データ表示画面ワーク
//=====================================
typedef struct {
  s16 page; // ページ
  s16 lastpage;
  s16 pos;  // ページ内の位置
  s16 lastpos;// 1つ前の位置

	// ボタンアニメ
	u16	btn_anmSeq;		// シーケンス
	u16	btn_anmCnt;		// カウンタ

  WFNOTE_FLIST_DRAWAREA drawdata[WFNOTE_DRAWAREA_NUM];
  GFL_BMPWIN  *drawBmpWin[2];
  s16 count;  // スクロールカウンタ
  u16 way;  // スクロール方向
  WF_2DCSYS* p_charsys; // キャラクタ表示システム

//  GFL_BMPWIN *backmsg;  // 戻るメッセージ用

  GFL_BMPWIN *talk; // トークメッセージ用
  STRBUF* p_talkstr;
  u32 msgidx;
//  GF_BGL_BMPWIN menu; // メニューリスト用
//  BMPLIST_DATA* p_menulist[BMPL_FLIST_MENU_NUM];  // メニューリスト
//    BMPLIST_WORK* p_listwk; // BMPメニューワーク
//#if USE_SEL_BOX
//  SELBOX_WORK*  p_listwk; ///<選択ボックスワーク
//#endif
	APP_TASKMENU_ITEMWORK	list1[4];
	APP_TASKMENU_ITEMWORK	list2[4];
	APP_TASKMENU_ITEMWORK	ynlist[2];
	APP_TASKMENU_RES * listRes;
	APP_TASKMENU_WORK * listWork;
	u32	listType;

//  BMPMENU_WORK* p_yesno;  // YESNOウィンドウ

  s32 wait;

  GFL_CLWK *p_clearact;		// 消しゴムアクター
  GFL_CLWK * p_pageact;		// ページ番号アクター
  u32 last_frame;

} WFNOTE_FRIENDLIST;

//-------------------------------------
/// 友達コード入力画面ワーク
//=====================================
typedef struct {
  GFL_PROCSYS *p_subproc; // サブプロセス
  NAMEIN_PARAM* p_namein; // 名前入力パラメータ
  CODEIN_PARAM* p_codein; // コード入力パラメータ
} WFNOTE_CODEIN;

//-------------------------------------
/// 自分の友達コード確認画面ワーク
//=====================================
typedef struct {
  // グラフィックデータ
  GFL_BMPWIN *msg;  // メッセージ面
  GFL_BMPWIN *code; // コード面
//  GFL_BMPWIN *back; // もどる面
  void* p_scrnbuff; // スクリーンデータ
  NNSG2dScreenData* p_scrn; // スクリーンデータ

  int ct; //カウンタ
} WFNOTE_MYCODE;


//-------------------------------------
/// 友達詳細情報描画画面
/// 画面構成スクリーンデータ
//=====================================
typedef struct {
  void* p_scrnbuff; // スクリーンデータ
  NNSG2dScreenData* p_scrn; // スクリーンデータ
} WFNOTE_FINFO_SCRNDATA;

//-------------------------------------
/// 友達詳細情報描画画面
//  表示エリア情報
//=====================================
typedef struct {
  WFNOTE_SCRNAREA scrnarea;
  u32 cgx;
  GFL_BMPWIN**  p_msg[ FINFO_PAGE_NUM ];  //表示物
  u32 msgnum[ FINFO_PAGE_NUM ];   // 今のmsgビットマップ数
} WFNOTE_FINFO_DRAWAREA;

//-------------------------------------
/// 友達詳細情報表示画面ワーク
//=====================================
typedef struct {
  s16 page; // 現在のページ
  s16 lastpage; // １つ前のページ
  s32 count;  // スクロールカウント
  WF_COMMON_WAY way;  // 方向
  u16 wait; // ウエイト
  u16 rep;  // 上下リピート判定
  BOOL frontier_draw; // フロンティア描画有無


  WFNOTE_FINFO_DRAWAREA drawarea[WFNOTE_DRAWAREA_NUM];// 表示エリア
  WFNOTE_FINFO_SCRNDATA scrn[FINFO_SCRNDATA_NUM]; // 画面構成スクリーンデータ
  GFL_BMPWIN  *drawBmpWin[2];
//  GFL_BMPWIN  *win[ FINFO_WIN_NUM ];  //固定ウィンドウ
} WFNOTE_FRIENDINFO;


//-------------------------------------
/// ビットマップ登録データ
//=====================================
typedef struct {
  u8 pos_x;
  u8 pos_y;
  u8 siz_x;
  u8 siz_y;
} WFNOTE_FINFO_BMPDATA;



//-------------------------------------
/// 友達手帳ワーク
//=====================================
typedef struct {
  u32 status;     // 今の状態
  WFNOTE_DRAW draw; // 表示システムワーク
  WFNOTE_DATA data; // データワーク
  WIFINOTE_PROC_PARAM*  pp;

  // statusワーク
  WFNOTE_MODESELECT mode_select;  // モードセレクト
  WFNOTE_FRIENDLIST friendlist;   // 友達データ表示
  WFNOTE_CODEIN   codein;     // 友達コード入力
  WFNOTE_MYCODE   mycode;     // 自分の友達コード確認
  WFNOTE_FRIENDINFO friendinfo;   // 友達詳細表示画面

  GFL_TCB       *vBlankTcb;

  s32 wait;
} WFNOTE_WK;

#pragma mark[>prototype
//-----------------------------------------------------------------------------
/**
 *          プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
/// 全体関数
//=====================================
GFL_PROC_RESULT WifiNoteProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
GFL_PROC_RESULT WifiNoteProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
GFL_PROC_RESULT WifiNoteProc_End( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

static void WFNOTE_VBlank( GFL_TCB *tcp, void *work );
static void WFNOTE_DrawInit( WFNOTE_WK* p_wk, HEAPID heapID );
static void WFNOTE_DrawExit( WFNOTE_WK* p_wk );


//-------------------------------------
/// データワーク関数
//=====================================
static void Data_Init( WFNOTE_DATA* p_data, WIFINOTE_PROC_PARAM* wp, HEAPID heapID );
static void Data_Exit( WFNOTE_DATA* p_data, WIFINOTE_PROC_PARAM* wp );
static void Data_SetReqStatus( WFNOTE_DATA* p_data, u8 reqstatus, u8 reqsubseq );
//static void Data_CheckKeyModeChange( WFNOTE_DATA* p_data );
static void Data_CODEIN_Set( WFNOTE_DATA* p_data, const STRBUF* cp_name, const STRBUF* cp_code );
static u32 Data_NewFriendDataSet( WFNOTE_DATA* p_data, STRBUF* p_code, STRBUF* p_name );
static void Data_NAMEIN_INIT_Set( WFNOTE_DATA* p_data, const STRCODE* cp_str );
static void Data_SELECT_ListIdxSet( WFNOTE_DATA* p_data, u32 idx );
static void Data_FrIdxMake( WFNOTE_DATA* p_data );
static u32 Data_FrIdxGetIdx( const WFNOTE_DATA* cp_data, u32 idx );

//-------------------------------------
/// 描画ワーク関数
//=====================================
static void Draw_Init( WFNOTE_DRAW* p_draw, const WFNOTE_DATA* cp_data, HEAPID heapID );
static void Draw_Main( WFNOTE_DRAW* p_draw );
static void Draw_VBlank( WFNOTE_DRAW* p_draw );
static void Draw_Exit( WFNOTE_DRAW* p_draw );
static void Draw_BankInit( void );
static void Draw_BgInit( WFNOTE_DRAW* p_draw, HEAPID heapID );
static void Draw_BgExit( WFNOTE_DRAW* p_draw );
static void Draw_ClactInit( WFNOTE_DRAW* p_draw, HEAPID heapID );
static void Draw_ClactExit( WFNOTE_DRAW* p_draw );
static void Draw_MsgInit( WFNOTE_DRAW* p_draw, HEAPID heapID );
static void Draw_MsgExit( WFNOTE_DRAW* p_draw );
static void Draw_BmpInit( WFNOTE_DRAW* p_draw, const WFNOTE_DATA* cp_data, HEAPID heapID );
static void Draw_BmpExit( WFNOTE_DRAW* p_draw );
static void Draw_SCRNDATA_Init( WFNOTE_DRAW* p_draw, HEAPID heapID );
static void Draw_SCRNDATA_Exit( WFNOTE_DRAW* p_draw );
static void Draw_BmpTitleWrite( WFNOTE_DRAW* p_draw, GFL_BMPWIN** win, u32 msg_idx );
static void Draw_BmpTitleOff( WFNOTE_DRAW* p_draw );
static void Draw_FriendCodeSetWordset( WFNOTE_DRAW* p_draw, u64 code );
static void Draw_FriendNameSetWordset( WFNOTE_DRAW* p_draw, const WFNOTE_DATA* cp_data, u32 idx, HEAPID heapID );
static void Draw_FriendGroupSetWordset( WFNOTE_DRAW* p_draw,const WFNOTE_DATA* cp_data, u32 idx, HEAPID heapID );
static BOOL Draw_FriendDaySetWordset( WFNOTE_DRAW* p_draw,const WFNOTE_DATA* cp_data, u32 idx );
static void Draw_NumberSetWordset( WFNOTE_DRAW* p_draw, u32 num );
static void Draw_WflbyGameSetWordSet( WFNOTE_DRAW* p_draw, u32 num );
static void Draw_CommonActInit( WFNOTE_DRAW* p_draw, HEAPID heapID );
static void Draw_CommonActExit( WFNOTE_DRAW* p_draw );
static void Draw_YazirushiSetDrawFlag( WFNOTE_DRAW* p_draw, BOOL flag );
static void Draw_YazirushiSetAnmFlag( WFNOTE_DRAW* p_draw, BOOL flag );
static void Draw_YazirushiAnmChg( WFNOTE_DRAW* p_draw, u8 idx, u8 anm_no);
static void Draw_CursorActorSet( WFNOTE_DRAW* p_draw, BOOL draw_f, u8 anm_no );
static void Draw_CursorActorPriSet( WFNOTE_DRAW* p_draw, u8 bg_pri, u32 s_pri );
static APP_TASKMENU_WORK * Draw_YesNoWinSet( APP_TASKMENU_ITEMWORK * list, APP_TASKMENU_RES ** res, BOOL putType, HEAPID heapID );
static int Draw_YesNoWinMain( APP_TASKMENU_WORK * wk );

//-------------------------------------
/// 選択モード関数
//=====================================
static void ModeSelect_Init( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static WFNOTE_STRET ModeSelect_Main( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void ModeSelect_Exit( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void ModeSelect_DrawInit( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void ModeSelect_DrawExit( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void ModeSelectSeq_Init( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static BOOL ModeSelectSeq_Main( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void ModeSelect_CursorDraw( WFNOTE_MODESELECT* p_wk, WFNOTE_DRAW* p_draw );
static void ModeSelect_CursorScrnPalChange( WFNOTE_MODESELECT* p_wk, WFNOTE_DRAW* p_draw );
static void ModeSelect_EndDataSet( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data );
static void ModeSelect_DrawOff( WFNOTE_MODESELECT* p_wk, WFNOTE_DRAW* p_draw );
static void ModeSelect_TalkMsgPrint( WFNOTE_MODESELECT* p_wk, WFNOTE_DRAW* p_draw, u32 msgidx, HEAPID heapID );
static BOOL ModeSelect_TalkMsgEndCheck( const WFNOTE_MODESELECT* cp_wk, WFNOTE_DATA* p_data , WFNOTE_DRAW *p_draw );
static void ModeSelect_TalkMsgOff( WFNOTE_MODESELECT* p_wk, WFNOTE_DRAW* p_draw );
static BOOL ModeSelect_StatusChengeCheck( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data );

//-------------------------------------
/// 友達データ表示関数
//=====================================
static void FList_Init( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static WFNOTE_STRET FList_Main( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FList_Exit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void FList_DrawInit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FList_DrawExit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void FList_DrawOn( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FList_DrawOff( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static u32 FListSeq_Main( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void FListSeq_ScrollInit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static BOOL FListSeq_ScrollMain( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FListSeq_MenuInit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static u32 FListSeq_MenuWait( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static BOOL FListSeq_CodeInit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FListSeq_DeleteInit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static BOOL FListSeq_DeleteYesNoDraw( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static u32 FListSeq_DeleteWait( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FList_DrawPage( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, u32 page, u32 pos, u32 draw_area, HEAPID heapID );
static void FList_DrawShiori( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DRAW* p_draw, u32 page );
static void FList_DrawShioriEff00( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DRAW* p_draw, u32 page0, u32 page1 );
static void FList_DrawShioriEff01( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DRAW* p_draw, u32 page0, u32 page1 );
static void FList_ScrSeqChange( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WF_COMMON_WAY way );
static void FList_TalkMsgWrite( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DRAW* p_draw, u32 msgid, HEAPID heapID );
static BOOL FList_TalkMsgEndCheck( const WFNOTE_FRIENDLIST* cp_wk, WFNOTE_DATA* p_data ,WFNOTE_DRAW* p_draw );
static void FList_TalkMsgOff( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DRAW * p_draw );
static void FList_TalkMsgClear( WFNOTE_FRIENDLIST* p_wk );
static u32 FList_FRIdxGet( const WFNOTE_FRIENDLIST* cp_wk );
static void FList_DeleteAnmStart( WFNOTE_FRIENDLIST* p_wk ,WFNOTE_DRAW* p_draw );
static BOOL FList_DeleteAnmMain( WFNOTE_FRIENDLIST* p_wk ,WFNOTE_DRAW* p_draw );
static void FList_DeleteAnmOff( WFNOTE_FRIENDLIST* p_wk ,WFNOTE_DRAW* p_draw);
static void FListDrawArea_Init( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNAREA* cp_area, u32 text_cgx, HEAPID heapID );
static void FListDrawArea_Exit( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw );
static void FListDrawArea_BackWrite( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNDATA* cp_scrn );
//static void FListDrawArea_CursorWrite( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, u32 pos );
static void FListDrawArea_ActiveListWrite( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, u32 pos ,BOOL active_f);
static void FListDrawArea_WritePlayer( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, WF_2DCSYS* p_charsys, const WFNOTE_SCRNDATA* cp_scrn, const WFNOTE_IDXDATA* cp_idx, u32 page, HEAPID heapID );
static void FListDrawArea_DrawOff( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNDATA* cp_scrn );
static void FListDrawArea_CharWkDel( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw );
static void FListDrawArea_SetPlayer( WFNOTE_FLIST_DRAWAREA* p_wk, WF_2DCSYS* p_charsys, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNDATA* cp_scrn, u32 pos, WIFI_LIST* p_list, u32 idx, HEAPID heapID );
static void FListDrawArea_SetNoPlayer( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNDATA* cp_scrn, u32 pos, HEAPID heapID );


//-------------------------------------
/// 友達コード入力画面ワーク
//=====================================
static void CodeIn_Init( WFNOTE_CODEIN* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static WFNOTE_STRET CodeIn_Main( WFNOTE_CODEIN* p_wk, WFNOTE_WK* p_sys, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void CodeIn_Exit( WFNOTE_CODEIN* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void CodeIn_DrawInit( WFNOTE_CODEIN* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void CodeIn_DrawExit( WFNOTE_CODEIN* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static NAMEIN_PARAM* CodeIn_NameInParamMake( WFNOTE_CODEIN* p_wk, WFNOTE_DATA* p_data, HEAPID heapID );

//-------------------------------------
/// 自分の友達コード確認画面ワーク
//=====================================
static void MyCode_Init( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static WFNOTE_STRET MyCode_Main( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void MyCode_Exit( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static BOOL MyCode_Input( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void MyCode_DrawInit( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void MyCode_DrawExit( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void MyCodeSeq_Init( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void MyCode_DrawOff( WFNOTE_MYCODE* p_wk, WFNOTE_DRAW* p_draw );
static void MyCode_BackButtonDraw( WFNOTE_MYCODE* p_wk, WFNOTE_DRAW* p_draw ,BOOL on_f);

//-------------------------------------
/// 友達詳細情報
//=====================================
static void FInfo_Init( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static WFNOTE_STRET FInfo_Main( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfo_Exit( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static u32 FInfoInput_Exe( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw ,u32 code );
static u32 FInfoInput_Key( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static u32 FInfoInput_Touch( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, BOOL* touch_f );
static u32 FInfo_Input( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void FInfo_ScrollInit( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static BOOL FInfo_ScrollMain( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfo_DrawInit( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfo_DrawExit( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw );
static void FInfo_DrawOn( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfo_DrawOff( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DRAW* p_draw );
static void FInfo_DrawOffFriendChange( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DRAW* p_draw );
static void FInfo_DrawPage( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, u32 drawarea, u32 page, HEAPID heapID );
static void FInfo_DrawBaseInfo( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfo_DrawOffBaseInfo( WFNOTE_FRIENDINFO* p_wk );
static void FInfo_CursorDraw( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw, u32 page );
static void FInfo_CursorOff( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw );
static void FInfo_ShioriDraw( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw, u32 page );
static void FInfo_ShioriOff( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw );
static void FInfo_ShioriAnm00( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw, u32 page00, u32 page01 );
static void FInfo_ShioriAnm01( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw, u32 page00, u32 page01 );
static BOOL FInfo_SelectListIdxAdd( WFNOTE_DATA* p_data, WF_COMMON_WAY way );
static void FInfoScrnData_Init( WFNOTE_FINFO_SCRNDATA* p_scrn, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfoScrnData_Exit( WFNOTE_FINFO_SCRNDATA* p_scrn );
static void FInfoBmpWin_Init( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DRAW* p_draw );
static void FInfoBmpWin_Exit( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DRAW* p_draw );
static void FInfoDrawArea_Init( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNAREA* cp_scrnarea, u32 cgx, HEAPID heapID );
static void FInfoDrawArea_Exit( WFNOTE_FINFO_DRAWAREA* p_wk );
static void FInfoDrawArea_MsgBmpInit( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, u32 idx, u32 num, const WFNOTE_FINFO_BMPDATA* cp_data, HEAPID heapID );
static void FInfoDrawArea_MsgBmpExit( WFNOTE_FINFO_DRAWAREA* p_wk, u32 idx );
static void FInfoDrawArea_MsgBmpOnVReq( WFNOTE_FINFO_DRAWAREA* p_wk, u32 idx );
static void FInfoDrawArea_MsgBmpOffVReq( WFNOTE_FINFO_DRAWAREA* p_wk, u32 idx );
static void FInfoDrawArea_Page( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, u32 page, const WFNOTE_FINFO_SCRNDATA* cp_scrn, BOOL bf_on,HEAPID heapID );
static void FInfoDrawArea_PageOff( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw );
static void FInfoDraw_Page00( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfoDraw_Page01( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfoDraw_Page02( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfoDraw_Page03( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfoDraw_Page04( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfoDraw_Page05( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfoDraw_Page06( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfoDraw_Page07( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID );
static void FInfoDraw_Clean( WFNOTE_DRAW* p_draw, const WFNOTE_SCRNAREA* cp_scrnarea );
static void FInfoDraw_Bmp( WFNOTE_FINFO_DRAWAREA* p_wk, u32 page, u32 bmp, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, u32 msg_idx, u32 x, u32 y, u32 col, STRBUF* p_str, STRBUF* p_tmp );
static void FInfoDraw_BaseBmp( GFL_BMPWIN** win, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw,u32 msg_idx, u32 x, u32 y, u32 col );
static void FInfo_PageChange( WFNOTE_FRIENDINFO* p_wk, s32 add );

#if NOTE_TEMP_COMMENT //未使用。DP・Pt用？
static BOOL FInfo_PofinnCaseCheck( WFNOTE_DATA* p_data, HEAPID heapID );
#endif

//-------------------------------------
/// 移植用
//=====================================
static u64 WFNOTE_STRBUF_GetNumber( STRBUF *str , BOOL *pbFlag );

static BOOL CheckUITrg( WFNOTE_DATA * p_data );
static void Draw_BlinkAnmInit( WFNOTE_DRAW * p_draw, HEAPID heapID );
static void Draw_BlinkAnmExit( WFNOTE_DRAW * p_draw );

static void SetTopPageButtonPassive( WFNOTE_DRAW * p_draw, u32 pos );
static void SetTopPageButtonActive( WFNOTE_DRAW * p_draw, u32 pos );
static void ChangeTopPageButton( WFNOTE_DRAW * p_draw, u32 now, u32 old );
static void ResetTopPageButton( WFNOTE_MODESELECT * p_wk );
static BOOL MainTopPageButtonDecedeAnime( WFNOTE_MODESELECT * p_wk );

static void SetListPageButton( u32 pos, u32 pal );
static void SetListPageButtonPassive( WFNOTE_DRAW * p_draw, u32 pos );
static void SetListPageButtonActive( WFNOTE_DRAW * p_draw, u32 pos );
static void ChangeListPageButton( WFNOTE_DRAW * p_draw, u32 now, u32 old );
static void ResetListPageButton( WFNOTE_FRIENDLIST * p_wk );
static BOOL MainListPageButtonDecedeAnime( WFNOTE_FRIENDLIST * p_wk );

static void SetCancelButtonAnime( WFNOTE_DRAW * p_draw );
static void SetDispActive( WFNOTE_DRAW * p_draw, BOOL flg );
static void ChangeListPageNumAnime( WFNOTE_FRIENDLIST * p_wk, u32 page );
static void PutListTitle( WFNOTE_DRAW * p_draw );
static void SetListDispActive( WFNOTE_FRIENDLIST * p_wk, WFNOTE_DRAW * p_draw, BOOL flg );



#pragma mark[>Data
//-----------------------------------------------------------------------------
/**
 *    データ
 */
//-----------------------------------------------------------------------------

const GFL_PROC_DATA WifiNoteProcData =
{
  WifiNoteProc_Init,
  WifiNoteProc_Main,
  WifiNoteProc_End,
};

//　スクリーンエリア
static const WFNOTE_SCRNAREA DATA_ScrnArea[ WFNOTE_DRAWAREA_NUM ] = {
  { 0, 0, 32, 24 },
  { 32, 0, 32, 24 },
  { -32, 0, 32, 24 }
};

// BGコントロールデータ
static const GFL_BG_BGCNT_HEADER DATA_BgCntTbl[ WFNOTE_BG_NUM ] = {
  { // DFRM_BACK
    0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0xc000, GX_BG_CHARBASE_0x00000,0x4000, GX_BG_EXTPLTT_01,
    3, 0, 0, FALSE
  },
  { // DFRM_MSG
    0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0xc800, GX_BG_CHARBASE_0x04000,0x8000, GX_BG_EXTPLTT_01,
    0, 0, 0, FALSE
  },
  { // DFRM_SCROLL
    0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0xd000, GX_BG_CHARBASE_0x10000,0x8000, GX_BG_EXTPLTT_01,
    2, 0, 0, FALSE
  },
  { // DFRM_SCRMSG
    0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x18000,0x8000, GX_BG_EXTPLTT_01,
    1, 0, 0, FALSE
  },
/*
  { // UFRM_MSG
    0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000,0x8000, GX_BG_EXTPLTT_01,
    0, 0, 0, FALSE
  },
  { // UFRM_BASE
    0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000,0x0000, GX_BG_EXTPLTT_01,
    1, 0, 0, FALSE
  },
*/
  { // UFRM_BACK
    0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000,0x8000, GX_BG_EXTPLTT_01,
    0, 0, 0, FALSE
  },
};

// スクロール用サーフェース
static GFL_REND_SURFACE_INIT DATA_ScrollSurfaceRect = {
  0, 0,
  256, 192,
  CLSYS_DRAW_MAIN
};

static const GFL_DISP_VRAM vramSetTable = {
  GX_VRAM_BG_128_A,							// メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,				// メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

  GX_VRAM_OBJ_128_B,						// メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_16_I,					// サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

  GX_VRAM_TEX_NONE,							// テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,					// テクスチャパレットスロット

  GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
  GX_OBJVRAMMODE_CHAR_1D_128K,	// サブOBJマッピングモード
};

//-------------------------------------
/// FRIENDLIST
//=====================================
static const WFNOTE_FLIST_CURSORDATA DATA_FListCursorData[ FLIST_CURSORDATA_NUM ] = {
  // プレイヤー0～8
  {
    { 2, 4, 15, 4 },
    GFL_BG_FRAME2_M, BGPLT_M_BACK_2, BGPLT_M_BACK_9,
  },
  {
    { 2, 8, 15, 4 },
    GFL_BG_FRAME2_M, BGPLT_M_BACK_2, BGPLT_M_BACK_9,
  },
  {
    { 2, 12, 15, 4 },
    GFL_BG_FRAME2_M, BGPLT_M_BACK_2, BGPLT_M_BACK_9,
  },
  {
    { 2, 16, 15, 4 },
    GFL_BG_FRAME2_M, BGPLT_M_BACK_2, BGPLT_M_BACK_9,
  },
  {
    { 16, 4, 15, 4 },
    GFL_BG_FRAME2_M, BGPLT_M_BACK_2, BGPLT_M_BACK_9,
  },
  {
    { 16, 8, 15, 4 },
    GFL_BG_FRAME2_M, BGPLT_M_BACK_2, BGPLT_M_BACK_9,
  },
  {
    { 16, 12, 15, 4 },
    GFL_BG_FRAME2_M, BGPLT_M_BACK_2, BGPLT_M_BACK_9,
  },
  {
    { 16, 16, 15, 4 },
    GFL_BG_FRAME2_M, BGPLT_M_BACK_2, BGPLT_M_BACK_9,
  },
  // 戻る
  {
    { FLIST_BACK_X, FLIST_BACK_Y, FLIST_BACK_SIZX, FLIST_BACK_SIZY },
    GFL_BG_FRAME0_M, BGPLT_M_BACK_1, BGPLT_M_BACK_9,
  },
};

// メニューデータ
static const WFNOTE_MENU_DATA DATA_FListMenuTbl[BMPL_FLIST_MENU_NUM][BMPL_FLIST_MENU_LISTNUM] = {
  {
    { msg_wifi_note_09, (u32)SEQ_FLIST_INFOINIT },
    { msg_wifi_note_10, (u32)SEQ_FLIST_NAMECHG_INIT },
    { msg_wifi_note_11, (u32)SEQ_FLIST_DELETE_INIT },
    { msg_wifi_note_12, (u32)SEQ_FLIST_MAIN },
  },
  {
    { msg_wifi_note_33, (u32)SEQ_FLIST_CODE_INIT },
    { msg_wifi_note_10, (u32)SEQ_FLIST_NAMECHG_INIT },
    { msg_wifi_note_11, (u32)SEQ_FLIST_DELETE_INIT },
    { msg_wifi_note_12, (u32)SEQ_FLIST_MAIN },
  }
};

#if USE_SEL_BOX
///セレクトボックス　ヘッダデータ構造体
static const SELBOX_HEAD_PRM DATA_SBoxHeadParamFList = {
  TRUE,SBOX_OFSTYPE_CENTER, ///<ループフラグ、左寄せorセンタリング

  4,  ///<項目表示オフセットX座標(dot)

  DFRM_MSG,       ///<フレームNo
  BGPLT_M_SBOX,     ///<フレームパレットID
  0,0,          ///oam BG Pri,Software Pri
  SBOX_FLIST_FCGX,SBOX_FLIST_WCGX,  ///<ウィンドウcgx,フレームcgx
  SBOX_FLIST_FCGX_SIZ,    ///<BMPウィンドウ占有キャラクタ領域サイズ(char)
};
#endif
//-------------------------------------
/// FRIENDINFO
//=====================================
static const WFNOTE_FINFO_BMPDATA DATA_Page00Msg[ FINFO_PAGE00_BA_NUM ] = {
  {
    FINFO_PAGE00_BA_CAP_X, FINFO_PAGE00_BA_CAP_BTT_Y,
    FINFO_PAGE00_BA_CAP_SIZX, FINFO_PAGE00_BA_CAP_SIZY,
  },
  /*
  {
    FINFO_PAGE00_BA_CAP_X, FINFO_PAGE00_BA_CAP_TRT_Y,
    FINFO_PAGE00_BA_CAP_SIZX, FINFO_PAGE00_BA_CAP_SIZY,
  },
  {
    FINFO_PAGE00_BA_BT_X, FINFO_PAGE00_BA_BT_Y,
    FINFO_PAGE00_BA_BT_SIZX, FINFO_PAGE00_BA_BT_SIZY,
  },
  {
    FINFO_PAGE00_BA_TR_X, FINFO_PAGE00_BA_TR_Y,
    FINFO_PAGE00_BA_TR_SIZX, FINFO_PAGE00_BA_TR_SIZY,
  },
  */
};
static const WFNOTE_FINFO_BMPDATA DATA_Page01Msg[ FINFO_PAGE01_BA_NUM ] = {
  {
    FINFO_PAGE01_BA_CAP_X, FINFO_PAGE01_BA_CAP_Y,
    FINFO_PAGE01_BA_CAP_SIZX, FINFO_PAGE01_BA_CAP_SIZY,
  },
  /*
  {
    FINFO_PAGE01_BA_DAT_X, FINFO_PAGE01_BA_DAT_Y,
    FINFO_PAGE01_BA_DAT_SIZX, FINFO_PAGE01_BA_DAT_SIZY,
  },
  */
};
static const WFNOTE_FINFO_BMPDATA DATA_Page02Msg[ FINFO_PAGE02_BA_NUM ] = {
  {
    FINFO_PAGE02_BA_TT_X, FINFO_PAGE02_BA_TT_Y,
    FINFO_PAGE02_BA_TT_SIZX, FINFO_PAGE02_BA_TT_SIZY,
  },
  /*
  {
    FINFO_PAGE02_BA_LV50TT_X, FINFO_PAGE02_BA_LV50TT_Y,
    FINFO_PAGE02_BA_LV50TT_SIZX, FINFO_PAGE02_BA_LV50TT_SIZY,
  },
  {
    FINFO_PAGE02_BA_LV50NO_X, FINFO_PAGE02_BA_LV50NO_Y,
    FINFO_PAGE02_BA_LV50NO_SIZX, FINFO_PAGE02_BA_LV50NO_SIZY,
  },
  {
    FINFO_PAGE02_BA_LV100TT_X, FINFO_PAGE02_BA_LV100TT_Y,
    FINFO_PAGE02_BA_LV100TT_SIZX, FINFO_PAGE02_BA_LV100TT_SIZY,
  },
  {
    FINFO_PAGE02_BA_LV100NO_X, FINFO_PAGE02_BA_LV100NO_Y,
    FINFO_PAGE02_BA_LV100NO_SIZX, FINFO_PAGE02_BA_LV100NO_SIZY,
  },
  */
};
static const WFNOTE_FINFO_BMPDATA DATA_Page03Msg[ FINFO_PAGE03_BA_NUM ] = {
  {
    FINFO_PAGE03_BA_X, FINFO_PAGE03_BA_Y,
    FINFO_PAGE03_BA_SIZX, FINFO_PAGE03_BA_SIZY,
  },
};
static const WFNOTE_FINFO_BMPDATA DATA_Page04Msg[ FINFO_PAGE04_BA_NUM ] = {
  {
    FINFO_PAGE04_BA_X, FINFO_PAGE04_BA_Y,
    FINFO_PAGE04_BA_SIZX, FINFO_PAGE04_BA_SIZY,
  },
};
static const WFNOTE_FINFO_BMPDATA DATA_Page05Msg[ FINFO_PAGE05_BA_NUM ] = {
  {
    FINFO_PAGE05_BA_X, FINFO_PAGE05_BA_Y,
    FINFO_PAGE05_BA_SIZX, FINFO_PAGE05_BA_SIZY,
  },
};
static const WFNOTE_FINFO_BMPDATA DATA_Page06Msg[ FINFO_PAGE06_BA_NUM ] = {
  {
    FINFO_PAGE06_BA_X, FINFO_PAGE06_BA_Y,
    FINFO_PAGE06_BA_SIZX, FINFO_PAGE06_BA_SIZY,
  },
};
static const WFNOTE_FINFO_BMPDATA DATA_Page07Msg[ FINFO_PAGE07_BA_NUM ] = {
  {
    FINFO_PAGE07_BA_X, FINFO_PAGE07_BA_Y,
    FINFO_PAGE07_BA_SIZX, FINFO_PAGE07_BA_SIZY,
  },
};

// すべてまとめたテーブル
static const WFNOTE_FINFO_BMPDATA* DATA_FInfo_PageBmpP[ FINFO_PAGE_NUM ] = {
  DATA_Page00Msg, DATA_Page06Msg, DATA_Page01Msg, DATA_Page02Msg,
  DATA_Page03Msg, DATA_Page04Msg, DATA_Page05Msg, DATA_Page07Msg,

};
static const u8 DATA_FInfo_PageBmpNum[ FINFO_PAGE_NUM ] = {
  FINFO_PAGE00_BA_NUM,
  FINFO_PAGE06_BA_NUM,
  FINFO_PAGE01_BA_NUM,
  FINFO_PAGE02_BA_NUM,
  FINFO_PAGE03_BA_NUM,
  FINFO_PAGE04_BA_NUM,
  FINFO_PAGE05_BA_NUM,
  FINFO_PAGE07_BA_NUM,
};







//----------------------------------------------------------------------------
/**
 *  @brief  プロセス　初期化
 *
 *  @param  proc    プロセスワーク
 *  @param  seq     シーケンス
 *
 *  @retval PROC_RES_CONTINUE = 0,    ///<動作継続中
 *  @retval PROC_RES_FINISH,      ///<動作終了
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WifiNoteProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  WFNOTE_WK* p_wk;


  // wifi_2dcharを使用するためにOVERLAYをLOAD
  GFL_OVERLAY_Load(FS_OVERLAY_ID(wifi2dmap));

  // ワーク作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFINOTE, 0xC0000 );
  p_wk = GFL_PROC_AllocWork( proc, sizeof(WFNOTE_WK), HEAPID_WIFINOTE );
  GFL_STD_MemFill( p_wk, 0, sizeof(WFNOTE_WK) );
/*
  if( pwk == NULL )
  {
    p_wk->pp = GFL_HEAP_AllocMemory( HEAPID_WIFINOTE, sizeof( WIFINOTE_PROC_PARAM));
    p_wk->pp->pGameData = GAMEDATA_Create( HEAPID_WIFINOTE );
  }
  else
  {
    p_wk->pp = pwk;
  }
*/
	p_wk->pp = pwk;

#ifdef	PM_DEBUG
//  デバッグ起動で名前が無い時用
  {
    MYSTATUS  *myStatus = GAMEDATA_GetMyStatus(p_wk->pp->pGameData);//SaveData_GetMyStatus( p_wk->pp->saveControlWork );
    if( MyStatus_CheckNameClear( myStatus ) == TRUE )
    {
      STRBUF  *name = GFL_STR_CreateBuffer( 6 , HEAPID_WIFINOTE );
      u16   nameCode[5] = {L'ぶ',L'ら',L'っ',L'く',0xFFFF};
      //名前のセット
      GFL_STR_SetStringCode( name , nameCode );
      MyStatus_SetMyNameFromString( myStatus , name );
      GFL_STR_DeleteBuffer( name );
    }
  }
#endif	// PM_DEBUG

  // データワーク初期
  p_wk->vBlankTcb = NULL;
  Data_Init( &p_wk->data, p_wk->pp, HEAPID_WIFINOTE );

  // 描画システムワーク初期化
  Draw_Init( &p_wk->draw, &p_wk->data, HEAPID_WIFINOTE );

  // 各画面のワーク初期化
//  ModeSelect_Init( &p_wk->mode_select, &p_wk->data, &p_wk->draw, HEAPID_WIFINOTE );
//  FList_Init( &p_wk->friendlist, &p_wk->data, &p_wk->draw, HEAPID_WIFINOTE );
//  CodeIn_Init( &p_wk->codein, &p_wk->data, &p_wk->draw, HEAPID_WIFINOTE );
//  MyCode_Init( &p_wk->mycode, &p_wk->data, &p_wk->draw, HEAPID_WIFINOTE );
//  FInfo_Init( &p_wk->friendinfo, &p_wk->data, &p_wk->draw, HEAPID_WIFINOTE );

  // 割り込み
  if( p_wk->vBlankTcb == NULL )
  {
    p_wk->vBlankTcb = GFUser_VIntr_CreateTCB( WFNOTE_VBlank , p_wk , 16 );
  }
//  sys_VBlankFuncChange( WFNOTE_VBlank, p_wk );
//  sys_HBlankIntrStop();

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *  @brief  プロセス　メイン処理
 *
 *  @param  proc    プロセスワーク
 *  @param  seq     シーケンス
 *
 *  @retval PROC_RES_CONTINUE = 0,    ///<動作継続中
 *  @retval PROC_RES_FINISH,      ///<動作終了
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WifiNoteProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  WFNOTE_WK* p_wk  = mywk;
  WFNOTE_STRET stret;

  switch( *seq ){
  case WFNOTE_PROCSEQ_MAIN:

    switch( p_wk->status ){
    case STATUS_MODE_SELECT:  // 最初の選択画面
      stret = ModeSelect_Main( &p_wk->mode_select, &p_wk->data, &p_wk->draw, HEAPID_WIFINOTE );
      break;

    case STATUS_FRIENDLIST: // 友達データ表示
      stret = FList_Main( &p_wk->friendlist, &p_wk->data, &p_wk->draw, HEAPID_WIFINOTE );
      break;

    case STATUS_CODEIN:   // 友達コード入力
      stret = CodeIn_Main( &p_wk->codein, p_wk, &p_wk->data, &p_wk->draw, HEAPID_WIFINOTE );
      break;

    case STATUS_MYCODE:   // 自分の友達コード確認
      stret = MyCode_Main( &p_wk->mycode, &p_wk->data, &p_wk->draw );
      break;

    case STATUS_FRIENDINFO: // 友達詳細データ
      stret = FInfo_Main( &p_wk->friendinfo, &p_wk->data, &p_wk->draw,  HEAPID_WIFINOTE );
      break;

    case STATUS_END:      // 終了
      return GFL_PROC_RES_FINISH;
    }

    // 状態切り替え処理
    if( stret == WFNOTE_STRET_FINISH ){
      p_wk->status = p_wk->data.reqstatus;
      p_wk->data.subseq = p_wk->data.reqsubseq;
      p_wk->wait = STATUS_CHANGE_WAIT;
      (*seq) = WFNOTE_PROCSEQ_WAIT;
    }
    break;

  case WFNOTE_PROCSEQ_WAIT:
    p_wk->wait --;
    if( p_wk->wait <= 0 ){
      (*seq) = WFNOTE_PROCSEQ_MAIN;
    }

    break;
  }


  // 描画メイン
  Draw_Main( &p_wk->draw );

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  プロセス　破棄
 *
 *  @param  proc    プロセスワーク
 *  @param  seq     シーケンス
 *
 *  @retval PROC_RES_CONTINUE = 0,    ///<動作継続中
 *  @retval PROC_RES_FINISH,  GF_BGL_BmpWinInit   ///<動作終了
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WifiNoteProc_End( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  WFNOTE_WK* p_wk  = mywk;

  // 割り込み
//  sys_VBlankFuncChange( NULL, NULL );
//  sys_HBlankIntrStop();
  GFL_TCB_DeleteTask( p_wk->vBlankTcb );
  p_wk->vBlankTcb = NULL;

  // 各画面のワーク破棄
//  ModeSelect_Exit( &p_wk->mode_select, &p_wk->data, &p_wk->draw );
//  FList_Exit( &p_wk->friendlist, &p_wk->data, &p_wk->draw );
//  CodeIn_Exit( &p_wk->codein, &p_wk->data, &p_wk->draw );
//  MyCode_Exit( &p_wk->mycode, &p_wk->data, &p_wk->draw );
//  FInfo_Exit( &p_wk->friendinfo, &p_wk->data, &p_wk->draw );

  // 描画システムワーク破棄
  Draw_Exit( &p_wk->draw );

  // データワーク破棄
  Data_Exit( &p_wk->data, p_wk->pp );

  //GFL_HEAP_FreeMemory( p_wk->pp );  //この処理は外側で行われる

/*
  if( pwk == NULL )
  {
    GFL_HEAP_FreeMemory( p_wk->pp->pGameData );
  }
*/

  GFL_PROC_FreeWork( proc );        // PROCワーク開放
  GFL_HEAP_DeleteHeap( HEAPID_WIFINOTE );

  // オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(wifi2dmap) );

  return GFL_PROC_RES_FINISH;
}




//-----------------------------------------------------------------------------
/**
 *    プライベート関数
 */
//-----------------------------------------------------------------------------
//-------------------------------------
/// 全体関数
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  VBlank関数
 *
 *  @param  p_work  システムワーク
 */
//-----------------------------------------------------------------------------
static void WFNOTE_VBlank( GFL_TCB *tcp, void *work )
{
  WFNOTE_WK* p_wk = work;

  // 描画VBLANK
  Draw_VBlank( &p_wk->draw );
}

//----------------------------------------------------------------------------
/**
 *  @brief  WFNOTE　初期化
 *
 *  @param  p_wk    ワーク
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void WFNOTE_DrawInit( WFNOTE_WK* p_wk, HEAPID heapID )
{
  // 描画システムワーク初期化
  Draw_Init( &p_wk->draw, &p_wk->data, heapID );

  // 各画面のワーク初期化
//  ModeSelect_DrawInit( &p_wk->mode_select, &p_wk->data, &p_wk->draw, heapID );
//  FList_DrawInit( &p_wk->friendlist, &p_wk->data, &p_wk->draw, heapID );
//  CodeIn_DrawInit( &p_wk->codein, &p_wk->data, &p_wk->draw, heapID );
//  MyCode_DrawInit( &p_wk->mycode, &p_wk->data, &p_wk->draw, heapID );
//  FInfo_DrawInit( &p_wk->friendinfo, &p_wk->data, &p_wk->draw, heapID );

  // 割り込み
  if( p_wk->vBlankTcb == NULL )
  {
    p_wk->vBlankTcb = GFUser_VIntr_CreateTCB( WFNOTE_VBlank , p_wk , 16 );
  }
//  sys_VBlankFuncChange( WFNOTE_VBlank, p_wk );
//  sys_HBlankIntrStop();

}

//----------------------------------------------------------------------------
/**
 *  @brief  WFNOTE  破棄
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void WFNOTE_DrawExit( WFNOTE_WK* p_wk )
{
  // 割り込み
//  sys_VBlankFuncChange( NULL, NULL );
//  sys_HBlankIntrStop();
  GFL_TCB_DeleteTask( p_wk->vBlankTcb );
  p_wk->vBlankTcb = NULL;

  // 各画面のワーク破棄
//  ModeSelect_DrawExit( &p_wk->mode_select, &p_wk->data, &p_wk->draw );
//  FList_DrawExit( &p_wk->friendlist, &p_wk->data, &p_wk->draw );
//  CodeIn_DrawExit( &p_wk->codein, &p_wk->data, &p_wk->draw );
//  MyCode_DrawExit( &p_wk->mycode, &p_wk->data, &p_wk->draw );
//  FInfo_DrawExit( &p_wk->friendinfo, &p_wk->data, &p_wk->draw );

  // 描画システムワーク破棄
  Draw_Exit( &p_wk->draw );
}

//-------------------------------------
/// 汎用サブ関数
//=====================================

//----------------------------------------------------------------------------
/*
 *  @brief  BmpWindowの追加と初期化
 */
//-----------------------------------------------------------------------------
static void Sub_BmpWinAdd( GFL_BMPWIN ** win, u8 frmnum,
  u8 posx, u8 posy, u8 sizx, u8 sizy, u8 palnum, u16 chrofs ,u8 clear_col)
{
//  OS_TPrintf("[BG:%d][size:%d]\n",frmnum,sizx*sizy);
  *win = GFL_BMPWIN_Create( frmnum , posx , posy , sizx , sizy , palnum , GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(*win) , clear_col );
}

//-------------------------------------
/// データワーク関数
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  データワーク初期化
 *
 *  @param  p_data    データワーク
 *  @param  p_save    セーブデータ
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void Data_Init( WFNOTE_DATA* p_data, WIFINOTE_PROC_PARAM* wp, HEAPID heapID )
{
//  CONFIG* config;

  p_data->pGameData = wp->pGameData;
  p_data->codein.p_name = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  p_data->codein.p_code = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );

  //キーモード初期化
  p_data->key_mode = GFL_UI_CheckTouchOrKey();

  //コンフィグ
//  config = SaveData_GetConfig(p_data->p_save);
  p_data->msg_spd = MSGSPEED_GetWait();
  p_data->win_type = WINTYPE_01;
//  if(CONFIG_GetInputMode(config) != INPUTMODE_L_A){
    p_data->lr_key_enable = TRUE;
//  }else{
//    p_data->lr_key_enable = FALSE;
//  }
#if NOTE_TEMP_COMMENT
  MsgPrintSkipFlagSet( TRUE );
  MsgPrintTouchPanelFlagSet( TRUE );
#endif
}

//----------------------------------------------------------------------------
/**
 *  @brief  データワーク破棄
 *
 *  @param  p_data  データワーク
 */
//-----------------------------------------------------------------------------
static void Data_Exit( WFNOTE_DATA* p_data, WIFINOTE_PROC_PARAM* wp )
{
#if NOTE_TEMP_COMMENT
  MsgPrintSkipFlagSet( FALSE );
  MsgPrintTouchPanelFlagSet( FALSE );
#endif

  //キーモード書き戻し
  GFL_UI_SetTouchOrKey(p_data->key_mode);

  GFL_STR_DeleteBuffer( p_data->codein.p_name );
  GFL_STR_DeleteBuffer( p_data->codein.p_code );
}


//----------------------------------------------------------------------------
/**
 *  @brief  リクエストステータス設定
 *
 *  @param  p_data    ワーク
 *  @param  reqstatus リクエストステータス
 *  @param  reqsubseq リクエストサブシーケンス
 */
//-----------------------------------------------------------------------------
static void Data_SetReqStatus( WFNOTE_DATA* p_data, u8 reqstatus, u8 reqsubseq )
{
  p_data->reqstatus = reqstatus;
  p_data->reqsubseq = reqsubseq;
}

//----------------------------------------------------------------------------
/**
 *  @brief  キーモード遷移チェック
 *
 *  @param  p_data    ワーク
 *  @param  reqstatus リクエストステータス
 *  @param  reqsubseq リクエストサブシーケンス
 */
//-----------------------------------------------------------------------------
/*
static void Data_CheckKeyModeChange( WFNOTE_DATA* p_data )
{
  if(GFL_UI_KEY_GetTrg() & GFL_PAD_BUTTON_KTST_CHG){
    p_data->key_mode = GFL_APP_END_KEY;
  }
}
*/

//----------------------------------------------------------------------------
/**
 *  @brief  データにコード入力画面の結果を設定する
 *
 *  @param  p_data    ワーク
 *  @param  cp_name   名前
 *  @param  cp_code   コード
 */
//-----------------------------------------------------------------------------
static void Data_CODEIN_Set( WFNOTE_DATA* p_data, const STRBUF* cp_name, const STRBUF* cp_code )
{
  if( cp_name ){
    GFL_STR_CopyBuffer( p_data->codein.p_name, cp_name );
  }
  if( cp_code ){
    GFL_STR_CopyBuffer( p_data->codein.p_code, cp_code );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  最新の友達データ設定
 *
 *  @param  p_data    ワーク
 *  @param  p_code    コード文字列
 *  @param  p_name    名前文字列
 */
//-----------------------------------------------------------------------------
static u32 Data_NewFriendDataSet( WFNOTE_DATA* p_data, STRBUF* p_code, STRBUF* p_name )
{
  int i;
  WIFI_LIST* p_list;
  BOOL result;
  BOOL set_check;
  u64 friendcode;
  u64 mycode;
  DWCFriendData *p_dwc_list = NULL;
  DWCUserData *p_dwc_userdata = NULL;
  int ret;
  int pos;

  p_list = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );

  mycode = DWC_CreateFriendKey( WifiList_GetMyUserInfo(p_list) );

  for( i=0; i<WIFILIST_FRIEND_MAX; i++ ){
    result = WifiList_IsFriendData( p_list, i );
    if( result == FALSE ){
      friendcode = WFNOTE_STRBUF_GetNumber(p_code, &set_check);

      if( (set_check == TRUE) && (friendcode !=mycode) ){

        p_dwc_userdata = WifiList_GetMyUserInfo( p_list );
        if(!DWC_CheckFriendKey(p_dwc_userdata, friendcode))
        {
          // 友達コードが違う
          return RCODE_NEWFRIEND_SET_FRIENDKEYNG;
        }
        ret = GFL_NET_DWC_CheckFriendCodeByToken(p_list, friendcode, &pos);
        if(ret == DWCFRIEND_INLIST){
          // もう設定ずみ
          return RCODE_NEWFRIEND_SET_SETING;
        }
        // 成功
        p_dwc_list = WifiList_GetDwcDataPtr(p_list, i);
        DWC_CreateFriendKeyToken(p_dwc_list, friendcode);
        // 名前と性別設定 中性で設定
        WifiList_SetFriendName(p_list, i, p_name);
        WifiList_SetFriendInfo(p_list, i, WIFILIST_FRIEND_SEX, PM_NEUTRAL);
        return RCODE_NEWFRIEND_SET_OK;
      }else{
        // 友達コードが違う
        return RCODE_NEWFRIEND_SET_FRIENDKEYNG;
      }
    }
  }

  GF_ASSERT(0);
  // ここはとおらないはず
  return RCODE_NEWFRIEND_SET_OK;
}

//----------------------------------------------------------------------------
/**
 *  @brief  名前入力  初期化データ設定
 *
 *  @param  p_data    データ
 *  @param  cp_str    文字列
 */
//-----------------------------------------------------------------------------
static void Data_NAMEIN_INIT_Set( WFNOTE_DATA* p_data, const STRCODE* cp_str )
{
  p_data->namein_init.cp_str = cp_str;
  p_data->namein_init.init = TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  選択したリストインデックスを設定する
 *
 *  @param  p_data    データ
 *  @param  idx     インデックス
 */
//-----------------------------------------------------------------------------
static void Data_SELECT_ListIdxSet( WFNOTE_DATA* p_data, u32 idx )
{
  p_data->select_listidx = idx;
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達リスト作成
 *
 *  @param  p_data    データ
 */
//-----------------------------------------------------------------------------
static void Data_FrIdxMake( WFNOTE_DATA* p_data )
{
  int i;
  WIFI_LIST* p_list;

  GFL_STD_MemFill( &p_data->idx, 0, sizeof(WFNOTE_IDXDATA) );

  p_list = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );

  p_data->idx.friendnum = 0;
  for( i=0; i<FLIST_FRIEND_MAX; i++ ){
    if( WifiList_IsFriendData( p_list, i ) == TRUE ){
      p_data->idx.fridx[ p_data->idx.friendnum ] = i;

      if(WifiList_GetFriendInfo( p_list, i , WIFILIST_FRIEND_SEX ) == PM_NEUTRAL){
        p_data->idx.infoidx[p_data->idx.friendnum] = 0xFF;
      }else{
        p_data->idx.infoidx[p_data->idx.friendnum] = p_data->idx.infonum++;
      }

      p_data->idx.friendnum ++;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  WIFIリストテーブルから友達番号を取得する
 *
 *  @param  cp_data   データ
 *  @param  idx     リストインデックス
 *
 *  @return 友達番号
 */
//-----------------------------------------------------------------------------
static u32 Data_FrIdxGetIdx( const WFNOTE_DATA* cp_data, u32 idx )
{
  return cp_data->idx.fridx[ idx ];
}



//-------------------------------------
/// 描画ワーク関数
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  描画ワーク初期化
 *
 *  @param  p_draw    ワーク
 *  @param  cp_data   データ
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void Draw_Init( WFNOTE_DRAW* p_draw, const WFNOTE_DATA* cp_data, HEAPID heapID )
{
  // まず行うもの
  GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );
  p_draw->p_handle = GFL_ARC_OpenDataHandle( ARCID_WIFI_NOTE, heapID );

  p_draw->heapID = heapID;
  Draw_BankInit();
  Draw_BgInit( p_draw, heapID );
  Draw_ClactInit( p_draw, heapID );
  Draw_MsgInit( p_draw, heapID );
  Draw_BmpInit( p_draw, cp_data, heapID );
  Draw_CommonActInit( p_draw, heapID );
  Draw_SCRNDATA_Init( p_draw, heapID );

	Draw_BlinkAnmInit( p_draw, heapID );

  // OAM面表示
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画ワークメイン
 *
 *  @param  p_draw    ワーク
 */
//-----------------------------------------------------------------------------
static void Draw_Main( WFNOTE_DRAW* p_draw )
{
  PRINTSYS_QUE_Main( p_draw->printQue );
  GFL_TCBL_Main( p_draw->msgTcblSys );

  GFL_CLACT_SYS_Main();
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画ワークVブランク
 *
 *  @param  p_draw    ワーク
 */
//-----------------------------------------------------------------------------
static void Draw_VBlank( WFNOTE_DRAW* p_draw )
{
  /*
  GF_BGL_VBlankFunc( p_draw->p_bgl );
  DoVramTransferManager();
  REND_OAMTrans();
  */
  GFL_CLACT_SYS_VBlankFunc();
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画ワーク　破棄
 *
 *  @param  p_draw  ワーク
 */
//-----------------------------------------------------------------------------
static void Draw_Exit( WFNOTE_DRAW* p_draw )
{
	Draw_BlinkAnmExit( p_draw );

  Draw_CommonActExit( p_draw );
  Draw_SCRNDATA_Exit( p_draw );
  Draw_BmpExit( p_draw );
  Draw_MsgExit( p_draw );
  Draw_ClactExit( p_draw );
  Draw_BgExit( p_draw );

  GFL_ARC_CloseDataHandle( p_draw->p_handle );
}

//----------------------------------------------------------------------------
/**
 *  @brief  バンク設定
 */
//-----------------------------------------------------------------------------
static void Draw_BankInit( void )
{
  GFL_DISP_SetBank( &vramSetTable );
}

//----------------------------------------------------------------------------
/**
 *  @brief  BG初期化
 *
 *  @param  p_draw    ワーク
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void Draw_BgInit( WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  int i;

  GFL_BG_Init( heapID );

  // BG SYSTEM
  {
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &BGsys_data );
  }

  // BGコントロール設定
  for( i=0; i<WFNOTE_BG_NUM; i++ ){
    GFL_BG_SetBGControl( DFRM_BACK+i, &DATA_BgCntTbl[i], GFL_BG_MODE_TEXT );
    GFL_BG_SetClearCharacter( DFRM_BACK+i, 32, 0, heapID );
    GFL_BG_ClearScreenCode( DFRM_BACK+i , 0 );
    GFL_BG_SetVisible( DFRM_BACK+i , TRUE );
  }

  // 基本的なバックグラウンド読込み＆転送
  // キャラクタデータは、スクロール面にも転送
  GFL_ARCHDL_UTIL_TransVramPalette( p_draw->p_handle, NARC_wifi_note_friend_bg_d_NCLR, PALTYPE_MAIN_BG,
      BGPLT_M_BACK_0, BGPLT_M_BACK_NUM*32, heapID );
  GFL_ARCHDL_UTIL_TransVramPalette( p_draw->p_handle, NARC_wifi_note_friend_bg_d_NCLR, PALTYPE_SUB_BG,
      BGPLT_S_BACK_0, BGPLT_S_BACK_NUM*32, heapID );

  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_draw->p_handle, NARC_wifi_note_friend_bg_d_NCGR,
      DFRM_BACK, 0, 0, FALSE, heapID );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_draw->p_handle, NARC_wifi_note_friend_bg_d_NCGR,
      DFRM_SCROLL, 0, 0, FALSE, heapID );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_draw->p_handle, NARC_wifi_note_friend_bg_d_NCGR,
      UFRM_BACK, 0, 0, FALSE, heapID );

  GFL_ARCHDL_UTIL_TransVramScreen( p_draw->p_handle, NARC_wifi_note_friend_back_d_NSCR,
      DFRM_BACK, 0, 0, FALSE, heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( p_draw->p_handle, NARC_wifi_note_friend_top_u_NSCR,
      UFRM_BACK, 0, 0, FALSE, heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  BG破棄
 *
 *  @param  p_draw    ワーク
 */
//-----------------------------------------------------------------------------
static void Draw_BgExit( WFNOTE_DRAW* p_draw )
{
  int i;

  // BGコントロール破棄
  for( i=0; i<WFNOTE_BG_NUM; i++ ){
    GFL_BG_FreeBGControl( DFRM_BACK+i );
  }

  GFL_BG_Exit();
}

//----------------------------------------------------------------------------
/**
 *  @brief  セルアクター初期化
 *
 *  @param  p_draw    ワーク
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void Draw_ClactInit( WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  //OBJの初期化
  GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
  cellSysInitData.oamst_main = 0x10;  //デバッグメータの分
  cellSysInitData.oamnum_main = 128-0x10;
  GFL_CLACT_SYS_Create( &cellSysInitData , &vramSetTable ,heapID );

  p_draw->cellUnit  = GFL_CLACT_UNIT_Create( 32 , 0, heapID );
  GFL_CLACT_UNIT_SetDefaultRend( p_draw->cellUnit );

  //スクロール用レンダラー作成
  p_draw->scrollRender = GFL_CLACT_USERREND_Create(&DATA_ScrollSurfaceRect,1,heapID );
  p_draw->cellUnitScroll = GFL_CLACT_UNIT_Create( 18 , 1 , heapID );
  GFL_CLACT_UNIT_SetUserRend( p_draw->cellUnitScroll , p_draw->scrollRender );

  //リソースの読み込み
	// 友達手帳専用
  p_draw->chrRes[CHR_RES_WOR] = GFL_CLGRP_CGR_Register(
																	p_draw->p_handle,
																	NARC_wifi_note_friend_obj_d_NCGR,
																	FALSE , CLSYS_DRAW_MAIN, heapID );
  p_draw->palRes[PAL_RES_WOR] = GFL_CLGRP_PLTT_Register(
																	p_draw->p_handle,
																	NARC_wifi_note_friend_obj_d_NCLR,
																	CLSYS_DRAW_MAIN, CLACT_PALNUM_WIFINOTE*0x20, heapID );
  p_draw->celRes[CEL_RES_WOR] = GFL_CLGRP_CELLANIM_Register(
																	p_draw->p_handle,
																	NARC_wifi_note_friend_obj_d_NCER,
																	NARC_wifi_note_friend_obj_d_NANR, heapID );
	// タッチバー
	{
	  ARCHANDLE * ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), heapID );
		p_draw->chrRes[CHR_RES_TB] = GFL_CLGRP_CGR_Register(
																	ah, APP_COMMON_GetBarIconCharArcIdx(),
																	FALSE, CLSYS_DRAW_MAIN, heapID );
		p_draw->palRes[PAL_RES_TB] = GFL_CLGRP_PLTT_Register(
																	ah, APP_COMMON_GetBarIconPltArcIdx(),
																	CLSYS_DRAW_MAIN, CLACT_PALNUM_TB*0x20, heapID );
		p_draw->celRes[CEL_RES_TB] = GFL_CLGRP_CELLANIM_Register(
																	ah,
																	APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_128K),
																	APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_128K),
																	heapID );
	  GFL_ARC_CloseDataHandle( ah );
	}
}


//----------------------------------------------------------------------------
/**
 *  @brief  セルアクター破棄
 *
 *  @param  p_draw  ワーク
 */
//-----------------------------------------------------------------------------
static void Draw_ClactExit( WFNOTE_DRAW* p_draw )
{
	u32	i;

	for( i=0; i<CHR_RES_MAX; i++ ){
		GFL_CLGRP_CGR_Release( p_draw->chrRes[i] );
	}
	for( i=0; i<PAL_RES_MAX; i++ ){
		GFL_CLGRP_PLTT_Release( p_draw->palRes[i] );
	}
	for( i=0; i<CEL_RES_MAX; i++ ){
		GFL_CLGRP_CELLANIM_Release( p_draw->celRes[i] );
	}
  GFL_CLACT_UNIT_Delete( p_draw->cellUnitScroll );
  GFL_CLACT_USERREND_Delete( p_draw->scrollRender );
  GFL_CLACT_UNIT_Delete( p_draw->cellUnit );
  GFL_CLACT_SYS_Delete();
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ　初期化
 *
 *  @param  p_draw    ワーク
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void Draw_MsgInit( WFNOTE_DRAW* p_draw, HEAPID heapID )
{
	GFL_FONTSYS_SetDefaultColor();		// デフォルト文字色セット（これがないと最初のメッセージ表示で文字が出ない）

  p_draw->p_wordset = WORDSET_Create( heapID );
  p_draw->p_msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_note_dat, heapID );

  //タッチフォントロード
  p_draw->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , heapID );

  p_draw->msgTcblSys = GFL_TCBL_Init( heapID , heapID , 3 , 0x100 );
  p_draw->printHandle = NULL;
  p_draw->printHandleMsg = NULL;

  p_draw->printQue = PRINTSYS_QUE_Create( heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ　破棄
 *
 *  @param  p_draw    ワーク
 */
//-----------------------------------------------------------------------------
static void Draw_MsgExit( WFNOTE_DRAW* p_draw )
{
  PRINTSYS_QUE_Clear( p_draw->printQue );
  PRINTSYS_QUE_Delete( p_draw->printQue );

  if( p_draw->printHandle != NULL ){
    PRINTSYS_PrintStreamDelete( p_draw->printHandle );
    p_draw->printHandle = NULL;
  }
  if( p_draw->printHandleMsg != NULL ){
    PRINTSYS_PrintStreamDelete( p_draw->printHandleMsg );
    p_draw->printHandleMsg = NULL;
  }

  GFL_TCBL_Exit( p_draw->msgTcblSys );
  //タッチフォントアンロード
  GFL_FONT_Delete( p_draw->fontHandle );

  GFL_MSG_Delete( p_draw->p_msgman );
  WORDSET_Delete( p_draw->p_wordset );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ビットマップ関連初期化
 *
 *  @param  p_draw
 *  @param  cp_data
 *  @param  heapID
 */
//-----------------------------------------------------------------------------
static void Draw_BmpInit( WFNOTE_DRAW* p_draw, const WFNOTE_DATA* cp_data, HEAPID heapID )
{
  // 基本ウィンドウデータ読込み
//  TalkFontPaletteLoad( PALTYPE_MAIN_BG, BGPLT_M_MSGFONT*0x20, heapID );
//  TalkFontPaletteLoad( PALTYPE_SUB_BG, BGPLT_S_MSGFONT*0x20, heapID );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , BGPLT_M_MSGFONT*0x20, 16*2, heapID );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , BGPLT_S_MSGFONT*0x20, 16*2, heapID );

  BmpWinFrame_GraphicSet( DFRM_MSG, BMPL_WIN_CGX_TALK ,
    BGPLT_M_TALKWIN, cp_data->win_type, heapID );

  // 基本ビットマップ
  GFL_BMPWIN_Init( heapID );
  Sub_BmpWinAdd( &p_draw->title ,
      DFRM_MSG,
      BMPL_TITLE_X, BMPL_TITLE_Y,
      BMPL_TITLE_SIZX, BMPL_TITLE_SIZY,
      BGPLT_M_MSGFONT, BMPL_TITLE_CGX , 0 );

  // タイトル用文字列バッファ作成
  p_draw->p_titlestr = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  p_draw->p_titletmp = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  p_draw->p_str = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  p_draw->p_tmp = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );

/*
#if USE_SEL_BOX
  p_draw->sbox = SelectBoxSys_AllocWork( heapID, NULL );
#endif

  //YesNoボタンシステムワーク確保
  p_draw->ynbtn_wk = TOUCH_SW_AllocWork(heapID);
*/
}

//----------------------------------------------------------------------------
/**
 *  @brief  ビットマップ破棄
 *
 *  @param  p_draw  ワーク
 */
//-----------------------------------------------------------------------------
static void Draw_BmpExit( WFNOTE_DRAW* p_draw )
{
/*
  //YesNoボタンシステムワーク解放
  TOUCH_SW_FreeWork( p_draw->ynbtn_wk );

#if USE_SEL_BOX
  SelectBoxSys_Free(p_draw->sbox);
#endif
*/

  GFL_STR_DeleteBuffer( p_draw->p_str );
  GFL_STR_DeleteBuffer( p_draw->p_tmp );
  GFL_STR_DeleteBuffer( p_draw->p_titlestr );
  GFL_STR_DeleteBuffer( p_draw->p_titletmp );
  GFL_BMPWIN_Delete( p_draw->title );
  GFL_BMPWIN_Exit();
}

//----------------------------------------------------------------------------
/**
 *  @brief  基本スクリーンデータ  初期化
 *
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void Draw_SCRNDATA_Init( WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  static const u8 sc_SCRNDATA[ WFNOTE_SCRNDATA_NUM ] = {
    NARC_wifi_note_friend_listwin_d_NSCR,
    NARC_wifi_note_friend_parts_d_NSCR,
  };
  int i;
  // スクリーンデータ初期化
  for( i=0; i<WFNOTE_SCRNDATA_NUM; i++ ){
    p_draw->scrn.p_scrnbuff[i] = GFL_ARCHDL_UTIL_LoadScreen( p_draw->p_handle,
        sc_SCRNDATA[i], FALSE, &p_draw->scrn.p_scrn[i], heapID );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  スクリーンデータ破棄
 *
 *  @param  p_draw  描画システム
 */
//-----------------------------------------------------------------------------
static void Draw_SCRNDATA_Exit( WFNOTE_DRAW* p_draw )
{
  int i;

  // スクリーンデータ初期化
  for( i=0; i<WFNOTE_SCRNDATA_NUM; i++ ){
    GFL_HEAP_FreeMemory( p_draw->scrn.p_scrnbuff[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  タイトルの描画
 *
 *  @param  p_draw  描画ワーク
 *  @param  msg_idx メッセージID
 */
//-----------------------------------------------------------------------------
static void Draw_BmpTitleWrite( WFNOTE_DRAW* p_draw, GFL_BMPWIN** win, u32 msg_idx )
{
  // タスク動作停止
  if( p_draw->printHandle != NULL ){
    PRINTSYS_PrintStreamDelete( p_draw->printHandle );
    p_draw->printHandle = NULL;
  }

  // クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(*win), 0 );

  GFL_MSG_GetString( p_draw->p_msgman, msg_idx, p_draw->p_titletmp );
  WORDSET_ExpandStr( p_draw->p_wordset, p_draw->p_titlestr, p_draw->p_titletmp );
/*
  p_draw->printHandle = PRINTSYS_PrintStream(*win,  0, BMPL_TITLE_PL_Y,
      p_draw->p_titlestr, p_draw->fontHandle , WFNOTE_TITLE_MSG_SPEED,
      p_draw->msgTcblSys , 10 , p_draw->heapID , PRINTSYS_LSB_GetB(WFNOTE_COL_WHITE) );
*/
	PRINTSYS_PrintQueColor(
		p_draw->printQue, GFL_BMPWIN_GetBmp(*win),
		0, BMPL_TITLE_PL_Y,
		p_draw->p_titlestr, p_draw->fontHandle, WFNOTE_COL_WHITE );

//    GF_BGL_BmpWinOnVReq(win);
	GFL_BMPWIN_TransVramCharacter( *win );
  GFL_BMPWIN_MakeScreen( *win );
  GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(*win) );
}

//----------------------------------------------------------------------------
/**
 *  @brief  タイトル表示Off
 *
 *  @param  p_draw    ワーク
 */
//-----------------------------------------------------------------------------
static void Draw_BmpTitleOff( WFNOTE_DRAW* p_draw )
{
  // タスク動作停止
  if( p_draw->printHandle != NULL ){
    PRINTSYS_PrintStreamDelete( p_draw->printHandle );
    p_draw->printHandle = NULL;
  }

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_draw->title), 0 );
  GFL_BMPWIN_TransVramCharacter( p_draw->title );
  GFL_BMPWIN_ClearTransWindow_VBlank( p_draw->title );
//  GF_BGL_BmpWinOffVReq(&p_draw->title);
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達コードをワードセットに設定
 *
 *  @param  p_draw  描画システム
 *  @param  code  コード
 */
//-----------------------------------------------------------------------------
static void Draw_FriendCodeSetWordset( WFNOTE_DRAW* p_draw, u64 code )
{
  u64 num;

  num = code / 100000000;
  WORDSET_RegisterNumber(p_draw->p_wordset, 1, num, 4,STR_NUM_DISP_ZERO,STR_NUM_CODE_DEFAULT);
  num = (code/10000) % 10000;
  WORDSET_RegisterNumber(p_draw->p_wordset, 2, num, 4,STR_NUM_DISP_ZERO,STR_NUM_CODE_DEFAULT);
  num = code % 10000;
  WORDSET_RegisterNumber(p_draw->p_wordset, 3, num, 4,STR_NUM_DISP_ZERO,STR_NUM_CODE_DEFAULT);

}

//----------------------------------------------------------------------------
/**
 *  @brief  友達の名前を設定する
 *
 *  @param  p_draw    描画システム
 *  @param  p_save    セーブデータ
 *  @param  idx     友達のインデックス
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void Draw_FriendNameSetWordset( WFNOTE_DRAW* p_draw, const WFNOTE_DATA* cp_data, u32 idx, HEAPID heapID )
{
  WIFI_LIST* p_wifilist;
  MYSTATUS* p_mystatus;

  p_wifilist = GAMEDATA_GetWiFiList(cp_data->pGameData); //SaveData_GetWifiListData( p_save );
  p_mystatus = MyStatus_AllocWork( heapID );
#if NOTE_DEBUG
  {
    STRBUF  *nameStr = GFL_STR_CreateBuffer( 10 , heapID );
    GFL_STR_SetStringCode( nameStr, WifiList_GetFriendNamePtr(p_wifilist, idx) );
    MyStatus_SetMyNameFromString( p_mystatus, nameStr );
    GFL_STR_DeleteBuffer( nameStr );
  }
#else
  MyStatus_SetMyName( p_mystatus, WifiList_GetFriendNamePtr(p_wifilist, idx) );
#endif
  WORDSET_RegisterPlayerName( p_draw->p_wordset, 0, p_mystatus );
  GFL_HEAP_FreeMemory( p_mystatus );
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達のグループをワードセットに設定する
 *
 *  @param  p_draw    描画システム
 *  @param  p_save    データ
 *  @param  idx     インデックス
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void Draw_FriendGroupSetWordset( WFNOTE_DRAW* p_draw, const WFNOTE_DATA* cp_data, u32 idx, HEAPID heapID )
{
#if 0
  MYSTATUS* p_mystatus = MyStatus_AllocWork( heapID );
  WIFI_LIST* p_wifilist = GAMEDATA_GetWiFiList(cp_data->pGameData); //SaveData_GetWifiListData( p_save );
#if NOTE_DEBUG
  {
    STRBUF  *nameStr = GFL_STR_CreateBuffer( 10 , heapID );
    GFL_STR_SetStringCode( nameStr, WifiList_GetFriendGroupNamePtr( p_wifilist, idx ) );
    MyStatus_SetMyNameFromString( p_mystatus, nameStr );
    GFL_STR_DeleteBuffer( nameStr );
  }
#else
  MyStatus_SetMyName(p_mystatus, WifiList_GetFriendGroupNamePtr( p_wifilist, idx ));
#endif
  WORDSET_RegisterPlayerName( p_draw->p_wordset, 0, p_mystatus );
  GFL_HEAP_FreeMemory( p_mystatus );
#endif
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達と最後に遊んだ日にちを設定
 *
 *  @param  p_draw    描画システム
 *  @param  p_save    セーブデータ
 *  @param  idx     友達リストインデックス
 *
 *  @retval TRUE  日にちがあった
 *  @retval FALSE 日にちがなかった
 */
//-----------------------------------------------------------------------------
static BOOL Draw_FriendDaySetWordset( WFNOTE_DRAW* p_draw, const WFNOTE_DATA* cp_data, u32 idx )
{
  WIFI_LIST* p_wifilist = GAMEDATA_GetWiFiList(cp_data->pGameData); //SaveData_GetWifiListData( p_save );
  u32 num;
  BOOL ret = TRUE;

  num = WifiList_GetFriendInfo( p_wifilist, idx, WIFILIST_FRIEND_LASTBT_DAY);
  if( num == 0 ){
    ret = FALSE;  // ０ということは１回も遊んでいない
  }
  WORDSET_RegisterNumber( p_draw->p_wordset, 2, num, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
  num = WifiList_GetFriendInfo( p_wifilist, idx, WIFILIST_FRIEND_LASTBT_YEAR);
  WORDSET_RegisterNumber( p_draw->p_wordset, 0, num, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
  num = WifiList_GetFriendInfo( p_wifilist, idx, WIFILIST_FRIEND_LASTBT_MONTH);
  WORDSET_RegisterNumber( p_draw->p_wordset, 1, num, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);

  return ret;
}

//----------------------------------------------------------------------------
/**
 *  @brief  ワードセットに数字を設定する
 *
 *  @param  p_draw    描画システム
 *  @param  num     数字
 */
//-----------------------------------------------------------------------------
static void Draw_NumberSetWordset( WFNOTE_DRAW* p_draw, u32 num )
{
  WORDSET_RegisterNumber( p_draw->p_wordset, 0, num, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
}

//----------------------------------------------------------------------------
/**
 *  @brief  ワードセットにロビーミニゲーム名を設定
 *
 *  @param  p_draw
 *  @param  num
 */
//-----------------------------------------------------------------------------
static void Draw_WflbyGameSetWordSet( WFNOTE_DRAW* p_draw, u32 num )
{
#if 0 //WiFi広場関連は削除
  WORDSET_RegisterWiFiLobbyGameName( p_draw->p_wordset, 0, num );
#endif
}

//----------------------------------------------------------------------------
/**
 *  @brief  矢印他　コモンアクター初期化
 *
 *  @param  p_draw  描画ワーク
 *  @param  heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void Draw_CommonActInit( WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  static GFL_CLWK_DATA act_add[4] = {
    { // ←
      WFNOTE_YAZIRUSHI_LX ,
      WFNOTE_YAZIRUSHI_LRY ,
      ACTANM_YAZI_LN ,
      WFNOTE_YAZIRUSHI_PRI,
      0
    },
    { // →
      WFNOTE_YAZIRUSHI_RX ,
      WFNOTE_YAZIRUSHI_LRY ,
      ACTANM_YAZI_RN ,
      WFNOTE_YAZIRUSHI_PRI,
      0
    },
    { // カーソル
      0 ,
      0 ,
      ACTANM_CUR_LIST ,
      WFNOTE_YAZIRUSHI_PRI,
      0
    },
    { // タッチバーキャンセルボタン
			TOUCHBAR_ICON_X_07,
			TOUCHBAR_ICON_Y,
			APP_COMMON_BARICON_RETURN,
      0,
      2
    },
  };
  int i;

  // 矢印アクター作成
  for( i=0; i<WFNOTE_YAZIRUSHINUM; i++ ){
    p_draw->p_yazirushi[i] = GFL_CLACT_WK_Create(
          p_draw->cellUnit,
          p_draw->chrRes[CHR_RES_WOR],
          p_draw->palRes[PAL_RES_WOR],
          p_draw->celRes[CEL_RES_WOR],
          &act_add[i], CLSYS_DEFREND_MAIN,
          heapID );

    GFL_CLACT_WK_SetAutoAnmFlag( p_draw->p_yazirushi[i], TRUE );
    GFL_CLACT_WK_SetDrawEnable( p_draw->p_yazirushi[i], FALSE );
  }
  //カーソルアクター作成
  //リスト画面のキャラとの前後関係でこっちのunitに登録
  p_draw->p_cur = GFL_CLACT_WK_Create(
        p_draw->cellUnitScroll,
        p_draw->chrRes[CHR_RES_WOR],
        p_draw->palRes[PAL_RES_WOR],
        p_draw->celRes[CEL_RES_WOR],
        &act_add[2] , CLSYS_DEFREND_MAIN,
        heapID );
  GFL_CLACT_WK_SetDrawEnable( p_draw->p_cur, FALSE );

	// タッチバーキャンセルボタンアクター作成
  p_draw->p_tb = GFL_CLACT_WK_Create(
        p_draw->cellUnit,
        p_draw->chrRes[CHR_RES_TB],
        p_draw->palRes[PAL_RES_TB],
        p_draw->celRes[CEL_RES_TB],
        &act_add[3] , CLSYS_DEFREND_MAIN,
        heapID );
  GFL_CLACT_WK_SetDrawEnable( p_draw->p_tb, TRUE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  矢印他　コモンアクター破棄
 *
 *  @param  p_draw  描画ワーク
 */
//-----------------------------------------------------------------------------
static void Draw_CommonActExit( WFNOTE_DRAW* p_draw )
{
  int i;

  //タッチバーキャンセルアクター破棄
  GFL_CLACT_WK_Remove( p_draw->p_tb );
  //カーソルアクター破棄
  GFL_CLACT_WK_Remove( p_draw->p_cur );
  // 矢印アクター破棄
  for( i=0; i<WFNOTE_YAZIRUSHINUM; i++ ){
    GFL_CLACT_WK_Remove( p_draw->p_yazirushi[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  矢印表示設定
 *
 *  @param  p_draw  描画ワーク
 *  @param  flag  フラグ
 */
//-----------------------------------------------------------------------------
static void Draw_YazirushiSetDrawFlag( WFNOTE_DRAW* p_draw, BOOL flag )
{
  int i;
  for( i=0; i<WFNOTE_YAZIRUSHINUM; i++ ){
    GFL_CLACT_WK_SetDrawEnable( p_draw->p_yazirushi[i], flag );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  矢印アニメ設定
 *
 *  @param  p_draw  描画ワーク
 *  @param  flag  フラグ
 */
//-----------------------------------------------------------------------------
static void Draw_YazirushiSetAnmFlag( WFNOTE_DRAW* p_draw, BOOL flag )
{
  int i;
  for( i=0; i<WFNOTE_YAZIRUSHINUM; i++ ){
    GFL_CLACT_WK_SetAutoAnmFlag( p_draw->p_yazirushi[i], flag );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  矢印アニメ再生
 *
 *  @param  p_draw  描画ワーク
 *  @param  flag  フラグ
 */
//-----------------------------------------------------------------------------
static void Draw_YazirushiAnmChg( WFNOTE_DRAW* p_draw, u8 idx, u8 anm_no)
{
  u8 anm_ofs = idx*2;
  if(idx >= ACT_YAZI_T){
    idx -= ACT_YAZI_T;
  }
  GFL_CLACT_WK_SetAutoAnmFlag( p_draw->p_yazirushi[idx], TRUE );
  GFL_CLACT_WK_SetAnmSeq( p_draw->p_yazirushi[idx], ACTANM_YAZI_LN+anm_ofs+anm_no);
  GFL_CLACT_WK_ResetAnm( p_draw->p_yazirushi[idx] );
}

//----------------------------------------------------------------------------
/**
 *  @brief  カーソルアクター表示設定
 *
 *  @param  p_draw  描画ワーク
 *  @param  flag  フラグ
 */
//-----------------------------------------------------------------------------
static void Draw_CursorActorSet( WFNOTE_DRAW* p_draw, BOOL draw_f, u8 anm_no )
{
  if(anm_no != ACTANM_NULL){
    GFL_CLACT_WK_SetAnmSeq( p_draw->p_cur, anm_no );
  }
  GFL_CLACT_WK_SetDrawEnable( p_draw->p_cur, draw_f );
}

//----------------------------------------------------------------------------
/**
 *  @brief  カーソルアクター表示プライオリティ設定
 *
 *  @param  p_draw  描画ワーク
 *  @param  flag  フラグ
 */
//-----------------------------------------------------------------------------
static void Draw_CursorActorPriSet( WFNOTE_DRAW* p_draw, u8 bg_pri, u32 s_pri )
{
  GFL_CLACT_WK_SetBgPri( p_draw->p_cur, bg_pri );
  GFL_CLACT_WK_SetSoftPri( p_draw->p_cur, s_pri );
}

//----------------------------------------------------------------------------
/**
 *  @brief  YesNoウィンドウ開始
 *
 *  @param  p_data  データ
 *  @param  p_draw  描画システム
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * Draw_YesNoWinSet( APP_TASKMENU_ITEMWORK * list, APP_TASKMENU_RES ** res, BOOL putType, HEAPID heapID )
{
	APP_TASKMENU_INITWORK	wk;

  wk.heapId   = heapID;
  wk.itemNum  = 2;
  wk.itemWork = list;
	wk.posType  = ATPT_RIGHT_DOWN;
  wk.charPosX = 32;
	if( putType == TRUE ){
		 wk.charPosY = 17;
	}else{
		 wk.charPosY = 12;
	}
	wk.w        = APP_TASKMENU_PLATE_WIDTH_YN_WIN;
	wk.h        = APP_TASKMENU_PLATE_HEIGHT_YN_WIN;

	return APP_TASKMENU_OpenMenu( &wk, *res );
}

//----------------------------------------------------------------------------
/**
 *  @brief  YesNoウィンドウ終了待ち
 *
 *  @param  p_data  データ
 *  @param  p_draw  描画システム
 */
//-----------------------------------------------------------------------------
static int Draw_YesNoWinMain( APP_TASKMENU_WORK * wk )
{
	APP_TASKMENU_UpdateMenu( wk );
	if( APP_TASKMENU_IsFinish( wk ) == TRUE ){
		APP_TASKMENU_CloseMenu( wk );
		return APP_TASKMENU_GetCursorPos( wk );
	}
	return -1;
}

//-------------------------------------
/// 選択モード関数
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  モード選択画面　ワーク初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画ワーク
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void ModeSelect_Init( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  p_data->key_mode = GFL_UI_CheckTouchOrKey();	// キータッチ設定
  ModeSelect_DrawInit( p_wk, p_data, p_draw, heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  モード選択画面  メイン
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 *
 *  @retval WFNOTE_STRET_CONTINUE,  // つづく
 *  @retval WFNOTE_STRET_FINISH,  // 作業は終わった
 */
//-----------------------------------------------------------------------------
static WFNOTE_STRET ModeSelect_Main( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  BOOL result;

  switch( p_data->subseq ){
  case SEQ_MODESEL_INIT:  // 初期化
    ModeSelect_Init( p_wk, p_data, p_draw, HEAPID_WIFINOTE );

    ModeSelectSeq_Init( p_wk, p_data, p_draw );
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_HOLEIN, WIPE_TYPE_HOLEIN, WIPE_FADE_BLACK,
        WIPE_DEF_DIV, WIPE_DEF_SYNC, heapID);
    p_data->subseq = SEQ_MODESEL_FADEINWAIT;
    break;

  case SEQ_MODESEL_FADEINWAIT:
    if( WIPE_SYS_EndCheck() ){
      p_data->subseq = SEQ_MODESEL_MAIN;
    }
    break;

  case SEQ_MODESEL_INIT_NOFADE:
    ModeSelect_Init( p_wk, p_data, p_draw, HEAPID_WIFINOTE );
    ModeSelectSeq_Init( p_wk, p_data, p_draw );
    p_data->subseq = SEQ_MODESEL_MAIN;
    break;

  case SEQ_MODESEL_MAIN:
		BLINKPALANM_Main( p_draw->bpawk );
    result = ModeSelectSeq_Main( p_wk, p_data, p_draw );
    if( result == FALSE ){
      break;
    }
		if( p_wk->cursor == MODESEL_CUR_END ){
			SetCancelButtonAnime( p_draw );
		}else{
			p_wk->btn_anmSeq = 0;
		}
    p_data->subseq = SEQ_MODESEL_MAIN_ENDWAIT;
    break;

  case SEQ_MODESEL_MAIN_ENDWAIT:
		// ボタンアニメ
		if( p_wk->cursor == MODESEL_CUR_END ){
			if( GFL_CLACT_WK_CheckAnmActive( p_draw->p_tb ) == TRUE ){
				break;
			}
		}else{
			if( MainTopPageButtonDecedeAnime( p_wk ) == TRUE ){
				break;
			}
		}

    // その状態に遷移して大丈夫かチェック
//    if( ModeSelect_StatusChengeCheck( p_wk, p_data ) == TRUE && !(GFL_UI_KEY_GetCont() & PAD_BUTTON_R) ){
    if( ModeSelect_StatusChengeCheck( p_wk, p_data ) == TRUE ){
      // カーソル位置に対応した終了設定を行う
      ModeSelect_EndDataSet( p_wk, p_data );

      // 終了チェック
      if( (p_wk->cursor == MODESEL_CUR_END) ||
        (p_wk->cursor == MODESEL_CUR_CODEIN) ){
        p_data->subseq = SEQ_MODESEL_FADEOUT;
      }else{
        ModeSelect_DrawOff( p_wk, p_draw );
        ModeSelect_Exit( p_wk, p_data, p_draw );
        return WFNOTE_STRET_FINISH;
      }
    }else{
//      if( p_wk->cursor == MODESEL_CUR_CODEIN ){
        // メッセージをだして
        // メッセージ終了待ちする
        ModeSelect_TalkMsgPrint( p_wk, p_draw, msg_wifi_note_30, heapID );
        p_data->subseq = SEQ_MODESEL_CODECHECK_RESLUTMSGWAIT;
//      }
    }
    break;
    // そのまま終了するとき
  case SEQ_MODESEL_FADEOUT:
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_HOLEOUT, WIPE_TYPE_HOLEOUT, WIPE_FADE_BLACK,
            WIPE_DEF_DIV, WIPE_DEF_SYNC, heapID);

    p_data->subseq ++;
    break;

  case SEQ_MODESEL_FADEOUTWAIT:
    if( WIPE_SYS_EndCheck() == TRUE ){
      ModeSelect_Exit( p_wk, p_data, p_draw );
      return WFNOTE_STRET_FINISH;
    }
    break;

    // コード入力画面からの復帰
  case SEQ_MODESEL_CODECHECK_INIT:
    ModeSelect_Init( p_wk, p_data, p_draw, HEAPID_WIFINOTE );

    ModeSelectSeq_Init( p_wk, p_data, p_draw );
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_HOLEIN, WIPE_TYPE_HOLEIN, WIPE_FADE_BLACK,
        WIPE_DEF_DIV, WIPE_DEF_SYNC, heapID);
    p_data->subseq = SEQ_MODESEL_CODECHECK_FADEINWAIT;
    break;

  case SEQ_MODESEL_CODECHECK_FADEINWAIT:
    if( WIPE_SYS_EndCheck() ){
      u64 code;
      MYSTATUS* p_friend;
      // メッセージの表示
      // 名前を設定
      p_friend = MyStatus_AllocWork( heapID );
      MyStatus_SetMyNameFromString(p_friend, p_data->codein.p_name );
      WORDSET_RegisterPlayerName( p_draw->p_wordset, 0, p_friend );
      GFL_HEAP_FreeMemory( p_friend );

      // 友達コード設定
      code = WFNOTE_STRBUF_GetNumber(p_data->codein.p_code, &result);
      if( result ){ // 数字に変換できたか
        Draw_FriendCodeSetWordset( p_draw, code );

        ModeSelect_TalkMsgPrint( p_wk, p_draw, msg_wifi_note_26, heapID );
        p_data->subseq = SEQ_MODESEL_CODECHECK_MSGWAIT;
      }else{

        // コードが間違っていると表示
        ModeSelect_TalkMsgPrint( p_wk, p_draw, msg_wifi_note_27, heapID );
        p_data->subseq = SEQ_MODESEL_CODECHECK_RESLUTMSGWAIT;  // ともだちコード違う表示
      }
    }
    break;

  case SEQ_MODESEL_CODECHECK_MSGWAIT:   //登録確認メッセージON
    if( ModeSelect_TalkMsgEndCheck( p_wk, p_data , p_draw ) == FALSE ){
      break;
    }

    // はいいいえウィンドウ表示
		p_wk->listWork = Draw_YesNoWinSet( p_wk->ynlist, &p_wk->listRes, TRUE, heapID );
    p_data->subseq = SEQ_MODESEL_CODECHECK_MSGYESNOWAIT;
    break;

  case SEQ_MODESEL_CODECHECK_MSGYESNOWAIT:  //はいいいえ選択待ち
    {
      int ret;
      u32 set_ret;
      static const u8 msgdata[ RCODE_NEWFRIEND_SET_NUM ] = {
        0,
        msg_wifi_note_27,
        msg_wifi_note_31,
      };

      ret = Draw_YesNoWinMain( p_wk->listWork );

      switch(ret){
      case 0:
				p_data->key_mode = GFL_UI_CheckTouchOrKey();
        // 登録作業をして結果を表示
        set_ret = Data_NewFriendDataSet( p_data, p_data->codein.p_code, p_data->codein.p_name );
        if( set_ret == RCODE_NEWFRIEND_SET_OK ){
          // 成功したのでそのまま終わる
          ModeSelect_TalkMsgOff( p_wk, p_draw );
					ResetTopPageButton( p_wk );
          p_data->subseq = SEQ_MODESEL_MAIN;
        }else{
          ModeSelect_TalkMsgPrint( p_wk, p_draw, msgdata[set_ret], heapID );
          p_data->subseq = SEQ_MODESEL_CODECHECK_RESLUTMSGWAIT;  // ともだちコード違う表示
        }
        break;
      case 1:
				p_data->key_mode = GFL_UI_CheckTouchOrKey();
        // メッセージを消して終わる
        ModeSelect_TalkMsgOff( p_wk, p_draw );
				ResetTopPageButton( p_wk );
        p_data->subseq = SEQ_MODESEL_MAIN;
        break;
      default:
        break;
      }
    }
    break;

  case SEQ_MODESEL_CODECHECK_RESLUTMSGWAIT: //選択結果表示
    if( ModeSelect_TalkMsgEndCheck( p_wk,p_data ,p_draw) == FALSE ){
      break;
    }
		if( CheckUITrg( p_data ) == TRUE ){
      ModeSelect_TalkMsgOff( p_wk, p_draw );
			ResetTopPageButton( p_wk );
      p_data->subseq = SEQ_MODESEL_MAIN;
    }
    break;
  }
  return WFNOTE_STRET_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  モード選択画面  ワーク破棄
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void ModeSelect_Exit( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  ModeSelect_DrawExit( p_wk, p_data, p_draw );
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画関係の初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void ModeSelect_DrawInit( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  STRBUF* p_str;
  STRBUF* p_tmp;
  int i;

  // ビットマップウィンドウ作成
  Sub_BmpWinAdd( &p_wk->msg, DFRM_SCRMSG,
      BMPL_MODESEL_MSG_X, BMPL_MODESEL_MSG_Y,
      BMPL_MODESEL_MSG_SIZX, BMPL_MODESEL_MSG_SIZY,
      BGPLT_M_MSGFONT, BMPL_MODESEL_MSGCGX , 0 );

  Sub_BmpWinAdd( &p_wk->talk, DFRM_MSG,
      BMPL_MODESEL_TALK_X, BMPL_MODESEL_TALK_Y,
      BMPL_MODESEL_TALK_SIZX, BMPL_MODESEL_TALK_SIZY,
      BGPLT_M_MSGFONT, BMPL_MODESEL_TALKCGX , 0 );

  // 会話ウィンドウ用メッセージバッファ確保
  p_wk->p_str = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );

  // メッセージ書き込み
  p_str = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  p_tmp = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );

  // 自分の名前をワードセットに設定
  WORDSET_RegisterPlayerName( p_draw->p_wordset, 0, GAMEDATA_GetMyStatus(p_data->pGameData) );

  for( i=0; i<3; i++ ){
    u32 siz;
    GFL_MSG_GetString( p_draw->p_msgman, msg_top_menu01+i, p_tmp );
    WORDSET_ExpandStr( p_draw->p_wordset, p_str, p_tmp );

    siz = PRINTSYS_GetStrWidth( p_str, p_draw->fontHandle , 0 );
    PRINTSYS_PrintQueColor(
			p_draw->printQue , GFL_BMPWIN_GetBmp(p_wk->msg),
			((BMPL_MODESEL_MSG_SIZX*8)-siz)/2,
			8+(40*i),
      p_str, p_draw->fontHandle, WFNOTE_COL_BLACK );
  }

  GFL_BMPWIN_TransVramCharacter( p_wk->msg );
  GFL_BG_LoadScreenV_Req( DFRM_SCRMSG );

  GFL_STR_DeleteBuffer( p_str );
  GFL_STR_DeleteBuffer( p_tmp );

  // カーソルスクリーン読み込み
  p_wk->p_scrnbuff = GFL_ARCHDL_UTIL_LoadScreen( p_draw->p_handle,
      NARC_wifi_note_friend_top_d_NSCR, FALSE, &p_wk->p_scrn, heapID );

	// はい・いいえ
	p_wk->ynlist[0].str      = GFL_MSG_CreateString( p_draw->p_msgman, msg_wifi_note_yes );
	p_wk->ynlist[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
	p_wk->ynlist[0].type     = APP_TASKMENU_WIN_TYPE_NORMAL;
	p_wk->ynlist[1].str      = GFL_MSG_CreateString( p_draw->p_msgman, msg_wifi_note_no );
	p_wk->ynlist[1].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
	p_wk->ynlist[1].type     = APP_TASKMENU_WIN_TYPE_NORMAL;

	p_wk->listRes = APP_TASKMENU_RES_Create(
										DFRM_MSG, BGPLT_M_SBOX, p_draw->fontHandle, p_draw->printQue, heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画関係の破棄
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void ModeSelect_DrawExit( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
	// はい・いいえ破棄
	APP_TASKMENU_RES_Delete( p_wk->listRes );
	GFL_STR_DeleteBuffer( p_wk->ynlist[0].str );
	GFL_STR_DeleteBuffer( p_wk->ynlist[1].str );

  // 会話文字列バッファ破棄
  GFL_STR_DeleteBuffer( p_wk->p_str );

  // ビットマップ破棄
  GFL_BMPWIN_Delete( p_wk->talk );
  GFL_BMPWIN_Delete( p_wk->msg );

  // スクリーンデータ破棄
  GFL_HEAP_FreeMemory( p_wk->p_scrnbuff );
}

//----------------------------------------------------------------------------
/**
 *  @brief  シーケンス初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データワーク
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void ModeSelectSeq_Init( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  // タイトル表示
  Draw_BmpTitleWrite( p_draw, &p_draw->title, msg_wifi_note_01 );
  GFL_BG_LoadScreenV_Req( DFRM_MSG );

  // 文字列描画
  //TODO
  //GF_BGL_BmpWinOnVReq( &p_wk->msg );
  GFL_BMPWIN_MakeScreen( p_wk->msg );
  GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(p_wk->msg) );
  // ボタン表示
  GFL_BG_WriteScreen( DFRM_SCROLL,
      p_wk->p_scrn->rawData, 0, 0,
      p_wk->p_scrn->screenWidth/8, p_wk->p_scrn->screenHeight/8 );
  // スクロール座標を元に戻す
  GFL_BG_SetScroll( DFRM_SCROLL, GFL_BG_SCROLL_X_SET, 0 );
  GFL_BG_SetScroll( DFRM_SCROLL, GFL_BG_SCROLL_Y_SET, 0 );

  // カーソルアクター表示設定
  Draw_CursorActorSet( p_draw, FALSE , ACTANM_CUR_MSEL );
//  Draw_CursorActorPriSet( p_draw, 1, 0 );

  // カーソル位置のパレットを設定
//  ModeSelect_CursorScrnPalChange( p_wk, p_draw );
//  ModeSelect_CursorDraw( p_wk, p_draw );
	if( p_data->key_mode == GFL_APP_END_KEY ){
		SetTopPageButtonActive( p_draw, p_wk->cursor );
	}else{
		GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
	}

  // BG面転送
//  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
}

//----------------------------------------------------------------------------
/**
 *  @brief  モード選択画面 キー入力
 *
 *  @param  p_wk    モード選択画面ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画ワーク
 *
 *  @retval TRUE  何かを選択
 *  @retval FALSE 何もおきていない
 */
//-----------------------------------------------------------------------------
/*
static BOOL ModeSelectInput_Key( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  BOOL move = FALSE;

  if( GFL_UI_KEY_GetTrg() & GFL_PAD_BUTTON_KTST_CHG){
    p_data->key_mode = GFL_APP_END_KEY;
  }

  // 選択チェック
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
    ModeSelect_CursorScrnPalChange( p_wk, p_draw );
    PMSND_PlaySE(WIFINOTE_DECIDE_SE);
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
    return TRUE;
  }

  // キャンセルチェック
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
    PMSND_PlaySE(WIFINOTE_BS_SE);
    p_wk->cursor = MODESEL_CUR_END; // 終了にする
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
    return TRUE;
  }

  // 移動チェック
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
    if( p_wk->cursor > 0 ){
      p_wk->cursor--;
      move = TRUE;
    }
  }else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
    if( p_wk->cursor < MODESEL_CUR_MYCODE ){
      p_wk->cursor++;
      move = TRUE;
    }
  }

  // 移動したら音と描画更新
  if( move == TRUE ){
    PMSND_PlaySE( WIFINOTE_MOVE_SE );
//    ModeSelect_CursorScrnPalChange( p_wk, p_draw );
    ModeSelect_CursorDraw( p_wk, p_draw );
    GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
  }
  return FALSE;
}
*/
static BOOL ModeSelectInput_Key( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
	// キー入力モードへ変更
	if( p_data->key_mode == GFL_APP_END_TOUCH ){
	  if( GFL_UI_KEY_GetTrg() & GFL_PAD_BUTTON_KTST_CHG){
			PMSND_PlaySE( WIFINOTE_MOVE_SE );
			ChangeTopPageButton( p_draw, p_wk->cursor, p_wk->cursor );
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
	    p_data->key_mode = GFL_APP_END_KEY;
	  }
		return FALSE;
	}

  // 選択チェック
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
    PMSND_PlaySE(WIFINOTE_DECIDE_SE);
		BLINKPALANM_InitAnimeCount( p_draw->bpawk );
    return TRUE;
  }

  // キャンセルチェック
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
    PMSND_PlaySE(WIFINOTE_BS_SE);
		SetTopPageButtonPassive( p_draw, p_wk->cursor );
    p_wk->cursor = MODESEL_CUR_END; // 終了にする
    return TRUE;
  }

  // 移動チェック
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
    if( p_wk->cursor > 0 ){
	    PMSND_PlaySE( WIFINOTE_MOVE_SE );
			ChangeTopPageButton( p_draw, p_wk->cursor-1, p_wk->cursor );
      p_wk->cursor--;
      return FALSE;
    }
  }else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
    if( p_wk->cursor < MODESEL_CUR_MYCODE ){
	    PMSND_PlaySE( WIFINOTE_MOVE_SE );
			ChangeTopPageButton( p_draw, p_wk->cursor+1, p_wk->cursor );
      p_wk->cursor++;
      return FALSE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  モード選択画面 タッチ入力
 *
 *  @param  p_wk    モード選択画面ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画ワーク
 *
 *  @retval TRUE  何かを選択
 *  @retval FALSE 何もおきていない
 */
//-----------------------------------------------------------------------------
/*
static BOOL ModeSelectInput_Touch( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw ,BOOL* touch_f)
{
  int ret;

  static const GFL_UI_TP_HITTBL TpRect[] = {
  //  {0,191,0,255}, ty,by,lx,rx
    {TP_MODESEL_BTN_Y01,TP_MODESEL_BTN_Y01+TP_MODESEL_BTN_H,TP_MODESEL_BTN_X,TP_MODESEL_BTN_X+TP_MODESEL_BTN_W},
    {TP_MODESEL_BTN_Y02,TP_MODESEL_BTN_Y02+TP_MODESEL_BTN_H,TP_MODESEL_BTN_X,TP_MODESEL_BTN_X+TP_MODESEL_BTN_W},
    {TP_MODESEL_BTN_Y03,TP_MODESEL_BTN_Y03+TP_MODESEL_BTN_H,TP_MODESEL_BTN_X,TP_MODESEL_BTN_X+TP_MODESEL_BTN_W},
    {GFL_UI_TP_HIT_END,0,0,0},
  };
  ret = GFL_UI_TP_HitTrg(TpRect);

  if(ret == GFL_UI_TP_HIT_NONE){
    return FALSE;
  }
  *touch_f = TRUE;
  p_data->key_mode = GFL_APP_END_TOUCH;
	GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );

  // 選択チェック
  p_wk->cursor = MODESEL_CUR_FLIST+ret;
  PMSND_PlaySE(WIFINOTE_DECIDE_SE);
  ModeSelect_CursorScrnPalChange( p_wk, p_draw );
  ModeSelect_CursorDraw( p_wk, p_draw );
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
  return TRUE;
}
*/
static int ModeSelectInput_Touch( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  int ret;

  static const GFL_UI_TP_HITTBL TpRect[] = {
  //  {0,191,0,255}, ty,by,lx,rx
    {TP_MODESEL_BTN_Y01,TP_MODESEL_BTN_Y01+TP_MODESEL_BTN_H,TP_MODESEL_BTN_X,TP_MODESEL_BTN_X+TP_MODESEL_BTN_W},
    {TP_MODESEL_BTN_Y02,TP_MODESEL_BTN_Y02+TP_MODESEL_BTN_H,TP_MODESEL_BTN_X,TP_MODESEL_BTN_X+TP_MODESEL_BTN_W},
    {TP_MODESEL_BTN_Y03,TP_MODESEL_BTN_Y03+TP_MODESEL_BTN_H,TP_MODESEL_BTN_X,TP_MODESEL_BTN_X+TP_MODESEL_BTN_W},
    {TOUCHBAR_ICON_Y,TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1,TOUCHBAR_ICON_X_07,TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1},
    {GFL_UI_TP_HIT_END,0,0,0},
  };
  ret = GFL_UI_TP_HitTrg(TpRect);

  if( ret != GFL_UI_TP_HIT_NONE ){
		if( ret == MODESEL_CUR_END ){
			PMSND_PlaySE( WIFINOTE_BS_SE );
			SetTopPageButtonPassive( p_draw, p_wk->cursor );
		}else{
			PMSND_PlaySE( WIFINOTE_DECIDE_SE );
			ChangeTopPageButton( p_draw, ret, p_wk->cursor );
		}
	  p_wk->cursor = MODESEL_CUR_FLIST+ret;
		GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
	  p_data->key_mode = GFL_APP_END_TOUCH;
  }

	return ret;
}

//----------------------------------------------------------------------------
/**
 *  @brief  モード選択画面メイン
 *
 *  @param  p_wk    モード選択画面ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画ワーク
 *
 *  @retval TRUE  何かを選択
 *  @retval FALSE 何もおきていない
 */
//-----------------------------------------------------------------------------
static BOOL ModeSelectSeq_Main( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
/*
  BOOL ret = FALSE,touch_f = FALSE;

  ret = ModeSelectInput_Touch( p_wk, p_data, p_draw ,&touch_f);

  if(touch_f){
    p_data->key_mode = GFL_APP_END_TOUCH;
    return ret;
  }
  return ModeSelectInput_Key( p_wk, p_data, p_draw );
*/
	int	ret;

  ret = ModeSelectInput_Touch( p_wk, p_data, p_draw );
	if( ret != GFL_UI_TP_HIT_NONE ){
		return TRUE;
	}
  return ModeSelectInput_Key( p_wk, p_data, p_draw );
}

//----------------------------------------------------------------------------
/**
 *  @brief  カーソル表示
 *
 *  @param  p_wk  ワーク
 *  @param  p_draw  描画システム
 */
//-----------------------------------------------------------------------------
static void ModeSelect_CursorDraw( WFNOTE_MODESELECT* p_wk, WFNOTE_DRAW* p_draw )
{
  GFL_CLACTPOS pos;
  pos.x = 128;
  pos.y = 40*p_wk->cursor+48;
  GFL_CLACT_WK_SetPos( p_draw->p_cur, &pos , CLSYS_DEFREND_MAIN );
}

//----------------------------------------------------------------------------
/**
 *  @brief  カーソルの位置に合わせてスクリーンのパレット
 *
 *  @param  p_wk  ワーク
 *  @param  p_draw  描画システム
 */
//-----------------------------------------------------------------------------
static void ModeSelect_CursorScrnPalChange( WFNOTE_MODESELECT* p_wk, WFNOTE_DRAW* p_draw )
{
  int i;
  u8 pal;

  for( i=0; i<MODESEL_CUR_NUM; i++ ){
    if( p_wk->cursor == i ){
      pal = MODESEL_CUR_PAL_ON;
    }else{
      pal = MODESEL_CUR_PAL_OFF;
    }
    GFL_BG_ChangeScreenPalette( DFRM_SCROLL,
      2, 4+5*i,
      MODESEL_CUR_CSIZX, MODESEL_CUR_CSIZY, pal );
  }
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
}

//----------------------------------------------------------------------------
/**
 *  @brief  カーソル位置に対応した終了データを設定する
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 */
//-----------------------------------------------------------------------------
static void ModeSelect_EndDataSet( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data )
{
  static const u8 reqstatus[ MODESEL_CUR_NUM ] = {
    STATUS_FRIENDLIST,
    STATUS_CODEIN,
    STATUS_MYCODE,
    STATUS_END
  };
  Data_SetReqStatus( p_data, reqstatus[p_wk->cursor], 0 );
}

//----------------------------------------------------------------------------
/**
 *  @brief  表示物を全部消す
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画ワーク
 */
//-----------------------------------------------------------------------------
static void ModeSelect_DrawOff( WFNOTE_MODESELECT* p_wk, WFNOTE_DRAW* p_draw )
{
  // タイトル
  Draw_BmpTitleOff( p_draw );

  // SCROLL面
  GFL_BG_FillScreen( DFRM_SCROLL, 0, 0, 0, 32, 24 , GFL_BG_SCRWRT_PALNL );
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );

  // SCRMSG面
  GFL_BG_FillScreen( DFRM_SCRMSG, 0, 0, 0, 32, 24 , GFL_BG_SCRWRT_PALNL );
  GFL_BG_LoadScreenV_Req( DFRM_SCRMSG );

  // msg
  GFL_BMPWIN_ClearTransWindow_VBlank( p_wk->msg );
//  GF_BGL_BmpWinOffVReq(&p_wk->msg);

  // カーソルアクター表示設定
  Draw_CursorActorSet( p_draw, FALSE , ACTANM_NULL );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージプリント
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画ワーク
 *  @param  msgidx    メッセージIDX
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void ModeSelect_TalkMsgPrint( WFNOTE_MODESELECT* p_wk, WFNOTE_DRAW* p_draw, u32 msgidx, HEAPID heapID )
{
  STRBUF* p_tmp;

  // タスク動作停止
  if( p_draw->printHandleMsg != NULL ){
    PRINTSYS_PrintStreamDelete( p_draw->printHandleMsg );
    p_draw->printHandleMsg = NULL;
  }

  p_tmp = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );

  // クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->talk), 15 );

  GFL_MSG_GetString( p_draw->p_msgman, msgidx, p_tmp );
  WORDSET_ExpandStr( p_draw->p_wordset, p_wk->p_str, p_tmp );
  p_draw->printHandleMsg = PRINTSYS_PrintStream(p_wk->talk, 0, 0,
      p_wk->p_str, p_draw->fontHandle , MSGSPEED_GetWait(),
      p_draw->msgTcblSys , 10 , p_draw->heapID , PRINTSYS_LSB_GetB(WFNOTE_COL_BLACK) );

  BmpWinFrame_Write( p_wk->talk, WINDOW_TRANS_OFF,
      BMPL_WIN_CGX_TALK, BGPLT_M_TALKWIN );

  GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->talk );

  GFL_STR_DeleteBuffer( p_tmp );

	// 画面をパッシブへ
	SetDispActive( p_draw, TRUE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージの終了待ち
 *
 *  @param  cp_wk ワーク
 *
 *  @retval TRUE  終了
 *  @retval FALSE まだ
 */
//-----------------------------------------------------------------------------
static BOOL ModeSelect_TalkMsgEndCheck( const WFNOTE_MODESELECT* cp_wk, WFNOTE_DATA* p_data , WFNOTE_DRAW *p_draw )
{
  if( PRINTSYS_PrintStreamGetState( p_draw->printHandleMsg ) == PRINTSTREAM_STATE_DONE ){
//    p_data->key_mode = MsgPrintGetKeyTouchStatus();
    p_data->key_mode = GFL_APP_END_KEY;
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  会話ウインドウを消す
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void ModeSelect_TalkMsgOff( WFNOTE_MODESELECT* p_wk, WFNOTE_DRAW* p_draw )
{
	SetDispActive( p_draw, FALSE );
  BmpWinFrame_Clear( p_wk->talk, WINDOW_TRANS_ON );
  GFL_BMPWIN_ClearTransWindow_VBlank( p_wk->talk );
//  GF_BGL_BmpWinOffVReq(&p_wk->talk);
}

//----------------------------------------------------------------------------
/**
 *  @brief  カーソル位置のステータスに遷移しても大丈夫かチェックする
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *
 *  @retval TRUE  大丈夫
 *  @retval FALSE だめ
 */
//-----------------------------------------------------------------------------
static BOOL ModeSelect_StatusChengeCheck( WFNOTE_MODESELECT* p_wk, WFNOTE_DATA* p_data )
{
  int  i;
  WIFI_LIST* p_list;

  if( p_wk->cursor == MODESEL_CUR_CODEIN ){
    // あきがあるかチェック
    p_list = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );
    for(i=0;i<WIFILIST_FRIEND_MAX;i++){
      if( !WifiList_IsFriendData( p_list, i ) ){  //
        return TRUE;
      }
    }
  }else{

    // 友達登録以外は無条件でOK
    return TRUE;
  }

  return FALSE;
}



//-------------------------------------
/// 友達データ表示関数
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  友達データ表示ワーク  初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FList_Init( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  FList_DrawInit( p_wk, p_data, p_draw,  heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達データ表示ワーク  メイン
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 *
 *  @retval WFNOTE_STRET_CONTINUE,  // つづく
 *  @retval WFNOTE_STRET_FINISH,  // 作業は終わった
 */
//-----------------------------------------------------------------------------
static WFNOTE_STRET FList_Main( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  u32 result;
  WIFI_LIST* p_wifilist;
  u32 listidx;

  switch( p_data->subseq ){
  case SEQ_FLIST_INIT_PAGEINIT: // 表示するページを初期化するとき
    p_wk->page = 0;
    p_wk->lastpage = 0;
    p_wk->pos = 0;
    p_wk->lastpos = 0;
  case SEQ_FLIST_INIT:  // 以前の状態で表示
    FList_Init( p_wk, p_data, p_draw, HEAPID_WIFINOTE );
    // リストデータ作成
    Data_FrIdxMake( p_data );

    // 描画初期化
    FList_DrawOn( p_wk, p_data, p_draw, heapID );

    p_data->subseq  = SEQ_FLIST_MAIN;
    break;

    // リスト動作
  case SEQ_FLIST_MAIN:
		BLINKPALANM_Main( p_draw->bpawk );
    result = FListSeq_Main( p_wk, p_data,  p_draw );
    switch( result ){
    case RCODE_FLIST_SEQMAIN_NONE:
      break;

    case RCODE_FLIST_SEQMAIN_PCHG_LEFT:// ページ変更
      FList_ScrSeqChange( p_wk, p_data, WF_COMMON_LEFT );
      break;

    case RCODE_FLIST_SEQMAIN_PCHG_RIGHT:// ページ変更
      FList_ScrSeqChange( p_wk, p_data, WF_COMMON_RIGHT );
      break;

    case RCODE_FLIST_SEQMAIN_CANCEL:  // 戻る
      p_data->subseq = SEQ_FLIST_END;
      break;

    case RCODE_FLIST_SEQMAIN_SELECTPL:  // 人を選択した
//      p_wk->wait = 8;
      p_data->subseq = SEQ_FLIST_MENUINIT;
      break;
    }
    break;

    // スクロール制御
  case SEQ_FLIST_SCRLLINIT:
    FListSeq_ScrollInit( p_wk, p_data, p_draw, heapID );
    p_data->subseq = SEQ_FLIST_SCRLL;
    break;

  case SEQ_FLIST_SCRLL:

    result = FListSeq_ScrollMain( p_wk, p_data, p_draw, heapID );
    if( result == TRUE ){
      p_data->subseq = SEQ_FLIST_MAIN;
    }
    break;

  case SEQ_FLIST_MENUINIT:  //　メニュー選択
		if( MainListPageButtonDecedeAnime( p_wk ) == TRUE ){
			break;
		}
/*
    if(p_wk->wait-- > 0){ //ちょっとウェイト
      break;
    }
    p_wk->wait = 0;
*/
    // 選択したリストインデックスをデータ構造体に設定
    Data_SELECT_ListIdxSet( p_data,
        FList_FRIdxGet( p_wk ) );

    FListSeq_MenuInit( p_wk, p_data, p_draw, heapID );
    p_data->subseq = SEQ_FLIST_MENUWAIT;
    break;

  case SEQ_FLIST_MENUWAIT:  // メニュー選択待ち
    p_data->subseq = FListSeq_MenuWait( p_wk, p_data, p_draw, heapID );
    break;

  case SEQ_FLIST_INFOINIT:    // 詳しく見るへ
    p_wk->wait = FLIST_INFO_WAIT;
    p_data->subseq = SEQ_FLIST_INFO;
    break;

  case SEQ_FLIST_INFO:    // 詳しく見るへ
    // 画面切り替えがきれいに見えるようにちょっとwait
    p_wk->wait --;
    if( p_wk->wait <= 0 ){
      FList_DrawOff( p_wk, p_data, p_draw );
      Data_SetReqStatus( p_data, STATUS_FRIENDINFO, 0 );

      FList_Exit( p_wk, p_data, p_draw );
      return WFNOTE_STRET_FINISH;
    }
    break;

  case SEQ_FLIST_INFORET: // 詳しく見るから戻ってきたとき
    // ページを設定
    p_wk->page = p_data->select_listidx / FLIST_PAGE_FRIEND_NUM;
    p_wk->pos = p_data->select_listidx % FLIST_PAGE_FRIEND_NUM;
    p_data->subseq = SEQ_FLIST_INIT;  // 初期化へ
    break;

  case SEQ_FLIST_NAMECHG_INIT:  // 名前変更
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_HOLEOUT, WIPE_TYPE_HOLEOUT, WIPE_FADE_BLACK,
        WIPE_DEF_DIV, WIPE_DEF_SYNC, heapID);

    // 文字列初期化データを設定
    listidx = FList_FRIdxGet( p_wk );
    p_wifilist = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );
    Data_NAMEIN_INIT_Set( p_data, WifiList_GetFriendNamePtr( p_wifilist, p_data->idx.fridx[listidx] ) );
    p_data->subseq = SEQ_FLIST_NAMECHG_WIPE;
    break;

  case SEQ_FLIST_NAMECHG_WIPE:  // 名前変更
    if( WIPE_SYS_EndCheck() ){
      // 名前入力画面へ
      FList_DrawOff( p_wk, p_data, p_draw );
      Data_SetReqStatus( p_data, STATUS_CODEIN, SEQ_CODEIN_NAMEINONLY );

      FList_Exit( p_wk, p_data, p_draw );
      return WFNOTE_STRET_FINISH;
    }
    break;

  case SEQ_FLIST_NAMECHG_WAIT:  // 名前変更
    // 名前を変更
    listidx = FList_FRIdxGet( p_wk );
    p_wifilist = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );
    WifiList_SetFriendName( p_wifilist, p_data->idx.fridx[listidx], p_data->codein.p_name );
  case SEQ_FLIST_NAMECHG_WAITWIPE:  // 名前変更 何もしないで復帰
    // 描画初期化
    FList_Init( p_wk, p_data, p_draw, HEAPID_WIFINOTE );
    FList_DrawOn( p_wk, p_data, p_draw, heapID );
    WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_HOLEIN, WIPE_TYPE_HOLEIN, WIPE_FADE_BLACK,
        WIPE_DEF_DIV, WIPE_DEF_SYNC, heapID);
    p_data->subseq = SEQ_FLIST_NAMECHG_WAITWIPEWAIT;
    break;

  case SEQ_FLIST_NAMECHG_WAITWIPEWAIT:  // 名前変更
    if( WIPE_SYS_EndCheck() ){
      p_data->subseq = SEQ_FLIST_MAIN;
    }
    break;

  case SEQ_FLIST_DELETE_INIT: // 破棄
    FListSeq_DeleteInit( p_wk, p_data, p_draw, heapID );
    p_data->subseq = SEQ_FLIST_DELETE_YESNODRAW;
    break;

  case SEQ_FLIST_DELETE_YESNODRAW:  // 破棄 YESNOウィンドウ表示
    result = FListSeq_DeleteYesNoDraw( p_wk, p_data, p_draw, heapID );
    if( result == TRUE ){
      p_data->subseq = SEQ_FLIST_DELETE_WAIT;
    }
    break;

  case SEQ_FLIST_DELETE_WAIT: // 破棄YESNOをまってから処理
		result = Draw_YesNoWinMain( p_wk->listWork );
    switch(result){
    case 0:
			p_data->key_mode = GFL_UI_CheckTouchOrKey();
      FList_TalkMsgOff( p_wk, p_draw );
      p_data->subseq =  SEQ_FLIST_DELETE_ANMSTART;
      break;
    case 1:
			p_data->key_mode = GFL_UI_CheckTouchOrKey();
      FList_TalkMsgOff( p_wk, p_draw );
      p_data->subseq =  SEQ_FLIST_MAIN;
      break;
    default:
      break;
    }
    break;

  case SEQ_FLIST_DELETE_ANMSTART: // 破棄YESNOをまってから処理
    FList_DeleteAnmStart( p_wk , p_draw );
    p_data->subseq = SEQ_FLIST_DELETE_ANMWAIT;
    break;

  case SEQ_FLIST_DELETE_ANMWAIT:  // 破棄YESNOをまってから処理
    if( FList_DeleteAnmMain( p_wk, p_draw ) == TRUE ){
      // 破棄処理
      p_wifilist = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );
      listidx = FList_FRIdxGet( p_wk );
      WifiList_ResetData( p_wifilist, p_data->idx.fridx[ listidx ] );
      // 友達インデックスを作り直す
      // 友達の表示を書き直す
      // リストデータ作成
      Data_FrIdxMake( p_data );

      // 描画初期化
      FList_DrawPage( p_wk, p_data, p_draw, p_wk->page, p_wk->pos, WFNOTE_DRAWAREA_MAIN, heapID );

      // メッセージを消して矢印を出す
//      Draw_YazirushiSetDrawFlag( p_draw, TRUE );
      p_data->subseq =  SEQ_FLIST_DELETE_END;
    }
    break;

  case SEQ_FLIST_DELETE_END:
    FList_DeleteAnmOff( p_wk, p_draw );
    p_data->subseq =  SEQ_FLIST_MAIN;
    break;

  case SEQ_FLIST_CODE_INIT: // コード切り替え
    result = FListSeq_CodeInit( p_wk, p_data, p_draw, heapID );
    if( result == TRUE ){
      p_data->subseq = SEQ_FLIST_CODE_WAIT;
    }else{
//      Draw_YazirushiSetDrawFlag( p_draw, TRUE );  // 矢印表示
      p_data->subseq = SEQ_FLIST_MAIN;
    }
    break;

  case SEQ_FLIST_CODE_WAIT: // 何かキーを押してもらう
    if( FList_TalkMsgEndCheck( p_wk,p_data,p_draw ) == FALSE ){
      break;
    }

		if( CheckUITrg( p_data ) == TRUE ){
//      Draw_YazirushiSetDrawFlag( p_draw, TRUE );  // 矢印表示
      p_data->subseq = SEQ_FLIST_MAIN;
      FList_TalkMsgOff( p_wk, p_draw );
    }
    break;

  case SEQ_FLIST_END:
		if( GFL_CLACT_WK_CheckAnmActive( p_draw->p_tb ) == TRUE ){
			break;
		}
    FList_DrawOff( p_wk, p_data, p_draw );
    Data_SetReqStatus( p_data, STATUS_MODE_SELECT, SEQ_MODESEL_INIT_NOFADE );

    FList_Exit( p_wk, p_data, p_draw );
    return WFNOTE_STRET_FINISH;
  }
  return WFNOTE_STRET_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達データ表示ワーク  破棄
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void FList_Exit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  FList_DrawExit( p_wk, p_data, p_draw );
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FList_DrawInit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  int i, j;
  STRBUF* p_str;
  BMPMENULIST_HEADER list_h;

  static const u16 sc_TEXTCGX[ 2 ] = {
    BMPL_FLIST_TEXT_CGX0,
    BMPL_FLIST_TEXT_CGX1,
  };

  // ビットマップ
  for( i=0; i < 2; i++ )
  {
    Sub_BmpWinAdd( &p_wk->drawBmpWin[i],
        DFRM_SCRMSG,
        BMPL_FLIST_TEXT_OFSX+MATH_ABS(DATA_ScrnArea[i].scrn_x),
        BMPL_FLIST_TEXT_OFSY+MATH_ABS(DATA_ScrnArea[i].scrn_y),
        BMPL_FLIST_TEXT_SIZX, BMPL_FLIST_TEXT_SIZY,
        BGPLT_M_MSGFONT, sc_TEXTCGX[i] , 0 );
  }

  //  描画エリア作成
  for( i=0; i < WFNOTE_DRAWAREA_NUM; i++ )
  {
    FListDrawArea_Init( &p_wk->drawdata[ i ], p_draw,
        &DATA_ScrnArea[i], sc_TEXTCGX[i], heapID );
  }
  p_wk->drawdata[ WFNOTE_DRAWAREA_MAIN ].text = &p_wk->drawBmpWin[0];
  p_wk->drawdata[ WFNOTE_DRAWAREA_RIGHT ].text = &p_wk->drawBmpWin[1];
  p_wk->drawdata[ WFNOTE_DRAWAREA_LEFT ].text = &p_wk->drawBmpWin[1];

  // キャラクタデータ初期化
  p_wk->p_charsys = WF_2DC_SysInit( p_draw->cellUnitScroll, NULL, FLIST_2DCHAR_NUM, heapID );

  // ユニオンキャラクタを読み込む
  WF_2DC_UnionResSet( p_wk->p_charsys, CLSYS_DRAW_MAIN,
      WF_2DC_MOVETURN, heapID );

/*
  // 戻るメッセージ作成
  Sub_BmpWinAdd( &p_wk->backmsg, DFRM_MSG,
      BMPL_FLIST_BKMSG_X, BMPL_FLIST_BKMSG_Y,
      BMPL_FLIST_BKMSG_SIZX, BMPL_FLIST_BKMSG_SIZY,
      BGPLT_M_MSGFONT, BMPL_FLIST_BKMSG_CGX, 0 );

  p_str = GFL_MSG_CreateString( p_draw->p_msgman, msg_flist_back );
  PRINTSYS_PrintQueColor( p_draw->printQue , GFL_BMPWIN_GetBmp(p_wk->backmsg), 0, 0, p_str, p_draw->fontHandle , WFNOTE_TCOL_BLACK );
  GFL_STR_DeleteBuffer( p_str );
*/

  // メニューデータ作成
/*
  for( i=0; i<BMPL_FLIST_MENU_NUM; i++ ){
    p_wk->p_menulist[i] = BmpMenuWork_ListCreate( BMPL_FLIST_MENU_LISTNUM, heapID );
    for( j=0; j<BMPL_FLIST_MENU_LISTNUM; j++ ){
      BmpMenuWork_ListAddArchiveString( p_wk->p_menulist[i], p_draw->p_msgman,
          DATA_FListMenuTbl[i][j].strid, DATA_FListMenuTbl[i][j].param , heapID );
    }
	}
*/
	for( i=0; i<4; i++ ){
		p_wk->list1[i].str      = GFL_MSG_CreateString( p_draw->p_msgman, DATA_FListMenuTbl[0][i].strid );
	  p_wk->list1[i].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
		p_wk->list1[i].type     = APP_TASKMENU_WIN_TYPE_NORMAL;

		p_wk->list2[i].str      = GFL_MSG_CreateString( p_draw->p_msgman, DATA_FListMenuTbl[1][i].strid );
	  p_wk->list2[i].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
		p_wk->list2[i].type     = APP_TASKMENU_WIN_TYPE_NORMAL;
	}
  p_wk->list1[3].type = APP_TASKMENU_WIN_TYPE_RETURN;
  p_wk->list2[3].type = APP_TASKMENU_WIN_TYPE_RETURN;
	// はい・いいえ
	p_wk->ynlist[0].str      = GFL_MSG_CreateString( p_draw->p_msgman, msg_wifi_note_yes );
	p_wk->ynlist[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
	p_wk->ynlist[0].type     = APP_TASKMENU_WIN_TYPE_NORMAL;
	p_wk->ynlist[1].str      = GFL_MSG_CreateString( p_draw->p_msgman, msg_wifi_note_no );
	p_wk->ynlist[1].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
	p_wk->ynlist[1].type     = APP_TASKMENU_WIN_TYPE_NORMAL;

	p_wk->listRes = APP_TASKMENU_RES_Create(
										DFRM_MSG, BGPLT_M_SBOX, p_draw->fontHandle, p_draw->printQue, heapID );

  // 会話ウィンドウ作成
  Sub_BmpWinAdd( &p_wk->talk, DFRM_MSG,
      BMPL_FLIST_TALK_X, BMPL_FLIST_TALK_Y,
      BMPL_FLIST_TALK_SIZX, BMPL_FLIST_TALK_SIZY,
      BGPLT_M_MSGFONT, BMPL_FLIST_TALK_CGX ,15);

  p_wk->p_talkstr = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );

  // 消しゴムアクターの作成
  {
    GFL_CLWK_DATA clearadd = {
      0,0,
      ACTANM_CLEAR,
      0,0,
    };
    p_wk->p_clearact = GFL_CLACT_WK_Create(
          p_draw->cellUnit,
          p_draw->chrRes[CHR_RES_WOR],
          p_draw->palRes[PAL_RES_WOR],
          p_draw->celRes[CEL_RES_WOR],
          &clearadd, CLSYS_DEFREND_MAIN,
          heapID );
    // とりあえず非表示
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clearact, FALSE );
  }
  // ページ番号アクターの作成
  {
    GFL_CLWK_DATA clearadd = {
      56, 180,
      ACTANM_PAGE_NUM1,
      0,0,
	  };
    p_wk->p_pageact = GFL_CLACT_WK_Create(
          p_draw->cellUnit,
          p_draw->chrRes[CHR_RES_WOR],
          p_draw->palRes[PAL_RES_WOR],
          p_draw->celRes[CEL_RES_WOR],
          &clearadd, CLSYS_DEFREND_MAIN,
          heapID );
    // とりあえず非表示
//    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clearact, FALSE );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画破棄
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void FList_DrawExit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  int i;

	// ページ番号アクター
  GFL_CLACT_WK_Remove( p_wk->p_pageact );
  // 消しゴムアクター
  GFL_CLACT_WK_Remove( p_wk->p_clearact );

  // 会話ウィンドウ
  GFL_BMPWIN_Delete( p_wk->talk );
  GFL_STR_DeleteBuffer( p_wk->p_talkstr );

  // メニューデータ破棄
	APP_TASKMENU_RES_Delete( p_wk->listRes );
	GFL_STR_DeleteBuffer( p_wk->ynlist[0].str );
	GFL_STR_DeleteBuffer( p_wk->ynlist[1].str );
  for( i=0; i<4; i++ ){
//    BmpMenuWork_ListDelete( p_wk->p_menulist[i] );
		GFL_STR_DeleteBuffer( p_wk->list1[i].str );
		GFL_STR_DeleteBuffer( p_wk->list2[i].str );
  }

  // メッセージ面破棄
//  GFL_BMPWIN_Delete( p_wk->backmsg );

  // ユニオンキャラクタを読み込む
  WF_2DC_UnionResDel( p_wk->p_charsys );

  // キャラクタデータ初期化
  WF_2DC_SysExit( p_wk->p_charsys );


  //  描画エリア作成
  for( i=0; i<WFNOTE_DRAWAREA_NUM; i++ ){
    FListDrawArea_Exit( &p_wk->drawdata[ i ], p_draw );
  }

  // ビットマップ
  for( i=0; i < 2; i++ )
  {
    GFL_BMPWIN_Delete( p_wk->drawBmpWin[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画開始関数
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FList_DrawOn( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  int i;

  // タイトル描画
  Draw_BmpTitleWrite( p_draw, &p_draw->title, msg_flist_title );

/*
  // 戻る描画
  GFL_BG_WriteScreenExpand( DFRM_BACK,
      FLIST_BACK_X, FLIST_BACK_Y, FLIST_BACK_SIZX, FLIST_BACK_SIZY,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
      FLIST_SCRBACK_X, FLIST_SCRBACK_Y,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );
*/
/*
  // 戻るのメッセージ描画
  GFL_BMPWIN_TransVramCharacter( p_wk->backmsg );
  GFL_BMPWIN_MakeScreen( p_wk->backmsg );
*/

  // BG面転送
  GFL_BG_LoadScreenV_Req( DFRM_BACK );
  // 矢印描画
//  Draw_YazirushiSetDrawFlag( p_draw, TRUE );

  //  最初のページ描画
  FList_DrawPage( p_wk, p_data, p_draw, p_wk->page, p_wk->pos, WFNOTE_DRAWAREA_MAIN, heapID );

  // カーソルアクター表示設定
//  Draw_CursorActorSet( p_draw, TRUE , ACTANM_CUR_LIST );
//  Draw_CursorActorPriSet( p_draw, FLIST_CURSOR_BGPRI, FLIST_CURSOR_PRI );
}

//----------------------------------------------------------------------------
/**
 *  @brief  表示OFF
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void FList_DrawOff( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  int i;

  // タイトルOFF
  Draw_BmpTitleOff( p_draw );

  // 戻る非表示
//  GFL_BMPWIN_ClearTransWindow_VBlank( p_wk->backmsg );
//  GF_BGL_BmpWinOffVReq( &p_wk->backmsg );

  // 描画エリアの表示OFF
  for( i=0; i<WFNOTE_DRAWAREA_NUM; i++ ){
    FListDrawArea_DrawOff( &p_wk->drawdata[i],
        p_draw, &p_draw->scrn );
  }
  // カーソルアクター表示設定
  Draw_CursorActorSet( p_draw, FALSE , ACTANM_NULL );
  Draw_CursorActorPriSet( p_draw, WFNOTE_CURSOR_BGPRI, WFNOTE_CURSOR_PRI );

  // 矢印描画OFF
//  Draw_YazirushiSetDrawFlag( p_draw, FALSE );

/*
  // 背景を元に戻す
  // もどる
  GFL_BG_WriteScreenExpand( DFRM_BACK,
      FLIST_BACK_X,
      FLIST_BACK_Y,
      FLIST_BACK_SIZX, FLIST_BACK_SIZY,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
      FLIST_CL_SCRBACK_X, FLIST_CL_SCRBACK_Y,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );
  // しおりぶぶん
  GFL_BG_WriteScreenExpand( DFRM_BACK,
      FLIST_SHIORI_X, FLIST_SHIORI_Y,
      FLIST_SHIORI_SIZX, FLIST_SHIORI_SIZY,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
      FLIST_CL_SHIORISCRN_X, FLIST_CL_SHIORISCRN_Y,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );
*/
  GFL_BG_LoadScreenV_Req( DFRM_BACK );
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達リスト表示  メインキー入力取得
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *
 *  @retval RCODE_FLIST_SEQMAIN_NONE,
 *  @retval RCODE_FLIST_SEQMAIN_PCHG_,// ページ変更
 *  @retval RCODE_FLIST_SEQMAIN_CANCEL, // 戻る
 *  @retval RCODE_FLIST_SEQMAIN_SELECTPL, // 人を選択した
 *
 */
//-----------------------------------------------------------------------------
static u32 FListInputMain_Exe( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw,u8 code )
{
  u16 lastpos;
  BOOL move;
  BOOL pagechang;
  WF_COMMON_WAY way;

  if(code == FLIST_MAIN_EXE_CANCEL){
    PMSND_PlaySE( WIFINOTE_BS_SE );
		SetCancelButtonAnime( p_draw );
    return RCODE_FLIST_SEQMAIN_CANCEL;
  }else if( code == FLIST_MAIN_EXE_DECIDE){
    // キャンセルチェック
    if( p_wk->pos == FLIST_CURSORDATA_BACK ){
			PMSND_PlaySE( WIFINOTE_BS_SE );
			SetCancelButtonAnime( p_draw );
      return RCODE_FLIST_SEQMAIN_CANCEL;
    }

    // その位置に友達がいるかチェック
    if( p_data->idx.friendnum <= p_wk->pos + (p_wk->page*FLIST_PAGE_FRIEND_NUM) ){
      return RCODE_FLIST_SEQMAIN_NONE;  // いない
    }

    // 友達の情報を表示する
    PMSND_PlaySE(WIFINOTE_DECIDE_SE);
//    FListDrawArea_ActiveListWrite( &p_wk->drawdata[WFNOTE_DRAWAREA_MAIN], p_draw, p_wk->pos ,TRUE );
		p_wk->btn_anmSeq = 0;
    return RCODE_FLIST_SEQMAIN_SELECTPL;
  }else if( code == FLIST_MAIN_EXE_LEFT ){
//    Draw_YazirushiAnmChg( p_draw, ACT_YAZI_L, ACT_YAZIANM_ACTIVE );
    return RCODE_FLIST_SEQMAIN_PCHG_LEFT;
  }else if( code == FLIST_MAIN_EXE_RIGHT ){
//    Draw_YazirushiAnmChg( p_draw, ACT_YAZI_R, ACT_YAZIANM_ACTIVE );
    return RCODE_FLIST_SEQMAIN_PCHG_RIGHT;
  }
  return RCODE_FLIST_SEQMAIN_NONE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達リスト表示  メインキー入力
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *
 *  @retval RCODE_FLIST_SEQMAIN_NONE,
 *  @retval RCODE_FLIST_SEQMAIN_PCHG_,// ページ変更
 *  @retval RCODE_FLIST_SEQMAIN_CANCEL, // 戻る
 *  @retval RCODE_FLIST_SEQMAIN_SELECTPL, // 人を選択した
 *
 */
//-----------------------------------------------------------------------------
static u32 FListInputMain_Key( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  u16 lastpos;
  BOOL move,pagechg,way;

	// キー入力モードへ変更
	if( p_data->key_mode == GFL_APP_END_TOUCH ){
		if( GFL_UI_KEY_GetTrg() & GFL_PAD_BUTTON_KTST_CHG){
			PMSND_PlaySE( WIFINOTE_MOVE_SE );
			ChangeListPageButton( p_draw, p_wk->pos, p_wk->pos );
			GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
	    p_data->key_mode = GFL_APP_END_KEY;
	  }
		return RCODE_FLIST_SEQMAIN_NONE;
	}

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
//		PMSND_PlaySE( WIFINOTE_MOVE_SE );
    return FListInputMain_Exe( p_wk, p_data, p_draw,FLIST_MAIN_EXE_DECIDE);
  }

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
    return FListInputMain_Exe( p_wk, p_data, p_draw,FLIST_MAIN_EXE_CANCEL);
  }

  lastpos = p_wk->pos;  // 前の座標を保存しておく もどる　からリストにカーソルが移動するときに使用する
  move = FALSE;
  pagechg = FALSE;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
		if( p_wk->pos == 0 ){
			p_wk->pos = 3;
		}else if( p_wk->pos == 4 ){
			p_wk->pos = 7;
		}else{
			p_wk->pos--;
		}
		move = TRUE;
  }else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
		if( p_wk->pos == 3 ){
			p_wk->pos = 0;
		}else if( p_wk->pos == 7 ){
			p_wk->pos = 4;
		}else{
			p_wk->pos++;
		}
		move = TRUE;
  }else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
		if( p_wk->pos < FLIST_PAGE_FRIEND_HALF ){
			p_wk->pos += FLIST_PAGE_FRIEND_HALF;
			move = TRUE;
			pagechg = TRUE;
			way = 0;
		}else{
			p_wk->pos -= FLIST_PAGE_FRIEND_HALF;
		}
		move = TRUE;
  }else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
		if( p_wk->pos < FLIST_PAGE_FRIEND_HALF ){
			p_wk->pos += FLIST_PAGE_FRIEND_HALF;
		}else{
			p_wk->pos -= FLIST_PAGE_FRIEND_HALF;
			pagechg = TRUE;
			way = 1;
		}
		move = TRUE;
  }else if( p_data->lr_key_enable ){
    if( p_data->lr_key_enable && (GFL_UI_KEY_GetRepeat() & PAD_BUTTON_L) ) {
      pagechg = TRUE;
      way = 0;
    }else if( p_data->lr_key_enable && (GFL_UI_KEY_GetRepeat() & PAD_BUTTON_R) ) {
      pagechg = TRUE;
      way = 1;
    }
  }

  if( pagechg == TRUE ){
    // スクロール処理へ移行する
    if(way){  //右
      p_wk->lastpage = p_wk->page;
      p_wk->page = (p_wk->page + 1) % FLIST_PAGE_MAX;
      return FListInputMain_Exe( p_wk, p_data, p_draw,FLIST_MAIN_EXE_RIGHT);
    }
    p_wk->lastpage = p_wk->page;
    p_wk->page -= 1;
    if( p_wk->page < 0 ){
      p_wk->page += FLIST_PAGE_MAX;
    }
    return FListInputMain_Exe( p_wk, p_data, p_draw,FLIST_MAIN_EXE_LEFT);
  }else if( move == TRUE ){
    // カーソルが動いた
    PMSND_PlaySE( WIFINOTE_MOVE_SE );
		ChangeListPageButton( p_draw, p_wk->pos, lastpos );
//    FListDrawArea_CursorWrite( &p_wk->drawdata[WFNOTE_DRAWAREA_MAIN], p_draw, p_wk->pos );
    p_wk->lastpos = lastpos;
  }
  return RCODE_FLIST_SEQMAIN_NONE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達リスト表示  メインタッチ入力
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *
 *  @retval RCODE_FLIST_SEQMAIN_NONE,
 *  @retval RCODE_FLIST_SEQMAIN_PCHG_,// ページ変更
 *  @retval RCODE_FLIST_SEQMAIN_CANCEL, // 戻る
 *  @retval RCODE_FLIST_SEQMAIN_SELECTPL, // 人を選択した
 *
 */
//-----------------------------------------------------------------------------
static u32 FListInputMain_Touch( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw ,BOOL* touch_f )
{
  u32 ret;
  s8  dif;

  static const GFL_UI_TP_HITTBL TpRect[] = {
  //  {0,191,0,255}, ty,by,lx,rx
    {TP_FLIST_BTN_Y01,TP_FLIST_BTN_Y01+TP_FLIST_BTN_H,TP_FLIST_BTN_X01,TP_FLIST_BTN_X01+TP_FLIST_BTN_W},
    {TP_FLIST_BTN_Y02,TP_FLIST_BTN_Y02+TP_FLIST_BTN_H,TP_FLIST_BTN_X01,TP_FLIST_BTN_X01+TP_FLIST_BTN_W},
    {TP_FLIST_BTN_Y03,TP_FLIST_BTN_Y03+TP_FLIST_BTN_H,TP_FLIST_BTN_X01,TP_FLIST_BTN_X01+TP_FLIST_BTN_W},
    {TP_FLIST_BTN_Y04,TP_FLIST_BTN_Y04+TP_FLIST_BTN_H,TP_FLIST_BTN_X01,TP_FLIST_BTN_X01+TP_FLIST_BTN_W},
    {TP_FLIST_BTN_Y01,TP_FLIST_BTN_Y01+TP_FLIST_BTN_H,TP_FLIST_BTN_X02,TP_FLIST_BTN_X02+TP_FLIST_BTN_W},
    {TP_FLIST_BTN_Y02,TP_FLIST_BTN_Y02+TP_FLIST_BTN_H,TP_FLIST_BTN_X02,TP_FLIST_BTN_X02+TP_FLIST_BTN_W},
    {TP_FLIST_BTN_Y03,TP_FLIST_BTN_Y03+TP_FLIST_BTN_H,TP_FLIST_BTN_X02,TP_FLIST_BTN_X02+TP_FLIST_BTN_W},
    {TP_FLIST_BTN_Y04,TP_FLIST_BTN_Y04+TP_FLIST_BTN_H,TP_FLIST_BTN_X02,TP_FLIST_BTN_X02+TP_FLIST_BTN_W},
    {TOUCHBAR_ICON_Y,TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1,TOUCHBAR_ICON_X_07,TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1},

    {TP_FLIST_BM_Y,TP_FLIST_BM_Y+TP_FLIST_BM_H-1,TP_FLIST_BM01_X,TP_FLIST_BM01_X+TP_FLIST_BM_W-1},
    {TP_FLIST_BM_Y,TP_FLIST_BM_Y+TP_FLIST_BM_H-1,TP_FLIST_BM02_X,TP_FLIST_BM02_X+TP_FLIST_BM_W-1},
    {TP_FLIST_BM_Y,TP_FLIST_BM_Y+TP_FLIST_BM_H-1,TP_FLIST_BM03_X,TP_FLIST_BM03_X+TP_FLIST_BM_W-1},
    {TP_FLIST_BM_Y,TP_FLIST_BM_Y+TP_FLIST_BM_H-1,TP_FLIST_BM04_X,TP_FLIST_BM04_X+TP_FLIST_BM_W-1},

    {GFL_UI_TP_HIT_END,0,0,0},
  };
  ret = GFL_UI_TP_HitTrg(TpRect);

  if(ret == GFL_UI_TP_HIT_NONE){
    return RCODE_FLIST_SEQMAIN_NONE;
  }
  dif = 0;
  if(ret <= 7){
    *touch_f = TRUE;

    PMSND_PlaySE( WIFINOTE_MOVE_SE );
		SetListPageButtonPassive( p_draw, p_wk->pos );
    p_wk->pos = ret;
//    FListDrawArea_CursorWrite( &p_wk->drawdata[WFNOTE_DRAWAREA_MAIN], p_draw, p_wk->pos );
    return FListInputMain_Exe( p_wk, p_data, p_draw,FLIST_MAIN_EXE_DECIDE);
	}else if(ret == 8){
    *touch_f = TRUE;
    p_wk->pos = ret;
    return FListInputMain_Exe( p_wk, p_data, p_draw,FLIST_MAIN_EXE_DECIDE);
  }else{
    dif = (ret - 9) - p_wk->page;
  }
  if(dif == 0){
    return RCODE_FLIST_SEQMAIN_NONE;
  }

  *touch_f = TRUE;
  p_wk->lastpage = p_wk->page;
  p_wk->page += dif;
  if(dif >= 0){
    p_wk->page = p_wk->page % FLIST_PAGE_MAX;
    return FListInputMain_Exe( p_wk, p_data, p_draw,FLIST_MAIN_EXE_RIGHT);
  }else{
    if( p_wk->page < 0 ){
      p_wk->page += FLIST_PAGE_MAX;
    }
    return FListInputMain_Exe( p_wk, p_data, p_draw,FLIST_MAIN_EXE_LEFT);
  }
  return RCODE_FLIST_SEQMAIN_NONE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達リスト表示  メイン
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *
 *  @retval RCODE_FLIST_SEQMAIN_NONE,
 *  @retval RCODE_FLIST_SEQMAIN_PCHG_,// ページ変更
 *  @retval RCODE_FLIST_SEQMAIN_CANCEL, // 戻る
 *  @retval RCODE_FLIST_SEQMAIN_SELECTPL, // 人を選択した
 *
 */
//-----------------------------------------------------------------------------
static u32 FListSeq_Main( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  u32 ret;
  BOOL touch_f = FALSE;

  ret = FListInputMain_Touch( p_wk, p_data, p_draw, &touch_f);
  if(touch_f){
		GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
		p_data->key_mode = GFL_APP_END_TOUCH;
    return ret;
  }
  return FListInputMain_Key( p_wk, p_data, p_draw );
}

//----------------------------------------------------------------------------
/**
 *  @brief  スクロールシーケンス初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FListSeq_ScrollInit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  u16 draw_area;

  if( p_wk->way == WF_COMMON_LEFT ){
    draw_area = WFNOTE_DRAWAREA_LEFT;
  }else{
    draw_area = WFNOTE_DRAWAREA_RIGHT;
  }

  // スクロールする先に情報を記入
  FList_DrawPage( p_wk, p_data, p_draw, p_wk->page, p_wk->pos, draw_area, heapID );

  // カーソルを消す
	SetListPageButtonPassive( p_draw, p_wk->pos );
//  FListDrawArea_CursorWrite( &p_wk->drawdata[ WFNOTE_DRAWAREA_MAIN ], p_draw, FLIST_CURSORDATA_OFF );

  // 矢印をとめる
//  Draw_YazirushiSetAnmFlag( p_draw, FALSE );

  // スクロール音
  PMSND_PlaySE( WIFINOTE_SCRLL_SE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  スクロールシーケンス  メイン
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 *
 *  @retval TRUE  終了
 *  @retval FALSE 途中
 */
//-----------------------------------------------------------------------------
static BOOL FListSeq_ScrollMain( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  u32 draw_area;
  s32 scrll_x;
  GFL_CLACTPOS surfacePos;
  u32 idx;
  static void (*pSHIORIFUNC[ 4 ])( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DRAW* p_draw, u32 page0, u32 page1 ) = {
    FList_DrawShioriEff00,
    NULL,
    FList_DrawShioriEff01,
    NULL
  };

  // カウンタが終わったらすべてを描画して終わる
  if( p_wk->count >= FLIST_SCROLL_COUNT ){

    // スクロール座標を元に戻す
    GFL_BG_SetScrollReq( DFRM_SCROLL, GFL_BG_SCROLL_X_SET, 0 );
    GFL_BG_SetScrollReq( DFRM_SCRMSG, GFL_BG_SCROLL_X_SET, 0 );
    // OAMにも反映
    surfacePos.x = DATA_ScrollSurfaceRect.lefttop_x;
    surfacePos.y = DATA_ScrollSurfaceRect.lefttop_y;
    GFL_CLACT_USERREND_SetSurfacePos( p_draw->scrollRender,0, &surfacePos );

    // 作業用グラフィックを消す
    if( p_wk->way == WF_COMMON_LEFT ){
      draw_area = WFNOTE_DRAWAREA_LEFT;
    }else{
      draw_area = WFNOTE_DRAWAREA_RIGHT;
    }
    FListDrawArea_DrawOff( &p_wk->drawdata[ draw_area ], p_draw, &p_draw->scrn );

    // メイン面に表示
    FList_DrawPage( p_wk, p_data, p_draw, p_wk->page, p_wk->pos, WFNOTE_DRAWAREA_MAIN, heapID );

    // 矢印動作開始
//    Draw_YazirushiSetAnmFlag( p_draw, TRUE );
    return TRUE;
  }

  scrll_x = (p_wk->count * FLIST_SCROLL_SIZX) / FLIST_SCROLL_COUNT;

  if( p_wk->way == WF_COMMON_LEFT ){
    scrll_x = -scrll_x;
  }

  //  スクロール座標設定
  GFL_BG_SetScrollReq( DFRM_SCROLL, GFL_BG_SCROLL_X_SET, scrll_x );
  GFL_BG_SetScrollReq( DFRM_SCRMSG, GFL_BG_SCROLL_X_SET, scrll_x );

  // OAMにも反映
  surfacePos.x = DATA_ScrollSurfaceRect.lefttop_x;
  surfacePos.y = DATA_ScrollSurfaceRect.lefttop_y;
  surfacePos.x += scrll_x;
  GFL_CLACT_USERREND_SetSurfacePos( p_draw->scrollRender,0, &surfacePos );

  // アニメーション
  if( p_wk->count % (FLIST_SCROLL_COUNT/4) ){
    idx = p_wk->count / (FLIST_SCROLL_COUNT/4);
    if( pSHIORIFUNC[idx] ){
      pSHIORIFUNC[idx]( p_wk, p_draw, p_wk->page, p_wk->lastpage );
    }
  }

  // カウント
  p_wk->count ++;
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  メニュー表示  初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FListSeq_MenuInit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  BMPMENULIST_HEADER list_h;
  WIFI_LIST* p_wifilist;
  u32 sex;
  u32 listidx;

  // wifiリストから性別を取得
  p_wifilist = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );
  listidx = FList_FRIdxGet( p_wk );
  sex = WifiList_GetFriendInfo( p_wifilist, p_data->idx.fridx[ listidx ], WIFILIST_FRIEND_SEX );

//  FListDrawArea_ActiveListWrite( &p_wk->drawdata[WFNOTE_DRAWAREA_MAIN], p_draw, p_wk->pos ,FALSE );

/*
  {
#if USE_SEL_BOX
    SELBOX_HEADER head;

    MI_CpuClear8(&head,sizeof(SELBOX_HEADER));

    head.prm = DATA_SBoxHeadParamFList;
    if( sex == PM_NEUTRAL ){
      head.list = (const BMPLIST_DATA*)p_wk->p_menulist[ BMPL_FLIST_MENU_CODE ];
    }else{
      head.list = (const BMPLIST_DATA*)p_wk->p_menulist[ BMPL_FLIST_MENU_NML ];
    }
    head.count = SBOX_FLIST_SEL_CT;
    head.fontHandle = p_draw->fontHandle;

    p_wk->p_listwk = SelectBoxSet(p_draw->sbox,&(head),p_data->key_mode,
        SBOX_FLIST_PX,SBOX_FLIST_PY,SBOX_FLIST_W,0);
#endif
  }
*/
	{
		APP_TASKMENU_INITWORK	wk;

	  wk.heapId   = heapID;
	  wk.itemNum  = 4;
    if( sex == PM_NEUTRAL ){
		  wk.itemWork = p_wk->list2;
			p_wk->listType = 1;
		}else{
		  wk.itemWork = p_wk->list1;
			p_wk->listType = 0;
		}
	  wk.posType  = ATPT_RIGHT_DOWN;
	  wk.charPosX = 32;
	  wk.charPosY = 24;
		wk.w        = SBOX_FLIST_W;
		wk.h        = APP_TASKMENU_PLATE_HEIGHT;

		p_wk->listWork = APP_TASKMENU_OpenMenu( &wk, p_wk->listRes );
	}

  // メッセージ表示
  Draw_FriendNameSetWordset( p_draw, p_data, p_data->idx.fridx[ listidx ], heapID );
  FList_TalkMsgWrite( p_wk, p_draw, msg_wifi_note_13, heapID );
  // やじるし非表示
//  Draw_YazirushiSetDrawFlag( p_draw, FALSE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メニュー表示  選択待ち
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 *
 *  @return 次に進んでほしいシーケンス
 */
//-----------------------------------------------------------------------------
static u32 FListSeq_MenuWait( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  u32 ret;
  u32 ret_seq = SEQ_FLIST_MENUWAIT;


  // メッセージ終了待ち
  if( FList_TalkMsgEndCheck( p_wk,p_data,p_draw ) == FALSE ){
    return ret_seq;
  }
/*
#if USE_SEL_BOX
  ret = SelectBoxMain(p_wk->p_listwk);
#else
  ret = SBOX_SELECT_CANCEL;
#endif
*/
	APP_TASKMENU_UpdateMenu( p_wk->listWork );
	if( APP_TASKMENU_IsFinish( p_wk->listWork ) == TRUE ){
		APP_TASKMENU_CloseMenu( p_wk->listWork );
		ret = APP_TASKMENU_GetCursorPos( p_wk->listWork );
	}else{
	 ret = SBOX_SELECT_NULL;
	}

  switch(ret){
  case SBOX_SELECT_NULL:
    return ret_seq;
  case 3:
  case SBOX_SELECT_CANCEL:
   //   PMSND_PlaySE(WIFINOTE_MENU_BS_SE);
		p_data->key_mode = GFL_UI_CheckTouchOrKey();
    ret_seq = SEQ_FLIST_MAIN;
    break;
  default:
  //    PMSND_PlaySE(WIFINOTE_MENU_DECIDE_SE);
		p_data->key_mode = GFL_UI_CheckTouchOrKey();
		ret_seq = DATA_FListMenuTbl[p_wk->listType][ret].param;
    break;
  }
/*
#if USE_SEL_BOX
  p_data->key_mode = SelectBoxGetKTStatus(p_wk->p_listwk);
  SelectBoxExit(p_wk->p_listwk);
#endif
*/

  // MAINにもどるので矢印を出す
  if( ret_seq == SEQ_FLIST_MAIN ){
//    Draw_YazirushiSetDrawFlag( p_draw, TRUE );
    FList_TalkMsgOff( p_wk, p_draw );
  }
  else if( ret_seq == SEQ_FLIST_INFOINIT ||
       ret_seq == SEQ_FLIST_NAMECHG_INIT )
  {
    //画面遷移なので完全消去
    FList_TalkMsgOff( p_wk, p_draw );
  }
  else
  {
    FList_TalkMsgClear( p_wk );
  }
  return ret_seq;
}

//----------------------------------------------------------------------------
/**
 *  @brief  コード表示  初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 *
 *  @retval TRUE  表示できた
 *  @retval FALSE 表示できない
 */
//-----------------------------------------------------------------------------
static BOOL FListSeq_CodeInit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  WIFI_LIST* p_wifilist;
  DWCFriendData* p_frienddata;
  u64 code;
  u32 listidx;

  p_wifilist = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );
  listidx = FList_FRIdxGet( p_wk );
  p_frienddata = WifiList_GetDwcDataPtr( p_wifilist,
      p_data->idx.fridx[ listidx ] );
  code = DWC_GetFriendKey( p_frienddata );
  if(code!=0){
    // コード表示
//    PMSND_PlaySE(WIFINOTE_DECIDE_SE);
    Draw_FriendCodeSetWordset( p_draw, code );
    Draw_FriendNameSetWordset( p_draw, p_data, p_data->idx.fridx[ listidx ], heapID );
    FList_TalkMsgWrite( p_wk, p_draw, msg_wifi_note_32, heapID );
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  破棄処理  初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FListSeq_DeleteInit( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  u32 listidx;

  listidx = FList_FRIdxGet( p_wk );
  Draw_FriendNameSetWordset( p_draw, p_data, p_data->idx.fridx[ listidx ], heapID );
  FList_TalkMsgWrite( p_wk, p_draw, msg_wifi_note_29, heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  YesNoウィンドウの表示
 *
 *  @param  p_wk  ワーク
 *  @param  p_data  データ
 *  @param  p_draw  描画システム
 *  @param  heapID  ヒープID
 *
 *  @retval TRUE  終了
 *  @retval TRUE  途中
 */
//-----------------------------------------------------------------------------
static BOOL FListSeq_DeleteYesNoDraw( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  if( FList_TalkMsgEndCheck( p_wk, p_data , p_draw ) == FALSE ){
    return FALSE;
  }

  p_wk->listWork = Draw_YesNoWinSet( p_wk->ynlist, &p_wk->listRes, FALSE, heapID );
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  破棄処理　YesNo選択後破棄処理
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 *
 *  @retval YesNo選択の戻り値
 */
//-----------------------------------------------------------------------------
static u32 FListSeq_DeleteWait( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  return  TRUE;
}


//----------------------------------------------------------------------------
/**
 *  @brief  ページ描画関数
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  page    ページ
 *  @param  pos     カーソル位置
 *  @param  draw_area 描画エリア
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FList_DrawPage( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, u32 page, u32 pos, u32 draw_area, HEAPID heapID )
{
  // メイン面への描画かチェック
  if( WFNOTE_DRAWAREA_MAIN == draw_area ){
    // ページとカーソル変更
    p_wk->page = page;
    p_wk->pos = pos;

    // ページに対応したしおりを表示
//    FList_DrawShiori( p_wk, p_draw, page );
		ChangeListPageNumAnime( p_wk, page );
  }

  // 背景を描画しなおす
  FListDrawArea_BackWrite( &p_wk->drawdata[draw_area], p_draw, &p_draw->scrn );

  // ページデータ描画
  FListDrawArea_WritePlayer( &p_wk->drawdata[draw_area], p_data,
      p_draw, p_wk->p_charsys, &p_draw->scrn, &p_data->idx, page, heapID );

  // カーソル描画
	ResetListPageButton( p_wk );
//  FListDrawArea_CursorWrite( &p_wk->drawdata[draw_area], p_draw, pos );
}

//----------------------------------------------------------------------------
/**
 *  @brief  しおりの表示
 *
 *  @param  p_wk  ワーク
 *  @param  p_draw  描画システム
 *  @param  page  ページ数
 */
//-----------------------------------------------------------------------------
/*
static void FList_DrawShiori( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DRAW* p_draw, u32 page )
{
  u32 x;

  // 大本をまず描画
  GFL_BG_WriteScreenExpand( DFRM_BACK,
      FLIST_SHIORI_X, FLIST_SHIORI_Y,
      FLIST_SHIORI_SIZX, FLIST_SHIORI_SIZY,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
      FLIST_SHIORISCRN_X, FLIST_SHIORISCRN_Y,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );

  // 今のページのしおりを描画
  x = (FLIST_SHIORI_ONEX*page) + FLIST_SHIORI_OFSX;
  GFL_BG_WriteScreenExpand( DFRM_BACK,
      x+FLIST_SHIORI_X, FLIST_SHIORI_Y + FLIST_SHIORI_OFSY,
      FLIST_SHIORIPOS_SIZ, FLIST_SHIORIPOS_SIZ,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
      x+FLIST_SHIORISCRN_POS_X, FLIST_SHIORISCRN_POS_Y,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );

  // BG面転送
  GFL_BG_LoadScreenV_Req( DFRM_BACK );
}
*/

//----------------------------------------------------------------------------
/**
 *  @brief  しおりアニメ　シーン０
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    データ
 *  @param  page0   ページ１
 *  @param  page1   ページ２
 */
//-----------------------------------------------------------------------------
static void FList_DrawShioriEff00( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DRAW* p_draw, u32 page0, u32 page1 )
{
/*
  u32 x;

  x = (FLIST_SHIORI_ONEX*page1) + FLIST_SHIORI_OFSX;
  GFL_BG_WriteScreenExpand( DFRM_BACK,
      x+FLIST_SHIORI_X, FLIST_SHIORI_Y + FLIST_SHIORI_OFSY,
      FLIST_SHIORIPOS_SIZ, FLIST_SHIORIPOS_SIZ,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
      x+FLIST_EFF_SHIORISCRN_POS_X, FLIST_EFF_SHIORISCRN_POS_Y,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );

  // BG面転送
  GFL_BG_LoadScreenV_Req( DFRM_BACK );
*/
}

//----------------------------------------------------------------------------
/**
 *  @brief  しおりアニメ　シーン１
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    データ
 *  @param  page0   ページ１
 *  @param  page1   ページ２
 */
//-----------------------------------------------------------------------------
static void FList_DrawShioriEff01( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DRAW* p_draw, u32 page0, u32 page1 )
{
/*
  u32 x;

  // 大本をまず描画
  GFL_BG_WriteScreenExpand( DFRM_BACK,
      FLIST_SHIORI_X, FLIST_SHIORI_Y,
      FLIST_SHIORI_SIZX, FLIST_SHIORI_SIZY,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
      FLIST_SHIORISCRN_X, FLIST_SHIORISCRN_Y,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );

  // 今のページのしおりを描画
  x = (FLIST_SHIORI_ONEX*page0) + FLIST_SHIORI_OFSX;
  GFL_BG_WriteScreenExpand( DFRM_BACK,
      x+FLIST_SHIORI_X, FLIST_SHIORI_Y + FLIST_SHIORI_OFSY,
      FLIST_SHIORIPOS_SIZ, FLIST_SHIORIPOS_SIZ,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
      x, FLIST_SHIORISCRN_POS_Y,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );

  // BG面転送
  GFL_BG_LoadScreenV_Req( DFRM_BACK );
*/
}

//----------------------------------------------------------------------------
/**
 *  @brief  スクロールシーケンスに変更する
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  way     方向
 */
//-----------------------------------------------------------------------------
static void FList_ScrSeqChange( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DATA* p_data, WF_COMMON_WAY way )
{
  p_data->subseq = SEQ_FLIST_SCRLLINIT;
  p_wk->way = way;
  p_wk->count = 0;
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ表示
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 *  @param  msgid   メッセージID
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FList_TalkMsgWrite( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DRAW* p_draw, u32 msgid, HEAPID heapID )
{
  STRBUF* p_tmp;

  if( p_draw->printHandleMsg != NULL ){
    PRINTSYS_PrintStreamDelete( p_draw->printHandleMsg );
    p_draw->printHandleMsg = NULL;
  }
  // クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->talk), 15 );

  p_tmp = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );

  GFL_MSG_GetString( p_draw->p_msgman, msgid, p_tmp );
  WORDSET_ExpandStr( p_draw->p_wordset, p_wk->p_talkstr, p_tmp );

  p_draw->printHandleMsg = PRINTSYS_PrintStream(p_wk->talk, 0, 0,
      p_wk->p_talkstr, p_draw->fontHandle , MSGSPEED_GetWait(),
      p_draw->msgTcblSys , 10 , p_draw->heapID , PRINTSYS_LSB_GetB(WFNOTE_COL_BLACK) );

  BmpWinFrame_Write( p_wk->talk, WINDOW_TRANS_OFF,
      BMPL_WIN_CGX_TALK, BGPLT_M_TALKWIN );

  GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->talk );

  GFL_STR_DeleteBuffer( p_tmp );

	SetListDispActive( p_wk, p_draw, TRUE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ表示終了待ち
 *
 *  @param  cp_wk ワーク
 *
 *  @retval TRUE  終了
 *  @retval FALSE 途中
 */
//-----------------------------------------------------------------------------
static BOOL FList_TalkMsgEndCheck( const WFNOTE_FRIENDLIST* cp_wk, WFNOTE_DATA* p_data ,WFNOTE_DRAW* p_draw)
{
  if( PRINTSYS_PrintStreamGetState( p_draw->printHandleMsg ) == PRINTSTREAM_STATE_DONE ){
//    p_data->key_mode = MsgPrintGetKeyTouchStatus();
    p_data->key_mode = GFL_APP_END_KEY;
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージを非表示にする
 *
 *  @param  p_wk    ワーク
 */
//-----------------------------------------------------------------------------
static void FList_TalkMsgOff( WFNOTE_FRIENDLIST* p_wk, WFNOTE_DRAW * p_draw )
{
  BmpWinFrame_Clear( p_wk->talk, WINDOW_TRANS_ON_V );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->talk ), 0 );
  GFL_BMPWIN_TransVramCharacter( p_wk->talk );

	PutListTitle( p_draw );
	ResetListPageButton( p_wk );

	SetListDispActive( p_wk, p_draw, FALSE );

  // 戻るの文字が消えるので表示
//  GFL_BMPWIN_MakeScreen( p_wk->backmsg );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージを消去
 *
 *  @param  p_wk    ワーク
 */
//-----------------------------------------------------------------------------
static void FList_TalkMsgClear( WFNOTE_FRIENDLIST* p_wk )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->talk ), WFNOTE_FONT_COL_WHITE );
  GFL_BMPWIN_TransVramCharacter( p_wk->talk );
}

//----------------------------------------------------------------------------
/**
 *  @brief  現在選択中の友達リストインデックスを取得
 *
 *  @param  cp_wk   ワーク
 *
 *  @return 友達リストインデックス
 */
//-----------------------------------------------------------------------------
static u32 FList_FRIdxGet( const WFNOTE_FRIENDLIST* cp_wk )
{
  u32 num;
  num = cp_wk->page * FLIST_PAGE_FRIEND_NUM;
  num += cp_wk->pos;
  return num;
}

//----------------------------------------------------------------------------
/**
 *  @brief  メニューの１行移動ごとのコールバック
 *
 *  @param  p_wk    ワーク
 *  @param  param   パラメータ
 *  @param  mode    モード
 */
//-----------------------------------------------------------------------------
static void FList_CB_MenuList( BMPMENULIST_WORK * p_wk, u32 param, u8 mode )
{
  if( mode == 0 ){
    PMSND_PlaySE(WIFINOTE_MENU_MOVE_SE);
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  リスト破棄アニメ
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FList_DeleteAnmStart( WFNOTE_FRIENDLIST* p_wk ,WFNOTE_DRAW* p_draw )
{
  GFL_CLACTPOS cellPos;
  u32 pos;


  // 表示ON
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clearact, TRUE );
  Draw_CursorActorSet( p_draw, FALSE, ACTANM_NULL );

  // アニメ初期
  GFL_CLACT_WK_SetAnmSeq( p_wk->p_clearact, ACTANM_CLEAR );
  GFL_CLACT_WK_ResetAnm( p_wk->p_clearact );
  p_wk->last_frame = 0;

  // オートアニメ設定
  GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clearact, TRUE );

  pos = p_wk->pos;

  // ポジションにあわせて座標を設定
  cellPos.x = ((MATH_ABS(DATA_ScrnArea[ WFNOTE_DRAWAREA_MAIN ].scrn_x)+DATA_FListCursorData[pos].scrn_data.scrn_x) * 8);
  cellPos.y = ((MATH_ABS(DATA_ScrnArea[ WFNOTE_DRAWAREA_MAIN ].scrn_y)+DATA_FListCursorData[pos].scrn_data.scrn_y) * 8) + FLIST_CLEARACT_Y;
  GFL_CLACT_WK_SetPos( p_wk->p_clearact, &cellPos , CLSYS_DEFREND_MAIN );

  // ウィンドウマスク設定ON
  G2_SetWndOBJInsidePlane( GX_WND_PLANEMASK_BG0|GX_WND_PLANEMASK_BG1|GX_WND_PLANEMASK_BG2, FALSE );
  G2_SetWndOutsidePlane( GX_WND_PLANEMASK_BG0|GX_WND_PLANEMASK_BG1|GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_BG3|GX_WND_PLANEMASK_OBJ, TRUE );
  GX_SetVisibleWnd( GX_WNDMASK_OW );//*/

  // 消しゴム音
  PMSND_PlaySE( WIFINOTE_CLEAN_SE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  リスト破棄アニメ  メイン
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL FList_DeleteAnmMain( WFNOTE_FRIENDLIST* p_wk ,WFNOTE_DRAW* p_draw)
{
  u32 frame;

  if( GFL_CLACT_WK_CheckAnmActive( p_wk->p_clearact ) == FALSE ){
    return TRUE;
  }

  // アニメが切り替わったら消しゴム音
  frame = GFL_CLACT_WK_GetAnmFrame( p_wk->p_clearact );
  if( (frame - p_wk->last_frame) >= 2 ){
    p_wk->last_frame = frame;
    PMSND_PlaySE( WIFINOTE_CLEAN_SE );
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  アニメ表示状態を消す
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FList_DeleteAnmOff( WFNOTE_FRIENDLIST* p_wk ,WFNOTE_DRAW* p_draw )
{
  GX_SetVisibleWnd( GX_WNDMASK_NONE );
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clearact, FALSE );
  GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clearact, FALSE );
//  Draw_CursorActorSet( p_draw, TRUE, ACTANM_NULL );
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画エリアワーク  初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 *  @param  cp_area   スクリーンエリア情報
 *  @param  text_cgx  テキスト面のｃｇｘ
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FListDrawArea_Init( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNAREA* cp_area, u32 text_cgx, HEAPID heapID )
{
  // スクリーンデータ保存
  p_wk->scrn_area = *cp_area;
/*
  // ビットマップ
  Sub_BmpWinAdd( &p_wk->text,
      DFRM_SCRMSG,
      BMPL_FLIST_TEXT_OFSX+MATH_ABS(cp_area->scrn_x),
      BMPL_FLIST_TEXT_OFSY+MATH_ABS(cp_area->scrn_y),
      BMPL_FLIST_TEXT_SIZX, BMPL_FLIST_TEXT_SIZY,
      BGPLT_M_MSGFONT, text_cgx , 0 );
*/
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画エリアワーク  破棄
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void FListDrawArea_Exit( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw )
{
//  GFL_BMPWIN_Delete( p_wk->text );
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画エリアに背景描画
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 *  @param  cp_scrn   スクリーンデータ
 */
//-----------------------------------------------------------------------------
static void FListDrawArea_BackWrite( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNDATA* cp_scrn )
{
  // 背景描画
  GFL_BG_WriteScreen( DFRM_SCROLL,
      cp_scrn->p_scrn[WFNOTE_SCRNDATA_BACK]->rawData,
      MATH_ABS(p_wk->scrn_area.scrn_x), MATH_ABS(p_wk->scrn_area.scrn_y),
      cp_scrn->p_scrn[WFNOTE_SCRNDATA_BACK]->screenWidth/8,
      cp_scrn->p_scrn[WFNOTE_SCRNDATA_BACK]->screenHeight/8 );

  // BG面転送
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );

}

//----------------------------------------------------------------------------
/**
 *  @brief  カーソル描画
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 *  @param  pos     カーソル位置
 */
//-----------------------------------------------------------------------------
/*
static void FListDrawArea_CursorWrite( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, u32 pos )
{
  int i;
  int pal;
  const WFNOTE_FLIST_CURSORDATA* dat;
  GFL_CLACTPOS cellPos;

  if(pos == FLIST_CURSORDATA_OFF){
    Draw_CursorActorSet( p_draw,FALSE, ACTANM_NULL );
    return;
  }
  dat = &DATA_FListCursorData[pos];
  cellPos.x = (dat->scrn_data.scrn_x+7)*8;
  cellPos.y = (dat->scrn_data.scrn_y+2)*8;
  GFL_CLACT_WK_SetPos(p_draw->p_cur,&cellPos,CLSYS_DEFREND_MAIN);
  Draw_CursorActorSet( p_draw,TRUE, ACTANM_NULL );
}
*/

//----------------------------------------------------------------------------
/**
 *  @brief  リストアクティブ状態描画
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 *  @param  pos     カーソル位置
 */
//-----------------------------------------------------------------------------
static void FListDrawArea_ActiveListWrite( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, u32 pos ,BOOL active_f)
{
  int i;
  int pal;
  const WFNOTE_FLIST_CURSORDATA* dat;

  for( i=0; i<FLIST_CURSORDATA_NUM; i++ ){
    dat = &DATA_FListCursorData[i];

    if( active_f == TRUE && pos == i ){
      pal = dat->on_pal;
    }else{
      pal = dat->off_pal;
    }

    // パレットカラーを合わせる
    GFL_BG_ChangeScreenPalette(
        dat->bg_frame,
        MATH_ABS(p_wk->scrn_area.scrn_x) + dat->scrn_data.scrn_x,
        MATH_ABS(p_wk->scrn_area.scrn_y) + dat->scrn_data.scrn_y,
        dat->scrn_data.scrn_sizx,
        dat->scrn_data.scrn_sizy, pal );
  }

  // BG面転送
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
  GFL_BG_LoadScreenV_Req( DFRM_BACK );
}

//----------------------------------------------------------------------------
/**
 *  @brief  プレイヤー情報表示
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  p_charsys キャラクタシステム
 *  @param  cp_scrn   スクリーンシステム
 *  @param  cp_idx    インデックスデータ
 *  @param  page    ページ
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FListDrawArea_WritePlayer( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, WF_2DCSYS* p_charsys, const WFNOTE_SCRNDATA* cp_scrn, const WFNOTE_IDXDATA* cp_idx, u32 page, HEAPID heapID )
{
  int i;
  int idx;
  WIFI_LIST* p_list;

  p_list = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );

  // クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(*p_wk->text), 0 );

  // アクターが残っていたらはき
  FListDrawArea_CharWkDel( p_wk, p_draw );

  // 名前とアクターの生成
  idx = page * FLIST_PAGE_FRIEND_NUM;
  for( i=0; i<FLIST_PAGE_FRIEND_NUM; i++ ){
    if( cp_idx->friendnum > (idx+i) ){
      // 表示する
      FListDrawArea_SetPlayer( p_wk, p_charsys, p_draw, cp_scrn, i, p_list, cp_idx->fridx[idx+i],  heapID );
    }else{
      // プレイヤーは表示しない
      FListDrawArea_SetNoPlayer( p_wk, p_draw, cp_scrn, i, heapID );
    }
  }

  // 書き込んだBGを更新
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );

  // テキスト面表示
  GFL_BMPWIN_MakeTransWindow_VBlank( *p_wk->text );
}

//----------------------------------------------------------------------------
/**
 *  @brief  表示OFF
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 *  @param  cp_scrn   スクリーンデータ
 */
//-----------------------------------------------------------------------------
static void FListDrawArea_DrawOff( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNDATA* cp_scrn )
{
  // アクター破棄
  FListDrawArea_CharWkDel( p_wk, p_draw );

  // 表示OFF
  GFL_BMPWIN_ClearTransWindow_VBlank( *p_wk->text );
//  GF_BGL_BmpWinOffVReq( &p_wk->text );

  // 背景クリア
  GFL_BG_FillScreen( DFRM_SCROLL,
      0,
      MATH_ABS(p_wk->scrn_area.scrn_x), MATH_ABS(p_wk->scrn_area.scrn_y),
      p_wk->scrn_area.scrn_sizx,
      p_wk->scrn_area.scrn_sizy, 0 );

  // BG面転送
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );

  // スクリーン面もクリア
  GFL_BG_FillScreen( DFRM_SCRMSG,
      0,
      MATH_ABS(p_wk->scrn_area.scrn_x), MATH_ABS(p_wk->scrn_area.scrn_y),
      p_wk->scrn_area.scrn_sizx,
      p_wk->scrn_area.scrn_sizy, 0 );
  GFL_BG_LoadScreenV_Req( DFRM_SCRMSG );
}

//----------------------------------------------------------------------------
/**
 *  @brief  一気にキャラクタワークを破棄する
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void FListDrawArea_CharWkDel( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw )
{
  int i;

  for( i=0; i<FLIST_PAGE_FRIEND_NUM; i++ ){
    if( p_wk->p_clwk[ i ] ){
      WF_2DC_WkDel( p_wk->p_clwk[ i ] );
      p_wk->p_clwk[ i ] = NULL;
    }
		if( p_wk->p_hate[i] != NULL ){
			GFL_CLACT_WK_Remove( p_wk->p_hate[i] );
			p_wk->p_hate[i] = NULL;
		}
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  プレイヤーデータ表示
 *
 *  @param  p_wk    ワーク
 *  @param  p_charsys キャラクタシステム
 *  @param  p_draw    描画システム
 *  @param  cp_scrn   スクリーンデータ
 *  @param  pos     位置
 *  @param  p_list    WiFiリスト
 *  @param  idx     WiFiリスト内のインデックス
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------

static void FListDrawArea_SetPlayer( WFNOTE_FLIST_DRAWAREA* p_wk, WF_2DCSYS* p_charsys, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNDATA* cp_scrn, u32 pos, WIFI_LIST* p_list, u32 idx, HEAPID heapID )
{
  int sex;
  int view;
  WF_2DC_WKDATA adddata;
  STRBUF* p_str;
  int x, y;
  PRINTSYS_LSB sc_SEXCOL[3];
  sc_SEXCOL[0] = WFNOTE_COL_BLUE;
  sc_SEXCOL[1] = WFNOTE_COL_RED;
  sc_SEXCOL[2] = WFNOTE_COL_BLACK;

  view = WifiList_GetFriendInfo( p_list, idx, WIFILIST_FRIEND_UNION_GRA );
  sex = WifiList_GetFriendInfo( p_list, idx, WIFILIST_FRIEND_SEX );

  if( sex == PM_NEUTRAL ){
    // いない絵を表示
    FListDrawArea_SetNoPlayer( p_wk, p_draw, cp_scrn, pos, heapID );
  }else{
    // キャラクタ表示
    adddata.x = ((p_wk->scrn_area.scrn_x+DATA_FListCursorData[pos].scrn_data.scrn_x) * 8) + FLIST_PLAYER_X;
    adddata.y = ((p_wk->scrn_area.scrn_y+DATA_FListCursorData[pos].scrn_data.scrn_y) * 8) + FLIST_PLAYER_Y;
    adddata.pri = FLIST_PLAYER_PRI;
    adddata.bgpri = FLIST_PLAYER_BGPRI;
//    p_wk->p_clwk[ pos ] = WF_2DC_WkAdd( p_charsys, &adddata, view, heapID );
    p_wk->p_clwk[ pos ] = WF_2DC_WkAdd( p_charsys, &adddata, 5, heapID );
    WF_2DC_WkPatAnmStart( p_wk->p_clwk[ pos ] , WF_2DC_ANMWAY , WF2DMAP_WAY_DOWN );
  }

  //  名前表示
  x = ((DATA_FListCursorData[pos].scrn_data.scrn_x) * 8) + FLIST_PLAYERNAME_X;
  y = ((DATA_FListCursorData[pos].scrn_data.scrn_y) * 8) + FLIST_PLAYERNAME_Y;
  p_str = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  GFL_STR_SetStringCode( p_str, WifiList_GetFriendNamePtr(p_list,idx) );

  PRINTSYS_PrintQueColor( p_draw->printQue , GFL_BMPWIN_GetBmp(*p_wk->text), x, y, p_str, p_draw->fontHandle , sc_SEXCOL[sex] );

  GFL_STR_DeleteBuffer( p_str );
}

//----------------------------------------------------------------------------
/**
 *  @brief  プレイヤーのいない状態を表示
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 *  @param  cp_scrn   スクリーンデータ
 *  @param  pos     位置
 */
//-----------------------------------------------------------------------------
static void FListDrawArea_SetNoPlayer( WFNOTE_FLIST_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNDATA* cp_scrn, u32 pos, HEAPID heapID )
{
	GFL_CLWK_DATA	dat;

	dat.pos_x = ((p_wk->scrn_area.scrn_x+DATA_FListCursorData[pos].scrn_data.scrn_x) * 8) + FLIST_HATENA_X;
	dat.pos_y = ((p_wk->scrn_area.scrn_y+DATA_FListCursorData[pos].scrn_data.scrn_y) * 8) + FLIST_HATENA_Y;
	dat.anmseq = ACTANM_HATENA;
	dat.softpri = FLIST_PLAYER_PRI;
	dat.bgpri = FLIST_PLAYER_BGPRI;

	p_wk->p_hate[pos] = GFL_CLACT_WK_Create(
												p_draw->cellUnitScroll,
												p_draw->chrRes[CHR_RES_WOR],
												p_draw->palRes[PAL_RES_WOR],
												p_draw->celRes[CEL_RES_WOR],
												&dat, CLSYS_DEFREND_MAIN,
												heapID );
}

//-------------------------------------
/// 友達コード入力画面ワーク
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  友達コード入力画面  初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void CodeIn_Init( WFNOTE_CODEIN* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  CodeIn_DrawInit( p_wk, p_data, p_draw, heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達コード入力画面  メイン関数
 *
 *  @param  p_wk    ワーク
 *  @param  p_sys   システムワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static WFNOTE_STRET CodeIn_Main( WFNOTE_CODEIN* p_wk, WFNOTE_WK* p_sys, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  int block[ CODE_BLOCK_MAX ];


  switch( p_data->subseq ){
  case SEQ_CODEIN_NAMEIN:

    // 名前入力、コード入力パラメータ作成
		GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
    p_wk->p_namein = CodeIn_NameInParamMake( p_wk, p_data, heapID );
		GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );

    CodeIn_BlockDataMake_4_4_4( block );
    p_wk->p_codein = CodeInput_ParamCreate( heapID, FRIENDCODE_MAXLEN, block );

    WFNOTE_DrawExit( p_sys ); // 画面ワーク全破棄
    p_wk->p_subproc = GFL_PROC_LOCAL_boot( heapID );
    GFL_PROC_LOCAL_CallProc( p_wk->p_subproc , FS_OVERLAY_ID(namein) ,
        &NameInputProcData, p_wk->p_namein );
    p_data->subseq = SEQ_CODEIN_NAMEIN_WAIT;
    break;

  case SEQ_CODEIN_NAMEIN_WAIT:
    // 終了待ち
    if( GFL_PROC_LOCAL_Main( p_wk->p_subproc ) != GFL_PROC_MAIN_NULL ){
      break;
    }
//    GFL_PROC_LOCAL_Exit( p_wk->p_subproc );

    // キャンセルチェック
    if( p_wk->p_namein->cancel == 0 ){
      // 次へ
      GFL_PROC_LOCAL_CallProc( p_wk->p_subproc , NO_OVERLAY_ID ,
        &CodeInput_ProcData, p_wk->p_codein );
      p_data->subseq = SEQ_CODEIN_CODEIN_WAIT;
    }else{
      // 終了へ
      GFL_PROC_LOCAL_Exit( p_wk->p_subproc );
      Data_SetReqStatus( p_data, STATUS_MODE_SELECT, SEQ_MODESEL_INIT );
      p_data->subseq = SEQ_CODEIN_END;
    }
    break;

  case SEQ_CODEIN_CODEIN_WAIT:
    // 終了待ち
    if( GFL_PROC_LOCAL_Main( p_wk->p_subproc ) != GFL_PROC_MAIN_NULL ){
      break;
    }
    GFL_PROC_LOCAL_Exit( p_wk->p_subproc );

    // データを設定
    Data_CODEIN_Set( p_data, p_wk->p_namein->strbuf, p_wk->p_codein->strbuf );
    Data_SetReqStatus( p_data, STATUS_MODE_SELECT, SEQ_MODESEL_CODECHECK_INIT );
    p_data->subseq = SEQ_CODEIN_END;
    break;

  case SEQ_CODEIN_END:
    // 名前入力、コード入力画面の破棄
		GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
    NAMEIN_FreeParam( p_wk->p_namein );
		GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );
    CodeInput_ParamDelete( p_wk->p_codein );

    WFNOTE_DrawInit( p_sys, heapID ); // 画面復帰
    return WFNOTE_STRET_FINISH;



  case SEQ_CODEIN_NAMEINONLY: // 名前入力のみ
    // 名前入力、コード入力パラメータ作成
		GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
    p_wk->p_namein = CodeIn_NameInParamMake( p_wk, p_data, heapID );
		GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );

    WFNOTE_DrawExit( p_sys ); // 画面ワーク全破棄
    p_wk->p_subproc = GFL_PROC_LOCAL_boot( heapID );
    GFL_PROC_LOCAL_CallProc( p_wk->p_subproc , FS_OVERLAY_ID(namein),
        &NameInputProcData, p_wk->p_namein );
    p_data->subseq = SEQ_CODEIN_NAMEINONLY_WAIT;
    break;

  case SEQ_CODEIN_NAMEINONLY_WAIT:  // 名前入力のみ
    // 終了待ち
    if( GFL_PROC_LOCAL_Main( p_wk->p_subproc ) != GFL_PROC_MAIN_NULL ){
      break;
    }
    GFL_PROC_LOCAL_Exit( p_wk->p_subproc );

		p_data->key_mode = GFL_UI_CheckTouchOrKey();

    // キャンセルチェック
    if( p_wk->p_namein->cancel==0 ){
      // ちゃんと入力されたので、その場合のFRIENDLISTに戻る
      Data_SetReqStatus( p_data, STATUS_FRIENDLIST, SEQ_FLIST_NAMECHG_WAIT );
      // データを設定
      Data_CODEIN_Set( p_data, p_wk->p_namein->strbuf, NULL );
      p_data->subseq = SEQ_CODEIN_NAMEINONLY_END;
    }else{
      // 終了へ
      Data_SetReqStatus( p_data, STATUS_FRIENDLIST, SEQ_FLIST_NAMECHG_WAITWIPE );
      p_data->subseq = SEQ_CODEIN_NAMEINONLY_END;
    }
    break;

  case SEQ_CODEIN_NAMEINONLY_END: // 名前入力のみ
    // 名前入力、コード入力画面の破棄
		GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
    NAMEIN_FreeParam( p_wk->p_namein );
		GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );

    WFNOTE_DrawInit( p_sys, heapID ); // 画面復帰
    return WFNOTE_STRET_FINISH;
  }
  return WFNOTE_STRET_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達コード入力画面  破棄
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void CodeIn_Exit( WFNOTE_CODEIN* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{

  CodeIn_DrawExit( p_wk, p_data, p_draw );
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画関係の初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープ
 */
//-----------------------------------------------------------------------------
static void CodeIn_DrawInit( WFNOTE_CODEIN* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画関係の破棄
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void CodeIn_DrawExit( WFNOTE_CODEIN* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
}

//----------------------------------------------------------------------------
/**
 *  @brief  名前入力　ワーク作成
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static NAMEIN_PARAM* CodeIn_NameInParamMake( WFNOTE_CODEIN* p_wk, WFNOTE_DATA* p_data, HEAPID heapID )
{
  NAMEIN_PARAM* p_param;

  p_param = NAMEIN_AllocParam( heapID, NAMEIN_FRIENDNAME, 0, 0,
              NAMEIN_PERSON_LENGTH,
              NULL );

  // 名前入力画面初期化dataチェック
  if( p_data->namein_init.init == TRUE ){
//@todo 言語対応
    #if( PM_LANG == LANG_JAPAN )
    {
      //if( IsJapaneseCode( *p_data->namein_init.cp_str ) ){
      if( TRUE ){
        GFL_STR_SetStringCode( p_param->strbuf, p_data->namein_init.cp_str );
      }else{
        GFL_STR_ClearBuffer( p_param->strbuf );
      }
    }
    #else
    {
      //if( IsJapaneseCode( *p_data->namein_init.cp_str ) == FALSE ){
      if( FALSE ){
        GFL_STR_SetStringCode( p_param->strbuf, p_data->namein_init.cp_str );
      }else{
        GFL_STR_ClearBuffer( p_param->strbuf );
      }
    }
    #endif
    p_data->namein_init.init = FALSE;
  }
  return p_param;
}

//-------------------------------------
/// 自分の友達コード確認画面ワーク
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  自分の友達コード確認画面ワーク初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープ
 */
//-----------------------------------------------------------------------------
static void MyCode_Init( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  MyCode_DrawInit( p_wk, p_data, p_draw, heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  自分の友達コード確認画面 メイン処理
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *
 *  @retval WFNOTE_STRET_CONTINUE,  // つづく
 *  @retval WFNOTE_STRET_FINISH,  // 作業は終わった
 */
//-----------------------------------------------------------------------------
static WFNOTE_STRET MyCode_Main( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  switch( p_data->subseq ){
  case SEQ_MYCODE_INIT:
    MyCode_Init( p_wk, p_data, p_draw, HEAPID_WIFINOTE );
    MyCodeSeq_Init( p_wk, p_data, p_draw );
    p_data->subseq = SEQ_MYCODE_MAIN;
    break;

  case SEQ_MYCODE_MAIN:
    if(!MyCode_Input( p_wk, p_data, p_draw )){
      break;
    }
    PMSND_PlaySE( WIFINOTE_BS_SE );
		SetCancelButtonAnime( p_draw );
//    MyCode_BackButtonDraw( p_wk, p_draw , TRUE);
//    p_wk->ct = WFNOTE_BTNANM_WAIT;
    p_data->subseq = SEQ_MYCODE_BTNANM;
    break;
  case SEQ_MYCODE_BTNANM:
/*
    if(--p_wk->ct > 0){
      break;
    }
*/
		if( GFL_CLACT_WK_CheckAnmActive( p_draw->p_tb ) == TRUE ){
			break;
		}
//    MyCode_BackButtonDraw( p_wk, p_draw , FALSE );
    Data_SetReqStatus( p_data, STATUS_MODE_SELECT, SEQ_MODESEL_INIT_NOFADE );
    MyCode_DrawOff( p_wk, p_draw ); // 表示OFF
    MyCode_Exit( p_wk, p_data, p_draw );
    return WFNOTE_STRET_FINISH;
  }
  return WFNOTE_STRET_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  自分の友達コード確認画面ワーク破棄
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void MyCode_Exit( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  MyCode_DrawExit( p_wk, p_data, p_draw );
}

//----------------------------------------------------------------------------
/**
 *  @brief  自分の友だちコード確認画面　入力制御
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *
 *  @retval FALSE,  // つづく
 *  @retval TRUE, // トップに戻る
 */
//-----------------------------------------------------------------------------
static BOOL MyCode_Input( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  static const GFL_UI_TP_HITTBL TpRect[] = {
  //  {0,191,0,255}, ty,by,lx,rx
    {TOUCHBAR_ICON_Y,TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1,TOUCHBAR_ICON_X_07,TOUCHBAR_ICON_X_07+TOUCHBAR_ICON_WIDTH-1},
    {GFL_UI_TP_HIT_END,0,0,0},
  };

  if( GFL_UI_TP_HitTrg(TpRect) == 0 ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
    p_data->key_mode = GFL_APP_END_TOUCH;
    return TRUE;
  }

//  Data_CheckKeyModeChange(p_data);
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
    p_data->key_mode = GFL_APP_END_KEY;
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画関係の初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void MyCode_DrawInit( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  u64 code;
  DWCUserData* p_userdata;
  WIFI_LIST* p_list;
  STRBUF* p_str;
  STRBUF* p_tmp;

  // ビットマップ作成
  Sub_BmpWinAdd( &p_wk->msg, DFRM_SCRMSG,
      BMPL_MYCODE_MSG_X, BMPL_MYCODE_MSG_Y,
      BMPL_MYCODE_MSG_SIZX, BMPL_MYCODE_MSG_SIZY,
      BGPLT_M_MSGFONT, BMPL_MYCODE_MSG_CGX , 0);
  Sub_BmpWinAdd( &p_wk->code, DFRM_SCRMSG,
      BMPL_MYCODE_CODE_X, BMPL_MYCODE_CODE_Y,
      BMPL_MYCODE_CODE_SIZX, BMPL_MYCODE_CODE_SIZY,
      BGPLT_M_MSGFONT, BMPL_MYCODE_CODE_CGX , 0);
/*
  Sub_BmpWinAdd( &p_wk->back, DFRM_SCRMSG,
      BMPL_MYCODE_BACK_X, BMPL_MYCODE_BACK_Y,
      BMPL_MYCODE_BACK_SIZX, BMPL_MYCODE_BACK_SIZY,
      BGPLT_M_MSGFONT, BMPL_MYCODE_BACK_CGX , 0);
*/

  p_str = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  p_tmp = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );

  // 自分のコード取得
  p_list = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData(p_data->p_save);
  p_userdata = WifiList_GetMyUserInfo( p_list );
  code = DWC_CreateFriendKey( p_userdata );

  if( code != 0 ){
    // コードあり
    // メッセージ
    GFL_MSG_GetString( p_draw->p_msgman, msg_mycode_codeon, p_str );
    PRINTSYS_PrintQueColor( p_draw->printQue , GFL_BMPWIN_GetBmp(p_wk->msg), 4, 0, p_str, p_draw->fontHandle , WFNOTE_COL_BLACK);

    // 数字
    Draw_FriendCodeSetWordset( p_draw, code );
    GFL_MSG_GetString( p_draw->p_msgman, msg_mycode_code, p_tmp );
    WORDSET_ExpandStr( p_draw->p_wordset, p_str, p_tmp );
    PRINTSYS_PrintQueColor( p_draw->printQue , GFL_BMPWIN_GetBmp(p_wk->code), 0, 0, p_str, p_draw->fontHandle , WFNOTE_COL_WHITE);
  }else{
    // コードなし
    // メッセージ
    GFL_MSG_GetString( p_draw->p_msgman, msg_mycode_codeoff, p_str );
    PRINTSYS_PrintQueColor( p_draw->printQue , GFL_BMPWIN_GetBmp(p_wk->msg), 4, 0, p_str, p_draw->fontHandle , WFNOTE_COL_BLACK);
  }
  //戻るボタン
/*
  GFL_MSG_GetString( p_draw->p_msgman, msg_mycode_back, p_str );
  PRINTSYS_PrintQueColor( p_draw->printQue , GFL_BMPWIN_GetBmp(p_wk->back), 0, 0, p_str, p_draw->fontHandle , WFNOTE_TCOL_BLACK);
*/
  GFL_STR_DeleteBuffer( p_str );
  GFL_STR_DeleteBuffer( p_tmp );

  // スクリーンデータ読み込み
  p_wk->p_scrnbuff = GFL_ARCHDL_UTIL_LoadScreen( p_draw->p_handle,
      NARC_wifi_note_friend_code_d_NSCR, FALSE, &p_wk->p_scrn, heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画関係の破棄
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void MyCode_DrawExit( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  GFL_BMPWIN_Delete( p_wk->msg );
  GFL_BMPWIN_Delete( p_wk->code );
//  GFL_BMPWIN_Delete( p_wk->back );
  GFL_HEAP_FreeMemory( p_wk->p_scrnbuff );
}

//----------------------------------------------------------------------------
/**
 *  @brief  初期化　シーケンス
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void MyCodeSeq_Init( WFNOTE_MYCODE* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{

  // タイトル設定
  WORDSET_RegisterPlayerName( p_draw->p_wordset, 0, GAMEDATA_GetMyStatus(p_data->pGameData) );
  Draw_BmpTitleWrite( p_draw, &p_draw->title, msg_mycode_title );

  // 背景設定
  GFL_BG_WriteScreen( DFRM_SCROLL,
      p_wk->p_scrn->rawData, 0, 0,
      p_wk->p_scrn->screenWidth/8, p_wk->p_scrn->screenHeight/8 );
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );

  // 文字列設定

  GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->msg );
  GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->code );
//  GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->back );
}

//----------------------------------------------------------------------------
/**
 *  @brief  表示OFF
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void MyCode_DrawOff( WFNOTE_MYCODE* p_wk, WFNOTE_DRAW* p_draw )
{
  // タイトル
  Draw_BmpTitleOff( p_draw );

  // SCROLL面
  GFL_BG_FillScreen( DFRM_SCROLL, 0, 0, 0, 32, 24, 0 );
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );

  // msg
  GFL_BMPWIN_ClearTransWindow_VBlank( p_wk->msg );
//  GF_BGL_BmpWinOffVReq(&p_wk->msg);
  GFL_BMPWIN_ClearTransWindow_VBlank( p_wk->code );
//  GF_BGL_BmpWinOffVReq(&p_wk->code);
//  GFL_BMPWIN_ClearTransWindow_VBlank( p_wk->back );
//  GF_BGL_BmpWinOffVReq(&p_wk->back);

  // SCRMSG面
  GFL_BG_FillScreen( DFRM_SCRMSG, 0, 0, 0, 32, 24, 0 );
  GFL_BG_LoadScreenV_Req( DFRM_SCRMSG );
}

//----------------------------------------------------------------------------
/**
 *  @brief  戻るボタン状態表示
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void MyCode_BackButtonDraw( WFNOTE_MYCODE* p_wk, WFNOTE_DRAW* p_draw ,BOOL on_f)
{
  u8 pal[] = {MODESEL_CUR_PAL_OFF,MODESEL_CUR_PAL_ON};

  GFL_BG_ChangeScreenPalette( DFRM_SCROLL, 17, 20, 14, 4, pal[on_f]);
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
}

//-------------------------------------
/// 友達詳細情報
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  友達詳細表示画面  初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfo_Init( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  // SaveDataからフロンティアを表示するかチェックする
  // 今は常に表示
//FIXME 正しいチェックを ↑はGS移植時の話
  //p_wk->frontier_draw = SysFlag_ArriveGet(SaveData_GetEventWork(p_data->p_save),FLAG_ARRIVE_D32R0101);
  p_wk->frontier_draw = TRUE;

  FInfo_DrawInit( p_wk, p_data, p_draw, heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達詳細表示画面  メイン
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static WFNOTE_STRET FInfo_Main( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  BOOL result;

  switch( p_data->subseq ){
  case SEQ_FINFO_INIT:
    p_wk->rep = FALSE;
    p_wk->page = 0;
  case SEQ_FINFO_INITRP:
    // 描画開始
    FInfo_Init( p_wk, p_data, p_draw, heapID );
    FInfo_DrawOn( p_wk, p_data, p_draw, heapID );
    p_data->subseq = SEQ_FINFO_MAIN;
    break;

  case SEQ_FINFO_MAIN:
    p_data->subseq = FInfo_Input( p_wk, p_data, p_draw );
    break;

  case SEQ_FINFO_SCRINIT:
    FInfo_ScrollInit( p_wk, p_data, p_draw, heapID );
    FInfo_ScrollMain( p_wk, p_data, p_draw, heapID );
    p_data->subseq = SEQ_FINFO_SCR;
    break;

  case SEQ_FINFO_SCR:
    result = FInfo_ScrollMain( p_wk, p_data, p_draw, heapID );
    if( result == TRUE ){
      p_data->subseq = SEQ_FINFO_SCR_WAIT_MSG;
    }
    break;

  case SEQ_FINFO_SCR_WAIT_MSG:
    if( PRINTSYS_QUE_IsFinished(p_draw->printQue) == TRUE ){
      // スクロール座標を元に戻す
      GFL_BG_SetScrollReq( DFRM_SCROLL, GFL_BG_SCROLL_X_SET, 0 );
      GFL_BG_SetScrollReq( DFRM_SCRMSG, GFL_BG_SCROLL_X_SET, 0 );

      p_data->subseq = SEQ_FINFO_MAIN;
    }
    break;

    // 表示する人をかえる
  case SEQ_FINFO_FRIENDCHG:
    if( p_wk->rep ){  // リピート中ならウエイト
      p_wk->wait --;
      if( p_wk->wait > 0 ){
        break;
      }
    }

    result = FInfo_SelectListIdxAdd( p_data, p_wk->way );
    if( result ){

      PMSND_PlaySE( WIFINOTE_MOVE_SE );

//      FInfo_DrawOff( p_wk, p_draw );
      FInfo_DrawOffFriendChange( p_wk, p_draw );
      Data_SetReqStatus( p_data, STATUS_FRIENDINFO, SEQ_FINFO_INITRP );
      // 次すぐ上下が押されていたらリピート
      p_wk->rep = TRUE;
      FInfo_DrawExit( p_wk, p_data, p_draw );
      return WFNOTE_STRET_FINISH;
    }

    // 変更できなかったので元に戻る
    p_data->subseq = SEQ_FINFO_MAIN;
    break;

  case SEQ_FINFO_END:
    FInfo_DrawOff( p_wk, p_draw );
    Data_SetReqStatus( p_data, STATUS_FRIENDLIST, SEQ_FLIST_INFORET );
    FInfo_DrawExit( p_wk, p_data, p_draw );
    return WFNOTE_STRET_FINISH;
  }
  return WFNOTE_STRET_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達詳細表示画面  破棄
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void FInfo_Exit( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  FInfo_DrawExit( p_wk, p_data, p_draw );
}

//----------------------------------------------------------------------------
/**
 *  @brief  詳細画面　入力ページが最終ページかどうか？
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static BOOL FInfoInput_IsEndPage( WFNOTE_FRIENDINFO* p_wk ,s16 page )
{
  s16 page_num;
  // フロンティアを描画するかチェック
  if( p_wk->frontier_draw == TRUE ){
    page_num = FINFO_PAGE_NUM;
  }else{
    page_num = FINFO_BFOFF_PAGE_NUM;
  }
  if(page == page_num-1){
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  詳細画面　入力コード実行
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static u32 FInfoInput_Exe( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw ,u32 code )
{
  u8 page_num;

  switch( code ){
  case FINFO_EXE_DECIDE:
    if( FInfoInput_IsEndPage( p_wk, p_wk->page )){
      PMSND_PlaySE( WIFINOTE_DECIDE_SE );
      return SEQ_FINFO_END;
    }
    break;
  case FINFO_EXE_CANCEL:
    PMSND_PlaySE( WIFINOTE_DECIDE_SE );
    return SEQ_FINFO_END;
  case FINFO_EXE_LEFT:
    FInfo_CursorDraw( p_wk, p_draw, p_wk->page );
    FInfo_ShioriDraw( p_wk, p_draw, p_wk->page );
    p_wk->way = WF_COMMON_LEFT;
    return SEQ_FINFO_SCRINIT;
  case FINFO_EXE_RIGHT:
    FInfo_CursorDraw( p_wk, p_draw, p_wk->page );
    FInfo_ShioriDraw( p_wk, p_draw, p_wk->page );
    p_wk->way = WF_COMMON_RIGHT;
    return SEQ_FINFO_SCRINIT;
  case FINFO_EXE_TOP:
    p_wk->way = WF_COMMON_TOP;
    p_wk->wait = FINFO_PAGECHG_WAIT;
    Draw_YazirushiAnmChg( p_draw, ACT_YAZI_T, ACT_YAZIANM_ACTIVE );
    return SEQ_FINFO_FRIENDCHG;
  case FINFO_EXE_BOTTOM:
    p_wk->way = WF_COMMON_BOTTOM;
    p_wk->wait = FINFO_PAGECHG_WAIT;
    Draw_YazirushiAnmChg( p_draw, ACT_YAZI_B, ACT_YAZIANM_ACTIVE );
    return SEQ_FINFO_FRIENDCHG;
  }
  return SEQ_FINFO_MAIN;
}

//----------------------------------------------------------------------------
/**
 *  @brief  詳細画面　キー入力取得
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static u32 FInfoInput_Key( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  if(GFL_UI_KEY_GetTrg() & GFL_PAD_BUTTON_KTST_CHG){
    p_data->key_mode = GFL_APP_END_KEY;
  }

  if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL) ){
    return FInfoInput_Exe( p_wk, p_data, p_draw ,FINFO_EXE_CANCEL );
  }else{
    if( ( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ) ||
      ( p_data->lr_key_enable && (GFL_UI_KEY_GetRepeat()  & PAD_BUTTON_L) ) ){
      FInfo_PageChange( p_wk, -1 );
      return FInfoInput_Exe( p_wk, p_data, p_draw ,FINFO_EXE_LEFT);
    }else if( ( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ) ||
          ( p_data->lr_key_enable && (GFL_UI_KEY_GetRepeat()  & PAD_BUTTON_R) ) ){
      FInfo_PageChange( p_wk, 1 );
      return FInfoInput_Exe( p_wk, p_data, p_draw ,FINFO_EXE_RIGHT);
    }else if( GFL_UI_KEY_GetCont() & PAD_KEY_UP ){
      return FInfoInput_Exe( p_wk, p_data, p_draw ,FINFO_EXE_TOP);
    }else if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN ){
      return FInfoInput_Exe( p_wk, p_data, p_draw ,FINFO_EXE_BOTTOM);
    }
  }
  return SEQ_FINFO_MAIN;
}

//----------------------------------------------------------------------------
/**
 *  @brief  詳細画面　タッチ入力取得
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static u32 FInfoInput_Touch( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, BOOL* touch_f )
{
  u32 ret;
  s16 dif;

  static const GFL_UI_TP_HITTBL TpRectBfOn[] = {
  //  {0,191,0,255}, ty,by,lx,rx
    {TP_FINFO_YAZI_Y1,TP_FINFO_YAZI_Y1+TP_FINFO_YAZI_H,TP_FINFO_YAZI_X,TP_FINFO_YAZI_X+TP_FINFO_YAZI_W},
    {TP_FINFO_YAZI_Y2,TP_FINFO_YAZI_Y2+TP_FINFO_YAZI_H,TP_FINFO_YAZI_X,TP_FINFO_YAZI_X+TP_FINFO_YAZI_W},
    {TP_FINFO_BTN_Y,TP_FINFO_BTN_Y+TP_FINFO_BTN_H,TP_FINFO_BTN01_X1,TP_FINFO_BTN01_X1+TP_FINFO_BTN01_W},
    {TP_FINFO_BTN_Y,TP_FINFO_BTN_Y+TP_FINFO_BTN_H,TP_FINFO_BTN01_X2,TP_FINFO_BTN01_X2+TP_FINFO_BTN01_W},
    {TP_FINFO_BTN_Y,TP_FINFO_BTN_Y+TP_FINFO_BTN_H,TP_FINFO_BTN01_X3,TP_FINFO_BTN01_X3+TP_FINFO_BTN01_W},
    {TP_FINFO_BTN_Y,TP_FINFO_BTN_Y+TP_FINFO_BTN_H,TP_FINFO_BTN01_X4,TP_FINFO_BTN01_X4+TP_FINFO_BTN01_W},
    {TP_FINFO_BTN_Y,TP_FINFO_BTN_Y+TP_FINFO_BTN_H,TP_FINFO_BTN01_X5,TP_FINFO_BTN01_X5+TP_FINFO_BTN01_W},
    {TP_FINFO_BTN_Y,TP_FINFO_BTN_Y+TP_FINFO_BTN_H,TP_FINFO_BTN01_X6,TP_FINFO_BTN01_X6+TP_FINFO_BTN01_W},
    {TP_FINFO_BTN_Y,TP_FINFO_BTN_Y+TP_FINFO_BTN_H,TP_FINFO_BTN01_X7,TP_FINFO_BTN01_X7+TP_FINFO_BTN01_W},
    {TP_FINFO_BTN_Y,TP_FINFO_BTN_Y+TP_FINFO_BTN_H,TP_FINFO_BTN01_X8,TP_FINFO_BTN01_X8+TP_FINFO_BTN01_W},
    {GFL_UI_TP_HIT_END,0,0,0},
  };
  static const GFL_UI_TP_HITTBL TpRectBfOff[] = {
  //  {0,191,0,255}, ty,by,lx,rx
    {TP_FINFO_YAZI_Y1,TP_FINFO_YAZI_Y1+TP_FINFO_YAZI_H,TP_FINFO_YAZI_X,TP_FINFO_YAZI_X+TP_FINFO_YAZI_W},
    {TP_FINFO_YAZI_Y2,TP_FINFO_YAZI_Y2+TP_FINFO_YAZI_H,TP_FINFO_YAZI_X,TP_FINFO_YAZI_X+TP_FINFO_YAZI_W},
    {TP_FINFO_BTN_Y,TP_FINFO_BTN_Y+TP_FINFO_BTN_H,TP_FINFO_BTN02_X1,TP_FINFO_BTN02_X1+TP_FINFO_BTN01_W},
    {TP_FINFO_BTN_Y,TP_FINFO_BTN_Y+TP_FINFO_BTN_H,TP_FINFO_BTN02_X2,TP_FINFO_BTN02_X2+TP_FINFO_BTN01_W},
    {TP_FINFO_BTN_Y,TP_FINFO_BTN_Y+TP_FINFO_BTN_H,TP_FINFO_BTN01_X8,TP_FINFO_BTN01_X8+TP_FINFO_BTN01_W},
    {GFL_UI_TP_HIT_END,0,0,0},
  };
  if(p_wk->frontier_draw){
    ret = GFL_UI_TP_HitTrg(TpRectBfOn);
  }else{
    ret = GFL_UI_TP_HitTrg(TpRectBfOff);
  }

  if(ret == GFL_UI_TP_HIT_NONE){
    return SEQ_FINFO_MAIN;
  }
  if(ret == 0){
    *touch_f = TRUE;
    return FInfoInput_Exe( p_wk, p_data, p_draw ,FINFO_EXE_TOP );
  }else if(ret == 1){
    *touch_f = TRUE;
    return FInfoInput_Exe( p_wk, p_data, p_draw ,FINFO_EXE_BOTTOM );
  }

  ret -= 2;
  if( FInfoInput_IsEndPage( p_wk, ret )){
    p_wk->page = ret;
    *touch_f = TRUE;
    return FInfoInput_Exe( p_wk, p_data, p_draw ,FINFO_EXE_CANCEL );
  }
  dif = ret - p_wk->page;
  if(dif == 0){
    return SEQ_FINFO_MAIN;
  }
  *touch_f = TRUE;

  FInfo_PageChange( p_wk, dif );
  if(dif >= 0){
    return FInfoInput_Exe( p_wk, p_data, p_draw ,FINFO_EXE_RIGHT);
  }else{
    return FInfoInput_Exe( p_wk, p_data, p_draw ,FINFO_EXE_LEFT);
  }
  return SEQ_FINFO_MAIN;
}

//----------------------------------------------------------------------------
/**
 *  @brief  詳細画面　入力取得
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static u32 FInfo_Input( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  u32 ret;
  BOOL touch_f = FALSE;

  if( (GFL_UI_KEY_GetCont() & (PAD_KEY_UP|PAD_KEY_DOWN)) == 0 ){
    p_wk->rep = FALSE;
  }
  ret = FInfoInput_Touch(p_wk, p_data, p_draw, &touch_f);
  if(touch_f){
    p_data->key_mode = GFL_APP_END_TOUCH;
    return ret;
  }
  return FInfoInput_Key( p_wk, p_data, p_draw );
}

//----------------------------------------------------------------------------
/**
 *  @brief  スクロール初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfo_ScrollInit( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  u32 drawarea;

  p_wk->count = 0;

  // 矢印のアニメをとめる
  Draw_YazirushiSetAnmFlag( p_draw, FALSE );

  // スクロール方向にページを書き込む
  if( p_wk->way == WF_COMMON_LEFT ){
    drawarea = WFNOTE_DRAWAREA_LEFT;
  }else{
    drawarea = WFNOTE_DRAWAREA_RIGHT;
  }
  FInfo_DrawPage( p_wk, p_data, p_draw, drawarea, p_wk->page, heapID );

  // スクロール音
  PMSND_PlaySE( WIFINOTE_SCRLL_SE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  スクロールメイン
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 *
 *  @retval TRUE  終了
 *  @retval FALSE 途中
 */
//-----------------------------------------------------------------------------
static BOOL FInfo_ScrollMain( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  u32 draw_area;
  s32 scrll_x;
  NNSG2dViewRect rect;
  u32 idx;

  // カウンタが終わったらすべてを描画して終わる
  if( p_wk->count > FINFO_SCROLL_COUNT ){

    // スクロール座標を元に戻す
    //メッセージ表示を待ってから戻すように
    //GFL_BG_SetScrollReq( DFRM_SCROLL, GFL_BG_SCROLL_X_SET, 0 );
    //GFL_BG_SetScrollReq( DFRM_SCRMSG, GFL_BG_SCROLL_X_SET, 0 );


    // 作業用グラフィックを消す
    if( p_wk->way == WF_COMMON_LEFT ){
      draw_area = WFNOTE_DRAWAREA_LEFT;
    }else{
      draw_area = WFNOTE_DRAWAREA_RIGHT;
    }
    // メイン面に表示
    FInfo_DrawPage( p_wk, p_data, p_draw, WFNOTE_DRAWAREA_MAIN,
        p_wk->page, heapID );

    // 矢印動作開始
    Draw_YazirushiSetAnmFlag( p_draw, TRUE );
    FInfo_ShioriDraw( p_wk, p_draw, 0xFF );
    return TRUE;
  }


  scrll_x = (p_wk->count * FINFO_SCROLL_SIZX) / FINFO_SCROLL_COUNT;

  if( p_wk->way == WF_COMMON_LEFT ){
    scrll_x = -scrll_x;
  }

  //  スクロール座標設定
  GFL_BG_SetScroll( DFRM_SCRMSG, GFL_BG_SCROLL_X_SET, scrll_x );

  // カウント
  p_wk->count ++;

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープ
 */
//-----------------------------------------------------------------------------
static void FInfo_DrawInit( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  int i, j;
  static const u32 tbl_DrawAreaCgx[ WFNOTE_DRAWAREA_NUM ] = {
    BMPL_FINFO_TEXT_CGX0,
    BMPL_FINFO_TEXT_CGX1,
    BMPL_FINFO_TEXT_CGX1,
  };

  // 各ページのスクリーンデータ読み込み
  FInfoScrnData_Init( p_wk->scrn, p_draw, heapID );
  //ベースウィンドウ追加
//  FInfoBmpWin_Init( p_wk, p_draw );

  // 描画用ビットマップ
  for( i=0; i < 2; i++ )
  {
    Sub_BmpWinAdd( &p_wk->drawBmpWin[i],
        DFRM_SCRMSG,
        FINFO_PAGE_X+MATH_ABS(DATA_ScrnArea[i].scrn_x),
        FINFO_PAGE_Y+MATH_ABS(DATA_ScrnArea[i].scrn_y),
        FINFO_PAGE_SIZX, FINFO_PAGE_SIZY,
        BGPLT_M_MSGFONT, 0 , 0 );
  }


  // 描画エリア初期化
  for( i=0; i<WFNOTE_DRAWAREA_NUM; i++ ){
    FInfoDrawArea_Init( &p_wk->drawarea[i],  p_draw, &DATA_ScrnArea[i],
      tbl_DrawAreaCgx[i], heapID );
    // 描画エリア内のビットマップ作成
    for( j=0; j<FINFO_PAGE_NUM; j++ ){
      FInfoDrawArea_MsgBmpInit( &p_wk->drawarea[i], p_draw, j,
          DATA_FInfo_PageBmpNum[j],
          DATA_FInfo_PageBmpP[j], heapID );

      p_wk->drawarea[WFNOTE_DRAWAREA_MAIN ].p_msg[j] = &p_wk->drawBmpWin[0];
      p_wk->drawarea[WFNOTE_DRAWAREA_RIGHT].p_msg[j] = &p_wk->drawBmpWin[1];
      p_wk->drawarea[WFNOTE_DRAWAREA_LEFT ].p_msg[j] = &p_wk->drawBmpWin[1];
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画破棄
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void FInfo_DrawExit( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw )
{
  int i, j;

  // 描画エリア破棄
  for( i=0; i<WFNOTE_DRAWAREA_NUM; i++ ){
    for( j=0; j<FINFO_PAGE_NUM; j++ ){
      FInfoDrawArea_MsgBmpExit( &p_wk->drawarea[i], j );
    }
    FInfoDrawArea_Exit( &p_wk->drawarea[i] );
  }
  // 描画用ビットマップ
  for( i=0; i < 2; i++ )
  {
    GFL_BMPWIN_Delete( p_wk->drawBmpWin[i] );
  }
  //ウィンドウ破棄
//  FInfoBmpWin_Exit( p_wk, p_draw );
  // 各ページのスクリーンデータ破棄
  FInfoScrnData_Exit( p_wk->scrn );
}

//----------------------------------------------------------------------------
/**
 *  @brief  表示開始
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfo_DrawOn( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  GFL_CLACTPOS pos;
  // タイトル表示
  Draw_FriendNameSetWordset( p_draw, p_data,
      p_data->idx.fridx[p_data->select_listidx], heapID );
//  Draw_BmpTitleWrite( p_draw, &p_wk->win[FINFO_WIN_TITLE], msg_wifi_note_14 );

  //上画面ベースBG表示
/*
  GFL_BG_WriteScreen( UFRM_BASE,
      p_wk->scrn[FINFO_SCRNDATA_S].p_scrn->rawData,
      0, 0,
      p_wk->scrn[FINFO_SCRNDATA_S].p_scrn->screenWidth/8,
      p_wk->scrn[FINFO_SCRNDATA_S].p_scrn->screenHeight/8 );
  GFL_BG_LoadScreenV_Req( UFRM_BASE );
*/
  //下画面ベースBG表示
  GFL_BG_WriteScreen( DFRM_BACK,
      p_wk->scrn[FINFO_SCRNDATA_MB].p_scrn->rawData,
      0, 0,
      p_wk->scrn[FINFO_SCRNDATA_MB].p_scrn->screenWidth/8,
      p_wk->scrn[FINFO_SCRNDATA_MB].p_scrn->screenHeight/8 );
  GFL_BG_LoadScreenV_Req( DFRM_BACK );
  GFL_BG_WriteScreen( DFRM_SCROLL,
      p_wk->scrn[FINFO_SCRNDATA_M].p_scrn->rawData,
      0, 0,
      p_wk->scrn[FINFO_SCRNDATA_M].p_scrn->screenWidth/8,
      p_wk->scrn[FINFO_SCRNDATA_M].p_scrn->screenHeight/8 );
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );

//  FInfo_DrawBaseInfo( p_wk, p_data, p_draw, heapID );

  Draw_CursorActorPriSet( p_draw, 0, 0 );

  // ページの表示
  FInfo_DrawPage( p_wk, p_data, p_draw, WFNOTE_DRAWAREA_MAIN,
      p_wk->page, heapID );

  // 矢印表示
  pos.x = WFNOTE_YAZIRUSHI_TBX;
  pos.y = WFNOTE_YAZIRUSHI_TY;
  GFL_CLACT_WK_SetPos( p_draw->p_yazirushi[ACT_YAZI_L], &pos , CLSYS_DEFREND_MAIN );
  Draw_YazirushiAnmChg( p_draw, ACT_YAZI_T, ACT_YAZIANM_NORMAL );

  pos.y = WFNOTE_YAZIRUSHI_BY;
  GFL_CLACT_WK_SetPos( p_draw->p_yazirushi[ACT_YAZI_R], &pos , CLSYS_DEFREND_MAIN );
  Draw_YazirushiAnmChg( p_draw, ACT_YAZI_B, ACT_YAZIANM_NORMAL );
  Draw_YazirushiSetDrawFlag( p_draw, TRUE );

  //キャラクタベース更新
  GFL_BG_SetPriority( DFRM_SCROLL, 0);
//  GF_BGL_BGControlReset( p_draw->p_bgl, DFRM_MSG, BGL_RESET_CHRBASE,GX_BG_CHARBASE_0x10000 );
}

//----------------------------------------------------------------------------
/**
 *  @brief  表示OFF
 *
 *  @param  p_wk  ワーク
 *  @param  p_draw  描画システム
 *
 */
//-----------------------------------------------------------------------------
static void FInfo_DrawOff( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DRAW* p_draw )
{
  int i;
  GFL_CLACTPOS pos;

  Draw_YazirushiSetDrawFlag( p_draw, FALSE );

  pos.x = WFNOTE_YAZIRUSHI_LX;
  pos.y = WFNOTE_YAZIRUSHI_LRY;
  GFL_CLACT_WK_SetPos( p_draw->p_yazirushi[ACT_YAZI_L], &pos , CLSYS_DEFREND_MAIN );
  Draw_YazirushiAnmChg( p_draw, ACT_YAZI_L, ACT_YAZIANM_NORMAL );

  pos.x = WFNOTE_YAZIRUSHI_RX;
  GFL_CLACT_WK_SetPos( p_draw->p_yazirushi[ACT_YAZI_R], &pos , CLSYS_DEFREND_MAIN );
  Draw_YazirushiAnmChg( p_draw, ACT_YAZI_R, ACT_YAZIANM_NORMAL );

  for( i=0; i<WFNOTE_DRAWAREA_NUM; i++ ){
    FInfoDrawArea_PageOff( &p_wk->drawarea[ i ], p_draw );
  }
  FInfo_ShioriOff( p_wk, p_draw );
  FInfo_CursorOff( p_wk, p_draw );
//  FInfo_DrawOffBaseInfo( p_wk );

//  GFL_BG_FillScreen( UFRM_BASE, 0, 0, 0, 32, 32, 0);
//  GFL_BG_FillScreen( UFRM_MSG, 0, 0, 0, 32, 32, 0);
  GFL_BG_FillScreen( DFRM_SCROLL, 0, 0, 0, 32, 32, 0);
//  GFL_BG_LoadScreenV_Req( UFRM_BASE );
//  GFL_BG_LoadScreenV_Req( UFRM_MSG );
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );

  GFL_BG_WriteScreen( DFRM_BACK,
      p_wk->scrn[FINFO_SCRNDATA_MO].p_scrn->rawData,
      0, 0,
      p_wk->scrn[FINFO_SCRNDATA_MO].p_scrn->screenWidth/8,
      p_wk->scrn[FINFO_SCRNDATA_MO].p_scrn->screenHeight/8 );
  GFL_BG_LoadScreenV_Req( DFRM_BACK );

  Draw_BmpTitleOff( p_draw );

//  GF_BGL_BGControlReset( p_draw->p_bgl, DFRM_MSG, BGL_RESET_CHRBASE,GX_BG_CHARBASE_0x00000 );
  GFL_BG_SetPriority( DFRM_SCROLL, 2);
}

//----------------------------------------------------------------------------
/**
 *  @brief  リスト変更時　表示OFF
 *
 *  @param  p_wk  ワーク
 *  @param  p_draw  描画システム
 *
 */
//-----------------------------------------------------------------------------
static void FInfo_DrawOffFriendChange( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DRAW* p_draw )
{
  int i;

//  Draw_YazirushiSetDrawFlag( p_draw, FALSE );

  for( i=0; i<WFNOTE_DRAWAREA_NUM; i++ ){
    FInfoDrawArea_PageOff( &p_wk->drawarea[ i ], p_draw );
  }
/*
  //
  for(i = FINFO_WIN_PAGE;i <= FINFO_WIN_TRGRA;i++){
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->win[i]), 0 );
    GFL_BMPWIN_TransVramCharacter(p_wk->win[i]);
  }
*/
  Draw_BmpTitleOff( p_draw );
}

//----------------------------------------------------------------------------
/**
 *  @brief  上画面ベース情報の表示
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfo_DrawBaseInfo( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
/*
  WIFI_LIST* p_wifilist;
  u32 sex,col,i;

  p_wifilist = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );
  sex = WifiList_GetFriendInfo( p_wifilist, p_data->idx.fridx[p_data->select_listidx], WIFILIST_FRIEND_SEX );

  for(i = 0; i < FINFO_WIN_NUM;i++){
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(p_wk->win[i]),0);
  }
  //ページナンバー
  WORDSET_RegisterNumber( p_draw->p_wordset, 0, p_data->idx.infoidx[p_data->select_listidx]+1,
              2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_RegisterNumber( p_draw->p_wordset, 1, p_data->idx.infonum,
              2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);

  FInfoDraw_BaseBmp( &p_wk->win[FINFO_WIN_PAGE],
        p_data, p_draw, msg_finfo_page, 0, 0, WFNOTE_COL_WHITE);

  // トレーナーの名前
  if(sex == PM_MALE ){
    col = WFNOTE_COL_BLUE;
  }else if( sex == PM_FEMALE ){
    col = WFNOTE_COL_RED;
  }else{
    col = WFNOTE_COL_BLACK;
  }
  Draw_FriendNameSetWordset( p_draw, p_data, p_data->idx.fridx[p_data->select_listidx],heapID );
  FInfoDraw_BaseBmp( &p_wk->win[FINFO_WIN_NAME],
        p_data, p_draw, msg_finfo_name, 0, 0, col);

  //グループ
  FInfoDraw_BaseBmp( &p_wk->win[FINFO_WIN_GRP01], p_data, p_draw, msg_finfo_grp01, 0, 0, WFNOTE_COL_WHITE );

  Draw_FriendGroupSetWordset( p_draw, p_data,
      p_data->idx.fridx[p_data->select_listidx], heapID );
  FInfoDraw_BaseBmp( &p_wk->win[FINFO_WIN_GRP02], p_data, p_draw, msg_finfo_grp02, 0, 0, WFNOTE_COL_BLACK );

  // 最後に遊んだ日付
  {
    u32 result;

    FInfoDraw_BaseBmp( &p_wk->win[FINFO_WIN_DAY01], p_data, p_draw,
        msg_finfo_day01,0, 0, WFNOTE_COL_WHITE );

    result = Draw_FriendDaySetWordset( p_draw, p_data,
        p_data->idx.fridx[p_data->select_listidx] );
    if( result ){
      FInfoDraw_BaseBmp( &p_wk->win[FINFO_WIN_DAY02], p_data, p_draw,
        msg_finfo_day02, 0, 0, WFNOTE_COL_BLACK );
    }
  }
  //トレーナーグラフィック
  {
    TR_CLACT_GRA arcdata;
    void*   p_trgrabuff;
    GFL_BMP_DATA* tempBmp;
    u32 num,no;

    num = WifiList_GetFriendInfo( p_wifilist,
        p_data->idx.fridx[p_data->select_listidx], WIFILIST_FRIEND_UNION_GRA );

    no = UnionView_GetTrainerInfo(sex, num, UNIONVIEW_TRTYPE);

    TrCLACTGraDataGet( no, TR_PARA_FRONT, &arcdata);
    //p_trgrabuff = GFL_BMP_LoadCharacter( arcdata.arcID, arcdata.ncbrID, FALSE , heapID );

    p_trgrabuff = GFL_HEAP_AllocMemory( heapID, (BMPL_FINFO_TRGRA_SIZX*BMPL_FINFO_TRGRA_SIZY)*32 );
    ChangesInto_1D_from_2D( arcdata.arcID, arcdata.ncbrID, heapID,
        BMPL_FINFO_TRGRA_CUTX,
        BMPL_FINFO_TRGRA_CUTY,
        BMPL_FINFO_TRGRA_SIZX,
        BMPL_FINFO_TRGRA_SIZY, p_trgrabuff);
    tempBmp = GFL_BMP_CreateWithData( p_trgrabuff , BMPL_FINFO_TRGRA_SIZX,BMPL_FINFO_TRGRA_SIZY,GFL_BMP_16_COLOR,heapID );
#if 0
    MI_CpuFill8(p_trgrabuff,0x11,32*10*10);
#endif
    // ビットマップに書き込む
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(p_wk->win[FINFO_WIN_TRGRA]),1);
    GFL_BMP_Print(
        tempBmp,
        GFL_BMPWIN_GetBmp(p_wk->win[FINFO_WIN_TRGRA]),
        0, 0,
        0, 0,
        BMPL_FINFO_TRGRA_SIZX*8,
        BMPL_FINFO_TRGRA_SIZY*8 ,
        GF_BMPPRT_NOTNUKI);
    GFL_BMP_Delete( tempBmp );
    GFL_HEAP_FreeMemory( p_trgrabuff );

    // トレーナーパレット転送
    GFL_ARC_UTIL_TransVramPalette( arcdata.arcID, arcdata.nclrID, PALTYPE_SUB_BG,
        BMPL_FINFO_TRGRA_PAL*32, 32, heapID );

    GFL_BG_ChangeScreenPalette( BMPL_FINFO_FRM,
          BMPL_FINFO_TRGRA_X,BMPL_FINFO_TRGRA_Y,
          BMPL_FINFO_TRGRA_SIZX, BMPL_FINFO_TRGRA_SIZY, BGPLT_S_TRGRA );

  }

  for(i = 0; i < FINFO_WIN_NUM;i++){
    GFL_BMPWIN_MakeTransWindow_VBlank(p_wk->win[i]);
  }
*/
}

//----------------------------------------------------------------------------
/**
 *  @brief  上画面ベース情報の表示Off
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfo_DrawOffBaseInfo( WFNOTE_FRIENDINFO* p_wk )
{
/*
  int i;

  for(i = 0;i < FINFO_WIN_NUM;i++){
    GFL_BMPWIN_TransVramCharacter(p_wk->win[i]);
  }
*/
}

//----------------------------------------------------------------------------
/**
 *  @brief  ページの表示
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  drawarea  描画エリア
 *  @param  page    ページ
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfo_DrawPage( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, u32 drawarea, u32 page, HEAPID heapID )
{
  if( drawarea == WFNOTE_DRAWAREA_MAIN ){
    p_wk->page = page;  // 表示ページ更新
    // しおり更新
//    FInfo_ShioriDraw( p_wk, p_draw, page );
    FInfo_CursorDraw( p_wk, p_draw, page );
  }
  // ページ描画
  FInfoDrawArea_Page( &p_wk->drawarea[ drawarea ], p_data,
      p_draw, page, &p_wk->scrn[FINFO_SCRNDATA_M], p_wk->frontier_draw,heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  カーソル描画  ON
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 *  @param  page    ページ
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfo_CursorDraw( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw, u32 page )
{
  GFL_CLACTPOS pos;
  u8 anmno;
  if( p_wk->frontier_draw == TRUE ){
    anmno = ACTANM_CUR_REC;
    pos.x = page*32+16;
    pos.y = 22*8;
    GFL_CLACT_WK_SetPos(p_draw->p_cur,&pos,CLSYS_DEFREND_MAIN);
  }else{
    if(page == 2){
      anmno = ACTANM_CUR_REC;
      pos.x = 8*32+16;
      pos.y = 22*8;
      GFL_CLACT_WK_SetPos(p_draw->p_cur,&pos,CLSYS_DEFREND_MAIN);
    }else{
      anmno = ACTANM_CUR_LIST;
      pos.x = page*(14*8)+56;
      pos.y = 22*8;
      GFL_CLACT_WK_SetPos(p_draw->p_cur,&pos,CLSYS_DEFREND_MAIN);
    }
  }
  Draw_CursorActorSet( p_draw,TRUE, anmno );
}
static void FInfo_CursorOff( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw )
{
  Draw_CursorActorSet( p_draw,FALSE, ACTANM_NULL );
}

//----------------------------------------------------------------------------
/**
 *  @brief  しおり描画  ON
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 *  @param  page    ページ
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfo_ShioriDraw( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw, u32 page )
{
  u32 x;
  static const u8 siz_x[2][8] = {
    {14,14,4,0,0,0,0,0},
    {4,4,4,4,4,4,4,4},
  };
  static const u8 pos_x[2][8] = {
    {0,14,28,0,0,0,0,0},
    {0,4,8,12,16,20,24,28},
  };
  static const u8 pos_y[2] = {FINFO_SHIORISCRN_POS_Y+8,FINFO_SHIORISCRN_POS_Y};

  // 大本をまず描画
  // フロンティアOFFなら大本しおりを変更
  if( p_wk->frontier_draw == TRUE ){
    GFL_BG_WriteScreenExpand( DFRM_SCROLL,
        FINFO_SHIORI_X, FINFO_SHIORI_Y,
        FINFO_SHIORI_SIZX, FINFO_SHIORI_SIZY,
        p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
        FINFO_SHIORISCRN_X, FINFO_SHIORISCRN_Y,
        p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
        p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );
  }else{
    GFL_BG_WriteScreenExpand( DFRM_SCROLL,
        FINFO_SHIORI_X, FINFO_SHIORI_Y,
        FINFO_SHIORI_SIZX, FINFO_SHIORI_SIZY,
        p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
        FINFO_SHIORISCRN_X, FINFO_SHIORISCRN_Y+8,
        p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
        p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );
  }
  if(page == 0xFF){
    // BG面転送
    GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
    return;
  }
  // 今のページのしおりを描画
  x = FINFO_SHIORI_X + (FINFO_SHIORI_ONEX*page) + FINFO_SHIORI_OFSX;
  GFL_BG_WriteScreenExpand( DFRM_SCROLL,
      pos_x[p_wk->frontier_draw][page], FINFO_SHIORI_Y,
      siz_x[p_wk->frontier_draw][page], FINFO_EFF_SHIORI_POS_SIZ,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
      pos_x[p_wk->frontier_draw][page], pos_y[p_wk->frontier_draw],
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );


  // BG面転送
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
}

//----------------------------------------------------------------------------
/**
 *  @brief  しおりOFF
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 */
//-----------------------------------------------------------------------------
static void FInfo_ShioriOff( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw )
{
  // しおりぶぶん
  GFL_BG_WriteScreenExpand( DFRM_SCROLL,
      FINFO_SHIORI_X, FINFO_SHIORI_Y,
      FINFO_SHIORI_SIZX, FINFO_SHIORI_SIZY,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->rawData,
      FINFO_CL_SHIORISCRN_X, FINFO_CL_SHIORISCRN_Y,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenWidth/8,
      p_draw->scrn.p_scrn[WFNOTE_SCRNDATA_KIT]->screenHeight/8 );
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
}


//----------------------------------------------------------------------------
/**
 *  @brief  しおりアニメ００
 *
 *  @param  p_wk  ワーク
 *  @param  p_draw  描画システム
 *  @param  page00  今のページ
 *  @param  page01  前のページ
 */
//-----------------------------------------------------------------------------
static void FInfo_ShioriAnm00( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw, u32 page00, u32 page01 )
{
  int x;

//  FInfo_ShioriDraw( p_wk, p_draw, page01 );
  FInfo_CursorDraw( p_wk, p_draw, page01 );
  // BG面転送
  GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
}

//----------------------------------------------------------------------------
/**
 *  @brief  しおりアニメ０１
 *
 *  @param  p_wk  ワーク
 *  @param  p_draw  描画システム
 *  @param  page00  今のページ
 *  @param  page01  前のページ
 */
//-----------------------------------------------------------------------------
static void FInfo_ShioriAnm01( WFNOTE_FRIENDINFO* p_wk,  WFNOTE_DRAW* p_draw, u32 page00, u32 page01 )
{
  int x;

  FInfo_ShioriDraw( p_wk, p_draw, 0xFF );
//  FInfo_CursorDraw( p_wk, p_draw, page00 );
}

//----------------------------------------------------------------------------
/**
 *  @brief  選択リストインデックス値　変更
 *
 *  @param  p_data  データ
 *  @param  way   方向    上かした
 *
 *  @retval TRUE  変更した
 *  @retval FALSE 変更できない
 */
//-----------------------------------------------------------------------------
static BOOL FInfo_SelectListIdxAdd( WFNOTE_DATA* p_data, WF_COMMON_WAY way )
{
  int i;
  u32 sex;
  s32 select_listidx;
  WIFI_LIST* p_wifilist;

  p_wifilist = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );
  select_listidx = p_data->select_listidx;

  if( way == WF_COMMON_TOP ){
    for( i=0; i<p_data->idx.friendnum-1; i++ ){
      select_listidx --;
      if( select_listidx < 0 ){
        select_listidx += p_data->idx.friendnum;
      }

      // その人の性別をチェック
      sex = WifiList_GetFriendInfo( p_wifilist, p_data->idx.fridx[select_listidx], WIFILIST_FRIEND_SEX );
      if( sex != PM_NEUTRAL ){
        p_data->select_listidx = select_listidx;
        return TRUE;
      }
    }
  }else{
    for( i=0; i<p_data->idx.friendnum-1; i++ ){
      select_listidx = (select_listidx + 1) % p_data->idx.friendnum;

      // その人の性別をチェック
      sex = WifiList_GetFriendInfo( p_wifilist, p_data->idx.fridx[select_listidx], WIFILIST_FRIEND_SEX );
      if( sex != PM_NEUTRAL ){
        p_data->select_listidx = select_listidx;
        return TRUE;
      }
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  スクリーンデータ初期化
 *
 *  @param  p_scrn    スクリーンデータ格納先
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfoScrnData_Init( WFNOTE_FINFO_SCRNDATA* p_scrn, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  int i;
  u16 id_tbl[] = {
    NARC_wifi_note_friend_recbase_d_NSCR,
    NARC_wifi_note_friend_record_d_NSCR,
    NARC_wifi_note_friend_back_d_NSCR,
    NARC_wifi_note_friend_record_u_NSCR,
  };

  for(i = 0;i < FINFO_SCRNDATA_NUM;i++){
    p_scrn->p_scrnbuff = GFL_ARCHDL_UTIL_LoadScreen(
        p_draw->p_handle, id_tbl[i], FALSE,
        &p_scrn->p_scrn, heapID );
    p_scrn++;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  スクリーンデータ破棄
 *
 *  @param  p_scrn    スクリーンデータ
 */
//-----------------------------------------------------------------------------
static void FInfoScrnData_Exit( WFNOTE_FINFO_SCRNDATA* p_scrn )
{
  int i;

  for(i = 0;i < FINFO_SCRNDATA_NUM;i++){
    GFL_HEAP_FreeMemory( p_scrn->p_scrnbuff );
    p_scrn++;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  Bmpウィンドウ初期化
 *
 *  @param  p_scrn    スクリーンデータ格納先
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
///ビットマップリスト用データ構造体
/*
typedef struct{
  u8  frm_num;  ///<ウインドウ使用フレーム
  u8  pos_x;    ///<ウインドウ領域の左上のX座標（キャラ単位で指定）
  u8  pos_y;    ///<ウインドウ領域の左上のY座標（キャラ単位で指定）
  u8  siz_x;    ///<ウインドウ領域のXサイズ（キャラ単位で指定）
  u8  siz_y;    ///<ウインドウ領域のYサイズ（キャラ単位で指定）
  u8  palnum;   ///<ウインドウ領域のパレットナンバー
  u16 chrnum;   ///<ウインドウキャラ領域の開始キャラクタナンバー
}WNOTE_BMPWIN_DAT;
*/
static void FInfoBmpWin_Init( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DRAW* p_draw )
{
/*
  int i;
  static const WNOTE_BMPWIN_DAT BmpWinData[] = {
   {  // ページ
    BMPL_FINFO_FRM, BMPL_FINFO_PAGE_X, BMPL_FINFO_PAGE_Y,
    BMPL_FINFO_PAGE_SIZX, BMPL_FINFO_PAGE_SIZY, BMPL_FINFO_PAL, BMPL_FINFO_PAGE_CGX
   },
   {  // タイトル
    BMPL_FINFO_FRM, BMPL_FINFO_TITLE_X, BMPL_FINFO_TITLE_Y,
    BMPL_FINFO_TITLE_SIZX, BMPL_FINFO_TITLE_SIZY, BMPL_FINFO_PAL, BMPL_FINFO_TITLE_CGX
   },
   {  // ネーム
    BMPL_FINFO_FRM, BMPL_FINFO_NAME_X, BMPL_FINFO_NAME_Y,
    BMPL_FINFO_NAME_SIZX, BMPL_FINFO_NAME_SIZY, BMPL_FINFO_PAL, BMPL_FINFO_NAME_CGX
   },
   {  // グループ
    BMPL_FINFO_FRM, BMPL_FINFO_GRP02_X, BMPL_FINFO_GRP02_Y,
    BMPL_FINFO_GRP02_SIZX, BMPL_FINFO_GRP02_SIZY, BMPL_FINFO_PAL, BMPL_FINFO_GRP02_CGX
   },
   {  // 日付
    BMPL_FINFO_FRM, BMPL_FINFO_DAY02_X, BMPL_FINFO_DAY02_Y,
    BMPL_FINFO_DAY02_SIZX, BMPL_FINFO_DAY02_SIZY, BMPL_FINFO_PAL, BMPL_FINFO_DAY02_CGX
   },
   {  // トレーナーグラフィック
    BMPL_FINFO_FRM, BMPL_FINFO_TRGRA_X, BMPL_FINFO_TRGRA_Y,
    BMPL_FINFO_TRGRA_SIZX, BMPL_FINFO_TRGRA_SIZY, BMPL_FINFO_TRGRA_PAL, BMPL_FINFO_TRGRA_CGX
   },
   {  // グループ
    BMPL_FINFO_FRM, BMPL_FINFO_GRP01_X, BMPL_FINFO_GRP01_Y,
    BMPL_FINFO_GRP01_SIZX, BMPL_FINFO_GRP01_SIZY, BMPL_FINFO_PAL, BMPL_FINFO_GRP01_CGX
   },
   {  // 日付
    BMPL_FINFO_FRM, BMPL_FINFO_DAY01_X, BMPL_FINFO_DAY01_Y,
    BMPL_FINFO_DAY01_SIZX, BMPL_FINFO_DAY01_SIZY, BMPL_FINFO_PAL, BMPL_FINFO_DAY01_CGX
   },
  };

  for( i=0; i<FINFO_WIN_NUM; i++ ){
    Sub_BmpWinAdd( &p_wk->win[i],   BmpWinData[i].frm_num ,
          BmpWinData[i].pos_x , BmpWinData[i].pos_y ,
          BmpWinData[i].siz_x , BmpWinData[i].siz_y ,
          BmpWinData[i].palnum , BmpWinData[i].chrnum ,
          0 );
  }
*/
}

//----------------------------------------------------------------------------
/**
 *  @brief  Bmpウィンドウ解放
 *
 *  @param  p_scrn    スクリーンデータ格納先
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfoBmpWin_Exit( WFNOTE_FRIENDINFO* p_wk, WFNOTE_DRAW* p_draw )
{
/*
  int i;
  for( i=0; i<FINFO_WIN_NUM; i++ ){
    GFL_BMPWIN_ClearTransWindow( p_wk->win[i] );
//    GF_BGL_BmpWinOff( &p_wk->win[i] );
    GFL_BMPWIN_Delete( p_wk->win[i] );
  }
*/
}

//----------------------------------------------------------------------------
/**
 *  @brief  表示エリアワーク  初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 *  @param  cp_scrnarea 表示エリアデータ
 *  @param  cgx     メッセージ面のCGX
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfoDrawArea_Init( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, const WFNOTE_SCRNAREA* cp_scrnarea, u32 cgx, HEAPID heapID )
{
  int i;
  p_wk->scrnarea = *cp_scrnarea;
  p_wk->cgx = cgx;

  for( i=0; i<FINFO_PAGE_NUM; i++ ){
//    p_wk->p_msg[i] = NULL;
    p_wk->msgnum[i] = 0;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画エリア　破棄
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FInfoDrawArea_Exit( WFNOTE_FINFO_DRAWAREA* p_wk )
{
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージビットマップ  初期化
 *
 *  @param  p_wk    ワーク
 *  @param  p_draw    描画システム
 *  @param  idx     インデックス
 *  @param  num     個数
 *  @param  cp_data   構成データ
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfoDrawArea_MsgBmpInit( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw, u32 idx, u32 num, const WFNOTE_FINFO_BMPDATA* cp_data, HEAPID heapID )
{
  int i;
  u16 cgx;

//  p_wk->p_msg[idx] = GF_BGL_BmpWinAllocGet( heapID, num );
//  p_wk->p_msg[idx] = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_BMPWIN*)*num );
  p_wk->msgnum[idx] = num;

  // 作成
/*
  cgx = p_wk->cgx;
  for( i=0; i<num; i++ ){
    Sub_BmpWinAdd( &p_wk->p_msg[idx][i], DFRM_SCRMSG,
        MATH_ABS(p_wk->scrnarea.scrn_x) + cp_data[i].pos_x,
        MATH_ABS(p_wk->scrnarea.scrn_y) + cp_data[i].pos_y,
        cp_data[i].siz_x, cp_data[i].siz_y,
        BGPLT_M_MSGFONT, cgx , 0 );
    cgx += ( cp_data[i].siz_x * cp_data[i].siz_y );
  }
*/
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージビットマップ破棄
 *
 *  @param  p_wk  ワーク
 *  @param  idx   インデックス
 */
//-----------------------------------------------------------------------------
static void FInfoDrawArea_MsgBmpExit( WFNOTE_FINFO_DRAWAREA* p_wk, u32 idx )
{
  u8 i;
  if( p_wk->p_msg[idx] )
  {
    for( i=0;i<p_wk->msgnum[idx];i++ )
    {
//      GFL_BMPWIN_Delete( p_wk->p_msg[idx][i] );
    }
//    GFL_HEAP_FreeMemory( *p_wk->p_msg[idx] );
    p_wk->p_msg[idx] = NULL;
    p_wk->msgnum[idx] = 0;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  更新
 *
 *  @param  p_wk  ワーク
 *  @param  idx   インデックス
 */
//-----------------------------------------------------------------------------
static void FInfoDrawArea_MsgBmpOnVReq( WFNOTE_FINFO_DRAWAREA* p_wk, u32 idx )
{
  int i;
  for( i=0; i<p_wk->msgnum[idx]; i++ ){
    GFL_BMPWIN_MakeTransWindow_VBlank( *p_wk->p_msg[idx] );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  表示OFF
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FInfoDrawArea_MsgBmpOffVReq( WFNOTE_FINFO_DRAWAREA* p_wk, u32 idx )
{
  int i;

  for( i=0; i<p_wk->msgnum[idx]; i++ ){
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(*p_wk->p_msg[idx]),0);
    GFL_BMPWIN_ClearTransWindow_VBlank( *p_wk->p_msg[idx] );
//    GF_BGL_BmpWinOffVReq( &p_wk->p_msg[idx][i] );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  描画アリアにページの書き込み
 *
 *  @param  p_wk    ワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画ワーク
 *  @param  page    ページ
 *  @param  cp_scrn   スクリーン描画データ
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfoDrawArea_Page( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data,
  WFNOTE_DRAW* p_draw, u32 page, const WFNOTE_FINFO_SCRNDATA* cp_scrn, BOOL bf_on,HEAPID heapID )
{
  static void (*pWriteBfOn[ FINFO_PAGE_NUM ])( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID ) = {
    FInfoDraw_Page00,
    FInfoDraw_Page06,
    FInfoDraw_Page01,
    FInfoDraw_Page02,
    FInfoDraw_Page03,
    FInfoDraw_Page04,
    FInfoDraw_Page05,
    FInfoDraw_Page07,
  };
  static void (*pWriteBfOff[ FINFO_BFOFF_PAGE_NUM ])( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID ) = {
    FInfoDraw_Page00,
    FInfoDraw_Page06,
    FInfoDraw_Page07,
  };

  // 今までの表示クリア
  FInfoDrawArea_PageOff( p_wk, p_draw );

  // ページ書き込み
  if(bf_on){
    pWriteBfOn[page]( p_wk, p_data, p_draw, heapID );
  }else{
    pWriteBfOff[page]( p_wk, p_data, p_draw, heapID );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ぺーじOFF
 *
 *  @param  p_wk  ワーク
 *  @param  p_draw  描画システム
 */
//-----------------------------------------------------------------------------
static void FInfoDrawArea_PageOff( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DRAW* p_draw )
{
  int i;
  for( i=0; i<FINFO_PAGE_NUM; i++ ){
    FInfoDrawArea_MsgBmpOffVReq( p_wk, i );
  }
  FInfoDraw_Clean( p_draw, &p_wk->scrnarea );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ページの書き込み
 *
 *  @param  p_wk    描画エリアワーク
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FInfoDraw_Page00( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  WIFI_LIST* p_wifilist;
  u32 i;
  u8 ofsx = 0;
  u8 ofsy = 0;

  p_wifilist = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );

  // 対戦成績タイトル
  FInfoDraw_Bmp( p_wk, 0, FINFO_PAGE00_BA_BTT,
      p_data, p_draw, msg_finfo_p00_btt,
      FINFO_PAGE00_BTT_X, FINFO_PAGE00_BTT_Y,
      WFNOTE_COL_BLACK, p_draw->p_str, p_draw->p_tmp );

  // かち & 負け
  for(i = 0;i < 2;i++){
    ofsx = (FINFO_PAGE00_BA_BT_X-FINFO_PAGE00_BA_CAP_X) * 8;
    ofsy = FINFO_PAGE00_BT_Y+i*FINFO_PAGE00_BT_OFSY
        + (FINFO_PAGE00_BA_BT_Y-FINFO_PAGE00_BA_CAP_BTT_Y)*8;

    FInfoDraw_Bmp( p_wk, 0, FINFO_PAGE00_BA_BT,
        p_data, p_draw, msg_finfo_p00_bt01+i,
        FINFO_PAGE00_BT_X+ofsx, ofsy,
        WFNOTE_COL_BLACK, p_draw->p_str, p_draw->p_tmp );

    Draw_NumberSetWordset( p_draw,
      WifiList_GetFriendInfo( p_wifilist, p_data->idx.fridx[p_data->select_listidx],
                  WIFILIST_FRIEND_BATTLE_WIN+i));

    ofsx = (FINFO_PAGE00_BA_TR_X-FINFO_PAGE00_BA_CAP_X)*8;
    FInfoDraw_Bmp( p_wk, 0, FINFO_PAGE00_BA_BT,
        p_data, p_draw, msg_finfo_p00_btnum,
        ofsx,ofsy,
        WFNOTE_COL_BLACK, p_draw->p_str, p_draw->p_tmp );
  }

  // ポケモン交換タイトル
  ofsx = (FINFO_PAGE00_BA_CAP_X-FINFO_PAGE00_BA_CAP_X)*8;
  ofsy = (FINFO_PAGE00_BA_CAP_TRT_Y-FINFO_PAGE00_BA_CAP_BTT_Y)*8;
  FInfoDraw_Bmp( p_wk, 0, FINFO_PAGE00_BA_TRT,
      p_data, p_draw, msg_finfo_p00_trt,
      FINFO_PAGE00_TRT_X+ofsx, FINFO_PAGE00_TRT_Y+ofsy,
      WFNOTE_COL_BLACK, p_draw->p_str, p_draw->p_tmp );

  Draw_NumberSetWordset( p_draw,
    WifiList_GetFriendInfo( p_wifilist, p_data->idx.fridx[p_data->select_listidx],
                WIFILIST_FRIEND_TRADE_NUM));

  ofsx = (FINFO_PAGE00_BA_TR_X-FINFO_PAGE00_BA_CAP_X)*8;
  ofsy = (FINFO_PAGE00_BA_TR_Y-FINFO_PAGE00_BA_CAP_BTT_Y)*8;
  FInfoDraw_Bmp( p_wk, 0, FINFO_PAGE00_BA_TR,
      p_data, p_draw, msg_finfo_p00_trnum,
      FINFO_PAGE00_TRN_X+ofsx, FINFO_PAGE00_TRN_Y+ofsy,
      WFNOTE_COL_BLACK, p_draw->p_str, p_draw->p_tmp );

  FInfoDrawArea_MsgBmpOnVReq( p_wk, 0 );
}

static void FInfoDraw_Page01( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  u32 num;
  u8 ofsx,ofsy;

  // タイトル
  ofsx = (FINFO_PAGE01_BA_CAP_X-FINFO_PAGE_X)*8;
  ofsy = (FINFO_PAGE01_BA_CAP_Y-FINFO_PAGE_Y)*8;
  FInfoDraw_Bmp( p_wk, 2, FINFO_PAGE01_BA_CAP,
      p_data, p_draw, msg_wifi_note_36,
      FINFO_PAGE01_BTT_X+ofsx, FINFO_PAGE01_BTT_Y+ofsy,
      WFNOTE_COL_BLACK, p_draw->p_str, p_draw->p_tmp );

  // 前回(げんざい)
  ofsx = (FINFO_PAGE01_BA_DAT_X-FINFO_PAGE_X)*8;
  ofsy = (FINFO_PAGE01_BA_DAT_Y-FINFO_PAGE_Y)*8;
  {
    BOOL clear_flag = TRUE;


    FInfoDraw_Bmp( p_wk, 2, FINFO_PAGE01_BA_DAT,
        p_data, p_draw, msg_finfo_frprm_win01+clear_flag,
        FINFO_PAGE01_DAT_X+ofsx, FINFO_PAGE01_DAT_Y01+ofsy,
        WFNOTE_COL_BLACK, p_draw->p_str, p_draw->p_tmp );
    num = 123;
    Draw_NumberSetWordset( p_draw, num );
    FInfoDraw_Bmp( p_wk, 2, FINFO_PAGE01_BA_DAT,
        p_data, p_draw, msg_finfo_frprm_wnum,
        FINFO_PAGE01_DATN_X+ofsx, FINFO_PAGE01_DAT_Y01+ofsy,
        WFNOTE_COL_BLACK, p_draw->p_str, p_draw->p_tmp );
  }

  //  最高
  FInfoDraw_Bmp( p_wk, 2, FINFO_PAGE01_BA_DAT,
      p_data, p_draw, msg_finfo_frprm_win03,
      FINFO_PAGE01_DAT_X+ofsx, FINFO_PAGE01_DAT_Y02+ofsy,
      WFNOTE_COL_BLACK, p_draw->p_str, p_draw->p_tmp );
  num = 456;
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 2, FINFO_PAGE01_BA_DAT,
      p_data, p_draw, msg_finfo_frprm_wnum,
      FINFO_PAGE01_DATN_X+ofsx, FINFO_PAGE01_DAT_Y02+ofsy,
      WFNOTE_COL_BLACK, p_draw->p_str, p_draw->p_tmp );

  FInfoDrawArea_MsgBmpOnVReq( p_wk, 2 );
}

static void FInfoDraw_Page02( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  STRBUF* p_str;
  STRBUF* p_tmp;
  u32 num;
  BOOL clear_flag;
  u32 stridx;
  int ofsx,ofsy;
  p_str = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  p_tmp = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );

  // タイトル
  ofsx = (FINFO_PAGE02_BA_TT_X-FINFO_PAGE_X)*8;
  ofsy = (FINFO_PAGE02_BA_TT_Y-FINFO_PAGE_Y)*8;
  FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_BA_TT,
      p_data, p_draw, msg_wifi_note_37,
      FINFO_PAGE02_TT_X+ofsx, FINFO_PAGE02_TT_Y+ofsy,
      WFNOTE_COL_BLACK, p_str, p_tmp );

  // LV50
  {
    // タイトル
    ofsx = (FINFO_PAGE02_BA_LV50TT_X-FINFO_PAGE_X)*8;
    ofsy = (FINFO_PAGE02_BA_LV50TT_Y-FINFO_PAGE_Y)*8;
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV50TT,
        p_data, p_draw, msg_wifi_note_43,
        FINFO_PAGE02_LV50TT_X+ofsx, FINFO_PAGE02_LV50TT_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV50TT,
        p_data, p_draw, msg_wifi_note_45,
        FINFO_PAGE02_LV50TT2_X+ofsx, FINFO_PAGE02_LV50TT_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );

    //前回
    clear_flag = TRUE;
    if( clear_flag == FALSE ){
      stridx = msg_finfo_frprm_win01;
    }else{
      stridx = msg_finfo_frprm_win02;
    }
    ofsx = (FINFO_PAGE02_BA_LV50NO_X-FINFO_PAGE_X)*8;
    ofsy = (FINFO_PAGE02_BA_LV50NO_Y-FINFO_PAGE_Y)*8;
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV50NO,
        p_data, p_draw, stridx,
        FINFO_PAGE02_LV50LT_X+ofsx, FINFO_PAGE02_LV50L_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );

    // 連勝
    num = 789;
    Draw_NumberSetWordset( p_draw, num );
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV50NO,
        p_data, p_draw, msg_finfo_frprm_wnum,
        FINFO_PAGE02_LV50LN0_X+ofsx, FINFO_PAGE02_LV50L_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );
    // 交換
    num = 147;
    Draw_NumberSetWordset( p_draw, num );
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV50NO,
        p_data, p_draw, msg_wifi_note_49,
        FINFO_PAGE02_LV50LN1_X+ofsx, FINFO_PAGE02_LV50L_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );


    //最高
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV50NO,
        p_data, p_draw, msg_finfo_frprm_win03,
        FINFO_PAGE02_LV50MT_X+ofsx, FINFO_PAGE02_LV50M_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );

    // 連勝
    num = 258;
    Draw_NumberSetWordset( p_draw, num );
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV50NO,
        p_data, p_draw, msg_finfo_frprm_wnum,
        FINFO_PAGE02_LV50MN0_X+ofsx, FINFO_PAGE02_LV50M_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );
    // 交換
    num = 369;
    Draw_NumberSetWordset( p_draw, num );
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV50NO,
        p_data, p_draw, msg_wifi_note_49,
        FINFO_PAGE02_LV50MN1_X+ofsx, FINFO_PAGE02_LV50M_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );
  }

  // LV100
  {
    // タイトル
    ofsx = (FINFO_PAGE02_BA_LV100TT_X-FINFO_PAGE_X)*8;
    ofsy = (FINFO_PAGE02_BA_LV100TT_Y-FINFO_PAGE_Y)*8;
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV100TT,
        p_data, p_draw, msg_wifi_note_44,
        FINFO_PAGE02_LV100TT_X+ofsx, FINFO_PAGE02_LV100TT_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV100TT,
        p_data, p_draw, msg_wifi_note_45,
        FINFO_PAGE02_LV100TT2_X+ofsx, FINFO_PAGE02_LV100TT_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );

    //前回
    clear_flag = TRUE;
    if( clear_flag == FALSE ){
      stridx = msg_finfo_frprm_win01;
    }else{
      stridx = msg_finfo_frprm_win02;
    }
    ofsx = (FINFO_PAGE02_BA_LV100NO_X-FINFO_PAGE_X)*8;
    ofsy = (FINFO_PAGE02_BA_LV100NO_Y-FINFO_PAGE_Y)*8;
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV100NO,
        p_data, p_draw, stridx,
        FINFO_PAGE02_LV100LT_X+ofsx, FINFO_PAGE02_LV100L_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );

    // 連勝
    num = 123;
    Draw_NumberSetWordset( p_draw, num );
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV100NO,
        p_data, p_draw, msg_finfo_frprm_wnum,
        FINFO_PAGE02_LV100LN0_X+ofsx, FINFO_PAGE02_LV100L_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );
    // 交換
    num = 456;
    Draw_NumberSetWordset( p_draw, num );
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV100NO,
        p_data, p_draw, msg_wifi_note_49,
        FINFO_PAGE02_LV100LN1_X+ofsx, FINFO_PAGE02_LV100L_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );


    //最高
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV100NO,
        p_data, p_draw, msg_finfo_frprm_win03,
        FINFO_PAGE02_LV100MT_X+ofsx, FINFO_PAGE02_LV100M_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );

    // 連勝
    num = 789;
    Draw_NumberSetWordset( p_draw, num );
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV100NO,
        p_data, p_draw, msg_finfo_frprm_wnum,
        FINFO_PAGE02_LV100MN0_X+ofsx, FINFO_PAGE02_LV100M_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );
    // 交換
    num = 255;
    Draw_NumberSetWordset( p_draw, num );
    FInfoDraw_Bmp( p_wk, 3, FINFO_PAGE02_LV100NO,
        p_data, p_draw, msg_wifi_note_49,
        FINFO_PAGE02_LV100MN1_X+ofsx, FINFO_PAGE02_LV100M_Y+ofsy,
        WFNOTE_COL_BLACK, p_str, p_tmp );
  }


  FInfoDrawArea_MsgBmpOnVReq( p_wk, 3 );

  GFL_STR_DeleteBuffer( p_str );
  GFL_STR_DeleteBuffer( p_tmp );
}
static void FInfoDraw_Page03( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  STRBUF* p_str;
  STRBUF* p_tmp;
  u32 num;
  BOOL clear_flag;
  u32 stridx;
#if USE_FRONTIER_DATA
  FRONTIER_SAVEWORK* p_fsave;

  p_fsave = SaveData_GetFrontier( p_data->p_save );
#endif
  p_str = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  p_tmp = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );


  // タイトル
  FInfoDraw_Bmp( p_wk, 4, FINFO_PAGE03_BA,
      p_data, p_draw, msg_wifi_note_38,
      FINFO_PAGE03_TT_X, FINFO_PAGE03_TT_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  FInfoDraw_Bmp( p_wk, 4, FINFO_PAGE03_BA,
      p_data, p_draw, msg_wifi_note_46,
      FINFO_PAGE03_CPT_X, FINFO_PAGE03_CPT_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );

  // 前回
#if USE_FRONTIER_DATA
  clear_flag = FrontierRecord_Get(p_fsave, FRID_CASTLE_MULTI_WIFI_CLEAR_BIT, p_data->idx.fridx[p_data->select_listidx]);
#else
  clear_flag = TRUE;
#endif
  if( clear_flag == FALSE ){
    stridx = msg_finfo_frprm_win01;
  }else{
    stridx = msg_finfo_frprm_win02;
  }
  FInfoDraw_Bmp( p_wk, 4, FINFO_PAGE03_BA,
      p_data, p_draw, stridx,
      FINFO_PAGE03_CPLT_X, FINFO_PAGE03_CPL_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  // くりあ
#if USE_FRONTIER_DATA
  num = FrontierRecord_Get(p_fsave, FRID_CASTLE_MULTI_WIFI_RENSHOU_CNT, p_data->idx.fridx[p_data->select_listidx]);
#else
  num = 147;
#endif
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 4, FINFO_PAGE03_BA,
      p_data, p_draw, msg_wifi_note_51,
      FINFO_PAGE03_CPLN0_X, FINFO_PAGE03_CPL_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  // CP
#if USE_FRONTIER_DATA
  num = FrontierRecord_Get(p_fsave, FRID_CASTLE_MULTI_WIFI_CP, p_data->idx.fridx[p_data->select_listidx]);
#else
  num = 258;
#endif
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 4, FINFO_PAGE03_BA,
      p_data, p_draw, msg_wifi_note_50,
      FINFO_PAGE03_CPLN1_X, FINFO_PAGE03_CPL_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );


  // 最高
  FInfoDraw_Bmp( p_wk, 4, FINFO_PAGE03_BA,
      p_data, p_draw, msg_finfo_frprm_win03,
      FINFO_PAGE03_CPMT_X, FINFO_PAGE03_CPM_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  // くりあ
#if USE_FRONTIER_DATA
  num = FrontierRecord_Get(p_fsave, FRID_CASTLE_MULTI_WIFI_RENSHOU, p_data->idx.fridx[p_data->select_listidx]);
#else
  num = 369;
#endif
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 4, FINFO_PAGE03_BA,
      p_data, p_draw, msg_wifi_note_51,
      FINFO_PAGE03_CPMN0_X, FINFO_PAGE03_CPM_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  // CP
#if USE_FRONTIER_DATA
  num = FrontierRecord_Get(p_fsave, FRID_CASTLE_MULTI_WIFI_REMAINDER_CP, p_data->idx.fridx[p_data->select_listidx]);
#else
  num = 123;
#endif
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 4, FINFO_PAGE03_BA,
      p_data, p_draw, msg_wifi_note_50,
      FINFO_PAGE03_CPMN1_X, FINFO_PAGE03_CPM_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );


  FInfoDrawArea_MsgBmpOnVReq( p_wk, 4 );


  GFL_STR_DeleteBuffer( p_str );
  GFL_STR_DeleteBuffer( p_tmp );
}
static void FInfoDraw_Page04( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  STRBUF* p_str;
  STRBUF* p_tmp;
  STRBUF* p_monsstr;
  u32 num;
  BOOL clear_flag;
  u32 stridx;
#if USE_FRONTIER_DATA
  FRONTIER_SAVEWORK* p_fsave;

  p_fsave = SaveData_GetFrontier( p_data->p_save );
#endif

  p_str = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  p_tmp = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );


  // タイトル
  FInfoDraw_Bmp( p_wk, 5, FINFO_PAGE04_BA,
      p_data, p_draw, msg_wifi_note_39,
      FINFO_PAGE04_TT_X, FINFO_PAGE04_TT_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );

#if USE_FRONTIER_DATA
  num = FrontierRecord_Get(p_fsave, FRID_STAGE_MULTI_WIFI_MONSNO, p_data->idx.fridx[p_data->select_listidx]);
#else
  num = 123;
#endif
  p_monsstr = GFL_MSG_CreateString( GlobalMsg_PokeName , num );
  PRINTSYS_PrintQueColor( p_draw->printQue , GFL_BMPWIN_GetBmp(p_wk->p_msg[5][FINFO_PAGE04_BA]),
      FINFO_PAGE04_TT_X, FINFO_PAGE04_TT2_Y,
      p_monsstr, p_draw->fontHandle , WFNOTE_COL_BLACK );
  GFL_STR_DeleteBuffer( p_monsstr );

  // 前回
#if USE_FRONTIER_DATA
  clear_flag = FrontierRecord_Get(p_fsave, FRID_STAGE_MULTI_WIFI_CLEAR_BIT, p_data->idx.fridx[p_data->select_listidx]);
#else
  clear_flag = TRUE;
#endif
  if( clear_flag == FALSE ){
    stridx = msg_finfo_frprm_win01;
  }else{
    stridx = msg_finfo_frprm_win02;
  }
  FInfoDraw_Bmp( p_wk, 5, FINFO_PAGE04_BA,
      p_data, p_draw, stridx,
      FINFO_PAGE04_LT_X, FINFO_PAGE04_L_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  // 値
#if USE_FRONTIER_DATA
  num = FrontierRecord_Get(p_fsave, FRID_STAGE_MULTI_WIFI_RENSHOU_CNT, p_data->idx.fridx[p_data->select_listidx]);
#else
  num = 789;
#endif
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 5, FINFO_PAGE04_BA,
      p_data, p_draw, msg_finfo_frprm_wnum,
      FINFO_PAGE04_LN_X, FINFO_PAGE04_L_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );

  // 最高
  FInfoDraw_Bmp( p_wk, 5, FINFO_PAGE04_BA,
      p_data, p_draw, msg_finfo_frprm_win03,
      FINFO_PAGE04_MT_X, FINFO_PAGE04_M_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  // 値
#if USE_FRONTIER_DATA
  num = FrontierRecord_Get(p_fsave, FRID_STAGE_MULTI_WIFI_RENSHOU, p_data->idx.fridx[p_data->select_listidx]);
#else
  num = 147;
#endif
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 5, FINFO_PAGE04_BA,
      p_data, p_draw, msg_finfo_frprm_wnum,
      FINFO_PAGE04_MN_X, FINFO_PAGE04_M_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );

  FInfoDrawArea_MsgBmpOnVReq( p_wk, 5 );

  GFL_STR_DeleteBuffer( p_str );
  GFL_STR_DeleteBuffer( p_tmp );
}
static void FInfoDraw_Page05( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  STRBUF* p_str;
  STRBUF* p_tmp;
  u32 num;
  BOOL clear_flag;
  u32 stridx;
#if USE_FRONTIER_DATA
  FRONTIER_SAVEWORK* p_fsave;

  p_fsave = SaveData_GetFrontier( p_data->p_save );
#endif

  p_str = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  p_tmp = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );

  // タイトル
  FInfoDraw_Bmp( p_wk, 6, FINFO_PAGE05_BA,
      p_data, p_draw, msg_wifi_note_40,
      FINFO_PAGE05_TT_X, FINFO_PAGE05_TT_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );

  // 前回
#if USE_FRONTIER_DATA
  clear_flag = FrontierRecord_Get(p_fsave, FRID_ROULETTE_MULTI_WIFI_CLEAR_BIT, p_data->idx.fridx[p_data->select_listidx]);
#else
  clear_flag = TRUE;
#endif
  if( clear_flag == FALSE ){
    stridx = msg_finfo_frprm_win01;
  }else{
    stridx = msg_finfo_frprm_win02;
  }
  FInfoDraw_Bmp( p_wk, 6, FINFO_PAGE05_BA,
      p_data, p_draw, stridx,
      FINFO_PAGE05_LT_X, FINFO_PAGE05_L_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  // 値
#if USE_FRONTIER_DATA
  num = FrontierRecord_Get(p_fsave, FRID_ROULETTE_MULTI_WIFI_RENSHOU_CNT, p_data->idx.fridx[p_data->select_listidx]);
#else
  num = 258;
#endif
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 6, FINFO_PAGE05_BA,
      p_data, p_draw, msg_finfo_frprm_win03,
      FINFO_PAGE05_LN_X, FINFO_PAGE05_L_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );

  // 最高
  FInfoDraw_Bmp( p_wk, 6, FINFO_PAGE05_BA,
      p_data, p_draw, msg_finfo_frprm_win03,
      FINFO_PAGE05_MT_X, FINFO_PAGE05_M_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  // 値
#if USE_FRONTIER_DATA
  num = FrontierRecord_Get(p_fsave, FRID_ROULETTE_MULTI_WIFI_RENSHOU, p_data->idx.fridx[p_data->select_listidx]);
#else
  num = 369;
#endif
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 6, FINFO_PAGE05_BA,
      p_data, p_draw, msg_finfo_frprm_win03,
      FINFO_PAGE05_MN_X, FINFO_PAGE05_M_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );



  FInfoDrawArea_MsgBmpOnVReq( p_wk, 6 );

  GFL_STR_DeleteBuffer( p_str );
  GFL_STR_DeleteBuffer( p_tmp );
}
// ミニゲーム
static void FInfoDraw_Page06( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  STRBUF* p_str;
  STRBUF* p_tmp;
  WIFI_LIST* p_wifilist;
  u32 num;
#if USE_FRONTIER_DATA
  FRONTIER_SAVEWORK* p_fsave;

  p_fsave = SaveData_GetFrontier( p_data->p_save );
#endif

  p_wifilist  = GAMEDATA_GetWiFiList(p_data->pGameData); //SaveData_GetWifiListData( p_data->p_save );
  p_str = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );
  p_tmp = GFL_STR_CreateBuffer( WFNOTE_STRBUF_SIZE, heapID );

  // タイトル
  FInfoDraw_Bmp( p_wk, 1, FINFO_PAGE06_BA,
      p_data, p_draw, msg_wifi_note_52,
      FINFO_PAGE06_TT_X, FINFO_PAGE06_TT_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );

  // たまいれ
//  Draw_WflbyGameSetWordSet( p_draw, WFLBY_GAME_BALLSLOW );
  FInfoDraw_Bmp( p_wk, 1, FINFO_PAGE06_BA,
      p_data, p_draw, msg_wifi_note_53,
      FINFO_PAGE06_BC_X, FINFO_PAGE06_BC_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  // 値
  num = 0;//WifiList_GetFriendInfo( p_wifilist, p_data->idx.fridx[p_data->select_listidx], WIFILIST_FRIEND_BALLSLOW_NUM );
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 1, FINFO_PAGE06_BA,
      p_data, p_draw, msg_wifi_note_56,
      FINFO_PAGE06_BC_N_X, FINFO_PAGE06_BC_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );

  // たまのり
//  Draw_WflbyGameSetWordSet( p_draw, WFLBY_GAME_BALANCEBALL );
  FInfoDraw_Bmp( p_wk, 1, FINFO_PAGE06_BA,
      p_data, p_draw, msg_wifi_note_53,
      FINFO_PAGE06_BB_X, FINFO_PAGE06_BB_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  // 値
  num = 0;//WifiList_GetFriendInfo( p_wifilist, p_data->idx.fridx[p_data->select_listidx], WIFILIST_FRIEND_BALANCEBALL_NUM );
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 1, FINFO_PAGE06_BA,
      p_data, p_draw, msg_wifi_note_56,
      FINFO_PAGE06_BB_N_X, FINFO_PAGE06_BB_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );

  // ふうせん
//  Draw_WflbyGameSetWordSet( p_draw, WFLBY_GAME_BALLOON );
  FInfoDraw_Bmp( p_wk, 1, FINFO_PAGE06_BA,
      p_data, p_draw, msg_wifi_note_53,
      FINFO_PAGE06_BL_X, FINFO_PAGE06_BL_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );
  // 値
  num =  0;//WifiList_GetFriendInfo( p_wifilist, p_data->idx.fridx[p_data->select_listidx], WIFILIST_FRIEND_BALLOON_NUM );
  Draw_NumberSetWordset( p_draw, num );
  FInfoDraw_Bmp( p_wk, 1, FINFO_PAGE06_BA,
      p_data, p_draw, msg_wifi_note_56,
      FINFO_PAGE06_BL_N_X, FINFO_PAGE06_BL_Y,
      WFNOTE_COL_BLACK, p_str, p_tmp );


  FInfoDrawArea_MsgBmpOnVReq( p_wk, 1 );

  GFL_STR_DeleteBuffer( p_str );
  GFL_STR_DeleteBuffer( p_tmp );
}

// エンドページ
static void FInfoDraw_Page07( WFNOTE_FINFO_DRAWAREA* p_wk, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, HEAPID heapID )
{
  // タイトル
  FInfoDraw_Bmp( p_wk, 7, FINFO_PAGE07_BA,
      p_data, p_draw, msg_finfo_backmsg,
      0, 0,
      WFNOTE_COL_BLACK, p_draw->p_str, p_draw->p_tmp );

  FInfoDrawArea_MsgBmpOnVReq( p_wk, 7 );
}

//----------------------------------------------------------------------------
/**
 *  @brief  表示面のクリア
 *
 *  @param  p_draw  描画システム
 *  @param  cp_scrnarea スクリーンエリア
 */
//-----------------------------------------------------------------------------
static void FInfoDraw_Clean( WFNOTE_DRAW* p_draw, const WFNOTE_SCRNAREA* cp_scrnarea )
{

  // スクリーン面もクリア
  GFL_BG_FillScreen( DFRM_SCRMSG,
      0,
      MATH_ABS(cp_scrnarea->scrn_x), MATH_ABS(cp_scrnarea->scrn_y),
      32,
      24, 0 );
  GFL_BG_LoadScreenV_Req( DFRM_SCRMSG );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ビットマップへの文字の描画
 *
 *  @param  p_wk    描画エリアワーク
 *  @param  page    ページ
 *  @param  bmp     ビットマップインデックス
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  msg_idx   メッセージインデックス
 *  @param  x     ｘ
 *  @param  y     ｙ
 *  @param  col     カラー
 *  @param  p_str   使用するSTRBUF
 *  @param  p_tmp   使用するSTRBUF
 */
//-----------------------------------------------------------------------------
static void FInfoDraw_Bmp( WFNOTE_FINFO_DRAWAREA* p_wk, u32 page, u32 bmp, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw, u32 msg_idx, u32 x, u32 y, u32 col, STRBUF* p_str, STRBUF* p_tmp )
{
  GFL_MSG_GetString( p_draw->p_msgman, msg_idx, p_tmp );
  WORDSET_ExpandStr( p_draw->p_wordset, p_str, p_tmp );
  PRINTSYS_PrintQueColor( p_draw->printQue , GFL_BMPWIN_GetBmp(p_wk->p_msg[page][bmp]), x, y, p_str, p_draw->fontHandle , col );
}

//----------------------------------------------------------------------------
/**
 *  @brief  詳細情報上画面　ビットマップへの文字の描画
 *
 *  @param  p_wk    描画エリアワーク
 *  @param  page    ページ
 *  @param  bmp     ビットマップインデックス
 *  @param  p_data    データ
 *  @param  p_draw    描画システム
 *  @param  msg_idx   メッセージインデックス
 *  @param  x     ｘ
 *  @param  y     ｙ
 *  @param  col     カラー
 *  @param  p_str   使用するSTRBUF
 *  @param  p_tmp   使用するSTRBUF
 */
//-----------------------------------------------------------------------------
static void FInfoDraw_BaseBmp( GFL_BMPWIN** win, WFNOTE_DATA* p_data, WFNOTE_DRAW* p_draw,
        u32 msg_idx, u32 x, u32 y, u32 col )
{
  GFL_MSG_GetString( p_draw->p_msgman, msg_idx, p_draw->p_tmp );
  WORDSET_ExpandStr( p_draw->p_wordset, p_draw->p_str, p_draw->p_tmp );
  PRINTSYS_PrintQueColor( p_draw->printQue , GFL_BMPWIN_GetBmp(*win), x, y, p_draw->p_str, p_draw->fontHandle , col );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ページ変更
 *
 *  @param  p_wk    ワーク
 *  @param  page    ページ
 */
//-----------------------------------------------------------------------------
static void FInfo_PageChange( WFNOTE_FRIENDINFO* p_wk, s32 add )
{
  int page_num;

  // フロンティアを描画するかチェック
  if( p_wk->frontier_draw == TRUE ){
    page_num = FINFO_PAGE_NUM;
  }else{
    page_num = FINFO_BFOFF_PAGE_NUM;
  }

  if( add > 0 ){
    p_wk->lastpage = p_wk->page;
    p_wk->page = (p_wk->page + add) % page_num;
  }else if( add < 0 ){
    p_wk->lastpage = p_wk->page;
    p_wk->page = p_wk->page + add;
    if( p_wk->page < 0 ){
      p_wk->page += page_num;
    }
  }
}

#if NOTE_TEMP_COMMENT
//未使用。DP・Pt用？
//----------------------------------------------------------------------------
/**
 *  @brief  ポフィンケース持っているかチェック
 *
 *  @param  p_data  ワーク
 */
//-----------------------------------------------------------------------------
static BOOL FInfo_PofinnCaseCheck( WFNOTE_DATA* p_data, HEAPID heapID )
{
  // ポルトケースがあるかチェック
  if( !MyItem_GetItemNum( SaveData_GetMyItem(p_data->p_save),ITEM_POFINKEESU, heapID ) ){
    return FALSE;
  }
  return TRUE;
}
#endif  //NOTE_TEMP_COMMENT

//-------------------------------------
/// 移植用
//=====================================

//------------------------------------------------------------------
/**
 * 文字数字を数字に変換する  18けた対応  この関数は文字コードに左右される
 *
 * @param src     変換したい数字が入っているバッファ
 * @param pbFlag    変換できたかどうか
 * @return  成功した場合変換数字
 *
 *  GSではWFNOTE内でしか使われていないのでここに移植
 *  ルーチンも文字コードからUNICODEに変わったので変更
 */
//------------------------------------------------------------------
static u64 WFNOTE_STRBUF_GetNumber( STRBUF *str , BOOL *pbFlag )
{
  const STRCODE eonCode = GFL_STR_GetEOMCode();
  const STRCODE *code = GFL_STR_GetStringCodePointer( str );

  u64 num = 0;
  u64 digit = 1;
  u16 arrDigit = 0;
  u16 tempNo;

  static const u8 MAX_DIGIT = 18;

  *pbFlag = FALSE;
  while( code[arrDigit] != eonCode )
  {
    if( arrDigit >= MAX_DIGIT )
    {
      //桁オーバー
      return num;
    }

    tempNo = code[arrDigit] - L'0';
    if( tempNo >= 10 )
    {
      tempNo = code[arrDigit] - L'０';
      if( tempNo >= 10 )
      {
        //想定外の文字が入っている
        return num;
      }
    }
    /*
    num += tempNo * digit;
    digit *= 10;
    */
    num *= 10;
    num += tempNo;
    arrDigit++;
  }
  *pbFlag = TRUE;
  return num;
}





//============================================================================================
//	↓追加処理　2010/01/18 by nakahiro
//============================================================================================

static BOOL CheckUITrg( WFNOTE_DATA * p_data )
{
	if( GFL_UI_TP_GetTrg() == TRUE ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
		p_data->key_mode = GFL_APP_END_TOUCH;
		return TRUE;
	}
	if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
		p_data->key_mode = GFL_APP_END_KEY;
		return TRUE;
	}
	return FALSE;
}


#define SCRN_MODESEL_BTN_PX		( 2 )
#define SCRN_MODESEL_BTN_PY		( 5 )
#define SCRN_MODESEL_BTN_SX		( 28 )
#define SCRN_MODESEL_BTN_SY		( 4 )
#define SCRN_MODESEL_BTN_OY		( 5 )

#define SCRN_MODELIST_BTN_PX		( 2 )
#define SCRN_MODELIST_BTN_PY		( 4 )
#define SCRN_MODELIST_BTN_SX		( 14 )
#define SCRN_MODELIST_BTN_SY		( 4 )
#define SCRN_MODELIST_BTN_OX		( 14 )
#define SCRN_MODELIST_BTN_OY		( 4 )

//--------------------------------------------------------------------------------------------
/**
 * @brief		選択中カーソルアニメ作成
 *
 * @param		p_draw    ワーク
 * @param		heapID		ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void Draw_BlinkAnmInit( WFNOTE_DRAW * p_draw, HEAPID heapID )
{
	p_draw->bpawk = BLINKPALANM_Create( BGPLT_M_BACK_3*16, 16, GFL_BG_FRAME0_M, heapID );
	BLINKPALANM_SetPalBufferArcHDL(
		p_draw->bpawk, p_draw->p_handle, NARC_wifi_note_friend_bg_d_NCLR, BGPLT_M_BACK_3*16, BGPLT_M_BACK_2*16 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		選択中カーソルアニメ削除
 *
 * @param		p_draw    ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void Draw_BlinkAnmExit( WFNOTE_DRAW * p_draw )
{
	BLINKPALANM_Exit( p_draw->bpawk );
}

//--------------------------------------------------------------------------------------------
//	ＢＧボタン
//--------------------------------------------------------------------------------------------
/*
  BGPLT_M_BACK_1,					// ボタン（通常時）
  BGPLT_M_BACK_2,					// ボタン（選択時のグラデ用１）
  BGPLT_M_BACK_3,					// ボタン（選択時のグラデ用１）
  BGPLT_M_BACK_4,					// ボタン（項目なし用）
*/

static void SetTopPageButton( u32 pos, u32 pal )
{
	GFL_BG_ChangeScreenPalette(
		DFRM_SCROLL,
		SCRN_MODESEL_BTN_PX, SCRN_MODESEL_BTN_PY+SCRN_MODESEL_BTN_OY*pos,
		SCRN_MODESEL_BTN_SX, SCRN_MODESEL_BTN_SY, pal );
	GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
}

static void SetTopPageButtonPassive( WFNOTE_DRAW * p_draw, u32 pos )
{
	SetTopPageButton( pos, BGPLT_M_BACK_1 );
	BLINKPALANM_InitAnimeCount( p_draw->bpawk );
}

static void SetTopPageButtonActive( WFNOTE_DRAW * p_draw, u32 pos )
{
	SetTopPageButton( pos, BGPLT_M_BACK_3 );
	BLINKPALANM_InitAnimeCount( p_draw->bpawk );
}

static void ChangeTopPageButton( WFNOTE_DRAW * p_draw, u32 now, u32 old )
{
	SetTopPageButtonPassive( p_draw, old );
	SetTopPageButtonActive( p_draw, now );
}

static void ResetTopPageButton( WFNOTE_MODESELECT * p_wk )
{
	u32	i;

	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
		for( i=0; i<3; i++ ){
			SetTopPageButton( i, BGPLT_M_BACK_1 );
		}
	}else{
		for( i=0; i<3; i++ ){
			if( i == p_wk->cursor ){
				SetTopPageButton( i, BGPLT_M_BACK_3 );
			}else{
				SetTopPageButton( i, BGPLT_M_BACK_1 );
			}
		}
	}
}

static BOOL MainTopPageButtonDecedeAnime( WFNOTE_MODESELECT * p_wk )
{
	switch( p_wk->btn_anmSeq ){
	case 0:
		SetTopPageButton( p_wk->cursor, BGPLT_M_BACK_1 );
		p_wk->btn_anmCnt = 0;
		p_wk->btn_anmSeq++;
		break;
	case 1:
		p_wk->btn_anmCnt++;
		if( p_wk->btn_anmCnt == 4 ){
			SetTopPageButton( p_wk->cursor, BGPLT_M_BACK_2 );
			p_wk->btn_anmCnt = 0;
			p_wk->btn_anmSeq++;
		}
		break;
	case 2:
		p_wk->btn_anmCnt++;
		if( p_wk->btn_anmCnt == 4 ){
			SetTopPageButton( p_wk->cursor, BGPLT_M_BACK_1 );
			p_wk->btn_anmCnt = 0;
			p_wk->btn_anmSeq++;
		}
		break;
	case 3:
		p_wk->btn_anmCnt++;
		if( p_wk->btn_anmCnt == 4 ){
			SetTopPageButton( p_wk->cursor, BGPLT_M_BACK_2 );
			p_wk->btn_anmCnt = 0;
			p_wk->btn_anmSeq++;
		}
		break;
	case 4:
		p_wk->btn_anmCnt++;
		if( p_wk->btn_anmCnt == 4 ){
			p_wk->btn_anmSeq = 0;
			p_wk->btn_anmCnt = 0;
			return FALSE;
		}
		break;
	}

	return TRUE;
}



static void SetListPageButton( u32 pos, u32 pal )
{
	GFL_BG_ChangeScreenPalette(
		DFRM_SCROLL,
		SCRN_MODELIST_BTN_PX+SCRN_MODELIST_BTN_OX*(pos/4),
		SCRN_MODELIST_BTN_PY+SCRN_MODELIST_BTN_OY*(pos%4),
		SCRN_MODELIST_BTN_SX, SCRN_MODELIST_BTN_SY, pal );
	GFL_BG_LoadScreenV_Req( DFRM_SCROLL );
}

static void SetListPageButtonPassive( WFNOTE_DRAW * p_draw, u32 pos )
{
	SetListPageButton( pos, BGPLT_M_BACK_1 );
	BLINKPALANM_InitAnimeCount( p_draw->bpawk );
}

static void SetListPageButtonActive( WFNOTE_DRAW * p_draw, u32 pos )
{
	SetListPageButton( pos, BGPLT_M_BACK_3 );
	BLINKPALANM_InitAnimeCount( p_draw->bpawk );
}

static void ChangeListPageButton( WFNOTE_DRAW * p_draw, u32 now, u32 old )
{
	SetListPageButtonPassive( p_draw, old );
	SetListPageButtonActive( p_draw, now );
}

static void ResetListPageButton( WFNOTE_FRIENDLIST * p_wk )
{
	u32	i;

	if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
		for( i=0; i<8; i++ ){
			SetListPageButton( i, BGPLT_M_BACK_1 );
		}
	}else{
		for( i=0; i<8; i++ ){
			if( i == p_wk->pos ){
				SetListPageButton( i, BGPLT_M_BACK_3 );
			}else{
				SetListPageButton( i, BGPLT_M_BACK_1 );
			}
		}
	}
}

static BOOL MainListPageButtonDecedeAnime( WFNOTE_FRIENDLIST * p_wk )
{
	switch( p_wk->btn_anmSeq ){
	case 0:
		SetListPageButton( p_wk->pos, BGPLT_M_BACK_1 );
		p_wk->btn_anmCnt = 0;
		p_wk->btn_anmSeq++;
		break;
	case 1:
		p_wk->btn_anmCnt++;
		if( p_wk->btn_anmCnt == 4 ){
			SetListPageButton( p_wk->pos, BGPLT_M_BACK_2 );
			p_wk->btn_anmCnt = 0;
			p_wk->btn_anmSeq++;
		}
		break;
	case 2:
		p_wk->btn_anmCnt++;
		if( p_wk->btn_anmCnt == 4 ){
			SetListPageButton( p_wk->pos, BGPLT_M_BACK_1 );
			p_wk->btn_anmCnt = 0;
			p_wk->btn_anmSeq++;
		}
		break;
	case 3:
		p_wk->btn_anmCnt++;
		if( p_wk->btn_anmCnt == 4 ){
			SetListPageButton( p_wk->pos, BGPLT_M_BACK_2 );
			p_wk->btn_anmCnt = 0;
			p_wk->btn_anmSeq++;
		}
		break;
	case 4:
		p_wk->btn_anmCnt++;
		if( p_wk->btn_anmCnt == 4 ){
			p_wk->btn_anmSeq = 0;
			p_wk->btn_anmCnt = 0;
			return FALSE;
		}
		break;
	}

	return TRUE;
}


//--------------------------------------------------------------------------------------------
//	戻るボタン
//--------------------------------------------------------------------------------------------

static void SetCancelButtonAnime( WFNOTE_DRAW * p_draw )
{
	GFL_CLACT_WK_SetAnmFrame( p_draw->p_tb, 0 );
	GFL_CLACT_WK_SetAnmSeq( p_draw->p_tb, APP_COMMON_BARICON_RETURN_ON );
	GFL_CLACT_WK_SetAutoAnmFlag( p_draw->p_tb, TRUE );
}


//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
#define	DISP_PASSIVE_BLEND_EV1	( 10 )
#define	DISP_PASSIVE_BLEND_EV2	( 6 )


static void SetDispActive( WFNOTE_DRAW * p_draw, BOOL flg )
{
	if( flg == TRUE ){
		GFL_CLACT_WK_SetObjMode( p_draw->p_tb, GX_OAM_MODE_XLU );
		G2_SetBlendAlpha(
			GX_BLEND_PLANEMASK_BG2,
			GX_BLEND_PLANEMASK_BG0 |
			GX_BLEND_PLANEMASK_BG2,
			DISP_PASSIVE_BLEND_EV1,
			DISP_PASSIVE_BLEND_EV2 );
	}else{
		GFL_CLACT_WK_SetObjMode( p_draw->p_tb, GX_OAM_MODE_NORMAL );
		G2_BlendNone();
	}
}


//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static void ChangeListPageNumAnime( WFNOTE_FRIENDLIST * p_wk, u32 page )
{
	GFL_CLACT_WK_SetAnmFrame( p_wk->p_pageact, 0 );
	GFL_CLACT_WK_SetAnmSeq( p_wk->p_pageact, ACTANM_PAGE_NUM1+page );
}

static void PutListTitle( WFNOTE_DRAW * p_draw )
{
  GFL_BMPWIN_MakeScreen( p_draw->title );
  GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(p_draw->title) );
}

static void SetListDispActive( WFNOTE_FRIENDLIST * p_wk, WFNOTE_DRAW * p_draw, BOOL flg )
{
	WFNOTE_FLIST_DRAWAREA * da;
	u32	i;

	da = &p_wk->drawdata[WFNOTE_DRAWAREA_MAIN];

	if( flg == TRUE ){
		GFL_CLACT_WK_SetObjMode( p_wk->p_pageact, GX_OAM_MODE_XLU );
		GFL_CLACT_WK_SetObjMode( p_draw->p_tb, GX_OAM_MODE_XLU );
		for( i=0; i<FLIST_PAGE_FRIEND_NUM; i++ ){
			if( da->p_hate[i] == NULL ){
				GFL_CLACT_WK_SetObjMode( WF_2DC_WkClWkGet(da->p_clwk[i]), GX_OAM_MODE_XLU );
			}else{
				GFL_CLACT_WK_SetObjMode( da->p_hate[i], GX_OAM_MODE_XLU );
			}
		}
		G2_SetBlendAlpha(
			GX_BLEND_PLANEMASK_BG2,
			GX_BLEND_PLANEMASK_BG0 |
			GX_BLEND_PLANEMASK_BG2,
			DISP_PASSIVE_BLEND_EV1,
			DISP_PASSIVE_BLEND_EV2 );
	}else{
		GFL_CLACT_WK_SetObjMode( p_wk->p_pageact, GX_OAM_MODE_NORMAL );
		GFL_CLACT_WK_SetObjMode( p_draw->p_tb, GX_OAM_MODE_NORMAL );
		for( i=0; i<FLIST_PAGE_FRIEND_NUM; i++ ){
			if( da->p_hate[i] == NULL ){
				GFL_CLACT_WK_SetObjMode( WF_2DC_WkClWkGet(da->p_clwk[i]), GX_OAM_MODE_NORMAL );
			}else{
				GFL_CLACT_WK_SetObjMode( da->p_hate[i], GX_OAM_MODE_NORMAL );
			}
		}
		G2_BlendNone();
	}
}
