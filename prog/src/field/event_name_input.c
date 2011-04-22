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
#include "savedata/resonance_resort_sv.h"
#include "savedata/etc_save.h"


//=====================================================================================
// ■非公開関数のプロトタイプ宣言
//=====================================================================================
static GMEVENT * EVENT_NameInput( GAMESYS_WORK* gameSystem,
    FIELDMAP_WORK* fieldmap, NAMEIN_PARAM* namein_param, BOOL fade_flag );
static GMEVENT_RESULT EVENT_FUNC_NameInput( GMEVENT* event, int* seq, void* wk ); 
static GMEVENT_RESULT EVENT_FUNC_NameInput_PartyPoke(
    GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_NameInput_Resort(
    GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_NameInput_Rival(
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
  BOOL           fade_flag;   // アプリ呼び出し前後にフェード処理をおこなうかどうか
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
    GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, NAMEIN_PARAM* namein_param, BOOL fade_flag )
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
  niw->fade_flag = fade_flag;
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
    if (niw->fade_flag) {
      GMEVENT_CallEvent( 
          event, EVENT_FieldSubProc(niw->gameSystem, niw->fieldmap, 
            FS_OVERLAY_ID(namein), &NameInputProcData, niw->nameInParam) );
    } else {
      GMEVENT_CallEvent( 
          event, EVENT_FieldSubProcNoFade(niw->gameSystem, niw->fieldmap, 
            FS_OVERLAY_ID(namein), &NameInputProcData, niw->nameInParam) );
    }
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
  BOOL           fade_flag;   // アプリ呼び出し前後のフェード有り無し
  STRBUF         *p_default_str;  // デフォルト名 update ichiraku
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
GMEVENT* EVENT_NameInput_PartyPoke(
    GAMESYS_WORK* gameSystem, u16* ret_wk, u16 party_index, BOOL fade_flag )
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
  work->fade_flag  = fade_flag;

  // ポケモン名入力画面を作成
  {
    GAMEDATA*   gdata = GAMESYSTEM_GetGameData( work->gameSystem );
    POKEPARTY*  party = GAMEDATA_GetMyPokemon( gdata );
    POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, party_index );
    MISC        *misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork(gdata) );
    u32        monsno = PP_Get( pp, ID_PARA_monsno, NULL );
    u32        formno = PP_Get( pp, ID_PARA_form_no, NULL ) | ( PP_Get( pp, ID_PARA_sex, NULL ) << 8 );

    // >> update ichiraku 2010/10/19
    // ポケモンのデフォルト名取得追加
    work->p_default_str = GFL_STR_CreateBuffer( 32, HEAPID_PROC );
    PP_Get( pp, ID_PARA_nickname, work->p_default_str );
    // <<

    work->nameInParam = NAMEIN_AllocParam( 
        HEAPID_PROC, NAMEIN_POKEMON, monsno, formno, NAMEIN_POKEMON_LENGTH, NULL, misc );
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
    GMEVENT_CallEvent( event,
        EVENT_NameInput(pniw->gameSystem, pniw->fieldmap, pniw->nameInParam, pniw->fade_flag) );
    ++(*seq);
    break;
  case 1: //----------------------------------------------- イベント終了
    // 入力された名前を反映させる
    if( pniw->nameInParam->cancel != TRUE ) { 
      // update ichiraku 2010/10/19
      // 自分で比較する
      if( GFL_STR_CompareBuffer( pniw->nameInParam->strbuf, pniw->p_default_str ) == FALSE )
      {
        PP_Put( pniw->pokeParam, ID_PARA_nickname, (u32)pniw->nameInParam->strbuf );
      }
      else
      {
        pniw->nameInParam->cancel = TRUE;
        ICHI_Printf("入力前と同じ名前だった\n");
      }
    }
    // 入力結果をワークに返す
    *(pniw->retWork) = (pniw->nameInParam->cancel != TRUE);
    if( *(pniw->retWork) == TRUE){
      //名前を入力していたら、レコードを加算する
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( pniw->gameSystem );
      RECORD * rec = GAMEDATA_GetRecordPtr( gdata );
      RECORD_Inc( rec, RECID_NICKNAME );
    }

    GFL_STR_DeleteBuffer( pniw->p_default_str );
    NAMEIN_FreeParam( pniw->nameInParam );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 

//=====================================================================================
// ■レゾナンスリゾート名入力画面呼び出しイベント
//===================================================================================== 

//-------------------------------------------------------------------------------------
// ■イベントワーク
//-------------------------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK*  gameSystem;  // ゲームシステム 
	FIELDMAP_WORK* fieldmap;    // フィールドマップ
  NAMEIN_PARAM*  nameInParam; // 名前入力画面のパラメータ
  u16*           retWork;     // 結果の格納先ワークへのポインタ
  RESORT_NAMEIN_TYPE type;    //タイプ
} RESORT_NAMEIN_WORK; 


//-------------------------------------------------------------------------------------
/**
 * @brief イベント(リゾート名入力画面呼び出し)
 * 
 * @param gameSystem
 * @param ret_wk      入力結果を受け取るワーク(TRUE:確定, FALSE:キャンセル)
 *
 * @return イベント
 *          入力画面を呼び出し, リゾート名を設定する。
 *          名前入力が確定されたかどうかを指定したワークに返す。
 */
//-------------------------------------------------------------------------------------
GMEVENT* EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE type, GAMESYS_WORK* gameSystem, u16* ret_wk )
{
  GAMEDATA*   gdata = GAMESYSTEM_GetGameData( gameSystem );
  MISC        *misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork(gdata) );
  MYSTATUS    *mystatus = GAMEDATA_GetMyStatus(gdata);
  NAMEIN_MODE mode;
  u32         len;

  GMEVENT* event;
  RESORT_NAMEIN_WORK * work;

  switch( type )
  {
  default:
    GF_ASSERT(0);
  case RESORT_NAMEIN_TYPE_CITY_NAME:
    mode = NAMEIN_FREE_RESORT_CITY;
    len = RESORT_CITY_NAME_LEN;
    break;
  case RESORT_NAMEIN_TYPE_AUTHER_NAME:
    mode = NAMEIN_FREE_RESORT_AUTHER;
    len = RESORT_MY_NAME_LEN;
    break;
  case RESORT_NAMEIN_TYPE_AUTHER_PHASE:
    mode = NAMEIN_FREE_RESORT_PHRASE;
    len = RESORT_PHRASE_LEN;
    break;
  }

  // イベント生成
  event = GMEVENT_Create( 
      gameSystem, NULL, EVENT_FUNC_NameInput_Resort, sizeof(RESORT_NAMEIN_WORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );
  work->retWork    = ret_wk;
  work->nameInParam = NAMEIN_AllocParam(
        HEAPID_PROC, mode, MyStatus_GetMySex(mystatus), 0, len, NULL, misc );
  work->type = type;

  return event;
}

//-------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数(リゾート用入力画面呼び出しイベント)
 */
//-------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_NameInput_Resort( GMEVENT* event, int* seq, void* wk )
{
  RESORT_NAMEIN_WORK* pniw = wk;

  switch( *seq )
  {
  case 0: //----------------------------------------------- 共通イベント呼び出し
    GMEVENT_CallEvent( event,
        EVENT_NameInput(pniw->gameSystem, pniw->fieldmap, pniw->nameInParam, TRUE) );
    ++(*seq);
    break;
  case 1: //----------------------------------------------- イベント終了
    if( pniw->nameInParam->cancel != TRUE )
    {
      GAMEDATA*   gdata = GAMESYSTEM_GetGameData( pniw->gameSystem );
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( gdata );
      RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( p_sv );
      RESORT_PEOPLE_ALL_DATA *p_people_all = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );
      RESORT_CITY_DATA *p_city = RESONANCE_RESORT_SV_GetCityData( p_resort );
      RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_people_all );
      const STRCODE *cp_strcode = GFL_STR_GetStringCodePointer(pniw->nameInParam->strbuf);
      switch( pniw->type )
      {
      case RESORT_NAMEIN_TYPE_CITY_NAME:
        RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_CITY_NAME, (u32)cp_strcode );
        break;
      case RESORT_NAMEIN_TYPE_AUTHER_NAME:
        RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_AUTHER_NAME, (u32)cp_strcode );
        break;
      case RESORT_NAMEIN_TYPE_AUTHER_PHASE:
        DEBUG_RESORT_DATA_SetPermission( TRUE );
        RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT9_PHRASE, (u32)cp_strcode);
        DEBUG_RESORT_DATA_SetPermission( FALSE );
        break;
      default:
        GF_ASSERT(0);
      }
    }
    if( pniw->retWork != NULL )
    {
      *(pniw->retWork) = (pniw->nameInParam->cancel != TRUE);
    }
    NAMEIN_FreeParam( pniw->nameInParam );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 


//=====================================================================================
// ■ライバル名入力画面呼び出しイベント
//===================================================================================== 

//-------------------------------------------------------------------------------------
// ■イベントワーク
//-------------------------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK*  gameSystem;  // ゲームシステム 
	FIELDMAP_WORK* fieldmap;    // フィールドマップ
  NAMEIN_PARAM*  nameInParam; // 名前入力画面のパラメータ
  u16*           retWork;     // 結果の格納先ワークへのポインタ
} RIVAL_NAMEIN_WORK; 

//----------------------------------------------------------------------------
/**
 *	@brief  ライバル名入力呼び出しイベント
 *
 *	@param	GAMESYS_WORK* gameSystem
 *	@param	ret_wk        入力結果を受け取るワーク(TRUE:確定 FALSE:キャンセル)
 *
 *	@return event
 */
//-----------------------------------------------------------------------------
GMEVENT* EVENT_NameInput_Rival( GAMESYS_WORK* gameSystem, u16* ret_wk )
{
  GAMEDATA*   gdata = GAMESYSTEM_GetGameData( gameSystem );
  MISC        *misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork(gdata) );


  GMEVENT* event;
  RIVAL_NAMEIN_WORK * work;

  // イベント生成
  event = GMEVENT_Create( 
      gameSystem, NULL, EVENT_FUNC_NameInput_Rival, sizeof(RIVAL_NAMEIN_WORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );
  work->retWork    = ret_wk;
  work->nameInParam = NAMEIN_AllocParam(  //TODO
        HEAPID_PROC, NAMEIN_RIVALNAME, 0, 0, NAMEIN_PERSON_LENGTH, NULL, misc );

  return event;
}

//-------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数(リゾート用入力画面呼び出しイベント)
 */
//-------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_NameInput_Rival( GMEVENT* event, int* seq, void* wk )
{
  RIVAL_NAMEIN_WORK* pniw = wk;

  switch( *seq )
  {
  case 0: //----------------------------------------------- 共通イベント呼び出し
    GMEVENT_CallEvent( event,
        EVENT_NameInput(pniw->gameSystem, pniw->fieldmap, pniw->nameInParam, TRUE) );
    ++(*seq);
    break;
  case 1: //----------------------------------------------- イベント終了
    if( pniw->nameInParam->cancel != TRUE )
    {
      GAMEDATA*   gdata = GAMESYSTEM_GetGameData( pniw->gameSystem );
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( gdata );
      ETC_SAVE_WORK *etcsave = SaveData_GetEtc( p_sv );
      const STRCODE *cp_strcode = GFL_STR_GetStringCodePointer(pniw->nameInParam->strbuf);

      // セーブデータに保存
      EtcSave_SetRivalName( etcsave, cp_strcode );
    }

    // 入力結果をワークに返す
    if( pniw->retWork != NULL )
    {
      *(pniw->retWork) = (pniw->nameInParam->cancel != TRUE);
    }

    NAMEIN_FreeParam( pniw->nameInParam );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

