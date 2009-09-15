 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_cache.h

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

#if	!defined(NITRO_MB_CACHE_H_)
#define NITRO_MB_CACHE_H_


#if	defined(__cplusplus)
extern  "C"
{
#endif


/* constant ---------------------------------------------------------------- */


#define	MB_CACHE_INFO_MAX	4

#define MB_CACHE_STATE_EMPTY   0
#define MB_CACHE_STATE_BUSY    1
#define MB_CACHE_STATE_READY   2
#define MB_CACHE_STATE_LOCKED  3


/* struct ------------------------------------------------------------------ */


/*
 * ページングによる仮想メモリのための構造体.
 * この構造体配列で大容量アドレス空間(CARD等)をキャッシュ管理する.
 */
    typedef struct
    {
        u32     src;                   /* logical source address */
        u32     len;                   /* cache length */
        u8     *ptr;                   /* pointer to cache buffer */
        u32     state;                 /* if 1, ready to use */
    }
    MBiCacheInfo;

    typedef struct
    {
        u32     lifetime;              /* if hit or timeout, set 0. */
        u32     recent;                /* unused */
        MBiCacheInfo *p_list;          /* unused */
        u32     size;                  /* unused */
        char    arc_name[FS_ARCHIVE_NAME_LEN_MAX + 1];  /* target archive */
        u32     arc_name_len;          /* archive name length */
        FSArchive *arc_pointer;
        u8      reserved[32 - FS_ARCHIVE_NAME_LEN_MAX - 1 - sizeof(u32) - sizeof(FSArchive*)];
        MBiCacheInfo list[MB_CACHE_INFO_MAX];
    }
    MBiCacheList;


/* function ---------------------------------------------------------------- */



/*---------------------------------------------------------------------------*
  Name:         MBi_InitCache

  Description:  キャッシュリストを初期化する.
                

  Arguments:    pl         キャッシュリスト

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    MBi_InitCache(MBiCacheList * pl);

/*---------------------------------------------------------------------------*
  Name:         MBi_AttachCacheBuffer

  Description:  キャッシュリストへバッファを割り当てる.
                
  Arguments:    pl         キャッシュリスト
                ptr        割り当てるバッファ
                src        ptr のソースアドレス
                len        ptr のバイトサイズ
                state      指定する初期状態
                           (MB_CACHE_STATE_READY か MB_CACHE_STATE_LOCKED)

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    MBi_AttachCacheBuffer(MBiCacheList * pl, u32 src, u32 len, void *ptr, u32 state);

/*---------------------------------------------------------------------------*
  Name:         MBi_ReadFromCache

  Description:  指定したアドレスの内容をキャッシュからリードする.

  Arguments:    pl         キャッシュリスト
                src        リード元アドレス
                len        リードサイズ (BYTE)
                dst        リード先アドレス.

  Returns:      キャッシュにヒットすればリードして TRUE, そうでなければ FALSE.
 *---------------------------------------------------------------------------*/
    BOOL    MBi_ReadFromCache(MBiCacheList * pl, u32 src, void *dst, u32 len);

/*---------------------------------------------------------------------------*
  Name:         MBi_TryLoadCache

  Description:  指定したアドレスの内容をキャッシュへロードする.
                最も番地の小さい READY のページキャッシュが破棄される.

  Arguments:    pl         キャッシュリスト
                src        リロード元アドレス
                len        リロードサイズ (BYTE)

  Returns:      リロードが開始できれば TRUE, そうでなければ FALSE.
                (リロード処理のエンジンがシステム内で単一の場合,
                 前回の処理を未了ならこの関数は FALSE を返すであろう)
 *---------------------------------------------------------------------------*/
    BOOL    MBi_TryLoadCache(MBiCacheList * pl, u32 src, u32 len);



#if	defined(__cplusplus)
}
#endif


#endif                                 /* NITRO_MB_CACHE_H_ */
