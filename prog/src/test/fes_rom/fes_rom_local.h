//=============================================================================
/**
 *
 *	@file		fes_rom_local.h
 *	@brief  フェス配信ロム画面ローカルヘッダ
 *	@author	Miyuki Iwasawa
 *	@data		2012.03.29
 *
 */
//=============================================================================

#pragma once

#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_accessor.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/beacon_status.h"
#include "gamesystem/msgspeed.h"
#include "tr_tool/trtype_def.h"
#include "system/tr2dgra.h"
#include "savedata/intrude_save.h"
#include "net_app/union/union_beacon_tool.h"
#include "app/townmap_data_sys.h"
#include "app/townmap_util.h"
#include "msg/msg_fes_rom.h"

#include "sound/pm_sndsys.h"
#include "sound/swan_sound_data.sadl"

//タッチバー
#include "ui/touchbar.h"
// 簡易会話表示システム
#include "system/pms_draw.h"

#include "fes_rom_def.h"
#include "fes_rom_gra.h"

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

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

//オブジェリソース定義
typedef struct _OBJ_RES_SRC{
  u8  type, pltt_ofs, pltt_siz;
  u16 pltt_id;
  u16 cgr_id;
  u16 cell_id;
  u16 cell_anm_id;
}OBJ_RES_SRC;

//オブジェリソース管理構造体
typedef struct _OBJ_RES_TBL{
  u32 pltt;
  u32 cgr;
  u32 cell;
}OBJ_RES_TBL;


// Bmpウィンドウ
typedef struct{
  GFL_BMPWIN *win;
  GFL_BMP_DATA* bmp;
  PRINT_UTIL putil;
//  GFL_BMPWIN* pms;  ///簡易会話
//  GFL_CLWK* cRank;
}BMP_WIN;

typedef struct _TARGET_WIN{
  BMP_WIN base;
  BMP_WIN input;
  BMP_WIN msg;
}TARGET_WIN;

typedef struct _BMPWIN_AREA{
  u8  line;
  u8  sx,ex,sy;
}BMPWIN_AREA;

#define PARAM_SET_ARG_MAX (3)

typedef struct _INPUT_PARAM{
  int min,max,keta;
  WSET_TYPE type;
}INPUT_PARAM;

typedef struct _PARAM_SET{
  PARAM_ID    id;
  WSET_TYPE   wset_type;
  u16         msg;
  EXE_CODE    exe_code;
  INPUT_CODE  input_code;
  int         arg[PARAM_SET_ARG_MAX];
  BMPWIN_AREA area;
}PARAM_SET;

///数値で管理できないパラメータ
typedef struct _PARAM_EX{
  STRBUF* tr_name;
  STRBUF* greeting;
}PARAM_EX;

typedef struct _MEDIT_PRM_EX{
  s16 min,max;   //パラメータのmix,max
  WSET_TYPE wset_type;
}MEDIT_PRM_EX;

typedef struct _MEDIT_PRM_BASE_SET{
  INPUT_CODE  input_code;
  WSET_TYPE wset_type;
  const INPUT_CODE* tbl;
  int   type_num;
  const u8* type_tbl;
}MEDIT_PRM_BASE_SET;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;
  HEAPID tmpHeapID;

  FESROM_APP_MODE       app_mode; //アプリ動作モード
  int                   seq;
	int										sub_seq;
  int                   ret_seq;  //リターンシーケンス
  int                   fadein_seq;
  int                   fadeout_seq;
  int                   task_ct;
  int                   initial_f;
  BOOL                  data_error_f;

  FES_ROM_SAVE_WK*   save_wk;
  FES_ROM_SAVE*      sys_save;
  FES_ROM_PARAM*     sys_param;
  FES_ROM_SYS_WORK*  sys_wk;
  GFL_TCBLSYS*       pTcbSys;

  TARGET_WIN            target[TARGET_VIEW_MAX];
  int                   pos;      //現在のカーソル位置
  int                   ret_pos;  //戻りのカーソル位置
//  int                   prmset_idx;  //param id の参照開始index
  const PARAM_SET*      prmset_tbl; //パラメータセットテーブルの参照ポインタ
  int                   prmset_num; //パラメータセットの要素数
  int                   param[PARAM_ID_MAX+1];
  PARAM_EX              param_ex;

  int                   edit_id;     //現在エディット中のミッションデータNo
  FEST_MISSION          e_fmwk;      //編集用
  FEST_MISSION          tmp_fmwk;    //テンポラリ用
  FEST_MISSION          send_fmwk;   //送信用
  FMISSION_DATA_MAN*    fm_data_man;
  FMISSION_ARCDATA*     fm_arc_tbl;
  FEST_MISSION_ID       sub_id;
  const MEDIT_PRM_BASE_SET* p_base_set;
  FMISSION_ARCDATA*     p_fmarc;

  int                   send_id;  //送信データNo
  int                   send_odds;
  int                   send_id_ct; //送信IDカウンター
  int                   send_time_sec;   //送信継続時間
  int                   send_lest_sec;   //送信継続時間
  int                   send_blank_sec;  //送信終了後インターバル時間
  int                   send_diff_sec;
  int                   send_exit_f;
  const FESROM_MISSION* send_m;
  s64                   send_rtc;

  s64                   old_date_time;
//  RTCDate               date;
//  RTCTime               time;

  u8                    flip_sw;
  u8                    list_max;
  u8                    list_top;
  u8                    list_top_ini;
  u8                    list[GAMEBEACON_INFO_TBL_MAX];

//  u8                    msg_spd;

	//描画設定
	FES_ROM_GRAPHIC_WORK	*graphic;
  ARCHANDLE* handle;
  ARCHANDLE* h_trgra;
  OBJ_RES_TBL objResNormal;
  OBJ_RES_TBL objResUnion;
  
  RES2D_CHAR  resCharUnion[UNION_CHAR_MAX];
  RES2D_PLTT  resPlttUnion;
  RES2D_PLTT  resPlttPanel;

	//タッチバー
	TOUCHBAR_WORK							*touchbar;

	//フォント
	GFL_FONT									*font;

	//プリントキュー
	PRINT_QUE									*print_que;
	GFL_MSGDATA								*msgman;
	GFL_MSGDATA								*msg_area;
  WORDSET                   *wset;
  STRBUF *str_tmp;
  STRBUF *str_expand;
  STRBUF *str_popup;  //ポップアップ用テンポラリ

#ifdef	BEACON_DETAIL_PRINT_TOOL
	//プリントユーティリティ
	PRINT_UTIL								print_util;
#endif	//BEACON_DETAIL_PRINT_TOOL

  BMP_WIN         win_popup;
  PMS_DRAW_WORK*  pms_draw;
 
  s16   icon_x,icon_y;
  u8    icon_enable_f;
  GFL_CLWK* pAct[ACT_MAX];

} FES_ROM_WORK;

extern void FES_ROM_SetFadeInOutSeq( FES_ROM_WORK* wk, int fadein_seq, int fadeout_seq );


