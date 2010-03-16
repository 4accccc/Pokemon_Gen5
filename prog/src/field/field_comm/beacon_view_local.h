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
#include "gamesystem/g_power.h"
#include "system/palanm.h"
#include "system/gfl_use.h"
#include "savedata/intrude_save.h"
#include "savedata/misc.h"
#include "savedata/myitem_savedata.h"
#include "app/app_taskmenu.h"
#include "app/app_keycursor.h"
#include "item/itemsym.h"
#include "msg/msg_beacon_status.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include "beacon_view_def.h"

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
//  u8  top;  //今描画されている先頭index
//  u8  next_panel; //次にデータが来た時書き込むパネルindex
  u8  view_top;
//  u8  view_btm;
  u8  view_max;
  u8  target;

  u8  g_power;
  u8  mine_power_f;

 // u8  panel_list[PANEL_VIEW_MAX];
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
  MISC* misc_sv;
  MYITEM_PTR item_sv;

  ////////////////////////////////////////
  BEACON_STATUS* b_status;
  GAMEBEACON_INFO_TBL*  infoStack;  //スタックワーク
  GAMEBEACON_INFO_TBL*  infoLog;    //ログテーブル
  GAMEBEACON_INFO*      tmpInfo;
  u16                   tmpTime;
  MY_DATA               my_data;

  BOOL      active;
  BOOL      my_power_f;
  int       seq;
  int       sub_seq;
  int       event_id;

  int       io_interval;
  u8        menu_active;
  u8        msg_spd;
  LOG_CTRL  ctrl;

  ////////////////////////////////////////
  HEAPID      heapID;
  HEAPID      tmpHeapID;
  HEAPID      heap_sID;
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

#ifdef PM_DEBUG
  u32 deb_stack_check_throw;
#endif
}BEACON_VIEW;


