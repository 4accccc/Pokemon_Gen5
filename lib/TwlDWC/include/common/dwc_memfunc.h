/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_MEMFUNC_H_
#define DWC_MEMFUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif


    /* --------------------------------------------------------------------
      enums
      ----------------------------------------------------------------------*/

    typedef enum
    {
        DWC_ALLOCTYPE_AUTH,
        DWC_ALLOCTYPE_AC,
        DWC_ALLOCTYPE_BM,
        DWC_ALLOCTYPE_UTIL,
        DWC_ALLOCTYPE_BASE,
        DWC_ALLOCTYPE_LANMATCH,
        DWC_ALLOCTYPE_GHTTP,
        DWC_ALLOCTYPE_RANKING,
        DWC_ALLOCTYPE_ENC,
        DWC_ALLOCTYPE_GS,
        DWC_ALLOCTYPE_ND,
        DWC_ALLOCTYPE_TAC,
        DWC_ALLOCTYPE_OPTION_CF,
        DWC_ALLOCTYPE_NHTTP,
        DWC_ALLOCTYPE_MAIL,
        DWC_ALLOCTYPE_LAST,

        DWC_ALLOCTYPE_NUM = DWC_ALLOCTYPE_LAST
    } DWCAllocType;



    /* --------------------------------------------------------------------
      typedefs
      ----------------------------------------------------------------------*/

    /**
     * DWCライブラリ全体で使用するメモリ確保関数
     *
     * DWCライブラリから使用されるメモリ確保関数です。スレッド排他処理を行う必要があります。
     * TWLモードで動作させる場合、メインメモリ上の拡張アリーナから領域を確保する関数は指定できません。
     *
     * Param:    align   バイトアライメントされたsizeバイト分のバッファへのポ
     *                  インタを返してください。
     * Param:    name    DWCライブラリ開発用の情報が含まれていますので、無視
     *                  してください。
     *
     * Return value:  メモリ確保に成功した場合、alignバイトアライメントされたsizeバイト分のバッファへのポインタ
     * メモリ確保に失敗した場合、NULL
     *
     * See also:  DWCFreeEx
     * 
     */
    typedef void* (*DWCAllocEx)( DWCAllocType name, u32   size, int align );

    /**
     * DWCライブラリ全体で使用するメモリ開放関数
     *
     * DWCライブラリから使用されるメモリ解放関数です。スレッド排他処理を行う必要があります。
     *
     * DWCAllocExで確保されたバッファを開放します。NULLポインタが渡されたときには何もせずに処理を終了してください。
     *
     * Param:    ptr     メモリ確保関数で確保されたバッファのポインタ。
     * Param:    name    DWCライブラリ開発用の情報が含まれていますので、無視
     *                  してください。
     * Param:    size    DWCライブラリ開発用の情報で正確な値が格納されていな
     *                  い場合がありますので、無視してください。
     *
     * See also:  DWCAllocEx
     *
     */
    typedef void  (*DWCFreeEx )( DWCAllocType name, void* ptr,  u32 size  );


    /* --------------------------------------------------------------------
      functions
      ----------------------------------------------------------------------*/
    void* DWC_Alloc     ( DWCAllocType name, u32 size );
    void* DWC_AllocEx   ( DWCAllocType name, u32 size, int align );
    void  DWC_Free      ( DWCAllocType name, void* ptr, u32 size );
    void* DWC_Realloc   ( DWCAllocType name, void* ptr, u32 oldsize, u32 newsize );
    void* DWC_ReallocEx ( DWCAllocType name, void* ptr, u32 oldsize, u32 newsize, int align );

    void* DWCi_GsMalloc  (size_t size);
    void* DWCi_GsRealloc (void* ptr, size_t size);
    void  DWCi_GsFree    (void* ptr);
    void* DWCi_GsMemalign(size_t boundary, size_t size);



#ifdef __cplusplus
}
#endif

#endif // DWC_MEMFUNC_H_
