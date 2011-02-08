//===============================================================
/*
 *  @file   field_fmission_info.c
 *  @brief  フェスミッション　フィールド情報表示システム
 *  @author Miyuki Iwasawa
 *  @date   11.02.08
 */
//===============================================================

#include <gflib.h>
#include "system/main.h"

#include "system/bmp_oam.h"
#include "print/wordset.h"
#include "field/zonedata.h"
#include "field/field_status.h"
#include "savedata/situation.h"
#include "savedata/save_control.h"

#include "buflen.h"
#include "field_oam_pal.h"  // for FLDOAM_PALNO_PLACENAME

#include "arc/arc_def.h"
#include "arc/fmission_gra.naix"
#include "arc/message.naix"
#include "msg/msg_place_name.h"

#include "../../../resource/fldmapdata/zonetable/zone_id.h"

#include "field_fmission_info.h"

#ifdef PM_DEBUG
#include "debug/debug_flg.h"
#endif 


typedef enum{
 ACT_MS_P,  //マイスコアプレート
 ACT_MSN01,
 ACT_MSN02,
 ACT_MSN03,
 ACT_TS_P,  //トータルスコアプレート
 ACT_TSN01,
 ACT_TSN02,
 ACT_TSN03,
 ACT_TIME_P, //タイムプレート
 ACT_TIMEN_C,
 ACT_TIMEN01,
 ACT_TIMEN02,
 ACT_TIMEN03,
 ACT_TIMEN04,
 ACT_MAX,
}ACT_ID;
 
#define ACT_SCOREN_NUM (3)
#define ACT_TIMEN_PLACE_NUM (2)
#define ACT_TIMEN_NUM (ACT_TIMEN_PLACE_NUM*2)

typedef enum{
 ACTANM_SNUM,
 ACTANM_LNUM,
 ACTANM_LNUM_C,
 ACTANM_MSCORE,
 ACTANM_TSCORE,
 ACTANM_TIME,
 ACTANM_MAX,
}ACT_ANMID;

#define ACT_RENDER_ID (0)

#include "field_fmission_info_def.h"

//===================================================================================
// ■フェスミッション情報表示システム
//===================================================================================
struct _FIELD_FMISSION_INFO{
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  ARCHANDLE* arc_h;
  HEAPID  heapID;

	// OBJ
	BMPOAM_SYS_PTR  bmpOamSys;               // BMPOAMシステム
  u32             resChar;
  u32             resCell;
	u32             resPltt; // パレットリソース
	GFL_CLUNIT*     cellUnit;    ///< セルアクターユニット
  GFL_CLSYS_REND* cellRender;  ///< レンダラーシステム
  GFL_CLWK*       pAct[ACT_MAX];  ///< アクター
};


static void _sub_ActorSysCreate( FIELD_FMISSION_INFO* wk, ARCHANDLE *handle );
static void _sub_ActorSysDelete( FIELD_FMISSION_INFO* wk );
static GFL_CLWK* _sub_ObjAdd( FIELD_FMISSION_INFO* wk, s16 x,s16 y,u8 anm, u8 spri );
static void _sub_ActorCreate( FIELD_FMISSION_INFO* wk );
static void _sub_ActorDelete( FIELD_FMISSION_INFO* wk );

static int get_num_keta(u32 num,u8 keta);

//====================================================================================
// ■作成・破棄
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief フェスミッション情報表示システムを作成する
 *
 * @param gameSystem
 * @param heapID     使用するヒープID
 *
 * @return 生成したシステム
 */
//------------------------------------------------------------------------------------
FIELD_FMISSION_INFO* FIELD_FMISSION_INFO_Create( GAMESYS_WORK* gsys, HEAPID heapID )
{
  FIELD_FMISSION_INFO* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_FMISSION_INFO));
  wk->arc_h = GFL_ARC_OpenDataHandle( ARCID_FMISSION_GRA, heapID );
  wk->heapID = heapID;

  _sub_ActorSysCreate( wk, wk->arc_h );
  _sub_ActorCreate( wk );

  GFL_ARC_CloseDataHandle( wk->arc_h );
  wk->arc_h = NULL;

  return wk;
}

//------------------------------------------------------------------------------------
/**
 * @brief フェスミッション情報表示システムを破棄する
 *
 * @param system
 */
//------------------------------------------------------------------------------------
void FIELD_FMISSION_INFO_Delete( FIELD_FMISSION_INFO* wk )
{
  _sub_ActorDelete( wk );
  _sub_ActorSysDelete( wk );
  
  GFL_HEAP_FreeMemory(wk);
  wk = NULL;
}

//====================================================================================
// ■動作
//====================================================================================
//------------------------------------------------------------------------------------
/**
 * @brief フェスミッション情報表示システムの動作処理
 *
 * @param system
 */
//------------------------------------------------------------------------------------
void FIELD_FMISSION_INFO_Process( FIELD_FMISSION_INFO* wk )
{
}

//------------------------------------------------------------------------------------
/**
 * @brief フェスミッション情報表示システムの描画処理
 *
 * @param system
 */
//------------------------------------------------------------------------------------
void FIELD_FMISSION_INFO_Draw( FIELD_FMISSION_INFO* wk )
{
}



//====================================================================================
// ■サブルーチン
//====================================================================================
//--------------------------------------------------------------
/**
 * アクターシステム作成
 *
 * @param   wk		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _sub_ActorSysCreate( FIELD_FMISSION_INFO* wk, ARCHANDLE *handle )
{
  int i;

  //セル系システムの作成
  wk->cellUnit = GFL_CLACT_UNIT_Create( ACT_MAX , 0 , wk->heapID );
 
  //レンダラー作成
  {
    const GFL_REND_SURFACE_INIT renderInitData = { 0,512,256,192,CLSYS_DRAW_MAIN, CLSYS_REND_CULLING_TYPE_NOT_AFFINE};
    
    wk->cellRender = GFL_CLACT_USERREND_Create( &renderInitData , 1 , wk->heapID );
    GFL_CLACT_UNIT_SetUserRend( wk->cellUnit, wk->cellRender );
  }
  //リソース読み込み
  {
    wk->resPltt = GFL_CLGRP_PLTT_RegisterEx(  handle, NARC_fmission_gra_field_info_NCLR, CLSYS_DRAW_MAIN,
                                FLDOAM_PALNO_FMISSION_INFO*0x20, 0, 1, wk->heapID );
    
    wk->resChar = GFL_CLGRP_CGR_Register( handle, NARC_fmission_gra_field_info_lz_NCGR,
                                TRUE, CLSYS_DRAW_MAIN, wk->heapID );
    
    wk->resCell = GFL_CLGRP_CELLANIM_Register( handle,
                                NARC_fmission_gra_field_info_NCER,
                                NARC_fmission_gra_field_info_NANR,
                                wk->heapID );
  }
  GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * アクターシステム削除
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_ActorSysDelete( FIELD_FMISSION_INFO* wk )
{
  int i;

  GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_OFF);
  
  GFL_CLGRP_CELLANIM_Release( wk->resCell );
  GFL_CLGRP_CGR_Release( wk->resChar );
  GFL_CLGRP_PLTT_Release( wk->resPltt );
 
  GFL_CLACT_USERREND_Delete( wk->cellRender );
  GFL_CLACT_UNIT_Delete( wk->cellUnit );
}

//--------------------------------------------------------------
/**
 * アクター作成
 *
 * @param   wk		
 * @param   handle		
 */
//--------------------------------------------------------------
static GFL_CLWK* _sub_ObjAdd( FIELD_FMISSION_INFO* wk, s16 x,s16 y,u8 anm, u8 spri )
{
  GFL_CLWK_DATA ini;
  GFL_CLWK* obj;

  ini.pos_x = x;
  ini.pos_y = y;
  ini.anmseq = anm;
  ini.bgpri = 3;
  ini.softpri = spri;
  
  obj = GFL_CLACT_WK_Create( wk->cellUnit, wk->resChar,wk->resPltt,wk->resCell,
          &ini, ACT_RENDER_ID, wk->heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( obj, FALSE );
  GFL_CLACT_WK_SetSRTAnimeFlag( obj, FALSE );

  return obj;
}

static void _sub_ActorCreate( FIELD_FMISSION_INFO* wk )
{
  int i;

  //プレート追加
  wk->pAct[ACT_MS_P] = _sub_ObjAdd( wk, AP_MSP_X, AP_MSP_Y, ACTANM_MSCORE, 1 );
  wk->pAct[ACT_TS_P] = _sub_ObjAdd( wk, AP_TSP_X, AP_TSP_Y, ACTANM_TSCORE, 1 );
  wk->pAct[ACT_TIME_P] = _sub_ObjAdd( wk, AP_TIMEP_X, AP_TIMEP_Y, ACTANM_TIME, 1 );

  //スコアナンバー追加
  for( i = 0;i < ACT_SCOREN_NUM;i++){
    wk->pAct[ACT_MSN01+i] = _sub_ObjAdd( wk, AP_MSN_X+(AP_MSN_OX*i), AP_MSN_Y, ACTANM_SNUM, 0 );
    wk->pAct[ACT_TSN01+i] = _sub_ObjAdd( wk, AP_TSN_X+(AP_TSN_OX*i), AP_TSN_Y, ACTANM_LNUM, 0 );
  }
  //タイムナンバー追加
  for( i = 0;i < ACT_TIMEN_PLACE_NUM;i++){
    wk->pAct[ACT_TIMEN01+i] = _sub_ObjAdd( wk, AP_TIMEN_X+(AP_TIMEN_OX*i), AP_TIMEN_Y, ACTANM_LNUM, 0 );
    wk->pAct[ACT_TIMEN03+i] = _sub_ObjAdd( wk, AP_TIMEN_X+(AP_TIMEN_OX*(i+ACT_TIMEN_PLACE_NUM))+AP_TIMENC_OX, AP_TIMEN_Y, ACTANM_LNUM, 0 );
  }
  //タイム区切り文字追加
  wk->pAct[ACT_TIMEN_C] = _sub_ObjAdd( wk, AP_TIMENC_X, AP_TIMENC_Y, ACTANM_LNUM_C, 0 );

  for(i = 0;i < ACT_MAX;i++){
    GFL_CLACT_WK_SetDrawEnable( wk->pAct[i], FALSE );
  }
}

//--------------------------------------------------------------
/**
 * アクター削除
 *
 * @param   wk		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _sub_ActorDelete( FIELD_FMISSION_INFO* wk )
{
  int i;
  
  for(i = 0;i < ACT_MAX;i++){
    GFL_CLACT_WK_SetDrawEnable( wk->pAct[i], FALSE );
    GFL_CLACT_WK_Remove( wk->pAct[i]);
  }
}

//--------------------------------------------------------------
/**
 *	@brief	数字の桁取得
 */
//--------------------------------------------------------------
static int get_num_keta(u32 num,u8 keta)
{
	const u32 tbl[] = {
	 1,10,100,1000,10000,100000,1000000,10000000
	};
	return (num%tbl[keta+1])/tbl[keta];
}

