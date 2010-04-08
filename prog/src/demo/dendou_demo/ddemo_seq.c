//============================================================================================
/**
 * @file		ddemo_seq.c
 * @brief		殿堂入りデモ画面
 * @author	Hiroyuki Nakamura
 * @date		10.04.06
 *
 *	モジュール名：DDEMOSEQ
 */
//============================================================================================
#include <gflib.h>

#include "system/wipe.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "sound/pm_wb_voice.h"

#include "ddemo_main.h"
#include "ddemo_seq.h"
#include "ddemo_obj.h"
#include "ddemo_spa.h"


static int MainSeq_Init( DDEMOMAIN_WORK * wk );
static int MainSeq_Release( DDEMOMAIN_WORK * wk );
static int MainSeq_Wipe( DDEMOMAIN_WORK * wk );
static int MainSeq_Wait( DDEMOMAIN_WORK * wk );

static int MainSeq_1stMain( DDEMOMAIN_WORK * wk );
static int MainSeq_2ndMain( DDEMOMAIN_WORK * wk );

static int SetFadeIn( DDEMOMAIN_WORK * wk, int next );
static int SetFadeOut( DDEMOMAIN_WORK * wk, int next );

static int SetWait( DDEMOMAIN_WORK * wk, int wait );

static void MakeRandMoveParam( DDEMOMAIN_WORK * wk );
static BOOL MoveObjRand( DDEMOMAIN_WORK * wk, u32 id );


static const pDDEMOMAIN_FUNC MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_Wipe,
	MainSeq_Wait,

	MainSeq_1stMain,
	MainSeq_2ndMain,
};

static const u8 TypePtcTbl[] = {
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
	WARTER_1,
};




BOOL DDEMOSEQ_MainSeq( DDEMOMAIN_WORK * wk )
{
	wk->mainSeq = MainSeq[wk->mainSeq]( wk );
	if( wk->mainSeq == MAINSEQ_END ){
		return FALSE;
	}

	DDEMOOBJ_AnmMain( wk );
	DDEMOMAIN_Main3D( wk );
	GFL_G3D_DOUBLE3D_SetSwapFlag();

	return TRUE;
}


static int MainSeq_Init( DDEMOMAIN_WORK * wk )
{
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

	DDEMOMAIN_InitVram();
	DDEMOMAIN_InitBg();
	DDEMOMAIN_InitMsg( wk );

	DDEMOMAIN_Init3D( wk );
	DDEMOMAIN_InitParticle( wk );

	DDEMOOBJ_Init( wk );

	DDEMOMAIN_SetBlendAlpha();

	DDEMOMAIN_GetPokeMax( wk );

	DDEMOMAIN_InitSound( wk );

	DDEMOMAIN_InitVBlank( wk );

	return SetFadeIn( wk, MAINSEQ_1ST_MAIN );
}

static int MainSeq_Release( DDEMOMAIN_WORK * wk )
{
	DDEMOMAIN_ExitVBlank( wk );

	DDEMOMAIN_ExitSound( wk );

	DDEMOOBJ_Exit( wk );

	DDEMOMAIN_ExitParticle( wk );
	DDEMOMAIN_Exit3D( wk );

	DDEMOMAIN_ExitMsg( wk );
	DDEMOMAIN_ExitBg();

	// 輝度を最低にしておく
	GX_SetMasterBrightness( -16 );
	GXS_SetMasterBrightness( -16 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );

	return MAINSEQ_END;
}

static int MainSeq_Wipe( DDEMOMAIN_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return wk->nextSeq;
	}
	return MAINSEQ_WIPE;
}

static int MainSeq_Wait( DDEMOMAIN_WORK * wk )
{
	if( wk->wait == 0 ){
		return wk->nextSeq;
	}
	wk->wait--;
	return MAINSEQ_WAIT;
}


#define	DEF_1ST_START_WAIT		( 32 )
#define	DEF_1ST_PM_LEFT_SPEED	( -16 )
#define	DEF_1ST_PM_LEFT_COUNT	( (256+96) / GFL_STD_Abs(DEF_1ST_PM_LEFT_SPEED) )
#define	DEF_1ST_PM_LEFT_WAIT	( 8 )
#define	DEF_1ST_TYPE_EFF_WAIT	( 8 )

#define	DEF_1ST_PM_RIGHT_SPEED	( 16 )
#define	DEF_1ST_PM_RIGHT_EPX		( 48 )
#define	DEF_1ST_MM_RIGHT_EPX		( 21*8 )
#define	DEF_1ST_IM_RIGHT_EPX		( 11*8 )
#define	DEF_1ST_PM_RIGHT_WAIT		( 128 )

#define	DEF_1ST_PM_OUT_SPEED	( 16 )
#define	DEF_1ST_PM_OUT_EPX		( -48 )
#define	DEF_1ST_MM_OUT_EPX		( -8 )
#define	DEF_1ST_IM_OUT_EPX		( 256+8 )

#define	DEF_1ST_LOOP_WAIT		( 32 )


static int MainSeq_1stMain( DDEMOMAIN_WORK * wk )
{
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		PMSND_StopBGM();
		return SetFadeOut( wk, MAINSEQ_RELEASE );
	}

	switch( wk->subSeq ){
	case 0:		// 初期ウェイト
		wk->subSeq++;
//		PMSND_PlayBGM( SEQ_BGM_E_DENDOUIRI );
		return SetWait( wk, DEF_1ST_START_WAIT );

	case 1:
		PMSND_PlaySE( SEQ_SE_DDEMO_02A );
		wk->subSeq++;
		break;

	case 2:
    if( PMSND_CheckPlayingSEIdx(SEQ_SE_DDEMO_02A) == FALSE ){
			PMSND_PlaySE( SEQ_SE_DDEMO_02B );
			wk->subSeq++;
		}
		break;

	case 3:		// ポケモンセット
    if( PMSND_CheckPlayingSEIdx(SEQ_SE_DDEMO_02B) == FALSE ){
			PMSND_PlaySE( SEQ_SE_DDEMO_01 );
			DDEMOMAIN_GetPokeData( wk );
			DDEMOMAIN_LoadPokeVoice( wk );
			DDEMOOBJ_AddPoke( wk );
			DDEMOOBJ_PrintPokeInfo( wk );
			DDEMOOBJ_MoveFontOamPos( wk );
			wk->subSeq++;
		}
		break;

	case 4:		// 右から左へポケモン移動
		if( wk->wait == DEF_1ST_PM_LEFT_COUNT ){
			wk->subSeq++;
			return SetWait( wk, DEF_1ST_PM_LEFT_WAIT );
		}else{
			DDEMOOBJ_Move( wk, DDEMOOBJ_ID_POKE_B, DEF_1ST_PM_LEFT_SPEED, 0 );
			wk->wait++;
		}
		break;

	case 5:		// タイプ別エフェクト表示
		DDEMOMAIN_SetEffectParticle( wk, TypePtcTbl[wk->type] );
		wk->subSeq++;
		return SetWait( wk, DEF_1ST_TYPE_EFF_WAIT );

	case 6:		// メッセージ移動
		{
			s16	x, y;
			DDEMOOBJ_GetPos( wk, DDEMOOBJ_ID_MES, &x, &y );
			if( x != DEF_1ST_MM_RIGHT_EPX ){
				DDEMOOBJ_Move( wk, DDEMOOBJ_ID_MES, DEF_1ST_PM_RIGHT_SPEED, 0 );
			}else{
				PMSND_PlaySE( SEQ_SE_ROTATION_B );
				MakeRandMoveParam( wk );
				wk->rndDx = x;
				wk->rndDy = y;
				wk->subSeq++;
			}
			DDEMOOBJ_MoveFontOamPos( wk );
		}
		break;

	case 7:		// 情報移動
		{
			BOOL	flg;
			s16	x, y;
			flg = MoveObjRand( wk, DDEMOOBJ_ID_MES );
			DDEMOOBJ_GetPos( wk, DDEMOOBJ_ID_INFO, &x, &y );
			if( x != DEF_1ST_IM_RIGHT_EPX ){
				DDEMOOBJ_Move( wk, DDEMOOBJ_ID_INFO, -DEF_1ST_PM_RIGHT_SPEED, 0 );
			}else if( flg == FALSE ){
				PMSND_PlaySE( SEQ_SE_ROTATION_B );
				MakeRandMoveParam( wk );
				wk->rndDx = x;
				wk->rndDy = y;
				wk->subSeq++;
			}
			DDEMOOBJ_MoveFontOamPos( wk );
		}
		break;

	case 8:		// ポケモン移動
		{
			BOOL	flg;
			s16	x, y;
			flg = MoveObjRand( wk, DDEMOOBJ_ID_INFO );
			DDEMOOBJ_GetPos( wk, DDEMOOBJ_ID_POKE, &x, &y );
			if( x != DEF_1ST_PM_RIGHT_EPX ){
				DDEMOOBJ_Move( wk, DDEMOOBJ_ID_POKE, DEF_1ST_PM_RIGHT_SPEED, 0 );
			}else if( flg == FALSE ){
				PMSND_PlaySE( SEQ_SE_ROTATION_B );
				MakeRandMoveParam( wk );
				wk->rndDx = x;
				wk->rndDy = y;
				wk->subSeq++;
			}
			DDEMOOBJ_MoveFontOamPos( wk );
		}
		break;

	case 9:		// 鳴き声＆キラキラ
		if( MoveObjRand( wk, DDEMOOBJ_ID_POKE ) == FALSE ){
			PMVOICE_PlayOnly( wk->voicePlayer );
			wk->subSeq++;
			return SetWait( wk, DEF_1ST_PM_RIGHT_WAIT );
		}
		break;

	case 10:		// 鳴き声＆キラキラ待ち
		if( PMVOICE_CheckPlay( wk->voicePlayer ) == FALSE ){
			wk->subSeq++;
		}
		break;

	case 11:		// ポケモンアウト	
		{
			s16	x, y;
			DDEMOOBJ_GetPos( wk, DDEMOOBJ_ID_POKE, &x, &y );
			if( x != DEF_1ST_PM_OUT_EPX ){
				DDEMOOBJ_Move( wk, DDEMOOBJ_ID_POKE, -DEF_1ST_PM_OUT_SPEED, 0 );
			}else{
				wk->subSeq++;
			}
		}
		break;

	case 12:		// 情報アウト
		{
			s16	x, y;
			DDEMOOBJ_GetPos( wk, DDEMOOBJ_ID_INFO, &x, &y );
			if( x != DEF_1ST_IM_OUT_EPX ){
				DDEMOOBJ_Move( wk, DDEMOOBJ_ID_INFO, DEF_1ST_PM_OUT_SPEED, 0 );
			}else{
				wk->subSeq++;
			}
			DDEMOOBJ_MoveFontOamPos( wk );
		}
		break;

	case 13:		// メッセージアウト
		{
			s16	x, y;
			DDEMOOBJ_GetPos( wk, DDEMOOBJ_ID_MES, &x, &y );
			if( x != DEF_1ST_MM_OUT_EPX ){
				DDEMOOBJ_Move( wk, DDEMOOBJ_ID_MES, -DEF_1ST_PM_OUT_SPEED, 0 );
				DDEMOOBJ_MoveFontOamPos( wk );
			}else{
				wk->subSeq++;
				return SetFadeOut( wk, MAINSEQ_1ST_MAIN );
			}
		}
		break;

	case 14:		// 終了チェック
		wk->pokePos++;
		if( wk->pokePos == wk->pokeMax ){
			wk->subSeq = 0;
			return MAINSEQ_2ND_MAIN;
		}else{
			wk->subSeq = 3;
			return SetFadeIn( wk, MAINSEQ_1ST_MAIN );
		}
	}

	return MAINSEQ_1ST_MAIN;
}



static int MainSeq_2ndMain( DDEMOMAIN_WORK * wk )
{
	PMSND_StopBGM();
	return MAINSEQ_RELEASE;

/*
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		return SetFadeOut( wk, MAINSEQ_RELEASE );
	}
	return MAINSEQ_2ND_MAIN;
*/
}




static int SetFadeIn( DDEMOMAIN_WORK * wk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_DENDOU_DEMO );
	wk->nextSeq = next;
	return MAINSEQ_WIPE;
}

static int SetFadeOut( DDEMOMAIN_WORK * wk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_DENDOU_DEMO );
	wk->nextSeq = next;
	return MAINSEQ_WIPE;
}

static int SetWait( DDEMOMAIN_WORK * wk, int wait )
{
	wk->wait = wait;
	wk->nextSeq = wk->mainSeq;
	return MAINSEQ_WAIT;
}



static void MakeRandMoveParam( DDEMOMAIN_WORK * wk )
{
	u32	i;

	wk->rndCnt = 0;

	for( i=0; i<8; i++ ){
		wk->rndMx[i] = GFUser_GetPublicRand( 8 ) - 4;
		wk->rndMy[i] = GFUser_GetPublicRand( 8 ) - 4;
	}
}

static BOOL MoveObjRand( DDEMOMAIN_WORK * wk, u32 id )
{
	if( wk->rndCnt == 8 ){
		DDEMOOBJ_SetPos( wk, id, wk->rndDx, wk->rndDy );
		return FALSE;
	}

	DDEMOOBJ_SetPos( wk, id, wk->rndDx+wk->rndMx[wk->rndCnt], wk->rndDy+wk->rndMy[wk->rndCnt] );
	wk->rndCnt++;
	return TRUE;
}
