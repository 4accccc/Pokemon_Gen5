//==============================================================================
/**
 * @file    monolith_tool.c
 * @brief   モノリス画面ツール
 * @author  matsuda
 * @date    2009.11.21(土)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "arc_def.h"
#include "monolith_common.h"
#include "monolith.naix"
#include "monolith_tool.h"


//==============================================================================
//  定数定義
//==============================================================================
///共通素材セルのアニメシーケンス番号
typedef enum{
  COMMON_ANMSEQ_PANEL_LARGE,
  COMMON_ANMSEQ_PANEL_SMALL,
  COMMON_ANMSEQ_UP,
  COMMON_ANMSEQ_DOWN,
  COMMON_ANMSEQ_BLACK_TOWN,
  COMMON_ANMSEQ_WHITE_TOWN,
  COMMON_ANMSEQ_RETURN,
}COMMON_ANMSEQ;

///ソフトプライオリティ
enum{
  SOFTPRI_PANEL_BMPFONT = 10,
  SOFTPRI_PANEL,
};

///アクターBGプライオリティ
enum{
  BGPRI_PANEL = 0,
};

///パネル(大)BMPFONT座標X
#define PANEL_LARGE_BMPFONT_POS_X   (PANEL_POS_X - PANEL_LARGE_CHARSIZE_X/2*8)
///パネル(小)BMPFONT座標X
#define PANEL_SMALL_BMPFONT_POS_X   (PANEL_POS_X - PANEL_SMALL_CHARSIZE_X/2*8)


//==============================================================================
//  データ
//==============================================================================
///パネルアクターヘッダ
static const GFL_CLWK_DATA ActHead_Panel = {
  PANEL_POS_X,                   //pos_x
  0,                             //pos_y
  COMMON_ANMSEQ_PANEL_LARGE,     //anmseq
  SOFTPRI_PANEL,                 //softpri
  BGPRI_PANEL,                   //bgpri
};

///パネルアクターのBMPOAMヘッダ
static const BMPOAM_ACT_DATA BmpOamHead_Panel = {
  NULL,                          //bmp
  0,                             //x
  0,                             //y
  0,                             //pltt_index
  0,                             //pal_offset
  SOFTPRI_PANEL_BMPFONT,         //soft_pri
  BGPRI_PANEL,                   //bg_pri
  0,                             //setSerface
  0,                             //draw_type
};


//==================================================================
/**
 * パネルアクター生成
 *
 * @param   setup       モノリス全画面共通設定データへのポインタ
 * @param   dest		    生成したパネルアクター代入先
 * @param   res_index		共通素材Index
 * @param   size		    パネルサイズ
 * @param   y		        Y座標(中心)
 * @param   msg_id      メッセージID
 */
//==================================================================
void MonolithTool_Panel_Create(MONOLITH_SETUP *setup, PANEL_ACTOR *dest, COMMON_RESOURCE_INDEX res_index, PANEL_SIZE size, int y, u32 msg_id)
{
  CLSYS_DEFREND_TYPE defrend_type;
  GFL_CLWK_DATA head = ActHead_Panel;
  BMPOAM_ACT_DATA bmpoam_head = BmpOamHead_Panel;
  int bmp_size_x;
  STRBUF *strbuf;
  
  //BMP作成
  bmp_size_x = (size == PANEL_SIZE_LARGE) ? PANEL_LARGE_CHARSIZE_X : PANEL_SMALL_CHARSIZE_X;
  dest->bmp = GFL_BMP_Create(bmp_size_x, PANEL_CHARSIZE_Y, GFL_BMP_16_COLOR, HEAPID_MONOLITH);
  
  //ヘッダ作成
  head.anmseq = (size == PANEL_SIZE_LARGE) ? COMMON_ANMSEQ_PANEL_LARGE : COMMON_ANMSEQ_PANEL_SMALL;
  head.pos_y = y;
  bmpoam_head.y = y + 8;
  bmpoam_head.bmp = dest->bmp;
  bmpoam_head.pltt_index = setup->common_res[res_index].pltt_bmpfont_index;
  if(res_index == COMMON_RESOURCE_INDEX_UP){
    defrend_type = CLSYS_DEFREND_MAIN;
    head.anmseq = COMMON_ANMSEQ_PANEL_LARGE;
    bmpoam_head.x = PANEL_LARGE_BMPFONT_POS_X;
    bmpoam_head.draw_type = CLSYS_DRAW_MAIN;
  }
  else{
    defrend_type = CLSYS_DEFREND_SUB;
    head.anmseq = COMMON_ANMSEQ_PANEL_SMALL;
    bmpoam_head.x = PANEL_SMALL_BMPFONT_POS_X;
    bmpoam_head.draw_type = CLSYS_DRAW_SUB;
  }
  bmpoam_head.setSerface = defrend_type;
  
  //生成
  dest->cap = GFL_CLACT_WK_Create(setup->clunit, setup->common_res[res_index].char_index, 
    setup->common_res[res_index].pltt_index, setup->common_res[res_index].cell_index, 
	  &head, defrend_type, HEAPID_MONOLITH);
  dest->bmpoam = BmpOam_ActorAdd(setup->bmpoam_sys, &bmpoam_head);
  
  //文字描画
  strbuf = GFL_MSG_CreateString( setup->mm_monolith, msg_id );
  PRINTSYS_PrintQue( setup->printQue, dest->bmp, 0, 0, strbuf, setup->font_handle );
  
  //内部フラグセット
  dest->trans_flag = TRUE;
}

//==================================================================
/**
 * パネルアクター削除
 *
 * @param   panel		パネルアクターへのポインタ
 */
//==================================================================
void MonolithTool_Panel_Delete(PANEL_ACTOR *panel)
{
  GFL_CLACT_WK_Remove(panel->cap);
  BmpOam_ActorDel(panel->bmpoam);
  GFL_BMP_Delete(panel->bmp);
  panel->cap = NULL;
  panel->bmpoam = NULL;
  panel->bmp = NULL;
}

//==================================================================
/**
 * パネルアクター更新処理
 *
 * @param   setup   モノリス全画面共通設定データへのポインタ
 * @param   panel		パネルアクターへのポインタ
 *
 * @retval  BOOL		TRUE:BMP転送を行った
 */
//==================================================================
BOOL MonolithTool_Panel_TransUpdate(MONOLITH_SETUP *setup, PANEL_ACTOR *panel)
{
  if( panel->trans_flag == TRUE 
      && PRINTSYS_QUE_IsExistTarget(setup->printQue, panel->bmp) == FALSE ){
    BmpOam_ActorBmpTrans(panel->bmpoam);
    panel->trans_flag = FALSE;
    return TRUE;
  }
  return FALSE;
}



#if 0
//--------------------------------------------------------------
/**
 * @brief   メッセージ描画関連の設定
 *
 * @param   cs		
 */
//--------------------------------------------------------------
static void CCLocal_MessageSetting(COMPATI_SYS *cs)
{
	int i;

	cs->drawwin[CCBMPWIN_TITLE].win = GFL_BMPWIN_Create(
		FRAME_MSG_S, 8, 2, 16, 2, CC_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
	cs->drawwin[CCBMPWIN_POINT].win = GFL_BMPWIN_Create(
		FRAME_MSG_S, 14, 8, 6, 2, CC_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
	cs->drawwin[CCBMPWIN_TALK].win = GFL_BMPWIN_Create(
		FRAME_MSG_S, 1, 18, 30, 4, CC_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
	for(i = 0; i < CCBMPWIN_MAX; i++){
		GF_ASSERT(cs->drawwin[i].win != NULL);
		cs->drawwin[i].bmp = GFL_BMPWIN_GetBmp(cs->drawwin[i].win);
		GFL_BMP_Clear( cs->drawwin[i].bmp, 0xff );
		GFL_BMPWIN_MakeScreen( cs->drawwin[i].win );
		GFL_BMPWIN_TransVramCharacter( cs->drawwin[i].win );
		PRINT_UTIL_Setup( cs->drawwin[i].printUtil, cs->drawwin[i].win );
	}
	GFL_BG_LoadScreenReq(FRAME_MSG_S);	//MakeScreenしたのを反映

	cs->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_MONOLITH );

//		PRINTSYS_Init( HEAPID_MONOLITH );
	cs->printQue = PRINTSYS_QUE_Create( HEAPID_MONOLITH );

	cs->mm = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_compati_check_dat, HEAPID_MONOLITH);
	for(i = 0; i < CCBMPWIN_MAX; i++){
		cs->strbuf_win[i] = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
	}
	cs->strbuf_point = GFL_MSG_CreateString(cs->mm, COMPATI_STR_009);
}

//--------------------------------------------------------------
/**
 * @brief   円アクター作成
 *
 * @param   cs		
 * @param   hdl		アーカイブハンドル
 */
//--------------------------------------------------------------
static void CCLocal_CircleActor_Create(COMPATI_SYS *cs, ARCHANDLE *hdl)
{
	int i;
	
	//リソース登録
	cs->cgr_id[CHARID_CIRCLE] = GFL_CLGRP_CGR_Register(
		hdl, NARC_compati_check_cc_circle_NCGR, FALSE, CLSYS_DRAW_MAIN, HEAPID_MONOLITH);
	cs->cell_id[CELLID_CIRCLE] = GFL_CLGRP_CELLANIM_Register(
		hdl, NARC_compati_check_cc_circle_NCER, NARC_compati_check_cc_circle_NANR, HEAPID_MONOLITH);
	//パレットリソース登録(押した円だけ色を変えるので円の数分登録)
	for(i = 0; i < CC_CIRCLE_MAX; i++){
		cs->pltt_id[PLTTID_CIRCLE + i] = GFL_CLGRP_PLTT_RegisterEx(hdl, 
			NARC_compati_check_cc_circle_NCLR,
			CLSYS_DRAW_MAIN, 0x20 * i, 0, 1, HEAPID_MONOLITH);
	}
	
	//アクター登録
	for(i = 0; i < CC_CIRCLE_MAX; i++){
		cs->circle_cap[i] = GFL_CLACT_WK_Create(cs->clunit, cs->cgr_id[CHARID_CIRCLE], 
			cs->pltt_id[PLTTID_CIRCLE + i], cs->cell_id[CELLID_CIRCLE], 
			&CircleObjParam, CLSYS_DEFREND_MAIN, HEAPID_MONOLITH);
		GFL_CLACT_WK_AddAnmFrame(cs->circle_cap[i], FX32_ONE);
		GFL_CLACT_WK_SetDrawEnable(cs->circle_cap[i], FALSE);
	}
}

//--------------------------------------------------------------
/**
 * @brief   円アクター破棄
 *
 * @param   cs		
 */
//--------------------------------------------------------------
static void CCLocal_CircleActor_Delete(COMPATI_SYS *cs)
{
	int i;
	
	//アクター削除
	for(i = 0; i < CC_CIRCLE_MAX; i++){
		GFL_CLACT_WK_Remove(cs->circle_cap[i]);
	}
	
	//リソース解放
	GFL_CLGRP_CGR_Release(cs->cgr_id[CHARID_CIRCLE]);
	GFL_CLGRP_CELLANIM_Release(cs->cell_id[CELLID_CIRCLE]);
	for(i = 0; i < CC_CIRCLE_MAX; i++){
		GFL_CLGRP_PLTT_Release(cs->pltt_id[PLTTID_CIRCLE + i]);
	}
}
#endif

