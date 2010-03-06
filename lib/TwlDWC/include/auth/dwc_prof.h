/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_PROF_H
#define DWC_PROF_H

#ifdef __cplusplus
extern "C"
{
#endif


    /* -------------------------------------------------------------------------
            define
       ------------------------------------------------------------------------- */

#define DWC_PROF_RESULT_STRING_LENGTH 51
#define DWC_PROF_MAX_WORDSNUM 50    // RVLDWCとの互換性のため



    /* -------------------------------------------------------------------------
            enum
       ------------------------------------------------------------------------- */

    /**
     * 不正ネームチェック中のステート
     *
     */
    typedef enum
    {
        DWC_PROF_STATE_NOT_INITIALIZED	= 0, ///< 未初期化
        DWC_PROF_STATE_OPERATING,		     ///< HTTP通信中
        DWC_PROF_STATE_SUCCESS,				 ///< 名前チェック成功
        DWC_PROF_STATE_FAIL					 ///< 名前チェック失敗
    } DWCProfState;

    /**
     * 不正文字列チェックのリージョンを示す列挙型
     *
     */
    typedef enum
    {
        DWC_PROF_REGION_SELF  = 0x00,   ///< 製品コードに従う
        DWC_PROF_REGION_JP    = 0x01,   ///< 日本
        DWC_PROF_REGION_US    = 0x02,   ///< 北米
        DWC_PROF_REGION_EU    = 0x04,   ///< 欧州
        DWC_PROF_REGION_KR    = 0x08,   ///< 韓国
        DWC_PROF_REGION_CN    = 0x10,   ///< 中国
        DWC_PROF_REGION_ALL   = 0x80    ///< 全リージョン
    } DWCProfRegion;



    /* -------------------------------------------------------------------------
            function - external
       ------------------------------------------------------------------------- */

    BOOL DWC_CheckProfanityAsync(const u16 **words, int wordsnum, const char *reserved, int timeout, char *result, int *badwordsnum);
    BOOL DWC_CheckProfanityExAsync(const u16 **words, int wordsnum, const char *reserved, int timeout, char *result, int *badwordsnum, DWCProfRegion region);
    DWCProfState DWC_CheckProfanityProcess(void);



#ifdef __cplusplus
}
#endif

#endif
