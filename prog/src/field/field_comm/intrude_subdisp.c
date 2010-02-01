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
#define INFOMSG_UPDATE_WAIT   (90)

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
  INTSUB_ACTOR_ENTRY,       ///<「参加」ボタン
  
//  INTSUB_ACTOR_MARK,        ///<自分のいるエリアを指す

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

///BMPOAMで使用するアクター数
#define INTSUB_ACTOR_BMPOAM_NUM   (16)

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
};

///アクターソフトプライオリティ
enum{
  SOFTPRI_TOUCH_TOWN = 90,
  SOFTPRI_AREA = 50,
  SOFTPRI_CUR_S = 20,
  SOFTPRI_CUR_L = 19,
  SOFTPRI_LV_NUM = 10,
  SOFTPRI_POINT_NUM = 10,
  SOFTPRI_INFOMSG = 8,
  SOFTPRI_ENTRY_BUTTON = 5,
};
///アクター共通BGプライオリティ
#define BGPRI_ACTOR_COMMON      (2)

///BGパレットINDEX
enum{
  INTSUB_BG_PAL_INFOMSG_BACK_NORMAL = 0, ///<infoメッセージの背景色(ノーマル)
  INTSUB_BG_PAL_INFOMSG_BACK_ACTIVE = 9, ///<infoメッセージの背景色(アクティブ時)
  
  INTSUB_BG_PAL_BASE_START = 5,   ///<プレイヤー毎に変わるパレット開始位置
  
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

//==============================================================================
//  構造体定義
//==============================================================================
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
  GFL_BMP_DATA *infomsg_bmp;
  BMPOAM_ACT_PTR infomsg_bmpoam;
  
  u8 my_net_id;
  u8 town_update_req;     ///<街アイコン更新リクエスト(TOWN_UPDATE_REQ_???)
  u8 now_bg_pal;          ///<現在のBGのパレット番号
  u8 wfbc_go;             ///<TRUE:WFBCへのワープを押した
  
  s16 infomsg_wait;       ///<インフォメーションメッセージ更新ウェイト
  u8 wfbc_seq;
  u8 bar_type;            ///<BG_BAR_TYPE_xxx
  
  u8 title_print_type;    ///<_TITLE_PRINT_xxx
  u8 infomsg_trans_req;
  u8 mission_ready_msg_seq;
  u8 mission_exe_msg_seq;
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
static void _IntSub_ActorCreate_LvNum(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_PointNum(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_EntryButton(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorUpdate_TouchTown(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_Area(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_CursorS(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_CursorL(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_EntryButton(INTRUDE_SUBDISP_PTR intsub, 
  INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_LvNum(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_PointNum(
  INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static OCCUPY_INFO * _IntSub_GetArreaOccupy(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_TitleMsgUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_InfoMsgUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub);
static u8 _IntSub_GetProfileRecvNum(INTRUDE_COMM_SYS_PTR intcomm);
static u8 _IntSub_TownPosGet(ZONEID zone_id, GFL_CLACTPOS *dest_pos, int net_id);
static void _SetRect(int x, int y, int half_size_x, int half_size_y, GFL_RECT *rect);
static BOOL _CheckRectHit(int x, int y, const GFL_RECT *rect);
static void _IntSub_TouchUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BGBarUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub);
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
  AREA_POST_LEFT = 0x5 * 8,                           ///<配置座標範囲の左端X
  AREA_POST_RIGHT = 0x1b * 8,                         ///<配置座標範囲の右端X
  AREA_POST_WIDTH = AREA_POST_RIGHT - AREA_POST_LEFT, ///<配置座標範囲の幅
  
  AREA_POST_Y = 3*8 + 4,                              ///<配置座標Y
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
  PRINT_UTIL_Trans( &intsub->printutil_title, intsub->print_que );
  if(intsub->infomsg_trans_req == TRUE 
      && PRINTSYS_QUE_IsExistTarget(intsub->print_que, intsub->infomsg_bmp) == FALSE){
    BmpOam_ActorBmpTrans(intsub->infomsg_bmpoam);
    intsub->infomsg_trans_req = FALSE;
  }
  
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
      default:
        GF_ASSERT(0);
        break;
      }
    }
  }
  
  _IntSub_BGBarUpdate(intcomm, intsub);
  //BGスクリーンカラー変更チェック
  _IntSub_BGColorUpdate(intcomm, intsub);
  
  //インフォメーションメッセージ
  _IntSub_TitleMsgUpdate(intcomm, intsub);
  _IntSub_InfoMsgUpdate(intcomm, intsub);
  
  //アクター更新
  _IntSub_ActorUpdate_TouchTown(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_Area(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_CursorS(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_CursorL(intsub, intcomm, area_occupy);
  _IntSub_ActorUpdate_EntryButton(intsub, intcomm, area_occupy);
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
  
  intsub->strbuf_temp = GFL_STR_CreateBuffer(64, HEAPID_FIELD_SUBSCREEN);
  intsub->strbuf_info = GFL_STR_CreateBuffer(64, HEAPID_FIELD_SUBSCREEN);
  intsub->strbuf_title = GFL_STR_CreateBuffer(64, HEAPID_FIELD_SUBSCREEN);
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
  _IntSub_ActorCreate_LvNum(intsub, handle);
  _IntSub_ActorCreate_PointNum(intsub, handle);
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
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
  }
  
  //タッチパレス島
  head.anmseq = PALACE_ACT_ANMSEQ_TOUCH_PALACE;
  head.pos_x = 128;
  head.pos_y = 0xb*8+4;
  intsub->act[INTSUB_ACTOR_TOUCH_PALACE] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], FALSE);  //表示OFF
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
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], TRUE);
    head.pos_x -= width;
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
  	0x1c*8, 0x10*8,             //X, Y座標
  	PALACE_ACT_ANMSEQ_BTN,      //アニメーションシーケンス
  	SOFTPRI_ENTRY_BUTTON,       //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  intsub->act[INTSUB_ACTOR_ENTRY] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELD_SUBSCREEN);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_ENTRY], FALSE);  //表示OFF
}

//==============================================================================
//  更新
//==============================================================================
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
  int i, profile_num, area_width, net_id;
  GFL_CLWK *act;
  GFL_CLACTPOS pos;
  int my_area, now_tbl_pos = 0;
  u8 pal_tbl[FIELD_COMM_MEMBER_MAX] = {0, 0, 0, 0};
  s32 tbl_count = 0, actno;
  
  profile_num = _IntSub_GetProfileRecvNum(intcomm);
  my_area = intcomm->intrude_status_mine.palace_area;
  area_width = AREA_POST_WIDTH / profile_num;
  pos.x = AREA_POST_LEFT + area_width;
  pos.y = AREA_POST_Y;
  
  //プロフィール受信済みのユーザーのパレットオフセットテーブルを作る
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(intcomm->recv_profile & (1 << net_id)){
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
    act = intsub->act[INTSUB_ACTOR_AREA_0 + actno];
    GFL_CLACT_WK_SetPos(act, &pos, CLSYS_DEFREND_SUB);
    GFL_CLACT_WK_SetPlttOffs(act, 
      INTSUB_ACTOR_PAL_BASE_START + pal_tbl[i], CLWK_PLTTOFFS_MODE_PLTT_TOP);
    GFL_CLACT_WK_SetDrawEnable(act, TRUE);
    pos.x += area_width;
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
  PLAYER_WORK *player_work = GAMESYSTEM_GetMyPlayerWork(intsub->gsys);
  ZONEID mine_zone_id = PLAYERWORK_getZoneID(player_work);
  GFL_CLWK *act;
  GFL_CLACTPOS pos;
  int my_net_id;
  int town_no;
  
  my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);
  town_no = _IntSub_TownPosGet(mine_zone_id, &pos, my_net_id);
  act = intsub->act[INTSUB_ACTOR_CUR_L];
  GFL_CLACT_WK_SetPos(act, &pos, CLSYS_DEFREND_SUB);
  GFL_CLACT_WK_SetDrawEnable(act, TRUE);
}

//--------------------------------------------------------------
/**
 * 更新処理：参加しますボタン
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_EntryButton(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  MISSION_STATUS m_st;
  
  m_st = MISSION_FIELD_CheckStatus(&intcomm->mission);
  switch(m_st){
  case MISSION_STATUS_NULL:
  case MISSION_STATUS_ENTRY:
  case MISSION_STATUS_READY:
  case MISSION_STATUS_EXE:
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_ENTRY], FALSE);
    break;
  case MISSION_STATUS_NOT_ENTRY:
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_ENTRY], TRUE);
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
 * タイトルメッセージ更新処理
 *
 * @param   intcomm		
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_TitleMsgUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int print_type = _TITLE_PRINT_NULL;
  int msg_id;
  
  if(intcomm == NULL || intcomm->palace_in == FALSE){
    print_type = _TITLE_PRINT_PALACE_GO;
    msg_id = msg_invasion_title_000;
  }
  else if(intcomm->intrude_status_mine.palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
    print_type = _TITLE_PRINT_MY_PALACE;
    msg_id = msg_invasion_title_001;
  }
  else{
    print_type = _TITLE_PRINT_OTHER_PALACE0 + intcomm->intrude_status_mine.palace_area;
    msg_id = msg_invasion_title_002;
  }
  
  if(intsub->title_print_type == print_type || print_type == _TITLE_PRINT_NULL){
    return; //既に表示済み
  }
  intsub->title_print_type = print_type;
  
  if(print_type >= _TITLE_PRINT_OTHER_PALACE0 && print_type <= _TITLE_PRINT_OTHER_PALACE3){
    const MYSTATUS *myst;
    myst = GAMEDATA_GetMyStatusPlayer(gamedata, intcomm->intrude_status_mine.palace_area);
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
static void _IntSub_InfoMsgUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  GAME_COMM_INFO_MESSAGE infomsg;
  int k;
  const MISSION_DATA *md;
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
  
  md = MISSION_GetRecvData(&intcomm->mission);
  if(md != NULL){ //ミッション発動中の場合はミッションを表示
#if 0
    if(MISSION_FIELD_CheckStatus(mission) == MISSION_STATUS_READY){
#else
    if(0){
#endif
      intsub->mission_exe_msg_seq = 0;
      switch(intsub->mission_ready_msg_seq){
      case 0:
        break;
      }
    }
    else{
      intsub->mission_ready_msg_seq = 0;
    }
  }
  else if(GameCommInfo_GetMessage(game_comm, &infomsg) == TRUE){
    intsub->mission_exe_msg_seq = 0;
    intsub->mission_ready_msg_seq = 0;
    GFL_MSG_GetString(intsub->msgdata, infomsg.message_id, intsub->strbuf_temp );
    for(k = 0 ; k < INFO_WORDSET_MAX; k++){
      if(infomsg.name[k]!=NULL){
        WORDSET_RegisterWord( intsub->wordset, infomsg.wordset_no[k], infomsg.name[k], 
          PM_MALE, TRUE, PM_LANG);
      }
    }
    WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_info, intsub->strbuf_temp);
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
          if(ZONEDATA_IsWfbc(intcomm->intrude_status[net_id].zone_id) == TRUE){
            intsub->wfbc_go = TRUE;
            intsub->wfbc_seq = 0;
          }
          else{
            Intrude_SetWarpPlayerNetID(game_comm, net_id);
            FIELD_SUBSCREEN_SetAction(subscreen, FIELD_SUBSCREEN_ACTION_INTRUDE_PLAYER_WARP);
          }
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
  
#if 0
  //ミッションアイコンタッチ判定
  if(MISSION_RecvCheck(&intcomm->mission) == TRUE){
    _SetRect(MISSION_ICON_POS_X, MISSION_ICON_POS_Y, 
      POWER_ICON_HITRANGE_HALF_X, POWER_ICON_HITRANGE_HALF_Y, &rect);
    if(_CheckRectHit(x, y, &rect) == TRUE){
      FIELD_SUBSCREEN_SetAction(subscreen, FIELD_SUBSCREEN_ACTION_INTRUDE_MISSION_PUT);
      return;
    }
  }
#endif
}

//--------------------------------------------------------------
/**
 * 接続人数を監視し、Barスクリーンの一人用と通信用を変更する
 *
 * @param   intcomm		
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BGBarUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub)
{
  if(intsub->bar_type == BG_BAR_TYPE_SINGLE){
    if(_IntSub_GetProfileRecvNum(intcomm) > 1){
      GFL_ARC_UTIL_TransVramScreen(
        ARCID_PALACE, NARC_palace_palace_bar2_lz_NSCR, INTRUDE_FRAME_S_BAR, 0, 
        0x800, TRUE, HEAPID_FIELDMAP);
      intsub->bar_type = BG_BAR_TYPE_COMM;
    }
  }
  else{ //BG_BAR_TYPE_COMM
    if(_IntSub_GetProfileRecvNum(intcomm) <= 1){
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
    INTRUDE_FRAME_S_BACKGROUND, _AREA_SCREEN_COLOR_START_X, _AREA_SCREEN_COLOR_START_Y, 
      _AREA_SCREEN_COLOR_SIZE_X, _AREA_SCREEN_COLOR_SIZE_Y, INTSUB_BG_PAL_BASE_START + bg_pal);
  GFL_BG_ChangeScreenPalette(
    INTRUDE_FRAME_S_BAR, _AREA_SCREEN_COLOR_START_X, _AREA_SCREEN_COLOR_START_Y, 
      _AREA_SCREEN_COLOR_SIZE_X, _AREA_SCREEN_COLOR_SIZE_Y, INTSUB_BG_PAL_BASE_START + bg_pal);

  GFL_BMPWIN_MakeScreen(intsub->printutil_title.win);

  GFL_BG_LoadScreenV_Req(INTRUDE_FRAME_S_BACKGROUND);
  GFL_BG_LoadScreenV_Req(INTRUDE_FRAME_S_BAR);
  intsub->now_bg_pal = bg_pal;
}

