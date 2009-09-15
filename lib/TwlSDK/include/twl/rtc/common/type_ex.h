/*---------------------------------------------------------------------------*
  Project:  TwlSDK - rtc - include
  File:     type_ex.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_RTC_COMMON_TYPE_EX_H_
#define TWL_RTC_COMMON_TYPE_EX_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

#include	<twl/types.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
/* FOUTデューティー定義 */
#define     RTC_FOUT_DUTY_1HZ         	0x0001
#define     RTC_FOUT_DUTY_2HZ           0x0002
#define     RTC_FOUT_DUTY_4HZ           0x0004
#define     RTC_FOUT_DUTY_8HZ           0x0008
#define     RTC_FOUT_DUTY_16HZ          0x0010
#define     RTC_FOUT_DUTY_32HZ          0x0020
#define     RTC_FOUT_DUTY_64HZ          0x0040
#define     RTC_FOUT_DUTY_128HZ         0x0080
#define     RTC_FOUT_DUTY_256HZ         0x0100
#define     RTC_FOUT_DUTY_512HZ         0x0200
#define     RTC_FOUT_DUTY_1KHZ          0x0400
#define     RTC_FOUT_DUTY_2KHZ          0x0800
#define     RTC_FOUT_DUTY_4KHZ          0x1000
#define     RTC_FOUT_DUTY_8KHZ          0x2000
#define     RTC_FOUT_DUTY_16KHZ         0x4000
#define     RTC_FOUT_DUTY_32KHZ         0x8000

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
/* アップカウンタ構造体 */
typedef union RTCRawCounter
{
    struct
    {
        u32 count:24;   // big endian
        u32 dummy:8;
    };

    u8  bytes[4];
}
RTCRawCounter;

/* FOUT構造体 */
typedef union RTCRawFout
{
    struct
    {
        u16 fout;
        u16 dummy0;
    };
    struct
    {
        u8  fout2;
        u8  fout1;
        u16 dummy1;
    };
}
RTCRawFout;

/* アラーム拡張構造体 */
typedef union
{
    u32     year:8;            	/* 年( 00 ~ 99 ) */
    u32     month:5;            /* 月( 01 ~ 12 ) */
    u32     dummy0:1;
    u32     ye:1;               /* 年設定有効フラグ */
    u32     me:1;               /* 月設定有効フラグ */
    u32     day:6;              /* 日( 01 ~ 31 ) 月/閏年により上限は変動 */
    u32     dummy1:1;
    u32     de:1;               /* 日設定有効フラグ */
    u32     dummy2:8;
}
RTCRawAlarmEx;

/* ARM9 <-> ARM7 受け渡しデータ構造体( 8バイト共用体 ) */
typedef union RTCRawDataEx
{
    struct
    {
        RTCRawAlarmEx alarmex;      // アラーム( 1 or 2 )拡張レジスタ設定
        union
        {
            RTCRawCounter counter;  // アップカウンタ
            RTCRawFout fout;        // FOUT1/FOUT2レジスタ設定
        };
    }
    a;

    u32     words[2];              	// 4バイトアクセス用
    u16     halfs[4];               // 2バイトアクセス用
    u8      bytes[8];               // バイトアクセス用
}
RTCRawDataEx;


/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_RTC_COMMON_TYPE_EX_H_ */
