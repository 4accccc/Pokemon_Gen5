/*---------------------------------------------------------------------------*
  Project:  NW4R - LayoutLib - build - demos - lyt - demolib
  File:     pad.cpp

  Copyright (C)2006-2008 Nintendo  All Rights Reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1.2 $
 *---------------------------------------------------------------------------*/

#include "lyt_demolib/lyt_pad.h"
#include "lyt_demolib/lyt_heap.h"
#include <demo.h>
#include <nw4r/misc.h>
#include <nw4r/ut.h>


namespace demo
{

namespace
{

const int NUM_RVL_PAD =
    PAD_MAX_CONTROLLERS < WPAD_MAX_CONTROLLERS ?
        PAD_MAX_CONTROLLERS:
        WPAD_MAX_CONTROLLERS;

struct  ButtonStatus
{
    u32     hold;
    u32     trig;
    u32     release;
};

ButtonStatus sDEMOPadButtonStatuss[NUM_RVL_PAD];
KPADStatus sKPADStatuss[NUM_RVL_PAD];

// WPADRegisterAllocatorに渡すアロケータ関数
// WPADRegisterAllocatorで渡すアロケータは、MEM2から確保する必要があります。
void*
AllocForWPAD(u32 size)
{
    NW4R_ASSERT(gDemoHeap2 != MEM_HEAP_INVALID_HANDLE);

    return MEMAllocFromExpHeapEx(gDemoHeap2, size, 32);
}

u8
FreeForWPAD(void* ptr)
{
    NW4R_ASSERT(gDemoHeap2 != MEM_HEAP_INVALID_HANDLE);

    MEMFreeToExpHeap(gDemoHeap2, ptr);
    return 1;
}

/*---------------------------------------------------------------------------*
  Name:         ResetPad

  Description:  ButtonStatusのボタン状態をリセットします。

  Arguments:    pPadButton:  ButtonStatus構造体へのポインタ。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void
ResetPad(ButtonStatus* pPadButton)
{
    std::memset(pPadButton, 0, sizeof(*pPadButton));
}

/*---------------------------------------------------------------------------*
  Name:         MapDEMOPadButton

  Description:  DEMOPADのボタン値をKPADのボタン値に変更します。

  Arguments:    padButton:  DEMOPADのボタン値。

  Returns:      KPADのボタン値。
 *---------------------------------------------------------------------------*/
u32
MapDEMOPadButton(u32 padButton)
{
    u32 kpadButton = 0;

#define MAP_BUTTON(pad, kpad) (kpadButton |= u32(padButton & pad ? kpad: 0))
    MAP_BUTTON(PAD_BUTTON_LEFT,   KPAD_BUTTON_LEFT);
    MAP_BUTTON(PAD_BUTTON_RIGHT,  KPAD_BUTTON_RIGHT);
    MAP_BUTTON(PAD_BUTTON_UP,     KPAD_BUTTON_UP);
    MAP_BUTTON(PAD_BUTTON_DOWN,   KPAD_BUTTON_DOWN);
    MAP_BUTTON(PAD_BUTTON_A,      KPAD_BUTTON_A);
    MAP_BUTTON(PAD_BUTTON_B,      KPAD_BUTTON_B);
    MAP_BUTTON(PAD_BUTTON_X,      KPAD_BUTTON_PLUS);
    MAP_BUTTON(PAD_BUTTON_Y,      KPAD_BUTTON_MINUS);
    MAP_BUTTON(PAD_BUTTON_START,  KPAD_BUTTON_HOME);
#undef MAP_BUTTON

    return kpadButton;
}

/*---------------------------------------------------------------------------*
  Name:         ConvertToKPADButton

  Description:  DEMOPADのボタンデータをKPADのボタン形式に変更します。

  Arguments:    pPadButton:  ButtonStatus構造体へのポインタ。
                i:           対象とするコントローラの0から始まるインデックス。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void
ConvertToKPADButton(
    ButtonStatus*   pPadButton,
    u32             i
)
{
    ResetPad(pPadButton);

    if (DEMOPadGetErr(i) == PAD_ERR_NONE)
    {
        pPadButton->hold    = MapDEMOPadButton(DEMOPadGetButton(i));
        pPadButton->trig    = MapDEMOPadButton(DEMOPadGetButtonDown(i));
        pPadButton->release = MapDEMOPadButton(DEMOPadGetButtonUp(i));
    }
}

/*---------------------------------------------------------------------------*
  Name:         ReadDEMOPad

  Description:  DEMOPADによるデータの取得を行います。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void
ReadDEMOPad()
{
    DEMOPadRead();

    for (u32 i = 0; i < NUM_RVL_PAD; ++i)
    {
        ButtonStatus* pPadButton = &sDEMOPadButtonStatuss[i];

        // エラーなしでなければリセットする
        if (DEMOPadGetErr(i) != PAD_ERR_NONE)
        {
            ResetPad(pPadButton);
        }
        else
        {
            ConvertToKPADButton(pPadButton, i);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         ReadKPAD

  Description:  KPADによるデータの取得を行います。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void
ReadKPAD()
{
    for (s32 i = 0; i < NUM_RVL_PAD; ++i)
    {
        const s32 numRead = KPADRead(i, &sKPADStatuss[i], 1);

        // 取得に失敗した場合は、エラーステータスをセットしておく
        if (numRead != 1)
        {
            sKPADStatuss[i].wpad_err = WPAD_ERR_NO_CONTROLLER;
        }
    }
}

}   // namespace

/*-------------------------------------------------------------------------*
  Name:         InitPad

  Description:  コントローラを初期化します。

  Arguments:    なし。

  Returns:      なし。
 *-------------------------------------------------------------------------*/
void
InitPad()
{
    // KPADの初期化
    NW4R_COMPILER_ASSERT( NUM_RVL_PAD <= WPAD_MAX_CONTROLLERS );
    WPADRegisterAllocator( &AllocForWPAD, &FreeForWPAD );
    KPADInit();

    // DEMOPADの初期化
    DEMOPadInit();  // DEMOInit 内で既に呼ばれているので、基本的に不要

    for (u32 i = 0; i < NUM_RVL_PAD; ++i)
    {
        ResetPad(&sDEMOPadButtonStatuss[i]);
    }
}

/*-------------------------------------------------------------------------*
  Name:         ReadPad

  Description:  コントローラの入力状態を更新します。

  Arguments:    なし。

  Returns:      なし。
 *-------------------------------------------------------------------------*/
void
ReadPad()
{
    ReadDEMOPad();
    ReadKPAD();
}

/*-------------------------------------------------------------------------*
  Name:         PadGet*

  Description:  コントローラの入力を取得します。

  Arguments:    i:  対象とするコントローラの0から始まるインデックス。

  Returns:      .
 *-------------------------------------------------------------------------*/
u32
PadGetButtonHold(u32 i)
{
    u32 ret = 0;

    if (sKPADStatuss[i].wpad_err == WPAD_ERR_NONE)
    {
        ret |= sKPADStatuss[i].hold;
    }

    if (DEMOPadGetErr(i) == PAD_ERR_NONE)
    {
        ret |= sDEMOPadButtonStatuss[i].hold;
    }

    return ret;
}

u32
PadGetButtonRelease(u32 i)
{
    u32 ret = 0;

    if (sKPADStatuss[i].wpad_err == WPAD_ERR_NONE)
    {
        ret |= sKPADStatuss[i].release;
    }

    if (DEMOPadGetErr(i) == PAD_ERR_NONE)
    {
        ret |= sDEMOPadButtonStatuss[i].release;
    }

    return ret;
}

u32
PadGetButtonTrigger(u32 i)
{
    u32 ret = 0;

    if (sKPADStatuss[i].wpad_err == WPAD_ERR_NONE)
    {
        ret |= sKPADStatuss[i].trig;
    }

    if (DEMOPadGetErr(i) == PAD_ERR_NONE)
    {
        ret |= sDEMOPadButtonStatuss[i].trig;
    }

    return ret;
}

const nw4r::math::VEC2
PadGetPostion(u32 i)
{
    nw4r::math::VEC2 ret(0.f, 0.f);

    if (sKPADStatuss[i].wpad_err == WPAD_ERR_NONE)
    {
        ret = nw4r::math::VEC2(sKPADStatuss[i].pos.x, sKPADStatuss[i].pos.y);
    }

    if (DEMOPadGetErr(i) == PAD_ERR_NONE)
    {
        const nw4r::ut::Rect dpdRect(- 1, - 1,  1,   1);    // LTRB
        const nw4r::ut::Rect padRect(-40,  40, 40, -40);    // LTRB

        const f32 stickX = DEMOPadGetStickX(i) / (padRect.right - padRect.left  ) * (dpdRect.right - dpdRect.left  );
        const f32 stickY = DEMOPadGetStickY(i) / (padRect.top   - padRect.bottom) * (dpdRect.top   - dpdRect.bottom);

        // 傾きがDPDの値を超える場合にキューブのスティックの値を採用する。
        if (stickX*stickX + stickY*stickY > ret.x*ret.x + ret.y*ret.y)
        {
            ret.x = stickX;
            ret.y = stickY;
        }
    }

    return ret;
}

}   // namespace demo
