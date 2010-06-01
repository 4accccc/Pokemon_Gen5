//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_rebattle_trainer.c
 *	@brief  再戦トレーナー用スクリプト
 *	@author	tomoya takahashi
 *	@date		2010.03.19
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "system/main.h"

#include "arc/arc_def.h"
#include "arc/fieldmap/rebattle_trainer.naix"

#include "item/itemsym.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "script_trainer.h"


#include "fieldmap.h"

#include "scrcmd_rebattle_trainer.h"

#include "rebattle_trainer.h"

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
///	再戦トレーナー情報
//=====================================
typedef struct {
  u16 id;
  u16 badge_num;
  u16 code;
  u16 trainer_id;
} REBATTLE_TRAINER_DATA;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static u32 ReBattle_SearchData( const REBATTLE_TRAINER_DATA* cp_data, u32 id, u32 badge_num );

//----------------------------------------------------------------------------
/**
 *	@brief  再戦トレーナー情報を確保
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdReBattleTrainerStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  HEAPID heapID = HEAPID_PROC;  // テンポラリ
  void* p_data;

  p_data = GFL_ARC_UTIL_Load( ARCID_REBATTLE_TRAINER, 0, FALSE, GFL_HEAP_LOWID(heapID) );

  // １つ前で削除していない
  GF_ASSERT( SCRIPT_GetReBattleTrainerData(sc) == NULL );
  SCRIPT_SetReBattleTrainerData( sc, p_data );

  TOMOYA_Printf( "再戦トレーナー情報を確保\n" );

	return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  再戦トレーナー情報を破棄
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdReBattleTrainerEnd( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  void* p_data;

  p_data = SCRIPT_GetReBattleTrainerData(sc);
  if( p_data ){
    GFL_HEAP_FreeMemory( p_data );
    SCRIPT_SetReBattleTrainerData( sc, NULL );

    TOMOYA_Printf( "再戦トレーナー情報を破棄\n" );
  }

	return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  再戦トレーナー　トレーナーID設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdReBattleTrainerSetUpTrID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 rebattle_id = SCRCMD_GetVMWorkValue( core, work );
  u16 objid = SCRCMD_GetVMWorkValue( core, work );
  u16 badge = SCRCMD_GetVMWorkValue( core, work );
  u16 index;
  EVENTDATA_SYSTEM * evdata = GAMEDATA_GetEventData( SCRCMD_WORK_GetGameData(work) );
  const REBATTLE_TRAINER_DATA* cp_data = SCRIPT_GetReBattleTrainerData(sc);

  GF_ASSERT( cp_data );

  index = ReBattle_SearchData( cp_data, rebattle_id, badge );


  TOMOYA_Printf( "設定情報　TrainerID[%d]\n", cp_data[index].trainer_id );

  // トレーナーIDの視線スクリプト設定
  EVENTDATA_ChangeNPCEventID( evdata, objid, SCRIPT_GetTrainerScriptID_ByTrainerID( cp_data[index].trainer_id ) );

	return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  再戦トレーナー　ダブルバトル トレーナーID設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdReBattleTrainerSetUp2vs2TrID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 rebattle_id = SCRCMD_GetVMWorkValue( core, work );
  u16 objid0 = SCRCMD_GetVMWorkValue( core, work );
  u16 objid1 = SCRCMD_GetVMWorkValue( core, work );
  u16 badge = SCRCMD_GetVMWorkValue( core, work );
  u16 index;
  EVENTDATA_SYSTEM * evdata = GAMEDATA_GetEventData( SCRCMD_WORK_GetGameData(work) );
  const REBATTLE_TRAINER_DATA* cp_data = SCRIPT_GetReBattleTrainerData(sc);

  GF_ASSERT( cp_data );

  index = ReBattle_SearchData( cp_data, rebattle_id, badge );


  TOMOYA_Printf( "設定情報　TrainerID[%d]\n", cp_data[index].trainer_id );

  // トレーナーIDの視線スクリプト設定
  EVENTDATA_ChangeNPCEventID( evdata, objid0, SCRIPT_GetTrainerScriptID_ByTrainerID( cp_data[index].trainer_id ) );
  EVENTDATA_ChangeNPCEventID( evdata, objid1, SCRIPT_GetTrainerScriptID_By2vs2TrainerID( cp_data[index].trainer_id ) );

	return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  再戦トレーナー　ランダムトレーナーの設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdReBattleTrainerSetUpRndTr( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 objid0 = SCRCMD_GetVMWorkValue( core, work );
  u16 objid1 = SCRCMD_GetVMWorkValue( core, work );
  u16 objid2 = SCRCMD_GetVMWorkValue( core, work );
  u16 index0 = SCRCMD_GetVMWorkValue( core, work );
  u16 index1 = SCRCMD_GetVMWorkValue( core, work );
  u16 index2 = SCRCMD_GetVMWorkValue( core, work );
  EVENTDATA_SYSTEM * evdata = GAMEDATA_GetEventData( SCRCMD_WORK_GetGameData(work) );
  const REBATTLE_TRAINER_DATA* cp_data = SCRIPT_GetReBattleTrainerData(sc);

  GF_ASSERT( cp_data );

  GF_ASSERT( index0 < RB_DATA_TBL_MAX );
  GF_ASSERT( index1 < RB_DATA_TBL_MAX );
  GF_ASSERT( index2 < RB_DATA_TBL_MAX );
  
  TOMOYA_Printf( "設定情報 0　OBJCode[%d] TrainerID[%d]\n", cp_data[ index0 ].code, cp_data[ index0 ].trainer_id );
  TOMOYA_Printf( "設定情報 1　OBJCode[%d] TrainerID[%d]\n", cp_data[ index1 ].code, cp_data[ index1 ].trainer_id );
  TOMOYA_Printf( "設定情報 2　OBJCode[%d] TrainerID[%d]\n", cp_data[ index2 ].code, cp_data[ index2 ].trainer_id );

  // 見た目設定
  EVENTDATA_ChangeNPCObjCode( evdata, objid0, cp_data[ index0 ].code );
  EVENTDATA_ChangeNPCObjCode( evdata, objid1, cp_data[ index1 ].code );
  EVENTDATA_ChangeNPCObjCode( evdata, objid2, cp_data[ index2 ].code );
  
  // トレーナーIDを設定
  EVENTDATA_ChangeNPCEventID( evdata, objid0, SCRIPT_GetTrainerScriptID_ByTrainerID( cp_data[ index0 ].trainer_id ) );
  EVENTDATA_ChangeNPCEventID( evdata, objid1, SCRIPT_GetTrainerScriptID_ByTrainerID( cp_data[ index1 ].trainer_id ) );
  EVENTDATA_ChangeNPCEventID( evdata, objid2, SCRIPT_GetTrainerScriptID_ByTrainerID( cp_data[ index2 ].trainer_id ) );

	return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  全トレーナーIDの再戦フラグをクリア
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdReBattleTrainerClearTrEventFlag( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK * gsys = SCRIPT_GetGameSysWork( sc );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );
  int i;
  const REBATTLE_TRAINER_DATA* cp_data = SCRIPT_GetReBattleTrainerData(sc);

  GF_ASSERT( cp_data );


  // すべてクリア
  for( i=0; i<RB_DATA_TBL_MAX; i++ ){
    SCRIPT_ResetEventFlagTrainer( evwork, cp_data[i].trainer_id );
  }
  
	return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief　再戦トレーナー　ランダムNPCインデックスの抽選
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdReBattleTrainerGetRndTrIndex( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 *ret_index0	= SCRCMD_GetVMWork( core, work );
  u16 *ret_index1	= SCRCMD_GetVMWork( core, work );
  u16 *ret_index2	= SCRCMD_GetVMWork( core, work );
  u16 *ret_index3	= SCRCMD_GetVMWork( core, work );
  u16 *ret_index4	= SCRCMD_GetVMWork( core, work );
  u16 *ret_index5	= SCRCMD_GetVMWork( core, work );
  u16 random_max = RB_DATA_TBL_MAX - RB_RANDOM_START;
  int i, j, k;
  u16 index[6] = {0};
  s16 data_index;
  BOOL setup;

  // かぶらないように乱数設定
  for( i=0; i<6; i++ ){

    setup = FALSE;

    // 抽選
    data_index = GFUser_GetPublicRand( random_max );

    // かぶらない数値の先頭からdata_index目の数値を使用
    for( j=0; j<(RB_DATA_TBL_MAX - RB_RANDOM_START); j++ ){

      // かぶりチェック
      for( k=0; k<i; k++ ){
        if( index[k] == j ){
          // スキップ
          break;
        }
      }

      if( k==i ){ // かぶり無しなら、data_indexをへらし、マイナスになったNPCを使用する。
        data_index --;
        if( data_index < 0 ){
          index[i] = j;
          setup = TRUE;
          break;
        }
      }
    }

    GF_ASSERT( setup );

    // 1体抽選からはずす
    random_max--;
  }

  *ret_index0 = index[0] + RB_RANDOM_START;
  *ret_index1 = index[1] + RB_RANDOM_START;
  *ret_index2 = index[2] + RB_RANDOM_START;

  *ret_index3 = index[3] + RB_RANDOM_START;
  *ret_index4 = index[4] + RB_RANDOM_START;
  *ret_index5 = index[5] + RB_RANDOM_START;

	return VMCMD_RESULT_CONTINUE;
}



//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  再戦データの検索
 *
 *	@param	cp_data     データ
 *	@param	id          ID
 *	@param	badge_num   バッジ数
 *
 *	@return インデックス
 */
//-----------------------------------------------------------------------------
static u32 ReBattle_SearchData( const REBATTLE_TRAINER_DATA* cp_data, u32 id, u32 badge_num )
{
  int i;

  for( i=0; i<RB_DATA_TBL_MAX; i++ ){

    if( (cp_data[i].id == id) && 
        (cp_data[i].badge_num == badge_num) ){
      return i;
    }
  }

  OS_TPrintf( "再戦Data id = %d  level = %d\n", id, badge_num );
  GF_ASSERT_MSG(0, "再戦データなし");

  return 0;
}



