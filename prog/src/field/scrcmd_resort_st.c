//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_resort_st.c
 *	@brief  レゾナンスリゾート常駐スクリプト
 *	@author	Toru=Nagihashi
 *	@data		2011.09.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
#include "system/vm_cmd.h"
#include "fieldmap.h"
#include "scrcmd_work.h"
#include "scrcmd.h"

#include "scrcmd_resort_st.h"


FS_EXTERN_OVERLAY( scrcmd_resort_common );
FS_EXTERN_OVERLAY( scrcmd_resort_shop );

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
 *	@brief  オーバーレイ切り替え  通常から店用へ
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortChangeOverlayCommonToShop( VMHANDLE *p_core, void *p_wk_adrs )
{
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( scrcmd_resort_common ) );
  GFL_OVERLAY_Load( FS_OVERLAY_ID( scrcmd_resort_shop ) );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  オーバーレイ切り替え  店用から通常へ
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortChangeOverlayShopToCommon( VMHANDLE *p_core, void *p_wk_adrs )
{
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( scrcmd_resort_shop ) );
  GFL_OVERLAY_Load( FS_OVERLAY_ID( scrcmd_resort_common ) );

  return VMCMD_RESULT_CONTINUE;
}


