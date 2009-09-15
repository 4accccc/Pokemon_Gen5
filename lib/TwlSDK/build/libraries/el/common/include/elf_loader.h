/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     elf_loader.h

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
#ifndef _ELF_LOADER_H_
#define _ELF_LOADER_H_




#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
//#include <stdio.h>
#include "elf.h"
#include <twl/el.h>



#ifdef __cplusplus
extern "C" {
#endif


/*------------------------------------------------------
  セクションヘッダ拡張(ロードアドレス等に対応)
 -----------------------------------------------------*/
typedef struct {
  void*         next;
  u16           index;
  u16           debug_flag;    /*0:デバッグ情報でない、1:デバッグ情報*/
  u32           loaded_adr;
  u32           alloc_adr;
  u32           loaded_size;
  Elf32_Shdr    Shdr;
}ELShdrEx;


/*------------------------------------------------------
  シンボルエントリ拡張(ロードアドレス等に対応)
 -----------------------------------------------------*/
typedef struct {
  void*      next;
  u16        debug_flag;       /*0:デバッグ情報でない、1:デバッグ情報*/
  u16        thumb_flag;
  u32        relocation_val;
  Elf32_Sym  Sym;
}ELSymEx;


/*------------------------------------------------------
  ベニヤのリンクリスト(ELi_DoRelocateで使用)
 -----------------------------------------------------*/
typedef struct {
  void* next;
  u32   adr;     /* ベニヤコードの先頭アドレス */
  u32   data;    /* ベニヤのリテラルプールに格納されている飛び先のアドレス値 */
}ELVeneer;


/*------------------------------------------------------
  インポート用の再配置情報テーブル

  後からアドレステーブルを参照すれば次のように解決する。
  S_ = AdrEntry.adr;
  T_ = (u32)(AdrEntry.thumb_flag);
 -----------------------------------------------------*/
typedef struct {
  void*       next;             /*次のエントリ*/
  char*       sym_str;          /*未解決の外部参照シンボル名*/
  u32         r_type;           /*リロケーションタイプ（ELF32_R_TYPE( Rela.r_info)）*/
  u32         S_;               /*未解決の外部参照シンボルアドレス*/
  s32         A_;               /*解決済み*/
  u32         P_;               /*解決済み*/
  u32         T_;               /*未解決の外部参照シンボルのARM/Thumbフラグ*/
  u32         sh_type;          /*SHT_REL or SHT_RELA*/
  struct ELObject*   Dlld;      /*解決時にインポートしたエントリを持つオブジェクト。未解決時はNULL*/
}ELImportEntry;



/* ELDesc の process値 */
typedef enum ELProcess {
  EL_PROC_NOTHING      = 0,
  EL_PROC_INITIALIZED = 0x5A,
  EL_PROC_COPIED      = 0xF0,
  EL_PROC_RELOCATED
} ELProcess;


/*------------------------------------------------------
  ELFオブジェクト個別の管理
 -----------------------------------------------------*/
typedef struct {
  void*          next;                   /* 次のオブジェクトエントリ */
  void*          lib_start;              /* ライブラリとして再配置されたアドレス */
  u32            lib_size;               /* ライブラリになった後のサイズ */
  u32            buf_limit_addr;         /* 再配置用に与えられたバッファの最終アドレス+1 */
  void*          buf_current;            /* バッファポインタ */
  ELAdrEntry*    ExportAdrEnt;           /* エクスポート情報 */
  ELAdrEntry*    HiddenAdrEnt;           /* アンリンク時のエクスポート情報退避場所 */
  ELImportEntry* ResolvedImportAdrEnt;   /* 解決したインポート情報 */
  ELImportEntry* UnresolvedImportAdrEnt; /* 解決していないインポート情報 */
  ELVeneer*      ELVenEntStart;          /* ベニヤのリンクリスト */
  ELVeneer*      ELV4tVenEntStart;       /* v4tベニヤのリンクリスト */
  u32            stat;                   /* 状況 */
  u32            file_id;                /* ROM経由でDLLを配置した場合のファイルID */
  u32            process;                /* ELProcess型をキャストして格納 */
  u32            result;                 /* ELResult型をキャストして格納 */
}ELObject;


/*------------------------------------------------------
  ELFオブジェクト全体の管理
 -----------------------------------------------------*/
typedef BOOL (*ELi_ReadFunc)( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
typedef struct {
  void*         ar_head;        /* ARまたはELFファイルの先頭アドレス */
  void*         elf_offset;     /* ELFオブジェクトの先頭へのオフセット */

  u16           reserve;
  u16           shentsize;      /* 1セクションヘッダのサイズ */
  u32           process;        /* ELProcess型をキャストして格納 */
  u32           result;         /* ELResult型をキャストして格納 */

  ELShdrEx*     ShdrEx;         /* ShdrExリストの先頭 */

  Elf32_Ehdr    CurrentEhdr;    /* ELFヘッダ */

  Elf32_Rel     Rel;            /* 再配置エントリ */
  Elf32_Rela    Rela;
  Elf32_Sym     Sym;            /* シンボルエントリ */
  Elf32_Shdr    SymShdr;

  ELSymEx*      SymEx;          /* SymExリストの先頭（非デバッグシンボルのみ繋がる） */
  ELSymEx**     SymExTbl;       /* SymExアドレスのテーブル（全シンボル数ぶん）*/
  u32           SymExTarget;    /* SymExリストを構築したシンボルセクションのセクション番号 */

  ELi_ReadFunc  i_elReadStub;   /* リードスタブ関数 */
  void*         FileStruct;     /* ファイル構造体 */
    
  u32           entry_adr;      /* エントリアドレス */
  
  ELObject*     ELObjectStart;  /* オブジェクトのリンクリスト */
  ELObject*     ELStaticObj;    /* リンクリストに繋がっているStatic用構造体へのポインタ */
}ELDesc;


/*---------------------------------------------------------
 ELFオブジェクトのサイズを求める
 --------------------------------------------------------*/
u32 EL_GetElfSize( const void* buf);

/*---------------------------------------------------------
 リンクされたライブラリのサイズを求める
 --------------------------------------------------------*/
u32 EL_GetLibSize( ELDlld my_dlld);


/*------------------------------------------------------
  ダイナミックリンクシステムを初期化する
 -----------------------------------------------------*/
#if 0
//#ifndef SDK_TWL
void ELi_Init( void);
#else
void ELi_Init( ELAlloc alloc, ELFree free);
void* ELi_Malloc( ELDesc* elElfDesc, ELObject* MYObject, size_t size);
#endif

/*------------------------------------------------------
  ELDesc構造体を初期化する
 -----------------------------------------------------*/
BOOL ELi_InitDesc( ELDesc* elElfDesc);

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをファイルからバッファに再配置する
 -----------------------------------------------------*/
ELDlld EL_LoadLibraryfromFile( ELDesc* elElfDesc, const char* FilePath, void* buf, u32 buf_size);

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをユーザのリードAPIを通して再配置する
 -----------------------------------------------------*/
ELDlld EL_LoadLibrary( ELDesc* elElfDesc, ELReadImage readfunc, u32 len, void* buf, u32 buf_size);

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをメモリからバッファに再配置する
 -----------------------------------------------------*/
ELDlld EL_LoadLibraryfromMem( ELDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf, u32 buf_size);

/*------------------------------------------------------
  アドレステーブルを使って未解決のシンボルを解決する
 -----------------------------------------------------*/
ELProcess ELi_ResolveAllLibrary( ELDesc* elElfDesc);


/*------------------------------------------------------
  アプリケーションからアドレステーブルにエントリを追加する
 -----------------------------------------------------*/
BOOL ELi_Export( ELDesc* elElfDesc, ELAdrEntry* AdrEnt);

/*------------------------------------------------------
  アドレステーブルにスタティック側のエントリを追加する
  （ELライブラリ内でWEAKシンボルとして定義されており、
    makelstが生成したファイルの定義で上書きされる）
 -----------------------------------------------------*/
void EL_AddStaticSym( void);


/*------------------------------------------------------
  アドレステーブルから指定文字列に該当するアドレスを返す
 -----------------------------------------------------*/
void* ELi_GetGlobalAdr( ELDesc* elElfDesc, ELDlld my_dlld, const char* ent_name);


/*------------------------------------------------------
  オブジェクトをアンリンクする
 -----------------------------------------------------*/
BOOL ELi_Unlink( ELDesc* elElfDesc, ELDlld my_dlld);


/*------------------------------------------------------
  オブジェクトに未解決な外部参照が残っていないかを調べる
 -----------------------------------------------------*/
BOOL EL_IsResolved( ELDlld my_dlld);


/*------------------------------------------------------
  エラーコード/プロセスコードのセット
 -----------------------------------------------------*/
void ELi_SetResultCode( ELDesc* elElfDesc, ELObject* MYObject, ELResult result);
void ELi_SetProcCode( ELDesc* elElfDesc, ELObject* MYObject, ELProcess process);


/*------------------------------------------------------
  ヒープを確保する
 -----------------------------------------------------*/
void* ELi_Malloc( ELDesc* elElfDesc, ELObject* MYObject, size_t size);


#ifdef __cplusplus
}    /* extern "C" */
#endif


#endif    /*_ELF_LOADER_H_*/
