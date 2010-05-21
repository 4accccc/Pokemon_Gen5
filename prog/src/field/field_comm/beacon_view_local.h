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
#include "net/wih_dwc.h"
#include "field/field_comm/intrude_work.h"
#include "field/fld_vreq.h"
#include "system/palanm.h"
#include "system/gfl_use.h"
#include "system/bmp_oam.h"
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
  GFL_BMPWIN*   win;
  GFL_BMP_DATA* bmp;
  PRINT_UTIL  putil;
  u8          frm;
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
  u8  data_idx; //データ配列参照実index

  u8  n_line; //現在のライン
  u8  rank; //ランク
  u8  sex;  //性別
  u8  timeout_f;  //タイムアウトフラグ
  u16 tr_id;  //トレーナーID
  s16 px;
  s16 py;
  GFL_CLWK* cPanel;
  GFL_CLWK* cUnion;
  GFL_CLWK* cIcon;
  FONT_OAM  msgOam;
	STRBUF* str;
	STRBUF* name;

  const PRINT_QUE* que;
  GFL_TCBL* tcb_print;
  GFL_TCBL* tcb_icon;
}PANEL_WORK;

/*
 *  @brief  ポップアップ制御
 */
typedef struct _LOG_CTRL{
  u8  max;  //ログ数
  u8  view_top;
  u8  view_max;
  u8  target;

  u8  g_power;
  u8  mine_power_f;
}LOG_CTRL;

/*
 *  @brief  セーブデータ他パラメータ
 */
typedef struct _MY_DATA{
  GPOWER_ID power;
  u32       tr_id;
  u8        sex;
  u8        union_char;
  u8        pm_version;  ///<PM_VERSION
}MY_DATA;


///すれ違い通信状態参照画面管理ワーク
typedef struct _BEACON_VIEW{
  GAMESYS_WORK *gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldWork; 
  FIELD_SUBSCREEN_WORK *subscreen;
  FLD_VREQ* fld_vreq;
  MISC* misc_sv;
  MYITEM_PTR item_sv;
  GAME_COMM_SYS_PTR game_comm;
  WIFI_LIST*  wifi_list;
  POWER_CONV_DATA* gpower_data;

  ////////////////////////////////////////
  MY_DATA               my_data;
  
  BEACON_STATUS* b_status;
  GAMEBEACON_INFO_TBL*  infoStack;  //スタックワーク
  GAMEBEACON_INFO_TBL*  infoLog;    //ログテーブル
  GAMEBEACON_INFO*      tmpInfo;
  GAMEBEACON_INFO*      tmpInfo2;
  u16                   tmpTime;
  u16                   newLogTime;
  GAMEBEACON_INFO*      newLogInfo;
  PANEL_WORK*           newLogPanel;

  u8                    newLogOfs;
  u8                    newLogIdx;
  u8                    newLogWait;
  u8                    old_list_max;
  BOOL                  first_entry_f;

  int       active;
  BOOL      my_power_f;
  int       seq;
  int       sub_seq;
  u16       event_id;
  u16       event_reserve_f;

  int       io_interval;      ///<スタックチェックインターバルカウンタ
  u8        msg_spd;            ///<メッセージスピード
  u8        gpower_check_req;   ///<発動中のGパワー確認イベントリクエストフラグ
  u8        init_f;             ///<初期化中フラグ
  LOG_CTRL  ctrl;

  //リスト連続スクロールコントロール
  u8          scr_repeat_f;       //リピート中フラグ
  u8          scr_repeat_end;     //リピート終了フラグ
  u8          scr_repeat_ret_seq; //リピート終了後の戻り先seq
  SCROLL_DIR  scr_repeat_dir; //スクロール方向保存
  u32         scr_repeat_ct;  //リピートカウンタ

  ///特殊Gパワー発動リクエストチェック
  u8        sp_gpower_req_ct;
  u8        sp_gpower_req[ SP_GPOWER_REQ_MAX ];

  ////////////////////////////////////////
  HEAPID      heapID;
  HEAPID      tmpHeapID;
  HEAPID      heap_sID;
  ARCHANDLE*  arc_handle;

  GFL_TCBLSYS*  pTcbSys;
  GFL_TCB*      tcbVIntr;

  int           eff_task_ct;  //エフェクトタスク数

  PALETTE_FADE_PTR pfd;

  GFL_FONT *fontHandle;
  PRINT_QUE *printQue;
  APP_TASKMENU_RES* menuRes;
  APP_KEYCURSOR_WORK* key_cursor;
  TMENU_ITEM tmenu[TMENU_YN_MAX];
  TMENU_ITEM tmenu_check[TMENU_CHECK_MAX];

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
  
  BMP_WIN    win[ WIN_MAX ];
 
  PANEL_WORK  panel[PANEL_MAX];
  GFL_CLWK*   pAct[ACT_MAX];
  u32 log_count;

  //リソース
  RES2D_CHAR  resCharIcon[BEACON_ICON_MAX];
  RES2D_CHAR  resCharUnion[UNION_CHAR_MAX];
  RES2D_PLTT  resPlttUnion;
  RES2D_PLTT  resPlttPanel;

}BEACON_VIEW;

/*
 *  @brief  イベントリクエスト
 */
extern void BEACON_VIEW_SUB_EventReserve( BEACON_VIEW_PTR wk, BEACON_DETAIL_EVENT ev_id);

/*
 *  @brief  イベントリクエスト
 */
extern void BEACON_VIEW_SUB_EventRequest( BEACON_VIEW_PTR wk, BEACON_DETAIL_EVENT ev_id);

/*
 *  @brief  イベントリクエストリセット
 */
extern void BEACON_VIEW_SUB_EventReserveReset( BEACON_VIEW_PTR wk );

