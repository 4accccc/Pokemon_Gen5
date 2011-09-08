//============================================================================================
/**
 * @file  options_savedata.c
 * @author  tamada
 * @date    2011.08.24
 * @brief ユーザーが設定する細かいセーブデータを集めるところ
 */
//============================================================================================

#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/options_local.h"

//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief
 */
//----------------------------------------------------------
int OPTIONS_SAVEDATA_GetWorkSize( void )
{
  return sizeof(OPTIONS_SAVEDATA);
}

//----------------------------------------------------------
/**
 * @brief
 */
//----------------------------------------------------------
void OPTIONS_SAVEDATA_Init( OPTIONS_SAVEDATA * sv )
{
  GFL_STD_MemClear( sv, OPTIONS_SAVEDATA_GetWorkSize() );
  MYPMS_Init( &sv->mypms_data );
  CGEAR_SV_Init( &sv->cgear_savedata );
  SHORTCUT_Init( &sv->shortcut );
  BOAST_MSG_SAVEDATA_Init( &sv->boast_message );
}

