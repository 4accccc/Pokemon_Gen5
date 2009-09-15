/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     el.c

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

#include "el_config.h"

#include <twl/el.h>


#include "elf.h"
#include "arch.h"
#include "elf_loader.h"
#include "loader_subset.h"


/*------------------------------------------------------
  extern 変数
 -----------------------------------------------------*/
extern ELAlloc     i_elAlloc;
extern ELFree      i_elFree;


/*------------------------------------------------------
  extern 関数
 -----------------------------------------------------*/
extern BOOL elRemoveObjEntry( ELObject** StartEnt, ELObject* ObjEnt);


/*------------------------------------------------------
  グローバル変数
 -----------------------------------------------------*/
ELDesc* i_eldesc     = NULL;    // for link
ELDesc* i_eldesc_sim = NULL;    // for simulation link


/*---------------------------------------------------------------------------*
  Name:         EL_Init

  Description:  ダイナミックリンクシステムを初期化する

  Arguments:    alloc    ヒープ取得関数
                free     ヒープ開放関数

  Returns:      成功すれば 0, 失敗すれば -1
 *---------------------------------------------------------------------------*/
s32 EL_Init( ELAlloc alloc, ELFree free)
{
    if( i_eldesc != NULL) {
        return( -1);    //Init済み
    }
  
    ELi_Init( alloc, free);

    i_eldesc = ELi_Malloc( NULL, NULL, ((sizeof( ELDesc))*2)); //for link and simulation-link
    if( i_eldesc == NULL) {
        return( -1);
    }

    (void)ELi_InitDesc( i_eldesc);
    i_eldesc_sim = &(i_eldesc[1]);
    (void)ELi_InitDesc( i_eldesc_sim);

    return( 0);
}

#if 1
/*---------------------------------------------------------------------------*
  Name:         EL_CalcEnoughBufferSizeforLink*

  Description:  動的リンクをテストして再配置するのに十分なバッファサイズを返す。
                動的オブジェクトが外部参照している場合、やや大き目のサイズを
                返す場合があります

  Arguments:    FilePath    動的オブジェクトのパス名
                buf         再配置先となるバッファアドレス（実際に書き込みは行いません）
                readfunc    アプリが用意したリード関数
                len         動的オブジェクトのサイズ
                obj_image   動的オブジェクトが存在するメモリ上のアドレス
                obj_len     動的オブジェクトのサイズ
                link_mode   動的リンクをテストする際のモード

  Returns:      成功すればバッファサイズ, 失敗すれば -1
 *---------------------------------------------------------------------------*/
s32 EL_CalcEnoughBufferSizeforLinkFile( const char* FilePath, const void* buf, ELLinkMode link_mode)
{
    ELDlld dlld;
    u32    size;

    if( link_mode != EL_LINKMODE_ONESHOT) {
        return( -1);
    }
    dlld = EL_LoadLibraryfromFile( i_eldesc_sim, FilePath, (void*)buf, 0xFFFFFFFF);
    if( dlld) {
        if( ELi_ResolveAllLibrary( i_eldesc_sim) != EL_PROC_RELOCATED) {
            return( -1);
        }
        size = ((ELObject*)dlld)->lib_size; //ELi_ResolveAllLibrary後なのでveneer込みのサイズ

        (void)ELi_Unlink( i_eldesc, dlld);
        (void)elRemoveObjEntry( &(i_eldesc_sim->ELObjectStart), (ELObject*)dlld);
        return( (s32)size);
    }
    return( -1);
}

/*------------------------------------------------------*/
s32 EL_CalcEnoughBufferSizeforLink( ELReadImage readfunc, u32 len, const void* buf, ELLinkMode link_mode)
{
    ELDlld dlld;
    u32    size;
  
    if( link_mode != EL_LINKMODE_ONESHOT) {
        return( -1);
    }
    dlld = EL_LoadLibrary( i_eldesc_sim, readfunc, len, (void*)buf, 0xFFFFFFFF);
    if( dlld) {
        if( ELi_ResolveAllLibrary( i_eldesc_sim) != EL_PROC_RELOCATED) {
            return( -1);
        }
        size = ((ELObject*)dlld)->lib_size; //ELi_ResolveAllLibrary後なのでveneer込みのサイズ

        (void)ELi_Unlink( i_eldesc, dlld);
        (void)elRemoveObjEntry( &(i_eldesc_sim->ELObjectStart), (ELObject*)dlld);
        return( (s32)size);
    }
    return( -1);
}

/*------------------------------------------------------*/
s32 EL_CalcEnoughBufferSizeforLinkImage( void* obj_image, u32 obj_len, const void* buf, ELLinkMode link_mode)
{
    ELDlld dlld;
    u32    size;
  
    if( link_mode != EL_LINKMODE_ONESHOT) {
        return( -1);
    }
    dlld = EL_LoadLibraryfromMem( i_eldesc_sim, obj_image, obj_len, (void*)buf, 0xFFFFFFFF);
    if( dlld) {
        if( ELi_ResolveAllLibrary( i_eldesc_sim) != EL_PROC_RELOCATED) {
            return( -1);
        }
        size = ((ELObject*)dlld)->lib_size; //ELi_ResolveAllLibrary後なのでveneer込みのサイズ

        (void)ELi_Unlink( i_eldesc, dlld);
        (void)elRemoveObjEntry( &(i_eldesc_sim->ELObjectStart), (ELObject*)dlld);
        return( (s32)size);
    }
    return( -1);
}
#endif


/*---------------------------------------------------------------------------*
  Name:         EL_Link*

  Description:  動的オブジェクトを（ファイル/ユーザAPI/メモリから）再配置する

  Arguments:    FilePath    動的オブジェクトのパス名
                readfunc    アプリが用意したリード関数
                len         動的オブジェクトのサイズ
                obj_image   動的オブジェクトが存在するメモリ上のアドレス
                obj_len     動的オブジェクトのサイズ
                buf         再配置先となるバッファアドレス

  Returns:      成功すれば動的モジュールのハンドル、失敗すれば 0
 *---------------------------------------------------------------------------*/
ELDlld EL_LinkFileEx( const char* FilePath, void* buf, u32 buf_size)
{
    return( EL_LoadLibraryfromFile( i_eldesc, FilePath, buf, buf_size));
}

/*------------------------------------------------------*/
ELDlld EL_LinkEx( ELReadImage readfunc, u32 len, void* buf, u32 buf_size)
{
    return( EL_LoadLibrary( i_eldesc, readfunc, len, buf, buf_size));
}

/*------------------------------------------------------*/
ELDlld EL_LinkImageEx( void* obj_image, u32 obj_len, void* buf, u32 buf_size)
{
    return( EL_LoadLibraryfromMem( i_eldesc, obj_image, obj_len, buf, buf_size));
}

/*---------------------------------------------------------------------------*
  Name:         EL_Link*

  Description:  動的オブジェクトを（ファイル/ユーザAPI/メモリから）再配置する

  Arguments:    FilePath    動的オブジェクトのパス名
                readfunc    アプリが用意したリード関数
                len         動的オブジェクトのサイズ
                obj_image   動的オブジェクトが存在するメモリ上のアドレス
                obj_len     動的オブジェクトのサイズ
                buf         再配置先となるバッファアドレス

  Returns:      成功すれば動的モジュールのハンドル、失敗すれば 0
 *---------------------------------------------------------------------------*/
ELDlld EL_LinkFile( const char* FilePath, void* buf)
{
    return( EL_LoadLibraryfromFile( i_eldesc, FilePath, buf, 0xFFFFFFFF));
}

/*------------------------------------------------------*/
ELDlld EL_Link( ELReadImage readfunc, u32 len, void* buf)
{
    return( EL_LoadLibrary( i_eldesc, readfunc, len, buf, 0xFFFFFFFF));
}

/*------------------------------------------------------*/
ELDlld EL_LinkImage( void* obj_image, u32 obj_len, void* buf)
{
    return( EL_LoadLibraryfromMem( i_eldesc, obj_image, obj_len, buf, 0xFFFFFFFF));
}


/*---------------------------------------------------------------------------*
  Name:         EL_ResolveAll

  Description:  未解決のシンボルを解決する

  Arguments:    None.

  Returns:      成功すれば EL_RELOCATED, 失敗すれば EL_FAILED
 *---------------------------------------------------------------------------*/
u16 EL_ResolveAll( void)
{
    if( ELi_ResolveAllLibrary( i_eldesc) == EL_PROC_RELOCATED) {
        return( EL_RELOCATED);
    }
    return( EL_FAILED);
}


/*---------------------------------------------------------------------------*
  Name:         EL_Export

  Description:  DLLシステムにシンボル情報を登録する

  Arguments:    None.

  Returns:      成功すれば TRUE
 *---------------------------------------------------------------------------*/
BOOL EL_Export( ELAdrEntry* AdrEnt)
{
    /* (TODO)i_eldesc_simに追加されないのでsimulationには反映されない */
    return ELi_Export( i_eldesc, AdrEnt);
}


/*---------------------------------------------------------------------------*
  Name:         EL_GetGlobalAdr

  Description:  動的モジュールからシンボルを探してアドレスを返す

  Arguments:    my_dlld     検索対象とする動的モジュールのハンドル
                ent_name    シンボル名

  Returns:      成功すればシンボルのアドレス、失敗すれば 0
 *---------------------------------------------------------------------------*/
void* EL_GetGlobalAdr( ELDlld my_dlld, const char* ent_name)
{
    return( ELi_GetGlobalAdr( i_eldesc, my_dlld, ent_name));
}


/*---------------------------------------------------------------------------*
  Name:         EL_Unlink

  Description:  動的モジュールをアンリンクする

  Arguments:    my_dlld     アンリンクする動的モジュールのハンドル

  Returns:      成功すれば EL_SUCCESS, 失敗すれば EL_FAILED
 *---------------------------------------------------------------------------*/
u16 EL_Unlink( ELDlld my_dlld)
{
    if( ELi_Unlink( i_eldesc, my_dlld) == TRUE) {
        /* 該当オブジェクト構造体を破棄するので ReLink不可 */
        (void)elRemoveObjEntry( &(i_eldesc->ELObjectStart), (ELObject*)my_dlld); // 失敗でも問題なし
        return( EL_SUCCESS);
    }
    return( EL_FAILED);
}


/*---------------------------------------------------------------------------*
  Name:         EL_GetResultCode

  Description:  最後に行った処理の詳細な結果を返す

  Arguments:    None.

  Returns:      ELResult型のエラーコード
 *---------------------------------------------------------------------------*/
ELResult EL_GetResultCode( void)
{
    if( i_eldesc == NULL) {
        return( EL_RESULT_FAILURE);
    }
    return( (ELResult)(i_eldesc->result));
}

#if 0
ELResult EL_GetResultCode( ELDlld my_dlld)
{
    ELObject*      MYObject;
  
    if( my_dlld == 0) {
        return( EL_RESULT_INVALID_PARAMETER);
    }
  
    // 結果を参照するオブジェクト
    MYObject = (ELObject*)my_dlld;

    return( MYObject->result);
}
#endif
