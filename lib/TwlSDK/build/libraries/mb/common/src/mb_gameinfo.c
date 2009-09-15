/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - libraries
  File:     mb_gameinfo.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

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
#include "mb_gameinfo.h"
#include "mb_common.h"
#include "mb_private.h"                // for MB_SCAN_TIME_NORMAL & MB_SCAN_TIME_LOKING & MB_OUTPUT, MB_DEBUG_OUTPUT


/*

　＊＊＊＊＊＊注意事項＊＊＊＊＊＊＊
　・スキャンロック中は、スキャン時間を親機のビーコン間隔以上にした方がいいとのこと。（200ms以上）
　　但し、これをする場合、動的にスキャン時間を変えることになるので、LifetimeCountやLockTimeCountの最大値
    や既にカウントされている値をどうするか？

*/


/*
 * 使用している wmDEMOlib 関数:
 *  ・無し
 *
 * 使用している WM API:
 *  ・WM_SetGameInfo (MBi_SendFixedBeacon, MBi_SendVolatBeacon)
 *
 *
 *
 *
 */

// define data---------------------------------------------

#define MB_GAME_INFO_LIFETIME_SEC       60      // 受信したゲーム情報の寿命（このsec値の間ビーコン受信がなければ削除される）
#define MB_SCAN_LOCK_SEC                8       // 特定親機をロックしてスキャンする最大時間
#define MB_SAME_BEACON_RECV_MAX_COUNT   3       // 親機が同じビーコンを出し続けていた場合、その親機のビーコンを受信し続ける最大カウント数。

#define MB_BEACON_DATA_SIZE         (WM_SIZE_USER_GAMEINFO - 8)
#define MB_BEACON_FIXED_DATA_SIZE   (MB_BEACON_DATA_SIZE   - 6) // ビーコン内の親機ゲーム情報固定領域のデータサイズ
#define MB_BEACON_VOLAT_DATA_SIZE   (MB_BEACON_DATA_SIZE   - 8) // ビーコン内の親機ゲーム情報流動領域のデータサイズ
#define MB_SEND_MEMBER_MAX_NUM      (MB_BEACON_VOLAT_DATA_SIZE / sizeof(MBUserInfo))    // １回のビーコンで送信できるメンバー情報の最大数

#define FIXED_NORMAL_SIZE            sizeof(MBGameInfoFixed)    // ゲーム情報固定領域のノーマルサイズ
#define FIXED_NO_ICON_SIZE          (sizeof(MBGameInfoFixed) - sizeof(MBIconInfo))      // ゲーム情報固定領域のアイコンなしサイズ
#define FIXED_FRAGMENT_MAX(size)    ( ( size / MB_BEACON_FIXED_DATA_SIZE) + ( size % MB_BEACON_FIXED_DATA_SIZE ? 1 : 0 ) )
                                                                                    // ゲーム情報固定領域をビーコン分割数

#define MB_LIFETIME_MAX_COUNT       ( MB_GAME_INFO_LIFETIME_SEC * 1000 / MB_SCAN_TIME_NORMAL + 1 )
#define MB_LOCKTIME_MAX_COUNT       ( MB_SCAN_LOCK_SEC          * 1000 / MB_SCAN_TIME_NORMAL + 1 )
                                                                                    // 親機ゲーム情報生存期間をScanTime単位に換算したもの
                                                                                    // スキャンロック時間　　をScanTime単位に換算したもの
#define MB_SCAN_COUNT_UNIT_NORMAL   ( 1 )
#define MB_SCAN_COUNT_UNIT_LOCKING  ( MB_SCAN_TIME_LOCKING / MB_SCAN_TIME_NORMAL )



typedef enum MbBeaconState
{
    MB_BEACON_STATE_STOP = 0,
    MB_BEACON_STATE_READY,
    MB_BEACON_STATE_FIXED_START,
    MB_BEACON_STATE_FIXED,
    MB_BEACON_STATE_VOLAT_START,
    MB_BEACON_STATE_VOLAT,
    MB_BEACON_STATE_NEXT_GAME
}
MbBeaconState;


/* ビーコンフォーマット構造体 */
typedef struct MbBeacon
{
    u32     ggid;                      // GGID
    u8      dataAttr:2;                // データ属性（MbBeaconDataAttrで指定）
    u8      fileNo:6;                  // ファイルNo.
    u8      seqNoFixed;                // Fixedデータシーケンス番号（データ内容に更新があった場合に加算される。）
    u8      seqNoVolat;                // Volatデータシーケンス番号（　〃　）
    u8      beaconNo;                  // ビーコンナンバー（ビーコン送信ごとにインクリメント）
    /* 元 MbBeaconData */
    union
    {
        struct
        {                              // MBGameInfoFixed送信時
            u16     sum;               // 16bit checksum
            u8      fragmentNo;        // 断片化データの現在番号
            u8      fragmentMaxNum;    // 断片化データの最大数
            u8      size;              // データサイズ
            u8      rsv;
            u8      data[MB_BEACON_FIXED_DATA_SIZE];    // データ実体
        }
        fixed;
        struct
        {                              // MBGameInfoVolatile送信時
            u16     sum;               // 16bit checksum
            u8      nowPlayerNum;      // 現在のプレイヤー数
            u8      pad[1];
            u16     nowPlayerFlag;     // 現在の全プレイヤーのプレイヤー番号をビットで示す。
            u16     changePlayerFlag;  // 今回の更新で変更されたプレイヤー情報の番号をビットで示す。（シーケンス番号が変わった瞬間にのみ判定に使用）
            MBUserInfo member[MB_SEND_MEMBER_MAX_NUM];  // 各子機のユーザー情報をMB_SEND_MEMBER_MAX_NUMつずつ送信（PlayerNo == 15（親機） なら終端。）
            u8      userVolatData[MB_USER_VOLAT_DATA_SIZE];     // ユーザがセットできるデータ
        }
        volat;
    }
    data;

}
MbBeacon;

/* 親機側ビーコン送信ステータス構造体 */
typedef struct MbBeaconSendStatus
{
    MBGameInfo *gameInfoListTop;       // ゲーム情報リスト先頭へのポインタ（片方向リスト）
    MBGameInfo *nowGameInfop;          // 現在送信中のゲーム情報へのポインタ
    u8     *srcp;                      // 現在の送信中のゲーム情報ソースへのポインタ
    u8      state;                     // ビーコン送信ステート（固定データ部、流動データ部送信。各データを全送信したらステート変更。）
    u8      seqNoFixed;                // 固定領域のシーケンス番号
    u8      seqNoVolat;                // 流動領域のシーケンス番号
    u8      fragmentNo;                // 　　〃　　　　　　のフラグメント番号（固定領域の場合）
    u8      fragmentMaxNum;            // 　　〃　　　　　　のフラグメント数　（〃）
    u8      beaconNo;
    u8      pad[2];
}
MbBeaconSendStatus;



// function's prototype------------------------------------
static BOOL MBi_ReadIconInfo(const char *filePathp, MBIconInfo *iconp, BOOL char_flag);
static void MBi_ClearSendStatus(void);
static BOOL MBi_ReadyBeaconSendStatus(void);
static void MBi_InitSendFixedBeacon(void);
static void MBi_SendFixedBeacon(u32 ggid, u16 tgid, u8 attribute);
static void MBi_InitSendVolatBeacon(void);
static void MBi_SendVolatBeacon(u32 ggid, u16 tgid, u8 attribute);

static void MBi_SetSSIDToBssDesc(WMBssDesc *bssDescp, u32 ggid);
static int MBi_GetStoreElement(WMBssDesc *bssDescp, MBBeaconMsgCallback Callbackp);
static void MBi_CheckCompleteGameInfoFragments(int index, MBBeaconMsgCallback Callbackp);
static void MBi_AnalyzeBeacon(WMBssDesc *bssDescp, int index, u16 linkLevel);
static void MBi_CheckTGID(WMBssDesc *bssDescp, int inex);
static void MBi_CheckSeqNoFixed(int index);
static void MBi_CheckSeqNoVolat(int index);
static void MBi_InvalidateGameInfoBssID(u8 *bssidp);
static void MBi_RecvFixedBeacon(int index);
static void MBi_RecvVolatBeacon(int index);

static void MBi_LockScanTarget(int index);
static void MBi_UnlockScanTarget(void);
static int mystrlen(u16 *str);


// const data----------------------------------------------

// global variables----------------------------------------

static MbBeaconSendStatus mbss;        // ビーコン送信ステータス

static MbBeaconRecvStatus mbrs;        // ビーコン受信ステータス
static MbBeaconRecvStatus *mbrsp = NULL;        // ビーコン受信ステータス

// static variables----------------------------------------
static MbScanLockFunc sLockFunc = NULL; // スキャンロック設定関数ポインタ
static MbScanUnlockFunc sUnlockFunc = NULL;     // スキャンロック解除用関数ポインタ

static MbBeacon bsendBuff ATTRIBUTE_ALIGN(32);  // ビーコン送信バッファ
static MbBeacon *brecvBuffp;           // ビーコン受信バッファ
static WMBssDesc bssDescbuf ATTRIBUTE_ALIGN(32);        // BssDescのテンポラリバッファ

static MBSendVolatCallbackFunc sSendVolatCallback = NULL;       // ユーザデータ送信コールバック
static u32 sSendVolatCallbackTiming;  // 送信コールバック発生タイミング

// function's description-----------------------------------------------


//=========================================================
// ビーコン受信ワークバッファ操作
//=========================================================
// 設定されているワークバッファを取得
const MbBeaconRecvStatus *MB_GetBeaconRecvStatus(void)
{
    return mbrsp;
}

// static変数からワークバッファを設定(旧互換用)
void MBi_SetBeaconRecvStatusBufferDefault(void)
{
    mbrsp = &mbrs;
}

// ビーコン受信ワークバッファの設定
void MBi_SetBeaconRecvStatusBuffer(MbBeaconRecvStatus * buf)
{
    mbrsp = buf;
}


// スキャンロック用関数を設定
void MBi_SetScanLockFunc(MbScanLockFunc lock, MbScanUnlockFunc unlock)
{
    sLockFunc = lock;
    sUnlockFunc = unlock;
}



//=========================================================
// 親機によるゲーム情報のビーコン送信
//=========================================================

// ビーコンで送信する親機ゲーム情報の生成
void MBi_MakeGameInfo(MBGameInfo *gameInfop,
                      const MBGameRegistry *mbGameRegp, const MBUserInfo *parent)
{
    BOOL    icon_disable;

    // とりあえず全クリア
    MI_CpuClear16(gameInfop, sizeof(MBGameInfo));

    // アイコンデータの登録
    gameInfop->dataAttr = MB_BEACON_DATA_ATTR_FIXED_NORMAL;
    icon_disable = !MBi_ReadIconInfo(mbGameRegp->iconCharPathp, &gameInfop->fixed.icon, TRUE);
    icon_disable |= !MBi_ReadIconInfo(mbGameRegp->iconPalettePathp, &gameInfop->fixed.icon, FALSE);

    if (icon_disable)
    {
        gameInfop->dataAttr = MB_BEACON_DATA_ATTR_FIXED_NO_ICON;
        MI_CpuClearFast(&gameInfop->fixed.icon, sizeof(MBIconInfo));
    }

    // GGID
    gameInfop->ggid = mbGameRegp->ggid;

    // 親のユーザー情報登録
    if (parent != NULL)
    {
        MI_CpuCopy16(parent, &gameInfop->fixed.parent, sizeof(MBUserInfo));
    }

    // 最大プレイ人数登録
    gameInfop->fixed.maxPlayerNum = mbGameRegp->maxPlayerNum;

    // ゲーム名＆ゲーム内容説明登録
    {
        int     len;

#define COPY_GAME_INFO_STRING   MI_CpuCopy16

        len = mystrlen(mbGameRegp->gameNamep) << 1;
        COPY_GAME_INFO_STRING((u8 *)mbGameRegp->gameNamep, gameInfop->fixed.gameName, (u16)len);
        // ゲーム内容説明の後にデータを埋め込めるように、NUL文字以降のデータも送信する。
        len = MB_GAME_INTRO_LENGTH * 2;
        COPY_GAME_INFO_STRING((u8 *)mbGameRegp->gameIntroductionp,
                              gameInfop->fixed.gameIntroduction, (u16)len);
    }

    // 親機分のプレイヤー情報を登録
    gameInfop->volat.nowPlayerNum = 1;
    gameInfop->volat.nowPlayerFlag = 0x0001;    // 親機のプレイヤー番号は0
    gameInfop->broadcastedPlayerFlag = 0x0001;

    // ※マルチブート開始時にはプレイメンバーはいないので、登録はなし。
}


// アイコンデータのリード
static BOOL MBi_ReadIconInfo(const char *filePathp, MBIconInfo *iconp, BOOL char_flag)
{
    FSFile  file;
    s32     size = char_flag ? MB_ICON_DATA_SIZE : MB_ICON_PALETTE_SIZE;
    u16    *dstp = char_flag ? iconp->data : iconp->palette;

    if (filePathp == NULL)
    {                                  // ファイル指定がなければFALSEリターン
        return FALSE;
    }

    FS_InitFile(&file);

    if (FS_OpenFileEx(&file, filePathp, FS_FILEMODE_R) == FALSE)
    {                                  // ファイルが開けなかったらFALSEリターン
        MB_DEBUG_OUTPUT("\t%s : file open error.\n", filePathp);
        return FALSE;
    }
    else if ((u32)size != FS_GetFileLength(&file))
    {
        MB_DEBUG_OUTPUT("\t%s : different file size.\n", filePathp);
        (void)FS_CloseFile(&file);
        return FALSE;
    }

    (void)FS_ReadFile(&file, dstp, size);
    (void)FS_CloseFile(&file);
    return TRUE;
}


// ゲーム情報の流動部分を更新する。
void MB_UpdateGameInfoMember(MBGameInfo *gameInfop,
                             const MBUserInfo *member, u16 nowPlayerFlag, u16 changePlayerFlag)
{
    int     i;
    u8      playerNum = 1;

    MI_CpuCopy16(member, &gameInfop->volat.member[0], sizeof(MBUserInfo) * MB_MEMBER_MAX_NUM);
    /* 子機数をカウント */
    for (i = 0; i < MB_MEMBER_MAX_NUM; i++)
    {
        if (nowPlayerFlag & (0x0002 << i))
        {
            playerNum++;
        }
    }
    gameInfop->volat.nowPlayerNum = playerNum;
    gameInfop->volat.nowPlayerFlag = (u16)(nowPlayerFlag | 0x0001);
    gameInfop->volat.changePlayerFlag = changePlayerFlag;
    gameInfop->seqNoVolat++;
}


// 文字列長の算出
static int mystrlen(u16 *str)
{
    int     len = 0;
    while (*str++)
        len++;
    return len;
}


// ゲーム情報を送信リストに追加
void MB_AddGameInfo(MBGameInfo *newGameInfop)
{
    MBGameInfo *gInfop = mbss.gameInfoListTop;

    if (!gInfop)
    {                                  // mbssの先頭がNULLなら先頭に登録。
        mbss.gameInfoListTop = newGameInfop;
    }
    else
    {                                  // さもなくば、リストを辿って行き、最後に追加。
        while (gInfop->nextp != NULL)
        {
            gInfop = gInfop->nextp;
        }
        gInfop->nextp = newGameInfop;
    }
    newGameInfop->nextp = NULL;        // 追加したゲーム情報のnextを終端とする。
}


// ゲーム情報を送信リストから削除
BOOL MB_DeleteGameInfo(MBGameInfo *gameInfop)
{
    MBGameInfo *gInfop = mbss.gameInfoListTop;
    MBGameInfo *before;

    while (gInfop != NULL)
    {
        if (gInfop != gameInfop)
        {
            before = gInfop;
            gInfop = gInfop->nextp;
            continue;
        }

        // 一致したのでリストから削除
        if ((u32)gInfop == (u32)mbss.gameInfoListTop)
        {                              // リスト先頭を削除する場合
            mbss.gameInfoListTop = mbss.gameInfoListTop->nextp;
        }
        else
        {
            before->nextp = gInfop->nextp;
        }

        if ((u32)gameInfop == (u32)mbss.nowGameInfop)   // 削除するゲーム情報が現在送信中の場合、現在送信中の
        {                              // ゲーム情報を再構築されたリストから再設定する。
            mbss.nowGameInfop = NULL;
            if (!MBi_ReadyBeaconSendStatus())
            {
                mbss.state = MB_BEACON_STATE_READY;     // ゲーム情報が全て削除されていたならば、ステータスをREADYに。
            }
        }
        return TRUE;
    }

    // ゲーム情報がない場合
    return FALSE;
}


// ゲーム情報送信設定の初期化
void MB_InitSendGameInfoStatus(void)
{
    mbss.gameInfoListTop = NULL;       // ゲーム情報リストを全削除。
    mbss.nowGameInfop = NULL;          // 現送信ゲームも削除。
    mbss.state = MB_BEACON_STATE_READY;
    sSendVolatCallback = NULL;
    MBi_ClearSendStatus();             // 送信ステータスもクリア。
}


// 送信ステータスのクリア
static void MBi_ClearSendStatus(void)
{
    mbss.seqNoFixed = 0;
    mbss.seqNoVolat = 0;
    mbss.fragmentNo = 0;
    mbss.fragmentMaxNum = 0;
    mbss.beaconNo = 0;
}


// ビーコン送信
void MB_SendGameInfoBeacon(u32 ggid, u16 tgid, u8 attribute)
{
    while (1)
    {
        switch (mbss.state)
        {
        case MB_BEACON_STATE_STOP:
        case MB_BEACON_STATE_READY:
            if (!MBi_ReadyBeaconSendStatus())
            {                          // ゲーム情報の送信準備
                return;
            }
            break;
        case MB_BEACON_STATE_FIXED_START:
            MBi_InitSendFixedBeacon();
            break;
        case MB_BEACON_STATE_FIXED:
            MBi_SendFixedBeacon(ggid, tgid, attribute);
            return;
        case MB_BEACON_STATE_VOLAT_START:
            MBi_InitSendVolatBeacon();
            break;
        case MB_BEACON_STATE_VOLAT:
            MBi_SendVolatBeacon(ggid, tgid, attribute);
            return;
        case MB_BEACON_STATE_NEXT_GAME:
            break;
        }
    }
}


// ビーコン送信ステータスを送信可能に準備する。
static BOOL MBi_ReadyBeaconSendStatus(void)
{
    MBGameInfo *nextGameInfop;

    // ゲーム情報が登録されていないなら、エラーリターン。
    if (!mbss.gameInfoListTop)
    {
        /* MB フラグと ENTRY フラグはここで落とす */
        (void)WM_SetGameInfo(NULL, (u16 *)&bsendBuff, WM_SIZE_USER_GAMEINFO, MBi_GetGgid(),
                             MBi_GetTgid(), WM_ATTR_FLAG_CS);
        return FALSE;
    }

    // 次に送信するGameInfoの選択
    if (!mbss.nowGameInfop || !mbss.nowGameInfop->nextp)
    {
        // まだ送信中のゲーム情報がないか、リストの最後の場合は、リストの先頭を送信するよう準備。
        nextGameInfop = mbss.gameInfoListTop;
    }
    else
    {
        // さもなくば、リストの次のゲームを送信するよう準備。
        nextGameInfop = mbss.nowGameInfop->nextp;
    }

    mbss.nowGameInfop = nextGameInfop;

    MBi_ClearSendStatus();
    mbss.seqNoVolat = mbss.nowGameInfop->seqNoVolat;

    mbss.state = MB_BEACON_STATE_FIXED_START;

    return TRUE;
}


// ゲーム情報の固定データ部の送信初期化
static void MBi_InitSendFixedBeacon(void)
{
    if (mbss.state != MB_BEACON_STATE_FIXED_START)
    {
        return;
    }

    if (mbss.nowGameInfop->dataAttr == MB_BEACON_DATA_ATTR_FIXED_NORMAL)
    {
        mbss.fragmentMaxNum = FIXED_FRAGMENT_MAX(FIXED_NORMAL_SIZE);
        mbss.srcp = (u8 *)&mbss.nowGameInfop->fixed;
    }
    else
    {
        mbss.fragmentMaxNum = FIXED_FRAGMENT_MAX(FIXED_NO_ICON_SIZE);
        mbss.srcp = (u8 *)&mbss.nowGameInfop->fixed.parent;
    }
    mbss.state = MB_BEACON_STATE_FIXED;
}


// ゲーム情報の固定データ部を分割してビーコン送信
static void MBi_SendFixedBeacon(u32 ggid, u16 tgid, u8 attribute)
{
    u32     lastAddr = (u32)mbss.nowGameInfop + sizeof(MBGameInfoFixed);

    if ((u32)mbss.srcp + MB_BEACON_FIXED_DATA_SIZE <= lastAddr)
    {
        bsendBuff.data.fixed.size = MB_BEACON_FIXED_DATA_SIZE;
    }
    else
    {
        bsendBuff.data.fixed.size = (u8)(lastAddr - (u32)mbss.srcp);
        MI_CpuClear16((void *)((u8 *)bsendBuff.data.fixed.data + bsendBuff.data.fixed.size),
                      (u32)(MB_BEACON_FIXED_DATA_SIZE - bsendBuff.data.fixed.size));
    }

    // ビーコン送信バッファにセット
    MB_DEBUG_OUTPUT("send fragment= %2d  adr = 0x%x\n", mbss.fragmentNo, mbss.srcp);
    MI_CpuCopy16(mbss.srcp, bsendBuff.data.fixed.data, bsendBuff.data.fixed.size);
    bsendBuff.data.fixed.fragmentNo = mbss.fragmentNo;
    bsendBuff.data.fixed.fragmentMaxNum = mbss.fragmentMaxNum;
    bsendBuff.dataAttr = mbss.nowGameInfop->dataAttr;
    bsendBuff.seqNoFixed = mbss.nowGameInfop->seqNoFixed;
    bsendBuff.seqNoVolat = mbss.seqNoVolat;
    bsendBuff.ggid = mbss.nowGameInfop->ggid;
    bsendBuff.fileNo = mbss.nowGameInfop->fileNo;
    bsendBuff.beaconNo = mbss.beaconNo++;
    bsendBuff.data.fixed.sum = 0;
    bsendBuff.data.fixed.sum = MBi_calc_cksum((u16 *)&bsendBuff.data, MB_BEACON_DATA_SIZE);

    // 送信ステータスの更新
    mbss.fragmentNo++;
    if (mbss.fragmentNo < mbss.fragmentMaxNum)
    {
        mbss.srcp += MB_BEACON_FIXED_DATA_SIZE;
    }
    else
    {
        mbss.state = MB_BEACON_STATE_VOLAT_START;
    }

    /* 親機プログラム自身ののGGIDを登録する */
    (void)WM_SetGameInfo(NULL, (u16 *)&bsendBuff, WM_SIZE_USER_GAMEINFO, ggid, tgid,
                         (u8)(attribute | WM_ATTR_FLAG_MB | WM_ATTR_FLAG_ENTRY));
    // マルチブートフラグはここで立てる。
}


// ゲーム情報の流動データ部の送信初期化
static void MBi_InitSendVolatBeacon(void)
{
    mbss.nowGameInfop->broadcastedPlayerFlag = 0x0001;
    mbss.seqNoVolat = mbss.nowGameInfop->seqNoVolat;
    mbss.state = MB_BEACON_STATE_VOLAT;
}


// ゲーム情報の流動データ部を分割してビーコン送信
static void MBi_SendVolatBeacon(u32 ggid, u16 tgid, u8 attribute)
{
    int     i;
    int     setPlayerNum;
    u16     remainPlayerFlag;

    if (mbss.seqNoVolat != mbss.nowGameInfop->seqNoVolat)
    {                                  // 送信中にデータ更新があったら送り直し。
        MBi_InitSendVolatBeacon();
    }

    // ビーコン情報をセット
    bsendBuff.dataAttr = MB_BEACON_DATA_ATTR_VOLAT;
    bsendBuff.seqNoFixed = mbss.nowGameInfop->seqNoFixed;
    bsendBuff.seqNoVolat = mbss.seqNoVolat;
    bsendBuff.ggid = mbss.nowGameInfop->ggid;
    bsendBuff.fileNo = mbss.nowGameInfop->fileNo;
    bsendBuff.beaconNo = mbss.beaconNo++;

    // 現在のプレイヤー情報の状態をセット
    bsendBuff.data.volat.nowPlayerNum = mbss.nowGameInfop->volat.nowPlayerNum;
    bsendBuff.data.volat.nowPlayerFlag = mbss.nowGameInfop->volat.nowPlayerFlag;
    bsendBuff.data.volat.changePlayerFlag = mbss.nowGameInfop->volat.changePlayerFlag;

    // アプリ設定データをセット
    if (sSendVolatCallbackTiming == MB_SEND_VOLAT_CALLBACK_TIMING_BEFORE
        && sSendVolatCallback != NULL)
    {
        sSendVolatCallback(mbss.nowGameInfop->ggid);
    }

    for (i = 0; i < MB_USER_VOLAT_DATA_SIZE; i++)
    {
        bsendBuff.data.volat.userVolatData[i] = mbss.nowGameInfop->volat.userVolatData[i];
    }

    MB_DEBUG_OUTPUT("send PlayerFlag = %x\n", mbss.nowGameInfop->volat.nowPlayerFlag);

    // 今回送信するプレイヤー情報のセット
    MI_CpuClear16(&bsendBuff.data.volat.member[0], sizeof(MBUserInfo) * MB_SEND_MEMBER_MAX_NUM);
    setPlayerNum = 0;
    remainPlayerFlag =
        (u16)(mbss.nowGameInfop->broadcastedPlayerFlag ^ mbss.nowGameInfop->volat.nowPlayerFlag);
    for (i = 0; i < MB_MEMBER_MAX_NUM; i++)
    {
        if ((remainPlayerFlag & (0x0002 << i)) == 0)
        {
            continue;
        }

        MB_DEBUG_OUTPUT("  member %d set.\n", i);

        MI_CpuCopy16(&mbss.nowGameInfop->volat.member[i],
                     &bsendBuff.data.volat.member[setPlayerNum], sizeof(MBUserInfo));
        mbss.nowGameInfop->broadcastedPlayerFlag |= 0x0002 << i;
        if (++setPlayerNum == MB_SEND_MEMBER_MAX_NUM)
        {
            break;
        }
    }
    if (setPlayerNum < MB_SEND_MEMBER_MAX_NUM)
    {                                  // 最大送信数に達しなかった時には終端する。
        bsendBuff.data.volat.member[setPlayerNum].playerNo = 0;
    }

    // チェックサムのセット
    bsendBuff.data.volat.sum = 0;
    bsendBuff.data.volat.sum = MBi_calc_cksum((u16 *)&bsendBuff.data, MB_BEACON_DATA_SIZE);


    // 送信終了チェック
    if (mbss.nowGameInfop->broadcastedPlayerFlag == mbss.nowGameInfop->volat.nowPlayerFlag)
    {
        mbss.state = MB_BEACON_STATE_READY;     // 全情報を送信し終わったら、次のゲーム情報を送信するための準備ステートへ。
    }

    /* 親機プログラム自身ののGGIDを登録する */

    (void)WM_SetGameInfo(NULL, (u16 *)&bsendBuff, WM_SIZE_USER_GAMEINFO, ggid, tgid,
                         (u8)(attribute | WM_ATTR_FLAG_MB | WM_ATTR_FLAG_ENTRY));
    // マルチブートフラグはここで立てる。

    if (sSendVolatCallbackTiming == MB_SEND_VOLAT_CALLBACK_TIMING_AFTER
        && sSendVolatCallback != NULL)
    {
        sSendVolatCallback(mbss.nowGameInfop->ggid);
    }

}


//=========================================================
// 子機によるゲーム情報のビーコン受信
//=========================================================

// ゲーム情報受信ステータスの初期化
void MB_InitRecvGameInfoStatus(void)
{
    MI_CpuClearFast(mbrsp, sizeof(MbBeaconRecvStatus));

    mbrsp->scanCountUnit = MB_SCAN_COUNT_UNIT_NORMAL;
}


// ビーコン受信
BOOL MB_RecvGameInfoBeacon(MBBeaconMsgCallback Callbackp, u16 linkLevel, WMBssDesc *bssDescp)
{
    int     index;

    // 今回取得したビーコンが、マルチブート親機かどうかを判定。
    if (!MBi_CheckMBParent(bssDescp))
    {
        return FALSE;
    }

    /* 取得したbssDescをテンポラリバッファへコピー */
    MI_CpuCopy16(bssDescp, &bssDescbuf, sizeof(WMBssDesc));

    /* bssDescpをローカルバッファに切り替える。 */
    bssDescp = &bssDescbuf;

    brecvBuffp = (MbBeacon *) bssDescp->gameInfo.userGameInfo;

    // チェックサム確認。
    if (MBi_calc_cksum((u16 *)&brecvBuffp->data, MB_BEACON_DATA_SIZE))
    {
        MB_DEBUG_OUTPUT("Beacon checksum error!\n");
        return FALSE;
    }

    // 親機ゲーム情報の格納場所を判定（既に同一親機のデータを受信しているなら、そこに続けて格納する）
    index = MBi_GetStoreElement(bssDescp, Callbackp);
    if (index < 0)
    {
        return FALSE;                  // 格納場所がないのでエラーリターン
    }
    MB_DEBUG_OUTPUT("GameInfo Index:%6d\n", index);

    // ビーコン解析
    MBi_AnalyzeBeacon(bssDescp, index, linkLevel);

    // 親機ゲーム情報の全ての断片が揃ったか判定し、コールバックで通知
    MBi_CheckCompleteGameInfoFragments(index, Callbackp);

    return TRUE;
}


// 今回取得したビーコンがマルチブート親機かどうかを判定する。
BOOL MBi_CheckMBParent(WMBssDesc *bssDescp)
{
    // マルチブート親機かどうかを判定する。
    if ((bssDescp->gameInfo.magicNumber != WM_GAMEINFO_MAGIC_NUMBER)
        || !(bssDescp->gameInfo.attribute & WM_ATTR_FLAG_MB))
    {
        MB_DEBUG_OUTPUT("not MB parent : %x%x\n",
                        *(u16 *)(&bssDescp->bssid[4]), *(u32 *)bssDescp->bssid);
        return FALSE;
    }
    else
    {

        MB_DEBUG_OUTPUT("MB parent     : %x%x",
                        *(u16 *)(&bssDescp->bssid[4]), *(u32 *)bssDescp->bssid);
        return TRUE;
    }
}


// bssDescにSSIDをセットする。
static void MBi_SetSSIDToBssDesc(WMBssDesc *bssDescp, u32 ggid)
{
    /* 
       SSIDの設定 

       ダウンロードアプリ固有のGGIDと、マルチブート親機のTGIDから
       SSIDを生成する。
       子機は、このSSIDを用いて親機アプリケーションと再接続を行う。
     */
    bssDescp->ssidLength = 32;
    ((u16 *)bssDescp->ssid)[0] = (u16)(ggid & 0x0000ffff);
    ((u16 *)bssDescp->ssid)[1] = (u16)((ggid & 0xffff0000) >> 16);
    ((u16 *)bssDescp->ssid)[2] = bssDescp->gameInfo.tgid;
}


// bssDescをもとに受信リスト要素のどこに格納するかを取得する。
static int MBi_GetStoreElement(WMBssDesc *bssDescp, MBBeaconMsgCallback Callbackp)
{
    int     i;

    // 既にこの親機の同一ゲーム情報を受信しているかどうかを判定（ "GGID", "BSSID", "fileNo"の３つが一致するなら、同一ゲーム情報と判断する）
    for (i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++)
    {
        MBGameInfoRecvList *info = &mbrsp->list[i];

        if ((mbrsp->usingGameInfoFlag & (0x01 << i)) == 0)
        {
            continue;
        }
        // GGIDが一致するか？
        if (info->gameInfo.ggid != brecvBuffp->ggid)
        {
            continue;
        }
        // MACアドレスが一致するか？
        if (!WM_IsBssidEqual(info->bssDesc.bssid, bssDescp->bssid))
        {
            continue;
        }
        // ファイルNo.が一致するか？
        if (mbrsp->list[i].gameInfo.fileNo != brecvBuffp->fileNo)
        {
            continue;
        }

        // =========================================
        // この親機情報に対して、既に受信場所が確保されていると判断。
        // =========================================
        if (!(mbrsp->validGameInfoFlag & (0x01 << i)))
        {
            MBi_LockScanTarget(i);     // まだ該当親機の情報が揃っていなければ、スキャンロックする。
        }
        return i;
    }

    // まだ受信していないので、リストのNULL位置を探して、そこを格納場所にする。
    for (i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++)
    {
        if (mbrsp->usingGameInfoFlag & (0x01 << i))
        {
            continue;
        }

        MI_CpuCopy16(bssDescp, &mbrsp->list[i].bssDesc, sizeof(WMBssDesc));
        // BssDescをコピー
        mbrsp->list[i].gameInfo.seqNoFixed = brecvBuffp->seqNoFixed;
        mbrsp->usingGameInfoFlag |= (u16)(0x01 << i);

        MB_DEBUG_OUTPUT("\n");
        // この親機のみをScan対象にロックする。
        MBi_LockScanTarget(i);
        return i;
    }

    // 格納場所が全て埋まっていた場合はコールバックで通知してエラーリターン
    if (Callbackp != NULL)
    {
        Callbackp(MB_BC_MSG_GINFO_LIST_FULL, NULL, 0);
    }
    return -1;
}


// 親機ゲーム情報の全ての断片が揃ったか判定し、コールバックで通知
static void MBi_CheckCompleteGameInfoFragments(int index, MBBeaconMsgCallback Callbackp)
{
    MBGameInfoRecvList *info = &mbrsp->list[index];

    /* いずれにせよアプリケーションへ親機発見を通知する */
    if (Callbackp != NULL)
    {
        Callbackp(MB_BC_MSG_GINFO_BEACON, info, index);
    }
    if ((info->getFragmentFlag == info->allFragmentFlag) && (info->getFragmentFlag)     // 新しく親機ゲーム情報が揃った場合
        && (info->gameInfo.volat.nowPlayerFlag)
        && (info->getPlayerFlag == info->gameInfo.volat.nowPlayerFlag))
    {
        if (mbrsp->validGameInfoFlag & (0x01 << index))
        {
            return;
        }
        mbrsp->validGameInfoFlag |= 0x01 << index;
        mbrsp->usefulGameInfoFlag |= 0x01 << index;
        MBi_UnlockScanTarget();        // Scan対象のロックを解除する。
        MB_DEBUG_OUTPUT("validated ParentInfo = %d\n", index);
        if (Callbackp != NULL)
        {
            Callbackp(MB_BC_MSG_GINFO_VALIDATED, info, index);
        }
    }
    else
    {                                  // 既に取得していた親機ゲーム情報が更新されて、不完全な状態になった場合
        if ((mbrsp->validGameInfoFlag & (0x01 << index)) == 0)
        {
            return;
        }
        mbrsp->validGameInfoFlag ^= (0x01 << index);
        MB_DEBUG_OUTPUT("Invaldated ParentInfo = %d\n", index);
        if (Callbackp != NULL)
        {
            Callbackp(MB_BC_MSG_GINFO_INVALIDATED, info, index);
        }
    }
}


// 親機のゲーム情報の寿命カウント（ついでにスキャンロック時間のカウントも行う）
void MB_CountGameInfoLifetime(MBBeaconMsgCallback Callbackp, BOOL found_parent)
{
    int     i;
    BOOL    unlock = FALSE;

    // ゲーム情報受信リストの寿命判定　＆　新規ロック対象ターゲットの有無を確認
    for (i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++)
    {
        MBGameInfoRecvList *info = &mbrsp->list[i];
        u16     mask = (u16)(0x0001 << i);
        if ((mbrsp->usingGameInfoFlag & mask) == 0)
        {
            continue;
        }
        // 寿命判定
        info->lifetimeCount -= mbrsp->scanCountUnit;
        if (info->lifetimeCount >= 0)
        {
            continue;
        }
        info->lifetimeCount = 0;
        if (mbrsp->validGameInfoFlag & mask)
        {
            if (Callbackp != NULL)
            {
                Callbackp(MB_BC_MSG_GINFO_LOST, info, i);
            }
        }
        if (mbrsp->nowScanTargetFlag & mask)
        {
            unlock = TRUE;
        }
        mbrsp->usingGameInfoFlag &= ~mask;
        MB_DeleteRecvGameInfo(i);      // コールバック通知後にゲーム情報を削除する。
        MB_DEBUG_OUTPUT("gameInfo %2d : lifetime end.\n", i);
    }

    // スキャンロック時間のカウント
    if (mbrsp->nowScanTargetFlag && mbrsp->nowLockTimeCount > 0)
    {
        mbrsp->nowLockTimeCount -= mbrsp->scanCountUnit;        // スキャンロック時間がタイムアウトしたら、ロックを解除して次のロック対象を探す。
        if (mbrsp->nowLockTimeCount < 0)
        {
            MB_DEBUG_OUTPUT("scan lock time up!\n");
            unlock = TRUE;
        }
        else if (!found_parent)
        {
            if (++mbrsp->notFoundLockTargetCount > 4)
            {
                MB_DEBUG_OUTPUT("scan lock target not found!\n");
                unlock = TRUE;
            }
        }
        else
        {
            mbrsp->notFoundLockTargetCount = 0;
        }
    }

    // スキャンアンロック処理。
    if (unlock)
    {
        mbrsp->nowLockTimeCount = 0;
        MBi_UnlockScanTarget();
    }
}


// ビーコン解析
static void MBi_AnalyzeBeacon(WMBssDesc *bssDescp, int index, u16 linkLevel)
{
    MBi_CheckTGID(bssDescp, index);    // TGIDのチェック
    MBi_CheckSeqNoFixed(index);        // seqNoFixedのチェック
    MBi_CheckSeqNoVolat(index);        // seqNoVolatのチェック

    // 受信ビーコンの共通部分のデータ取得
    {
        MBGameInfoRecvList *info = &mbrsp->list[index];

        // 同じビーコンを出し続ける異常な状態の親機を除外する。
        if (info->beaconNo == brecvBuffp->beaconNo)
        {
            if (++info->sameBeaconRecvCount > MB_SAME_BEACON_RECV_MAX_COUNT)
            {
                info->lifetimeCount = 0;        // 親機情報の寿命をゼロにして、削除させる。
                MB_OUTPUT("The parent broadcast same beacon.: %d\n", index);
                MBi_InvalidateGameInfoBssID(&info->bssDesc.bssid[0]);
                return;                // この親機が他にもゲーム情報を配信していたら、それも削除させる。
            }
        }
        else
        {
            info->sameBeaconRecvCount = 0;
        }
        // 正常な状態の親機のデータ受信
        info->beaconNo = brecvBuffp->beaconNo;
        info->lifetimeCount = MB_LIFETIME_MAX_COUNT;    // 親機情報の延命。
        info->gameInfo.ggid = brecvBuffp->ggid; // ggid    の取得。
        info->gameInfo.fileNo = brecvBuffp->fileNo;     // fileNo  の取得。
        info->linkLevel = linkLevel;   // 電波強度の取得。
        // bssDescにSSIDをセットする。
        MBi_SetSSIDToBssDesc(&info->bssDesc, info->gameInfo.ggid);
    }

    // 受信ビーコンのデータ種類毎のデータ取得
    if (brecvBuffp->dataAttr == MB_BEACON_DATA_ATTR_VOLAT)
    {
        MBi_RecvVolatBeacon(index);
    }
    else
    {
        MBi_RecvFixedBeacon(index);
    }
}


// tgidのチェック
static void MBi_CheckTGID(WMBssDesc *bssDescp, int index)
{
    if (mbrsp->list[index].bssDesc.gameInfo.tgid == bssDescp->gameInfo.tgid)
    {
        return;
    }

    // tgidがアップしていたら、その親機は立ち上げ直されているとみなして、全データをクリアして取り直す。
    MB_DEBUG_OUTPUT("\ntgid updated! : %x%x", *(u16 *)(&bssDescp->bssid[4]),
                    *(u32 *)bssDescp->bssid);
    MB_DeleteRecvGameInfoWithoutBssdesc(index);
    MI_CpuCopy16(bssDescp, &mbrsp->list[index].bssDesc, sizeof(WMBssDesc));
    // tgid更新の場合は、新しいbssDescをコピーする。
    MBi_LockScanTarget(index);         // 該当親機をスキャンロックする。
}


// FixedデータのSeqNoチェック
static void MBi_CheckSeqNoFixed(int index)
{
    // シーケンス番号が更新されていた場合の対応
    if (mbrsp->list[index].gameInfo.seqNoFixed == brecvBuffp->seqNoFixed)
    {
        return;
    }
    // シーケンス番号が更新されていたら、これまでに受信したデータをクリアする。
    MB_DEBUG_OUTPUT("\n seqNoFixed updated!");
    MB_DeleteRecvGameInfoWithoutBssdesc(index);
    MBi_LockScanTarget(index);         // 該当親機をスキャンロックする。
    mbrsp->list[index].gameInfo.seqNoFixed = brecvBuffp->seqNoFixed;
}


// VolatデータのSeqNoチェック
static void MBi_CheckSeqNoVolat(int index)
{
    MBGameInfoRecvList *grecvp = &mbrsp->list[index];

    // シーケンス番号が更新されていた場合の対応
    if (mbrsp->list[index].gameInfo.seqNoVolat != brecvBuffp->seqNoVolat)
    {
        MB_DEBUG_OUTPUT("\n seqNoVolat updated!");
        MBi_LockScanTarget(index);     // スキャンロック可能か調べてロックする。
    }
}


// 対象BSSIDの親機ゲーム情報を全て無効にする。
static void MBi_InvalidateGameInfoBssID(u8 *bssidp)
{
    int     i;
    for (i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++)
    {
        if ((mbrsp->usingGameInfoFlag & (0x01 << i)) == 0)
        {
            continue;
        }

        if (!WM_IsBssidEqual(bssidp, mbrsp->list[i].bssDesc.bssid))
        {
            continue;
        }

        // 削除対象のデータと判定。
        mbrsp->list[i].lifetimeCount = 0;       // 親機情報の寿命をゼロにして、削除させる。
        MB_OUTPUT("The parent broadcast same beacon.: %d\n", i);
    }
}


// ゲーム情報の固定データ部を分割してビーコン受信
static void MBi_RecvFixedBeacon(int index)
{
    MBGameInfoRecvList *grecvp = &mbrsp->list[index];
    u32     lastAddr = (u32)&grecvp->gameInfo + sizeof(MBGameInfoFixed);
    u8     *dstp;

    // シーケンス番号が更新されておらず、かつ既に取得済みのビーコンなら受信しない。
    if (grecvp->gameInfo.seqNoFixed == brecvBuffp->seqNoFixed)
    {
        if (grecvp->getFragmentFlag & (0x01 << brecvBuffp->data.fixed.fragmentNo))
        {
            return;
        }
    }

    // 受信ビーコンが受信バッファをオーバーしないかチェック。
    if (brecvBuffp->dataAttr == MB_BEACON_DATA_ATTR_FIXED_NORMAL)
    {
        dstp = (u8 *)&grecvp->gameInfo.fixed;
    }
    else
    {
        dstp = (u8 *)&grecvp->gameInfo.fixed.parent;
    }
    dstp += MB_BEACON_FIXED_DATA_SIZE * brecvBuffp->data.fixed.fragmentNo;
    // ゲーム情報バッファの受信アドレスを算出。

    if ((u32)dstp + brecvBuffp->data.fixed.size > lastAddr)
    {
        MB_DEBUG_OUTPUT("recv beacon gInfoFixed Buffer over!\n");
        // バッファをオーバーするビーコンデータは無視する。
        return;
    }

    // 受信ビーコンを対象ゲーム情報バッファにセット
    MB_DEBUG_OUTPUT("recv fragment= %2d  adr = 0x%x", brecvBuffp->data.fixed.fragmentNo, dstp);
    MI_CpuCopy16(brecvBuffp->data.fixed.data, dstp, brecvBuffp->data.fixed.size);
    grecvp->gameInfo.dataAttr = brecvBuffp->dataAttr;
    grecvp->getFragmentFlag |= 0x01 << brecvBuffp->data.fixed.fragmentNo;
    grecvp->allFragmentFlag = (u32)((0x01 << brecvBuffp->data.fixed.fragmentMaxNum) - 1);
    MB_DEBUG_OUTPUT("\t now fragment = 0x%x \t all fragment = 0x%x\n",
                    grecvp->getFragmentFlag, grecvp->allFragmentFlag);
}


// ゲーム情報の流動データ部を分割してビーコン受信
static void MBi_RecvVolatBeacon(int index)
{
    int     i;
    MBGameInfoRecvList *grecvp = &mbrsp->list[index];

    /* ユーザ定義データは常に受信する */
    for (i = 0; i < MB_USER_VOLAT_DATA_SIZE; i++)
    {
        grecvp->gameInfo.volat.userVolatData[i] = brecvBuffp->data.volat.userVolatData[i];
    }
    MI_CpuCopy16(brecvBuffp, &grecvp->bssDesc.gameInfo.userGameInfo, WM_SIZE_USER_GAMEINFO);

    // メンバー情報更新を検出した際の処理
    if (grecvp->gameInfo.seqNoVolat != brecvBuffp->seqNoVolat)
    {
        if ((u8)(grecvp->gameInfo.seqNoVolat + 1) == brecvBuffp->seqNoVolat)
        {                              // seqNoVolatが１つずれなら、変更されていないメンバー情報の引継ぎ
            for (i = 0; i < MB_MEMBER_MAX_NUM; i++)
            {
                if (brecvBuffp->data.volat.changePlayerFlag & (0x02 << i))
                {
                    MI_CpuClear16(&grecvp->gameInfo.volat.member[i], sizeof(MBUserInfo));
                }
            }
            grecvp->getPlayerFlag &= ~brecvBuffp->data.volat.changePlayerFlag;
            mbrsp->validGameInfoFlag &= ~(0x0001 << index);
        }
        else
        {                              // seqNoVolatがもっとずれていたら、これまで受信した全メンバー情報をクリア
            MI_CpuClear16(&grecvp->gameInfo.volat.member[0],
                          sizeof(MBUserInfo) * MB_MEMBER_MAX_NUM);
            grecvp->getPlayerFlag = 0;
            mbrsp->validGameInfoFlag &= ~(0x0001 << index);
        }
        grecvp->gameInfo.seqNoVolat = brecvBuffp->seqNoVolat;
    }
    else if (grecvp->getPlayerFlag == brecvBuffp->data.volat.nowPlayerFlag)
    {
        return;                        // シーケンス番号が更新されておらず、かつ既に取得済みのビーコンなら受信しない。
    }

    // プレイヤー情報の読み出し
    grecvp->gameInfo.volat.nowPlayerNum = brecvBuffp->data.volat.nowPlayerNum;
    grecvp->gameInfo.volat.nowPlayerFlag = brecvBuffp->data.volat.nowPlayerFlag;
    grecvp->gameInfo.volat.changePlayerFlag = brecvBuffp->data.volat.changePlayerFlag;
    grecvp->getPlayerFlag |= 0x0001;

    // 各メンバーのユーザー情報の読み出し
    for (i = 0; i < MB_SEND_MEMBER_MAX_NUM; i++)
    {
        int     playerNo = (int)brecvBuffp->data.volat.member[i].playerNo;
        if (playerNo == 0)
        {
            continue;
        }
        MB_DEBUG_OUTPUT("member %d recv.\n", playerNo);
        MI_CpuCopy16(&brecvBuffp->data.volat.member[i],
                     &grecvp->gameInfo.volat.member[playerNo - 1], sizeof(MBUserInfo));
        grecvp->getPlayerFlag |= 0x01 << playerNo;
    }
}


// スキャン対象を単一親機にロックする
static void MBi_LockScanTarget(int index)
{
    /* 既にロック中のターゲットがいるなら無視 */
    if (mbrsp->nowScanTargetFlag)
    {
        return;
    }

    if (sLockFunc != NULL)
    {
        sLockFunc(mbrsp->list[index].bssDesc.bssid);
    }
    mbrsp->scanCountUnit = MB_SCAN_COUNT_UNIT_LOCKING;

    mbrsp->nowScanTargetFlag = (u16)(0x01 << index);    // 新しいスキャンロック対象をビットで示す。
    mbrsp->nowLockTimeCount = MB_LOCKTIME_MAX_COUNT;    // スキャンロック時間を初期化
    MB_DEBUG_OUTPUT("scan target locked. : %x %x %x %x %x %x\n",
                    mbrsp->list[index].bssDesc.bssid[0],
                    mbrsp->list[index].bssDesc.bssid[1],
                    mbrsp->list[index].bssDesc.bssid[2],
                    mbrsp->list[index].bssDesc.bssid[3],
                    mbrsp->list[index].bssDesc.bssid[4], mbrsp->list[index].bssDesc.bssid[5]);
}


// スキャン対象のロックを解除する。
static void MBi_UnlockScanTarget(void)
{
    if (mbrsp->nowScanTargetFlag == 0)
    {
        return;
    }

    if (sUnlockFunc != NULL)
    {
        sUnlockFunc();
    }
    mbrsp->scanCountUnit = MB_SCAN_COUNT_UNIT_NORMAL;
    mbrsp->nowScanTargetFlag = 0;
    mbrsp->notFoundLockTargetCount = 0;

    MB_DEBUG_OUTPUT(" unlock target\n");
}


// 受信したゲーム情報を完全に削除（ゲーム情報有効フラグも削除）
void MB_DeleteRecvGameInfo(int index)
{
    mbrsp->usefulGameInfoFlag &= ~(0x0001 << index);
    mbrsp->validGameInfoFlag &= ~(0x0001 << index);
    MI_CpuClear16(&mbrsp->list[index], sizeof(MBGameInfoRecvList));
}


// 受信したゲーム情報をbssDescを除いて削除する。
void MB_DeleteRecvGameInfoWithoutBssdesc(int index)
{
    mbrsp->usefulGameInfoFlag &= ~(0x0001 << index);
    mbrsp->validGameInfoFlag &= ~(0x0001 << index);
    mbrsp->list[index].getFragmentFlag = 0;
    mbrsp->list[index].allFragmentFlag = 0;
    mbrsp->list[index].getPlayerFlag = 0;
    mbrsp->list[index].linkLevel = 0;
    MI_CpuClear16(&(mbrsp->list[index].gameInfo), sizeof(MBGameInfo));
}

// 受信した親機情報構造体へのポインタを取得する
MBGameInfoRecvList *MB_GetGameInfoRecvList(int index)
{
    // 有効なデータがなければNULLを返す
    if ((mbrsp->usefulGameInfoFlag & (0x01 << index)) == 0)
    {
        return NULL;
    }

    return &mbrsp->list[index];
}


//=========================================================
// ユーザVolatileデータの設定
//=========================================================

/*---------------------------------------------------------------------------*
  Name:         MB_SetSendVolatileCallback

  Description:  マルチブートのビーコン送信コールバックを設定します。
  
  Arguments:    callback    送信完了のコールバック関数です。
                            データが送信される度にコールバックが呼び出されます。
                timing      コールバック発生タイミング。
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MB_SetSendVolatCallback(MBSendVolatCallbackFunc callback, u32 timing)
{
    OSIntrMode enabled = OS_DisableInterrupts();

    sSendVolatCallback = callback;
    sSendVolatCallbackTiming = timing;

    (void)OS_RestoreInterrupts(enabled);
}



/*---------------------------------------------------------------------------*
  Name:         MB_SetUserVolatData

  Description:  マルチブートのビーコン中の空き領域にユーザデータを設定します。
  
  Arguments:    ggid        MB_RegisterFileで設定したプログラムのggidを指定して、
                            このファイルのビーコンにユーザデータを付加します。
                userData    設定するユーザデータへのポインタ。
                size        設定するユーザデータのサイズ (最大8バイトまで)
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MB_SetUserVolatData(u32 ggid, const u8 *userData, u32 size)
{
    MBGameInfo *gameInfo;

    SDK_ASSERT(size <= MB_USER_VOLAT_DATA_SIZE);
    SDK_NULL_ASSERT(userData);

    gameInfo = mbss.gameInfoListTop;
    if (gameInfo == NULL)
    {
        return;
    }

    while (gameInfo->ggid != ggid)
    {
        if (gameInfo == NULL)
        {
            return;
        }
        gameInfo = gameInfo->nextp;
    }

    {
        u32     i;

        OSIntrMode enabled = OS_DisableInterrupts();

        for (i = 0; i < size; i++)
        {
            gameInfo->volat.userVolatData[i] = userData[i];
        }

        (void)OS_RestoreInterrupts(enabled);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MB_GetUserVolatData

  Description:  ビーコン中の空き領域にあるユーザデータを取得します。
  
  Arguments:    gameInfo    Scan時に取得するgameInfoパラメータへのポインタ。
  
  Returns:      ユーザデータへのポインタ.
 *---------------------------------------------------------------------------*/
void   *MB_GetUserVolatData(const WMGameInfo *gameInfo)
{
    MbBeacon *beacon;

    SDK_NULL_ASSERT(gameInfo);

    if (!(gameInfo->attribute & WM_ATTR_FLAG_MB))
    {
        return NULL;
    }

    beacon = (MbBeacon *) (gameInfo->userGameInfo);

    if (beacon->dataAttr != MB_BEACON_DATA_ATTR_VOLAT)
    {
        return NULL;
    }

    return beacon->data.volat.userVolatData;
}

