//==============================================================================
/**
 * @file    beacon_view.c
 * @brief   すれ違い通信状態参照画面
 * @author  matsuda
 * @date    2009.12.13(日)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "field/field_subscreen.h"
#include "gamesystem/game_beacon.h"
#include "infowin/infowin.h"
#include "app_menu_common.naix"
#include "arc_def.h"
#include "font/font.naix"
#include "field/beacon_view.h"
#include "print\printsys.h"
#include "message.naix"

#include "beacon_status.naix"
#include "wifi_unionobj.naix"
#include "beacon_view_local.h"

//==============================================================================
//  定数定義
//==============================================================================


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void BEACON_VIEW_TouchUpdata(BEACON_VIEW_PTR view);
static void _BeaconView_SystemSetup(BEACON_VIEW_PTR view);
static void _BeaconView_SystemExit(BEACON_VIEW_PTR view);
static void _BeaconView_BGLoad(BEACON_VIEW_PTR view, ARCHANDLE *handle);
static void _BeaconView_BGUnload(BEACON_VIEW_PTR view);
static void _BeaconView_ActorResourceLoad(BEACON_VIEW_PTR view, ARCHANDLE *handle);
static void _BeaconView_ActorResourceUnload(BEACON_VIEW_PTR view);
static void _BeaconView_ActorCreate(BEACON_VIEW_PTR view, ARCHANDLE *handle);
static void _BeaconView_ActorDelete(BEACON_VIEW_PTR view);
static void _BeaconView_BmpWinCreate(BEACON_VIEW_PTR view);
static void _BeaconView_BmpWinDelete(BEACON_VIEW_PTR view);

static void obj_ObjResInit( BEACON_VIEW_PTR wk, OBJ_RES_TBL* res, const OBJ_RES_SRC* srcTbl, ARCHANDLE* p_handle );
static void obj_ObjResRelease( BEACON_VIEW_PTR wk, OBJ_RES_TBL* res );
static void panel_PanelObjAdd( BEACON_VIEW_PTR wk, u8 idx );
static void panel_PanelObjDel( BEACON_VIEW_PTR wk, u8 idx );

//==============================================================================
//  データ
//==============================================================================
///タッチ範囲テーブル
static const GFL_UI_TP_HITTBL TouchRect[] = {
  {//Gパワー
    0x15*8,
    0x18*8,
    0*8,
    6*8,
  },
  {//おめでとう
    0x15*8,
    0x18*8,
    6*8,
    12*8,
  },
  {//戻る
    0x15*8,
    0x18*8,
    (0x20-3)*8,
    0x20*8,
  },
  { GFL_UI_TP_HIT_END, 0, 0, 0 },
};


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * すれ違い参照画面：初期設定
 *
 * @param   gsys		
 *
 * @retval  BEACON_VIEW_PTR		
 */
//==================================================================
BEACON_VIEW_PTR BEACON_VIEW_Init(GAMESYS_WORK *gsys, FIELD_SUBSCREEN_WORK *subscreen)
{
  BEACON_VIEW* wk;
  
  wk = GFL_HEAP_AllocClearMemory(HEAPID_BVIEW_TMP, sizeof(BEACON_VIEW));
  wk->gsys = gsys;
  wk->subscreen = subscreen;
 
  wk->heapID = HEAPID_FIELDMAP;
  wk->tmpHeapID = GFL_HEAP_LOWID( HEAPID_FIELDMAP );

  wk->arc_handle = GFL_ARC_OpenDataHandle(ARCID_BEACON_STATUS, HEAPID_BVIEW_TMP);
  
  _BeaconView_SystemSetup(wk);
  _BeaconView_BGLoad(wk, wk->arc_handle);
  _BeaconView_ActorResourceLoad(wk, wk->arc_handle);
  _BeaconView_ActorCreate(wk, wk->arc_handle);
  _BeaconView_BmpWinCreate(wk);
  
  GFL_ARC_CloseDataHandle(wk->arc_handle);

  return wk;
}

//==================================================================
/**
 * すれ違い参照画面：破棄設定
 *
 * @param   view		
 */
//==================================================================
void BEACON_VIEW_Exit(BEACON_VIEW_PTR wk)
{
  _BeaconView_BmpWinDelete( wk );
  _BeaconView_ActorDelete( wk );
  _BeaconView_ActorResourceUnload( wk );
  _BeaconView_BGUnload( wk );
  _BeaconView_SystemExit( wk );

  GFL_HEAP_FreeMemory( wk );
}

//==================================================================
/**
 * すれ違い参照画面：更新
 *
 * @param   view		
 */
//==================================================================
void BEACON_VIEW_Update(BEACON_VIEW_PTR wk, BOOL bActive )
{
  const GAMEBEACON_INFO *info;
  u32 old_log_count;
  s32 new_log_num, copy_src, copy_dest, write_start;
  int i;
  
  BEACON_VIEW_TouchUpdata( wk );

  PRINTSYS_QUE_Main(wk->printQue);
  for(i = 0; i < VIEW_LOG_MAX; i++){
    PRINT_UTIL_Trans( &wk->print_util[i], wk->printQue );
  }
  
  //BMP描画
  {
    old_log_count = wk->log_count;
    wk->log_count = GAMEBEACON_Get_LogCount();
    new_log_num = wk->log_count - old_log_count;  //更新されたログ件数
    
    if(new_log_num > 0){
      copy_dest = 0;
      for(copy_src = new_log_num; copy_src < VIEW_LOG_MAX; copy_src++){
        GFL_BMP_DATA *bmp_src, *bmp_dest;
        
        bmp_src = GFL_BMPWIN_GetBmp(wk->win[copy_src]);
        bmp_dest = GFL_BMPWIN_GetBmp(wk->win[copy_dest]);
        GFL_BMP_Copy( bmp_src, bmp_dest );
        GFL_BMPWIN_TransVramCharacter( wk->win[copy_dest] );
        copy_dest++;
      }
      
      write_start = new_log_num - 1;
      if(write_start >= VIEW_LOG_MAX){
        write_start = VIEW_LOG_MAX - 1;
      }
      for( ; copy_dest < VIEW_LOG_MAX; copy_dest++){
        info = GAMEBEACON_Get_BeaconLog(write_start);
        if(info != NULL){
          GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[copy_dest]), 0x00 );
          GFL_MSG_GetString(wk->mm_status, GAMEBEACON_GetMsgID(info), wk->strbuf_temp);
          GAMEBEACON_Wordset(info, wk->wordset, wk->tmpHeapID );
          WORDSET_ExpandStr( wk->wordset, wk->strbuf_expand, wk->strbuf_temp );
          PRINT_UTIL_PrintColor( &wk->print_util[copy_dest], wk->printQue, 0, 0, 
            wk->strbuf_expand, wk->fontHandle, PRINTSYS_LSB_Make( 15, 2, 0 ) );
        }
        write_start--;
        if(write_start < 0){
          break;
        }
      }
    }
  }
}

//==================================================================
/**
 * すれ違い参照画面：描画
 *
 * @param   wk		
 */
//==================================================================
void BEACON_VIEW_Draw(BEACON_VIEW_PTR wk)
{
  ;
}

//--------------------------------------------------------------
/**
 * タッチ判定
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void BEACON_VIEW_TouchUpdata(BEACON_VIEW_PTR wk)
{
  u32 tp_x, tp_y;
  
  int tp_ret = GFL_UI_TP_HitTrg(TouchRect);
  
  switch(tp_ret){
  case 0: //Gパワー
    OS_TPrintf("Gパワー ビーコンセット\n");
    GAMEBEACON_Set_EncountDown();
    break;
  case 1: //おめでとう
    OS_TPrintf("おめでとう ビーコンセット\n");
    GAMEBEACON_Set_Congratulations();
    break;
  case 2: //戻る
    FIELD_SUBSCREEN_SetAction( wk->subscreen , FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_CGEAR);
    break;
  }
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * システム関連のセットアップ
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _BeaconView_SystemSetup(BEACON_VIEW_PTR wk)
{
	wk->printQue = PRINTSYS_QUE_Create( wk->heapID );
	wk->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
  wk->wordset = WORDSET_Create( wk->heapID);

	wk->mm_status = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_beacon_status_dat, wk->heapID );

  wk->strbuf_temp = GFL_STR_CreateBuffer( 128, wk->heapID );
  wk->strbuf_expand = GFL_STR_CreateBuffer( 128, wk->heapID );
}

//--------------------------------------------------------------
/**
 * システム関連の破棄
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _BeaconView_SystemExit(BEACON_VIEW_PTR wk)
{
  GFL_STR_DeleteBuffer(wk->strbuf_temp);
  GFL_STR_DeleteBuffer(wk->strbuf_expand);

  GFL_MSG_Delete(wk->mm_status);
  
	WORDSET_Delete(wk->wordset);
  GFL_FONT_Delete(wk->fontHandle);
  PRINTSYS_QUE_Delete(wk->printQue);
}

//--------------------------------------------------------------
/**
 * BGロード
 *
 * @param   view		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _BeaconView_BGLoad( BEACON_VIEW_PTR wk, ARCHANDLE *handle )
{
	static const GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
  	{//GFL_BG_FRAME1_S
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
  		1, 0, 0, FALSE
  	},
  	{//GFL_BG_FRAME2_S
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
  		2, 0, 0, FALSE
  	},
  	{//GFL_BG_FRAME3_S
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
  		3, 0, 0, FALSE
  	},
	};
	GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &TextBgCntDat[2], GFL_BG_MODE_TEXT );

	//BG VRAMクリア
	GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);

	//レジスタOFF
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);
	G2S_BlendNone();
  
  //パレット転送
  GFL_ARCHDL_UTIL_TransVramPalette(handle, NARC_beacon_status_bstatus_bg_nclr, 
    PALTYPE_SUB_BG, 0, 0x20 * 4, wk->tmpHeapID);
  //フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
    0x20*FONT_PAL, 0x20, wk->tmpHeapID);
	
  //キャラ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    handle, NARC_beacon_status_bstatus_bg_lz_ncgr, GFL_BG_FRAME3_S, 0, 
    0, TRUE, wk->tmpHeapID);
  
  //スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_beacon_status_bstatus_bg01_lz_nscr, GFL_BG_FRAME1_S, 0, 
    0x800, TRUE, wk->tmpHeapID);
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_beacon_status_bstatus_bg02_lz_nscr, GFL_BG_FRAME2_S, 0, 
    0x800, TRUE, wk->tmpHeapID);
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_beacon_status_bstatus_bg03_lz_nscr, GFL_BG_FRAME3_S, 0, 
    0x800, TRUE, wk->tmpHeapID);

	GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
}

//--------------------------------------------------------------
/**
 * BGアンロード
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _BeaconView_BGUnload(BEACON_VIEW_PTR wk )
{
	GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
	GFL_BG_FreeBGControl(GFL_BG_FRAME1_S);
	GFL_BG_FreeBGControl(GFL_BG_FRAME2_S);
	GFL_BG_FreeBGControl(GFL_BG_FRAME3_S);
}

//--------------------------------------------------------------
/**
 * アクターで使用するリソースのロード
 *
 * @param   view		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _BeaconView_ActorResourceLoad( BEACON_VIEW_PTR wk, ARCHANDLE *handle)
{
  int i;
  ARCHANDLE* handle_union = GFL_ARC_OpenDataHandle( ARCID_WIFIUNIONCHAR, wk->tmpHeapID );

  {
    const OBJ_RES_SRC srcNormal[OBJ_RES_MAX] = {
      { 1, NARC_beacon_status_bstatus_obj_nclr },
      { 1, NARC_beacon_status_bstatus_obj_ncgr },
      { 1, NARC_beacon_status_bstatus_obj_ncer },
    };
    obj_ObjResInit( wk, &wk->objResNormal, srcNormal, handle );
  }
  {
    const OBJ_RES_SRC srcIcon[OBJ_RES_MAX] = {
      { 0, 0 },
      { PANEL_MAX, NARC_beacon_status_bstatus_icon01_ncgr },
      { 1, NARC_beacon_status_bstatus_icon01_ncer },
    };
    obj_ObjResInit( wk, &wk->objResIcon, srcIcon, handle );
  }
  
  {
    const OBJ_RES_SRC srcUnion[OBJ_RES_MAX] = {
     { 0, 0 },
     { PANEL_MAX, NARC_wifi_unionobj_front00_NCGR },
     { 1,NARC_wifi_unionobj_front00_NCER },
    };
    obj_ObjResInit( wk, &wk->objResUnion, srcUnion, handle_union );
  }

  //再転送用のリソースをロードしておく
  wk->resPlttUnion.buf = GFL_ARC_LoadDataAllocByHandle( handle_union,
                          NARC_wifi_unionobj_wf_match_top_trainer_NCLR,
                          wk->tmpHeapID );
  NNS_G2dGetUnpackedPaletteData( wk->resPlttUnion.buf, &wk->resPlttUnion.p_pltt );
  wk->resPlttUnion.dat = (u16*)wk->resPlttUnion.p_pltt->pRawData;

  for(i = 0;i < UNION_CHAR_MAX;i++){
    wk->resCharUnion[i].buf = GFL_ARC_LoadDataAllocByHandle( handle_union,
                              NARC_wifi_unionobj_front00_NCGR+i,
                              wk->tmpHeapID );
    NNS_G2dGetUnpackedCharacterData( wk->resCharUnion[i].buf, &wk->resCharUnion[i].p_char );
  }

  for(i = 0;i < ICON_MAX;i++){
    wk->resCharIcon[i].buf = GFL_ARC_LoadDataAllocByHandle( handle,
                          NARC_beacon_status_bstatus_icon01_ncgr+i,
                          wk->tmpHeapID );
    NNS_G2dGetUnpackedCharacterData( wk->resCharIcon[i].buf, &wk->resCharIcon[i].p_char );
  }
  GFL_ARC_CloseDataHandle( handle_union );
}

//--------------------------------------------------------------
/**
 * アクターで使用するアクターのアンロード
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _BeaconView_ActorResourceUnload( BEACON_VIEW_PTR wk )
{
  int i;

  for( i = 0;i < ICON_MAX;i++){
    GFL_HEAP_FreeMemory( wk->resCharIcon[i].buf );
  }
  for( i = 0;i < UNION_CHAR_MAX;i++){
    GFL_HEAP_FreeMemory( wk->resCharUnion[i].buf );
  }
  GFL_HEAP_FreeMemory( wk->resPlttUnion.buf );

  obj_ObjResRelease( wk, &wk->objResUnion );
  obj_ObjResRelease( wk, &wk->objResIcon );
  obj_ObjResRelease( wk, &wk->objResNormal );
}

//--------------------------------------------------------------
/**
 * アクター作成
 *
 * @param   wk		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _BeaconView_ActorCreate( BEACON_VIEW_PTR wk, ARCHANDLE *handle )
{
  int i;

  //セル系システムの作成
  wk->cellUnit = GFL_CLACT_UNIT_Create( BEACON_VIEW_OBJ_MAX , 0 , wk->heapID );

  for(i = 0; i < PANEL_MAX;i++){
    panel_PanelObjAdd( wk, i );
  }

  GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * アクター削除
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _BeaconView_ActorDelete( BEACON_VIEW_PTR wk )
{
  int i;

  GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_OFF);
  
  for(i = 0; i < PANEL_MAX;i++){
    panel_PanelObjDel( wk, i );
  }
  GFL_CLACT_UNIT_Delete( wk->cellUnit );
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _BeaconView_BmpWinCreate(BEACON_VIEW_PTR view)
{
  int i;
  
  for(i = 0; i < VIEW_LOG_MAX; i++){
    view->win[i] = GFL_BMPWIN_Create( GFL_BG_FRAME2_S, 2, 2 + 5*i, 28, 4, 
      FONT_PAL, GFL_BMP_CHRAREA_GET_B );
    PRINT_UTIL_Setup( &view->print_util[i], view->win[i] );
    GFL_BMPWIN_MakeTransWindow(view->win[i]);
  }
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _BeaconView_BmpWinDelete(BEACON_VIEW_PTR wk)
{
  int i;
  
  for(i = 0; i < VIEW_LOG_MAX; i++){
    GFL_BMPWIN_Delete( wk->win[i] );
  }
}


//--------------------------------------------------------------
/**
 * アクターリソース取得
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void obj_ObjResInit( BEACON_VIEW_PTR wk, OBJ_RES_TBL* res, const OBJ_RES_SRC* srcTbl, ARCHANDLE* p_handle )
{
  int i;

  MI_CpuClear8( res, sizeof(OBJ_RES_TBL));

  for(i = 0;i < OBJ_RES_MAX;i++){
    res->res[i].num = srcTbl[i].num;
    if(res->res[i].num != 0){
      res->res[i].tbl = GFL_HEAP_AllocClearMemory( wk->heapID, sizeof(u32)* res->res[i].num );
    }
  }

  for(i = 0;i < res->res[OBJ_RES_PLTT].num;i++){
    IWASAWA_Printf( "pal res idx = %d, src = %d\n",i,srcTbl[OBJ_RES_PLTT].srcID+i);
    res->res[OBJ_RES_PLTT].tbl[i] = 
      GFL_CLGRP_PLTT_Register(  p_handle, srcTbl[OBJ_RES_PLTT].srcID+i,
                                CLSYS_DRAW_SUB, 0, wk->heapID );
  }
  for(i = 0;i < res->res[OBJ_RES_CGR].num;i++){
    IWASAWA_Printf( "char res idx = %d, src = %d\n",i,srcTbl[OBJ_RES_CGR].srcID+i);
    res->res[OBJ_RES_CGR].tbl[i] = 
      GFL_CLGRP_CGR_Register( p_handle, srcTbl[OBJ_RES_CGR].srcID+i,
                              FALSE, CLSYS_DRAW_SUB, wk->heapID );
  }
  for(i = 0;i < res->res[OBJ_RES_CELLANIM].num;i++){
    IWASAWA_Printf( "cell res idx = %d, src = %d\n",i,srcTbl[OBJ_RES_CELLANIM].srcID+(i*2));
    res->res[OBJ_RES_CELLANIM].tbl[i] = 
      GFL_CLGRP_CELLANIM_Register( p_handle, 
          srcTbl[OBJ_RES_CELLANIM].srcID+(i*2),
          srcTbl[OBJ_RES_CELLANIM].srcID+1+(i*2),
          wk->heapID );
  }
}

//--------------------------------------------------------------
/**
 * アクターリソース解放
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void obj_ObjResRelease( BEACON_VIEW_PTR wk, OBJ_RES_TBL* res )
{
  int i;

  for(i = 0;i < res->res[OBJ_RES_CELLANIM].num;i++){
    GFL_CLGRP_CELLANIM_Release( res->res[OBJ_RES_CELLANIM].tbl[i] );
  }
  for(i = 0;i < res->res[OBJ_RES_CGR].num;i++){
    GFL_CLGRP_CGR_Release( res->res[OBJ_RES_CGR].tbl[i] );
  }
  for(i = 0;i < res->res[OBJ_RES_PLTT].num;i++){
    GFL_CLGRP_PLTT_Release( res->res[OBJ_RES_PLTT].tbl[i] );
  }
  for(i = 0;i < OBJ_RES_MAX;i++){
    if(res->res[i].tbl != NULL){
      GFL_HEAP_FreeMemory( res->res[i].tbl );
    }
  }
}

/*
 *  @brief  アクター追加
 */
static GFL_CLWK* obj_ObjAdd(
  BEACON_VIEW_PTR wk, u32 cgrNo, u32 palNo, u32 cellNo,
  s16 x, s16 y, u8 anm, u8 bg_pri, u8 spri)
{
  GFL_CLWK_DATA ini;
  GFL_CLWK* obj;

  //セルの生成
  ini.pos_x = x;
  ini.pos_y = y;
  ini.anmseq = anm;
  ini.bgpri = bg_pri;
  ini.softpri = spri;
  
  obj = GFL_CLACT_WK_Create( wk->cellUnit,
          cgrNo,palNo,cellNo,
          &ini, CLSYS_DEFREND_SUB, wk->heapID );

  return obj;
}

/**
 *  @brief  パネル生成
 */
static void panel_PanelObjAdd( BEACON_VIEW_PTR wk, u8 idx )
{
  PANEL_WORK* pp = &(wk->panel[idx]);

  MI_CpuClear8( pp, sizeof(PANEL_WORK));

  pp->id = idx;

  pp->px = ACT_PANEL_OX*idx;
  pp->py = ACT_PANEL_OY*idx;

  //パネルアクター
  pp->cPanel = obj_ObjAdd( wk,
    wk->objResNormal.res[OBJ_RES_CGR].tbl[0],
    wk->objResNormal.res[OBJ_RES_PLTT].tbl[0],
    wk->objResNormal.res[OBJ_RES_CELLANIM].tbl[0],
    pp->px, pp->py, ACTANM_PANEL, OBJ_BG_PRI, OBJ_SPRI_PANEL+idx);
  GFL_CLACT_WK_SetPlttOffs( pp->cPanel, idx, CLWK_PLTTOFFS_MODE_OAM_COLOR );

  //Unionオブジェアクター
  pp->cUnion = obj_ObjAdd( wk,
    wk->objResUnion.res[OBJ_RES_CGR].tbl[0],
    wk->objResNormal.res[OBJ_RES_PLTT].tbl[0],
    wk->objResUnion.res[OBJ_RES_CELLANIM].tbl[0],
    pp->px+ACT_UNION_OX, pp->py+ACT_UNION_OY, 0, OBJ_BG_PRI, OBJ_SPRI_UNION+idx);
  GFL_CLACT_WK_SetPlttOffs( pp->cUnion, idx + ACT_PAL_UNION , CLWK_PLTTOFFS_MODE_PLTT_TOP );

  //Iconオブジェアクター
  pp->cIcon = obj_ObjAdd( wk,
    wk->objResIcon.res[OBJ_RES_CGR].tbl[0],
    wk->objResNormal.res[OBJ_RES_PLTT].tbl[0],
    wk->objResIcon.res[OBJ_RES_CELLANIM].tbl[0],
    pp->px+ACT_ICON_OX, pp->py+ACT_ICON_OY, 0, OBJ_BG_PRI, OBJ_SPRI_ICON+idx);

  GFL_CLACT_WK_SetDrawEnable( pp->cPanel, TRUE );
  GFL_CLACT_WK_SetDrawEnable( pp->cUnion, TRUE );
  GFL_CLACT_WK_SetDrawEnable( pp->cIcon, TRUE );
}

/**
 *  @brief  パネル削除
 */
static void panel_PanelObjDel( BEACON_VIEW_PTR wk, u8 idx )
{
  PANEL_WORK* pp = &(wk->panel[idx]);

  GFL_CLACT_WK_SetDrawEnable( pp->cIcon, FALSE );
  GFL_CLACT_WK_Remove( pp->cIcon );
  GFL_CLACT_WK_SetDrawEnable( pp->cUnion, FALSE );
  GFL_CLACT_WK_Remove( pp->cUnion );
  GFL_CLACT_WK_SetDrawEnable( pp->cPanel, FALSE );
  GFL_CLACT_WK_Remove( pp->cPanel );
 
  MI_CpuClear8( pp, sizeof(PANEL_WORK));
}

