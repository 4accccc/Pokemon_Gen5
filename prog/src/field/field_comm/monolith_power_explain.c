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
#include "gamesystem/g_power.h"
#include "field/monolith_tool_notwifi.h"

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
//
static s32  mpe_chk_have_number( MONOLITH_APP_PARENT *appwk, const GPOWER_ID id );


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
  
  switch(*seq){
  case 0:
    pew = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_POWEREXPLAIN_WORK), HEAPID_MONOLITH);
    GFL_STD_MemClear(pew, sizeof(MONOLITH_POWEREXPLAIN_WORK));
    pew->print_select_gpower_id = GPOWER_ID_NULL;
    
    //BG
    _Setup_BGFrameSetting();
    _Setup_BGGraphicLoad(appwk->setup);
    _Setup_BmpWin_Create(appwk->setup, pew);

    _Write_Title(appwk->setup, pew);
    _Write_EqpPower(appwk, appwk->setup, pew);
    
    (*seq)++;
    break;
  case 1:
    {
      int i, no_trans = FALSE;
      
      for(i = 0; i < BMPWIN_MAX; i++){
        if(PRINT_UTIL_Trans(&pew->print_util[i], appwk->setup->printQue) == FALSE){
          no_trans = TRUE;
        }
      }
    	if(no_trans == FALSE){
      	GFL_BG_SetVisible(GFL_BG_FRAME0_M, VISIBLE_ON);
      	GFL_BG_SetVisible(GFL_BG_FRAME2_M, VISIBLE_ON);
      	GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
      	return GFL_PROC_RES_FINISH;
      }
    }
    break;
  }
  
  return GFL_PROC_RES_CONTINUE;
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
  u8 trans_ret[BMPWIN_MAX];
  
  if(appwk->up_proc_finish == TRUE){// || appwk->force_finish == TRUE){
    return GFL_PROC_RES_FINISH;
  }
  
  for(i = 0; i < BMPWIN_MAX; i++){
    trans_ret[i] = PRINT_UTIL_Trans(&pew->print_util[i], appwk->setup->printQue);
  }
  
  // 処理中は更新しない
  if( PRINTSYS_QUE_IsFinished( appwk->setup->printQue ) == FALSE ){
    return( GFL_PROC_RES_CONTINUE );
  }
  
  switch(*seq){
  case 0:
    if(appwk->common->power_eqp_update == TRUE && trans_ret[BMPWIN_EQP] == TRUE){
      _Write_EqpPower(appwk, appwk->setup, pew);
      appwk->common->power_eqp_update = FALSE;
    }
    if(appwk->common->power_select_no != pew->print_select_gpower_id
        && trans_ret[BMPWIN_EXPLAIN] == TRUE){
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

  if(PRINTSYS_QUE_IsFinished(appwk->setup->printQue) == FALSE){
    return GFL_PROC_RES_CONTINUE;
  }

  GFL_DISP_GX_SetVisibleControlDirect(GX_PLANEMASK_BG3);
  
  //BG
  _Setup_BmpWin_Exit(pew);
  _Setup_BGFrameExit();

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
  pew->bmpwin[BMPWIN_EQP] = GFL_BMPWIN_Create(GFL_BG_FRAME0_M, 2, 4, 28, 8, 
    MONOLITH_BG_UP_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( pew->bmpwin[BMPWIN_EQP] );
  GFL_BMPWIN_MakeScreen( pew->bmpwin[BMPWIN_EQP] );

  //選択されているパワー
  pew->bmpwin[BMPWIN_EXPLAIN] = GFL_BMPWIN_Create(GFL_BG_FRAME0_M, 2, 0xf, 28, 9, 
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
 * @param   MONOLITH_APP_PARENT			*appwk
 * @param   MONOLITH_SETUP				*setup
 * @param   MONOLITH_POWEREXPLAIN_WORK	*pew
 *
 * @retval
 */
//--------------------------------------------------------------
static void _Write_EqpPower(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_POWEREXPLAIN_WORK *pew)
{
  STRBUF *str_name;
  s16 i;
  s16 flg=0;
  
  // クリア
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pew->bmpwin[BMPWIN_EQP]), 0x0000);
  //
  // 所持をチェック
  for(i=0;i<INTRUDE_EQUIP_GPOWER_MAX;i++){
    if(appwk->common->gpower_no[i] != GPOWER_ID_NULL){
      str_name = GFL_MSG_CreateString(setup->mm_power, setup->powerdata[appwk->common->gpower_no[i]].msg_id_title);
      PRINT_UTIL_Print(&pew->print_util[BMPWIN_EQP], setup->printQue, 
        0, 16*i, str_name, setup->font_handle);
      GFL_STR_DeleteBuffer(str_name);
      str_name = 0;
      flg = 1;
    }
  }
  // 何も表示していない場合
  // (=>持っていないメッセージ表示)
  if(flg == 0){
    str_name = GFL_MSG_CreateString(setup->mm_monolith, msg_mono_pow_up_001);
    PRINT_UTIL_Print(&pew->print_util[BMPWIN_EQP], setup->printQue, 
      0, 0, str_name, setup->font_handle);
    GFL_STR_DeleteBuffer(str_name);
    str_name = 0;
  }
}

//--------------------------------------------------------------
/**
 * 下画面で選択中のパワー描画
 *
 * @param   MONOLITH_APP_PARENT			*appwk
 * @param   MONOLITH_SETUP				*setup
 * @param   MONOLITH_POWEREXPLAIN_WORK	*pew
 *
 * @retval
 */
//--------------------------------------------------------------
static void _Write_SelectPower(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_POWEREXPLAIN_WORK *pew)
{
  GPOWER_ID gpower_id = appwk->common->power_select_no;
  s32 num;
  s32 flg=0;
  
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pew->bmpwin[BMPWIN_EXPLAIN]), 0x0000);
  
  if(gpower_id != GPOWER_ID_NULL){
    // すでにあるかチェック
    num = mpe_chk_have_number( appwk, gpower_id );
    if(num != -1){
      flg = 1;
    }
    // 取得済み
    if(appwk->common->power_mono_use == MONO_USE_POWER_OK){
      flg = 1;
    }
    // 説明文表示
    if(flg == 1){
      STRBUF *str_explain, *str_point, *str_expand;
      
      WORDSET_RegisterNumber(setup->wordset, 0, setup->powerdata[gpower_id].point, 4, 
        STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
      
      str_explain = GFL_MSG_CreateString(
        setup->mm_power_explain, setup->powerdata[gpower_id].msg_id_explain);
      str_point = GFL_MSG_CreateString(setup->mm_monolith, msg_mono_pow_up_002);

      str_expand = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
      WORDSET_ExpandStr(setup->wordset, str_expand, str_point );

      PRINT_UTIL_Print(&pew->print_util[BMPWIN_EXPLAIN], setup->printQue, 
        0, 4, str_explain, setup->font_handle);
      PRINT_UTIL_Print(&pew->print_util[BMPWIN_EXPLAIN], setup->printQue, 
        112, 4+16*3, str_expand, setup->font_handle);

      GFL_STR_DeleteBuffer(str_explain);
      GFL_STR_DeleteBuffer(str_point);
      GFL_STR_DeleteBuffer(str_expand);
    }
    // レベルが足りないメッセージ
    else{
      const OCCUPY_INFO *occupy = MonolithTool_GetOccupyInfo(appwk);
      int num, msg_id;
      STRBUF *str_msg, *str_expand;
      
      if(setup->powerdata[gpower_id].level_w > occupy->white_level){
        num = setup->powerdata[gpower_id].level_w;
        msg_id = msg_mono_pow_up_003;
      }
      else{
        num = setup->powerdata[gpower_id].level_b;
        msg_id = msg_mono_pow_up_004;
      }
      WORDSET_RegisterNumber(setup->wordset, 0, num, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
      WORDSET_RegisterNumber(setup->wordset, 1, occupy->white_level, 
        3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
      WORDSET_RegisterNumber(setup->wordset, 2, occupy->black_level, 
        3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);

      str_msg = GFL_MSG_CreateString(setup->mm_monolith, msg_id);
      str_expand = GFL_STR_CreateBuffer( 128, HEAPID_MONOLITH );
      WORDSET_ExpandStr(setup->wordset, str_expand, str_msg );

      PRINT_UTIL_Print(&pew->print_util[BMPWIN_EXPLAIN], setup->printQue, 
        0, 4, str_expand, setup->font_handle);

      GFL_STR_DeleteBuffer(str_expand);
      GFL_STR_DeleteBuffer(str_msg);
    }
  }
  else{ //PrintUtilが実行されないため、クリアしたキャラを転送しておく
    GFL_BMPWIN_TransVramCharacter( pew->bmpwin[BMPWIN_EXPLAIN] );
  }
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//--------------------------------------------------------------
/**
 * GPOWERをすでに持っているか
 *
 * @param   MONOLITH_POWEREXPLAIN_WORK 	*pew
 * @param   const GPOWER_ID				id
 *
 * @retval  s32		(-1:なし、0-2:持っている場所)
 */
//--------------------------------------------------------------
static s32 mpe_chk_have_number( MONOLITH_APP_PARENT *appwk, const GPOWER_ID id )
{
  s32 i;
  
  // チェック
  for(i=0;i<INTRUDE_EQUIP_GPOWER_MAX;i++){
    if( appwk->common->gpower_no[i] == id ){
      return( i );
    }
  }
  // なかった
  return( -1 );
}

