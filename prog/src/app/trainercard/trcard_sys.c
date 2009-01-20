/**
 *	@file	trcard_sys.c
 *	@brief	トレーナーカードシステム
 *	@author	Miyuki Iwasawa
 *	@date	08.01.16
 */

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "savedata/config.h"
#include "savedata/trainercard_data.h"

//#include "app/mysign.h"
#include "app/trainer_card.h"
#include "trcard_sys.h"


typedef struct _TR_CARD_SYS{
 int heapID;
 
 void	*app_wk;	///<簡易会話モジュールワークの保存
 GFL_PROCSYS*	procSys;		///<サブプロセスワーク

 TRCARD_CALL_PARAM* tcp;
 TR_CARD_DATA *pTrCardData;

}TR_CARD_SYS;

//プロトタイプ宣言　ローカル
//================================================================
///オーバーレイプロセス
GFL_PROC_RESULT TrCardSysProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
GFL_PROC_RESULT TrCardSysProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
GFL_PROC_RESULT TrCardSysProc_End( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

//================================================================
///データ定義エリア
//================================================================
///オーバーレイプロセス定義データ
enum{
	CARD_INIT,
	CARD_WAIT,
	SIGN_INIT,
	SIGN_WAIT,
	CARDSYS_END,
};

static int sub_CardInit(TR_CARD_SYS* wk);
static int sub_CardWait(TR_CARD_SYS* wk);
static int sub_SignInit(TR_CARD_SYS* wk);
static int sub_SignWait(TR_CARD_SYS* wk);

//------------------------------------------------------------------
/**
 * @brief	サブプロセス呼び出し処理
 * @param	proc	サブプロセスポインタを保持するワークへのポインタ
 */
//------------------------------------------------------------------
static BOOL TrCardSysProcCall(GFL_PROCSYS ** procSys)
{
	if (*procSys) {
		if (GFL_PROC_LOCAL_Main(*procSys)) {
			GFL_PROC_LOCAL_Exit(*procSys);
			*procSys = NULL;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 *	@brief	トレーナーカードシステム呼び出し　初期化
 */
GFL_PROC_RESULT TrCardSysProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	TR_CARD_SYS* wk = NULL;
	TRCARD_CALL_PARAM* pp = (TRCARD_CALL_PARAM*)pwk;
	
	//ヒープ作成
	GFL_HEAP_CreateHeap(GFL_HEAPID_APP,HEAPID_TRCARD_SYS,0x1000);
	wk = GFL_PROC_AllocWork(proc,sizeof(TR_CARD_SYS),HEAPID_TRCARD_SYS);
	MI_CpuClear8(wk,sizeof(TR_CARD_SYS));

	//ヒープID保存
	wk->heapID = HEAPID_TRCARD_SYS;

	//データテンポラリ作成
	wk->tcp = pp;
	wk->pTrCardData = &pp->TrCardData;
	return GFL_PROC_RES_FINISH;
}

/**
 *	@brief	トレーナーカード　システムメイン
 */
GFL_PROC_RESULT TrCardSysProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	TR_CARD_SYS* wk = (TR_CARD_SYS*)mywk;
	
	switch(*seq){
	case CARD_INIT:
		*seq = sub_CardInit(wk);
		break;
	case CARD_WAIT:
		*seq = sub_CardWait(wk);
		break;
	case SIGN_INIT:
		*seq = sub_SignInit(wk);
		break;
	case SIGN_WAIT:
		*seq = sub_SignWait(wk);
		break;
	case CARDSYS_END:
	default:
		return GFL_PROC_RES_FINISH;
	}
	return GFL_PROC_RES_CONTINUE;
}

/**
 *	@brief	トレーナーカード　システム終了
 */
GFL_PROC_RESULT TrCardSysProc_End( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	TR_CARD_SYS* wk = (TR_CARD_SYS*)mywk;

	MI_CpuClear8(wk,sizeof(TR_CARD_SYS));

	//ワークエリア解放
	GFL_PROC_FreeWork(proc);
	
	GFL_HEAP_DeleteHeap(HEAPID_TRCARD_SYS);
	return GFL_PROC_RES_FINISH;
}

/**
 *	@brief	トレーナーカード呼び出し
 */
static int sub_CardInit(TR_CARD_SYS* wk)
{
	// オーバーレイID宣言
//	FS_EXTERN_OVERLAY(trainer_card);

	// プロセス定義データ
	static const GFL_PROC_DATA TrCardProcData = {
		TrCardProc_Init,
		TrCardProc_Main,
		TrCardProc_End,
	};
//	wk->proc = PROC_Create(&TrCardProcData,wk->tcp,wk->heapID);
	wk->procSys = GFL_PROC_LOCAL_boot( wk->heapID );
	GFL_PROC_LOCAL_CallProc( wk->procSys, NO_OVERLAY_ID, &TrCardProcData,(void*)wk->tcp);
	return CARD_WAIT;
}

/**
 *	@brief	トレーナーカード待ち
 */
static int sub_CardWait(TR_CARD_SYS* wk)
{
	if(!TrCardSysProcCall(&wk->procSys)){
		return CARD_WAIT;
	}

	if(wk->tcp->value){
		return SIGN_INIT;
	}
	return CARDSYS_END;
}

/**
 *	@brief	サイン呼び出し
 */
static int sub_SignInit(TR_CARD_SYS* wk)
{
//	FS_EXTERN_OVERLAY(mysign);
#if 0
	// プロセス定義データ
	const GFL_PROC_DATA MySignProcData = {
		MySignProc_Init,
		MySignProc_Main,
		MySignProc_End,
	};
		
//	wk->proc = PROC_Create(&MySignProcData,(void*)wk->tcp->savedata,wk->heapID);
	wk->procSys = GFL_PROC_LOCAL_boot( wk->heapID );
	GFL_PROC_LOCAL_CallProc( wk->procSys, NO_OVERLAY_ID, &MySignProcData,wk->tcp);
#endif
	return SIGN_WAIT;
}

/**
 *	@brief	サイン待ち
 */
static int sub_SignWait(TR_CARD_SYS* wk)
{
	if(!TrCardSysProcCall(&wk->procSys)){
		return SIGN_WAIT;
	}
	//サインデータを呼び出しテンポラリに書き戻し
	{
		TR_CARD_SV_PTR trc_ptr = TRCSave_GetSaveDataPtr((SAVE_CONTROL_WORK*)wk->tcp->savedata);
		//サインデータの有効/無効フラグを取得(金銀ローカルでのみ有効)
		wk->tcp->TrCardData.MySignValid = TRCSave_GetSigned(trc_ptr);
		//サインデータをセーブデータからコピー
		MI_CpuCopy8(TRCSave_GetSignDataPtr(trc_ptr),
				wk->tcp->TrCardData.SignRawData, SIGN_SIZE_X*SIGN_SIZE_Y*8 );
	}
	return CARD_INIT;
}


#if 0
#pragma mark [>end edit


#pragma mark [>start edit
#endif


