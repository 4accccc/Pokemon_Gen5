/*
 *  @file   fmission_view_def.h
 *  @brief  フェスミッションログ画面　リテラル定義
 *  @author Miyuki Iwasawa
 *  @date   11.03.08
 */

#pragma once

///メインシーケンス
enum{
  SEQ_MAIN,
  SEQ_LOG_ENTRY,
  SEQ_VIEW_UPDATE,
  SEQ_LIST_SCROLL_REPEAT,
  SEQ_RETIRE,
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
 EV_CALL_INFO_VIEW,
 EV_MISSION_RETIRE,
 EV_GPOWER_USE,
 EV_GPOWER_USE_MINE,
 EV_GPOWER_CHECK_TMENU_YN,
 EV_GPOWER_CHECK_TMENU_CHK,
 EV_CALL_TALKMSG_INPUT,

#ifdef PM_DEBUG
 EV_DEBUG_MENU_CALL,
#endif  //PM_DEBUG
}BEACON_DETAIL_EVENT;

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
#define ACT_PAL_PANEL     (4)   //3本占有
#define ACT_PAL_UNION     (9)   //5本占有
#define ACT_PAL_WMI       (14)  //通信アイコン占有領域
#define ACT_PAL_LOCALIZE  (15)  //ローカライズ用空きパレット

///フォントカラー
#define	FCOL_FNTOAM   ( PRINTSYS_MACRO_LSB(1,2,0) )	 ///<OAMフォント黒抜
#define FCOL_FNTOAM_W ( PRINTSYS_MACRO_LSB(15,3,0))  ///<Oam白抜き
#define FCOL_FNTOAM_B ( PRINTSYS_MACRO_LSB(2,3,0))  ///<Oam白抜き
#define FCOL_WHITE_N  ( PRINTSYS_MACRO_LSB(15,2,0) ) ///<BG白抜き
#define FCOL_POPUP_BASE (9)
#define FCOL_POPUP_MAIN (1)
#define FCOL_POPUP_SDW  (2)
#define FCOL_POPUP      ( PRINTSYS_MACRO_LSB(FCOL_POPUP_MAIN,FCOL_POPUP_SDW,FCOL_POPUP_BASE))  //BGポップアップ
#define FCOL_TITLE_BASE  (0xD)
#define FCOL_TITLE       ( PRINTSYS_MACRO_LSB(1,2,FCOL_TITLE_BASE))  //タイトル表示

///名前を灰色表示するまで経過時間
#define BEACON_TIMEOUT_FRAME  (3*60*60) //60sec*3

///表示するログ件数
#define VIEW_LOG_MAX    (4)

#define MISSION_LOG_MAX   (30)  //ログ管理数
#define PANEL_MAX         (4)   //同時描画されるパネル数
#define PANEL_VIEW_START  (1)
#define PANEL_VIEW_END    (3)
#define PANEL_VIEW_MAX    (3)   //画面内に描画されるパネル数
#define PANEL_LINE_END    (4)

#define PANEL_DATA_BLANK (0xFF)

typedef enum{
 PANEL_FOAM_NAME,
 PANEL_FOAM_MSG,
 PANEL_FOAM_MAX,
}PANEL_FOAM_ID;

typedef enum{
 PANEL_TYPE_SYS,
 PANEL_TYPE_ACT,  
 PANEL_TYPE_MSG,
}PANEL_TYPE;

///ポップアップメッセージバッファ長
#define BUFLEN_POPUP_MSG  (18*6*2+EOM_SIZE)
#define BUFLEN_TMP_MSG    (BUFLEN_POPUP_MSG)

///メニュー
typedef enum{
 MENU_F_POWER = 0x01,
 MENU_F_HELLO = 0x02,
 MENU_F_RETIRE = 0x04,
 MENU_F_INFO = 0x08,
 MENU_F_UP = 0x10,
 MENU_F_DOWN = 0x20,
 MENU_F_UPDOWN = (MENU_F_UP|MENU_F_DOWN),
 MENU_F_ALL = 0xFF,
}MENU_FLAG;

typedef enum{
 MENU_POWER,
 MENU_HELLO,
 MENU_RETIRE,
 MENU_INFO,
 MENU_UP,
 MENU_DOWN,
 MENU_UPDOWN,
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
#define FVIEW_SE_DECIDE (SEQ_SE_DECIDE1)
#define FVIEW_SE_MENU   (SEQ_SE_SYS_79)
#define FVIEW_SE_SELECT (SEQ_SE_SELECT1)
#define FVIEW_SE_CANCEL (SEQ_SE_CANCEL1)
#define FVIEW_SE_UPDOWN (SEQ_SE_SELECT4)
#define FVIEW_SE_NORMAL_LOG (SEQ_SE_SYS_11)
#define FVIEW_SE_NEW_PLAYER (SEQ_SE_SW_FES_01)
#define FVIEW_SE_FREEWORD   (SEQ_SE_SW_FES_02)
#define FVIEW_SE_SCOREUP    (SEQ_SE_SW_FES_03)
#define FVIEW_SE_ICON   (SEQ_SE_SELECT2)

///////////////////////////////////////////////////
//BMP関連

//パネルメッセージ表示oamウィンドウ

//全15文字(うち漢字等の入力可能文字以外は12文字以内でないとBitmap領域に入りません)
#define BMP_PANEL_MSG_OAM_SX  (24)  //パネルOAM(23でいいけど、ライブラリが結局24char取る)
#define BMP_PANEL_MSG_OAM_SY  (2)

//プレイヤー名 日本＆ハングル(12dot*5) 欧米(6dot*7)
#define BMP_PANEL_NAME_OAM_SX  (8)  //パネルOAM
#define BMP_PANEL_NAME_OAM_SY  (2)

//ポップアップウィンドウ
#define BMP_POPUP_PX (2)
#define BMP_POPUP_PY (22)
#define BMP_POPUP_SX  (28)
#define BMP_POPUP_SY  (6)
#define BMP_POPUP_FRM (FRM_POPUP)
#define BMP_POPUP_PAL (FONT_PAL_POPUP)
//タイトルウィンドウ
#define BMP_TITLE_PX  (5)
#define BMP_TITLE_PY  (0)
#define BMP_TITLE_SX  (25)
#define BMP_TITLE_SY  (5)
#define BMP_TITLE_FRM (FRM_MENU)
#define BMP_TITLE_PAL (FONT_PAL_POPUP)

#define BMP_TITLE_OX  (0)
#define BMP_TITLE_OY  (4)
#define BMP_ENTRY_STR_OX    (16)
#define BMP_ENTRY_STR_OY    (3*8)
#define BMP_ENTRY_NUM_DSX   (9*8)
#define BMP_ENTRY_NUM_DSY   (16)
#define BMP_ENTRY_NUM_OX    ((BMP_TITLE_SX*8)-BMP_ENTRY_NUM_DSX)
#define BMP_ENTRY_NUM_OY    (BMP_ENTRY_STR_OY)

enum{
 WIN_POPUP,
 WIN_TITLE,
 WIN_MAX,
};

///パネルスクロールフレーム
#define PANEL_SCROLL_FRAME (8)

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
#define FMISSION_VIEW_OBJ_MAX (64) ///<画面内に表示するOBJの登録max数

///OBJ BGプライオリティ
#define OBJ_BG_PRI (3)
#define OBJ_MENU_BG_PRI (1)

///OBJソフトウェアプライオリティ
enum{
 OBJ_SPRI_MSG = 0,
 OBJ_SPRI_MENU = 1,
 OBJ_SPRI_ICON = OBJ_SPRI_MSG + PANEL_MAX,
 OBJ_SPRI_UNION = OBJ_SPRI_ICON + PANEL_MAX,
 OBJ_SPRI_RANK = OBJ_SPRI_UNION + PANEL_MAX,
 OBJ_SPRI_PANEL = OBJ_SPRI_RANK + PANEL_MAX,
};

///アクターID
enum{
 ACT_POWER,
 ACT_HELLO,
 ACT_RETIRE,
 ACT_INFO,
 ACT_UP,
 ACT_DOWN,
 ACT_MAX,
};

///ノーマルOBJ アニメID
enum{
 ACTANM_PANEL_SYS,
 ACTANM_PANEL_ACT,
 ACTANM_PANEL_MSG,
 ACTANM_POWER_ON,
 ACTANM_POWER_ANM,
 ACTANM_POWER_OFF,
 ACTANM_HELLO_ON,
 ACTANM_HELLO_ANM,
 ACTANM_HELLO_OFF,
 ACTANM_RETIRE_ON,
 ACTANM_RETIRE_ANM,
 ACTANM_RETIRE_OFF,
 ACTANM_INFO_ON,
 ACTANM_INFO_ANM,
 ACTANM_INFO_OFF,
 ACTANM_UP_ON,
 ACTANM_UP_ANM,
 ACTANM_UP_OFF,
 ACTANM_DOWN_ON,
 ACTANM_DOWN_ANM,
 ACTANM_DOWN_OFF,
};

#define ACT_ANM_SET (3)

#define ACT_PANEL_OX  (0)
#define ACT_PANEL_OY  (5*8)
#define ACT_PANEL_PX  (8)
#define ACT_PANEL_PY  (6*8-ACT_PANEL_OY)

#define ACT_UNION_OX  (2*8)
#define ACT_UNION_OY  (2*8)
#define ACT_NAME_OX   (6*8)
#define ACT_NAME_OY   (4)
#define ACT_MSG_OX    (6*8)
#define ACT_MSG_OY    (16+4)
#define ACT_RANK_OX   (19*8)
#define ACT_RANK_OY   (4*8)

#define ACT_MENU_PX (20*8+4)
#define ACT_MENU_PY (21*8+4)
#define ACT_MENU_OX (4*8)
#define ACT_MENU_OY (0)
#define ACT_MENU_NUM  (6)
#define ACT_MENU_FMISSION_NUM  (3)
#define ACT_MENU_SX (24)
#define ACT_MENU_SY (24)
#define ACT_MENU_FMISSION_PX (ACT_MENU_PX+ACT_MENU_OX)

#define ACT_UP_PX (11*8)
#define ACT_DOWN_PX (ACT_UP_PX+8*3)

#define ACT_INFO_PX (2*8)
#define ACT_INFO_PY (4)

/////////////////////////////////////////////
//タッチパネル
#define IO_INTERVAL (30*5)

//パネルの矩形 LT,LB,RB,RT
#define TP_PANEL_X1 (0)
#define TP_PANEL_Y1 (5)
#define TP_PANEL_X2 (TP_PANEL_X1+12)
#define TP_PANEL_Y2 (TP_PANEL_Y1+38)
#define TP_PANEL_X3 (TP_PANEL_X2+198)
#define TP_PANEL_Y3 (TP_PANEL_Y2)
#define TP_PANEL_X4 (TP_PANEL_X1+198)
#define TP_PANEL_Y4 (TP_PANEL_Y1)

/*
  0
1   4
 2 3
*/
#define TP_UP_X1  (6 + ACT_UP_PX )
#define TP_UP_Y1  (3 + ACT_UP_PY )
#define TP_UP_X2  (1 + ACT_UP_PX )
#define TP_UP_Y2  (12 + ACT_UP_PY )
#define TP_UP_X3  (9 + ACT_UP_PX )
#define TP_UP_Y3  (31 + ACT_UP_PY )
#define TP_UP_X4  (31 + ACT_UP_PX )
#define TP_UP_Y4  (TP_UP_Y3 + ACT_UP_PY )
#define TP_UP_X5  (24 + ACT_UP_PX )
#define TP_UP_Y5  (10 + ACT_UP_PY )

/*
 0 4
1   3 
  2
*/
#define TP_DOWN_X1  (1  + ACT_DOWN_PX )
#define TP_DOWN_Y1  (1  + ACT_DOWN_PY )
#define TP_DOWN_X2  (8  + ACT_DOWN_PX )
#define TP_DOWN_Y2  (22 + ACT_DOWN_PY )
#define TP_DOWN_X3  (23 + ACT_DOWN_PX )
#define TP_DOWN_Y3  (30 + ACT_DOWN_PY )
#define TP_DOWN_X4  (31 + ACT_DOWN_PX )
#define TP_DOWN_Y4  (22 + ACT_DOWN_PY )
#define TP_DOWN_X5  (24 + ACT_DOWN_PX )
#define TP_DOWN_Y5  (1  + ACT_DOWN_PY )


