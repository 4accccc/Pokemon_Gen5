//*****************************************************************************
/**
 *@file		ui_def.h
 *@brief	ユーザーインターフェイス用グループ内定義
 *@author	k.ohno
 *@data		2006.11.16
 */
//*****************************************************************************

#ifndef __UI_DEF_H__
#define	__UI_DEF_H__

//------------------------------------------------------------------
/**
 * @brief	各機能の管理構造体定義
 */
//------------------------------------------------------------------
typedef struct _UI_TPSYS  UI_TPSYS;
typedef struct _UI_KEYSYS  UI_KEYSYS;

extern UI_TPSYS* _UI_GetTPSYS(const UISYS* pUI);

extern UI_KEYSYS* _UI_GetKEYSYS(const UISYS* pUI);


#endif  //__UI_DEF_H__
