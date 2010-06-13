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


//============================================================================================
//	定数定義
//============================================================================================
// シーン１データ
#define	DEF_1ST_START_WAIT			( 32 )																						// 開始ウェイト
#define	DEF_1ST_PM_LEFT_SPEED		( -16 )																						// ポケモン左移動速度
#define	DEF_1ST_PM_LEFT_COUNT		( (256+96) / GFL_STD_Abs(DEF_1ST_PM_LEFT_SPEED) )	// ポケモン左移動カウント	
#define	DEF_1ST_PM_LEFT_WAIT		( 8 )																							// ポケモン左移動後のウェイト
#define	DEF_1ST_TYPE_EFF_WAIT		( 8 )																							// タイプ別エフェクト後のウェイト
#define	DEF_1ST_PM_RIGHT_SPEED	( 16 )																						// ポケモン右移動速度
#define	DEF_1ST_PM_RIGHT_EPX		( 48 )																						// ポケモン右移動終了座標
#define	DEF_1ST_MM_RIGHT_EPX		( 21*8 )																					//「おめでとう」右移動終了座標
#define	DEF_1ST_IM_RIGHT_EPX		( 11*8 )																					// ポケモン情報左移動終了座標
#define	DEF_1ST_PM_RIGHT_WAIT		( 128 )																						// ポケモン右移動後のウェイト
#define	DEF_1ST_PM_OUT_SPEED		( 16 )																						// ポケモン画面アウト移動速度
#define	DEF_1ST_PM_OUT_EPX			( -48 )																						// ポケモン画面アウト終了座標
#define	DEF_1ST_MM_OUT_EPX			( -8 )																						//「おめでとう」アウト終了座標
#define	DEF_1ST_IM_OUT_EPX			( 256+8 )																					// ポケモン情報アウト終了座標
#define	DEF_1ST_LOOP_WAIT				( 32 )																						// シーン１ループ開始ウェイト

// シーン２データ
#define	DEF_2ND_START_WAIT				( 32 )																					// 開始ウェイト
#define	DEF_2ND_PLAYER_FALL_SPEED	( 8 )																						// プレイヤー落下速度
#define	DEF_2ND_PLAYER_FALL_COUNT	( (192+128+24+64)/DEF_2ND_PLAYER_FALL_SPEED )		// プレイヤー落下カウント
#define	DEF_2ND_WIN_OPEN_WAIT			( 32 )																					// ウィンドウオープン開始ウェイト
#define	DEF_2ND_WIN_PUT_WAIT			( 16 )																					// ウィンドウオープン後のウェイト
#define	DEF_2ND_INFO_PUT_WAIT			( 32 )																					// プレイ情報表示後のウェイト
#define	DEF_2ND_MES_PUT_WAIT			( 60*5 )																				//「おめでとう」表示後のウェイト
#define	DEF_2ND_POKEIN_WAIT_MAX		( 256 )																					// ポケモン表示開始ウェイト
#define	DEF_2ND_POKEIN_WAIT				( 128 )																					// 次のポケモン表示開始ウェイト
#define	DEF_2ND_POKEIN_SPEED			( -8 )																					// ポケモン画面イン速度
#define	DEF_2ND_POKEIN_COUNT			( 20 )																					// ポケモン画面イン移動カウント
#define	DEF_2ND_POKEOUT_WAIT			( 128 )																					// ポケモン画面アウト開始ウェイト
#define	DEF_2ND_POKEOUT_SPEED			( -8 )																					// ポケモン画面アウト速度
#define	DEF_2ND_POKEOUT_COUNT			( 44-DEF_2ND_POKEIN_COUNT )											// ポケモン画面アウト移動カウント
#define	DEF_2ND_FADE_DIV					( 188 )																					// 終了画面フェード分割数
#define	DEF_2ND_BGM_FADE					( 220 )																					// 終了BGMフェードフレーム数


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( DDEMOMAIN_WORK * wk );
static int MainSeq_Release( DDEMOMAIN_WORK * wk );
static int MainSeq_Wipe( DDEMOMAIN_WORK * wk );
static int MainSeq_Wait( DDEMOMAIN_WORK * wk );

static int MainSeq_1stInit( DDEMOMAIN_WORK * wk );
static int MainSeq_1stMain( DDEMOMAIN_WORK * wk );
static int MainSeq_1stExit( DDEMOMAIN_WORK * wk );
static int MainSeq_2ndInit( DDEMOMAIN_WORK * wk );
static int MainSeq_2ndMain( DDEMOMAIN_WORK * wk );
static int MainSeq_2ndExit( DDEMOMAIN_WORK * wk );

static int SetFadeIn( DDEMOMAIN_WORK * wk, int next );
static int SetFadeOut( DDEMOMAIN_WORK * wk, int next, int div );

static int SetWait( DDEMOMAIN_WORK * wk, int wait );

static void MakeRandMoveParam( DDEMOMAIN_WORK * wk );
static BOOL MoveObjRand( DDEMOMAIN_WORK * wk, u32 id );

static void RotationBgScene2( DDEMOMAIN_WORK * wk );

#ifdef	PM_DEBUG
static int Debug_CameraCheck( DDEMOMAIN_WORK * wk );
#endif	// PM_DEBUG


//============================================================================================
//	グローバル
//============================================================================================

// メインシーケンス
static const pDDEMOMAIN_FUNC MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_Wipe,
	MainSeq_Wait,

	MainSeq_1stInit,
	MainSeq_1stMain,
	MainSeq_1stExit,

	MainSeq_2ndInit,
	MainSeq_2ndMain,
	MainSeq_2ndExit,
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		メイン処理
 *
 * @param		wk		ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL DDEMOSEQ_MainSeq( DDEMOMAIN_WORK * wk )
{
	wk->mainSeq = MainSeq[wk->mainSeq]( wk );
	if( wk->mainSeq == MAINSEQ_END ){
		return FALSE;
	}

	if( wk->scene == 1 ){
		DDEMOOBJ_MainScene1( wk );
		DDEMOMAIN_MainDouble3D( wk );
	}else if( wk->scene == 2 ){
		DDEMOOBJ_MainScene2( wk );
		DDEMOMAIN_Main3D( wk );
		RotationBgScene2( wk );
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：初期化
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Init( DDEMOMAIN_WORK * wk )
{
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 輝度を最低にしておく
	GX_SetMasterBrightness( -16 );
	GXS_SetMasterBrightness( -16 );

	DDEMOMAIN_InitBg();
	DDEMOMAIN_InitMsg( wk );
	DDEMOMAIN_InitSound( wk );

	DDEMOMAIN_GetPokeMax( wk );

#ifdef	PM_DEBUG
	if( wk->debugMode == TRUE ){
		return MAINSEQ_2ND_INIT;
	}
#endif

	return MAINSEQ_1ST_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：解放
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Release( DDEMOMAIN_WORK * wk )
{
	DDEMOMAIN_ExitSound( wk );
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ワイプ処理
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Wipe( DDEMOMAIN_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return wk->nextSeq;
	}
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ウェイト処理
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Wait( DDEMOMAIN_WORK * wk )
{
	if( wk->wait == 0 ){
		return wk->nextSeq;
	}
	wk->wait--;
	return MAINSEQ_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：シーン１初期化
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_1stInit( DDEMOMAIN_WORK * wk )
{
	DDEMOMAIN_InitVram( 0 );
	DDEMOMAIN_InitBgMode();

	DDEMOMAIN_Init3D( wk );
	DDEMOMAIN_InitParticle();
	DDEMOMAIN_InitDouble3D();

	DDEMOOBJ_Init( wk, 0 );
	DDEMOOBJ_InitScene1( wk );

	DDEMOMAIN_SetBlendAlpha();

	DDEMOMAIN_InitScene1VBlank( wk );

	wk->scene = 1;

	return MAINSEQ_1ST_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：シーン１解放
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_1stExit( DDEMOMAIN_WORK * wk )
{
	DDEMOMAIN_ExitVBlank( wk );

	G2_BlendNone();
	G2S_BlendNone();

	DDEMOOBJ_ExitScene1( wk );
	DDEMOOBJ_Exit( wk );

	DDEMOMAIN_ExitDouble3D();
	DDEMOMAIN_ExitParticle();
	DDEMOMAIN_Exit3D( wk );

	wk->scene = 0;

	return MAINSEQ_2ND_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：シーン１メイン
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_1stMain( DDEMOMAIN_WORK * wk )
{
	switch( wk->subSeq ){
	case 0:		// 初期ウェイト
		wk->subSeq++;
		return SetWait( wk, DEF_1ST_START_WAIT );

	case 1:
		PMSND_PlaySE( SEQ_SE_DDEMO_02A );
		wk->subSeq++;
		break;

	case 2:
		wk->wait++;
		if( wk->wait == 48 ){
			PMSND_PlaySE( SEQ_SE_DDEMO_02B );
			wk->wait = 0;
			wk->subSeq++;
		}
		break;

	case 3:		// ポケモンセット
    if( PMSND_CheckPlayingSEIdx(SEQ_SE_DDEMO_02B) == FALSE ){
			PMSND_PlaySE( SEQ_SE_DDEMO_01 );
			DDEMOMAIN_GetPokeData( wk );
			DDEMOMAIN_LoadPokeVoice( wk );
			DDEMOMAIN_CreateTypeParticle( wk );
			DDEMOMAIN_CreateNameParticle( wk );
			DDEMOOBJ_AddPoke( wk );
			DDEMOOBJ_PrintPokeInfo( wk );
			DDEMOOBJ_MoveFontOamPos( wk );
			DDEMOOBJ_SetTypeWindow( wk );
			wk->subSeq++;
			return SetFadeIn( wk, MAINSEQ_1ST_MAIN );
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
		DDEMOMAIN_SetTypeParticle( wk, 0 );
		DDEMOMAIN_SetTypeParticle( wk, 1 );
		wk->subSeq++;
		return SetWait( wk, DEF_1ST_TYPE_EFF_WAIT );

	case 6:
		DDEMOMAIN_SetNameParticle( wk, 1 );
		wk->subSeq++;

	case 7:		// メッセージ移動
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

	case 8:
		DDEMOMAIN_SetNameParticle( wk, 0 );
		wk->subSeq++;

	case 9:		// 情報移動
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

	case 10:		// ポケモン移動
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

	case 11:		// 鳴き声＆キラキラ
		if( MoveObjRand( wk, DDEMOOBJ_ID_POKE ) == FALSE ){
			PMVOICE_PlayOnly( wk->voicePlayer );
			DDEMOOBJ_SetVanish( wk, DDEMOOBJ_ID_EFF, TRUE );
			DDEMOOBJ_SetAutoAnm( wk, DDEMOOBJ_ID_EFF, 6 );
			wk->subSeq++;
			return SetWait( wk, DEF_1ST_PM_RIGHT_WAIT );
		}
		break;

	case 12:		// 鳴き声＆キラキラ待ち
		if( PMVOICE_CheckPlay( wk->voicePlayer ) == FALSE &&
				DDEMOOBJ_CheckAnm( wk, DDEMOOBJ_ID_EFF ) == FALSE ){
			wk->subSeq++;
		}
		break;

	case 13:		// ポケモンアウト	
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

	case 14:		// 情報アウト
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

	case 15:		// メッセージアウト
		{
			s16	x, y;
			DDEMOOBJ_GetPos( wk, DDEMOOBJ_ID_MES, &x, &y );
			if( x != DEF_1ST_MM_OUT_EPX ){
				DDEMOOBJ_Move( wk, DDEMOOBJ_ID_MES, -DEF_1ST_PM_OUT_SPEED, 0 );
				DDEMOOBJ_MoveFontOamPos( wk );
			}else{
				wk->subSeq++;
				return SetFadeOut( wk, MAINSEQ_1ST_MAIN, WIPE_DEF_DIV );
			}
		}
		break;

	case 16:		// 終了チェック
		DDEMOMAIN_DeleteNameParticle( wk );
		DDEMOMAIN_DeleteTypeParticle( wk );
		wk->pokePos++;
		if( wk->pokePos == wk->pokeMax ){
			wk->pokePos = 0;
			wk->subSeq = 0;
			return MAINSEQ_1ST_EXIT;
		}else{
			wk->subSeq = 3;
		}
		break;
	}

	return MAINSEQ_1ST_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：シーン２初期化
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_2ndInit( DDEMOMAIN_WORK * wk )
{
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );		// サブ画面をメインに

	DDEMOMAIN_InitVram( 1 );
	DDEMOMAIN_InitBgMode();

	DDEMOMAIN_InitScene2BgFrame();
	DDEMOMAIN_LoadScene2BgGraphic();

	DDEMOMAIN_Init3D( wk );
	DDEMOMAIN_InitMcss( wk );

	DDEMOOBJ_Init( wk, 1 );
	DDEMOOBJ_InitScene2( wk );

	DDEMOMAIN_InitScene2VBlank( wk );

	wk->scene = 2;

	return SetFadeIn( wk, MAINSEQ_2ND_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：シーン２解放
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_2ndExit( DDEMOMAIN_WORK * wk )
{
	DDEMOMAIN_ExitVBlank( wk );

	DDEMOOBJ_ExitScene2( wk );
	DDEMOOBJ_Exit( wk );

	DDEMOMAIN_ExitScene2BgFrame();

	DDEMOMAIN_ExitMcss( wk );
	DDEMOMAIN_Exit3D( wk );

	wk->scene = 0;

	return MAINSEQ_RELEASE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：シーン２メイン
 *
 * @param		wk		ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_2ndMain( DDEMOMAIN_WORK * wk )
{
#ifdef	PM_DEBUG
	if( wk->debugMode == TRUE ){
		return Debug_CameraCheck( wk );
	}
#endif

	// 終了チェック
	if( wk->endFlag == TRUE ){
		if( WIPE_SYS_EndCheck() == TRUE && PMSND_CheckFadeOnBGM() == FALSE ){
			PMSND_StopBGM();
			return MAINSEQ_2ND_EXIT;
		}
	}

	switch( wk->subSeq ){
	case 0:		// 初期化
		wk->subSeq++;
		return SetWait( wk, DEF_2ND_START_WAIT );

	case 1:
		PMSND_PlaySE( SEQ_SE_DDEMO_03 );
		wk->subSeq++;
		break;

	case 2:		// 主人公落下
		if( wk->wait != DEF_2ND_PLAYER_FALL_COUNT ){
			DDEMOOBJ_Move( wk, DDEMOOBJ_ID_PLAYER_M, 0, DEF_2ND_PLAYER_FALL_SPEED );
			DDEMOOBJ_Move( wk, DDEMOOBJ_ID_PLAYER_S, 0, DEF_2ND_PLAYER_FALL_SPEED );
			wk->wait++;
		}else{
			PMSND_PlaySE( SEQ_SE_ROTATION_B );
			wk->subSeq++;
			return SetWait( wk, DEF_2ND_WIN_OPEN_WAIT );
		}
		break;

	case 3:		// ウィンドウオープン
		DDEMOOBJ_SetVanish( wk, DDEMOOBJ_ID_2ND_MES, TRUE );
		DDEMOOBJ_SetVanish( wk, DDEMOOBJ_ID_2ND_INFO, TRUE );
		DDEMOOBJ_SetAutoAnm( wk, DDEMOOBJ_ID_2ND_MES, 2 );
		DDEMOOBJ_SetAutoAnm( wk, DDEMOOBJ_ID_2ND_INFO, 2 );
		wk->subSeq++;
		break;

	case 4:		// ウィンドウ表示待ち
		if( DDEMOOBJ_CheckAnm( wk, DDEMOOBJ_ID_2ND_MES ) == FALSE ){
			PMSND_PlaySE( SEQ_SE_DDEMO_04 );
			wk->subSeq++;
			return SetWait( wk, DEF_2ND_WIN_PUT_WAIT );
		}
		break;

	case 5:		// プレイヤー情報表示
		BOX2OBJ_FontOamVanish( wk, DDEMOOBJ_FOAM_PLAYER, TRUE );
		wk->subSeq++;
		return SetWait( wk, DEF_2ND_INFO_PUT_WAIT );

	case 6:		// メッセージ表示
		BOX2OBJ_FontOamVanish( wk, DDEMOOBJ_FOAM_MES2, TRUE );
		wk->subSeq++;
		break;

	case 7:		// ウェイト
		if( wk->wait == DEF_2ND_MES_PUT_WAIT ){
			wk->wait = 0;
			wk->subSeq++;
		}else{
			wk->wait++;
		}
		break;

	// 終了待ち
	case 8:
		{
			BOOL	flg = FALSE;
			if( wk->pokePos == 0 ){
				if( wk->wait == DEF_2ND_POKEIN_WAIT_MAX ){
					flg = TRUE;
				}
			}else{
				if( wk->wait == DEF_2ND_POKEIN_WAIT ){
					flg = TRUE;
				}
			}
			if( flg == TRUE ){
				DDEMOMAIN_AddMcss( wk );
				wk->wait = 0;
				wk->subSeq++;
				break;
			}
			wk->wait++;
		}
		break;

	case 9:
		if( wk->wait != DEF_2ND_POKEIN_COUNT ){
			DDEMOMAIN_MoveMcss( wk, DEF_2ND_POKEIN_SPEED );
			wk->wait++;
		}else{
			MCSS_ResetAnmStopFlag( wk->mcssWork );
			wk->wait = 0;
			wk->subSeq++;
		}
		break;

	case 10:
		if( wk->wait != DEF_2ND_POKEOUT_WAIT ){
			wk->wait++;
		}else{
			DDEMOMAIN_SetMcssCallBack( wk );
			wk->wait = 0;
			wk->subSeq++;
		}
		break;

	case 11:
		if( wk->mcssAnmEndFlg == FALSE ){
			break;
		}
		if( wk->wait != DEF_2ND_POKEOUT_COUNT ){
			DDEMOMAIN_MoveMcss( wk, DEF_2ND_POKEOUT_SPEED );
			wk->wait++;
		}else{
			DDEMOMAIN_DelMcss( wk );
			wk->wait = 0;
			wk->pokePos++;
			if( wk->pokePos == wk->pokeMax ){
				wk->pokePos = 0;
			}
			wk->subSeq = 8;
		}
		break;
	}

	if( wk->subSeq >= 6 ){
		DDEMOOBJ_SetRandomFlash( wk );
	}
	if( wk->subSeq >= 8 && wk->endFlag == FALSE ){
		if( ( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ) ||
				GFL_UI_TP_GetTrg() == TRUE ){
			wk->endFlag = TRUE;
		  PMSND_FadeOutBGM( DEF_2ND_BGM_FADE );
			SetFadeOut( wk, MAINSEQ_2ND_EXIT, DEF_2ND_FADE_DIV );
		}
	}

	return MAINSEQ_2ND_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードイン設定
 *
 * @param		wk		ワーク
 * @param		next	フェード後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetFadeIn( DDEMOMAIN_WORK * wk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_DENDOU_DEMO );
	wk->nextSeq = next;
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウト設定
 *
 * @param		wk		ワーク
 * @param		next	フェード後のシーケンス
 * @param		div		フェード分割数
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetFadeOut( DDEMOMAIN_WORK * wk, int next, int div )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, div, WIPE_DEF_SYNC, HEAPID_DENDOU_DEMO );
	wk->nextSeq = next;
	return MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ウェイト設定
 *
 * @param		wk		ワーク
 * @param		wait	ウェイト
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetWait( DDEMOMAIN_WORK * wk, int wait )
{
	wk->wait = wait;
	wk->nextSeq = wk->mainSeq;
	return MAINSEQ_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タイプ別ウィンドウのランダム動作設定
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MakeRandMoveParam( DDEMOMAIN_WORK * wk )
{
	u32	i;

	wk->rndCnt = 0;

	for( i=0; i<8; i++ ){
		wk->rndMx[i] = GFUser_GetPublicRand( 8 ) - 4;
		wk->rndMy[i] = GFUser_GetPublicRand( 8 ) - 4;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タイプ別ウィンドウのランダム動作
 *
 * @param		wk		ワーク
 * @param		id		OBJ ID
 *
 * @retval	"TRUE = 動作中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーン２のボール面回転動作
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void RotationBgScene2( DDEMOMAIN_WORK * wk )
{
	GFL_BG_SetRadianReq( GFL_BG_FRAME3_M, GFL_BG_RADION_INC, 1 );
	GFL_BG_SetRadianReq( GFL_BG_FRAME3_S, GFL_BG_RADION_INC, 1 );
}




//============================================================================================
#ifdef	PM_DEBUG
//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
#include "system/mcss_tool.h"
#include "msg/msg_dendou_demo.h"

static void Debug_AddMcss( DDEMOMAIN_WORK * wk )
{
	MCSS_ADD_WORK   add;
	POKEMON_PARAM * pp;
	VecFx32 scale = { FX_F32_TO_FX32(16), FX_F32_TO_FX32(16), FX32_ONE };

	while( 1 ){
		pp = PP_Create( wk->debugMons+1, 50, 0, HEAPID_DENDOU_DEMO_L );
   if( PP_GetSex( pp ) == wk->debugNowSex ){
		 break;
		}
		GFL_HEAP_FreeMemory( pp );
	}
	PP_ChangeFormNo( pp, wk->debugForm );

	MCSS_TOOL_MakeMAWPP( pp, &add, MCSS_DIR_FRONT );

	wk->mcssWork = MCSS_Add( wk->mcss, FX32_CONST(128+48), FX32_CONST(-96+48), 0, &add );
	MCSS_SetScale( wk->mcssWork, &scale );
	MCSS_SetAnmStopFlag( wk->mcssWork );

	GFL_HEAP_FreeMemory( pp );
}

static void Debug_PokeDataGet( DDEMOMAIN_WORK * wk )
{
	POKEMON_PERSONAL_DATA * ppd = POKE_PERSONAL_OpenHandle( wk->debugMons+1, 0, HEAPID_DENDOU_DEMO_L );

	wk->debugFormMax = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_form_max ) - 1;
	wk->debugSex = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );

	// 性別なし
	if( wk->debugSex == POKEPER_SEX_UNKNOWN ){
		wk->debugNowSex = PTL_SEX_UNKNOWN;
	// ♂のみ
	}else if( wk->debugSex == POKEPER_SEX_MALE ){
		wk->debugNowSex = PTL_SEX_MALE;
	// ♀のみ
	}else if( wk->debugSex == POKEPER_SEX_FEMALE ){
		wk->debugNowSex = PTL_SEX_FEMALE;
	}else{
		wk->debugNowSex = PTL_SEX_MALE;
	}

	POKE_PERSONAL_CloseHandle( ppd );
}

static void Debug_NextPokeGet( DDEMOMAIN_WORK * wk )
{
	// フォルムチェック
	if( wk->debugForm >= wk->debugFormMax ){
		wk->debugForm = 0;
	}else{
		wk->debugForm++;
		return;
	}

	// 性別チェック
	if( wk->debugNowSex == PTL_SEX_MALE ){
		if( wk->debugSex != POKEPER_SEX_MALE ){
			wk->debugNowSex = PTL_SEX_FEMALE;
			return;
		}
	}

	wk->debugMons = wk->debugNext;
	wk->debugNext++;
	if( wk->debugNext >= MONSNO_END ){
		wk->debugNext -= MONSNO_END;
	}
	Debug_PokeDataGet( wk );
}

static void Debug_InfoPut( DDEMOMAIN_WORK * wk )
{
	STRBUF * str;
	u32	x;

	GFL_BMP_Clear( wk->fobj[DDEMOOBJ_FOAM_PLAYER].bmp, 0 );

	str = GFL_MSG_CreateString( wk->mman, DDEMO_DEBUG_01 );
	WORDSET_RegisterNumber( wk->wset, 0, wk->debugMons+1, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	WORDSET_RegisterNumber( wk->wset, 1, wk->debugForm, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINTSYS_PrintColor( wk->fobj[DDEMOOBJ_FOAM_PLAYER].bmp, 8, 0, wk->exp, wk->font, PRINTSYS_LSB_Make(15,2,0) );
	GFL_STR_DeleteBuffer( str );

	if( wk->debugNowSex == PTL_SEX_MALE ){
		str = GFL_MSG_CreateString( wk->mman, DDEMO_DEBUG_02 );
		PRINTSYS_PrintColor( wk->fobj[DDEMOOBJ_FOAM_PLAYER].bmp, 136, 0, str, wk->font, PRINTSYS_LSB_Make(15,2,0) );
		GFL_STR_DeleteBuffer( str );
	}else if( wk->debugNowSex == PTL_SEX_FEMALE ){
		str = GFL_MSG_CreateString( wk->mman, DDEMO_DEBUG_03 );
		PRINTSYS_PrintColor( wk->fobj[DDEMOOBJ_FOAM_PLAYER].bmp, 136, 0, str, wk->font, PRINTSYS_LSB_Make(15,2,0) );
		GFL_STR_DeleteBuffer( str );
	}

	str = GFL_MSG_CreateString( wk->mman, DDEMO_DEBUG_04 );
	WORDSET_RegisterNumber( wk->wset, 0, wk->debugNext+1, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINTSYS_PrintColor( wk->fobj[DDEMOOBJ_FOAM_PLAYER].bmp, 152, 0, wk->exp, wk->font, PRINTSYS_LSB_Make(15,2,0) );
	GFL_STR_DeleteBuffer( str );

	BmpOam_ActorBmpTrans( wk->fobj[DDEMOOBJ_FOAM_PLAYER].oam );
}

static int Debug_CameraCheck( DDEMOMAIN_WORK * wk )
{
	// 終了チェック
	if( wk->endFlag == TRUE ){
		if( WIPE_SYS_EndCheck() == TRUE && PMSND_CheckFadeOnBGM() == FALSE ){
			PMSND_StopBGM();
			return MAINSEQ_2ND_EXIT;
		}
	}

	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
		wk->debugNext += 10;
		if( wk->debugNext >= MONSNO_END ){
			wk->debugNext -= MONSNO_END;
		}
		Debug_InfoPut( wk );
	}else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
		wk->debugNext -= 10;
		if( wk->debugNext < 0 ){
			wk->debugNext += MONSNO_END;
		}
		Debug_InfoPut( wk );
	}else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
		wk->debugNext += 1;
		if( wk->debugNext >= MONSNO_END ){
			wk->debugNext -= MONSNO_END;
		}
		Debug_InfoPut( wk );
	}else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
		wk->debugNext -= 1;
		if( wk->debugNext == 0 ){
			wk->debugNext += MONSNO_END;
		}
		Debug_InfoPut( wk );
	}

	switch( wk->subSeq ){
	case 0:		// 初期化
		wk->subSeq++;
		return SetWait( wk, DEF_2ND_START_WAIT );

	case 1:
		PMSND_PlaySE( SEQ_SE_DDEMO_03 );
		wk->subSeq++;
		break;

	case 2:		// 主人公落下
		if( wk->wait != DEF_2ND_PLAYER_FALL_COUNT ){
			DDEMOOBJ_Move( wk, DDEMOOBJ_ID_PLAYER_M, 0, DEF_2ND_PLAYER_FALL_SPEED );
			DDEMOOBJ_Move( wk, DDEMOOBJ_ID_PLAYER_S, 0, DEF_2ND_PLAYER_FALL_SPEED );
			wk->wait++;
		}else{
			PMSND_PlaySE( SEQ_SE_ROTATION_B );
			wk->subSeq++;
			return SetWait( wk, DEF_2ND_WIN_OPEN_WAIT );
		}
		break;

	case 3:		// ウィンドウオープン
		DDEMOOBJ_SetVanish( wk, DDEMOOBJ_ID_2ND_MES, TRUE );
		DDEMOOBJ_SetVanish( wk, DDEMOOBJ_ID_2ND_INFO, TRUE );
		DDEMOOBJ_SetAutoAnm( wk, DDEMOOBJ_ID_2ND_MES, 2 );
		DDEMOOBJ_SetAutoAnm( wk, DDEMOOBJ_ID_2ND_INFO, 2 );
		wk->subSeq++;
		break;

	case 4:		// ウィンドウ表示待ち
		if( DDEMOOBJ_CheckAnm( wk, DDEMOOBJ_ID_2ND_MES ) == FALSE ){
			PMSND_PlaySE( SEQ_SE_DDEMO_04 );
			wk->subSeq++;
			return SetWait( wk, DEF_2ND_WIN_PUT_WAIT );
		}
		break;

	case 5:		// プレイヤー情報表示
		BOX2OBJ_FontOamVanish( wk, DDEMOOBJ_FOAM_PLAYER, TRUE );
		wk->subSeq++;
		return SetWait( wk, DEF_2ND_INFO_PUT_WAIT );

	case 6:		// メッセージ表示
		BOX2OBJ_FontOamVanish( wk, DDEMOOBJ_FOAM_MES2, TRUE );
		wk->subSeq++;
		break;

	case 7:		// ウェイト
		Debug_PokeDataGet( wk );
		wk->wait = 0;
		wk->subSeq++;
		break;

	// 終了待ち
	case 8:
		Debug_InfoPut( wk );
		Debug_AddMcss( wk );
		wk->wait = 0;
		wk->subSeq++;
		break;

	case 9:
		if( wk->wait != DEF_2ND_POKEIN_COUNT ){
			DDEMOMAIN_MoveMcss( wk, DEF_2ND_POKEIN_SPEED );
			wk->wait++;
		}else{
			MCSS_ResetAnmStopFlag( wk->mcssWork );
			wk->wait = 0;
			wk->subSeq++;
		}
		break;

	case 10:
		if( wk->wait != DEF_2ND_POKEOUT_WAIT ){
			wk->wait++;
		}else{
			wk->wait = 0;
			wk->subSeq++;
		}
		break;

	case 11:
		if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A ){
			MCSS_SetAnmStopFlag( wk->mcssWork );
			wk->subSeq++;
		}
		break;

	case 12:
		if( wk->wait != DEF_2ND_POKEOUT_COUNT ){
			DDEMOMAIN_MoveMcss( wk, DEF_2ND_POKEOUT_SPEED );
			wk->wait++;
		}else{
			DDEMOMAIN_DelMcss( wk );
			Debug_NextPokeGet( wk );
			wk->wait = 0;
			wk->subSeq = 8;
		}
		break;
	}

	if( wk->subSeq >= 8 && wk->endFlag == FALSE ){
		if( ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ) ||
				GFL_UI_TP_GetTrg() == TRUE ){
			wk->endFlag = TRUE;
		  PMSND_FadeOutBGM( DEF_2ND_BGM_FADE );
			SetFadeOut( wk, MAINSEQ_2ND_EXIT, DEF_2ND_FADE_DIV );
		}
	}

	return MAINSEQ_2ND_MAIN;
}

#endif
