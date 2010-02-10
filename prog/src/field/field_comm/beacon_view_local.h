/**
 *  @file   beacon_view_local.h
 *  @brief  すれ違い通信画面ローカルヘッダ
 *  @author Miyuki Iwasawa
 *  @date   10.01.19
 */

#pragma once

#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/game_beacon_accessor.h"
#include "gamesystem/beacon_status.h"
#include "system/palanm.h"
#include "system/gfl_use.h"
#include "app/app_taskmenu.h"
#include "app/app_keycursor.h"
#include "msg/msg_beacon_status.h"

enum{
  SEQ_MAIN,
  SEQ_VIEW_UPDATE,
  SEQ_GPOWER_USE,
  SEQ_THANK_YOU,
  SEQ_RETURN_CGEAR,
  SEQ_CALL_DETAIL_VIEW,
  SEQ_END,
};

enum{
 SSEQ_THANKS_ICON_ANM,
 SSEQ_THANKS_ICON_ANM_WAIT,
 SSEQ_THANKS_MAIN,
 SSEQ_THANKS_VIEW_UPDATE,
 SSEQ_THANKS_DECIDE,
 SSEQ_THANKS_DECIDE_WAIT,
 SSEQ_THANKS_END,
};

typedef enum{
 EV_NONE,
 EV_RETURN_CGEAR,
 EV_CALL_DETAIL_VIEW,
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
#define BG_PAL_LOCALIZE   (12)  //ローカライズ用空きパレット
#define FONT_PAL          (13)  //フォントパレット占有
#define BG_PAL_TASKMENU   (14)  //TaskMenuが2本占有
#define BG_PAL_TASKMENU_2 (15)  //↓

#define BG_PALANM_AREA    (FONT_PAL+1)  //フォントパレット用領域までパレットアニメ影響下に置く

#define ACT_PAL_FONT      (3)
#define ACT_PAL_PANEL     (4)
#define ACT_PAL_UNION     (9)
#define ACT_PAL_WMI       (14)  //通信アイコン占有領域
#define ACT_PAL_LOCALIZE  (15)  //ローカライズ用空きパレット

///フォントカラー
#define	FCOL_FNTOAM   ( PRINTSYS_LSB_Make(1,2,0) )	 ///<OAMフォント黒抜
#define FCOL_FNTOAM_W ( PRINTSYS_LSB_Make(15,3,0))  ///<Oam白抜き
#define FCOL_WHITE_N  ( PRINTSYS_LSB_Make(15,2,0) ) ///<BG白抜き
#define FCOL_POPUP_BASE (7)
#define FCOL_POPUP_MAIN (15)
#define FCOL_POPUP_SDW  (14)
#define FCOL_POPUP      ( PRINTSYS_LSB_Make(FCOL_POPUP_MAIN,FCOL_POPUP_SDW,FCOL_POPUP_BASE))  //BGポップアップ

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
 MENU_POWER = 1,
 MENU_THANKS = 2,
 MENU_RETURN = 4,
 MENU_ALL = 7,
}MENU_ID;

typedef enum{
 MENU_ST_ON,
 MENU_ST_ANM,
 MENU_ST_OFF,
}MENU_STATE;

///////////////////////////////////////////////////
//BMP関連

//パネルメッセージ表示oamウィンドウ
#define BMP_PANEL_OAM_SX  (15)  //パネルOAM
#define BMP_PANEL_OAM_SY  (2)

//ログ数表示oamウィンドウ
#define BMP_LOGNUM_OAM_PX (4)
#define BMP_LOGNUM_OAM_PY (8*19)
#define BMP_LOGNUM_OAM_SX (6)   //ログ数表示OAM
#define BMP_LOGNUM_OAM_SY (2)
#define BMP_LOGNUM_OAM_BGPRI  (3)
#define BMP_LOGNUM_OAM_SPRI   (0)
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

enum{
 WIN_POPUP,
 WIN_MENU,
 WIN_MAX,
};

///アイコンポップタイム
#define ICON_POP_TIME (30*3)

///ポップアップスクロール
#define POPUP_HEIGHT  (8*8)
#define POPUP_DIFF    (8)
#define POPUP_COUNT   (POPUP_HEIGHT/POPUP_DIFF)
#define POPUP_WAIT    (30*3)

//Gパワーポップアップタイプ
enum{
 GPOWER_USE_MINE, //自分
 GPOWER_USE_BEACON, //他人
};

//タスクメニュー数
enum{
 TMENU_YES,
 TMENU_NO,
 TMENU_MAX,
};

#define TMENU_YN_PX (16)
#define TMENU_YN_PY (21)
#define TMENU_YN_W  (8)

////////////////////////////////////////////////////
//アクター関連定義

#define ACT_RENDER_ID (0)

//OBJリソース参照コード
enum{
 OBJ_RES_PLTT,
 OBJ_RES_CGR,
 OBJ_RES_CELLANIM,
 OBJ_RES_MAX,
};

#define UNION_CHAR_MAX      (16)  ///<ユニオンキャラクターmax
#define BEACON_VIEW_OBJ_MAX (128) ///<画面内に表示するOBJの登録max数

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
 ACT_THANKS,
 ACT_RETURN,
 ACT_UP,
 ACT_DOWN,
 ACT_MAX,
};

///ノーマルOBJ アニメID
enum{
 ACTANM_PANEL,
 ACTANM_RANK,
 ACTANM_POWER_ON,
 ACTANM_POWER_ANM,
 ACTANM_POWER_OFF,
 ACTANM_THANKS_ON,
 ACTANM_THANKS_ANM,
 ACTANM_THANKS_OFF,
 ACTANM_RETURN_ON,
 ACTANM_RETURN_ANM,
 ACTANM_RETURN_OFF,
 ACTANM_UP_ON,
 ACTANM_UP_ANM,
 ACTANM_UP_OFF,
 ACTANM_DOWN_ON,
 ACTANM_DOWN_ANM,
 ACTANM_DOWN_OFF,
};

//アイコン種類定義
enum{
 ICON_HELLO,
 ICON_BTL_START,
 ICON_BTL_WIN,
 ICON_POKE_GET,
 ICON_POKE_LVUP,
 ICON_POKE_SHINKA,
 ICON_GPOWER,
 ICON_ITEM_GET,
 ICON_SPECIAL,
 ICON_UNION,
 ICON_THANKS,
 ICON_INFO,
 ICON_MAX,
};


#define ACT_ANM_SET (3)

#define ACT_PANEL_OX  (13)
#define ACT_PANEL_OY  (5*8)
#define ACT_PANEL_PX  (-ACT_PANEL_OX)
#define ACT_PANEL_PY  (-ACT_PANEL_OY)
#define ACT_PANEL_SI_SX (-26*8)  //スライドイン時のスタートポジション
#define ACT_PANEL_SI_SY (0)

#define ACT_UNION_OX  (4*8)
#define ACT_UNION_OY  (3*8)
#define ACT_ICON_OX   (ACT_UNION_OX+28)
#define ACT_ICON_OY   (ACT_UNION_OY)
#define ACT_MSG_OX    (9*8)
#define ACT_MSG_OY    (2*8)
#define ACT_RANK_OX   (19*8)
#define ACT_RANK_OY   (4*8)

#define ACT_MENU_PX (20*8)
#define ACT_MENU_PY (21*8)
#define ACT_MENU_OX (4*8)
#define ACT_MENU_OY (0)
#define ACT_MENU_NUM  (3)
#define ACT_MENU_SX (24)
#define ACT_MENU_SY (24)

#define ACT_UP_PX (26*8)
#define ACT_UP_PY (1*8)
#define ACT_DOWN_PX (27*8)
#define ACT_DOWN_PY (5*8)

//スクロール方向定義
enum{
  SCROLL_UP,
  SCROLL_DOWN,
  SCROLL_RIGHT,
};

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

//==============================================================================
//  構造体定義
//==============================================================================

//オブジェリソース管理
typedef struct _OBJ_RES{
  u32   num;
  u32*  tbl;
}OBJ_RES;

typedef struct _OBJ_RES_SRC{
  u16 num;
  u16 srcID;
}OBJ_RES_SRC;

//オブジェリソース管理構造体
typedef struct _OBJ_RES_TBL{
  OBJ_RES res[OBJ_RES_MAX];
}OBJ_RES_TBL;

//キャラクターリソース
typedef struct _RES2D_CHAR{
  NNSG2dCharacterData* p_char;
  void* buf;
}RES2D_CHAR;

//パレットリソース
typedef struct _RES2D_PLTT{
  NNSG2dPaletteData* p_pltt;
  u16* dat;
  void* buf;
}RES2D_PLTT;

// Bmpウィンドウ
typedef struct{
  GFL_BMPWIN *win;
  GFL_BMP_DATA* bmp;
  PRINT_UTIL putil;
}BMP_WIN;

// OAMフォントワーク
typedef struct {
	BMPOAM_ACT_PTR oam;
	GFL_BMP_DATA * bmp;
}FONT_OAM;

//TaskMenuワーク
typedef struct {
  APP_TASKMENU_WIN_WORK *work;
  APP_TASKMENU_ITEMWORK item;
}TMENU_ITEM;

//パネル管理構造体
typedef struct _PANEL_WORK{
  u8  id; //パネルID
  u8  data_ofs; //データ参照オフセット
  u8  data_idx; //データ配列参照実index

  u8  n_line; //現在のライン
  u8  rank; //ランク
  u8  sex;  //性別
  u32 tr_id;  //トレーナーID
  s16 px;
  s16 py;
  GFL_CLWK* cPanel;
  GFL_CLWK* cUnion;
  GFL_CLWK* cIcon;
  GFL_CLWK* cRank;
  FONT_OAM  msgOam;
	STRBUF* str;
	STRBUF* name;

  GFL_TCBL* tcb;
}PANEL_WORK;

/*
 *  @brief  ポップアップ制御
 */
typedef struct _LOG_CTRL{
  u8  max;  //ログ数
  u8  top;  //今描画されている先頭index
  u8  next_panel; //次にデータが来た時書き込むパネルindex
  u8  view_top;
  u8  view_btm;
  u8  view_max;
  u8  target;

  u8  panel_list[PANEL_VIEW_MAX];
}LOG_CTRL;

/*
 *  @brief  セーブデータ他パラメータ
 */
typedef struct _MY_DATA{
  GPOWER_ID power;
  u32       tr_id;
  u8        sex;
  u8        union_char;
}MY_DATA;


///すれ違い通信状態参照画面管理ワーク
typedef struct _BEACON_VIEW{
  GAMESYS_WORK *gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldWork; 
  FIELD_SUBSCREEN_WORK *subscreen;

  ////////////////////////////////////////
  BEACON_STATUS* b_status;
  GAMEBEACON_INFO_TBL*  infoStack;  //スタックワーク
  GAMEBEACON_INFO_TBL*  infoLog;    //ログテーブル
  GAMEBEACON_INFO*      tmpInfo;
  u16                   tmpTime;
  MY_DATA               my_data;

  BOOL      active;
  int       seq;
  int       sub_seq;
  int       event_id;

  int       io_interval;
  u8        msg_spd;
  LOG_CTRL  ctrl;

  ////////////////////////////////////////
  HEAPID      heapID;
  HEAPID      tmpHeapID;
  ARCHANDLE*  arc_handle;

  GFL_TCBLSYS*  pTcbSys;
  GFL_TCB*      tcbVIntr;

  int           eff_task_ct;

  PALETTE_FADE_PTR pfd;

  GFL_FONT *fontHandle;
  PRINT_QUE *printQue;
  PRINT_STREAM* printStream;
  APP_TASKMENU_RES* menuRes;
  APP_KEYCURSOR_WORK* key_cursor;
  TMENU_ITEM tmenu[TMENU_MAX];

  WORDSET *wordset;
  GFL_MSGDATA *mm_status;
  STRBUF *str_tmp;
  STRBUF *str_expand;
  STRBUF *str_popup;  //ポップアップ用テンポラリ

  BMPOAM_SYS_PTR bmpOam;
  GFL_CLUNIT* cellUnit;
  GFL_CLSYS_REND* cellRender;
  GFL_CLACTPOS  cellSurfacePos;
  OBJ_RES_TBL objResNormal;
  OBJ_RES_TBL objResUnion;
  OBJ_RES_TBL objResIcon;
  
  FONT_OAM   foamLogNum; //現在のログ数表示
  BMP_WIN    win[ WIN_MAX ];
 
  PANEL_WORK  panel[PANEL_MAX];
  GFL_CLWK*   pAct[ACT_MAX];
  u32 log_count;


  //リソース
  RES2D_CHAR  resCharIcon[ICON_MAX];
  RES2D_CHAR  resCharUnion[UNION_CHAR_MAX];
  RES2D_PLTT  resPlttUnion;
  RES2D_PLTT  resPlttPanel;
}BEACON_VIEW;


