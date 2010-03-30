//==============================================================================
/**
 * @file    intrude_subdisp.c
 * @brief   侵入下画面
 * @author  matsuda
 * @date    2009.10.16(金)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "infowin/infowin.h"
#include "app_menu_common.naix"
#include "field/fieldmap.h"
#include "field/intrude_subdisp.h"
#include "field/intrude_common.h"
#include "palace.naix"
#include "fieldmap/zone_id.h"
#include "intrude_types.h"
#include "infowin\infowin.h"
#include "intrude_main.h"
#include "intrude_work.h"
#include "intrude_mission.h"
#include "field/zonedata.h"
#include "intrude_comm_command.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_invasion.h"
#include "system/bmp_oam.h"
#include "intrude_mission_field.h"
#include "sound/pm_sndsys.h"
#include "field/event_intrude_subscreen.h"
#include "field/scrcmd_intrude.h"
#include "intrude_field.h"


//==============================================================================
//  定数定義
//==============================================================================
///サブ画面BGフレーム定義
enum{
  INTRUDE_FRAME_S_NULL_0 = GFL_BG_FRAME0_S,          ///<未使用
  INTRUDE_FRAME_S_NULL_1 = GFL_BG_FRAME1_S,          ///<未使用
  INTRUDE_FRAME_S_BAR = GFL_BG_FRAME2_S,             ///<プレート面
  INTRUDE_FRAME_S_BACKGROUND = GFL_BG_FRAME3_S,      ///<背景面
};

///インフォメーションメッセージ更新ウェイト
#define INFOMSG_UPDATE_WAIT   (60)

///フォントBGパレット展開位置
#define _FONT_BG_PALNO      (14)

///アクターIndex
enum{
  INTSUB_ACTOR_TOUCH_TOWN_0,
  INTSUB_ACTOR_TOUCH_TOWN_1,
  INTSUB_ACTOR_TOUCH_TOWN_2,
  INTSUB_ACTOR_TOUCH_TOWN_3,
  INTSUB_ACTOR_TOUCH_TOWN_4,
  INTSUB_ACTOR_TOUCH_TOWN_5,
  INTSUB_ACTOR_TOUCH_TOWN_6,
  INTSUB_ACTOR_TOUCH_TOWN_7,
  INTSUB_ACTOR_TOUCH_TOWN_MAX = INTSUB_ACTOR_TOUCH_TOWN_7,

  INTSUB_ACTOR_TOUCH_TOWN_EFF_0,
  INTSUB_ACTOR_TOUCH_TOWN_EFF_1,
  INTSUB_ACTOR_TOUCH_TOWN_EFF_2,
  INTSUB_ACTOR_TOUCH_TOWN_EFF_3,
  INTSUB_ACTOR_TOUCH_TOWN_EFF_4,
  INTSUB_ACTOR_TOUCH_TOWN_EFF_5,
  INTSUB_ACTOR_TOUCH_TOWN_EFF_6,
  INTSUB_ACTOR_TOUCH_TOWN_EFF_7,
  INTSUB_ACTOR_TOUCH_TOWN_EFF_MAX = INTSUB_ACTOR_TOUCH_TOWN_EFF_7,

  INTSUB_ACTOR_TOUCH_PALACE,
  INTSUB_ACTOR_TOUCH_PALACE_EFF,
  
  INTSUB_ACTOR_AREA_0,
  INTSUB_ACTOR_AREA_1,
  INTSUB_ACTOR_AREA_2,
  INTSUB_ACTOR_AREA_MAX = INTSUB_ACTOR_AREA_2,

  INTSUB_ACTOR_CUR_S_0,
  INTSUB_ACTOR_CUR_S_1,
  INTSUB_ACTOR_CUR_S_2,
  INTSUB_ACTOR_CUR_S_MAX = INTSUB_ACTOR_CUR_S_2,

  INTSUB_ACTOR_CUR_L,       ///<自分の居場所を指す
  INTSUB_ACTOR_ENTRY,       ///<「参加」ボタン

  INTSUB_ACTOR_WARP_NG_0,
  INTSUB_ACTOR_WARP_NG_1,
  INTSUB_ACTOR_WARP_NG_2,
  INTSUB_ACTOR_WARP_NG_MAX = INTSUB_ACTOR_WARP_NG_2,
  
//  INTSUB_ACTOR_MARK,        ///<自分のいるエリアを指す

  INTSUB_ACTOR_LV_NUM_KETA_0,
  INTSUB_ACTOR_LV_NUM_KETA_1,
  INTSUB_ACTOR_LV_NUM_KETA_2,
  INTSUB_ACTOR_LV_NUM_KETA_MAX = INTSUB_ACTOR_LV_NUM_KETA_2,

  INTSUB_ACTOR_MAX,
};

///BMPOAMで使用するアクター数
#define INTSUB_ACTOR_BMPOAM_NUM   (16 + 4)

///OBJパレットINDEX
enum{
  INTSUB_ACTOR_PAL_BASE_START = 1,    ///<プレイヤー毎に変わるパレット開始位置
  
  INTSUB_ACTOR_PAL_MAX = 7,
  
  INTSUB_ACTOR_PAL_FONT = 0xd,
};

///palace_obj.nceのアニメーションシーケンス
enum{
//  PALACE_ACT_ANMSEQ_TOWN_G,
//  PALACE_ACT_ANMSEQ_TOWN_W,
//  PALACE_ACT_ANMSEQ_TOWN_B,
  PALACE_ACT_ANMSEQ_AREA,
  PALACE_ACT_ANMSEQ_CUR_S,
  PALACE_ACT_ANMSEQ_CUR_L_MALE,
  PALACE_ACT_ANMSEQ_CUR_L_FEMALE,
//  PALACE_ACT_ANMSEQ_MARK,
//  PALACE_ACT_ANMSEQ_POWER,
//  PALACE_ACT_ANMSEQ_MISSION,
//  PALACE_ACT_ANMSEQ_INCLUSION,
  PALACE_ACT_ANMSEQ_SENKYO_EFF,
  PALACE_ACT_ANMSEQ_LV_NUM,
  PALACE_ACT_ANMSEQ_POINT_NUM,
  PALACE_ACT_ANMSEQ_TOUCH_TOWN,
  PALACE_ACT_ANMSEQ_TOUCH_PALACE,
  PALACE_ACT_ANMSEQ_BTN,
  PALACE_ACT_ANMSEQ_WARP_OK,
  PALACE_ACT_ANMSEQ_WARP_NG,
  PALACE_ACT_ANMSEQ_TOUCH_TOWN_EFF,
  PALACE_ACT_ANMSEQ_TOUCH_PALACE_EFF,
};

///アクターソフトプライオリティ
enum{
  SOFTPRI_TOUCH_TOWN = 90,
  SOFTPRI_TOUCH_TOWN_EFF = 100,
  SOFTPRI_AREA = 50,
  SOFTPRI_CUR_S = 20,
  SOFTPRI_CUR_L = 19,
  SOFTPRI_WARP_NG = 30,
  SOFTPRI_LV_NUM = 10,
  SOFTPRI_INFOMSG = 8,
  SOFTPRI_ENTRY_BUTTON = 5,
  SOFTPRI_ENTRY_BUTTON_MSG = 4,
};
///アクター共通BGプライオリティ
#define BGPRI_ACTOR_COMMON      (2)

///BGパレットINDEX
enum{
  INTSUB_BG_PAL_INFOMSG_BACK_NORMAL = 0, ///<infoメッセージの背景色(ノーマル)
  INTSUB_BG_PAL_INFOMSG_BACK_ACTIVE = 9, ///<infoメッセージの背景色(アクティブ時)
  
  INTSUB_BG_PAL_BASE_START_TOWN = 1,   ///<プレイヤー毎に変わるパレット開始位置(街BG)
  INTSUB_BG_PAL_BASE_START_BACK = 5,   ///<プレイヤー毎に変わるパレット開始位置(背景)
  
  INTSUB_BG_PAL_MAX = 10,
};

///街アイコン更新リクエスト
enum{
  TOWN_UPDATE_REQ_NONE,           ///<リクエスト無し
  TOWN_UPDATE_REQ_NOT_EFFECT,     ///<街アイコンの更新はするが占拠エフェクトは無し
  TOWN_UPDATE_REQ_UPDATE,         ///<街アイコンの更新＋変化があれば占拠エフェクト
};

enum{
  _TOWN_NO_FREE = 0xfe,       ///<街でもパレスでもない
  _TOWN_NO_PALACE = 0xff,     ///<パレス
};

enum{
  NO_TOWN_CURSOR_POS_SPACE_X = 24,
  NO_TOWN_CURSOR_POS_X = 128 - NO_TOWN_CURSOR_POS_SPACE_X * 2,
  NO_TOWN_CURSOR_POS_Y = 0xe*8,
};

enum{
  TOUCH_RANGE_PLAYER_ICON_X = 8,    ///<相手プレイヤーアイコンのタッチ範囲X(半径)
  TOUCH_RANGE_PLAYER_ICON_Y = 8,    ///<相手プレイヤーアイコンのタッチ範囲Y(半径)
};

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
///侵入エリアによってカラーを変更するBGスクリーンの位置とサイズ
enum{
  _AREA_SCREEN_COLOR_START_X = 0,
  _AREA_SCREEN_COLOR_START_Y = 2,
  _AREA_SCREEN_COLOR_SIZE_X = 32,
  _AREA_SCREEN_COLOR_SIZE_Y = 16,
};

///インフォメーションメッセージのアクティブ化によってカラーを変更するBGスクリーンの位置とサイズ
enum{
  _INFOMSG_SCREEN_COLOR_START_X = 0,
  _INFOMSG_SCREEN_COLOR_START_Y = 0x12,
  _INFOMSG_SCREEN_COLOR_SIZE_X = 32,
  _INFOMSG_SCREEN_COLOR_SIZE_Y = 4,
};

enum{
  INFOMSG_BMP_SIZE_X = 30,  //キャラ単位
  INFOMSG_BMP_SIZE_Y = 4,   //キャラ単位
};

enum{
  ENTRYMSG_BMP_SIZE_X = 6,  //キャラ単位
  ENTRYMSG_BMP_SIZE_Y = 2,   //キャラ単位
};

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
///街アイコンのタッチ判定の矩形ハーフサイズ
#define TOWN_ICON_HITRANGE_HALF   (8)

enum{
  BG_BAR_TYPE_SINGLE,   ///<一人用のBGスクリーン(palace_bar.nsc)
  BG_BAR_TYPE_COMM,     ///<通信用のBGスクリーン(palace_bar2.nsc)
};

///タイトルメッセージ表示タイプ
enum{
  _TITLE_PRINT_NULL,              ///<何も表示していない
  _TITLE_PRINT_PALACE_GO,         ///<タッチでパレスへいけます
  _TITLE_PRINT_MY_PALACE,         ///<自分のパレスにいます
  _TITLE_PRINT_OTHER_PALACE0,     ///<通信相手のパレスにいます
  _TITLE_PRINT_OTHER_PALACE1,     ///<通信相手のパレスにいます
  _TITLE_PRINT_OTHER_PALACE2,     ///<通信相手のパレスにいます
  _TITLE_PRINT_OTHER_PALACE3,     ///<通信相手のパレスにいます
};

///参加ボタンの表示タイプ
enum{
  ENTRY_BUTTON_MSG_PATERN_ENTRY,    ///<「さんか」
  ENTRY_BUTTON_MSG_PATERN_BACK,     ///<「もどる」
  
  ENTRY_BUTTON_MSG_PATERN_MAX,
};

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
///通信相手が不明ゾーンにいる場合の表示座標X
#define NOTHING_ZONE_SUB_POS_X            (240)
///通信相手が不明ゾーンにいる場合の表示座標Y
#define NOTHING_ZONE_SUB_POS_Y            (64)
///通信相手が不明ゾーンにいる場合の表示座標間隔Y
#define NOTHING_ZONE_SUB_POS_Y_SPACE      (24)

//--------------------------------------------------------------
//  イベントNo
//--------------------------------------------------------------
enum{
  _EVENT_REQ_NO_NULL,             ///<イベントリクエスト無し
  _EVENT_REQ_NO_TOWN_WARP,        ///<街へワープ
  _EVENT_REQ_NO_PLAYER_WARP,      ///<プレイヤーのいる所へワープ
  _EVENT_REQ_NO_MISSION_ENTRY,    ///<ミッションに参加
};


//==============================================================================
//  構造体定義
//==============================================================================
///intcommから取得出来るパラメータ類(非通信の時はソロ用の値がセットされる)
typedef struct{
  u8 now_palace_area;       ///<現在自分がいるパレスエリア
  u8 recv_profile;          ///<受信したプロフィール(bit管理)
  u8 recv_num;              ///<受信したプロフィール人数
  u8 palace_in;             ///<TRUE:パレス島におとずれた
  MISSION_STATUS m_status;  ///<ミッション状況
  s32 m_timer;              ///<ミッションタイマー
  const MISSION_DATA *p_md; ///<受信しているミッションデータへのポインタ
}INTRUDE_COMM_PARAM;

///侵入下画面制御ワーク
typedef struct _INTRUDE_SUBDISP{
  GAMESYS_WORK *gsys;
  
  u32 index_cgr;
  u32 index_pltt;
  u32 index_pltt_font;
  u32 index_cell;

  GFL_FONT *font_handle;
  GFL_TCBLSYS *tcbl_sys;
  WORDSET *wordset;
  PRINT_QUE *print_que;
	GFL_MSGDATA *msgdata;
	
	STRBUF *strbuf_temp;
	STRBUF *strbuf_info;
	STRBUF *strbuf_title;
	
	PRINT_UTIL printutil_title;  ///<タイトルメッセージ
	
	GFL_CLUNIT *clunit;
  GFL_CLWK *act[INTSUB_ACTOR_MAX];

  BMPOAM_SYS_PTR bmpoam_sys;
  GFL_BMP_DATA *entrymsg_bmp[ENTRY_BUTTON_MSG_PATERN_MAX];
  GFL_BMP_DATA *backmsg_bmp;
  GFL_BMP_DATA *infomsg_bmp;
  BMPOAM_ACT_PTR entrymsg_bmpoam[ENTRY_BUTTON_MSG_PATERN_MAX];
  BMPOAM_ACT_PTR infomsg_bmpoam;
  
  INTRUDE_COMM_PARAM comm;  ///<intcommから取得出来るパラメータ類(非通信の時はソロ用の値)
  
  u8 my_net_id;
  u8 town_update_req;     ///<街アイコン更新リクエスト(TOWN_UPDATE_REQ_???)
  u8 now_bg_pal;          ///<現在のBGのパレット番号
  u8 wfbc_go;             ///<TRUE:WFBCへのワープを押した
  
  s16 infomsg_wait;       ///<インフォメーションメッセージ更新ウェイト
  u8 wfbc_seq;
  u8 bar_type;            ///<BG_BAR_TYPE_xxx
  
  u8 title_print_type;    ///<_TITLE_PRINT_xxx
  u8 infomsg_trans_req;
  u8 print_mission_status;  ///<現在表示しているミッション状況
  u8 back_exit;           ///< TRUE:「もどる」ボタンを押して下画面終了モードになっている
  
  u16 warp_zone_id;
  u8 event_req;           ///< _EVENT_REQ_NO_xxx
  u8 padding;
}INTRUDE_SUBDISP;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _IntSub_SystemSetup(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_SystemExit(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BGLoad(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_BGUnload(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BmpWinAdd(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BmpWinDel(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BmpOamCreate(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_BmpOamDelete(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorResouceLoad(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorResourceUnload(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorCreate(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorDelete(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorCreate_TouchTown(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_Area(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_CursorS(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_CursorL(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_WarpNG(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_LvNum(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_EntryButton(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_Delete_EntryButton(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorUpdate_TouchTown(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_Area(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_CursorS(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_CursorL(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy, ZONEID my_zone_id);
static void _IntSub_ActorUpdate_EntryButton(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_LvNum(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy);
static OCCUPY_INFO * _IntSub_GetArreaOccupy(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_TitleMsgUpdate(INTRUDE_SUBDISP_PTR intsub, ZONEID my_zone_id);
static void _IntSub_InfoMsgUpdate(INTRUDE_SUBDISP_PTR intsub);
static void _SetRect(int x, int y, int half_size_x, int half_size_y, GFL_RECT *rect);
static BOOL _CheckRectHit(int x, int y, const GFL_RECT *rect);
static void _IntSub_TouchUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BGBarUpdate(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BGColorUpdate(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_CommParamInit(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm);
static void _IntSub_CommParamUpdate(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm);


//==============================================================================
//  データ
//==============================================================================
///パレスアイコン座標
enum{
  PALACE_ICON_POS_X = 128,        ///<パレスアイコン座標X
  PALACE_ICON_POS_Y = 0xb*8+4,    ///<パレスアイコン座標Y
  PALACE_ICON_HITRANGE_HALF = 16, ///<パレスアイコンのタッチ判定半径
  
  PALACE_CURSOR_POS_X = PALACE_ICON_POS_X,      ///<パレスアイコンをカーソルが指す場合の座標X
  PALACE_CURSOR_POS_Y = PALACE_ICON_POS_Y - 16, ///<パレスアイコンをカーソルが指す場合の座標Y
};

///エリアアイコン配置座標
enum{
  AREA_POST_LEFT = 12 * 8,                           ///<配置座標範囲の左端X
  AREA_POST_RIGHT = 0x14 * 8,                         ///<配置座標範囲の右端X
  AREA_POST_WIDTH = AREA_POST_RIGHT - AREA_POST_LEFT, ///<配置座標範囲の幅
  
  AREA_POST_Y = 3*8 + 4,                              ///<配置座標Y
};

///参加ボタンのアイコン座標
enum{
  ENTRY_BUTTON_POS_X = 0x1c*8,        ///<参加ボタンのアイコン座標X
  ENTRY_BUTTON_POS_Y = 0x10*8,        ///<参加ボタンのアイコン座標Y
  ENTRY_BUTTON_HITRANGE_HALF_X = 16,  ///<参加ボタンのタッチ判定半径X
  ENTRY_BUTTON_HITRANGE_HALF_Y = 8,   ///<参加ボタンのタッチ判定半径Y
};

///各通信プレイヤー毎にずらす値
static const s8 WearOffset[FIELD_COMM_MEMBER_MAX][2] = {
  {-4, -4}, {4, -4}, {0, 4},
};



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 侵入下画面：初期設定
 *
 * @param   gsys		
 *
 * @retval  INTRUDE_SUBDISP_PTR		
 */
//==================================================================
INTRUDE_SUBDISP_PTR INTRUDE_SUBDISP_Init(GAMESYS_WORK *gsys)
{
  INTRUDE_SUBDISP_PTR intsub;
  ARCHANDLE *handle;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  
  intsub = GFL_HEAP_AllocClearMemory(HEAPID_FIELD_SUBSCREEN, sizeof(INTRUDE_SUBDISP));
  intsub->gsys = gsys;
  intsub->now_bg_pal = 0xff;  //初回に必ず更新がかかるように0xff
  
  intsub->my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  _IntSub_CommParamInit(intsub, Intrude_Check_CommConnect(game_comm));
  
  handle = GFL_ARC_OpenDataHandle(ARCID_PALACE, HEAPID_FIELDMAP);
  
  _IntSub_SystemSetup(intsub);
  _IntSub_BGLoad(intsub, handle);
  _IntSub_BmpWinAdd(intsub);
  _IntSub_ActorResouceLoad(intsub, handle);
  _IntSub_ActorCreate(intsub, handle);
  _IntSub_BmpOamCreate(intsub, handle);
  
  GFL_ARC_CloseDataHandle(handle);

  return intsub;
}

//==================================================================
/**
 * 侵入下画面：破棄設定
 *
 * @param   intsub		
 */
//==================================================================
void INTRUDE_SUBDISP_Exit(INTRUDE_SUBDISP_PTR intsub)
{
  _IntSub_BmpOamDelete(intsub);
  _IntSub_Delete_EntryButton(intsub);
  _IntSub_ActorDelete(intsub);
  _IntSub_ActorResourceUnload(intsub);
  _IntSub_BmpWinDel(intsub);
  _IntSub_BGUnload(intsub);
  _IntSub_SystemExit(intsub);

  GFL_HEAP_FreeMemory(intsub);
}

//==================================================================
/**
 * 侵入下画面：更新
 *
 * @param   intsub		
 */
//==================================================================
void INTRUDE_SUBDISP_Update(INTRUDE_SUBDISP_PTR intsub, BOOL bActive)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  int i;

  intsub->my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  _IntSub_CommParamUpdate(intsub, intcomm);
  
	GFL_TCBL_Main( intsub->tcbl_sys );
	PRINTSYS_QUE_Main( intsub->print_que );
  PRINT_UTIL_Trans( &intsub->printutil_title, intsub->print_que );
  if(intsub->infomsg_trans_req == TRUE 
      && PRINTSYS_QUE_IsExistTarget(intsub->print_que, intsub->infomsg_bmp) == FALSE){
    BmpOam_ActorBmpTrans(intsub->infomsg_bmpoam);
    intsub->infomsg_trans_req = FALSE;
  }
  for(i = 0; i < ENTRY_BUTTON_MSG_PATERN_MAX; i++){
    if(PRINTSYS_QUE_IsExistTarget(intsub->print_que, intsub->entrymsg_bmp[i]) == FALSE){
      BmpOam_ActorBmpTrans(intsub->entrymsg_bmpoam[i]);
    }
  }
  
  //タッチ判定チェック
  _IntSub_TouchUpdate(intcomm, intsub);
  
  //WFBCへのワープチェック
  if(intcomm != NULL){
    if(intsub->wfbc_go == TRUE){
      int palace_area = Intrude_GetPalaceArea(intcomm);
      
      if(GFL_NET_IsConnectMember(palace_area) == FALSE){
        intsub->wfbc_go = FALSE;
        OS_TPrintf("WFBCへの飛び先相手がいなくなったのでキャンセル netID=%d\n", palace_area);
      }
      else{
        switch(intsub->wfbc_seq){
        case 0:
          if(IntrudeSend_WfbcReq(intcomm, palace_area) == TRUE){
            intsub->wfbc_seq++;
          }
          break;
        case 1:
          if(Intrude_GetRecvWfbc(intcomm) == TRUE){
            FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(intsub->gsys);
            FIELD_SUBSCREEN_WORK *subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);

            intsub->warp_zone_id = ZONE_ID_PLC10;
            intsub->event_req = _EVENT_REQ_NO_TOWN_WARP;
            intsub->wfbc_go = FALSE;
          }
          break;
        default:
          GF_ASSERT(0);
          break;
        }
      }
    }
  }
}

//==================================================================
/**
 * 侵入下画面：描画
 *
 * @param   intsub		
 */
//==================================================================
void INTRUDE_SUBDISP_Draw(INTRUDE_SUBDISP_PTR intsub, BOOL bActive)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  PLAYER_WORK *player_work = GAMESYSTEM_GetMyPlayerWork(intsub->gsys);
  ZONEID my_zone_id = PLAYERWORK_getZoneID(player_work);
  OCCUPY_INFO *area_occupy;
  
  area_occupy = _IntSub_GetArreaOccupy(intsub);

  _IntSub_BGBarUpdate(intsub);
  //BGスクリーンカラー変更チェック
  _IntSub_BGColorUpdate(intsub);
  
  //インフォメーションメッセージ
  _IntSub_TitleMsgUpdate(intsub, my_zone_id);
  _IntSub_InfoMsgUpdate(intsub);
  
  //アクター更新
  _IntSub_ActorUpdate_TouchTown(intsub, area_occupy);
  _IntSub_ActorUpdate_Area(intsub, area_occupy);
  _IntSub_ActorUpdate_CursorS(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_CursorL(intsub, area_occupy, my_zone_id);
  _IntSub_ActorUpdate_EntryButton(intsub, area_occupy);
  _IntSub_ActorUpdate_LvNum(intsub, area_occupy);
}

//==================================================================
/**
 * 侵入下画面：イベント起動チェック
 *
 * @param   intsub		
 * @param   bEvReqOK		TRUE:イベント起動してもよい　FALSE：他のイベントが起動中
 *
 * @retval  GMEVENT*		
 */
//==================================================================
GMEVENT* INTRUDE_SUBDISP_EventCheck(INTRUDE_SUBDISP_PTR intsub, BOOL bEvReqOK, FIELD_SUBSCREEN_WORK* subscreen )
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(intsub->gsys);

  if(bEvReqOK == FALSE || fieldWork == NULL){
    return NULL;
  }
  
  if(intsub->back_exit == TRUE){
    FIELD_SUBSCREEN_ChangeForce( subscreen, FIELD_SUBSCREEN_NORMAL );
    return NULL;
  }

  switch(intsub->event_req){
  case _EVENT_REQ_NO_TOWN_WARP:
    PMSND_PlaySE( SEQ_SE_FLD_102 );
    return EVENT_IntrudeTownWarp(intsub->gsys, fieldWork, intsub->warp_zone_id);
  case _EVENT_REQ_NO_PLAYER_WARP:
    PMSND_PlaySE( SEQ_SE_FLD_102 );
    return EVENT_IntrudePlayerWarp(intsub->gsys, fieldWork, Intrude_GetWarpPlayerNetID(game_comm));
  case _EVENT_REQ_NO_MISSION_ENTRY:
    return EVENT_Intrude_MissionStartWait(intsub->gsys);
  }
  
  return NULL;
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * システム関連のセットアップ
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_SystemSetup(INTRUDE_SUBDISP_PTR intsub)
{
  intsub->clunit = GFL_CLACT_UNIT_Create(
    INTSUB_ACTOR_MAX + INTSUB_ACTOR_BMPOAM_NUM, 5, HEAPID_FIELD_SUBSCREEN);
  intsub->bmpoam_sys = BmpOam_Init(HEAPID_FIELD_SUBSCREEN, intsub->clunit);

	intsub->tcbl_sys = GFL_TCBL_Init(HEAPID_FIELD_SUBSCREEN, HEAPID_FIELD_SUBSCREEN, 4, 32);
	intsub->font_handle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_FIELD_SUBSCREEN );
	intsub->wordset = WORDSET_Create(HEAPID_FIELD_SUBSCREEN);
  intsub->print_que = PRINTSYS_QUE_Create( HEAPID_FIELD_SUBSCREEN );
  intsub->msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
    NARC_message_invasion_dat, HEAPID_FIELD_SUBSCREEN );
  
  intsub->strbuf_temp = GFL_STR_CreateBuffer(128, HEAPID_FIELD_SUBSCREEN);
  intsub->strbuf_info = GFL_STR_CreateBuffer(128, HEAPID_FIELD_SUBSCREEN);
  intsub->strbuf_title = GFL_STR_CreateBuffer(128, HEAPID_FIELD_SUBSCREEN);
}

//--------------------------------------------------------------
/**
 * システム関連の破棄
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_SystemExit(INTRUDE_SUBDISP_PTR intsub)
{
  GFL_STR_DeleteBuffer(intsub->strbuf_temp);
  GFL_STR_DeleteBuffer(intsub->strbuf_info);
  GFL_STR_DeleteBuffer(intsub->strbuf_title);
  
  GFL_MSG_Delete(intsub->msgdata);
  PRINTSYS_QUE_Clear(intsub->print_que);
  PRINTSYS_QUE_Delete(intsub->print_que);
  WORDSET_Delete(intsub->wordset);
	GFL_FONT_Delete(intsub->font_handle);
	GFL_TCBL_Exit(intsub->tcbl_sys);

  BmpOam_Exit(intsub->bmpoam_sys);
  GFL_CLACT_UNIT_Delete(intsub->clunit);
}

//--------------------------------------------------------------
/**
 * BGロード
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_BGLoad(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
	static const GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
  	{//INTRUDE_FRAME_S_BAR
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x7000,GX_BG_EXTPLTT_01,
  		3, 0, 0, FALSE
  	},
  	{//INTRUDE_FRAME_S_BACKGROUND
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, 0x7000,GX_BG_EXTPLTT_01,
  		3, 0, 0, FALSE
  	},
	};
	GFL_BG_SetBGControl( INTRUDE_FRAME_S_BAR, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( INTRUDE_FRAME_S_BACKGROUND, &TextBgCntDat[1], GFL_BG_MODE_TEXT );

	//BG VRAMクリア
	GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);

	//レジスタOFF
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);
	G2S_BlendNone();
	
  //キャラ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    handle, NARC_palace_palace_bg_lz_NCGR, INTRUDE_FRAME_S_BACKGROUND, 0, 
    0, TRUE, HEAPID_FIELDMAP);
  
  //スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_palace_palace_bg_lz_NSCR, INTRUDE_FRAME_S_BACKGROUND, 0, 
    0x800, TRUE, HEAPID_FIELDMAP);
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_palace_palace_bar_lz_NSCR, INTRUDE_FRAME_S_BAR, 0, 
    0x800, TRUE, HEAPID_FIELDMAP);
  
  //パレット転送
  GFL_ARCHDL_UTIL_TransVramPalette(handle, NARC_palace_palace_bg_NCLR, PALTYPE_SUB_BG, 0, 
    0x20 * INTSUB_BG_PAL_MAX, HEAPID_FIELDMAP);
	//フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, 
    PALTYPE_SUB_BG, _FONT_BG_PALNO * 0x20, 0x20, HEAPID_FIELDMAP);

	GFL_BG_SetVisible( INTRUDE_FRAME_S_NULL_0, VISIBLE_OFF );
	GFL_BG_SetVisible( INTRUDE_FRAME_S_NULL_1, VISIBLE_OFF );
	GFL_BG_SetVisible( INTRUDE_FRAME_S_BAR, VISIBLE_ON );
	GFL_BG_SetVisible( INTRUDE_FRAME_S_BACKGROUND, VISIBLE_ON );
}

//--------------------------------------------------------------
/**
 * BGアンロード
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BGUnload(INTRUDE_SUBDISP_PTR intsub)
{
	GFL_BG_SetVisible( INTRUDE_FRAME_S_BAR, VISIBLE_OFF );
	GFL_BG_SetVisible( INTRUDE_FRAME_S_BACKGROUND, VISIBLE_OFF );
	GFL_BG_FreeBGControl(INTRUDE_FRAME_S_BAR);
	GFL_BG_FreeBGControl(INTRUDE_FRAME_S_BACKGROUND);
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BmpWinAdd(INTRUDE_SUBDISP_PTR intsub)
{
  GFL_BMPWIN *bmpwin;
  
  bmpwin = GFL_BMPWIN_Create( INTRUDE_FRAME_S_BAR, 
    0, 0, 0x20, 2, _FONT_BG_PALNO, GFL_BMP_CHRAREA_GET_B );
  PRINT_UTIL_Setup( &intsub->printutil_title, bmpwin );
  GFL_BMPWIN_MakeTransWindow(bmpwin);
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BmpWinDel(INTRUDE_SUBDISP_PTR intsub)
{
  GFL_BMPWIN_Delete(intsub->printutil_title.win);
}

//--------------------------------------------------------------
/**
 * BMPOAM作成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_BmpOamCreate(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  BMPOAM_ACT_DATA bmpoam_head = {
    NULL,   //OAMとして表示させるBMPデータへのポインタ
    8, 0x12*8,   //X, Y
    0,      //適用するパレットのindex(GFL_CLGRP_PLTT_Registerの戻り値)
    0,      //pal_offset(pltt_indexのパレット内でのオフセット)
    SOFTPRI_INFOMSG,
    BGPRI_ACTOR_COMMON,
    CLSYS_DEFREND_SUB,
    CLSYS_DRAW_SUB,
  };

  intsub->infomsg_bmp = GFL_BMP_Create( 
    INFOMSG_BMP_SIZE_X, INFOMSG_BMP_SIZE_Y, GFL_BMP_16_COLOR, HEAPID_FIELD_SUBSCREEN );
  
  bmpoam_head.bmp = intsub->infomsg_bmp;
  bmpoam_head.pltt_index = intsub->index_pltt_font;
  intsub->infomsg_bmpoam = BmpOam_ActorAdd(intsub->bmpoam_sys, &bmpoam_head);
}

//--------------------------------------------------------------
/**
 * BMPOAM削除
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BmpOamDelete(INTRUDE_SUBDISP_PTR intsub)
{
  BmpOam_ActorDel(intsub->infomsg_bmpoam);
  GFL_BMP_Delete(intsub->infomsg_bmp);
}

//--------------------------------------------------------------
/**
 * アクターで使用するリソースのロード
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorResouceLoad(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  //OBJ共通パレット
  intsub->index_pltt = GFL_CLGRP_PLTT_RegisterEx(
    handle, NARC_palace_palace_obj_NCLR, CLSYS_DRAW_SUB, 0, 0, 
    INTSUB_ACTOR_PAL_MAX, HEAPID_FIELDMAP );

  //OBJ共通リソース
  intsub->index_cgr = GFL_CLGRP_CGR_Register(
    handle, NARC_palace_palace_obj_lz_NCGR, TRUE, CLSYS_DRAW_SUB, HEAPID_FIELDMAP);
  intsub->index_cell = GFL_CLGRP_CELLANIM_Register(
    handle, NARC_palace_palace_obj_NCER, 
    NARC_palace_palace_obj_NANR, HEAPID_FIELD_SUBSCREEN);
  
  //font oam用のフォントパレット
  {
    ARCHANDLE *font_pal_handle;
    font_pal_handle = GFL_ARC_OpenDataHandle(ARCID_FONT, HEAPID_FIELDMAP);
    intsub->index_pltt_font = GFL_CLGRP_PLTT_RegisterComp(
      font_pal_handle, NARC_font_default_nclr, CLSYS_DRAW_SUB, 
      INTSUB_ACTOR_PAL_FONT * 0x20, HEAPID_FIELDMAP );
    GFL_ARC_CloseDataHandle(font_pal_handle);
  }
}

//--------------------------------------------------------------
/**
 * アクターで使用するアクターのアンロード
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_ActorResourceUnload(INTRUDE_SUBDISP_PTR intsub)
{
  GFL_CLGRP_PLTT_Release(intsub->index_pltt_font);
  GFL_CLGRP_PLTT_Release(intsub->index_pltt);
  GFL_CLGRP_CGR_Release(intsub->index_cgr);
  GFL_CLGRP_CELLANIM_Release(intsub->index_cell);
}

//--------------------------------------------------------------
/**
 * アクター作成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  _IntSub_ActorCreate_TouchTown(intsub, handle);
  _IntSub_ActorCreate_Area(intsub, handle);
  _IntSub_ActorCreate_CursorS(intsub, handle);
  _IntSub_ActorCreate_CursorL(intsub, handle);
  _IntSub_ActorCreate_WarpNG(intsub, handle);
  _IntSub_ActorCreate_LvNum(intsub, handle);
  _IntSub_ActorCreate_EntryButton(intsub, handle);
}

//--------------------------------------------------------------
/**
 * アクター削除
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_ActorDelete(INTRUDE_SUBDISP_PTR intsub)
{
  int i;
  
  for(i = 0; i < INTSUB_ACTOR_MAX; i++){
    GFL_CLACT_WK_Remove(intsub->act[i]);
  }
}

//--------------------------------------------------------------
/**
 * タッチ出来る街アクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_TouchTown(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i, effno;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_TOUCH_TOWN,   //アニメーションシーケンス
  	SOFTPRI_TOUCH_TOWN,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  GFL_CLWK_DATA eff_head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_TOUCH_TOWN_EFF,   //アニメーションシーケンス
  	SOFTPRI_TOUCH_TOWN_EFF,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  GF_ASSERT(PALACE_TOWN_DATA_MAX == (INTSUB_ACTOR_TOUCH_TOWN_MAX - INTSUB_ACTOR_TOUCH_TOWN_0 + 1));
  effno = INTSUB_ACTOR_TOUCH_TOWN_EFF_0;
  for(i = INTSUB_ACTOR_TOUCH_TOWN_0; i <= INTSUB_ACTOR_TOUCH_TOWN_MAX; i++){
    head.pos_x = PalaceTownData[i - INTSUB_ACTOR_TOUCH_TOWN_0].subscreen_x;
    head.pos_y = PalaceTownData[i - INTSUB_ACTOR_TOUCH_TOWN_0].subscreen_y;

    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF

    eff_head.pos_x = head.pos_x;
    eff_head.pos_y = head.pos_y;
    intsub->act[effno] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &eff_head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[effno], TRUE);  //オートアニメON
    GFL_CLACT_WK_SetDrawEnable(intsub->act[effno], FALSE);  //表示OFF
    effno++;
  }
  
  //タッチパレス島
  head.anmseq = PALACE_ACT_ANMSEQ_TOUCH_PALACE;
  head.pos_x = PALACE_ICON_POS_X;
  head.pos_y = PALACE_ICON_POS_Y;
  intsub->act[INTSUB_ACTOR_TOUCH_PALACE] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], FALSE);  //表示OFF
  //タッチパレス島エフェクト
  eff_head.anmseq = PALACE_ACT_ANMSEQ_TOUCH_PALACE_EFF;
  eff_head.pos_x = PALACE_ICON_POS_X;
  eff_head.pos_y = PALACE_ICON_POS_Y;
  intsub->act[INTSUB_ACTOR_TOUCH_PALACE_EFF] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &eff_head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
  GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[INTSUB_ACTOR_TOUCH_PALACE_EFF], TRUE);  //オートアニメON
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE_EFF], FALSE);  //表示OFF
}

//--------------------------------------------------------------
/**
 * 接続エリアアイコンアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_Area(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_AREA,     //アニメーションシーケンス
  	SOFTPRI_AREA,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  for(i = INTSUB_ACTOR_AREA_0; i <= INTSUB_ACTOR_AREA_MAX; i++){
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetPlttOffs(intsub->act[i], 
      INTSUB_ACTOR_PAL_BASE_START + i-INTSUB_ACTOR_AREA_0, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
  }
}

//--------------------------------------------------------------
/**
 * 通信相手の居場所を示すカーソルアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_CursorS(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_CUR_S,    //アニメーションシーケンス
  	SOFTPRI_CUR_S,              //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  for(i = INTSUB_ACTOR_CUR_S_0; i <= INTSUB_ACTOR_CUR_S_MAX; i++){
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetPlttOffs(intsub->act[i], 
      INTSUB_ACTOR_PAL_BASE_START + i-INTSUB_ACTOR_CUR_S_0, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
    GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[i], TRUE);  //オートアニメON
  }
}

//--------------------------------------------------------------
/**
 * 自分の居場所を示すカーソルアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_CursorL(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_CUR_L_MALE,     //アニメーションシーケンス
  	SOFTPRI_CUR_L,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  MYSTATUS *myst = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(intsub->gsys));
  
  if(MyStatus_GetMySex(myst) == PM_FEMALE){
    head.anmseq = PALACE_ACT_ANMSEQ_CUR_L_FEMALE;
  }
  intsub->act[INTSUB_ACTOR_CUR_L] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
  GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_CUR_L], 
    INTSUB_ACTOR_PAL_BASE_START + intsub->my_net_id, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_CUR_L], FALSE);  //表示OFF
  GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[INTSUB_ACTOR_CUR_L], TRUE);  //オートアニメON
}

//--------------------------------------------------------------
/**
 * 通信相手のワープNGを示すアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_WarpNG(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_WARP_NG,    //アニメーションシーケンス
  	SOFTPRI_WARP_NG,              //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  for(i = INTSUB_ACTOR_WARP_NG_0; i <= INTSUB_ACTOR_WARP_NG_MAX; i++){
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetPlttOffs(intsub->act[i], 
      INTSUB_ACTOR_PAL_BASE_START + i-INTSUB_ACTOR_WARP_NG_0, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
  }
}

//--------------------------------------------------------------
/**
 * レベル数値アクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_LvNum(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_LV_NUM,   //アニメーションシーケンス
  	SOFTPRI_LV_NUM,             //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  static const struct{
    u16 x;
    u16 y;
  }LvNumPos[] = {
    {0x1a*8 + 4*8, 0x16*8},   //一の位
    {0x1a*8 + 2*8, 0x16*8},   //十の位
    {0x1a*8, 0x16*8},         //百の位
  };
  
  GF_ASSERT(NELEMS(LvNumPos) == (INTSUB_ACTOR_LV_NUM_KETA_MAX - INTSUB_ACTOR_LV_NUM_KETA_0 + 1));
  for(i = INTSUB_ACTOR_LV_NUM_KETA_0; i <= INTSUB_ACTOR_LV_NUM_KETA_MAX; i++){
    head.pos_x = LvNumPos[i - INTSUB_ACTOR_LV_NUM_KETA_0].x;
    head.pos_y = LvNumPos[i - INTSUB_ACTOR_LV_NUM_KETA_0].y;
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], TRUE);
  }
}

//--------------------------------------------------------------
/**
 * 参加ボタンアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_EntryButton(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  static const GFL_CLWK_DATA head = {
  	ENTRY_BUTTON_POS_X, ENTRY_BUTTON_POS_Y,             //X, Y座標
  	PALACE_ACT_ANMSEQ_BTN,      //アニメーションシーケンス
  	SOFTPRI_ENTRY_BUTTON,       //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  int i;
  
  intsub->act[INTSUB_ACTOR_ENTRY] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_ENTRY], FALSE);  //表示OFF

  for(i = 0; i < ENTRY_BUTTON_MSG_PATERN_MAX; i++){//「さんか」BMPOAM
    BMPOAM_ACT_DATA bmpoam_head = {
      NULL,   //OAMとして表示させるBMPデータへのポインタ
      ENTRY_BUTTON_POS_X - 16, ENTRY_BUTTON_POS_Y - 8,   //X, Y
      0,      //適用するパレットのindex(GFL_CLGRP_PLTT_Registerの戻り値)
      0,      //pal_offset(pltt_indexのパレット内でのオフセット)
      SOFTPRI_ENTRY_BUTTON_MSG,
      BGPRI_ACTOR_COMMON,
      CLSYS_DEFREND_SUB,
      CLSYS_DRAW_SUB,
    };
    STRBUF *entry_str;
    
    intsub->entrymsg_bmp[i] = GFL_BMP_Create( 
      ENTRYMSG_BMP_SIZE_X, ENTRYMSG_BMP_SIZE_Y, GFL_BMP_16_COLOR, HEAPID_FIELD_SUBSCREEN );
    
    bmpoam_head.bmp = intsub->entrymsg_bmp[i];
    bmpoam_head.pltt_index = intsub->index_pltt_font;
    intsub->entrymsg_bmpoam[i] = BmpOam_ActorAdd(intsub->bmpoam_sys, &bmpoam_head);
    BmpOam_ActorSetDrawEnable(intsub->entrymsg_bmpoam[i], FALSE);

    entry_str = GFL_MSG_CreateString( intsub->msgdata, msg_invasion_subdisp_000 + i );
    PRINTSYS_PrintQueColor( intsub->print_que, intsub->entrymsg_bmp[i], 0, 0, entry_str, 
      intsub->font_handle, PRINTSYS_MACRO_LSB(15,2,0) );
    GFL_STR_DeleteBuffer(entry_str);
  }
}

//--------------------------------------------------------------
/**
 * 「さんか」ボタンの削除
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_Delete_EntryButton(INTRUDE_SUBDISP_PTR intsub)
{
  int i;
  
  //アクターはまとめて削除されるので、それ以外のBMPOAM等を削除
  for(i = 0; i < ENTRY_BUTTON_MSG_PATERN_MAX; i++){
    BmpOam_ActorDel(intsub->entrymsg_bmpoam[i]);
    GFL_BMP_Delete(intsub->entrymsg_bmp[i]);
  }
}

//==============================================================================
//  更新
//==============================================================================
//--------------------------------------------------------------
/**
 * 更新処理：タッチ街アクター
 *
 * @param   intsub		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_TouchTown(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int i;
  
  if(intsub->comm.now_palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
    //自分のエリアの為、パレス島しかタッチ出来ない
    for(i = 0; i <= INTSUB_ACTOR_TOUCH_TOWN_MAX - INTSUB_ACTOR_TOUCH_TOWN_0; i++){
      GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_0 + i], FALSE);
      GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_EFF_0 + i], FALSE);
    }
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], TRUE);
    
    GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_PALACE_EFF], 
      INTSUB_ACTOR_PAL_BASE_START + intsub->comm.now_palace_area, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE_EFF], TRUE);
  }
  else{
    for(i = 0; i <= INTSUB_ACTOR_TOUCH_TOWN_MAX - INTSUB_ACTOR_TOUCH_TOWN_0; i++){
      GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_0 + i], TRUE);
      GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_EFF_0 + i], TRUE);
      GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_EFF_0 + i], 
        INTSUB_ACTOR_PAL_BASE_START + intsub->comm.now_palace_area, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    }
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], TRUE);
    
    GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_PALACE_EFF], 
      INTSUB_ACTOR_PAL_BASE_START + intsub->comm.now_palace_area, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE_EFF], TRUE);
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：エリアアイコン
 *
 * @param   intsub		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_Area(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int i, profile_num, net_id;
  GFL_CLWK *act;
  GFL_CLACTPOS pos;
  int my_area, now_tbl_pos = 0;
  u8 pal_tbl[FIELD_COMM_MEMBER_MAX] = {0, 0, 0};
  s32 tbl_count = 0, actno;
  static const pos_tbl[2] = {AREA_POST_LEFT, AREA_POST_RIGHT};
  
  profile_num = intsub->comm.recv_num;
  my_area = intsub->comm.now_palace_area;
  pos.y = AREA_POST_Y;
  
  //プロフィール受信済みのユーザーのパレットオフセットテーブルを作る
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(intsub->comm.recv_profile & (1 << net_id)){
      pal_tbl[tbl_count] = net_id;  //net_id = パレットオフセット
      if(net_id == my_area){
        now_tbl_pos = tbl_count;  //自分が今いるパレスエリアのテーブル位置を記憶
      }
      tbl_count++;
    }
  }
  
  actno = 0;
  for(i = 0; i < tbl_count; i++){
    if(now_tbl_pos == i){
      continue;
    }
    GF_ASSERT(actno < NELEMS(pos_tbl));
    pos.x = pos_tbl[actno];
    act = intsub->act[INTSUB_ACTOR_AREA_0 + actno];
    GFL_CLACT_WK_SetPos(act, &pos, CLSYS_DEFREND_SUB);
    GFL_CLACT_WK_SetPlttOffs(act, 
      INTSUB_ACTOR_PAL_BASE_START + pal_tbl[i], CLWK_PLTTOFFS_MODE_PLTT_TOP);
    GFL_CLACT_WK_SetDrawEnable(act, TRUE);
    actno++;
  }
  for( ; i < FIELD_COMM_MEMBER_MAX; i++){
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_AREA_0 + i], FALSE);
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：通信相手カーソル
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_CursorS(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int my_area, net_id;
  GFL_CLWK *act, *act_hate;
  GFL_CLACTPOS pos, pos_hate;
  INTRUDE_STATUS *ist;
  
  my_area = intsub->comm.now_palace_area;
  
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    act = intsub->act[INTSUB_ACTOR_CUR_S_0 + net_id];
    act_hate = intsub->act[INTSUB_ACTOR_WARP_NG_0 + net_id];
    if(net_id == GAMEDATA_GetIntrudeMyID(gamedata)){
      GFL_CLACT_WK_SetDrawEnable(act, FALSE);
      GFL_CLACT_WK_SetDrawEnable(act_hate, FALSE);
      continue;
    }
    else if(intcomm == NULL){
      GFL_CLACT_WK_SetDrawEnable(act, FALSE);
      GFL_CLACT_WK_SetDrawEnable(act_hate, FALSE);
      continue;
    }
    else if(intsub->comm.recv_profile & (1 << net_id)){
      ist = &intcomm->intrude_status[net_id];
    }
    else{
      GFL_CLACT_WK_SetDrawEnable(act, FALSE);
      GFL_CLACT_WK_SetDrawEnable(act_hate, FALSE);
      continue;
    }
    
    if(ist->palace_area == my_area){  //このプレイヤーがいる街を指す
      if(ZONEDATA_IsPalace(ist->zone_id) == TRUE){
        pos.x = PALACE_CURSOR_POS_X + WearOffset[net_id][0];
        pos.y = PALACE_CURSOR_POS_Y + WearOffset[net_id][1];
      }
      else{
        const PALACE_ZONE_SETTING *zonesetting = IntrudeField_GetZoneSettingData(ist->zone_id);
        if(zonesetting != NULL){
          pos.x = zonesetting->sub_x + WearOffset[net_id][0];
          pos.y = zonesetting->sub_y + WearOffset[net_id][1];
          GFL_CLACT_WK_SetDrawEnable(act_hate, FALSE);
        }
        else{ //不明ゾーン
          pos.x = NOTHING_ZONE_SUB_POS_X - 8;
          pos.y = NOTHING_ZONE_SUB_POS_Y + NOTHING_ZONE_SUB_POS_Y_SPACE * net_id;;
          pos_hate = pos;
          pos_hate.x = NOTHING_ZONE_SUB_POS_X;
          GFL_CLACT_WK_SetPos(act_hate, &pos_hate, CLSYS_DEFREND_SUB);
          GFL_CLACT_WK_SetDrawEnable(act_hate, TRUE);
        }
      }
      GFL_CLACT_WK_SetPos(act, &pos, CLSYS_DEFREND_SUB);
    }
    else{ //このプレイヤーがいるパレスエリアを指す
      int s;
      GFL_CLACT_WK_SetDrawEnable(act_hate, FALSE);
      for(s = 0; s < FIELD_COMM_MEMBER_MAX; s++){
        if(GFL_CLACT_WK_GetPlttOffs(intsub->act[INTSUB_ACTOR_AREA_0 + s]) == INTSUB_ACTOR_PAL_BASE_START + net_id){
          GFL_CLACT_WK_GetPos( intsub->act[INTSUB_ACTOR_AREA_0 + s], &pos, CLSYS_DRAW_SUB );
          break;
        }
      }
      if(s == FIELD_COMM_MEMBER_MAX){
        GFL_CLACT_WK_SetDrawEnable(act, FALSE);
        continue;
      }
      pos.x += WearOffset[net_id][0];
      pos.y += WearOffset[net_id][1];
      GFL_CLACT_WK_SetPos(act, &pos, CLSYS_DEFREND_SUB);
    }
    GFL_CLACT_WK_SetDrawEnable(act, TRUE);
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：自分カーソル
 *
 * @param   intsub		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_CursorL(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy, ZONEID my_zone_id)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  GFL_CLWK *act;
  GFL_CLACTPOS pos;
  int my_net_id;
  
  my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);
  if(ZONEDATA_IsPalace(my_zone_id) == TRUE){
    pos.x = PALACE_CURSOR_POS_X + WearOffset[intsub->my_net_id][0];
    pos.y = PALACE_CURSOR_POS_Y + WearOffset[intsub->my_net_id][1];
  }
  else{
    const PALACE_ZONE_SETTING *zonesetting = IntrudeField_GetZoneSettingData(my_zone_id);
    if(zonesetting != NULL){
      pos.x = zonesetting->sub_x + WearOffset[intsub->my_net_id][0];
      pos.y = zonesetting->sub_y + WearOffset[intsub->my_net_id][1];
    }
    else{
      pos.x = NOTHING_ZONE_SUB_POS_X - 8;
      pos.y = NOTHING_ZONE_SUB_POS_Y + NOTHING_ZONE_SUB_POS_Y_SPACE * intsub->my_net_id;
    }
  }
  act = intsub->act[INTSUB_ACTOR_CUR_L];
  GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_CUR_L], 
    INTSUB_ACTOR_PAL_BASE_START + intsub->my_net_id, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  GFL_CLACT_WK_SetPos(act, &pos, CLSYS_DEFREND_SUB);
  GFL_CLACT_WK_SetDrawEnable(act, TRUE);
}

//--------------------------------------------------------------
/**
 * 更新処理：参加しますボタン
 *
 * @param   intsub		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_EntryButton(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy)
{
  if(intsub->back_exit == FALSE && intsub->comm.recv_num <= 1){
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_ENTRY], TRUE);
    BmpOam_ActorSetDrawEnable(intsub->entrymsg_bmpoam[ENTRY_BUTTON_MSG_PATERN_ENTRY], FALSE);
    BmpOam_ActorSetDrawEnable(intsub->entrymsg_bmpoam[ENTRY_BUTTON_MSG_PATERN_BACK], TRUE);
    return;
  }
  else{
    BmpOam_ActorSetDrawEnable(intsub->entrymsg_bmpoam[ENTRY_BUTTON_MSG_PATERN_BACK], FALSE);
  }
  
  switch(intsub->comm.m_status){
  case MISSION_STATUS_NULL:
  case MISSION_STATUS_READY:
  case MISSION_STATUS_EXE:
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_ENTRY], FALSE);
    BmpOam_ActorSetDrawEnable(intsub->entrymsg_bmpoam[ENTRY_BUTTON_MSG_PATERN_ENTRY], FALSE);
    break;
  case MISSION_STATUS_NOT_ENTRY:
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_ENTRY], TRUE);
    BmpOam_ActorSetDrawEnable(intsub->entrymsg_bmpoam[ENTRY_BUTTON_MSG_PATERN_ENTRY], TRUE);
    break;
  default:
    GF_ASSERT(0);
    return;
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：レベル数値
 *
 * @param   intsub		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_LvNum(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int i, level;
  OCCUPY_INFO *my_occupy;
  GFL_CLWK *act;
  
  my_occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
//  level = my_occupy->intrude_level;
  level = intsub->comm.m_timer;
  
  for(i = 0; i <= INTSUB_ACTOR_LV_NUM_KETA_MAX - INTSUB_ACTOR_LV_NUM_KETA_0; i++){
    act = intsub->act[INTSUB_ACTOR_LV_NUM_KETA_0 + i];
    GFL_CLACT_WK_SetAnmIndex(act, level % 10);
    level /= 10;
  }
}

//--------------------------------------------------------------
/**
 * タイトルメッセージ更新処理
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_TitleMsgUpdate(INTRUDE_SUBDISP_PTR intsub, ZONEID my_zone_id)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int print_type = _TITLE_PRINT_NULL;
  int msg_id;
  
  if(ZONEDATA_IsPalace(my_zone_id) == TRUE){
    if(intsub->comm.now_palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
      print_type = _TITLE_PRINT_MY_PALACE;
      msg_id = msg_invasion_title_001;
    }
    else{
      print_type = _TITLE_PRINT_OTHER_PALACE0 + intsub->comm.now_palace_area;
      msg_id = msg_invasion_title_002;
    }
  }
  else{
    print_type = _TITLE_PRINT_PALACE_GO;
    msg_id = msg_invasion_title_000;
  }
  
  if(intsub->title_print_type == print_type || print_type == _TITLE_PRINT_NULL){
    return; //既に表示済み
  }
  intsub->title_print_type = print_type;
  
  if(print_type >= _TITLE_PRINT_OTHER_PALACE0 && print_type <= _TITLE_PRINT_OTHER_PALACE3){
    const MYSTATUS *myst;
    myst = GAMEDATA_GetMyStatusPlayer(gamedata, intsub->comm.now_palace_area);
    WORDSET_RegisterPlayerName( intsub->wordset, 0, myst );
  }
  
  GFL_MSG_GetString( intsub->msgdata, msg_id, intsub->strbuf_temp );
  WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_title, intsub->strbuf_temp);
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(intsub->printutil_title.win), 0 );
  PRINT_UTIL_PrintColor( &intsub->printutil_title, intsub->print_que, 
    0, 0, intsub->strbuf_title, intsub->font_handle, PRINTSYS_MACRO_LSB(15,2,0) );
}

//--------------------------------------------------------------
/**
 * インフォメーションメッセージ更新処理
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_InfoMsgUpdate(INTRUDE_SUBDISP_PTR intsub)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  GAME_COMM_INFO_MESSAGE infomsg;
  int k;
  BOOL msg_on = FALSE;
#if 0
  MISSION_STATUS mission_status;
#endif

  if(intsub->infomsg_trans_req == TRUE){
    return;
  }
  if(intsub->infomsg_wait > 0){
    intsub->infomsg_wait--;
    return;
  }
  
  if(intsub->comm.m_status != MISSION_STATUS_NULL && intsub->comm.p_md != NULL
      && intsub->print_mission_status != intsub->comm.m_status){
    //ミッション発動中の場合はミッション関連のメッセージを優先して表示
    //ミッションは状況を示すメッセージの為、キューに貯めずに現在の状態をそのまま表示
    switch(intsub->comm.m_status){
    case MISSION_STATUS_NOT_ENTRY: //ミッションは実施されているが参加していない
      GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_004, intsub->strbuf_info );
      break;
    case MISSION_STATUS_READY:     //ミッション開始待ち
      GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_005, intsub->strbuf_info );
      break;
    case MISSION_STATUS_EXE:       //ミッション中
      GFL_MSG_GetString(intsub->msgdata, 
        msg_invasion_info_m01 + intsub->comm.p_md->cdata.type, intsub->strbuf_info );
      break;
    default:
      GF_ASSERT_MSG(0, "m_status=%d\n", intsub->comm.m_status);
      return;
    }
    intsub->print_mission_status = intsub->comm.m_status;
    msg_on = TRUE;
  }
  else if(GameCommInfo_GetMessage(game_comm, &infomsg) == TRUE){
    intsub->print_mission_status = MISSION_STATUS_NULL;
    GFL_MSG_GetString(intsub->msgdata, infomsg.message_id, intsub->strbuf_temp );
    for(k = 0 ; k < INFO_WORDSET_MAX; k++){
      if(infomsg.name[k]!=NULL){
        WORDSET_RegisterWord( intsub->wordset, infomsg.wordset_no[k], infomsg.name[k], 
          infomsg.wordset_sex[k], TRUE, PM_LANG);
      }
    }
    WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_info, intsub->strbuf_temp);
    msg_on = TRUE;
  }
  
  if(msg_on == TRUE){
    GFL_BMP_Clear( intsub->infomsg_bmp, 0 );
    PRINTSYS_PrintQueColor( intsub->print_que, intsub->infomsg_bmp, 0, 0, intsub->strbuf_info, 
      intsub->font_handle, PRINTSYS_MACRO_LSB(15,2,0) );
    intsub->infomsg_trans_req = TRUE;
    intsub->infomsg_wait = INFOMSG_UPDATE_WAIT;
    OS_TPrintf("メッセージ描画：id=%d\n", infomsg.message_id);
  }
}


//==============================================================================
//  ツール
//==============================================================================
//--------------------------------------------------------------
/**
 * 侵入しているエリアの占拠情報ポインタを取得する
 *
 * @param   intsub		
 *
 * @retval  OCCUPY_INFO *		侵入先の占拠情報
 */
//--------------------------------------------------------------
static OCCUPY_INFO * _IntSub_GetArreaOccupy(INTRUDE_SUBDISP_PTR intsub)
{
  OCCUPY_INFO *area_occupy;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);

  if(intsub->comm.now_palace_area == GAMEDATA_GetIntrudeMyID(gamedata)
      || intsub->comm.now_palace_area == PALACE_AREA_NO_NULL){
    area_occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
  }
  else{
    area_occupy = GAMEDATA_GetOccupyInfo(gamedata, intsub->comm.now_palace_area);
  }
  
  return area_occupy;
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * 中心値と半径を指定してRECTを作成する
 *
 * @param   x		        中心値X
 * @param   y		        中心値Y
 * @param   half_size		半径X
 * @param   half_size		半径Y
 * @param   rect		    代入先
 */
//--------------------------------------------------------------
static void _SetRect(int x, int y, int half_size_x, int half_size_y, GFL_RECT *rect)
{
  rect->top = y - half_size_y;
  rect->bottom = y + half_size_y;
  rect->left = x - half_size_x;
  rect->right = x + half_size_x;
}

//--------------------------------------------------------------
/**
 * 指定座標が矩形内に収まっているかヒットチェック
 *
 * @param   x		    座標X
 * @param   y		    座標Y
 * @param   rect		矩形
 *
 * @retval  BOOL		TRUE:ヒット。　FALSE:未ヒット
 */
//--------------------------------------------------------------
static BOOL _CheckRectHit(int x, int y, const GFL_RECT *rect)
{
  if(rect->left < x && rect->right > x && rect->top < y && rect->bottom > y){
    return TRUE;
  }
  return FALSE;
}


//--------------------------------------------------------------
/**
 * タッチ判定
 *
 * @param   intcomm		
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_TouchUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub)
{
  u32 x, y;
  int i, my_net_id, net_id;
  GFL_RECT rect;
  GFL_CLACTPOS pos;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(intsub->gsys);
  FIELD_SUBSCREEN_WORK *subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  GFL_CLWK *act;

  if(intsub->event_req != _EVENT_REQ_NO_NULL || intsub->wfbc_go == TRUE 
      || GFL_UI_TP_GetPointTrg(&x, &y) == FALSE 
      || FIELD_SUBSCREEN_GetAction( subscreen ) != FIELD_SUBSCREEN_ACTION_NONE){
    return;
  }
  
  //プレイヤーアイコンタッチ判定
  my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(net_id != my_net_id && (intsub->comm.recv_profile & (1 << net_id))){
      act = intsub->act[INTSUB_ACTOR_CUR_S_0 + net_id];
      if(GFL_CLACT_WK_GetDrawEnable(act) == TRUE){
        GFL_CLACT_WK_GetPos( act, &pos, CLSYS_DRAW_SUB );
        if(x - TOUCH_RANGE_PLAYER_ICON_X <= pos.x && x + TOUCH_RANGE_PLAYER_ICON_X >= pos.x
            && y - TOUCH_RANGE_PLAYER_ICON_Y <= pos.y && y + TOUCH_RANGE_PLAYER_ICON_Y >= pos.y){
          if(intcomm != NULL && ZONEDATA_IsWfbc(intcomm->intrude_status[net_id].zone_id) == TRUE){
            intsub->wfbc_go = TRUE;
            intsub->wfbc_seq = 0;
          }
          else{
            Intrude_SetWarpPlayerNetID(game_comm, net_id);
            intsub->event_req = _EVENT_REQ_NO_PLAYER_WARP;
          }
          return;
        }
      }
    }
  }
  
  //街タッチ判定
  if(intsub->comm.now_palace_area != GAMEDATA_GetIntrudeMyID(gamedata)){
    for(i = 0; i < PALACE_TOWN_DATA_MAX; i++){
      _SetRect(PalaceTownData[i].subscreen_x, PalaceTownData[i].subscreen_y, 
        TOWN_ICON_HITRANGE_HALF, TOWN_ICON_HITRANGE_HALF, &rect);
      if(_CheckRectHit(x, y, &rect) == TRUE){
        if(i == PALACE_TOWN_WFBC){
          intsub->wfbc_go = TRUE;
          intsub->wfbc_seq = 0;
        }
        else{
          const PALACE_ZONE_SETTING *zonesetting = IntrudeField_GetZoneSettingData(PalaceTownData[i].front_zone_id);
          intsub->warp_zone_id = zonesetting->warp_zone_id;
          intsub->event_req = _EVENT_REQ_NO_TOWN_WARP;
        }
        return;
      }
    }
  }

  //パレス島タッチ判定
  _SetRect(PALACE_ICON_POS_X, PALACE_ICON_POS_Y, 
    PALACE_ICON_HITRANGE_HALF, PALACE_ICON_HITRANGE_HALF, &rect);
  if(_CheckRectHit(x, y, &rect) == TRUE){
    intsub->warp_zone_id = ZONE_ID_PALACE01;
    intsub->event_req = _EVENT_REQ_NO_TOWN_WARP;
    return;
  }
  
  ///参加ボタンタッチ判定
  if(intcomm != NULL && intsub->comm.m_status == MISSION_STATUS_NOT_ENTRY){
    _SetRect(ENTRY_BUTTON_POS_X, ENTRY_BUTTON_POS_Y, 
      ENTRY_BUTTON_HITRANGE_HALF_X, ENTRY_BUTTON_HITRANGE_HALF_Y, &rect);
    if(_CheckRectHit(x, y, &rect) == TRUE){
      MISSION_SetMissionEntry(intcomm, &intcomm->mission);
      intsub->event_req = _EVENT_REQ_NO_MISSION_ENTRY;
    }
  }
  ///もどるボタンタッチ判定
  if(intsub->back_exit == FALSE 
      && BmpOam_ActorGetDrawEnable(intsub->entrymsg_bmpoam[ENTRY_BUTTON_MSG_PATERN_BACK]) == TRUE){
    _SetRect(ENTRY_BUTTON_POS_X, ENTRY_BUTTON_POS_Y, 
      ENTRY_BUTTON_HITRANGE_HALF_X, ENTRY_BUTTON_HITRANGE_HALF_Y, &rect);
    if(_CheckRectHit(x, y, &rect) == TRUE){
      intsub->back_exit = TRUE;
    }
  }
}

//--------------------------------------------------------------
/**
 * 接続人数を監視し、Barスクリーンの一人用と通信用を変更する
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BGBarUpdate(INTRUDE_SUBDISP_PTR intsub)
{
  if(intsub->bar_type == BG_BAR_TYPE_SINGLE){
    if(intsub->comm.recv_num > 1){
      GFL_ARC_UTIL_TransVramScreen(
        ARCID_PALACE, NARC_palace_palace_bar2_lz_NSCR, INTRUDE_FRAME_S_BAR, 0, 
        0x800, TRUE, HEAPID_FIELDMAP);
      intsub->bar_type = BG_BAR_TYPE_COMM;
    }
  }
  else{ //BG_BAR_TYPE_COMM
    if(intsub->comm.recv_num <= 1){
      GFL_ARC_UTIL_TransVramScreen(
        ARCID_PALACE, NARC_palace_palace_bar_lz_NSCR, INTRUDE_FRAME_S_BAR, 0, 
        0x800, TRUE, HEAPID_FIELDMAP);
      intsub->bar_type = BG_BAR_TYPE_SINGLE;
    }
  }
}

//--------------------------------------------------------------
/**
 * 侵入エリアを監視し、BGのカラーを変更する
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BGColorUpdate(INTRUDE_SUBDISP_PTR intsub)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  int bg_pal;
  
  bg_pal = intsub->comm.now_palace_area;
  if(bg_pal == intsub->now_bg_pal){
    return;
  }
  
  GFL_BG_ChangeScreenPalette(
    INTRUDE_FRAME_S_BACKGROUND, _AREA_SCREEN_COLOR_START_X, _AREA_SCREEN_COLOR_START_Y, 
    _AREA_SCREEN_COLOR_SIZE_X, _AREA_SCREEN_COLOR_SIZE_Y, 
    INTSUB_BG_PAL_BASE_START_BACK + bg_pal);
  GFL_BG_ChangeScreenPalette(
    INTRUDE_FRAME_S_BAR, _AREA_SCREEN_COLOR_START_X, _AREA_SCREEN_COLOR_START_Y, 
    _AREA_SCREEN_COLOR_SIZE_X, _AREA_SCREEN_COLOR_SIZE_Y, 
    INTSUB_BG_PAL_BASE_START_TOWN + bg_pal);

  GFL_BMPWIN_MakeScreen(intsub->printutil_title.win);

  GFL_BG_LoadScreenV_Req(INTRUDE_FRAME_S_BACKGROUND);
  GFL_BG_LoadScreenV_Req(INTRUDE_FRAME_S_BAR);
  intsub->now_bg_pal = bg_pal;
}

//--------------------------------------------------------------
/**
 * intcommから引き出すパラメータバッファの初期化
 *
 * @param   intsub		
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void _IntSub_CommParamInit(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm)
{
  INTRUDE_COMM_PARAM *comm = &intsub->comm;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  
  if(intcomm == NULL){
    comm->now_palace_area = GAMEDATA_GetIntrudeMyID(gamedata);
    comm->recv_profile = 1 << comm->now_palace_area;
    comm->recv_num = 1;
    comm->palace_in = FALSE;
    comm->m_status = MISSION_STATUS_NULL;
    comm->m_timer = 0;
  }
  else{
    _IntSub_CommParamUpdate(intsub, intcomm);
  }
}

//--------------------------------------------------------------
/**
 * intcommから引き出すパラメータの更新処理
 *
 * @param   intsub		
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void _IntSub_CommParamUpdate(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm)
{
  INTRUDE_COMM_PARAM *comm = &intsub->comm;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  
  if(intcomm == NULL){
    //一部を除いて最後の状態から何も更新しない
    intsub->comm.m_status = MISSION_STATUS_NULL;
    intsub->comm.p_md = NULL;
  }
  else{
    const MISSION_DATA *md;

    comm->now_palace_area = intcomm->intrude_status_mine.palace_area;
    comm->recv_profile = intcomm->recv_profile;
    comm->recv_num = MATH_CountPopulation((u32)(comm->recv_profile));
    comm->palace_in = intcomm->palace_in;
    comm->m_status = MISSION_FIELD_CheckStatus(&intcomm->mission);
    comm->m_timer = MISSION_GetMissionTimer(&intcomm->mission);

    comm->p_md = MISSION_GetRecvData(&intcomm->mission);
  }
}
