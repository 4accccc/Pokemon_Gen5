//======================================================================
/**
 * @file  scrcmd_poke_win.c
 * @brief  ポケモン表示ウィンドウスクリプトコマンド用関数
 * @author  Nozomu Satio
 *
 */
//======================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "system/poke2dgra.h"       //for POKE2GRA

#include "fieldmap.h"
#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "event_fieldmap_control.h"

#include "scrcmd_poke_win.h"

typedef struct POKE_WIN_WORK_tag{
  GAMESYS_WORK * gsys;
  GAMEDATA * gdata;
  FIELDMAP_WORK * fieldWork;

	GFL_BMPWIN * pokeWin;

	GFL_CLUNIT * ClUnit;
	GFL_CLWK * ClWk;
	u32	ChrRes;
	u32	PalRes;
	u32	CelRes;

}POKE_WIN_WORK;

static void InitPokeObj( POKE_WIN_WORK * wk,
    const int inMonsNo, const int inFormNo, const int inSex, const BOOL inRare, const int inRnd );
static void DelPokeObj( POKE_WIN_WORK * wk );
static GMEVENT_RESULT PokeWinEvt( GMEVENT* event, int* seq, void* work );

//--------------------------------------------------------------
/**
 * ポケモンウィンドウ出す
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdDispPokeWin( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GMEVENT *call_event;
  POKE_WIN_WORK *evt_wk;

  u16 monsno = SCRCMD_GetVMWorkValue( core, wk );  // コマンド第1引数
  u16 form = SCRCMD_GetVMWorkValue( core, wk );  // コマンド第2引数
  u16 sex = SCRCMD_GetVMWorkValue( core, wk );  // コマンド第3引数
  u16 rare = SCRCMD_GetVMWorkValue( core, wk );  // コマンド第4引数

  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  call_event = GMEVENT_Create( gsys, NULL, PokeWinEvt, sizeof(POKE_WIN_WORK) );

  evt_wk = GMEVENT_GetEventWork(call_event);
  InitPokeObj(evt_wk, monsno, form, sex, rare, 0);

  SCRIPT_CallEvent( sc, call_event );

  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * ポケモンOBJ作成
 * @param   wk        ワークポインタ
 * @param   inMonsNo  モンスターナンバー
 * @param   inFormNo  フォルムナンバー
 * @param   inSex     性別
 * @param   inRare    レア
 * @param   inRnd     個性乱数  スクリプトではu16の値受け渡しが限界なので現行は0を入れる
 * @retval  none
 */
//--------------------------------------------------------------
static void InitPokeObj( POKE_WIN_WORK * wk,
    const int inMonsNo, const int inFormNo, const int inSex, const BOOL inRare, const int inRnd )
{
  ARCHANDLE * ah;
  HEAPID heapID;

  heapID = GFL_HEAP_LOWID(HEAPID_FIELDMAP);

	wk->ClUnit = GFL_CLACT_UNIT_Create( 1, 0, heapID );

	ah = POKE2DGRA_OpenHandle( heapID );

	wk->ChrRes = POKE2DGRA_OBJ_CGR_Register(
      ah, inMonsNo, inFormNo, inSex, inRare, POKEGRA_DIR_FRONT, FALSE, inRnd, CLSYS_DRAW_MAIN, heapID);
	wk->PalRes = POKE2DGRA_OBJ_PLTT_Register(
      ah, inMonsNo, inFormNo, inSex, inRare, POKEGRA_DIR_FRONT, FALSE, CLSYS_DRAW_MAIN, 0, heapID);

  GFL_ARC_CloseDataHandle( ah );
	
  wk->CelRes = POKE2DGRA_OBJ_CELLANM_Register(
      inMonsNo, inFormNo, inSex, inRare, POKEGRA_DIR_FRONT, FALSE, APP_COMMON_MAPPING_64K, CLSYS_DRAW_MAIN, heapID);

	{
		GFL_CLWK_DATA	dat = { 128, 72, 0, 0, 0 };
		wk->ClWk = GFL_CLACT_WK_Create(
								wk->ClUnit,
								wk->ChrRes, wk->PalRes, wk->CelRes,
								&dat, CLSYS_DRAW_MAIN, heapID );
	}
}

//--------------------------------------------------------------
/**
 * ポケモンOBJ解放
 * @param  wk    ワークポインタ
 * @retval none
 */
//--------------------------------------------------------------
static void DelPokeObj( POKE_WIN_WORK * wk )
{
  GFL_CLACT_WK_Remove( wk->ClWk );
	GFL_CLGRP_CGR_Release( wk->ChrRes );
	GFL_CLGRP_PLTT_Release( wk->PalRes );
	GFL_CLGRP_CELLANIM_Release( wk->CelRes );
	GFL_CLACT_UNIT_Delete( wk->ClUnit );
}

//--------------------------------------------------------------
/**
 * ポケモンウィンドウ表示イベント
 * @param   event　　　       イベントポインタ
 * @param   seq　　　　       シーケンス
 * @param   work　　　　      ワークポインタ
 * @return  GMEVENT_RESULT    GMEVENT_RES_FINISHでイベント終了
 */
//--------------------------------------------------------------
static GMEVENT_RESULT PokeWinEvt( GMEVENT* event, int* seq, void* work )
{
  POKE_WIN_WORK *evt_wk = (POKE_WIN_WORK*)work;

  if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    //解放処理
    DelPokeObj( evt_wk );
    //終了
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

