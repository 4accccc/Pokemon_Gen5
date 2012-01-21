//===============================================================================
/*
 *  @file event_debug_livecomm.c
 *  @brief  リンクフェスティバルデバッグ メニューイベント
 *  @author Miyuki Iwasawa
 *  @date   10.04.19
 */
//===============================================================================

#ifdef PM_DEBUG

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "font/font.naix"

#include "field/field_msgbg.h"
#include "fieldmap.h"
#include "field/zonedata.h"

#include "sound/pm_sndsys.h"

#include "field_sound.h"
#include "field/eventwork.h"

#include "event_debug_local.h"
#include "debug/debug_str_conv.h" // for DEB_STR_CONV_SjisToStrcode
#include "print/wordset.h"  //WORDSET
#include "net/net_whpipe.h"
#include "app/townmap_util.h"
#include "app/name_input.h"
#include "net_app/union/union_beacon_tool.h"

#include "debug_message.naix"
#include "msg/debug/msg_d_numinput.h"
#include "msg/debug/msg_d_linkfest.h"

#include "arc/fieldmap/zone_id.h"
#include "arc/fieldmap/debug_symbol.naix"  //DEBUG_SYM_
#include "arc/fieldmap/debug_list.h"  //DEBUG_SCR_

#include "poke_tool/monsno_def.h"
#include "tr_tool/trno_def.h"
#include "item/itemsym.h"
#include "waza_tool/wazano_def.h"

#include "savedata/intrude_save.h"
#include "savedata/playtime.h"
#include "savedata/fmission_save.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "../../../resource/fldmapdata/flagwork/work_define.h"

#include "gamesystem/link_festival.h"
#include "gamesystem/link_festival_log.h"
#include "gamesystem/fest_mission_local.h"
#include "gamesystem/fest_mission_data.h"
#include "gamesystem/fmission_log_local.h"
#include "gamesystem/fmission_log.h"
#include "gamesystem/fmission_log_accessor_fv.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/game_beacon_local.h" //許可なく他でinclude禁止
#include "field/beacon_view.h"
#include "field/field_comm/beacon_view_local.h"
#include "event_debug_livecomm.h"
#include "event_debug_linkfest.h"
#include "event_fmission.h"
#include "script_def.h"
#include "fes_gimmick.h"
#include "fes_gimmick_item.h"
#include "fes_gimmick_npc.h"
#include "fes_gimmick_local.h"

//======================================================================
//
//定数定義
//
//======================================================================
typedef enum{
  SEQ_MENU_INIT,
  SEQ_MENU_MAIN,
  SEQ_EXIT,
}SEQ_ID;

enum{
 MENU_UPDATE_STOP,
 MENU_BEACON_REQ,
 MENU_STACK_CHECK,
 MENU_STACK_CLEAR,
 MENU_RECV_BUF_CHECK,
 MENU_RECV_BUF_CLEAR,
 MENU_MY_GPOWER,
 MENU_MISSION_END,
 MENU_MISSION_MEMBER_CHECK,
 MENU_MISSION_EVPOS_CHECK,
 MENU_MISSION_EVFLAG_CHECK,
 MENU_TUTORIAL_CLEAR,
 MENU_MISSION_OPEN_FLAG_SET,
 MENU_MISSION_ENTRY_FLAG_SET,
 MENU_MISSION_ENTRY_STACK_CHECK,
 //フィールド
 MENU_MISSION_BEACON_WAIT,
 MENU_MISSION_LEVEL_OPEN,
 MENU_MISSION_OPEN_NUM_SET,
 MENU_MISSION_JOIN_NUM_SET,
 MENU_MISSION_JOIN_NUM_MAX_SET,
 MENU_MISSION_CLEAR_NUM_SET,
 MENU_MISSION_TOP_SCORE_NUM_SET,
 MENU_MISSION_HIGH_SCORE_TOTAL_ALL_SET,
 MENU_MISSION_HIGH_SCORE_MINE_ALL_SET,
 MENU_MISSION_NEW_FLAG_ALL_SET,
 MENU_MISSION_EXP_WHITE_SET,
 MENU_MISSION_EXP_BLACK_SET,
 //ログ
 MENU_MISSION_MY_SCORE,
 MENU_MISSION_TIME_STOP,
 MENU_MISSION_SPOT_MODE,
 MENU_MISSION_MEMBER_NUM,
 MENU_EXIT,
};

/////////////////////////////////////////////
///パレット
#define PANO_FONT (14)
#define FBMP_COL_WHITE  (15)
#define FCOL_WIN_BASE (15)
#define FCOL_WIN_W  ( PRINTSYS_MACRO_LSB(1,2,FCOL_WIN_BASE) ) ///<BG白抜き
#define FCOL_WIN_BLUE  ( PRINTSYS_MACRO_LSB(5,6,FCOL_WIN_BASE) ) ///<BG青抜き

#define BUFLEN_BEACON_MSG (8*2*2+EOM_SIZE)

/////////////////////////////////////////////
///スタックチェック
#define SCHECK_LINE_MAX (4)
#define SCHECK_LINE_OY  (16*3)
#define SCHECK_PAGE_MAX (8)

/////////////////////////////////////////////
//データチェックモード定義
typedef enum{
 DATA_CHECK_MODE_MEMBER,
 DATA_CHECK_MODE_EVPOS,
 DATA_CHECK_MODE_EVFLAG,
}DATA_CHECK_MODE;

/////////////////////////////////////////////
///メンバーチェック
#define MEMBER_CHECK_LINE_MAX (5)
#define MEMBER_CHECK_LINE_OY  (16*2)
#define MEMBER_CHECK_PAGE_MAX (20)

/////////////////////////////////////////////
///フェスログビーコンリクエスト
#define LOGREQ_LINE_MAX (3+FAKE_DATA_MAX)
#define LOGREQ_WIN_MAIN_SX  (31)

#define LOGREQ_FWAZA_MAX  (10)

typedef enum{
 TR_PAT_MINE,
 TR_PAT_FAKE,
 TR_PAT_MAX,
}TR_PATTERN;

typedef enum{
  FAKE_ID,
  FAKE_NAME,
  FAKE_ZONE,
  FAKE_SEX,
  FAKE_VERSION,
  FAKE_LANG,
  FAKE_SCORE,
  FAKE_TR_VIEW,
  FAKE_DATA_MAX,
}FAKE_DATA_ID;

typedef enum{
  BPRM_WSET_NONE,
  BPRM_WSET_TRNAME,
  BPRM_WSET_TRNAME_EX,
  BPRM_WSET_MONSNAME,
  BPRM_WSET_ITEMNAME,
  BPRM_WSET_FWAZA,
  BPRM_WSET_GPOWER,
  BPRM_WSET_ACTION,
  BPRM_WSET_LOG_REQ_TYPE,
  BPRM_WSET_SEX,
  BPRM_WSET_VERSION,
  BPRM_WSET_LANG,
  BPRM_WSET_ZONE,
  BPRM_WSET_FREEWORD,
  BPRM_WSET_TR_VIEW,
}BPRM_WSET_TYPE;

typedef enum{
 LOG_PSET_DEFAULT,     //デフォルト(トレーナー名)  
 LOG_PSET_MONSNO,      //ポケモン種族名
 LOG_PSET_ITEM,        //アイテム名
 LOG_PSET_GPOWER,      //デルパワー
 LOG_PSET_WAZA,        //技
 LOG_PSET_FREEWORD,    //フリーワード
 LOG_PSET_MAX,
}LOG_PSET_TYPE;

typedef enum{
 BEACON_ARG_NONE,
 BEACON_ARG_U8,
 BEACON_ARG_U16,
 BEACON_ARG_U32,
 BEACON_ARG_STR,
 BEACON_ARG_U16_STR,
 BEACON_ARG_GDATA_U32,
 BEACON_ARG_FWORD,
 BEACON_ARG_GPOWER,
 BEACON_ARG_FWAZA,
}BEACON_ARG_TYPE;

typedef enum{
 NINPUT_STACK_CHECK_THROW,
 NINPUT_GPOWER,
 //フィールドメニュ
 NINPUT_MISSION_BEACON_WAIT,
 NINPUT_MISSION_LEVEL_OPEN,
 NINPUT_MISSION_OPEN_NUM_SET,
 NINPUT_MISSION_JOIN_NUM_SET,
 NINPUT_MISSION_JOIN_NUM_MAX_SET,
 NINPUT_MISSION_CLEAR_NUM_SET,
 NINPUT_MISSION_TOP_SCORE_NUM_SET,
 NINPUT_MISSION_HIGH_SCORE_TOTAL_ALL_SET,
 NINPUT_MISSION_HIGH_SCORE_MINE_ALL_SET,
 NINPUT_MISSION_NEW_FLAG_ALL_SET,
 NINPUT_MISSION_EXP_WHITE_SET,
 NINPUT_MISSION_EXP_BLACK_SET,
 //ログメニュー
 NINPUT_MISSION_MY_SCORE,
 NINPUT_MISSION_TIME_STOP,
 NINPUT_MISSION_SPOT_MODE,
 NINPUT_MISSION_MEMBER_NUM,
}NINPUT_ID;

typedef void (*BEACON_SET_FUNC_NONE)(void);
typedef void (*BEACON_SET_FUNC_U8)(u8);
typedef void (*BEACON_SET_FUNC_U16)(u16);
typedef void (*BEACON_SET_FUNC_U32)(u32);
typedef void (*BEACON_SET_FUNC_STR)(const STRBUF*);
typedef void (*BEACON_SET_FUNC_U16_STR)(u16,const STRBUF*);
typedef void (*BEACON_SET_FUNC_GDATA_U32)(GAMEDATA* gdata,u32);
typedef void (*BEACON_SET_FUNC_GDATA_STR)(GAMEDATA* gdata, const STRBUF*);
typedef void (*BEACON_SET_FUNC_GPOWER)(GPOWER_ID);

//======================================================================
//
//構造体定義
//
//======================================================================
typedef struct _D_LOG_PARAM{
  void* func;
  LOG_PSET_TYPE  prm_type; //パラメータタイプ
  BEACON_ARG_TYPE   arg_type; //引数の型タイプ
}D_LOG_PARAM;

typedef struct _D_FAKE_PRM_SET{
  BPRM_WSET_TYPE  wset_type;
  int keta;
  int min,max;
}D_FAKE_PRM_SET;

typedef struct _D_LOG_PRM_SET{
  int min,max;   //パラメータのmix,max
  BPRM_WSET_TYPE wset_type;
}D_LOG_PRM_SET;

typedef struct _D_BEACON_REQ{
  TR_PATTERN  trainer;  ///トレーナーのパターン
  u32         tr_id;
  GAMEBEACON_ACTION action;

  u16 tr_no;
  u16 mons_no;
  u16 item_no;
  u16 ptime_h;  //プレイ時間(時)
}D_BEACON_REQ;

typedef struct _DMENU_LINK_FEST{
  HEAPID  heapID;
  HEAPID  tmpHeapID;

  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldmap;

  SAVE_CONTROL_WORK* save;
  INTRUDE_SAVE_WORK* int_sv;
  MYITEM_PTR item_sv;
  MISC* misc_sv;
  MYSTATUS* my_status;
  FMISSION_SAVE* fmission_sv;

  LINK_FESTIVAL* fest_sys;
  FEST_MISSION* fmwk;

  BEACON_VIEW*    view_wk;
  BEACON_STATUS* b_status;
  GAMEBEACON_INFO_TBL*  infoStack;  //スタックワーク
  GAMEBEACON_INFO_TBL*  infoLog;    //ログテーブル
 
  GFL_FONT* fontHandle;
	GFL_MSGDATA* msgman;						//メッセージマネージャー
  WORDSET* wset;
  STRBUF *str_tmp;
  STRBUF *str_expand;
  const char* pAllZoneName;
  FLDMENUFUNC *menuFunc;

  int               log_prm[LOG_PSET_MAX];
  int               fake_prm[TR_PAT_MAX][FAKE_DATA_MAX];
  STRBUF*           beacon_nickname;
  STRBUF*           beacon_fword;

  GAMEBEACON_INFO*  tmpInfo;
  u16               tmpTime;
  BMP_WIN           win_ninput;
}DMENU_LINK_FEST;

typedef int (*NINPUT_GET_FUNC)( DMENU_LINK_FEST* wk, int param );
typedef void (*NINPUT_SET_FUNC)( DMENU_LINK_FEST* wk, int param, int value );

/// 数値入力：パラメータ
typedef struct {
  int min;  ///<入力最小値
  int max;  ///<入力最大値
  int* ret_wk;
  ///値を取得するための関数
  NINPUT_SET_FUNC set_func;
  NINPUT_GET_FUNC get_func;
}NINPUT_PARAM;

typedef struct{
  int min, max;
}NINPUT_MINMAX;

//======================================================================
//
//データ定義
//
//======================================================================

static const DEBUG_MENU_LIST DATA_DebugLinkFestMenuList[] =
{
  { dlf_menu_01, (void*)MENU_MISSION_MY_SCORE, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_02, (void*)MENU_MISSION_TIME_STOP, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_03, (void*)MENU_MISSION_END, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_04, (void*)MENU_BEACON_REQ, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_05, (void*)MENU_UPDATE_STOP, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_06, (void*)MENU_STACK_CHECK, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_07, (void*)MENU_STACK_CLEAR, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_08, (void*)MENU_MISSION_MEMBER_NUM, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_09, (void*)MENU_MISSION_MEMBER_CHECK, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_10, (void*)MENU_MISSION_EVPOS_CHECK, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_11, (void*)MENU_MISSION_EVFLAG_CHECK, GFL_OVERLAY_BLANK_ID },

//  { dlf_menu_08, (void*)MENU_RECV_BUF_CHECK, GFL_OVERLAY_BLANK_ID },
//  { dlf_menu_09, (void*)MENU_RECV_BUF_CLEAR, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_12, (void*)MENU_MISSION_SPOT_MODE, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_exit, (void*)MENU_EXIT, GFL_OVERLAY_BLANK_ID },
};

static const DEBUG_MENU_LIST DATA_DebugLinkFestFromFieldMenuList[] =
{
  { dlf_fldmenu_01, (void*)MENU_TUTORIAL_CLEAR, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_02, (void*)MENU_MISSION_OPEN_FLAG_SET,   GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_03, (void*)MENU_MISSION_ENTRY_FLAG_SET,   GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_04, (void*)MENU_MISSION_ENTRY_STACK_CHECK, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_05, (void*)MENU_MISSION_LEVEL_OPEN, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_06, (void*)MENU_MISSION_OPEN_NUM_SET, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_07, (void*)MENU_MISSION_JOIN_NUM_SET, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_08, (void*)MENU_MISSION_JOIN_NUM_MAX_SET, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_09, (void*)MENU_MISSION_CLEAR_NUM_SET, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_10, (void*)MENU_MISSION_TOP_SCORE_NUM_SET, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_11, (void*)MENU_MISSION_HIGH_SCORE_TOTAL_ALL_SET, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_12, (void*)MENU_MISSION_HIGH_SCORE_MINE_ALL_SET, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_13, (void*)MENU_MISSION_NEW_FLAG_ALL_SET, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_14, (void*)MENU_MISSION_EXP_WHITE_SET, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_15, (void*)MENU_MISSION_EXP_BLACK_SET, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_16, (void*)MENU_MISSION_BEACON_WAIT, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_exit, (void*)MENU_EXIT, GFL_OVERLAY_BLANK_ID },
};

static const DEBUG_MENU_INITIALIZER DebugLinkFestMenuData = {
  NARC_debug_message_d_linkfest_dat,
  NELEMS(DATA_DebugLinkFestMenuList),
  DATA_DebugLinkFestMenuList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 30, 17,
  NULL,
  NULL
};
static const DEBUG_MENU_INITIALIZER DebugLinkFestFromFieldMenuData = {
  NARC_debug_message_d_linkfest_dat,
  NELEMS(DATA_DebugLinkFestFromFieldMenuList),
  DATA_DebugLinkFestFromFieldMenuList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 30, 17,
  NULL,
  NULL
};
/*
 *  @brief  FMISSION_LOG_ACTIOONの並びと同一である必要があります
 */
static const D_LOG_PARAM DATA_DebugLogParam[FMISSION_LOG_ACT_MAX] = {
  ///< 0
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  ///< 1
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  ///< 2
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  ///< 3
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },

  ///<FMISSION_LOG_ACT_POKE_ENCOUNT  [ポケモン名]とエンカウントした 4
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },
  ///<FMISSION_LOG_ACT_POKE_WIN  [ポケモン名]を倒した 5
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, }, 
  ///<FMISSION_LOG_ACT_POKE_CAPTURE [ポケモン名]を捕まえた 6
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_U16,	},
  /// FMISSION_LOG_ACT_POKE_ESCAPE [ポケモン名]に逃げられたor逃げた 7
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_TR_ENCOUNT トレーナー戦開始 8
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_TR_WIN トレーナー戦勝利 9
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_GAMEOVER 全滅した 10 
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_ITEM_USE [アイテム名]を使用した 11 
  { NULL,                                 LOG_PSET_ITEM,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_ITEM_GET [アイテム名]を手に入れた 12
  { NULL,                                 LOG_PSET_ITEM,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_FIELD_SKILL [フィールド技]を使った 13
  { NULL,                                 LOG_PSET_WAZA,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_SHOPPING 買い物中 14
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_GPOWER デルパワーを使った 15
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_FERRIS_WHEEL 観覧車に乗った 16
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_WATCH_SIGNBOARD 看板を調べている 17
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// MISSION_LOG_ACT_SODATEYA_EGG タマゴを受け取った 18
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_EFF_ENC_WIN [ポケモン名]を倒した(揺れ草) 19
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_EFF_ENC_CAPTURE [ポケモン名]を捕まえた(揺れ草) 20
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_MUSHROOM_WIN [ポケモン名]を倒した(キノコエンカウント) 21
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_MUSHROOM_CAPTURE [ポケモン名]を捕まえた(キノコエンカウント) 22
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_KIRA_ITEM_GET [アイテム名]を見つけた(エフェクトアイテム) 23
  { NULL,                                 LOG_PSET_ITEM,	  BEACON_ARG_NONE, },

  /// FMISSION_LOG_ACT_TOWER_OF_HEAVEN 鐘を鳴らした 24
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_FEATHER_GET [アイテム名]を見つけた(羽) 25
  { NULL,                                 LOG_PSET_ITEM,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_JEWEL_GET [アイテム名]を見つけた(ジュエルと石) 26
  { NULL,                                 LOG_PSET_ITEM,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_EGG_HATCH [ポケモン名]が生まれた 27
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_FISHING_ENCOUNT [ポケモン名]を釣り上げた 28
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_ANIMAL_TRAIL_ENCOUNT [ポケモン名]を見つけた(獣道) 29
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },

  /// FMISSION_LOG_ACT_HELP_CHILD まいごを助けた 30
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_PRESENT_ITEM <[アイテム名]を渡した 31
  { NULL,                                 LOG_PSET_ITEM,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_ITEM_TRADE 道具交換成功 32
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_ITEM_BUY [アイテム名]を買った 33
  { NULL,                                 LOG_PSET_ITEM,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_RPS_WIN じゃんけんに買った 34
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_MEMORY_QUIZ_CLEAR 記憶クイズに正解した 35
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_TR_BATTLE_WIN トレーナーを倒した 36
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_GET_A_PRESENT [アイテム名]をもらった 37
  { NULL,                                 LOG_PSET_ITEM,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_POKE_QUIZ_CLEAR 図鑑クイズに正解 38
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_RUN_OUT_OF_MONEY お金が足りない 39
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_RPS_LOSE じゃんけんに負けた 40
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_QUIZ_WRONG_ANSWER クイズに不正解(記憶・図鑑クイズ共用) 41
  { NULL,                                 LOG_PSET_DEFAULT,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_POKE_DOWN [ポケモン名]が瀕死 42
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_POKE_HP_LITTLE [ポケモン名]のHPが少ない 43
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_POKE_PP_LITTLE [ポケモン名]のPPが少ない 44
  { NULL,                                 LOG_PSET_MONSNO,	  BEACON_ARG_NONE, },
  /// FMISSION_LOG_ACT_FREEWORD 一言メッセージ 45
  { NULL,                                 LOG_PSET_FREEWORD,	  BEACON_ARG_NONE, },
};

static const  D_LOG_PRM_SET DATA_DebugLogParamSet[LOG_PSET_MAX] = {
 { -1, -1, BPRM_WSET_NONE,}, //LOG_PSET_DEFAULT, デフォルト
 { MONSNO_HUSIGIDANE, MONSNO_END, BPRM_WSET_MONSNAME }, //LOG_PSET_MONSNO,    ポケモン種族名
 { ITEM_MASUTAABOORU, ITEM_DATA_MAX, BPRM_WSET_ITEMNAME }, //LOG_PSET_ITEM,       アイテム名
 { 0, GPOWER_ID_CAPTURE_MAX, BPRM_WSET_GPOWER }, //LOG_PSET_GPOWER,      Gパワー名
 { 0, LOGREQ_FWAZA_MAX-1, BPRM_WSET_FWAZA }, //LOG_PSET_WAZA,        技名
 { 0, NAMEIN_SHORTCUT_RET_MAX-1, BPRM_WSET_FREEWORD }, //LOG_PSET_FREEWORD,    フリーワード8文字
};

static const D_FAKE_PRM_SET DATA_FakeParamSet[] = {
  { BPRM_WSET_NONE, 5, 0, 0xFFFF,}, //FAKE_ID,
  { BPRM_WSET_TRNAME_EX, 3, 0, TRID_MAX-1,}, //FAKE_NAME,
  { BPRM_WSET_ZONE, 3, 0, ZONE_ID_MAX-1,}, //FAKE_ZONE,
  { BPRM_WSET_SEX, 1, PM_MALE, PM_FEMALE,}, //FAKE_SEX,
  { BPRM_WSET_VERSION, 3, VERSION_W2, VERSION_B2,}, //FAKE_VERSION,
  { BPRM_WSET_LANG, 1, LANG_JAPAN, LANG_KOREA,}, //FAKE_LANG,
  { BPRM_WSET_NONE, 4, 0, 9999}, //FAKE_SCORE
  { BPRM_WSET_TR_VIEW, 2, 0, 15}, //FAKE_TR_VIEW,
};

////////////////////////////////////////////////
//フィールド技リスト
static const u16 DATA_FieldWazaTbl[LOGREQ_FWAZA_MAX] = {
  WAZANO_IAIGIRI,
  WAZANO_SORAWOTOBU,
  WAZANO_NAMINORI,
  WAZANO_KAIRIKI,
  WAZANO_ANAWOHORU,
  WAZANO_TEREPOOTO,
  WAZANO_TAKINOBORI,
  WAZANO_HURASSYU,
  WAZANO_DAIBINGU,
  WAZANO_OSYABERI,
};

//================================================
//数値入力パラメータ
static const NINPUT_MINMAX DATA_NInputMinMax[] = {
 { 0, 9999 },  //MENU_MISSION_MY_SCORE:
 { 0, 1 },  //MENU_MISSION_TIME_STOP:
 { 0, 1 },  //MENU_MISSION_SPOT_MODE:
 { 0, 99 },  //MENU_MISSION_MEMBER_NUM:
};
static const NINPUT_MINMAX DATA_NInputFieldMinMax[] = {
 { 3, 60 },  //MENU_MISSION_BEACON_WAIT:
 { 0, 4 },  //MENU_MISSION_LEVEL_OPEN:
 { 0, 9999 }, // MENU_MISSION_OPEN_NUM_SET:
 { 0, 9999 }, // MENU_MISSION_JOIN_NUM_SET:
 { 0, LINKFEST_JOIN_MAX }, // MENU_MISSION_JOIN_NUM_MAX_SET:
 { 0, 9999 }, // MENU_MISSION_CLEAR_NUM_SET:
 { 0, 9999 }, // MENU_MISSION_TOP_SCORE_NUM_SET:
 { 0, FMISSION_SCORE_MAX }, // MENU_MISSION_HIGH_SCORE_TOTAL_ALL_SET:
 { 0, FMISSION_SCORE_MAX }, // MENU_MISSION_HIGH_SCORE_MINE_ALL_SET:
 { 0, 1 }, // MENU_MISSION_NEW_FLAG_ALL_SET:
 { 0, FMISSION_SV_HILINK_LEVEL_EXP_STEP-1 }, // MENU_MISSION_EXP_WHITE_SET:
 { 0, FMISSION_SV_HILINK_LEVEL_EXP_STEP-1 }, // MENU_MISSION_EXP_BLACK_SET:
};

//======================================================================
//
//プロトタイプ
//
//======================================================================

static GMEVENT_RESULT event_LinkFestMain( GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT event_LinkFestFromFieldMain( GMEVENT * event, int *seq, void * work);
static void dmenu_RefarenceGet( DMENU_LINK_FEST* wk, GAMESYS_WORK* gsys );
static void dmenu_WorkInit( DMENU_LINK_FEST* wk );
static void dmenu_WorkRelease( DMENU_LINK_FEST* wk );

static void dmenu_TutorialClear( DMENU_LINK_FEST* wk );
static void dmenu_MissionEvFlagSet( DMENU_LINK_FEST* wk, int mode );

static void sub_BmpWinAdd( BMP_WIN* bmpwin, u8 px, u8 py, u8 sx, u8 sy );
static void sub_BmpWinDel( BMP_WIN* bmpwin );
static void sub_GetMsgToBuf( DMENU_LINK_FEST* wk, u8 msg_id );
static inline void sub_FreeWordset( WORDSET* wset, u8 idx, STRBUF* str );
static void sub_LogParamWordset( DMENU_LINK_FEST* wk, u8 idx, int value, BPRM_WSET_TYPE type );

static GMEVENT* event_CreateEventNumInput( DMENU_LINK_FEST* wk, int key, int min, int max, NINPUT_SET_FUNC set_func, NINPUT_GET_FUNC get_func );
static GMEVENT* event_CreateEventNumInputRetWork( DMENU_LINK_FEST* wk, int key, int min, int max, int* ret_wk, BPRM_WSET_TYPE wset_type );
static GMEVENT* event_CreateEventStackCheck( GAMESYS_WORK* gsys, DMENU_LINK_FEST* wk, GAMEBEACON_INFO_TBL* info_tbl );
static GMEVENT* event_CreateEventDataCheck( GAMESYS_WORK* gsys, DMENU_LINK_FEST* wk, DATA_CHECK_MODE mode );
static GMEVENT* event_CreateEventLogBeaconReq( GAMESYS_WORK* gsys, DMENU_LINK_FEST* wk );

static int ninput_GetMiscCount( DMENU_LINK_FEST* wk, int param );
static void ninput_SetMiscCount( DMENU_LINK_FEST* wk, int param, int value );
static int ninput_GetDebugParam( DMENU_LINK_FEST* wk, int param );
static void ninput_SetDebugParam( DMENU_LINK_FEST* wk, int param, int value );

//======================================================================
//
//関数定義
//
//======================================================================

//--------------------------------------------------------------
/**
 * リンクフェスティバルデバッグメニューイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 */
//--------------------------------------------------------------
GMEVENT* DEBUG_EVENT_LinkFest( GAMESYS_WORK* gsys, void* parent_work )
{
  DMENU_LINK_FEST* wk;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, event_LinkFestMain, sizeof(DMENU_LINK_FEST) );

  wk = GMEVENT_GetEventWork( event );
  wk->view_wk = (BEACON_VIEW*)parent_work;
  
  dmenu_RefarenceGet( wk, gsys );
  dmenu_WorkInit( wk );

  return event;
}

//--------------------------------------------------------------
/**
 * リンクフェスティバルデバッグメニュー トップメニューイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 */
//--------------------------------------------------------------
GMEVENT* DEBUG_EVENT_LinkFestFromField( GAMESYS_WORK* gsys,void* parent_work )
{
  DMENU_LINK_FEST* wk;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, event_LinkFestFromFieldMain, sizeof(DMENU_LINK_FEST) );

  wk = GMEVENT_GetEventWork( event );
  wk->view_wk = NULL; //(BEACON_VIEW*)parent_work;

  dmenu_RefarenceGet( wk, gsys );
  dmenu_WorkInit( wk );

  return event;
}

//-----------------------------------------------------------------
/*
 *  @brief  リンクフェスタデバッグメニューワーク 各種参照初期化
 */
//-----------------------------------------------------------------
static void dmenu_RefarenceGet( DMENU_LINK_FEST* wk, GAMESYS_WORK* gsys )
{
  wk->gsys = gsys; 
  wk->gdata = GAMESYSTEM_GetGameData( gsys ); 
  wk->b_status = GAMEDATA_GetBeaconStatus( wk->gdata );
  wk->fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  wk->save = GAMEDATA_GetSaveControlWork( wk->gdata ); 
  wk->int_sv = SaveData_GetIntrude( wk->save );
  wk->item_sv = GAMEDATA_GetMyItem( wk->gdata );
  wk->misc_sv = (MISC*)SaveData_GetMiscConst( wk->save );
  wk->my_status = GAMEDATA_GetMyStatus( wk->gdata );
  wk->fmission_sv = SAVEDATA_GetFMissionSave( wk->save );

  wk->fest_sys = GAMESYSTEM_GetLinkFestivalWork( gsys );
  wk->fmwk = LINK_FESTIVAL_GetMissionWork( wk->fest_sys );

  wk->heapID = FIELDMAP_GetHeapID( wk->fieldmap );
  wk->tmpHeapID = GFL_HEAP_LOWID( wk->heapID );
}

//-----------------------------------------------------------------
/*
 *  @brief  リンクフェスタデバッグメニューワーク初期化
 */
//-----------------------------------------------------------------
static void dmenu_WorkInit( DMENU_LINK_FEST* wk )
{
  int i;

  wk->fontHandle = FLDMSGBG_GetFontHandle( FIELDMAP_GetFldMsgBG( wk->fieldmap ) );

  wk->wset = WORDSET_CreateEx( 16,64,wk->heapID );
  wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
      ARCID_DEBUG_MESSAGE, NARC_debug_message_d_linkfest_dat, wk->heapID );
  
  wk->str_tmp = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );
  wk->str_expand = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );

  wk->pAllZoneName = ZONEDATA_GetAllZoneName( wk->heapID );

  wk->beacon_nickname = GFL_STR_CreateBuffer( BUFLEN_BEACON_MSG, wk->heapID );
  wk->beacon_fword = GFL_STR_CreateBuffer( BUFLEN_BEACON_MSG, wk->heapID );

  wk->tmpInfo = GAMEBEACON_Alloc( wk->heapID );

  //リンクフェスタデータ初期化
  {
    POKEPARTY * party = GAMEDATA_GetMyPokemon( wk->gdata );
    u32 party_max = PokeParty_GetPokeCount( party );
    u8 party_top = PokeParty_GetMemberTopIdxNotEgg( party );
    POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, PokeParty_GetMemberTopIdxNotEgg( party ) );

    //ニックネーム初期値
    PP_Get( pp, ID_PARA_nickname, wk->beacon_nickname );

    //フリーワード初期値
    GFL_STR_SetStringCodeOrderLength( wk->beacon_fword,
      MISC_CrossComm_GetSelfIntroduction( wk->misc_sv ), 8+EOM_SIZE );

    for(i = 0;i < LOG_PSET_MAX;i++){
      const D_LOG_PRM_SET* prm_set = &DATA_DebugLogParamSet[i];
      if( prm_set->min >= 0 ){
        wk->log_prm[ i ] = prm_set->min;
      }
    }
  }
  //フェイクデータ初期化
  {
    int* fake = &wk->fake_prm[TR_PAT_MINE][0];
    OSOwnerInfo owner_info;
  
    OS_GetOwnerInfo(&owner_info);
 
    fake[FAKE_ID] = MyStatus_GetID_Low( wk->my_status );
    fake[FAKE_NAME] = 0;
    fake[FAKE_ZONE] = PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork( wk->gdata ));
    fake[FAKE_SEX] = MyStatus_GetMySex( wk->my_status );
    fake[FAKE_VERSION] = GET_VERSION();  //PM_VERSION;
    fake[FAKE_LANG] = PM_LANG;
    fake[FAKE_SCORE] = 0; //owner_info.favoriteColor;
    fake[FAKE_TR_VIEW] = MyStatus_GetTrainerView( wk->my_status );
    if( fake[FAKE_SEX] ){
      fake[FAKE_TR_VIEW] -= UNION_VIEW_INDEX_WOMAN_START;
    }

    MI_CpuCopy8( fake, &(wk->fake_prm[TR_PAT_FAKE][0]),sizeof(int)*FAKE_DATA_MAX);
   
    //IDだけはずらしておく
    wk->fake_prm[TR_PAT_FAKE][FAKE_ID] = GFUser_GetPublicRand0( 0xFFFF );
    wk->fake_prm[TR_PAT_FAKE][FAKE_NAME] = 1; //GFUser_GetPublicRand0( 0xFFFF );
  }
  
  //送信バッファを初期化しておく
  GAMEBEACON_Setting( wk->gdata );
}

//-----------------------------------------------------------------
/*
 *  @brief  リンクフェスタデバッグメニューワーク解放
 */
//-----------------------------------------------------------------
static void dmenu_WorkRelease( DMENU_LINK_FEST* wk )
{
  GFL_HEAP_FreeMemory( wk->tmpInfo );
  
  GFL_STR_DeleteBuffer( wk->beacon_fword );
  GFL_STR_DeleteBuffer( wk->beacon_nickname);
  
  GFL_HEAP_FreeMemory( (void*)wk->pAllZoneName );

  GFL_STR_DeleteBuffer( wk->str_expand );
  GFL_STR_DeleteBuffer( wk->str_tmp );

  GFL_MSG_Delete( wk->msgman );
  WORDSET_Delete( wk->wset );

  //リンクフェスティバルバッファを元に戻す
  GAMEBEACON_Setting( wk->gdata );
}

//--------------------------------------------------------------
/*
 *  @brief  リンクフェスタデバッグメニューメインイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_LinkFestMain( GMEVENT * event, int *seq, void * work)
{
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
  DMENU_LINK_FEST* wk = (DMENU_LINK_FEST*)work;

  switch(*seq)
  {
  case SEQ_MENU_INIT:
    wk->menuFunc = DEBUGFLDMENU_Init( wk->fieldmap, wk->heapID,  &DebugLinkFestMenuData );
    (*seq)++;
    break;
  case SEQ_MENU_MAIN:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( wk->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( wk->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL || ret == MENU_EXIT ){  //キャンセル
        *seq = SEQ_EXIT; 
        break;
      }
      switch( ret ){
      case MENU_UPDATE_STOP:
        GMEVENT_CallEvent( event, event_CreateEventNumInput( wk, NINPUT_STACK_CHECK_THROW, 0, 1,
            ninput_SetDebugParam, ninput_GetDebugParam) );
        break;
      case MENU_BEACON_REQ:
        GMEVENT_CallEvent( event, event_CreateEventLogBeaconReq( wk->gsys, wk ) );
        break;
      case MENU_STACK_CHECK:
        GMEVENT_CallEvent( event, event_CreateEventStackCheck( wk->gsys, wk, wk->view_wk->infoStack ) );
        break;
      case MENU_STACK_CLEAR:
        FMISSION_LOG_TBL_Clear( LINK_FESTIVAL_GetMissionLogStackTbl( wk->fest_sys ) );
        break;
      case MENU_RECV_BUF_CHECK:
        GMEVENT_CallEvent( event, event_CreateEventStackCheck( wk->gsys, wk, NULL) );
        break;
      case MENU_RECV_BUF_CLEAR:
        DEBUG_RecvBeaconBufferClear();
        break;
      case MENU_MISSION_MEMBER_CHECK:
        GMEVENT_CallEvent( event, event_CreateEventDataCheck( wk->gsys, wk, DATA_CHECK_MODE_MEMBER ) );
        break;
      case MENU_MISSION_EVPOS_CHECK:
        if( wk->fmwk->arc.m_type == FMISSION_TYPE_ITEM ||
            wk->fmwk->arc.m_type == FMISSION_TYPE_NPC ){
          GMEVENT_CallEvent( event, event_CreateEventDataCheck( wk->gsys, wk, DATA_CHECK_MODE_EVPOS ) );
        }
        break;
      case MENU_MISSION_EVFLAG_CHECK:
        if( wk->fmwk->arc.m_type == FMISSION_TYPE_ITEM ||
            wk->fmwk->arc.m_type == FMISSION_TYPE_NPC ){
          GMEVENT_CallEvent( event, event_CreateEventDataCheck( wk->gsys, wk, DATA_CHECK_MODE_EVFLAG ) );
        }
        break;
      case MENU_MISSION_MY_SCORE:
      case MENU_MISSION_TIME_STOP:
      case MENU_MISSION_SPOT_MODE:
      case MENU_MISSION_MEMBER_NUM:
        {
          int idx = ret-MENU_MISSION_MY_SCORE;
          GMEVENT_CallEvent( event, event_CreateEventNumInput( wk, NINPUT_MISSION_MY_SCORE+idx,
            DATA_NInputMinMax[idx].min, DATA_NInputMinMax[idx].max, 
            ninput_SetDebugParam, ninput_GetDebugParam) );
        }
        break;
      case MENU_MISSION_END:
        FEST_MISSION_DEBUG_KillMission( wk->fmwk );
        *seq = SEQ_EXIT; 
        return GMEVENT_RES_CONTINUE;
      }
      *seq = SEQ_MENU_INIT; 
    }
    break;
  case SEQ_EXIT:
  default:
    dmenu_WorkRelease( wk );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/*
 *  @brief  リンクフェスタデバッグメニュー トップメインイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_LinkFestFromFieldMain( GMEVENT * event, int *seq, void * work)
{
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
  DMENU_LINK_FEST* wk = (DMENU_LINK_FEST*)work;

  switch(*seq)
  {
  case SEQ_MENU_INIT:
    wk->menuFunc = DEBUGFLDMENU_Init( wk->fieldmap, wk->heapID,  &DebugLinkFestFromFieldMenuData );
    (*seq)++;
    break;
  case SEQ_MENU_MAIN:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( wk->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( wk->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL || ret == MENU_EXIT ){  //キャンセル
        *seq = SEQ_EXIT; 
        break;
      }
      switch( ret ){
      case MENU_RECV_BUF_CHECK:
        GMEVENT_CallEvent( event, event_CreateEventStackCheck( wk->gsys, wk, NULL ) );
        break;
      case MENU_RECV_BUF_CLEAR:
        DEBUG_RecvBeaconBufferClear();
        break;
      case MENU_MY_GPOWER:
        GMEVENT_CallEvent( event, event_CreateEventNumInput( wk, NINPUT_GPOWER, 0, GPOWER_ID_CAPTURE_S+1,
            ninput_SetDebugParam, ninput_GetDebugParam) );
        break;
      case MENU_TUTORIAL_CLEAR:
        dmenu_TutorialClear( wk );
        break;
      case MENU_MISSION_OPEN_FLAG_SET:
        dmenu_MissionEvFlagSet( wk, 0 );
        break;
      case MENU_MISSION_ENTRY_FLAG_SET:
        dmenu_MissionEvFlagSet( wk, 1 );
        break;
      case MENU_MISSION_BEACON_WAIT:
      case MENU_MISSION_LEVEL_OPEN:
      case MENU_MISSION_OPEN_NUM_SET:
      case MENU_MISSION_JOIN_NUM_SET:
      case MENU_MISSION_JOIN_NUM_MAX_SET:
      case MENU_MISSION_CLEAR_NUM_SET:
      case MENU_MISSION_TOP_SCORE_NUM_SET:
      case MENU_MISSION_HIGH_SCORE_TOTAL_ALL_SET:
      case MENU_MISSION_HIGH_SCORE_MINE_ALL_SET:
      case MENU_MISSION_NEW_FLAG_ALL_SET:
      case MENU_MISSION_EXP_WHITE_SET:
      case MENU_MISSION_EXP_BLACK_SET:
        {
          int idx = ret-MENU_MISSION_BEACON_WAIT;
          GMEVENT_CallEvent( event, event_CreateEventNumInput( wk, NINPUT_MISSION_BEACON_WAIT+idx,
            DATA_NInputFieldMinMax[idx].min, DATA_NInputFieldMinMax[idx].max, 
            ninput_SetDebugParam, ninput_GetDebugParam) );
        }
        break;
      }
      *seq = SEQ_MENU_INIT; 
    }
    break;
  case SEQ_EXIT:
  default:
    dmenu_WorkRelease( wk );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

/**
 *  @brief  チュートリアルクリアメニュー
 */
static void dmenu_TutorialClear( DMENU_LINK_FEST* wk )
{
  EVENTWORK * ev = GAMEDATA_GetEventWork( wk->gdata );

  EVENTWORK_SetEventFlag( ev, SYS_FLAG_FES_TUTORIAL_CLEAR );

  *( EVENTWORK_GetEventWorkAdrs( ev, WK_SYS_FES_TUTORIAL ) ) = SCR_FES_TUTORIAL_END;
}

/**
 *  @brief  ミッション開放イベントフラグセットメニュー
 */
static void dmenu_MissionEvFlagSet( DMENU_LINK_FEST* wk, int mode )
{
  int i;
  EVENTWORK * ev = GAMEDATA_GetEventWork( wk->gdata );
  FMISSION_ARCDATA* pData =  FMISSION_DATA_AllocArcAllDirect( wk->tmpHeapID );

  dmenu_TutorialClear( wk );

  for( i = 0; i< FMISSION_ID_MAX;i++ ){
    u16 id;

    if( mode == 0 ){
      id = pData[i].param.open_flag_id;
      if( id != 0 ){
        EVENTWORK_SetEventFlag( ev, id );
      }
    }
    id = pData[i].param.entry_flag_id;
    if( id != 0 ){
      EVENTWORK_SetEventFlag( ev, id );
    }
  }
  GFL_HEAP_FreeMemory( pData );
}

//-----------------------------------------------------------------
/*
 *  @brief  ウィンドウ追加 
 */
//-----------------------------------------------------------------
static void sub_BmpWinAdd( BMP_WIN* bmpwin, u8 px, u8 py, u8 sx, u8 sy )
{
  bmpwin->win = GFL_BMPWIN_Create( FLDBG_MFRM_MSG,
		px, py, sx, sy, PANO_FONT, GFL_BMP_CHRAREA_GET_B );

  bmpwin->bmp = GFL_BMPWIN_GetBmp( bmpwin->win );
  bmpwin->frm = GFL_BMPWIN_GetFrame( bmpwin->win );
  
  GFL_BMP_Clear( bmpwin->bmp, FCOL_WIN_BASE );
	GFL_BMPWIN_MakeScreen( bmpwin->win );
	GFL_BMPWIN_TransVramCharacter( bmpwin->win );
	GFL_BG_LoadScreenV_Req( bmpwin->frm );
}

//-----------------------------------------------------------------
/*
 *  @brief  ウィンドウ削除
 */
//-----------------------------------------------------------------
static void sub_BmpWinDel( BMP_WIN* bmpwin )
{
  GFL_BMPWIN_ClearScreen( bmpwin->win );
	GFL_BG_LoadScreenV_Req( bmpwin->frm );

  GFL_BMPWIN_Delete( bmpwin->win );
}

//----------------------------------------------------------
/*
 *  @brief  文字列整形
 *
 *  指定メッセージを wk->str_expandにつめる
 */
//----------------------------------------------------------
static void sub_GetMsgToBuf( DMENU_LINK_FEST* wk, u8 msg_id )
{
  GFL_MSG_GetString( wk->msgman, msg_id, wk->str_tmp);
  WORDSET_ExpandStr( wk->wset, wk->str_expand, wk->str_tmp );
}

//----------------------------------------------------------
/*
 *  @brief  フリーワード展開
 */
//----------------------------------------------------------
static inline void sub_FreeWordset( WORDSET* wset, u8 idx, STRBUF* str )
{
  WORDSET_RegisterWord( wset, idx, str, PM_NEUTRAL, TRUE, PM_LANG );
}

//----------------------------------------------------------
/*
 *  ゾーンID文字列展開
 */
//----------------------------------------------------------
static void sub_GetZoneIDStr( const char * allName, u32 zoneID, STRBUF * strBuf )
{
  u16 pStrCode[64];
  
  DEB_STR_CONV_SjisToStrcode( &allName[ ZONEDATA_NAME_LENGTH * zoneID ], pStrCode, 64 );
  GFL_STR_SetStringCode( strBuf, pStrCode );
}

//----------------------------------------------------------
/*
 *  @brief  指定パラメータワード展開
 */
//----------------------------------------------------------
static void sub_LogParamWordset( DMENU_LINK_FEST* wk, u8 idx, int value, BPRM_WSET_TYPE type )
{
  switch( type ){
  case BPRM_WSET_TRNAME:
    WORDSET_RegisterTrainerName( wk->wset, idx, value );
    return;
  case BPRM_WSET_TRNAME_EX:
    if( value == 0){
      MyStatus_CopyNameString( wk->my_status, wk->str_tmp );
    }else{
      WORDSET_RegisterTrainerName( wk->wset, idx, value );
      return;
    }
    break;
  case BPRM_WSET_MONSNAME:
    WORDSET_RegisterPokeMonsNameNo( wk->wset, idx, value );
    return;
  case BPRM_WSET_ITEMNAME:
    WORDSET_RegisterItemName( wk->wset, idx, value );
    return;
  case BPRM_WSET_FWAZA:
    WORDSET_RegisterWazaName( wk->wset, idx, DATA_FieldWazaTbl[value]);
    return;
  case BPRM_WSET_GPOWER:
    WORDSET_RegisterGPowerName( wk->wset, idx, value );
    return;
  case BPRM_WSET_ACTION:
    GFL_MSG_GetString( wk->msgman, dlf_action_00+value, wk->str_tmp );
    break;
  case BPRM_WSET_LOG_REQ_TYPE:
    GFL_MSG_GetString( wk->msgman, dlf_log_req_type01+value, wk->str_tmp );
    break;
  case BPRM_WSET_SEX:
    GFL_MSG_GetString( wk->msgman, dlf_sex_male+value, wk->str_tmp );
    break;
  case BPRM_WSET_VERSION:
    GFL_MSG_GetString( wk->msgman, dlf_version_white+(value-VERSION_W2), wk->str_tmp );
    break;
  case BPRM_WSET_LANG:
    GFL_MSG_GetString( wk->msgman, dlf_lang_01+(value-LANG_JAPAN), wk->str_tmp );
    break;
  case BPRM_WSET_ZONE:
    WORDSET_RegisterPlaceName( wk->wset, 7, ZONEDATA_GetPlaceNameID(value) );
    sub_GetMsgToBuf( wk, dlf_tmp_buf );
    sub_GetZoneIDStr( wk->pAllZoneName, value, wk->str_tmp );
    GFL_STR_AddString( wk->str_tmp, wk->str_expand );
    break;
  case BPRM_WSET_FREEWORD:
    GFL_MSG_GetString( wk->msgman, dlf_freeword_icon_01+value, wk->str_tmp );
    break;
  case BPRM_WSET_TR_VIEW:
    {
      u8 ofs = 0;
      if( wk->fake_prm[ TR_PAT_FAKE-(idx==3)][FAKE_SEX] ){
        ofs = UNION_VIEW_INDEX_WOMAN_START;
      }
      GFL_MSG_GetString( wk->msgman, dlf_tr_view_01+value+ofs, wk->str_tmp );
    }
    break;
  }
  sub_FreeWordset( wk->wset, idx, wk->str_tmp );
}

//----------------------------------------------------------
/*
 *  @brief  STRBUFからSTRCODE[]にコピー
 */
//----------------------------------------------------------
static void sub_GetStringCode( STRBUF* buf, STRCODE* code, int len )
{
  STRCODE code_eom = GFL_STR_GetEOMCode();
  const STRCODE *src_code;
  int i;
 
  src_code = GFL_STR_GetStringCodePointer(buf);
  for(i = 0; i < len; i++){
    code[i] = src_code[i];
    if(src_code[i] == code_eom){
      break;
    }
  }
}

//----------------------------------------------------------
/*
 *  @brief  GAMEBEACON_INFOにパーソナルデータを展開
 */
//----------------------------------------------------------
static void sub_BeaconInfoSet( DMENU_LINK_FEST* wk, TR_PATTERN pat )
{
  int i;
  u8  old_detail;
  GAMEBEACON_INFO* info = DEBUG_SendBeaconInfo_GetPtr();
  int* fake = &wk->fake_prm[pat][0]; 
  
  //直前のdetail_noだけは引き継ぐ
  old_detail = info->details.details_no;
  GAMEBEACON_Setting( wk->gdata );
  info->details.details_no = old_detail;

  if( pat == TR_PAT_MINE ){
    return;
  }
  
  //id
  info->trainer_id = (u16)fake[FAKE_ID];
  //name
  sub_LogParamWordset( wk, 7, fake[FAKE_NAME], BPRM_WSET_TRNAME_EX );
  sub_GetMsgToBuf( wk, dlf_tmp_buf );
  sub_GetStringCode( wk->str_expand, info->name, PERSON_NAME_SIZE);
  //zone
  info->zone_id = fake[FAKE_ZONE];
  info->townmap_root_zone_id = TOWNMAP_UTIL_GetRootZoneID( wk->gdata, info->zone_id);
  //sex
  info->sex = fake[FAKE_SEX];
  //pm_version
  info->pm_version = fake[FAKE_VERSION];
  //language
  info->language = fake[FAKE_LANG];
  //color
//  info->favorite_color_index = fake[FAKE_COLOR];
  //tr_char
  info->trainer_view = fake[FAKE_TR_VIEW];
}


//===================================================================================
//数値入力イベント
//===================================================================================
typedef struct _EVWK_NINPUT{
  DMENU_LINK_FEST* dlf_wk;

  u8    decide_f;
  int   key;
  int   value;

  BPRM_WSET_TYPE wset_type;

  NINPUT_PARAM  prm;
  BMP_WIN win;
}EVWK_NINPUT;

static GMEVENT_RESULT event_NumInputMain( GMEVENT * event, int *seq, void * work);
static void ninput_PrintNumWin( DMENU_LINK_FEST* wk, EVWK_NINPUT* evwk, BMP_WIN* bmpwin, int num );

//--------------------------------------------------------------
/**
 * 数値入力イベント生成コア
 * @param wk  DEBUG_MENU_EVENT_WORK*
 */
//--------------------------------------------------------------
static GMEVENT* event_CreateEventNumInputCore( DMENU_LINK_FEST* wk,
    int key, int min, int max, NINPUT_SET_FUNC set_func, NINPUT_GET_FUNC get_func,
    int* ret_wk, BPRM_WSET_TYPE wset_type )
{
  EVWK_NINPUT* evwk;
  GMEVENT* event;

  event = GMEVENT_Create( wk->gsys, NULL, event_NumInputMain, sizeof(EVWK_NINPUT) );

  evwk = GMEVENT_GetEventWork( event );
  evwk->dlf_wk = wk; 

  evwk->prm.min = min;
  evwk->prm.max = max;
  evwk->prm.ret_wk = ret_wk;
  evwk->prm.set_func = set_func;
  evwk->prm.get_func = get_func;

  evwk->wset_type = wset_type;
  evwk->key = key;

  if( ret_wk != NULL ){
    evwk->value = *ret_wk;
  }else{
    evwk->value = (get_func)( wk, key );
  }
  return event;
}

//--------------------------------------------------------------
/**
 * 数値入力イベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 */
//--------------------------------------------------------------
static GMEVENT* event_CreateEventNumInput( DMENU_LINK_FEST* wk, int key, int min, int max, NINPUT_SET_FUNC set_func, NINPUT_GET_FUNC get_func )
{
  return event_CreateEventNumInputCore( wk,
            key, min, max, set_func, get_func, NULL, BPRM_WSET_NONE );
}
static GMEVENT* event_CreateEventNumInputRetWork( DMENU_LINK_FEST* wk, int key, int min, int max, int* ret_wk, BPRM_WSET_TYPE wset_type )
{
  return event_CreateEventNumInputCore( wk,
            key, min, max, NULL, NULL, ret_wk, wset_type );
}

//--------------------------------------------------------------
/*
 *  @brief  数値入力イベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_NumInputMain( GMEVENT * event, int *seq, void * work)
{
  EVWK_NINPUT* evwk = (EVWK_NINPUT*)work;
  DMENU_LINK_FEST* wk = evwk->dlf_wk;
  NINPUT_PARAM* prm = &evwk->prm;

  switch(*seq)
  {
  case 0:
    sub_BmpWinAdd( &evwk->win, 1, 24-2, 30, 2 );
    ninput_PrintNumWin( wk, evwk, &evwk->win, evwk->value );
    (*seq)++;
    break;
  case 1:
    {
      int trg = GFL_UI_KEY_GetTrg();
      int cont = GFL_UI_KEY_GetCont();
      int diff;
      int before, after;
      
      if( trg & PAD_BUTTON_B ){
        (*seq)++;
        break;
      }
      if( trg & PAD_BUTTON_A ){
        if( prm->ret_wk != NULL ){
          *prm->ret_wk = evwk->value;
        }else{
          (prm->set_func)( wk, evwk->key, evwk->value );
        }
        (*seq)++;
        break;
      }
      after = before = evwk->value;
      if( evwk->value < prm->min || evwk->value > prm->max ){
        break;
      }
      diff = 0;
      if (trg & PAD_KEY_UP){
        diff = 1;
      } else if (trg & PAD_KEY_DOWN) {
        diff = -1;
      } else if (trg & PAD_KEY_LEFT){
        diff = -10;
      } else if (trg & PAD_KEY_RIGHT){
        diff = 10;
      }
      if( cont & PAD_BUTTON_R ){
        diff *= 10;
      }else if( cont & PAD_BUTTON_L ){
        diff *= 100;
      }
      if(diff == 0){
        break;
      }
      if( (diff < 0) && ( (after-prm->min) < (diff*-1))){
        if( before > prm->min ){
          after = prm->min; //いったんmin
        }else{
          after = prm->max; //回り込み
        }
      }else if( (diff > 0) && ((prm->max-after) < diff) ){
        if( before < prm->max ){
          after = prm->max; //いったんmax
        }else{
          after = prm->min; //回り込み
        }
      }else{
        after = before+diff;
      }
      if (after != before ) {
        ninput_PrintNumWin( wk, evwk, &evwk->win, after );
        evwk->value = after;
      }
    }
    break;
  default:
    sub_BmpWinDel( &evwk->win );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/// 数値入力ウィンドウ：表示更新
//--------------------------------------------------------------
static void ninput_PrintNumWin( DMENU_LINK_FEST* wk, EVWK_NINPUT* evwk, BMP_WIN* bmpwin, int num )
{
  WORDSET_RegisterNumber(wk->wset, 0, num, 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

  if( evwk->wset_type == BPRM_WSET_NONE ){
    sub_GetMsgToBuf( wk, dlf_num_input01 );
  }else{
    sub_LogParamWordset( wk, 1, num, evwk->wset_type );
    sub_GetMsgToBuf( wk, dlf_num_input02 );
  }

  GFL_BMP_Clear( bmpwin->bmp, FCOL_WIN_BASE );
	PRINTSYS_PrintColor( bmpwin->bmp, 0, 0, wk->str_expand, wk->fontHandle, FCOL_WIN_W );

  GFL_BMPWIN_TransVramCharacter( bmpwin->win );  //transfer characters
	GFL_BG_LoadScreenV_Req( bmpwin->frm );  //transfer screen
}

//===================================================================================
//スタックチェックイベント
//===================================================================================
typedef struct _EVWK_STACK_CHECK{
  DMENU_LINK_FEST* dlf_wk;

  FMISSION_LOG_TBL* log;

  u8      page;
  BMP_WIN win;
}EVWK_STACK_CHECK;

static GMEVENT_RESULT event_StackCheckMain( GMEVENT * event, int *seq, void * work);
static void scheck_GetBeaconRecvLogUpdate( EVWK_STACK_CHECK* evwk );
static void scheck_ParamDraw( DMENU_LINK_FEST* wk, EVWK_STACK_CHECK* evwk, u8 idx, u8 line, FMISSION_LOG* p_log );
static void scheck_PageDraw( DMENU_LINK_FEST* wk, EVWK_STACK_CHECK* evwk, u8 page );

//--------------------------------------------------------------
/**
 * スタックチェックイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 */
//--------------------------------------------------------------
static GMEVENT* event_CreateEventStackCheck( GAMESYS_WORK* gsys, DMENU_LINK_FEST* wk, GAMEBEACON_INFO_TBL* info_tbl )
{
  EVWK_STACK_CHECK* evwk;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, event_StackCheckMain, sizeof(EVWK_STACK_CHECK) );

  evwk = GMEVENT_GetEventWork( event );
  evwk->dlf_wk = wk; 
  evwk->log = LINK_FESTIVAL_GetMissionLogStackTbl( wk->fest_sys );  //スタックポインタ

  return event;
}

//--------------------------------------------------------------
/*
 *  @brief  スタックチェックイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_StackCheckMain( GMEVENT * event, int *seq, void * work)
{
  EVWK_STACK_CHECK* evwk = (EVWK_STACK_CHECK*)work;
  DMENU_LINK_FEST* wk = evwk->dlf_wk;

  switch(*seq)
  {
  case 0:
    sub_BmpWinAdd( &evwk->win, 0, 0, 32, 24 );
    scheck_PageDraw( wk, evwk, evwk->page );
    (*seq)++;
    break;
  case 1:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( trg & PAD_BUTTON_B){
        (*seq)++;
      }else if( trg & PAD_BUTTON_START ){
        evwk->page = 0;
        scheck_PageDraw( wk, evwk, evwk->page );
      }else if( trg & PAD_KEY_LEFT ){
        evwk->page = (evwk->page+(SCHECK_PAGE_MAX-1))%SCHECK_PAGE_MAX;
        scheck_PageDraw( wk, evwk, evwk->page );
      }else if( trg & PAD_KEY_RIGHT ){
        evwk->page = (evwk->page+1)%SCHECK_PAGE_MAX;
        scheck_PageDraw( wk, evwk, evwk->page );
      }
    }
    break;
  default:
    sub_BmpWinDel( &evwk->win );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/*
 *  @brief  スタック情報書き出し
 */
//--------------------------------------------------------------
static void scheck_ParamDraw( DMENU_LINK_FEST* wk, EVWK_STACK_CHECK* evwk, u8 idx, u8 line, FMISSION_LOG* p_log )
{
  int val;
  
  WORDSET_RegisterNumber( wk->wset, 0, idx, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

  if( p_log != NULL ){
    //プレイヤー名取得
    FMISSION_LOG_GetName( p_log, wk->str_tmp );
    sub_FreeWordset( wk->wset, 1, wk->str_tmp );
    
    //トレーナーID
    WORDSET_RegisterNumber( wk->wset, 2, p_log->tr_id, 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    //スコア
    WORDSET_RegisterNumber( wk->wset, 3, p_log->score, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    //自分？
    WORDSET_RegisterNumber( wk->wset, 4, p_log->mine_f, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    //性別
    WORDSET_RegisterNumber( wk->wset, 5, p_log->sex, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    //トレーナーView
    GFL_MSG_GetString( wk->msgman, dlf_tr_view_01+FMISSION_LOG_GetTrView( p_log ), wk->str_tmp );
    sub_FreeWordset( wk->wset, 6, wk->str_tmp );
  
    sub_GetMsgToBuf( wk, dlf_stack_check_param01 );
	  PRINTSYS_PrintColor( evwk->win.bmp, 0, line*SCHECK_LINE_OY, wk->str_expand, wk->fontHandle, FCOL_WIN_W );

    //アクションナンバー
    WORDSET_RegisterNumber( wk->wset, 0, p_log->action, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    //アクションパラメータ
    WORDSET_RegisterNumber( wk->wset, 1, p_log->param, 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    sub_GetMsgToBuf( wk, dlf_stack_check_param02);
	  PRINTSYS_PrintColor( evwk->win.bmp, 0, line*SCHECK_LINE_OY+32, wk->str_expand, wk->fontHandle, FCOL_WIN_W );

    //アクション
    FMISSION_LOG_ActionWordset( p_log,  wk->wset, GFL_HEAP_LOWID( wk->heapID ));
    sub_GetMsgToBuf( wk, dlf_action_log_01+FMISSION_LOG_GetAction(p_log)-1 );
	  PRINTSYS_PrintColor( evwk->win.bmp, 64, line*SCHECK_LINE_OY+32, wk->str_expand, wk->fontHandle, FCOL_WIN_W );
  }else{
    sub_GetMsgToBuf( wk, dlf_stack_check_param_none );
	  PRINTSYS_PrintColor( evwk->win.bmp, 0, line*SCHECK_LINE_OY, wk->str_expand, wk->fontHandle, FCOL_WIN_W );
  }
}

//--------------------------------------------------------------
/*
 *  @brief  スタック情報書き出し
 */
//--------------------------------------------------------------
static void scheck_PageDraw( DMENU_LINK_FEST* wk, EVWK_STACK_CHECK* evwk, u8 page )
{
  int i,idx,ofs,entry_num;
  GFL_BMP_Clear( evwk->win.bmp, FCOL_WIN_BASE );

  ofs = page*SCHECK_LINE_MAX;
  entry_num = FMISSION_LOG_TBL_GetEntryNum( evwk->log );

  for(i = 0;i < SCHECK_LINE_MAX;i++){
    idx = ofs+i;
    if( idx >= FMISSION_LOG_MAX ){
      break;
    }
    if( idx < entry_num ){
      scheck_ParamDraw( wk, evwk, idx, i, &evwk->log->log[idx] );
    }else{
      scheck_ParamDraw( wk, evwk, idx, i, NULL );
    }
  }
	GFL_BMPWIN_TransVramCharacter( evwk->win.win );
	GFL_BG_LoadScreenV_Req( evwk->win.frm );
}

//===================================================================================
//メンバー/配置データチェックイベント
//===================================================================================
typedef struct _EVWK_DATA_CHECK{
  DMENU_LINK_FEST* dlf_wk;
  FES_GIMMICK* fes_gmk;
  const char *allZoneName;
  u8      mode;
  u8      mode_param;
  u8      page;
  u8      data_num;
  u8      line_max;
  u8      page_max;
  u8      line_oy; 
  BMP_WIN win;
}EVWK_DATA_CHECK;

static GMEVENT_RESULT event_DataCheckMain( GMEVENT * event, int *seq, void * work);
static void dataCheck_PageDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, u8 page );
static void memberCheck_ParamDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, u8 idx, u8 line, LINKFEST_LOG* p_log );
static void memberCheck_PageDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, u8 page );
static void evposCheck_ItemParamDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, s8 idx, u8 line );
static void evposCheck_NpcParamDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, s8 idx, u8 line );
static void evposCheck_FlagParamDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, s8 idx, u8 line );
static void evposCheck_PageDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, u8 page );

//--------------------------------------------------------------
/**
 * データチェックイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 */
//--------------------------------------------------------------
static GMEVENT* event_CreateEventDataCheck( GAMESYS_WORK* gsys, DMENU_LINK_FEST* wk, DATA_CHECK_MODE mode )
{
  EVWK_DATA_CHECK* evwk;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, event_DataCheckMain, sizeof(EVWK_DATA_CHECK) );

  evwk = GMEVENT_GetEventWork( event );
  evwk->dlf_wk = wk; 
  evwk->mode = mode;
  evwk->fes_gmk = FIELDMAP_GetFesGimmickSys( wk->fieldmap );
  evwk->data_num = DEB_FES_GIMMICK_PosNumGet( evwk->fes_gmk );
  evwk->allZoneName = ZONEDATA_GetAllZoneName( wk->heapID );

  switch( mode ){
  case DATA_CHECK_MODE_MEMBER:
    evwk->line_max = MEMBER_CHECK_LINE_MAX;
    evwk->line_oy = MEMBER_CHECK_LINE_OY;
    evwk->page_max = MEMBER_CHECK_PAGE_MAX;
    break;
  case DATA_CHECK_MODE_EVPOS:
    evwk->page_max = 1;
    if( wk->fmwk->arc.m_type == FMISSION_TYPE_ITEM ){
      evwk->mode_param = 0;
      evwk->line_max = 8;
      evwk->line_oy = 16;
    }else{
      evwk->mode_param = 1;
      evwk->line_max = 4;
      evwk->line_oy = 32;
    }
    break;
  case DATA_CHECK_MODE_EVFLAG:
    evwk->page_max = 1;
    evwk->mode_param = 2;
    evwk->line_max = 6;
    evwk->line_oy = 16;
    break;
  }

  return event;
}

//--------------------------------------------------------------
/*
 *  @brief  データチェックイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_DataCheckMain( GMEVENT * event, int *seq, void * work)
{
  EVWK_DATA_CHECK* evwk = (EVWK_DATA_CHECK*)work;
  DMENU_LINK_FEST* wk = evwk->dlf_wk;

  switch(*seq)
  {
  case 0:
    sub_BmpWinAdd( &evwk->win, 0, 0, 32, 24 );
    dataCheck_PageDraw( wk, evwk, evwk->page );
    (*seq)++;
    break;
  case 1:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( trg & PAD_BUTTON_B){
        (*seq)++;
      }else if( trg & PAD_BUTTON_START ){
        evwk->page = 0;
        dataCheck_PageDraw( wk, evwk, evwk->page );
      }else if( trg & PAD_KEY_LEFT ){
        evwk->page = (evwk->page+(evwk->page_max-1))%evwk->page_max;
        dataCheck_PageDraw( wk, evwk, evwk->page );
      }else if( trg & PAD_KEY_RIGHT ){
        evwk->page = (evwk->page+1)%evwk->page_max;
        dataCheck_PageDraw( wk, evwk, evwk->page );
      }
    }
    break;
  default:
    GFL_HEAP_FreeMemory( (void*)evwk->allZoneName );
    sub_BmpWinDel( &evwk->win );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/*
 *  @brief  情報書き出し
 */
//--------------------------------------------------------------
static void dataCheck_PageDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, u8 page )
{
  switch( evwk->mode ){
  case DATA_CHECK_MODE_MEMBER:
    memberCheck_PageDraw( wk, evwk, page );
    break;
  case DATA_CHECK_MODE_EVPOS:
  case DATA_CHECK_MODE_EVFLAG:
    evposCheck_PageDraw( wk, evwk, page );
    break;
  }
}

//--------------------------------------------------------------
/*
 *  @brief  メンバー情報書き出し
 */
//--------------------------------------------------------------
static void memberCheck_ParamDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, u8 idx, u8 line, LINKFEST_LOG* p_log )
{
  WORDSET_RegisterNumber( wk->wset, 0, idx, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

  if( p_log != NULL ){
    //トレーナーID
    WORDSET_RegisterNumber( wk->wset, 1, p_log->score.tr_id, 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    //スコア
    WORDSET_RegisterNumber( wk->wset, 2, p_log->score.value, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    //マップ名
    WORDSET_RegisterNumber( wk->wset, 3, p_log->score.tmap_root, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterPlaceName( wk->wset, 4, ZONEDATA_GetPlaceNameID(p_log->score.tmap_root ) );

    sub_GetMsgToBuf( wk, dlf_member_check_param01 );
  }else{
    sub_GetMsgToBuf( wk, dlf_member_check_param_none );
  }
	PRINTSYS_PrintColor( evwk->win.bmp, 0, line*evwk->line_oy, wk->str_expand, wk->fontHandle, FCOL_WIN_W );
}

//--------------------------------------------------------------
/*
 *  @brief  メンバー情報書き出し
 */
//--------------------------------------------------------------
static void memberCheck_PageDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, u8 page )
{
  int i,idx,ofs,join_num;
  GFL_BMP_Clear( evwk->win.bmp, FCOL_WIN_BASE );

  ofs = page*evwk->line_max;
  join_num = LINK_FESTIVAL_GetMissionJoinNum( wk->fest_sys )-1;  //自分の数は除くので-1

  for(i = 0;i < evwk->line_max;i++){
    idx = ofs+i;
    if( idx >= LINKFEST_LOG_MAX ){
      break;
    }
    if( idx < join_num ){
      memberCheck_ParamDraw( wk, evwk, idx, i, DEB_LINKFEST_GetEntryLogParam( wk->fest_sys, idx) );
    }else{
      memberCheck_ParamDraw( wk, evwk, idx, i, NULL );
    }
  }
	GFL_BMPWIN_TransVramCharacter( evwk->win.win );
	GFL_BG_LoadScreenV_Req( evwk->win.frm );
}

//--------------------------------------------------------------
/*
 *  @brief  イベントポジション展開
 */
//--------------------------------------------------------------
static void posCheck_PosWordset( DMENU_LINK_FEST* wk, VecFx32* vec, u8 buf_idx )
{
  WORDSET_RegisterNumber( wk->wset, buf_idx+0, SIZE_GRID_FX32(vec->x), 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  WORDSET_RegisterNumber( wk->wset, buf_idx+1, SIZE_GRID_FX32(vec->y), 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  WORDSET_RegisterNumber( wk->wset, buf_idx+2, SIZE_GRID_FX32(vec->z), 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
}

//ゾーンID文字列展開
static void posCheck_ZoneIDWordset( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, u8 buf_idx, u16 zone_id )
{
  u16 pStrCode[64];
  WORDSET_RegisterNumber( wk->wset, buf_idx+0, zone_id, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    
  DEB_STR_CONV_SjisToStrcode( &evwk->allZoneName[ ZONEDATA_NAME_LENGTH * zone_id ], pStrCode, 64 );
  GFL_STR_SetStringCode( wk->str_tmp, pStrCode );
  sub_FreeWordset( wk->wset, buf_idx+1, wk->str_tmp );
}

//--------------------------------------------------------------
/*
 *  @brief  アイテムミッション情報書き出し
 */
//--------------------------------------------------------------
static void evposCheck_ItemParamDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, s8 idx, u8 line )
{
  WORDSET_RegisterNumber( wk->wset, 0, idx, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

  if( idx < evwk->data_num ){
    VecFx32 vec = {0,0,0};
    int param[2] = { 0, 0 };
    BOOL flag = DEB_FES_GIMMICK_PosGet( evwk->fes_gmk, idx, &vec, param );

    //idx
    WORDSET_RegisterNumber( wk->wset, 1, param[1], 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    //ポジション
    posCheck_PosWordset( wk, &vec, 2 );
    //アイテム名
    WORDSET_RegisterItemName( wk->wset,5, param[0] );

    sub_GetMsgToBuf( wk, dlf_item_check_param01+flag );
  }else{
    sub_GetMsgToBuf( wk, dlf_check_param_none );
  }
	PRINTSYS_PrintColor( evwk->win.bmp, 0, line*evwk->line_oy+24, wk->str_expand, wk->fontHandle, FCOL_WIN_W );
}


//--------------------------------------------------------------
/*
 *  @brief  NPCミッション情報書き出し
 */
//--------------------------------------------------------------
static void evposCheck_NpcParamDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, s8 idx, u8 line )
{
  PRINTSYS_LSB col = FCOL_WIN_W;

  WORDSET_RegisterNumber( wk->wset, 0, idx, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

  if( idx < evwk->data_num ){
    VecFx32 vec = {0,0,0};
    int param[FES_GIMMICK_NPC_POINT_PARAM_MAX+1] = {0,0,0,0};
    BOOL flag = DEB_FES_GIMMICK_PosGet( evwk->fes_gmk, idx, &vec, param );
   
    //フラグ
    WORDSET_RegisterNumber( wk->wset, 1, flag, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    //インデックス
    WORDSET_RegisterNumber( wk->wset, 2, param[FES_GIMMICK_NPC_POINT_PARAM_MAX], 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    //ポジション
    posCheck_PosWordset( wk, &vec, 3 );
  
    if( flag ){
      col = FCOL_WIN_BLUE; 
    }
    //パラメータ
    switch( wk->fmwk->beacon.base_param ){
    case FMISSION_NPC_TYPE_PRESENT_ITEM:
      WORDSET_RegisterItemName( wk->wset, 6, param[NITEM_PRESENT_PARAM_ITEM] );
      break;
    case FMISSION_NPC_TYPE_ITEM_TRADE:
      WORDSET_RegisterNumber( wk->wset, 6, param[NITEM_TRADE_PARAM_MY_CATEGORY], 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( wk->wset, 7, param[NITEM_TRADE_PARAM_NPC_CATEGORY], 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( wk->wset, 8, param[NITEM_TRADE_PARAM_TBLNO], 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      break;
    case FMISSION_NPC_TYPE_SHOPPING:
      WORDSET_RegisterItemName( wk->wset, 6, param[NITEM_SHOPPING_PARAM_ITEM] );
      WORDSET_RegisterNumber( wk->wset, 7, param[NITEM_SHOPPING_PARAM_PRICE], 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
      break;
    case FMISSION_NPC_TYPE_TR_BATTLE:
      WORDSET_RegisterTrainerName( wk->wset, 6, param[NPC_TRAINER_PARAM_TRID] );
      break;
    case FMISSION_NPC_TYPE_GET_A_PRESENT:
      WORDSET_RegisterItemName( wk->wset, 6, param[NITEM_GPRESENT_PARAM_ITEM] );
      break;
    case FMISSION_NPC_TYPE_POKE_QUIZ:
      WORDSET_RegisterPokeMonsNameNo( wk->wset, 6, param[NPC_POKE_QUIZ_PARAM_MONSNO] );
      break;
    }
    sub_GetMsgToBuf( wk, dlf_npc_check_param01+wk->fmwk->beacon.base_param );
  }else{
    sub_GetMsgToBuf( wk, dlf_check_param_none );
  }
	PRINTSYS_PrintColor( evwk->win.bmp, 0, line*evwk->line_oy+24, wk->str_expand, wk->fontHandle, col );
}

//--------------------------------------------------------------
/*
 *  @brief  ミッションフラグ情報書き出し
 */
//--------------------------------------------------------------
static void evposCheck_FlagParamDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, s8 idx, u8 line )
{
  u16 zone_id, flag_num;
  u8* flags;
  int i, tbl_num, data_max,pos_num;

  if( wk->fmwk->arc.m_type == FMISSION_TYPE_ITEM ){
    FES_GIMMICK_ITEM_WORK* item_wk = evwk->fes_gmk->wk_buf;
    data_max = FES_GIMMICK_ITEM_FLAG_WORK_NUM;
    tbl_num = item_wk->flag_wk.tbl_num;
    zone_id = item_wk->flag_wk.tbl[idx].zone_id;
    flag_num = item_wk->flag_wk.tbl[idx].flag_num;
    flags = item_wk->flag_wk.tbl[idx].flag;
  }else{
    FES_GIMMICK_NPC_WORK* npc_wk = evwk->fes_gmk->wk_buf;
    data_max = FES_GIMMICK_NPC_FLAG_WORK_NUM;
    tbl_num = npc_wk->flag_wk.tbl_num;
    zone_id = npc_wk->flag_wk.tbl[idx].zone_id;
    flag_num = npc_wk->flag_wk.tbl[idx].flag_num;
    flags = npc_wk->flag_wk.tbl[idx].flag;
  }
  WORDSET_RegisterNumber( wk->wset, 0, idx, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

  if( idx < tbl_num ){
    posCheck_ZoneIDWordset( wk, evwk, 1, zone_id );
    WORDSET_RegisterNumber( wk->wset, 3, flag_num, 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

    for(i = 0;i < data_max; i++ ){
      WORDSET_RegisterNumber( wk->wset, 4+i, flags[i], 1, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    }
    sub_GetMsgToBuf( wk, dlf_flag_check_param01+(wk->fmwk->arc.m_type!=FMISSION_TYPE_ITEM) );
  }else{
    sub_GetMsgToBuf( wk, dlf_check_param_none );
  }
	PRINTSYS_PrintColor( evwk->win.bmp, 0, line*evwk->line_oy+24, wk->str_expand, wk->fontHandle, FCOL_WIN_W );
}

//--------------------------------------------------------------
/*
 *  @brief  ミッション情報書き出し
 */
//--------------------------------------------------------------
static void evposCheck_PageDraw( DMENU_LINK_FEST* wk, EVWK_DATA_CHECK* evwk, u8 page )
{
  int i,idx,ofs;
  u16 zone_id;
  GFL_BMP_Clear( evwk->win.bmp, FCOL_WIN_BASE );

  ofs = page*evwk->line_max;

  //マジックキー描画
  zone_id = FIELDMAP_GetZoneID( wk->fieldmap );
  posCheck_ZoneIDWordset( wk, evwk, 0, zone_id );
  WORDSET_RegisterNumber( wk->wset, 2, wk->fmwk->key, 10, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  sub_GetMsgToBuf( wk, dlf_check_param_magick_key );
  PRINTSYS_PrintColor( evwk->win.bmp, 0, 0, wk->str_expand, wk->fontHandle, FCOL_WIN_W );

  for(i = 0;i < evwk->line_max;i++){
    idx = ofs+i;
    switch( evwk->mode_param ){
    case 0:
      evposCheck_ItemParamDraw( wk, evwk, idx, i );
      break;
    case 1:
      evposCheck_NpcParamDraw( wk, evwk, idx, i );
      break;
    case 2:
      evposCheck_FlagParamDraw( wk, evwk, idx, i );
      break;
    }
  }
	GFL_BMPWIN_TransVramCharacter( evwk->win.win );
	GFL_BG_LoadScreenV_Req( evwk->win.frm );
}



//===================================================================================
//ミッションリクエストユーティリティイベント
//===================================================================================
typedef struct _EVWK_LOG_BEACON_REQ{
  DMENU_LINK_FEST* dlf_wk;

  u8      line;
  int     type;
  int     action;

  const D_LOG_PARAM* log_prm;
  const D_LOG_PRM_SET* log_prm_set;

  BMP_WIN win_main;
  BMP_WIN win_line;
}EVWK_LOG_BEACON_REQ;

static GMEVENT_RESULT event_LogBeaconReqMain( GMEVENT * event, int *seq, void * work);
static void mreq_PrintCursor( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk, u8 line );
static void mreq_PrintLine( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk, u8 line, STRBUF* str );
static void mreq_DrawMissionType( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk );
static void mreq_DrawAction( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk );
static void mreq_DrawFake( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk );
static void mreq_LogBeaconSend( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk, u8 direct_mode );

//--------------------------------------------------------------
/**
 * ミッションリクエストイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 */
//--------------------------------------------------------------
static GMEVENT* event_CreateEventLogBeaconReq( GAMESYS_WORK* gsys, DMENU_LINK_FEST* wk )
{
  EVWK_LOG_BEACON_REQ* evwk;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, event_LogBeaconReqMain, sizeof(EVWK_LOG_BEACON_REQ) );

  evwk = GMEVENT_GetEventWork( event );
  evwk->dlf_wk = wk; 

  evwk->action = FMISSION_LOG_ACT_GRP_BEACON;
  return event;
}

//--------------------------------------------------------------
/*
 *  @brief  ミッションリクエストイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_LogBeaconReqMain( GMEVENT * event, int *seq, void * work)
{
  EVWK_LOG_BEACON_REQ* evwk = (EVWK_LOG_BEACON_REQ*)work;
  DMENU_LINK_FEST* wk = evwk->dlf_wk;

  switch(*seq)
  {
  case 0:
    sub_BmpWinAdd( &evwk->win_main, 1, 0, 31, 22 );
    sub_BmpWinAdd( &evwk->win_line, 0, 0, 1, 22 );
    mreq_PrintCursor( wk, evwk, evwk->line );
    (*seq)++;
    break;
  case 1:
    evwk->log_prm = &DATA_DebugLogParam[ evwk->action ];
    evwk->log_prm_set = &DATA_DebugLogParamSet[ evwk->log_prm->prm_type ];
    mreq_DrawMissionType( wk, evwk );
    mreq_DrawAction( wk, evwk );
    mreq_DrawFake( wk, evwk );
    (*seq)++;
    break;
  case 2:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( trg & PAD_BUTTON_B){
        (*seq) = 0xFF;
        break;
      }else if( trg & PAD_KEY_UP ){
        evwk->line = (evwk->line+(LOGREQ_LINE_MAX-1))%LOGREQ_LINE_MAX;
        mreq_PrintCursor( wk, evwk, evwk->line );
        break;
      }else if( trg & PAD_KEY_DOWN ){
        evwk->line = (evwk->line+1)%LOGREQ_LINE_MAX;
        mreq_PrintCursor( wk, evwk, evwk->line );
        break;
      }else if( trg & PAD_BUTTON_START ){
        mreq_LogBeaconSend( wk, evwk, FALSE );
        break;
      }

      if( (trg & PAD_BUTTON_A) == 0 ){
        break;
      }
      switch( evwk->line ){
      case 0:
        GMEVENT_CallEvent( event, event_CreateEventNumInputRetWork( wk, 0, 0, TR_PAT_MAX-1,
            &evwk->type, BPRM_WSET_LOG_REQ_TYPE));
        break;
      case 1:
        GMEVENT_CallEvent( event, event_CreateEventNumInputRetWork( wk, 0, 
            FMISSION_LOG_ACT_GRP_BEACON, FMISSION_LOG_ACT_MAX-1,
            &evwk->action, BPRM_WSET_ACTION ));
        break;
      case 2:
        if( evwk->log_prm_set->min >= 0 ){
          GMEVENT_CallEvent( event, event_CreateEventNumInputRetWork( wk, 0, 
            evwk->log_prm_set->min, evwk->log_prm_set->max,
            &wk->log_prm[evwk->log_prm->prm_type], evwk->log_prm_set->wset_type ));
        }
        break;
      default:
        {
          u8 fake_id = evwk->line-3;
          const D_FAKE_PRM_SET* f_prm = &DATA_FakeParamSet[fake_id];
          GMEVENT_CallEvent( event, event_CreateEventNumInputRetWork( wk, 0, 
            f_prm->min, f_prm->max, &(wk->fake_prm[TR_PAT_FAKE][fake_id]),f_prm->wset_type ));
        }
        break;
      }
      (*seq) = 1;
    }
    break;
  case 3:
    (*seq) = 1;
    break;
  default:
    sub_BmpWinDel( &evwk->win_line );
    sub_BmpWinDel( &evwk->win_main );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

static void mreq_PrintCursor( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk, u8 line )
{
  GFL_BMP_Clear( evwk->win_line.bmp, FCOL_WIN_BASE );
  GFL_MSG_GetString( wk->msgman, dlf_cursor, wk->str_tmp );
	PRINTSYS_PrintColor( evwk->win_line.bmp, 0, line*16, wk->str_tmp, wk->fontHandle, FCOL_WIN_W );
	GFL_BMPWIN_TransVramCharacter( evwk->win_line.win );
}

static void mreq_PrintLine( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk, u8 line, STRBUF* str )
{
  GFL_BMP_Fill( evwk->win_main.bmp, 0, line*16, LOGREQ_WIN_MAIN_SX*8, 16, FCOL_WIN_BASE );
	PRINTSYS_PrintColor( evwk->win_main.bmp, 0, line*16, str, wk->fontHandle, FCOL_WIN_W );
	GFL_BMPWIN_TransVramCharacter( evwk->win_main.win );
}

static void mreq_DrawMissionType( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk )
{
  GFL_MSG_GetString( wk->msgman, dlf_log_req_type01+evwk->type, wk->str_tmp );
  mreq_PrintLine( wk, evwk, 0, wk->str_tmp );
}
static void mreq_DrawAction( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk )
{
  int value;

  WORDSET_RegisterNumber( wk->wset, 0, evwk->action, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  sub_LogParamWordset( wk, 1, evwk->action, BPRM_WSET_ACTION );

  sub_GetMsgToBuf( wk, dlf_log_action_id );
  mreq_PrintLine( wk, evwk, 1, wk->str_expand );

  //パラメータプリント
  value = wk->log_prm[evwk->log_prm->prm_type];

  WORDSET_RegisterNumber( wk->wset, 0, value,  5, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
  sub_FreeWordset( wk->wset, 2, wk->beacon_nickname );
  sub_FreeWordset( wk->wset, 3, wk->beacon_fword );

  sub_LogParamWordset( wk, 1, value, evwk->log_prm_set->wset_type );
  sub_GetMsgToBuf( wk, dlf_beacon_req_param01+evwk->log_prm->prm_type );
  mreq_PrintLine( wk, evwk, 2, wk->str_expand );
}

static void mreq_DrawFakeLine( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk, FAKE_DATA_ID fake_id )
{
  u8 line = 3+fake_id;
  int* mine = &wk->fake_prm[ TR_PAT_MINE ][0];
  int* fake = &wk->fake_prm[ TR_PAT_FAKE ][0];
  const D_FAKE_PRM_SET* f_prm = &DATA_FakeParamSet[fake_id];

  WORDSET_RegisterNumber(wk->wset, 0, fake[fake_id], f_prm->keta, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  WORDSET_RegisterNumber(wk->wset, 1, mine[fake_id], f_prm->keta, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  sub_LogParamWordset( wk, 2, fake[fake_id], f_prm->wset_type );
  sub_LogParamWordset( wk, 3, mine[fake_id], f_prm->wset_type );
  
  sub_GetMsgToBuf( wk, dlf_fake_tr_id+fake_id );
  mreq_PrintLine( wk, evwk, line, wk->str_expand );
}

static void mreq_DrawFake( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk )
{
  int i;
  
  for(i = 0;i < FAKE_DATA_MAX;i++){
    mreq_DrawFakeLine( wk, evwk, i );
  }
}

static void mreq_LogBeaconSend( DMENU_LINK_FEST* wk, EVWK_LOG_BEACON_REQ* evwk, u8 direct_mode )
{
  u16 score;
  int value = -1;
  int* fake = &wk->fake_prm[evwk->type][0]; 

  sub_BeaconInfoSet( wk, evwk->type );

  if( evwk->log_prm_set->max >= 0 ){
    value = wk->log_prm[ evwk->log_prm->prm_type ];
  }

  DEBUG_SendBeaconPriorityEgnoreFlagSet( TRUE );
  score = wk->fmwk->my_score;
  if( evwk->type != TR_PAT_MINE ){
    wk->fmwk->my_score = fake[FAKE_SCORE];  //スコア偽装
  }

  if( evwk->action < FMISSION_LOG_ACT_GRP_BEACON ){
    OS_Printf("SetFesBeacon = %d, param = %d\n", evwk->action, value );
  }else if( evwk->action == FMISSION_LOG_ACT_FREEWORD ){
    GAMEBEACON_Set_FreeWord( (const STRBUF*)wk->beacon_fword, value );
    OS_Printf("SetFesFreeWord = %d, param = %d\n", evwk->action, value );
  }else{
    GAMEBEACON_Set_FMissionLogAction( evwk->action, value );
    OS_Printf("SetLogAction = %d, param = %d\n", evwk->action, value );
  } 
  if( evwk->type != TR_PAT_MINE ){
    wk->fmwk->my_score = score;  //スコアを書き戻す
  }
  DEBUG_SendBeaconPriorityEgnoreFlagSet( FALSE );

#if 0
  if(direct_mode && wk->view_wk != NULL){  //ビーコンを飛ばすと同時に直接自分のスタックに積む
    RTCTime recv_time;
    u16 time;
    GAMEBEACON_INFO* info = DEBUG_SendBeaconInfo_GetPtr();

    GFL_RTC_GetTime(&recv_time);
    time = (recv_time.hour << 8) | recv_time.minute;
    GAMEBEACON_InfoTbl_SetBeacon(
      wk->view_wk->infoStack, info, time, FALSE);
  }
#endif
}

//===========================================================================
//数値入力ゲッター＆セッター
//===========================================================================
//--------------------------------------------------------------
/*
 *  @brief  Misc関係　ゲット 
 */
//--------------------------------------------------------------
static int ninput_GetMiscCount( DMENU_LINK_FEST* wk, int param )
{
  switch(param){
  case 0:
    return MISC_CrossComm_GetSuretigaiCount( wk->misc_sv );
  case 1:
    return MISC_CrossComm_GetThanksRecvCount( wk->misc_sv );
  }
  return 0;
}

//--------------------------------------------------------------
/*
 *  @brief  Misc関係　セット 
 */
//--------------------------------------------------------------
static void ninput_SetMiscCount( DMENU_LINK_FEST* wk, int param, int value )
{
  switch(param){
  case 0:
    MISC_CrossComm_SetSuretigaiCount( wk->misc_sv, value );
 //   DEBUG_BEACON_VIEW_SuretigaiCountSet( wk->view_wk, value );
    break;
  case 1:
    MISC_CrossComm_SetThanksRecvCount( wk->misc_sv, value );
    break;
  }
}

//--------------------------------------------------------------
/*
 *  @brief  デバッグフラグ関係　ゲット 
 */
//--------------------------------------------------------------
static int ninput_GetDebugParam( DMENU_LINK_FEST* wk, int param )
{
  switch(param){
  case NINPUT_STACK_CHECK_THROW:
    return DEB_LINKFEST_GetLogStackCheckThrowFlag( wk->fest_sys );
  case NINPUT_GPOWER: 
    {
      INTRUDE_SAVE_WORK* int_sv = SaveData_GetIntrude(wk->save);
      GPOWER_ID gpower_id = ISC_SAVE_GetGPowerID(int_sv,0);
      if( gpower_id == GPOWER_ID_NULL ){
        return 0;
      }else{
        return gpower_id+1;
      }
    }
    break;
  case NINPUT_MISSION_BEACON_WAIT:
    return DEB_LINKFEST_GetBeaconWait( wk->fest_sys )/60;
  case NINPUT_MISSION_OPEN_NUM_SET:
    return FMISSION_SAVE_GetMissionOpenNum( wk->fmission_sv );
  case NINPUT_MISSION_JOIN_NUM_SET:
    return FMISSION_SAVE_GetMissionEntryNum( wk->fmission_sv );
  case NINPUT_MISSION_JOIN_NUM_MAX_SET:
    return FMISSION_SAVE_GetMissionMaxJoinNum( wk->fmission_sv );
  case NINPUT_MISSION_CLEAR_NUM_SET:
    return FMISSION_SAVE_GetMissionClearNum( wk->fmission_sv );
  case NINPUT_MISSION_TOP_SCORE_NUM_SET:
    return FMISSION_SAVE_GetMissionTopScoreNum( wk->fmission_sv );
  case NINPUT_MISSION_HIGH_SCORE_TOTAL_ALL_SET:
    return FMISSION_SAVE_GetMissionStatusHighScore( wk->fmission_sv, 1 );
  case NINPUT_MISSION_HIGH_SCORE_MINE_ALL_SET:
    return FMISSION_SAVE_GetMissionStatusHighScoreMine( wk->fmission_sv, 1 );
  case NINPUT_MISSION_NEW_FLAG_ALL_SET:
    return FMISSION_SAVE_CheckMissionStatusNewFlag( wk->fmission_sv, 1 );
  case NINPUT_MISSION_EXP_WHITE_SET:
  case NINPUT_MISSION_EXP_BLACK_SET:
    return FMISSION_SAVE_GetHilinkLevelExp( wk->fmission_sv, param-NINPUT_MISSION_EXP_WHITE_SET );
  }
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return 0;
  }

  switch(param){
  case NINPUT_MISSION_MY_SCORE:
    return wk->fmwk->my_score;
  case NINPUT_MISSION_TIME_STOP:
    return DEB_LINKFEST_GetTimeStopFlag( wk->fest_sys );
  case NINPUT_MISSION_SPOT_MODE:
    return DEB_LINKFEST_GetSpotModeFlag( wk->fest_sys );
  case NINPUT_MISSION_MEMBER_NUM:
    return DEB_LINKFEST_GetEntryLogNum( wk->fest_sys );
  }
  return 0;
}

//--------------------------------------------------------------
/*
 *  @brief  デバッグフラグ関係　セット 
 */
//--------------------------------------------------------------
static void ninput_SetDebugParam( DMENU_LINK_FEST* wk, int param, int value )
{
  switch(param){
  case NINPUT_GPOWER:
    {
      INTRUDE_SAVE_WORK* int_sv = SaveData_GetIntrude(wk->save);
      if( value == 0 ){
        value = GPOWER_ID_NULL;
      }else{
        value -= 1;
      }
      ISC_SAVE_SetGPowerID( int_sv, value, 0 );
    }
    break;
  case NINPUT_MISSION_BEACON_WAIT:
    DEB_LINKFEST_SetBeaconWait( wk->fest_sys, value*60 );
    break;
  case NINPUT_MISSION_LEVEL_OPEN:
    DEBUG_FMISSION_SV_SetMissionStatusClearLevelAll( wk->fmission_sv,  value );
    break;
  case NINPUT_MISSION_OPEN_NUM_SET:
    FMISSION_SAVE_SetMissionOpenNum( wk->fmission_sv, value );
    break;
  case NINPUT_MISSION_JOIN_NUM_SET:
    FMISSION_SAVE_SetMissionEntryNum( wk->fmission_sv, value );
    break;
  case NINPUT_MISSION_JOIN_NUM_MAX_SET:
    FMISSION_SAVE_SetMissionMaxJoinNum( wk->fmission_sv, value );
    break;
  case NINPUT_MISSION_CLEAR_NUM_SET:
    FMISSION_SAVE_SetMissionClearNum( wk->fmission_sv, value );
    break;
  case NINPUT_MISSION_TOP_SCORE_NUM_SET:
    FMISSION_SAVE_SetMissionTopScoreNum( wk->fmission_sv, value );
    break;
  case NINPUT_MISSION_HIGH_SCORE_TOTAL_ALL_SET:
  case NINPUT_MISSION_HIGH_SCORE_MINE_ALL_SET:
  case NINPUT_MISSION_NEW_FLAG_ALL_SET:
    {
      int i;
      for( i = 0; i < FMISSION_ID_MAX;i++ ){
        switch( param ){
        case NINPUT_MISSION_HIGH_SCORE_TOTAL_ALL_SET:
          FMISSION_SAVE_SetMissionStatusHighScore( wk->fmission_sv, i, value );
          break;
        case NINPUT_MISSION_HIGH_SCORE_MINE_ALL_SET:
          FMISSION_SAVE_SetMissionStatusHighScoreMine( wk->fmission_sv, i, value );
          break;
        case NINPUT_MISSION_NEW_FLAG_ALL_SET:
          if( value ){
            FMISSION_SAVE_SetMissionStatusNewFlag( wk->fmission_sv, i );
          }else{
            FMISSION_SAVE_ResetMissionStatusNewFlag( wk->fmission_sv, i );
          }
          break;
        }
      }
    }
    break;
  case NINPUT_MISSION_EXP_WHITE_SET:
  case NINPUT_MISSION_EXP_BLACK_SET:
    FMISSION_SAVE_SetHilinkLevelExp( wk->fmission_sv, param-NINPUT_MISSION_EXP_WHITE_SET, value );
    break;
  }
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return;
  }
  switch(param){
  case NINPUT_STACK_CHECK_THROW:
    DEB_LINKFEST_SetLogStackCheckThrowFlag( wk->fest_sys, value );
    break;
  case NINPUT_MISSION_MY_SCORE:
    wk->fmwk->my_score = value;
    LINK_FESTIVAL_FInfoScoreUpdate( wk->fest_sys, 0 );  //フィールド情報表示を更新
    break;
  case NINPUT_MISSION_TIME_STOP:
    DEB_LINKFEST_SetTimeStopFlag( wk->fest_sys, value );
    break;
  case NINPUT_MISSION_SPOT_MODE:
    DEB_LINKFEST_SetSpotModeFlag( wk->fest_sys, value );
    break;
  case NINPUT_MISSION_MEMBER_NUM:
    DEB_LINKFEST_SetEntryLogNum( wk->fest_sys, value );
    break;
  }
}










#endif  //PM_DEBUG

