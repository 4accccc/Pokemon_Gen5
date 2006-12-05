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
} ;

static UISYS* _pUI = NULL;   ///< 一個しか生成されないのでここでポインタ管理

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
    _pUI = pUI;
    return pUI;
}

//==============================================================================
/**
 * @brief UIMain処理
 * @param   pUI    UISYS
 * @return  none
 */
//==============================================================================

void GFI_UI_sysMain(UISYS* pUI)
{
    GFL_UI_Key_sysMain();
    GFL_UI_TP_sysMain();
}

//==============================================================================
/**
 * @brief UIMain処理
 * @param   none
 * @return  none
 */
//==============================================================================
void GFL_UI_sysMain(void)
{
    GFI_UI_sysMain(_pUI);
}

//==============================================================================
/**
 * @brief   UI終了処理
 * @param   pUI    UISYS
 * @return  none
 */
//==============================================================================

void GFI_UI_sysEnd(UISYS* pUI)
{
    GFL_UI_Key_sysEnd();
    GFL_UI_TP_sysEnd();
    GFL_HEAP_FreeMemory(pUI);
}

//==============================================================================
/**
 * @brief   UI終了処理
 * @param   none
 * @return  none
 */
//==============================================================================

void GFL_UI_sysEnd(void)
{
    GFI_UI_sysEnd(_pUI);
}

//------------------------------------------------------------------
/**
 * @brief   スリープ状態を禁止する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   sleepTypeBit スリープ管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFI_UI_SleepDisable(UISYS* pUI,const u8 sleepTypeBit)
{
	pUI->DontSleep |= sleepTypeBit;
}

//------------------------------------------------------------------
/**
 * @brief   スリープ状態を禁止する
 * @param   sleepTypeBit スリープ管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFL_UI_SleepDisable(const u8 sleepTypeBit)
{
    GFI_UI_SleepDisable(_pUI, sleepTypeBit);
}

//------------------------------------------------------------------
/**
 * @brief   スリープ状態を許可する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   sleepTypeBit スリープ管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFI_UI_SleepEnable(UISYS* pUI, const u8 sleepTypeBit)
{
	pUI->DontSleep &= ~(sleepTypeBit);
}

//------------------------------------------------------------------
/**
 * @brief   スリープ状態を許可する
 * @param   sleepTypeBit スリープ管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFL_UI_SleepEnable(const u8 sleepTypeBit)
{
    GFI_UI_SleepEnable(_pUI, sleepTypeBit);
}

//------------------------------------------------------------------
/**
 * @brief   ソフトウエアリセット状態を禁止する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   softResetBit リセット管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFI_UI_SoftResetDisable(UISYS* pUI,const u8 softResetBit)
{
	pUI->DontSoftReset |= softResetBit;
}

//------------------------------------------------------------------
/**
 * @brief   ソフトウエアリセット状態を禁止する
 * @param   softResetBit リセット管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFL_UI_SoftResetDisable(const u8 softResetBit)
{
    GFI_UI_SoftResetDisable(_pUI, softResetBit);
}

//------------------------------------------------------------------
/**
 * @brief   ソフトウエアリセット状態を許可する
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   softResetBit リセット管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFI_UI_SoftResetEnable(UISYS* pUI, const u8 softResetBit)
{
	pUI->DontSoftReset &= ~(softResetBit);
}

//------------------------------------------------------------------
/**
 * @brief   ソフトウエアリセット状態を許可する
 * @param   softResetBit リセット管理BIT
 * @return  none
 */
//------------------------------------------------------------------
void GFL_UI_SoftResetEnable(const u8 softResetBit)
{
    GFI_UI_SoftResetEnable(_pUI, softResetBit);
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


//------------------------------------------------------------------
/**
 * @brief   UI管理構造体を引き出す グループ内関数
 * @param   none
 * @return  ユーザーインターフェイス管理構造体
 */
//------------------------------------------------------------------
UISYS* _UI_GetUISYS(void)
{
    return _pUI;
}
