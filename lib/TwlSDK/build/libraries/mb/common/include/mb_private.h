/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_private.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

/*
    MBライブラリの各ソースファイルでは、このヘッダファイルのみをインクルードしています。
*/

#ifndef _MB_PRIVATE_H_
#define _MB_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#if	!defined(NITRO_FINALROM)
/*
#define PRINT_DEBUG
*/
#endif

#define CALLBACK_WM_STATE       0

#include <nitro/types.h>
#include <nitro/wm.h>
#include <nitro/mb.h>
#include "mb_wm_base.h"
#include "mb_common.h"
#include "mb_block.h"
#include "mb_rom_header.h"
#include "mb_gameinfo.h"
#include "mb_fileinfo.h"
#include "mb_child.h"

/* キャッシュ方式のための新機構 */
#include "mb_cache.h"
#include "mb_task.h"

/* デバッグ出力 */
#ifdef  PRINT_DEBUG
#define MB_OUTPUT( ... )        OS_TPrintf( __VA_ARGS__ )
#define MB_DEBUG_OUTPUT( ... )  MBi_DebugPrint(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )

#else
#define MB_OUTPUT( ... )        ((void)0)
#define MB_DEBUG_OUTPUT( ... )  ((void)0)
#endif

// ----------------------------------------------------------------------------
// definition

/* ----------------------------------------------------------------------------

    ブロック転送関連定義

   ----------------------------------------------------------------------------*/

#define MB_IPL_VERSION                          (0x0001)
#define MB_MAX_BLOCK                            12288   // max 12288 * 256 = 3.0Mbyte

#define MB_NUM_PARENT_INFORMATIONS              16      /* 最大親機数 */

#define MB_SCAN_TIME_NORMAL                     WM_DEFAULT_SCAN_PERIOD
#define MB_SCAN_TIME_NORMAL_FOR_FAKE_CHILD      20
#define MB_SCAN_TIME_LOCKING                    220

//SDK_COMPILER_ASSERT( sizeof(MBDownloadFileInfo) <= MB_COMM_BLOCK_SIZE );


/* ----------------------------------------------------------------------------

    内部使用型定義

   ----------------------------------------------------------------------------*/

/*
 * MB ライブラリで使用するワーク領域.
 */

typedef struct MB_CommCommonWork
{
    //  --- 共通    ---
    //  送受信バッファ
    u32     sendbuf[MB_MAX_SEND_BUFFER_SIZE / sizeof(u32)] ATTRIBUTE_ALIGN(32);
    u32     recvbuf[MB_MAX_RECV_BUFFER_SIZE * MB_MAX_CHILD * 2 / sizeof(u32)] ATTRIBUTE_ALIGN(32);
    MBUserInfo user;                   // 自機のユーザー情報
    u16     isMbInitialized;
    int     block_size_max;
    BOOL    start_mp_busy;             /* WM_StartMP() is operating */
    BOOL    is_started_ex;             /* WM_StartParentEX() called */
    u8      padding[28];
}
MB_CommCommonWork;

typedef struct MB_CommPWork
{
    MB_CommCommonWork common;

    //  --- 親機用  ---
    MBUserInfo childUser[MB_MAX_CHILD] ATTRIBUTE_ALIGN(4);
    u16     childversion[MB_MAX_CHILD]; // 子機のバージョン情報(子機台数分)
    u32     childggid[MB_MAX_CHILD];   // 子機のGGID(子機台数分)
    MBCommPStateCallback parent_callback;       // 親機の状態コールバック関数ポインタ
    int     p_comm_state[MB_MAX_CHILD]; // それぞれの子機に対するstateを持つ
    u8      file_num;                  // 登録されているファイル数
    u8      cur_fileid;                // 現在送信するFile番号
    s8      fileid_of_child[MB_MAX_CHILD];      // 子機のリクエストしてきたFile番号(リクエスト無しの場合は-1)
    u8      child_num;                 // エントリー子機数
    u16     child_entry_bmp;           // 子機のエントリー管理bitmap
    MbRequestPieceBuf req_data_buf;    // 子機の分割リクエスト受信用のバッファ
    u16     req2child[MB_MAX_CHILD];   // 子機へ送信するリクエスト
    MBUserInfo childUserBuf;           // アプリケーションに渡すための、子機情報バッファ

    //  ファイル情報
    struct
    {
        // DownloadFileInfoのバッファ
        MBDownloadFileInfo dl_fileinfo;
        MBGameInfo game_info;
        MB_BlockInfoTable blockinfo_table;
        MBGameRegistry *game_reg;
        void   *src_adr;               //  親機上のブートイメージ先頭アドレス
        u16     currentb;              // 現在のブロック
        u16     nextb;                 // 次回送信するブロック
        u16     pollbmp;               // データ送信用PollBitmap
        u16     gameinfo_child_bmp;    // GameInfo更新のための現在のメンバbitmap
        u16     gameinfo_changed_bmp;  // GameInfo更新のための変更が発生したメンバbitmap
        u8      active;
        u8      update;

        /*
         * キャッシュ方式リードの対応のために追加.
         * これらが指す内容は src_adr の中.
         */
        MBiCacheList *cache_list;      /* キャッシュリスト */
        u32    *card_mapping;          /* 各セグメント先頭の CARD アドレス */

    }
    fileinfo[MB_MAX_FILE];

    BOOL    useWvrFlag;                // WVRを使用しているかどうかのフラグ
    u8      padding2[20];

    /* タスクスレッドのために追加 */
    u8      task_work[2 * 1024];
    MBiTaskInfo cur_task;

}
MB_CommPWork;


typedef struct MB_CommCWork
{
    MB_CommCommonWork common;

    //  --- 子機用  ---
    WMBssDesc bssDescbuf ATTRIBUTE_ALIGN(32);   // WMBssDescのバックアップ
    MBDownloadFileInfo dl_fileinfo;    // 子機のDownloadFileInfoバッファ
    MBCommCStateCallbackFunc child_callback;    // 子機の状態コールバック関数ポインタ
    int     c_comm_state;              // 子機のstate
    int     connectTargetNo;           // 接続対象のMbBeaconRecvStatusリストNo.
    u8      fileid;                    // リクエストしたFile番号
    u8      _padding1[1];
    u16     user_req;
    u16     got_block;                 // Download済みブロック数
    u16     total_block;               // Downloadファイル総ブロック
    u8      recvflag[MB_MAX_BLOCK / 8]; // ブロック受信状態を表すフラグ
    MB_BlockInfoTable blockinfo_table;
    int     last_recv_seq_no;          // 前回受信したブロックシーケンス番号
    u8      boot_end_flag;             // BOOT_READYによるMB終了処理中か否かのフラグ
    u8      _padding2[15];
}
MB_CommCWork;


/* ----------------------------------------------------------------------------

    内部使用変数定義

   ----------------------------------------------------------------------------*/

extern MB_CommCommonWork *mbc;
/* 親機用ワークへのポインタマクロ */
#define pPwork                                  ((MB_CommPWork*)mbc)
/* 子機用ワークへのポインタマクロ */
#define pCwork                                  ((MB_CommCWork*)mbc)


#ifdef __cplusplus
}
#endif

#endif /*  _MB_PRIVATE_H_  */
