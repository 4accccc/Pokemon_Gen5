//======================================================================
/**
 * @file	script.h
 * @brief	スクリプト制御メイン部分
 * @author	Satoshi Nohara
 * @date	05.08.04
 *
 * 01.03.28	Sousuke Tamada
 * 03.04.15	Satoshi Nohara
 * 05.04.25 Hiroyuki Nakamura
 */
//======================================================================
#ifndef SCRIPT_H
#define SCRIPT_H

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "eventwork.h"

#include "fldmmdl.h"    //MMDL

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
//	debug
//--------------------------------------------------------------
#define SCRIPT_PL_NULL	//PL箇所無効化
#define DEBUG_SCRIPT		//定義でデバッグ機能有効

//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  SCRID_NULL = 0
};
//========================================================================
//	struct
//========================================================================
//--------------------------------------------------------------
//	スクリプト制御ワークへの不完全型ポインタ
//--------------------------------------------------------------
typedef	struct _TAG_SCRIPT_WORK SCRIPT_WORK;

//======================================================================
//	proto
//======================================================================
//--------------------------------------------------------------
/**
 * スクリプトイベントセット
 * @param	scr_id		スクリプトID
 * @param	obj			話しかけ対象OBJのポインタ(ない時はNULL)
 * @retval	none
 */
//--------------------------------------------------------------
extern GMEVENT * SCRIPT_SetEventScript(
		GAMESYS_WORK *gsys, u16 scr_id, MMDL *obj,
		HEAPID temp_heapID );

//--------------------------------------------------------------
/**
 * スクリプトイベントコール
 * @param	event		GMEVENT_CONTROL型のポインタ
 * @param	scr_id		スクリプトID
 * @param	obj			話しかけ対象OBJのポインタ(ない時はNULL)
 * @param	ret_script_wk	スクリプト結果を代入するワークのポインタ
 * @retval	none
 */
//--------------------------------------------------------------
extern SCRIPT_WORK * SCRIPT_CallScript( GMEVENT *event,
	u16 scr_id, MMDL *obj, void *ret_script_wk, HEAPID temp_heapID );

//--------------------------------------------------------------
/**
 * スクリプトイベント切替
 * @param	event		GMEVENT_CONTROL型のポインタ
 * @param	scr_id		スクリプトID
 * @param	obj			話しかけ対象OBJのポインタ(ない時はNULL)
 * @retval	SCRIPT_WORK
 * 他のイベントからスクリプトへの切替を行う
 */
//--------------------------------------------------------------
extern SCRIPT_WORK * SCRIPT_ChangeScript( GMEVENT *event,
		u16 scr_id, MMDL *obj, HEAPID temp_heapID );


//------------------------------------------------------------------
/**
 * @brief スクリプトIDのチェック
 * @param script_id
 * @return  BOOL  FALSEのとき、実行可能なスクリプト
 */
//------------------------------------------------------------------
extern BOOL SCRIPT_IsValidScriptID( u16 script_id );

//======================================================================
//
//
//    特殊スクリプト関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * 特殊スクリプト呼び出し
 * @param gsys
 * @param heapID
 * @param script_id
 */
//--------------------------------------------------------------
extern void SCRIPT_CallSpecialScript( GAMESYS_WORK *gsys, HEAPID heapID, u16 script_id );

//------------------------------------------------------------------
/**
 * @brief	ゲーム開始 スクリプト初期設定の実行
 */
//------------------------------------------------------------------
extern void SCRIPT_CallGameStartInitScript( GAMESYS_WORK *gsys, HEAPID heapID );

#ifdef  PM_DEBUG
//------------------------------------------------------------------
/**
 * @brief	ゲーム開始 スクリプト初期設定の実行（デバッグスタート）
 */
//------------------------------------------------------------------
extern void SCRIPT_CallDebugGameStartInitScript( GAMESYS_WORK *gsys, HEAPID heapID );
#endif  //PM_DEBUG

//------------------------------------------------------------------
/**
 * @brief	ゲームクリア時処理のスクリプト実行
 */
//------------------------------------------------------------------
extern void SCRIPT_CallGameClearScript( GAMESYS_WORK *gsys, HEAPID heapID );

//------------------------------------------------------------------
/**
 * @brief 特殊スクリプト呼び出し：フィールド復帰（初回のみ）
 * @param gsys
 * @param heapID
 * @retval  TRUE    特殊スクリプトを実行した
 * @retval  FALSE   特殊スクリプトは存在しない
 */
//------------------------------------------------------------------
extern BOOL SCRIPT_CallFieldInitScript( GAMESYS_WORK * gsys, HEAPID heapID );

//------------------------------------------------------------------
/**
 * @brief 特殊スクリプト呼び出し：ゾーン切り替え
 * @param gsys
 * @param heapID
 * @retval  TRUE    特殊スクリプトを実行した
 * @retval  FALSE   特殊スクリプトは存在しない
 */
//------------------------------------------------------------------
extern BOOL SCRIPT_CallZoneChangeScript( GAMESYS_WORK * gsys, HEAPID heapID);

//------------------------------------------------------------------
/**
 * @brief 特殊スクリプト呼び出し：フィールド復帰
 * @param gsys
 * @param heapID
 * @retval  TRUE    特殊スクリプトを実行した
 * @retval  FALSE   特殊スクリプトは存在しない
 */
//------------------------------------------------------------------
extern BOOL SCRIPT_CallFieldRecoverScript( GAMESYS_WORK * gsys, HEAPID heapID );

//------------------------------------------------------------------
/**
 * @brief	シーン起動スクリプトの検索
 * @param gsys
 * @param heapID
 * @retval  NULL      現在はシーン起動スクリプトが存在しない
 * @retval  GMEVENT   起動するべきスクリプトをキーに生成されたイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT * SCRIPT_SearchSceneScript( GAMESYS_WORK * gsys, HEAPID heapID);


//======================================================================
//
//	フラグ関連
//
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	マップ内限定のセーブフラグをクリアする
 *
 *
 * @return	none
 */
//------------------------------------------------------------------
extern void LocalEventFlagClear( EVENTWORK * eventwork );

//------------------------------------------------------------------
/**
 * @brief	1日経過ごとにクリアされるフラグをクリアする
 *
 *
 * @return	none
 */
//------------------------------------------------------------------
extern void TimeEventFlagClear( EVENTWORK * eventwork );


//======================================================================
//
//	トレーナーフラグ関連
//
//	・スクリプトIDから、トレーナーIDを取得して、フラグチェック
//	BOOL CheckEventFlagTrainer( fsys, GetTrainerIdByScriptId(scr_id) );
//
//======================================================================

//--------------------------------------------------------------
/**
 * スクリプトIDから、トレーナーIDを取得
 *
 * @param   scr_id		スクリプトID
 *
 * @retval  "トレーナーID = フラグインデックス"
 */
//--------------------------------------------------------------
extern u16 SCRIPT_GetTrainerID_ByScriptID( u16 scr_id );

//--------------------------------------------------------------
/**
 * スクリプトIDから、左右どちらのトレーナーか取得
 *
 * @param   scr_id		スクリプトID
 *
 * @retval  "0=左、1=右"
 */
//--------------------------------------------------------------
extern BOOL SCRIPT_GetTrainerLR_ByScriptID( u16 scr_id );

//--------------------------------------------------------------
/**
 * トレーナーIDから、ダブルバトルタイプか取得
 *
 * @param   tr_id		トレーナーID
 *
 * @retval  "0=シングルバトル、1=ダブルバトル"
 */
//--------------------------------------------------------------
extern BOOL SCRIPT_CheckTrainer2vs2Type( u16 tr_id );

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをチェックする
 *
 * @param	tr_id		トレーナーID
 *
 * @retval	"1 = フラグON"
 * @retval	"0 = フラグOFF"
 */
//------------------------------------------------------------------
extern BOOL SCRIPT_CheckEventFlagTrainer( EVENTWORK *ev, u16 tr_id );

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをセットする
 *
 * @param	tr_id		トレーナーID
 *
 * @return	none
 */
//------------------------------------------------------------------
extern void SCRIPT_SetEventFlagTrainer( EVENTWORK *ev, u16 tr_id );

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをリセットする
 *
 * @param	tr_id		トレーナーID
 *
 * @return	none
 */
//------------------------------------------------------------------
extern void SCRIPT_ResetEventFlagTrainer( EVENTWORK *ev, u16 tr_id );


extern void SCRIPT_SetTrainerEyeData( GMEVENT *event, MMDL *mmdl,
    s16 range, u16 dir, u16 scr_id, u16 tr_id, int tr_type, int tr_no );

//--------------------------------------------------------------
/**
 * プログラムからスクリプトへの引数となるパラメータをセット
 * @param	sc SCRIPT_WORK
 * @param	prm0	パラメータ０（SCWK_PARAM0）
 * @param	prm1	パラメータ１（SCWK_PARAM1）
 * @param	prm2	パラメータ２（SCWK_PARAM2）
 * @param	prm3	パラメータ３（SCWK_PARAM3）
 * @retval none
 */
//--------------------------------------------------------------
extern void SCRIPT_SetScriptWorkParam( SCRIPT_WORK *sc, u16 prm0, u16 prm1, u16 prm2, u16 prm3 );

//--------------------------------------------------------------
/**
 * サブプロセス用ワークのポインターアドレスを取得
 * @param	sc SCRIPT_WORK
 * @retval none
 *
 * 使い終わったら必ずNULLクリアすること！
 */
//--------------------------------------------------------------
extern void** SCRIPT_SetSubProcWorkPointerAdrs( SCRIPT_WORK *sc );

//--------------------------------------------------------------
/**
 * サブプロセス用ワークのポインターを取得
 * @param	sc SCRIPT_WORK
 * @retval none
 *
 * 使い終わったら必ずNULLクリアすること！
 */
//--------------------------------------------------------------
extern void* SCRIPT_SetSubProcWorkPointer( SCRIPT_WORK *sc );

//--------------------------------------------------------------
/**
 * サブプロセス用ワーク領域の解放(ポインタがNULLでなければFree)
 * @param	sc SCRIPT_WORK
 * @retval none
 */
//--------------------------------------------------------------
extern void SCRIPT_FreeSubProcWorkPointer( SCRIPT_WORK *sc );

//--------------------------------------------------------------
/**
 * スクリプトイベントからスクリプトワークを取得
 * @param	GMEVENT *event
 * @retval  SCRIPT_WORK*
 */
//--------------------------------------------------------------
extern SCRIPT_WORK* SCRIPT_GetEventWorkToScriptWork( GMEVENT *event );

#endif	/* SCRIPT_H */
