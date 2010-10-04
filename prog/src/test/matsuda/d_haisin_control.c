//==============================================================================
/**
 * @file    d_haisin_control.c
 * @brief   簡単な説明を書く
 * @author  matsuda
 * @date    2010.08.18(水)
 */
//==============================================================================
#include <gflib.h>
#ifdef PM_DEBUG
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include <backup_system.h>
//#include "savedata/contest_savedata.h"
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"

#include "net\network_define.h"

#include "print\printsys.h"
#include "print\gf_font.h"
#include "arc_def.h"
#include "message.naix"
#include "debug_message.naix"
#include "test_graphic\d_taya.naix"
#include "msg\debug\msg_d_matsu.h"
#include "msg\msg_power.h"
#include "test/performance.h"
#include "font/font.naix"

#include "system/bmp_menu.h"
#include "system/bmp_menuwork.h"
#include "system/bmp_menulist.h"
#include "print/wordset.h"

#include "d_haisin_power.h"
#include "app/name_input.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"


//==============================================================================
//  
//==============================================================================
extern void GPowerHaisin_SendBeacon_Init(const STRBUF *name, u16 trainer_id, const STRBUF *selfmsg, u8 union_index);

//==============================================================================
//  外部データ
//==============================================================================
extern const GFL_PROC_DATA HaisinMenuProcData;
extern GFL_PROC_DATA DebugGPowerFuncProcData;

FS_EXTERN_OVERLAY(namein);
FS_EXTERN_OVERLAY(ohno_debug);

//==============================================================================
//  ローカル変数
//==============================================================================
static NAMEIN_PARAM *namein_param;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
void HaisinData_Save(HAISIN_CONTROL_WORK *control);
BOOL HaisinData_Load(HAISIN_CONTROL_WORK *control);


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT HaisinControlProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	HAISIN_CONTROL_WORK* control;
	GFL_MSGDATA		*mm;
	int i;
	
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GOTO_DEBUG, 0x1000 );
	control = GFL_PROC_AllocWork( proc, sizeof(HAISIN_CONTROL_WORK), HEAPID_GOTO_DEBUG );
	MI_CpuClear8(control, sizeof(HAISIN_CONTROL_WORK));

  control->name = GFL_STR_CreateBuffer(64, HEAPID_GOTO_DEBUG);
  control->selfmsg = GFL_STR_CreateBuffer(64, HEAPID_GOTO_DEBUG);
	mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_d_matsu_dat, HEAPID_GOTO_DEBUG );
  GFL_MSG_GetString( mm, DM_MSG_HAISIN_000, control->name );
  GFL_MSG_GetString( mm, DM_MSG_HAISIN_005, control->selfmsg );
  GFL_MSG_Delete(mm);

  for(i = 0; i < HAISIN_POWER_MAX; i++){
    control->powergroup.hp[i].g_power_id = GPOWER_ID_NULL;
  }

  control->powergroup.start_time_hour = 8;
  control->powergroup.start_time_min = 0;
  control->powergroup.end_time_hour = 20;
  control->powergroup.end_time_min = 0;
  control->powergroup.movemode = 0;
  
  control->powergroup.beacon_space_time = 3;
  
  //ロード
  control->bsavedata = HaisinData_Load(control);
  
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT HaisinControlProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	HAISIN_CONTROL_WORK* control = mywk;
	enum{
    SEQ_MENU,
    SEQ_MENU_RETURN,
    SEQ_NAMEIN,
    SEQ_NAMEIN_RETURN,
    SEQ_START,
    SEQ_START_RETURN,
  };
  
	switch(*seq){
	case SEQ_MENU:
    if( ( GFL_UI_KEY_GetCont() == (PAD_BUTTON_L|PAD_BUTTON_R|PAD_BUTTON_X) ) ||
        (control->bsavedata==FALSE)){
      control->bsavedata = FALSE;
      GFL_PROC_SysCallProc( NO_OVERLAY_ID, &HaisinMenuProcData, control );
    }
    else{
      control->return_mode = HAISIN_RETURN_MODE_START;
    }
    (*seq)++;
	  break;
	case SEQ_MENU_RETURN:
	  switch(control->return_mode){
	  case HAISIN_RETURN_MODE_NAMEIN:
	  case HAISIN_RETURN_MODE_SELFMSG:
	    *seq = SEQ_NAMEIN;
	    break;
	  case HAISIN_RETURN_MODE_START:
	    (*seq) = SEQ_START;
	    break;
	  default:
	    OS_ResetSystem(0);
	    break;
    }
    break;
  
  case SEQ_NAMEIN:
    {
      MISC      *p_misc     = SaveData_GetMisc( SaveControl_GetPointer() );
      int msg_len;
      
      if(control->return_mode == HAISIN_RETURN_MODE_NAMEIN){
        msg_len = 5;
      }
      else{
        msg_len = 8;//GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN;
      }
      namein_param	= NAMEIN_AllocParam( HEAPID_GOTO_DEBUG,
      																			NAMEIN_MYNAME,
      																		  NAMEIN_TRAINER_VIEW_UNKNOWN, 0,
      																			msg_len, NULL, p_misc );

      GFL_PROC_SysCallProc( FS_OVERLAY_ID(namein), &NameInputProcData, namein_param );
    }
    (*seq)++;
    break;
  case SEQ_NAMEIN_RETURN:
    if(control->return_mode == HAISIN_RETURN_MODE_NAMEIN){
      GFL_STR_CopyBuffer(control->name, namein_param->strbuf);
    }
    else{
      GFL_STR_CopyBuffer(control->selfmsg, namein_param->strbuf);
    }
    NAMEIN_FreeParam(namein_param);
  	GX_SetMasterBrightness(0);
  	GXS_SetMasterBrightness(0);
    *seq = SEQ_MENU;
    break;
    
  case SEQ_START:
    {
      int i, s;
      //データを前詰め
      for(i = 0; i < HAISIN_POWER_MAX; i++){
        if(control->powergroup.hp[i].g_power_id == GPOWER_ID_NULL){
          for(s = i+1; s < HAISIN_POWER_MAX; s++){
            if(control->powergroup.hp[s].g_power_id < GPOWER_ID_MAX){
              control->powergroup.hp[i].g_power_id = control->powergroup.hp[s].g_power_id;
              control->powergroup.hp[s].g_power_id = GPOWER_ID_NULL;
              break;
            }
          }
        }
      }
    }

    {
      u16 trainer_id = GFUser_GetPublicRand(0xffff);
      if(trainer_id == 0){
        trainer_id = 1;
      }
      GPowerHaisin_SendBeacon_Init(control->name, trainer_id, control->selfmsg, control->union_index);
    }

	  GFL_PROC_SysCallProc( FS_OVERLAY_ID(ohno_debug), &DebugGPowerFuncProcData, &control->powergroup );
    (*seq)++;
    break;
  case SEQ_START_RETURN:
    (*seq) = SEQ_MENU;
    break;
  }
	
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT HaisinControlProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	HAISIN_CONTROL_WORK* control = mywk;

  OS_ResetSystem(0);
	
	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_GOTO_DEBUG);
	
	return GFL_PROC_RES_FINISH;
}


//==============================================================================
//  セーブ・ロード
//==============================================================================
//==================================================================
/**
 * 現在の設定でセーブを実行する
 *
 * @param   control		
 */
//==================================================================
void HaisinData_Save(HAISIN_CONTROL_WORK *control)
{
  HAISIN_SAVEDATA save_data;
  
  GFL_STD_MemClear(&save_data, sizeof(HAISIN_SAVEDATA));

  GFL_STR_GetStringCode( control->name, save_data.name, SAVELEN_PLAYER_NAME + EOM_SIZE);
  GFL_STR_GetStringCode( control->selfmsg, save_data.selfmsg, 8 + EOM_SIZE);
  save_data.union_index = control->union_index;
  save_data.powergroup = control->powergroup;
  save_data.beacon_space_time = control->powergroup.beacon_space_time;
  
  save_data.magic_key = HAISIN_SAVE_MAGICKEY;
  
  GFL_BACKUP_DirectFlashSave(0, &save_data, sizeof(HAISIN_SAVEDATA));
  PMSND_PlaySE(SEQ_SE_SAVE);
}

//==================================================================
/**
 * セーブデータがあればロードを行う
 *
 * @param   control		
 *
 * @retval  BOOL		TRUE:ロード成功　FALSE：データ無し
 */
//==================================================================
BOOL HaisinData_Load(HAISIN_CONTROL_WORK *control)
{
  HAISIN_SAVEDATA load_data;
  BOOL ret;
  
  ret = GFL_BACKUP_DirectFlashLoad(0, &load_data, sizeof(HAISIN_SAVEDATA));

  if(ret == FALSE || load_data.magic_key != HAISIN_SAVE_MAGICKEY){
    return FALSE;
  }

  GFL_STR_SetStringCode( control->name, load_data.name );
  GFL_STR_SetStringCode( control->selfmsg, load_data.selfmsg );
  control->union_index = load_data.union_index;
  control->powergroup = load_data.powergroup;
  control->powergroup.beacon_space_time = load_data.beacon_space_time;
  
  {
    int i,power_count=0;
    for(i = 0; i < HAISIN_POWER_MAX; i++){
      if(control->powergroup.hp[i].g_power_id < GPOWER_ID_MAX){
        power_count++;
      }
    }
    if(power_count != 0){
      control->powergroup.data_num = power_count;
    }
  }

  return TRUE;
}


//==============================================================================
//	
//==============================================================================
//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA HaisinControlProcData = {
	HaisinControlProcInit,
	HaisinControlProcMain,
	HaisinControlProcEnd,
};

#endif //PM_DEBUG

