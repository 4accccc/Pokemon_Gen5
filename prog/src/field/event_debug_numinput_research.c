#ifdef PM_DEBUG
////////////////////////////////////////////////////////////////////////
/**
 *  @brief  すれ違い調査関連のグ数値入力
 *  @file   event_debug_numinput_research.c
 *  @author obata
 *  @date   2010.04.16
 */
//////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "fieldmap.h"
#include "field/eventwork.h"
#include "event_debug_local.h"
#include "gamesystem/gamesystem.h"
#include "event_debug_numinput.h"
#include "event_debug_numinput_research.h"

#include "system/main.h"
#include "field/field_msgbg.h"
#include "print/wordset.h"

#include "arc_def.h"
#include "debug_message.naix"
#include "msg/debug/msg_d_numinput_r.h"

#include "savedata/questionnaire_save.h"
#include "app/research_radar/question_id.h"
//#include "app/research_radar/answer_num_question.cdat"

//======================================================================
//======================================================================
///項目選択用データの定義
typedef enum {
  D_NINPUT_DATA_LIST,   ///<固定データ
  D_NINPUT_DATA_BIN,    ///<自動生成データ
}D_NINPUT_DATA_TYPE;

//数値入力メインイベント制御ワーク
typedef struct _DNINPUT_EV_WORK{
	int page;
  HEAPID  heapID;

  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldWork;

  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
}DNINPUT_EV_WORK;

///数値入力リストデータ
typedef struct {
  u16 gmm_id;
  u32 param;
  const DEBUG_NUMINPUT_PARAM * dni_param;
}DEBUG_NUMINPUT_LIST;

///数値入力リスト初期化パラメータ定義
typedef struct{ 

  // リストの最大数
  const u32 count_or_id;
  
  // 各項目の数値入力時処理指定データ
  const DEBUG_NUMINPUT_LIST * pList;

} DEBUG_NUMINPUT_INITIALIZER;



//======================================================================
//プロトタイプ
//======================================================================
static GMEVENT_RESULT dninput_MainEvent( GMEVENT * event, int *seq, void * work);
static void SetResearchDataAtRandom( GAMESYS_WORK* gameSystem );
static void SetResearchDataAtRandom_today( GAMESYS_WORK* gameSystem, u32 rand_max );
static void SetResearchDataAtRandom_total( GAMESYS_WORK* gameSystem, u32 rand_max );
static void ClearResearchData( GAMESYS_WORK* gameSystem );

//--------------------------------------------------------------
///サブメニューイベント：デバッグフラグ操作
//--------------------------------------------------------------
static GMEVENT * DEBUG_EVENT_FLDMENU_FlagWork(
    GAMESYS_WORK * gsys, const DEBUG_NUMINPUT_INITIALIZER * init );

#include "debug_numinput_research.cdat"


static  const DEBUG_NUMINPUT_INITIALIZER DATA_researchTeam = { 
  NELEMS(NumInputList_researchTeam), NumInputList_researchTeam, };

static const DEBUG_NUMINPUT_INITIALIZER DATA_Q_today = { NELEMS(NumInputList_todayCountOfQ), NumInputList_todayCountOfQ };
static const DEBUG_NUMINPUT_INITIALIZER DATA_Q_total = { NELEMS(NumInputList_totalCountOfQ), NumInputList_totalCountOfQ };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q01 = { NELEMS(NumInputList_todayCountOfA_Q01), NumInputList_todayCountOfA_Q01 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q02 = { NELEMS(NumInputList_todayCountOfA_Q02), NumInputList_todayCountOfA_Q02 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q03 = { NELEMS(NumInputList_todayCountOfA_Q03), NumInputList_todayCountOfA_Q03 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q04 = { NELEMS(NumInputList_todayCountOfA_Q04), NumInputList_todayCountOfA_Q04 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q05 = { NELEMS(NumInputList_todayCountOfA_Q05), NumInputList_todayCountOfA_Q05 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q06 = { NELEMS(NumInputList_todayCountOfA_Q06), NumInputList_todayCountOfA_Q06 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q07 = { NELEMS(NumInputList_todayCountOfA_Q07), NumInputList_todayCountOfA_Q07 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q08 = { NELEMS(NumInputList_todayCountOfA_Q08), NumInputList_todayCountOfA_Q08 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q09 = { NELEMS(NumInputList_todayCountOfA_Q09), NumInputList_todayCountOfA_Q09 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q10 = { NELEMS(NumInputList_todayCountOfA_Q10), NumInputList_todayCountOfA_Q10 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q11 = { NELEMS(NumInputList_todayCountOfA_Q11), NumInputList_todayCountOfA_Q11 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q12 = { NELEMS(NumInputList_todayCountOfA_Q12), NumInputList_todayCountOfA_Q12 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q13 = { NELEMS(NumInputList_todayCountOfA_Q13), NumInputList_todayCountOfA_Q13 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q14 = { NELEMS(NumInputList_todayCountOfA_Q14), NumInputList_todayCountOfA_Q14 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q15 = { NELEMS(NumInputList_todayCountOfA_Q15), NumInputList_todayCountOfA_Q15 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q16 = { NELEMS(NumInputList_todayCountOfA_Q16), NumInputList_todayCountOfA_Q16 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q17 = { NELEMS(NumInputList_todayCountOfA_Q17), NumInputList_todayCountOfA_Q17 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q18 = { NELEMS(NumInputList_todayCountOfA_Q18), NumInputList_todayCountOfA_Q18 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q19 = { NELEMS(NumInputList_todayCountOfA_Q19), NumInputList_todayCountOfA_Q19 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q20 = { NELEMS(NumInputList_todayCountOfA_Q20), NumInputList_todayCountOfA_Q20 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q21 = { NELEMS(NumInputList_todayCountOfA_Q21), NumInputList_todayCountOfA_Q21 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q22 = { NELEMS(NumInputList_todayCountOfA_Q22), NumInputList_todayCountOfA_Q22 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q23 = { NELEMS(NumInputList_todayCountOfA_Q23), NumInputList_todayCountOfA_Q23 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q24 = { NELEMS(NumInputList_todayCountOfA_Q24), NumInputList_todayCountOfA_Q24 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q25 = { NELEMS(NumInputList_todayCountOfA_Q25), NumInputList_todayCountOfA_Q25 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q26 = { NELEMS(NumInputList_todayCountOfA_Q26), NumInputList_todayCountOfA_Q26 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q27 = { NELEMS(NumInputList_todayCountOfA_Q27), NumInputList_todayCountOfA_Q27 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q28 = { NELEMS(NumInputList_todayCountOfA_Q28), NumInputList_todayCountOfA_Q28 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q29 = { NELEMS(NumInputList_todayCountOfA_Q29), NumInputList_todayCountOfA_Q29 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_today_Q30 = { NELEMS(NumInputList_todayCountOfA_Q30), NumInputList_todayCountOfA_Q30 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q01 = { NELEMS(NumInputList_totalCountOfA_Q01), NumInputList_totalCountOfA_Q01 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q02 = { NELEMS(NumInputList_totalCountOfA_Q02), NumInputList_totalCountOfA_Q02 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q03 = { NELEMS(NumInputList_totalCountOfA_Q03), NumInputList_totalCountOfA_Q03 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q04 = { NELEMS(NumInputList_totalCountOfA_Q04), NumInputList_totalCountOfA_Q04 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q05 = { NELEMS(NumInputList_totalCountOfA_Q05), NumInputList_totalCountOfA_Q05 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q06 = { NELEMS(NumInputList_totalCountOfA_Q06), NumInputList_totalCountOfA_Q06 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q07 = { NELEMS(NumInputList_totalCountOfA_Q07), NumInputList_totalCountOfA_Q07 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q08 = { NELEMS(NumInputList_totalCountOfA_Q08), NumInputList_totalCountOfA_Q08 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q09 = { NELEMS(NumInputList_totalCountOfA_Q09), NumInputList_totalCountOfA_Q09 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q10 = { NELEMS(NumInputList_totalCountOfA_Q10), NumInputList_totalCountOfA_Q10 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q11 = { NELEMS(NumInputList_totalCountOfA_Q11), NumInputList_totalCountOfA_Q11 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q12 = { NELEMS(NumInputList_totalCountOfA_Q12), NumInputList_totalCountOfA_Q12 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q13 = { NELEMS(NumInputList_totalCountOfA_Q13), NumInputList_totalCountOfA_Q13 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q14 = { NELEMS(NumInputList_totalCountOfA_Q14), NumInputList_totalCountOfA_Q14 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q15 = { NELEMS(NumInputList_totalCountOfA_Q15), NumInputList_totalCountOfA_Q15 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q16 = { NELEMS(NumInputList_totalCountOfA_Q16), NumInputList_totalCountOfA_Q16 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q17 = { NELEMS(NumInputList_totalCountOfA_Q17), NumInputList_totalCountOfA_Q17 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q18 = { NELEMS(NumInputList_totalCountOfA_Q18), NumInputList_totalCountOfA_Q18 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q19 = { NELEMS(NumInputList_totalCountOfA_Q19), NumInputList_totalCountOfA_Q19 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q20 = { NELEMS(NumInputList_totalCountOfA_Q20), NumInputList_totalCountOfA_Q20 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q21 = { NELEMS(NumInputList_totalCountOfA_Q21), NumInputList_totalCountOfA_Q21 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q22 = { NELEMS(NumInputList_totalCountOfA_Q22), NumInputList_totalCountOfA_Q22 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q23 = { NELEMS(NumInputList_totalCountOfA_Q23), NumInputList_totalCountOfA_Q23 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q24 = { NELEMS(NumInputList_totalCountOfA_Q24), NumInputList_totalCountOfA_Q24 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q25 = { NELEMS(NumInputList_totalCountOfA_Q25), NumInputList_totalCountOfA_Q25 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q26 = { NELEMS(NumInputList_totalCountOfA_Q26), NumInputList_totalCountOfA_Q26 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q27 = { NELEMS(NumInputList_totalCountOfA_Q27), NumInputList_totalCountOfA_Q27 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q28 = { NELEMS(NumInputList_totalCountOfA_Q28), NumInputList_totalCountOfA_Q28 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q29 = { NELEMS(NumInputList_totalCountOfA_Q29), NumInputList_totalCountOfA_Q29 };
static const DEBUG_NUMINPUT_INITIALIZER DATA_A_total_Q30 = { NELEMS(NumInputList_totalCountOfA_Q30), NumInputList_totalCountOfA_Q30 };

/// 数値入力　メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DNumInput_MenuFuncHeader =
{
  1,    //リスト項目数
  9,    //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  1,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

// 調査隊関連
static const FLDMENUFUNC_LIST DATA_DNumInputMenu_team[] =
{
  { dni_research_team, (void*)&DATA_researchTeam },
};

// 今日の回答人数 ( 質問ごと )
static const FLDMENUFUNC_LIST DATA_DNumInputMenu_Qtoday[] =
{
  { dni_q_today, (void*)&DATA_Q_today },
};

// いままでの回答人数 ( 質問ごと )
static const FLDMENUFUNC_LIST DATA_DNumInputMenu_Qtotal[] =
{
  { dni_q_total,     (void*)&DATA_Q_total },
};

// 今日の回答人数 ( 回答ごと )
static const FLDMENUFUNC_LIST DATA_DNumInputMenu_Atoday[] =
{
  { dni_a_today_q01, (void*)&DATA_A_today_Q01 },
  { dni_a_today_q02, (void*)&DATA_A_today_Q02 },
  { dni_a_today_q03, (void*)&DATA_A_today_Q03 },
  { dni_a_today_q04, (void*)&DATA_A_today_Q04 },
  { dni_a_today_q05, (void*)&DATA_A_today_Q05 },
  { dni_a_today_q06, (void*)&DATA_A_today_Q06 },
  { dni_a_today_q07, (void*)&DATA_A_today_Q07 },
  { dni_a_today_q08, (void*)&DATA_A_today_Q08 },
  { dni_a_today_q09, (void*)&DATA_A_today_Q09 },
  { dni_a_today_q10, (void*)&DATA_A_today_Q10 },
  { dni_a_today_q11, (void*)&DATA_A_today_Q11 },
  { dni_a_today_q12, (void*)&DATA_A_today_Q12 },
  { dni_a_today_q13, (void*)&DATA_A_today_Q13 },
  { dni_a_today_q14, (void*)&DATA_A_today_Q14 },
  { dni_a_today_q15, (void*)&DATA_A_today_Q15 },
  { dni_a_today_q16, (void*)&DATA_A_today_Q16 },
  { dni_a_today_q17, (void*)&DATA_A_today_Q17 },
  { dni_a_today_q18, (void*)&DATA_A_today_Q18 },
  { dni_a_today_q19, (void*)&DATA_A_today_Q19 },
  { dni_a_today_q20, (void*)&DATA_A_today_Q20 },
  { dni_a_today_q21, (void*)&DATA_A_today_Q21 },
  { dni_a_today_q22, (void*)&DATA_A_today_Q22 },
  { dni_a_today_q23, (void*)&DATA_A_today_Q23 },
  { dni_a_today_q24, (void*)&DATA_A_today_Q24 },
  { dni_a_today_q25, (void*)&DATA_A_today_Q25 },
  { dni_a_today_q26, (void*)&DATA_A_today_Q26 },
  { dni_a_today_q27, (void*)&DATA_A_today_Q27 },
  { dni_a_today_q28, (void*)&DATA_A_today_Q28 },
  { dni_a_today_q29, (void*)&DATA_A_today_Q29 },
  { dni_a_today_q30, (void*)&DATA_A_today_Q30 },
};

// いままでの回答人数 ( 回答ごと )
static const FLDMENUFUNC_LIST DATA_DNumInputMenu_Atotal[] =
{
  { dni_a_total_q01, (void*)&DATA_A_total_Q01 },
  { dni_a_total_q02, (void*)&DATA_A_total_Q02 },
  { dni_a_total_q03, (void*)&DATA_A_total_Q03 },
  { dni_a_total_q04, (void*)&DATA_A_total_Q04 },
  { dni_a_total_q05, (void*)&DATA_A_total_Q05 },
  { dni_a_total_q06, (void*)&DATA_A_total_Q06 },
  { dni_a_total_q07, (void*)&DATA_A_total_Q07 },
  { dni_a_total_q08, (void*)&DATA_A_total_Q08 },
  { dni_a_total_q09, (void*)&DATA_A_total_Q09 },
  { dni_a_total_q10, (void*)&DATA_A_total_Q10 },
  { dni_a_total_q11, (void*)&DATA_A_total_Q11 },
  { dni_a_total_q12, (void*)&DATA_A_total_Q12 },
  { dni_a_total_q13, (void*)&DATA_A_total_Q13 },
  { dni_a_total_q14, (void*)&DATA_A_total_Q14 },
  { dni_a_total_q15, (void*)&DATA_A_total_Q15 },
  { dni_a_total_q16, (void*)&DATA_A_total_Q16 },
  { dni_a_total_q17, (void*)&DATA_A_total_Q17 },
  { dni_a_total_q18, (void*)&DATA_A_total_Q18 },
  { dni_a_total_q19, (void*)&DATA_A_total_Q19 },
  { dni_a_total_q20, (void*)&DATA_A_total_Q20 },
  { dni_a_total_q21, (void*)&DATA_A_total_Q21 },
  { dni_a_total_q22, (void*)&DATA_A_total_Q22 },
  { dni_a_total_q23, (void*)&DATA_A_total_Q23 },
  { dni_a_total_q24, (void*)&DATA_A_total_Q24 },
  { dni_a_total_q25, (void*)&DATA_A_total_Q25 },
  { dni_a_total_q26, (void*)&DATA_A_total_Q26 },
  { dni_a_total_q27, (void*)&DATA_A_total_Q27 },
  { dni_a_total_q28, (void*)&DATA_A_total_Q28 },
  { dni_a_total_q29, (void*)&DATA_A_total_Q29 },
  { dni_a_total_q30, (void*)&DATA_A_total_Q30 },
};

// 調査隊関連
static const DEBUG_MENU_INITIALIZER DATA_DNumInput_MenuInitializer_team = {
  NARC_debug_message_d_numinput_r_dat,    // メッセージアーカイブ
  NELEMS(DATA_DNumInputMenu_team),  // 項目数max
  DATA_DNumInputMenu_team,          // メニュー項目リスト
  &DATA_DNumInput_MenuFuncHeader,   // メニュヘッダ
  1, 4, 30, 16,
  NULL/*DEBUG_SetMenuWorkZoneID_SelectZone*/,
  NULL,
};
// 今日の回答人数 ( 質問 )
static const DEBUG_MENU_INITIALIZER DATA_DNumInput_MenuInitializer_Qtoday = {
  NARC_debug_message_d_numinput_r_dat,    // メッセージアーカイブ
  NELEMS(DATA_DNumInputMenu_Qtoday),  // 項目数max
  DATA_DNumInputMenu_Qtoday,          // メニュー項目リスト
  &DATA_DNumInput_MenuFuncHeader,   // メニュヘッダ
  1, 4, 30, 16,
  NULL/*DEBUG_SetMenuWorkZoneID_SelectZone*/,
  NULL,
};
// 今日の回答人数 ( 回答 )
static const DEBUG_MENU_INITIALIZER DATA_DNumInput_MenuInitializer_Atoday = {
  NARC_debug_message_d_numinput_r_dat,    // メッセージアーカイブ
  NELEMS(DATA_DNumInputMenu_Atoday),  // 項目数max
  DATA_DNumInputMenu_Atoday,          // メニュー項目リスト
  &DATA_DNumInput_MenuFuncHeader,   // メニュヘッダ
  1, 4, 30, 16,
  NULL/*DEBUG_SetMenuWorkZoneID_SelectZone*/,
  NULL,
};
// いままでの回答人数 ( 質問 )
static const DEBUG_MENU_INITIALIZER DATA_DNumInput_MenuInitializer_Qtotal = {
  NARC_debug_message_d_numinput_r_dat,    // メッセージアーカイブ
  NELEMS(DATA_DNumInputMenu_Qtotal),  // 項目数max
  DATA_DNumInputMenu_Qtotal,          // メニュー項目リスト
  &DATA_DNumInput_MenuFuncHeader,   // メニュヘッダ
  1, 4, 30, 16,
  NULL/*DEBUG_SetMenuWorkZoneID_SelectZone*/,
  NULL,
};
// いままでの回答人数 ( 回答 )
static const DEBUG_MENU_INITIALIZER DATA_DNumInput_MenuInitializer_Atotal = {
  NARC_debug_message_d_numinput_r_dat,    // メッセージアーカイブ
  NELEMS(DATA_DNumInputMenu_Atotal),  // 項目数max
  DATA_DNumInputMenu_Atotal,          // メニュー項目リスト
  &DATA_DNumInput_MenuFuncHeader,   // メニュヘッダ
  1, 4, 30, 16,
  NULL/*DEBUG_SetMenuWorkZoneID_SelectZone*/,
  NULL,
};

static const DEBUG_MENU_INITIALIZER DATA_DNumInput_ListMenuInitializer = {
  NARC_debug_message_d_numinput_r_dat,  //メッセージアーカイブ
  0,  //項目数max
  NULL, //メニュー項目リスト
  &DATA_DNumInput_MenuFuncHeader, //メニュヘッダ
  1, 4, 30, 16,
  NULL/*DEBUG_SetMenuWorkZoneID_SelectZone*/,
  NULL,
};


//======================================================================
//
//関数定義
//
//======================================================================
//--------------------------------------------------------------
/**
 * 数値入力のジャンル選択メニューイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
GMEVENT* DEBUG_EVENT_FLDMENU_ResearchNumInput( GAMESYS_WORK* gsys, void* wk, int page )
{
  DEBUG_MENU_EVENT_WORK* dme_wk = (DEBUG_MENU_EVENT_WORK*)wk;
  DNINPUT_EV_WORK* work;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, dninput_MainEvent, sizeof(DNINPUT_EV_WORK) );

  work = GMEVENT_GetEventWork( event );
  work->gsys      = gsys; 
  work->gdata     = dme_wk->gdata; 
  work->fieldWork = dme_wk->fieldWork;
  work->heapID    = dme_wk->heapID;
	work->page      = page;

  return event;
}

//--------------------------------------------------------------
/*
 *  @brief  数値入力メインメニューイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT dninput_MainEvent( GMEVENT * event, int *seq, void * work)
{
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
  DNINPUT_EV_WORK* wk = (DNINPUT_EV_WORK*)work;

  switch( *seq ) {
  case 0:
		{
			const DEBUG_MENU_INITIALIZER* menu = NULL;
			switch( wk->page ) {
			case 0: menu = &DATA_DNumInput_MenuInitializer_team; break;
			case 1: menu = &DATA_DNumInput_MenuInitializer_Qtotal; break;
			case 2: menu = &DATA_DNumInput_MenuInitializer_Atotal; break;
			case 3: menu = &DATA_DNumInput_MenuInitializer_Qtoday; break;
			case 4: menu = &DATA_DNumInput_MenuInitializer_Atoday; break;
      case 5: SetResearchDataAtRandom( gsys ); break; // ランダムセットアップ
      case 6: ClearResearchData( gsys ); break; // 調査データクリア
			default: GF_ASSERT(0);
			}
      if( menu ) {
        wk->menuFunc = DEBUGFLDMENU_Init( wk->fieldWork, wk->heapID, menu );
      }
      else {
        return GMEVENT_RES_FINISH;
      }
		}
    (*seq)++;
    break;

  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( wk->menuFunc );

			if( ret == FLDMENUFUNC_NULL ) { break; }

			FLDMENUFUNC_DeleteMenu( wk->menuFunc );

			if( ret == FLDMENUFUNC_CANCEL ) {  // キャンセル
				(*seq)++;
				break;
			}

			if( ret != NULL ) {
				const DEBUG_NUMINPUT_INITIALIZER *init;
				init = (const DEBUG_NUMINPUT_INITIALIZER *)ret;
				GMEVENT_CallEvent( event, DEBUG_EVENT_FLDMENU_FlagWork( gsys, init ) );
				*seq = 0;
			}
		}
    break;

	case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//======================================================================
//
//
//    デバッグ数値入力  本体
//
//
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  _DISP_INITX = 1,
  _DISP_INITY = 18,
  _DISP_SIZEX = 30,
  _DISP_SIZEY = 4,

  PANO_FONT = 14,
  FBMP_COL_WHITE = 15,
};

#define WINCLR_COL(col) (((col)<<4)|(col))
//--------------------------------------------------------------
/**
 * @brief デバッグ数値入力制御ワーク
 */
//--------------------------------------------------------------
typedef struct {
  /// 操作対象指定ID
  u32 select_id;
  /// 操作制御パラメータ
  const DEBUG_NUMINPUT_PARAM * dni_param;

  GAMESYS_WORK * gsys;
  GAMEDATA * gamedata;
  FIELDMAP_WORK * fieldmap;
  HEAPID heapID;
  GFL_FONT * fontHandle;

	GFL_MSGDATA* msgman;						//メッセージマネージャー
  WORDSET * wordset;
  GFL_BMPWIN * bmpwin;

  u32 value;
}DEBUG_NUMINPUT_WORK;

//--------------------------------------------------------------
/// 数値入力ウィンドウ：生成
//--------------------------------------------------------------
static void createNumWin( DEBUG_NUMINPUT_WORK * wk )
{
	GFL_BMP_DATA *bmp;
	GFL_BMPWIN *bmpwin;
  
  wk->wordset = WORDSET_Create( wk->heapID );
  wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
      ARCID_DEBUG_MESSAGE, NARC_debug_message_d_numinput_r_dat, wk->heapID);

	bmpwin = GFL_BMPWIN_Create( FLDBG_MFRM_MSG,
		_DISP_INITX, _DISP_INITY, _DISP_SIZEX, _DISP_SIZEY,
		PANO_FONT, GFL_BMP_CHRAREA_GET_B );
  wk->bmpwin = bmpwin;
  
	bmp = GFL_BMPWIN_GetBmp( bmpwin );
	
	GFL_BMP_Clear( bmp, WINCLR_COL(FBMP_COL_WHITE) );
	GFL_BMPWIN_MakeScreen( bmpwin );
	GFL_BMPWIN_TransVramCharacter( bmpwin );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );
}

//--------------------------------------------------------------
/// 数値入力ウィンドウ：削除
//--------------------------------------------------------------
static void deleteNumWin( DEBUG_NUMINPUT_WORK * wk )
{
  GFL_BMPWIN * bmpwin = wk->bmpwin;

	GFL_BMPWIN_ClearScreen( bmpwin );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );

  GFL_BMPWIN_Delete( bmpwin );

  GFL_MSG_Delete( wk->msgman );
  WORDSET_Delete( wk->wordset );
}

//--------------------------------------------------------------
/// 数値入力ウィンドウ：表示更新
//--------------------------------------------------------------
static void printNumWin( DEBUG_NUMINPUT_WORK * wk, u32 num )
{
  const DEBUG_NUMINPUT_PARAM * def = wk->dni_param;
  GFL_BMPWIN * bmpwin = wk->bmpwin;

  STRBUF * strbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
  STRBUF * expandBuf = GFL_STR_CreateBuffer( 64, wk->heapID );

  GFL_MSG_GetString( wk->msgman, dni_number_string, strbuf );
  WORDSET_RegisterNumber(wk->wordset, 0, num,
                         10, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( wk->wordset, expandBuf, strbuf );

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp( bmpwin ), WINCLR_COL(FBMP_COL_WHITE) );

  if( num < def->min || num > def->max ){
    GFL_FONTSYS_SetColor( 2, 2, 15 );
  }else{
    GFL_FONTSYS_SetColor( 1, 2, 15 );
  }
	PRINTSYS_Print( GFL_BMPWIN_GetBmp( bmpwin ), 1, 10, expandBuf, wk->fontHandle );		

  GFL_BMPWIN_TransVramCharacter( bmpwin );  //transfer characters
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );  //transfer screen

  GFL_STR_DeleteBuffer( strbuf );
  GFL_STR_DeleteBuffer( expandBuf );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void dniSetValue( DEBUG_NUMINPUT_WORK * dni_wk , u32 value )
{
  const DEBUG_NUMINPUT_PARAM * def = dni_wk->dni_param;
  if (def->set_func == NULL) return;
  def->set_func( dni_wk->gsys, dni_wk->gamedata, dni_wk->select_id, value );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static u32 dniGetValue( DEBUG_NUMINPUT_WORK * dni_wk )
{
  const DEBUG_NUMINPUT_PARAM * def = dni_wk->dni_param;
  if (def->get_func == NULL) return 0;
  return def->get_func( dni_wk->gsys, dni_wk->gamedata, dni_wk->select_id );
}
//--------------------------------------------------------------
/**
 * @brief デバッグ数値入力制御イベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT DebugNumInputEvent( GMEVENT * event , int *seq, void * work )
{
  DEBUG_NUMINPUT_WORK * dni_wk = work;
  EVENTWORK *evwork = GAMEDATA_GetEventWork( dni_wk->gamedata );
  const DEBUG_NUMINPUT_PARAM * def = dni_wk->dni_param;

  switch (*seq)
  {
  case 0:
    {
      dni_wk->value = dniGetValue( dni_wk );
      createNumWin( dni_wk );
      printNumWin( dni_wk, dni_wk->value );
      (*seq) ++;
      break;
    }
  case 1:
    {
      int trg = GFL_UI_KEY_GetRepeat();
      int cont = GFL_UI_KEY_GetCont();
      int diff;
      u32 before, after;
      if ( trg & PAD_BUTTON_B ) { //キャンセル
        (*seq) ++;
        break;
      }
      if ( trg & PAD_BUTTON_A ) { //決定
        dniSetValue( dni_wk, dni_wk->value );
        (*seq) ++;
        break;
      }
      after = before = dni_wk->value;
      if( dni_wk->value < def->min || dni_wk->value > def->max ){
        break;
      }
      diff = 0;
      if (trg & PAD_KEY_UP){
        diff = 1;
      } else if (trg & PAD_KEY_DOWN) {
        diff = -1;
      } else if (trg & PAD_KEY_LEFT){
        diff = -10;
      } else if (trg & PAD_KEY_RIGHT){
        diff = 10;
      }
      if( cont & PAD_BUTTON_R ){
        diff *= 10;
      }else if( cont & PAD_BUTTON_L ){
        diff *= 100;
      }
      if(diff == 0){
        break;
      }
      if( (diff < 0) && ( (after-def->min) < (diff*-1))){
        if( before > def->min ){
          after = def->min; //いったんmin
        }else{
          after = def->max; //回り込み
        }
      }else if( (diff > 0) && ((def->max-after) < diff) ){
        if( before < def->max ){
          after = def->max; //いったんmax
        }else{
          after = def->min; //回り込み
        }
      }else{
        after = before+diff;
      }
      if (after != before ) {
        printNumWin( dni_wk, after );
        dni_wk->value = after;
      }
    }
    break;
  case 2:
    deleteNumWin( dni_wk );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief デバッグ数値入力制御イベント生成
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_ResearchNumInput(
    GAMESYS_WORK * gsys, const DEBUG_NUMINPUT_PARAM * dni_param, u32 id )
{
  GMEVENT * event;
  DEBUG_NUMINPUT_WORK * wk;
  event = GMEVENT_Create( gsys, NULL, DebugNumInputEvent, sizeof(DEBUG_NUMINPUT_WORK) );
  wk = GMEVENT_GetEventWork( event );

  wk->select_id = id;
  wk->dni_param = dni_param;
  wk->gsys = gsys;
  wk->gamedata = GAMESYSTEM_GetGameData( gsys );
  wk->fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  wk->fontHandle = FLDMSGBG_GetFontHandle( FIELDMAP_GetFldMsgBG( wk->fieldmap ) );
  wk->heapID = FIELDMAP_GetHeapID( wk->fieldmap );

  return event;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GAMEDATA * gamedata;
  FIELDMAP_WORK * fieldWork;
  
  DEBUG_MENU_CALLPROC call_proc;
  FLDMENUFUNC *menuFunc;

  const DEBUG_NUMINPUT_INITIALIZER * init;
}DEBUG_FLAGMENU_EVENT_WORK;

//--------------------------------------------------------------
/// メニュー項目最大数取得：デバッグスクリプト
//--------------------------------------------------------------
static u16 DEBUG_GetDebugListMax( GAMESYS_WORK* gsys, void* cb_work )
{
  DEBUG_FLAGMENU_EVENT_WORK * df_work = cb_work;
  return df_work->init->count_or_id;
}

//--------------------------------------------------------------
/// メニュー項目生成：デバッグスクリプト
//--------------------------------------------------------------
static void DEBUG_SetMenuWork_DebugList(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  DEBUG_FLAGMENU_EVENT_WORK * df_work = cb_work;
  const DEBUG_NUMINPUT_INITIALIZER * init = df_work->init;
  int id,max = DEBUG_GetDebugListMax(gsys, cb_work);
  GFL_MSGDATA * msgman = GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_d_numinput_r_dat, df_work->heapID);
  STRBUF *str = GFL_STR_CreateBuffer( 64, heapID );

  for( id = 0; id < max; id++ ){
    u16 real_id;
    GFL_STR_ClearBuffer( str );
    GFL_MSG_GetString( msgman, init->pList[id].gmm_id, str );
    FLDMENUFUNC_AddStringListData( list, str, id, heapID );
  }
  GFL_HEAP_FreeMemory( str );
  GFL_MSG_Delete( msgman );
}
//--------------------------------------------------------------
/**
 * @brief フィールドデバッグメニュー初期化データ：デバッグフラグ・ワーク操作
 */
//--------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER DebugListSelectData = {
  0,
  0,                                ///<項目最大数（固定リストでない場合、０）
  NULL,                             ///<参照するメニューデータ（生成する場合はNULL)
  &DATA_DebugMenuList_ZoneSel,      ///<メニュー表示指定データ（流用）
  1, 1, 30, 16,
  DEBUG_SetMenuWork_DebugList,    ///<メニュー生成関数へのポインタ
  DEBUG_GetDebugListMax,          ///<項目最大数取得関数へのポインタ
};


//--------------------------------------------------------------
/**
 * イベント：デバッグスクリプト選択
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugFlagWorkSelectMenuEvent(
    GMEVENT *event, int *seq, void *wk )
{
	DEBUG_FLAGMENU_EVENT_WORK * new_wk = wk;
  EVENTWORK *evwork = GAMEDATA_GetEventWork( new_wk->gamedata );
  const DEBUG_NUMINPUT_INITIALIZER * init = new_wk->init;
  
  switch( (*seq) ){
  case 0:
    new_wk->menuFunc = DEBUGFLDMENU_InitEx(
        new_wk->fieldWork, new_wk->heapID,  &DebugListSelectData, new_wk );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( new_wk->menuFunc );
      
      // 操作無し
      if( ret == FLDMENUFUNC_NULL ){ break; }

      //キャンセル
      if( ret == FLDMENUFUNC_CANCEL ) {  
        FLDMENUFUNC_DeleteMenu( new_wk->menuFunc );
        return( GMEVENT_RES_FINISH );
      }

      GMEVENT_CallEvent( event, 
          DEBUG_EVENT_ResearchNumInput( new_wk->gmSys, init->pList[ret].dni_param, init->pList[ret].param ) );
      break;
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * DEBUG_EVENT_FLDMENU_FlagWork(
    GAMESYS_WORK * gsys, const DEBUG_NUMINPUT_INITIALIZER * init )
{
	GMEVENT * new_event = GMEVENT_Create( gsys, NULL,
      debugFlagWorkSelectMenuEvent, sizeof(DEBUG_FLAGMENU_EVENT_WORK) );
	DEBUG_FLAGMENU_EVENT_WORK * new_wk = GMEVENT_GetEventWork( new_event );

	GFL_STD_MemClear( new_wk, sizeof(DEBUG_FLAGMENU_EVENT_WORK) );
  new_wk->heapID = HEAPID_PROC;
  new_wk->gmSys = gsys;
  new_wk->gamedata = GAMESYSTEM_GetGameData( gsys );
  new_wk->fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  new_wk->call_proc = NULL;
  new_wk->menuFunc = NULL;

  new_wk->init = init;

  return new_event;
}


//--------------------------------------------------------------
/**
 * @brief 調査人数をランダムに設定する
 */
//--------------------------------------------------------------
static void SetResearchDataAtRandom( GAMESYS_WORK* gameSystem )
{ 
  u8 q_id, a_id;

  // 今日の調査人数
  SetResearchDataAtRandom_today( gameSystem, 99 );

  // いままでの調査人数
  SetResearchDataAtRandom_total( gameSystem, 9999 );
}

//--------------------------------------------------------------
/**
 * @brief 調査人数をランダムに設定する ( 今日の調査人数 )
 */
//--------------------------------------------------------------
static void SetResearchDataAtRandom_today( GAMESYS_WORK* gameSystem, u32 rand_max )
{
  u8 q_id, a_id;
  GAMEDATA* gameData;
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* QSave;

  gameData = GAMESYSTEM_GetGameData( gameSystem );
  save     = GAMEDATA_GetSaveControlWork( gameData );
  QSave    = SaveData_GetQuestionnaire( save );

  for( q_id=0; q_id < QUESTION_ID_NUM; q_id++ )
  {
    u32 total_add_count = 0;

    for( a_id=0; a_id < AnswerNum_question[ q_id ]; a_id++ )
    {
      u32 add_count, added_count;
      u32 before_count, after_count;

      add_count = GFUser_GetPublicRand0( rand_max );
      before_count = QuestionnaireWork_GetTodayAnswerNum( QSave, q_id, a_id + 1 );
      QuestionnaireWork_AddTodayAnswerNum( QSave, q_id, a_id + 1, add_count );
      after_count = QuestionnaireWork_GetTodayAnswerNum( QSave, q_id, a_id + 1 );
      added_count = after_count - before_count;
      total_add_count += added_count;
    }
    QuestionnaireWork_AddTodayCount( QSave, q_id, total_add_count );
  }
}

//--------------------------------------------------------------
/**
 * @brief 調査人数をランダムに設定する ( いままでの調査人数 )
 */
//--------------------------------------------------------------
static void SetResearchDataAtRandom_total( GAMESYS_WORK* gameSystem, u32 rand_max )
{
  u8 q_id, a_id;
  GAMEDATA* gameData;
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* QSave;

  gameData = GAMESYSTEM_GetGameData( gameSystem );
  save     = GAMEDATA_GetSaveControlWork( gameData );
  QSave    = SaveData_GetQuestionnaire( save );

  for( q_id=0; q_id < QUESTION_ID_NUM; q_id++ )
  {
    u32 total_add_count = 0;

    for( a_id=0; a_id < AnswerNum_question[ q_id ]; a_id++ )
    {
      u32 add_count, added_count;
      u32 before_count, after_count;

      add_count = GFUser_GetPublicRand0( rand_max );
      before_count = QuestionnaireWork_GetTotalAnswerNum( QSave, q_id, a_id + 1 );
      QuestionnaireWork_AddTotalAnswerNum( QSave, q_id, a_id + 1, add_count );
      after_count = QuestionnaireWork_GetTotalAnswerNum( QSave, q_id, a_id + 1 );
      added_count = after_count - before_count;
      total_add_count += added_count;
    }
    QuestionnaireWork_AddTotalCount( QSave, q_id, total_add_count );
  }
}

//--------------------------------------------------------------
/**
 * @brief 調査データをクリアする
 */
//--------------------------------------------------------------
static void ClearResearchData( GAMESYS_WORK* gameSystem )
{
  GAMEDATA* gameData;
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* QSave;

  gameData = GAMESYSTEM_GetGameData( gameSystem );
  save     = GAMEDATA_GetSaveControlWork( gameData );
  QSave    = SaveData_GetQuestionnaire( save );

  QuestionnaireWork_ClearResearch( QSave );
}


#endif  //PM_DEBUG
