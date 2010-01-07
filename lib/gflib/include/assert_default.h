//=============================================================================================
/**
 * @file	assert_default.h
 * @date	2007.07.03
 * @author	GAME FREAK inc.
 */
//=============================================================================================
#ifndef __ASSERT_DEFAULT_H__
#define __ASSERT_DEFAULT_H__

#include "assert.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------
/**
 * 実機動作時に、ASSERT情報を実機画面表示するモードに切り替える
 * ※ 実機動作時以外は何もしない
 */
//------------------------------------------------------------------
#ifdef GF_ASSERT_ENABLE
extern void GFL_ASSERT_SetLCDMode( void );
extern void ASSERT_DEFAULT_dispInit( void );
extern void ASSERT_DEFAULT_dispMain( const char* str );
extern void ASSERT_DEFAULT_dispFinish( void );
#else
#define GFL_ASSERT_SetLCDMode()	/* */
#endif

#ifdef __cplusplus
}/* extern "C" */
#endif


#endif
