//==============================================================================
/**
 * @file    union_subdisp.c
 * @brief   ユニオン下画面
 * @author  matsuda
 * @date    2009.07.27(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "net_app/union/union_main.h"
#include "net_app/union/union_subdisp.h"
#include "unionroom.naix"


//==============================================================================
//  定数定義
//==============================================================================
enum{
  UNISUB_ACTOR_APPEAL_BATTLE,
  UNISUB_ACTOR_APPEAL_TRADE,
  UNISUB_ACTOR_APPEAL_RECORD,
  UNISUB_ACTOR_APPEAL_PICTURE,
  UNISUB_ACTOR_APPEAL_GURUGURU,
  
  UNISUB_ACTOR_MAX,
};

///アクターソフトプライオリティ
enum{
  SOFTPRI_APPEAL_ICON = 10,
};

///アクターBGプライオリティ
enum{
  BGPRI_APPEAL_ICON = 0,
};


//==============================================================================
//  構造体定義
//==============================================================================
///ユニオン下画面制御ワーク
typedef struct _UNION_SUBDISP{
  GAMESYS_WORK *gsys;

  u32 index_cgr;
  u32 index_pltt;
  u32 index_cell;

	GFL_CLUNIT *clunit;
  GFL_CLWK *act[UNISUB_ACTOR_MAX];
  
  u8 appeal_no;    ///<選択中のアピール番号
  u8 padding[3];
}UNION_SUBDISP;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _UniSub_SystemSetup(UNION_SUBDISP_PTR unisub);
static void _UniSub_SystemExit(UNION_SUBDISP_PTR unisub);
static void _UniSub_BGLoad(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle);
static void _UniSub_BGUnload(UNION_SUBDISP_PTR unisub);
static void _UniSub_ActorResouceLoad(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle);
static void _UniSub_ActorResourceUnload(UNION_SUBDISP_PTR unisub);
static void _UniSub_ActorCreate(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle);
static void _UniSub_ActorDelete(UNION_SUBDISP_PTR unisub);
static void _UniSub_TouchUpdate(UNION_SUBDISP_PTR unisub);
static void _UniSub_IconPalChange(UNION_SUBDISP_PTR unisub, int act_index);


//==============================================================================
//  データ
//==============================================================================
///ボタンと対応したplay_category
ALIGN4 static const u8 AppealNo_CategoryTbl[] = {
  UNION_APPEAL_BATTLE,      ///<戦闘
  UNION_APPEAL_TRADE,       ///<交換
  UNION_APPEAL_RECORD,      ///<レコードコーナー
  UNION_APPEAL_PICTURE,     ///<お絵かき
  UNION_APPEAL_GURUGURU,    ///<ぐるぐる交換
};


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ユニオン下画面：初期設定
 *
 * @param   gsys		
 *
 * @retval  UNION_SUBDISP_PTR		
 */
//==================================================================
UNION_SUBDISP_PTR UNION_SUBDISP_Init(GAMESYS_WORK *gsys)
{
  UNION_SUBDISP_PTR unisub;
  ARCHANDLE *handle;
  
  unisub = GFL_HEAP_AllocClearMemory(HEAPID_FIELDMAP, sizeof(UNION_SUBDISP));
  unisub->gsys = gsys;
  
  handle = GFL_ARC_OpenDataHandle(ARCID_UNION, HEAPID_FIELDMAP);
  
  _UniSub_SystemSetup(unisub);
  _UniSub_BGLoad(unisub, handle);
  _UniSub_ActorResouceLoad(unisub, handle);
  _UniSub_ActorCreate(unisub, handle);
  
  GFL_ARC_CloseDataHandle(handle);
  
  return unisub;
}

//==================================================================
/**
 * ユニオン下画面：破棄設定
 *
 * @param   unisub		
 */
//==================================================================
void UNION_SUBDISP_Exit(UNION_SUBDISP_PTR unisub)
{
  _UniSub_ActorDelete(unisub);
  _UniSub_ActorResourceUnload(unisub);
  _UniSub_BGUnload(unisub);
  _UniSub_SystemExit(unisub);

  GFL_HEAP_FreeMemory(unisub);
}

//==================================================================
/**
 * ユニオン下画面：更新
 *
 * @param   unisub		
 */
//==================================================================
void UNION_SUBDISP_Update(UNION_SUBDISP_PTR unisub)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(unisub->gsys);
  UNION_SYSTEM_PTR unisys;
  
  _UniSub_TouchUpdate(unisub);
  
  if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_UNION){
    unisys = GameCommSys_GetAppWork(game_comm);
    if(unisys != NULL){
      UnionMain_SetAppealNo(unisys, unisub->appeal_no);
    }
  }
}

//==================================================================
/**
 * ユニオン下画面：描画
 *
 * @param   unisub		
 */
//==================================================================
void UNION_SUBDISP_Draw(UNION_SUBDISP_PTR unisub)
{
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * システム関連のセットアップ
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_SystemSetup(UNION_SUBDISP_PTR unisub)
{
  unisub->clunit = GFL_CLACT_UNIT_Create(UNISUB_ACTOR_MAX, 5, HEAPID_FIELDMAP);
}

//--------------------------------------------------------------
/**
 * システム関連の破棄
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_SystemExit(UNION_SUBDISP_PTR unisub)
{
  GFL_CLACT_UNIT_Delete(unisub->clunit);
}

//--------------------------------------------------------------
/**
 * BGロード
 *
 * @param   unisub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _UniSub_BGLoad(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle)
{
	GFL_BG_BGCNT_HEADER TextBgCntDat = {
		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x4000,GX_BG_EXTPLTT_01,
		3, 0, 0, FALSE
	};
	GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &TextBgCntDat, GFL_BG_MODE_TEXT );

	//BG VRAMクリア
	GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
	
  GFL_ARCHDL_UTIL_TransVramBgCharacter(handle, NARC_unionroom_unionbg_NCGR, GFL_BG_FRAME3_S, 0, 
    0, FALSE, HEAPID_FIELDMAP);
  GFL_ARCHDL_UTIL_TransVramScreen(handle, NARC_unionroom_unionbg_test_NSCR, GFL_BG_FRAME3_S, 0, 
    0x800, FALSE, HEAPID_FIELDMAP);
  GFL_ARCHDL_UTIL_TransVramPalette(handle, NARC_unionroom_unionbg_NCLR, PALTYPE_SUB_BG, 0, 
    0x20*3, HEAPID_FIELDMAP);

	GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
}

//--------------------------------------------------------------
/**
 * BGアンロード
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_BGUnload(UNION_SUBDISP_PTR unisub)
{
	GFL_BG_FreeBGControl(GFL_BG_FRAME3_S);
	GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
}

//--------------------------------------------------------------
/**
 * アクターで使用するリソースのロード
 *
 * @param   unisub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _UniSub_ActorResouceLoad(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle)
{
  //OBJ共通パレット
  unisub->index_pltt = GFL_CLGRP_PLTT_RegisterEx(
    handle, NARC_unionroom_unionobj_NCLR, CLSYS_DRAW_SUB, 0, 0, 2, HEAPID_FIELDMAP );

  //アピールアイコン
  unisub->index_cgr = GFL_CLGRP_CGR_Register(
    handle, NARC_unionroom_appeal_icon_NCGR, FALSE, CLSYS_DRAW_SUB, HEAPID_FIELDMAP);
  unisub->index_cell = GFL_CLGRP_CELLANIM_Register(
    handle, NARC_unionroom_appeal_icon_NCER, 
    NARC_unionroom_appeal_icon_NANR, HEAPID_FIELDMAP);
}

//--------------------------------------------------------------
/**
 * アクターで使用するアクターのアンロード
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_ActorResourceUnload(UNION_SUBDISP_PTR unisub)
{
  GFL_CLGRP_PLTT_Release(unisub->index_pltt);
  GFL_CLGRP_CGR_Release(unisub->index_cgr);
  GFL_CLGRP_CELLANIM_Release(unisub->index_cell);
}

//--------------------------------------------------------------
/**
 * アクター作成
 *
 * @param   unisub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _UniSub_ActorCreate(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 192 - 16,                   //X, Y座標
  	0,
  	SOFTPRI_APPEAL_ICON,    //ソフトプライオリティ
  	BGPRI_APPEAL_ICON,      //BGプライオリティ
  };
  
  for(i = UNISUB_ACTOR_APPEAL_BATTLE; i <= UNISUB_ACTOR_APPEAL_GURUGURU; i++){
    head.pos_x = 32 + i * 48;
    head.anmseq = i;
    unisub->act[i] = GFL_CLACT_WK_Create(unisub->clunit, 
      unisub->index_cgr, unisub->index_pltt, unisub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
  }
}

//--------------------------------------------------------------
/**
 * アクター削除
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_ActorDelete(UNION_SUBDISP_PTR unisub)
{
  int i;
  
  for(i = 0; i < UNISUB_ACTOR_MAX; i++){
    GFL_CLACT_WK_Remove(unisub->act[i]);
  }
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * タッチ判定
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_TouchUpdate(UNION_SUBDISP_PTR unisub)
{
  u32 x, y;
  int i;
  GFL_CLACTPOS clpos;
  struct{
    s16 top;
    s16 bottom;
    s16 left;
    s16 right;
  }crect;
  
  if(GFL_UI_TP_GetPointTrg(&x, &y) == FALSE){
    return;
  }
  
  for(i = UNISUB_ACTOR_APPEAL_BATTLE; i <= UNISUB_ACTOR_APPEAL_GURUGURU; i++){
    GFL_CLACT_WK_GetPos(unisub->act[i], &clpos, CLSYS_DEFREND_SUB);
    crect.top = clpos.y - 16;
    crect.bottom = clpos.y + 16;
    crect.left = clpos.x - 16;
    crect.right = clpos.x + 16;
    if(crect.left < x && crect.right > x && crect.top < y && crect.bottom > y){
      _UniSub_IconPalChange(unisub, i);
      return;
    }
  }
}

//--------------------------------------------------------------
/**
 * 対象のアイコンの色を変える
 *
 * @param   unisub		
 * @param   act_index		
 */
//--------------------------------------------------------------
static void _UniSub_IconPalChange(UNION_SUBDISP_PTR unisub, int act_index)
{
  int i;
  UNION_APPEAL select_appeal;
  
  select_appeal = AppealNo_CategoryTbl[act_index - UNISUB_ACTOR_APPEAL_BATTLE];
  if(select_appeal == unisub->appeal_no){
    unisub->appeal_no = UNION_APPEAL_NULL;
  }
  else{
    unisub->appeal_no = select_appeal;
  }
  
  for(i = UNISUB_ACTOR_APPEAL_BATTLE; i <= UNISUB_ACTOR_APPEAL_GURUGURU; i++){
    if(i == act_index && unisub->appeal_no != UNION_APPEAL_NULL){
      GFL_CLACT_WK_SetPlttOffs(unisub->act[i], 1, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    }
    else{
      GFL_CLACT_WK_SetPlttOffs(unisub->act[i], 0, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    }
  }
}

