/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     loader_subset.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-10#$
  $Rev: 9620 $
  $Author: shirait $
 *---------------------------------------------------------------------------*/

#ifndef _LOADER_SUBSET_H_
#define _LOADER_SUBSET_H_

#include "elf.h"
#include "elf_loader.h"


/*------------------------------------------------------
  ベニヤをバッファにコピーする
    start : 呼び出し元アドレス
    data : 飛び先アドレス
 -----------------------------------------------------*/
void* ELi_CopyVeneerToBuffer( ELDesc* elElfDesc, ELObject* MYObject, u32 start, u32 data, s32 threshold);


/*------------------------------------------------------
  ベニヤをバッファにコピーする
    start : 呼び出し元
    data : 飛び先
    threshold : この範囲内に既にベニヤがあれば使いまわす
 -----------------------------------------------------*/
void* ELi_CopyV4tVeneerToBuffer( ELDesc* elElfDesc, ELObject* MYObject, u32 start, u32 data, s32 threshold);


/*------------------------------------------------------
  セグメントをバッファにコピーする
 -----------------------------------------------------*/
void* ELi_CopySegmentToBuffer( ELDesc* elElfDesc, ELObject* MYObject, Elf32_Phdr* Phdr);


/*------------------------------------------------------
  セクションをバッファにコピーする
 -----------------------------------------------------*/
void* ELi_CopySectionToBuffer( ELDesc* elElfDesc, ELObject* MYObject, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  セクションをバッファに確保する（コピーしない）
 -----------------------------------------------------*/
void* ELi_AllocSectionToBuffer( ELDesc* elElfDesc, ELObject* MYObject, Elf32_Shdr* Shdr);
    

/*------------------------------------------------------
  指定インデックスのプログラムヘッダをバッファに取得する
 -----------------------------------------------------*/
void ELi_GetPhdr( ELDesc* elElfDesc, u32 index, Elf32_Phdr* Phdr);


/*------------------------------------------------------
  指定インデックスのセクションヘッダをバッファに取得する
 -----------------------------------------------------*/
void ELi_GetShdr( ELDesc* elElfDesc, u32 index, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  指定インデックスのセクションのエントリをバッファに取得する
 -----------------------------------------------------*/
void ELi_GetSent( ELDesc* elElfDesc, u32 index, void* entry_buf, u32 offset, u32 size);


/*------------------------------------------------------
  指定セクションヘッダの指定インデックスのエントリをバッファに取得する
  （エントリサイズが固定のセクションのみ）
 -----------------------------------------------------*/
void ELi_GetEntry( ELDesc* elElfDesc, Elf32_Shdr* Shdr, u32 index, void* entry_buf);


/*------------------------------------------------------
  STRセクションヘッダの指定インデックスの文字列を取得する
 -----------------------------------------------------*/
void ELi_GetStrAdr( ELDesc* elElfDesc, u32 strsh_index, u32 ent_index, char* str, u32 len);


/*------------------------------------------------------
  シンボルを再定義する
 -----------------------------------------------------*/
BOOL ELi_RelocateSym( ELDesc* elElfDesc, ELObject* MYObject, u32 relsh_index);

/*------------------------------------------------------
  グローバルシンボルをアドレステーブルに入れる
 -----------------------------------------------------*/
BOOL ELi_GoPublicGlobalSym( ELDesc* elElfDesc, ELObject* MYObject, u32 symtblsh_index);

/*------------------------------------------------------
  ELi_RelocateSymやELi_GoPublicGlobalSymの中で作成したシンボルリストを
  開放する（どちらも呼び終わったら最後にこのAPIを呼んでください）
 -----------------------------------------------------*/
void ELi_FreeSymList( ELDesc* elElfDesc);

/*------------------------------------------------------
  未解決情報をもとにシンボルを解決する
 -----------------------------------------------------*/
BOOL ELi_DoRelocate( ELDesc* elElfDesc, ELObject* MYObject, ELImportEntry* UnresolvedInfo);


/*------------------------------------------------------
  リストから指定インデックスのELSymExを取り出す
 -----------------------------------------------------*/
//ELSymEx* ELi_GetSymExfromList( ELSymEx* SymExStart, u32 index);


/*------------------------------------------------------
  リストから指定インデックスのELShdrExを取り出す
 -----------------------------------------------------*/
ELShdrEx* ELi_GetShdrExfromList( ELShdrEx* ShdrExStart, u32 index);


/*------------------------------------------------------
  指定インデックスのセクションがデバッグ情報かどうか判定する
 -----------------------------------------------------*/
BOOL ELi_ShdrIsDebug( ELDesc* elElfDesc, u32 index);


/*------------------------------------------------------
  elElfDescのSymExテーブルを調べ、指定インデックスの
　指定オフセットにあるコードがARMかTHUMBかを判定する
 -----------------------------------------------------*/
u32 ELi_CodeIsThumb( ELDesc* elElfDesc, u16 sh_index, u32 offset);


/*---------------------------------------------------------
 インポート情報エントリを初期化する
 --------------------------------------------------------*/
static void ELi_InitImport( ELImportEntry* ImportInfo);


/*------------------------------------------------------
  インポート情報テーブルからエントリを抜き出す（ImpEntは削除しない！）
 -----------------------------------------------------*/
BOOL ELi_ExtractImportEntry( ELImportEntry** StartEnt, ELImportEntry* ImpEnt);


/*---------------------------------------------------------
 インポート情報テーブルにエントリを追加する
 --------------------------------------------------------*/
void ELi_AddImportEntry( ELImportEntry** ELUnrEntStart, ELImportEntry* UnrEnt);


/*------------------------------------------------------
  インポート情報テーブルを全部解放する
 -----------------------------------------------------*/
void ELi_FreeImportTbl( ELImportEntry** ELImpEntStart);


/*------------------------------------------------------
  ベニヤテーブルを解放する
 -----------------------------------------------------*/
void* ELi_FreeVenTbl( ELDesc* elElfDesc, ELObject* MYObject);



#endif /*_LOADER_SUBSET_H_*/
