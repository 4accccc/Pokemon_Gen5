//============================================================================================
/**
 * @file	script_work.c
 * @brief	スクリプトグローバルワーク定義
 * @date  2010.01.26
 * @author  tamada
 */
//============================================================================================
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_local.h"

#include "scrcmd_work.h"
#include "eventwork_def.h"

#include "field/zonedata.h"

#include "print/wordset.h"    //WORDSET

#include "trainer_eye_data.h"   //EV_TRAINER_EYE_HITDATA
#include "field/eventdata_system.h" //EVENTDATA_GetSpecialScriptData

#include "fieldmap.h"   //FIELDMAP_GetFldMsgBG
#include "battle/battle.h"

#include "musical/musical_event.h"  //MUSICAL_EVENT_WORK


//============================================================================================
//  define
//============================================================================================
enum
{
	WORDSET_SCRIPT_SETNUM = 32,		///<デフォルトバッファ数
	WORDSET_SCRIPT_BUFLEN = 64,		///<デフォルトバッファ長（文字数）
  SCR_MSG_BUF_SIZE	= (512),				///<メッセージバッファサイズ
  //SCR_MSG_BUF_SIZE =	(1024)				//メッセージバッファサイズ
};


#define SCRIPT_MAGIC_NO		(0x3643f)

#define SCRIPT_MSG_NON (0xffffffff)

#define SCRIPT_NOT_EXIST_ID (0xffff)

enum {
  TEMP_WORK_START = SCWK_AREA_START,
  TEMP_WORK_END = USERWK_AREA_END,
  TEMP_WORK_SIZE = TEMP_WORK_END - TEMP_WORK_START,

  USERWK_SIZE = USERWK_AREA_END - USERWK_AREA_START,
};

//============================================================================================
//	struct
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトグローバルワーク定義
 *
 * 各仮想マシンをまたいで存在するスクリプト用ワーク
 */
//--------------------------------------------------------------
struct _TAG_SCRIPT_WORK
{
  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
  //script.c でPrivateなメンバ
  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
	u32 magic_no;			//イベントのワークがスクリプト制御ワークかを判別
	
  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
  //SCRIPT_WORK生成時に引数で初期化され、ReadOnlyで外部公開されるメンバ
  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
	u16 start_scr_id;			//メインのスクリプトID
	MMDL *target_obj;

	HEAPID main_heapID;

	GAMESYS_WORK *gsys;
  GMEVENT *gmevent; //スクリプトを実行しているGMEVENT*
	SCRIPT_FLDPARAM fld_param;

	u16 *ret_script_wk;			//スクリプト結果を代入するワークのポインタ

  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
  //script.c内で使用しない、外部公開用メンバ
  //  public R/W
  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
	WORDSET* wordset;				//単語セット
	STRBUF* msg_buf;				//メッセージバッファポインタ
	STRBUF* tmp_buf;				//テンポラリバッファポインタ


  void *msgWin;           //メニュー系流用メッセージウィンドウ

  void *mw;				//ビットマップメニューワーク

	void * subproc_work; //サブプロセスとのやりとりに使用するワークへの*

	u8 anm_count;			//アニメーションしている数
	
  u8 sound_se_flag;   ///< サウンドSE再生チェックフラグ

	//トレーナー視線情報
	EV_TRAINER_EYE_HITDATA eye_hitdata[TRAINER_EYE_HITMAX];
	
	u16 scrTempWork[TEMP_WORK_SIZE];		//ワーク(ANSWORK,TMPWORKなどの代わり)

  MUSICAL_EVENT_WORK *musicalEventWork;

};

//============================================================================================
//	プロトタイプ宣言
//============================================================================================


//イベントワーク、スクリプトワークアクセス関数
static u16 * getTempWork( SCRIPT_WORK * sc, u16 work_no );

static void initFldParam(SCRIPT_FLDPARAM * fparam, GAMESYS_WORK * gsys);

//------------------------------------------------------------------
//------------------------------------------------------------------
static void initFldParam(SCRIPT_FLDPARAM * fparam, GAMESYS_WORK * gsys)
{
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  MI_CpuClear32( fparam, sizeof(SCRIPT_FLDPARAM) );
  fparam->fieldMap = fieldmap;
  if (fieldmap != NULL)
  {
    fparam->msgBG = FIELDMAP_GetFldMsgBG(fieldmap);
  }
}


//============================================================================================
//
//  スクリプトワーク
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプト用グローバルワークの生成
 * @param main_heapID
 * @param gsys
 * @param event
 * @param	scr_id		スクリプトID
 * @param	ret_wk		スクリプト結果を代入するワークのポインタ
 * @return	SCRIPT_WORK			SCRIPT型のポインタ
 *
 * スクリプトコマンド全般からグローバルアクセス可能なデータを保持する
 */
//--------------------------------------------------------------
SCRIPT_WORK * SCRIPTWORK_Create( HEAPID main_heapID,
    GAMESYS_WORK * gsys, GMEVENT * event, u16 scr_id, void* ret_wk)
{
  SCRIPT_WORK * sc;

	sc = GFL_HEAP_AllocClearMemory( main_heapID, sizeof(SCRIPT_WORK) );
	sc->magic_no = SCRIPT_MAGIC_NO;
	sc->main_heapID = main_heapID;

	sc->gsys = gsys;
  sc->gmevent = event;
	sc->start_scr_id  = scr_id;	//メインのスクリプトID
	sc->target_obj = NULL;
	sc->ret_script_wk = ret_wk;	//スクリプト結果を代入するワーク
  
  //メッセージ関連
  sc->wordset = WORDSET_CreateEx(
    WORDSET_SCRIPT_SETNUM, WORDSET_SCRIPT_BUFLEN, main_heapID );
  sc->msg_buf = GFL_STR_CreateBuffer( SCR_MSG_BUF_SIZE, main_heapID );
  sc->tmp_buf = GFL_STR_CreateBuffer( SCR_MSG_BUF_SIZE, main_heapID );

  //フィールドパラメータ生成
	initFldParam( &sc->fld_param, gsys );
  //ミュージカルの制御イベントで渡される
  sc->musicalEventWork = NULL;

  //チェックビット処理化
  SCREND_CHK_ClearBits();

  return sc;
}

//--------------------------------------------------------------
/**
 * @brief スクリプト用グローバルワークの削除
 */
//--------------------------------------------------------------
void SCRIPTWORK_Delete( SCRIPT_WORK * sc )
{
  sc->magic_no = 0;
  WORDSET_Delete( sc->wordset );
  GFL_STR_DeleteBuffer( sc->msg_buf );
  GFL_STR_DeleteBuffer( sc->tmp_buf );

  GFL_HEAP_FreeMemory( sc );
}

//============================================================================================
//	スクリプト制御ワークのメンバーアクセス
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * SCRIPT_GetEvent( SCRIPT_WORK * sc )
{
  GF_ASSERT(sc->magic_no == SCRIPT_MAGIC_NO);
  return sc->gmevent;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
GAMESYS_WORK * SCRIPT_GetGameSysWork( SCRIPT_WORK * sc )
{
  GF_ASSERT(sc->magic_no == SCRIPT_MAGIC_NO);
  return sc->gsys;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
SCRIPT_FLDPARAM * SCRIPT_GetFieldParam( SCRIPT_WORK * sc )
{
  FIELDMAP_WORK * fieldmap;
  GF_ASSERT( sc->magic_no == SCRIPT_MAGIC_NO );
  //スクリプトコマンド実行中にフィールドへの出入りがあると、
  //条件が変わってポインタが正しい値を指していない可能性がある。
  //なので、アクセスタイミングでの初期化処理を行う。
  initFldParam( &sc->fld_param, sc->gsys );
  GF_ASSERT( sc->fld_param.fieldMap != NULL );
  return &sc->fld_param;
}

//--------------------------------------------------------------
/**
 * スクリプト制御ワークのメッセージウィンドウポインタ取得
 * @param	sc		    SCRIPT型のポインタ
 * @param	msgWin		取得するメンバID(script.h参照)
 * @return	"アドレス"
 */
//--------------------------------------------------------------
void * SCRIPT_GetMsgWinPointer( SCRIPT_WORK *sc )
{
  return sc->msgWin;
}

//--------------------------------------------------------------
/**
 * スクリプト制御ワークのメッセージウィンドウポインタセット
 * @param	sc		    SCRIPT型のポインタ
 * @param	msgWin		取得するメンバID(script.h参照)
 * @return	"アドレス"
 */
//--------------------------------------------------------------
void SCRIPT_SetMsgWinPointer( SCRIPT_WORK *sc, void* msgWin )
{
  sc->msgWin = msgWin;
}

//--------------------------------------------------------------
/**
 * @brief 単語セット保持ポインタの取得
 * @param	sc		    SCRIPT_WORKのポインタ
 */
//--------------------------------------------------------------
WORDSET * SCRIPT_GetWordSet( SCRIPT_WORK * sc )
{
  return sc->wordset;
}
//--------------------------------------------------------------
/**
 * @brief MSGバッファの取得
 * @param	sc		    SCRIPT_WORKのポインタ
 */
//--------------------------------------------------------------
STRBUF * SCRIPT_GetMsgBuffer( SCRIPT_WORK * sc )
{
  return sc->msg_buf;
}

//--------------------------------------------------------------
/**
 * @brief MSG展開用テンポラリバッファの取得
 * @param	sc		    SCRIPT_WORKのポインタ
 */
//--------------------------------------------------------------
STRBUF * SCRIPT_GetMsgTempBuffer( SCRIPT_WORK * sc )
{
  return sc->tmp_buf;
}
//--------------------------------------------------------------
/**
 * @brief SE制御用フラグの取得
 */
//--------------------------------------------------------------
u8 * SCRIPT_GetSoundSeFlag( SCRIPT_WORK * sc )
{
  return &sc->sound_se_flag;
}
//--------------------------------------------------------------
/**
 * @brief スクリプト変数：アニメカウントの取得
 */
//--------------------------------------------------------------
u8 * SCRIPT_GetAnimeCount( SCRIPT_WORK * sc )
{
  return &sc->anm_count;
}
//--------------------------------------------------------------
/**
 * @brief   開始時のスクリプトID取得
 * @param	sc		    SCRIPT_WORKのポインタ
 * @return  u16   スクリプトID
 */
//--------------------------------------------------------------
u16 SCRIPT_GetStartScriptID( const SCRIPT_WORK * sc )
{
  return sc->start_scr_id;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
void * SCRIPT_GetFLDMENUFUNC( SCRIPT_WORK * sc )
{
  return sc->mw;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
void SCRIPT_SetFLDMENUFUNC( SCRIPT_WORK * sc, void * mw )
{
  sc->mw = mw;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
MMDL * SCRIPT_GetTargetObj( SCRIPT_WORK * sc )
{
  return sc->target_obj;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
void SCRIPT_SetTargetObj( SCRIPT_WORK * sc, MMDL * obj )
{
	sc->target_obj = obj;		//話しかけ対象OBJのポインタセット
	if( obj != NULL ){
    u16 *objid;
    objid = getTempWork( sc, SCWK_TARGET_OBJID );
		*objid = MMDL_GetOBJID( obj ); //話しかけ対象OBJIDのセット
	}
}
//--------------------------------------------------------------
/**
 * スクリプト制御ワークのメンバーアドレス設定(ミュージカル
 * @param	sc	  SCRIPT_WORK型のポインタ
 */
//--------------------------------------------------------------
void SCRIPT_SetMemberWork_Musical( SCRIPT_WORK *sc, void *musEveWork )
{
	if( sc->magic_no != SCRIPT_MAGIC_NO ){
		GF_ASSERT_MSG(0, "起動(確保)していないスクリプトのワークにアクセスしています！" );
	}
	sc->musicalEventWork = musEveWork;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
void * SCRIPT_GetMemberWork_Musical( SCRIPT_WORK * sc )
{
  GF_ASSERT_MSG( sc->musicalEventWork != NULL , "ミュージカルワークがNULL！" );
  return sc->musicalEventWork;
}


//--------------------------------------------------------------
/**
 * サブプロセス用ワークのポインターアドレスを取得
 * @param	sc SCRIPT_WORK
 * @retval none
 *
 * 使い終わったら必ずNULLクリアすること！
 */
//--------------------------------------------------------------
void** SCRIPT_SetSubProcWorkPointerAdrs( SCRIPT_WORK *sc )
{
  return &sc->subproc_work;
}

//--------------------------------------------------------------
/**
 * サブプロセス用ワークのポインターを取得
 * @param	sc SCRIPT_WORK
 * @retval none
 *
 * 使い終わったら必ずNULLクリアすること！
 */
//--------------------------------------------------------------
void * SCRIPT_SetSubProcWorkPointer( SCRIPT_WORK *sc )
{
  return sc->subproc_work;
}

//--------------------------------------------------------------
/**
 * サブプロセス用ワーク領域の解放(ポインタがNULLでなければFree)
 * @param	sc SCRIPT_WORK
 * @retval none
 */
//--------------------------------------------------------------
void SCRIPT_FreeSubProcWorkPointer( SCRIPT_WORK *sc )
{
  if(sc->subproc_work != NULL){
    GFL_HEAP_FreeMemory(sc->subproc_work);
    sc->subproc_work = NULL;
  }
}



//============================================================================================
//	スクリプトワーク・フラグ関連
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static u16 * getTempWork( SCRIPT_WORK * sc, u16 work_no )
{
  GF_ASSERT( work_no >= TEMP_WORK_START );
  GF_ASSERT( work_no < TEMP_WORK_END );
  return &(sc->scrTempWork[ work_no - TEMP_WORK_START ]);
}
//------------------------------------------------------------------
/**
 * @brief	イベントワークアドレスを取得
 * @param	ev			イベントワークへのポインタ
 * @param	work_no		ワークナンバー
 *
 * @return	"ワークのアドレス"
 *
 * @li	work_no < 0x8000	通常のセーブワーク
 * @li	work_no >= 0x8000	スクリプト制御ワークの中に確保しているワーク
 */
//------------------------------------------------------------------
u16 * SCRIPT_GetEventWork( SCRIPT_WORK *sc, GAMEDATA *gdata, u16 work_no )
{
	EVENTWORK *ev;
	ev = GAMEDATA_GetEventWork( gdata );
	
	if( work_no < SVWK_START ){
		return NULL;
	}
	
	if( work_no < SCWK_START ){
		return EVENTWORK_GetEventWorkAdrs(ev,work_no);
	}
	
	//スクリプト制御ワークの中で、ANSWORKなどのワークを確保しています
  return getTempWork( sc, work_no );
}

//------------------------------------------------------------------
/**
 * @brief	イベントワークの値を取得
 *
 * @param	work_no		ワークナンバー
 *
 * @return	"ワークの値"
 */
//------------------------------------------------------------------
u16 SCRIPT_GetEventWorkValue( SCRIPT_WORK *sc, GAMEDATA *gdata, u16 work_no )
{
	u16 *res = SCRIPT_GetEventWork( sc, gdata, work_no );
	if( res == NULL ){ return work_no; }
	return *res;
}

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
BOOL SCRIPT_SetEventWorkValue(
	SCRIPT_WORK *sc, u16 work_no, u16 value )
{
  GAMEDATA * gdata;
	u16* res;
  GF_ASSERT(sc->magic_no == SCRIPT_MAGIC_NO);
  gdata = GAMESYSTEM_GetGameData( sc->gsys );
	res = SCRIPT_GetEventWork( sc, gdata, work_no );
	if( res == NULL ){ return FALSE; }
	*res = value;
	return TRUE;
}
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
void SCRIPT_SetScriptWorkParam( SCRIPT_WORK *sc, u16 prm0, u16 prm1, u16 prm2, u16 prm3 )
{
  GF_ASSERT(sc->magic_no == SCRIPT_MAGIC_NO);
  SCRIPT_SetEventWorkValue( sc, SCWK_PARAM0, prm0 );
  SCRIPT_SetEventWorkValue( sc, SCWK_PARAM1, prm1 );
  SCRIPT_SetEventWorkValue( sc, SCWK_PARAM2, prm2 );
  SCRIPT_SetEventWorkValue( sc, SCWK_PARAM3, prm3 );
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void * SCRIPT_BackupUserWork( SCRIPT_WORK * sc )
{
  u16 * backup_work = GFL_HEAP_AllocClearMemory( sc->main_heapID, sizeof(u16) * USERWK_SIZE );
  GFL_STD_MemCopy16( getTempWork( sc, USERWK_AREA_START ), backup_work, sizeof(u16) * USERWK_SIZE );
  return backup_work;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void SCRIPT_RestoreUserWork( SCRIPT_WORK * sc, void *backup_work )
{
  GFL_STD_MemCopy16( backup_work, getTempWork( sc, USERWK_AREA_START ), sizeof(u16) * USERWK_SIZE );
}

//============================================================================================
//
//	トレーナーフラグ関連
//	・スクリプトIDから、トレーナーIDを取得して、フラグチェック
//
//============================================================================================
//ID_TRAINER_2VS2_OFFSET, ID_TRAINER_OFFSET
#include "../../../resource/fldmapdata/script/scrid_offset/scr_offset_id.h"

#define GET_TRAINER_FLAG(id)	( TR_FLAG_START+(id) )

//--------------------------------------------------------------
/**
 * トレーナー視線情報を格納 事前にSCRIPT_SetEventScript()を起動しておく事
 * @param	event SCRIPT_SetEventScript()戻り値。
 * @param	mmdl 視線がヒットしたFIELD_OBJ_PTR
 * @param	range		グリッド単位の視線距離
 * @param	dir			移動方向
 * @param	scr_id		視線ヒットしたスクリプトID
 * @param	tr_id		視線ヒットしたトレーナーID
 * @param	tr_type		トレーナータイプ　シングル、ダブル、タッグ識別
 * @param	tr_no		何番目にヒットしたトレーナーなのか
 */
//--------------------------------------------------------------
void SCRIPT_SetTrainerEyeData( GMEVENT *event, MMDL *mmdl,
    s16 range, u16 dir, u16 scr_id, u16 tr_id, int tr_type, int tr_no )
{
  SCRIPT_WORK *sc =  SCRIPT_GetScriptWorkFromEvent( event );
	EV_TRAINER_EYE_HITDATA *eye = &sc->eye_hitdata[tr_no];
  GF_ASSERT( tr_no < TRAINER_EYE_HITMAX );
  TRAINER_EYE_HITDATA_Set(eye, mmdl, range, dir, scr_id, tr_id, tr_type, tr_no );
}

//--------------------------------------------------------------
/**
 * トレーナー視線情報の取得
 */
//--------------------------------------------------------------
void * SCRIPT_GetTrainerEyeData( SCRIPT_WORK * sc, u32 tr_no )
{
  GF_ASSERT( tr_no < TRAINER_EYE_HITMAX );
  return &sc->eye_hitdata[tr_no];
}


//--------------------------------------------------------------
/**
 * スクリプトIDから、トレーナーIDを取得
 *
 * @param   scr_id		スクリプトID
 *
 * @retval  "トレーナーID = フラグインデックス"
 */
//--------------------------------------------------------------
u16 SCRIPT_GetTrainerID_ByScriptID( u16 scr_id )
{
#if 0 //pl
	if( scr_id < ID_TRAINER_2VS2_OFFSET ){
		return (scr_id - ID_TRAINER_OFFSET + 1);		//1オリジン
	}else{
		return (scr_id - ID_TRAINER_2VS2_OFFSET + 1);		//1オリジン
	}
#else
	if( scr_id < ID_TRAINER_2VS2_OFFSET ){
		return (scr_id - ID_TRAINER_OFFSET);		//0オリジン
	}else{
		return (scr_id - ID_TRAINER_2VS2_OFFSET);		//0オリジン
	}
#endif
}

//--------------------------------------------------------------
/**
 * スクリプトIDから、左右どちらのトレーナーか取得
 * @param   scr_id		スクリプトID
 * @retval  "0=左、1=右"
 */
//--------------------------------------------------------------
BOOL SCRIPT_GetTrainerLR_ByScriptID( u16 scr_id )
{
	if( scr_id < ID_TRAINER_2VS2_OFFSET ){
		return 0;
	}else{
		return 1;
	}
}

//--------------------------------------------------------------
/**
 * トレーナーIDから、ダブルバトルタイプか取得
 * @param   tr_id		トレーナーID
 * @retval  "0=シングルバトル、1=ダブルバトル"
 */
//--------------------------------------------------------------
BOOL SCRIPT_CheckTrainer2vs2Type( u16 tr_id )
{
  u32 rule = TT_TrainerDataParaGet( tr_id, ID_TD_fight_type );
  
  //OS_Printf( "check 2v2 TRID=%d, type = %d\n", tr_id, rule );

  switch( rule ){
  case BTL_RULE_SINGLE:
    return( 0 );
  case BTL_RULE_DOUBLE:
  case BTL_RULE_TRIPLE: //kari
    return( 1 );
  }
  
//  GF_ASSERT( 0 );
  return( 1 );
}

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをチェックする
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @param	tr_id		トレーナーID
 * @retval	"1 = フラグON"
 * @retval	"0 = フラグOFF"
 */
//------------------------------------------------------------------
BOOL SCRIPT_CheckEventFlagTrainer( EVENTWORK *ev, u16 tr_id )
{
  return EVENTWORK_CheckEventFlag(ev,GET_TRAINER_FLAG(tr_id) );
}

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをセットする
 *
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @param	tr_id		トレーナーID
 *
 * @return	none
 */
//------------------------------------------------------------------
void SCRIPT_SetEventFlagTrainer( EVENTWORK *ev, u16 tr_id )
{
  EVENTWORK_SetEventFlag( ev, GET_TRAINER_FLAG(tr_id) );
}

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをリセットする
 *
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @param	tr_id		トレーナーID
 *
 * @return	none
 */
//------------------------------------------------------------------
void SCRIPT_ResetEventFlagTrainer( EVENTWORK *ev, u16 tr_id )
{
	EVENTWORK_ResetEventFlag( ev, GET_TRAINER_FLAG(tr_id) );
}


