//============================================================================================
/**
 * @file	event_fldmmdl_control.c
 * @brief
 * @date	2009.06.10
 * @author	tamada GAME FREAK inc.
 */
//============================================================================================
#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/zonedata.h"
#include "field/fieldmap.h"
#include "field/location.h"
#include "field/rail_location.h"  //RAIL_LOCATION
#include "field/eventdata_sxy.h"
#include "field/fldmmdl.h"

#include "./event_fieldmap_control.h"
#include "event_mapchange.h"
#include "sound/pm_sndsys.h"		//サウンドシステム参照

#include "fldmmdl.h"

#include "script.h"
#include "script_def.h"

#include "event_fldmmdl_control.h"
#include "fldmmdl_code.h"

#define SCRCMD_PL_NULL


//======================================================================
///	動作モデル監視に使用するインライン関数定義
//======================================================================
#define	PLAYER_BIT			(1<<0)
#define	PLAYER_PAIR_BIT		(1<<1)
#define	OTHER_BIT			(1<<2)
#define	OTHER_PAIR_BIT		(1<<3)

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  GAMEDATA * gamedata;
  u8 step_watch_bit;
}OBJ_PAUSE_ALL_WORK;

static inline void InitStepWatchBit(OBJ_PAUSE_ALL_WORK * work)
{
	work->step_watch_bit = 0;
}

static inline BOOL CheckStepWatchBit(const OBJ_PAUSE_ALL_WORK * work, int mask)
{
	return (work->step_watch_bit & mask) != 0;
}

static inline void SetStepWatchBit(OBJ_PAUSE_ALL_WORK * work, int mask)
{
	work->step_watch_bit |= mask;
}

static inline void ResetStepWatchBit(OBJ_PAUSE_ALL_WORK * work, int mask)
{
	work->step_watch_bit &= (0xff ^ mask);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENTFUNC_ObjPauseAll(GMEVENT * event, int *seq, void*work)
{
	OBJ_PAUSE_ALL_WORK * opaw = work;
	GAMESYS_WORK  * gsys = opaw->gsys;
	FIELDMAP_WORK * fieldmap = opaw->fieldmap;
	GAMEDATA * gamedata = opaw->gamedata;

  switch (*seq)
  {
  case 0:
    {
      MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDL *fmmdl = NULL;  //とりあえず
      //MMDL *player = PlayerGetMMdl( work );
      MMDL *player = MMDLSYS_SearchOBJID( fmmdlsys, MMDL_ID_PLAYER );
      MMDL *player_pair = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
      
#ifndef SCRCMD_PL_NULL
      MMDL *other_pair = FieldOBJ_MovePairSearch(*fldobj);
#else
      MMDL *other_pair = NULL;
#endif

      InitStepWatchBit(opaw);
      MMDLSYS_PauseMoveProc( fmmdlsys );
      
      if( MMDL_CheckEndAcmd(player) == FALSE ){
        SetStepWatchBit(opaw, PLAYER_BIT);
        MMDL_OffMoveBitMoveProcPause( player );
      }
      
      if( fmmdl != NULL ){
        if( MMDL_CheckMoveBitMove(fmmdl) == TRUE ){
          SetStepWatchBit(opaw, OTHER_BIT);
          MMDL_OffMoveBitMoveProcPause( fmmdl );
        }
      }

      if( player_pair ){
        #ifndef SCRCMD_PL_NULL
        if( SysFlag_PairCheck(SaveData_GetEventWork(fsys->savedata)) == 1
            && FieldOBJ_StatusBitCheck_Move(player_pair) != 0) {
          SetStepWatchBit(opaw, PLAYER_PAIR_BIT);
          FieldOBJ_MovePauseClear( player_pair );
        }
        #else
        GF_ASSERT( 0 );
        #endif
      }
      
      if( other_pair ){
        if( MMDL_CheckMoveBitMove(other_pair) == TRUE ){
          SetStepWatchBit(opaw, OTHER_PAIR_BIT);
          MMDL_OffMoveBitMoveProcPause( other_pair );
        }
      }
      ++ *seq;

    }
    break;

  case 1:
    {
      	MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
        MMDL *fmmdl = NULL;  //とりあえず
        //MMDL *player = PlayerGetMMdl( work );
        MMDL *player = MMDLSYS_SearchOBJID( fmmdlsys, MMDL_ID_PLAYER );
        MMDL *player_pair = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
        
     	//自機動作停止チェック
      if( CheckStepWatchBit(opaw, PLAYER_BIT) &&
        MMDL_CheckEndAcmd(player) == TRUE ){
        MMDL_OnMoveBitMoveProcPause( player );
        ResetStepWatchBit(opaw, PLAYER_BIT);
      }
      
      //話しかけ対象動作停止チェック
      if( CheckStepWatchBit(opaw, OTHER_BIT) &&
        MMDL_CheckMoveBitMove(fmmdl) == FALSE ){
        MMDL_OnMoveBitMoveProcPause( fmmdl );
        ResetStepWatchBit(opaw, OTHER_BIT);
      }
      
      //自機の連れ歩き動作停止チェック
      if( CheckStepWatchBit(opaw, PLAYER_PAIR_BIT) ){
        MMDL *player_pair = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
        
        if( MMDL_CheckMoveBitMove(player_pair) == FALSE ){
          MMDL_OnMoveBitMoveProcPause( player_pair );
          ResetStepWatchBit(opaw, PLAYER_PAIR_BIT);
        }
      }
      
      //話しかけ対象の連れ歩き動作停止チェック
      if( CheckStepWatchBit(opaw, OTHER_PAIR_BIT) ){
        #ifndef SCRCMD_PL_NULL
        MMDL *other_pair = FieldOBJ_MovePairSearch(*fldobj);
        if (FieldOBJ_StatusBitCheck_Move(other_pair) == 0) {
          FieldOBJ_MovePause(other_pair);
          ResetStepWatchBit(opaw, OTHER_PAIR_BIT);
        }
        #else
        GF_ASSERT( 0 );
        #endif
      }
      
      if( opaw->step_watch_bit == 0 ){
        ++ *seq;
      }
    }
    break;

  case 2:
    return GMEVENT_RES_FINISH;
    break;
  }

  return GMEVENT_RES_CONTINUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_ObjPauseAll(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap)
{
  OBJ_PAUSE_ALL_WORK * opaw;
  GMEVENT * event;

  event = GMEVENT_Create( gsys, NULL, EVENTFUNC_ObjPauseAll, sizeof(OBJ_PAUSE_ALL_WORK) );
  opaw = GMEVENT_GetEventWork(event);
  opaw->gsys = gsys;
  opaw->fieldmap = fieldmap;
  opaw->gamedata = GAMESYSTEM_GetGameData(gsys);

  return event;
}



//======================================================================
//======================================================================
#define SCRCMD_ACMD_MAX (8)
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  GAMEDATA * gamedata;
  u16 obj_id;
  void * anm_tbl;
	GFL_TCB *tcb_anm_tbl[SCRCMD_ACMD_MAX];
}OBJ_ANIME_EVENT_WORK;


static MMDL * FieldObjPtrGetByObjId( MMDLSYS * fmmdlsys, u16 obj_id );
static BOOL checkAnimeTCB( OBJ_ANIME_EVENT_WORK *work );
static void setAnimeTCB( OBJ_ANIME_EVENT_WORK *work, GFL_TCB *tcb );
static void initAnimeTCB( OBJ_ANIME_EVENT_WORK *work);
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENTFUNC_ObjAnime( GMEVENT * event, int *seq, void*work)
{
  OBJ_ANIME_EVENT_WORK * oaew = (OBJ_ANIME_EVENT_WORK *)work;
  MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( oaew->fieldmap );
	MMDL *fmmdl;
	GFL_TCB *anm_tcb;
  //int * num;
  switch (*seq)
  {
  case 0:
    fmmdl = FieldObjPtrGetByObjId( fmmdlsys, oaew->obj_id );
    
    //エラーチェック
    if( fmmdl == NULL )
    {
      OS_Printf( "obj_id = %d\n", oaew->obj_id );
      GF_ASSERT( (0) && "対象のフィールドOBJのポインタ取得失敗！" );
      return GMEVENT_RES_FINISH;				//08.06.12 プラチナで追加
    }
    
    //アニメーションコマンドリストセット
    anm_tcb = MMDL_SetAcmdList( fmmdl, (MMDL_ACMD_LIST*)oaew->anm_tbl );
    
    //TCBセット
    setAnimeTCB( work, anm_tcb );

    //アニメーションの数を足す
    //num = SCRIPT_GetMemberWork( sc, ID_EVSCR_ANMCOUNT );
    //(*num)++;
    
    ++ *seq;
    break;

  case 1:
    if (checkAnimeTCB(work) == FALSE)
    {
      return GMEVENT_RES_FINISH;
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_ObjAnime( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, u16 obj_id, void * anm_tbl )
{
  int i;
  GMEVENT * event;
  OBJ_ANIME_EVENT_WORK * oaew;
  event = GMEVENT_Create( gsys, NULL, EVENTFUNC_ObjAnime, sizeof(OBJ_ANIME_EVENT_WORK) );
  oaew = GMEVENT_GetEventWork( event );
  initAnimeTCB(oaew);
  oaew->gsys = gsys;
  oaew->fieldmap = fieldmap;
  oaew->gamedata = GAMESYSTEM_GetGameData(gsys);
  oaew->obj_id = obj_id;
  oaew->anm_tbl = anm_tbl;
  
  return event;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static const MMDL_ACMD_LIST anime_up_table[] = {
  AC_WALK_U_8F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_down_table[] = {
  AC_WALK_D_8F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_left_table[] = {
  AC_WALK_L_8F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_right_table[] = {
  AC_WALK_R_8F, 1,
  ACMD_END, 0
};

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_PlayerOneStepAnime( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap)
{
  const MMDL_ACMD_LIST * tbl;
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
  MMDL *fmmdl = FIELD_PLAYER_GetMMdl( player );
  u16 dir = MMDL_GetDirDisp( fmmdl );
  
  switch (dir) {
  case DIR_UP:  tbl = anime_up_table; break;
  case DIR_DOWN:  tbl = anime_down_table; break;
  case DIR_LEFT:  tbl = anime_left_table; break;
  case DIR_RIGHT:  tbl = anime_right_table; break;
  }
  
  return EVENT_ObjAnime(gsys, fieldmap, MMDL_ID_PLAYER, (void*)tbl);
}

//--------------------------------------------------------------
/**
 * FIELD_OBJ_PTRを取得
 * @param	fsys	FIELDSYS_WORK型のポインタ
 * @param	obj_id	OBJID
 * @retval	"FIELD_OBJ_PTR"
 */
//--------------------------------------------------------------
static MMDL * FieldObjPtrGetByObjId( MMDLSYS * fmmdlsys, u16 obj_id )
{
	MMDL *dummy;
	MMDL *fmmdl;
	//MMDLSYS *fmmdlsys;
	
	//fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
	
	//連れ歩きOBJ判別IDが渡された時
	if( obj_id == SCR_OBJID_MV_PAIR ){
		fmmdl = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
	//透明ダミーOBJ判別IDが渡された時
	}else if( obj_id == SCR_OBJID_DUMMY ){
		//SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
		//dummy = SCRIPT_GetMemberWork( sc, ID_EVSCR_DUMMY_OBJ );
	//対象のフィールドOBJのポインタ取得
	}else{
		fmmdl = MMDLSYS_SearchOBJID( fmmdlsys, obj_id );
	}
	
	return fmmdl;
}


//--------------------------------------------------------------
//
//--------------------------------------------------------------
static void initAnimeTCB( OBJ_ANIME_EVENT_WORK *work)
{
  int i;
  for (i = 0; i < SCRCMD_ACMD_MAX; i++) 
  {
    work->tcb_anm_tbl[i] = NULL;
  }
}

static void setAnimeTCB( OBJ_ANIME_EVENT_WORK *work, GFL_TCB *tcb )
{
	int i;
	for( i = 0; i < SCRCMD_ACMD_MAX; i++ ){
		if( work->tcb_anm_tbl[i] == NULL ){
			work->tcb_anm_tbl[i] = tcb;
			return;
		}
	}
	GF_ASSERT( 0 );
}

static BOOL checkAnimeTCB( OBJ_ANIME_EVENT_WORK *work )
{
	BOOL flag = FALSE;
	int i;
	for( i = 0; i < SCRCMD_ACMD_MAX; i++ ){
		if( work->tcb_anm_tbl[i] != NULL ){
			if( MMDL_CheckEndAcmdList(work->tcb_anm_tbl[i]) == TRUE ){
				MMDL_EndAcmdList( work->tcb_anm_tbl[i] );
				work->tcb_anm_tbl[i] = NULL;
			}else{
				flag = TRUE;
			}
		}
	}
	return( flag );
}

#if 0
//======================================================================
//	動作モデル	
//======================================================================
static void EvAnmSetTCB(
	SCRCMD_WORK *work, GFL_TCB *anm_tcb, MMDL_ACMD_LIST *list );

//--------------------------------------------------------------
/**
 * アニメーション
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"0"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjAnime( VMHANDLE *core, void *wk )
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
		return 0;				//08.06.12 プラチナで追加
	}
	
	//アニメーションコマンドリストセット
	p = (VM_CODE*)(core->adrs+pos);
	anm_tcb = MMDL_SetAcmdList( fmmdl, (MMDL_ACMD_LIST*)p );
	
	//アニメーションの数を足す
	num = SCRIPT_GetMemberWork( sc, ID_EVSCR_ANMCOUNT );
	(*num)++;
	
	//TCBセット
	EvAnmSetTCB( work, anm_tcb, NULL );
	return 0;
}

//--------------------------------------------------------------
/**
 * アニメーション終了待ち
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval	"1"
 */
//--------------------------------------------------------------
static VMCMD_RESULT EvCmdObjAnimeWait( VMHANDLE * core, void *wk )
{
	VMCMD_SetWait( core, EvObjAnimeWait );
	return 1;
}

//return 1 = 終了
static BOOL EvObjAnimeWait(VMHANDLE * core, void *wk )
{
	SCRCMD_WORK *work = wk;
	
	if( SCRCMD_WORK_CheckMMdlAnmTCB(work) == FALSE ){
		return 1;
	}
	
	return 0;
}

//--------------------------------------------------------------
/**
 * FIELD_OBJ_PTRを取得
 * @param	fsys	FIELDSYS_WORK型のポインタ
 * @param	obj_id	OBJID
 * @retval	"FIELD_OBJ_PTR"
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
 * アニメ終了監視TCB セット
 * @param	fsys	FIELDSYS_WORK型のポインタ
 * @param	anm_tcb	TCB_PTR型
 * @retval	none
 */
//--------------------------------------------------------------
static void EvAnmSetTCB(
	SCRCMD_WORK *work, GFL_TCB *anm_tcb, MMDL_ACMD_LIST *list )
{
#if 0
	EV_ANM_WORK* wk = NULL;
	wk = sys_AllocMemory(HEAPID_FIELD, sizeof(EV_ANM_WORK));

	if( wk == NULL ){
		GF_ASSERT( (0) && "scrcmd.c メモリ確保失敗！" );
		return;
	}

	wk->fsys	= fsys;
	wk->anm_tcb	= anm_tcb;
	wk->list	= list;
	wk->tcb		= TCB_Add( EvAnmMainTCB, wk, 0 );
	return;
#else
	SCRCMD_WORK_SetMMdlAnmTCB( work, anm_tcb );
#endif
}
#endif

