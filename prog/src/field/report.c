
#include <gflib.h>

#include "arc_def.h"
#include "pm_define.h"
#include "system/main.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "pokeicon/pokeicon.h"
#include "font/font.naix"
#include "field/zonedata.h"
#include "savedata/playtime.h"

#include "report.h"
#include "report_gra.naix"

#include "message.naix"
#include "msg/msg_report.h"



// タイトル					15x3 = 45
// 日付							13x2 = 26
// 場所							15x2 = 30
// ジムバッジ				12x2 = 24
// 図鑑							11x2 = 22
// プレイ時間				15x2 = 30
// 前回のレポート		26x2 = 52
/* 合計：229 */

// タイトル
#define	BMPWIN_TITLE_FRM	( GFL_BG_FRAME1_S )
#define	BMPWIN_TITLE_PX		( 1 )
#define	BMPWIN_TITLE_PY		( 0 )
#define	BMPWIN_TITLE_SX		( 15 )
#define	BMPWIN_TITLE_SY		( 3 )
#define	BMPWIN_TITLE_PAL	( 0 )
// 日付
#define	BMPWIN_DATE_FRM		( GFL_BG_FRAME1_S )
#define	BMPWIN_DATE_PX		( 3 )
#define	BMPWIN_DATE_PY		( 7 )
#define	BMPWIN_DATE_SX		( 13 )
#define	BMPWIN_DATE_SY		( 2 )
#define	BMPWIN_DATE_PAL		( 0 )
// 場所
#define	BMPWIN_PLACE_FRM	( GFL_BG_FRAME1_S )
#define	BMPWIN_PLACE_PX		( 3 )
#define	BMPWIN_PLACE_PY		( 9 )
#define	BMPWIN_PLACE_SX		( 15 )
#define	BMPWIN_PLACE_SY		( 2 )
#define	BMPWIN_PLACE_PAL	( 0 )
// ジムバッジ
#define	BMPWIN_BADGE_FRM	( GFL_BG_FRAME1_S )
#define	BMPWIN_BADGE_PX		( 3 )
#define	BMPWIN_BADGE_PY		( 16 )
#define	BMPWIN_BADGE_SX		( 12 )
#define	BMPWIN_BADGE_SY		( 2 )
#define	BMPWIN_BADGE_PAL	( 0 )
// 図鑑
#define	BMPWIN_ZUKAN_FRM	( GFL_BG_FRAME1_S )
#define	BMPWIN_ZUKAN_PX		( 17 )
#define	BMPWIN_ZUKAN_PY		( 16 )
#define	BMPWIN_ZUKAN_SX		( 11 )
#define	BMPWIN_ZUKAN_SY		( 2 )
#define	BMPWIN_ZUKAN_PAL	( 0 )
// プレイ時間
#define	BMPWIN_TIME_FRM		( GFL_BG_FRAME1_S )
#define	BMPWIN_TIME_PX		( 3 )
#define	BMPWIN_TIME_PY		( 18 )
#define	BMPWIN_TIME_SX		( 15 )
#define	BMPWIN_TIME_SY		( 2 )
#define	BMPWIN_TIME_PAL		( 0 )
// 前回のレポート
#define	BMPWIN_REPORT_FRM	( GFL_BG_FRAME1_S )
#define	BMPWIN_REPORT_PX	( 3 )
#define	BMPWIN_REPORT_PY	( 21 )
#define	BMPWIN_REPORT_SX	( 26 )
#define	BMPWIN_REPORT_SY	( 2 )
#define	BMPWIN_REPORT_PAL	( 0 )

// BMPWIN INDEX
enum {
	BMPWIN_TITLE = 0,		// タイトル
	BMPWIN_DATE,				// 日付
	BMPWIN_PLACE,				// 場所
	BMPWIN_BADGE,				// ジムバッジ
	BMPWIN_ZUKAN,				// 図鑑
	BMPWIN_TIME,				// プレイ時間
	BMPWIN_REPORT,			// 前回のレポート
	BMPWIN_MAX
};

#define FCOL_P00WN		( PRINTSYS_LSB_Make(15,14,0) )	// フォントカラー：００白抜

// ポケモンアイコン表示座標
#define	POKEICON_PX		( 40 )
#define	POKEICON_PY		( 104 )
#define	POKEICON_SX		( 32 )


struct _REPORT_WORK {
	GAMESYS_WORK * gameSys;				// ゲームシステム

	PRINT_QUE * que;							// プリントキュー
	PRINT_UTIL	win[BMPWIN_MAX];	// BMPWIN

	// OBJ
	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk[TEMOTI_POKEMAX];
	u32	chrRes[TEMOTI_POKEMAX];
	u32	palRes;
	u32	celRes;

	int	seq;

	HEAPID heapID;
};



static void InitBg( HEAPID heapID );
static void ExitBg(void);
static void LoadBgGraphic( HEAPID heapID );
static void InitBmp( REPORT_WORK * wk );
static void ExitBmp( REPORT_WORK * wk );
static void InitObj( REPORT_WORK * wk );
static void ExitObj( REPORT_WORK * wk );


// BMPWIN DATA
static const u8 BmpWinData[][6] =
{
	{	// タイトル
		BMPWIN_TITLE_FRM, BMPWIN_TITLE_PX, BMPWIN_TITLE_PY,
		BMPWIN_TITLE_SX, BMPWIN_TITLE_SY, BMPWIN_TITLE_PAL
	},
	{	// 日付
		BMPWIN_DATE_FRM, BMPWIN_DATE_PX, BMPWIN_DATE_PY,
		BMPWIN_DATE_SX, BMPWIN_DATE_SY, BMPWIN_DATE_PAL
	},
	{	// 場所
		BMPWIN_PLACE_FRM, BMPWIN_PLACE_PX, BMPWIN_PLACE_PY,
		BMPWIN_PLACE_SX, BMPWIN_PLACE_SY, BMPWIN_PLACE_PAL
	},
	{	// ジムバッジ
		BMPWIN_BADGE_FRM, BMPWIN_BADGE_PX, BMPWIN_BADGE_PY,
		BMPWIN_BADGE_SX, BMPWIN_BADGE_SY, BMPWIN_BADGE_PAL
	},
	{	// 図鑑
		BMPWIN_ZUKAN_FRM, BMPWIN_ZUKAN_PX, BMPWIN_ZUKAN_PY,
		BMPWIN_ZUKAN_SX, BMPWIN_ZUKAN_SY, BMPWIN_ZUKAN_PAL
	},
	{	// プレイ時間
		BMPWIN_TIME_FRM, BMPWIN_TIME_PX, BMPWIN_TIME_PY,
		BMPWIN_TIME_SX, BMPWIN_TIME_SY, BMPWIN_TIME_PAL
	},
	{	// 前回のレポート
		BMPWIN_REPORT_FRM, BMPWIN_REPORT_PX, BMPWIN_REPORT_PY,
		BMPWIN_REPORT_SX, BMPWIN_REPORT_SY, BMPWIN_REPORT_PAL
	}
};

// OBJ DATA
static const GFL_CLWK_DATA	PokeIconData = { POKEICON_PX, POKEICON_PY, POKEICON_ANM_DEATH, 0, 0 };





REPORT_WORK * REPORT_Init( GAMESYS_WORK * gs, HEAPID heapID )
{
/*
	名前
	現在の時間
	現在地
	手持ちポケモン
	バッジ数
	図鑑登録数
	プレイ時間
	前回のレポート時間
*/
	REPORT_WORK * wk;

	wk = GFL_HEAP_AllocMemory( heapID, sizeof(REPORT_WORK) );

	wk->gameSys = gs;
	wk->heapID  = heapID;
	wk->seq     = 0;

	InitBg( wk->heapID );
	LoadBgGraphic( wk->heapID );
	InitObj( wk );
	InitBmp( wk );

	return wk;
}

void REPORT_Exit( REPORT_WORK * wk )
{
	ExitBmp( wk );
	ExitObj( wk );
	ExitBg();

	GFL_HEAP_FreeMemory( wk );
}

void REPORT_Update( REPORT_WORK * wk )
{
	u32	i;

	if( wk->seq == 0 ){
		if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
			GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
			GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
			GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_ON );
			{
				POKEPARTY * party;
				party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(wk->gameSys) );
				for( i=0; i<PokeParty_GetPokeCount(party); i++ ){
					GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], TRUE );
				}
			}
			wk->seq = 1;
		}
	}

	PRINTSYS_QUE_Main( wk->que );
	for( i=0; i<BMPWIN_MAX; i++ ){
		PRINT_UTIL_Trans( &wk->win[i], wk->que );
	}
}




static void InitBg( HEAPID heapID )
{
	GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );

	{	// 背景
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x3800, GX_BG_CHARBASE_0x00000, 0x2800,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_S );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 0x20, 0, heapID );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	}
	{	// フォント面
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x04000, 0x4000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
			};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME1_S, 0x20, 0, heapID );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
	}
	{	// メッセージ面
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2800, GX_BG_CHARBASE_0x04000, 0x4000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_S );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME2_S, 0x20, 0, heapID );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
	}
}

static void ExitBg(void)
{
	GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
}

static void LoadBgGraphic( HEAPID heapID )
{
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_REPORT_GRA, heapID );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_report_gra_base_bg_NCLR, PALTYPE_SUB_BG, 0, 0x20, heapID );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_report_gra_base_bg_lz_NCGR, GFL_BG_FRAME0_S, 0, 0, TRUE, heapID );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_report_gra_base_bg_lz_NSCR, GFL_BG_FRAME0_S, 0, 0, TRUE, heapID );

	GFL_ARC_CloseDataHandle( ah );
}

static void InitBmp( REPORT_WORK * wk )
{
	GFL_MSGDATA * mman;
	WORDSET * wset;
	STRBUF * exp;
	STRBUF * str;
	GFL_FONT * font;
	GAMEDATA * gd;
	SAVE_CONTROL_WORK * sv;
	u32	i;

	wk->que = PRINTSYS_QUE_Create( wk->heapID );

	for( i=0; i<BMPWIN_MAX; i++ ){
		wk->win[i].win = GFL_BMPWIN_Create(
											BmpWinData[i][0],
											BmpWinData[i][1],
											BmpWinData[i][2],
											BmpWinData[i][3],
											BmpWinData[i][4],
											BmpWinData[i][5],
											GFL_BMP_CHRAREA_GET_B );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[i].win), 0 );
	}

	mman = GFL_MSG_Create(
					GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_report_dat, wk->heapID );
	wset = WORDSET_Create( wk->heapID );
	font = GFL_FONT_Create(
					ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	exp  = GFL_STR_CreateBuffer( 256, wk->heapID );

	gd = GAMESYSTEM_GetGameData( wk->gameSys );
	sv = GAMEDATA_GetSaveControlWork( gd );

	// タイトル
	str = GFL_MSG_CreateString( mman, REPORT_STR_01 );
	WORDSET_RegisterPlayerName( wset, 0, GAMEDATA_GetMyStatus(gd) );
	WORDSET_ExpandStr( wset, exp, str );
	PRINT_UTIL_PrintColor( &wk->win[BMPWIN_TITLE], wk->que, 0, 4, exp, font, FCOL_P00WN );
	GFL_STR_DeleteBuffer( str );

	// 日付
	str = GFL_MSG_CreateString( mman, REPORT_STR_02 );
	{
		RTCDate	date;
		RTC_GetDate( &date );
		WORDSET_RegisterNumber( wset, 0, date.year, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
		WORDSET_RegisterNumber( wset, 1, date.month, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
		WORDSET_RegisterNumber( wset, 2, date.day, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
	}
	WORDSET_ExpandStr( wset, exp, str );
	PRINT_UTIL_PrintColor( &wk->win[BMPWIN_DATE], wk->que, 0, 0, exp, font, FCOL_P00WN );
	GFL_STR_DeleteBuffer( str );
	str = GFL_MSG_CreateString( mman, REPORT_STR_03 );
	{
		RTCTime	time;
		RTC_GetTime( &time );
		WORDSET_RegisterNumber( wset, 0, time.hour, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
		WORDSET_RegisterNumber( wset, 1, time.minute, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
	}
	WORDSET_ExpandStr( wset, exp, str );
	PRINT_UTIL_PrintColor( &wk->win[BMPWIN_DATE], wk->que, 72, 0, exp, font, FCOL_P00WN );
	GFL_STR_DeleteBuffer( str );

	// 場所
	str = GFL_MSG_CreateString( mman, REPORT_STR_04 );
	{
		u16	num = ZONEDATA_GetPlaceNameID( PLAYERWORK_getZoneID(GAMESYSTEM_GetMyPlayerWork(wk->gameSys)) );
		WORDSET_RegisterPlaceName( wset, 0, num );
	}
	WORDSET_ExpandStr( wset, exp, str );
	PRINT_UTIL_PrintColor( &wk->win[BMPWIN_PLACE], wk->que, 0, 0, exp, font, FCOL_P00WN );
	GFL_STR_DeleteBuffer( str );

	// ジムバッジ
	str = GFL_MSG_CreateString( mman, REPORT_STR_05 );
	PRINT_UTIL_PrintColor( &wk->win[BMPWIN_BADGE], wk->que, 0, 0, str, font, FCOL_P00WN );
	GFL_STR_DeleteBuffer( str );
	str = GFL_MSG_CreateString( mman, REPORT_STR_06 );
	{
		int	num = MISC_GetBadgeCount( GAMEDATA_GetMiscWork(gd) );
		WORDSET_RegisterNumber( wset, 0, num, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
	}
	WORDSET_ExpandStr( wset, exp, str );
	PRINT_UTIL_PrintColor( &wk->win[BMPWIN_BADGE], wk->que, 0, 0, exp, font, FCOL_P00WN );
	GFL_STR_DeleteBuffer( str );

	// 図鑑
	str = GFL_MSG_CreateString( mman, REPORT_STR_07 );
	PRINT_UTIL_PrintColor( &wk->win[BMPWIN_ZUKAN], wk->que, 0, 0, str, font, FCOL_P00WN );
	GFL_STR_DeleteBuffer( str );
	str = GFL_MSG_CreateString( mman, REPORT_STR_08 );
	{
		u16	num = ZUKANSAVE_GetZukanPokeSeeCount( GAMEDATA_GetZukanSave(gd), wk->heapID );
		WORDSET_RegisterNumber( wset, 0, num, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
	}
	WORDSET_ExpandStr( wset, exp, str );
	PRINT_UTIL_PrintColor( &wk->win[BMPWIN_ZUKAN], wk->que, 0, 0, exp, font, FCOL_P00WN );
	GFL_STR_DeleteBuffer( str );

	// プレイ時間
	str = GFL_MSG_CreateString( mman, REPORT_STR_09 );
	PRINT_UTIL_PrintColor( &wk->win[BMPWIN_TIME], wk->que, 0, 0, str, font, FCOL_P00WN );
	GFL_STR_DeleteBuffer( str );
	str = GFL_MSG_CreateString( mman, REPORT_STR_10 );
	{
		PLAYTIME * ptime = SaveData_GetPlayTime( sv );
		WORDSET_RegisterNumber( wset, 0, PLAYTIME_GetHour(ptime), 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
		WORDSET_RegisterNumber( wset, 1, PLAYTIME_GetMinute(ptime), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
	}
	WORDSET_ExpandStr( wset, exp, str );
	PRINT_UTIL_PrintColor( &wk->win[BMPWIN_TIME], wk->que, 80, 0, exp, font, FCOL_P00WN );
	GFL_STR_DeleteBuffer( str );

	// 前回のレポート
	// 前回のセーブデータが自分のものであるときのみ表示
	if( SaveControl_IsOverwritingOtherData(sv) == FALSE && SaveData_GetExistFlag(sv) == TRUE ){
		PLAYTIME * ptime = SaveData_GetPlayTime( sv );
		str = GFL_MSG_CreateString( mman, REPORT_STR_11 );
		PRINT_UTIL_PrintColor( &wk->win[BMPWIN_REPORT], wk->que, 0, 0, str, font, FCOL_P00WN );
		GFL_STR_DeleteBuffer( str );
		str = GFL_MSG_CreateString( mman, REPORT_STR_12 );
		WORDSET_RegisterNumber( wset, 0, PLAYTIME_GetSaveYear(ptime), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
		WORDSET_RegisterNumber( wset, 1, PLAYTIME_GetSaveMonth(ptime), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
		WORDSET_RegisterNumber( wset, 2, PLAYTIME_GetSaveDay(ptime ), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
		WORDSET_ExpandStr( wset, exp, str );
		PRINT_UTIL_PrintColor( &wk->win[BMPWIN_REPORT], wk->que, 104, 0, exp, font, FCOL_P00WN );
		GFL_STR_DeleteBuffer( str );
		str = GFL_MSG_CreateString( mman, REPORT_STR_13 );
		WORDSET_RegisterNumber( wset, 0, PLAYTIME_GetSaveHour(ptime), 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
		WORDSET_RegisterNumber( wset, 1, PLAYTIME_GetSaveMinute(ptime), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
		WORDSET_ExpandStr( wset, exp, str );
		PRINT_UTIL_PrintColor( &wk->win[BMPWIN_REPORT], wk->que, 176, 0, exp, font, FCOL_P00WN );
		GFL_STR_DeleteBuffer( str );
	}

	GFL_STR_DeleteBuffer( exp );
	GFL_FONT_Delete( font );
	WORDSET_Delete( wset );
	GFL_MSG_Delete( mman );

	for( i=0; i<BMPWIN_MAX; i++ ){
		GFL_BMPWIN_MakeScreen( wk->win[i].win );
	}
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_S );
}

static void ExitBmp( REPORT_WORK * wk )
{
	u32	i;

	for( i=0; i<BMPWIN_MAX; i++ ){
		GFL_BMPWIN_Delete( wk->win[i].win );
	}

	PRINTSYS_QUE_Delete( wk->que );
}

static void InitObj( REPORT_WORK * wk )
{
  ARCHANDLE * ah;
	POKEPARTY * party;
	GFL_CLWK_DATA	dat;
	u32	max;
	u32	i;

	party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(wk->gameSys) );
	max   = PokeParty_GetPokeCount( party );

	wk->clunit = GFL_CLACT_UNIT_Create( max, 0, wk->heapID );

	ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, wk->heapID );

	// キャラ
	for( i=0; i<max; i++ ){
		POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, i );
		wk->chrRes[i] = GFL_CLGRP_CGR_Register(
											ah, POKEICON_GetCgxArcIndex(PP_GetPPPPointerConst(pp)),
											FALSE, CLSYS_DRAW_SUB, wk->heapID );
	}
	// パレット
  wk->palRes = GFL_CLGRP_PLTT_RegisterComp(
								ah, POKEICON_GetPalArcIndex(), CLSYS_DRAW_SUB, 0, wk->heapID );
	// セル・アニメ
  wk->celRes = GFL_CLGRP_CELLANIM_Register(
								ah, POKEICON_GetCellArcIndex(), POKEICON_GetAnmArcIndex(), wk->heapID );

	dat = PokeIconData;
	for( i=0; i<max; i++ ){
		POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, i );
		wk->clwk[i] = GFL_CLACT_WK_Create(
										wk->clunit,
										wk->chrRes[i],
										wk->palRes,
										wk->celRes, 
										&dat, CLSYS_DRAW_SUB, wk->heapID );
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], FALSE );
		dat.pos_x += POKEICON_SX;

		GFL_CLACT_WK_SetPlttOffs(
			wk->clwk[i], POKEICON_GetPalNumGetByPPP(PP_GetPPPPointerConst(pp)), CLWK_PLTTOFFS_MODE_PLTT_TOP );
	}

  GFL_ARC_CloseDataHandle( ah );
}

static void ExitObj( REPORT_WORK * wk )
{
	POKEPARTY * party;
	u32	i;

	party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(wk->gameSys) );

	for( i=0; i<PokeParty_GetPokeCount(party); i++ ){
		GFL_CLACT_WK_Remove( wk->clwk[i] );
		GFL_CLGRP_CGR_Release( wk->chrRes[i] );
	}

	GFL_CLGRP_PLTT_Release( wk->palRes );
	GFL_CLGRP_CELLANIM_Release( wk->celRes );

	GFL_CLACT_UNIT_Delete( wk->clunit );
}
