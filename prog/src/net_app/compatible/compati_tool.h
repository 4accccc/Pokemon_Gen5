//==============================================================================
/**
 * @file	compati_tool.h
 * @brief	相性チェックゲームのツール類のヘッダ
 * @author	matsuda
 * @date	2009.02.10(火)
 */
//==============================================================================
#ifndef __COMPATI_TOOL_H__
#define __COMPATI_TOOL_H__

//==============================================================================
//	外部関数宣言
//==============================================================================
extern void CCTOOL_CircleDataGet(CC_CIRCLE_PACKAGE *dest);
extern void CCTOOL_CircleTouchCheck(const CC_CIRCLE_PACKAGE *ccp, CCT_TOUCH_SYS *cts);


#endif	//__COMPATI_TOOL_H__
