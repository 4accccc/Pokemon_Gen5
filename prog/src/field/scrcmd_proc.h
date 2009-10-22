//======================================================================
/**
 * @file    scrcmd_proc.h
 * @brief   スクリプトコマンド用関数　各種アプリ呼び出し系(常駐)
 * @author  Miyuki Iwasawa 
 * @date    09.10.22
 */
//======================================================================

#pragma once

//--------------------------------------------------------------
/**
 * @brief フィールドマッププロセス生成
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdFieldOpen( VMHANDLE *core, void *wk );

//--------------------------------------------------------------
/**
 * @brief フィールドマッププロセス破棄
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdFieldClose( VMHANDLE *core, void *wk );

//--------------------------------------------------------------
/**
 * @brief サブプロセスの終了を待ちます
 * @param   core    仮想マシン制御構造体へのポインタ
 * @retval  TRUE    終了
 * @retval  FALSE   待ち
 */
//--------------------------------------------------------------
extern BOOL SCMD_WaitSubProcEnd( VMHANDLE *core, void *wk );
extern BOOL SCMD_WaitSubProcEndNonFree( VMHANDLE *core, void *wk );

//--------------------------------------------------------------
/**
 * @brief サブプロセスワーク解放
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdFreeSubProcWork( VMHANDLE *core, void *wk );

//--------------------------------------------------------------
/**
 * @brief   バッグプロセスを呼び出します
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 *
 * ＊サブプロセスワークの解放処理を行わないので、結果の取得後に
 * 　明示的に EvCmdFreeSubProcWork()を呼び出すこと
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdCallBagProc( VMHANDLE *core, void *wk );

//--------------------------------------------------------------
/**
 * @brief   バッグプロセスのリターンコードを取得します
 * @param   core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 *
 * ＊EvCmdFreeSubProcWork() の前に呼び出すこと
 *
 * バッグプロセスの終了モードと選択アイテムNoを返します
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdGetBagProcResult( VMHANDLE *core, void *wk );

//--------------------------------------------------------------
/**
 * @brief   ボックスプロセスを呼び出します
 * @param   core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdCallBoxProc( VMHANDLE *core, void *wk );
