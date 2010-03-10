//============================================================================================
/**
 * @file		fskill_osyaberi.c
 * @brief		フィールド技：「おしゃべり」
 * @author	Hiroyuki Nakamura
 * @date		10.03.05
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/bmp_winframe.h"
#include "system/poke2dgra.h"
#include "sound/pm_sndsys.h"
#include "sound/pm_wb_voice.h"
#include "sound/snd_mic.h"

#include "fieldmap.h"

#include "fskill_osyaberi.h"

#include "message.naix"
#include "msg/msg_perap.h"



//============================================================================================
//============================================================================================
enum {
	SEQ_INIT = 0,			// 初期化
	SEQ_CREATE,
	SEQ_RESET_MSG,		//「忘れさせますか」
	SEQ_RESET_YESNO,	//「忘れさせますか」はい・いいえ
	SEQ_MIC_INIT,				// マイク準備
	SEQ_MIC_INIT_WAIT,	// マイク準備待ち
	SEQ_START_MSG,		// 開始メッセージ
	SEQ_MAIN,					// 覚えさせる
	SEQ_ANM,					// アニメ
	SEQ_END_MSG,			//「覚えた」
	SEQ_VOICE_PLAY,		// 再生
	SEQ_VOICE_WAIT,		// 鳴き声終了待ち
	SEQ_REC_ERR,			// 録音失敗
	SEQ_RELEASE,

	SEQ_END,
};

typedef struct {
	PERAPVOICE * sv;

  GAMESYS_WORK * gsys;
  GAMEDATA * gdata;
  FIELDMAP_WORK * fieldWork;
  FIELD_PLAYER * field_player;
	POKEMON_PARAM * pp;

	GFL_MSGDATA * msgData;
	FLDMSGBG * msgBG;
	FLDMSGWIN_STREAM * msgWin;
	FLDMENUFUNC * menuFunc;
	WORDSET * wset;
	STRBUF * exp;

//	GFL_ARCUTIL_TRANSINFO	winCgx;
	GFL_BMPWIN * pokeWin;

	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk;
	u32	chrRes;
	u32	palRes;
	u32	celRes;

	u8	anmLoop;
	s8	anmJump;

	u8	poke_pos;
//	u8	recTime;
	u32	voicePlayer;
	BOOL	recFlag;

	void * testBuff;

//	u8	subSeq;
//	u8	bgPri[4];
//	int	disp;

}OSYABERI_WORK;

#define	TMP_STR_BUFF_SIZ		( 1024 )

#define	REC_TIME		( 30*1 )


#define	POKEWIN_FRM		( GFL_BG_FRAME1_M )
#define	POKEWIN_PX		( 10 )
#define	POKEWIN_PY		( 3 )
#define	POKEWIN_SX		( 12 )
#define	POKEWIN_SY		( 12 )
#define	POKEWIN_PAL		( 13 )
#define	POKEWIN_CGX		( 1 )



//============================================================================================
//============================================================================================
static GMEVENT_RESULT MainEvent( GMEVENT * event, int * seq, void * work );
static void VoiceRec_CallBack( MICResult result, void * arg );
static void FieldPauseOn( OSYABERI_WORK * wk );
static void FieldPauseOff( OSYABERI_WORK * wk );
static void PutPokeWin( OSYABERI_WORK * wk );
static void DelPokeWin( OSYABERI_WORK * wk );
static BOOL MainPerapAnm( OSYABERI_WORK * wk );

extern u16 PMV_DBG_CustomVoicePlay( void*	wave,				// [in]波形データ
															u32		size,				// [in]波形サイズ(MAX 26000)
															int		rate,				// [in]波形再生レート
															int		speed,			// [in]波形再生スピード
															s8		volume);			// [in]再生ボリューム
extern u8 * SND_MIC_GetTestBuffer(void);


//--------------------------------------------------------------------------------------------
/**
 * @brief		イベント起動
 *
 * @param		gsys				GAMESYS_WORK
 * @param		fieldmap		FIELDMAP_WORK
 * @param		poke_pos		手持ち位置
 *
 * @return	GMEVENT
 */
//--------------------------------------------------------------------------------------------
GMEVENT * EVENT_FieldSkillOsyaberi( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, u8 poke_pos )
{
	GMEVENT * event;
	OSYABERI_WORK * wk;
	
	event = GMEVENT_Create( gsys, NULL, MainEvent, sizeof(OSYABERI_WORK) );

	wk = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( wk, sizeof(OSYABERI_WORK) );

	wk->gsys  = gsys;
	wk->gdata = GAMESYSTEM_GetGameData( gsys );

	wk->sv = SaveData_GetPerapVoice( GAMEDATA_GetSaveControlWork(wk->gdata) );

	wk->fieldWork    = fieldmap;
	wk->field_player = FIELDMAP_GetFieldPlayer( fieldmap );

	wk->pp = PokeParty_GetMemberPointer( GAMEDATA_GetMyPokemon(wk->gdata), poke_pos );

	wk->msgBG = FIELDMAP_GetFldMsgBG( wk->fieldWork );

	wk->poke_pos = poke_pos;
	wk->recFlag  = FALSE;

	return event;
}

static GMEVENT_RESULT MainEvent( GMEVENT * event, int * seq, void * work )
{
	OSYABERI_WORK * wk = work;

	switch( *seq ){
	case SEQ_INIT:					// 初期化
		FieldPauseOn( wk );
		*seq = SEQ_CREATE;
		break;

	case SEQ_CREATE:
		wk->msgData = FLDMSGBG_CreateMSGDATA( wk->msgBG, NARC_message_perap_dat );
		wk->msgWin  = FLDMSGWIN_STREAM_AddTalkWin( wk->msgBG, wk->msgData );
		wk->wset    = WORDSET_Create( HEAPID_FIELDMAP );
		wk->exp     = GFL_STR_CreateBuffer( TMP_STR_BUFF_SIZ, HEAPID_FIELDMAP );
		PutPokeWin( wk );

		if( PERAPVOICE_GetExistFlag(wk->sv) == TRUE ){
			FLDMSGWIN_STREAM_PrintStart( wk->msgWin, 0, 0, perap_msg_00 );
			*seq = SEQ_RESET_MSG;
		}else{
			FLDMSGWIN_STREAM_PrintStart( wk->msgWin, 0, 0, perap_msg_06 );
			*seq = SEQ_MIC_INIT;
		}
		break;

	case SEQ_RESET_MSG:			//「忘れさせますか」
		if( FLDMSGWIN_STREAM_Print( wk->msgWin ) == TRUE ){
			wk->menuFunc = FLDMENUFUNC_AddYesNoMenu( wk->msgBG, FLDMENUFUNC_YESNO_YES );
			*seq = SEQ_RESET_YESNO;
		}
		break;

	case SEQ_RESET_YESNO:		//「忘れさせますか」はい・いいえ
		switch( FLDMENUFUNC_ProcYesNoMenu(wk->menuFunc) ){
		case FLDMENUFUNC_YESNO_YES:
			FLDMENUFUNC_DeleteMenu( wk->menuFunc );
			FLDMSGWIN_STREAM_ClearMessage( wk->msgWin );
			FLDMSGWIN_STREAM_PrintStart( wk->msgWin, 0, 0, perap_msg_06 );
			*seq = SEQ_MIC_INIT;
			break;

		case FLDMENUFUNC_YESNO_NO:
			FLDMENUFUNC_DeleteMenu( wk->menuFunc );
			*seq = SEQ_RELEASE;
			break;
		}
		break;

	case SEQ_MIC_INIT:			// マイク準備
		if( FLDMSGWIN_STREAM_Print( wk->msgWin ) == TRUE ){
		  GFL_UI_SleepDisable( GFL_UI_SLEEP_MIC );		// スリープ禁止
			SND_MIC_Init( HEAPID_FIELDMAP );
			*seq = SEQ_MIC_INIT_WAIT;
		}
		break;

	case SEQ_MIC_INIT_WAIT:	// マイク準備待ち
		SND_MIC_Main();
		if( SND_MIC_IsAmpOnWaitFlag() == TRUE ){
			PMSND_PauseBGM( TRUE );
			FLDMSGWIN_STREAM_ClearMessage( wk->msgWin );
			FLDMSGWIN_STREAM_PrintStart( wk->msgWin, 0, 0, perap_msg_01 );
			*seq = SEQ_START_MSG;
		}
		break;

	case SEQ_START_MSG:			// 開始メッセージ
		if( FLDMSGWIN_STREAM_Print( wk->msgWin ) == TRUE ){
/*
			if( SND_PERAP_VoiceRecStart( VoiceRec_CallBack, &wk->recFlag ) == MIC_RESULT_SUCCESS ){
				*seq = SEQ_MAIN;
			}else{
				STRBUF * str = GFL_MSG_CreateString( wk->msgData, perap_msg_03 );
				WORDSET_RegisterPokeNickName( wk->wset, 0, wk->pp );
				WORDSET_ExpandStr( wk->wset, wk->exp, str );
				GFL_STR_DeleteBuffer( str );
				FLDMSGWIN_STREAM_ClearMessage( wk->msgWin );
				FLDMSGWIN_STREAM_PrintStrBufStart( wk->msgWin, 0, 0, wk->exp );
				SND_MIC_Exit();
				GFL_UI_SleepEnable( GFL_UI_SLEEP_MIC );		// スリープ許可
				PMSND_PauseBGM( FALSE );
				*seq = SEQ_REC_ERR;		// 失敗
			}
*/
			MICAutoParam mic;
			wk->testBuff = GFL_HEAP_AllocClearMemory( HEAPID_FIELDMAP, 10000 );
			mic.type			= MIC_SAMPLING_TYPE_SIGNED_8BIT;	//サンプリング種別
			mic.buffer		= (void*)MATH_ROUNDUP32( (int)(wk->testBuff) );
			mic.size			= 8000;
//			mic.rate			= MIC_SAMPLING_RATE_8K;
			mic.rate      = MIC_SAMPLING_RATE_8180;
			mic.loop_enable   = FALSE;
			mic.full_callback = VoiceRec_CallBack;
			mic.full_arg      = &wk->recFlag;
			{
//				MICResult	ret = MIC_StartAutoSampling( &mic );
				MICResult	ret = MIC_StartLimitedSampling( &mic );
				OS_Printf( "ret = %d\n", ret );
				OS_Printf( "mic = 0x%08x, buf = 0x%08x\n", mic.buffer, wk->testBuff );
			}
			*seq = SEQ_MAIN;
		}
		break;

	case SEQ_MAIN:					// 覚えさせる
		if( wk->recFlag == TRUE ){
			SND_PERAP_VoiceDataSave( wk->sv );
			SND_MIC_Exit();
			GFL_UI_SleepEnable( GFL_UI_SLEEP_MIC );		// スリープ許可
			PMSND_PauseBGM( FALSE );
			*seq = SEQ_ANM;
		}
		break;

	case SEQ_ANM:						// アニメ
		if( MainPerapAnm( wk ) == FALSE ){
			STRBUF * str = GFL_MSG_CreateString( wk->msgData, perap_msg_02 );
			WORDSET_RegisterPokeNickName( wk->wset, 0, wk->pp );
			WORDSET_ExpandStr( wk->wset, wk->exp, str );
			GFL_STR_DeleteBuffer( str );
			FLDMSGWIN_STREAM_ClearMessage( wk->msgWin );
			FLDMSGWIN_STREAM_PrintStrBufStart( wk->msgWin, 0, 0, wk->exp );
			*seq = SEQ_END_MSG;
		}
		break;

	case SEQ_END_MSG:				//「覚えた」
		if( FLDMSGWIN_STREAM_Print( wk->msgWin ) == TRUE ){
			*seq = SEQ_VOICE_PLAY;
		}
		break;

	case SEQ_VOICE_PLAY:		// 再生
		{
//			PMV_REF	pmvRef;
//	    PMV_MakeRefDataMine( &pmvRef );
//			OS_Printf( "addr = 0x%08x\n", wk->sv );
			wk->voicePlayer = PMV_DBG_CustomVoicePlay(
													(void*)MATH_ROUNDUP32( (int)(wk->testBuff) ),
													8000,
													MIC_SAMPLING_RATE_8180,//MIC_SAMPLING_RATE_8K,
													32768,
													127 );
//			wk->voicePlayer = PMVOICE_Play( MONSNO_PERAPPU, 0, 64, FALSE, 0, 0, FALSE, (u32)&pmvRef );
		}
		*seq = SEQ_VOICE_WAIT;
		break;

	case SEQ_VOICE_WAIT:		// 鳴き声終了待ち
		if( PMVOICE_CheckPlay( wk->voicePlayer ) == FALSE ){
			GFL_HEAP_FreeMemory( wk->testBuff );
			*seq = SEQ_RELEASE;
		}
		break;

	case SEQ_REC_ERR:				// 録音失敗
		if( FLDMSGWIN_STREAM_Print( wk->msgWin ) == TRUE ){
			*seq = SEQ_RELEASE;
		}
		break;

	case SEQ_RELEASE:
		DelPokeWin( wk );
		GFL_STR_DeleteBuffer( wk->exp );
		WORDSET_Delete( wk->wset );
		FLDMSGWIN_STREAM_Delete( wk->msgWin );
		FLDMSGBG_DeleteMSGDATA( wk->msgData );
		*seq = SEQ_END;
		break;

	case SEQ_END:
		FieldPauseOff( wk );
		return GMEVENT_RES_FINISH;
	}

	return GMEVENT_RES_CONTINUE;
}

static void VoiceRec_CallBack( MICResult result, void * arg )
{
	BOOL * flg = arg;
	*flg = TRUE;
}


static void FieldPauseOn( OSYABERI_WORK * wk )
{
	FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( wk->gsys );
	// OBJポーズ
	MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys(fieldmap) );
	// メイン処理フック
//	FIELDMAP_SetMainFuncHookFlg( fieldmap, TRUE );
}

static void FieldPauseOff( OSYABERI_WORK * wk )
{
	FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( wk->gsys );
	//メインフック解除
//	FIELDMAP_SetMainFuncHookFlg( fieldmap, FALSE );
	//ＯＢＪのポーズ解除
	MMDLSYS_ClearPauseMoveProc( FIELDMAP_GetMMdlSys(fieldmap) );
}





static void InitPerapObj( OSYABERI_WORK * wk )
{
  ARCHANDLE * ah;
	POKEMON_PASO_PARAM * ppp;
	BOOL	fast;

	wk->clunit = GFL_CLACT_UNIT_Create( 1, 0, HEAPID_FIELDMAP );

	ppp = PP_GetPPPPointer( wk->pp );

	ah = POKE2DGRA_OpenHandle( HEAPID_FIELDMAP );

	fast = PPP_FastModeOn( ppp );
	wk->chrRes = POKE2DGRA_OBJ_CGR_RegisterPPP( ah, ppp, POKEGRA_DIR_FRONT, CLSYS_DRAW_MAIN, HEAPID_FIELDMAP );
	wk->palRes = POKE2DGRA_OBJ_PLTT_RegisterPPP( ah, ppp, POKEGRA_DIR_FRONT, CLSYS_DRAW_MAIN, 0, HEAPID_FIELDMAP );
	wk->celRes = POKE2DGRA_OBJ_CELLANM_RegisterPPP( ppp, POKEGRA_DIR_FRONT, APP_COMMON_MAPPING_64K, CLSYS_DRAW_MAIN, HEAPID_FIELDMAP );
	PPP_FastModeOff( ppp, fast );

  GFL_ARC_CloseDataHandle( ah );

	{
		GFL_CLWK_DATA	dat = { 128, 72, 0, 0, 0 };
		wk->clwk = GFL_CLACT_WK_Create(
								wk->clunit,
								wk->chrRes, wk->palRes, wk->celRes,
								&dat, CLSYS_DRAW_MAIN, HEAPID_FIELDMAP );
	}
}

static void ExitPerapObj( OSYABERI_WORK * wk )
{
	GFL_CLACT_WK_Remove( wk->clwk );
	GFL_CLGRP_CGR_Release( wk->chrRes );
	GFL_CLGRP_PLTT_Release( wk->palRes );
	GFL_CLGRP_CELLANIM_Release( wk->celRes );
	GFL_CLACT_UNIT_Delete( wk->clunit );
}

static BOOL MainPerapAnm( OSYABERI_WORK * wk )
{
	GFL_CLACTPOS	pos;

	if( wk->anmLoop == 0 ){
		wk->anmJump = -4;
	}
	if( wk->anmJump == -4 ){
		wk->anmJump = 3;
		wk->anmLoop++;
		if( wk->anmLoop == 3 ){
			return FALSE;
		}
	}

	GFL_CLACT_WK_GetPos( wk->clwk, &pos, CLSYS_DRAW_MAIN );
	pos.y -= wk->anmJump;
	GFL_CLACT_WK_SetPos( wk->clwk, &pos, CLSYS_DRAW_MAIN );

	wk->anmJump--;

	return TRUE;
}

static void PutPokeWin( OSYABERI_WORK * wk )
{
	wk->pokeWin = GFL_BMPWIN_Create(
									POKEWIN_FRM, POKEWIN_PX, POKEWIN_PY,
									POKEWIN_SX, POKEWIN_SY, POKEWIN_PAL, GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->pokeWin), 0xff);
	GFL_BMPWIN_TransVramCharacter( wk->pokeWin );
	GFL_BMPWIN_MakeScreen( wk->pokeWin );
	BmpWinFrame_Write( wk->pokeWin, WINDOW_TRANS_ON_V, POKEWIN_CGX, POKEWIN_PAL );

	InitPerapObj( wk );
}

static void DelPokeWin( OSYABERI_WORK * wk )
{
	ExitPerapObj( wk );

	BmpWinFrame_Clear( wk->pokeWin, WINDOW_TRANS_ON_V );
	GFL_BMPWIN_Delete( wk->pokeWin );
}
