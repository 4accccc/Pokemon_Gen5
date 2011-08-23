//==============================================================================
/**
 * @file    monolith_up_fmission.c
 * @brief   モノリス：フェスミッション選択画面(上画面)
 * @author  
 * @date    
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "monolith_common.h"
#include "arc_def.h"
#include "monolith_tool.h"
#include "msg/msg_monolith.h"
#include "intrude_work.h"
#include "monolith.naix"


//==============================================================================
//  定数定義
//==============================================================================
enum{
  BMPWIN_TITLE,        ///< タイトル
  BMPWIN_EXPLAIN,      ///< 説明
  //
  BMPWIN_TIME_L,       ///< 制限時間(左)
  BMPWIN_CLEAR_L,      ///< クリア目標(左)
  BMPWIN_GAIN_L,       ///< 報酬(左)
  BMPWIN_SCORE_L,      ///< ハイスコア(左)
  //
  BMPWIN_TIME_R,       ///< 制限時間(右)
  BMPWIN_CLEAR_R,      ///< クリア目標(右)
  BMPWIN_GAIN_R,       ///< 報酬(右)
  BMPWIN_SCORE_R,      ///< ハイスコア(右)
  //
  BMPWIN_MAX,
};

//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  GFL_BMPWIN 	*bmpwin[BMPWIN_MAX];	// bmpwin
  PRINT_UTIL 	print_util[BMPWIN_MAX];	// print_util
  
  s32  			mode;
}MONOLITH_UP_WORK;

//-----------------------------------------
// 文字用の定義(表示領域の設定:CANVAS)
//------------------------------------------
typedef struct{
  u8				frmnumber;			// BG上下(frame定義なので注意)(0xffの場合は終了判定)
  u8				x;					// 座標
  u8				y;
  u8				width;				// サイズ
  u8				height;
  u8				palnum;				// パレット番号
}MONOLITH_CANVAS_DATA;

//==============================================================================
//  データ定義
//==============================================================================
// CANVASの設定
// (printutilと対応)
const MONOLITH_CANVAS_DATA mono_canvas_tbl[] = {
  { GFL_BG_FRAME0_M,	2,	0,	28,	3,	MONOLITH_BG_UP_FONT_PALNO,},	// タイトル
  { GFL_BG_FRAME0_M,	2,	4,	28,	8,	MONOLITH_BG_UP_FONT_PALNO,},	// 説明文
  //
  { GFL_BG_FRAME0_M,	2,	15,	12,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// 制限時間(左)
  { GFL_BG_FRAME0_M,	2,	17,	12,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// クリア目標(左)
  { GFL_BG_FRAME0_M,	2,	19,	12,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// 報酬(左)
  { GFL_BG_FRAME0_M,	2,	21,	12,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// ハイスコア(左)
  //
  { GFL_BG_FRAME0_M,   14,	15,	15,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// 制限時間(右)
  { GFL_BG_FRAME0_M,   14,	17,	15,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// クリア目標(右)
  { GFL_BG_FRAME0_M,   14,	19,	15,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// 報酬(右)
  { GFL_BG_FRAME0_M,   14,	21,	15,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// ハイスコア(右)
  
  { 0xff, 0, 0, 0, 0, 0,}, // end code
};

// 文字列、offset(-1はなし)
const s16 mono_mes_tbl[] = {
  msg_mono_fmission_010,4,		// ミッションせつめい
  0,					-1,		// せつめい文(dummy)
  //
  msg_mono_fmission_011,0,		// 制限時間
  msg_mono_fmission_012,0,		// クリア目標
  msg_mono_fmission_013,0,		// 報酬
  msg_mono_fmission_014,0,		// ハイスコア
  //
  msg_mono_fmission_015,-1,		// 制限時間
  msg_mono_fmission_016,-1,		// クリア目標
  msg_mono_fmission_017,-1,		// 報酬
  msg_mono_fmission_016,-1,		// ハイスコア
};

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT UpFMisssionProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT UpFMisssionProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT UpFMisssionProc_End(  GFL_PROC * proc, int * seq, void * pwk, void * mywk);

static void _Setup_BGFrameSetting(void);
static void _Setup_BGFrameExit(void);
static void _Setup_BGGraphicLoad(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup);

static void _Setup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk);
static void _Setup_BmpWin_Exit(MONOLITH_UP_WORK *wk);
static void _Write_String( MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk, const s32 mode );

//==============================================================================
//  データ
//==============================================================================
///モノリスパワーPROCデータ
const GFL_PROC_DATA MonolithAppProc_Up_FMission = {
  UpFMisssionProc_Init,
  UpFMisssionProc_Main,
  UpFMisssionProc_End,
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
static GFL_PROC_RESULT UpFMisssionProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
  MONOLITH_UP_WORK *mpw = mywk;
  ARCHANDLE *hdl;
  
  switch(*seq){
  case 0:
    mpw = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_UP_WORK), HEAPID_MONOLITH);
    GFL_STD_MemClear(mpw, sizeof(MONOLITH_UP_WORK));
    
    //BG
    _Setup_BGFrameSetting();
    _Setup_BGGraphicLoad(appwk, appwk->setup);
    _Setup_BmpWin_Create(appwk->setup, mpw);
    // 文字列表示
    _Write_String( appwk->setup, mpw, 0 );
    
    (*seq)++;
    break;
  case 1:
    {
      int i, no_trans = FALSE;
      
      for(i = 0; i < BMPWIN_MAX; i++){
        if(PRINT_UTIL_Trans(&mpw->print_util[i], appwk->setup->printQue) == FALSE){
          no_trans = TRUE;
        }
      }
      if(no_trans == FALSE){
        GFL_BG_SetVisible(GFL_BG_FRAME0_M, VISIBLE_ON);
        GFL_BG_SetVisible(GFL_BG_FRAME2_M, VISIBLE_ON);
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
static GFL_PROC_RESULT UpFMisssionProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
  MONOLITH_UP_WORK *mpw = mywk;
  int i;
  u8 trans_ret[BMPWIN_MAX];
  
  if(appwk->up_proc_finish == TRUE){// || appwk->force_finish == TRUE){
    return GFL_PROC_RES_FINISH;
  }
  
  for(i = 0; i < BMPWIN_MAX; i++){
    trans_ret[i] = PRINT_UTIL_Trans(&mpw->print_util[i], appwk->setup->printQue);
  }
  //
  switch(*seq){
  case 0:
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
static GFL_PROC_RESULT UpFMisssionProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
  MONOLITH_UP_WORK *mpw = mywk;
  
  if(PRINTSYS_QUE_IsFinished(appwk->setup->printQue) == FALSE){
    return GFL_PROC_RES_CONTINUE;
  }
  
  GFL_DISP_GX_SetVisibleControlDirect(GX_PLANEMASK_BG3);
  
  //BG
  _Setup_BGFrameExit();
  _Setup_BmpWin_Exit( mpw );
  
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
	// (0:文字、2:下地、3:BG(共通))
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
	//
	GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME2_M,   &bgcnt_frame[1],   GFL_BG_MODE_TEXT );
	//
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
static void _Setup_BGGraphicLoad(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup)
{
  GFL_ARCHDL_UTIL_TransVramScreen(setup->hdl, NARC_monolith_mono_bgu_fmission_lz_NSCR, 
    GFL_BG_FRAME2_M, 0, 0, TRUE, HEAPID_MONOLITH);
  //
  GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   setup		
 * @param   wk		
 */
//--------------------------------------------------------------
static void _Setup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk)
{
  s32   i;
  const MONOLITH_CANVAS_DATA  *adr = &mono_canvas_tbl[0];
  
  for(i=0;i<BMPWIN_MAX;i++){
    // 終了条件
    if(adr->frmnumber == 0xff){
      break;
    }
    // セット
    wk->bmpwin[i] = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
    //
    GFL_BMPWIN_TransVramCharacter( wk->bmpwin[i] );
    GFL_BMPWIN_MakeScreen( wk->bmpwin[i] );
    //
    adr++;
  }
  // 関連付け
  for(i=0;i<BMPWIN_MAX;i++){
    PRINT_UTIL_Setup(&wk->print_util[i], wk->bmpwin[i]);
  }
  GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   wk
 */
//--------------------------------------------------------------
static void _Setup_BmpWin_Exit(MONOLITH_UP_WORK *wk)
{
  int i;
  
  for(i=0;i<BMPWIN_MAX;i++){
    GFL_BMPWIN_Delete(wk->bmpwin[i]);
  }
}

//--------------------------------------------------------------
/**
 * 文字列表示
 *
 * @param   setup		
 * @param   wk
 */
//--------------------------------------------------------------
static void _Write_String( MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk, const s32 mode )
{
  s32       buf[4];
  s32       i;
  s32       width;
  STRBUF	*str;
  STRBUF	*expbuf;
  
  // 初期化
  if(mode == 0){
    // 固定のものはそのまま表示
    for(i=0;i<BMPWIN_MAX;i++){
      if(mono_mes_tbl[i*2+1] != -1){
        str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[i*2] );
        PRINT_UTIL_Print(&wk->print_util[i], setup->printQue, 0, mono_mes_tbl[i*2+1], str, setup->font_handle);
        GFL_STR_DeleteBuffer(str);
      }
    }
  }
  // 置き換えること
  buf[0] = 59;
  buf[1] = 58;
  buf[2] = 9999;
  buf[3] = 99;
  
  //-----------
  // 説明文(仮)
  //-----------
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->print_util[BMPWIN_EXPLAIN].win ), 0 );
  str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[0] );		// test
  PRINT_UTIL_Print(&wk->print_util[BMPWIN_EXPLAIN], setup->printQue, 0, 0, str, setup->font_handle);
  GFL_STR_DeleteBuffer(str);
  
  //-----------
  // 制限時間
  //-----------
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->print_util[BMPWIN_TIME_R].win ), 0 );
  WORDSET_RegisterNumber(setup->wordset, 0, buf[0], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_RegisterNumber(setup->wordset, 1, buf[1], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  expbuf = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
  str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[BMPWIN_TIME_R*2] );
  WORDSET_ExpandStr( setup->wordset, expbuf, str );
  // 右そろえ
  width = PRINTSYS_GetStrWidth( expbuf, setup->font_handle, 0 );
  PRINT_UTIL_Print(&wk->print_util[BMPWIN_TIME_R], setup->printQue, 120-width, 0, expbuf, setup->font_handle);
  GFL_STR_DeleteBuffer(str);
  GFL_STR_DeleteBuffer(expbuf);
  //
  //-----------
  // クリア目標
  //-----------
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->print_util[BMPWIN_CLEAR_R].win ), 0 );
  WORDSET_RegisterNumber(setup->wordset, 0, buf[2], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  expbuf = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
  str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[BMPWIN_CLEAR_R*2] );
  WORDSET_ExpandStr( setup->wordset, expbuf, str );
  // 右そろえ
  width = PRINTSYS_GetStrWidth( expbuf, setup->font_handle, 0 );
  PRINT_UTIL_Print(&wk->print_util[BMPWIN_CLEAR_R], setup->printQue, 120-width, 0, expbuf, setup->font_handle);
  GFL_STR_DeleteBuffer(str);
  GFL_STR_DeleteBuffer(expbuf);
  //
  //-----------
  // 報酬(仮)
  //-----------
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->print_util[BMPWIN_GAIN_R].win ), 0 );
  WORDSET_RegisterItemName( setup->wordset, 0, 1 );		// 決めうち(要修正)
  WORDSET_RegisterNumber(setup->wordset, 1, buf[3], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  expbuf = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
  str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[BMPWIN_GAIN_R*2] );
  WORDSET_ExpandStr( setup->wordset, expbuf, str );
  // 右そろえ
  width = PRINTSYS_GetStrWidth( expbuf, setup->font_handle, 0 );
  PRINT_UTIL_Print(&wk->print_util[BMPWIN_GAIN_R], setup->printQue, 120-width, 0, expbuf, setup->font_handle);
  GFL_STR_DeleteBuffer(str);
  GFL_STR_DeleteBuffer(expbuf);
  //
  //-----------
  // ハイスコア
  //-----------
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->print_util[BMPWIN_SCORE_R].win ), 0 );
  WORDSET_RegisterNumber(setup->wordset, 0, buf[2], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  expbuf = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
  str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[BMPWIN_SCORE_R*2] );
  WORDSET_ExpandStr( setup->wordset, expbuf, str );
  // 右そろえ
  width = PRINTSYS_GetStrWidth( expbuf, setup->font_handle, 0 );
  PRINT_UTIL_Print(&wk->print_util[BMPWIN_SCORE_R], setup->printQue, 120-width, 0, expbuf, setup->font_handle);
  GFL_STR_DeleteBuffer(str);
  GFL_STR_DeleteBuffer(expbuf);
}

