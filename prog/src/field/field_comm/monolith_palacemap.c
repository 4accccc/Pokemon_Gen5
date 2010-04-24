//==============================================================================
/**
 * @file    monolith_palacemap.c
 * @brief   モノリス：パレスマップ
 * @author  matsuda
 * @date    2009.11.25(水)
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
///バランスゲージ描画用の設定値
enum{
  BALANCE_GAUGE_CHARA_MAX = 30,     ///<バランスゲージの横長キャラ数
  BALANCE_GAUGE_DOTTO_MAX = BALANCE_GAUGE_CHARA_MAX * 8,    ///<バランスゲージのドット数
  BALANCE_SCRN_START_X = 1,
  BALANCE_SCRN_START_Y = 0x14,
  BALANCE_SCRN_BLACK_CHARNO_START = 1,  ///<ブラックゲージのキャラクタNo開始位置
  BALANCE_SCRN_MAXWHITE_CHARNO = 9,     ///<ホワイトゲージ(MAX)のキャラクタNo開始位置
  ONECHARA_DOT = 8,   ///<1つのキャラクタのドット数
  BALANCE_GAUGE_PALNO = 1,          ///<バランスゲージのパレット番号
};


//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  MONOLITH_BMPSTR bmpstr_title;
  GFL_CLWK *act_town[MISSION_LIST_MAX]; ///<街アイコンアクターへのポインタ
}MONOLITH_PALACEMAP_WORK;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT MonolithPalaceMapProc_Init(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithPalaceMapProc_Main(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithPalaceMapProc_End(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void _Setup_BGFrameSetting(void);
static void _Setup_BGFrameExit(void);
static void _Setup_BGGraphicLoad(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup);
static void _TownIcon_AllCreate(MONOLITH_PALACEMAP_WORK *mpw, MONOLITH_APP_PARENT *appwk);
static void _TownIcon_AllDelete(MONOLITH_PALACEMAP_WORK *mpw);
static void _TownIcon_AllUpdate(MONOLITH_PALACEMAP_WORK *mpw, MONOLITH_APP_PARENT *appwk);
static void _BalanceGaugeRewrite(MONOLITH_APP_PARENT *appwk);


//==============================================================================
//  データ
//==============================================================================
///モノリスパワーPROCデータ
const GFL_PROC_DATA MonolithAppProc_Up_PalaceMap = {
  MonolithPalaceMapProc_Init,
  MonolithPalaceMapProc_Main,
  MonolithPalaceMapProc_End,
};

//--------------------------------------------------------------
//  街アイコン
//--------------------------------------------------------------
///街アイコン表示座標テーブル
static const GFL_POINT TownIconPosTbl[] = {
  {11*8, 6*8},
  {0x15*8, 6*8},

  {8*8, 10*8},
  {0x18*8, 10*8},

  {11*8, 0xe*8},
  {0x15*8, 0xe*8},
};
SDK_COMPILER_ASSERT(NELEMS(TownIconPosTbl) == MISSION_LIST_MAX);




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
static GFL_PROC_RESULT MonolithPalaceMapProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_PALACEMAP_WORK *mpw = mywk;
	ARCHANDLE *hdl;
  
  switch(*seq){
  case 0:
    mpw = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_PALACEMAP_WORK), HEAPID_MONOLITH);
    GFL_STD_MemClear(mpw, sizeof(MONOLITH_PALACEMAP_WORK));
    
    //BG
    _Setup_BGFrameSetting();
    _Setup_BGGraphicLoad(appwk, appwk->setup);
    //OBJ
    {
      MYSTATUS *myst = MonolithTool_GetMystatus(appwk);

      STRBUF *strbuf = 	GFL_STR_CreateBuffer(PERSON_NAME_SIZE + EOM_SIZE, HEAPID_MONOLITH);

    	GFL_STR_SetStringCodeOrderLength(
    	  strbuf, MyStatus_GetMyName(myst), PERSON_NAME_SIZE + EOM_SIZE);
      WORDSET_RegisterWord(appwk->setup->wordset, 0, strbuf, MyStatus_GetMySex(myst), TRUE, PM_LANG);

      GFL_STR_DeleteBuffer(strbuf);
    }
    MonolithTool_Bmpoam_Create(appwk->setup, &mpw->bmpstr_title, COMMON_RESOURCE_INDEX_UP, 
      128, 12, 30, 2, msg_mono_title_000, appwk->setup->wordset);
    _TownIcon_AllCreate(mpw, appwk);
    
    (*seq)++;
    break;
  case 1:
  	GFL_BG_SetVisible(GFL_BG_FRAME2_M, VISIBLE_ON);
  	GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
  	return GFL_PROC_RES_FINISH;
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
static GFL_PROC_RESULT MonolithPalaceMapProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_PALACEMAP_WORK *mpw = mywk;
  int tp_ret;
  
  if(appwk->up_proc_finish == TRUE || appwk->force_finish == TRUE){
    return GFL_PROC_RES_FINISH;
  }
  
  MonolithTool_Bmpoam_TransUpdate(appwk->setup, &mpw->bmpstr_title);
  _TownIcon_AllUpdate(mpw, appwk);

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
static GFL_PROC_RESULT MonolithPalaceMapProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_PALACEMAP_WORK *mpw = mywk;
  
  GFL_DISP_GX_SetVisibleControlDirect(GX_PLANEMASK_BG3);

  //OBJ
  _TownIcon_AllDelete(mpw);
  MonolithTool_Bmpoam_Delete(&mpw->bmpstr_title);
  //BG
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
		{//GFL_BG_FRAME2_M
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, MONO_BG_COMMON_CHARBASE, MONO_BG_COMMON_CHAR_SIZE,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		},
	};

	GFL_BG_SetBGControl( GFL_BG_FRAME2_M,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );

	GFL_BG_FillScreen( GFL_BG_FRAME2_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
}

//--------------------------------------------------------------
/**
 * BGフレーム解放
 */
//--------------------------------------------------------------
static void _Setup_BGFrameExit(void)
{
	GFL_BG_SetVisible(GFL_BG_FRAME2_M, VISIBLE_OFF);
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
	GFL_ARCHDL_UTIL_TransVramScreen(setup->hdl, NARC_monolith_mono_bgu_map_lz_NSCR, 
		GFL_BG_FRAME2_M, 0, 0, TRUE, HEAPID_MONOLITH);
  _BalanceGaugeRewrite(appwk);
  
	GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
}

//--------------------------------------------------------------
/**
 * @brief   街アイコンアクター生成
 */
//--------------------------------------------------------------
static void _TownIcon_AllCreate(MONOLITH_PALACEMAP_WORK *mpw, MONOLITH_APP_PARENT *appwk)
{
  int i;
  OCCUPY_INFO *occupy;
  
  occupy = MonolithTool_GetOccupyInfo(appwk);
  
  for(i = 0; i < MISSION_LIST_MAX; i++){
    mpw->act_town[i] = MonolithTool_TownIcon_Create(
      appwk->setup, occupy, i, &TownIconPosTbl[i], COMMON_RESOURCE_INDEX_UP);
  }
}

//--------------------------------------------------------------
/**
 * @brief   街アイコンアクター削除
 */
//--------------------------------------------------------------
static void _TownIcon_AllDelete(MONOLITH_PALACEMAP_WORK *mpw)
{
  int i;
  
  for(i = 0; i < MISSION_LIST_MAX; i++){
    MonolithTool_TownIcon_Delete(mpw->act_town[i]);
  }
}

//--------------------------------------------------------------
/**
 * 街アイコン全更新
 *
 * @param   mpw		
 */
//--------------------------------------------------------------
static void _TownIcon_AllUpdate(MONOLITH_PALACEMAP_WORK *mpw, MONOLITH_APP_PARENT *appwk)
{
  int i;
  OCCUPY_INFO *occupy;
  
  occupy = MonolithTool_GetOccupyInfo(appwk);
  for(i = 0; i < MISSION_LIST_MAX; i++){
    MonolithTool_TownIcon_Update(mpw->act_town[i], occupy, i);
  }
}

//--------------------------------------------------------------
/**
 * バランスゲージのスクリーンを書き直す
 *
 * @param   appwk		
 */
//--------------------------------------------------------------
static void _BalanceGaugeRewrite(MONOLITH_APP_PARENT *appwk)
{
  const OCCUPY_INFO *occupy = MonolithTool_GetOccupyInfo(appwk);
  int lv_w, lv_b, lv_total;
  int dot_w, dot_b;
  int pos;
  u16 *scrnbuf;
  
  lv_w = occupy->white_level;
  lv_b = occupy->black_level;
  lv_total = lv_w + lv_b;
  
  if(lv_w == lv_b){ //初期値がレベル0同士の場合があるので
    dot_w = BALANCE_GAUGE_DOTTO_MAX / 2;
    dot_b = BALANCE_GAUGE_DOTTO_MAX - dot_w;
  }
  else{
    dot_w = BALANCE_GAUGE_DOTTO_MAX * lv_w / lv_total;
    if(lv_b > 0){ //小数切捨てによって出た部分がレベル0のブラックに加算されないようにチェック
      dot_b = BALANCE_GAUGE_DOTTO_MAX - dot_w;
    }
    else{
      dot_b = 0;
      dot_w = BALANCE_GAUGE_DOTTO_MAX;
    }
  }
  //レベルが1以上あるなら計算上ドットが0になっていも1ドットは入れる
  if(lv_w > 0 && dot_w == 0){
    dot_w++;
    dot_b--;
  }
  else if(lv_b > 0 && dot_b == 0){
    dot_b++;
    dot_w--;
  }
  
  scrnbuf = GFL_HEAP_AllocClearMemory(HEAPID_MONOLITH, BALANCE_GAUGE_CHARA_MAX * sizeof(u16));
  
  //黒のゲージを描画
  pos = 0;
  while(dot_b > 0){
    if(dot_b >= ONECHARA_DOT){
      scrnbuf[pos] = BALANCE_SCRN_BLACK_CHARNO_START;
      dot_b -= ONECHARA_DOT;
    }
    else{
      scrnbuf[pos] = ONECHARA_DOT - dot_b + BALANCE_SCRN_BLACK_CHARNO_START;
      dot_b = 0;
    }
    pos++;
  }
  //残りを白MAXで埋める
  for( ; pos < BALANCE_GAUGE_CHARA_MAX; pos++){
    scrnbuf[pos] = BALANCE_SCRN_MAXWHITE_CHARNO;
  }
  //パレットNoをスクリーンに入れる
  for(pos = 0; pos < BALANCE_GAUGE_CHARA_MAX; pos++){
    scrnbuf[pos] |= BALANCE_GAUGE_PALNO << 12;
  }
  
  //スクリーン描画
  GFL_BG_WriteScreen( GFL_BG_FRAME2_M, scrnbuf, 
    BALANCE_SCRN_START_X, BALANCE_SCRN_START_Y, BALANCE_GAUGE_CHARA_MAX, 1);
  
  GFL_HEAP_FreeMemory(scrnbuf);
}

