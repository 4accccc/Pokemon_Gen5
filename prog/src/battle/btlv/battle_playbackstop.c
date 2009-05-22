//==============================================================================
/**
 * @file	battle_playbackstop.c
 * @brief	戦闘録画再生中の停止ボタン制御
 * @author	matsuda change by soga
 * @date	2009.04.15(水)
 */
//==============================================================================
#if 0
#include "common.h"
#include "battle/battle_common.h"
#include "battle_input.h"
#include "battle/battle_tcb_pri.h"
#include "battle/fight_tool.h"
#include "battle_playbackstop.h"
#include "system/snd_tool.h"
#include "battle_snd_def.h"
#endif

#include "gflib.h"

#include "poke_tool/poke_tool.h"
#include "battle_input.h"
#include "battle_playbackstop.h"
#include "battle_tcb_pri.h"

#include "arc_def.h"

//MAKEを通すために
//中断処理をするフラグを見ている箇所をコメントアウト

//==============================================================================
//	定数定義
//==============================================================================
///サーバーに停止フラグをセットする前に少しウェイトを入れる(押した時のSEを聞かせるため)
#define RPS_END_WAIT		(8)


//==============================================================================
//	構造体定義
//==============================================================================
///録画再生の停止ボタン制御システム構造体
typedef struct{
//	BATTLE_WORK *bw;	///<バトルワークへのポインタ
	BI_PARAM_PTR	bip;	
	HEAPID	heapID;
	s16 seq;			///<現在のシーケンス番号
	s16 wait;			///<ウェイトカウンタ
}PLAYBACK_STOP_WORK;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void PlayBackStop_Main( GFL_TCB *tcb, void *work );



//--------------------------------------------------------------
/**
 * @brief   戦闘録画再生の停止ボタン制御システム作成
 * @param   bw		
 * @retval  制御システムのTCBポインタ
 */
//--------------------------------------------------------------
GFL_TCB * PlayBackStopButton_Create( BI_PARAM_PTR bip, GFL_TCBSYS *tcbsys, HEAPID heapID )
{
	PLAYBACK_STOP_WORK *psw;
	GFL_TCB *tcb;

	psw = GFL_HEAP_AllocMemory( heapID, sizeof( PLAYBACK_STOP_WORK ) );
	MI_CpuClear8( psw, sizeof( PLAYBACK_STOP_WORK ) );
	
	psw->bip = bip;
	psw->heapID = heapID;
	tcb = GFL_TCB_AddTask( tcbsys, PlayBackStop_Main, psw, TCBPRI_PLAYBACK_STOP );
	
	return tcb;
}

//--------------------------------------------------------------
/**
 * @brief   戦闘録画再生の停止ボタン制御システム解放
 * @param   tcb		制御システムのTCBポインタ
 */
//--------------------------------------------------------------
void PlayBackStopButton_Free( GFL_TCB *tcb )
{
	PLAYBACK_STOP_WORK *psw;
	
	psw = GFL_TCB_GetWork( tcb );
	GFL_HEAP_FreeMemory( psw );
	GFL_TCB_DeleteTask( tcb );
}


//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   戦闘録画再生の停止ボタン制御システム：メイン処理
 *
 * @param   tcb			TCBへのポインタ
 * @param   work		PLAYBACK_STOP_WORK構造体
 */
//--------------------------------------------------------------
static void PlayBackStop_Main( GFL_TCB *tcb, void *work )
{
	PLAYBACK_STOP_WORK *psw = work;
	enum{
		SEQ_INIT,
		SEQ_IN_EFF_WAIT,
		SEQ_MAIN,
		SEQ_WAIT,
		SEQ_END,
	};
	
	switch(psw->seq){
	case SEQ_INIT:
		GF_ASSERT( psw->bip != NULL );
		
		{
			ARCHANDLE* hdl_bg, *hdl_obj;

			hdl_bg  = GFL_ARC_OpenDataHandle( ARCID_BATT_BG,  psw->heapID ); 
			hdl_obj = GFL_ARC_OpenDataHandle( ARCID_BATT_OBJ, psw->heapID );
			BINPUT_CreateBG(hdl_bg, hdl_obj, psw->bip, BINPUT_TYPE_PLAYBACK_STOP, FALSE, NULL);
			GFL_ARC_CloseDataHandle( hdl_bg );
			GFL_ARC_CloseDataHandle( hdl_obj );
		}
		psw->seq++;
		break;
	case SEQ_IN_EFF_WAIT:
		if(BINPUT_EffectEndCheck( psw->bip ) == TRUE){
			psw->seq++;
		}
		break;
	case SEQ_MAIN:
#if 0
		if(BattleWorkBattleRecStopFlagCheck(psw->bw) == TRUE){
			if(BINPUT_TouchCheck( psw->bip ) == TRUE){
				Snd_SePlay(BSE_TP_DECIDE);
				psw->seq++;
			}
		}
#endif
		break;
	case SEQ_WAIT:
#if 0
		psw->wait++;
		if(psw->wait > RPS_END_WAIT){
			BattleWorkBattleRecStopFlagSet(psw->bw,BI_SYSMSG_STOP);
			psw->seq++;
		}
#endif
		break;

	case SEQ_END:	//全ての処理終了。Freeを呼ばれるのを待つ
		break;
	}
}

