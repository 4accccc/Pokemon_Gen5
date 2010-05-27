//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		debug_wfbc.c
 *	@brief  WFBCオートデバック機能
 *	@author	tomoya takahashi
 *	@date		2010.05.24
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef PM_DEBUG

#include "debug_wfbc.h"

#include "field/field_wfbc_data.h" 

#include "fieldmap/zone_id.h"
#include "event_mapchange.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	全人物PATTERNチェック
//=====================================
typedef struct {

  FIELD_WFBC_CORE* p_wfbc_data;
  GAMESYS_WORK* p_gamesys;
  FIELDMAP_WORK* p_fieldmap;

  int p[10];

  int wait;

  int pattern_count;
  
} EVENT_DEBUG_WFBC_PEOPLE;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static GMEVENT_RESULT ev_DEBUG_WFBC_PeopleCheck( GMEVENT* p_event, int* p_seq, void* p_work );

static BOOL ev_DEBUG_WFBC_PeopleCheckNextID( u32 index, const EVENT_DEBUG_WFBC_PEOPLE* cp_wk, int* p_nextID );

static GMEVENT_RESULT ev_DEBUG_WFBC_100SetUp( GMEVENT* p_event, int* p_seq, void* p_work );

//----------------------------------------------------------------------------
/**
 *	@brief  人物全PATTERNチェック
 */
//-----------------------------------------------------------------------------
GMEVENT * DEBUG_WFBC_PeopleCheck(
    GAMESYS_WORK *gsys )
{
  EVENT_DEBUG_WFBC_PEOPLE* p_wk;
  GMEVENT* p_event;
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );

  p_event = GMEVENT_Create( gsys, NULL,
      ev_DEBUG_WFBC_PeopleCheck, sizeof(EVENT_DEBUG_WFBC_PEOPLE) );

  p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->p_wfbc_data = GAMEDATA_GetMyWFBCCoreData( p_gdata );

  p_wk->p_gamesys = gsys;
  p_wk->p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );


  return p_event;
}


//----------------------------------------------------------------------------
/**
 *	@brief  100回SetUpを読んでみる。
 */
//-----------------------------------------------------------------------------
GMEVENT * DEBUG_WFBC_100SetUp(
    GAMESYS_WORK *gsys )
{
  EVENT_DEBUG_WFBC_PEOPLE* p_wk;
  GMEVENT* p_event;
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );

  p_event = GMEVENT_Create( gsys, NULL,
      ev_DEBUG_WFBC_100SetUp, sizeof(EVENT_DEBUG_WFBC_PEOPLE) );

  p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->p_wfbc_data = GAMEDATA_GetMyWFBCCoreData( p_gdata );

  p_wk->p_gamesys = gsys;
  p_wk->p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );


  return p_event;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ev_DEBUG_WFBC_100SetUp( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_DEBUG_WFBC_PEOPLE* p_wk = p_work;
  int i;
  HEAPID heapID = FIELDMAP_GetHeapID( p_wk->p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_wk->p_gamesys );
  const MYSTATUS* cp_mystatus = GAMEDATA_GetMyStatus(p_gdata);

  for( i=0; i<100; i++ ){
    FIELD_WFBC_CORE_Clear( p_wk->p_wfbc_data );
    FIELD_WFBC_CORE_SetUp( p_wk->p_wfbc_data, cp_mystatus, heapID );
  }

  return GMEVENT_RES_FINISH;
}


//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  人物PATTERN総チェック
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ev_DEBUG_WFBC_PeopleCheck( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_DEBUG_WFBC_PEOPLE* p_wk = p_work;

  switch( *p_seq ){
  // 初期化
  case 0:
    {
      int i;
      for( i=0; i<10; i++ ){
        p_wk->p[i]  = i;
      }
    }
    p_wk->pattern_count = 1;
    (*p_seq)++;
    break;

  // ジャンプ
  case 1:

    {
      int i;
      OS_SetPrintOutput_Arm9( 2 );
      TOMOYA_Printf( "wfbcData[%d]:", p_wk->pattern_count );
      // データ設定
      for( i=0; i<10; i++ ){
        p_wk->p_wfbc_data->people[i].data_in = TRUE;
        p_wk->p_wfbc_data->people[i].npc_id  = p_wk->p[i];
        p_wk->p_wfbc_data->people[i].mood  = 50;
        p_wk->p_wfbc_data->people[i].one_day_msk  = FIELD_WFBC_ONEDAY_MSK_INIT;
        TOMOYA_Printf( "%d ", p_wk->p[i] );
      }
      TOMOYA_Printf( "\n" );
      OS_SetPrintOutput_Arm9( 0 );
    }
    
    p_wk->wait = 60;
    {
      GMEVENT* p_subevent;
      p_subevent = DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->p_gamesys, p_wk->p_fieldmap, ZONE_ID_BC10 );
      GMEVENT_CallEvent( p_event, p_subevent );
    }
    
    (*p_seq)++;
    break;

  // ウエイト
  case 2:
    p_wk->wait --;
    if( p_wk->wait <= 0 ){
      (*p_seq)++;
    }
    break;
    
  // 次のPATTERN
  case 3:
    {

      int check_index;
      int next_id;
      
      // カウントアップ
      for( check_index = 9; check_index>=0; check_index-- ){

        if( ev_DEBUG_WFBC_PeopleCheckNextID( check_index, p_wk, &next_id ) == FALSE ){
          p_wk->p[check_index] = next_id;
          break;
        }

        // check_indexの桁ではすべてチェックしたのでLOOPクリア
        p_wk->p[check_index] = -1;
      }

      // check_indexがマイナスになったら終わり
      if( check_index<0 ){

        OS_SetPrintOutput_Arm9( 2 );
        TOMOYA_Printf( "ALL WFBC PEOPLE CHECK END\n" );
        OS_SetPrintOutput_Arm9( 0 );
        return( GMEVENT_RES_FINISH );
      }

      // check_indexより大きいインデックスのワークを再設定
      for( check_index += 1; check_index<10; check_index++ ){
        GF_ASSERT( p_wk->p[check_index] == -1 );
        p_wk->p[check_index] = 0;
        if( ev_DEBUG_WFBC_PeopleCheckNextID( check_index, p_wk, &next_id ) == FALSE ){
          p_wk->p[check_index] = next_id;
        }else{
          GF_ASSERT( 0 ); // おかしい。
        }
      }
    }
    (*p_seq) = 1;
    p_wk->pattern_count++;
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}


//----------------------------------------------------------------------------
/**
 *	@brief  次のチェックIDを調べる
 *
 *	@param	index インデックス　０～９
 *	@param	p_wk      ワーク
 *	@param	p_nextID  次のID可能先
 *
 *	@retval p_nextIDがFIELD_WFBC_NPCID_MAXを超える
 */
//-----------------------------------------------------------------------------
static BOOL ev_DEBUG_WFBC_PeopleCheckNextID( u32 index, const EVENT_DEBUG_WFBC_PEOPLE* cp_wk, int* p_nextID )
{
  int i;
  int check;
  BOOL not_hit;
  BOOL ret = FALSE;

  
  check = cp_wk->p[index];

  do{
    check ++;
    not_hit = TRUE;

    for( i=0; i<10; i++ ){

      if( i != index ){
        if( cp_wk->p[i] == check ){
          not_hit = FALSE;
        }
      }
    } 
    
  }while( not_hit == FALSE );

  *p_nextID = check;
  
  if( check >= FIELD_WFBC_NPCID_MAX ){
    return TRUE;
  }
  return FALSE;
  
}



#endif //PM_DEBUG

