//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_scrcmd.h
 *	@brief  スクリプトコマンド
 *	@author	Toru=Nagihashi
 *	@date		2011.02.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//system
#include "system/vm.h"

//def
#include "scrcmd_resort_def.h"
//=============================================================================
/**
 *					スクリプトコマンド外部公開
*/
//=============================================================================
extern VMCMD_RESULT EvCmdResortTalkStart( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortTalkEnd( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortGetData( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortGetMsgID( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortAction( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortJobChange( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortKeepPeople( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortSetWord( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortDeletePeple( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdResortIsJobChange( VMHANDLE *p_core, void *p_wk_adrs );


//施設生成メッセージ  何ができたか
//特殊NPCメッセージ   何をしたい人がどこにできたか
//入れ替え用
//最近MSG …  優先度を持っているプライオリティ高い順
//行きたい場所メッセージ
//施設選択
//対応チェック

//○お店用
//１日１回IDくじ抽選
//  →LV　リストが増える
//なつきどアップ
//  →LV　なつきど　度合い？
//１日数回レベルを上げる
//  →レベルを上げる段階数が増える
//ダース販売
//  →
//きのみ販売
//努力値をくれる
//タマゴ孵化してくれる
//宝石販売
//掘り出し物販売
//戦える
//道具をくれる  くれる道具
//卵をくれる    くれる卵
