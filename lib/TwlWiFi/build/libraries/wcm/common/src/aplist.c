/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - WCM - libraries
  File:     aplist.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-10#$
  $Rev: 1108 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/
#include "wcm_private.h"
#include "wcm_message.h"

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#if WCM_DEBUG

// 警告文用テキスト雛形
#endif

/*---------------------------------------------------------------------------*
    内部関数プロトタイプ
 *---------------------------------------------------------------------------*/
static WCMApList*   WcmAllocApList(void);
static void         WcmFreeApList(WCMApList* aplist);
static WCMApList*   WcmGetOldestApList(void);
static WCMApList*   WcmGetLastApList(void);
static WCMApList*   WcmSearchApList(u8* bssid);
static WCMApList*   WcmSearchIndexedApList(u32 index);
static void         WcmAppendApList(WCMApList* aplist);

/*---------------------------------------------------------------------------*
  Name:         WCM_ClearApList

  Description:  AP 情報保持リストの内容を消去して初期状態に戻す。
                ロック状態如何に関わらず情報保持領域をクリアする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCM_ClearApList(void)
{
    OSIntrMode  e = OS_DisableInterrupts();
    WCMWork*    w = WCMi_GetSystemWork();

    // 初期化済みを確認
    if (w == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return;
    }

    // AP 情報保持リスト用バッファを必要に応じてクリア
    if ((w->config.pbdbuffer != NULL) && (w->config.nbdbuffer > 0))
    {
        MI_CpuClear8(w->config.pbdbuffer, (u32) (w->config.nbdbuffer));
    }

    (void)OS_RestoreInterrupts(e);
}

/*---------------------------------------------------------------------------*
  Name:         WCM_CountApList

  Description:  AP 情報保持リスト内で保持されている AP 情報管理ブロック数を
                取得する。
    NOTICE:     WCM_LockApList関数にてロックされていない場合には関数呼出し後に
                割り込みによってブロック数は増減する可能性がある点に注意。

  Arguments:    None.

  Returns:      s32     -   AP 情報管理ブロック数を返す。
                            正常にブロック数を取得できない場合には 0 を返す。
 *---------------------------------------------------------------------------*/
s32 WCM_CountApList(void)
{
    OSIntrMode          e = OS_DisableInterrupts();
    WCMWork*        w = WCMi_GetSystemWork();
    s32         count = 0;
    WCMApListHeader*    pHeader;

    // 初期化済みを確認
    if (w == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return 0;
    }

    // AP 情報保持リストから、保持している AP 情報管理ブロック数を取得
    pHeader = (WCMApListHeader *) (w->config.pbdbuffer);
    if ((pHeader != NULL) && (w->config.nbdbuffer > sizeof(WCMApListHeader)))
    {
        count = (s32) (pHeader->count);
    }

    // 正常終了
    (void)OS_RestoreInterrupts(e);
    return count;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_LockApList

  Description:  AP 情報保持リストのロック状態を変更する。
    NOTICE:     ロック中は割り込みによって AP を発見した場合でも AP 情報保持
                リストの内容は変更されず、発見した AP 情報は破棄される点に注意。

  Arguments:    lock    -   WCM_APLIST_LOCK     : ロックする。
                            WCM_APLIST_UNLOCK   : ロックを解放する。
                            その他              : WCM_APLIST_LOCK とみなす。

  Returns:      s32     -   呼び出し前のロック状態を返す。
                            WCM_APLIST_LOCK     : ロックされていた。
                            WCM_APLIST_UNLOCK   : ロックは解放されていた。
 *---------------------------------------------------------------------------*/
s32 WCM_LockApList(s32 lock)
{
    OSIntrMode  e = OS_DisableInterrupts();
    WCMWork*    w = WCMi_GetSystemWork();

    // 初期化済みを確認
    if (w == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return 0;
    }

    // 引数に合わせてロック状態を変更
    if (lock != WCM_APLIST_UNLOCK)
    {
        lock = (w->apListLock ? WCM_APLIST_LOCK : WCM_APLIST_UNLOCK);
        w->apListLock = WCM_APLIST_LOCK;
    }
    else
    {
        lock = (w->apListLock ? WCM_APLIST_LOCK : WCM_APLIST_UNLOCK);
        w->apListLock = WCM_APLIST_UNLOCK;
    }

    // 正常終了
    (void)OS_RestoreInterrupts(e);
    return lock;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_PointApList

  Description:  AP 情報保持リスト内から、指定された index が割り当てられている
                AP 情報を抽出する。
    NOTICE:     WCM_LockApList関数によってロックされていない場合には関数呼出し
                後に割り込みによって取得したポインタが示すバッファの内容が書き
                換わる可能性がある点に注意。

  Arguments:    index       -   index を指定する。WCM_CountApList関数によって
                                得られる数 未満の index を指定する必要がある。

  Returns:      WCMBssDesc* -   抽出した AP 情報へのポインタを返す。
                                抽出に失敗した場合は NULL を返す。
 *---------------------------------------------------------------------------*/
WMBssDesc*  WCM_PointApList(s32 index)
{
    return (WMBssDesc*) WCM_PointApListEx(index);
}

WCMBssDesc* WCM_PointApListEx(s32 index)
{
    OSIntrMode  e = OS_DisableInterrupts();
    WCMWork*    w = WCMi_GetSystemWork();
    WCMBssDesc*  bd = NULL;
    WCMApList*  p;

    // 初期化済みを確認
    if (w == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return NULL;
    }

    // index で指定された AP 情報管理ブロックを検索
    p = WcmSearchIndexedApList((u32) index);

    // 見つからなかった場合は異常終了
    if (p == NULL)
    {
        WCMi_Warning(WCMi_WText_NoIndexData, index);
        (void)OS_RestoreInterrupts(e);
        return NULL;
    }

    // 正常終了
    bd = (WCMBssDesc *) (p->data);
    (void)OS_RestoreInterrupts(e);
    return bd;
}

/*---------------------------------------------------------------------------*
  Name:         WCM_PointApListLinkLevel

  Description:  AP 情報保持リスト内から、指定された index が割り当てられている
                AP 情報が登録された際の電波強度を抽出する。

  Arguments:    index       -   index を指定する。WCM_CountApList関数によって
                                得られる数 未満の index を指定する必要がある。

  Returns:      WMLinkLevel -   抽出した AP 情報登録時の電波強度を返す。
                                抽出に失敗した場合は WM_LINK_LEVEL_0 を返す。
 *---------------------------------------------------------------------------*/
WMLinkLevel WCM_PointApListLinkLevel(s32 index)
{
    OSIntrMode  e = OS_DisableInterrupts();
    WCMWork*    w = WCMi_GetSystemWork();
    WMLinkLevel ll = WM_LINK_LEVEL_0;
    WCMApList*  p;

    // 初期化済みを確認
    if (w == NULL)
    {
        WCMi_Warning(WCMi_WText_NotInit);
        (void)OS_RestoreInterrupts(e);
        return WM_LINK_LEVEL_0;
    }

    // index で指定された AP 情報管理ブロックを検索
    p = WcmSearchIndexedApList((u32) index);

    // 見つからなかった場合は異常終了
    if (p == NULL)
    {
        WCMi_Warning(WCMi_WText_NoIndexData, index);
        (void)OS_RestoreInterrupts(e);
        return WM_LINK_LEVEL_0;
    }

    // 正常終了
    ll = (WMLinkLevel) (p->linkLevel);
    (void)OS_RestoreInterrupts(e);
    return ll;
} 

/*---------------------------------------------------------------------------*
  Name:         WCMi_EntryApList

  Description:  スキャンの結果 AP が発見された際に呼び出される内部関数。
                ロックが解放されている状態であれば、AP 情報保持リストを
                config 設定に従って編集する。
    NOTICE:     割り込み禁止中に呼び出されることを前提としているため、任意の
                場所からの呼び出しは禁止。

  Arguments:    bssDesc     -   発見された AP 情報へのポインタ。
                linkLevel   -   AP が発見された際の電波強度。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_EntryApList(WCMBssDesc* bssDesc, u16 linkLevel)
{
    WCMWork*    w = WCMi_GetSystemWork();
    WCMApList*  aplist;

    // 初期化確認
    if (w == NULL)
    {
        return;
    }

    // AP リスト編集許可確認
    if (w->apListLock != WCM_APLIST_UNLOCK)
    {
        return;
    }

    // AP 以外の NITRO 親機の場合はスキップ
#ifdef SDK_TWL
    if (OS_IsRunOnTwl())
    {
        if (w->deviceId == WCM_DEVICEID_NWM)
        {
            if (WCMi_GetGameInfo(bssDesc) != NULL)
            {
                return;
            }
        }
    }
#endif
    if (w->deviceId == WCM_DEVICEID_WM)
    {
        if (bssDesc->gameInfoLength != 0)
        {
            return;
        }
    }

    // 編集すべき AP 情報保持バッファへのポインタを取得
    {
        // 既に情報を保持されている AP かどうかを判定
        aplist = WcmSearchApList(bssDesc->bssid);

        // 情報の保持されていない AP であった場合
        if (aplist == NULL)
        {
            // 新規に情報を保持するメモリエリアを確保
            aplist = WcmAllocApList();
        }

        // 情報を保持するバッファが一杯で、かつ"交換"設定になっている場合
        if ((aplist == NULL) && (w->config.nbdmode == WCM_APLIST_MODE_EXCHANGE))
        {
            // 最も情報が古い AP 情報が保持されているメモリエリアを取得
            aplist = WcmGetOldestApList();
        }
    }

    // bssDesc から不要な Infomation Elements を取り除いたものをコピー
    // AP 情報保持バッファを編集
    if (aplist != NULL)
    {
        WCMBssDesc* destBssDesc = (WCMBssDesc*) aplist->data;

        WCMi_BssDescCopy(bssDesc, destBssDesc); //BssDesc のコピー
        aplist->linkLevel = linkLevel;

        // リストの最後尾 ( = 最新の AP 情報 ) に移動
        WcmAppendApList(aplist);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmAllocApList

  Description:  AP 情報保持リスト領域から、新たな AP 情報を管理するブロックを
                割り当てる。確保するブロックの index は 0 から順番とする。

  Arguments:    None.

  Returns:      WCMApList*  -   割り当てた AP 情報管理ブロックへのポインタ。
                                割り当てに失敗した場合は NULL を返す。
 *---------------------------------------------------------------------------*/
static WCMApList* WcmAllocApList(void)
{
    WCMWork*        w = WCMi_GetSystemWork();
    WCMApListHeader*    pHeader = (WCMApListHeader *) (w->config.pbdbuffer);
    WCMApList*          p = NULL;

    // AP 情報保持リストが使用可能な状態にあることを確認
    if ((pHeader != NULL) && (w->config.nbdbuffer > sizeof(WCMApListHeader)))
    {
        // いくつブロックを確保可能か計算
        u32 maxBlock = (w->config.nbdbuffer - sizeof(WCMApListHeader)) / sizeof(WCMApList);

        // ひとつでもブロックを確保可能か
        if (maxBlock > 0)
        {
            s32 i;

            // 余剰メモリブロックがあるか
            if (maxBlock > pHeader->count)
            {
                // 余剰メモリブロックを検索
                for (i = 0; i < maxBlock; i++)
                {
                    p = (WCMApList *) ((u32) pHeader + sizeof(WCMApListHeader) + (i * sizeof(WCMApList)));
                    if (p->state == WCM_APLIST_BLOCK_EMPTY)
                    {
                        break;
                    }
                }

                if (i < maxBlock)
                {
                    // メモリブロックを"使用中"に変更
                    p->state = WCM_APLIST_BLOCK_OCCUPY;
                    p->index = pHeader->count;

                    // リストの最後尾に新規に確保したブロックを追加
                    p->next = NULL;
                    p->previous = pHeader->tail;
                    pHeader->tail = p;
                    if (p->previous)
                    {
                        ((WCMApList *) (p->previous))->next = p;
                    }
                    else
                    {
                        pHeader->head = p;
                    }

                    // リスト数をインクリメント
                    pHeader->count++;
                }
            }
        }
    }

    return p;
}

/*---------------------------------------------------------------------------*
  Name:         WcmFreeApList

  Description:  AP 情報保持リスト領域から割り当てられている AP 情報を管理する
                ブロックを解放して再利用できる状態に戻す。
                指定されたブロックが存在しない場合など、正常にブロック解放処理を
                行えない場合でも何も通知しない。
    NOTICE:     リスト管理の例を示すために存在し、内部では使用されていない。

  Arguments:    aplist  -   解放する AP 情報管理ブロックへのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmFreeApList(WCMApList* aplist)
{
    WCMWork*        w = WCMi_GetSystemWork();
    WCMApListHeader*    pHeader = (WCMApListHeader *) (w->config.pbdbuffer);
    WCMApList*          p;

    // パラメータを確認
    if (aplist == NULL)
    {
        return; // パラメータ異常
    }

    // AP 情報保持リストが使用可能な状態にあることを確認
    if ((pHeader != NULL) && (w->config.nbdbuffer > sizeof(WCMApListHeader)))
    {
        // ポインタの一致する AP情報管理ブロックをリストから検索
        p = pHeader->head;
        while (p)
        {
            if (p == aplist)
            {
                // リストから除去
                if (aplist->previous)
                {
                    ((WCMApList *) (aplist->previous))->next = aplist->next;
                }
                else
                {
                    pHeader->head = aplist->next;
                }

                if (aplist->next)
                {
                    ((WCMApList *) (aplist->next))->previous = aplist->previous;
                }
                else
                {
                    pHeader->tail = aplist->previous;
                }

                // 除去したブロックの index を引き継ぎ( index の歯抜けを防ぐ為 )
                if (pHeader->count > 0)
                {
                    u32 index = aplist->index;

                    aplist = WcmSearchIndexedApList((u32) (pHeader->count - 1));
                    if (aplist != NULL)
                    {
                        aplist->index = index;
                    }

                    pHeader->count--;
                }

                // 使用していたブロックを解放
                MI_CpuClear8(aplist, sizeof(WCMApList));
            }

            p = p->next;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WcmGetOldestApList

  Description:  AP 情報保持リスト領域内に管理されている AP 情報から、最も古くに
                登録された AP 情報管理ブロックを検索する。

  Arguments:    None.

  Returns:      WCMApList*  -   AP 情報管理ブロックへのポインタを返す。
                                リストに一つも AP 情報が登録されていない場合には
                                NULL を返す。
 *---------------------------------------------------------------------------*/
static WCMApList* WcmGetOldestApList(void)
{
    WCMWork*        w = WCMi_GetSystemWork();
    WCMApListHeader*    pHeader = (WCMApListHeader *) (w->config.pbdbuffer);

    // AP 情報保持リストが使用可能な状態にあることを確認
    if ((pHeader != NULL) && (w->config.nbdbuffer > sizeof(WCMApListHeader)))
    {
        // リストの先頭の AP 情報管理ブロックが最古
        return pHeader->head;
    }

    // 異常終了
    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         WcmGetLastApList

  Description:  AP 情報保持リスト領域内に管理されている AP 情報から、最も新しく
                登録された AP 情報管理ブロックを検索する。
    NOTICE:     リスト管理の例を示すために存在し、内部では使用されていない。

  Arguments:    None.

  Returns:      WCMApList*  -   AP 情報管理ブロックへのポインタを返す。
                                リストに一つも AP 情報が登録されていない場合には
                                NULL を返す。
 *---------------------------------------------------------------------------*/
static WCMApList* WcmGetLastApList(void)
{
    WCMWork*        w = WCMi_GetSystemWork();
    WCMApListHeader*    pHeader = (WCMApListHeader *) (w->config.pbdbuffer);

    // AP 情報保持リストが使用可能な状態にあることを確認
    if ((pHeader != NULL) && (w->config.nbdbuffer > sizeof(WCMApListHeader)))
    {
        // リストの最後尾の AP 情報管理ブロックが最新
        return pHeader->tail;
    }

    // 異常終了
    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         WcmSearchApList

  Description:  AP 情報保持リスト領域内に管理されている AP 情報から、指定された
                BSSID を持つ AP 情報管理ブロックを検索する。

  Arguments:    bssid       -   検索条件の BSSID へのポインタを指定する。

  Returns:      WCMApList*  -   AP 情報管理ブロックへのポインタを返す。
                                BSSID の合致する AP 情報が存在しない場合には
                                NULL を返す。
 *---------------------------------------------------------------------------*/
static WCMApList* WcmSearchApList(u8* bssid)
{
    WCMWork*        w = WCMi_GetSystemWork();
    WCMApListHeader*    pHeader = (WCMApListHeader *) (w->config.pbdbuffer);
    WCMApList*          p = NULL;

    // パラメータ確認
    if (bssid == NULL)
    {
        return NULL;    // パラメータ異常
    }

    // AP 情報保持リストが使用可能な状態にあることを確認
    if ((pHeader != NULL) && (w->config.nbdbuffer > sizeof(WCMApListHeader)))
    {
        // BSSID の一致する AP 情報管理ブロックを検索
        p = pHeader->head;
        while (p)
        {
            if (WCM_CompareBssID(((WMBssDesc *) (p->data))->bssid, bssid))
            {
                break;
            }

            p = p->next;
        }
    }

    return p;
}

/*---------------------------------------------------------------------------*
  Name:         WcmSearchIndexedApList

  Description:  AP 情報保持リスト領域内に管理されている AP 情報から、指定された
                index を持つ AP 情報管理ブロックを検索する。

  Arguments:    index       -   index を指定する。

  Returns:      WCMApList*  -   AP 情報管理ブロックへのポインタを返す。
                                index の合致する AP 情報が存在しない場合には
                                NULL を返す。
 *---------------------------------------------------------------------------*/
static WCMApList* WcmSearchIndexedApList(u32 index)
{
    WCMWork*        w = WCMi_GetSystemWork();
    WCMApListHeader*    pHeader = (WCMApListHeader *) (w->config.pbdbuffer);
    WCMApList*          p = NULL;

    // AP 情報保持リストが使用可能な状態にあることを確認
    if ((pHeader != NULL) && (w->config.nbdbuffer > sizeof(WCMApListHeader)))
    {
        p = pHeader->head;
        while (p)
        {
            // index の一致する AP 情報管理ブロックを検索
            if (p->index == index)
            {
                break;
            }

            p = p->next;
        }
    }

    return p;
}

/*---------------------------------------------------------------------------*
  Name:         WcmAppendApList

  Description:  AP 情報保持リスト内で管理されている AP 情報管理ブロックを最新
                情報として登録し直す。この際、ブロックに与えられている index は
                変更されない。リストに管理されていない場合には新規に追加された
                ブロックとして扱う。

  Arguments:    aplist  -   登録し直す AP 情報管理ブロックへのポインタを指定。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WcmAppendApList(WCMApList* aplist)
{
    WCMWork*        w = WCMi_GetSystemWork();
    WCMApListHeader*    pHeader = (WCMApListHeader *) (w->config.pbdbuffer);
    WCMApList*          p = NULL;

    // パラメータ確認
    if (aplist == NULL)
    {
        return; // パラメータ異常
    }

    // AP 情報保持リストが使用可能な状態にあることを確認
    if ((pHeader != NULL) && (w->config.nbdbuffer > sizeof(WCMApListHeader)))
    {
        // 既にリストに存在することを確認
        p = pHeader->head;
        while (p)
        {
            if (p == aplist)
            {
                // 一旦リストから除去
                if (p->previous)
                {
                    ((WCMApList *) (p->previous))->next = p->next;
                }
                else
                {
                    pHeader->head = p->next;
                }

                if (p->next)
                {
                    ((WCMApList *) (p->next))->previous = p->previous;
                }
                else
                {
                    pHeader->tail = p->previous;
                }
                break;
            }

            p = p->next;
        }

        // リストの最後尾に追加
        aplist->next = NULL;
        aplist->previous = pHeader->tail;
        pHeader->tail = aplist;
        if (aplist->previous)
        {
            ((WCMApList *) (aplist->previous))->next = aplist;
        }
        else
        {
            pHeader->head = aplist;
        }

        // 新規に追加した場合はリスト数もインクリメント
        if (p == NULL)
        {
            aplist->index = pHeader->count;
            pHeader->count++;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_BssDescCopy

  Description:  WCMBssDesc を不要な Infomation Elements を除いてコピーする。

  Arguments:    srcBssDesc  - コピー元の AP 情報
                destBssDesc - コピー先の AP 情報

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_BssDescCopy(WCMBssDesc* srcBssDesc, WCMBssDesc* destBssDesc)
{
    WCMWork*        w = WCMi_GetSystemWork();
    
    MI_CpuClearFast(destBssDesc, WCM_APLIST_SIZE);
    MI_CpuCopyFast(srcBssDesc, destBssDesc, WCM_APLIST_SIZE - WM_SIZE_GAMEINFO );       // IE 以外をコピー

    destBssDesc -> otherElementCount = 0;

    // BSSDesc.length は TWL 無線ではバイト単位、NITRO 無線ではハーフワード単位
    if( w -> deviceId == WCM_DEVICEID_WM )
    {
        destBssDesc -> length            = (WCM_APLIST_SIZE - WM_SIZE_GAMEINFO) / sizeof(u16);
    }
    else
    {
        destBssDesc -> length            = (WCM_APLIST_SIZE - WM_SIZE_GAMEINFO);
    }

    // 必要な Infomation Elements のみを抽出（ただし 192 Byte に収まる範囲で）
    {
        u8* pSrc    =(u8*)&(srcBssDesc->gameInfo);
        u8* pDest   =(u8*)&(destBssDesc->gameInfo);
        u8  ieLength=0;
        u16 i       =0; //異常な BssDesc では 256 個を超える IE が含まれている可能性がある

        const u8 wpaTag[4]      ={0x00,0x50,0xF2,0x01};

        const u8 wmmTag[5]      ={0x00,0x50,0xF2,0x02,0x00};
//        const u8 wmmParamTag[5] ={0x00,0x50,0xF2,0x02,0x01};

        for( i=0; i < srcBssDesc->otherElementCount; i++ )
        {
            u8 id  = pSrc[0];
            u8 len = pSrc[1];

            if( (id == WCM_IE_ID_EX_SUPPORT_RATE)   ||
                (id == WCM_IE_ID_ERP_INFORMATION_A)  ||
                (id == WCM_IE_ID_ERP_INFORMATION_B)  ||
                (id == WCM_IE_ID_COUNTRY)           ||
                (id == WCM_IE_ID_RSN) )
            {
                /* 必要なIE が見つかったのでコピー */
                MI_CpuCopy(pSrc, pDest, (u8)(len + 2) );

                pDest += len + 2;
                ieLength += len + 2;
                destBssDesc->otherElementCount++;
            }
            /* WPA IE は別途処理 */
            else if(( id == WCM_IE_ID_VENDOR_SPECIFIC) &&
                    ((MI_CpuComp8(pSrc + 2, wpaTag, 4) == 0) || (MI_CpuComp8(pSrc + 2, wmmTag, 5) == 0)) )
            {
                MI_CpuCopy(pSrc, pDest, (u8)(len + 2) );

                pDest += len + 2;
                ieLength += len + 2;
                destBssDesc->otherElementCount++;
            }

            pSrc += len +2;
        }
        // BSSDesc.length は TWL 無線ではバイト単位、NITRO 無線ではハーフワード単位
        if( w -> deviceId == WCM_DEVICEID_WM )
        {
            destBssDesc -> length += ieLength + 1 / sizeof(u16);
        }
        else // TWL 無線
        {
            destBssDesc -> length += ieLength;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WCMi_GetGameInfo

  Description:  WCMBssDesc の中身を調査して MP 親機のビーコンだった場合 WMGameInfo を取得する

  Arguments:    bssDesc  - AP 情報

  Returns:      WMGameInfo
 *---------------------------------------------------------------------------*/
WMGameInfo* WCMi_GetGameInfo(const WCMBssDesc* bssDesc)
{
    WCMWork*        w = WCMi_GetSystemWork();
    const u8 tag[4] = { 0x00, 0x09, 0xBF, 0x00 };   // GameInfo 固有のタグ
    
    /* NULL check */
    if (bssDesc == NULL)
    {
        return NULL;
    }
    
#ifdef SDK_TWL
    if (OS_IsRunOnTwl())
    {
        if(w->deviceId == WCM_DEVICEID_NWM) /* TWL 無線を使った場合 */
        {
            NWMInfoElements *elem = NULL;
            // WM と NWM の BssDesc における IE の扱いが少し異なるため処理を分ける
            if (bssDesc->elementCount > 0)
            {
                elem = NWM_GetVenderInfoElements((NWMBssDesc*)bssDesc, NWM_ELEMENTID_NINTENDO, (u8*)tag);
            }

            if (elem != NULL)
            {
                WMGameInfo* gameInfo = (WMGameInfo*)((elem->element) + 4);
                if (gameInfo->magicNumber == 0x0001) /* WM のリファレンスに従い、マジックナンバーをチェック */
                {
                    return gameInfo;
                }
            }
        }
    }
    else
#endif
    {
        if(w->deviceId == WCM_DEVICEID_WM) /* NITRO 無線を使った場合 */
        {
            if( (bssDesc->gameInfoLength != 0) && (WM_IsValidGameBeacon((WMBssDesc*)bssDesc) == TRUE) )
            {
                return &(((WMBssDesc*)bssDesc)->gameInfo);
            }
        }
    }
        return NULL;
}
/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
