//============================================================================================
/**
 * @file		b_scenario_sub.h
 * @brief		ポケウッド台本(sub)(主にデータ関連)
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
	BS_CVS_SCENARIO_SLASH,
	BS_CVS_SCENARIO_MAX,	// max
	
	BS_CVS_INFO_CAST = 0,				// INFO
	BS_CVS_INFO_NAME,
	BS_CVS_INFO_SEX,
	BS_CVS_INFO_LEVEL,
	BS_CVS_INFO_ITEM,
	BS_CVS_INFO_ITEM01,
	BS_CVS_INFO_SKILL00,
	BS_CVS_INFO_SKILL01,
	BS_CVS_INFO_SKILL02,
	BS_CVS_INFO_SKILL03,
	BS_CVS_INFO_STATUS,
	BS_CVS_INFO_STATUS01,
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

// スクリーン書き換え
enum{
	BS_SCR_TOP_00 		= 0,			// TOP
	BS_SCR_TOP_01,
	BS_SCR_SCENARIO_00,					// SCENARIO
	BS_SCR_INFO_00,						// INFO
};

// シーケンス
enum{
	BS_SEQ_BACK_00 = 0,				// 戻る(通常)
	BS_SEQ_BACK_01,					// 戻る(決定アニメ)
	BS_SEQ_BACK_02,					// 戻る(反応なし)
	BS_SEQ_LEFT_00,					// 左(通常)
	BS_SEQ_LEFT_01,					// 左(決定アニメ)
	BS_SEQ_LEFT_02,					// 左(反応なし)
	BS_SEQ_RIGHT_00,				// 右(通常)
	BS_SEQ_RIGHT_01,				// 右(決定アニメ)
	BS_SEQ_RIGHT_02,				// 右(反応なし)
	BS_SEQ_UPMARK,					// 上マーク
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

//
// タッチペン
extern s32  BsCmn_ChkTp( BSCENARIO_WORK *wk, const s32 mode );

// obj関連
extern void BsCmn_InitObj( BSCENARIO_WORK* wk );
extern void BsCmn_ExitObj( BSCENARIO_WORK* wk );
extern void BsCmn_ObjVisible( BSCENARIO_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
extern void BsCmn_ChgTypeObj( BSCENARIO_WORK * wk, const s32 wnum, const s32 mode, const u32 type );
extern void BsCmn_ChgTypePoke( BSCENARIO_WORK * wk, const s32 wnum, const u32 id, const s32 sex );
extern void BsCmn_DispCursor( BSCENARIO_WORK *wk, const BOOL bflg, const s32 mode, const s32 num );
//
extern void BsCmn_DispArrow( BSCENARIO_WORK *wk, const s32 mode, const BOOL bflg );
extern void BsCmn_ChgSeq(    BSCENARIO_WORK *wk, const s32 wnum, const s32 seqnum );
extern BOOL BsCmn_IsAnimEnd( BSCENARIO_WORK *wk, const s32 wnum );
//
extern void BsCmn_ChgScreen( BSCENARIO_WORK *wk, const s32 num, const s32 mode );
extern void BsCmn_InitScreenAnime( BSCENARIO_WORK *wk, const s32 num );
extern void BsCmn_MainScreenAnime( BSCENARIO_WORK *wk );
extern BOOL BsCmn_IsScreenAnimeEnd( BSCENARIO_WORK *wk );
//
extern void BsCmn_DispUpMark( BSCENARIO_WORK *wk, const s32 mode );


