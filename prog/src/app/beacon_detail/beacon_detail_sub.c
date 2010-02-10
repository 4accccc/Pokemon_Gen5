//=============================================================================
/**
 *
 *	@file		beacon_detail_sub.c
 *	@brief  すれ違い詳細画面 サブ
 *	@author	Miyuki Iwasawa
 *	@data		2010.02.01
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

// 簡易会話表示システム
#include "system/pms_draw.h"
#include "system/palanm.h"

//テクスチャをOAMに転送
#include "system/nsbtx_to_clwk.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "beacon_detail_gra.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//タッチバー
#include "ui/touchbar.h"

//ポケパラ、ポケモンNO
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

//アプリ共通素材
#include "app/app_menu_common.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "app/beacon_detail.h"

#include "message.naix"
#include "msg/msg_beacon_detail.h"  // GMM
#include "beacon_status.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "wifi_unionobj.naix"
#include "wmi.naix"

#include "beacon_detail_local.h"
#include "beacon_detail_sub.h"


#include "arc/wifi_unionobj_plt.cdat" //ユニオンキャラパレット参照テーブル

//////////////////////////////////////////////////////////////
//定数定義

typedef struct BEACON_WIN_COLOR{
 PRINTSYS_LSB color;
 u8           base;
}BEACON_WIN_COLOR;

static const BEACON_WIN_COLOR DATA_BeaconWinColor[] = {
  { FCOL_WIN01, FCOL_WIN_BASE1 }, 
  { FCOL_WIN02, FCOL_WIN_BASE2 }, 
};

#define FCOL_BEACON_BASE( n ) ( DATA_BeaconWinColor[n].base ) 
#define FCOL_BEACON_COL( n )  ( DATA_BeaconWinColor[n].color )

typedef struct _BEACON_DETAIL{
  u8  action;
  u8  msg_id;
  u8  icon;
}BEACON_DETAIL;

static const BEACON_DETAIL DATA_BeaconDetail[] = {
  { GAMEBEACON_ACTION_SEARCH, msg_beacon_000, 0 },
  { GAMEBEACON_ACTION_BATTLE_WILD_POKE_START, msg_beacon_001, 1 },
  { GAMEBEACON_ACTION_BATTLE_SP_POKE_START, msg_beacon_002, 1 },
  { GAMEBEACON_ACTION_BATTLE_TRAINER_START, msg_beacon_003, 2 },
  { GAMEBEACON_ACTION_BATTLE_LEADER_START, msg_beacon_004, 2 },
  { GAMEBEACON_ACTION_BATTLE_BIGFOUR_START, msg_beacon_005, 2 },
  { GAMEBEACON_ACTION_BATTLE_CHAMPION_START, msg_beacon_006, 2 },
};

#define BEACON_DETAIL_PARAM_NUM (NELEMS(DATA_BeaconDetail))

//////////////////////////////////////////////////////////////
//プロトタイプ
static void sub_PlttVramTrans( u16* p_data, u8 target, u16 pos, u16 num );
static void sub_UpDownButtonActiveControl( BEACON_DETAIL_WORK* wk );
static const BEACON_DETAIL* sub_GetBeaconDetailParam( GAMEBEACON_ACTION action );
static void sub_DetailWordset(const GAMEBEACON_INFO *info, WORDSET *wordset );

static void act_AnmStart( GFL_CLWK* act, u8 anm_no );
static void act_SetPosition( GFL_CLWK* act, s16 x, s16 y, u16 setsf );
static void print_GetMsgToBuf( BEACON_DETAIL_WORK* wk, u8 msg_id );


static void draw_UnionObjUpdate( BEACON_DETAIL_WORK* wk, u8 char_no );
static void draw_BeaconWindowIni( BEACON_DETAIL_WORK* wk );
static void draw_BeaconData( BEACON_DETAIL_WORK* wk, BMP_WIN* win, STRBUF* str, u8 px, u8 fx, u8 sx, u8 sy, u8 col_idx );
static void draw_BeaconWindowScroll( BEACON_WIN* bp, s16 py );
static void draw_BeaconWindowVisibleSet( BEACON_DETAIL_WORK* wk, u8 idx, BOOL visible_f );
static void draw_BeaconWindow( BEACON_DETAIL_WORK* wk, GAMEBEACON_INFO* info, u16 time, u8 idx );
static void draw_UpdateUnderView( BEACON_DETAIL_WORK* wk );

static void effReq_PageScroll( BEACON_DETAIL_WORK* wk, u8 dir );

/*
 *  @brief  ビーコン詳細画面初期描画
 */
void BeaconDetail_InitialDraw( BEACON_DETAIL_WORK* wk )
{
  int i;

  draw_BeaconWindowIni( wk );

  GAMEBEACON_InfoTblRing_GetBeacon( wk->infoLog, wk->tmpInfo, &wk->tmpTime, wk->list[0]);
  draw_BeaconWindow( wk, wk->tmpInfo, wk->tmpTime, wk->flip_sw );
  draw_BeaconWindowVisibleSet( wk, wk->flip_sw, TRUE );
  draw_BeaconWindowVisibleSet( wk, wk->flip_sw^1, FALSE );
  draw_UpdateUnderView( wk );

  sub_UpDownButtonActiveControl( wk );
}

/*
 *  @brief  ビーコン詳細画面　入力取得
 */
int BeaconDetail_InputCheck( BEACON_DETAIL_WORK* wk )
{
  TOUCHBAR_ICON icon = TOUCHBAR_GetTrg( wk->touchbar );
	
  if( icon == TOUCHBAR_SELECT_NONE ){
    return SEQ_MAIN;
  }

  if( icon == TOUCHBAR_ICON_RETURN || icon == TOUCHBAR_ICON_CLOSE ){
    return SEQ_FADEOUT;
  }

  if( icon == TOUCHBAR_ICON_CUR_U && wk->list_top > 0 ){
    effReq_PageScroll( wk, SCROLL_DOWN );
    return SEQ_EFF_WAIT;
  }
  if( icon == TOUCHBAR_ICON_CUR_D && wk->list_top < (wk->list_max-1) ){
    effReq_PageScroll( wk, SCROLL_UP );
    return SEQ_EFF_WAIT;
  }
  return SEQ_MAIN;
}

/*
 *  @パレット転送
 */
static void sub_PlttVramTrans( u16* p_data, u8 target, u16 pos, u16 num  )
{
  u32 adrs = pos*2;
  u32 siz = num*2;

  DC_FlushRange( p_data, siz );

  switch( target ){
  case FADE_MAIN_BG:
    GX_LoadBGPltt( p_data, adrs, siz );
    break;
  case FADE_SUB_BG:
    GXS_LoadBGPltt( p_data, adrs, siz );
    break;
  case FADE_MAIN_OBJ:
    GX_LoadOBJPltt( p_data, adrs, siz );
    break;
  case FADE_SUB_OBJ:
    GXS_LoadOBJPltt( p_data, adrs, siz );
    break;
  }
}

/*
 *  @brief  UpDownボタンアクティブセット
 */
static void sub_UpDownButtonActiveControl( BEACON_DETAIL_WORK* wk )
{
  if( wk->list_top > 0 ){
    TOUCHBAR_SetActive( wk->touchbar,  TOUCHBAR_ICON_CUR_U, TRUE );
  }else{
    TOUCHBAR_SetActive( wk->touchbar,  TOUCHBAR_ICON_CUR_U, FALSE );
  }
  if( wk->list_top < (wk->list_max-1) ){
    TOUCHBAR_SetActive( wk->touchbar,  TOUCHBAR_ICON_CUR_D, TRUE );
  }else{
    TOUCHBAR_SetActive( wk->touchbar,  TOUCHBAR_ICON_CUR_D, FALSE );
  }
}

/*
 *  @brief  アクションナンバーから詳細描画タイプを取得
 */
static const BEACON_DETAIL* sub_GetBeaconDetailParam( GAMEBEACON_ACTION action )
{
  int i;

  for(i = 0;i < BEACON_DETAIL_PARAM_NUM; i++){
    if( action == DATA_BeaconDetail[i].action ){
      return &DATA_BeaconDetail[i];
    }
  }
  return &DATA_BeaconDetail[0];
}

//==================================================================
/**
 * ビーコン詳細情報の内容をWORDSETする
 *
 * @param   info		        対象のビーコン情報へのポインタ
 * @param   wordset		
 * @param   temp_heap_id		内部でテンポラリとして使用するヒープID
 */
//==================================================================
static void sub_DetailWordset(const GAMEBEACON_INFO *info, WORDSET *wordset )
{
  GAMEBEACON_ACTION action = GAMEBEACON_Get_Action_ActionNo(info);

  switch( action ){
  case GAMEBEACON_ACTION_BATTLE_WILD_POKE_START:
  case GAMEBEACON_ACTION_BATTLE_SP_POKE_START:
    WORDSET_RegisterPokeMonsNameNo( wordset, 1,GAMEBEACON_Get_Action_Monsno(info));
    break;
  case GAMEBEACON_ACTION_BATTLE_TRAINER_START:
  case GAMEBEACON_ACTION_BATTLE_LEADER_START:
  case GAMEBEACON_ACTION_BATTLE_BIGFOUR_START:
  case GAMEBEACON_ACTION_BATTLE_CHAMPION_START:
    WORDSET_RegisterTrainerName( wordset, 0, GAMEBEACON_Get_Action_TrNo(info) );
    break;
  default:
    WORDSET_RegisterPlaceName( wordset, 0, GAMEBEACON_Get_ZoneID( info ) );
    break;
  }
}


/*
 *  @brief  アクター アニメスタート
 */
static void act_AnmStart( GFL_CLWK* act, u8 anm_no )
{
  GFL_CLACT_WK_SetAnmSeq( act, anm_no );
  GFL_CLACT_WK_ResetAnm( act );
}

/*
 *  @brief  アクター　座標セット
 */
static void act_SetPosition( GFL_CLWK* act, s16 x, s16 y, u16 setsf )
{
  GFL_CLACTPOS pos;
  pos.x = x;
  pos.y = y;
  
  GFL_CLACT_WK_SetPos( act, &pos, setsf );
}

/*
 *  @brief  文字列整形
 *
 *  指定メッセージを wk->str_expandにつめる
 */
static void print_GetMsgToBuf( BEACON_DETAIL_WORK* wk, u8 msg_id )
{
  GFL_MSG_GetString( wk->msg, msg_id, wk->str_tmp);
  WORDSET_ExpandStr( wk->wset, wk->str_expand, wk->str_tmp );
}


/*
 *  @brief  Unionアクター更新
 */
static void draw_UnionObjUpdate( BEACON_DETAIL_WORK* wk, u8 char_no )
{
  u8 sex, p_ofs;
 
  //キャラクタ転送
  GFL_CLGRP_CGR_Replace(  wk->objResUnion.cgr,
                          wk->resCharUnion[ char_no ].p_char );
  //パレット転送
  p_ofs = 16*sc_wifi_unionobj_plt[char_no];
//  PaletteWorkSet( wk->pfd, &wk->resPlttUnion.dat[p_ofs], FADE_SUB_OBJ, (ACT_PAL_UNION+pp->id)*16, 0x20 );
  sub_PlttVramTrans( &wk->resPlttUnion.dat[p_ofs], FADE_MAIN_OBJ, PLTID_OBJ_UNION_M*16, 16 );
}


/*
 *  @brief  ビーコンウィンドウ初期描画
 */
static void draw_BeaconWindowIni( BEACON_DETAIL_WORK* wk )
{
  int i,j;
  BEACON_WIN* bp;
  
  //プロフィール見出し
  for(i = 0;i < BEACON_PROF_MAX;i++){
    GFL_MSG_GetString( wk->msg, msg_prof01+i, wk->str_tmp );
  
    for(j = 0;j < BEACON_WIN_MAX;j++){
      bp = &wk->beacon_win[j];
  
      GFL_BMP_Clear( bp->prof[i].bmp, FCOL_BEACON_BASE(i%2) );

      PRINT_UTIL_PrintColor( &bp->prof[i].putil, wk->print_que,
          0, 0, wk->str_tmp, wk->font, FCOL_BEACON_COL(i%2) );
      GFL_BMPWIN_MakeTransWindow( bp->prof[i].win );
    }
  }
  //出身地見出し
  GFL_MSG_GetString( wk->msg, msg_home_town, wk->str_tmp );
  for(j = 0;j < BEACON_WIN_MAX;j++){
    bp = &wk->beacon_win[j];

    GFL_BMP_Clear( bp->home[0].bmp, FCOL_BEACON_BASE(0) );

    PRINT_UTIL_PrintColor( &bp->home[0].putil, wk->print_que,
          0, 0, wk->str_tmp, wk->font, FCOL_BEACON_COL(0) );
    GFL_BMPWIN_MakeTransWindow( bp->home[0].win );
  }

  //レコード見出し
  GFL_MSG_GetString( wk->msg, msg_record, wk->str_tmp);
  
  for(i = 0;i < BEACON_WIN_MAX;i++){
    bp = &wk->beacon_win[i];
    
    GFL_BMP_Clear( bp->record.bmp, FCOL_BEACON_BASE(0) );
    PRINT_UTIL_PrintColor( &bp->record.putil, wk->print_que,
          0, 0, wk->str_tmp, wk->font, FCOL_BEACON_COL(0) );
    GFL_BMPWIN_MakeTransWindow( bp->record.win );
  }
}

/*
 *  @brief  ビーコンウィンドウ描画 サブ
 */
static void draw_BeaconData( BEACON_DETAIL_WORK* wk, BMP_WIN* win, STRBUF* str, u8 px, u8 fx, u8 sx, u8 sy, u8 col_idx)
{
  GFL_BMP_Fill( win->bmp, px*8, 0, sx*8, sy*8, FCOL_BEACON_BASE(col_idx) );
  PRINT_UTIL_PrintColor( &win->putil, wk->print_que,
      fx, 0, str, wk->font, FCOL_BEACON_COL(col_idx) );
  GFL_BMPWIN_MakeTransWindow( win->win );
}

/*
 *  @brief  ビーコンウィンドウ 表示制御
 */
static void draw_BeaconWindowVisibleSet( BEACON_DETAIL_WORK* wk, u8 idx, BOOL visible_f )
{
  BEACON_WIN* bp = &wk->beacon_win[idx];

  GFL_CLACT_WK_SetDrawEnable( bp->cTrainer, visible_f );
  if(bp->rank > 0){
    GFL_CLACT_WK_SetDrawEnable( bp->cRank, visible_f );
  }
  PMS_DRAW_VisibleSet( wk->pms_draw, idx, visible_f );
	GFL_BG_SetVisible( bp->frame, visible_f );
}

/*
 *  @brief  ビーコンウィンドウポジションセット
 */
static void draw_BeaconWindowScroll( BEACON_WIN* bp, s16 py )
{
  GFL_BG_SetScroll( bp->frame, GFL_BG_SCROLL_Y_SET, -py );
 
  act_SetPosition( bp->cTrainer, ACT_TRAINER_PX, ACT_TRAINER_PY+py, ACT_SF_SUB );
  act_SetPosition( bp->cRank, ACT_RANK_PX, ACT_RANK_PY+py, ACT_SF_SUB );
}

/*
 *  @brief  ビーコンウィンドウ描画
 */
static void draw_BeaconWindow( BEACON_DETAIL_WORK* wk, GAMEBEACON_INFO* info, u16 time, u8 idx )
{
  BEACON_WIN* bp = &wk->beacon_win[idx];

  //パネルベースカラー
  {
    u16 pal[2];
    GAMEBEACON_Get_FavoriteColor((GXRgb*)&pal[0], info);
    SoftFade(&pal[0], &pal[1], 1, 3, 0x0000); //補色を作成
 
    sub_PlttVramTrans( pal, FADE_SUB_BG, 16*idx+1, 2);
  }

  //プレイヤー名
  GAMEBEACON_Get_PlayerNameToBuf( info, wk->str_tmp );
  draw_BeaconData( wk, &bp->prof[0], wk->str_tmp,
      BMP_PROF01_DAT_PX, BMP_PROF01_DAT_PX*8, BMP_PROF01_DAT_SX, BMP_PROF01_DAT_SY, 0 );
  //仕事
  draw_BeaconData( wk, &bp->prof[1], wk->str_tmp,
      BMP_PROF01_DAT_PX, BMP_PROF01_DAT_PX*8, BMP_PROF01_DAT_SX, BMP_PROF01_DAT_SY, 1 );
  //趣味
  draw_BeaconData( wk, &bp->prof[2], wk->str_tmp,
      BMP_PROF01_DAT_PX, BMP_PROF01_DAT_PX*8, BMP_PROF01_DAT_SX, BMP_PROF01_DAT_SY, 0 );
  //プレイ時間
  {
    u16 hour,min;
    GAMEBEACON_Get_PlayTime( info, &hour, &min );

    WORDSET_RegisterNumber( wk->wset, 2,
        hour, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wk->wset, 3,
        min, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  
    print_GetMsgToBuf( wk, msg_prof_ptime );
    draw_BeaconData( wk, &bp->prof[3], wk->str_expand,
        BMP_PROF02_DAT_PX, 0, BMP_PROF02_DAT_SX, BMP_PROF02_DAT_SY, 1);
  }
  //出身地
  GFL_MSG_GetString( wk->msg_area, GAMEBEACON_Get_Area(info) , wk->str_tmp);
  draw_BeaconData( wk, &bp->home[1], wk->str_tmp,
      0, 0, BMP_HOME02_SX, BMP_HOME02_SY, 0 );

  //レコード
  WORDSET_RegisterNumber( wk->wset, 0,
      /*GAMEBEACON_Get_ThanksRecvCount(info)*/99999, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wk->wset, 1,
      GAMEBEACON_Get_SuretigaiCount(info), 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  
  print_GetMsgToBuf( wk, msg_record_num );
  draw_BeaconData( wk, &bp->record, wk->str_expand,
      BMP_RECORD_DAT_PX, 0, BMP_RECORD_DAT_SX, BMP_RECORD_DAT_SY, 0);

  //簡易会話
  {
    PMS_DATA pms;
   
    PMS_DRAW_Clear( wk->pms_draw, idx, FALSE );
    PMSDAT_SetDebugRandom( &pms );
    PMSDAT_SetDeco( &pms, 0, PMS_DECOID_HERO + idx );
    PMS_DRAW_Print( wk->pms_draw, bp->pms, &pms , idx );
  }
  //ユニオンキャラクタナンバーを取得
  bp->union_char = GAMEBEACON_Get_TrainerView( info );

  //トレーナーOBJリソース再転送
  TR2DGRA_OBJ_RES_Replace( wk->h_trgra, UnionView_GetTrType( bp->union_char),
      wk->objResTrainer[idx].cgr, wk->objResTrainer[idx].pltt, wk->tmpHeapID );

  //ランク
  {
    int rank = GAMEBEACON_Get_ResearchTeamRank(info);

    rank = 5;
    if( rank == 0){
      GFL_CLACT_WK_SetDrawEnable( bp->cRank, FALSE );
    }else{
      act_AnmStart( bp->cRank, ACTANM_RANK01+(rank-1) );
      GFL_CLACT_WK_SetDrawEnable( bp->cRank, TRUE );
    }
  }

}

/*
 *  @brief  下画面カレントビーコン情報プリント
 */
static void draw_UpdateUnderView( BEACON_DETAIL_WORK* wk )
{
  u16 zone;
  GAMEBEACON_ACTION action;
  const BEACON_DETAIL* pd;

  GAMEBEACON_InfoTblRing_GetBeacon( wk->infoLog, wk->tmpInfo, &wk->tmpTime, wk->list[wk->list_top]);
  action = GAMEBEACON_Get_Action_ActionNo( wk->tmpInfo );
  pd = sub_GetBeaconDetailParam( action );

  draw_UnionObjUpdate( wk, GAMEBEACON_Get_TrainerView( wk->tmpInfo ) );
  zone = GAMEBEACON_Get_ZoneID( wk->tmpInfo );

	//プレイヤーの場所から、データインデックスを取得
  {
    s16 x,y;
	  u16 dataIndex = TOWNMAP_DATA_SearchRootZoneID( wk->tmap, GAMEBEACON_Get_TownmapRootZoneID( wk->tmpInfo ) );

	  //タウンマップ上の座標取得
	  x = TOWNMAP_DATA_GetParam( wk->tmap, dataIndex, TOWNMAP_DATA_PARAM_POS_X );
	  y = TOWNMAP_DATA_GetParam( wk->tmap, dataIndex, TOWNMAP_DATA_PARAM_POS_Y );

    act_SetPosition( wk->pAct[ACT_ICON_TR], x, y, ACT_SF_MAIN );

    if( pd->icon != 0 ){
      act_SetPosition( wk->pAct[ACT_ICON_EV], x-24, y, ACT_SF_MAIN );
      act_AnmStart( wk->pAct[ACT_ICON_EV], ACTANM_ICON_TR + (pd->icon-1) );
    }
    GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_ICON_EV], (pd->icon != 0) );
  }
  //ポップアップメッセージ描画
  sub_DetailWordset( wk->tmpInfo, wk->wset );
  print_GetMsgToBuf( wk, pd->msg_id );

  GFL_BMP_Clear( wk->win_popup.bmp, FCOL_POPUP_BASE );
  PRINT_UTIL_PrintColor( &wk->win_popup.putil, wk->print_que, 0, 0, wk->str_expand, wk->font, FCOL_POPUP );
  GFL_BMPWIN_MakeTransWindow( wk->win_popup.win );
}


/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ アップダウン 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_MSG_UPDOWN{
  u8  dir;
  u8  frame;
  u8  ct;
  s8  y;
  s8  diff;

  int* task_ct;
}TASKWK_MSG_UPDOWN;

static void taskAdd_MsgUpdown( BEACON_DETAIL_WORK* wk, u8 dir, int* task_ct );
static void tcb_MsgUpdown( GFL_TCBL *tcb , void* work);

/*
 *  @brief  メッセージウィンドウ アップダウンタスク登録
 */
static void taskAdd_MsgUpdown( BEACON_DETAIL_WORK* wk, u8 dir, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_MSG_UPDOWN* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_MsgUpdown, sizeof(TASKWK_MSG_UPDOWN), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_MSG_UPDOWN ));

  twk->dir = dir;
  if( twk->dir == SCROLL_UP ){
    twk->y = 0;
    twk->diff = POPUP_DIFF;
  }else{
    twk->y = POPUP_HEIGHT;
    twk->diff = -POPUP_DIFF;
  }
  twk->frame = POPUP_COUNT;

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_MsgUpdown( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_MSG_UPDOWN* twk = (TASKWK_MSG_UPDOWN*)tcb_wk;
 
  if( twk->ct++ < twk->frame ){
    twk->y += twk->diff;
    GFL_BG_SetScroll( BG_FRAME_POPUP_M, GFL_BG_SCROLL_Y_SET, twk->y );
    IWASAWA_Printf("Popup %d\n",twk->y);
    return;
  }

  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  メッセージウィンドウ ポップアップ 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_WIN_POPUP{
  u8  seq;
  u8  wait;
  int child_task;
 
  BEACON_DETAIL_WORK* bdw;
  int* task_ct;
}TASKWK_WIN_POPUP;

static void taskAdd_WinPopup( BEACON_DETAIL_WORK* wk, STRBUF* str, int* task_ct );
static void tcb_WinPopup( GFL_TCBL *tcb , void* work);

static void effReq_PopupMsgSys( BEACON_DETAIL_WORK* wk, u8 msg_id )
{
  print_GetMsgToBuf( wk, msg_id );
  taskAdd_WinPopup( wk, wk->str_expand, &wk->eff_task_ct);
}

/*
 *  @brief  メッセージウィンドウ ポップアップタスク登録
 */
static void taskAdd_WinPopup( BEACON_DETAIL_WORK* wk, STRBUF* str, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_WIN_POPUP* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_WinPopup, sizeof(TASKWK_WIN_POPUP), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_WIN_POPUP ));

  GFL_STR_CopyBuffer( wk->str_popup, str );
  
  twk->bdw = wk;
  twk->wait = POPUP_WAIT;

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_WinPopup( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_WIN_POPUP* twk = (TASKWK_WIN_POPUP*)tcb_wk;

  switch( twk->seq ){
  case 0:
//    print_PopupWindow( twk->bdw, twk->bdw->str_popup, 0 );
//    PRINT_UTIL_PrintColor( &wk->win[WIN_POPUP].putil, wk->print_que, 0, 0, str, wk->font, FCOL_POPUP );
//    GFL_BMPWIN_MakeTransWindow( wk->win[WIN_POPUP].win );
    twk->seq++;
    return;
  case 1:
    taskAdd_MsgUpdown( twk->bdw, SCROLL_UP, &twk->child_task );
    twk->seq++;
    return;
  case 2:
    if( twk->child_task ){
      return;
    }
    twk->seq++;
    return;
  case 3:
    if( twk->wait-- > 0 ){
      return;
    }
    taskAdd_MsgUpdown( twk->bdw, SCROLL_DOWN, &twk->child_task );
    twk->seq++;
    return;
  case 4:
    if( twk->child_task ){
      return;
    }
  }
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  ビーコンウィンドウ スクロール 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_BWIN_SCROLL{
  u8  win_idx;
  u8  wait;
  u8  frame;
  s16 py;
  s16 dy;

  int child_task;
 
  BEACON_WIN* bp;
  BEACON_DETAIL_WORK* bdw;
  int* task_ct;
}TASKWK_BWIN_SCROLL;

static void taskAdd_BWinScroll( BEACON_DETAIL_WORK* wk, u8 win_idx, u8 spos, u8 dir, int* task_ct );
static void tcb_BWinScroll( GFL_TCBL *tcb , void* work);

static void effReq_BWinScroll( BEACON_DETAIL_WORK* wk, u8 msg_id )
{
  print_GetMsgToBuf( wk, msg_id );
//  taskAdd_BWinScroll( wk, wk->str_expand, &wk->eff_task_ct);
}

/*
 *  @brief  ビーコンウィンドウ スクロールタスク登録
 */
static void taskAdd_BWinScroll( BEACON_DETAIL_WORK* wk, u8 win_idx, u8 spos, u8 dir, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_BWIN_SCROLL* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_BWinScroll, sizeof(TASKWK_BWIN_SCROLL), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_BWIN_SCROLL ));

  twk->bp = &wk->beacon_win[win_idx];
  twk->win_idx = win_idx;
  twk->py = PAGE_SCROLL_PY+(spos*PAGE_SCROLL_H);

  if( dir == SCROLL_UP ){
    twk->dy -= PAGE_SCROLL_DY;
  }else{
    twk->dy += PAGE_SCROLL_DY;
  }
  twk->wait = PAGE_SCROLL_TIME;

  draw_BeaconWindowScroll( twk->bp, twk->py );

  if( spos != SCROLL_POS_DEF ){
    draw_BeaconWindowVisibleSet( wk, twk->win_idx, TRUE );
  }
  twk->bdw = wk;

  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_BWinScroll( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_BWIN_SCROLL* twk = (TASKWK_BWIN_SCROLL*)tcb_wk;

  if( twk->wait-- > 0 ){
    twk->py += twk->dy;
    draw_BeaconWindowScroll( twk->bp, twk->py );
    return;
  }

  if( twk->py != 0 ){
    draw_BeaconWindowVisibleSet( twk->bdw, twk->win_idx, FALSE );
  }
  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

/////////////////////////////////////////////////////////////////////
/*
 *  @brief  ビーコンウィンドウ スクロール 
 */
/////////////////////////////////////////////////////////////////////
typedef struct _TASKWK_PAGE_SCROLL{
  u8  seq;
  u8  wait;
  u8  frame;
  u8  next;
  u8  dir;

  int child_task;
 
  BEACON_WIN* bp;
  BEACON_DETAIL_WORK* bdw;
  int* task_ct;
}TASKWK_PAGE_SCROLL;

static void taskAdd_PageScroll( BEACON_DETAIL_WORK* wk, u8 dir, int* task_ct );
static void tcb_PageScroll( GFL_TCBL *tcb , void* work);

static void effReq_PageScroll( BEACON_DETAIL_WORK* wk, u8 dir )
{
  taskAdd_PageScroll( wk, dir, &wk->eff_task_ct );
}

/*
 *  @brief  ビーコンウィンドウ スクロールタスク登録
 */
static void taskAdd_PageScroll( BEACON_DETAIL_WORK* wk, u8 dir, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_PAGE_SCROLL* twk;

  tcb = GFL_TCBL_Create( wk->pTcbSys, tcb_PageScroll, sizeof(TASKWK_PAGE_SCROLL), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_PAGE_SCROLL ));

  twk->dir = dir;
  if( dir == SCROLL_UP ){
    twk->next = wk->list_top+1;
  }else{
    twk->next = wk->list_top-1;
  }
  //次に表示されるウィンドウを描く
  GAMEBEACON_InfoTblRing_GetBeacon( wk->infoLog, wk->tmpInfo, &wk->tmpTime, wk->list[twk->next]);
  draw_BeaconWindow( wk, wk->tmpInfo, wk->tmpTime, wk->flip_sw^1 );

  twk->bdw = wk;
  twk->task_ct = task_ct;
  (*task_ct)++;
}

static void tcb_PageScroll( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_PAGE_SCROLL* twk = (TASKWK_PAGE_SCROLL*)tcb_wk;
  static const u8 pos_tbl[] = { SCROLL_POS_DOWN, SCROLL_POS_UP };

  switch( twk->seq ){
  case 0:
    taskAdd_BWinScroll( twk->bdw, twk->bdw->flip_sw, SCROLL_POS_DEF, twk->dir, &twk->child_task );
    taskAdd_BWinScroll( twk->bdw, twk->bdw->flip_sw^1, pos_tbl[twk->dir], twk->dir, &twk->child_task );
    twk->seq++;
    return;
  case 1:
    if( twk->child_task ){
      return;
    }
    break;
  }
  twk->bdw->flip_sw ^= 1;
  twk->bdw->list_top = twk->next;
  draw_UpdateUnderView( twk->bdw );
  sub_UpDownButtonActiveControl( twk->bdw );

  --(*twk->task_ct);
  GFL_TCBL_Delete(tcb);
}

