/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_child.h

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
/* ==========================================================================

    MBライブラリの子機用ヘッダです。
    マルチブート子機およびIPLにて当ライブラリを使用する場合は、
    nitro/mb.hに加え、このヘッダをインクルードしてください。

   ==========================================================================*/


#ifndef _MB_CHILD_H_
#define _MB_CHILD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/memorymap.h>
#include <nitro/mb.h>
#include "mb_fileinfo.h"
#include "mb_gameinfo.h"

/* ---------------------------------------------------------------------

        define定数

   ---------------------------------------------------------------------*/

#define MB_CHILD_SYSTEM_BUF_SIZE        (0x6000)

#define MB_MAX_SEND_BUFFER_SIZE         (0x400)
#define MB_MAX_RECV_BUFFER_SIZE         (0x80)

/* ----------------------------------------------------------------------------

    マルチブートRAMアドレス定義(内部使用)

   ----------------------------------------------------------------------------*/

/* 
    マルチブートプログラム配置可能範囲

    無線マルチブートにおけるARM9コードの初期ロード最大サイズは2.5MBです。
    
    ARM9コードのロード可能領域は
    MB_LOAD_AREA_LO ～ MB_LOAD_AREA_HI
    の範囲です。
 */
#define MB_LOAD_AREA_LO                         ( HW_MAIN_MEM )
#define MB_LOAD_AREA_HI                         ( HW_MAIN_MEM + 0x002c0000 )
#define MB_LOAD_MAX_SIZE                        ( MB_LOAD_AREA_HI - MB_LOAD_AREA_LO )


/*
    マルチブート時のARM7 staticの子機側での一時受信バッファアドレス定義
    
    ARM7コードのロード可能領域は

    a) 0x02000000 ～ 0x02300000
       (MB_LOAD_AREA_LO ～ MB_ARM7_STATIC_RECV_BUFFER_END)
    b) 0x02300000 ～ 0x023fe000
       (MB_ARM7_STATIC_RECV_BUFFER_END ～ 0x023fe000)
    c) 0x037f8000 ～ 0x0380f000
       (内部WRAM)

    のいずれかになります。
    
    ※注意
    ARM7コードを内部WRAMもしくは 0x02300000 以降にロードする場合、

    0x022c0000 ～ 0x02300000
    (MB_LOAD_AREA_HI ～ MB_ARM7_STATIC_RECV_BUFFER_END)
    にバッファを設け、このバッファに受信させたのちに
    ブート時に指定アドレスに再配置するようにしています。
    
    ・0x02300000 のアドレスをまたぐ配置のARM7コード
    ・0x02300000 以降のアドレスで、サイズが 0x40000 ( MB_ARM7_STATIC_RECV_BUFFER_SIZE ) 
    　を超えるARM7コードは正常動作を保証できませんので、
    　このようなコードの作成を禁止いたします。
 */

#define MB_ARM7_STATIC_RECV_BUFFER              MB_LOAD_AREA_HI
#define MB_ARM7_STATIC_RECV_BUFFER_END          ( HW_MAIN_MEM + 0x00300000 )
#define MB_ARM7_STATIC_RECV_BUFFER_SIZE         ( MB_ARM7_STATIC_RECV_BUFFER_END - MB_LOAD_AREA_HI )
#define MB_ARM7_STATIC_LOAD_AREA_HI             ( MB_LOAD_AREA_HI + MB_ARM7_STATIC_RECV_BUFFER_SIZE )
#define MB_ARM7_STATIC_LOAD_WRAM_MAX_SIZE       ( 0x18000 - 0x1000 )    // 0x18000 = HW_WRAM_SIZE + HW_PRV_WRAM_SIZE

/* BssDescの一時格納場所 */
#define MB_BSSDESC_ADDRESS                      ( HW_MAIN_MEM + 0x003fe800 )
/* MBDownloadFileInfoの一時格納場所 */
#define MB_DOWNLOAD_FILEINFO_ADDRESS            ( MB_BSSDESC_ADDRESS + ( ( ( MB_BSSDESC_SIZE ) + (32) - 1 ) & ~((32) - 1) ) )
/* マルチブートを起動した時点で差さっているROMカードのROMヘッダ退避場所 */
#define MB_CARD_ROM_HEADER_ADDRESS              ( MB_DOWNLOAD_FILEINFO_ADDRESS + ( ( ( MB_DOWNLOAD_FILEINFO_SIZE ) + (32) - 1 ) & ~((32) - 1) ) )
/* ROM Headerの格納場所 */
#define MB_ROM_HEADER_ADDRESS                   (HW_MAIN_MEM + 0x007ffe00)


/* ---------------------------------------------------------------------

        マルチブートライブラリ(MB)子機用API. IPL で使用されます

   ---------------------------------------------------------------------*/

/* 子機パラメータ設定 & スタート */
int     MB_StartChild(void);

/* MB子機で使用するワークメモリのサイズを取得 */
int     MB_GetChildSystemBufSize(void);

/* 子機状態通知コールバックの設定 */
void    MB_CommSetChildStateCallback(MBCommCStateCallbackFunc callback);

/* 子機状態の取得 */
int     MB_CommGetChildState(void);

/* ダウンロード開始 */
BOOL    MB_CommStartDownload(void);

/* 子機のダウンロード進捗状況を 0 ～ 100 で取得 */
u16     MB_GetChildProgressPercentage(void);

/* 親機への接続試み＆ダウンロードファイル要求 */
int     MB_CommDownloadRequest(int index);      // MbBeaconRecvStatusのリストNo.で接続要求する親機を指定。

#ifdef __cplusplus
}
#endif

#endif /*  _MB_CHILD_H_    */
