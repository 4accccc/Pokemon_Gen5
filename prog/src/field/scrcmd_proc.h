//======================================================================
/**
 * @file    scrcmd_proc.h
 * @brief   スクリプトコマンド用関数　各種アプリ呼び出し系(常駐)
 * @author  Miyuki Iwasawa 
 * @date    09.10.22
 *
 * フェード＆フィールド解放＆生成までをコモンのイベントシーケンス
 * (EVENT_FieldSubProc,EVENT_FieldSubProc_Callbackなど)に任せる場合、プロセス呼び出しコマンド自体を
 * 常駐に配置する必要がないので、scrcmd_proc_fld.c(フィールド常駐)に置いてください
 *
 * フェードやフィールドプロセスコントロールを自前で行うプロセス呼び出し法を取る必要がある場合
 * このソースにおいてください。特に必要がなければ、常駐メモリ節約のため、EVENT_FieldSubProc等
 * 用意されたプロセス呼び出しイベントを利用してください
 */
//======================================================================

#pragma once

//サブプロセス呼び出し用ワーク
typedef struct _CALL_PROC_WORK_TAG CALL_PROC_WORK;

struct _CALL_PROC_WORK_TAG{
  void* proc_work; //プロセス用ワーク
  void* cb_work; //コールバック用ワーク
  void (*cb_func)(CALL_PROC_WORK* cpw); 
};

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

/*
 *  @brief  サブプロセスコールとWait関数のセットアップ(メモリ解放バージョン)
 *
 *  @param  core
 *  @param  work
 *  @param  ov_id         サブプロセスのオーバーレイID
 *  @param  proc_data     サブプロセスの関数指定
 *  @param  proc_work     サブプロセス用ワーク。NULL指定可
 *  @param  callback      サブプロセス終了時に呼び出すコールバック関数ポインタ。NULL指定可
 *  @param  callback_work 汎用ワーク。NULL指定可
 *
 *  注：callbackにNULLを指定した場合、サブプロセス終了時に
 *  proc_workとcallback_workに対して GFL_HEAP_FreeMemory()を呼び出します。
 *  callbackを指定した場合は、callback中で明示的に解放をしてください
 */
extern void EVFUNC_CallSubProc( VMHANDLE *core, SCRCMD_WORK *work,
    FSOverlayID ov_id, const GFL_PROC_DATA * proc_data, void* proc_work,
    void (*cb_func)(CALL_PROC_WORK* cpw), void* callback_work );

//--------------------------------------------------------------
/**
 * @brief サブプロセスの終了を待ちます
 * @param   core    仮想マシン制御構造体へのポインタ
 * @retval  TRUE    終了
 * @retval  FALSE   待ち
 *
 * ＊使用は玉田さんの許可制です。
 * 　プロセスコールは原則として、呼び出し～戻り値取得までをOneコマンドで実現する設計にすること！
 *   EVFUNC_CallSubProc()を使いコールバック付きでサブプロセスを呼び出せます
 *
 * ＊こちらで待った場合、明示的に EvCmdFreeSubProcWork( VMHANDLE *core, void *wk ) を使って解放してください
 */
//--------------------------------------------------------------
extern BOOL EVFUNC_WaitSubProcEndNonFree( VMHANDLE *core, void *wk );

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
 * @brief   バッグプロセスを呼び出し、戻り値を取得します
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdCallBagProc( VMHANDLE *core, void *wk );

//--------------------------------------------------------------
/**
 * @brief   メールボックス画面プロセスを呼び出します
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdCallMailBoxProc( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdCallWazaRemindProc( VMHANDLE *core, void *wk );

//--------------------------------------------------------------
/**
 * @brief   モノリス画面プロセスを呼び出します
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdCallMonolithProc( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdDemoScene( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdCallTVTDemo( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdCallZukanAward( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdCallPsel( VMHANDLE *core, void *wk );
