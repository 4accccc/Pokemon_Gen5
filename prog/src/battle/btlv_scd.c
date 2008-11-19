//=============================================================================================
/**
 * @file	btlv_scd.c
 * @brief	ポケモンWB バトル 描画 下画面制御モジュール
 * @author	taya
 *
 * @date	2008.11.18	作成
 */
//=============================================================================================

#include "btl_common.h"

#include "btlv_core.h"
#include "btlv_scd.h"

//--------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------
struct _BTLV_SCD {

	const BTLV_CORE* vcore;

};


BTLV_SCD*  BTLV_SCD_Create( const BTLV_CORE* vcore, HEAPID heapID )
{
	BTLV_SCD* wk = GFL_HEAP_AllocMemory( heapID, sizeof(BTLV_SCD) );

	wk->vcore = vcore;

	return wk;
}

void BTLV_SCD_Delete( BTLV_SCD* wk )
{
	GFL_HEAP_FreeMemory( wk );
}



void BTLV_SCD_Setup( BTLV_SCD* wk )
{
#if 0
	// 個別フレーム設定
	static const GFL_BG_BGCNT_HEADER bgcntText = {
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	};

	GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText,   GFL_BG_MODE_TEXT );

		GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

	GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

		GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );

//		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, wk->heapID );
		GFL_ARC_UTIL_TransVramPalette( ARCID_D_TAYA, NARC_d_taya_default_nclr, PALTYPE_MAIN_BG, 0, 0, wk->heapID );
//		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0xff, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

		GFL_ARC_UTIL_TransVramPalette( ARCID_D_TAYA, NARC_d_taya_default_nclr, PALTYPE_SUB_BG, 0, 0, wk->heapID );
		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0xff, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	}

	GFL_BMPWIN_TransVramCharacter( wk->win );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );

	wk->strbuf = GFL_STR_CreateBuffer( 1024, wk->heapID );
	wk->seq = 0;



	wk->printQue = PRINTSYS_QUE_Create( wk->heapID );
	PRINT_UTIL_Setup( wk->printUtil, wk->win );

//	GFL_ASSERT_SetLCDMode();

	return GFL_PROC_RES_FINISH;
#endif

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


void BTLV_SCD_StartActionSelect( BTLV_SCD* wk, const POKEMON_PARAM* pp )
{
	
}

BOOL BTLV_SCD_WaitActionSelect( BTLV_SCD* wk )
{
	return TRUE;
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
