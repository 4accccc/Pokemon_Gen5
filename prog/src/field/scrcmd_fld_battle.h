//======================================================================
/**
 * @file	scrcmd_fld_battle.h
 * @brief	スクリプトコマンド：バトル関連ユーティリティ(フィールド常駐)
 * @author  iwasawa GAMEFREAK inc.
 * @date	09.12.09
 */
//======================================================================

#pragma once
#include "savedata/regulation.h"  //REGULATION

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

//--------------------------------------------------------------
/**
 * @brief EvCmdBtlUtilPartySelect用イベント生成
 * @param gsys
 * @param[in]   same        TRUEのとき、重複を許可する
 * @param[in]   regulation  チェックに使用するレギュレーション
 * @param[out]  ret_wk      結果を返すワークへのポインタ
 * @param[in]   heapID      使用するテンポラリヒープ指定
 */
//--------------------------------------------------------------
extern GMEVENT* BtlPartySelectCheck( GAMESYS_WORK* gsys, const BOOL same, const REGULATION* regulation, u16* ret_wk, HEAPID heapID );

//----------------------------------------------------------------------------
/**
 *	@brief  レギュレーションチェック時に、手持ち選択が発生するかチェック
 *
 *	@param	gsys        ゲームシステム
 *	@param	same        同一種類ポケモンを許可するか
 *	@param	*regulation レギュレーション
 *	@param	heapID      ヒープID
 *
 *	@retval TRUEのとき呼出が発生する
 */
//-----------------------------------------------------------------------------
extern BOOL SCR_FLD_BATTLE_GetIsCallBtlPartySelect( GAMESYS_WORK* gsys, const BOOL same, const REGULATION* regulation ,HEAPID heapID );

