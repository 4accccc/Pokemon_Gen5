//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_medalinfo.c
 *	@brief  メダル詳細画面呼び出し
 *	@author	Masayuki Onoue
 *	@date		2011.05.09
 *
 *	@date   2011.05.12  リゾート用に改造 Toru Nagihashi
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "../fieldmap.h"

#include "../script.h"
#include "../script_local.h"
#include "../script_def.h"
#include "../scrcmd.h"
#include "../scrcmd_work.h"
#include "scrcmd_medalinfo.h"

#include "../event_fieldmap_control.h"

#include "app/medal_info.h"

#include "savedata/resonance_resort_sv.h"
#include "resort_people.h"
#include "resort_binary.h"
#include "resort_reception.h"
#include "resort_field.h"
#include "../gmk_tmp_wk.h"

//#define VOLUMECONTROL // アプリ起動時の音量調整(OFFだとボリュームの調整を行いません

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void MedalInfoEndCallback( void* p_work );

//scrcmd_resort.cのソースを使う
extern RESORT_PEOPLE * Resort_Scrcmd_GetFrontPeople( FIELDMAP_WORK *p_fieldmap, RESORT_PEOPLE_SYS *p_people_sys );
static RESORT_PEOPLE * Resort_ScrcmdMedalInfo_GetFrontPeople( FIELDMAP_WORK *p_fieldmap );

//-------------------------------------
///	
//=====================================
typedef struct {
  MEDALINFO_INIT_PARAM p_medal_info_wk;
  GAMESYS_WORK        *p_gsys;
  GAMEDATA            *p_gamedata;
  STRBUF              *p_tr_name;
} SCR_MEDAL_INFO_WK;
//--------------------------------------------------------------
/**
 * メダル詳細画面呼び出し
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCallMedalInfo( VMHANDLE *core, void *wk )
{
  MEDALINFO_INIT_PARAM *p_medal_info_wk;
  SCRCMD_WORK   *p_work     = (SCRCMD_WORK*)wk;
  GAMESYS_WORK  *p_gsys     = SCRCMD_WORK_GetGameSysWork( p_work );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gsys );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gsys );
  SCRIPT_WORK   *p_sc       = SCRCMD_WORK_GetScriptWork( p_work );
  STRCODE       name[PERSON_NAME_SIZE+EOM_SIZE];
  
  u16 medal_id;
  u16 get_year;
  u16 get_month;
  u16 get_day;
  u16 medal_cnt;
  u16 box_rank;
  GMEVENT *p_ev; 

  SCR_MEDAL_INFO_WK *p_call_wk = GFL_HEAP_AllocClearMemory( HEAPID_PROC, sizeof(SCR_MEDAL_INFO_WK) );
  p_medal_info_wk = &p_call_wk->p_medal_info_wk;

  //自分のからメダル情報を取得
  {
    RESORT_PEOPLE *p_people = Resort_ScrcmdMedalInfo_GetFrontPeople( p_fieldmap );
    RESORT_DATE date = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_EXT1_RECOMMEND_DATE, NULL );

    RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_NAME, name );
    medal_id = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_EXT1_RECOMMEND_MEDAL_ID, NULL );
    get_year  = RESORT_DATE_GET_YEAR(date);
    get_month = RESORT_DATE_GET_MONTH(date);
    get_day   = RESORT_DATE_GET_DAY(date);
    medal_cnt = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_EXT1_MEDAL_NUM, NULL );
    box_rank  = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_EXT1_MEDAL_BOX_RANK, NULL );

  }

  // 名前の設定
  p_call_wk->p_tr_name = GFL_STR_CreateBuffer( PERSON_NAME_SIZE + EOM_SIZE, HEAPID_PROC );
  GFL_STR_SetStringCode( p_call_wk->p_tr_name , name);

  // パラメータの設定
  p_medal_info_wk->p_trainer_name = p_call_wk->p_tr_name;
  p_medal_info_wk->medal_id       = medal_id;
  p_medal_info_wk->get_year       = get_year;
  p_medal_info_wk->get_month      = get_month;
  p_medal_info_wk->get_day        = get_day;
  p_medal_info_wk->medal_cnt      = medal_cnt;
  p_medal_info_wk->box_rank       = box_rank;
  p_medal_info_wk->parent_heap_id = GFL_HEAPID_APP;
  p_medal_info_wk->call_mode      = MEDALINFO_CALL_TOP;

   
  p_call_wk->p_gamedata = p_gamedata;
  p_call_wk->p_gsys     = p_gsys;

#ifdef VOLUMECONTROL
  // 音量を下げる
  FSND_HoldBGMVolume_forApp(GAMEDATA_GetFieldSound(p_gamedata), GAMESYSTEM_GetIssSystem(p_gsys) );
#endif
  
  // イベントの作成
  p_ev = EVENT_FieldSubProc_Callback(p_gsys ,p_fieldmap ,FS_OVERLAY_ID(medalinfo), &medalInfo_ProcData, p_medal_info_wk, MedalInfoEndCallback, p_call_wk);

  SCRIPT_CallEvent( p_sc ,p_ev );
  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ開放処理
 *
 *	@param	p_work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MedalInfoEndCallback( void* p_work )
{
  SCR_MEDAL_INFO_WK *p_callWork = p_work;
 
#ifdef VOLUMECONTROL
  // 音量を元に戻す
  FSND_ReleaseBGMVolume_fromApp(GAMEDATA_GetFieldSound(p_callWork->p_gamedata), GAMESYSTEM_GetIssSystem(p_callWork->p_gsys) );
#endif

  GFL_STR_DeleteBuffer( p_callWork->p_tr_name );
  GFL_HEAP_FreeMemory( p_callWork );
}

//----------------------------------------------------------------------------
/**
 *	@brief  目の前の人物を取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 *
 *	@return 目の前の人物を取得
 */
//-----------------------------------------------------------------------------
static RESORT_PEOPLE * Resort_ScrcmdMedalInfo_GetFrontPeople( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_PEOPLE_SYS *p_people_sys = NULL;
  {

    if(GMK_TMP_CheckAssignID( p_fieldmap, RESORT_RECEPTION_ASSIGN_ID ) )
    {
      p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_fieldmap );
    }
    else if( GMK_TMP_CheckAssignID( p_fieldmap, RESORT_FIELD_ASSIGN_ID ) )
    {
      p_people_sys = RESORT_FIELD_GetPeopleSys( p_fieldmap );
    }
    GF_ASSERT( p_people_sys != NULL );
  }

  return Resort_Scrcmd_GetFrontPeople( p_fieldmap, p_people_sys );
}
