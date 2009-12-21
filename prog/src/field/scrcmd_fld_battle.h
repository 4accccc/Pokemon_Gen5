//======================================================================
/**
 * @file	scrcmd_fld_battle.h
 * @brief	スクリプトコマンド：バトル関連ユーティリティ(フィールド常駐)
 * @author  iwasawa GAMEFREAK inc.
 * @date	09.12.09
 */
//======================================================================

#pragma once

//--------------------------------------------------------------
/**
 * @brief 指定のレギュレーションで参加できないポケモン種名をタグ展開し、その数を返す
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdBtlUtilSetRegulationOutPokeName( VMHANDLE * core, void *wk );

//--------------------------------------------------------------
/**
 * @brief 手持ちかバトルボックスかを選択させるウィンドウ表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdBtlUtilPartySelect( VMHANDLE * core, void *wk );

