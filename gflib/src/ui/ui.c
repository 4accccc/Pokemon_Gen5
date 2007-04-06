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
#include "fade.h"

//==============================================================================
//
//			定義、関数宣言
//
//==============================================================================

#define _SOFTRESET_TYPE_NORMAL    (0)  // 電源投入時と同じ
#define _SOFTRESET_TYPE_NETERROR  (1)  // 通信エラーの時



//------------------------------------------------------------------
/**
 * @brief	_UI_SYS 型宣言
 * ユーザーインターフェイス情報
 */
//------------------------------------------------------------------
struct _UI_SYS {
    UI_KEYSYS* pKey;       ///< キー管理構造体
    GFL_UI_SLEEPRELEASE_FUNC* pRelease;  ///< スリープ解除時に呼ばれる関数
    void* pWork;           ///< スリープ解除時に引数として渡すワーク
    u8 DontSleep;          ///< スリープ状態にしてはいけない場合BITがたっている 大丈夫な場合0
    u8 DontSoftReset;      ///< ソフトリセットしたくない場合BITがたっている 大丈夫な場合0
    u8 AgbCasetteVersion;          ///< AGBカセット対応時のカセットバージョン 通常は0
    PMBackLightSwitch lightState;  ///< バックライトのIPL状態
} ;

static UISYS* _pUI = NULL;   ///< 一個しか生成されないのでここでポインタ管理
static void _UI_SleepFunc(void);
static void _UI_ResetLoop(int resetNo);

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

void GFL_UI_Boot(const HEAPID heapID)
{
    UISYS* pUI = GFL_HEAP_AllocMemory(heapID, sizeof(UISYS));

    MI_CpuClear8(pUI, sizeof(UISYS));
    pUI->pKey = GFL_UI_KEY_Init(heapID);
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

void GFI_UI_Main(UISYS* pUI)
{
    GFL_UI_KEY_Main();
    _UI_SleepFunc();  // スリープ状態管理

    if ((GFL_UI_KEY_GetCont() & PAD_BUTTON_SOFTRESET) == PAD_BUTTON_SOFTRESET) {
        if(pUI->DontSoftReset == 0){  // 抑制するBITが何も無ければReset
            _UI_ResetLoop(_SOFTRESET_TYPE_NORMAL);  // この関数内でsoftresetされる
        }
    }

}

//==============================================================================
/**
 * @brief UIMain処理
 * @param   none
 * @return  none
 */
//==============================================================================
void GFL_UI_Main(void)
{
    GFI_UI_Main(_pUI);
}

//==============================================================================
/**
 * @brief   UI終了処理
 * @param   pUI    UISYS
 * @return  none
 */
//==============================================================================

void GFI_UI_Exit(UISYS* pUI)
{
    GFL_UI_KEY_Exit();
    GFL_HEAP_FreeMemory(pUI);
}

//==============================================================================
/**
 * @brief   UI終了処理
 * @param   none
 * @return  none
 */
//==============================================================================

void GFL_UI_Exit(void)
{
    GFI_UI_Exit(_pUI);
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
 * @brief   スリープ解除時に呼ばれる関数をセットする
 * @param   pUI		ユーザーインターフェイス管理構造体
 * @param   pFunc   スリープ解除時に呼ばれる関数
 * @param   pWork   呼ばれる際に渡すワーク
 * @return  none
 */
//------------------------------------------------------------------
void GFI_UI_SleepReleaseSetFunc(UISYS* pUI, GFL_UI_SLEEPRELEASE_FUNC* pFunc, void* pWork)
{
    pUI->pRelease = pFunc;
    pUI->pWork = pWork;
}

//------------------------------------------------------------------
/**
 * @brief   スリープ解除時に呼ばれる関数をセットする
 * @param   pFunc   スリープ解除時に呼ばれる関数
 * @param   pWork   呼ばれる際に渡すワーク
 * @return  none
 */
//------------------------------------------------------------------
void GFL_UI_SleepReleaseSetFunc(GFL_UI_SLEEPRELEASE_FUNC* pFunc, void* pWork)
{
    GFI_UI_SleepReleaseSetFunc(_pUI, pFunc, pWork);
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
            GFL_UI_TP_AutoSamplingStop();
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
            {
                GFL_UI_SLEEPRELEASE_FUNC* pRelease = pUI->pRelease;
                pRelease(pUI->pWork);
            }
            GFL_UI_TP_AutoSamplingReStart();
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

//---------------------------------------------------------------------------
/**
 * @brief	ソフトウエアリセットが起きた場合の処理
 * @param	resetNo   OS_ResetSystemに渡すリセット種類
 */
//---------------------------------------------------------------------------
static void _UI_ResetLoop(int resetNo)
{
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0,16, _GFI_FADE_BASESPEED );
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT_SUB, 0,16, _GFI_FADE_BASESPEED );
    GFL_NET_Disconnect();  // 通信リセットへ移行
#if 0  //後日追加 @@OO
    // もしセーブしてたらキャンセルしておかないとリセットできない
    SaveData_DivSave_Cancel(SaveData_GetPointer());
#endif
    while (1) {
        // 通信としてリセットしてもいい状態 + メモリーカード終了
        if(GFL_NET_IsResetEnable() && CARD_TryWaitBackupAsync()){

            GFL_BG_Exit();  // 名前書き換え後エリア開放を入れるテストを行う  k.ohno

            OS_ResetSystem(resetNo);  // 切断確認後終了
        }
        OS_WaitIrq(TRUE, OS_IE_V_BLANK);

        GFL_UI_Main();   // 必要最小のGFL関数
        GFL_NET_Main();
        GFL_FADE_Main();
        
    }
}

