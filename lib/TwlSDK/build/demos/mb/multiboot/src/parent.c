/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot
  File:     parent.c

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
#include <nitro.h>
#include <nitro/wm.h>
#include <nitro/mb.h>

#include "common.h"
#include "dispfunc.h"


/*
 * 親機側の基本的な処理.
 *
 * MBライブラリのサンプルはマルチブート機能を使うため、
 * 同じ通信環境(無線あるいは有線)どうしの開発機材を複数必要とします。
 * $TwlSDK/bin/ARM9-TS/Rom/ ディレクトリにある
 * mb_child_NITRO.srl, mb_child_TWL.srl プログラムが最終実機におけるマルチブート子機と同等の
 * 機能を提供するサンプルになっていますので、
 * このバイナリをサンプルプログラムと同じ方法で他の機材に読み込み、
 * 一緒に実行してください。
 */


/******************************************************************************/
/* variables */

/*
 * ダウンロードプログラムのセグメント情報を読み込むメモリ.
 */

static u8 *p_segbuf[GAME_PROG_MAX];
static u8 cursor = 0;
static MBUserInfo myUser;
static u16 parent_channel = PARENT_CHANNEL;
static u16 parent_send_size = 256;
static u8 entry_max = 15;

/*
 * このサンプル内で親機側から子機への応答を自動制御するためのスイッチ.
 *
 * 子機からの要求は親機の応答を待ちますので,
 * この応答タイミングを全子機でそろえたり個別にしたりすることで
 * マルチブートアプリケーションの動作を設定できます.
 *
 * 例として, 単に体験版を配信するだけならば個別に応答し,
 * 複数人対戦であればエントリー人数到達時に一括応答する, 等が考えられます.
 */

static u8 auto_accept = 0;             /* 接続許可を自動化 */
static u8 auto_send = 0;               /* ダウンロード許可を自動化 */
static u8 auto_boot = 0;               /* ブート許可を自動化 */

/******************************************************************************/
/* function */

/* ファイルバッファポインタを初期化 */
void InitFileBuffer(void)
{
    int     i;

    for (i = 0; i < GAME_PROG_MAX; i++)
    {
        p_segbuf[i] = NULL;
    }

}

/* 確保ずみのファイルバッファを開放 */
static void FreeFileBuffer(void)
{
    int     i;

    for (i = 0; i < GAME_PROG_MAX; i++)
    {
        if (p_segbuf[i])
        {
            OS_Free(p_segbuf[i]);
            p_segbuf[i] = NULL;
        }

    }

}

/* MBUserInfoの生成 */
static void MakeUserInfo(MBUserInfo *user, u8 favoriteColor, u8 playerNo, u16 *name,
                         u16 name_length)
{
    SDK_ASSERT(user != NULL);
    user->favoriteColor = favoriteColor;
    user->nameLength = (u8)(name_length);
    MI_CpuCopy8((char *)name, (char *)user->name, (u32)(name_length * 2));
    user->playerNo = playerNo;
}

/* Channel を巡回変更していく */
static BOOL changeChannel(u16 *p_channel)
{
    u16     channel_bmp, channel, i;

    /* channel bitmap 取得 */
    channel_bmp = WM_GetAllowedChannel();

    /* 使用可能なchannelが無い場合は、OS_Panicをコール． */
    if (channel_bmp == 0)
    {
        OS_Panic("No Available Parent channel\n");
        return FALSE;
    }

    /* 使用可能なchannelが存在した場合． */
    for (i = 0, channel = *p_channel;
         i < 16; i++, channel = (u16)((channel == 16) ? 1 : channel + 1))
    {
        if (channel_bmp & (1 << (channel - 1)))
        {
            /* 検出されたchannelが前のものと
               同じだった場合は、別のchannelを
               検索する。
             */
            if (*p_channel != channel)
            {
                *p_channel = channel;
                break;
            }
        }

    }

    return TRUE;

}

/* 親機ファイルダウンロード状態通知コールバック */
static void ParentStateCallback(u16 aid, u32 status, void *arg)
{

    switch (status)
    {
    case MB_COMM_PSTATE_INIT_COMPLETE:
        /* 初期化処理 完了 */
        {
            WMStartParentCallback *p = (WMStartParentCallback *)arg;
            BgSetMessage(WHITE, "MB lib Init complete");
        }
        break;

    case MB_COMM_PSTATE_CONNECTED:
        /* 子機から接続が来た瞬間の通知 */
        {
            WMStartParentCallback *p = (WMStartParentCallback *)arg;

            BgSetMessage(YELLOW, "Connected [%2d]", aid);
            BgSetMessage(YELLOW, "MACAddress %04x %04x %04x",
                         p->macAddress[0], p->macAddress[1], p->macAddress[2]);
        }
        break;

    case MB_COMM_PSTATE_DISCONNECTED:
        /* 子機が接続を切った時の通知 */
        {
            WMStartParentCallback *p = (WMStartParentCallback *)arg;

            BgSetMessage(RED, "Disconnected [%2d]", aid);
            /* 切断時のReasonCode表示 */
            //                  BgSetMessage( WHITE, "reason code : %2d", p->reason);
        }
        break;

    case MB_COMM_PSTATE_KICKED:
        /* 子機をキックした時の通知 */
        BgSetMessage(RED, "Entry Refused [%2d]", aid);
        break;

    case MB_COMM_PSTATE_REQ_ACCEPTED:
        /* 子機からのダウンロード要求を受けた時の通知 */
        BgSetMessage(YELLOW, "Download Request [%2d]", aid);
        break;

    case MB_COMM_PSTATE_SEND_PROCEED:
        /* 子機へバイナリ送信を開始する時の通知 */
        BgSetMessage(CYAN, "Start Sending [%2d]", aid);
        break;

    case MB_COMM_PSTATE_SEND_COMPLETE:
        /* 子機へのバイナリ送信が終了した時の通知 */
        BgSetMessage(CYAN, "Send Completed [%2d]", aid);
        if (auto_boot == 1)
        {
            /* 自動ブート要求 */
            (void)MB_CommBootRequest(aid);
            BgSetMessage(WHITE, "-->Boot Request [%2d]", aid);
        }
        break;

    case MB_COMM_PSTATE_BOOT_REQUEST:
        /* 子機へのブート要求送信時の通知 */
        BgSetMessage(CYAN, "Send boot request [%2d]", aid);
        break;

    case MB_COMM_PSTATE_BOOT_STARTABLE:
        /* 一連のダウンロード処理が終わり、マルチブート対応アプリを起動可になった時の通知 */
        BgSetMessage(YELLOW, "Boot ready [%2d]", aid);
        break;

    case MB_COMM_PSTATE_REQUESTED:
        /* 子機からエントリーリクエストが来た瞬間の通知 */
        BgSetMessage(YELLOW, "Entry Requested [%2d]", aid);

        if (auto_accept == 1)
        {
            /* 自動エントリー許可 */
            (void)MB_CommResponseRequest(aid, MB_COMM_RESPONSE_REQUEST_ACCEPT);
            BgSetMessage(WHITE, "-->Entry Accept [%2d]", aid);
        }
        break;

    case MB_COMM_PSTATE_MEMBER_FULL:
        /* 定員に達したゲームに子機が接続してきた時の通知 */
        BgSetMessage(RED, "Entry Member full [%2d]", aid);
        break;

    case MB_COMM_PSTATE_END:
        /* 親機が終了した時の通知 */
        BgSetMessage(WHITE, "MB lib End");
        prog_state = STATE_MENU;
        break;

    case MB_COMM_PSTATE_WAIT_TO_SEND:
        /* 子機への送信待機状態が始まった時の通知 */
        BgSetMessage(CYAN, "Waiting to send [%2d]", aid);
        if (auto_send == 1)
        {
            /* 自動送信開始 */
            (void)MB_CommStartSending(aid);
            BgSetMessage(WHITE, "-->Start Sending [%2d]", aid);
        }
        break;

    }
}



/* 親機初期化 */
void ParentInit(void)
{
    int     i;

    OSOwnerInfo MyInfo;

    OS_GetOwnerInfo(&MyInfo);
    MakeUserInfo(&myUser, MyInfo.favoriteColor, 0, (u16 *)MyInfo.nickName, MyInfo.nickNameLength);

    BgClear();
    BgSetMessage(WHITE, "Initializing Parent.");

    /*
     * MB 親機制御を開始します.
     * ここでは全ての準備が整っただけの待機状態になります.
     */

    /* 再初期化時に、以前使用していたヒープを解放. */
    FreeFileBuffer();

    /* マルチブートライブラリ初期化 */
    (void)MB_Init(cwork, &myUser, MY_GGID, MB_TGID_AUTO, MB_DMA_NO);

    /* 送受信データサイズを指定 */
    (void)MB_SetParentCommParam(parent_send_size, entry_max);
    (void)MB_CommSetParentStateCallback(ParentStateCallback);
    // MB_StartParent は内部で WM_Initialize を呼ぶ
    if(MB_StartParent(parent_channel) != MB_SUCCESS)
    {
        OS_Panic("MB_StartParent() failed.");
    }

    BgSetMessage(WHITE, "** Parameters **");
    BgSetMessage(WHITE, "channel      : %2d", parent_channel);
    BgSetMessage(WHITE, "send size    : %3d", parent_send_size);
    BgSetMessage(WHITE, "max children : %2d", entry_max);
    BgSetMessage(WHITE, "GGID:%08x TGID:%04x", MY_GGID, MB_GetTgid());

    prog_state = STATE_MB_PARENT;
    cursor = 0;

    /*
     * 続いて, ダウンロードプログラムファイル情報を登録します.
     *
     * プログラムファイル全体のうち, マルチブート起動時に必要なのは
     * ARM9-static セグメントおよび ARM7-static セグメントのみです.
     * (それ以外はオーバーレイやデータファイル等になります)
     *
     * プログラムからこれらの情報だけを読み出すために,
     * まず MB_ReadSegment() 関数を使用します.
     * MB_GetSegmentLength() 関数でサイズを取得することにより
     * このサンプルではメモリを動的に確保しています.
     *
     * プログラムファイルは FS_ReadFile() で読み出せる必要があります.
     * 通常はプログラムを CARD-ROM 内にファイルとして保持するはずなので
     * 問題はありませんが, もし特殊なマルチブートシステムを想定する場合,
     * FSArchive で独自アーカイブを構築して対処してください.
     */
    for (i = 0; i < GAME_PROG_MAX; ++i)
    {
        FSFile  file[1];
        FS_InitFile(file);

        /*
         * 子機プログラムファイルを開く.
         * このファイルは以下の MB_ReadSegment() で使用します.
         */
        if (!FS_OpenFileEx(file, mbGameList[i].romFilePathp, FS_FILEMODE_R))
        {
            OS_TPrintf("ParentInit : file cannot open (mbGameList[i].romFilePathp=\"%s)\"\n",
                       mbGameList[i].romFilePathp ? mbGameList[i].romFilePathp : "(NULL)");
        }
        else
        {
            /*
             * セグメント情報のサイズを取得.
             * 正当なダウンロードプログラムでない場合,
             * このサイズに 0 が返ってきます.
             */
            u32     length = MB_GetSegmentLength(file);
            if (length == 0)
            {
                OS_TPrintf
                    ("ParentInit : specified file may be invalid format. (mbGameList[i].romFilePathp=\"%s)\"\n",
                     mbGameList[i].romFilePathp ? mbGameList[i].romFilePathp : "(NULL)");
            }
            else
            {
                /*
                 * セグメント情報のためのメモリを確保.
                 * このメモリはサイズさえ充分であれば
                 * 静的に用意されていても構いません.
                 */
                p_segbuf[i] = (u8 *)OS_Alloc(length);
                if (!p_segbuf[i])
                {
                    OS_TPrintf("ParentInit : memory allocation failed. (%d BYTE)\n", length);
                }
                else
                {
                    /*
                     * セグメント情報をファイルから抽出.
                     * 抽出したセグメント情報は, ダウンロードプログラム配信中
                     * メインメモリ上に常駐させておく必要があります.
                     *
                     * サイズ取得が成功したにも関わらず
                     * ここで抽出処理が失敗するのは,
                     * ファイルハンドルに何か変更を与えた場合などです.
                     * (ファイルを閉じた, 位置をシークした, ...)
                     */
                    if (!MB_ReadSegment(file, p_segbuf[i], length))
                    {
                        OS_TPrintf("ParentInit : failed to extract segment\n");
                    }
                    else
                    {
                        /*
                         * 抽出したセグメント情報と MBGameRegistry で
                         * ダウンロードプログラムを登録します.
                         *
                         */

                        if (MB_RegisterFile(&mbGameList[i], p_segbuf[i]))
                        {
                            BgSetMessage(LIGHT_GREEN, "Registered\"%s\"",
                                         mbGameList[i].romFilePathp);
                        }
                        else
                        {
                            OS_TPrintf("ParentInit : failed to register file No %d\n", i);
                        }
                    }
                }
            }
        }
        if (FS_IsFile(file))
            (void)FS_CloseFile(file);
    }

}


/* 1 フレームごとの親機メイン処理 */
void ParentMain(void)
{
    char    userName[MB_USER_NAME_LENGTH * 2 + 1];
    const int children_num = MB_CommGetChildrenNumber();
    const u16 keyData = ReadKeySetTrigger(PAD_Read());
    enum
    { DISP_OX = 2, DISP_OY = 3 };
    u16     i;
    OSIntrMode enabled;
    enum
    { PSTATE_NUM = 16 };
    const char *pstate_string[] = {
        "NONE        ",                //      STATE_NONE
        "INIT OK     ",                //      STATE_INIT_COMPLETE
        "CONNECTED   ",                //      STATE_CONNECTED
        "DISCONNECTED",                //      STATE_DISCONNECTED
        "KICKED      ",                //      STATE_KICKED
        "ENTRY OK    ",                //      STATE_REQ_ACCEPTED
        "SENDING     ",                //      STATE_SEND_PROCEED
        "SEND END    ",                //      STATE_SEND_COMPLETE
        "BOOT REQ    ",                //      STATE_BOOT_REQUEST
        "BOOT READY  ",                //      STATE_BOOT_STARTABLE
        "ENTRY REQ   ",                //      STATE_REQUESTED
        "MEMBER FULL ",                //      STATE_MEMBER_FULL
        "END         ",                //      STATE_END
        "ERROR       ",                //      STATE_ERROR
        "WAIT TO SEND",                //      STATE_WAIT_TO_SEND
    };

    BgPrintf(DISP_OX, DISP_OY - 2, WHITE, "CH:%2d  Max Children:%2d", parent_channel, entry_max);
    BgPutString(DISP_OX, DISP_OY - 1, WHITE, "AID USERNAME STATE        ");
    BgPutString(DISP_OX, DISP_OY + MB_MAX_CHILD + 1, WHITE, "A:operate B:refuse SEL:restart");
    BgPutString(DISP_OX, DISP_OY + MB_MAX_CHILD + 2, WHITE, "START:operate all children");

    enabled = OS_DisableInterrupts();  /* 割り込み禁止 */

    /* 子機リストの表示 */
    for (i = 0; i < entry_max; ++i)
    {
        const u16 aid = (u16)(i + 1);
        const MBUserInfo *p_child = MB_CommGetChildUser(aid);
        int     p_state;

        BgPutString(DISP_OX, (s16)(DISP_OY + i), WHITE, "                         ");

        BgPrintf(DISP_OX + 0, (s16)(DISP_OY + i), WHITE, "%2d  ", aid);

        if (p_child && p_child->nameLength)
        {
            u8      palette = p_child->favoriteColor;
            MI_CpuCopy8(p_child->name, userName, (u32)(p_child->nameLength * 2));
            BgPutStringN((s16)(DISP_OX + 4), (s16)(DISP_OY + i), palette, userName, 8);
        }

        p_state = MB_CommGetParentState(aid);

        if (p_state < PSTATE_NUM)
        {
            BgPrintf((s16)(DISP_OX + 13), (s16)(DISP_OY + i), WHITE,
                     "%s  ", (char *)pstate_string[p_state]);
        }

        BgPutChar((s16)(DISP_OX - 2), (s16)(DISP_OY + i), RED, (char)((i == cursor) ? '*' : ' '));
    }

    (void)OS_RestoreInterrupts(enabled);        /* 割り込み禁止解除 */

    /* 全部の子機が対象 */
    if (keyData & PAD_BUTTON_START)
    {
        BOOL    result = FALSE;

        if (!auto_accept)
        {
            for (i = 1; i <= entry_max; i++)
            {
                /* エントリー許可 */
                if (MB_COMM_PSTATE_REQUESTED == MB_CommGetParentState(i))
                {
                    result = MB_CommResponseRequest(i, MB_COMM_RESPONSE_REQUEST_ACCEPT);
                }
            }
            if (TRUE == result)
                return;
        }

        if (!auto_send)
        {
            result = MB_CommStartSendingAll();
            if (TRUE == result)
                return;
        }

        if (!auto_boot)
        {
            result = MB_CommBootRequestAll();
            if (TRUE == result)
                return;
        }
    }

    /* 指定子機へ個別に送る */
    if (keyData & PAD_BUTTON_A)
    {
        const u16 aid = (u16)(cursor + 1);

        BgSetMessage(WHITE, "AID%2d State:%s", aid, pstate_string[MB_CommGetParentState(aid)]);

        /* エントリー許可 */
        if (MB_COMM_PSTATE_REQUESTED == MB_CommGetParentState(aid))
        {
            if (!auto_accept)
            {
                (void)MB_CommResponseRequest(aid, MB_COMM_RESPONSE_REQUEST_ACCEPT);
                BgSetMessage(WHITE, "-->Entry Accept [%2d]", aid);
            }
        }
        /* 送信開始 */
        else if (MB_COMM_PSTATE_WAIT_TO_SEND == MB_CommGetParentState(aid))
        {
            if (!auto_send)
            {
                (void)MB_CommStartSending(aid);
                BgSetMessage(WHITE, "-->Start Sending [%2d]", aid);
            }
        }
        else
            /* ブート要求 */
        if (TRUE == MB_CommIsBootable(aid))
        {
            if (!auto_boot)
            {
                (void)MB_CommBootRequest(aid);
                BgSetMessage(WHITE, "-->Boot Request [%2d]", aid);
            }
        }
    }
    else
        /* 指定子機へエントリー拒否 */
    if (keyData & PAD_BUTTON_B)
    {
        const u16 aid = (u16)(cursor + 1);

        BgSetMessage(WHITE, "AID%2d State:%s", aid, pstate_string[MB_CommGetParentState(aid)]);
        if (MB_COMM_PSTATE_REQUESTED == MB_CommGetParentState(aid))
        {
            if (!auto_accept)
            {
                (void)MB_CommResponseRequest(aid, MB_COMM_RESPONSE_REQUEST_KICK);
                BgSetMessage(CYAN, "Entry Refuse [%2d]", aid);
            }
        }
    }

    /* SELECT を押したら終了 -> 再初期化 */
    if (keyData & PAD_BUTTON_SELECT)
    {
        /* チャンネル変更 */
        (void)changeChannel(&parent_channel);
        MB_End();
    }

    /* カーソル番号変更 */
    if (keyData & PAD_KEY_DOWN)
    {
        (void)RotateU8(&cursor, 0, (u8)(entry_max - 1), 1);
    }
    else if (keyData & PAD_KEY_UP)
    {
        (void)RotateU8(&cursor, 0, (u8)(entry_max - 1), -1);
    }

}

/* 親機MP送信サイズをセット */
void SetParentSendSize(u16 p_size)
{
    parent_send_size = p_size;
}

/* 親機MP送信サイズを取得 */
u16 GetParentSendSize(void)
{
    return parent_send_size;
}

/* 子機最大接続数をセット */
void SetMaxEntry(u8 num)
{
    entry_max = num;
}

/* 子機最大接続数を取得 */
u8 GetMaxEntry(void)
{
    return entry_max;
}

/* 親機のオート操作設定 */
void SetAutoOperation(u8 fAccept, u8 fSend, u8 fBoot)
{
    auto_accept = fAccept;
    auto_send = fSend;
    auto_boot = fBoot;
}
