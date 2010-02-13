//==============================================================================
/**
 * @file    monolith_power_explain.c
 * @brief   パワー説明画面
 * @author  matsuda
 * @date    2010.02.12(金)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "monolith_common.h"
#include "arc_def.h"
#include "monolith_tool.h"
#include "msg/msg_monolith.h"
#include "msg/msg_mission_monolith.h"
#include "mission_types.h"
#include "intrude_mission.h"
#include "intrude_types.h"
#include "gamesystem/g_power.h"

#include "monolith.naix"


//==============================================================================
//  定数定義
//==============================================================================
enum{
  BMPWIN_TITLE,        ///<タイトル
  BMPWIN_EQP,          ///<装備しているパワーの説明
  BMPWIN_EXPLAIN,      ///<下画面で選択しているパワーの説明
  
  BMPWIN_MAX,
};

//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  GFL_BMPWIN *bmpwin[BMPWIN_MAX];
  PRINT_UTIL print_util[BMPWIN_MAX];
  
  GPOWER_ID print_select_gpower_id;   ///<描画したGPOWER_ID
  
  POWER_CONV_DATA *powerdata;
}MONOLITH_POWEREXPLAIN_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT MonolithPowerExplainProc_Init(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithPowerExplainProc_Main(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithPowerExplainProc_End(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void _Setup_BGFrameSetting(void);
static void _Setup_BGFrameExit(void);
static void _Setup_BGGraphicLoad(MONOLITH_SETUP *setup);
static void _Setup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_POWEREXPLAIN_WORK *pew);
static void _Setup_BmpWin_Exit(MONOLITH_POWEREXPLAIN_WORK *pew);
static void _Write_Title(MONOLITH_SETUP *setup, MONOLITH_POWEREXPLAIN_WORK *pew);
static void _Write_EqpPower(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_POWEREXPLAIN_WORK *pew);
static void _Write_SelectPower(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_POWEREXPLAIN_WORK *pew);


//==============================================================================
//  データ
//==============================================================================
///モノリスパワーROCデータ
const GFL_PROC_DATA MonolithAppProc_Up_PowerExplain = {
  MonolithPowerExplainProc_Init,
  MonolithPowerExplainProc_Main,
  MonolithPowerExplainProc_End,
};



//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   プロセス関数：初期化
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MonolithPowerExplainProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_POWEREXPLAIN_WORK *pew = mywk;
	ARCHANDLE *hdl;
  
  pew = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_POWEREXPLAIN_WORK), HEAPID_MONOLITH);
  GFL_STD_MemClear(pew, sizeof(MONOLITH_POWEREXPLAIN_WORK));
  pew->print_select_gpower_id = GPOWER_ID_NULL;
  
  pew->powerdata = GPOWER_PowerData_LoadAlloc(HEAPID_MONOLITH);
  
  //BG
  _Setup_BGFrameSetting();
  _Setup_BGGraphicLoad(appwk->setup);
  _Setup_BmpWin_Create(appwk->setup, pew);

  _Write_Title(appwk->setup, pew);
  _Write_EqpPower(appwk, appwk->setup, pew);
  
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：メイン
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MonolithPowerExplainProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_POWEREXPLAIN_WORK *pew = mywk;
  int i;

  if(appwk->up_proc_finish == TRUE || appwk->force_finish == TRUE){
    return GFL_PROC_RES_FINISH;
  }
  
  for(i = 0; i < BMPWIN_MAX; i++){
    PRINT_UTIL_Trans(&pew->print_util[i], appwk->setup->printQue);
  }
  
  switch(*seq){
  case 0:
    if(appwk->common->power_eqp_update == TRUE){
      _Write_EqpPower(appwk, appwk->setup, pew);
      appwk->common->power_eqp_update = FALSE;
    }
    if(appwk->common->power_select_no != pew->print_select_gpower_id){
      _Write_SelectPower(appwk, appwk->setup, pew);
      pew->print_select_gpower_id = appwk->common->power_select_no;
    }
    return GFL_PROC_RES_CONTINUE;
  }
  
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：終了
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MonolithPowerExplainProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_POWEREXPLAIN_WORK *pew = mywk;
  
  //BG
  _Setup_BmpWin_Exit(pew);
  _Setup_BGFrameExit();

  GPOWER_PowerData_Unload(pew->powerdata);
  
  GFL_PROC_FreeWork(proc);
  return GFL_PROC_RES_FINISH;
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   BGフレーム設定
 */
//--------------------------------------------------------------
static void _Setup_BGFrameSetting(void)
{
	static const GFL_BG_BGCNT_HEADER bgcnt_frame[] = {
		{//GFL_BG_FRAME0_M
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x0c000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		},
		{//GFL_BG_FRAME2_M
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, MONO_BG_COMMON_CHARBASE, MONO_BG_COMMON_CHAR_SIZE,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		},
	};

	GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME2_M,   &bgcnt_frame[1],   GFL_BG_MODE_TEXT );

	GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( GFL_BG_FRAME2_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_SetVisible(GFL_BG_FRAME0_M, VISIBLE_ON);
	GFL_BG_SetVisible(GFL_BG_FRAME2_M, VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * BGフレーム解放
 */
//--------------------------------------------------------------
static void _Setup_BGFrameExit(void)
{
	GFL_BG_SetVisible(GFL_BG_FRAME0_M, VISIBLE_OFF);
	GFL_BG_SetVisible(GFL_BG_FRAME2_M, VISIBLE_OFF);
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_M);
  GFL_BG_FreeBGControl(GFL_BG_FRAME2_M);
}

//--------------------------------------------------------------
/**
 * @brief   BGグラフィックをVRAMへ転送
 *
 * @param   hdl		アーカイブハンドル
 */
//--------------------------------------------------------------
static void _Setup_BGGraphicLoad(MONOLITH_SETUP *setup)
{
	GFL_ARCHDL_UTIL_TransVramScreen(setup->hdl, NARC_monolith_mono_bgu_power_lz_NSCR, 
		GFL_BG_FRAME2_M, 0, 0, TRUE, HEAPID_MONOLITH);

	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   setup		
 * @param   pew		
 */
//--------------------------------------------------------------
static void _Setup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_POWEREXPLAIN_WORK *pew)
{
  int i;
  
  //タイトル
  pew->bmpwin[BMPWIN_TITLE] = GFL_BMPWIN_Create(GFL_BG_FRAME0_M, 2, 0, 28, 3, 
    MONOLITH_BG_UP_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( pew->bmpwin[BMPWIN_TITLE] );
  GFL_BMPWIN_MakeScreen( pew->bmpwin[BMPWIN_TITLE] );

  //装備パワー
  pew->bmpwin[BMPWIN_EQP] = GFL_BMPWIN_Create(GFL_BG_FRAME0_M, 2, 4, 28, 7, 
    MONOLITH_BG_UP_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( pew->bmpwin[BMPWIN_EQP] );
  GFL_BMPWIN_MakeScreen( pew->bmpwin[BMPWIN_EQP] );

  //選択されているパワー
  pew->bmpwin[BMPWIN_EXPLAIN] = GFL_BMPWIN_Create(GFL_BG_FRAME0_M, 2, 0x10, 28, 9, 
    MONOLITH_BG_UP_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( pew->bmpwin[BMPWIN_EXPLAIN] );
  GFL_BMPWIN_MakeScreen( pew->bmpwin[BMPWIN_EXPLAIN] );
  
  for(i = 0; i < BMPWIN_MAX; i++){
    PRINT_UTIL_Setup(&pew->print_util[i], pew->bmpwin[i]);
  }
  
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   pew		
 */
//--------------------------------------------------------------
static void _Setup_BmpWin_Exit(MONOLITH_POWEREXPLAIN_WORK *pew)
{
  int i;
  for(i = 0; i < BMPWIN_MAX; i++){
    GFL_BMPWIN_Delete(pew->bmpwin[i]);
  }
}

//--------------------------------------------------------------
/**
 * タイトル描画
 *
 * @param   setup		
 * @param   pew		
 */
//--------------------------------------------------------------
static void _Write_Title(MONOLITH_SETUP *setup, MONOLITH_POWEREXPLAIN_WORK *pew)
{
  STRBUF *str_title;
  
  str_title = GFL_MSG_CreateString(setup->mm_monolith, msg_mono_pow_up_000);
  PRINT_UTIL_Print(&pew->print_util[BMPWIN_TITLE], setup->printQue, 
    0, 4, str_title, setup->font_handle);
  GFL_STR_DeleteBuffer(str_title);
}

//--------------------------------------------------------------
/**
 * 装備パワー描画
 *
 * @param   appwk		
 * @param   setup		
 * @param   pew		
 */
//--------------------------------------------------------------
static void _Write_EqpPower(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_POWEREXPLAIN_WORK *pew)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(appwk->parent->gsys);
  INTRUDE_SAVE_WORK *intsave = SaveData_GetIntrude( GAMEDATA_GetSaveControlWork( gamedata ) );
  GPOWER_ID gpower_id = ISC_SAVE_GetGPowerID(intsave);
  STRBUF *str_name, *str_explain;
  u16 msgid_title;
  
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pew->bmpwin[BMPWIN_EQP]), 0x0000);
  
  if(gpower_id == GPOWER_ID_NULL){
    str_name = GFL_MSG_CreateString(setup->mm_monolith, msg_mono_pow_up_001);
  }
  else{
    str_name = GFL_MSG_CreateString(setup->mm_power, pew->powerdata[gpower_id].msg_id_title);
    str_explain = GFL_MSG_CreateString(
      setup->mm_power_explain, pew->powerdata[gpower_id].msg_id_explain);
    PRINT_UTIL_Print(&pew->print_util[BMPWIN_EQP], setup->printQue, 
      0, 16, str_explain, setup->font_handle);
    GFL_STR_DeleteBuffer(str_explain);
  }
  PRINT_UTIL_Print(&pew->print_util[BMPWIN_EQP], setup->printQue, 
    0, 0, str_name, setup->font_handle);
  GFL_STR_DeleteBuffer(str_name);
}

//--------------------------------------------------------------
/**
 * 下画面で選択中のパワー描画
 *
 * @param   appwk		
 * @param   setup		
 * @param   pew		
 */
//--------------------------------------------------------------
static void _Write_SelectPower(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_POWEREXPLAIN_WORK *pew)
{
  GPOWER_ID gpower_id = appwk->common->power_select_no;
  STRBUF *str_name, *str_explain, *str_point, *str_expand;
  
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pew->bmpwin[BMPWIN_EXPLAIN]), 0x0000);
  
  if(gpower_id != GPOWER_ID_NULL){
    WORDSET_RegisterNumber(setup->wordset, 0, pew->powerdata[gpower_id].point, 4, 
      STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    
    str_name = GFL_MSG_CreateString(setup->mm_power, pew->powerdata[gpower_id].msg_id_title);
    str_explain = GFL_MSG_CreateString(
      setup->mm_power_explain, pew->powerdata[gpower_id].msg_id_explain);
    str_point = GFL_MSG_CreateString(setup->mm_monolith, msg_mono_pow_up_002);

    str_expand = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
    WORDSET_ExpandStr(setup->wordset, str_expand, str_point );

    PRINT_UTIL_Print(&pew->print_util[BMPWIN_EXPLAIN], setup->printQue, 
      0, 0, str_name, setup->font_handle);
    PRINT_UTIL_Print(&pew->print_util[BMPWIN_EXPLAIN], setup->printQue, 
      0, 16, str_explain, setup->font_handle);
    PRINT_UTIL_Print(&pew->print_util[BMPWIN_EXPLAIN], setup->printQue, 
      0, 16+16*3, str_expand, setup->font_handle);
    
    GFL_STR_DeleteBuffer(str_name);
    GFL_STR_DeleteBuffer(str_explain);
    GFL_STR_DeleteBuffer(str_point);
    GFL_STR_DeleteBuffer(str_expand);
  }
  else{ //PrintUtilが実行されないため、クリアしたキャラを転送しておく
    GFL_BMPWIN_TransVramCharacter( pew->bmpwin[BMPWIN_EXPLAIN] );
  }
}

