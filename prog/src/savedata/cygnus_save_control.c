//============================================================================================
/**
 * @file  cygnus_save_control.c
 * @brief   3DS連動：CYGNUS-SWAN共有領域へのアクセス制御
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

//WriteFlashで1度に書き込める最小単位256バイト以内に収めておく
SDK_COMPILER_ASSERT( sizeof(CYGNUS_SAVEDATA) < 256 );

static CYGNUS_SAVEDATA sCygnusSavedata = {0};

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
void CYGNUSSAVE_CONTROL_Load( CYGNUS_SAVE_CONTROL * cygsave_ctrl )
{
  BOOL result;
  result = GFL_BACKUP_DirectFlashLoad(
      CYGSAVE_FLASH_ADRS, &sCygnusSavedata, sizeof(CYGNUS_SAVEDATA) );
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
        CYGSAVE_FLASH_ADRS, &sCygnusSavedata, sizeof(CYGNUS_SAVEDATA) );
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

//----------------------------------------------------------
/**
 * @brief CYGNUSセーブデータの書き込み
 */
//----------------------------------------------------------
void CYGNUSSAVE_CONTROL_Save( void )
{
  GFL_BACKUP_DirectFlashSave( CYGSAVE_FLASH_ADRS, &sCygnusSavedata, sizeof(CYGNUS_SAVEDATA) );
}

//----------------------------------------------------------
/**
 * @brief CYGNUSセーブデータの削除
 */
//----------------------------------------------------------
void CYGNUSSAVE_CONTROL_Erase( HEAPID heapID )
{
  CYGNUS_SAVEDATA * empty_data = GFL_HEAP_AllocClearMemory( heapID, sizeof(CYGNUS_SAVEDATA) );
  GFL_BACKUP_DirectFlashSave( CYGSAVE_FLASH_ADRS, empty_data, sizeof(CYGNUS_SAVEDATA) );
  GFL_HEAP_FreeMemory( empty_data );
}

//============================================================================================
//============================================================================================
//----------------------------------------------------------
//----------------------------------------------------------
CYGNUS_SAVEDATA * CYGNUSSAVE_CONTROL_GetPtr( CYGNUS_SAVE_CONTROL * cygsave_ctrl )
{
  return &sCygnusSavedata;
}

//----------------------------------------------------------
//----------------------------------------------------------
static BOOL CYGNUSSAVE_CONTROL_CheckSaveData( const CYGNUS_SAVE_CONTROL * cygsave_ctrl )
{
  CYGSAVE_STATE state = CYGNUS_SAVEDATA_GetStatus( &sCygnusSavedata );
  return (state != CYGSAVE_STATE_NON );
}

//----------------------------------------------------------
/**
 * @brief
 */
//----------------------------------------------------------
BOOL CYGNUSSAVE_CONTROL_CheckSaveDataDirect( HEAPID heapID )
{
  CYGNUS_SAVE_CONTROL * ctrl;
  BOOL result;
  ctrl = CYGNUSSAVE_CONTROL_Create( heapID );
  CYGNUSSAVE_CONTROL_Load( ctrl );
  result = CYGNUSSAVE_CONTROL_CheckSaveData( ctrl );
  CYGNUSSAVE_CONTROL_Delete( ctrl );
  return result;
}

//----------------------------------------------------------
/// CYGNUS_SAVEDATAをDUMMY3DSと同期した状態にする
//----------------------------------------------------------
static void CYGNUSSAVE_CONTROL_Sync( SAVE_CONTROL_WORK * sv_ctrl )
{
  CYGNUS_SAVEDATA * cygsave = &sCygnusSavedata;
  DUMMY3DS_SAVEDATA * d3ds = SaveData_GetDummy3ds( sv_ctrl );
  u32 swan_flag   = DUMMY3DS_SAVEDATA_GetData( d3ds, DUMMY3DS_SAVEDATA_PARAM_FLAG);
  u32 onetime_key = DUMMY3DS_SAVEDATA_GetData( d3ds, DUMMY3DS_SAVEDATA_PARAM_ONETIME_KEY);
  //BOOL played_flag = DUMMY3DS_SAVEDATA_GetData( d3ds, DUMMY3DS_SAVEDATA_PARAM_IS_PLAYED );
  CYGNUS_SAVEDATA_SetSend( cygsave, onetime_key, swan_flag );
}

//----------------------------------------------------------
/// 内部データを反映したCYGNUS_SAVEDATAをバックアップ領域に書き込む
//----------------------------------------------------------
void CYGNUSSAVE_CONTROL_SyncBackup( SAVE_CONTROL_WORK * sv_ctrl )
{
  CYGNUSSAVE_CONTROL_Sync( sv_ctrl );
  CYGNUSSAVE_CONTROL_Save();
}



