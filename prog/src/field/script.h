//======================================================================
/**
 * @file	script.h
 * @bfief	スクリプト制御メイン部分
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


#include "field/fieldmap_proc.h"
#include "field/field_msgbg.h"

#if 0
#include "field_common.h"
#include "field_event.h"
#include "vm.h"					//VM_CMD
#include "script_def.h"			//特殊スクリプト定義
#endif

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
//	debug
//--------------------------------------------------------------
#define SCRIPT_PL_NULL	//PL箇所無効化
#define DEBUG_SCRIPT		//定義でデバッグ機能有効

//--------------------------------------------------------------
// SCRIPTのメンバーID定義
//--------------------------------------------------------------
enum{
	ID_EVSCR_EVWIN = 0,				//イベントウィンドウワークのポインタ

	ID_EVSCR_MSGWINDAT,				//会話ウィンドウビットマップデータ
	ID_EVSCR_MENUWORK,				//ビットマップメニューワークのポインタ

	ID_EVSCR_MSGINDEX,				//会話ウィンドウメッセージインデックス
	ID_EVSCR_ANMCOUNT,				//アニメーションの数
	ID_EVSCR_WIN_OPEN_FLAG,			//会話ウィンドウを開いたかフラグ

	ID_EVSCR_SCRIPT_ID,				//メインのスクリプトID

	//ID_EVSCR_PLAYER_DIR,			//イベント起動時の主人公の向き
	ID_EVSCR_TARGET_OBJ,			//話しかけ対象のOBJのポインタ
	ID_EVSCR_DUMMY_OBJ,				//透明ダミーのOBJのポインタ
	ID_EVSCR_RETURN_SCRIPT_WK,		//イベント結果を代入するワークのポインタ

	ID_EVSCR_WORDSET,				//単語セット
	ID_EVSCR_MSGBUF,				//メッセージバッファのポインタ
	ID_EVSCR_TMPBUF,				//テンポラリバッファのポインタ
	//ID_EVSCR_WAITICON,				///<待機アイコンのポインタ

	ID_EVSCR_SUBPROC_WORK,			//サブプロセスのパラメータ
	ID_EVSCR_PWORK,					//ワークへの汎用ポインタ
	//ID_EVSCR_EOA,					//フィールドエフェクトへのポインタ
	//ID_EVSCR_PLAYER_TCB,			//自機形態TCBのポインタ

	ID_EVSCR_WIN_FLAG,

  ID_EVSCR_TRAINER0,

  ID_EVSCR_TRAINER1,

	//コインウインドウ
	//ID_EVSCR_COINWINDAT,

	//お金ウインドウ
	//ID_EVSCR_GOLDWINDAT,

	//ID_EVSCR_REPORTWIN,		///<レポート情報ウィンドウ

	/*WB*/
	//GAMESYS_WORK
	ID_EVSCR_WK_GAMESYS_WORK,
	//HEAPID
	ID_EVSCR_WK_HEAPID,
	//SCRIPT_FLDPARAM
	ID_EVSCR_WK_FLDPARAM,
	//GMEVENT
  ID_EVSCR_WK_GMEVENT,
  //TEMP HEAPID
  ID_EVSCR_WK_TEMP_HEAPID,


	ID_EVSCR_WK_END,				//ワークの終了ID

	//↑↑↑↑↑ここはワークの数に影響するので注意！↑↑↑↑↑
};


//仮想マシンのナンバー
typedef enum{
	VMHANDLE_MAIN = 0,		//メイン
	VMHANDLE_SUB1,			//サブ
	VMHANDLE_MAX,			//最大数
}VMHANDLE_ID;

//========================================================================
//	struct
//========================================================================
//--------------------------------------------------------------
//	スクリプト制御ワークへの不完全型ポインタ
//--------------------------------------------------------------
typedef	struct _TAG_SCRIPT_WORK SCRIPT_WORK;

//--------------------------------------------------------------
///	SCRIPT_FLDPARAM スクリプト制御ワーク　フィールドパラメータ
//--------------------------------------------------------------
typedef struct
{
	FLDMSGBG *msgBG;
  FIELDMAP_WORK *fieldMap;
}SCRIPT_FLDPARAM;

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
		HEAPID temp_heapID, const SCRIPT_FLDPARAM *fparam );

extern void SCRIPT_SetTrainerEyeData( GMEVENT *event, MMDL *mmdl,
    s16 range, u16 dir, u16 scr_id, u16 tr_id, int tr_type, int tr_no );

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
extern void SCRIPT_CallScript( GMEVENT *event,
	u16 scr_id, MMDL *obj, void *ret_script_wk, HEAPID heapID );

//--------------------------------------------------------------
/**
 * 特殊スクリプト呼び出し
 * @param gsys
 * @param heapID
 * @param script_id
 */
//--------------------------------------------------------------
extern void SCRIPT_CallSpecialScript( GAMESYS_WORK *gsys, HEAPID heapID, u16 script_id );

//--------------------------------------------------------------
/**
 * スクリプトイベント切替
 * @param	event		GMEVENT_CONTROL型のポインタ
 * @param	scr_id		スクリプトID
 * @param	obj			話しかけ対象OBJのポインタ(ない時はNULL)
 * @retval	none
 * 他のイベントからスクリプトへの切替を行う
 */
//--------------------------------------------------------------
extern void SCRIPT_ChangeScript( GMEVENT *event,
		u16 scr_id, MMDL *obj, HEAPID heapID );

extern void SCRIPT_AddVMachine( SCRIPT_WORK *sc, u16 zone_id, u16 scr_id, VMHANDLE_ID vm_id );
extern BOOL SCRIPT_GetVMExists( SCRIPT_WORK *sc, VMHANDLE_ID vm_id );
//======================================================================
//	スクリプト制御ワークのメンバーアクセス
//======================================================================
//--------------------------------------------------------------
/**
 * スクリプト制御ワークのメンバーアドレス取得
 * @param	id		取得するメンバID(script.h参照)
 * @return	"アドレス"
 */
//--------------------------------------------------------------
extern void * SCRIPT_GetMemberWork( SCRIPT_WORK *sc, u32 id );

//--------------------------------------------------------------
/**
 * スクリプト制御ワークの"next_func"にメニュー呼び出しをセット
 * @return	none
 * フィールドメニュー呼び出し限定！
 * 流れが把握しずらくなるので、汎用的に使えないようにしている！
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
extern void SCRIPT_SetNextScript( FLDCOMMON_WORK* fsys );
#endif

extern GMEVENT * SCRIPT_GetEvent( SCRIPT_WORK * sc );
extern GAMESYS_WORK * SCRIPT_GetGameSysWork( SCRIPT_WORK * sc );

//======================================================================
//	
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	イベントワークアドレスを取得
 *
 * @param	work_no		ワークナンバー
 *
 * @return	"ワークのアドレス"
 *
 * @li	work_no < 0x8000	通常のセーブワーク
 * @li	work_no >= 0x8000	スクリプト制御ワークの中に確保しているワーク
 */
//------------------------------------------------------------------
extern u16 * SCRIPT_GetEventWork( SCRIPT_WORK *sc, GAMEDATA *gdata, u16 work_no );

//------------------------------------------------------------------
/**
 * @brief	イベントワークの値を取得
 *
 * @param	work_no		ワークナンバー
 *
 * @return	"ワークの値"
 */
//------------------------------------------------------------------
extern u16 SCRIPT_GetEventWorkValue( SCRIPT_WORK *sc, GAMEDATA *gdata, u16 work_no );

//------------------------------------------------------------------
/**
 * @brief	イベントワークの値をセット
 *
 * @param	work_no		ワークナンバー
 * @param	value		セットする値
 *
 * @return	"TRUE=セット出来た、FALSE=セット出来なかった"
 */
//------------------------------------------------------------------
extern BOOL SCRIPT_SetEventWorkValue(
		SCRIPT_WORK* sc, u16 work_no, u16 value );

//------------------------------------------------------------------
/**
 * @brief	スクリプトから指定するOBJコードを取得
 *
 * @param	no			0-15
 *
 * @return	"OBJキャラコード"
 */
//------------------------------------------------------------------
extern u16 GetEvDefineObjCode( SCRIPT_WORK * sc, u16 no );

//------------------------------------------------------------------
/**
 * @brief	スクリプトから指定するOBJコードをセット
 *
 * @param	no			0-15
 * @param	obj_code	OBJコード
 *
 * @return	"TRUE=セット出来た、FALSE=セット出来なかった"
 */
//------------------------------------------------------------------
extern BOOL SetEvDefineObjCode( SCRIPT_WORK * sc, u16 no, u16 obj_code );


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

//------------------------------------------------------------------
/**
 * @brief	ゲーム開始 スクリプト初期設定の実行
 */
//------------------------------------------------------------------
extern void SCRIPT_CallGameStartInitScript( GAMESYS_WORK *gsys, HEAPID heapID );

//------------------------------------------------------------------
/**
 * @brief	特殊スクリプト検索して実行
 *
 * @param	key			特殊スクリプトID
 *
 * @return	"TRUE=特殊スクリプト実行、FALSE=何もしない"
 */
//------------------------------------------------------------------
extern GMEVENT * SCRIPT_SearchSceneScript( GAMESYS_WORK * gsys, HEAPID heapID);
extern BOOL SCRIPT_CallFieldInitScript( GAMESYS_WORK * gsys, HEAPID heapID );
extern BOOL SCRIPT_CallFieldRecoverScript( GAMESYS_WORK * gsys, HEAPID heapID );




#endif	/* SCRIPT_H */


