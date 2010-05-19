//============================================================================================
/**
 * @file		cdemo_comm.c
 * @brief		コマンドデモ画面 コマンド処理
 * @author	Hiroyuki Nakamura
 * @date		09.05.07
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "sound/pm_sndsys.h"

#include "cdemo_main.h"
#include "cdemo_seq.h"
#include "cdemo_comm.h"

#include "op_demo.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	NCGR_START_IDX	( 0 )		// キャラ開始位置
#define	NSCR_START_IDX	( 0 )		// スクリーン開始位置
#define	NCLR_START_IDX	( 0 )		// パレット開始位置

typedef int (*pCommandFunc)(CDEMO_WORK*,const int*);	// コマンド関数


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int Comm_Wait( CDEMO_WORK * wk, const int * comm );
static int Comm_BlackIn( CDEMO_WORK * wk, const int * comm );
static int Comm_BlackOut( CDEMO_WORK * wk, const int * comm );
static int Comm_WhiteIn( CDEMO_WORK * wk, const int * comm );
static int Comm_WhiteOut( CDEMO_WORK * wk, const int * comm );
static int Comm_FadeMain( CDEMO_WORK * wk, const int * comm );
static int Comm_BgLoad( CDEMO_WORK * wk, const int * comm );
static int Comm_BgVanish( CDEMO_WORK * wk, const int * comm );
static int Comm_BgPriChg( CDEMO_WORK * wk, const int * comm );
static int Comm_AlphaInit( CDEMO_WORK * wk, const int * comm );
static int Comm_AlphaStart( CDEMO_WORK * wk, const int * comm );
static int Comm_AlphaNone( CDEMO_WORK * wk, const int * comm );
static int Comm_BGMSave( CDEMO_WORK * wk, const int * comm );
static int Comm_BGMStop( CDEMO_WORK * wk, const int * comm );
static int Comm_BGMPlay( CDEMO_WORK * wk, const int * comm );
static int Comm_ObjVanish( CDEMO_WORK * wk, const int * comm );
static int Comm_ObjPut( CDEMO_WORK * wk, const int * comm );
static int Comm_ObjPriChange( CDEMO_WORK * wk, const int * comm );
static int Comm_ObjBgPriChange( CDEMO_WORK * wk, const int * comm );
static int Comm_BgScreenAnime( CDEMO_WORK * wk, const int * comm );

static int Comm_DebugPrint( CDEMO_WORK * wk, const int * comm );


//============================================================================================
//	グローバル
//============================================================================================

// コマンド関数テーブル
static const pCommandFunc CommFunc[] = {
	Comm_Wait,
	Comm_BlackIn,
	Comm_BlackOut,
	Comm_WhiteIn,
	Comm_WhiteOut,
	Comm_FadeMain,

	Comm_BgLoad,
	Comm_BgVanish,
	Comm_BgPriChg,
	Comm_AlphaInit,
	Comm_AlphaStart,
	Comm_AlphaNone,

	Comm_BGMSave,
	Comm_BGMStop,
	Comm_BGMPlay,

	Comm_ObjVanish,
	Comm_ObjPut,
	Comm_ObjBgPriChange,

	Comm_BgScreenAnime,

	Comm_DebugPrint,
};

// コマンドサイズ
static const int CommandSize[] = {
	2,		// 00: ウェイト							COMM_WAIT, wait,

	3,		// 01: ブラックイン					COMM_BLACK_IN, div, sync
	3,		// 02: ブラックアウト				COMM_BLACK_OUT, div, sync
	3,		// 03: ホワイトイン					COMM_WHITE_IN, div, sync
	3,		// 04: ホワイトアウト				COMM_WHITE_OUT, div, sync
	1,		// 05: フェードメイン				COMM_FADE_MAIN,

	3,		// 06: BG読み込み									COMM_BG_LOAD, frm, graphic,
	3,		// 07: BG表示切り替え							COMM_BG_VANISH, frm, flg,
	3,		// 08: BGプライオリティ切り替え		COMM_BG_PRI_CHG, frm, pri,
	4,		// 09: アルファブレンド設定				COMM_ALPHA_INIT, plane, frm, init,
	3,		// 10: アルファブレンド開始				COMM_ALPHA_START, end, cnt,
	1,		// 11: アルファ無効								COMM_ALPHA_NONE,

	1,		// 12: 再生中のBGMを記憶			COMM_BGM_SAVE,
	1,		// 13: 再生中のBGMを停止			COMM_BGM_STOP,
	2,		// 14: BGM再生								COMM_BGM_PLAY, no

	2,		// 15: OBJ表示切り替え							COMM_OBJ_VANISH, flg,
	3,		// 16: OBJ座標設定									COMM_OBJ_PUT, x, y,
	2,		// 17: OBJとBGのプライオリティ設定	COMM_OBJ_BG_PRI_CHANGE, pri,

  2,		// 18: BGスクリーンアニメ			CDEMOCOMM_BG_FRM_ANIME, wait,

	2,		// 19: デバッグプリント				COMM_DEBUG_PRINT, id

	1,		// 終了								COMM_END,
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		メイン処理
 *
 * @param		wk		ワーク
 *
 * @return	処理内容
 */
//--------------------------------------------------------------------------------------------
int CDEMOCOMM_Main( CDEMO_WORK * wk )
{
	int	ret = CommFunc[ wk->commPos[0] ]( wk, wk->commPos );

	if( ret != CDEMOCOMM_RET_TRUE ){
		wk->commPos = &wk->commPos[ CommandSize[wk->commPos[0]] ];
	}

	return ret;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：ウェイト
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_NEXT = 次のコマンドへ"
 * @retval	"COMM_RET_TRUE = コマンド実行中"
 *
 * @li	comm[0] = COMM_WAIT
 * @li	comm[1] = wait
 */
//--------------------------------------------------------------------------------------------
static int Comm_Wait( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_WAIT, "COMM_WAIT: comm[0] = %d\n", comm[0] );

	if( wk->cnt == comm[1] ){
		wk->cnt = 0;
		return CDEMOCOMM_RET_NEXT;
	}
	wk->cnt++;
	return CDEMOCOMM_RET_TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：ブラックイン
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_BLACK_IN
 * @li	comm[1] = div
 * @li	comm[2] = sync
 */
//--------------------------------------------------------------------------------------------
static int Comm_BlackIn( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BLACK_IN, "COMM_BLACK_IN: comm[0] = %d\n", comm[0] );

	CDEMOMAIN_FadeInSet( wk, comm[1], comm[2], CDEMOSEQ_MAIN_MAIN );
	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：ブラックアウト
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_BLACK_OUT
 * @li	comm[1] = div
 * @li	comm[2] = sync
 */
//--------------------------------------------------------------------------------------------
static int Comm_BlackOut( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BLACK_OUT, "COMM_BLACK_OUT: comm[0] = %d\n", comm[0] );

	CDEMOMAIN_FadeOutSet( wk, comm[1], comm[2], CDEMOSEQ_MAIN_MAIN );
	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：ホワイトイン
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_WHITE_IN
 * @li	comm[1] = div
 * @li	comm[2] = sync
 */
//--------------------------------------------------------------------------------------------
static int Comm_WhiteIn( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_WHITE_IN, "COMM_WHITE_IN: comm[0] = %d\n", comm[0] );

	CDEMOMAIN_WhiteInSet( wk, comm[1], comm[2], CDEMOSEQ_MAIN_MAIN );
	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：ホワイトアウト
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_WHITE_OUT
 * @li	comm[1] = div
 * @li	comm[2] = sync
 */
//--------------------------------------------------------------------------------------------
static int Comm_WhiteOut( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_WHITE_OUT, "COMM_WHITE_OUT: comm[0] = %d\n", comm[0] );

	CDEMOMAIN_WhiteOutSet( wk, comm[1], comm[2], CDEMOSEQ_MAIN_MAIN );
	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：フェードメイン
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_NEXT = 次のコマンドへ"
 *
 * @li	comm[0] = COMM_FADE_MAIN
 */
//--------------------------------------------------------------------------------------------
static int Comm_FadeMain( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_FADE_MAIN, "COMM_FADE_MAIN: comm[0] = %d\n", comm[0] );

	wk->main_seq = CDEMOSEQ_MAIN_FADE;
	wk->next_seq = CDEMOSEQ_MAIN_MAIN;
	return CDEMOCOMM_RET_NEXT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：BG読み込み
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_BG_LOAD
 * @li	comm[1] = frm
 * @li	comm[2] = graphic
 */
//--------------------------------------------------------------------------------------------
static int Comm_BgLoad( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BG_LOAD, "COMM_BG_LOAD: comm[0] = %d\n", comm[0] );

/*
	ArcUtil_HDL_BgCharSet(
		awk->ah, NCGR_START_IDX + comm[2],
		awk->bgl, comm[1], 0, 0, TRUE, HEAPID_LEGEND_DEMO );

	LoadPaletteRequest( awk, comm[1], comm[2] );

	ArcUtil_HDL_ScrnSet(
		awk->ah, NSCR_START_IDX + comm[2],
		awk->bgl, comm[1], 0, 0, TRUE, HEAPID_LEGEND_DEMO );
*/
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		wk->gra_ah, NCGR_START_IDX + comm[2], comm[1], 0, 0, TRUE, HEAPID_COMMAND_DEMO );
	if( comm[2] == 0 ){
		GFL_ARCHDL_UTIL_TransVramScreen(
			wk->gra_ah, NSCR_START_IDX + comm[2], comm[1], 0, 0, FALSE, HEAPID_COMMAND_DEMO );
	}
//	GFL_ARCHDL_UTIL_TransVramPalette(
//		wk->gra_ah, NCLR_START_IDX + comm[2], PALTYPE_MAIN_BG_EX, 0, 0, HEAPID_COMMAND_DEMO );
	CDEMOMAIN_LoadPaletteRequest( wk, comm[1], NCLR_START_IDX + comm[2] );


	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：BG表示切り替え
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_BG_VANISH
 * @li	comm[1] = frm
 * @li	comm[2] = flg
 */
//--------------------------------------------------------------------------------------------
static int Comm_BgVanish( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BG_VANISH, "COMM_BG_VANISH: comm[0] = %d\n", comm[0] );

	GFL_BG_SetVisible( comm[1], comm[2] );
	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：BGプライオリティ切り替え
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_BG_PRI_CHG
 * @li	comm[1] = frm
 * @li	comm[2] = pri
 */
//--------------------------------------------------------------------------------------------
static int Comm_BgPriChg( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BG_PRI_CHG, "COMM_BG_PRI_CHG: comm[0] = %d\n", comm[0] );

	GFL_BG_SetPriority( comm[1], comm[2] );
	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：アルファブレンド設定
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_ALPHA_INIT
 * @li	comm[1] = plane
 * @li	comm[2] = frm
 * @li	comm[3] = init
 */
//--------------------------------------------------------------------------------------------
static int Comm_AlphaInit( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_ALPHA_INIT, "COMM_ALPHA_INIT: comm[0] = %d\n", comm[0] );

	if( comm[1] == CDEMO_PRM_BLEND_PLANE_1 ){
		wk->alpha_plane1 = comm[2];
		wk->alpha_ev1    = comm[3];
	}else{
		wk->alpha_plane2 = comm[2];
		wk->alpha_ev2    = comm[3];
	}

	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：アルファブレンド開始
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_NEXT = 次のコマンドへ"
 *
 * @li	comm[0] = COMM_ALPHA_START
 * @li	comm[1] = end
 * @li	comm[2] = cnt
 */
//--------------------------------------------------------------------------------------------
static int Comm_AlphaStart( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_ALPHA_START, "COMM_ALPHA_START: comm[0] = %d\n", comm[0] );

	wk->alpha_end_ev = comm[1];
	wk->alpha_mv_frm = comm[2];

	wk->main_seq = CDEMOSEQ_MAIN_ALPHA_BLEND;
	wk->next_seq = CDEMOSEQ_MAIN_MAIN;

	return CDEMOCOMM_RET_NEXT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：アルファ無効
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_ALPHA_NONE
 */
//--------------------------------------------------------------------------------------------
static int Comm_AlphaNone( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_ALPHA_NONE, "COMM_ALPHA_NONE: comm[0] = %d\n", comm[0] );

	G2_BlendNone();

	wk->alpha_plane1 = 0;
	wk->alpha_ev1    = 0;
	wk->alpha_plane2 = 0;
	wk->alpha_ev2    = 0;

	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：BGM記憶
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_BGM_SAVE
 */
//--------------------------------------------------------------------------------------------
static int Comm_BGMSave( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BGM_SAVE, "COMM_BGM_SAVE: comm[0] = %d\n", comm[0] );

//	awk->bgm = Snd_NowBgmNoGet();

	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：BGM停止
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_BGM_STOP
 */
//--------------------------------------------------------------------------------------------
static int Comm_BGMStop( CDEMO_WORK * wk, const int * comm )
{
//	u16	bgm;

	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BGM_STOP, "COMM_BGM_STOP: comm[0] = %d\n", comm[0] );
/*
	bgm = Snd_NowBgmNoGet();
	Snd_BgmStop( bgm, 0 );
*/
	PMSND_StopBGM();

	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：BGM再生
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_BGM_PLAY
 * @li	comm[1] = no
 */
//--------------------------------------------------------------------------------------------
static int Comm_BGMPlay( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BGM_PLAY, "COMM_BGM_PLAY: comm[0] = %d\n", comm[0] );
/*
	if( comm[1] == PRM_BGM_PLAY_SAVE ){
		Snd_BgmPlay( awk->bgm );
	}else{
		Snd_BgmPlay( comm[1] );
	}
*/
//	PMSND_PlayBGM( comm[1] );
	PMSND_PlayBGM_WideChannel( comm[1] );

	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：OBJ表示切り替え
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_OBJ_VANISH
 * @li	comm[1] = flg
 */
//--------------------------------------------------------------------------------------------
static int Comm_ObjVanish( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_OBJ_VANISH, "COMM_OBJ_VANISH: comm[0] = %d\n", comm[0] );

//	CATS_ObjectEnableCap( awk->cap[PRM_OBJ_ARCEUS], comm[1] );

	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：OBJ座標設定
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_OBJ_PUT
 * @li	comm[1] = x
 * @li	comm[2] = y
 */
//--------------------------------------------------------------------------------------------
static int Comm_ObjPut( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_OBJ_PUT, "COMM_OBJ_PUT: comm[0] = %d\n", comm[0] );

//	CATS_ObjectPosSetCap( awk->cap[PRM_OBJ_ARCEUS], comm[1], comm[2] );

	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：OBJとBGのプライオリティ設定
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_OBJ_BG_PRI_CHANGE
 * @li	comm[1] = pri
 */
//--------------------------------------------------------------------------------------------
static int Comm_ObjBgPriChange( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_OBJ_BG_PRI_CHANGE, "COMM_OBJ_BG_PRI_CHANGE: comm[0] = %d\n", comm[0] );

//	CATS_ObjectBGPriSetCap( awk->cap[PRM_OBJ_ARCEUS], comm[1] );

	return CDEMOCOMM_RET_FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：ＢＧスクリーンアニメ
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_BG_FRM_ANIME
 * @li	comm[1] = ウェイト
 */
//--------------------------------------------------------------------------------------------
static int Comm_BgScreenAnime( CDEMO_WORK * wk, const int * comm )
{
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_BG_FRM_ANIME, "COMM_BG_FRM_ANIME: comm[0] = %d\n", comm[0] );

//	wk->bgsa_chr  = NCGR_START_IDX + comm[1];
//	wk->bgsa_pal  = NCLR_START_IDX + comm[1];
//	wk->bgsa_scr  = NSCR_START_IDX + comm[1];
	wk->bgsa_num  = 0;
	wk->bgsa_wait = comm[1];
	wk->bgsa_cnt  = 0;
	wk->bgsa_load = 0;
	wk->bgsa_seq  = 0;

	wk->main_seq = CDEMOSEQ_MAIN_BG_SCRN_ANM;

	return CDEMOCOMM_RET_NEXT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンド：デバッグプリント
 *
 * @param		wk		ワーク
 * @param		comm	コマンドデータ
 *
 * @retval	"COMM_RET_FALSE = コマンド終了"
 *
 * @li	comm[0] = COMM_DEBUG_PRINT
 */
//--------------------------------------------------------------------------------------------
static int Comm_DebugPrint( CDEMO_WORK * wk, const int * comm )
{
#ifdef PM_DEBUG
	GF_ASSERT_MSG( comm[0] == CDEMOCOMM_DEBUG_PRINT, "COMM_DEBUG_PRINT: comm[0] = %d\n", comm[0] );

	OS_Printf( "frame ID: %d = %d\n", comm[1], wk->debug_count );
	{
		RTCTime	time;
		RTC_GetTime( &time );
		OS_Printf( "sec = %d\n", time.second );
	}
	{
		if( comm[1] == 0 ){
			wk->stick = OS_GetTick();
		}else{
			wk->etick = OS_GetTick();
			OS_Printf( "tick = %d\n", OS_TicksToMilliSeconds32( wk->etick - wk->stick ) );
		}
	}
#endif	// PM_DEBUG

	return CDEMOCOMM_RET_FALSE;
}

