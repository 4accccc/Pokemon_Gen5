//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_demo_gimmick.h
 *	@brief  ギミック関係スクリプトコマンド
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.03
 *
 *	@note   SWANで追加されたデモで使用するギミックの
 *	        スクリプトコマンドを集約しています
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "system/vm.h"

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

// ジャイアントホール
extern VMCMD_RESULT EvCmdGianthole_Disp( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGianthole_AnmStart( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGianthole_StoneFloat( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGianthole_NPokeAnmStart( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGianthole_NPokeAnmEnd( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGianthole_ObjOffsetSet( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGianthole_WildWin( VMHANDLE *core, void *wk );

// 物語開始
// c12 ヒオウギシティ
extern VMCMD_RESULT EvCmdC12_AnmStart( VMHANDLE *core, void *wk );

// エンディング
// C09P03 エンディング用のポケモンリーグ
extern VMCMD_RESULT EvCmdEnding_FireWorks( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdEnding_FireWorksSeEnd( VMHANDLE *core, void *wk );
// C12 エンディング
extern VMCMD_RESULT EvCmdEnding_C12Init( VMHANDLE *core, void *wk );

