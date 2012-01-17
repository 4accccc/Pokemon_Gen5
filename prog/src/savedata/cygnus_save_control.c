//============================================================================================
/**
 * @file  cygnus_save_control.c
 * @brief   3DS連動：内部確保領域
 * @date    2012.01.16
 * @author  tamada
 */
//============================================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "savedata/save_control.h"
#include "savedata/cygnus_savedata.h"
#include "savedata/cygnus_save_control.h"


//============================================================================================
//============================================================================================
//----------------------------------------------------------
//----------------------------------------------------------
struct _CYGNUS_SAVE_CONTROL{
  u8 cygsave_state;
  u8 save_seq;
  u16 lock_id;

  CYGNUS_SAVEDATA savedata;
};

//============================================================================================
//============================================================================================
//----------------------------------------------------------
//----------------------------------------------------------
CYGNUS_SAVE_CONTROL * CYGNUSSAVE_CONTROL_Create( HEAPID heapID )
{
  CYGNUS_SAVE_CONTROL * cyg_ctrl;
  cyg_ctrl = GFL_HEAP_AllocClearMemory( heapID, sizeof(CYGNUS_SAVE_CONTROL) );
  return cyg_ctrl;
}

//----------------------------------------------------------
//----------------------------------------------------------
void CYGNUSSAVE_CONTROL_Delete( CYGNUS_SAVE_CONTROL * cygsave_ctrl )
{
  GFL_HEAP_FreeMemory( cygsave_ctrl );
}

//----------------------------------------------------------
//----------------------------------------------------------
void CYGNUSSAVE_CONTROL_Load( CYGNUS_SAVE_CONTROL * cygsave_ctrl, u32 onetime_key )
{
  BOOL result;
  result = GFL_BACKUP_DirectFlashLoad(
      CYGSAVE_FLASH_ADRS, &cygsave_ctrl->savedata, sizeof(CYGNUS_SAVEDATA) );
}

//----------------------------------------------------------
//----------------------------------------------------------
void CYGNUSSAVE_CONTROL_SaveAsyncInit( CYGNUS_SAVE_CONTROL * cygsave_ctrl )
{
  //スリープ禁止
  GFL_UI_SleepDisable(GFL_UI_SLEEP_EXTRA_SAVE);
  //リセット禁止
  GFL_UI_SoftResetDisable(GFL_UI_SOFTRESET_SVLD);

  cygsave_ctrl->save_seq = 0;
}

//----------------------------------------------------------
//----------------------------------------------------------
BOOL CYGNUSSAVE_CONTROL_SaveAsyncMain( CYGNUS_SAVE_CONTROL * cygsave_ctrl )
{
  BOOL error_result;

  switch (cygsave_ctrl->save_seq)
  {
  case 0:
    cygsave_ctrl->lock_id = GFL_BACKUP_DirectFlashSaveAsyncInit(
        CYGSAVE_FLASH_ADRS, &cygsave_ctrl->savedata, sizeof(CYGNUS_SAVEDATA) );
    cygsave_ctrl->save_seq ++;
    break;
  case 1:
    if ( GFL_BACKUP_DirectFlashSaveAsyncMain(cygsave_ctrl->lock_id, &error_result ) == TRUE )
    {
      cygsave_ctrl->save_seq ++;
    }
    break;
  case 2:
    GFL_UI_SoftResetEnable(GFL_UI_SOFTRESET_SVLD);
    GFL_UI_SleepEnable(GFL_UI_SLEEP_EXTRA_SAVE);
    return TRUE;  //セーブ完了
  }
  return FALSE; //セーブ継続中
}


//============================================================================================
//============================================================================================
//----------------------------------------------------------
//----------------------------------------------------------
CYGNUS_SAVEDATA * CYGNUSSAVE_CONTROL_GetPtr( CYGNUS_SAVE_CONTROL * cygsave_ctrl )
{
  return &cygsave_ctrl->savedata;
}

