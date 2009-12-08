//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：Wi-Fi関連(常駐)
 * @file   scrcmd_wifi_st.h
 * @author iwasawa
 * @date   2009.12.08
 */
//////////////////////////////////////////////////////////////////////

#pragma once

//--------------------------------------------------------------------
/**
 * @brief   GeoNetを呼び出す
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdWifiGeoNetCall( VMHANDLE* core, void* wk );

//--------------------------------------------------------------------
/**
 * @brief   Wi-Fiランダムマッチを呼び出す
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdWifiRandomMatchEventCall( VMHANDLE* core, void* wk );

//--------------------------------------------------------------------
/**
 * @brief   バトルレコーダーイベントを呼び出す
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 *
 * 呼び出しモード指定 field/script_def.h
 *  SCRCMD_BTL_RECORDER_MODE_VIDEO   (0)
 *  SCRCMD_BTL_RECORDER_MODE_MUSICAL (1)
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdWifiBattleRecorderEventCall( VMHANDLE* core, void* wk );



