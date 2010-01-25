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

enum{
  SEQ_MAIN,

};

/////////////////////////////////////
//リテラル
#define HEAPID_BVIEW_TMP      GFL_HEAP_LOWID( HEAPID_FIELDMAP )
#define HEAPID_BEACON_VIEW    (HEAPID_FIELDMAP)

///パレット展開位置
#define FONT_PAL    (15)
#define ACT_PAL_FONT  (3)
#define ACT_PAL_PANEL (4)
#define ACT_PAL_UNION (9)

#define	FCOL_FNTOAM	( PRINTSYS_LSB_Make(1,2,0) )		// フォントカラー：OAMフォント黒抜

///表示するログ件数
#define VIEW_LOG_MAX    (4)

#define BS_LOG_MAX  (30)  //ログ管理数
#define PANEL_MAX   (5)   //同時描画されるパネル数
#define PANEL_VIEW_MAX    (4)   //画面内に描画されるパネル数
#define PANEL_LINE_END    (5)

#define PANEL_DATA_BLANK (0xFF)

///パネル文字列バッファ長
#define BUFLEN_PANEL_MSG  (10+EOM_SIZE)
///トレーナー名バッファ長
#define BUFLEN_TR_NAME  (PERSON_NAME_SIZE+EOM_SIZE)

#define ACT_RENDER_ID (0)

//OBJリソース参照コード
enum{
 OBJ_RES_PLTT,
 OBJ_RES_CGR,
 OBJ_RES_CELLANIM,
 OBJ_RES_MAX,
};

#define UNION_CHAR_MAX      (16)  ///<ユニオンキャラクターmax
#define BEACON_VIEW_OBJ_MAX (5*8) ///<画面内に表示するOBJの登録max数


#define FRM_POPUP  ( GFL_BG_FRAME1_S )
#define FRM_PANEL  ( GFL_BG_FRAME2_S )
#define FRM_BACK  ( GFL_BG_FRAME3_S )

///OBJ BGプライオリティ
#define OBJ_BG_PRI (3)

///OBJソフトウェアプライオリティ
enum{
 OBJ_SPRI_MSG = 0,
 OBJ_SPRI_ICON = OBJ_SPRI_MSG + PANEL_MAX,
 OBJ_SPRI_UNION = OBJ_SPRI_ICON + PANEL_MAX,
 OBJ_SPRI_PANEL = OBJ_SPRI_UNION + PANEL_MAX,
};

///ノーマルOBJ アニメID
enum{
 ACTANM_PANEL,
};

#define ACT_PANEL_OX  (13)
#define ACT_PANEL_OY  (5*8)
#define ACT_PANEL_PX  (-ACT_PANEL_OX)
#define ACT_PANEL_PY  (-ACT_PANEL_OY)

#define ACT_UNION_OX  (4*8)
#define ACT_UNION_OY  (3*8)
#define ACT_ICON_OX   (ACT_UNION_OX+28)
#define ACT_ICON_OY   (ACT_UNION_OY)
#define ACT_MSG_OX    (9*8)
#define ACT_MSG_OY    (2*8)

///アイコンパターン数
enum{
  ICON_HELLO,
  ICON_BTL_START,
  ICON_BTL_WIN,
  ICON_POKE_GET,
  ICON_LV_UP,
  ICON_SHINKA,
  ICON_GPOWER,
  ICON_ITEM_GET,
  ICON_SPATIAL,
  ICON_UNION,
  ICON_CM,
  ICON_THANKS,
  ICON_MAX,
};

#define BMP_PANEL_OAM_SX  (15)
#define BMP_PANEL_OAM_SY  (2)

//スクロール方向定義
#define SCROLL_UP   (0)
#define SCROLL_DOWN (1)
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

// OAMフォントワーク
typedef struct {
	BMPOAM_ACT_PTR oam;
	GFL_BMP_DATA * bmp;
}FONT_OAM;

//パネル管理構造体
typedef struct _PANEL_WORK{
  u8  id; //パネルID
  u8  data_ofs; //データ参照オフセット
  u8  data_idx; //データ配列参照実index

  u8  n_line; //現在のライン
  s16 px;
  s16 py;
  GFL_CLWK* cPanel;
  GFL_CLWK* cUnion;
  GFL_CLWK* cIcon;
  FONT_OAM  msgOam;
	STRBUF* str;
	STRBUF* name;

  struct _PANEL_WORK* next;
  struct _PANEL_WORK* prev;
}PANEL_WORK;

typedef struct _LOG_CTRL{
  u8  max;  //ログ数
  u8  top;  //今描画されている先頭index
  u8  next_panel; //次にデータが来た時書き込むパネルindex
  u8  view_top;
  u8  view_btm;
  u8  view_max;

  u8  panel_list[PANEL_VIEW_MAX];
}LOG_CTRL;

///すれ違い通信状態参照画面管理ワーク
typedef struct _BEACON_VIEW{
  GAMESYS_WORK *gsys;
  GAMEDATA* gdata;
  FIELD_SUBSCREEN_WORK *subscreen;

  ////////////////////////////////////////
  BEACON_STATUS* b_status;
  GAMEBEACON_INFO_TBL*  infoStack;  //スタックワーク
  GAMEBEACON_INFO_TBL*  infoLog;    //ログテーブル
  GAMEBEACON_INFO*      tmpInfo;
  u16                   tmpTime;

  BOOL      active;
  int       seq;
  LOG_CTRL  ctrl;

  ////////////////////////////////////////
  HEAPID      heapID;
  HEAPID      tmpHeapID;
  ARCHANDLE*  arc_handle;

  GFL_TCBLSYS*  pTcbSys;

  GFL_FONT *fontHandle;
  PRINT_QUE *printQue;
  WORDSET *wordset;
  GFL_MSGDATA *mm_status;
  STRBUF *strbuf_temp;
  STRBUF *strbuf_expand;

  BMPOAM_SYS_PTR bmpOam;
  GFL_CLUNIT* cellUnit;
  GFL_CLSYS_REND* cellRender;
  GFL_CLACTPOS  cellSurfacePos;
  OBJ_RES_TBL objResNormal;
  OBJ_RES_TBL objResUnion;
  OBJ_RES_TBL objResIcon;

  GFL_BMPWIN *win[VIEW_LOG_MAX];
  PRINT_UTIL print_util[VIEW_LOG_MAX];
 
  PANEL_WORK  panel[PANEL_MAX];
  u32 log_count;


  //リソース
  RES2D_CHAR  resCharIcon[ICON_MAX];
  RES2D_CHAR  resCharUnion[UNION_CHAR_MAX];
  RES2D_PLTT  resPlttUnion;
  RES2D_PLTT  resPlttPanel;
}BEACON_VIEW;


