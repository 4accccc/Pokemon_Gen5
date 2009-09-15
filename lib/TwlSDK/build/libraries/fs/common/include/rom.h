/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     rom.h

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


#ifndef NITRO_FS_ROM_H_
#define NITRO_FS_ROM_H_

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/card/hash.h>
#include <nitro/fs/file.h>
#include <nitro/fs/archive.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* functions */
#if defined(FS_IMPLEMENT)

/*---------------------------------------------------------------------------*
  Name:         FSi_InitRomArchive

  Description:  "rom" アーカイブを初期化

  Arguments:    default_dma_no   ROMアクセスに使用するDMAチャンネル。
                                 0-3の範囲外であればCPUでアクセス。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FSi_InitRomArchive(u32 default_dma_no);

/*---------------------------------------------------------------------------*
  Name:         FSi_EndRomArchive

  Description:  "rom" アーカイブを解放

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FSi_EndRomArchive(void);

/*---------------------------------------------------------------------------*
  Name:         FSi_MountSRLFile

  Description:  SRLファイルに含まれるROMファイルシステムをマウント

  Arguments:    arc              マウントに使用するFSArchive構造体。
                                 名前は登録済みでなければならない。
                file             すでにオープンされているマウント対象ファイル。
                                 マウント中はこの構造体を破棄してはならない。
                hash             ハッシュコンテキスト構造体。

  Returns:      処理が成功すればTRUE
 *---------------------------------------------------------------------------*/
BOOL FSi_MountSRLFile(FSArchive *arc, FSFile *file, CARDRomHashContext *hash);

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertPathToFATFS

  Description:  指定されたパス名をFATFS形式に置換する

  Arguments:    dst                 格納先バッファ
                src                 変換元パス
                ignorePermission    アクセス権を無視してよい場合はTRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_ConvertPathToFATFS(char *dst, const char *src, BOOL ignorePermission);

FSResult FSi_ConvertError(u32 error);

/*---------------------------------------------------------------------------*
  Name:         FSi_MountFATFS

  Description:  FATFSインタフェースをファイルシステムにマウントする。

  Arguments:    index               使用する配列要素
                arcname             アーカイブ名
                drivename           ドライブ名

  Returns:      アーカイブが正しくマウントされればTRUE
 *---------------------------------------------------------------------------*/
BOOL FSi_MountFATFS(u32 index, const char *arcname, const char *drivename);

/*---------------------------------------------------------------------------*
  Name:         FSi_MountDefaultArchives

  Description:  IPLに与えられた起動引数を参照してデフォルトアーカイブをマウント

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_MountDefaultArchives(void);

#else /* FS_IMPLEMENT */

/*---------------------------------------------------------------------------*
  Name:         FSi_ReadRomDirect

  Description:  指定のROMアドレスを直接リード

  Arguments:    src              転送元オフセット
                dst              転送先アドレス
                len              転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FSi_ReadRomDirect(const void *src, void *dst, u32 len);

#endif


#ifdef __cplusplus
} /* extern "C" */
#endif


/*---------------------------------------------------------------------------*/


#endif /* NITRO_FS_ROM_H_ */
