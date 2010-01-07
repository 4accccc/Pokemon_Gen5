//======================================================================
/**
 * @file	scrcmd_fldmmdl.c
 * @brief	スクリプトコマンド：動作モデル関連
 * @date  2009.09.13
 * @author	tamada GAMEFREAK inc.
 *
 * 2009.09.13 scrcmd.cより分離させた
 */
//======================================================================

#include <gflib.h>

#include "system/vm_cmd.h"

#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_fldmmdl.h"

#include "script_def.h" //SCR_OBJID_
#include "script_local.h"
#include "field_player.h" //FIELD_PLAYER
#include "fieldmap.h"     //FIELDMAP_
#include "field/field_const.h"  //GRID_TO_FX32
#include "eventdata_local.h"
#include "field/eventdata_sxy.h"

#include "fieldmap_ctrl.h"
#include "fieldmap_ctrl_grid.h"
#include "field_player_grid.h"

#include "field_task_player_drawoffset.h"

#include "event_fldmmdl_control.h"

//======================================================================
//  ツール関数
//======================================================================
static MMDL * scmd_GetMMdlPlayer( SCRCMD_WORK *work );

//======================================================================
///  動作モデル監視に使用するインライン関数定義
//======================================================================
#define  PLAYER_BIT      (1<<0)
#define  PLAYER_PAIR_BIT    (1<<1)
#define  OTHER_BIT      (1<<2)
#define  OTHER_PAIR_BIT    (1<<3)

static const MMDL_ACMD_LIST anime_up_table[] = {
  AC_STAY_WALK_U_8F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_down_table[] = {
  AC_STAY_WALK_D_8F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_left_table[] = {
  AC_STAY_WALK_L_8F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_right_table[] = {
  AC_STAY_WALK_R_8F, 1,
  ACMD_END, 0
};


static u8 step_watch_bit;

static inline void InitStepWatchBit(void)
{
  step_watch_bit = 0;
}

static inline BOOL CheckStepWatchBit(int mask)
{
  return (step_watch_bit & mask) != 0;
}

static inline void SetStepWatchBit(int mask)
{
  step_watch_bit |= mask;
}

static inline void ResetStepWatchBit(int mask)
{
  step_watch_bit &= (0xff ^ mask);
}

//======================================================================
//  動作モデル  
//======================================================================
//--------------------------------------------------------------
/**
 * FIELD_OBJ_PTRを取得
 * @param  fsys  FIELDSYS_WORK型のポインタ
 * @param  obj_id  OBJID
 * @retval  "FIELD_OBJ_PTR"
 */
//--------------------------------------------------------------
static MMDL * FieldObjPtrGetByObjId( SCRCMD_WORK *work, u16 obj_id )
{
  MMDL *dummy;
  MMDL *fmmdl;
  MMDLSYS *fmmdlsys;
  
  fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  
  //連れ歩きOBJ判別IDが渡された時
  if( obj_id == SCR_OBJID_MV_PAIR ){
    fmmdl = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
  //透明ダミーOBJ判別IDが渡された時
  }else if( obj_id == SCR_OBJID_DUMMY ){
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    dummy = SCRIPT_GetMemberWork( sc, ID_EVSCR_DUMMY_OBJ );
  //対象のフィールドOBJのポインタ取得
  }else{
    fmmdl = MMDLSYS_SearchOBJID( fmmdlsys, obj_id );
  }
  
  return fmmdl;
}

//--------------------------------------------------------------
/**
 * 動作モデルアニメーション
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjAnime( VMHANDLE *core, void *wk )
{
  u8 *num;
  VM_CODE *p;
  GFL_TCB *anm_tcb;
  MMDL *fmmdl; //対象のフィールドOBJのポインタ
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 obj_id = SCRCMD_GetVMWorkValue(core,work); //obj ID
  u32 pos = VMGetU32(core); //list pos
  
  fmmdl = FieldObjPtrGetByObjId( work, obj_id );
  
  //エラーチェック
  if( fmmdl == NULL ){
    OS_Printf( "obj_id = %d\n", obj_id );
    GF_ASSERT( (0) && "対象のフィールドOBJのポインタ取得失敗！" );
    return VMCMD_RESULT_CONTINUE;        //08.06.12 プラチナで追加
  }
  
  //アニメーションコマンドリストセット
  p = (VM_CODE*)(core->adrs+pos);
  anm_tcb = MMDL_SetAcmdList( fmmdl, (MMDL_ACMD_LIST*)p );
  
  //アニメーションの数を足す
  num = SCRIPT_GetMemberWork( sc, ID_EVSCR_ANMCOUNT );
  (*num)++;
  
  //TCBセット
  SCRCMD_WORK_SetMMdlAnmTCB( work, anm_tcb );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 動作モデルアニメーション終了待ち　ウェイと部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvObjAnimeWait(VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  
  if( SCRCMD_WORK_CheckMMdlAnmTCB(work) == FALSE ){
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * アニメーション終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjAnimeWait( VMHANDLE * core, void *wk )
{
  VMCMD_SetWait( core, EvObjAnimeWait );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 動作コード取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMoveCodeGet( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  u16 *ret_wk  = SCRCMD_GetVMWork( core, work );
  
  *ret_wk = 0;
  mmdl = MMDLSYS_SearchOBJID( mmdlsys, SCRCMD_GetVMWorkValue(core,work) );
  GF_ASSERT( mmdl != NULL && "SCRCMD GET MOVECODE NON OBJ" );
  *ret_wk = MMDL_GetMoveCode( mmdl );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJの位置情報取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjPosGet( VMHANDLE *core, void *wk )
{
  u16 *x,*z;
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *mmdl = MMDLSYS_SearchOBJID(
      mmdlsys, SCRCMD_GetVMWorkValue(core,work) );
  
  x = SCRCMD_GetVMWork( core, work );
  z = SCRCMD_GetVMWork( core, work );
  *x = MMDL_GetGridPosX( mmdl );
  *z = MMDL_GetGridPosZ( mmdl );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 自機の位置情報取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerPosGet( VMHANDLE *core, void *wk )
{
  u16 *x,*z;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  
  x = SCRCMD_GetVMWork( core, work );
  z = SCRCMD_GetVMWork( core, work );
  *x = MMDL_GetGridPosX( mmdl );
  *z = MMDL_GetGridPosZ( mmdl );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 自機の向き取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerDirGet( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDL *player = scmd_GetMMdlPlayer( work );

  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  
  *ret_wk = MMDL_GetDirDisp( player );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJを追加
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjAdd( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  u16 gx = VMGetU16( core );
  u16 gz = VMGetU16( core );
  u16 dir = VMGetU16( core );
  u16 id = VMGetU16( core );
  u16 code = VMGetU16( core );
  u16 move = VMGetU16( core );
  int zone_id = FIELDMAP_GetZoneID( fparam->fieldMap );
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  
  mmdl = MMDLSYS_AddMMdlParam(
      mmdlsys, gx, gz, dir, id, code, move, zone_id );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJを削除
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjDel( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  u16 id = SCRCMD_GetVMWorkValue( core, work );
  MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
  GF_ASSERT( mmdl != NULL && "OBJ DEL 対象のOBJが居ません\n" );
  MMDL_Delete( mmdl );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJを追加 配置データから
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjAddEvent( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
	EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
	u16 count = EVENTDATA_GetNpcCount( evdata );
	PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
  int zone_id = PLAYERWORK_getZoneID( player );
  u16 id = SCRCMD_GetVMWorkValue( core, work );
  
  if( count ){
    EVENTWORK *evwork =  GAMEDATA_GetEventWork( gdata );
		const MMDL_HEADER *header = EVENTDATA_GetNpcData( evdata );
		MMDL *mmdl = MMDLSYS_AddMMdlHeaderID(
        mmdlsys, header, zone_id, count, evwork, id );
    GF_ASSERT( mmdl != NULL );
  }else{
    GF_ASSERT( 0 );
  }
  
  return( VMCMD_RESULT_CONTINUE );
}

//--------------------------------------------------------------
/**
 * OBJを削除　OBJ指定イベントフラグをONに
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjDelEvent( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *evwork =  GAMEDATA_GetEventWork( gdata );
  u16 id = SCRCMD_GetVMWorkValue( core, work );
  
  mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
  GF_ASSERT( mmdl != NULL );
  
  MMDL_DeleteEvent( mmdl, evwork );
  return( VMCMD_RESULT_CONTINUE );
}

//--------------------------------------------------------------
/**
 * OBJデータの座標情報を変更
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjPosChange( VMHANDLE *core, void *wk )
{
	MMDL *mmdl;
  SCRCMD_WORK *work = wk;
	u16 obj_id	= SCRCMD_GetVMWorkValue( core, work );
	u16 x		= SCRCMD_GetVMWorkValue(core,work);
	u16 y		= SCRCMD_GetVMWorkValue(core,work);
	u16 z		= SCRCMD_GetVMWorkValue(core,work);
	u16 dir		= SCRCMD_GetVMWorkValue(core,work);
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  
	//対象のフィールドOBJのポインタ取得
	mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
  GF_ASSERT( mmdl != NULL && "ERROR OBJ ID" );
  
  MMDL_InitGridPosition( mmdl, x, y, z, dir );
	return VMCMD_RESULT_CONTINUE;
}

#if 0
//--------------------------------------------------------------
/**
 * SXYデータの座標情報を変更
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
static BOOL EvCmdSxyPosChange( VM_MACHINE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16 id = SCRCMD_GetVMWorkValue(core,work);
	u16 gx = SCRCMD_GetVMWorkValue(core,work);
	u16 gz = SCRCMD_GetVMWorkValue(core,work);
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
  
	EventData_NpcDataPosChange( core->fsys, id, gx, gz );
	return 0;
}
#endif

//======================================================================
//  動作モデル　イベント関連
//======================================================================
//--------------------------------------------------------------
/**
 * @brief  OBJ動作終了待ち　ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitTalkObj( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDL **fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  MMDL *player = scmd_GetMMdlPlayer( work );
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );

  //自機動作停止チェック
  if( CheckStepWatchBit(PLAYER_BIT) &&
    MMDL_CheckEndAcmd(player) == TRUE ){
    MMDL_OnMoveBitMoveProcPause( player );
    ResetStepWatchBit(PLAYER_BIT);
  }
  
  //話しかけ対象動作停止チェック
  if( CheckStepWatchBit(OTHER_BIT) &&
    MMDL_CheckMoveBitMove(*fmmdl) == FALSE ){
    MMDL_OnMoveBitMoveProcPause( *fmmdl );
    ResetStepWatchBit(OTHER_BIT);
  }
  
  //自機の連れ歩き動作停止チェック
  if( CheckStepWatchBit(PLAYER_PAIR_BIT) ){
    MMDL *player_pair = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
    
    if( MMDL_CheckMoveBitMove(player_pair) == FALSE ){
      MMDL_OnMoveBitMoveProcPause( player_pair );
      ResetStepWatchBit(PLAYER_PAIR_BIT);
    }
  }
  
  //話しかけ対象の連れ歩き動作停止チェック
  if( CheckStepWatchBit(OTHER_PAIR_BIT) ){
    #ifndef SCRCMD_PL_NULL
    MMDL *other_pair = FieldOBJ_MovePairSearch(*fldobj);
    if (FieldOBJ_StatusBitCheck_Move(other_pair) == 0) {
      FieldOBJ_MovePause(other_pair);
      ResetStepWatchBit(OTHER_PAIR_BIT);
    }
    #else
    GF_ASSERT( 0 );
    #endif
  }
  
  if( step_watch_bit == 0 ){
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 会話イベント用全OBJ動作停止
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTalkObjPauseAll( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL **fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  MMDL *player = scmd_GetMMdlPlayer( work );
  MMDL *player_pair = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
  
#ifndef SCRCMD_PL_NULL
  MMDL *other_pair = FieldOBJ_MovePairSearch(*fldobj);
#else
  MMDL *other_pair = NULL;
#endif

  InitStepWatchBit();
  MMDLSYS_PauseMoveProc( fmmdlsys );
  
  if( MMDL_CheckEndAcmd(player) == FALSE ){
    SetStepWatchBit(PLAYER_BIT);
    MMDL_OffMoveBitMoveProcPause( player );
  }
  
  if( *fmmdl != NULL ){
    if( MMDL_CheckMoveBitMove(*fmmdl) == TRUE ){
      SetStepWatchBit(OTHER_BIT);
      MMDL_OffMoveBitMoveProcPause( *fmmdl );
    }
  }

  if( player_pair ){
    #ifndef SCRCMD_PL_NULL
    if( SysFlag_PairCheck(SaveData_GetEventWork(fsys->savedata)) == 1
        && FieldOBJ_StatusBitCheck_Move(player_pair) != 0) {
      SetStepWatchBit(PLAYER_PAIR_BIT);
      FieldOBJ_MovePauseClear( player_pair );
    }
    #else
    GF_ASSERT( 0 );
    #endif
  }
  
  if( other_pair ){
    if( MMDL_CheckMoveBitMove(other_pair) == TRUE ){
      SetStepWatchBit(OTHER_PAIR_BIT);
      MMDL_OffMoveBitMoveProcPause( other_pair );
    }
  }
  
  VMCMD_SetWait( core, EvWaitTalkObj );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 全OBJ動作停止
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
#if 0
static VMCMD_RESULT EvCmdObjPauseAll( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc;
  MMDL **fmmdl;
  
  sc = SCRCMD_WORK_GetScriptWork( work );
  fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  
  if( (*fmmdl) == NULL ){
    MMDLSYS *fmmdlsys;
    fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
    MMDLSYS_PauseMoveProc( fmmdlsys );
    
    #ifndef SCRCMD_PL_NULL
    //08.06.18
    //話しかけの対象がいないBGやPOSの時
    //連れ歩きOBJの移動動作中かのチェックをしていない
    //
    //ふれあい広場などで、連れ歩きOBJに対して、
    //スクリプトでアニメを発行すると、
    //アニメが行われず終了待ちにいかないでループしてしまう
    
    {
      FIELD_OBJ_PTR player_pair =
        FieldOBJSys_MoveCodeSearch( fsys->fldobjsys, MV_PAIR );
      //ペアが存在している
      if (player_pair) {
        //連れ歩きフラグが立っていて、移動動作中なら
        if( SysFlag_PairCheck(
          SaveData_GetEventWork(fsys->savedata)) == 1
          && FieldOBJ_StatusBitCheck_Move(player_pair) != 0) {
          
          //ペアの動作ポーズ解除
          FieldOBJ_MovePauseClear( player_pair );
          
          //移動動作の終了待ちをセット
          VM_SetWait( core, EvWaitPairObj );
          return VMCMD_RESULT_SUSPEND;
        }
      }
    }
    #endif
  }else{
    EvCmdTalkObjPauseAll( core, wk );
  }
  
  return VMCMD_RESULT_SUSPEND;
}
#endif

VMCMD_RESULT SCRCMD_SUB_ObjPauseAll( VMHANDLE *core, SCRCMD_WORK *work )
{
  SCRIPT_WORK *sc;
  MMDL **fmmdl;
  
  sc = SCRCMD_WORK_GetScriptWork( work );
  fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  
  if( (*fmmdl) == NULL ){
    MMDLSYS *fmmdlsys;
    fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
    MMDLSYS_PauseMoveProc( fmmdlsys );
    
    #ifndef SCRCMD_PL_NULL
    //08.06.18
    //話しかけの対象がいないBGやPOSの時
    //連れ歩きOBJの移動動作中かのチェックをしていない
    //
    //ふれあい広場などで、連れ歩きOBJに対して、
    //スクリプトでアニメを発行すると、
    //アニメが行われず終了待ちにいかないでループしてしまう
    
    {
      FIELD_OBJ_PTR player_pair =
        FieldOBJSys_MoveCodeSearch( fsys->fldobjsys, MV_PAIR );
      //ペアが存在している
      if (player_pair) {
        //連れ歩きフラグが立っていて、移動動作中なら
        if( SysFlag_PairCheck(
          SaveData_GetEventWork(fsys->savedata)) == 1
          && FieldOBJ_StatusBitCheck_Move(player_pair) != 0) {
          
          //ペアの動作ポーズ解除
          FieldOBJ_MovePauseClear( player_pair );
          
          //移動動作の終了待ちをセット
          VM_SetWait( core, EvWaitPairObj );
          return VMCMD_RESULT_SUSPEND;
        }
      }
    }
    #endif
  }else{
    return( EvCmdTalkObjPauseAll(core,work) );
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 全OBJ動作再開
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
#if 0
static VMCMD_RESULT EvCmdObjPauseClearAll( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDLSYS_ClearPauseMoveProc( fmmdlsys );
  return VMCMD_RESULT_SUSPEND;
}
#endif
void SCRCMD_SUB_PauseClearAll( SCRCMD_WORK *work )
{
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDLSYS_ClearPauseMoveProc( fmmdlsys );
}

//--------------------------------------------------------------
/**
 * 話しかけたOBJ自機方向への振り向き
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjTurn( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDL *jiki,**fmmdl;
  u16 dir;
  
  jiki = scmd_GetMMdlPlayer( work );

  dir = MMDL_GetDirDisp( jiki );
  
  dir = MMDL_TOOL_FlipDir( dir );
  fmmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );
  
  if( (*fmmdl) == NULL ){
    return VMCMD_RESULT_CONTINUE;
  }
  
  MMDL_SetDirDisp( *fmmdl, dir );
  MMDL_SetDrawStatus( *fmmdl, DRAW_STA_STOP );
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * @brief 指定位置のOBJを検索する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetObjID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  u16 gx, gy, gz;
  u16 *ret_id     = SCRCMD_GetVMWork( core, work );
  u16 *ret_valid  = SCRCMD_GetVMWork( core, work );
  MMDL *mmdl;
  u16 dir;
  VecFx32 pos;
  fx32 fy;
  
  gx = SCRCMD_GetVMWorkValue( core, work );
  gy = SCRCMD_GetVMWorkValue( core, work );
  gz = SCRCMD_GetVMWorkValue( core, work );
  fy = GRID_TO_FX32( gy );
  mmdl = MMDLSYS_SearchGridPosEx(
      FIELDMAP_GetMMdlSys(fieldmap), gx, gz, fy, GRID_HALF_FX32, FALSE );
  if ( mmdl )
  {
    *ret_valid = TRUE;
    *ret_id    = MMDL_GetOBJID( mmdl );
  }
  else
  {
    *ret_valid = FALSE;
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief プレイヤーの前方1グリッドにあるOBJを検索する
 * @param core 仮想マシン制御構造体へのポインタ
 * @param wk
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetFrontObjID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  MMDLSYS*       mmdl_sys = FIELDMAP_GetMMdlSys( fieldmap );
  FIELD_PLAYER*    player = FIELDMAP_GetFieldPlayer( fieldmap );
  u16*             ret_id = SCRCMD_GetVMWork( core, work );  // コマンド第一引数(オブジェIDの格納先)
  u16*          ret_valid = SCRCMD_GetVMWork( core, work );  // コマンド第二引数(オブジェの有無)
  MMDL* mmdl;
  s16 gx, gy, gz;
  fx32 fy;

  // 自機の前方のグリッド座標を取得
  FIELD_PLAYER_GetFrontGridPos( player, &gx, &gy, &gz );
  fy = GRID_TO_FX32( gy );
  // 前方にある動作モデルを取得
  mmdl = MMDLSYS_SearchGridPosEx( mmdl_sys, gx, gz, fy, GRID_HALF_FX32, FALSE );
  // 結果を格納
  if( mmdl )
  {
    *ret_valid = TRUE;
    *ret_id    = MMDL_GetOBJID( mmdl );
  }
  else
  {
    *ret_valid = FALSE;
  }
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  自機
//======================================================================
//--------------------------------------------------------------
/**
 * 自機リクエスト
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerRequest( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  u16 req = VMGetU16( core );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  
  if( FIELDMAP_GetMapControlType(fieldmap) == FLDMAP_CTRLTYPE_GRID ){
    FIELD_PLAYER_GRID *gjiki =
      FIELDMAP_GetPlayerGrid( fieldmap );
    FIELD_PLAYER_GRID_SetRequest( gjiki, req );
    FIELD_PLAYER_GRID_UpdateRequest( gjiki );
  }else{
    FIELD_PLAYER_NOGRID *gjiki = FIELDMAP_GetPlayerNoGrid( fieldmap );

    FIELD_PLAYER_NOGRID_SetRequestBit( gjiki, req );
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 自機の上下動演出リクエスト
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerUpDown( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  FIELD_TASK* move;
  FIELD_TASK_MAN* man = FIELDMAP_GetTaskManager( fieldmap );
  VecFx32 vec;
  u16 type = SCRCMD_GetVMWorkValue( core, work );
  u16 frame_param = SCRCMD_GetVMWorkValue( core, work );
  u16 length = SCRCMD_GetVMWorkValue( core, work );
  int frame;
  if (type == 0) {
    frame = frame_param;
  } else {
    frame = - frame_param;
  }

  VEC_Set( &vec, 0, (- length) <<FX32_SHIFT, 0 );
  move     = FIELD_TASK_TransDrawOffset( fieldmap, frame, &vec );
  FIELD_TASK_MAN_AddTask( man, move, NULL );

  return VMCMD_RESULT_CONTINUE;
}


//======================================================================
//
//
//    ツール関数
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * 自機MMDL取得　仮
 * @param work SCRCMD_WORK
 * @retval
 */
//--------------------------------------------------------------
static MMDL * scmd_GetMMdlPlayer( SCRCMD_WORK *work )
{
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *fmmdl = MMDLSYS_SearchOBJID( fmmdlsys, MMDL_ID_PLAYER );
  return( fmmdl );
}

//--------------------------------------------------------------
/**
 * 方向起動ＰＯＳ用向き直り処理   @todo
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPosPlayerTurn( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  u16 dir;
  {
    int i;
    VecFx32 pos;
    FIELD_PLAYER *fld_player;
    u16 player_dir;

    u16 dir_base[4] = {DIR_UP,DIR_DOWN,DIR_LEFT,DIR_RIGHT};

    EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
    EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );

    fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
    FIELD_PLAYER_GetPos( fld_player, &pos );
    for (i=0;i<4;i++)
    {
      const POS_EVENT_DATA *evt_data = EVENTDATA_GetPosEvent(
          evdata, evwork, &pos, dir_base[i] );
      if ( evt_data != NULL )
      {
        dir = dir_base[i];
        break;
      }
    }
  
    //自機の見た目の向きを取得
    player_dir = FIELD_PLAYER_GetDir( fld_player );
    //向きの比較をする
    if (player_dir != dir)
    {
      GMEVENT * child;
      void *tbl;
      if ( dir == DIR_UP ){
        tbl = (void*)anime_up_table;
      }else if (dir == DIR_DOWN){
        tbl = (void*)anime_down_table;
      }else if (dir == DIR_LEFT){
        tbl = (void*)anime_left_table;
      }else if (dir == DIR_RIGHT){
        tbl = (void*)anime_right_table;
      }else{
        GF_ASSERT(0);
        return VMCMD_RESULT_CONTINUE;
      }
      child = EVENT_ObjAnime( gsys, fieldmap, MMDL_ID_PLAYER, tbl );
      SCRIPT_CallEvent( sc, child );
      return VMCMD_RESULT_SUSPEND;
    }
  }
  
  return VMCMD_RESULT_CONTINUE;
}


