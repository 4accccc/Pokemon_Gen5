//============================================================================================
/**
 * @file		dpc_seq.c
 * @brief		殿堂入り確認画面 シーケンス処理
 * @author	Hiroyuki Nakamura
 * @date		10.03.29
 *
 *	モジュール名：DPCSEQ
 */
//============================================================================================
#include <gflib.h>

#include "system/wipe.h"
#include "sound/pm_sndsys.h"
#include "app/app_menu_common.h"

#include "dpc_main.h"
#include "dpc_seq.h"
#include "dpc_bmp.h"
#include "dpc_obj.h"
#include "dpc_ui.h"
#include "dpc_snd_def.h"


//============================================================================================
//	定数定義
//============================================================================================

// メインシーケンス
enum {
	MAINSEQ_INIT = 0,
	MAINSEQ_RELEASE,
	MAINSEQ_WIPE,
	MAINSEQ_BUTTON_ANM,

	MAINSEQ_PAGE_MAIN,
	MAINSEQ_PAGE_CHANGE,

	MAINSEQ_POKE_INIT,
	MAINSEQ_POKE_MAIN,
	MAINSEQ_POKE_EXIT,
	MAINSEQ_POKE_MOVE,

	MAINSEQ_END_SET,

	MAINSEQ_END,
};

#define	POKE_MOVE_RAD		( 4 )


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( DPCMAIN_WORK * wk );
static int MainSeq_Release( DPCMAIN_WORK * wk );
static int MainSeq_Wipe( DPCMAIN_WORK * wk );
static int MainSeq_ButtonAnm( DPCMAIN_WORK * wk );
static int MainSeq_PageMain( DPCMAIN_WORK * wk );
static int MainSeq_PageChange( DPCMAIN_WORK * wk );
static int MainSeq_PokeInit( DPCMAIN_WORK * wk );
static int MainSeq_PokeMain( DPCMAIN_WORK * wk );
static int MainSeq_PokeExit( DPCMAIN_WORK * wk );
static int MainSeq_PokeMove( DPCMAIN_WORK * wk );
static int MainSeq_EndSet( DPCMAIN_WORK * wk );

static int SetFadeIn( DPCMAIN_WORK * wk, int next );
static int SetFadeOut( DPCMAIN_WORK * wk, int next );
static int SetButtonAnime( DPCMAIN_WORK * wk, u32 id, u32 anm, int next );

static BOOL ChangePage( DPCMAIN_WORK * wk, s8 mv );
static BOOL ChangePoke( DPCMAIN_WORK * wk, s8 mv );
static BOOL GetPokeMoveVec( DPCMAIN_WORK * wk, u32 pos );
static u8 GetPokeRotationVal( DPCMAIN_WORK * wk, s8 now, s8 pos, s8 mv );

static void MakePokePosRad( DPCMAIN_WORK * wk, s8 mv );
static BOOL MovePokeObj( DPCMAIN_WORK * wk );

FS_EXTERN_OVERLAY(ui_common);


//============================================================================================
//	グローバル
//============================================================================================

// メインシーケンス
static const pDENDOUPC_FUNC MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_Wipe,
	MainSeq_ButtonAnm,

	MainSeq_PageMain,
	MainSeq_PageChange,

	MainSeq_PokeInit,
	MainSeq_PokeMain,
	MainSeq_PokeExit,
	MainSeq_PokeMove,

	MainSeq_EndSet,
};

// ポケモン移動速度
static const u8 PokeMoveSpeed[] = { 5, 5, 5, 5, 4, 4 };

// ポケモン数に対応した移動量
static const u8 PokeMoveCount[] = {
	DPCOBJ_POKEMAX1_SPACE_RAD,
	DPCOBJ_POKEMAX2_SPACE_RAD,
	DPCOBJ_POKEMAX3_SPACE_RAD,
	DPCOBJ_POKEMAX4_SPACE_RAD,
	DPCOBJ_POKEMAX5_SPACE_RAD,
	DPCOBJ_POKEMAX6_SPACE_RAD,
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = 終了"
 */
//--------------------------------------------------------------------------------------------
int DPCSEQ_MainSeq( DPCMAIN_WORK * wk )
{
	wk->mainSeq = MainSeq[wk->mainSeq]( wk );
	if( wk->mainSeq == MAINSEQ_END ){
		return FALSE;
	}

	DPCOBJ_AnmMain( wk );
	DPCBMP_PrintUtilTrans( wk );

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Init( DPCMAIN_WORK * wk )
{
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// サブ画面をメインに
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );
	// 輝度を最低にしておく
	GX_SetMasterBrightness( -16 );
	GXS_SetMasterBrightness( -16 );

	DPCMAIN_CreatePokeData( wk );

	DPCMAIN_InitVram();
	DPCMAIN_InitBg();
	DPCMAIN_LoadBgGraphic();
	DPCMAIN_InitPaletteFade( wk );
	DPCMAIN_InitMsg( wk );

	DPCBMP_Init( wk );
	DPCOBJ_Init( wk );

	MainSeq_PageChange( wk );

	DPCMAIN_SetBlendAlpha();

	GFL_NET_WirelessIconEasy_HoldLCD( TRUE, HEAPID_DENDOU_PC );
	GFL_NET_ReloadIcon();

	DPCMAIN_InitVBlank( wk );

	return SetFadeIn( wk, MAINSEQ_PAGE_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Release( DPCMAIN_WORK * wk )
{
	DPCMAIN_ExitVBlank( wk );

	DPCOBJ_Exit( wk );
	DPCBMP_Exit( wk );

	DPCMAIN_ExitMsg( wk );
	DPCMAIN_ExitPaletteFade( wk );
	DPCMAIN_ExitBg();

	DPCMAIN_ExitPokeData( wk );

	// 輝度を最低にしておく
	GX_SetMasterBrightness( -16 );
	GXS_SetMasterBrightness( -16 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );

	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

	return MAINSEQ_END;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Wipe( DPCMAIN_WORK * wk )
{
	if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
		if( WIPE_SYS_EndCheck() == TRUE ){
			return wk->nextSeq;
		}
	}
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ButtonAnm( DPCMAIN_WORK * wk )
{
	if( DPCOBJ_CheckAnm( wk, wk->buttonID ) == FALSE ){
		return wk->buttonSeq;
	}
	return MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PageMain( DPCMAIN_WORK * wk )
{
	int	ret;

	if( PRINTSYS_QUE_IsFinished( wk->que ) == FALSE ){
		return MAINSEQ_PAGE_MAIN;
	}

	ret = DPCUI_PageMain( wk );

	switch( ret ){
	case DPCUI_ID_LEFT:
		if( ChangePage( wk, -1 ) == TRUE ){
			PMSND_PlaySE( DPC_SE_PAGE_CHANGE );
			return SetButtonAnime( wk, DPCOBJ_ID_ARROW_L, APP_COMMON_BARICON_CURSOR_LEFT_ON, MAINSEQ_PAGE_CHANGE );
		}
		break;

	case DPCUI_ID_RIGHT:
		if( ChangePage( wk, 1 ) == TRUE ){
			PMSND_PlaySE( DPC_SE_PAGE_CHANGE );
			return SetButtonAnime( wk, DPCOBJ_ID_ARROW_R, APP_COMMON_BARICON_CURSOR_RIGHT_ON, MAINSEQ_PAGE_CHANGE );
		}
		break;

	case DPCUI_ID_EXIT:
		PMSND_PlaySE( DPC_SE_EXIT );
		wk->dat->retMode = DENDOUPC_RET_CLOSE;
		return SetButtonAnime( wk, DPCOBJ_ID_EXIT, APP_COMMON_BARICON_EXIT_ON, MAINSEQ_END_SET );

	case DPCUI_ID_RETURN:
		PMSND_PlaySE( DPC_SE_CANCEL );
		wk->dat->retMode = DENDOUPC_RET_NORMAL;
		return SetButtonAnime( wk, DPCOBJ_ID_RETURN, APP_COMMON_BARICON_RETURN_ON, MAINSEQ_END_SET );

	case DPCUI_ID_MODE_CHANGE:
		PMSND_PlaySE( DPC_SE_POKE_MODE );
		wk->pokeChg = wk->pokePos;
		return MAINSEQ_POKE_INIT;

	case DPCUI_ID_POKE1:
	case DPCUI_ID_POKE2:
	case DPCUI_ID_POKE3:
	case DPCUI_ID_POKE4:
	case DPCUI_ID_POKE5:
	case DPCUI_ID_POKE6:
		wk->pokeChg = ret - DPCUI_ID_POKE1;
//		if( wk->pokeChg == wk->pokePos ){
			PMSND_PlaySE( DPC_SE_POKE_MODE );
//		}
		return MAINSEQ_POKE_INIT;

	default:
		break;
	}

	return MAINSEQ_PAGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PageChange( DPCMAIN_WORK * wk )
{
	wk->pokePos = 0;

	DPCOBJ_AddPoke( wk );

	DPCBMP_PutTitle( wk );
	DPCBMP_PutPage( wk );
//	DPCBMP_PutInfo( wk );

	return MAINSEQ_PAGE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PokeInit( DPCMAIN_WORK * wk )
{
	switch( wk->subSeq ){
	case 0:
		DPCOBJ_SetAutoAnm( wk, DPCOBJ_ID_ARROW_L, APP_COMMON_BARICON_CURSOR_LEFT_OFF );
		DPCOBJ_SetAutoAnm( wk, DPCOBJ_ID_ARROW_R, APP_COMMON_BARICON_CURSOR_RIGHT_OFF );
		DPCOBJ_SetAutoAnm( wk, DPCOBJ_ID_EXIT, APP_COMMON_BARICON_EXIT_OFF );
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON );
		DPCOBJ_InitFadeEvy( wk, TRUE );
		DPCMAIN_RequestPaletteFade( wk );
		wk->subSeq++;
		break;

	case 1:
		if( DPCMAIN_CheckPaletteFade( wk ) == FALSE ){
			wk->subSeq = 0;
			if( wk->pokeChg == wk->pokePos ){
				wk->pokePos = wk->pokeChg;
				DPCBMP_PutInfo( wk );
				return MAINSEQ_POKE_MAIN;
			}else{
				s8	now = wk->pokePos;
				wk->pokePos = wk->pokeChg;
				// 左
				if( GetPokeMoveVec( wk, wk->pokeChg ) == TRUE ){
					wk->pokeMove = -PokeMoveSpeed[wk->party[wk->page].pokeMax-1];
					MakePokePosRad( wk, -GetPokeRotationVal(wk,now,wk->pokeChg,1) );
				// 右
				}else{
					wk->pokeMove = PokeMoveSpeed[wk->party[wk->page].pokeMax-1];
					MakePokePosRad( wk, GetPokeRotationVal(wk,now,wk->pokeChg,-1) );
				}
				PMSND_PlaySE( DPC_SE_POKE_CHANGE );
				return MAINSEQ_POKE_MOVE;
			}
		}
		break;
	}

	return MAINSEQ_POKE_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PokeMain( DPCMAIN_WORK * wk )
{
	int	ret;

	if( PRINTSYS_QUE_IsFinished( wk->que ) == FALSE ){
		return MAINSEQ_POKE_MAIN;
	}

	ret = DPCUI_PokeMain( wk );

	switch( ret ){
	case DPCUI_ID_LEFT:
		if( ChangePoke( wk, 1 ) == TRUE ){
			wk->pokeMove = -PokeMoveSpeed[wk->party[wk->page].pokeMax-1];
			MakePokePosRad( wk, -1 );
			PMSND_PlaySE( DPC_SE_POKE_CHANGE );
			return MAINSEQ_POKE_MOVE;
		}
		break;

	case DPCUI_ID_RIGHT:
		if( ChangePoke( wk, -1 ) == TRUE ){
			wk->pokeMove = PokeMoveSpeed[wk->party[wk->page].pokeMax-1];
			MakePokePosRad( wk, 1 );
			PMSND_PlaySE( DPC_SE_POKE_CHANGE );
			return MAINSEQ_POKE_MOVE;
		}
		break;

	case DPCUI_ID_RETURN:
		PMSND_PlaySE( DPC_SE_CANCEL );
		return SetButtonAnime( wk, DPCOBJ_ID_RETURN, APP_COMMON_BARICON_RETURN_ON, MAINSEQ_POKE_EXIT );

	case DPCUI_ID_POKE1:
	case DPCUI_ID_POKE2:
	case DPCUI_ID_POKE3:
	case DPCUI_ID_POKE4:
	case DPCUI_ID_POKE5:
	case DPCUI_ID_POKE6:
		{
			ret -= DPCUI_ID_POKE1;
			if( ret != wk->pokePos ){
				s8	now = wk->pokePos;
				wk->pokePos = ret;
				// 左
				if( GetPokeMoveVec( wk, ret ) == TRUE ){
					wk->pokeMove = -PokeMoveSpeed[wk->party[wk->page].pokeMax-1];
					MakePokePosRad( wk, -GetPokeRotationVal(wk,now,ret,1) );
				// 右
				}else{
					wk->pokeMove = PokeMoveSpeed[wk->party[wk->page].pokeMax-1];
					MakePokePosRad( wk, GetPokeRotationVal(wk,now,ret,-1) );
				}
				PMSND_PlaySE( DPC_SE_POKE_CHANGE );
				return MAINSEQ_POKE_MOVE;
			}
		}
		break;

	default:
		break;
	}

	return MAINSEQ_POKE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PokeExit( DPCMAIN_WORK * wk )
{
	switch( wk->subSeq ){
	case 0:
		DPCOBJ_InitFadeEvy( wk, FALSE );
		DPCMAIN_RequestPaletteFade( wk );
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_OFF );
		wk->subSeq++;
		break;

	case 1:
		if( DPCMAIN_CheckPaletteFade( wk ) == FALSE ){
			DPCOBJ_SetAutoAnm( wk, DPCOBJ_ID_ARROW_L, APP_COMMON_BARICON_CURSOR_LEFT );
			DPCOBJ_SetAutoAnm( wk, DPCOBJ_ID_ARROW_R, APP_COMMON_BARICON_CURSOR_RIGHT );
			DPCOBJ_SetAutoAnm( wk, DPCOBJ_ID_EXIT, APP_COMMON_BARICON_EXIT );
			DPCBMP_ClearInfo( wk );
			wk->subSeq = 0;
			return MAINSEQ_PAGE_MAIN;
		}
	}

	return MAINSEQ_POKE_EXIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_PokeMove( DPCMAIN_WORK * wk )
{
	u32	i;

	if( MovePokeObj( wk ) == FALSE ){
		for( i=0; i<wk->party[wk->page].pokeMax; i++ ){
			wk->posEvy[i] = wk->tmpEvy[i];
		}
		DPCMAIN_CheckPaletteFade( wk );
		DPCBMP_PutInfo( wk );
		return MAINSEQ_POKE_MAIN;
	}

	for( i=0; i<wk->party[wk->page].pokeMax; i++ ){
		if( wk->nowEvy[i] > wk->tmpEvy[i] ){
			wk->nowEvy[i]--;
		}else if( wk->nowEvy[i] < wk->tmpEvy[i] ){
			wk->nowEvy[i]++;
		}
	}
	DPCMAIN_RequestPaletteFade( wk );
	return MAINSEQ_POKE_MOVE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_EndSet( DPCMAIN_WORK * wk )
{
	return SetFadeOut( wk, MAINSEQ_RELEASE );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードイン設定
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 * @param		next	フェード後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetFadeIn( DPCMAIN_WORK * wk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_DENDOU_PC );
	wk->nextSeq = next;
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウト設定
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 * @param		next	フェード後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetFadeOut( DPCMAIN_WORK * wk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_DENDOU_PC );
	wk->nextSeq = next;
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボタンアニメ設定
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 * @param		id		ボタンＩＤ
 * @param		anm		アニメ番号
 * @param		next	アニメ後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetButtonAnime( DPCMAIN_WORK * wk, u32 id, u32 anm, int next )
{
	DPCOBJ_SetAutoAnm( wk, id, anm );
	wk->buttonID  = id;
	wk->buttonSeq = next;
	return MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 * @param		mv		切り替え方向
 *
 * @retval	"TRUE = 切り替え可"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL ChangePage( DPCMAIN_WORK * wk, s8 mv )
{
	s8	tmp = wk->page;

	wk->page += mv;
	if( wk->page < 0 ){
		wk->page = wk->pageMax - 1;
	}else if( wk->page >= wk->pageMax ){
		wk->page = 0;
	}
	if( wk->page == tmp ){
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン切り替え
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 * @param		mv		切り替え方向
 *
 * @retval	"TRUE = 切り替え可"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL ChangePoke( DPCMAIN_WORK * wk, s8 mv )
{
	s8	tmp = wk->pokePos;

	wk->pokePos += mv;
	if( wk->pokePos < 0 ){
		wk->pokePos = wk->party[wk->page].pokeMax - 1;
	}else if( wk->pokePos >= wk->party[wk->page].pokeMax ){
		wk->pokePos = 0;
	}
	if( wk->pokePos == tmp ){
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン移動方向取得
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 * @param		pos		位置
 *
 * @retval	"TRUE = 現在位置が画面左"
 * @retval	"FALSE = 現在位置が画面右"
 */
//--------------------------------------------------------------------------------------------
static BOOL GetPokeMoveVec( DPCMAIN_WORK * wk, u32 pos )
{
	s16	x, y;

	DPCOBJ_GetPos( wk, DPCOBJ_GetDefaultPoke(wk)+pos, &x, &y );

	if( x < 128 ){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン移動量取得
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 * @param		now		現在位置
 * @param		pos		移動位置
 * @param		mv		位置方向
 *
 * @return	移動量
 */
//--------------------------------------------------------------------------------------------
static u8 GetPokeRotationVal( DPCMAIN_WORK * wk, s8 now, s8 pos, s8 mv )
{
	u8	i;

	for( i=1; i<6; i++ ){
		now += mv;
		if( now < 0 ){
			now = wk->party[wk->page].pokeMax - 1;
		}else if( now >= wk->party[wk->page].pokeMax ){
			now = 0;
		}
		if( now == pos ){
			break;
		}
	}
	return i;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン移動データ作成
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 * @param		mv		移動量
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MakePokePosRad( DPCMAIN_WORK * wk, s8 mv )
{
	s16	pos1, pos2;
	u32	i;

	pos1 = wk->pokePos;
	pos2 = wk->pokePos + mv;
	if( pos2 < 0 ){
		pos2 += wk->party[wk->page].pokeMax;
	}else if( pos2 >= wk->party[wk->page].pokeMax ){
		pos2 -= wk->party[wk->page].pokeMax;
	}

	for( i=0; i<wk->party[wk->page].pokeMax; i++ ){

		wk->posRad[pos1] = wk->nowRad[pos2];
		wk->tmpEvy[pos1] = wk->posEvy[pos2];

		pos1 += (mv/GFL_STD_Abs(mv));
		if( pos1 < 0 ){
			pos1 = wk->party[wk->page].pokeMax - 1;
		}else if( pos1 >= wk->party[wk->page].pokeMax ){
			pos1 = 0;
		}
		pos2 += (mv/GFL_STD_Abs(mv));
		if( pos2 < 0 ){
			pos2 = wk->party[wk->page].pokeMax - 1;
		}else if( pos2 >= wk->party[wk->page].pokeMax ){
			pos2 = 0;
		}
	}

	wk->pokeMoveCnt = PokeMoveCount[wk->party[wk->page].pokeMax-1] / GFL_STD_Abs(wk->pokeMove) * GFL_STD_Abs(mv);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン移動
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @retval	"TRUE = 移動中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL MovePokeObj( DPCMAIN_WORK * wk )
{
	u32	id;
	u32	i;

	id = DPCOBJ_GetDefaultPoke( wk );

	if( wk->pokeMoveCnt == 0 ){
		for( i=0; i<wk->party[wk->page].pokeMax; i++ ){
			wk->nowRad[i] = wk->posRad[i];
			DPCOBJ_SetPokePos( wk, id+i, wk->nowRad[i] );
		}
		DPCOBJ_ChangePokePriority( wk );
		return FALSE;
	}

	for( i=0; i<wk->party[wk->page].pokeMax; i++ ){
		wk->nowRad[i] += wk->pokeMove;
		if( wk->nowRad[i] < 0 ){
			wk->nowRad[i] += 360;
		}else if( wk->nowRad[i] >= 360 ){
			wk->nowRad[i] -= 360;
		}
		DPCOBJ_SetPokePos( wk, id+i, wk->nowRad[i] );
	}
	DPCOBJ_ChangePokePriority( wk );

	wk->pokeMoveCnt--;

	return TRUE;
}
