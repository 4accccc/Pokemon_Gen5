//==============================================================================
/**
 * @file	battle_playbackstop.h
 * @brief	戦闘録画再生中の停止ボタン制御のヘッダ
 * @author	matsuda change by soga
 * @date	2009.04.15(水)
 */
//==============================================================================
#ifndef __BATTLE_PLAYBACKSTOP_H__
#define __BATTLE_PLAYBACKSTOP_H__


//==============================================================================
//	外部関数宣言
//==============================================================================
extern GFL_TCB * PlayBackStopButton_Create( BI_PARAM_PTR bip, GFL_TCBSYS *tcbsys, HEAPID heapID );
extern void PlayBackStopButton_Free( GFL_TCB *tcb );


#endif	//__BATTLE_PLAYBACKSTOP_H__

