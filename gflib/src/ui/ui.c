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
    UI_KEYSYS* pKey;       ///< キー管理構造体
	u8 DontSleep;          ///< スリープ状態にしてはいけない場合BITがたっている 大丈夫な場合0
	u8 DontSoftReset;      ///< ソフトリセットしたくない場合BITがたっている 大丈夫な場合0
	u8 DS_Boot_Flag;       ///<
    u8 AgbCasetteVersion;          ///< AGBカセット対応時のカセットバージョン 通常は0
    PMBackLightSwitch lightState;  ///< バックライトのIPL状態
} ;

static UISYS* _pUI = NULL;   ///< 一個しか生成されないのでここでポインタ管理
static void _UI_SleepFunc(void);

//==============================================================================
//
//			関数
//
//==============================================================================

//==============================================================================
/**
 * @brief UI初期化
 * @param   heapID    ヒープ確保を行うID
 * @return  none
 */
//==============================================================================

void GFL_UI_sysInit(const int heapID)
{
    UISYS* pUI = GFL_HEAP_AllocMemory(heapID, sizeof(UISYS));

    MI_CpuClear8(pUI, sizeof(UISYS));
    pUI->pKey = GFL_UI_Key_sysInit(heapID);
    _pUI = pUI;

    OS_EnableIrq();  // この関数は会議で相談した後で移動する  @@OO
    
    PM_GetBackLight(NULL,&pUI->lightState);  // バックライト状態読み込み
    //return pUI;
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
    _UI_SleepFunc();  // スリープ状態管理
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
    GFL_HEAP_FreeMemory(pUI);
}

//==============================================================================
/**
 * @brief   UI終了処理
 * @param   none
 * @return  none
 */
//==============================================================================

void GFL_UI_sysExit(void)
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
    if(_pUI==NULL){
        OS_TPanic("no init");
    }
    return _pUI;
}

//---------------------------------------------------------------------------
/**
 * @brief	スリープ状態の管理
 * @param	none
 */
//---------------------------------------------------------------------------
static void _UI_SleepFunc(void)
{
    PMBackLightSwitch up,down;
    PMWakeUpTrigger trigger;
    UISYS* pUI = _UI_GetUISYS();
    
    if(PAD_DetectFold()){ // ふたが閉まっている
        if(pUI->DontSleep == 0){  // スリープしていい場合
            GFL_UI_TPAutoSamplingStop();
            trigger = PM_TRIGGER_COVER_OPEN|PM_TRIGGER_CARD;
            // 特定のAGBカートリッジが刺さっている場合のみ復帰条件にカートリッジ設定
            if(pUI->AgbCasetteVersion)
                trigger |= PM_TRIGGER_CARTRIDGE;
            //SLEEP
            PM_GoSleepMode( trigger, 0, 0 );
            // 復帰後、カードが抜かれていたら電源OFF
            if(CARD_IsPulledOut()){
                PM_ForceToPowerOff();
            } else if((OS_GetIrqMask() & OS_IE_CARTRIDGE) && CTRDG_IsPulledOut()){
                // 復帰後、カートリッジが抜かれていたら…
                if(PAD_DetectFold()){
                    // まだふたが閉まっている状態ならば再度スリープに入った後に電源OFF
                    PM_GoSleepMode( PM_TRIGGER_COVER_OPEN|PM_TRIGGER_CARD, 0, 0 );
                    PM_ForceToPowerOff();
                } else {
                    // ふたが開いていたら電源OFF
                    PM_ForceToPowerOff();
                }
            }
            GFL_UI_TPAutoSamplingReStart();
        } else{
            // もしもカートリッジが抜かれたらSLEEP→電源OFF
            if((OS_GetIrqMask() & OS_IE_CARTRIDGE) && CTRDG_IsPulledOut()){
                PM_GoSleepMode( PM_TRIGGER_COVER_OPEN|PM_TRIGGER_CARD, 0, 0 );
                PM_ForceToPowerOff();
            }
            //BK OFF
            PM_GetBackLight(&up,&down);
            if(PM_BACKLIGHT_ON == up){
                PM_SetBackLight(PM_LCD_ALL,PM_BACKLIGHT_OFF);
            }
        }
    } else{  // 開いている
        PM_GetBackLight(&up,&down);
        if(PM_BACKLIGHT_OFF == up){
            PM_SetBackLight(PM_LCD_ALL, pUI->lightState);
        }
    }
}

