/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - include
  File:     exifenc.h

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

#ifndef TWL_SSP_EXIFENC_H_
#define TWL_SSP_EXIFENC_H_

#include <twl/ssp/common/ssp_jpeg_type.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegEncoderMakerNoteEx

  Description:  JPEGのエンコード時にメーカーノートに追加するデータ

  Arguments:    tag    : 埋め込むデータの種類
                buffer : データのバッファ
                size   : データのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SSP_SetJpegEncoderMakerNoteEx(SSPJpegMakernote tag, const u8* buffer, u32 size);

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegEncoderMakerNote

  Description:  JPEGのエンコード時にメーカーノートに追加するDSiカメラ用のデータ

  Arguments:    buffer : DSiカメラ用のデータのバッファ
                size   : データのサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void SSP_SetJpegEncoderMakerNote(const u8* buffer, u32 size)
{
    SSP_SetJpegEncoderMakerNoteEx(SSP_MAKERNOTE_PHOTO, buffer, size);
}

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegEncoderDateTime

  Description:  JPEGのエンコード時に Exif IFD の DateTimeOriginal と
                DateTimeDigitized タグに設定する日時を指定する

  Arguments:    buffer : YYYY:MM:DD HH:MM:SS 形式の日時を示す文字列

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SSP_SetJpegEncoderDateTime(const u8* buffer);

/*---------------------------------------------------------------------------*
  Name:         SSP_GetDateTime

  Description:  内部で RTC_GetDateTime 関数を呼び出すだけの関数

  Arguments:    None.

  Returns:      成功したならば TRUE を返す.
 *---------------------------------------------------------------------------*/
BOOL SSP_GetDateTime( RTCDate* date , RTCTime* time );

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegEncoderDateTimeNow

  Description:  JPEGのエンコード時に Exif IFD の DateTimeOriginal と
                DateTimeDigitized タグに現在の時刻を設定する

  Arguments:    None.

  Returns:      現在時刻の設定に成功したら TRUE.
 *---------------------------------------------------------------------------*/
BOOL SSP_SetJpegEncoderDateTimeNow(void);

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegEncoderSignMode

  Description:  JPEGのエンコード時に署名を付加する機能の有効/無効を切り替える

  Arguments:    TRUE なら ON に、FALSE なら OFF にする

  Returns:      以前に設定されていたモードを返す
 *---------------------------------------------------------------------------*/
BOOL SSP_SetJpegEncoderSignMode(BOOL mode);

u32 SSP_ExifEncode(u8* l_dst, u32 width, u32 height, const u8* thumb_src, u32 thumb_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SSP_EXIFENC_H_ */
#endif
