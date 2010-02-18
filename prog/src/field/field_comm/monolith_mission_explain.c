//==============================================================================
/**
 * @file    monolith_mission_explain.c
 * @brief   モノリス：ミッション説明
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
#include "msg/msg_mission_monolith.h"
#include "mission_types.h"
#include "intrude_mission.h"
#include "intrude_types.h"

#include "monolith.naix"


//==============================================================================
//  定数定義
//==============================================================================
enum{
  BMPWIN_TYPE,        ///<ミッションタイプ名
  BMPWIN_EXPLAIN,     ///<ミッション説明
  
  BMPWIN_MAX,
};

//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  GFL_BMPWIN *bmpwin[BMPWIN_MAX];
  PRINT_UTIL print_util[BMPWIN_MAX];
  u8 view_town;                         ///<ミッション説明を表示中の街番号
  u8 padding[3];
}MONOLITH_MSEXPLAIN_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT MonolithMissionExplainProc_Init(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithMissionExplainProc_Main(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithMissionExplainProc_End(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void _Setup_BGFrameSetting(void);
static void _Setup_BGFrameExit(void);
static void _Setup_BGGraphicLoad(MONOLITH_SETUP *setup);
static void _Setup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_MSEXPLAIN_WORK *mmw);
static void _Setup_BmpWin_Exit(MONOLITH_MSEXPLAIN_WORK *mmw);
static void _Write_MissionExplain(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_MSEXPLAIN_WORK *mmw, int select_town);


//==============================================================================
//  データ
//==============================================================================
///モノリスパワーROCデータ
const GFL_PROC_DATA MonolithAppProc_Up_MissionExplain = {
  MonolithMissionExplainProc_Init,
  MonolithMissionExplainProc_Main,
  MonolithMissionExplainProc_End,
};

///街番号からミッションタイプを取得するテーブル
const u32 TownNo_to_Type[] = {
  MISSION_TYPE_PERSONALITY, ///<性格
  MISSION_TYPE_VICTORY,     ///<勝利(LV)
  MISSION_TYPE_OCCUR,       ///<発生(エンカウント)
  MISSION_TYPE_SKILL,       ///<技
  MISSION_TYPE_SIZE,        ///<大きさ
  MISSION_TYPE_BASIC,       ///<基礎
  MISSION_TYPE_ITEM,        ///<道具
  MISSION_TYPE_ATTRIBUTE,   ///<属性
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
static GFL_PROC_RESULT MonolithMissionExplainProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_MSEXPLAIN_WORK *mmw = mywk;
	ARCHANDLE *hdl;
  
  mmw = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_MSEXPLAIN_WORK), HEAPID_MONOLITH);
  GFL_STD_MemClear(mmw, sizeof(MONOLITH_MSEXPLAIN_WORK));
  
  //BG
  _Setup_BGFrameSetting();
  _Setup_BGGraphicLoad(appwk->setup);
  _Setup_BmpWin_Create(appwk->setup, mmw);
  
  mmw->view_town = 0xff;  //初回の描画を通す為に存在しない街番号を設定
  
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
static GFL_PROC_RESULT MonolithMissionExplainProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_MSEXPLAIN_WORK *mmw = mywk;
  int i;
  int print_finish_count = 0;

  if(appwk->up_proc_finish == TRUE || appwk->force_finish == TRUE){
    return GFL_PROC_RES_FINISH;
  }
  
  for(i = 0; i < BMPWIN_MAX; i++){
    if(PRINT_UTIL_Trans(&mmw->print_util[i], appwk->setup->printQue) == TRUE){
      print_finish_count++;
    }
  }
  if(print_finish_count == BMPWIN_MAX){
    if(mmw->view_town != appwk->common->mission_select_town){
      _Write_MissionExplain(appwk, appwk->setup, mmw, appwk->common->mission_select_town);
      mmw->view_town = appwk->common->mission_select_town;
    }
  }
  
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
static GFL_PROC_RESULT MonolithMissionExplainProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_MSEXPLAIN_WORK *mmw = mywk;
  
  //BG
  _Setup_BmpWin_Exit(mmw);
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
	GFL_ARCHDL_UTIL_TransVramScreen(setup->hdl, NARC_monolith_mono_bgu_mission_lz_NSCR, 
		GFL_BG_FRAME2_M, 0, 0, TRUE, HEAPID_MONOLITH);

	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   setup		
 * @param   mmw		
 */
//--------------------------------------------------------------
static void _Setup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_MSEXPLAIN_WORK *mmw)
{
  int i;
  
  //ミッションタイプ名
  mmw->bmpwin[BMPWIN_TYPE] = GFL_BMPWIN_Create(GFL_BG_FRAME0_M, 2, 0, 28, 3, 
    MONOLITH_BG_UP_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( mmw->bmpwin[BMPWIN_TYPE] );
  GFL_BMPWIN_MakeScreen( mmw->bmpwin[BMPWIN_TYPE] );

  //ミッション説明
  mmw->bmpwin[BMPWIN_EXPLAIN] = GFL_BMPWIN_Create(GFL_BG_FRAME0_M, 2, 5, 28, 16, 
    MONOLITH_BG_UP_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( mmw->bmpwin[BMPWIN_EXPLAIN] );
  GFL_BMPWIN_MakeScreen( mmw->bmpwin[BMPWIN_EXPLAIN] );
  
  for(i = 0; i < BMPWIN_MAX; i++){
    PRINT_UTIL_Setup(&mmw->print_util[i], mmw->bmpwin[i]);
  }
  
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   mmw		
 */
//--------------------------------------------------------------
static void _Setup_BmpWin_Exit(MONOLITH_MSEXPLAIN_WORK *mmw)
{
  GFL_BMPWIN_Delete(mmw->bmpwin[BMPWIN_TYPE]);
  GFL_BMPWIN_Delete(mmw->bmpwin[BMPWIN_EXPLAIN]);
}

//--------------------------------------------------------------
/**
 * ミッション説明描画
 *
 * @param   appwk		
 * @param   setup		
 * @param   mmw		
 * @param   select_town		
 */
//--------------------------------------------------------------
static void _Write_MissionExplain(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_MSEXPLAIN_WORK *mmw, int select_town)
{
  STRBUF *str_type, *str_explain, *expand_explain;
  u32 explain_msgid;
  const MISSION_DATA *mdata;
  
  expand_explain = GFL_STR_CreateBuffer( 256, HEAPID_MONOLITH );

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(mmw->bmpwin[BMPWIN_TYPE]), 0x0000);
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(mmw->bmpwin[BMPWIN_EXPLAIN]), 0x0000);

  if(select_town == SELECT_TOWN_ENFORCEMENT 
      && MISSION_RecvCheck(&appwk->parent->intcomm->mission) == TRUE){  //念のため2重にチェック
    //ミッション実施中
    mdata = MISSION_GetRecvData(&appwk->parent->intcomm->mission);
  }
  else{
    //ミッション選択中
    if(select_town > NELEMS(TownNo_to_Type)){
      select_town = 0;
    }
    mdata = &appwk->parent->list.md[TownNo_to_Type[select_town]];
  }

  explain_msgid = mdata->cdata.msg_id_contents_monolith;
  
  str_type = GFL_MSG_CreateString(setup->mm_mission_mono, msg_mistype_000 + mdata->cdata.type);
  str_explain = GFL_MSG_CreateString(setup->mm_mission_mono, explain_msgid);
  
  MISSIONDATA_Wordset(appwk->parent->intcomm, mdata, setup->wordset, HEAPID_MONOLITH);
  WORDSET_ExpandStr(setup->wordset, expand_explain, str_explain );

  PRINT_UTIL_Print(&mmw->print_util[BMPWIN_TYPE], setup->printQue, 
    0, 4, str_type, setup->font_handle);
  PRINT_UTIL_Print(&mmw->print_util[BMPWIN_EXPLAIN], setup->printQue, 
    0, 0, expand_explain, setup->font_handle);
  
  GFL_STR_DeleteBuffer(str_type);
  GFL_STR_DeleteBuffer(str_explain);
  GFL_STR_DeleteBuffer(expand_explain);
}

