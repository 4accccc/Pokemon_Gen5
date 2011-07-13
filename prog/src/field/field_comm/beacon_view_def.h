/*
 *  @file   beacon_view_def.h
 *  @brief  ビーコンメイン画面　リテラル定義
 *  @author Miyuki Iwasawa
 *  @date   10.03.04
 */

#pragma once

///メインシーケンス
enum{
  SEQ_MAIN,
  SEQ_LOG_ENTRY,
  SEQ_VIEW_UPDATE,
  SEQ_LIST_SCROLL_REPEAT,
  SEQ_SCROLL_BAR,
  SEQ_GPOWER_USE,
  SEQ_THANK_YOU,
  SEQ_EV_REQ_WAIT,
  SEQ_END,
};

///サブシーケンス
enum{
 SSEQ_THANKS_ICON_ANM,
 SSEQ_THANKS_ICON_ANM_WAIT,
 SSEQ_THANKS_MAIN,
 SSEQ_THANKS_VIEW_UPDATE,
 SSEQ_THANKS_DECIDE,
 SSEQ_THANKS_DECIDE_WAIT,
 SSEQ_THANKS_END,
#ifdef BUGFIX_AF_BTS8022_20100806
 SSEQ_THANKS_END_WAIT,
#endif
};

///イベントリクエストID
typedef enum{
 EV_NONE,
 EV_RETURN_CGEAR,
 EV_RETURN_CGEAR_INTRUDE,     //ハイリンク侵入呼び出し
 EV_RETURN_CGEAR_WIRELESS_TR, //ライブキャスター呼び出し
 EV_CALL_DETAIL_VIEW,
 EV_CALL_MEDAL_VIEW,
 EV_CALL_GPOWER_APP,
 EV_GPOWER_USE,
 EV_GPOWER_CHECK_TMENU_YN,
 EV_GPOWER_CHECK_TMENU_CHK,
 EV_CALL_TALKMSG_INPUT,
 EV_FMISSION_START,
 EV_FMISSION_CHECK,

#ifdef PM_DEBUG
 EV_DEBUG_MENU_CALL,
#endif  //PM_DEBUG
}BEACON_DETAIL_EVENT;

///スペシャルGパワーリクエストID
typedef enum{
  SP_GPOWER_REQ_NULL,
  SP_GPOWER_REQ_HATCH_UP,
  SP_GPOWER_REQ_NATSUKI_UP,
  SP_GPOWER_REQ_SALE,
  SP_GPOWER_REQ_EXP_UP,
  SP_GPOWER_REQ_MONEY_UP,
  SP_GPOWER_REQ_CAPTURE_UP,
  SP_GPOWER_REQ_MAX = SP_GPOWER_REQ_CAPTURE_UP,
}SP_GPOWER_REQ;

/////////////////////////////////////
//リテラル
#define FRM_MENUMSG ( GFL_BG_FRAME0_S )
#define FRM_MENU   ( GFL_BG_FRAME1_S )
#define FRM_POPUP  ( GFL_BG_FRAME2_S )
#define FRM_BACK   ( GFL_BG_FRAME3_S )

//BGアルファ設定
#define ALPHA_1ST_NORMAL  (GX_BLEND_PLANEMASK_BG2)
#define ALPHA_1ST_PASSIVE (GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG2)
#define ALPHA_2ND         (GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ)
#define ALPHA_EV2_NORMAL  (3)
#define ALPHA_EV1_NORMAL  (16-ALPHA_EV2_NORMAL)
#define ALPHA_EV2_PASSIVE  (6)
#define ALPHA_EV1_PASSIVE  (16-ALPHA_EV2_PASSIVE)

///パレット展開位置
#define FONT_PAL_POPUP    (1)
#define FONT_PAL_LOGNUM   (4)   //ログ人数表示bmp パレット
#define BG_PAL_LOCALIZE   (12)  //ローカライズ用空きパレット
#define FONT_PAL          (13)  //フォントパレット占有
#define BG_PAL_TASKMENU   (14)  //TaskMenuが2本占有
#define BG_PAL_TASKMENU_2 (15)  //↓

#define BG_PALANM_AREA    (FONT_PAL+1)  //フォントパレット用領域までパレットアニメ影響下に置く

///アクターパレット展開位置
#define ACT_PAL_FONT      (3)
#define ACT_PAL_PANEL     (4)   //5本占有
#define ACT_PAL_UNION     (9)   //5本占有
#define ACT_PAL_WMI       (14)  //通信アイコン占有領域
#define ACT_PAL_LOCALIZE  (15)  //ローカライズ用空きパレット

///アクターパネル色リロードパレット展開位置
#define ACT_PANEL_COLOR_BLACK (0) //フレーム色(ブラックバージョン)
#define ACT_PANEL_COLOR_WHITE (1) //フレーム色(ホワイトバージョン)
#define ACT_PANEL_COLOR_RANK_MARK_B  (2) //ランクマークOn/Off切り替え用(ブラックバージョン)
#define ACT_PANEL_COLOR_RANK_MARK_W  (3) //ランクマークOn/Off切り替え用(ホワイトバージョン)

///パネルアクター　カラー転送数
#define ACT_PANEL_COL_CHANGE_NUM  (5) //カラーバージョンで変更される色数
#define ACT_PANEL_COL_TRANS_NUM   (ACT_PANEL_COL_CHANGE_NUM+2)  //カラーバージョン変更+DSカラー2色
///パネルアクター　ランクマーク色転送カラーオフセット
#define ACT_PANEL_COL_OFS_RANK    (8)

///フォントカラー
#define	FCOL_FNTOAM_B   ( PRINTSYS_MACRO_LSB(1,2,0) )	 ///<OAMフォント黒抜
#define FCOL_FNTOAM_GB  ( PRINTSYS_MACRO_LSB(2,3,0))  ///<Oam灰抜き
#define FCOL_FNTOAM_W   ( PRINTSYS_MACRO_LSB(4,5,0))  ///<Oam白抜き
#define FCOL_FNTOAM_GW  ( PRINTSYS_MACRO_LSB(5,6,0))  ///<Oam灰抜き
#define FCOL_WHITE_N  ( PRINTSYS_MACRO_LSB(15,2,0) ) ///<BG白抜き
#define FCOL_POPUP_BASE (9)
#define FCOL_POPUP_MAIN (1)
#define FCOL_POPUP_SDW  (2)
#define FCOL_POPUP      ( PRINTSYS_MACRO_LSB(FCOL_POPUP_MAIN,FCOL_POPUP_SDW,FCOL_POPUP_BASE))  //BGポップアップ
#define FCOL_LOGNUM_BASE  (1)
#define FCOL_LOGNUM     ( PRINTSYS_MACRO_LSB(15,14,FCOL_LOGNUM_BASE))  //ログ人数表示

///名前を灰色表示するまで経過時間
#define BEACON_TIMEOUT_FRAME  (3*60*60) //60sec*3

///表示するログ件数
#define VIEW_LOG_MAX    (4)

#define BS_LOG_MAX  (30)  //ログ管理数
#define PANEL_MAX   (5)   //同時描画されるパネル数
#define PANEL_VIEW_START  (1)
#define PANEL_VIEW_END    (4)
#define PANEL_VIEW_MAX    (4)   //画面内に描画されるパネル数
#define PANEL_LINE_END    (5)

#define PANEL_DATA_BLANK (0xFF)

///ポップアップメッセージバッファ長
#define BUFLEN_POPUP_MSG  (18*6*2+EOM_SIZE)
#define BUFLEN_TMP_MSG    (BUFLEN_POPUP_MSG)

///パネル文字列バッファ長
#define BUFLEN_PANEL_MSG  (10+EOM_SIZE)
///トレーナー名バッファ長
#define BUFLEN_TR_NAME  (PERSON_NAME_SIZE+EOM_SIZE)

///メニュー
typedef enum{
 MENU_F_POWER = 1,
 MENU_F_HELLO = 2,
 MENU_F_THANKS = 4,
 MENU_F_RETURN = 8,
 MENU_F_ALL = 0xF,
}MENU_FLAG;

typedef enum{
 MENU_POWER,
 MENU_HELLO,
 MENU_THANKS,
 MENU_RETURN,
 MENU_ALL,
}MENU_ID;

typedef enum{
 MENU_ST_ON,
 MENU_ST_ANM,
 MENU_ST_OFF,
}MENU_STATE;

///////////////////////////////////////////////////
//パネル表示コード
typedef enum{
  PANEL_DRAW_UPDATE,
  PANEL_DRAW_NEW,
  PANEL_DRAW_INI,
}PANEL_DRAW_TYPE;

///////////////////////////////////////////////////
//SE関連
#define BVIEW_SE_DECIDE (SEQ_SE_DECIDE1)
#define BVIEW_SE_MENU   (SEQ_SE_SYS_79)
#define BVIEW_SE_SELECT (SEQ_SE_SELECT1)
#define BVIEW_SE_CANCEL (SEQ_SE_CANCEL1)
#define BVIEW_SE_UPDOWN (SEQ_SE_SELECT4)
#define BVIEW_SE_NEW_PLAYER (SEQ_SE_SYS_11)
#define BVIEW_SE_ICON   (SEQ_SE_SELECT2)

///////////////////////////////////////////////////
//BMP関連

//パネルメッセージ表示oamウィンドウ
#define BMP_PANEL_OAM_SX  (12)  //パネルOAM
#define BMP_PANEL_OAM_SY  (2)

//ポップアップウィンドウ
#define BMP_POPUP_PX (2)
#define BMP_POPUP_PY (22)
#define BMP_POPUP_SX  (28)
#define BMP_POPUP_SY  (6)
#define BMP_POPUP_FRM (FRM_POPUP)
#define BMP_POPUP_PAL (FONT_PAL_POPUP)
//メニューバーウィンドウ
#define BMP_MENU_PX  (1)
#define BMP_MENU_PY  (21)
#define BMP_MENU_SX  (15)
#define BMP_MENU_SY  (3)
#define BMP_MENU_FRM (FRM_MENUMSG)
#define BMP_MENU_PAL (FONT_PAL)
//ログ数表示ウィンドウ
#define BMP_LOGNUM_PX (8)
#define BMP_LOGNUM_PY (19*8)
#define BMP_LOGNUM_SX (6)   //ログ数表示OAM
#define BMP_LOGNUM_SY (2)
#define BMP_LOGNUM_FRM (FRM_BACK)

//ミッション内容表示ウィンドウ
#define BMP_FMISSION_PX (4)
#define BMP_FMISSION_PY (2)
#define BMP_FMISSION_SX (24)   //
#define BMP_FMISSION_SY (6)
#define BMP_FMISSION_FRM (FRM_BACK)
#define BMP_FMISSION_PAL (FONT_PAL_LOGNUM)
//ミッションスコア表示ウィンドウ
#define BMP_FMSCORE_PX (2)
#define BMP_FMSCORE_PY (9)
#define BMP_FMSCORE_SX (28)   //
#define BMP_FMSCORE_SY (2)
#define BMP_FMSCORE_FRM (FRM_BACK)
#define BMP_FMSCORE_PAL (FONT_PAL_LOGNUM)
//ミッションタイム表示ウィンドウ
#define BMP_FMTIME_PX (16)
#define BMP_FMTIME_PY (12)
#define BMP_FMTIME_SX (14)   //
#define BMP_FMTIME_SY (2)
#define BMP_FMTIME_FRM (FRM_BACK)
#define BMP_FMTIME_PAL (FONT_PAL_LOGNUM)

enum{
 WIN_POPUP,
 WIN_MENU,
 WIN_MAX,
};

///パネルスクロールフレーム
#define PANEL_SCROLL_FRAME (8)

///スクロールバーが有効になるパネル数
#define PANEL_SCROLL_BAR_ENABLE_NUM (PANEL_VIEW_MAX+1)

///アイコンポップタイム
#define ICON_POP_TIME (30*3)

//Gパワーポップアップタイプ
enum{
 GPOWER_USE_MINE, //自分
 GPOWER_USE_BEACON, //他人
 GPOWER_USE_SPECIAL, //特殊デルパワー
 GPOWER_USE_MAX,
};

////////////////////////////////////////////////////
//アクター関連定義

#define UNION_CHAR_MAX      (16)  ///<ユニオンキャラクターmax
#define BEACON_VIEW_OBJ_MAX (64) ///<画面内に表示するOBJの登録max数

#define PANEL_NUM_ACTMAX    (3)       ///<パネルに表示するメダル数OBJの数
#define PANEL_MEDAL_NONE    (0xFF)    ///<メダル数無効値

///OBJ BGプライオリティ
#define OBJ_BG_PRI (3)
#define OBJ_MENU_BG_PRI (1)

///OBJソフトウェアプライオリティ
enum{
 OBJ_SPRI_LOGNUM = 0,
 OBJ_SPRI_MSG = 1,
 OBJ_SPRI_MENU = 2,
 OBJ_SPRI_SWITCH = 2,
 OBJ_SPRI_BAR = 3,
 OBJ_SPRI_NUM = OBJ_SPRI_MSG + PANEL_MAX,
 OBJ_SPRI_MEDAL = OBJ_SPRI_NUM + PANEL_MAX,
 OBJ_SPRI_ICON = OBJ_SPRI_MEDAL + PANEL_MAX,
 OBJ_SPRI_UNION = OBJ_SPRI_ICON + PANEL_MAX,
 OBJ_SPRI_RANK = OBJ_SPRI_UNION + PANEL_MAX,
 OBJ_SPRI_PANEL = OBJ_SPRI_RANK + PANEL_MAX,
};

///アクターID
enum{
 ACT_POWER,
 ACT_HELLO,
 ACT_THANKS,
 ACT_RETURN,
 ACT_UP,
 ACT_DOWN,
 ACT_SWITCH,
 ACT_BAR,
 ACT_LOGNUM_WIN,
 ACT_MAX,
};

///ノーマルOBJ アニメID
enum{
 ACTANM_PANEL,
 ACTANM_POWER_ON,
 ACTANM_POWER_ANM,
 ACTANM_POWER_OFF,
 ACTANM_HELLO_ON,
 ACTANM_HELLO_ANM,
 ACTANM_HELLO_OFF,
 ACTANM_THANKS_ON,
 ACTANM_THANKS_ANM,
 ACTANM_THANKS_OFF,
 ACTANM_RETURN_ON,
 ACTANM_RETURN_ANM,
 ACTANM_RETURN_OFF,
 ACTANM_UP_DEF,
 ACTANM_UP_ANM,
 ACTANM_UP_ON,
 ACTANM_UP_OFF,
 ACTANM_DOWN_DEF,
 ACTANM_DOWN_ANM,
 ACTANM_DOWN_ON,
 ACTANM_DOWN_OFF,
 ACTANM_SWITCH,
 ACTANM_MEDAL_N,
 ACTANM_MEDAL_ON,
 ACTANM_MEDAL_OFF,
 ACTANM_NUM,
 ACTANM_LOGNUM_WIN,
 ACTANM_BAR,
};

#define ACT_ANM_SET (3)

#define ACT_PANEL_SX  (27*8)
#define ACT_PANEL_SY  (5*8)
#define ACT_PANEL_OY  (ACT_PANEL_SY)
#define ACT_PANEL_PX  ( 8 )
#define ACT_PANEL_PY  (-ACT_PANEL_OY)
#define ACT_PANEL_SI_SX (-ACT_PANEL_SX)  //スライドイン時のスタートポジション
#define ACT_PANEL_SI_SY (0)

#define ACT_UNION_OX  (3*8)
#define ACT_UNION_OY  (3*8)
#define ACT_ICON_OX   (ACT_UNION_OX+28)
#define ACT_ICON_OY   (ACT_UNION_OY)
#define ACT_MSG_OX    (9*8)
#define ACT_MSG_OY    (2*8)
#define ACT_RANK_OX   (19*8)
#define ACT_RANK_OY   (4*8)
#define ACT_MEDAL_OX  (23*8+4)
#define ACT_MEDAL_OY  (1*8+4)
#define ACT_NUM_OX    (ACT_MEDAL_OX+12)
#define ACT_NUM_OY    (ACT_MEDAL_OY+16)
#define ACT_NUM_OSX   (8)

#define ACT_MENU_PX (16*8+4)
#define ACT_MENU_PY (21*8+4)
#define ACT_MENU_OX (4*8)
#define ACT_MENU_OY (0)
#define ACT_MENU_NUM  (4)
#define ACT_MENU_SX (24)
#define ACT_MENU_SY (24)

#define ACT_UPDOWN_SX (16)
#define ACT_UPDOWN_SY (16)
#define ACT_UP_PX (29*8)
#define ACT_UP_PY (3*8)
#define ACT_DOWN_PX (ACT_UP_PX)
#define ACT_DOWN_PY (18*8)

#define ACT_SWITCH_PX (ACT_UP_PX)
#define ACT_SWITCH_PY (ACT_UP_PY+ACT_UPDOWN_SY+4)
#define ACT_BAR_PX    (ACT_UP_PX)
#define ACT_BAR_PY    (ACT_SWITCH_PY)
#define ACT_BAR_SX    (16)
#define ACT_BAR_SY    (11*8)

#define ACT_LOGNUM_WIN_PX  (0)
#define ACT_LOGNUM_WIN_PY  (19*8)

/////////////////////////////////////////////
//タッチパネル
#define IO_INTERVAL (30*5)

//パネルの矩形 LT,LB,RB,RT
#define TP_PANEL_TOP    (ACT_PANEL_PY+ACT_PANEL_OY)
#define TP_PANEL_BOTTOM (TP_PANEL_TOP+ACT_PANEL_SY)
#define TP_PANEL_LEFT   (ACT_PANEL_PX)
#define TP_PANEL_RIGHT  (TP_PANEL_LEFT+ACT_PANEL_SX)

#define TP_MEDAL_R      (8)
#define TP_MEDAL_X      (ACT_PANEL_PX+ACT_MEDAL_OX+TP_MEDAL_R)
#define TP_MEDAL_Y      (ACT_PANEL_PY+ACT_PANEL_OY+ACT_MEDAL_OY+TP_MEDAL_R)

#define TP_UP_TOP       (ACT_UP_PY)
#define TP_UP_BOTTOM    (TP_UP_TOP+ACT_UPDOWN_SY)
#define TP_UP_LEFT      (ACT_UP_PX)
#define TP_UP_RIGHT     (TP_UP_LEFT+ACT_UPDOWN_SX)

#define TP_DOWN_TOP     (ACT_DOWN_PY)
#define TP_DOWN_BOTTOM  (TP_DOWN_TOP+ACT_UPDOWN_SY)
#define TP_DOWN_LEFT    (ACT_DOWN_PX)
#define TP_DOWN_RIGHT   (TP_DOWN_LEFT+ACT_UPDOWN_SX)

#define TP_BAR_TOP          (ACT_BAR_PY)
#define TP_BAR_BOTTOM       (ACT_BAR_PY+ACT_BAR_SY)
#define TP_BAR_BOTTOM_1ST   (TP_BAR_BOTTOM+8) //初期タッチ時に許容する範囲(スイッチのYサイズ分足しておく)
#define TP_BAR_LEFT     (ACT_BAR_PX)
#define TP_BAR_RIGHT    (ACT_BAR_PX+ACT_BAR_SX)
#define TP_BAR_HEIGHT   (0)

