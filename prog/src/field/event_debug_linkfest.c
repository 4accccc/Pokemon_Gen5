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

#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission_data.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/game_beacon_local.h" //許可なく他でinclude禁止
#include "field/beacon_view.h"
#include "field/field_comm/beacon_view_local.h"
#include "event_debug_livecomm.h"
#include "event_debug_linkfest.h"
#include "event_festmission.h"

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
 MENU_SURETIGAI_NUM,
 MENU_THANKS_NUM,
 MENU_STACK_CLEAR,
 MENU_MEMBER_CLEAR,
 MENU_RECV_BUF_CHECK,
 MENU_RECV_BUF_CLEAR,
 MENU_MY_GPOWER,
 MENU_MISSION_START,
 MENU_MISSION_END,
 MENU_MISSION_REQ,
 MENU_MISSION_BEACON_WAIT,
 MENU_EXIT,
};

/////////////////////////////////////////////
///パレット
#define PANO_FONT (14)
#define FBMP_COL_WHITE  (15)
#define FCOL_WIN_BASE (15)
#define FCOL_WIN_W  ( PRINTSYS_MACRO_LSB(1,2,FCOL_WIN_BASE) ) ///<BG白抜き

#define BUFLEN_BEACON_MSG (8*2*2+EOM_SIZE)

/////////////////////////////////////////////
///スタックチェック
#define SCHECK_LINE_MAX (4)
#define SCHECK_LINE_OY  (16*3)
#define SCHECK_PAGE_MAX (8)


/////////////////////////////////////////////
///ビーコンリクエスト
#define BREQ_LINE_MAX (3+FAKE_DATA_MAX)
#define BREQ_WIN_MAIN_SX  (31)

#define BREQ_FWAZA_MAX  (10)

typedef enum{
 TR_PAT_MINE,
 TR_PAT_FAKE,
 TR_PAT_MAX,
}TR_PATTERN;

typedef enum{
  FAKE_ID,
  FAKE_TIME,
  FAKE_TARGET,
  FAKE_PARAM,
  FAKE_ITEM,
  FAKE_ITEM_NUM,
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
  BPRM_WSET_MISSION_REQ_TYPE,
  BPRM_WSET_SEX,
  BPRM_WSET_VERSION,
  BPRM_WSET_LANG,
  BPRM_WSET_ZONE,
}BPRM_WSET_TYPE;

typedef enum{
 MISSION_PSET_DEFAULT,     //デフォルト(トレーナー名)  
 MISSION_PSET_TR_ID,      //対戦相手名
 MISSION_PSET_MONSNO,    //ポケモン種族名
 MISSION_PSET_NICKNAME,    //ポケモンニックネーム
 MISSION_PSET_ITEM,        //アイテム名
 MISSION_PSET_PTIME,       //プレイタイム
 MISSION_PSET_SURETIGAI_CT,//リンクフェスタ回数
 MISSION_PSET_THANKS,      //御礼回数
 MISSION_PSET_WAZA,        //技名
 MISSION_PSET_VICTORY,     //サブウェイ挑戦中の連勝数1-7
 MISSION_PSET_TH_RANK,     //トライアルハウスランク
 MISSION_PSET_GPOWER,      //Gパワー名
 MISSION_PSET_ID,          //プレイヤーID
 MISSION_PSET_FREEWORD,    //フリーワード8文字
 MISSION_PSET_MISSION_ID,    //ミッションID
 MISSION_PSET_MISSION_PRM,    //ミッションID
 MISSION_PSET_MONSNO_ZERO,    //モンスターナンバー0可
 MISSION_PSET_MAX,
}MISSION_PSET_TYPE;

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
 NINPUT_MISSION_BEACON_WAIT,
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

typedef struct _D_BEACON_PARAM{
  void* func;
  MISSION_PSET_TYPE  prm_type; //パラメータタイプ
  BEACON_ARG_TYPE   arg_type; //引数の型タイプ
}D_BEACON_PARAM;

typedef struct _D_MISSION_PARAM{
  MISSION_PSET_TYPE  prm_type;
}D_MISSION_PARAM;

typedef struct _D_FAKE_PRM_SET{
  BPRM_WSET_TYPE  wset_type;
  int keta;
  int min,max;
}D_FAKE_PRM_SET;

typedef struct _D_MISSION_PRM_SET{
  int min,max;   //パラメータのmix,max
  BPRM_WSET_TYPE wset_type;
}D_MISSION_PRM_SET;

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

  LINK_FESTIVAL* fest_sys;
  FEST_MISSION* fest_mission;

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

  int               mission_prm[MISSION_PSET_MAX];
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

//======================================================================
//
//データ定義
//
//======================================================================

static const DEBUG_MENU_LIST DATA_DebugLinkFestMenuList[] =
{
  { dlf_menu_01, (void*)MENU_UPDATE_STOP, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_02, (void*)MENU_BEACON_REQ, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_03, (void*)MENU_STACK_CHECK, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_04, (void*)MENU_SURETIGAI_NUM, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_05, (void*)MENU_THANKS_NUM, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_06, (void*)MENU_STACK_CLEAR, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_07, (void*)MENU_MEMBER_CLEAR, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_08, (void*)MENU_RECV_BUF_CHECK, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_09, (void*)MENU_RECV_BUF_CLEAR, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_exit, (void*)MENU_EXIT, GFL_OVERLAY_BLANK_ID },
};

static const DEBUG_MENU_LIST DATA_DebugLinkFestFromFieldMenuList[] =
{
  { dlf_fldmenu_01, (void*)MENU_MISSION_START, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_02, (void*)MENU_MISSION_END, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_03, (void*)MENU_MISSION_REQ, GFL_OVERLAY_BLANK_ID },
  { dlf_fldmenu_04, (void*)MENU_MISSION_BEACON_WAIT, GFL_OVERLAY_BLANK_ID },
  { dlf_menu_exit, (void*)MENU_EXIT, GFL_OVERLAY_BLANK_ID },
};

static const DEBUG_MENU_INITIALIZER DebugLinkFestMenuData = {
  NARC_debug_message_d_linkfest_dat,
  NELEMS(DATA_DebugLinkFestMenuList),
  DATA_DebugLinkFestMenuList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 24, 17,
  NULL,
  NULL
};
static const DEBUG_MENU_INITIALIZER DebugLinkFestFromFieldMenuData = {
  NARC_debug_message_d_linkfest_dat,
  NELEMS(DATA_DebugLinkFestFromFieldMenuList),
  DATA_DebugLinkFestFromFieldMenuList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 24, 17,
  NULL,
  NULL
};

/*
 *
 */
static const D_MISSION_PARAM DATA_DebugMissionParam[FMISSION_TYPE_MAX] = {
  { 0 },
  { MISSION_PSET_MONSNO_ZERO },
};

static const  D_MISSION_PRM_SET DATA_DebugMissionParamSet[MISSION_PSET_MAX] = {
 { -1, -1, BPRM_WSET_NONE,}, //MISSION_PSET_DEFAULT, デフォルト(トレーナー名)  
 { 1, TRID_MAX-1, BPRM_WSET_TRNAME }, //MISSION_PSET_TR_ID,      対戦相手名
 { MONSNO_HUSIGIDANE, MONSNO_END, BPRM_WSET_MONSNAME }, //MISSION_PSET_MONSNO,    ポケモン種族名
 { -1, -1, BPRM_WSET_NONE }, //MISSION_PSET_NICKNAME,    ポケモンニックネーム
 { ITEM_MASUTAABOORU, ITEM_DATA_MAX, BPRM_WSET_ITEMNAME }, //MISSION_PSET_ITEM,        アイテム名
 { 0, 999, BPRM_WSET_NONE }, //MISSION_PSET_PTIME,       プレイタイム
 { 0, CROSS_COMM_SURETIGAI_COUNT_MAX, BPRM_WSET_NONE }, //MISSION_PSET_SURETIGAI_CT, リンクフェスタ回数
 { 0, CROSS_COMM_THANKS_RECV_COUNT_MAX, BPRM_WSET_NONE }, //MISSION_PSET_THANKS, 御礼回数
 { 0, BREQ_FWAZA_MAX-1, BPRM_WSET_FWAZA }, //MISSION_PSET_WAZA,        技名
 { 1, 7, BPRM_WSET_NONE }, //MISSION_PSET_VICTORY,     サブウェイ挑戦中の連勝数1-7
 { 0, 5, BPRM_WSET_NONE }, //MISSION_PSET_TH_RANK,     トライアルハウスランク
 { 0, GPOWER_ID_CAPTURE_MAX, BPRM_WSET_GPOWER }, //MISSION_PSET_GPOWER,      Gパワー名
 { 0, 65535, BPRM_WSET_NONE }, //MISSION_PSET_ID,     プレイヤーID
 { -1, -1, BPRM_WSET_NONE }, //MISSION_PSET_FREEWORD,    フリーワード8文字
 { 1, FMISSION_TYPE_END, BPRM_WSET_NONE }, //MISSION_PSET_MISSION_TYP,    
 { 1, 65535, BPRM_WSET_NONE }, //MISSION_PSET_MISSION_PRM,    
 { 0, MONSNO_END, BPRM_WSET_MONSNAME }, //MISSION_PSET_MONSNO_ZERO,    ポケモン種族名
};

static const D_FAKE_PRM_SET DATA_FakeParamSet[] = {
  { BPRM_WSET_NONE, 5, 0, 0xFFFF,}, //FAKE_ID,
  { BPRM_WSET_NONE, 5, 0, 0xFFFF,}, //FAKE_TIME,
  { BPRM_WSET_NONE, 5, 0, 0xFFFF,}, //FAKE_TARGET,
  { BPRM_WSET_NONE, 5, 0, 0xFFFF,}, //FAKE_PARAM,
  { BPRM_WSET_ITEMNAME, 5, 1, ITEM_DATA_MAX}, //FAKE_ITEM,
  { BPRM_WSET_NONE, 5, 0, 0xFFFF,}, //FAKE_ITEM_NUM,
  { BPRM_WSET_TRNAME_EX, 3, 0, TRID_MAX-1,}, //FAKE_NAME,
  { BPRM_WSET_ZONE, 3, 0, ZONE_ID_MAX-1,}, //FAKE_ZONE,
  { BPRM_WSET_SEX, 1, PM_MALE, PM_FEMALE,}, //FAKE_SEX,
  { BPRM_WSET_VERSION, 3, VERSION_WHITE, VERSION_B2,}, //FAKE_VERSION,
  { BPRM_WSET_LANG, 1, LANG_JAPAN, LANG_KOREA,}, //FAKE_LANG,
  { BPRM_WSET_NONE, 2, 0, 15}, //FAKE_COLOR,
};

////////////////////////////////////////////////
//フィールド技リスト
static const u16 DATA_FieldWazaTbl[BREQ_FWAZA_MAX] = {
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

static void sub_BmpWinAdd( BMP_WIN* bmpwin, u8 px, u8 py, u8 sx, u8 sy );
static void sub_BmpWinDel( BMP_WIN* bmpwin );
static void sub_GetMsgToBuf( DMENU_LINK_FEST* wk, u8 msg_id );
static inline void sub_FreeWordset( WORDSET* wset, u8 idx, STRBUF* str );
static void sub_MissionParamWordset( DMENU_LINK_FEST* wk, u8 idx, int value, BPRM_WSET_TYPE type );

static GMEVENT* event_CreateEventNumInput( DMENU_LINK_FEST* wk, int key, int min, int max, NINPUT_SET_FUNC set_func, NINPUT_GET_FUNC get_func );
static GMEVENT* event_CreateEventNumInputRetWork( DMENU_LINK_FEST* wk, int key, int min, int max, int* ret_wk, BPRM_WSET_TYPE wset_type );
static GMEVENT* event_CreateEventStackCheck( GAMESYS_WORK* gsys, DMENU_LINK_FEST* wk, GAMEBEACON_INFO_TBL* info_tbl );
static GMEVENT* event_CreateEventMissionReq( GAMESYS_WORK* gsys, DMENU_LINK_FEST* wk );

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

  wk->fest_sys = GAMESYSTEM_GetLinkFestivalWork( gsys );
  wk->fest_mission = LINK_FESTIVAL_GetMissionWork( wk->fest_sys );

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

    for(i = 0;i < MISSION_PSET_MAX;i++){
      const D_MISSION_PRM_SET* prm_set = &DATA_DebugMissionParamSet[i];
      if( prm_set->min >= 0 ){
        wk->mission_prm[ i ] = prm_set->min;
      }
    }
  }
  //フェイクデータ初期化
  {
    int* fake = &wk->fake_prm[TR_PAT_MINE][0];
    OSOwnerInfo owner_info;
  
    OS_GetOwnerInfo(&owner_info);
 
    fake[FAKE_ID] = MyStatus_GetID_Low( wk->my_status );
    fake[FAKE_TIME] = 3;
    fake[FAKE_TARGET] = 10;
    fake[FAKE_PARAM] = 0;
    fake[FAKE_ITEM] = ITEM_DERUDAMA;
    fake[FAKE_ITEM_NUM] = 3;

    MI_CpuCopy8( fake, &(wk->fake_prm[TR_PAT_FAKE][0]),sizeof(int)*FAKE_DATA_MAX);
   
    //IDだけはずらしておく
    wk->fake_prm[TR_PAT_FAKE][FAKE_ID] = GFUser_GetPublicRand0( 0xFFFF );
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
        GMEVENT_CallEvent( event, event_CreateEventMissionReq( wk->gsys, wk ) );
        break;
      case MENU_STACK_CHECK:
        GMEVENT_CallEvent( event, event_CreateEventStackCheck( wk->gsys, wk, wk->view_wk->infoStack ) );
        break;
      case MENU_SURETIGAI_NUM:
        GMEVENT_CallEvent( event, event_CreateEventNumInput( wk, 0, wk->view_wk->ctrl.max, CROSS_COMM_SURETIGAI_COUNT_MAX,
            ninput_SetMiscCount, ninput_GetMiscCount) );
        break;
      case MENU_THANKS_NUM:
        GMEVENT_CallEvent( event, event_CreateEventNumInput( wk, 1, 0, CROSS_COMM_THANKS_RECV_COUNT_MAX,
            ninput_SetMiscCount, ninput_GetMiscCount) );
        break;
      case MENU_STACK_CLEAR:
        GAMEBEACON_InfoTbl_Clear( wk->view_wk->infoStack );
        break;
      case MENU_MEMBER_CLEAR:
        DEBUG_BEACON_VIEW_MemberListClear( wk->view_wk );
        break;
      case MENU_RECV_BUF_CHECK:
        GMEVENT_CallEvent( event, event_CreateEventStackCheck( wk->gsys, wk, NULL) );
        break;
      case MENU_RECV_BUF_CLEAR:
        DEBUG_RecvBeaconBufferClear();
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
      case MENU_MISSION_START:
        {
          GMEVENT* mission_ev;
          FEST_MISSION mission;

          FMISSION_DATA_GetMissionParam( NULL, FMISSION_ID_POKE_SEARCH_TABUNNE, &mission );

          mission_ev = GMEVENT_CreateOverlayEventCall( wk->gsys,
            FS_OVERLAY_ID( event_fmission ), EVENT_FestMissionStart, &mission );
          
          GMEVENT_ChangeEvent(event, mission_ev);
        }
        dmenu_WorkRelease( wk );
        return GMEVENT_RES_CONTINUE;
      case MENU_MISSION_END:
        FEST_MISSION_DEBUG_KillMission( wk->fest_mission );
        break;
      case MENU_MISSION_REQ:
        GMEVENT_CallEvent( event, event_CreateEventMissionReq( wk->gsys, wk ) );
        break;
      case MENU_MISSION_BEACON_WAIT:
        GMEVENT_CallEvent( event, event_CreateEventNumInput( wk, NINPUT_MISSION_BEACON_WAIT, 3, 60, 
            ninput_SetDebugParam, ninput_GetDebugParam) );
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
static void sub_MissionParamWordset( DMENU_LINK_FEST* wk, u8 idx, int value, BPRM_WSET_TYPE type )
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
  case BPRM_WSET_MISSION_REQ_TYPE:
    GFL_MSG_GetString( wk->msgman, dlf_mission_req_type01+value, wk->str_tmp );
    break;
  case BPRM_WSET_SEX:
    GFL_MSG_GetString( wk->msgman, dlf_sex_male+value, wk->str_tmp );
    break;
  case BPRM_WSET_VERSION:
    GFL_MSG_GetString( wk->msgman, dlf_version_black+(value-VERSION_BLACK), wk->str_tmp );
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
#if 0
  //直前のdetail_noだけは引き継ぐ
  old_detail = info->details.details_no;
  GAMEBEACON_Setting( wk->gdata );
  info->details.details_no = old_detail;

  switch(pat){
  case TR_PAT_MINE:
    return;
  case TR_PAT_FAKE:
    break;
  case TR_PAT_RND:
    for(i = 0;i < FAKE_DATA_MAX;i++){
      int range;
      const D_FAKE_PRM_SET* f_prm = &DATA_FakeParamSet[i];

      range = (f_prm->max-f_prm->min)+1;
      fake[i] = (GFUser_GetPublicRand0( range ))+f_prm->min;
    }
    info->trainer_view = GFUser_GetPublicRand0( 8 );
    break;
  }
  //id
  info->trainer_id = (u16)fake[FAKE_ID];
  //name
  sub_MissionParamWordset( wk, 7, fake[FAKE_NAME], BPRM_WSET_TRNAME_EX );
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
  info->favorite_color_index = fake[FAKE_COLOR];
#endif
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
    sub_MissionParamWordset( wk, 1, num, evwk->wset_type );
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
  GAMEBEACON_INFO_TBL *info_tbl;

  GAMEBEACON_LOG recv_log[GAMEBEACON_SYSTEM_LOG_MAX]; ///<受信ログ退避用バッファ
  s8      start_log;
  s8      end_log;
  s8      log_num;
  RTCTime recv_time;
  u32     recv_time_ct;

  u8      page;
  BMP_WIN win;
}EVWK_STACK_CHECK;

static GMEVENT_RESULT event_StackCheckMain( GMEVENT * event, int *seq, void * work);
static void scheck_GetBeaconRecvLogUpdate( EVWK_STACK_CHECK* evwk );
static void scheck_ParamDraw( DMENU_LINK_FEST* wk, EVWK_STACK_CHECK* evwk, u8 idx, u8 line, GAMEBEACON_INFO* info, u16 time, u16 time_sec );
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
  evwk->info_tbl = info_tbl;

  scheck_GetBeaconRecvLogUpdate( evwk );

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
        if( evwk->info_tbl == NULL ){
          scheck_GetBeaconRecvLogUpdate( evwk );
          evwk->page = 0;
          scheck_PageDraw( wk, evwk, evwk->page );
        }
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
/**
 * デバッグ用：現在の受信ログバッファのコピーを取得
 */
//--------------------------------------------------------------
static void scheck_GetBeaconRecvLogUpdate( EVWK_STACK_CHECK* evwk )
{
  DEBUG_GetBeaconRecvLogCopy( evwk->recv_log, &evwk->log_num, &evwk->start_log, &evwk->end_log );

  GFL_RTC_GetTime(&evwk->recv_time);
  //秒変換して覚えておく
  evwk->recv_time_ct = evwk->recv_time.hour*3600+evwk->recv_time.minute*60+evwk->recv_time.second;
}

//--------------------------------------------------------------
/*
 *  @brief  スタック情報書き出し
 */
//--------------------------------------------------------------
static void scheck_ParamDraw( DMENU_LINK_FEST* wk, EVWK_STACK_CHECK* evwk, u8 idx, u8 line, GAMEBEACON_INFO* info, u16 time, u16 time_sec )
{
  int val;
  u8 msg_id;
  GAMEBEACON_ACTION action;

  WORDSET_RegisterNumber( wk->wset, 0, idx, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

  if( info != NULL ){
    msg_id = dlf_stack_check_param01;
  
    //プレイヤー名取得
    GAMEBEACON_Get_PlayerNameToBuf( info, wk->str_tmp );
    sub_FreeWordset( wk->wset, 1, wk->str_tmp );

    //トレーナーID
    WORDSET_RegisterNumber( wk->wset, 2, GAMEBEACON_Get_TrainerID( info ), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

    //アクション
    action = GAMEBEACON_Get_Action_ActionNo( info );
    WORDSET_RegisterNumber( wk->wset, 3, action, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    if( action < 2 ){
      GFL_MSG_GetString( wk->msgman, dlf_action_null+action, wk->str_tmp);
    }else{
      GFL_MSG_GetString( wk->msgman, dlf_action_00+action, wk->str_tmp);
    }
    sub_FreeWordset( wk->wset, 4, wk->str_tmp );

    //タイム
    WORDSET_RegisterNumber( wk->wset, 5, time>>8, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( wk->wset, 6, time&0xFF, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

    //カラー
    val = GAMEBEACON_Get_FavoriteColorIndex( info );
    WORDSET_RegisterNumber( wk->wset, 7, val, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    
    //経過時間
    WORDSET_RegisterNumber( wk->wset, 8, time_sec, 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  }else{
    msg_id = dlf_stack_check_param02;
  }
  sub_GetMsgToBuf( wk, msg_id );
	PRINTSYS_PrintColor( evwk->win.bmp, 0, line*SCHECK_LINE_OY, wk->str_expand, wk->fontHandle, FCOL_WIN_W );
}

//--------------------------------------------------------------
/**
 * デバッグ用：指定オフセット位置の受信ログバッファの内容を取得
 */
//--------------------------------------------------------------
static BOOL scheck_GetBeaconRecvLog( EVWK_STACK_CHECK* evwk, int idx, GAMEBEACON_INFO* info, u16* pTime, u16* pTimeSec )
{
  u16 id;
  u32 time_ct;
  u8  hour,min;

  MI_CpuClear8( info, sizeof( GAMEBEACON_INFO ));
  *pTime = 0;

  if( idx >= evwk->log_num ){
    return FALSE;
  }
  if( evwk->start_log <= evwk->end_log ){
    id = evwk->start_log+idx;
    if( id > evwk->end_log ){
      return FALSE;
    }
  }else{
    id = (evwk->start_log+idx)%GAMEBEACON_SYSTEM_LOG_MAX; 
    if( id > evwk->end_log && id < evwk->start_log ){
      return FALSE;  
    }
  }
  *info = evwk->recv_log[id].info;
  time_ct = (evwk->recv_log[id].time)/60; //秒単位に変換
  *pTimeSec = time_ct;

  time_ct = evwk->recv_time_ct-time_ct;
  hour = (time_ct/3600)%24;
  min = (time_ct/60)%60;
  *pTime = ((hour<<8)|(min));

  return TRUE;
}

//--------------------------------------------------------------
/*
 *  @brief  スタック情報書き出し
 */
//--------------------------------------------------------------
static void scheck_PageDraw( DMENU_LINK_FEST* wk, EVWK_STACK_CHECK* evwk, u8 page )
{
  int i,idx,ofs;
  u16 time_sec = 0;
  GFL_BMP_Clear( evwk->win.bmp, FCOL_WIN_BASE );

  ofs = page*SCHECK_LINE_MAX;

  for(i = 0;i < SCHECK_LINE_MAX;i++){
    BOOL ret;
    idx = ofs+i;
    if( idx >= BS_LOG_MAX ){
      break;
    }
    if( evwk->info_tbl != NULL ){
      ret = GAMEBEACON_InfoTbl_GetBeacon( evwk->info_tbl, wk->tmpInfo, &wk->tmpTime, idx);
      if( ret ){
        time_sec = GAMEBEACON_Get_RecvBeaconTime(GAMEBEACON_Get_TrainerID( wk->tmpInfo ));
      }else{
        time_sec = 0;
      }
    }else{
      ret = scheck_GetBeaconRecvLog( evwk, idx, wk->tmpInfo, &wk->tmpTime, &time_sec );
    }
    if(ret){
      scheck_ParamDraw( wk, evwk, idx, i, wk->tmpInfo, wk->tmpTime, time_sec );
    }else{
      scheck_ParamDraw( wk, evwk, idx, i, NULL, 0, 0 );
    }
  }
	GFL_BMPWIN_TransVramCharacter( evwk->win.win );
	GFL_BG_LoadScreenV_Req( evwk->win.frm );
}

//===================================================================================
//ミッションリクエストユーティリティイベント
//===================================================================================
typedef struct _EVWK_MISSION_REQ{
  DMENU_LINK_FEST* dlf_wk;

  u8      line;
  int     type;
  int     action;

  const D_MISSION_PARAM* m_prm;
  const D_MISSION_PRM_SET* m_prm_set;

  BMP_WIN win_main;
  BMP_WIN win_line;
}EVWK_MISSION_REQ;

static GMEVENT_RESULT event_MissionReqMain( GMEVENT * event, int *seq, void * work);
static void mreq_PrintCursor( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk, u8 line );
static void mreq_PrintLine( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk, u8 line, STRBUF* str );
static void mreq_DrawMissionType( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk );
static void mreq_DrawMissionID( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk );
static void mreq_DrawFake( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk );
static void mreq_MissionStart( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk, u8 direct_mode, GMEVENT* parent );

//--------------------------------------------------------------
/**
 * ミッションリクエストイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 */
//--------------------------------------------------------------
static GMEVENT* event_CreateEventMissionReq( GAMESYS_WORK* gsys, DMENU_LINK_FEST* wk )
{
  EVWK_MISSION_REQ* evwk;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, event_MissionReqMain, sizeof(EVWK_MISSION_REQ) );

  evwk = GMEVENT_GetEventWork( event );
  evwk->dlf_wk = wk; 

  evwk->action = 1; //GAMEBEACON_ACTION_BATTLE_WILD_POKE_START;
  return event;
}

//--------------------------------------------------------------
/*
 *  @brief  ミッションリクエストイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_MissionReqMain( GMEVENT * event, int *seq, void * work)
{
  EVWK_MISSION_REQ* evwk = (EVWK_MISSION_REQ*)work;
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
    evwk->m_prm = &DATA_DebugMissionParam[ evwk->action ];
    evwk->m_prm_set = &DATA_DebugMissionParamSet[ evwk->m_prm->prm_type ];
    mreq_DrawMissionType( wk, evwk );
    mreq_DrawMissionID( wk, evwk );
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
        evwk->line = (evwk->line+(BREQ_LINE_MAX-1))%BREQ_LINE_MAX;
        mreq_PrintCursor( wk, evwk, evwk->line );
        break;
      }else if( trg & PAD_KEY_DOWN ){
        evwk->line = (evwk->line+1)%BREQ_LINE_MAX;
        mreq_PrintCursor( wk, evwk, evwk->line );
        break;
      }else if( trg & PAD_BUTTON_START ){
        mreq_MissionStart( wk, evwk, FALSE, event );
        sub_BmpWinDel( &evwk->win_line );
        sub_BmpWinDel( &evwk->win_main );
        return GMEVENT_RES_CONTINUE;
      }

      if( (trg & PAD_BUTTON_A) == 0 ){
        break;
      }
      switch( evwk->line ){
      case 0:
        GMEVENT_CallEvent( event, event_CreateEventNumInputRetWork( wk, 0, 0, TR_PAT_MAX-1,
            &evwk->type, BPRM_WSET_MISSION_REQ_TYPE));
        break;
      case 1:
        GMEVENT_CallEvent( event, event_CreateEventNumInputRetWork( wk, 0, 
            1, FMISSION_TYPE_END,
            &evwk->action, BPRM_WSET_ACTION ));
        break;
      case 2:
        if( evwk->m_prm_set->min >= 0 ){
          GMEVENT_CallEvent( event, event_CreateEventNumInputRetWork( wk, 0, 
            evwk->m_prm_set->min, evwk->m_prm_set->max,
            &wk->mission_prm[evwk->m_prm->prm_type], evwk->m_prm_set->wset_type ));
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

static void mreq_PrintCursor( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk, u8 line )
{
  GFL_BMP_Clear( evwk->win_line.bmp, FCOL_WIN_BASE );
  GFL_MSG_GetString( wk->msgman, dlf_cursor, wk->str_tmp );
	PRINTSYS_PrintColor( evwk->win_line.bmp, 0, line*16, wk->str_tmp, wk->fontHandle, FCOL_WIN_W );
	GFL_BMPWIN_TransVramCharacter( evwk->win_line.win );
}

static void mreq_PrintLine( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk, u8 line, STRBUF* str )
{
  GFL_BMP_Fill( evwk->win_main.bmp, 0, line*16, BREQ_WIN_MAIN_SX*8, 16, FCOL_WIN_BASE );
	PRINTSYS_PrintColor( evwk->win_main.bmp, 0, line*16, str, wk->fontHandle, FCOL_WIN_W );
	GFL_BMPWIN_TransVramCharacter( evwk->win_main.win );
}

static void mreq_DrawMissionType( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk )
{
  GFL_MSG_GetString( wk->msgman, dlf_mission_req_type01+evwk->type, wk->str_tmp );
  mreq_PrintLine( wk, evwk, 0, wk->str_tmp );
}

static void mreq_DrawMissionID( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk )
{
  int value;

  WORDSET_RegisterNumber( wk->wset, 0, evwk->action, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  sub_MissionParamWordset( wk, 1, evwk->action, BPRM_WSET_ACTION );

  sub_GetMsgToBuf( wk, dlf_mprm_req_id );
  mreq_PrintLine( wk, evwk, 1, wk->str_expand );

  //パラメータプリント
  value = wk->mission_prm[evwk->m_prm->prm_type];

  WORDSET_RegisterNumber( wk->wset, 0, value,  5, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
  sub_FreeWordset( wk->wset, 2, wk->beacon_nickname );
  sub_FreeWordset( wk->wset, 3, wk->beacon_fword );

  sub_MissionParamWordset( wk, 1, value, evwk->m_prm_set->wset_type );
  sub_GetMsgToBuf( wk, dlf_beacon_req_param01+evwk->m_prm->prm_type );
  mreq_PrintLine( wk, evwk, 2, wk->str_expand );
}

static void mreq_DrawFakeLine( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk, FAKE_DATA_ID fake_id )
{
  u8 line = 3+fake_id;
  int* mine = &wk->fake_prm[ TR_PAT_MINE ][0];
  int* fake = &wk->fake_prm[ TR_PAT_FAKE ][0];
  const D_FAKE_PRM_SET* f_prm = &DATA_FakeParamSet[fake_id];

  WORDSET_RegisterNumber(wk->wset, 0, fake[fake_id], f_prm->keta, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  WORDSET_RegisterNumber(wk->wset, 1, mine[fake_id], f_prm->keta, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  sub_MissionParamWordset( wk, 2, fake[fake_id], f_prm->wset_type );
  sub_MissionParamWordset( wk, 3, mine[fake_id], f_prm->wset_type );
  
  sub_GetMsgToBuf( wk, dlf_mprm_id+fake_id );
  mreq_PrintLine( wk, evwk, line, wk->str_expand );
}

static void mreq_DrawFake( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk )
{
  int i;
  
  for(i = 0;i < FAKE_DATA_MAX;i++){
    mreq_DrawFakeLine( wk, evwk, i );
  }
}

static void mreq_MissionStart( DMENU_LINK_FEST* wk, EVWK_MISSION_REQ* evwk, u8 direct_mode, GMEVENT* parent )
{
  int value = -1;
  int* fake = &wk->fake_prm[evwk->type][0]; 
  GMEVENT* mission_ev;
  FEST_MISSION mission;

  OS_Printf("type=%d\n",evwk->type);
  if( evwk->m_prm_set->max >= 0 ){
    value = wk->mission_prm[ evwk->m_prm->prm_type ];
  }
  FMISSION_DATA_GetMissionParam( NULL, FMISSION_ID_POKE_SEARCH_TABUNNE, &mission );
  mission.type = evwk->action;
  mission.item = fake[FAKE_ITEM];
  mission.item_num = fake[FAKE_ITEM_NUM];
  mission.target = fake[FAKE_TARGET];
  mission.param = fake[FAKE_PARAM];
  mission.time = fake[FAKE_TIME];
//  FEST_MISSION_ParamSet( &mission, evwk->action, fake[FAKE_ITEM], fake[FAKE_ITEM_NUM], fake[FAKE_TARGET], fake[FAKE_PARAM], fake[FAKE_TIME] );

  mission_ev = GMEVENT_CreateOverlayEventCall( wk->gsys,
    FS_OVERLAY_ID( event_fmission ), EVENT_FestMissionStart, &mission );
          
  GMEVENT_ChangeEvent(parent, mission_ev);
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
    return DEBUG_BEACON_STATUS_GetStackCheckThrowFlag( wk->b_status );
  case NINPUT_GPOWER:
    {
      INTRUDE_SAVE_WORK* int_sv = SaveData_GetIntrude(wk->save);
      GPOWER_ID gpower_id = ISC_SAVE_GetGPowerID(int_sv);
      if( gpower_id == GPOWER_ID_NULL ){
        return 0;
      }else{
        return gpower_id+1;
      }
    }
    break;
  case NINPUT_MISSION_BEACON_WAIT:
    return DEB_LINKFEST_GetBeaconWait( wk->fest_sys )/60;
    break;
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
  case NINPUT_STACK_CHECK_THROW:
    DEBUG_BEACON_STATUS_SetStackCheckThrowFlag( wk->b_status, value );
    break;
  case NINPUT_GPOWER:
    {
      INTRUDE_SAVE_WORK* int_sv = SaveData_GetIntrude(wk->save);
      if( value == 0 ){
        value = GPOWER_ID_NULL;
      }else{
        value -= 1;
      }
      ISC_SAVE_SetGPowerID( int_sv, value );
    }
    break;
  case NINPUT_MISSION_BEACON_WAIT:
    DEB_LINKFEST_SetBeaconWait( wk->fest_sys, value*60 );
    break;
  }
}










#endif  //PM_DEBUG

