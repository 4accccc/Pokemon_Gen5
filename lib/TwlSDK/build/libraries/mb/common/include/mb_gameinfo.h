/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_gameinfo.h

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

/*
 * このヘッダファイルは内部実装及び子機側でのみ使用します.
 * 通常のマルチブート親機を作成する上では必要ありません.
 */


#ifndef MB_GAME_INFO_H_
#define MB_GAME_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <nitro/types.h>
#include <nitro/wm.h>
#include <nitro/mb/mb_gameinfo.h>


//=============================================================================
// 
// データ型　定義
//
//=============================================================================

#define MB_GAMEINFO_PARENT_FLAG                     (0x0001)
#define MB_GAMEINFO_CHILD_FLAG( __child_aid__ )     ( 1 << ( __child_aid__ ) )

//---------------------------------------------------------
// 親機から子機にビーコンで配信するゲーム情報
//---------------------------------------------------------

/*
 * 子機側ゲーム情報受信リスト構造体.
 */
typedef struct MBGameInfoRecvList
{
    MBGameInfo gameInfo;               // 親機ゲーム情報
    WMBssDesc bssDesc;                 // 親機接続用情報
    u32     getFragmentFlag;           // 現在受信した断片ビーコンをビットで示す。
    u32     allFragmentFlag;           // fragmentMaxNumをビットに換算した値
    u16     getPlayerFlag;             // 現在受信済みのプレイヤーフラグをビットで示す。
    s16     lifetimeCount;             // この情報に対する寿命カウンタ（この親機のビーコンを受信したら、寿命が延びる）
    u16     linkLevel;                 /* 親機からのBeacon受信強度を示す、4段階の値. */
    u8      beaconNo;                  // 最後に受信したビーコン番号
    u8      sameBeaconRecvCount;       // 同じビーコン番号を連続して受信した回数
}
MBGameInfoRecvList, MbGameInfoRecvList;


/*
 * 子機側ビーコン受信ステータス構造体
 */
typedef struct MbBeaconRecvStatus
{
    u16     usingGameInfoFlag;         // ゲーム情報の受信に使用中のgameInfo配列要素をビットで示す。
    u16     usefulGameInfoFlag;        // 一旦は全てのビーコンを受信して、validGameInfoFlagが立ったゲーム情報を示す。
    // （通信メンバーの更新等で、validGameInfoFlagは一時的に落ちる場合があるので、表示や接続にはこちらのフラグを使用して判断する。）
    u16     validGameInfoFlag;         // ゲーム情報を完全に受信しているgameInfo配列要素をビットで示す。
    u16     nowScanTargetFlag;         // 現在のScanターゲットをビットで示す。
    s16     nowLockTimeCount;          // 現在のScanターゲットの残りロック時間。
    s16     notFoundLockTargetCount;   // 現在のScanTargetが連続で見つからなかった回数
    u16     scanCountUnit;             // 現在のスキャン時間をカウント数に換算した値
    u8      pad[2];
    MBGameInfoRecvList list[MB_GAME_INFO_RECV_LIST_NUM];        // ゲーム情報受信リスト
}
MbBeaconRecvStatus;
/*
 * ※子機側は、
 * getFragmentFlag == allFragmentFlag　かつ　
 * getPlayerFlag   == gameInfo.volat.nowPlayerFlag
 * で、親機情報取得完了となる。
 */


/*
 * MB_RecvGameInfoBeacon, MB_CountGameInfoLifetime の
 * コールバック関数で返されるmsg.
 */
typedef enum MbBeaconMsg
{
    MB_BC_MSG_GINFO_VALIDATED = 1,
    MB_BC_MSG_GINFO_INVALIDATED,
    MB_BC_MSG_GINFO_LOST,
    MB_BC_MSG_GINFO_LIST_FULL,
    MB_BC_MSG_GINFO_BEACON
}
MbBeaconMsg;

typedef void (*MBBeaconMsgCallback) (MbBeaconMsg msg, MBGameInfoRecvList * gInfop, int index);

typedef void (*MbScanLockFunc) (u8 *macAddress);
typedef void (*MbScanUnlockFunc) (void);

/******************************************************************************/
/* 以下は内部使用 */


//------------------
// 親機側関数
//------------------

    // 送信ステータスの初期化。
void    MB_InitSendGameInfoStatus(void);

    // MbGameRegistryからMbGameInfoを作成する。
void    MBi_MakeGameInfo(MBGameInfo *gameInfop,
                         const MBGameRegistry *mbGameRegp, const MBUserInfo *parent);

    // MBGameInfoの子機メンバー情報を更新する。
void    MB_UpdateGameInfoMember(MBGameInfo *gameInfop,
                                const MBUserInfo *member, u16 nowPlayerFlag, u16 changePlayerFlag);

    // 生成したMBGameInfoをビーコンで発信するよう送信リストに追加する。
void    MB_AddGameInfo(MBGameInfo *newGameInfop);

    // 送信リストに追加しているMBGameInfoを削除する。
BOOL    MB_DeleteGameInfo(MBGameInfo *gameInfop);

    // 送信リストに登録されているMBGameInfoをビーコンに乗せて発信する。
void    MB_SendGameInfoBeacon(u32 ggid, u16 tgid, u8 attribute);


//------------------
// 子機側関数
//------------------
    // ビーコン受信ステータスバッファをstaticに確保
void    MBi_SetBeaconRecvStatusBufferDefault(void);
    // ビーコン受信ステータスバッファを設定
void    MBi_SetBeaconRecvStatusBuffer(MbBeaconRecvStatus * buf);

    // 取得したビーコンがマルチブート親機かどうかを判定
BOOL    MBi_CheckMBParent(WMBssDesc *bssDescp);

    // 受信ステータスの初期化
void    MB_InitRecvGameInfoStatus(void);

    // 受信したビーコンからMBGameInfoを取り出す。
BOOL    MB_RecvGameInfoBeacon(MBBeaconMsgCallback Callbackp, u16 linkLevel, WMBssDesc *bssDescp);

    // 親機情報リストの寿命カウント
void    MB_CountGameInfoLifetime(MBBeaconMsgCallback Callbackp, BOOL found_parent);

    // スキャンロック関数を設定
void    MBi_SetScanLockFunc(MbScanLockFunc lockFunc, MbScanUnlockFunc unlockFunc);

    // 受信した親機情報構造体へのポインタを取得する
MBGameInfoRecvList *MB_GetGameInfoRecvList(int index);

/* ビーコン受信状態を取得 */
const MbBeaconRecvStatus *MB_GetBeaconRecvStatus(void);

/* 指定されたゲーム情報の削除 */
void    MB_DeleteRecvGameInfo(int index);
void    MB_DeleteRecvGameInfoWithoutBssdesc(int index);


#ifdef __cplusplus
}
#endif

#endif // MB_GAME_INFO_H_
