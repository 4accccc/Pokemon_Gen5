//======================================================================
/**
 * @file	script_local.h
 * @brief	スクリプト制御：
 * @date  2009.09.13
 * @author  tamada GAMEFREAK inc.
 *
 * script.hから、スクリプトシステムやコマンド以外に公開する
 * 必然性がないものを移動した
 */
//======================================================================
#pragma once

#include "field/fieldmap_proc.h"
#include "field/field_msgbg.h"

#include "scrend_check_bit.h"
#include "scrcmd_work.h"      //for SCRCMD_WORK

//--------------------------------------------------------------
///	SCRIPT_FLDPARAM スクリプト制御ワーク　フィールドパラメータ
//--------------------------------------------------------------
typedef struct
{
	FLDMSGBG *msgBG;
  FIELDMAP_WORK *fieldMap;
}SCRIPT_FLDPARAM;

typedef struct
{
  GAMESYS_WORK *gsys;
  SCRCMD_WORK   *ScrCmdWk;
  SCRIPT_WORK *ScrWk;
}SCREND_CHECK;

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

	//ID_EVSCR_SCRIPT_ID,				//メインのスクリプトID

	//ID_EVSCR_PLAYER_DIR,			//イベント起動時の主人公の向き
	ID_EVSCR_TARGET_OBJ,			//話しかけ対象のOBJのポインタ
	ID_EVSCR_DUMMY_OBJ,				//透明ダミーのOBJのポインタ
	ID_EVSCR_RETURN_SCRIPT_WK,		//イベント結果を代入するワークのポインタ

	//ID_EVSCR_WORDSET,				//単語セット
	ID_EVSCR_MSGBUF,				//メッセージバッファのポインタ
	ID_EVSCR_TMPBUF,				//テンポラリバッファのポインタ
	//ID_EVSCR_WAITICON,				///<待機アイコンのポインタ

	//ID_EVSCR_SUBPROC_WORK,			//サブプロセスのパラメータ
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
	//ID_EVSCR_WK_GAMESYS_WORK,
	//HEAPID
	ID_EVSCR_WK_HEAPID,
	//SCRIPT_FLDPARAM
	//ID_EVSCR_WK_FLDPARAM,
	//GMEVENT
  //ID_EVSCR_WK_GMEVENT,
  //TEMP HEAPID
  ID_EVSCR_WK_TEMP_HEAPID,

  ID_EVSCR_MUSICAL_EVENT_WORK,  //ミュージカルワーク(控え室のみ有効

  ID_EVSCR_SOUND_SE_FLAG,  //SOUND SE管理ワーク

	ID_EVSCR_WK_END,				//ワークの終了ID

	//↑↑↑↑↑ここはワークの数に影響するので注意！↑↑↑↑↑
};


//仮想マシンのナンバー
typedef enum{
	VMHANDLE_MAIN = 0,		//メイン
	VMHANDLE_SUB1,			//サブ
	VMHANDLE_MAX,			//最大数
}VMHANDLE_ID;

//======================================================================
//
//
//  スクリプト内部制御・コマンドから呼ばれる関数
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトシステム：他のイベント呼び出し
 * @param sc
 * @param call_event  呼び出すイベントへのポインタ
 *
 * スクリプトコマンドから使用する。
 * 指定したイベントが終了した後は、現在のスクリプトに復帰し実行される。
 *
 */
//--------------------------------------------------------------
extern void SCRIPT_CallEvent( SCRIPT_WORK *sc, GMEVENT * call_event );

//--------------------------------------------------------------
/**
 * @brief スクリプトシステム：スクリプト終了→他のイベントへ切り替え
 * @param sc
 * @param next_event  スクリプト終了後に稼動するイベントへのポインタ
 *
 * スクリプトコマンドから使用する。
 * スクリプトが終了した後、指定したイベントに切り替わる。
 */
//--------------------------------------------------------------
extern void SCRIPT_EntryNextEvent( SCRIPT_WORK *sc, GMEVENT * next_event );

//--------------------------------------------------------------
/**
 * @brief スクリプトシステム：仮想マシンの追加
 * @param sc
 * @param zone_id 起動するゾーン指定ID
 * @param scr_id  起動するスクリプト指定ID
 * @param vm_id   仮想マシン指定ID
 *
 *  主にスクリプトコマンドから使用する。
 *  スクリプトを実行する仮想マシンを生成する。
 */
//--------------------------------------------------------------
extern void SCRIPT_AddVMachine( SCRIPT_WORK *sc, u16 zone_id, u16 scr_id, VMHANDLE_ID vm_id );

//--------------------------------------------------------------
/**
 * @brief スクリプトシステム：仮想マシンの存在チェック
 * @param sc
 * @param vm_id   仮想マシン指定ID
 * @retval  TRUE    指定したIDの仮想マシンが存在する
 * @retval  FALSE   指定したIDの仮想マシンは存在しない
 */
//--------------------------------------------------------------
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


extern GMEVENT * SCRIPT_GetEvent( SCRIPT_WORK * sc );
extern GAMESYS_WORK * SCRIPT_GetGameSysWork( SCRIPT_WORK * sc );
extern SCRIPT_FLDPARAM * SCRIPT_GetFieldParam( SCRIPT_WORK * sc );
extern void * SCRIPT_GetMsgWinPointer( SCRIPT_WORK *sc );
extern void SCRIPT_SetMsgWinPointer( SCRIPT_WORK *sc, void* msgWin );
extern WORDSET * SCRIPT_GetWordSet( SCRIPT_WORK * sc );

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

//--------------------------------------------------------------
/**
 * スクリプト制御ワークのメンバーアドレス設定(ミュージカル
 * @param	sc	  SCRIPT_WORK型のポインタ
 */
//--------------------------------------------------------------
extern void SCRIPT_SetMemberWork_Musical( SCRIPT_WORK *sc, void *musEveWork );

//--------------------------------------------------------------
/**
 * @brief   開始時のスクリプトID取得
 * @param	sc		    SCRIPT_WORKのポインタ
 * @return  u16   スクリプトID
 */
//--------------------------------------------------------------
extern u16 SCRIPT_GetStartScriptID( const SCRIPT_WORK * sc );

