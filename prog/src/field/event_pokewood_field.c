//============================================================================================
/**
 *	@file		event_pokewood_field.c
 *	@brief  ポケウッド用イベント、フィールド常駐
 *	@author	tamada  GAME FREAK inc.
 *  @date   2011.11.23
 *
 * 画面暗転などがないのでフィールド常駐に存在すればよいポケウッド用のイベント
 */
//============================================================================================

#include "gflib.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"
#include "system/main.h"


#include "fieldmap.h"

#include "fieldmap_func.h"

#include "event_pokewood.h"

//#include "arc/message.naix"

//#include "app/pkwd_book.h"  //PKWD_BOOK_PARAM
//#include "pokewood_setup.h" //PKWD_BOOK_PARAM_Create
//#include "app/pkwd_record_monitor.h"  //PKWD_RECORD_MONITOR

#include "field/pokewood.h"
#include "pokewood_system.h"
#include "savedata/pokewood_rec.h"
#include "savedata/pokewood_savedata.h"
//#include "pkwd_scenario_open.h"

//#include "msg/msg_pw_movietitle.h"

//#include "../battle/pokewood/pokewood_senario.h"
//#include "tr_tool/trtype_def.h"
//#include "pkwd_scenario_open.h"


//============================================================================================
//  PKWD　イベント　外部データの録画データとゲーム進行データの一括セーブ
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief 撮影セーブ時のパラメータ更新処理
 */
//--------------------------------------------------------------
static void updatePokewoodSaveParams(
    PKWDSYS * pkwdsys, GAMEDATA * gamedata, POKEWOOD_SAVEDATA * pkwdsave, PKWD_REC_POS rec_pos )
{
  // 上映フラグをクリア
  POKEWOOD_SAVEDATA_SetMovieShowed( pkwdsave, rec_pos, FALSE );

  // 看板を設定
  PKWDSYS_SetKanban( gamedata, pkwdsys, rec_pos );

  // 興行収入のレコード登録
  PKWDSYS_RecordSet( gamedata, pkwdsys );

  // メダル取得計算
  PKWDSYS_CheckPokewoodMedal( gamedata );

  // 成績モニター登録処理
  PKWDSYS_BoxofficeRecordSet( gamedata, pkwdsys );

  // この2つの関数の順番は入れ替えないこと！(スターポケモンが登録されるのが早くなる)
  {
    // シナリオ単位で興行成績データが上回っていれば更新
    PKWDSYS_RecordSet( gamedata, pkwdsys );

    // 手持ちポケモンを出していた場合はスターポイントを加算
    PKWDSYS_CalcMyPokeStarPoint( gamedata, pkwdsys );
  }
}

//--------------------------------------------------------------
/// ポケウッドセーブイベント制御ワーク
//--------------------------------------------------------------
typedef struct {
  GAMEDATA * gamedata;
  PKWDSYS * pkwdsys;
  PKWD_REC_POS dst_pos;
  u16 save_tmp;
  u16 * ret_wk;
}EVENT_POKEWOOD_SAVE_WORK, EPSW;

//--------------------------------------------------------------
/**
 * @brief　ポケウッド録画データセーブイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT eventPokewoodSave( GMEVENT * event, int *seq, void * wk )
{
  typedef enum {
    PKWD_SAVE_SEQ_INIT = 0,
    PKWD_SAVE_SEQ_SAVE,
    PKWD_SAVE_SEQ_END,
  }PKWD_SAVE_SEQ;

  EPSW * epsw = (EPSW *)wk;

  SAVE_RESULT save_res;
  HEAPID heapID = HEAPID_FIELDMAP;
  POKEWOOD_SAVEDATA * pkwdsave = GAMEDATA_GetPokewoodSavedata( epsw->gamedata );


  switch ((PKWD_SAVE_SEQ)*seq)
  {
  case PKWD_SAVE_SEQ_INIT:
    OS_Printf("POKEWOOD Save Start(%d)\n", epsw->dst_pos );
    //看板などのセーブデータを更新する
    updatePokewoodSaveParams( epsw->pkwdsys, epsw->gamedata, pkwdsave, epsw->dst_pos );
    POKEWOOD_REC_Init( heapID );
    POKEWOOD_REC_Restore( PKWDSYS_GetBackupArea( epsw->pkwdsys ) );
    *seq = PKWD_SAVE_SEQ_SAVE;
    /* FALL THROUGH */

  case PKWD_SAVE_SEQ_SAVE:
    save_res = POKEWOOD_REC_Save( epsw->gamedata, heapID, epsw->dst_pos, &epsw->save_tmp );
    switch ( save_res )
    {
    case SAVE_RESULT_NG:
      if ( epsw->ret_wk )
      {
        *(epsw->ret_wk) = FALSE;
      }
      (*seq) = PKWD_SAVE_SEQ_END;
      break;
    case SAVE_RESULT_OK:
      if ( epsw->ret_wk )
      {
        *(epsw->ret_wk) = TRUE;
      }
      (*seq) = PKWD_SAVE_SEQ_END;
      break;
    }
    break;

  case PKWD_SAVE_SEQ_END:
    POKEWOOD_REC_Exit();
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/** 
 * @brief ポケウッド録画データセーブイベントの生成
 * @param   gsys      GAMESYS_WORK
 * @param   ret_wk    結果を受け取るためのワークへのポインタ
 * @return  GMEVENT   生成したイベント
 */
//--------------------------------------------------------------
GMEVENT * PKWD_EVENT_Save( GAMESYS_WORK * gsys, u16 * ret_wk )
{
  GMEVENT * event;
  EPSW * epsw;
  event = GMEVENT_Create( gsys, NULL, eventPokewoodSave, sizeof(EPSW) );
  epsw = GMEVENT_GetEventWork( event );
  epsw->gamedata = GAMESYSTEM_GetGameData( gsys );
  epsw->pkwdsys = * ( GAMEDATA_GetPKWDSysPointer( epsw->gamedata ) );
  epsw->dst_pos = PKWDSYS_GetPlayNo( epsw->pkwdsys );
  epsw->save_tmp = 0;
  epsw->ret_wk = ret_wk;
  return event;
}


