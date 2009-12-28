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
#define INFOMSG_UPDATE_WAIT   (150)

///フォントBGパレット展開位置
#define _FONT_BG_PALNO      (14)

///アクターIndex
enum{
  INTSUB_ACTOR_TOWN_0,
  INTSUB_ACTOR_TOWN_1,
  INTSUB_ACTOR_TOWN_2,
  INTSUB_ACTOR_TOWN_3,
  INTSUB_ACTOR_TOWN_4,
  INTSUB_ACTOR_TOWN_5,
  INTSUB_ACTOR_TOWN_6,
  INTSUB_ACTOR_TOWN_7,
  INTSUB_ACTOR_TOWN_MAX = INTSUB_ACTOR_TOWN_7,

  INTSUB_ACTOR_TOUCH_TOWN_0,
  INTSUB_ACTOR_TOUCH_TOWN_1,
  INTSUB_ACTOR_TOUCH_TOWN_2,
  INTSUB_ACTOR_TOUCH_TOWN_3,
  INTSUB_ACTOR_TOUCH_TOWN_4,
  INTSUB_ACTOR_TOUCH_TOWN_5,
  INTSUB_ACTOR_TOUCH_TOWN_6,
  INTSUB_ACTOR_TOUCH_TOWN_7,
  INTSUB_ACTOR_TOUCH_TOWN_MAX = INTSUB_ACTOR_TOUCH_TOWN_7,

  INTSUB_ACTOR_TOUCH_PALACE,
  
  INTSUB_ACTOR_AREA_0,
  INTSUB_ACTOR_AREA_1,
  INTSUB_ACTOR_AREA_2,
  INTSUB_ACTOR_AREA_3,
  INTSUB_ACTOR_AREA_MAX = INTSUB_ACTOR_AREA_3,

  INTSUB_ACTOR_CUR_S_0,
  INTSUB_ACTOR_CUR_S_1,
  INTSUB_ACTOR_CUR_S_2,
  INTSUB_ACTOR_CUR_S_3,
  INTSUB_ACTOR_CUR_S_MAX = INTSUB_ACTOR_CUR_S_3,
  
  INTSUB_ACTOR_CUR_L,       ///<自分の居場所を指す
  
  INTSUB_ACTOR_MARK,        ///<自分のいるエリアを指す

  INTSUB_ACTOR_POWER,
  INTSUB_ACTOR_MISSION,
  INTSUB_ACTOR_INCLUSION,
  
  INTSUB_ACTOR_SENKYO_EFF_0,
  INTSUB_ACTOR_SENKYO_EFF_1,
  INTSUB_ACTOR_SENKYO_EFF_2,
  INTSUB_ACTOR_SENKYO_EFF_3,
  INTSUB_ACTOR_SENKYO_EFF_4,
  INTSUB_ACTOR_SENKYO_EFF_5,
  INTSUB_ACTOR_SENKYO_EFF_6,
  INTSUB_ACTOR_SENKYO_EFF_7,
  INTSUB_ACTOR_SENKYO_EFF_MAX = INTSUB_ACTOR_SENKYO_EFF_7,

  INTSUB_ACTOR_LV_NUM_KETA_0,
  INTSUB_ACTOR_LV_NUM_KETA_1,
  INTSUB_ACTOR_LV_NUM_KETA_2,
  INTSUB_ACTOR_LV_NUM_KETA_MAX = INTSUB_ACTOR_LV_NUM_KETA_2,

  INTSUB_ACTOR_POINT_NUM_KETA_0,
  INTSUB_ACTOR_POINT_NUM_KETA_1,
  INTSUB_ACTOR_POINT_NUM_KETA_2,
  INTSUB_ACTOR_POINT_NUM_KETA_3,
  INTSUB_ACTOR_POINT_NUM_KETA_4,
  INTSUB_ACTOR_POINT_NUM_KETA_5,
  INTSUB_ACTOR_POINT_NUM_KETA_6,
  INTSUB_ACTOR_POINT_NUM_KETA_7,
  INTSUB_ACTOR_POINT_NUM_KETA_8,
  INTSUB_ACTOR_POINT_NUM_KETA_MAX = INTSUB_ACTOR_POINT_NUM_KETA_8,

  INTSUB_ACTOR_MAX,
};

///OBJパレットINDEX
enum{
  INTSUB_ACTOR_PAL_BASE_START = 1,    ///<プレイヤー毎に変わるパレット開始位置
  
  INTSUB_ACTOR_PAL_MAX = 7,
};

///palace_obj.nceのアニメーションシーケンス
enum{
  PALACE_ACT_ANMSEQ_TOWN_G,
  PALACE_ACT_ANMSEQ_TOWN_W,
  PALACE_ACT_ANMSEQ_TOWN_B,
  PALACE_ACT_ANMSEQ_AREA,
  PALACE_ACT_ANMSEQ_CUR_S,
  PALACE_ACT_ANMSEQ_CUR_L,
  PALACE_ACT_ANMSEQ_MARK,
  PALACE_ACT_ANMSEQ_POWER,
  PALACE_ACT_ANMSEQ_MISSION,
  PALACE_ACT_ANMSEQ_INCLUSION,
  PALACE_ACT_ANMSEQ_SENKYO_EFF,
  PALACE_ACT_ANMSEQ_LV_NUM,
  PALACE_ACT_ANMSEQ_POINT_NUM,
  PALACE_ACT_ANMSEQ_TOUCH_TOWN,
  PALACE_ACT_ANMSEQ_TOUCH_PALACE,
};

///アクターソフトプライオリティ
enum{
  SOFTPRI_TOWN = 100,
  SOFTPRI_TOUCH_TOWN = 90,
  SOFTPRI_AREA = 50,
  SOFTPRI_CUR_S = 20,
  SOFTPRI_CUR_L = 19,
  SOFTPRI_MARK = SOFTPRI_AREA - 1,
  SOFTPRI_POWER = 10,
  SOFTPRI_MISSION = 10,
  SOFTPRI_INCLUSION = 10,
  SOFTPRI_SENKYO_EFF = SOFTPRI_MARK - 1,
  SOFTPRI_LV_NUM = 5,
  SOFTPRI_POINT_NUM = 5,
};
///アクター共通BGプライオリティ
#define BGPRI_ACTOR_COMMON      (2)

///BGパレットINDEX
enum{
  INTSUB_BG_PAL_SYSTEM,       ///<全プレイヤー共通のシステム色
  INTSUB_BG_PAL_BASE_START,   ///<プレイヤー毎に変わるパレット開始位置
  
  INTSUB_BG_PAL_MAX = 5,
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
///街アイコンのタッチ判定の矩形ハーフサイズ
#define TOWN_ICON_HITRANGE_HALF   (8)


//==============================================================================
//  構造体定義
//==============================================================================
///侵入下画面制御ワーク
typedef struct _INTRUDE_SUBDISP{
  GAMESYS_WORK *gsys;
  
  u32 index_cgr;
  u32 index_pltt;
  u32 index_cell;

  GFL_FONT *font_handle;
  GFL_TCBLSYS *tcbl_sys;
  WORDSET *wordset;
  PRINT_QUE *print_que;
	GFL_MSGDATA *msgdata;
	
	STRBUF *strbuf_temp;
	STRBUF *strbuf_info;
	
	PRINT_UTIL printutil_info;  ///<インフォメーションメッセージ
	
	GFL_CLUNIT *clunit;
  GFL_CLWK *act[INTSUB_ACTOR_MAX];
  
  u8 my_net_id;
  u8 town_update_req;     ///<街アイコン更新リクエスト(TOWN_UPDATE_REQ_???)
  u8 now_bg_pal;          ///<現在のBGのパレット番号
  u8 wfbc_go;             ///<TRUE:WFBCへのワープを押した
  
  s16 infomsg_wait;       ///<インフォメーションメッセージ更新ウェイト
  u8 wfbc_seq;
  u8 padding;
  
  u32 senkyo_eff_bit;     ///<占拠エフェクト起動中の街(bit管理)
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
static void _IntSub_ActorResouceLoad(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorResourceUnload(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorCreate(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorDelete(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorCreate_Town(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_TouchTown(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_SenkyoEff(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_Area(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_CursorS(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_CursorL(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_Mark(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_Power(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_LvNum(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_PointNum(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorUpdate_Town(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_TouchTown(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_SenkyoEff(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_Area(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_CursorS(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_CursorL(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_Mark(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_Power(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_LvNum(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_PointNum(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static OCCUPY_INFO * _IntSub_GetArreaOccupy(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_InfoMsgUpdate(INTRUDE_SUBDISP_PTR intsub);
static u8 _IntSub_GetProfileRecvNum(INTRUDE_COMM_SYS_PTR intcomm);
static u8 _IntSub_TownPosGet(ZONEID zone_id, GFL_CLACTPOS *dest_pos, int net_id);
static void _SetRect(int x, int y, int half_size_x, int half_size_y, GFL_RECT *rect);
static BOOL _CheckRectHit(int x, int y, const GFL_RECT *rect);
static void _IntSub_TouchUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_TownChangeCheck(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy, u8 update_req);
static void _IntSub_SenkyoEff_Start(INTRUDE_SUBDISP_PTR intsub, u32 eff_bit);
static void _IntSub_SenkyoEff_EndWatch(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_TownActChange(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy, int town_tblno);
static void _IntSub_TownActBitChange(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy, u32 change_bit);
static void _IntSub_BGColorUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub);


//==============================================================================
//  データ
//==============================================================================
///パレスアイコン座標
enum{
  PALACE_ICON_POS_X = 128,      ///<パレスアイコン座標X
  PALACE_ICON_POS_Y = 8*0xa,    ///<パレスアイコン座標Y
  PALACE_ICON_HITRANGE_HALF = 16, ///<パレスアイコンのタッチ判定半径
  
  PALACE_CURSOR_POS_X = PALACE_ICON_POS_X,      ///<パレスアイコンをカーソルが指す場合の座標X
  PALACE_CURSOR_POS_Y = PALACE_ICON_POS_Y - 20, ///<パレスアイコンをカーソルが指す場合の座標Y
};

///エリアアイコン配置座標
enum{
  AREA_POST_LEFT = 0xd * 8,                           ///<配置座標範囲の左端X
  AREA_POST_RIGHT = 0x13 * 8,                         ///<配置座標範囲の右端X
  AREA_POST_WIDTH = AREA_POST_RIGHT - AREA_POST_LEFT, ///<配置座標範囲の幅
  
  AREA_POST_Y = 8,                                    ///<配置座標Y
};

///POWER系アイコン配置座標
enum{
  POWER_ICON_POS_X = 0x15*8,
  POWER_ICON_POS_Y = 8*8,

  MISSION_ICON_POS_X = 0x1d*8,
  MISSION_ICON_POS_Y = 3*8,

  INCLUSION_ICON_POS_X = 24,
  INCLUSION_ICON_POS_Y = MISSION_ICON_POS_Y,

  POWER_ICON_HITRANGE_HALF_X = 24, ///<パワー系アイコンのタッチ判定半径X
  POWER_ICON_HITRANGE_HALF_Y = 8,  ///<パワー系アイコンのタッチ判定半径Y
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
  
  intsub = GFL_HEAP_AllocClearMemory(HEAPID_FIELDMAP, sizeof(INTRUDE_SUBDISP));
  intsub->gsys = gsys;
  intsub->now_bg_pal = 0xff;  //初回に必ず更新がかかるように0xff
  
  intsub->my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  if(intsub->my_net_id > FIELD_COMM_MEMBER_MAX){
    GF_ASSERT_MSG(0, "my_net_id = %d\n", intsub->my_net_id);
    intsub->my_net_id = 0;
  }
  
  handle = GFL_ARC_OpenDataHandle(ARCID_PALACE, HEAPID_FIELDMAP);
  
  _IntSub_SystemSetup(intsub);
  _IntSub_BGLoad(intsub, handle);
  _IntSub_BmpWinAdd(intsub);
  _IntSub_ActorResouceLoad(intsub, handle);
  _IntSub_ActorCreate(intsub, handle);
  
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
void INTRUDE_SUBDISP_Update(INTRUDE_SUBDISP_PTR intsub)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  INTRUDE_COMM_SYS_PTR intcomm = GameCommSys_GetAppWork(game_comm);
  OCCUPY_INFO *area_occupy;
  
  if(intcomm == NULL){
    return;
  }
  
	GFL_TCBL_Main( intsub->tcbl_sys );
	PRINTSYS_QUE_Main( intsub->print_que );
  PRINT_UTIL_Trans( &intsub->printutil_info, intsub->print_que );

  area_occupy = _IntSub_GetArreaOccupy(intcomm, intsub);
  
  //タッチ判定チェック
  _IntSub_TouchUpdate(intcomm, intsub);
  
  //WFBCへのワープチェック
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

          Intrude_SetWarpTown(game_comm, PALACE_TOWN_WFBC);
          FIELD_SUBSCREEN_SetAction(subscreen, FIELD_SUBSCREEN_ACTION_INTRUDE_TOWN_WARP);
          intsub->wfbc_go = FALSE;
        }
        break;
      }
    }
  }
  
  //BGスクリーンカラー変更チェック
  _IntSub_BGColorUpdate(intcomm, intsub);
  
  //インフォメーションメッセージ
  _IntSub_InfoMsgUpdate(intsub);
  
  //アクター更新
  _IntSub_ActorUpdate_Town(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_TouchTown(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_SenkyoEff(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_Area(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_CursorS(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_CursorL(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_Mark(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_Power(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_LvNum(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_PointNum(intsub, intcomm, area_occupy);

	GFL_CLACT_SYS_Main();
}

//==================================================================
/**
 * 侵入下画面：描画
 *
 * @param   intsub		
 */
//==================================================================
void INTRUDE_SUBDISP_Draw(INTRUDE_SUBDISP_PTR intsub)
{
  ;
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
  intsub->clunit = GFL_CLACT_UNIT_Create(INTSUB_ACTOR_MAX, 5, HEAPID_FIELDMAP);

	intsub->tcbl_sys = GFL_TCBL_Init(HEAPID_FIELDMAP, HEAPID_FIELDMAP, 4, 32);
	intsub->font_handle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_FIELDMAP );
	intsub->wordset = WORDSET_Create(HEAPID_FIELDMAP);
  intsub->print_que = PRINTSYS_QUE_Create( HEAPID_FIELDMAP );
  intsub->msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
    NARC_message_invasion_dat, HEAPID_FIELDMAP );
  
  intsub->strbuf_temp = GFL_STR_CreateBuffer(64, HEAPID_FIELDMAP);
  intsub->strbuf_info = GFL_STR_CreateBuffer(64, HEAPID_FIELDMAP);
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
  
  GFL_MSG_Delete(intsub->msgdata);
  PRINTSYS_QUE_Delete(intsub->print_que);
  WORDSET_Delete(intsub->wordset);
	GFL_FONT_Delete(intsub->font_handle);
	GFL_TCBL_Exit(intsub->tcbl_sys);

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
  GFL_BG_ClearScreen(INTRUDE_FRAME_S_BAR);
  
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
    0, 0x14, 0x20, 2, _FONT_BG_PALNO, GFL_BMP_CHRAREA_GET_B );
  PRINT_UTIL_Setup( &intsub->printutil_info, bmpwin );
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
  GFL_BMPWIN_Delete(intsub->printutil_info.win);
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
    NARC_palace_palace_obj_NANR, HEAPID_FIELDMAP);
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
  _IntSub_ActorCreate_Town(intsub, handle);
  _IntSub_ActorCreate_TouchTown(intsub, handle);
  _IntSub_ActorCreate_SenkyoEff(intsub, handle);
  _IntSub_ActorCreate_Area(intsub, handle);
  _IntSub_ActorCreate_CursorS(intsub, handle);
  _IntSub_ActorCreate_CursorL(intsub, handle);
  _IntSub_ActorCreate_Mark(intsub, handle);
  _IntSub_ActorCreate_Power(intsub, handle);
  _IntSub_ActorCreate_LvNum(intsub, handle);
  _IntSub_ActorCreate_PointNum(intsub, handle);
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
 * 街アクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_Town(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_TOWN_G,   //アニメーションシーケンス
  	SOFTPRI_TOWN,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  GF_ASSERT(PALACE_TOWN_DATA_MAX == (INTSUB_ACTOR_TOWN_MAX - INTSUB_ACTOR_TOWN_0 + 1));
  GF_ASSERT(PALACE_TOWN_DATA_MAX == INTRUDE_TOWN_MAX);
  for(i = INTSUB_ACTOR_TOWN_0; i <= INTSUB_ACTOR_TOWN_MAX; i++){
    head.pos_x = PalaceTownData[i - INTSUB_ACTOR_TOWN_0].subscreen_x;
    head.pos_y = PalaceTownData[i - INTSUB_ACTOR_TOWN_0].subscreen_y;
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
  }
  
  intsub->town_update_req = TOWN_UPDATE_REQ_NOT_EFFECT;
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
  int i;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_TOUCH_TOWN,   //アニメーションシーケンス
  	SOFTPRI_TOUCH_TOWN,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  GF_ASSERT(PALACE_TOWN_DATA_MAX == (INTSUB_ACTOR_TOUCH_TOWN_MAX - INTSUB_ACTOR_TOUCH_TOWN_0 + 1));
  for(i = INTSUB_ACTOR_TOUCH_TOWN_0; i <= INTSUB_ACTOR_TOUCH_TOWN_MAX; i++){
    head.pos_x = PalaceTownData[i - INTSUB_ACTOR_TOUCH_TOWN_0].subscreen_x;
    head.pos_y = PalaceTownData[i - INTSUB_ACTOR_TOUCH_TOWN_0].subscreen_y;
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
  }
  
  //タッチパレス島
  head.anmseq = PALACE_ACT_ANMSEQ_TOUCH_PALACE;
  head.pos_x = 128;
  head.pos_y = 0xa*8;
  intsub->act[INTSUB_ACTOR_TOUCH_PALACE] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], FALSE);  //表示OFF
}

//--------------------------------------------------------------
/**
 * 占拠エフェクトアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_SenkyoEff(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_SENKYO_EFF,   //アニメーションシーケンス
  	SOFTPRI_SENKYO_EFF,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  GF_ASSERT(PALACE_TOWN_DATA_MAX == (INTSUB_ACTOR_SENKYO_EFF_MAX - INTSUB_ACTOR_SENKYO_EFF_0 + 1));
  for(i = INTSUB_ACTOR_SENKYO_EFF_0; i <= INTSUB_ACTOR_SENKYO_EFF_MAX; i++){
    head.pos_x = PalaceTownData[i].subscreen_x;
    head.pos_y = PalaceTownData[i].subscreen_y;
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
  }
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
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
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
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
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
  static const GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_CUR_L,     //アニメーションシーケンス
  	SOFTPRI_CUR_L,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  intsub->act[INTSUB_ACTOR_CUR_L] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
  GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_CUR_L], 
    INTSUB_ACTOR_PAL_BASE_START + intsub->my_net_id, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_CUR_L], FALSE);  //表示OFF
  GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[INTSUB_ACTOR_CUR_L], TRUE);  //オートアニメON
}

//--------------------------------------------------------------
/**
 * 自分の侵入エリアを示すマークアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_Mark(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  static const GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_MARK,     //アニメーションシーケンス
  	SOFTPRI_MARK,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  intsub->act[INTSUB_ACTOR_MARK] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
  GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_MARK], 
    INTSUB_ACTOR_PAL_BASE_START + intsub->my_net_id, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_MARK], FALSE);  //表示OFF
}

//--------------------------------------------------------------
/**
 * パワーマークアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_Power(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  GFL_CLWK_DATA head = {
  	POWER_ICON_POS_X, POWER_ICON_POS_Y,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_POWER,     //アニメーションシーケンス
  	SOFTPRI_POWER,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  //POWER
  intsub->act[INTSUB_ACTOR_POWER] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_POWER], FALSE);  //表示OFF
  GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[INTSUB_ACTOR_POWER], TRUE);  //オートアニメON

  //MISSION
  head.pos_x = MISSION_ICON_POS_X;
  head.pos_y = MISSION_ICON_POS_Y;
  head.anmseq = PALACE_ACT_ANMSEQ_MISSION;
  intsub->act[INTSUB_ACTOR_MISSION] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_MISSION], FALSE);  //表示OFF
  GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[INTSUB_ACTOR_MISSION], TRUE);  //オートアニメON

  //INCLUSION
  head.pos_x = INCLUSION_ICON_POS_X;
  head.pos_y = INCLUSION_ICON_POS_Y;
  head.anmseq = PALACE_ACT_ANMSEQ_INCLUSION;
  intsub->act[INTSUB_ACTOR_INCLUSION] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_INCLUSION], FALSE);  //表示OFF
  GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[INTSUB_ACTOR_INCLUSION], TRUE);  //オートアニメON
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
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], TRUE);
  }
}

//--------------------------------------------------------------
/**
 * ポイント数値アクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_PointNum(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	4 * 8, 0x17 * 8,               //X, Y座標
  	PALACE_ACT_ANMSEQ_POINT_NUM,   //アニメーションシーケンス
  	SOFTPRI_POINT_NUM,             //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,            //BGプライオリティ
  };
  u16 width = 8;
  
  head.pos_x += width * (INTSUB_ACTOR_POINT_NUM_KETA_MAX - INTSUB_ACTOR_POINT_NUM_KETA_0);
  for(i = INTSUB_ACTOR_POINT_NUM_KETA_0; i <= INTSUB_ACTOR_POINT_NUM_KETA_MAX; i++){
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], TRUE);
    head.pos_x -= width;
  }
}

//==============================================================================
//  更新
//==============================================================================
//--------------------------------------------------------------
/**
 * 更新処理：街アクター
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_Town(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  if(intsub->town_update_req == TOWN_UPDATE_REQ_NONE){
    if(intcomm->area_occupy_update == TRUE){
      intsub->town_update_req = TOWN_UPDATE_REQ_UPDATE;
      intcomm->area_occupy_update = FALSE;
    }
  }
  
  if(intsub->town_update_req != TOWN_UPDATE_REQ_NONE && intsub->senkyo_eff_bit == 0){
    _IntSub_TownChangeCheck(intsub, area_occupy, intsub->town_update_req);
    intsub->town_update_req = FALSE;
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：タッチ街アクター
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_TouchTown(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int i;
  
  if(intcomm->intrude_status_mine.palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
    //自分のエリアの為、パレス島しかタッチ出来ない
    for(i = INTSUB_ACTOR_TOUCH_TOWN_0; i <= INTSUB_ACTOR_TOUCH_TOWN_MAX; i++){
      GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);
    }
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], TRUE);
  }
  else{
    for(i = INTSUB_ACTOR_TOUCH_TOWN_0; i <= INTSUB_ACTOR_TOUCH_TOWN_MAX; i++){
      GFL_CLACT_WK_SetDrawEnable(intsub->act[i], TRUE);
    }
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], TRUE);
  }
}

//--------------------------------------------------------------
/**
 * 街アクターと占拠情報を比較し、変更があれば対応したアニメに変更、占拠エフェクト起動をする
 *
 * @param   intsub		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_TownChangeCheck(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy, u8 update_req)
{
  int i;
  GFL_CLWK *act;
  int white_town, black_town;
  u32 eff_bit;      //占拠エフェクト起動する必要有
  u32 change_bit;   //街アイコンと占拠情報に相違があった
  int anmseq;
  
  white_town = 0;
  black_town = 0;
  eff_bit = 0;
  change_bit = 0;
  
  for(i = 0; i < INTRUDE_TOWN_MAX; i++){
    act = intsub->act[INTSUB_ACTOR_TOWN_0 + i];
    anmseq = GFL_CLACT_WK_GetAnmSeq(act);
    switch(anmseq){
    case PALACE_ACT_ANMSEQ_TOWN_G:
      switch(area_occupy->town.town_occupy[i]){
      case OCCUPY_TOWN_WHITE:
        white_town++;
        change_bit |= 1 << i;
        eff_bit |= 1 << i;
        break;
      case OCCUPY_TOWN_BLACK:
        black_town++;
        change_bit |= 1 << i;
        eff_bit |= 1 << i;
        break;
      }
      break;
    case PALACE_ACT_ANMSEQ_TOWN_W:
      switch(area_occupy->town.town_occupy[i]){
      case OCCUPY_TOWN_WHITE:
        white_town++;
        break;
      case OCCUPY_TOWN_BLACK:
        black_town++;
        change_bit |= 1 << i;
        eff_bit |= 1 << i;
        break;
      default:
        change_bit |= 1 << i;
        break;
      }
      break;
    case PALACE_ACT_ANMSEQ_TOWN_B:
      switch(area_occupy->town.town_occupy[i]){
      case OCCUPY_TOWN_WHITE:
        white_town++;
        change_bit |= 1 << i;
        eff_bit |= 1 << i;
        break;
      case OCCUPY_TOWN_BLACK:
        black_town++;
        break;
      default:
        change_bit |= 1 << i;
        break;
      }
      break;
    }
  }
  
  if(update_req == TOWN_UPDATE_REQ_NOT_EFFECT){
    eff_bit = 0;
  }
  else{
    if(white_town == INTRUDE_TOWN_MAX || black_town == INTRUDE_TOWN_MAX){
      for(i = 0; i < INTRUDE_TOWN_MAX; i++){
        eff_bit |= 1 << i;  //全ての街で占拠エフェクトが出るように全bitをON
      }
    }
  }
  
  if(eff_bit > 0){
    _IntSub_SenkyoEff_Start(intsub, eff_bit);
    intsub->senkyo_eff_bit |= eff_bit;
  }
  if(change_bit > 0){
    _IntSub_TownActBitChange(intsub, area_occupy, change_bit);
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：占拠エフェクト
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_SenkyoEff(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  _IntSub_SenkyoEff_EndWatch(intsub);
}

//--------------------------------------------------------------
/**
 * 更新処理：エリアアイコン
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_Area(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int net_id, profile_num, area_width;
  GFL_CLWK *act;
  GFL_CLACTPOS pos;
  
  profile_num = _IntSub_GetProfileRecvNum(intcomm);
  area_width = AREA_POST_WIDTH / (profile_num + 1);
  pos.x = AREA_POST_LEFT + area_width;
  pos.y = AREA_POST_Y;
  
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    act = intsub->act[INTSUB_ACTOR_AREA_0 + net_id];
    if(net_id == GAMEDATA_GetIntrudeMyID(gamedata) 
        || (intcomm->recv_profile & (1 << net_id))){
      GFL_CLACT_WK_SetPos(act, &pos, CLSYS_DEFREND_SUB);
      GFL_CLACT_WK_SetDrawEnable(act, TRUE);
      pos.x += area_width;
    }
    else{ //自分ではないし、プロフィールも受信していない
      GFL_CLACT_WK_SetDrawEnable(act, FALSE);
    }
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
  int my_area, net_id, profile_num, pos_count, s, area_width;
  GFL_CLWK *act;
  GFL_CLACTPOS pos;
  INTRUDE_STATUS *ist;
  s16 base_x;
  u8 palace_num[FIELD_COMM_MEMBER_MAX];
  u8 town_num[PALACE_TOWN_DATA_MAX];
  static const s8 TownWearOffset[] = {3, -3, 6, 0};   //最後の0は4BYTEアライメント
  static const s8 PalaceWearOffset[] = {0, 3, -3, 0}; //最後の0は4BYTEアライメント
  int town_no;
  
  profile_num = _IntSub_GetProfileRecvNum(intcomm);
  area_width = AREA_POST_WIDTH / (profile_num + 1);
  base_x = AREA_POST_LEFT + area_width;

  my_area = intcomm->intrude_status_mine.palace_area;
  pos_count = -1;
  
  GFL_STD_MemClear(town_num, sizeof(town_num[0]) * PALACE_TOWN_DATA_MAX);
  GFL_STD_MemClear(palace_num, sizeof(palace_num[0]) * FIELD_COMM_MEMBER_MAX);
  GF_ASSERT(NELEMS(PalaceWearOffset) == FIELD_COMM_MEMBER_MAX);
  
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    act = intsub->act[INTSUB_ACTOR_CUR_S_0 + net_id];
    if(net_id == GAMEDATA_GetIntrudeMyID(gamedata)){
      GFL_CLACT_WK_SetDrawEnable(act, FALSE);
      continue;
    }
    else if(intcomm->recv_profile & (1 << net_id)){
      ist = &intcomm->intrude_status[net_id];
    }
    else{
      GFL_CLACT_WK_SetDrawEnable(act, FALSE);
      continue;
    }
    
    if(ist->palace_area == my_area){  //このプレイヤーがいる街を指す
      town_no = _IntSub_TownPosGet(ist->zone_id, &pos, net_id);
      if(town_no == _TOWN_NO_PALACE){
        town_no = 0;
      }
      if(town_no != _TOWN_NO_FREE){
        pos.x += TownWearOffset[town_num[town_no]];
        town_num[town_no]++;
      }
      GFL_CLACT_WK_SetPos(act, &pos, CLSYS_DEFREND_SUB);
    }
    else{ //このプレイヤーがいるパレスエリアを指す
      if(intcomm->recv_profile & (1 << ist->palace_area)){
        pos_count = 0;
        for(s = 0; s < ist->palace_area; s++){
          if(intcomm->recv_profile & (1 << s)){
            pos_count++;
          }
        }
        pos.x = base_x + area_width * pos_count + PalaceWearOffset[palace_num[pos_count]];
        pos.y = AREA_POST_Y;
        GFL_CLACT_WK_SetPos(act, &pos, CLSYS_DEFREND_SUB);
        palace_num[pos_count]++;
      }
      else{
        //このプレイヤーが居るエリアが、こちらはまだプロフィールを受信していないエリアの場合は無視
        GFL_CLACT_WK_SetDrawEnable(act, FALSE);
        continue;
      }
    }
    GFL_CLACT_WK_SetDrawEnable(act, TRUE);
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：自分カーソル
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_CursorL(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  GFL_CLWK *act;
  GFL_CLACTPOS pos;
  int my_net_id;
  
  my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);
  _IntSub_TownPosGet(intcomm->intrude_status_mine.zone_id, &pos, my_net_id);
  act = intsub->act[INTSUB_ACTOR_CUR_L];
  GFL_CLACT_WK_SetPos(act, &pos, CLSYS_DEFREND_SUB);
  GFL_CLACT_WK_SetDrawEnable(act, TRUE);
}

//--------------------------------------------------------------
/**
 * 更新処理：自分侵入エリアマーク
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_Mark(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int net_id, profile_num;
  GFL_CLWK *act;
  u32 recv_profile;
  int area_width, s, my_area, pos_count;
  GFL_CLACTPOS pos;
  
  profile_num = _IntSub_GetProfileRecvNum(intcomm);
  area_width = AREA_POST_WIDTH / (profile_num + 1);
  pos.x = AREA_POST_LEFT + area_width;
  pos.y = AREA_POST_Y;

  my_area = intcomm->intrude_status_mine.palace_area;
  if(my_area == PALACE_AREA_NO_NULL){
    my_area = 0;
  }
  pos_count = 0;
  for(s = 0; s < my_area; s++){
    if(intcomm->recv_profile & (1 << s)){
      pos_count++;
    }
  }
  pos.x += area_width * pos_count;
  act = intsub->act[INTSUB_ACTOR_MARK];
  GFL_CLACT_WK_SetPos(act, &pos, CLSYS_DEFREND_SUB);
  GFL_CLACT_WK_SetDrawEnable(act, TRUE);
}

//--------------------------------------------------------------
/**
 * 更新処理：POWER
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_Power(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  //MISSION
  if(MISSION_RecvCheck(&intcomm->mission) == TRUE){
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_MISSION], TRUE);
  }
  else{
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_MISSION], FALSE);
  }
  
  //INCLUSION
  if(ZONEDATA_IsPalaceField(intcomm->intrude_status_mine.zone_id) == TRUE
      || ZONEDATA_IsPalace(intcomm->intrude_status_mine.zone_id) == TRUE
      || ZONEDATA_IsWfbc(intcomm->intrude_status_mine.zone_id) == TRUE){
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_INCLUSION], FALSE);
  }
  else{
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_INCLUSION], TRUE);
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：レベル数値
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_LvNum(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int i, level;
  OCCUPY_INFO *my_occupy;
  GFL_CLWK *act;
  
  my_occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
//  level = my_occupy->intrude_level;
  level = MISSION_GetMissionTimer(&intcomm->mission);
  
  for(i = 0; i <= INTSUB_ACTOR_LV_NUM_KETA_MAX - INTSUB_ACTOR_LV_NUM_KETA_0; i++){
    act = intsub->act[INTSUB_ACTOR_LV_NUM_KETA_0 + i];
    GFL_CLACT_WK_SetAnmIndex(act, level % 10);
    level /= 10;
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：ポイント数値
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_PointNum(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int i, point;
  OCCUPY_INFO *my_occupy;
  GFL_CLWK *act;
  
  my_occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
  point = my_occupy->intrude_point;
  
  for(i = 0; i <= INTSUB_ACTOR_POINT_NUM_KETA_MAX - INTSUB_ACTOR_POINT_NUM_KETA_0; i++){
    act = intsub->act[INTSUB_ACTOR_POINT_NUM_KETA_0 + i];
    GFL_CLACT_WK_SetAnmIndex(act, point % 10);
    point /= 10;
  }
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
  
  if(intsub->infomsg_wait > 0){
    intsub->infomsg_wait--;
    return;
  }
  
  if(GameCommInfo_GetMessage(game_comm, &infomsg) == TRUE){
    GFL_MSG_GetString(intsub->msgdata, infomsg.message_id, intsub->strbuf_temp );
    for(k = 0 ; k < INFO_WORDSET_MAX; k++){
      if(infomsg.name[k]!=NULL){
        WORDSET_RegisterWord( intsub->wordset, infomsg.wordset_no[k], infomsg.name[k], 
          PM_MALE, TRUE, PM_LANG);
      }
    }
    WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_info, intsub->strbuf_temp);
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(intsub->printutil_info.win), 0 );
    PRINT_UTIL_PrintColor( &intsub->printutil_info, intsub->print_que, 
      0, 0, intsub->strbuf_info, intsub->font_handle, PRINTSYS_MACRO_LSB(15,2,0) );
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
 * @param   intcomm		
 * @param   intsub		
 *
 * @retval  OCCUPY_INFO *		侵入先の占拠情報
 */
//--------------------------------------------------------------
static OCCUPY_INFO * _IntSub_GetArreaOccupy(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub)
{
  OCCUPY_INFO *area_occupy;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);

  if(intcomm->intrude_status_mine.palace_area == GAMEDATA_GetIntrudeMyID(gamedata)
      || intcomm->intrude_status_mine.palace_area == PALACE_AREA_NO_NULL){
    area_occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
  }
  else{
    area_occupy = GAMEDATA_GetOccupyInfo(gamedata, intcomm->intrude_status_mine.palace_area);
  }
  
  return area_occupy;
}

//--------------------------------------------------------------
/**
 * プロフィール受信人数を取得
 * @param   intcomm		
 * @retval  u8		プロフィール受信人数
 */
//--------------------------------------------------------------
static u8 _IntSub_GetProfileRecvNum(INTRUDE_COMM_SYS_PTR intcomm)
{
  u32 recv_profile;
  u8 profile_num;
  
  recv_profile = intcomm->recv_profile; //MATH_CountPopulationを使用するためu32に代入
  profile_num = MATH_CountPopulation(recv_profile);
  if(profile_num > FIELD_COMM_MEMBER_MAX){
    GF_ASSERT_MSG(0, "profile_num = %d\n", profile_num);
    profile_num = 1;
  }
  return profile_num;
}

//--------------------------------------------------------------
/**
 * 街アイコン配置座標を取得する
 *
 * @param   zone_id		
 * @param   dest_pos		
 * @param   net_id
 * 
 * @retval  街No(パレスエリアの場合は_TOWN_NO_PALACE, どこにも属さない場合は_TOWN_NO_FREE)
 */
//--------------------------------------------------------------
static u8 _IntSub_TownPosGet(ZONEID zone_id, GFL_CLACTPOS *dest_pos, int net_id)
{
  int i;
  
  for(i = 0; i < PALACE_TOWN_DATA_MAX; i++){
    if(PalaceTownData[i].reverse_zone_id == zone_id
        || PalaceTownData[i].front_zone_id == zone_id){
      dest_pos->x = PalaceTownData[i].subscreen_x;
      dest_pos->y = PalaceTownData[i].subscreen_y;
      return i;
    }
  }
  
  if(ZONEDATA_IsPalace(zone_id) == TRUE){
    dest_pos->x = PALACE_CURSOR_POS_X;
    dest_pos->y = PALACE_CURSOR_POS_Y;
    return _TOWN_NO_PALACE;
  }
  else{
    dest_pos->x = NO_TOWN_CURSOR_POS_X + NO_TOWN_CURSOR_POS_SPACE_X * net_id;
    dest_pos->y = NO_TOWN_CURSOR_POS_Y;
    return _TOWN_NO_FREE;
  }
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

  if(intsub->wfbc_go == TRUE || GFL_UI_TP_GetPointTrg(&x, &y) == FALSE 
      || FIELD_SUBSCREEN_GetAction( subscreen ) != FIELD_SUBSCREEN_ACTION_NONE){
    return;
  }
  
  //プレイヤーアイコンタッチ判定
  my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(net_id != my_net_id && (intcomm->recv_profile & (1 << net_id))){
      act = intsub->act[INTSUB_ACTOR_CUR_S_0 + net_id];
      if(GFL_CLACT_WK_GetDrawEnable(act) == TRUE){
        GFL_CLACT_WK_GetPos( act, &pos, CLSYS_DRAW_SUB );
        if(x - TOUCH_RANGE_PLAYER_ICON_X <= pos.x && x + TOUCH_RANGE_PLAYER_ICON_X >= pos.x
            && y - TOUCH_RANGE_PLAYER_ICON_Y <= pos.y && y + TOUCH_RANGE_PLAYER_ICON_Y >= pos.y){
          Intrude_SetWarpPlayerNetID(game_comm, net_id);
          FIELD_SUBSCREEN_SetAction(subscreen, FIELD_SUBSCREEN_ACTION_INTRUDE_PLAYER_WARP);
          return;
        }
      }
    }
  }
  
  //街タッチ判定
  if(intcomm->intrude_status_mine.palace_area != GAMEDATA_GetIntrudeMyID(gamedata)){
    for(i = 0; i < PALACE_TOWN_DATA_MAX; i++){
      _SetRect(PalaceTownData[i].subscreen_x, PalaceTownData[i].subscreen_y, 
        TOWN_ICON_HITRANGE_HALF, TOWN_ICON_HITRANGE_HALF, &rect);
      if(_CheckRectHit(x, y, &rect) == TRUE){
        if(i == PALACE_TOWN_WFBC){
          intsub->wfbc_go = TRUE;
          intsub->wfbc_seq = 0;
          return;
        }
        else{
          Intrude_SetWarpTown(game_comm, i);
          FIELD_SUBSCREEN_SetAction(subscreen, FIELD_SUBSCREEN_ACTION_INTRUDE_TOWN_WARP);
          return;
        }
      }
    }
  }

  //パレス島タッチ判定
  _SetRect(PALACE_ICON_POS_X, PALACE_ICON_POS_Y, 
    PALACE_ICON_HITRANGE_HALF, PALACE_ICON_HITRANGE_HALF, &rect);
  if(_CheckRectHit(x, y, &rect) == TRUE){
    Intrude_SetWarpTown(game_comm, PALACE_TOWN_DATA_PALACE);
    FIELD_SUBSCREEN_SetAction(subscreen, FIELD_SUBSCREEN_ACTION_INTRUDE_TOWN_WARP);
    return;
  }
  
  //ミッションアイコンタッチ判定
  if(MISSION_RecvCheck(&intcomm->mission) == TRUE){
    _SetRect(MISSION_ICON_POS_X, MISSION_ICON_POS_Y, 
      POWER_ICON_HITRANGE_HALF_X, POWER_ICON_HITRANGE_HALF_Y, &rect);
    if(_CheckRectHit(x, y, &rect) == TRUE){
      FIELD_SUBSCREEN_SetAction(subscreen, FIELD_SUBSCREEN_ACTION_INTRUDE_MISSION_PUT);
      return;
    }
  }
}

//--------------------------------------------------------------
/**
 * 占拠エフェクト開始
 *
 * @param   intsub		
 * @param   eff_bit		エフェクト対象の街(bit管理)
 */
//--------------------------------------------------------------
static void _IntSub_SenkyoEff_Start(INTRUDE_SUBDISP_PTR intsub, u32 eff_bit)
{
  GFL_CLWK *act;
  int i;
  
  for(i = 0; i < INTRUDE_TOWN_MAX; i++){
    if(eff_bit & (1 << i)){
      act = intsub->act[INTSUB_ACTOR_SENKYO_EFF_0 + i];
      GFL_CLACT_WK_ResetAnm(act);
      GFL_CLACT_WK_SetDrawEnable(act, TRUE);
      GFL_CLACT_WK_SetAutoAnmFlag(act, TRUE);
    }
  }
}

//--------------------------------------------------------------
/**
 * 占拠エフェクト終了監視
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_SenkyoEff_EndWatch(INTRUDE_SUBDISP_PTR intsub)
{
  int i;
  GFL_CLWK *act;

  if(intsub->senkyo_eff_bit > 0){
    for(i = 0; i < INTRUDE_TOWN_MAX; i++){
      if(intsub->senkyo_eff_bit & (1 << i)){
        act = intsub->act[INTSUB_ACTOR_TOWN_0 + i];
        if(GFL_CLACT_WK_CheckAnmActive( act ) == FALSE){
          GFL_CLACT_WK_SetDrawEnable(act, FALSE);
          GFL_CLACT_WK_SetAutoAnmFlag(act, FALSE);
          intsub->senkyo_eff_bit ^= (1 << i);
        }
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * 街アイコンの表示を現在のパラメータで更新
 *
 * @param   intsub		
 * @param   town_tblno		
 */
//--------------------------------------------------------------
static void _IntSub_TownActChange(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy, int town_tblno)
{
  GFL_CLWK *act = intsub->act[INTSUB_ACTOR_TOWN_0 + town_tblno];
  
  if(area_occupy->town.town_occupy[town_tblno] == OCCUPY_TOWN_WHITE){
    GFL_CLACT_WK_SetAnmSeqDiff(act, PALACE_ACT_ANMSEQ_TOWN_W);
    GFL_CLACT_WK_SetDrawEnable(act, TRUE);
  }
  else if(area_occupy->town.town_occupy[town_tblno] == OCCUPY_TOWN_BLACK){
    GFL_CLACT_WK_SetAnmSeqDiff(act, PALACE_ACT_ANMSEQ_TOWN_B);
    GFL_CLACT_WK_SetDrawEnable(act, TRUE);
  }
  else{
    GFL_CLACT_WK_SetDrawEnable(act, FALSE);
  }
}

//--------------------------------------------------------------
/**
 * 街アイコンの表示を現在のパラメータで更新(bit指定)
 *
 * @param   intsub		
 * @param   change_bit		
 */
//--------------------------------------------------------------
static void _IntSub_TownActBitChange(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy, u32 change_bit)
{
  int i;
  
  for(i = 0; i < INTRUDE_TOWN_MAX; i++){
    if(change_bit & (1 << i)){
      _IntSub_TownActChange(intsub, area_occupy, i);
    }
  }
}

//--------------------------------------------------------------
/**
 * 侵入エリアを監視し、BGのカラーを変更する
 *
 * @param   intcomm		
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BGColorUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  int bg_pal;
  
  bg_pal = intcomm->intrude_status_mine.palace_area;
  if(bg_pal == intsub->now_bg_pal){
    return;
  }
  
  if(intcomm->intrude_status_mine.palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
    GameCommInfo_MessageEntry_MyPalace(game_comm, intcomm->intrude_status_mine.palace_area);
  }
  else{
    GameCommInfo_MessageEntry_IntrudePalace(game_comm, bg_pal);
  }
  
  GFL_BG_ChangeScreenPalette(
    INTRUDE_FRAME_S_BACKGROUND, 0, 0, 32, 0x13, INTSUB_BG_PAL_BASE_START + bg_pal);
  GFL_BG_LoadScreenV_Req(INTRUDE_FRAME_S_BACKGROUND);
  intsub->now_bg_pal = bg_pal;
}
