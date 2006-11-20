//==============================================================================
/**
 * @file	ui.c
 * @brief	ユーザーインターフェイス管理
 * @author	GAME FREAK Inc.
 * @date	2005.07.29
 */
//==============================================================================

#include "gflib.h" 
#include "ui.h"
#include "ui_def.h" 

//==============================================================================
//
//			定義、関数宣言
//
//==============================================================================


//------------------------------------------------------------------
/**
 * @brief	_UI_SYS 型宣言
 * ユーザーインターフェイス情報
 */
//------------------------------------------------------------------
struct _UI_SYS {
    UI_TPSYS* pTP;         ///< タッチパネル管理構造体
    UI_KEYSYS* pKey;       ///< キー管理構造体
	u8 DontSleep;          ///< スリープ状態にしてはいけない場合BITがたっている 大丈夫な場合0
	u8 DontSoftReset;      ///< ソフトリセットしたくない場合BITがたっている 大丈夫な場合0
	u8 DS_Boot_Flag;       ///< 
};


//==============================================================================
//
//			関数
//
//==============================================================================

//==============================================================================
/**
 * @brief UI初期化
 * @param   heapID    ヒープ確保を行うID
 * @return  UISYS  workハンドル
 */
//==============================================================================

UISYS* GFL_UI_sysInit(const int heapID)
{
    UISYS* pUI = GFL_HEAP_AllocMemory(heapID, sizeof(UISYS));

    MI_CpuClear8(pUI, sizeof(UISYS));
    pUI->pKey = GFL_UI_Key_sysInit(heapID);
    pUI->pTP = GFL_UI_TP_sysInit(heapID);
    return pUI;
}

//==============================================================================
/**
 * @brief UIMain処理
 * @param   pUI    UISYS
 * @return  none
 */
//==============================================================================

//初期化
void GFL_UI_sysMain(UISYS* pUI)
{
    GFL_UI_Key_sysMain(pUI);
    GFL_UI_TP_sysMain(pUI);
    
}

//==============================================================================
/**
 * @brief   UI終了処理
 * @param   pUI    UISYS
 * @return  none
 */
//==============================================================================

void GFL_UI_sysEnd(UISYS* pUI)
{
    GFL_HEAP_FreeMemory(pUI->pKey);
    GFL_HEAP_FreeMemory(pUI->pTP);
    GFL_HEAP_FreeMemory(pUI);
}

//------------------------------------------------------------------
/**
 * @brief   スリープ状態を禁止する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   sleepTypeBit スリープ管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFL_UI_SleepDisable(UISYS* pUI,const u8 sleepTypeBit)
{
	pUI->DontSleep |= sleepTypeBit;
}

//------------------------------------------------------------------
/**
 * @brief   スリープ状態を許可する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   sleepTypeBit スリープ管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFL_UI_SleepEnable(UISYS* pUI, const u8 sleepTypeBit)
{
	pUI->DontSleep &= ~(sleepTypeBit);
}

//------------------------------------------------------------------
/**
 * @brief   ソフトウエアリセット状態を禁止する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   softResetBit リセット管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFL_UI_SoftResetDisable(UISYS* pUI,const u8 softResetBit)
{
	pUI->DontSoftReset |= softResetBit;
}

//------------------------------------------------------------------
/**
 * @brief   ソフトウエアリセット状態を許可する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   softResetBit リセット管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFL_UI_SoftResetEnable(UISYS* pUI, const u8 softResetBit)
{
	pUI->DontSoftReset &= ~(softResetBit);
}

//------------------------------------------------------------------
/**
 * @brief      タッチパネル管理構造体を引き出す グループ内関数
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @return  タッチパネル管理構造体
 */
//------------------------------------------------------------------

UI_TPSYS* _UI_GetTPSYS(const UISYS* pUI)
{
    return pUI->pTP;
}

//------------------------------------------------------------------
/**
 * @brief   キー管理構造体を引き出す グループ内関数
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @return  キー管理構造体
 */
//------------------------------------------------------------------

UI_KEYSYS* _UI_GetKEYSYS(const UISYS* pUI)
{
    return pUI->pKey;       ///< キー管理構造体
}


