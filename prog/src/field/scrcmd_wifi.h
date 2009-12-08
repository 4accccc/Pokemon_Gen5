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
 * @brief   フィールドの通信状態を取得する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 *
 * コマンドの戻り値 game_comm.h GAME_COMM_NO_NULL 他
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdGetFieldCommNo( VMHANDLE* core, void* wk );


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

//--------------------------------------------------------------------
/**
 * @brief   GTSネゴシエーションイベントを呼び出す
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdWifiGTSEventCall( VMHANDLE* core, void* wk );

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
extern VMCMD_RESULT EvCmdWifiRandomMatchCall( VMHANDLE* core, void* wk );

//--------------------------------------------------------------------
/**
 * @brief   バトルビデオを呼び出す
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdWifiBattleVideoCall( VMHANDLE* core, void* wk );

//--------------------------------------------------------------------
/**
 * @brief   ミュージカルショットを呼び出す
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdWifiMusicalShotCall( VMHANDLE* core, void* wk );

