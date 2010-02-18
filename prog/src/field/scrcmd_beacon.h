//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：ビーコン通信/Gパワー関連
 * @file   scrcmd_beacon.h
 * @author iwasawa
 * @date   2010.02.15
 */
//////////////////////////////////////////////////////////////////////

#pragma once

//--------------------------------------------------------------------
/**
 * @brief ビーコン送信リクエスト呼び出し 
 *
 * @param core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT:w
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdBeaconSetRequest( VMHANDLE* core, void* wk );

//--------------------------------------------------------------------
/**
 * @brief フィニッシュ待ちのGパワーチェック 
 *
 * @param core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT:
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdCheckGPowerFinish( VMHANDLE* core, void* wk );

//--------------------------------------------------------------------
/**
 * @brief Gパワーフィールドエフェクト
 *
 * @param core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT:
 */
//--------------------------------------------------------------------
extern VMCMD_RESULT EvCmdGPowerUseEffect( VMHANDLE* core, void* wk );



