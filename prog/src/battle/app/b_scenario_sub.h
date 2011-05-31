//============================================================================================
/**
 * @file		b_scenario_sub.h
 * @brief		戦闘用シナリオサブ画面(データ関連をまとめる)
 * @author
 * @date		05.02.10
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定義
//============================================================================================
// canvasの番号
enum{
	BS_CVS_TOP_TITLE = 0,				// TOP
	BS_CVS_TOP_PLOT,
	BS_CVS_TOP_GOAL,
	BS_CVS_TOP_TITLE_MES,
	BS_CVS_TOP_PLOT_MES,
	BS_CVS_TOP_GOAL_MES,
	BS_CVS_TOP_LABEL00,
	BS_CVS_TOP_LABEL01,
	BS_CSV_TOP_MAX,			// max
	//
	BS_CVS_SCENARIO_SCENARIO = 0,		// SCENARIO
	BS_CVS_SCENARIO_SCENE00,
	BS_CVS_SCENARIO_SCENE01,
	BS_CVS_SCENARIO_SCENE00_MES,
	BS_CVS_SCENARIO_SCENE01_MES,
	BS_CVS_SCENARIO_LABEL00,
	BS_CVS_SCENARIO_NUM00,
	BS_CVS_SCENARIO_NUM01,
	BS_CVS_SCENARIO_MAX,	// max
	
	BS_CVS_INFO_CAST = 0,				// INFO
	BS_CVS_INFO_NAME,
	BS_CVS_INFO_LEVEL,
	BS_CVS_INFO_ITEM,
	BS_CVS_INFO_SKILL00,
	BS_CVS_INFO_SKILL01,
	BS_CVS_INFO_SKILL02,
	BS_CVS_INFO_SKILL03,
	BS_CVS_INFO_STATUS,
	BS_CVS_INFO_LABEL00,
	BS_CVS_INFO_MAX,		// max
};

// タッチペン領域
enum{
	BS_TP_TOP_LABEL00 = 0,				// TOP
	BS_TP_TOP_LABEL01,
	BS_TP_TOP_RETURN,
	//
	BS_TP_SCENARIO_LABEL00 = 0,			// SCENARIO
	BS_TP_SCENARIO_LEFT,
	BS_TP_SCENARIO_RIGHT,
	BS_TP_SCENARIO_RETURN,
	//
	BS_TP_INFO_LABEL00 = 0,				// INFO
	BS_TP_INFO_LEFT,
	BS_TP_INFO_RIGHT,
	BS_TP_INFO_RETURN,
};

//============================================================================================
//	宣言
//============================================================================================
// bg関連
extern void BsCmn_InitBg( BSCENARIO_WORK* wk );
extern void BsCmn_BgScroll(BSCENARIO_WORK* wk, const s32 mode );

// 文字列関連
extern void BsCmn_InitString( BSCENARIO_WORK* wk, const s32 mode );
extern void BsCmn_MainPrint(  BSCENARIO_WORK *wk );
void BsCmn_DispString( BSCENARIO_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 pmode, 
							const PRINTSYS_LSB col, const s32 fmode );
extern void BsCmn_DispStringNumber( BSCENARIO_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 num, const s32 lnum,
							const u32 pmode, const s32 fmode);
extern void BsCmn_ExitString( BSCENARIO_WORK* wk );
extern void BsCmn_ClearCanvas( BSCENARIO_WORK* wk, const s32 mode );
extern void BsCmn_ClearAllCanvas( BSCENARIO_WORK* wk );

//
// タッチペン
extern s32  BsCmn_ChkTp( BSCENARIO_WORK *wk, const s32 mode );



