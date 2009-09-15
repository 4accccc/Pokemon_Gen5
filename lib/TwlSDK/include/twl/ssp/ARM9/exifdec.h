/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - include
  File:     exifdec.h

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

#ifndef TWL_SSP_EXIFDEC_H_
#define TWL_SSP_EXIFDEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderDateTime

  Description:  JPEGのデコード時の日時情報を取得する

  Arguments:    buffer : 日時情報を入れるバッファ

  Returns:      サイズ
 *---------------------------------------------------------------------------*/
u16 SSP_GetJpegDecoderDateTime(u8* buffer);

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderSoftware

  Description:  JPEGの Software タグに登録されているタイトルIDの下位4バイトを取得する

  Arguments:    buffer : Software 情報を入れるバッファ

  Returns:      サイズ
 *---------------------------------------------------------------------------*/
u16 SSP_GetJpegDecoderSoftware(char* buffer);

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderMakerNoteAddrEx

  Description:  JPEGのデコード時のメーカーノートの先頭アドレスを取得する

  Arguments:    tag : 取得するデータの種類.

  Returns:      アドレス
 *---------------------------------------------------------------------------*/
u8* SSP_GetJpegDecoderMakerNoteAddrEx(SSPJpegMakernote tag);

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderMakerNoteSizeEx

  Description:  JPEGのデコード時のメーカーノートのサイズを取得する

  Arguments:    tag : 取得するデータの種類.

  Returns:      サイズ
 *---------------------------------------------------------------------------*/
u16 SSP_GetJpegDecoderMakerNoteSizeEx(SSPJpegMakernote tag);

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderMakerNoteAddr

  Description:  JPEGのデコード時のメーカーノートのDSiカメラ用データの先頭アドレスを取得する

  Arguments:    None.

  Returns:      アドレス
 *---------------------------------------------------------------------------*/
static inline u8* SSP_GetJpegDecoderMakerNoteAddr(void)
{
    return SSP_GetJpegDecoderMakerNoteAddrEx(SSP_MAKERNOTE_PHOTO);
}

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegDecoderMakerNoteSize

  Description:  JPEGのデコード時のメーカーノートのDSiカメラ用データのサイズを取得する

  Arguments:    None.

  Returns:      サイズ
 *---------------------------------------------------------------------------*/
static inline u16 SSP_GetJpegDecoderMakerNoteSize(void)
{
    return SSP_GetJpegDecoderMakerNoteSizeEx(SSP_MAKERNOTE_PHOTO);
}

/*---------------------------------------------------------------------------*
  Name:         SSP_SetJpegDecoderSignMode

  Description:  JPEGのデコード時に署名をチェックする機能の有効/無効を切り替える

  Arguments:    TRUE なら ON に、FALSE なら OFF にする

  Returns:      以前に設定されていたモードを返す
 *---------------------------------------------------------------------------*/
BOOL SSP_SetJpegDecoderSignMode(BOOL mode);

s32 SSP_ExifDecode(u8* src, u32 src_size, int* cur, int option);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SSP_EXIFDEC_H_ */
#endif
