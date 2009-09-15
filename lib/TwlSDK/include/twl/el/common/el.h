/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     el.h

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
#ifndef TWL_COMMON_EL_H_
#define TWL_COMMON_EL_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*------------------------------------------------------
  typedef
 -----------------------------------------------------*/
typedef u32   ELDlld;
typedef void *(*ELAlloc)(size_t size);
typedef void  (*ELFree)(void* ptr);
typedef u32   (*ELReadImage)( u32 offset, void* buf, u32 size);


/*------------------------------------------------------
  EL_CalcEnoughBufferSizeforLink* で指定する定数
 -----------------------------------------------------*/
typedef enum ELLinkMode {
  EL_LINKMODE_ROUGH = 0,
  EL_LINKMODE_ONESHOT,
  EL_LINKMODE_RELATION
} ELLinkMode;

/*------------------------------------------------------
  エラーコード
 -----------------------------------------------------*/
#define EL_SUCCESS   0
#define EL_FAILED    1
#define EL_RELOCATED 0xF1    //for EL_ResolveAll only

/*------------------------------------------------------
  EL_GetResultCodeで取得する詳細なエラーコード
 -----------------------------------------------------*/
typedef enum ELResult {
  EL_RESULT_SUCCESS = 0,
  EL_RESULT_FAILURE = 1,
  EL_RESULT_INVALID_PARAMETER,
  EL_RESULT_INVALID_ELF,
  EL_RESULT_UNSUPPORTED_ELF,
  EL_RESULT_CANNOT_ACCESS_ELF, //ELFファイルの open/readエラー
  EL_RESULT_NO_MORE_RESOURCE   //malloc失敗
} ELResult;


/*------------------------------------------------------
  エクスポート用のアドレステーブル
 -----------------------------------------------------*/
typedef struct {
  void*      next;              /*次のアドレスエントリ*/
  char*      name;              /*文字列*/
  void*      adr;               /*アドレス*/
  u16        func_flag;         /*0:データ、1:関数*/
  u16        thumb_flag;        /*0:armコード、1:thumbコード*/
}ELAdrEntry;


/*---------------------------------------------------------------------------*
  Name:         EL_Init

  Description:  ダイナミックリンクシステムを初期化する

  Arguments:    alloc    ヒープ取得関数
                free     ヒープ開放関数

  Returns:      成功すれば 0, 失敗すれば -1
 *---------------------------------------------------------------------------*/
s32 EL_Init( ELAlloc alloc, ELFree free);


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
                            （処理時間と精度のトレードオフを選択するための定数）

  Returns:      成功すればバッファサイズ, 失敗すれば -1
 *---------------------------------------------------------------------------*/
s32 EL_CalcEnoughBufferSizeforLinkFile( const char* FilePath, const void* buf, ELLinkMode link_mode);
s32 EL_CalcEnoughBufferSizeforLink( ELReadImage readfunc, u32 len, const void* buf, ELLinkMode link_mode);
s32 EL_CalcEnoughBufferSizeforLinkImage( void* obj_image, u32 obj_len, const void* buf, ELLinkMode link_mode);


/*---------------------------------------------------------------------------*
  Name:         EL_Link*Ex

  Description:  与えられたバッファに収まるかをチェックしながら、
                動的オブジェクトを（ファイル/ユーザAPI/メモリから）再配置する

  Arguments:    FilePath    動的オブジェクトのパス名
                readfunc    アプリが用意したリード関数
                len         動的オブジェクトのサイズ
                obj_image   動的オブジェクトが存在するメモリ上のアドレス
                obj_len     動的オブジェクトのサイズ
                buf         再配置先となるバッファアドレス
                buf_size    再配置先となるバッファのサイズ

  Returns:      成功すれば動的モジュールのハンドル、失敗すれば 0
 *---------------------------------------------------------------------------*/
ELDlld EL_LinkFileEx( const char* FilePath, void* buf, u32 buf_size);
ELDlld EL_LinkEx( ELReadImage readfunc, u32 len, void* buf, u32 buf_size);
ELDlld EL_LinkImageEx( void* obj_image, u32 obj_len, void* buf, u32 buf_size);

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
ELDlld EL_LinkFile( const char* FilePath, void* buf);
ELDlld EL_Link( ELReadImage readfunc, u32 len, void* buf);
ELDlld EL_LinkImage( void* obj_image, u32 obj_len, void* buf);


/*---------------------------------------------------------------------------*
  Name:         EL_ResolveAll

  Description:  未解決のシンボルを解決する

  Arguments:    None.

  Returns:      成功すれば EL_RELOCATED, 失敗すれば EL_FAILED
 *---------------------------------------------------------------------------*/
u16 EL_ResolveAll( void);


/*---------------------------------------------------------------------------*
  Name:         EL_Export

  Description:  DLLシステムにシンボル情報を登録する

  Arguments:    None.

  Returns:      成功すれば TRUE
 *---------------------------------------------------------------------------*/
BOOL EL_Export( ELAdrEntry* AdrEnt);


/*---------------------------------------------------------------------------*
  Name:         EL_AddStaticSym

  Description:  DLLシステムに静的モジュールのシンボル情報を登録する
                （ELライブラリ内でWEAKシンボルとして定義されており、
                  makelstが生成したファイルの定義で上書きされる）
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void EL_AddStaticSym( void);


/*---------------------------------------------------------------------------*
  Name:         EL_GetGlobalAdr

  Description:  動的モジュールからシンボルを探してアドレスを返す

  Arguments:    my_dlld     検索対象とする動的モジュールのハンドル
                ent_name    シンボル名

  Returns:      成功すればシンボルのアドレス、失敗すれば 0
 *---------------------------------------------------------------------------*/
void* EL_GetGlobalAdr( ELDlld my_dlld, const char* ent_name);


/*---------------------------------------------------------------------------*
  Name:         EL_Unlink

  Description:  動的モジュールをアンリンクする

  Arguments:    my_dlld     アンリンクする動的モジュールのハンドル

  Returns:      成功すれば EL_SUCCESS, 失敗すれば EL_FAILED
 *---------------------------------------------------------------------------*/
u16 EL_Unlink( ELDlld my_dlld);


/*---------------------------------------------------------------------------*
  Name:         EL_IsResolved

  Description:  動的モジュールの外部参照が全て解決済みかどうかを調べる

  Arguments:    my_dlld     調べる対象とする動的モジュールのハンドル

  Returns:      未解決な外部参照が残っていなければ TRUE
 *---------------------------------------------------------------------------*/
BOOL EL_IsResolved( ELDlld my_dlld);


/*---------------------------------------------------------------------------*
  Name:         EL_GetLibSize

  Description:  リンクされた動的モジュールのサイズを返す
                動的モジュールにまだ未解決な外部参照が残っていてもその時点での
                サイズを返します。
                EL_ResolveAllがEL_RELOCATEDを返すのを確認してからであれば、
                再配置が完了した後の最終的なサイズを得ることができます。

  Arguments:    my_dlld     サイズを調べる対象とする動的モジュールのハンドル

  Returns:      成功すればサイズ、失敗すれば 0
 *---------------------------------------------------------------------------*/
u32 EL_GetLibSize( ELDlld my_dlld);


/*---------------------------------------------------------------------------*
  Name:         EL_GetResultCode

  Description:  最後に行った処理の詳細な結果を返す

  Arguments:    None.

  Returns:      ELResult型のエラーコード
 *---------------------------------------------------------------------------*/
ELResult EL_GetResultCode( void);


#ifdef __cplusplus
}    /* extern "C" */
#endif

#endif    /*TWL_COMMON_EL_H_*/
