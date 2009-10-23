//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：Wi-Fi関連(フィールド常駐)
 * @file   scrcmd_wifi.h
 * @author iwasawa
 * @date   2009.10.20
 *
 */
//////////////////////////////////////////////////////////////////////
#pragma once

//--------------------------------------------------------------------
/**
 * @brief    GSIDが正しいかをチェックする
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdWifiCheckMyGSID( VMHANDLE* core, void* wk );

//--------------------------------------------------------------------
/**
 * @brief   友だち手帳に登録されている人数を返す 
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdWifiGetFriendNum( VMHANDLE* core, void* wk );

//--------------------------------------------------------------------
/**
 * @brief   Wi-Fiクラブイベントを呼び出す
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdWifiClubEventCall( VMHANDLE* core, void* wk );

