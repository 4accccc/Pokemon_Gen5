/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WBT - demos - wbt-1
  File:     main.c

  Copyright 2006-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
 
/*---------------------------------------------------------------------------*
    WMライブラリを使用した無線による通信を行う単機能サンプルです。
    自動的に周辺のwbt-1デモと接続します。

    HOWTO:
        1. 親機として通信を開始する場合はＡボタンを押して下さい。
           周辺に同じwbt-1デモの子機が見つかり次第、自動的にその子機と
           通信を開始します。同時に通信できる子機は15台までです。
        2. 子機として通信を開始する場合はＢボタンを押して下さい。
           周辺に同じwbt-1デモの親機が見つかり次第、自動的にその親機と
           通信を開始します。
        3. 接続中のビジー画面や、接続完了後の通信画面においてＳＴＡＲＴボタンを
           押すことで接続がリセットされ、初期画面に戻ります。
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include <nitro/wm.h>
#include <nitro/wbt.h>

#include    "font.h"
#include    "text.h"
#include    "wh.h"
#include    "bt.h"


#define MIYA_PRINT 1


/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     KEY_REPEAT_START    25     // キーリピート開始までのフレーム数
#define     KEY_REPEAT_SPAN     10     // キーリピートの間隔フレーム数

// ゲーム情報
/* MP通信関係の定数 */
#define     DEFAULT_GGID            0x003fff30

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
// キー入力情報
typedef struct KeyInfo
{
    u16     cnt;                       // 未加工入力値
    u16     trg;                       // 押しトリガ入力
    u16     up;                        // 離しトリガ入力
    u16     rep;                       // 押し維持リピート入力

}
KeyInfo;


/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void ModeSelect(void);          // 親機/子機 選択画面
static void ModeStartParent(void);     // 使用率が低いチャンネルを計算し終えた状態
static void ModeError(void);           // エラー表示画面
static void ModeWorking(void);         // ビジー画面
static void ModeParent(void);          // 親機 通信画面
static void ModeChild(void);           // 子機 通信画面
static void VBlankIntr(void);          // Vブランク割込みハンドラ

// データ受信時に呼び出される関数
static void ParentReceiveCallback(u16 aid, u16 *data, u16 length);
static void ChildReceiveCallback(u16 aid, u16 *data, u16 length);

// データ送信時に呼び出される関数
void    ParentSendCallback(void);
void    ChildSendCallback(void);

// ブロック転送状態通知関数
static void BlockTransferCallback(void *arg);
static void SendCallback(BOOL result);

// 汎用サブルーチン
static void KeyRead(KeyInfo * pKey);
static void ClearString(int vram_num);
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...);
static void InitializeAllocateSystem(void);


/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static KeyInfo gKey;                   // キー入力
static s32 gFrame;                     // フレームカウンタ

// 表示用送受信バッファ
static u8 gSendBuf[256] ATTRIBUTE_ALIGN(32);
static BOOL gRecvFlag[1 + WM_NUM_MAX_CHILD];

static int send_counter[16];
static int recv_counter[16];

static BOOL gFirstSendAtChild = TRUE;

#define PRINTF mprintf

#define VRAM_SIZE 2*32*32
static u8 g_screen[NUM_OF_SCREEN][VRAM_SIZE] ATTRIBUTE_ALIGN(32);


static TEXT_CTRL textctrl[NUM_OF_SCREEN];
TEXT_CTRL *tc[NUM_OF_SCREEN];

static int vram_num[2];
static int screen_toggle = 0;

static u32 v_blank_intr_counter = 0;
#define TEXT_HEAPBUF_SIZE 0x8000
static u8 text_heap_buffer[TEXT_HEAPBUF_SIZE];

static BOOL wbt_available = FALSE;
static u16 connected_bitmap = 0;


/*---------------------------------------------------------------------------*
  Name:         InitWireless

  Description:  ワイヤレスの初期化・再初期化処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitWireless(void)
{
    //********************************
    // 無線初期化
    connected_bitmap = 0;

    if (!WH_Initialize())
    {
        OS_Panic("WH_Initialize failed.");
    }
    WH_SetSessionUpdateCallback(BlockTransferCallback);
    
    //********************************
    if (wbt_available)
    {
        bt_stop();
        WBT_End();
        wbt_available = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain / TwlMain

  Description:  初期化及びメインループ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    int     i;

    // 各種初期化
    OS_Init();
    OS_InitTick();

    // メモリ割当
    (void)init_text_buf_sys((void *)&(text_heap_buffer[0]),
                            (void *)&(text_heap_buffer[TEXT_HEAPBUF_SIZE]));

    for (i = 0; i < NUM_OF_SCREEN; i++)
    {
        tc[i] = &(textctrl[i]);
        init_text(tc[i], (u16 *)&(g_screen[i]), 4 /* pal no. */ );
    }

    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    // 表示設定初期化

    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);


    /* BG0表示設定 */
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf000,      // SCR base block 31
                     GX_BG_CHARBASE_0x00000,    // CHR base block 0
                     GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    // 2D表示設定
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));

    // OBJ表示設定
    GX_SetBankForOBJ(GX_VRAM_OBJ_128_B);
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);

    GX_SetVisiblePlane(GX_PLANEMASK_BG0 | GX_PLANEMASK_OBJ);

    /* サブBG0表示設定 */
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_48_HI);
    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xb800,     // SCR ベースブロック 23
                      GX_BG_CHARBASE_0x00000,   // CHR ベースブロック 0
                      GX_BG_EXTPLTT_01);
    G2S_SetBG0Priority(0);
    G2S_BG0Mosaic(FALSE);
    GXS_SetGraphicsMode(GX_BGMODE_0);
    GXS_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GXS_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));

    GX_SetBankForSubOBJ(GX_VRAM_SUB_OBJ_128_D);
    GXS_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);

    GXS_SetVisiblePlane(GX_PLANEMASK_BG0 | GX_PLANEMASK_OBJ);

    G2_SetBG0Offset(0, 0);
    G2S_SetBG0Offset(0, 0);


    InitializeAllocateSystem();

    // LCD表示開始
    GX_DispOn();
    GXS_DispOn();

    // for V Blank
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    (void)GX_VBlankIntr(TRUE);


    // デバッグ文字列出力
    PRINTF("ARM9: WBT-1 demo started.\n");
    vram_num[0] = 1;
    vram_num[1] = 0;

    // 無線初期化 (WHライブラリを使用)
    InitWireless();
    WH_SetGgid(DEFAULT_GGID);

    // メインループ

    for (gFrame = 0; TRUE; gFrame++)
    {
        text_buf_to_vram(tc[vram_num[0]]);
        text_buf_to_vram(tc[vram_num[1]]);

        if (gKey.trg & PAD_BUTTON_SELECT)
        {
            screen_toggle ^= 1;
        }
        if (gKey.trg & PAD_BUTTON_L)
        {
            vram_num[screen_toggle]--;
            if (vram_num[screen_toggle] < 0)
            {
                vram_num[screen_toggle] = (NUM_OF_SCREEN - 1);
            }
        }
        else if (gKey.trg & PAD_BUTTON_R)
        {
            vram_num[screen_toggle]++;
            if (vram_num[screen_toggle] > (NUM_OF_SCREEN - 1))
            {
                vram_num[screen_toggle] = 0;
            }
        }

        // キー入力情報取得
        KeyRead(&gKey);
        
        // 通信状態により処理を振り分け
        switch (WH_GetSystemState())
        {
        case WH_SYSSTATE_IDLE:
            ModeSelect();
            break;
        case WH_SYSSTATE_MEASURECHANNEL:
            ModeStartParent();
            break;
        case WH_SYSSTATE_ERROR:
        case WH_SYSSTATE_CONNECT_FAIL:
            WH_Reset();
            break;
        case WH_SYSSTATE_FATAL:
            ModeError();
            break;
        case WH_SYSSTATE_SCANNING:
        case WH_SYSSTATE_BUSY:
            ModeWorking();
            break;
        case WH_SYSSTATE_CONNECTED:
            // 親機か子機かでさらに分岐する
            switch (WH_GetConnectMode())
            {
            case WH_CONNECTMODE_MP_PARENT:
                ModeParent();
                break;
            case WH_CONNECTMODE_MP_CHILD:
                ModeChild();
                break;
            }
            break;
        }

        // Ｖブランク待ち
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeSelect

  Description:  親機/子機 選択画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeSelect(void)
{
    // カウンタクリア
    MI_CpuClear(send_counter, sizeof(send_counter));
    MI_CpuClear(recv_counter, sizeof(recv_counter));
    
    gFirstSendAtChild = TRUE;
   
    connected_bitmap = 0; 
    if (wbt_available)
    {
        bt_stop();
        WBT_End();
        wbt_available = FALSE;
    }
    
    
    PrintString(3, 10, 0xf, "Push A to connect as PARENT");
    PrintString(3, 12, 0xf, "Push B to connect as CHILD");

    if (gKey.trg & PAD_BUTTON_A)
    {
        //********************************
        WBT_InitParent(BT_PARENT_PACKET_SIZE, BT_CHILD_PACKET_SIZE, bt_callback);
        WH_SetReceiver(ParentReceiveCallback);
        bt_register_blocks();
        (void)WH_StartMeasureChannel();
        wbt_available = TRUE;
        //********************************
    }
    else if (gKey.trg & PAD_BUTTON_B)
    {
        static const u8 ANY_PARENT[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        //********************************
        WBT_InitChild(bt_callback);
        WH_SetReceiver(ChildReceiveCallback);
        (void)WH_ChildConnectAuto(WH_CONNECTMODE_MP_CHILD, ANY_PARENT, 0);
        wbt_available = TRUE;
        //********************************
        vram_num[1] = 2;
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeStartParent

  Description:  使用率の低いチャンネルを計算し終えたときの処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeStartParent(void)
{
    (void)WH_ParentConnect(WH_CONNECTMODE_MP_PARENT, 0x0000, WH_GetMeasureChannel());
}

/*---------------------------------------------------------------------------*
  Name:         ModeError

  Description:  エラー表示画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeError(void)
{
    PrintString(5, 10, 0x1, "======= ERROR! =======");
    PrintString(5, 13, 0xf, " Fatal error occured.");
    PrintString(5, 14, 0xf, "Please reboot program.");
}

/*---------------------------------------------------------------------------*
  Name:         ModeWorking

  Description:  ビジー画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeWorking(void)
{
    PrintString(9, 11, 0xf, "Now working...");

    if (gKey.trg & PAD_BUTTON_START)
    {
        //********************************
        WH_Finalize();
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeParent

  Description:  親機 通信画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeParent(void)
{
    PrintString(8, 1, 0x2, "Parent mode");
    PrintString(4, 3, 0x4, "Send:     %08X", send_counter[0]);

    PrintString(4, 5, 0x4, "Receive:");
    {
        s32     i;

        for (i = 1; i < (WM_NUM_MAX_CHILD + 1); i++)
        {
            if (gRecvFlag[i])
            {
                PrintString(5, (s16)(6 + i), 0x4, "Child%02d: %08X", i, recv_counter[i]);
            }
            else
            {
                PrintString(5, (s16)(6 + i), 0x7, "No child");
            }
        }
    }
    
    if (gKey.trg & PAD_BUTTON_START)
    {
        //********************************
        WH_Finalize();
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeChild

  Description:  子機 通信画面での処理。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeChild(void)
{
    PrintString(8, 1, 0x2, "Child mode");
    PrintString(4, 3, 0x4, "Send:     %08X", send_counter[0]);
    PrintString(4, 5, 0x4, "Receive:");
    PrintString(5, 7, 0x4, "Parent:  %08X", recv_counter[0]);
    
    if (gFirstSendAtChild)
    {
        // 1回目のデータ送信
        ChildSendCallback();
        gFirstSendAtChild = FALSE;
    }

    if (gKey.trg & PAD_BUTTON_START)
    {
        //********************************
        WH_Finalize();
        //********************************
    }
    else if (gKey.trg & PAD_BUTTON_Y)
    {
        bt_start();
    }
    else if (gKey.trg & PAD_BUTTON_X)
    {
        bt_stop();
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Ｖブランク割込みベクトル。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    vu16   *ptr;
    int     i;

    DC_FlushRange((void *)&(g_screen[vram_num[0]]), VRAM_SIZE);
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(&(g_screen[vram_num[0]]), 0, VRAM_SIZE);

    DC_FlushRange((void *)&(g_screen[vram_num[1]]), VRAM_SIZE);
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GXS_LoadBG0Scr(&(g_screen[vram_num[1]]), 0, VRAM_SIZE);

    if (screen_toggle)
        ptr = (u16 *)G2S_GetBG0ScrPtr();
    else
        ptr = (u16 *)G2_GetBG0ScrPtr();

    for (i = 0; i < 32; i++)
    {
        *ptr = (u16)((2 << 12) | (0x00ff & '='));
        ptr++;
    }


    v_blank_intr_counter++;
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         ParentReceiveCallback

  Description:  親機として子機からのデータ受信時に呼び出される関数。

  Arguments:    aid     - 送信元子機の aid
                data    - 受信データへのポインタ (NULL で切断通知)
                length  - 受信データのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ParentReceiveCallback(u16 aid, u16 *data, u16 length)
{
	recv_counter[aid]++;
    if (data != NULL)
    {
        gRecvFlag[aid] = TRUE;
        // コピー元は2バイトアライン(4バイトアラインでない)
//        recv_counter[aid]++;
        WBT_MpParentRecvHook((u8 *)data, length, aid);
    }
    else
    {
        gRecvFlag[aid] = FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ChildReceiveCallback

  Description:  子機として親機からのデータ受信時に呼び出される関数。

  Arguments:    aid     - 送信元親機の aid (常に 0)
                data    - 受信データへのポインタ (NULL で切断通知)
                length  - 受信データのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ChildReceiveCallback(u16 aid, u16 *data, u16 length)
{
    (void)aid;
    recv_counter[0]++;
    if (data != NULL)
    {
        gRecvFlag[0] = TRUE;
        // コピー元は2バイトアライン(4バイトアラインでない)
        WBT_MpChildRecvHook((u8 *)data, length);
    }
    else
    {
        gRecvFlag[0] = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         BlockTransferCallback

  Description:  ブロック転送状態通知関数。

  Arguments:    arg     - 通知元 WM 関数のコールバックポインタ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void BlockTransferCallback(void *arg)
{
    int connectMode = WH_GetConnectMode();
    
    switch (((WMCallback*)arg)->apiid)
    {
    case WM_APIID_START_MP:
        {                              /* MP ステート開始 */
           WMStartMPCallback *cb = (WMStartMPCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_MP_START:
                if (connectMode == WH_CONNECTMODE_MP_PARENT)
                {
                    ParentSendCallback();
                }
                else if (connectMode == WH_CONNECTMODE_MP_CHILD)
                {
                    WBT_SetOwnAid(WH_GetCurrentAid());
                    mfprintf(tc[2], "aid = %d\n", WH_GetCurrentAid());
                    bt_start();
                    ChildSendCallback();
                }
                break;
            }
        }
        break;
    case WM_APIID_SET_MP_DATA:
        {                              /* 単発の MP 通信完了 */
            if (connectMode == WH_CONNECTMODE_MP_PARENT)
            {
                if (connected_bitmap != 0)
                {
                    ParentSendCallback();
                }
            }
            else if (connectMode == WH_CONNECTMODE_MP_CHILD)
            {
                ChildSendCallback();
            }
        }
        break;
    case WM_APIID_START_PARENT:
        {                              /* 新規の子機接続 */
            WMStartParentCallback *cb = (WMStartParentCallback *)arg;
            if (connectMode == WH_CONNECTMODE_MP_PARENT)
            {
                switch (cb->state)
                {
                case WM_STATECODE_CONNECTED:
                    if (connected_bitmap == 0)
                    {
                        ParentSendCallback();
                    }
                    connected_bitmap |= (1 << cb->aid);
                    break;
                case WM_STATECODE_DISCONNECTED:
                    connected_bitmap &= ~(1 << cb->aid);
                    break;
                }
            }
        }
        break;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ParentSendCallback

  Description:  親機として子機へのデータ送信時に呼び出される関数。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ParentSendCallback(void)
{
    const u16 size = (u16)WBT_MpParentSendHook(gSendBuf, WC_PARENT_DATA_SIZE_MAX);
    send_counter[0]++;
    (void)WH_SendData(gSendBuf, size, NULL);
}


/*---------------------------------------------------------------------------*
  Name:         ChildSendCallback

  Description:  子機として親機からのデータ受信時に呼び出される関数。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ChildSendCallback(void)
{
    const u16 size = (u16)WBT_MpChildSendHook(gSendBuf, WC_CHILD_DATA_SIZE_MAX);
    send_counter[0]++;
    (void)WH_SendData(gSendBuf, size, NULL);
}

/*---------------------------------------------------------------------------*
  Name:         KeyRead

  Description:  キー入力情報を編集する。
                押しトリガ、離しトリガ、押し継続リピートを検出。

  Arguments:    pKey  - 編集するキー入力情報構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void KeyRead(KeyInfo * pKey)
{
    static u16 repeat_count[12];
    int     i;
    u16     r;

    r = PAD_Read();
    pKey->trg = 0x0000;
    pKey->up = 0x0000;
    pKey->rep = 0x0000;

    for (i = 0; i < 12; i++)
    {
        if (r & (0x0001 << i))
        {
            if (!(pKey->cnt & (0x0001 << i)))
            {
                pKey->trg |= (0x0001 << i);     // 押しトリガ
                repeat_count[i] = 1;
            }
            else
            {
                if (repeat_count[i] > KEY_REPEAT_START)
                {
                    pKey->rep |= (0x0001 << i); // 押し継続リピート
                    repeat_count[i] = KEY_REPEAT_START - KEY_REPEAT_SPAN;
                }
                else
                {
                    repeat_count[i]++;
                }
            }
        }
        else
        {
            if (pKey->cnt & (0x0001 << i))
            {
                pKey->up |= (0x0001 << i);      // 離しトリガ
            }
        }
    }
    pKey->cnt = r;                     // 未加工キー入力
}

/*---------------------------------------------------------------------------*
  Name:         ClearString

  Description:  仮想スクリーンをクリアする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClearString(int vram_num)
{
    MI_DmaClear32(0, (void *)&(g_screen[vram_num]), VRAM_SIZE);
}


/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  仮想スクリーンに文字列を配置する。文字列は32文字まで。

  Arguments:    x       - 文字列の先頭を配置する x 座標( × 8 ドット )。
                y       - 文字列の先頭を配置する y 座標( × 8 ドット )。
                palette - 文字の色をパレット番号で指定。
                text    - 配置する文字列。終端文字はNULL。
                ...     - 仮想引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...)
{
    va_list vlist;
    char    temp[32 + 2];
    s32     i;

    va_start(vlist, text);
    (void)OS_VSNPrintf(temp, 32, text, vlist);
    va_end(vlist);
    *(u16 *)(&temp[31]) = 0x0000;
    for (i = 0;; i++)
    {
        if (temp[i] == 0x00)
        {
            break;
        }
        tc[1]->screen[(y * 32) + x + i] = (u16)((palette << 12) | temp[i]);
    }
}


/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  メインメモリ上のアリーナにてメモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    mprintf(" arena lo = %08x\n", OS_GetMainArenaLo());
    mprintf(" arena hi = %08x\n", OS_GetMainArenaHi());

    // OS_Initは呼ばれているという前提
    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
