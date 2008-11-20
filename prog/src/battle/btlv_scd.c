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

#include "btl_common.h"
#include "btl_util.h"
#include "btl_string.h"
#include "btl_action.h"

#include "btlv_core.h"
#include "btlv_scd.h"

#include "arc_def.h"
#include "test_graphic/d_taya.naix"
#include "msg/msg_btl_ui.h"

//--------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------
struct _BTLV_SCD {

	GFL_BMPWIN*			win;
	GFL_BMP_DATA*		bmp;
	GFL_FONT*			font;
	STRBUF*				strbuf;

	PRINT_QUE*			printQue;
	PRINT_UTIL			printUtil;

	BTL_PROC			subProc;

	const BTL_POKEPARAM*	bpp;
	BTL_ACTION_PARAM		selAction;

	const BTLV_CORE* vcore;
	HEAPID	heapID;
};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static BOOL selectAction_init( int* seq, void* wk_adrs );
static BOOL selectAction_loop( int* seq, void* wk_adrs );






BTLV_SCD*  BTLV_SCD_Create( const BTLV_CORE* vcore, GFL_TCBLSYS* tcbl, GFL_FONT* font, HEAPID heapID )
{
	BTLV_SCD* wk = GFL_HEAP_AllocMemory( heapID, sizeof(BTLV_SCD) );

	wk->vcore = vcore;
	wk->heapID = heapID;
	wk->font = font;
	wk->strbuf = GFL_STR_CreateBuffer( 64, heapID );

	wk->printQue = PRINTSYS_QUE_Create( wk->heapID );

	return wk;
}

void BTLV_SCD_Delete( BTLV_SCD* wk )
{
	PRINTSYS_QUE_Delete( wk->printQue );
	GFL_STR_DeleteBuffer( wk->strbuf );
	GFL_HEAP_FreeMemory( wk );
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

	GFL_ARC_UTIL_TransVramPalette( ARCID_D_TAYA, NARC_d_taya_default_nclr, PALTYPE_SUB_BG, 0, 0, wk->heapID );

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


#if 0
			
		printf( core->strBuf );
		printf("[A]たたかう  [B]どうぐ  [C]ポケモン  [D]にげる\n");
		(*seq) = SEQ_SELECT_MAIN;
		break;
	case SEQ_SELECT_MAIN:
		switch( getch_upr() ){
		case 'A':
			printf("闘うのね\n");
			(*seq) = SEQ_SEL_FIGHT;
			break;

		case 'B':
			printf("アイテムね\n");
			(*seq) = SEQ_SEL_ITEM;
			break;

		case 'C':
			printf("交替すんのね\n");
			(*seq) = SEQ_SEL_CHANGE;
			break;

		case 'D':
			printf("逃げんのね\n");
			(*seq) = SEQ_SEL_ESCAPE;
			break;

		default:
			return FALSE;
		}
		break;

	case SEQ_SEL_FIGHT:
		{
			const BTL_POKEPARAM* p = BTL_MAIN_GetFrontPokeDataConst( core->mainModule, core->myClientID );
			int i, max = BTL_POKEPARAM_GetWazaCount( p );
			printf("ワザを選んで\n");
			for(i=0; i<max; i++)
			{
				printf("[%c]%s  ", 'A'+i, BTRSTR_GetWazaName( BTL_POKEPARAM_GetWazaNumber(p, i) ));
			}
			printf("[R]もどる\n");
			printf("\n");
		}
		(*seq)++;
		/* fallthru */
	case SEQ_SEL_FIGHT+1:
		{
			int key = getch_upr();

			if( key == 'R' )
			{
				(*seq) = SEQ_INIT;
				break;
			}

			if( key >= 'A' && key <= 'D' )
			{
				wk->idx1 = (key - 'A');
				if( BTL_MAIN_GetRule(core->mainModule) == BTL_RULE_SINGLE )
				{
					// シングル戦なら対象選択シーケンス不要
					BTL_ACTION_SetFightParam( &core->actionParam, wk->idx1, 0 );
					return TRUE;
				}
				else
				{
					(*seq)++;
					break;
				}
			}
		}
		break;
	case SEQ_SEL_FIGHT+2:
		printf("そのワザを誰に使う？  A:あいて B:あいつ C:そいつ D:ドイツ R:もどる\n");
		(*seq)++;
		/* fallthru */
	case SEQ_SEL_FIGHT+3:
		{
			int key = getch_upr();

			if( key == 'R' )
			{
				(*seq) = SEQ_SEL_FIGHT;
				break;
			}

			if( key >= 'A' && key <= 'D' )
			{
				wk->idx2 = (key - 'A');
				BTL_ACTION_SetFightParam( &core->actionParam, wk->idx1, wk->idx2 );
				ret = TRUE;
			}
		}
		break;

	case SEQ_SEL_ITEM:
		printf("なに使う？ A:きずぐすり B:やくそう C:ひのきのぼう D:おにぎり R:もどる\n");
		(*seq)++;
		/* fallthru */
	case SEQ_SEL_ITEM+1:
		{
			int key = getch_upr();

			if( key == 'R' )
			{
				(*seq) = SEQ_INIT;
				break;
			}

			if( key >= 'A' && key <= 'D' )
			{
				wk->idx1 = (key - 'A');
				(*seq)++;
				break;
			}
		}
		break;
	case SEQ_SEL_ITEM+2:
		printf("だれに使う？ A:じぶん  B:なかま  C:あいて  D:あなた  R:もどる\n");
		(*seq)++;
		/* fallthru */
	case SEQ_SEL_ITEM+3:
		{
			int key = getch_upr();

			if( key == 'R' )
			{
				(*seq) = SEQ_SEL_ITEM;
				break;
			}

			if( key >= 'A' && key <= 'D' )
			{
				wk->idx2 = (key - 'A');
				BTL_ACTION_SetItemParam( &core->actionParam, wk->idx1, wk->idx2 );
				ret = TRUE;
			}
		}
		break;

	case SEQ_SEL_CHANGE:
		printf("ポケモンを選んでね\n");
		{
			const BTL_PARTY* party = BTL_MAIN_GetPartyDataConst( core->mainModule, core->myClientID );
			int i;

			wk->maxElems = BTL_PARTY_GetMemberCount( party );
			for(i=0; i<wk->maxElems; i++)
			{
				const BTL_POKEPARAM* pp = BTL_PARTY_GetMemberDataConst( party, i );
				printf("[%c]%s  ", 'A'+i, BTRSTR_GetMonsName( BTL_POKEPARAM_GetMonsNo(pp)));
			}
			printf("[R]もどる\n");
		}
		(*seq)++;
		/* fallthru */
	case SEQ_SEL_CHANGE+1:
		{
			int key = getch_upr();

			if( key == 'R' )
			{
				(*seq) = SEQ_INIT;
				break;
			}

			if( key >= 'A' && key < 'A'+wk->maxElems )
			{
				u16 idx = (key - 'A');

				if( idx != BTL_MAIN_GetFrontMemberIdx(core->mainModule, core->myClientID) )
				{
					BTL_ACTION_SetChangeParam( &core->actionParam, idx );
					return TRUE;
				}
				else
				{
					const BTL_POKEPARAM* pp = BTL_MAIN_GetFrontPokeDataConst( core->mainModule, core->myClientID );
					printf("%sはもう出てるよ\n", BTRSTR_GetMonsName(BTL_POKEPARAM_GetMonsNo(pp)));
					(*seq)=SEQ_SEL_CHANGE;
				}
			}
		}
		break;

	case SEQ_SEL_ESCAPE:
		core->actionParam.escape.cmd = BTL_ACTION_ESCAPE;
		ret = TRUE;
		break;

	}

	return ret;
}

#endif


void BTLV_SCD_StartActionSelect( BTLV_SCD* wk, const BTL_POKEPARAM* bpp )
{
	wk->bpp = bpp;
	BTL_UTIL_SetupProc( &wk->subProc, wk, selectAction_init, selectAction_loop );
}

BOOL BTLV_SCD_WaitActionSelect( BTLV_SCD* wk )
{
	if( BTL_UTIL_CallProc( &wk->subProc ) )
	{
		return TRUE;
	}
	return FALSE;
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

		printBtn( wk,   0,  0, 128, 96, 0x09, BTLMSG_UI_SEL_FIGHT );
		printBtn( wk, 128,  0, 128, 96, 0x0d, BTLMSG_UI_SEL_ITEM );
		printBtn( wk,   0, 96, 128, 96, 0x07, BTLMSG_UI_SEL_POKEMON );
		printBtn( wk, 128, 96, 128, 96, 0x08, BTLMSG_UI_SEL_ESCAPE );

		(*seq)++;
		break;

	case 1:
		if( PRINT_UTIL_Trans(&wk->printUtil, wk->printQue) )
		{
			return TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

static BOOL selectAction_loop( int* seq, void* wk_adrs )
{
	enum {
		SEQ_START = 0,
		SEQ_SEL_FIGHT = 100,
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
				TAYA_Printf("タッチされたよ %d\n", hit);
				switch( hit ){
				default:
				case BTL_ACTION_FIGHT:
					(*seq) = SEQ_SEL_FIGHT;
					break;
				case BTL_ACTION_ITEM:
					(*seq) = SEQ_SEL_FIGHT;
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
			(*seq)++;
		}
		break;
	case SEQ_SEL_FIGHT+1:
		if( PRINT_UTIL_Trans(&wk->printUtil, wk->printQue) )
		{
			(*seq)++;
		}
		break;
	case SEQ_SEL_FIGHT+2:
		{
			int hit = GFL_UI_TP_HitTrg( TP_HitTbl );
			if( hit != GFL_UI_TP_HIT_NONE )
			{
				BTL_ACTION_SetFightParam( &wk->selAction, hit, 0 );
				TAYA_Printf("ワザえらんだよ %d\n", hit);
				return TRUE;
			}
		}
		break;

	}
	return FALSE;
}

void BTLV_SCD_GetSelectAction( BTLV_SCD* wk, BTL_ACTION_PARAM* action )
{
	*action = wk->selAction;
}




void BTLV_SCD_StartPokemonSelect( BTLV_SCD* wk )
{

}

BOOL BTLV_SCD_WaitPokemonSelect( BTLV_SCD* wk )
{
#if 0
	typedef struct {
		u16 idx1;
		u16 idx2;
		u16 maxElems;
	}SEQ_WORK;

	SEQ_WORK* wk = workBufer;

	switch( *seq ){

	case 0:
		printf("ポケモンを選んでね\n");
		{
			const BTL_PARTY* party = BTL_MAIN_GetPartyDataConst( core->mainModule, core->myClientID );
			int i;

			wk->maxElems = BTL_PARTY_GetMemberCount( party );
			for(i=0; i<wk->maxElems; i++)
			{
				const BTL_POKEPARAM* pp = BTL_PARTY_GetMemberDataConst( party, i );
				printf("[%c]%s  ", 'A'+i, BTRSTR_GetMonsName( BTL_POKEPARAM_GetMonsNo(pp)));
			}
		}
		(*seq)++;
		/* fallthru */
	case 1:
		{
			int key = getch_upr();

			if( key >= 'A' && key < 'A'+wk->maxElems )
			{
				u16 idx = (key - 'A');

				if( idx == BTL_MAIN_GetFrontMemberIdx(core->mainModule, core->myClientID) )
				{
					const BTL_POKEPARAM* pp = BTL_MAIN_GetFrontPokeDataConst( core->mainModule, core->myClientID );
					printf("%sはもう出てるよ\n", BTRSTR_GetMonsName(BTL_POKEPARAM_GetMonsNo(pp)));
					(*seq)=0;
				}
				else
				{
					const BTL_PARTY* party = BTL_MAIN_GetPartyDataConst( core->mainModule, core->myClientID );
					const BTL_POKEPARAM* pp = BTL_PARTY_GetMemberDataConst( party, idx );
					if( BTL_POKEPARAM_GetValue(pp, BPP_HP) == 0 )
					{
						printf("%sはたたかえません！\n", BTRSTR_GetMonsName(BTL_POKEPARAM_GetMonsNo(pp)));
						(*seq)=0;
					}
					else
					{
						BTL_ACTION_SetChangeParam( &core->actionParam, idx );
						return TRUE;
					}
				}
			}
		}
		break;
	}

#endif
	return TRUE;
}
