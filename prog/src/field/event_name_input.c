///////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief 名前入力イベント
 * @file  event_name_input.c
 * @author obata
 * @date   2009.10.20
 *
 */
///////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "fieldmap.h"
#include "event_name_input.h"
#include "app/name_input.h"
#include "event_fieldmap_control.h" // for EVENT_FieldSubProc
#include "system/main.h" // for HEAPID_PROC


//=====================================================================================
// ■非公開関数のプロトタイプ宣言
//=====================================================================================
static GMEVENT * EVENT_NameInput( 
    GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, NAMEIN_PARAM* namein_param );
static GMEVENT_RESULT EVENT_FUNC_NameInput( GMEVENT* event, int* seq, void* wk ); 
static GMEVENT_RESULT EVENT_FUNC_NameInput_PartyPoke(
    GMEVENT* event, int* seq, void* wk );


//=====================================================================================
// ■名前入力画面呼び出しイベント
//===================================================================================== 

//-------------------------------------------------------------------------------------
// ■イベントワーク
//-------------------------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK*  gameSystem;
	FIELDMAP_WORK* fieldmap;
  NAMEIN_PARAM*  nameInParam; // 名前入力画面のパラメータ
} NAMEIN_WORK; 


//-------------------------------------------------------------------------------------
/**
 * @brief イベント ( 名前入力画面呼び出し )
 * 
 * @param gameSystem
 * @param ret_wk      入力結果を受け取るワーク(TRUE:確定, FALSE:キャンセル)
 * @param party_index 名前を入力するポケモンのパーティー内インデックス
 *
 * @return イベント
 */
//-------------------------------------------------------------------------------------
static GMEVENT* EVENT_NameInput( 
    GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, NAMEIN_PARAM* namein_param )
{
	GMEVENT* event;
	NAMEIN_WORK* niw;

  // イベント生成
  event = GMEVENT_Create( gameSystem, NULL, EVENT_FUNC_NameInput, sizeof(NAMEIN_WORK) );

  // イベントワーク初期化
  niw = GMEVENT_GetEventWork( event );
	niw->gameSystem = gameSystem;
	niw->fieldmap = fieldmap;
  niw->nameInParam = namein_param;
	return event;
}

//-------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数(名前入力画面呼び出しイベント)
 */
//-------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_NameInput(GMEVENT * event, int * seq, void * wk)
{
	NAMEIN_WORK*   niw = wk;
	GAMESYS_WORK* gameSystem = niw->gameSystem;

	switch( *seq ) 
  {
	case 0: // 名前入力画面呼び出し
    GMEVENT_CallEvent( 
        event, EVENT_FieldSubProc(niw->gameSystem, niw->fieldmap, 
          FS_OVERLAY_ID(namein), &NameInputProcData, niw->nameInParam) );
    (*seq)++;
		break;
	case 1: // イベント終了
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
}


//=====================================================================================
// ■手持ちポケモン名入力画面呼び出しイベント
//===================================================================================== 

//-------------------------------------------------------------------------------------
// ■イベントワーク
//-------------------------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK*  gameSystem;  // ゲームシステム 
	FIELDMAP_WORK* fieldmap;    // フィールドマップ
  NAMEIN_PARAM*  nameInParam; // 名前入力画面のパラメータ
  POKEMON_PARAM* pokeParam;   // 名前入力対象ポケモン
  u16*           retWork;     // 結果の格納先ワークへのポインタ
} PARTY_NAMEIN_WORK; 

//-------------------------------------------------------------------------------------
/**
 * @brief イベント(手持ちポケモン名入力画面呼び出し)
 * 
 * @param gameSystem
 * @param ret_wk      入力結果を受け取るワーク(TRUE:確定, FALSE:キャンセル)
 * @param party_index 名前を入力するポケモンのパーティー内インデックス
 *
 * @return イベント
 *          ポケモン名入力画面を呼び出し, 指定した手持ちポケモン名を設定する。
 *          名前入力が確定されたかどうかを指定したワークに返す。
 *          TRUE:  入力した名前を確定
 *          FALSE: 名前入力をキャンセル
 */
//-------------------------------------------------------------------------------------
GMEVENT* EVENT_NameInput_PartyPoke( GAMESYS_WORK* gameSystem, u16* ret_wk, u16 party_index )
{
  GMEVENT* event;
  PARTY_NAMEIN_WORK* work;

  // インデックスを検査
  {
    GAMEDATA*  gdata = GAMESYSTEM_GetGameData( gameSystem );
    POKEPARTY* party = GAMEDATA_GetMyPokemon( gdata );
    int  party_count = PokeParty_GetPokeCount( party );
    if( (party_index < 0) || (party_count <= party_index) ) {
      OS_Printf( "名前入力コマンド: インデックス指定に誤りがあります。\n" );
      GF_ASSERT(0);
      party_index = 0;
    }
  }

  // イベント生成
  event = GMEVENT_Create( 
      gameSystem, NULL, EVENT_FUNC_NameInput_PartyPoke, sizeof(PARTY_NAMEIN_WORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );
  work->retWork    = ret_wk;

  // ポケモン名入力画面を作成
  {
    GAMEDATA*   gdata = GAMESYSTEM_GetGameData( work->gameSystem );
    POKEPARTY*  party = GAMEDATA_GetMyPokemon( gdata );
    POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, party_index );
    MISC        *misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork(gdata) );
    u32        monsno = PP_Get( pp, ID_PARA_monsno, NULL );
    u32        formno = PP_Get( pp, ID_PARA_form_no, NULL ) | ( PP_Get( pp, ID_PARA_sex, NULL ) << 8 );
    GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
    work->nameInParam = NAMEIN_AllocParam( 
        HEAPID_PROC, NAMEIN_POKEMON, monsno, formno, NAMEIN_POKEMON_LENGTH, NULL, misc );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );
    work->pokeParam   = pp;
  }

  // 作成したイベントを返す
  return event;
}

//-------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数(手持ちポケモン名入力画面呼び出しイベント)
 */
//-------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_NameInput_PartyPoke( GMEVENT* event, int* seq, void* wk )
{
  PARTY_NAMEIN_WORK* pniw = wk;

  switch( *seq )
  {
  case 0: //----------------------------------------------- 共通イベント呼び出し
    GMEVENT_CallEvent( 
        event, EVENT_NameInput(pniw->gameSystem, pniw->fieldmap, pniw->nameInParam) );
    ++(*seq);
    break;
  case 1: //----------------------------------------------- イベント終了
    // 入力された名前を反映させる
    if( pniw->nameInParam->cancel != TRUE ) { 
      PP_Put( pniw->pokeParam, ID_PARA_nickname, (u32)pniw->nameInParam->strbuf );
    }
    // 入力結果をワークに返す
    *(pniw->retWork) = (pniw->nameInParam->cancel != TRUE);
    GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
    NAMEIN_FreeParam( pniw->nameInParam );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
