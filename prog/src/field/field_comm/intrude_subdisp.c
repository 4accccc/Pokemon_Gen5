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
};

///アクターソフトプライオリティ
enum{
  SOFTPRI_TOWN = 100,
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


//==============================================================================
//  構造体定義
//==============================================================================
///侵入下画面制御ワーク
typedef struct _INTRUDE_SUBDISP{
  GAMESYS_WORK *gsys;
  
  u32 index_cgr;
  u32 index_pltt;
  u32 index_cell;

	GFL_CLUNIT *clunit;
  GFL_CLWK *act[INTSUB_ACTOR_MAX];
  
  u8 my_net_id;
  u8 padding[3];
}INTRUDE_SUBDISP;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _IntSub_SystemSetup(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_SystemExit(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BGLoad(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_BGUnload(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorResouceLoad(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorResourceUnload(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorCreate(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorDelete(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorCreate_Town(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_SenkyoEff(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_Area(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_CursorS(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_CursorL(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_Mark(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_Power(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_LvNum(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_PointNum(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);


//==============================================================================
//  データ
//==============================================================================
///街の座標
static const struct{
  u16 x;
  u16 y;
}TownPos[] = {
  {7*8, 3*8},
  {0x10*8, 3*8},
  {0x19*8, 3*8},
  {2*8, 0xa*8},
  {0x1e*8, 0xa*8},
  {7*8, 0x11*8},
  {0x10*8, 0x11*8},
  {0x19*8, 0x11*8},
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

  intsub->my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  if(intsub->my_net_id > FIELD_COMM_MEMBER_MAX){
    GF_ASSERT_MSG(0, "my_net_id = %d\n", intsub->my_net_id);
    intsub->my_net_id = 0;
  }
  
  handle = GFL_ARC_OpenDataHandle(ARCID_PALACE, HEAPID_FIELDMAP);
  
  _IntSub_SystemSetup(intsub);
  _IntSub_BGLoad(intsub, handle);
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
  INTRUDE_COMM_SYS_PTR intcomm = GameCommSys_GetAppWork(game_comm);
  
  
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
  
  GF_ASSERT(NELEMS(TownPos) == (INTSUB_ACTOR_TOWN_MAX - INTSUB_ACTOR_TOWN_0 + 1));
  GF_ASSERT(NELEMS(TownPos) == INTRUDE_TOWN_MAX);
  for(i = INTSUB_ACTOR_TOWN_0; i <= INTSUB_ACTOR_TOWN_MAX; i++){
    head.pos_x = TownPos[i].x;
    head.pos_y = TownPos[i].y;
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
  }
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
  
  GF_ASSERT(NELEMS(TownPos) == (INTSUB_ACTOR_SENKYO_EFF_MAX - INTSUB_ACTOR_SENKYO_EFF_0 + 1));
  for(i = INTSUB_ACTOR_SENKYO_EFF_0; i <= INTSUB_ACTOR_SENKYO_EFF_MAX; i++){
    head.pos_x = TownPos[i].x;
    head.pos_y = TownPos[i].y;
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
    head.anmseq++;
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
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_POWER,     //アニメーションシーケンス
  	SOFTPRI_POWER,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  //POWER
  intsub->act[INTSUB_ACTOR_POWER] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_POWER], FALSE);  //表示OFF

  //MISSION
  head.anmseq = PALACE_ACT_ANMSEQ_MISSION;
  intsub->act[INTSUB_ACTOR_MISSION] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_MISSION], FALSE);  //表示OFF

  //INCLUSION
  head.anmseq = PALACE_ACT_ANMSEQ_INCLUSION;
  intsub->act[INTSUB_ACTOR_INCLUSION] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_INCLUSION], FALSE);  //表示OFF
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
    {0x1a*8, 0x16*8},         //百の位
    {0x1a*8 + 2*8, 0x16*8},   //十の位
    {0x1a*8 + 4*8, 0x16*8},   //一の位
  };
  
  GF_ASSERT(NELEMS(LvNumPos) == (INTSUB_ACTOR_LV_NUM_KETA_MAX - INTSUB_ACTOR_LV_NUM_KETA_0 + 1));
  for(i = INTSUB_ACTOR_LV_NUM_KETA_0; i <= INTSUB_ACTOR_LV_NUM_KETA_MAX; i++){
    head.pos_x = LvNumPos[i].x;
    head.pos_y = LvNumPos[i].y;
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
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
  
  for(i = INTSUB_ACTOR_POINT_NUM_KETA_0; i <= INTSUB_ACTOR_POINT_NUM_KETA_MAX; i++){
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
    head.pos_x += width;
  }
}

