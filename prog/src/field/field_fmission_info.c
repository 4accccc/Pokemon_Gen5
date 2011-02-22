//===============================================================
/*
 *  @file   field_fmission_info.c
 *  @brief  フェスミッション　フィールド情報表示システム
 *  @author Miyuki Iwasawa
 *  @date   11.02.08
 */
//===============================================================

#include <gflib.h>
#include <tcbl.h>
#include "system/main.h"

#include "system/bmp_oam.h"
#include "print/wordset.h"
#include "field/fieldmap.h"
#include "field/zonedata.h"
#include "field/field_status.h"
#include "savedata/situation.h"
#include "savedata/save_control.h"

#include "buflen.h"
#include "field_oam_pal.h"  // for FLDOAM_PALNO_PLACENAME

#include "gamesystem/link_festival.h"

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
 ACT_SCORE_KIRA,
 ACT_SCORE_UP,
 ACT_TIME_P, //タイムプレート
 ACT_TIMEN_C,
 ACT_TIMEN01,
 ACT_TIMEN02,
 ACT_TIMEN03,
 ACT_TIMEN04,
 ACT_TIME_KIRA,
 ACT_TIME_BONUS,
 ACT_MAX,
}ACT_ID;
 
#define ACT_SCOREN_PLACE_NUM (3)
#define ACT_TIMEN_PLACE_NUM (2)
#define ACT_TIMEN_NUM (ACT_TIMEN_PLACE_NUM*2)

#define ACT_SCORE_START (ACT_MS_P)
#define ACT_SCORE_NUM   (ACT_SCORE_UP+1)
#define ACT_TIME_START  (ACT_TIME_P)
#define ACT_TIME_NUM    (ACT_TIME_BONUS-ACT_SCORE_UP)

typedef enum{
 ACTANM_SNUM,
 ACTANM_LNUM,
 ACTANM_LNUM_C,
 ACTANM_MSCORE,
 ACTANM_TSCORE,
 ACTANM_TIME,
 ACTANM_SCORE_UP,
 ACTANM_TIME_BONUS,
 ACTANM_KIRA,
 ACTANM_MAX,
}ACT_ANMID;

#define ACT_RENDER_ID (0)

#define SCORE_UP_VIEW_TIME  (60)
#define TIME_BONUS_VIEW_TIME  (60)

#include "field_fmission_info_def.h"

//===================================================================================
// ■フェスミッション情報表示システム
//===================================================================================
struct _FIELD_FMISSION_INFO{
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldmap;
  ARCHANDLE* arc_h;
  HEAPID  heapID;
  LINK_FESTIVAL*  fest_sys;
  const FEST_MISSION* fmwk; 

  //
  u8              visible_f:1; //パネルが隠れているか？
  u8              t_scoreup_f:1;  //スコアアップタスクが生きているか？
  u8              t_tbonus_f:1;   //タイムボーナスタスクが生きているか？
  u8              score_up_ct;
  u8              time_bonus_ct;
  int             old_sec;

	// OBJ
	BMPOAM_SYS_PTR  bmpOamSys;               // BMPOAMシステム
  u32             resChar;
  u32             resCell;
	u32             resPltt; // パレットリソース
	GFL_CLUNIT*     cellUnit;    ///< セルアクターユニット
  GFL_CLSYS_REND* cellRender;  ///< レンダラーシステム
  GFL_CLWK*       pAct[ACT_MAX];  ///< アクター
  GFL_CLACTPOS    actPos[ACT_MAX]; ///<初期アクター位置保存
  
  GFL_TCBLSYS*    tcbsys;
};

typedef struct _TASKWK_SCROLL{
  FIELD_FMISSION_INFO* sys_wk;
  u8  act_idx;
  u8  act_num;

  int   frame;
  fx32  ay;
}TASKWK_SCROLL;

typedef struct _TASKWK_SCOREUP{
  FIELD_FMISSION_INFO* sys_wk;
}TASKWK_SCOREUP;

typedef struct _TASKWK_TBONUS{
  FIELD_FMISSION_INFO* sys_wk;
}TASKWK_TBONUS;

static void _sub_ActorSysCreate( FIELD_FMISSION_INFO* wk, ARCHANDLE *handle );
static void _sub_ActorSysDelete( FIELD_FMISSION_INFO* wk );
static GFL_CLWK* _sub_ObjAdd( FIELD_FMISSION_INFO* wk, s16 x,s16 y,u8 anm, u8 spri );
static void _sub_ActorCreate( FIELD_FMISSION_INFO* wk );
static void _sub_ActorDelete( FIELD_FMISSION_INFO* wk );

static int get_num_keta(u32 num,u8 keta);
static void panel_Hide( FIELD_FMISSION_INFO* wk );
static void panel_TimeUpdate( FIELD_FMISSION_INFO* wk, BOOL force_f );
static void panel_ScoreUpdate( FIELD_FMISSION_INFO* wk, int score, u8 act_idx );
static void panel_ScrollInit( FIELD_FMISSION_INFO* wk );
static void taskAdd_PanelScroll( FIELD_FMISSION_INFO* wk, u8 act_idx, u8 act_num, s16 ofs );
static void taskAddCheck_ScoreUp( FIELD_FMISSION_INFO* wk );
static void taskAddCheck_TimeBonus( FIELD_FMISSION_INFO* wk );

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
  wk->fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  wk->fest_sys = GAMESYSTEM_GetLinkFestivalWork( gsys );
  wk->fmwk = (const FEST_MISSION*)LINK_FESTIVAL_GetMissionWork( wk->fest_sys );

  wk->old_sec = 0xFFFF; //無効値で初期化

  _sub_ActorSysCreate( wk, wk->arc_h );
  _sub_ActorCreate( wk );
  
  // TCB初期化
  wk->tcbsys = GFL_TCBL_Init( wk->heapID, wk->heapID, 16, 128 );

  GFL_ARC_CloseDataHandle( wk->arc_h );
  wk->arc_h = NULL;
 
  //最初は隠す
  panel_Hide( wk );

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
  //今生きている全てのタスクを削除
  GFL_TCBL_DeleteAll( wk->tcbsys );
  // TCB削除
  GFL_TCBL_Exit( wk->tcbsys );

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
  BOOL event_f;
  
  if( !FEST_MISSION_IsEntry( wk->fmwk )){
    return;
  }

  event_f = FIELDMAP_CheckDoEvent(wk->fieldmap);

  if( wk->visible_f ){
    if( event_f ){
      panel_Hide( wk );
    }else{
      taskAddCheck_ScoreUp( wk );
      taskAddCheck_TimeBonus( wk );
    }
  }else if( !wk->visible_f && !event_f ){
    panel_ScrollInit( wk );
  }
  GFL_TCBL_Main( wk->tcbsys );
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
  if( !FEST_MISSION_IsEntry( wk->fmwk ) || !wk->visible_f){
    return;
  }
  panel_TimeUpdate( wk, FALSE );
  panel_ScoreUpdate( wk,  LINK_FESTIVAL_FInfoScoreGet( wk->fest_sys ), ACT_TSN01 );
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
  GFL_CLACT_WK_SetDrawEnable( obj, FALSE );

  return obj;
}

static void _sub_ActorCreate( FIELD_FMISSION_INFO* wk )
{
  int i;

  //プレート追加
  wk->pAct[ACT_MS_P] = _sub_ObjAdd( wk, AP_MSP_X, AP_MSP_Y, ACTANM_MSCORE, 2 );
  wk->pAct[ACT_TS_P] = _sub_ObjAdd( wk, AP_TSP_X, AP_TSP_Y, ACTANM_TSCORE, 2 );
  wk->pAct[ACT_TIME_P] = _sub_ObjAdd( wk, AP_TIMEP_X, AP_TIMEP_Y, ACTANM_TIME, 2 );

  //スコアナンバー追加
  for( i = 0;i < ACT_SCOREN_PLACE_NUM;i++){
    wk->pAct[ACT_MSN01+i] = _sub_ObjAdd( wk, AP_MSN_X+(AP_MSN_OX*i), AP_MSN_Y, ACTANM_SNUM, 1 );
    wk->pAct[ACT_TSN01+i] = _sub_ObjAdd( wk, AP_TSN_X+(AP_TSN_OX*i), AP_TSN_Y, ACTANM_LNUM, 1 );
  }

  //タイムナンバー追加
  for( i = 0;i < ACT_TIMEN_PLACE_NUM;i++){
    wk->pAct[ACT_TIMEN01+i] = _sub_ObjAdd( wk, AP_TIMEN_X+(AP_TIMEN_OX*i), AP_TIMEN_Y, ACTANM_LNUM, 1 );
    wk->pAct[ACT_TIMEN03+i] = _sub_ObjAdd( wk, AP_TIMEN_X+(AP_TIMEN_OX*(i+ACT_TIMEN_PLACE_NUM))+AP_TIMENC_OX, AP_TIMEN_Y, ACTANM_LNUM, 1 );
  }
  //タイム区切り文字追加
  wk->pAct[ACT_TIMEN_C] = _sub_ObjAdd( wk, AP_TIMENC_X, AP_TIMENC_Y, ACTANM_LNUM_C, 1 );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->pAct[ACT_TIMEN_C], TRUE );

  //キラパネル
  wk->pAct[ACT_SCORE_KIRA] = _sub_ObjAdd( wk, AP_TSP_KIRA_X, AP_TSP_KIRA_Y, ACTANM_KIRA, 0);
  wk->pAct[ACT_TIME_KIRA] = _sub_ObjAdd( wk, AP_TIME_KIRA_X, AP_TIME_KIRA_Y, ACTANM_KIRA, 0);
  GFL_CLACT_WK_SetAutoAnmFlag( wk->pAct[ACT_SCORE_KIRA], TRUE );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->pAct[ACT_TIME_KIRA], TRUE );

  //ボーナスパネル
  wk->pAct[ACT_SCORE_UP] = _sub_ObjAdd( wk, AP_SCORE_UP_X, AP_SCORE_UP_Y, ACTANM_SCORE_UP, 2 );
  wk->pAct[ACT_TIME_BONUS] = _sub_ObjAdd( wk, AP_TIME_BONUS_X, AP_TIME_BONUS_Y, ACTANM_TIME_BONUS, 2 );

  for(i = 0;i < ACT_MAX;i++){
    GFL_CLACT_WK_GetPos( wk->pAct[i], &wk->actPos[i], ACT_RENDER_ID );
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
 *	@brief	パネル全体表示制御
 */
//--------------------------------------------------------------
static void panel_VisibleSet( FIELD_FMISSION_INFO* wk, BOOL visible_f )
{
  int i;
  for(i = 0;i < ACT_MAX;i++){
    GFL_CLACT_WK_SetDrawEnable( wk->pAct[i], visible_f );
  }
}

//--------------------------------------------------------------
/**
 *	@brief	パネルを隠す
 */
//--------------------------------------------------------------
static void panel_Hide( FIELD_FMISSION_INFO* wk )
{
  panel_VisibleSet( wk, FALSE );
  wk->visible_f = FALSE;
  wk->score_up_ct = 0;  //0リセット
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

//--------------------------------------------------------------
/**
 *	@brief	アクター数字描画
 */
//--------------------------------------------------------------
static void panel_NumberDraw( FIELD_FMISSION_INFO* wk, int num, int keta, u8 act_idx, BOOL full_f )
{
  int i,idx = act_idx+(keta-1);

  for(i = 0;i < keta;i++){
    int value = get_num_keta( num, i );
    if( i > 0 && !full_f){
      GFL_CLACT_WK_SetDrawEnable( wk->pAct[idx], (value!=0) );
    }
    GFL_CLACT_WK_SetAnmIndex( wk->pAct[idx], value );
//    GFL_CLACT_WK_SetAnmFrame( wk->pAct[idx], FX32_CONST(value));
    --idx;
  }
}

//--------------------------------------------------------------
/**
 *	@brief	時間パネル更新
 */
//--------------------------------------------------------------
static void panel_TimeUpdate( FIELD_FMISSION_INFO* wk, BOOL force_f )
{
  int sec = wk->fmwk->life/60;
  int min = sec/60;

  if( sec == wk->old_sec && !force_f ){
    return;
  }
  panel_NumberDraw( wk, min%60, ACT_TIMEN_PLACE_NUM, ACT_TIMEN01, TRUE );
  panel_NumberDraw( wk, sec%60, ACT_TIMEN_PLACE_NUM, ACT_TIMEN03, TRUE );
  wk->old_sec = sec; 
}

//--------------------------------------------------------------
/**
 *	@brief	スコアパネル描画更新
 */
//--------------------------------------------------------------
static void panel_ScoreUpdate( FIELD_FMISSION_INFO* wk, int score, u8 act_idx )
{
  panel_NumberDraw( wk, score, ACT_SCOREN_PLACE_NUM, act_idx, FALSE );
}

//--------------------------------------------------------------
/**
 *	@brief	スクロールイン初期化
 */
//--------------------------------------------------------------
static void panel_ScrollInit( FIELD_FMISSION_INFO* wk )
{
  //初期位置セット
  taskAdd_PanelScroll( wk, ACT_SCORE_START, ACT_SCORE_NUM, AP_MSP_SY+AP_TSP_SY);
  taskAdd_PanelScroll( wk, ACT_TIME_START, ACT_TIME_NUM, -AP_TIMEP_SY);
  
  taskAddCheck_ScoreUp( wk );
  taskAddCheck_TimeBonus( wk );

  //描画状態セット
  panel_TimeUpdate( wk, TRUE );
  panel_ScoreUpdate( wk, wk->fmwk->target, ACT_MSN01 );
  panel_ScoreUpdate( wk, LINK_FESTIVAL_FInfoScoreGet( wk->fest_sys ), ACT_TSN01 );

  wk->visible_f = TRUE;
}

//--------------------------------------------------------------
/**
 *	@brief	スクロールタスク追加
 */
//--------------------------------------------------------------
static void tcb_PanelScroll( GFL_TCBL *tcb , void* tcb_wk);

static void taskAdd_PanelScroll( FIELD_FMISSION_INFO* wk, u8 act_idx, u8 act_num, s16 ofs )
{
  int i;
  GFL_TCBL* tcb;
  TASKWK_SCROLL* twk;

  tcb = GFL_TCBL_Create( wk->tcbsys, tcb_PanelScroll, sizeof(TASKWK_SCROLL), 0 );
  twk = GFL_TCBL_GetWork(tcb);

  MI_CpuClear8( twk, sizeof( TASKWK_SCROLL ));
  twk->sys_wk = wk;
  twk->act_idx = act_idx;
  twk->act_num = act_num;
  twk->frame = 8;

  //初期位置セット
  for(i = 0;i < act_num;i++){
   	GFL_CLACTPOS pos;
    int idx = act_idx+i;

    pos.x = wk->actPos[idx].x;
    pos.y = wk->actPos[idx].y+ofs;
    GFL_CLACT_WK_SetPos( wk->pAct[idx], &pos, ACT_RENDER_ID );
    if( i < (act_num-2)){
      GFL_CLACT_WK_SetDrawEnable( wk->pAct[idx], TRUE );
    }
  }
  twk->ay = FX_Div(FX32_CONST(ofs),FX32_CONST(twk->frame));
}

static void tcb_PanelScroll( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_SCROLL* twk = (TASKWK_SCROLL*)tcb_wk;
  FIELD_FMISSION_INFO* wk = twk->sys_wk;
  int i,idx;
  GFL_CLACTPOS pos;

  if( !wk->visible_f ){
    GFL_TCBL_Delete(tcb);
    return;
  }

  if( twk->frame-- > 0){

    for(i = twk->act_idx;i < twk->act_idx+twk->act_num;i++){
      pos.x = wk->actPos[i].x; 
      pos.y = wk->actPos[i].y+FX_Whole(twk->ay*twk->frame);
      GFL_CLACT_WK_SetPos( wk->pAct[i], &pos, ACT_RENDER_ID );
    }
    return;
  }
  //最終座標
  for(i = twk->act_idx;i < twk->act_idx+twk->act_num;i++){
    GFL_CLACT_WK_SetPos( wk->pAct[i], &wk->actPos[i], ACT_RENDER_ID );
  }
  GFL_TCBL_Delete(tcb);
}

//--------------------------------------------------------------
/**
 *	@brief	スコアアップタスク追加
 */
//--------------------------------------------------------------
static void tcb_ScoreUp( GFL_TCBL *tcb , void* tcb_wk);

static void taskAddCheck_ScoreUp( FIELD_FMISSION_INFO* wk )
{
  GFL_TCBL* tcb;
  TASKWK_SCOREUP* twk;
 
  if( LINK_FESTIVAL_FInfoScoreGet( wk->fest_sys ) >= wk->fmwk->my_score ){
    return; //更新の必要無し
  }
  wk->score_up_ct = SCORE_UP_VIEW_TIME;
  LINK_FESTIVAL_FInfoScoreInc( wk->fest_sys );

  if( wk->t_scoreup_f ){
    return; //すでにタスクがあるので、時間更新だけ
  }
  wk->t_scoreup_f = TRUE;

  tcb = GFL_TCBL_Create( wk->tcbsys, tcb_ScoreUp, sizeof(TASKWK_SCOREUP), 0 );
  twk = GFL_TCBL_GetWork(tcb);

  MI_CpuClear8( twk, sizeof( TASKWK_SCOREUP ));
  twk->sys_wk = wk;
  
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_SCORE_UP], TRUE );
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_SCORE_KIRA], TRUE );
}

static void tcb_ScoreUp( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_SCOREUP* twk = (TASKWK_SCOREUP*)tcb_wk;
  FIELD_FMISSION_INFO* wk = twk->sys_wk;

  if( wk->score_up_ct ){
    --wk->score_up_ct;
    return;
  }
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_SCORE_UP], FALSE );
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_SCORE_KIRA], FALSE );

  wk->t_scoreup_f = FALSE;
  GFL_TCBL_Delete(tcb);
}


//--------------------------------------------------------------
/**
 *	@brief	タイムボーナス表示タスク追加
 */
//--------------------------------------------------------------
static void tcb_TimeBonus( GFL_TCBL *tcb , void* tcb_wk);

static void taskAddCheck_TimeBonus( FIELD_FMISSION_INFO* wk )
{
  GFL_TCBL* tcb;
  TASKWK_TBONUS* twk;
  u16 bonus = LINK_FESTIVAL_FInfoTimeBonusGet( wk->fest_sys );

  if( !bonus ){
    return; //更新の必要無し
  }
  wk->time_bonus_ct = TIME_BONUS_VIEW_TIME;
  LINK_FESTIVAL_FInfoTimeBonusReset( wk->fest_sys );

  if( wk->t_tbonus_f ){
    return; //すでにタスクがあるので、時間更新だけ
  }
  wk->t_tbonus_f = TRUE;

  tcb = GFL_TCBL_Create( wk->tcbsys, tcb_TimeBonus, sizeof(TASKWK_TBONUS), 0 );
  twk = GFL_TCBL_GetWork(tcb);

  MI_CpuClear8( twk, sizeof( TASKWK_TBONUS ));
  twk->sys_wk = wk;
  
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_TIME_BONUS], TRUE );
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_TIME_KIRA], TRUE );
}

static void tcb_TimeBonus( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_TBONUS* twk = (TASKWK_TBONUS*)tcb_wk;
  FIELD_FMISSION_INFO* wk = twk->sys_wk;

  if( wk->time_bonus_ct ){
    --wk->time_bonus_ct;
    return;
  }
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_TIME_BONUS], FALSE );
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_TIME_KIRA], FALSE );

  wk->t_tbonus_f = FALSE;
  GFL_TCBL_Delete(tcb);
}


