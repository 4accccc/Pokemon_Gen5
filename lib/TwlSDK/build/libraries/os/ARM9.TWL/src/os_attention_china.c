/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_attentionLimitedChina.c

  Copyright 2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-10#$
  $Rev: 10883 $
  $Author: mizutani_nakaba $
 *---------------------------------------------------------------------------*/

#include <os_attention.h>
#include <nitro/os/common/interrupt.h>

#ifdef SDK_TWLLTD


/*---------------------------------------------------------------------------*
 * 関数宣言
 *---------------------------------------------------------------------------*/
static u8*  OSi_LoadImageChina(IMAGE_OBJ_INDEX index, u32 *p_size);
extern void OSi_WaitVBlank(void);
extern void OSi_VBlankIntr(void);
extern void OSi_PrepareAttention(void);
extern void OSi_ShowAttention(void);



/*---------------------------------------------------------------------------*
  Name:         OSi_LoadImage

  Description:  イメージファイルのロード(仮) ※実際にはポインタを返すだけ

  Arguments:    index   -   ロードするファイルのインデックス.
                p_size  -   ファイルサイズを格納する u32 へのポインタ.
                            不要なら NULL を指定して無視させることができる.

  Returns:      イメージデータの先頭アドレスを返す
 *---------------------------------------------------------------------------*/
static u8 *OSi_LoadImageChina(IMAGE_OBJ_INDEX index, u32 *p_size)
{
    extern u8   _binary_attention_limited_china_01nbfc_bin[];
    extern u8   _binary_attention_limited_china_01nbfc_bin_end[];
    extern u8   _binary_attention_limited_china_01nbfs_bin[];
    extern u8   _binary_attention_limited_china_01nbfs_bin_end[];
    extern u8   _binary_attention_limited_china_02nbfc_bin[];
    extern u8   _binary_attention_limited_china_02nbfc_bin_end[];
    extern u8   _binary_attention_limited_china_02nbfs_bin[];
    extern u8   _binary_attention_limited_china_02nbfs_bin_end[];
    extern u8   _binary_attention_limited_china_nbfp_bin[];
    extern u8   _binary_attention_limited_china_nbfp_bin_end[];

    static u8 *ptr_table[IMAGE_OBJ_NUM] =
    {
        _binary_attention_limited_china_01nbfc_bin,
        _binary_attention_limited_china_01nbfs_bin,
        _binary_attention_limited_china_02nbfc_bin,
        _binary_attention_limited_china_02nbfs_bin,
        _binary_attention_limited_china_nbfp_bin
    };

    static u8 *ptr_end_table[IMAGE_OBJ_NUM] =
    {
        _binary_attention_limited_china_01nbfc_bin_end,
        _binary_attention_limited_china_01nbfs_bin_end,
        _binary_attention_limited_china_02nbfc_bin_end,
        _binary_attention_limited_china_02nbfs_bin_end,
        _binary_attention_limited_china_nbfp_bin_end
    };

    if(p_size)
    {
        *p_size = (u32)(ptr_end_table[index] - ptr_table[index]);
    }

    return (u8 *)ptr_table[index];
}

/*---------------------------------------------------------------------------*
  Name:         OS_ShowAttentionOfLimitedRomChina

  Description:  show string of attention to run limited mode on NITRO for China.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void OS_ShowAttentionOfLimitedRomChina(void)
{
    /* 準備 */
    OSi_PrepareAttention();

    /* 画像データのロード */
    {
        u32     plt_size;
        void    const *data_plt     = OSi_LoadImageChina(IMAGE_OBJ_PAL, &plt_size);
        void    const *data_01_chr  = OSi_LoadImageChina(IMAGE_OBJ_01_CHR, NULL);
        void    const *data_01_scr  = OSi_LoadImageChina(IMAGE_OBJ_01_SCR, NULL);
        void    const *data_02_chr  = OSi_LoadImageChina(IMAGE_OBJ_02_CHR, NULL);
        void    const *data_02_scr  = OSi_LoadImageChina(IMAGE_OBJ_02_SCR, NULL);

        /* 上画面用データを VRAM-A にロード */
        MI_UncompressLZ16(data_01_chr, (u32 *)HW_BG_VRAM);
        MI_UncompressLZ16(data_01_scr, (u32 *)(HW_BG_VRAM + 0xf000));

        /* 下画面用データを VRAM-C にロード */
        MI_UncompressLZ16(data_02_chr, (u32 *)HW_DB_BG_VRAM);
        MI_UncompressLZ16(data_02_scr, (u32 *)(HW_DB_BG_VRAM + 0xf000));

        /* パレットデータを 標準パレットRAM にロード */
        SVC_CpuCopyFast(data_plt, (u32 *)(HW_BG_PLTT),    plt_size);
        SVC_CpuCopyFast(data_plt, (u32 *)(HW_DB_BG_PLTT), plt_size);
    }

    /* 表示 ※この中でループ */
    OSi_ShowAttention();
}


#endif  // #ifdef SDK_TWLLTD
/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
