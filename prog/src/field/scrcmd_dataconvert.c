//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_dataconvert.c
 *	@brief  データコンバート用スクリプトコマンド
 *	@author	Toru=Nagihashi
 *	@data		2011.07.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//システム
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/vm_cmd.h"
#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "fieldmap.h"
#include "savedata/outside_permanent.h"
#include "event_fieldmap_control.h"

//外部公開
#include "scrcmd_dataconvert.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  フラグを取得
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs  スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdDataConv_GetFlag( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  DATA_CONVERT_SAVE *p_dataconv = SaveControl_GetDataConvert( p_sv );
  u16 flag    = SCRCMD_GetVMWorkValue( p_core, p_wk_adrs );
  u16 *p_ret  = SCRCMD_GetVMWork( p_core, p_scrcmd );
  *p_ret      = DATA_CONVERT_SAVE_GetFlag( p_dataconv, flag );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワークを取得
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs  スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdDataConv_GetWork( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  DATA_CONVERT_SAVE *p_dataconv = SaveControl_GetDataConvert( p_sv );
  u16 wk    = SCRCMD_GetVMWorkValue( p_core, p_wk_adrs );
  u16 *p_ret  = SCRCMD_GetVMWork( p_core, p_scrcmd );
  *p_ret      = DATA_CONVERT_SAVE_GetWork( p_dataconv, wk );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受け取っているかちぇっく
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs  スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdDataConv_IsExist( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );

  DATA_CONVERT_SAVE *p_dataconv = SaveControl_GetDataConvert( p_sv );

  u16 id    = SCRCMD_GetVMWorkValue( p_core, p_wk_adrs );
  u16 *p_ret  = SCRCMD_GetVMWork( p_core, p_scrcmd );

  *p_ret = DATA_CONVERT_SAVE_IsExist( p_dataconv, id );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  コンバート元持ち主名を単語登録
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs  スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdDataConv_WordSetOwnerName( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  DATA_CONVERT_SAVE *p_dataconv = SaveControl_GetDataConvert( p_sv );
  WORDSET *p_wordset    = SCRIPT_GetWordSet( p_sc );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( p_scrcmd );
  u16 id    = SCRCMD_GetVMWorkValue( p_core, p_wk_adrs );
  u16 buffIdx   = SCRCMD_GetVMWorkValue( p_core, p_wk_adrs );

  u8 sex = DATA_CONVERT_SAVE_GetSex( p_dataconv, id );
  const STRCODE *cp_name = (const STRCODE *)DATA_CONVERT_SAVE_GetName( p_dataconv, id );
  STRBUF *p_str = GFL_STR_CreateBuffer( 64, GFL_HEAP_LOWID(heapID) );
  GFL_STR_SetStringCodeOrderLength(  p_str, cp_name, PERSON_NAME_SIZE+EOM_SIZE );

  GF_ASSERT( DATA_CONVERT_SAVE_IsExist( p_dataconv, id ) );

  WORDSET_RegisterWord( p_wordset, buffIdx, p_str, sex, TRUE, PM_LANG );

  GFL_STR_DeleteBuffer( p_str );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ミュージカルをコンバートしているかチェック
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs  スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdDataConv_GetData( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  MUSICAL_SAVE *p_musical = MUSICAL_SAVE_GetMusicalSave( p_sv );
  DATA_CONVERT_SAVE *p_dataconv = SaveControl_GetDataConvert( p_sv );
  u16 param    = SCRCMD_GetVMWorkValue( p_core, p_wk_adrs );
  u16 *p_ret  = SCRCMD_GetVMWork( p_core, p_scrcmd );

  switch( param )
  {
  case DATACONV_GET_IS_CONVERT_MUSICAL:
    *p_ret = MUSICAL_SAVE_IsConvertItem( p_musical );
    break;
  case DATACONV_GET_OVER_SCORE:
    *p_ret = ( DATA_CONVERT_SAVE_GetData( p_dataconv, DATA_CONVERT_GET_ID_MUSICAL_POINT, NULL ) < MUSICAL_SAVE_GetSumPoint( p_musical ) );
    break;
  default:
    GF_ASSERT(0);
  }

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ミュージカルをコンバート
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs  スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdDataConv_ConvertMusical( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );

  DATA_CONVERT_SAVE *p_dataconv = SaveControl_GetDataConvert( p_sv );
  MUSICAL_SAVE *p_musical = MUSICAL_SAVE_GetMusicalSave( p_sv );

  u16 *p_ret  = SCRCMD_GetVMWork( p_core, p_scrcmd );

  const u8 *cp_item_bit = (const u8 *)DATA_CONVERT_SAVE_GetData( p_dataconv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, NULL );
  int i;

  *p_ret = FALSE;

  //アイテムコンバート
  for( i = 0; i < MUSICAL_ITEM_MAX_REAL; i++ )
  {
    if( cp_item_bit[i/8] & (1<<(i%8)) )
    {
      if( MUSICAL_SAVE_ChackHaveItem( p_musical, i ) == FALSE )
      {
        OS_TPrintf( "musical item[%d] add!", i );
        MUSICAL_SAVE_AddItem( p_musical, i );
        *p_ret = TRUE;
      }
    }
  }

  //コンバート完了
  MUSICAL_SAVE_SetConvertItem( p_musical, TRUE );

  return VMCMD_RESULT_CONTINUE;
}
