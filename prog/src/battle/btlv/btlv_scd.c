//=============================================================================================
/**
 * @file	btlv_scd.c
 * @brief	ポケモンWB バトル 描画 下画面制御モジュール
 * @author	taya
 *
 * @date	2008.11.18	作成
 */
//=============================================================================================
#include <ui.h>
#include "print/printsys.h"

#include "battle/btl_common.h"
#include "battle/btl_util.h"
#include "battle/btl_string.h"
#include "battle/btl_action.h"

#include "btlv_core.h"
#include "btlv_scd.h"
#include "btlv_mcss.h"

#include "arc_def.h"
#include "font/font.naix"
#include "msg/msg_btl_ui.h"


/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
	TEST_SELWIN_COL1 = 0x04,
	TEST_SELWIN_COL2 = 0x06,
	TEST_SELWIN_COL3 = 0x07,
	TEST_SELWIN_COL4 = 0x0A,

	POKEWIN_WIDTH  = 15*8,
	POKEWIN_HEIGHT = 4*8,

	POKEWIN_1_X = 0,
	POKEWIN_1_Y = 8,
	POKEWIN_2_X = 256-POKEWIN_WIDTH,
	POKEWIN_2_Y = POKEWIN_1_Y,

	POKEWIN_3_X = 0,
	POKEWIN_3_Y = POKEWIN_1_Y+POKEWIN_HEIGHT+8,
	POKEWIN_4_X = 256-POKEWIN_WIDTH,
	POKEWIN_4_Y = POKEWIN_3_Y,

	POKEWIN_5_X = 0,
	POKEWIN_5_Y = POKEWIN_3_Y+POKEWIN_HEIGHT+8,
	POKEWIN_6_X = 256-POKEWIN_WIDTH,
	POKEWIN_6_Y = POKEWIN_5_Y,

	SUBPROC_STACK_DEPTH = 4,
};

//--------------------------------------------------------------
/**
 *	ワザ対象の選択／確認画面用パラメータ
 */
//--------------------------------------------------------------
enum {
	STW_CONFIRM_POKE,
	STW_CONFIRM_FIELD,
};
typedef u8 StwConfirmType;

typedef struct {

	u8 selectablePokeCount;				///< 選択できるポケモンの候補数（0なら選択不可->確認モード）
	u8 confirmCount;							///< 確認モード対象の数
	StwConfirmType	confirmType;	///< 確認モード対象タイプ（0:ポケモン, 1:場）

	u8 pos[ BTL_POS_MAX ];		///< 選択・確認するポケモンの位置

}SEL_TARGET_WORK;

//--------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------
struct _BTLV_SCD {

	GFL_BMPWIN*			win;
	GFL_BMP_DATA*		bmp;
	GFL_FONT*				font;
	STRBUF*					strbuf;

	PRINT_QUE*			printQue;
	PRINT_UTIL			printUtil;

	BTL_PROC			subProc;
	BTL_PROC			subProcStack[ SUBPROC_STACK_DEPTH ];
	u32						subProcStackPtr;

	const BTL_POKEPARAM*	bpp;
	BTL_ACTION_PARAM*			destActionParam;

	const BTL_POKESELECT_PARAM*		pokesel_param;
	BTL_POKESELECT_RESULT*				pokesel_result;

	SEL_TARGET_WORK		selTargetWork;
	u8								selTargetDone;

	const BTLV_CORE* vcore;
	const BTL_MAIN_MODULE* mainModule;
	HEAPID	heapID;
};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void spstack_init( BTLV_SCD* wk );
static void spstack_push( BTLV_SCD* wk, BPFunc initFunc, BPFunc loopFunc );
static BOOL spstack_call( BTLV_SCD* wk );
static void printBtn( BTLV_SCD* wk, u16 posx, u16 posy, u16 sizx, u16 sizy, u16 col, u16 strID );
static void printBtnWaza( BTLV_SCD* wk, u16 btnIdx, u16 col, const STRBUF* str );
static void printCommWait( BTLV_SCD* wk );
static BOOL selectAction_init( int* seq, void* wk_adrs );
static BOOL selectAction_loop( int* seq, void* wk_adrs );
static void stw_init( SEL_TARGET_WORK* stw );
static void stw_convert_pos_to_index( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, u8 num );
static void stw_setSelectablePoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos );
static void stw_setConfirmPoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos );
static void stw_setConfirmField( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos );
static BOOL stw_is_enable_hitpos( SEL_TARGET_WORK* stw, int hitPos, const BTL_MAIN_MODULE* mainModule, u8* target_idx );
static inline u8 stwdraw_vpos_to_tblidx( u8 vpos );
static void stwdraw_button( const u8* pos, u8 count, u8 format, BTLV_SCD* wk );
static void stw_draw( const SEL_TARGET_WORK* stw, BTLV_SCD* work );
static BOOL stw_draw_wait( BTLV_SCD* wk );
static BOOL selectTarget_init( int* seq, void* wk_adrs );
static BOOL selectTarget_loop( int* seq, void* wk_adrs );
static void seltgt_init_setup_work( SEL_TARGET_WORK* stw, BTLV_SCD* wk );
static BOOL selectPokemon_init( int* seq, void* wk_adrs );
static BOOL selectPokemon_loop( int* seq, void* wk_adrs );






BTLV_SCD*  BTLV_SCD_Create( const BTLV_CORE* vcore, const BTL_MAIN_MODULE* mainModule, GFL_TCBLSYS* tcbl, GFL_FONT* font, u8 playerClientID, HEAPID heapID )
{
	BTLV_SCD* wk = GFL_HEAP_AllocMemory( heapID, sizeof(BTLV_SCD) );

	wk->vcore = vcore;
	wk->mainModule = mainModule;
	wk->heapID = heapID;
	wk->font = font;
	wk->strbuf = GFL_STR_CreateBuffer( 128, heapID );

	wk->printQue = PRINTSYS_QUE_Create( wk->heapID );

	spstack_init( wk );

	return wk;
}

void BTLV_SCD_Setup( BTLV_SCD* wk )
{
	// 個別フレーム設定
	static const GFL_BG_BGCNT_HEADER bgcntText = {
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	};

	GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

	GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );

	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 0, 0, wk->heapID );

	GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0xff, 1, 0 );
	GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

	wk->win = GFL_BMPWIN_Create( GFL_BG_FRAME0_S, 0, 0, 32, 24, 0, GFL_BMP_CHRAREA_GET_F );
	wk->bmp = GFL_BMPWIN_GetBmp( wk->win );
	GFL_BMP_Clear( wk->bmp, 0x0f );
	PRINT_UTIL_Setup( &wk->printUtil, wk->win );
	GFL_BMPWIN_MakeScreen( wk->win );
	GFL_BMPWIN_TransVramCharacter( wk->win );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
}

void BTLV_SCD_Delete( BTLV_SCD* wk )
{
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BMPWIN_Delete( wk->win );

	PRINTSYS_QUE_Delete( wk->printQue );
	GFL_STR_DeleteBuffer( wk->strbuf );
	GFL_HEAP_FreeMemory( wk );
}

//-------------------------------------------------
// サブプロセススタック管理

static void spstack_init( BTLV_SCD* wk )
{
	int i;
	for(i=0; i<SUBPROC_STACK_DEPTH; i++)
	{
		BTL_UTIL_ClearProc( &wk->subProcStack[i] );
	}
	wk->subProcStackPtr = 0;
}

static void spstack_push( BTLV_SCD* wk, BPFunc initFunc, BPFunc loopFunc )
{
	GF_ASSERT(wk->subProcStackPtr < SUBPROC_STACK_DEPTH);

	{
		BTL_PROC* proc = &wk->subProcStack[wk->subProcStackPtr++];
		BTL_UTIL_SetupProc( proc, wk, initFunc, loopFunc );
	}
}
static BOOL spstack_call( BTLV_SCD* wk )
{
	if( wk->subProcStackPtr > 0 )
	{
		BTL_PROC* proc = &wk->subProcStack[ wk->subProcStackPtr - 1 ];

		if( BTL_UTIL_CallProc(proc) )
		{
			BTL_UTIL_ClearProc( proc );
			wk->subProcStackPtr--;
		}
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
//=============================================================================================
//	アクション選択処理
//=============================================================================================
void BTLV_SCD_StartActionSelect( BTLV_SCD* wk, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* dest )
{
	wk->bpp = bpp;
	wk->destActionParam = dest;
	spstack_push( wk, selectAction_init, selectAction_loop );
}

BOOL BTLV_SCD_WaitActionSelect( BTLV_SCD* wk )
{
	return spstack_call( wk );
}

static const GFL_UI_TP_HITTBL TP_HitTbl[] = {
	{  0,  95,   0,  127 },
	{  0,  95,  128, 255 },
	{ 96, 192,   0,  127 },
	{ 96, 192,  128, 255 },
	{ GFL_UI_TP_HIT_END, 0, 0, 0 },
};

static const struct {
		u8 x;
		u8 y;
		u8 w;
		u8 h;
}BtlPos[] = {
	{   0,  0, 128, 96 },
	{ 128,  0, 128, 96 },
	{   0, 96, 128, 96 },
	{ 128, 96, 128, 96 },
};


static void printBtn( BTLV_SCD* wk, u16 posx, u16 posy, u16 sizx, u16 sizy, u16 col, u16 strID )
{
	u32 strWidth, drawX, drawY;

	GFL_BMP_Fill( wk->bmp, posx, posy, sizx, sizy, col );
	BTL_STR_GetUIString( wk->strbuf, strID );
	strWidth = PRINTSYS_GetStrWidth( wk->strbuf, wk->font, 0 );
	drawX = posx + (sizx - strWidth) / 2;
	drawY = posy + (sizy - 16) / 2;
	PRINT_UTIL_Print( &wk->printUtil, wk->printQue, drawX, drawY, wk->strbuf, wk->font );
}

static void printBtnWaza( BTLV_SCD* wk, u16 btnIdx, u16 col, const STRBUF* str )
{
	u32 strWidth, drawX, drawY;
	u8 posx, posy, sizx, sizy;

	posx = BtlPos[btnIdx].x;
	posy = BtlPos[btnIdx].y;
	sizx = BtlPos[btnIdx].w;
	sizy = BtlPos[btnIdx].h;

	GFL_BMP_Fill( wk->bmp, posx, posy, sizx, sizy, col );

	// soga
	if( str == NULL ) return;

	strWidth = PRINTSYS_GetStrWidth( wk->strbuf, wk->font, 0 );
	drawX = posx + (sizx - strWidth) / 2;
	drawY = posy + (sizy - 32) / 2;
	PRINT_UTIL_Print( &wk->printUtil, wk->printQue, drawX, drawY, wk->strbuf, wk->font );
}


static BOOL selectAction_init( int* seq, void* wk_adrs )
{
	BTLV_SCD* wk = wk_adrs;

	switch( *seq ){
	case 0:
		GFL_BMP_Clear( wk->bmp, 0x0f );

		printBtn( wk,   0,  0, 128, 96, TEST_SELWIN_COL1, BTLMSG_UI_SEL_FIGHT );
		printBtn( wk, 128,  0, 128, 96, TEST_SELWIN_COL2, BTLMSG_UI_SEL_ITEM );
		printBtn( wk,   0, 96, 128, 96, TEST_SELWIN_COL3, BTLMSG_UI_SEL_POKEMON );
		printBtn( wk, 128, 96, 128, 96, TEST_SELWIN_COL4, BTLMSG_UI_SEL_ESCAPE );

		(*seq)++;
		break;

	case 1:
		PRINTSYS_QUE_Main( wk->printQue );
		if( PRINT_UTIL_Trans(&wk->printUtil, wk->printQue) )
		{
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static BOOL selectAction_loop( int* seq, void* wk_adrs )
{
	enum {
		SEQ_START = 0,
		SEQ_SEL_FIGHT = 100,
		SEQ_SEL_FIGHT_FINISH = 180,
		SEQ_SEL_ITEM = 200,
		SEQ_SEL_POKEMON = 300,
		SEQ_SEL_ESCAPE = 400,
	};

	BTLV_SCD* wk = wk_adrs;

	switch( *seq ){
	case SEQ_START:
		{
			int hit = GFL_UI_TP_HitTrg( TP_HitTbl );
			if( hit != GFL_UI_TP_HIT_NONE )
			{
				switch( hit ){
				default:
				case BTL_ACTION_FIGHT:
					(*seq) = SEQ_SEL_FIGHT;
					break;
				case BTL_ACTION_ITEM:
//					(*seq) = SEQ_SEL_FIGHT;
					break;
				case BTL_ACTION_CHANGE:
					(*seq) = SEQ_SEL_POKEMON;
					break;
				case BTL_ACTION_ESCAPE:
					(*seq) = SEQ_SEL_ESCAPE;
					break;
				}
			}
		}
		break;

	case SEQ_SEL_FIGHT:
		{
			u16 wazaCnt, wazaID, i;
			u8 PP, PPMax;

			wazaCnt = BTL_POKEPARAM_GetWazaCount( wk->bpp );
			for(i=0; i<wazaCnt; i++)
			{
				wazaID = BTL_POKEPARAM_GetWazaParticular( wk->bpp, i, &PP, &PPMax );
				BTL_STR_MakeWazaUIString( wk->strbuf, wazaID, PP, PPMax );
				printBtnWaza( wk, i, 0x0a, wk->strbuf );
			}
			// soga
			for( ; i<PTL_WAZA_MAX; i++){
				printBtnWaza( wk, i, 0x0f, NULL );
			}
			(*seq)++;
		}
		break;
	case SEQ_SEL_FIGHT+1:
		PRINTSYS_QUE_Main( wk->printQue );
		if( PRINT_UTIL_Trans(&wk->printUtil, wk->printQue) )
		{
			(*seq)++;
		}
		break;
	case SEQ_SEL_FIGHT+2:
		{
			int hit = GFL_UI_TP_HitTrg( TP_HitTbl );
			// soga
//			if( hit != GFL_UI_TP_HIT_NONE )
			if( ( hit != GFL_UI_TP_HIT_NONE ) && ( hit <= BTL_POKEPARAM_GetWazaCount( wk->bpp ) - 1 ) )
			{
				BTL_ACTION_SetFightParam( wk->destActionParam, hit, 0 );

				if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_SINGLE )
				{
					(*seq) = SEQ_SEL_FIGHT_FINISH;
				}
				else
				{
					BTL_Printf("ワザターゲット選択（確認）へ\n");
					spstack_push( wk, selectTarget_init, selectTarget_loop );
					(*seq)++;
				}
			}
			break;
		case SEQ_SEL_FIGHT+3:
			if( wk->selTargetDone )
			{
				BTL_Printf("ちゃんと選んだど\n");
				(*seq) = SEQ_SEL_FIGHT_FINISH;
			}
			else
			{
				(*seq) = SEQ_SEL_FIGHT;
			}
			break;
		}
		break;

	case SEQ_SEL_FIGHT_FINISH:
		return TRUE;


	case SEQ_SEL_POKEMON:
		BTL_ACTION_SetChangeBegin( wk->destActionParam );
		return TRUE;

	case SEQ_SEL_POKEMON+1:
		return TRUE;


	case SEQ_SEL_ESCAPE:
		BTL_ACTION_SetEscapeParam( wk->destActionParam );
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------
// ターゲット選択画面描画処理
//--------------------------------------------------------------------------------------
enum {
// 2vs2
//		D B
//
// A C
	STW_BTN_WIDTH = 14,
	STW_BTN_HEIGHT = 10,

	STW_BTNPOS_D_X = 1,
	STW_BTNPOS_D_Y = 1,
	STW_BTNPOS_B_X = STW_BTNPOS_D_X + STW_BTN_WIDTH + 1,
	STW_BTNPOS_B_Y = STW_BTNPOS_D_Y,
	STW_BTNPOS_A_X = STW_BTNPOS_D_X,
	STW_BTNPOS_A_Y = STW_BTNPOS_D_Y + STW_BTN_HEIGHT + 1,
	STW_BTNPOS_C_X = STW_BTNPOS_B_X,
	STW_BTNPOS_C_Y = STW_BTNPOS_A_Y,

};
// 描画位置に対応したボタンタッチ位置テーブル
static const GFL_UI_TP_HITTBL STW_HitTbl[] = {
	{ STW_BTNPOS_A_Y*8, (STW_BTNPOS_A_Y+STW_BTN_HEIGHT)*8, STW_BTNPOS_A_X*8, (STW_BTNPOS_A_X+STW_BTN_WIDTH)*8 },
	{ STW_BTNPOS_B_Y*8, (STW_BTNPOS_B_Y+STW_BTN_HEIGHT)*8, STW_BTNPOS_B_X*8, (STW_BTNPOS_B_X+STW_BTN_WIDTH)*8 },
	{ STW_BTNPOS_C_Y*8, (STW_BTNPOS_C_Y+STW_BTN_HEIGHT)*8, STW_BTNPOS_C_X*8, (STW_BTNPOS_C_X+STW_BTN_WIDTH)*8 },
	{ STW_BTNPOS_D_Y*8, (STW_BTNPOS_D_Y+STW_BTN_HEIGHT)*8, STW_BTNPOS_D_X*8, (STW_BTNPOS_D_X+STW_BTN_WIDTH)*8 },

	{ GFL_UI_TP_HIT_END, 0, 0 }
};
// ↑描画位置から上記テーブルインデックスを引くためのテーブル
static const u8 STW_HitTblIndex[] = {
	BTLV_MCSS_POS_A,
	BTLV_MCSS_POS_B,
	BTLV_MCSS_POS_C,
	BTLV_MCSS_POS_D,
};

/*
typedef struct {

	u8 selectablePokeCount;		///< 選択できるポケモンの候補数（0なら選択不可->確認モード）
	u8 confirmCount;					///< 確認モード対象の数
	u8 confirmType;						///< 確認モード対象タイプ（0:ポケモン, 1:場）

	u8 pos[ BTL_POS_MAX ];		///< 選択・確認するポケモンの位置

}SEL_TARGET_WORK;
*/
// 構造体初期化
static void stw_init( SEL_TARGET_WORK* stw )
{
	GFL_STD_MemClear( stw, sizeof( *stw ) );
}
// pos(戦闘位置）配列を、ポケモン選択インターフェイス用のインデックスに変換
static void stw_convert_pos_to_index( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, u8 num )
{
	u8 i;

	for(i=0; i<num; ++i)
	{
		stw->pos[i] = BTL_MAIN_BtlPosToViewPos( mainModule, stw->pos[i] );
	}
}
// 選択可能ポケモンをセット
static void stw_setSelectablePoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos )
{
	u8 num = BTL_MAIN_ExpandBtlPos( mainModule, exPos, stw->pos );
	stw->selectablePokeCount = num;
}
// 確認のみ（ポケモン）をセット
static void stw_setConfirmPoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos )
{
	u8 num = BTL_MAIN_ExpandBtlPos( mainModule, exPos, stw->pos );
	stw->confirmCount = num;
	stw->confirmType = STW_CONFIRM_POKE;
}
// 確認のみ（場）をセット
static void stw_setConfirmField( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos )
{
	u8 num = BTL_MAIN_ExpandBtlPos( mainModule, exPos, stw->pos );
	stw->confirmCount = num;
	stw->confirmType = STW_CONFIRM_FIELD;
}
//
static BOOL stw_is_enable_hitpos( SEL_TARGET_WORK* stw, int hitPos, const BTL_MAIN_MODULE* mainModule, u8* target_idx )
{
	GF_ASSERT_MSG(hitPos<NELEMS(STW_HitTblIndex), "hitPos=%d", hitPos);

	if( stw->selectablePokeCount )
	{
		u8 hitVpos = STW_HitTblIndex[ hitPos ];
		u8 vpos, i;

		for(i=0; i<stw->selectablePokeCount; ++i)
		{
			vpos = BTL_MAIN_BtlPosToViewPos( mainModule, stw->pos[i] );
			if( vpos == hitVpos )
			{
				*target_idx = i;
				return TRUE;
			}
		}
		return FALSE;
	}
	else
	{
		// 確認のみモードの場合、どこをタッチしてもOKにしておく
		*target_idx = 0;
		return TRUE;
	}
}
// 
static inline u8 stwdraw_vpos_to_tblidx( u8 vpos )
{
	u8 i;
	for(i=0; i<NELEMS(STW_HitTblIndex); ++i)
	{
		if( STW_HitTblIndex[i] == vpos )
		{
			return i;
		}
	}
	GF_ASSERT(0);
	return 0;
}
// ボタン描画
static void stwdraw_button( const u8* pos, u8 count, u8 format, BTLV_SCD* wk )
{
	static const struct {
		u8 back;
		u8 letter;
		u8 shadow;
	}color_tbl[] = {
		{ 0x0e, 0x01, 0x02 },
		{ 0x06, 0x0f, 0x02 },
		{ 0x0a, 0x0f, 0x02 },
	};
	const BTL_POKEPARAM* bpp;
	const POKEMON_PARAM* pp;

	u16 x, y, width, height, str_x, str_y, str_width;
	u8 vpos, idx;
	PRINTSYS_LSB printColor;

//	GFL_FONTSYS_SetColor( color_tbl[format].letter, color_tbl[format].shadow, color_tbl[format].back );
	printColor = PRINTSYS_LSB_Make( color_tbl[format].letter, color_tbl[format].shadow, color_tbl[format].back );

	while( count-- )
	{
		bpp = BTL_MAIN_GetFrontPokeDataConst( wk->mainModule, *pos );
		pp  = BTL_POKEPARAM_GetSrcData( bpp );
		vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, *pos );
		idx = stwdraw_vpos_to_tblidx( vpos );

		x = STW_HitTbl[idx].rect.left;
		y = STW_HitTbl[idx].rect.top;
		width = (STW_HitTbl[idx].rect.right - STW_HitTbl[idx].rect.left);
		height = (STW_HitTbl[idx].rect.bottom - STW_HitTbl[idx].rect.top);

		GFL_BMP_Fill( wk->bmp, x, y, width, height, color_tbl[format].back );
		PP_Get( pp, ID_PARA_nickname, wk->strbuf );
		str_width = PRINTSYS_GetStrWidth( wk->strbuf, wk->font, 0 );
		str_x = x + (width - str_width) / 2;
		str_y = y + (height - 16) / 2;

		PRINT_UTIL_PrintColor( &wk->printUtil, wk->printQue, str_x, str_y, wk->strbuf, wk->font, printColor );
		pos++;
	}
//	GFL_FONTSYS_SetDefaultColor();
}
static void stw_draw( const SEL_TARGET_WORK* stw, BTLV_SCD* work )
{
	GFL_BMP_Clear( work->bmp, 0x0f );

	// ポケモン選択モード
	if( stw->selectablePokeCount )
	{
		stwdraw_button( stw->pos, stw->selectablePokeCount, 0, work );
	}
	// 確認のみ
	else
	{
		GF_ASSERT(stw->confirmCount);
		if( stw->confirmType == STW_CONFIRM_POKE )
		{
			stwdraw_button( stw->pos, stw->confirmCount, 1, work );
		}
		else
		{
			stwdraw_button( stw->pos, stw->confirmCount, 2, work );
		}
	}
}
static BOOL stw_draw_wait( BTLV_SCD* wk )
{
	PRINTSYS_QUE_Main( wk->printQue );
	return PRINT_UTIL_Trans( &wk->printUtil, wk->printQue );
}


//=============================================================================================
//	ワザターゲット選択（確認）処理
//=============================================================================================
static BOOL selectTarget_init( int* seq, void* wk_adrs )
{
	BTLV_SCD* wk = wk_adrs;

	switch( *seq ){
	case 0:
		seltgt_init_setup_work( &wk->selTargetWork, wk );
		stw_draw( &wk->selTargetWork, wk );
		(*seq)++;
		break;
	case 1:
		if( stw_draw_wait(wk) )
		{
			(*seq)++;
			return TRUE;
		}
		break;
	default:
		return TRUE;
	}
	return FALSE;
}
static BOOL selectTarget_loop( int* seq, void* wk_adrs )
{
	BTLV_SCD* wk = wk_adrs;

	switch( *seq ){
	case 0:
		{
			int hit = GFL_UI_TP_HitTrg( STW_HitTbl );
			if( hit != GFL_UI_TP_HIT_NONE )
			{
				u8 target_idx;
				if( stw_is_enable_hitpos( &wk->selTargetWork, hit, wk->mainModule, &target_idx ) )
				{
					BTL_ACTION_SetFightParam( wk->destActionParam, wk->destActionParam->fight.wazaIdx, target_idx );
					wk->selTargetDone = TRUE;
					return TRUE;
				}
			}

			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
			{
				wk->selTargetDone = FALSE;
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

static void seltgt_init_setup_work( SEL_TARGET_WORK* stw, BTLV_SCD* wk )
{
	WazaID waza = BTL_POKEPARAM_GetWazaNumber( wk->bpp, wk->destActionParam->fight.wazaIdx );
	WazaTarget target = WAZADATA_GetTarget( waza );
	BtlPokePos  basePos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, BTL_POKEPARAM_GetID(wk->bpp) );

	stw_init( stw );

	switch( target ){
	case WAZA_TARGET_SINGLE:				///< 自分以外の１体（選択）
		stw_setSelectablePoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_WITHOUT_ME, basePos) );
		break;
	case WAZA_TARGET_SINGLE_ENEMY:	///< 敵１体（選択）
		stw_setSelectablePoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_ENEMY_ALL, basePos) );
		break;

	case WAZA_TARGET_ENEMY2:				///< 敵側２体
	case WAZA_TARGET_RANDOM:				///< 敵ランダム
		stw_setConfirmPoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_ENEMY_ALL, basePos) );
		break;
	case WAZA_TARGET_OTHER_ALL:			///< 自分以外全部
		stw_setConfirmPoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_WITHOUT_ME, basePos) );
		break;
	case WAZA_TARGET_ONLY_USER:			///< 自分１体のみ
		stw_setConfirmPoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_DEFAULT, basePos) );
		break;

	case WAZA_TARGET_TEAM_USER:			///< 自分側陣地
		stw_setConfirmField( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_MYSIDE_ALL, basePos) );
		break;

	case WAZA_TARGET_TEAM_ENEMY:		///< 敵側陣地
		stw_setConfirmField( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_ENEMY_ALL, basePos) );
		break;

	case WAZA_TARGET_FIELD:					///< 場に効く（天候系など）
		stw_setConfirmField( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_ALL, basePos) );
		break;

	default:
		GF_ASSERT_MSG(0, "illegal target(%d)", target);
		break;
	}
}


//=============================================================================================
//	ポケモン選択処理
//=============================================================================================


//=============================================================================================
/**
 * ポケモン選択開始
 *
 * @param   wk		
 *
 */
//=============================================================================================
void BTLV_SCD_StartPokemonSelect( BTLV_SCD* wk )
{
//	BTL_UTIL_SetupProc( &wk->subProc, wk, selectPokemon_init, selectPokemon_loop );
	spstack_push( wk, selectPokemon_init, selectPokemon_loop );
}

BOOL BTLV_SCD_WaitPokemonSelect( BTLV_SCD* wk )
{
//	return BTL_UTIL_CallProc( &wk->subProc );
	return spstack_call( wk );
}

void BTLV_SCD_PokeSelect_Start( BTLV_SCD* wk, const BTL_POKESELECT_PARAM* param, BTL_POKESELECT_RESULT* result )
{
	GF_ASSERT( wk->pokesel_param == NULL );

	wk->pokesel_param = param;
	wk->pokesel_result = result;
	BTL_Printf("ポケモン選択開始 : %d 体えらぶ\n", param->numSelect );
	spstack_push( wk, selectPokemon_init, selectPokemon_loop );
}
BOOL BTLV_SCD_PokeSelect_Wait( BTLV_SCD* wk )
{
	return spstack_call( wk );
}

// ポケモン選択画面：構築
static BOOL selectPokemon_init( int* seq, void* wk_adrs )
{
	static const struct {
		u8 x;
		u8 y;
	}winpos[] = {
		{ POKEWIN_1_X, POKEWIN_1_Y },
		{ POKEWIN_2_X, POKEWIN_2_Y },
		{ POKEWIN_3_X, POKEWIN_3_Y },
		{ POKEWIN_4_X, POKEWIN_4_Y },
		{ POKEWIN_5_X, POKEWIN_5_Y },
		{ POKEWIN_6_X, POKEWIN_6_Y },
	};

	BTLV_SCD* wk = wk_adrs;

	switch(*seq) {
	case 0:
		{
			const BTL_PARTY* party;
			const BTL_POKEPARAM* bpp;
			u16 members, hp, i;
			u8 col;

			GFL_BMP_Clear( wk->bmp, 0xff );

			party = wk->pokesel_param->party;
			members = BTL_PARTY_GetMemberCount( party );

			for(i=0; i<members; i++)
			{
				bpp = BTL_PARTY_GetMemberDataConst( party, i );
				hp = BTL_POKEPARAM_GetValue( bpp, BPP_HP );
				col = (hp)? 6 : 4;
				BTL_STR_MakeStatusWinStr( wk->strbuf, bpp, hp );
				GFL_BMP_Fill( wk->bmp, winpos[i].x, winpos[i].y, POKEWIN_WIDTH, POKEWIN_HEIGHT, col );
				PRINT_UTIL_Print( &wk->printUtil, wk->printQue, winpos[i].x+2, winpos[i].y+2, wk->strbuf, wk->font );
			}
			(*seq)++;
		}
		break;

	case 1:
		PRINTSYS_QUE_Main( wk->printQue );
		if( PRINT_UTIL_Trans(&wk->printUtil, wk->printQue) )
		{
			return TRUE;
		}
		break;
	}

	return FALSE;
}

// ポケモン選択画面：動作
static BOOL selectPokemon_loop( int* seq, void* wk_adrs )
{
	enum {
		SEQ_INIT = 0,
		SEQ_WAIT_SELECT,
		SEQ_WARNING_START,
		SEQ_WARNING_WAIT_PRINT,
		SEQ_WARNING_WAIT_AGREE,
	};

	enum {
		POKEWIN_1_BTM = POKEWIN_1_Y+POKEWIN_HEIGHT - 1,
		POKEWIN_1_RGT = POKEWIN_1_X+POKEWIN_WIDTH - 1,
		POKEWIN_2_BTM = POKEWIN_2_Y+POKEWIN_HEIGHT - 1,
		POKEWIN_2_RGT = POKEWIN_2_X+POKEWIN_WIDTH - 1,
		POKEWIN_3_BTM = POKEWIN_3_Y+POKEWIN_HEIGHT - 1,
		POKEWIN_3_RGT = POKEWIN_3_X+POKEWIN_WIDTH - 1,
		POKEWIN_4_BTM = POKEWIN_4_Y+POKEWIN_HEIGHT - 1,
		POKEWIN_4_RGT = POKEWIN_4_X+POKEWIN_WIDTH - 1,
		POKEWIN_5_BTM = POKEWIN_5_Y+POKEWIN_HEIGHT - 1,
		POKEWIN_5_RGT = POKEWIN_5_X+POKEWIN_WIDTH - 1,
		POKEWIN_6_BTM = POKEWIN_6_Y+POKEWIN_HEIGHT - 1,
		POKEWIN_6_RGT = POKEWIN_6_X+POKEWIN_WIDTH - 1,

		WARNWIN_WIDTH = 256-(8*2),
		WARNWIN_HEIGHT = 32,
		WARNWIN_X = 8,
		WARNWIN_Y = 192-8-WARNWIN_HEIGHT,
	};

	static const GFL_UI_TP_HITTBL hitTbl[] = {
		{ POKEWIN_1_Y, POKEWIN_1_BTM, POKEWIN_1_X, POKEWIN_1_RGT },
		{ POKEWIN_2_Y, POKEWIN_2_BTM, POKEWIN_2_X, POKEWIN_2_RGT },
		{ POKEWIN_3_Y, POKEWIN_3_BTM, POKEWIN_3_X, POKEWIN_3_RGT },
		{ POKEWIN_4_Y, POKEWIN_4_BTM, POKEWIN_4_X, POKEWIN_4_RGT },
		{ POKEWIN_5_Y, POKEWIN_5_BTM, POKEWIN_5_X, POKEWIN_5_RGT },
		{ POKEWIN_6_Y, POKEWIN_6_BTM, POKEWIN_6_X, POKEWIN_6_RGT },
		{ GFL_UI_TP_HIT_END, 0, 0, 0 },
	};

	BTLV_SCD* wk = wk_adrs;
	const BTL_POKESELECT_PARAM* param = wk->pokesel_param;
	BTL_POKESELECT_RESULT *res = wk->pokesel_result;

	switch( *seq ){
	case SEQ_INIT:
		BTL_POKESELECT_RESULT_Init( res, param );
		(*seq) = SEQ_WAIT_SELECT;

		/* fallthru */

	// ポケモン名ウィンドウのタッチ選択待ち
	case SEQ_WAIT_SELECT:
		{
			int hitpos = GFL_UI_TP_HitTrg( hitTbl );
			if( hitpos != GFL_UI_TP_HIT_NONE )
			{
				const BTL_PARTY* party = param->party;

				if( hitpos < BTL_PARTY_GetMemberCount(party) )
				{
					const BTL_POKEPARAM* bpp;
					BtlPokeselReason  result;

					bpp = BTL_PARTY_GetMemberDataConst( party, hitpos );
					result = BTL_POKESELECT_CheckProhibit( param, res, hitpos );

					switch( result ){
					default:
						GF_ASSERT(0);
					case BTL_POKESEL_CANT_FIGHTING:
						BTL_STR_MakeWarnStr( wk->strbuf, bpp, BTLSTR_UI_WARN_FightingPoke );
						break;

					case BTL_POKESEL_CANT_DEAD:
						BTL_STR_MakeWarnStr( wk->strbuf, bpp, BTLSTR_UI_WARN_NotAlivePoke );
						break;

					case BTL_POKESEL_CANT_SELECTED:
						BTL_STR_MakeWarnStr( wk->strbuf, bpp, BTLSTR_UI_WARN_SelectedPoke );
						break;

					case BTL_POKESEL_CANT_NONE:
						// --- ここまで来たら正しいポケを選んでる ---
						BTL_POKESELECT_RESULT_Push( res, hitpos );
						if( BTL_POKESELECT_IsDone(res) )
						{
							// 次回に正しく初期化されずに呼び出されたら止まるようにNULLクリアしておく
							wk->pokesel_param = NULL;
							wk->pokesel_result = NULL;
							return TRUE;
						}
						else
						{
							return FALSE;
						}
					}
					// --- ここにくるのは警告メッセージ表示シーケンス ---
					(*seq) = SEQ_WARNING_START;
				}
			}
		}
		break;

	// 警告表示中（タッチ待ち）
	case SEQ_WARNING_START:
		PRINT_UTIL_Print( &wk->printUtil, wk->printQue, WARNWIN_X, WARNWIN_Y, wk->strbuf, wk->font );
		(*seq) = SEQ_WARNING_WAIT_PRINT;
		/* fallthru */
	case SEQ_WARNING_WAIT_PRINT:
		PRINTSYS_QUE_Main( wk->printQue );
		if( PRINT_UTIL_Trans(&wk->printUtil, wk->printQue) )
		{
			(*seq) = SEQ_WARNING_WAIT_AGREE;
		}
		break;
	case SEQ_WARNING_WAIT_AGREE:
		if(	GFL_UI_TP_GetTrg()
		||	GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A)
		){
				GFL_BMP_Fill( wk->bmp, WARNWIN_X, WARNWIN_Y, WARNWIN_WIDTH, WARNWIN_HEIGHT, 0x0f );
				GFL_BMPWIN_TransVramCharacter( wk->win );
				(*seq) = SEQ_WAIT_SELECT;
		}
		break;
	}

	return FALSE;
}

//=============================================================================================
//	「通信待機中」表示オン・オフ
//=============================================================================================

void BTLV_SCD_StartCommWaitInfo( BTLV_SCD* wk )
{
	printCommWait( wk );
}
BOOL BTLV_SCD_WaitCommWaitInfo( BTLV_SCD* wk )
{
	PRINTSYS_QUE_Main( wk->printQue );
	if( PRINT_UTIL_Trans(&wk->printUtil, wk->printQue) )
	{
		return TRUE;
	}
	return FALSE;
}
void BTLV_SCD_ClearCommWaitInfo( BTLV_SCD* wk )
{
	GFL_BMP_Clear( wk->bmp, 0x0f );
	GFL_BMPWIN_TransVramCharacter( wk->win );
}

static void printCommWait( BTLV_SCD* wk )
{
	u32 strWidth, drawX, drawY;

	GFL_BMP_Clear( wk->bmp, 0x0f );

	BTL_STR_GetUIString( wk->strbuf, BTLSTR_UI_COMM_WAIT );
	strWidth = PRINTSYS_GetStrWidth( wk->strbuf, wk->font, 0 );
	drawX = (256 - strWidth) / 2;
	drawY = (192 - 16) / 2;
	PRINT_UTIL_Print( &wk->printUtil, wk->printQue, drawX, drawY, wk->strbuf, wk->font );
}

