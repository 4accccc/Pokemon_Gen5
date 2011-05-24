//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		scrcmd_animal_trail.h
//   @brief   スクリプト けものみち
//   @author	Masayuki Onoue
//   @date		2011.05.13
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern VMCMD_RESULT EvCmdGetAnimalTrailData( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdWarpAnimalTrail( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdResetNowAnimalTrailObject( VMHANDLE* core, void* wk );
extern VMCMD_RESULT EvCmdExitAnimalTrail( VMHANDLE* core, void* wk );

