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
#include "gamesystem/fest_mission_data.h"
#include "monolith.naix"
//
#include "savedata/fmission_save.h"

//==============================================================================
//  定数定義
//==============================================================================
enum{
  BMPWIN_TITLE,        ///< タイトル
  BMPWIN_EXPLAIN,      ///< 説明
  //
  BMPWIN_TIME_L,       ///< 制限時間(左)
  BMPWIN_CLEAR_L,      ///< クリア目標(左)
  BMPWIN_SCORE_L,      ///< ハイスコア(左)
  //
  BMPWIN_TIME_R,       ///< 制限時間(右)
  BMPWIN_CLEAR_R,      ///< クリア目標(右)
  BMPWIN_SCORE_R,      ///< ハイスコア(右)(トータル)
  BMPWIN_SCORE_R2,     ///< ハイスコア(右)(マイスコア)
  //
  BMPWIN_MAX,
};

//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  GFL_BMPWIN 			*bmpwin[BMPWIN_MAX];	// bmpwin
  PRINT_UTIL 			print_util[BMPWIN_MAX];	// print_util
  
  FMISSION_DATA_MAN		*FMissionManager;		// manager
  ARCHANDLE				*Handle;
  FMISSION_ARCDATA		*ArcData;
  //
  s32  					Id;						// 表示するID
  s32					Level;					// 表示するレベル
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
  { GFL_BG_FRAME0_M,	2,	19,	12,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// ハイスコア(左)
  //
  { GFL_BG_FRAME0_M,   14,	15,	15,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// 制限時間(右)
  { GFL_BG_FRAME0_M,   14,	17,	15,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// クリア目標(右)
  { GFL_BG_FRAME0_M,   14,	19,	15,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// ハイスコア(右)(トータル)
  { GFL_BG_FRAME0_M,   14,	21,	15,	2,	MONOLITH_BG_UP_FONT_PALNO,},	// ハイスコア(右)(マイスコア)
  
  { 0xff, 0, 0, 0, 0, 0,}, // end code
};

// 文字列、offset(-1はなし),opt0,opt1
const s16 mono_mes_tbl[] = {
  msg_mono_fmission_010, 4,0,0,		// ミッションせつめい
  0,					-1,0,0,		// せつめい文(dummy)
  //
  msg_mono_fmission_011, 0,0,0,		// 制限時間
  msg_mono_fmission_012, 0,0,0,		// クリア目標
  msg_mono_fmission_026,-1,0,0,		// さいこうきろく
  //
  msg_mono_fmission_015,-1,msg_mono_fmission_019,0,		// 制限時間(特別処理)
  msg_mono_fmission_016,-1,0,0,		// クリア目標(mono_mes_clear_tblを使用のため未使用)
  msg_mono_fmission_016,-1,msg_mono_fmission_024,0,		// ハイスコア(トータル)
  msg_mono_fmission_016,-1,msg_mono_fmission_025,0,		// ハイスコア(マイスコア)
};

// クリア目標(単位を変更するため:FMISSION_UNIT_IDと対応)
const s16 mono_mes_clear_tbl[] = {
  msg_mono_fmission_023,			// なし
  msg_mono_fmission_021,			// 「にん」
  msg_mono_fmission_016,			// 「ひき」
  msg_mono_pow_001,					// 「こ」
  msg_mono_fmission_022,			// 「かい」
};

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT UpFMisssionProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT UpFMisssionProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT UpFMisssionProc_End(  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
//
static void up_fmission_init( MONOLITH_APP_PARENT *appwk, MONOLITH_UP_WORK *wk );
//
static void _Setup_BGFrameSetting(void);
static void _Setup_BGFrameExit(void);
static void _Setup_BGGraphicLoad(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup);

static void _Setup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk);
static void _Setup_BmpWin_Exit(MONOLITH_UP_WORK *wk);
static void _Write_String( MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk, const s32 mode );

//
static void fmission_init( MONOLITH_UP_WORK *wk );
static void fmission_exit( MONOLITH_UP_WORK *wk );
//
static void fmission_title( MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk );
static void fmission_explain( MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk, const s32 id, const s32 level );
static void fmission_string(  MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk, const s32 id, const s32 level, const s32 mode );
static void fmission_highscore( MONOLITH_APP_PARENT *appwk, MONOLITH_UP_WORK *wk, const s32 id );
//
static void fmission_clear_string( MONOLITH_UP_WORK *wk, const s32 mode );
static void fmission_clear_all( MONOLITH_UP_WORK *wk );

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
    
    // 初期化まとめ
    up_fmission_init( appwk, mpw );
    //
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
  int flg=0;
  u8 trans_ret[BMPWIN_MAX];
  
  if(appwk->up_proc_finish == TRUE){// || appwk->force_finish == TRUE){
    return GFL_PROC_RES_FINISH;
  }
  
  for(i = 0; i < BMPWIN_MAX; i++){
    trans_ret[i] = PRINT_UTIL_Trans(&mpw->print_util[i], appwk->setup->printQue);
  }
  //
  // 一つ前のデータと比較
  if((appwk->common->fmission_id != mpw->Id)||(appwk->common->fmission_level != mpw->Level)){
//    OS_Printf("id : %d -> %d(level :%d)\n",mpw->Id, appwk->common->fmission_id, appwk->common->fmission_level);
    mpw->Id    = appwk->common->fmission_id;
    mpw->Level = appwk->common->fmission_level;
    // 表示
    if(mpw->Id != 0xff){
      _Write_String( appwk, appwk->setup, mpw, 1 );
    }
    // (タイトルを除き)クリア
    else{
      fmission_clear_all( mpw );
    }
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
  
  // フェスミッション破棄
  fmission_exit( mpw );
  
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
 * @brief   初期化まとめ
 */
//--------------------------------------------------------------
static void up_fmission_init( MONOLITH_APP_PARENT *appwk, MONOLITH_UP_WORK *wk )
{
  wk->Id    = 0xff;
  wk->Level = 0;
  //
  appwk->common->fmission_id    = 0xff;
  appwk->common->fmission_level = 0;
  
  //BG
  _Setup_BGFrameSetting();
  _Setup_BGGraphicLoad( appwk, appwk->setup );
  _Setup_BmpWin_Create( appwk->setup, wk );
  
  // フェスミッション初期化
  fmission_init( wk );
  
  // タイトル表示
  fmission_title( appwk->setup, wk );
}

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
 * 文字列表示まとめ
 *
 * @param   MONOLITH_APP_PARENT	*appwk
 * @param   MONOLITH_SETUP		*setup
 * @param   MONOLITH_UP_WORK	*wk
 * @param   const s32	(0:初期化、1:通常)(未使用)
 */
//--------------------------------------------------------------
static void _Write_String( MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk, const s32 mode )
{
  s32       i;
  STRBUF	*str;
  
  // 固定のものはそのまま表示
  for(i=0;i<BMPWIN_MAX;i++){
    if(i != 0){		// タイトルは除く
      if(mono_mes_tbl[i*4+1] != -1){
        str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[i*4] );
        PRINT_UTIL_Print(&wk->print_util[i], setup->printQue, 0, mono_mes_tbl[i*4+1], str, setup->font_handle);
        GFL_STR_DeleteBuffer(str);
      }
    }
  }
  // 説明文
  fmission_explain( setup, wk, wk->Id, wk->Level );
  
  // 制限時間
  fmission_string( setup, wk, wk->Id, wk->Level, BMPWIN_TIME_R );
  //
  // クリア目標
  fmission_string( setup, wk, wk->Id, wk->Level, BMPWIN_CLEAR_R );
  //
  // 報酬
//  fmission_string( setup, wk, wk->Id, wk->Level, BMPWIN_GAIN_R );
  //
  // ハイスコア
  fmission_highscore( appwk, wk, wk->Id );
}

//--------------------------------------------------------------
/**
 * ミッション処理初期化(念のため、別にしておく)
 *
 * @param   MONOLITH_UP_WORK	*wk
 */
//--------------------------------------------------------------
static void fmission_init( MONOLITH_UP_WORK *wk )
{
  // 初期化
  wk->FMissionManager = FMISSION_DATA_Init( HEAPID_MONOLITH );
  wk->Handle  = GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, HEAPID_MONOLITH );
  wk->ArcData = FMISSION_DATA_AllocArcAllByHandle( wk->Handle, HEAPID_MONOLITH );
}

//--------------------------------------------------------------
/**
 * ミッション処理破棄(念のため、別にしておく)
 *
 * @param   MONOLITH_UP_WORK	*wk
 */
//--------------------------------------------------------------
static void fmission_exit( MONOLITH_UP_WORK *wk )
{
  // 破棄
  FMISSION_DATA_Release( wk->FMissionManager );
  wk->FMissionManager = 0;
  //
  GFL_ARC_CloseDataHandle( wk->Handle );
  //
  GFL_HEAP_FreeMemory( wk->ArcData );
}

//--------------------------------------------------------------
/**
 * タイトル表示
 *
 * @param   MONOLITH_SETUP		*setup
 * @param   MONOLITH_UP_WORK	*wk
 */
//--------------------------------------------------------------
static void fmission_title( MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk )
{
  STRBUF  *str;
  
  str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[BMPWIN_TITLE*4] );
  PRINT_UTIL_Print(&wk->print_util[BMPWIN_TITLE], setup->printQue, 0, mono_mes_tbl[BMPWIN_TITLE*4+1], str, setup->font_handle );
  GFL_STR_DeleteBuffer(str);
}

//--------------------------------------------------------------
/**
 * 説明文
 *
 * @param   MONOLITH_SETUP			*setup
 * @param   MONOLITH_UP_WORK		*wk
 * @param   const s32				(id)
 * @param   const s32				(level)
 */
//--------------------------------------------------------------
static void fmission_explain( MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk, const s32 id, const s32 level )
{
  STRBUF		*str;
  FEST_MISSION  fmwk;
  //
  // 一旦クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->print_util[BMPWIN_EXPLAIN].win ), 0 );
  //
  // FEST_MISSION取得(?)
  FMISSION_DATA_GetMissionParam( wk->FMissionManager, id, level, &fmwk );
  // 説明文取得
  str = GFL_STR_CreateBuffer( 256, HEAPID_MONOLITH );
  FMISSION_DATA_GetExpStr( wk->FMissionManager, str, &fmwk, HEAPID_MONOLITH );
  PRINT_UTIL_Print(&wk->print_util[BMPWIN_EXPLAIN], setup->printQue, 0, 0, str, setup->font_handle);
  GFL_STR_DeleteBuffer(str);
}

//--------------------------------------------------------------
/**
 * 文字列表示まとめ
 *
 * @param   MONOLITH_SETUP			*setup
 * @param   MONOLITH_UP_WORK		*wk
 * @param   const s32				(id)
 * @param   const s32				(level)
 * @param   const s32				種類
 */
//--------------------------------------------------------------
static void fmission_string( MONOLITH_SETUP *setup, MONOLITH_UP_WORK *wk, const s32 id, const s32 level, const s32 mode )
{
  u16		width;
  u16		buf[4];
  STRBUF	*str;
  STRBUF	*expbuf;
  
  // 一旦クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->print_util[mode].win ), 0 );
  
  // 制限時間
  if(mode == BMPWIN_TIME_R){
    // 制限時間の取得
    buf[0] = wk->ArcData[wk->Id].param.level[wk->Level].time;
    buf[1] = buf[0]/60;		// 分
    buf[2] = buf[0]%60;		// 秒
    //
    WORDSET_RegisterNumber(setup->wordset, 0, buf[1], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    WORDSET_RegisterNumber(setup->wordset, 1, buf[2], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    expbuf = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
    if(buf[2] != 0){		// 通常
      str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[mode*4] );
    }else{					// 秒がない
      str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[mode*4+2] );
    }
    WORDSET_ExpandStr( setup->wordset, expbuf, str );
  }
  // クリア目標
  else if(mode == BMPWIN_CLEAR_R){
    // 目標取得
    buf[0] = wk->ArcData[wk->Id].param.level[wk->Level].goal;
    // 単位
    buf[1] = wk->ArcData[wk->Id].common.unit;
    //
    WORDSET_RegisterNumber(setup->wordset, 0, buf[0], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    expbuf = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
    str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_clear_tbl[buf[1]] );
    WORDSET_ExpandStr( setup->wordset, expbuf, str );
  }
#if 0
  // 報酬
  else if(mode == BMPWIN_GAIN_R){
    // 報酬アイテム取得
    buf[0] = wk->ArcData[wk->Id].param.item;
    // 報酬数取得
    buf[1] = wk->ArcData[wk->Id].param.level[wk->Level].pay;
    //
    WORDSET_RegisterItemName( setup->wordset, 0, buf[0] );
    WORDSET_RegisterNumber(   setup->wordset, 1, buf[1], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    expbuf = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
    if(wk->ArcData[wk->Id].param.pay_bonus == FMISSION_PAY_BONUS_NONE){
      str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[mode*4] );
    }else{
      str = GFL_MSG_CreateString( setup->mm_monolith, mono_mes_tbl[mode*4+2] );
    }
    WORDSET_ExpandStr( setup->wordset, expbuf, str );
  }
#endif
  // ハイスコア(別で対応)
  else if(mode == BMPWIN_SCORE_R){
  }
  //
  // ここまででexpbufにセットされているものとする
  // 右そろえ
  width = PRINTSYS_GetStrWidth( expbuf, setup->font_handle, 0 );
  PRINT_UTIL_Print(&wk->print_util[mode], setup->printQue, 120-width, 0, expbuf, setup->font_handle);
  GFL_STR_DeleteBuffer(str);
  GFL_STR_DeleteBuffer(expbuf);
}

//--------------------------------------------------------------
/**
 * ハイスコア表示
 *
 * @param   MONOLITH_APP_PARENT		*appwk
 * @param   MONOLITH_UP_WORK		*wk
 * @param   const s32				(id)
 */
//--------------------------------------------------------------
static void fmission_highscore( MONOLITH_APP_PARENT *appwk, MONOLITH_UP_WORK *wk, const s32 id )
{
  u8           buf[2];
  u16          time[2];
  s32          lv;
  s32          flg=0;
  s32          dflg=0;
//  s32          num;
  s32          width;
  STRBUF       *str;
  STRBUF       *expbuf;
  FEST_MISSION fmwk;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(appwk->parent->gsys);
  SAVE_CONTROL_WORK *work = GAMEDATA_GetSaveControlWork( gamedata );
  FMISSION_SAVE *fsave = SAVEDATA_GetFMissionSave( work );
  //
  // レベルは問わない
  FMISSION_DATA_GetMissionParam( wk->FMissionManager, id, 0, &fmwk );
  //
  // 仕様変更により、必ずスコアを表示
  // マイスコアが0の場合は非表示
  time[0] = FMISSION_SAVE_GetMissionStatusHighScoreMine( fsave, id );
  if(time[0] == 0){
    dflg = 1;
  }else{
    // トータル
    time[1] = FMISSION_SAVE_GetMissionStatusHighScore( fsave, id );
  }
  //
  // 一旦クリア
  fmission_clear_string( wk, BMPWIN_SCORE_L );
  fmission_clear_string( wk, BMPWIN_SCORE_R );
  fmission_clear_string( wk, BMPWIN_SCORE_R2 );
  //
  if(dflg == 0){
    str = GFL_MSG_CreateString( appwk->setup->mm_monolith, mono_mes_tbl[BMPWIN_SCORE_L*4] );
    PRINT_UTIL_Print(&wk->print_util[BMPWIN_SCORE_L], appwk->setup->printQue, 0, 0, str, appwk->setup->font_handle);
    GFL_STR_DeleteBuffer(str);
    
    //----------------
    // トータル表示
    //----------------
    // 仕様変更により、必ずスコアを表示
    WORDSET_RegisterNumber(appwk->setup->wordset, 0, time[1], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    lv = wk->ArcData[wk->Id].common.unit;
    str = GFL_MSG_CreateString( appwk->setup->mm_monolith, mono_mes_clear_tbl[lv] );
    expbuf = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
    WORDSET_ExpandStr( appwk->setup->wordset, expbuf, str );
    GFL_STR_DeleteBuffer(str);
    // 右そろえ(数値+単位)
    width = PRINTSYS_GetStrWidth( expbuf, appwk->setup->font_handle, 0 );
    PRINT_UTIL_Print(&wk->print_util[BMPWIN_SCORE_R], appwk->setup->printQue, 120-width, 0, expbuf, appwk->setup->font_handle);
    GFL_STR_DeleteBuffer(expbuf);
    // 「トータル」表示
    str = GFL_MSG_CreateString( appwk->setup->mm_monolith, mono_mes_tbl[BMPWIN_SCORE_R*4+2] );
    PRINT_UTIL_Print(&wk->print_util[BMPWIN_SCORE_R], appwk->setup->printQue, 16, 0, str, appwk->setup->font_handle);
    GFL_STR_DeleteBuffer(str);
    //
    //----------------
    // マイスコア表示
    //----------------
    WORDSET_RegisterNumber(appwk->setup->wordset, 0, time[0], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    str = GFL_MSG_CreateString( appwk->setup->mm_monolith, mono_mes_clear_tbl[lv] );
    expbuf = GFL_STR_CreateBuffer( 64, HEAPID_MONOLITH );
    WORDSET_ExpandStr( appwk->setup->wordset, expbuf, str );
    GFL_STR_DeleteBuffer(str);
    // 右そろえ(数値+単位)
    width = PRINTSYS_GetStrWidth( expbuf, appwk->setup->font_handle, 0 );
    PRINT_UTIL_Print(&wk->print_util[BMPWIN_SCORE_R2], appwk->setup->printQue, 120-width, 0, expbuf, appwk->setup->font_handle);
    GFL_STR_DeleteBuffer(expbuf);
    // 「マイスコア」表示
    str = GFL_MSG_CreateString( appwk->setup->mm_monolith, mono_mes_tbl[BMPWIN_SCORE_R2*4+2] );
    PRINT_UTIL_Print(&wk->print_util[BMPWIN_SCORE_R2], appwk->setup->printQue, 0, 0, str, appwk->setup->font_handle);
    GFL_STR_DeleteBuffer(str);
  }
}

//--------------------------------------------------------------
/**
 * 文字列クリア
 *
 * @param   MONOLITH_UP_WORK		*wk
 * @param   const s32				id
 */
//--------------------------------------------------------------
static void fmission_clear_string( MONOLITH_UP_WORK *wk, const s32 mode )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->print_util[mode].win ), 0 );
  wk->print_util[mode].transReq = TRUE;
}

//--------------------------------------------------------------
/**
 * 文字列クリアまとめ
 *
 * @param   MONOLITH_UP_WORK		*wk
 */
//--------------------------------------------------------------
static void fmission_clear_all( MONOLITH_UP_WORK *wk )
{
  s32  i;
  
  // タイトル以外は全部消す
  for(i=0;i<BMPWIN_MAX;i++){
    if(i != BMPWIN_TITLE){
      fmission_clear_string( wk, i );
    }
  }
}

