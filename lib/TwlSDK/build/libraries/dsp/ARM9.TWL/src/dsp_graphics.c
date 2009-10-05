/*---------------------------------------------------------------------------*
  Project:  TwlSDK - dsp
  File:     dsp_graphics.c

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/graphics.h>

#include "dsp_process.h"

extern const u8 DSPiFirmware_graphics[];

/*---------------------------------------------------------------------------*/
/* variables */

static DSPPipe binOut[1];
static BOOL DSPiGraphicsAvailable = FALSE;
static DSPProcessContext DSPiProcessGraphics[1];

static u16 replyReg;

/* 制約条件を満たしてしまっているか判定する（満たしているならば偽） */
static BOOL CheckLimitation(f32 rx, f32 ry, DSPGraphicsScalingMode mode, u16 src_width);

/*---------------------------------------------------------------------------*/
/* functions */
/*---------------------------------------------------------------------------*
  Name:         CheckLimitation

  Description:  DSP_Scaling*() の仕様による制約条件を満たしているかどうかを判定する
  
  Arguments:    mode : 拡大（縮小）方式
                rx, ry : 横倍率、縦倍率
                src_width : 入力画像の幅
  
  Returns:      制約条件を満たしている場合は FALSE
 *---------------------------------------------------------------------------*/
static BOOL CheckLimitation(f32 rx, f32 ry, DSPGraphicsScalingMode mode, u16 src_width)
{
    // 処理後のデータサイズが元データサイズ以上になる場合
    if (rx * ry >= 1.0f)
    {
        switch(mode)
        {
        case DSP_GRAPHICS_SCALING_MODE_N_NEIGHBOR:
        case DSP_GRAPHICS_SCALING_MODE_BILINEAR:
            if (DSP_CALC_SCALING_SIZE(src_width, rx) * ry >= 8192)
            {
                return FALSE;
            }
            break;
        case DSP_GRAPHICS_SCALING_MODE_BICUBIC:
            if (DSP_CALC_SCALING_SIZE(src_width, rx) * ry >= 4096)
            {
                return FALSE;
            }
            break;
        }
    }
    else    // 処理後のデータサイズが元データサイズよりも小さくなる場合
    {
        switch(mode)
        {
        case DSP_GRAPHICS_SCALING_MODE_N_NEIGHBOR:
        case DSP_GRAPHICS_SCALING_MODE_BILINEAR:
            if (src_width >= 8192)
            {
                return FALSE;
            }
            break;
        case DSP_GRAPHICS_SCALING_MODE_BICUBIC:
            if (src_width >= 4096)
            {
                return FALSE;
            }
            break;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_OpenStaticComponentGraphicsCore

  Description:  グラフィックスコンポーネント用のメモリファイルを開く。
                ファイルシステムに事前に用意しておく必要がなくなるが
                静的メモリとしてリンクされるためプログラムサイズが増加する。
  
  Arguments:    file : メモリファイルを開くFSFile構造体。
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_OpenStaticComponentGraphicsCore(FSFile *file)
{
    extern const u8 DSPiFirmware_graphics[];
    (void)DSPi_CreateMemoryFile(file, DSPiFirmware_graphics);
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_GraphicsEvents(viod *userdata)

  Description:  DSP グラフィックスコンポーネントのイベントハンドラ

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
static void DSPi_GraphicsEvents(void *userdata)
{
    (void)userdata;
    
    // 非同期処理を行っているときは、終了通知の有無を調べる
    if ( isAsync )
    {
        replyReg = DSP_RecvData(DSP_GRAPHICS_REP_REGISTER);
        if (replyReg == DSP_STATE_SUCCESS)
        {
            isBusy = FALSE;
            isAsync = FALSE;
                
            if ( callBackFunc != NULL)
            {
                callBackFunc();
            }
        }
        else if (replyReg == DSP_STATE_FAIL)
        {
            OS_TWarning("a process on DSP is failed.\n");
            isBusy = FALSE;
            isAsync = FALSE;
        }
        else
        {
            OS_TWarning("unknown error occured.\n");
            isBusy = FALSE;
            isAsync = FALSE;
        }
    }
    else    // 同期版
    {
//        replyReg = DSP_RecvData(DSP_GRAPHICS_REP_REGISTER);
//        isBusy = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadGraphicsCore

  Description:  グラフィックスコンポーネントをDSPへロード。
  
  Arguments:    file  : グラフィックスコンポーネントのファイル。
                slotB : コードメモリのために使用を許可されたWRAM-B。
                slotC : データメモリのために使用を許可されたWRAM-C。
  
  Returns:      グラフィックスコンポーネントが正しくDSPへロードされればTRUE。
 *---------------------------------------------------------------------------*/
BOOL DSPi_LoadGraphicsCore(FSFile *file, int slotB, int slotC)
{
    if (!DSPiGraphicsAvailable)
    {
        isBusy = FALSE;
        isAsync = FALSE;
        DSP_InitProcessContext(DSPiProcessGraphics, "graphics");
        // (DSP側のリンカスクリプトファイルに指定があればこの明示設定は不要)
        DSPiProcessGraphics->flags |= DSP_PROCESS_FLAG_USING_OS;
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           DSPi_GraphicsEvents, NULL);
        if (DSP_ExecuteProcess(DSPiProcessGraphics, file, slotB, slotC))
        {
            DSPiGraphicsAvailable = TRUE;
        }
        
        (void)DSP_LoadPipe(binOut, DSP_PIPE_BINARY, DSP_PIPE_OUTPUT);
    }
    
    return DSPiGraphicsAvailable;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_UnloadGraphicsCore

  Description:  DSPのグラフィックスコンポーネントを終了する。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSPi_UnloadGraphicsCore(void)
{
    if(DSPiGraphicsAvailable)
    {
        DSP_QuitProcess(DSPiProcessGraphics);
        DSPiGraphicsAvailable = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_YuvToRgbConvertCore

  Description:  YUV->RGB 変換を行います

  Arguments:    src       : 処理元データアドレス
                dest      : 処理後データ格納先アドレス
                size      : src データサイズ
                callback  : 変換終了後に実行されるコールバック関数のポインタ
                async     : 非同期で処理を実行するなら TRUE

  Returns:      成功ならTRUE
 *---------------------------------------------------------------------------*/
BOOL DSPi_ConvertYuvToRgbCore(const void* src, void* dst, u32 size, DSP_GraphicsCallback callback, BOOL async)
{
    DSPYuv2RgbParam yr_param;
    u32 offset = 0;
    u16 command;
    
    // もう一度 DSP がビジー状態かどうか確認する
    if (isBusy)
    {
        OS_TPrintf("dsp is busy.\n");
        return FALSE;
    }
    isBusy = TRUE;
    
    // コールバック関数の登録
    callBackFunc = callback;
    isAsync = async;
    
    if (async)
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           DSPi_GraphicsEvents, NULL);
    }
    else
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           NULL, NULL);
    }
    
    // DSP へ開始する処理の内容を通知する
    command = DSP_G_FUNCID_YUV2RGB;
    DSP_SendData(DSP_GRAPHICS_COM_REGISTER, command);
    
    // DSP へパラメータ送信
    yr_param.size = size;
    yr_param.src = (u32)((u32)src + offset);
    yr_param.dst = (u32)((u32)dst + offset);
    
    DSP_WritePipe(binOut, &yr_param, sizeof(DSPYuv2RgbParam));
    
    if (async)
    {
        return TRUE;
    }
    else
    {
        // DSP からのリプライを待つ
        while (!DSP_RecvDataIsReady(DSP_GRAPHICS_REP_REGISTER)) {
            OS_Sleep(1);
        }
        replyReg = DSP_RecvData(DSP_GRAPHICS_REP_REGISTER);
        isBusy = FALSE;
        
        if ( replyReg == DSP_STATE_SUCCESS)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_ScalingCore

  Description:  画像データの拡大縮小処理を実行

  Returns:      成功ならば TRUE を返す
 *---------------------------------------------------------------------------*/
BOOL DSPi_ScalingCore(const void* src, void* dst, u16 img_width, u16 img_height, f32 rx, f32 ry, DSPGraphicsScalingMode mode,
                      u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback, BOOL async)
{
    DSPScalingParam sc_param;
    
    u16 command;
    
    // 制約条件チェック
    SDK_TASSERTMSG(CheckLimitation(rx, ry, mode, width), "DSP_Scaling: arguments exceed the limit.");
    
    // もう一度 DSP がビジー状態かどうか確認する
    if (isBusy)
    {
        OS_TPrintf("dsp is busy.\n");
        return FALSE;
    }
    isBusy = TRUE;
    
    // コールバック関数の登録
    callBackFunc = callback;
    isAsync = async;
    if (async)
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           DSPi_GraphicsEvents, NULL);
    }
    else
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           NULL, NULL);
    }
    
    // DSP へ開始する処理の内容を通知する
    command = DSP_G_FUNCID_SCALING;
    DSP_SendData(DSP_GRAPHICS_COM_REGISTER, command);

    // パラメータを DSP へ転送
    sc_param.src = (u32)src;
    sc_param.dst = (u32)dst;
    sc_param.mode = mode;
    sc_param.img_width = img_width;
    sc_param.img_height = img_height;
    sc_param.rate_w = (u16)(rx * 1000);
    sc_param.rate_h = (u16)(ry * 1000);
    sc_param.block_x = x;
    sc_param.block_y = y;
    sc_param.width = width;
    sc_param.height = height;

    DSP_WritePipe(binOut, &sc_param, sizeof(DSPScalingParam));
    
    // 非同期で処理を実行するかで分岐
    if(isAsync)
    {
        return TRUE;
    }
    else
    {
        // DSP からのリプライを待つ
        while (!DSP_RecvDataIsReady(DSP_GRAPHICS_REP_REGISTER)) {
            OS_Sleep(1);
        }
        
        replyReg = DSP_RecvData(DSP_GRAPHICS_REP_REGISTER);
        isBusy = FALSE;
        
        if ( replyReg == DSP_STATE_SUCCESS )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    
    return FALSE;
}

BOOL DSPi_ScalingFxCore(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rx, fx32 ry, DSPGraphicsScalingMode mode,
                      u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback, BOOL async)
{
    DSPScalingParam sc_param;
    
    u16 command;
    
    // 制約条件チェック
    SDK_TASSERTMSG(CheckLimitation(FX_FX32_TO_F32(rx), FX_FX32_TO_F32(ry), mode, width), "DSP_Scaling: arguments exceed the limit.");
    
    // もう一度 DSP がビジー状態かどうか確認する
    if (isBusy)
    {
        OS_TPrintf("dsp is busy.\n");
        return FALSE;
    }
    isBusy = TRUE;
    
    // コールバック関数の登録
    callBackFunc = callback;
    isAsync = async;
    
    if (async)
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           DSPi_GraphicsEvents, NULL);
    }
    else
    {
        DSP_SetProcessHook(DSPiProcessGraphics,
                           DSP_HOOK_REPLY_(DSP_GRAPHICS_REP_REGISTER),
                           NULL, NULL);
    }
    
    // DSP へ開始する処理の内容を通知する
    command = DSP_G_FUNCID_SCALING;
    DSP_SendData(DSP_GRAPHICS_COM_REGISTER, command);
    
    // パラメータを DSP へ転送
    sc_param.src = (u32)src;
    sc_param.dst = (u32)dst;
    sc_param.mode = mode;
    sc_param.img_width = img_width;
    sc_param.img_height = img_height;
    sc_param.rate_w = (u16)(rx / 4096.0f * 1000);
    sc_param.rate_h = (u16)(ry / 4096.0f * 1000);
    sc_param.block_x = x;
    sc_param.block_y = y;
    sc_param.width = width;
    sc_param.height = height;

    DSP_WritePipe(binOut, &sc_param, sizeof(DSPScalingParam));
    
    // 非同期で処理を実行するかで分岐
    if(isAsync)
    {
        return TRUE;
    }
    else
    {
        // DSP からのリプライを待つ
        while (!DSP_RecvDataIsReady(DSP_GRAPHICS_REP_REGISTER)) {
            OS_Sleep(1);
        }
        replyReg = DSP_RecvData(DSP_GRAPHICS_REP_REGISTER);
        isBusy = FALSE;
        
        if ( replyReg == DSP_STATE_SUCCESS )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    
    return FALSE;
}

