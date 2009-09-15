/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - STD
  File:     std_unicode.c

  Copyright 2006-2009 Nintendo.  All rights reserved.

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

#include <nitro/std/unicode.h>


#if defined(SDK_ARM9) || !defined(SDK_NITRO)
// メモリ制約上、NITROモードのARM7ではUnicode変換機能を利用できない

#if defined(SDK_ARM9)
#define STD_UNICODE_STATIC_IMPLEMENTATION
#endif

#if defined(STD_UNICODE_STATIC_IMPLEMENTATION)
// ARM9はテーブルを自身で保持して実行コードも常駐モジュール内に配置する

// 配列のポインタをWEAKシンボルにして、HYBRIDでコードテーブルを
// ltdmainに追い出せるようにしています。
// しかしWEAKシンボルで、同じファイルにあると外部で上書きされないので、
// 配列のポインタを別ファイルstd_unicode_array.cに分けました。
extern const u8    *STD_Unicode2SjisArray;
extern const u16   *STD_Sjis2UnicodeArray;
#else
// ARM7はARM9のテーブルを参照して実行コードもLTD領域に分割配置する
static const u8    *STD_Unicode2SjisArray = NULL;
static const u16   *STD_Sjis2UnicodeArray = NULL;
static STDResult STDi_ConvertStringSjisToUnicodeCore(u16 *dst, int *dst_len,
                                                     const char *src, int *src_len,
                                                     STDConvertUnicodeCallback callback)
                                                     __attribute__((never_inline));
static STDResult STDi_ConvertStringUnicodeToSjisCore(char *dst, int *dst_len,
                                                     const u16 *src, int *src_len,
                                                     STDConvertSjisCallback callback)
                                                     __attribute__((never_inline));
#endif


/*****************************************************************************/
/* function */

/*---------------------------------------------------------------------------*
  Name:         STDi_GetUnicodeConversionTable

  Description:  Unicode変換テーブルを取得

  Arguments:    u2s               Unicode -> Sjis の変換テーブルポインタ格納先
                s2u               Sjis -> Unicode の変換テーブルポインタ格納先

  Returns:      None.
 *---------------------------------------------------------------------------*/
void STDi_GetUnicodeConversionTable(const u8 **u2s, const u16 **s2u)
{
    if (u2s)
    {
        *u2s = STD_Unicode2SjisArray;
    }
    if (s2u)
    {
        *s2u = STD_Sjis2UnicodeArray;
    }
}

/*---------------------------------------------------------------------------*
  Name:         STDi_AttachUnicodeConversionTable

  Description:  Unicode変換テーブルをSTDライブラリへ割り当て

  Arguments:    u2s               Unicode -> Sjis の変換テーブル
                s2u               Sjis -> Unicode の変換テーブル

  Returns:      None.
 *---------------------------------------------------------------------------*/
void STDi_AttachUnicodeConversionTable(const u8 *u2s, const u16 *s2u)
{
#if defined(STD_UNICODE_STATIC_IMPLEMENTATION)
    (void)u2s;
    (void)s2u;
#else
    STD_Unicode2SjisArray = u2s;
    STD_Sjis2UnicodeArray = s2u;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         STD_ConvertStringSjisToUnicode

  Description:  ShiftJIS文字列をUnicode文字列に変換.

  Arguments:    dst               変換先バッファ.
                                  NULL を指定すると格納処理は無視される.
                dst_len           変換先バッファの最大文字数を格納して渡し,
                                  実際に格納された文字数を受け取るポインタ.
                                  NULL を与えた場合は無視される.
                src               変換元バッファ.
                src_len           変換すべき最大文字数を格納して渡し,
                                  実際に変換された文字数を受け取るポインタ.
                                  この指定よりも文字列終端の位置が優先される.
                                  負の値を格納して渡すか NULL を与えた場合は
                                  終端位置までの文字数を指定したとみなされる.
                callback          変換できない文字が現れた時に呼ばれるコールバック.
                                  NULLを指定した場合, 変換できない文字の位置で
                                  変換処理を終了する.

  Returns:      変換処理の結果.
 *---------------------------------------------------------------------------*/
STDResult STD_ConvertStringSjisToUnicode(u16 *dst, int *dst_len,
                                         const char *src, int *src_len,
                                         STDConvertUnicodeCallback callback)
#if !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
{
    // テーブルが解決していなければASCII変換で代替
    if (STD_Unicode2SjisArray == NULL)
    {
        STDResult   result = STD_RESULT_SUCCESS;
        int         i;
        int         max = 0x7FFFFFFF;
        if (src_len && (*src_len >= 0))
        {
            max = *src_len;
        }
        if (dst && dst_len && (*dst_len >= 0) && (*dst_len < max))
        {
            max = *dst_len;
        }
        for (i = 0; i < max; ++i)
        {
            int     c = ((const u8 *)src)[i];
            if (c == 0)
            {
               break;
            }
            else if (c >= 0x80)
            {
               result = STD_RESULT_ERROR;
               break;
            }
            dst[i] = (u16)c;
        }
        if (src_len)
        {
            *src_len = i;
        }
        if (dst_len)
        {
            *dst_len = i;
        }
        return result;
    }
    else
    {
        return STDi_ConvertStringSjisToUnicodeCore(dst, dst_len, src, src_len, callback);
    }
}
#include <twl/ltdmain_begin.h>
static STDResult STDi_ConvertStringSjisToUnicodeCore(u16 *dst, int *dst_len,
                                                     const char *src, int *src_len,
                                                     STDConvertUnicodeCallback callback)
#endif // !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
{
    STDResult retval = STD_RESULT_SUCCESS;
    if (!src)
    {
        retval = STD_RESULT_INVALID_PARAM;
    }
    else
    {
        int     src_pos = 0;
        int     dst_pos = 0;
        int     src_max = (src_len && (*src_len >= 0)) ? *src_len : 0x7FFFFFFF;
        int     dst_max = (dst && dst_len && (*dst_len >= 0)) ? *dst_len : 0x7FFFFFFF;

        /* いずれかのバッファ長が終端に達するまで1文字ずつ処理 */
        while ((dst_pos < dst_max) && (src_pos < src_max))
        {
            u16     dst_tmp[4];
            int     dst_count = 0;
            int     src_count;
            u32     c1 = (u8)src[src_pos];
            /* 文字列長よりも終端の検出を優先する */
            if (!c1)
            {
                break;
            }
            /* ASCII [00, 7E] */
            else if (c1 <= 0x7E)
            {
                dst_tmp[0] = (u16)c1;
                src_count = 1;
                dst_count = 1;
            }
            /* 半角カナ [A1, DF] */
            else if ((c1 >= 0xA1) && (c1 <= 0xDF))
            {
                dst_tmp[0] = (u16)(c1 + (0xFF61 - 0xA1));
                src_count = 1;
                dst_count = 1;
            }
            /* ShiftJIS */
            else if (STD_IsSjisCharacter(&src[src_pos]))
            {
                src_count = 2;
                if (src_pos + src_count <= src_max)
                {
                    /*
                     * 変換テーブルの範囲は
                     * 第1バイト { 81 - 9F, E0 - FC }, 第2バイト { 40 - FF }.
                     * テーブル計算時に除算を避ける効率化のため
                     * 第2バイト { 7F, FD, FE, FF } は省略せず残してある.
                     */
                    u32     c2 = (u8)src[src_pos + 1];
                    c1 -= 0x81 + ((c1 >= 0xE0) ? (0xE0 - 0xA0) : 0);
                    dst_tmp[0] = STD_Sjis2UnicodeArray[c1 * 0xC0 + (c2 - 0x40)];
                    dst_count = (dst_tmp[0] ? 1 : 0);
                }
            }
            /* 変換できない文字が現れたらコールバックを呼び出す */
            if (dst_count == 0)
            {
                if (!callback)
                {
                    retval = STD_RESULT_CONVERSION_FAILED;
                }
                else
                {
                    src_count = src_max - src_pos;
                    dst_count = sizeof(dst_tmp) / sizeof(*dst_tmp);
                    retval = (*callback) (dst_tmp, &dst_count, &src[src_pos], &src_count);
                }
                if (retval != STD_RESULT_SUCCESS)
                {
                    break;
                }
            }
            /* 変換元または変換先が終端を超えるならここで終了 */
            if ((src_pos + src_count > src_max) || (dst_pos + dst_count > dst_max))
            {
                break;
            }
            /* バッファが有効ならば変換結果を格納 */
            if (dst)
            {
                int     i;
                for (i = 0; i < dst_count; ++i)
                {
                    MI_StoreLE16(&dst[dst_pos + i], (u16)dst_tmp[i]);
                }
            }
            src_pos += src_count;
            dst_pos += dst_count;
        }
        /* 変換文字数を返す */
        if (src_len)
        {
            *src_len = src_pos;
        }
        if (dst_len)
        {
            *dst_len = dst_pos;
        }
    }
    return retval;
}
#if !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
#include <twl/ltdmain_end.h>
#endif // !defined(STD_UNICODE_STATIC_IMPLEMENTATION)

/*---------------------------------------------------------------------------*
  Name:         STD_ConvertStringUnicodeToSjis

  Description:  Unicode文字列をShiftJIS文字列に変換

  Arguments:    dst               変換先バッファ.
                                  NULL を指定すると格納処理は無視される.
                dst_len           変換先バッファの最大文字数を格納して渡し,
                                  実際に格納された文字数を受け取るポインタ.
                                  NULL を与えた場合は無視される.
                src               変換元バッファ.
                src_len           変換すべき最大文字数を格納して渡し,
                                  実際に変換された文字数を受け取るポインタ.
                                  この指定よりも文字列終端の位置が優先される.
                                  負の値を格納して渡すか NULL を与えた場合は
                                  終端位置までの文字数を指定したとみなされる.
                callback          変換できない文字が現れた時に呼ばれるコールバック.
                                  NULLを指定した場合, 変換できない文字の位置で
                                  変換処理を終了する.

  Returns:      変換処理の結果.
 *---------------------------------------------------------------------------*/
STDResult STD_ConvertStringUnicodeToSjis(char *dst, int *dst_len,
                                         const u16 *src, int *src_len,
                                         STDConvertSjisCallback callback)
#if !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
{
    // テーブルが解決していなければASCII変換で代替
    if (STD_Unicode2SjisArray == NULL)
    {
        STDResult   result = STD_RESULT_SUCCESS;
        int         i;
        int         max = 0x7FFFFFFF;
        if (src_len && (*src_len >= 0))
        {
            max = *src_len;
        }
        if (dst && dst_len && (*dst_len >= 0) && (*dst_len < max))
        {
            max = *dst_len;
        }
        for (i = 0; i < max; ++i)
        {
            int     c = ((const u16 *)src)[i];
            if (c == 0)
            {
               break;
            }
            else if (c >= 0x80)
            {
               result = STD_RESULT_ERROR;
               break;
            }
            dst[i] = (char)c;
        }
        if (src_len)
        {
            *src_len = i;
        }
        if (dst_len)
        {
            *dst_len = i;
        }
        return result;
    }
    else
    {
        return STDi_ConvertStringUnicodeToSjisCore(dst, dst_len, src, src_len, callback);
    }
}
#include <twl/ltdmain_begin.h>
static STDResult STDi_ConvertStringUnicodeToSjisCore(char *dst, int *dst_len,
                                                     const u16 *src, int *src_len,
                                                     STDConvertSjisCallback callback)
#endif // !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
{
    STDResult retval = STD_RESULT_SUCCESS;
    if (!src)
    {
        retval = STD_RESULT_INVALID_PARAM;
    }
    else
    {
        int     src_pos = 0;
        int     dst_pos = 0;
        int     src_max = (src_len && (*src_len >= 0)) ? *src_len : 0x7FFFFFFF;
        int     dst_max = (dst && dst_len && (*dst_len >= 0)) ? *dst_len : 0x7FFFFFFF;
        /* いずれかのバッファ長が終端に達するまで1文字ずつ処理 */
        while ((dst_pos < dst_max) && (src_pos < src_max))
        {
            char    dst_tmp[4];
            int     dst_count = 0;
            int     src_count = 1;
            u32     w = MI_LoadLE16(&src[src_pos]);
            /* 文字列長よりも終端の検出を優先する */
            if (!w)
            {
                break;
            }
            /* 外字 (私用領域) */
            else if ((w >= 0xE000) && (w < 0xF8FF))
            {
                const u32 sjis_page = 188UL;
                const u32 offset = w - 0xE000;
                u32     c1 = offset / sjis_page;
                u32     c2 = offset - c1 * sjis_page;
                dst_tmp[0] = (char)(c1 + 0xF0);
                dst_tmp[1] = (char)(c2 + ((c2 < 0x3F) ? 0x40 : 0x41));
                dst_count = 2;
            }
            else
            {
                /*
                 * 変換テーブルの範囲は
                 * [0000-0480), [2000-2680), [3000-3400), [4E00-9FA8), [F928-FFE6).
                 * 単純に, 0000で埋められた領域を大きい順に削除して詰めている.
                 */
				/* *INDENT-OFF* */
                static const int table[][2] =
                {
                    {0x0000,0x0480 - 0x0000,},
                    {0x2000,0x2680 - 0x2000,},
                    {0x3000,0x3400 - 0x3000,},
                    {0x4E00,0x9FA8 - 0x4E00,},
                    {0xF928,0xFFE6 - 0xF928,},
                };
				enum { table_max = sizeof(table) / (sizeof(int) * 2) };
				/* *INDENT-ON* */
                int     i;
                int     index = 0;
                for (i = 0; i < table_max; ++i)
                {
                    const int offset = (int)(w - table[i][0]);
                    /* 無効な範囲 */
                    if (offset < 0)
                    {
                        break;
                    }
                    /* 有効な範囲 */
                    else if (offset < table[i][1])
                    {
                        index += offset;
                        dst_tmp[0] = (char)STD_Unicode2SjisArray[index * 2 + 0];
                        if (dst_tmp[0])
                        {
                            dst_tmp[1] = (char)STD_Unicode2SjisArray[index * 2 + 1];
                            dst_count = dst_tmp[1] ? 2 : 1;
                        }
                        break;
                    }
                    /* それ以上の範囲 */
                    else
                    {
                        index += table[i][1];
                    }
                }
            }
            /* 変換できない文字が現れたらコールバックを呼び出す */
            if (dst_count == 0)
            {
                if (!callback)
                {
                    retval = STD_RESULT_CONVERSION_FAILED;
                }
                else
                {
                    src_count = src_max - src_pos;
                    dst_count = sizeof(dst_tmp) / sizeof(*dst_tmp);
                    retval = (*callback) (dst_tmp, &dst_count, &src[src_pos], &src_count);
                }
                if (retval != STD_RESULT_SUCCESS)
                {
                    break;
                }
            }
            /* 変換元または変換先が終端を超えるならここで終了 */
            if ((src_pos + src_count > src_max) || (dst_pos + dst_count > dst_max))
            {
                break;
            }
            /* バッファが有効ならば変換結果を格納 */
            if (dst)
            {
                int     i;
                for (i = 0; i < dst_count; ++i)
                {
                    MI_StoreLE8(&dst[dst_pos + i], (u8)dst_tmp[i]);
                }
            }
            src_pos += src_count;
            dst_pos += dst_count;
        }
        /* 変換文字数を返す */
        if (src_len)
        {
            *src_len = src_pos;
        }
        if (dst_len)
        {
            *dst_len = dst_pos;
        }
    }
    return retval;
}
#if !defined(STD_UNICODE_STATIC_IMPLEMENTATION)
#include <twl/ltdmain_end.h>
#endif // !defined(STD_UNICODE_STATIC_IMPLEMENTATION)


#endif // defined(SDK_ARM9) || !defined(SDK_NITRO)
