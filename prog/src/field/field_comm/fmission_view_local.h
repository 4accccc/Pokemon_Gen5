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
#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission.h"
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
#include "msg/msg_fmission_log.h"

#include "sound/pm_sndsys.h"
#include "sound/swan_sound_data.sadl"

#include "fmission_view_def.h"
#include "beacon_view_common.h"

//==============================================================================
//  構造体定義
//==============================================================================

//パネル管理構造体
typedef struct _PANEL_WORK{
  u8  id; //パネルID
  u8  data_idx; //データ配列参照実index

  u8  n_line; //現在のライン
  u8  sex:1;  //性別
  u8  type:7; //パネルタイプ

  u16 tr_id;  //トレーナーID
  u16 act_prm;  //<アクションパラメータ
  s16 px;
  s16 py;
  GFL_CLWK* cPanel;
  GFL_CLWK* cUnion;
  FONT_OAM  msgOam[PANEL_FOAM_MAX];
	STRBUF* str[PANEL_FOAM_MAX];
  GFL_TCBL* tcb_print[PANEL_FOAM_MAX];

  const PRINT_QUE* que;
  GFL_TCBL* tcb_print_name;
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

  u8  fm_entry;
  u32 fm_my_score;
  u32 fm_score;
  u32 fm_sec;
  FEST_MISSION mission; //フェスタミッション
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
typedef struct _FMISSION_VIEW{
  GAMESYS_WORK *gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldWork; 
  FIELD_SUBSCREEN_WORK *subscreen;
  FLD_VREQ* fld_vreq;
  MISC* misc_sv;
  MYITEM_PTR item_sv;
  GAME_COMM_SYS_PTR game_comm;
  WIFI_LIST*  wifi_list;
  LINK_FESTIVAL*  fest_sys;
  FEST_MISSION*  fmwk;
  POWER_CONV_DATA* gpower_data;

  ////////////////////////////////////////
  MY_DATA               my_data;
 
  BEACON_STATUS* b_status;
  FMISSION_LOG_TBL*     logStack;  //スタックワーク
  FMISSION_LOG_TBL*     logView;   //ログテーブル
  FMISSION_LOG*         tmpLog;
  FMISSION_LOG*         tmpLog2;
  FMISSION_LOG*         newLog;
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
  u8        event_reserve_f;
  u8        event_reserve_exec;

  int       io_interval;      ///<スタックチェックインターバルカウンタ
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

  int           eff_task_ct;      //エフェクトタスク数
  int           en_print_task_ct;   //エントリー数プリントタスク数

  PALETTE_FADE_PTR pfd;

  GFL_FONT *fontHandle;
//  GFL_FONT *sfontHandle;
  PRINT_QUE *printQue;
  APP_TASKMENU_RES* menuRes;
  APP_KEYCURSOR_WORK* key_cursor;
  BVC_TMENU_WORK* tmenu;

  WORDSET *wordset;
  GFL_MSGDATA *mm_sys;
  GFL_MSGDATA *mm_fmission;
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

}FMISSION_VIEW;

#include "fmission_view_setup.h"
