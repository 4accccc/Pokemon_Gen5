/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     loader_subset.c

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
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "loader_subset.h"
#include <istdbglibpriv.h>


/*------------------------------------------------------
  extern変数
 -----------------------------------------------------*/
//#ifndef SDK_TWL
//#else
extern ELAlloc   i_elAlloc;
extern ELFree    i_elFree;
//#endif
extern ELDesc* i_eldesc_sim;


/*------------------------------------------------------
  extern関数
 -----------------------------------------------------*/
extern BOOL        elRemoveAdrEntry( ELAdrEntry* AdrEnt);
extern void        elAddAdrEntry( ELAdrEntry** ELAdrEntStart, ELAdrEntry* AdrEnt);
extern ELAdrEntry* elGetAdrEntry( ELDesc* elElfDesc, const char* ent_name, ELObject** ExpObjEnt);


/*------------------------------------------------------
  static関数
 -----------------------------------------------------*/
static u32 el_veneer[3] = { //ARMコード
    0xE59FC000,    //(LDR r12,[PC])
    0xE12FFF1C,    //(BX  r12)
    0x00000000,    //(data)
};

static u16 el_veneer_t[10] = { //v4t用Thumbコード
    0xB580,  //(PUSH {R7,LR})
    0x46FE,  //(NOP)
    0x46FE,  //(MOV LR, PC)
    0x2707,  //(MOV R7, 7)
    0x44BE,  //(ADD LR,R7)
    0x4F01,  //(LDR R7,[PC+4])
    0x4738,  //(BX R7) ---> ARM routine
    0xBD80,  //(POP {R7,PC} <---LR(v4tでは無条件でThumb分岐)
    0x0000,  //(data) ARM routine address
    0x0000,  //(data)
};

static BOOL ELi_CheckBufRest( ELDesc* elElfDesc, ELObject* MYObject, void* start, u32 size);

static BOOL     ELi_BuildSymList( ELDesc* elElfDesc, u32 symsh_index);
static ELSymEx* ELi_GetSymExfromList( ELSymEx* SymExStart, u32 index);
static ELSymEx* ELi_GetSymExfromTbl( ELSymEx** SymExTbl, u32 index);
static void     ELi_InitImport( ELImportEntry* ImportInfo);

//インポート情報テーブルからエントリを抜き出す（ImpEntは削除しない！）
BOOL ELi_ExtractImportEntry( ELImportEntry** StartEnt, ELImportEntry* ImpEnt);
void ELi_AddImportEntry( ELImportEntry** ELUnrEntStart, ELImportEntry* UnrEnt);

static void      ELi_AddVeneerEntry( ELVeneer** start, ELVeneer* VenEnt);
static BOOL      ELi_RemoveVenEntry( ELVeneer** start, ELVeneer* VenEnt);
static ELVeneer* ELi_GetVenEntry( ELVeneer** start, u32 data);
static BOOL      ELi_IsFar( u32 P, u32 S, s32 threshold);


/*------------------------------------------------------
  アドレスをアラインメントする
 -----------------------------------------------------*/
#define ALIGN( addr, align_size ) (((addr)  & ~((align_size) - 1)) + (align_size))

static u32 ELi_ALIGN( u32 addr, u32 align_size);
u32 ELi_ALIGN( u32 addr, u32 align_size)
{
    u32 aligned_addr;
    
    if( (addr % align_size) == 0) {
        aligned_addr = addr;
    }else{
        aligned_addr = (((addr) & ~((align_size) - 1)) + (align_size));
    }
    
    return aligned_addr;
}


/*------------------------------------------------------
  IS-TWL_DEBUGGERへの通知関数
 -----------------------------------------------------*/
SDK_WEAK_SYMBOL BOOL _ISTDbgLib_RegistVeneerInfo( ISTDOVERLAYPROC nProc, ISTDVENEERTYPE vType, u32 nVeneerAddress, u32 nJumpAddress)
{
#pragma unused( nProc)
#pragma unused( vType)
#pragma unused( nVeneerAddress)
#pragma unused( nJumpAddress)
    return( TRUE);
}

SDK_WEAK_SYMBOL BOOL _ISTDbgLib_UnregistVeneerInfo( ISTDOVERLAYPROC nProc, ISTDVENEERTYPE vType, u32 nVeneerAddress)
{
#pragma unused( nProc, vType, nVeneerAddress)
    return( TRUE);
}

static BOOL ELi_REGISTER_VENEER_INFO( ISTDOVERLAYPROC nProc, ISTDVENEERTYPE vType, u32 nVeneerAddress, u32 nJumpAddress, BOOL enable);
static BOOL ELi_UNREGISTER_VENEER_INFO( ISTDOVERLAYPROC nProc, ISTDVENEERTYPE vType, u32 nVeneerAddress, BOOL enable);

static BOOL ELi_REGISTER_VENEER_INFO( ISTDOVERLAYPROC nProc, ISTDVENEERTYPE vType, u32 nVeneerAddress, u32 nJumpAddress, BOOL enable)
{
    if( enable) {
        return( _ISTDbgLib_RegistVeneerInfo( nProc, vType, nVeneerAddress, nJumpAddress));
    }
    return( FALSE);
}

static BOOL ELi_UNREGISTER_VENEER_INFO( ISTDOVERLAYPROC nProc, ISTDVENEERTYPE vType, u32 nVeneerAddress, BOOL enable)
{
    if( enable) {
        return( _ISTDbgLib_UnregistVeneerInfo( nProc, vType, nVeneerAddress));
    }
    return( FALSE);
}

/*------------------------------------------------------
  コピーしたい内容がバッファの余白に収まるかチェックする
    start : 絶対アドレス
    size  : コピーしようとするサイズ
 -----------------------------------------------------*/
static BOOL ELi_CheckBufRest( ELDesc* elElfDesc, ELObject* MYObject, void* start, u32 size)
{
    if( ((u32)start + size) > (MYObject->buf_limit_addr)) {
        ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_NO_MORE_RESOURCE);
        return( FALSE);
    }else{
        return( TRUE);
    }
}

/*------------------------------------------------------
  ベニヤをバッファにコピーする
    start : 呼び出し元
    data : 飛び先
    threshold : この範囲内に既にベニヤがあれば使いまわす
 -----------------------------------------------------*/
void* ELi_CopyVeneerToBuffer( ELDesc* elElfDesc, ELObject* MYObject, u32 start, u32 data, s32 threshold)
{
#pragma unused(elElfDesc)
    u32*        el_veneer_dest;
    u32         load_start;
    Elf32_Addr  sh_size;
    ELVeneer*   elVenEnt;

    if( (elElfDesc == i_eldesc_sim)&&(data == 0)) {
    }else{
        /*--- すでにベニヤがあればそれを使う ---*/
        elVenEnt = ELi_GetVenEntry( &(MYObject->ELVenEntStart), data);
        if( elVenEnt != NULL) {
            if( ELi_IsFar( start, elVenEnt->adr, threshold) == FALSE) {
                PRINTDEBUG( "Veneer Hit!\n");
                return( (void*)elVenEnt->adr);
            }else{
                /*ベニヤが遠すぎて使えないのでリンクリストから削除*/
                (void)ELi_RemoveVenEntry( &(MYObject->ELVenEntStart), elVenEnt);
            }
        }
        /*--------------------------------------*/
    }

    /*アラインメントをとる*/
    load_start = ELi_ALIGN( ((u32)(MYObject->buf_current)), 4);
    /*サイズ設定*/
    sh_size = 12;//el_veneer

    /* バッファを越えないかチェック */
    if( ELi_CheckBufRest( elElfDesc, MYObject, (void*)load_start, sh_size) == FALSE) {
        return( NULL);
    }
  
    if( elElfDesc != i_eldesc_sim) {
        /* simulationでなければ実際にコピー */
        OSAPI_CPUCOPY8( el_veneer, (u32*)load_start, sh_size);
        el_veneer_dest = (u32*)load_start;
        el_veneer_dest[2] = data;
    }

    /*バッファポインタを移動*/
    MYObject->buf_current = (void*)(load_start + sh_size);

    /*--- ベニヤリストに追加 ---*/
    elVenEnt = ELi_Malloc( elElfDesc, MYObject, sizeof( ELVeneer));
    if(elVenEnt == NULL)
        return 0;
    elVenEnt->adr = load_start;
    elVenEnt->data = data;
    ELi_AddVeneerEntry( &(MYObject->ELVenEntStart), elVenEnt);
    (void)ELi_REGISTER_VENEER_INFO( ISTDRELOCATIONPROC_AUTO, ISTDVENEERTYPE_AUTO, elVenEnt->adr, elVenEnt->data, (elElfDesc != i_eldesc_sim));
    /*--------------------------*/

    /*ロードした先頭アドレスを返す*/
    return (void*)load_start;
}


/*------------------------------------------------------
  ベニヤをバッファにコピーする
    start : 呼び出し元
    data : 飛び先
    threshold : この範囲内に既にベニヤがあれば使いまわす
 -----------------------------------------------------*/
void* ELi_CopyV4tVeneerToBuffer( ELDesc* elElfDesc, ELObject* MYObject, u32 start, u32 data, s32 threshold)
{
#pragma unused(elElfDesc)
    u32*        el_veneer_dest;
    u32         load_start;
    Elf32_Addr  sh_size;
    ELVeneer*   elVenEnt;

    if( (elElfDesc == i_eldesc_sim)&&(data == 0)) {
        /*--- すでにベニヤがあればそれを使う ---*/
        elVenEnt = ELi_GetVenEntry( &(MYObject->ELV4tVenEntStart), data);
        if( elVenEnt != NULL) {
            if( ELi_IsFar( start, elVenEnt->adr, threshold) == FALSE) {
                PRINTDEBUG( "Veneer Hit!\n");
                return( (void*)elVenEnt->adr);
            }else{
                /*ベニヤが遠すぎて使えないのでリンクリストから削除*/
                (void)ELi_RemoveVenEntry( &(MYObject->ELV4tVenEntStart), elVenEnt);
            }
        }
        /*--------------------------------------*/
    }

    /*アラインメントをとる*/
    load_start = ELi_ALIGN( ((u32)(MYObject->buf_current)), 4);
    /*サイズ設定*/
    sh_size = 20;//el_veneer_t

    /* バッファを越えないかチェック */
    if( ELi_CheckBufRest( elElfDesc, MYObject, (void*)load_start, sh_size) == FALSE) {
        return( NULL);
    }
  
    if( elElfDesc != i_eldesc_sim) {
        /* simulationでなければ実際にコピー */
        OSAPI_CPUCOPY8( el_veneer_t, (u32*)load_start, sh_size);
        el_veneer_dest = (u32*)load_start;
        el_veneer_dest[4] = data;
    }

    /*バッファポインタを移動*/
    MYObject->buf_current = (void*)(load_start + sh_size);

    /*--- ベニヤリストに追加 ---*/
    elVenEnt = ELi_Malloc( elElfDesc, MYObject, sizeof( ELVeneer));
    if(elVenEnt == NULL)
        return 0;
    elVenEnt->adr = load_start;
    elVenEnt->data = data;
    ELi_AddVeneerEntry( &(MYObject->ELV4tVenEntStart), elVenEnt);
    (void)ELi_REGISTER_VENEER_INFO( ISTDRELOCATIONPROC_AUTO, ISTDVENEERTYPE_AUTO, elVenEnt->adr, elVenEnt->data, (elElfDesc != i_eldesc_sim));
    /*--------------------------*/

    /*ロードした先頭アドレスを返す*/
    return (void*)load_start;
}


/*------------------------------------------------------
  セグメントをバッファにコピーする
 -----------------------------------------------------*/
void* ELi_CopySegmentToBuffer( ELDesc* elElfDesc, ELObject* MYObject, Elf32_Phdr* Phdr)
{
    u32 load_start;
    
    load_start = ELi_ALIGN( Phdr->p_vaddr, Phdr->p_align);
    
    if( elElfDesc != i_eldesc_sim) {
        /* simulationでなければ実際にコピー */
        if( elElfDesc->i_elReadStub( (void*)load_start,
                                    elElfDesc->FileStruct,
                                    (u32)(elElfDesc->ar_head),
                                    (u32)(elElfDesc->elf_offset)+(u32)(Phdr->p_offset),
                                    Phdr->p_filesz) == FALSE)
        {
            ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_CANNOT_ACCESS_ELF);
            return( NULL);
        }
    }
    
    return( (void*)load_start);
}

/*------------------------------------------------------
  セクションをバッファにコピーする
 -----------------------------------------------------*/
void* ELi_CopySectionToBuffer( ELDesc* elElfDesc, ELObject* MYObject, Elf32_Shdr* Shdr)
{
    u32         load_start;
    Elf32_Addr  sh_size;

    /*アラインメントをとる*/
    load_start = ELi_ALIGN( ((u32)(MYObject->buf_current)), (Shdr->sh_addralign));
    /*サイズ設定*/
    sh_size = Shdr->sh_size;

    /* バッファを越えないかチェック */
    if( ELi_CheckBufRest( elElfDesc, MYObject, (void*)load_start, sh_size) == FALSE) {
        return( NULL);
    }

    if( elElfDesc != i_eldesc_sim) {
        /* simulationでなければ実際にコピー */
        if( elElfDesc->i_elReadStub( (void*)load_start,
                                    elElfDesc->FileStruct,
                                    (u32)(elElfDesc->ar_head),
                                    (u32)(elElfDesc->elf_offset)+(u32)(Shdr->sh_offset),
                                    sh_size) == FALSE)
        {
            ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_CANNOT_ACCESS_ELF);
            return( NULL);
        }
    }
    /*バッファポインタを移動*/
    MYObject->buf_current = (void*)(load_start + sh_size);

    /*ロードした先頭アドレスを返す*/
    return (void*)load_start;
}


/*------------------------------------------------------
  セクションをバッファに確保（コピーしない）し、
  その領域を0で埋める
 -----------------------------------------------------*/
void* ELi_AllocSectionToBuffer( ELDesc* elElfDesc, ELObject* MYObject, Elf32_Shdr* Shdr)
{
#pragma unused(elElfDesc)
    u32         load_start;
    Elf32_Addr  sh_size;

    /*アラインメントをとる*/
    load_start = ELi_ALIGN( ((u32)(MYObject->buf_current)), (Shdr->sh_addralign));
    /*サイズ設定*/
    sh_size = Shdr->sh_size;

    /* バッファを越えないかチェック */
    if( ELi_CheckBufRest( elElfDesc, MYObject, (void*)load_start, sh_size) == FALSE) {
        return( NULL);
    }
  
    /*バッファポインタを移動*/
    MYObject->buf_current = (void*)(load_start + sh_size);

    /*0で埋める（.bssセクションを想定しているため）*/
    OSAPI_CPUFILL8( (void*)load_start, 0, sh_size);
    
    /*確保した先頭アドレスを返す*/
    return (void*)load_start;
}


/*------------------------------------------------------
  指定インデックスのプログラムヘッダをバッファに取得する
 -----------------------------------------------------*/
void ELi_GetPhdr( ELDesc* elElfDesc, u32 index, Elf32_Phdr* Phdr)
{
    u32 offset;
    
    offset = (elElfDesc->CurrentEhdr.e_phoff) +
             ((u32)(elElfDesc->CurrentEhdr.e_phentsize) * index);
    
    (void)elElfDesc->i_elReadStub( Phdr,
                             elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head),
                             (u32)(elElfDesc->elf_offset) + offset,
                             sizeof( Elf32_Shdr));
}

/*------------------------------------------------------
  指定インデックスのセクションヘッダをバッファに取得する
 -----------------------------------------------------*/
void ELi_GetShdr( ELDesc* elElfDesc, u32 index, Elf32_Shdr* Shdr)
{
    u32 offset;
    
    offset = (elElfDesc->CurrentEhdr.e_shoff) + ((u32)(elElfDesc->shentsize) * index);
    
    (void)elElfDesc->i_elReadStub( Shdr,
                             elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head),
                             (u32)(elElfDesc->elf_offset) + offset,
                             sizeof( Elf32_Shdr));
}

/*------------------------------------------------------
  指定インデックスのセクションのエントリをバッファに取得する
 -----------------------------------------------------*/
void ELi_GetSent( ELDesc* elElfDesc, u32 index, void* entry_buf, u32 offset, u32 size)
{
    Elf32_Shdr  Shdr;

    ELi_GetShdr( elElfDesc, index, &Shdr);
    
    (void)elElfDesc->i_elReadStub( entry_buf,
                             elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head),
                             (u32)(elElfDesc->elf_offset) + (u32)(Shdr.sh_offset) + offset,
                             size);
}


/*------------------------------------------------------
  指定セクションヘッダの指定インデックスのエントリをバッファに取得する
    （Rel,Rela,Symなどエントリサイズが固定のセクションのみ）
    Shdr : 指定セクションヘッダ
    index : エントリ番号(0～)
 -----------------------------------------------------*/
void ELi_GetEntry( ELDesc* elElfDesc, Elf32_Shdr* Shdr, u32 index, void* entry_buf)
{
    u32 offset;

    offset = (u32)(Shdr->sh_offset) + ((Shdr->sh_entsize) * index);
    
    (void)elElfDesc->i_elReadStub( entry_buf,
                             elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head),
                             (u32)(elElfDesc->elf_offset) + offset,
                             Shdr->sh_entsize);
}

/*------------------------------------------------------
  STRセクションヘッダの指定インデックスの文字列を取得する

    Shdr : 指定セクションヘッダ
    index : エントリインデックス(0～)
 -----------------------------------------------------*/
void ELi_GetStrAdr( ELDesc* elElfDesc, u32 strsh_index, u32 ent_index, char* str, u32 len)
{
    /*文字列エントリの先頭アドレス*/
    ELi_GetSent( elElfDesc, strsh_index, str, ent_index, len);
}

/*------------------------------------------------------
  シンボルを再定義する

    <Relセクションヘッダ>
    RelShdr->sh_link : シンボルセクション番号
    RelShdr->sh_info : ターゲットセクション番号(例：rel.textの.textを表す)
    
    <Relエントリ>
    Rel->r_offset : ターゲットセクション内のオフセットアドレス
    ELF32_R_SYM( Rel->r_info) : シンボルエントリ番号
    ELF32_R_TYPE( Rel->r_info) : リロケートタイプ

    <Symセクションヘッダ>
    SymShdr->sh_link : シンボルの文字列テーブルセクション番号
    
    <Symエントリ>
    Sym->st_name : シンボルの文字列エントリ番号
    Sym->st_value : シンボルの所属するセクション内のオフセットアドレス
    Sym->st_size : シンボルの所属するセクション内でのサイズ
    Sym->st_shndx : シンボルの所属するセクションの番号
    ELF32_ST_BIND( Sym->st_info) : バインド(LOCAL or GLOBAL)
    ELF32_ST_TYPE( Sym->st_info) : タイプ(シンボルが関連付けられている対象)
 -----------------------------------------------------*/
BOOL ELi_RelocateSym( ELDesc* elElfDesc, ELObject* MYObject, u32 relsh_index)
{
    u32                 i;
    u32                 num_of_rel;     //再定義すべきシンボルの数
    Elf32_Shdr          RelOrRelaShdr;  //RELまたはRELAセクションヘッダ
    Elf32_Rela          CurrentRela;    //RELまたはRELAエントリのコピー先
    Elf32_Shdr*         SymShdr;
    ELSymEx*            CurrentSymEx;
    ELShdrEx*           TargetShdrEx;
    ELShdrEx*           CurrentShdrEx;
    u32                 relocation_adr;
    char                sym_str[128];
    u32                 copy_size;
    ELAdrEntry*         CurrentAdrEntry;
    u32                 sym_loaded_adr = 0;
    u32                 thumb_func_flag = 0;
    ELImportEntry       UnresolvedInfo;
    ELImportEntry*      UnrEnt;
    ELObject*           ExpObjEnt;
    u32                 unresolved_num = 0;

    /*REL or RELAセクションヘッダ取得*/
    ELi_GetShdr( elElfDesc, relsh_index, &RelOrRelaShdr);

    /*ターゲットセクションのEXヘッダ*/
    TargetShdrEx = ELi_GetShdrExfromList( elElfDesc->ShdrEx, RelOrRelaShdr.sh_info);
    
    num_of_rel = (RelOrRelaShdr.sh_size) / (RelOrRelaShdr.sh_entsize);    //再定義すべきシンボルの数

    /*シンボルリストを作成する*/
    if(ELi_BuildSymList( elElfDesc, RelOrRelaShdr.sh_link) == FALSE)
        return FALSE;
    /*ELi_BuildSymListを呼ぶとelElfDesc->SymShdrがセットされる*/
    SymShdr = &(elElfDesc->SymShdr);
    PRINTDEBUG( "SymShdr->link:%02x, SymShdr->info:%02x\n", SymShdr->sh_link, SymShdr->sh_info);


    /*--- 再定義が必要なシンボルの再定義 ---*/
    for( i=0; i<num_of_rel; i++) {
        
        /*- RelまたはRelaエントリ取得 -*/
        ELi_GetEntry( elElfDesc, &RelOrRelaShdr, i, &CurrentRela);
        
        if( RelOrRelaShdr.sh_type == SHT_REL) {
            CurrentRela.r_addend = 0;
        }
        /*-----------------------------*/

        /*シンボルExエントリ取得(ELi_GetSymExfromTblでもよい)*/
        CurrentSymEx = (ELSymEx*)(elElfDesc->SymExTbl[ELF32_R_SYM( CurrentRela.r_info)]);

//        if( CurrentSymEx->debug_flag == 1) {            /*デバッグ情報の場合*/
        if( CurrentSymEx == NULL) {
        }else{                                            /*デバッグ情報でない場合*/
            /**/
            ELi_InitImport( &UnresolvedInfo);
            /*書き換えるアドレス（仕様書でいう「P」）*/
            relocation_adr = (TargetShdrEx->loaded_adr) + (CurrentRela.r_offset);
            UnresolvedInfo.r_type = ELF32_R_TYPE( CurrentRela.r_info);
            UnresolvedInfo.A_ = (CurrentRela.r_addend);
            UnresolvedInfo.P_ = (relocation_adr);
            UnresolvedInfo.sh_type = (RelOrRelaShdr.sh_type);
            
            /*シンボルのアドレスを突き止める*/
            if( CurrentSymEx->Sym.st_shndx == SHN_UNDEF) {
                /*アドレステーブルから検索*/
                ELi_GetStrAdr( elElfDesc, SymShdr->sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
                CurrentAdrEntry = elGetAdrEntry( elElfDesc, sym_str, &ExpObjEnt);
              
                /*シンボル文字列確定*/
                copy_size = (u32)OSAPI_STRLEN( sym_str) + 1;
                UnresolvedInfo.sym_str = ELi_Malloc( elElfDesc, MYObject, copy_size);
                if(UnresolvedInfo.sym_str == NULL)
                    return FALSE;
                OSAPI_CPUCOPY8( sym_str, UnresolvedInfo.sym_str, copy_size);

                if( CurrentAdrEntry) {    /*アドレステーブルから見つかったとき*/
                    sym_loaded_adr = (u32)(CurrentAdrEntry->adr);
                    /*THUMB関数フラグ（仕様書でいう「T」）THUMB or ARM の判別*/
                    thumb_func_flag = CurrentAdrEntry->thumb_flag;
                    PRINTDEBUG( "\n symbol found %s : %8x\n", sym_str, sym_loaded_adr);
                }else{                    /*アドレステーブルから見つからないときは解決しない*/
                    /*未解決テーブルに追加登録*/
                    copy_size = sizeof( ELImportEntry);
                    UnrEnt = ELi_Malloc( elElfDesc, MYObject, copy_size);
                    if(UnrEnt == NULL)
                        return FALSE;
                    OSAPI_CPUCOPY8( &UnresolvedInfo, UnrEnt, copy_size);
                    ELi_AddImportEntry( &(MYObject->UnresolvedImportAdrEnt), UnrEnt);

                    unresolved_num++;    /*未解決シンボル数をカウント*/
                    PRINTDEBUG( "\n WARNING! cannot find symbol : %s\n", sym_str);
                }
            }else{ /* 自分自身で解決するとき */
                /*シンボルの所属するセクションのExヘッダ取得*/
                CurrentShdrEx = ELi_GetShdrExfromList( elElfDesc->ShdrEx, CurrentSymEx->Sym.st_shndx);
                sym_loaded_adr = CurrentShdrEx->loaded_adr;
                sym_loaded_adr += CurrentSymEx->Sym.st_value;    //sym_loaded_adrは仕様書でいう「S」
                /*THUMB関数フラグ（仕様書でいう「T」）THUMB or ARM の判別*/
                thumb_func_flag = CurrentSymEx->thumb_flag;
                ExpObjEnt = MYObject;
            }

            if( ExpObjEnt) {        /* 誰かか自分がエクスポートしていた場合は解決する */
                /*仕様書と同じ変数名にする*/
                UnresolvedInfo.S_ = (sym_loaded_adr);
                UnresolvedInfo.T_ = (thumb_func_flag);

                /*解決済みテーブルに追加登録(別オブジェクトを参照する場合のみ)*/
                if( ExpObjEnt != MYObject) {
                    copy_size = sizeof( ELImportEntry);
                    UnrEnt = ELi_Malloc( elElfDesc, MYObject, copy_size);
                    if(UnrEnt == NULL)
                        return FALSE;
                    OSAPI_CPUCOPY8( &UnresolvedInfo, UnrEnt, copy_size);
                    UnrEnt->Dlld = (struct ELObject*)ExpObjEnt;    /* 解決に使用したオブジェクトを記録 */
                    ELi_AddImportEntry( &(MYObject->ResolvedImportAdrEnt), UnrEnt);
                }
              
                /*--------------- シンボルの解決(再配置の実行) ---------------*/
                /*CurrentSymEx->relocation_val =*/
                if( ELi_DoRelocate( elElfDesc, MYObject, &UnresolvedInfo) == FALSE) {
                    return( FALSE);
                }
                /*------------------------------------------------------------*/
            }
        }
    }
    /*-----------------------------------*/


    /*シンボルリストを開放する*/
//    ELi_FreeSymList( elElfDesc);

    
    /* 再配置完了後 */
    if( unresolved_num == 0) {
        ELi_SetProcCode( elElfDesc, NULL, EL_PROC_RELOCATED);
    }
    return TRUE;
}


/*------------------------------------------------------
  グローバルシンボルをアドレステーブルに入れる

    <Symセクションヘッダ>
    SymShdr->sh_link : シンボルの文字列テーブルセクション番号
    
    <Symエントリ>
    Sym->st_name : シンボルの文字列エントリ番号
    Sym->st_value : シンボルの所属するセクション内のオフセットアドレス
    Sym->st_size : シンボルの所属するセクション内でのサイズ
    Sym->st_shndx : シンボルの所属するセクションの番号
    ELF32_ST_BIND( Sym->st_info) : バインド(LOCAL or GLOBAL)
    ELF32_ST_TYPE( Sym->st_info) : タイプ(シンボルが関連付けられている対象)
 -----------------------------------------------------*/
BOOL ELi_GoPublicGlobalSym( ELDesc* elElfDesc, ELObject* MYObject, u32 symtblsh_index)
{
    Elf32_Shdr   SymShdr;
    ELSymEx*     CurrentSymEx;
    ELShdrEx*    CurrentShdrEx;
    ELAdrEntry*  ExportAdrEntry;
    ELObject*    DmyObjEnt;
    char         sym_str[128];
    u32          copy_size;
    
    /*SYMTABセクションヘッダ取得*/
    ELi_GetShdr( elElfDesc, symtblsh_index, &SymShdr);
    
    /*シンボルリストを作成する*/
    if(ELi_BuildSymList( elElfDesc, symtblsh_index) == FALSE)
        return FALSE;
    
    /*--- ライブラリ内のGLOBALシンボルをアドレステーブルに公開する ---*/
    CurrentSymEx = elElfDesc->SymEx; //Tblからでなく、ここからたどればdebug情報は含まれない
    while( CurrentSymEx != NULL) {
//        CurrentSymEx = ELi_GetSymExfromList( elElfDesc->SymExTbl, i);
//        if( CurrentSymEx != NULL) {
        /*GLOBALで、かつ関連するセクションがライブラリ内に存在する場合*/
        if( (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_GLOBAL) &&
            (CurrentSymEx->Sym.st_shndx != SHN_UNDEF)) {
            
            ExportAdrEntry = ELi_Malloc( elElfDesc, MYObject, sizeof(ELAdrEntry));    /*メモリ確保*/
            if(ExportAdrEntry == NULL)
                return FALSE;
            ExportAdrEntry->next = NULL;
            
            ELi_GetStrAdr( elElfDesc, SymShdr.sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
            copy_size = (u32)OSAPI_STRLEN( sym_str) + 1;
            ExportAdrEntry->name = ELi_Malloc( elElfDesc, MYObject, copy_size);
            if(ExportAdrEntry->name == NULL)
                return FALSE;
            OSAPI_CPUCOPY8( sym_str, ExportAdrEntry->name, copy_size);

            CurrentShdrEx = ELi_GetShdrExfromList( elElfDesc->ShdrEx, CurrentSymEx->Sym.st_shndx);
            //Sym.st_valueは偶数/奇数でARM/Thumbを判別できるように調整されている場合があるので、その調整を削除して正味の値を出す
            ExportAdrEntry->adr = (void*)(CurrentShdrEx->loaded_adr + ((CurrentSymEx->Sym.st_value)&0xFFFFFFFE));
            ExportAdrEntry->func_flag = (u16)(ELF32_ST_TYPE( CurrentSymEx->Sym.st_info));
            ExportAdrEntry->thumb_flag = CurrentSymEx->thumb_flag;

            if( elGetAdrEntry( elElfDesc, ExportAdrEntry->name, &DmyObjEnt) == NULL) {    //入ってなかったら
                elAddAdrEntry( &(MYObject->ExportAdrEnt), ExportAdrEntry);    /*登録*/
            }
        }
        CurrentSymEx = CurrentSymEx->next;
//        }
    }
    /*----------------------------------------------------------------*/

    /*シンボルリストを開放する*/
//    ELi_FreeSymList( elElfDesc);
    return TRUE;
}


/*------------------------------------------------------
  シンボルリストを作成する

 -----------------------------------------------------*/
static BOOL ELi_BuildSymList( ELDesc* elElfDesc, u32 symsh_index)
{
    u32         i;
    u32         num_of_sym;        //シンボルの全体数
    u16         debug_flag;
    Elf32_Sym   TestSym;
    ELSymEx*    CurrentSymEx;
    ELShdrEx*   CurrentShdrEx;
    Elf32_Shdr* SymShdr;
    ELSymEx     DmySymEx;

    if( elElfDesc->SymExTarget == symsh_index) {
        PRINTDEBUG( "%s skip.\n", __FUNCTION__);
        return TRUE;                              //既にリスト作成済み
    }else{
        ELi_FreeSymList( elElfDesc); /*シンボルリストを開放する*/
    }
    
    PRINTDEBUG( "%s build\n", __FUNCTION__);
    
    /*SYMTABセクションヘッダ取得*/
    ELi_GetShdr( elElfDesc, symsh_index, &(elElfDesc->SymShdr));
    SymShdr = &(elElfDesc->SymShdr);
    
    num_of_sym = (SymShdr->sh_size) / (SymShdr->sh_entsize);    //シンボルの全体数
    elElfDesc->SymExTbl = ELi_Malloc( elElfDesc, NULL, num_of_sym * 4);
    if(elElfDesc->SymExTbl == NULL)
        return FALSE;
    /*---------- ELSymExのリストを作る ----------*/
    CurrentSymEx = &DmySymEx;
    for( i=0; i<num_of_sym; i++) {
        
        /*シンボルエントリをコピー*/
        ELi_GetEntry( elElfDesc, (Elf32_Shdr*)SymShdr, i, &TestSym);
        /*-- デバッグ情報フラグをセット --*/
        CurrentShdrEx = ELi_GetShdrExfromList( elElfDesc->ShdrEx, TestSym.st_shndx);
        if( CurrentShdrEx) {
            debug_flag = CurrentShdrEx->debug_flag;
        }else{
            debug_flag = 0;
        }/*-------------------------------*/

        if( debug_flag == 1) {
            elElfDesc->SymExTbl[i] = NULL;
        }else{
            CurrentSymEx->next = ELi_Malloc( elElfDesc, NULL, sizeof(ELSymEx));
            if(CurrentSymEx->next == NULL)
                return FALSE;
            CurrentSymEx = (ELSymEx*)(CurrentSymEx->next);
            
            OSAPI_CPUCOPY8( &TestSym, &(CurrentSymEx->Sym), sizeof(TestSym));
            
            elElfDesc->SymExTbl[i] = CurrentSymEx;
            
            PRINTDEBUG( "sym_no: %02x ... st_shndx: %04x\n", i, CurrentSymEx->Sym.st_shndx);
        }
    }
    
    CurrentSymEx->next = NULL;
    elElfDesc->SymEx = DmySymEx.next;
    /*-------------------------------------------*/
    

    /*-------- ARM or Thumb 判定(ELi_CodeIsThumbはSymExが出来てないと動かないため) --------*/
    CurrentSymEx = elElfDesc->SymEx;
    while( CurrentSymEx != NULL) { //ここからたどればdebug情報は無い
        /*-- ELSymExのThumbフラグをセット（関数シンボルだけ必要）--*/
        if( ELF32_ST_TYPE( CurrentSymEx->Sym.st_info) == STT_FUNC) {
            CurrentSymEx->thumb_flag = (u16)(ELi_CodeIsThumb( elElfDesc,
                                                              CurrentSymEx->Sym.st_shndx,
                                                              CurrentSymEx->Sym.st_value));
        }else{
            CurrentSymEx->thumb_flag = 0;
        }/*--------------------------------------------------------*/
        
        CurrentSymEx = CurrentSymEx->next;
    }
    /*-------------------------------------------------------------------------------------*/


    elElfDesc->SymExTarget = symsh_index;
    return TRUE;
}


/*------------------------------------------------------
  シンボルリストを開放する

 -----------------------------------------------------*/
void ELi_FreeSymList( ELDesc* elElfDesc)
{
    ELSymEx*  CurrentSymEx;
    ELSymEx*  FwdSymEx;

    /*--- まずSymExTblを開放する ---*/
    if( elElfDesc->SymExTbl != NULL) {
        OSAPI_FREE( elElfDesc->SymExTbl);
        elElfDesc->SymExTbl = NULL;
    }
    
    /*------- ELSymExのリストを解放する -------*/
    CurrentSymEx = elElfDesc->SymEx;
    if( CurrentSymEx) {
        do {
            FwdSymEx = CurrentSymEx;
            CurrentSymEx = CurrentSymEx->next;
            OSAPI_FREE( FwdSymEx);
        }while( CurrentSymEx != NULL);
        elElfDesc->SymEx = NULL;
    }
    /*-----------------------------------------*/

    elElfDesc->SymExTarget = 0xFFFFFFFF;
}


/*------------------------------------------------------
  未解決情報をもとにシンボルを解決する

    r_type,S_,A_,P_,T_が全て分かっている必要がある。
 -----------------------------------------------------*/
#define _S_    (UnresolvedInfo->S_)
#define _A_    (UnresolvedInfo->A_)
#define _P_    (UnresolvedInfo->P_)
#define _T_    (UnresolvedInfo->T_)
BOOL    ELi_DoRelocate( ELDesc* elElfDesc, ELObject* MYObject, ELImportEntry* UnresolvedInfo)
{
    s32    signed_val;
    u32    relocation_val = 0;
    u32    relocation_adr;
    BOOL   ret_val;
    BOOL   veneer_flag = TRUE;

    ret_val = TRUE;
    relocation_adr = _P_;

    switch( (UnresolvedInfo->r_type)) {
      case R_ARM_PC24:
      case R_ARM_PLT32:
      case R_ARM_CALL:
      case R_ARM_JUMP24:
        if( UnresolvedInfo->sh_type == SHT_REL) {
            _A_ = (s32)(((*(vu32*)relocation_adr)|0xFF800000) << 2);    //一般的には-8になっているはず
        }
        /*----------------- veneer -----------------*/
        if( (elElfDesc == i_eldesc_sim)&&(_S_ == 0)) {
            veneer_flag = TRUE; //simulationで解決不能な場合はveneer前提とする
        }else{
            veneer_flag = ELi_IsFar( _P_, _S_|_T_, 0x2000000); //+-32MBytes内か?
        }
#if (TARGET_ARM_V5 == 1)
        if( veneer_flag == TRUE) {
#else //(TARGET_ARM_V4 == 1)
        //ARM->ThumbおよびARM->ARMでロングブランチのときは veneer 使用
        if( (_T_) || (veneer_flag == TRUE)) {
#endif
            //_A_はPC相対分岐のときの調整値なので、絶対分岐の場合は関係ない
            _S_ = (u32)ELi_CopyVeneerToBuffer( elElfDesc,
                                               MYObject,
                                               _P_,
                                               (u32)(( (s32)(_S_) /*+ _A_*/) | (s32)(_T_)),
                                               0x2000000);
            if(_S_ == 0)
                return FALSE;
            _T_ = 0; //veneerはARMコードのため
        }/*-----------------------------------------*/
        if( elElfDesc == i_eldesc_sim) {  //simulation時は実際にシンボル再定義しない
            break;
        }
        signed_val = (( (s32)(_S_) + _A_) | (s32)(_T_)) - (s32)(_P_);
        if( _T_) {        /*BLX命令でARMからThumbに飛ぶ(v5未満だとBL→ベニアでBXという仕組みが必要)*/
            relocation_val = (0xFA000000) | ((signed_val>>2) & 0x00FFFFFF) | (((signed_val>>1) & 0x1)<<24);
        }else{            /*BL命令でARMからARMに飛ぶ*/
            signed_val >>= 2;
            relocation_val = (*(vu32*)relocation_adr & 0xFF000000) | (signed_val & 0x00FFFFFF);
        }
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_ABS32:
        if( elElfDesc == i_eldesc_sim) { //simulation時は実際にシンボル再定義しない
            break;
        }
        relocation_val = (( _S_ + _A_) | _T_);
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_REL32:
      case R_ARM_PREL31:
        if( elElfDesc == i_eldesc_sim) { //simulation時は実際にシンボル再定義しない
            break;
        }
        relocation_val = (( _S_ + _A_) | _T_) - _P_;
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_LDR_PC_G0:
        /*----------------- veneer -----------------*/
        if( (elElfDesc == i_eldesc_sim)&&(_S_ == 0)) {
            veneer_flag = TRUE; //simulationで解決不能な場合はveneer前提とする
        }else{
            veneer_flag = ELi_IsFar( _P_, _S_|_T_, 0x2000000); //+-32MBytes内か?
        }
#if (TARGET_ARM_V5 == 1)
        if( veneer_flag == TRUE) {
#else //(TARGET_ARM_V4 == 1)
        //ARM->ThumbおよびARM->ARMでロングブランチのときは veneer 使用
        if( (_T_) || (veneer_flag == TRUE)) {
#endif
            //_A_はPC相対分岐のときの調整値なので、絶対分岐の場合は関係ない
            _S_ = (u32)ELi_CopyVeneerToBuffer( elElfDesc,
                                               MYObject,
                                               _P_,
                                               (u32)(( (s32)(_S_) /*+ _A_*/) | (s32)(_T_)),
                                               0x2000000);
            if(_S_ == 0)
                return FALSE;
            _T_ = 0; //veneerはARMコードのため
        }/*-----------------------------------------*/
        if( elElfDesc == i_eldesc_sim) {  //simulation時は実際にシンボル再定義しない
            break;
        }
        signed_val = ( (s32)(_S_) + _A_) - (s32)(_P_);
        signed_val >>= 2;
        relocation_val = (*(vu32*)relocation_adr & 0xFF000000) | (signed_val & 0x00FFFFFF);
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_ABS16:
      case R_ARM_ABS12:
      case R_ARM_THM_ABS5:
      case R_ARM_ABS8:
        if( elElfDesc == i_eldesc_sim) { //simulation時は実際にシンボル再定義しない
            break;
        }
        relocation_val = _S_ + _A_;
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_THM_PC22:/*別名：R_ARM_THM_CALL*/
        if( UnresolvedInfo->sh_type == SHT_REL) {
            _A_ = (s32)(((*(vu16*)relocation_adr & 0x07FF)<<11) + ((*((vu16*)(relocation_adr)+1)) & 0x07FF));
            _A_ = (s32)((_A_ | 0xFFC00000) << 1);    //一般的には-4になっているはず(PCは現命令アドレス+4なので)
        }
        /*----------------- veneer -----------------*/
        if( (elElfDesc == i_eldesc_sim)&&(_S_ == 0)) {
            veneer_flag = TRUE; //simulationで解決不能な場合はveneer前提とする
        }else{
            veneer_flag = ELi_IsFar( _P_, _S_|_T_, 0x400000); //+-4MBytes内か?
        }
#if (TARGET_ARM_V5 == 1)
        if( veneer_flag == TRUE) {
            //_A_はPC相対分岐のときの調整値なので、絶対分岐の場合は関係ない
            _S_ = (u32)ELi_CopyVeneerToBuffer( elElfDesc,
                                               MYObject,
                                               _P_,
                                               (u32)(( (s32)(_S_) /*+ _A_*/) | (s32)(_T_)),
                                               0x400000);
            if(_S_ == 0)
                return FALSE;
            _T_ = 0; //veneerはARMコードのため
        }/*-----------------------------------------*/
#else //(TARGET_ARM_V4 == 1)
        /*----------------- v4t veneer -----------------*/
        //Thumb->ARMおよびThumb->Thumbでロングブランチのときは v4T veneer 使用
        if( (_T_ == 0) || (veneer_flag == TRUE)) {
            _S_ = (u32)ELi_CopyV4tVeneerToBuffer( elElfDesc,
                                                  MYObject,
                                                  _P_,
                                                  (u32)(( (s32)(_S_)) | (s32)(_T_)),
                                                  0x400000);
            if(_S_ == 0)
                return FALSE;
            _T_ = 1; //v4t veneerはThumbコードのため
        }
        /*---------------------------------------------*/
#endif
        if( elElfDesc == i_eldesc_sim) { //simulation時は実際にシンボル再定義しない
            break;
        }
        signed_val = (( (s32)(_S_) + _A_) | (s32)(_T_)) - (s32)(_P_);
        signed_val >>= 1;
        if( _T_) {    /*BL命令でThumbからThumbに飛ぶ*/
            relocation_val = (u32)(*(vu16*)relocation_adr & 0xF800) | ((signed_val>>11) & 0x07FF) +
                                   ((((*((vu16*)(relocation_adr)+1)) & 0xF800) | (signed_val & 0x07FF)) << 16);
        }else{        /*BLX命令でThumbからARMに飛ぶ(v5未満だとBL→ベニアでBXという仕組みが必要)*/
            if( (signed_val & 0x1)) {    //_P_が4バイトアラインされていないとここに来る
                signed_val += 1;
            }
            relocation_val = (u32)(*(vu16*)relocation_adr & 0xF800) | ((signed_val>>11) & 0x07FF) +
                                   ((((*((vu16*)(relocation_adr)+1)) & 0xE800) | (signed_val & 0x07FF)) << 16);
        }
        *(vu16*)relocation_adr = (vu16)relocation_val;
        *((vu16*)relocation_adr+1) = (vu16)((u32)(relocation_val) >> 16);
        break;
      case R_ARM_NONE:
        /*R_ARM_NONEはターゲットのシンボルを保持し、リンカにデッドストリップ
          されないようにします*/
        break;
      case R_ARM_THM_JUMP24:
      default:
        ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_UNSUPPORTED_ELF);
        ret_val = FALSE;
        PRINTDEBUG( "ERROR! : unsupported relocation type (0x%x)!\n", (UnresolvedInfo->r_type));
        PRINTDEBUG( "S = 0x%x\n", _S_);
        PRINTDEBUG( "A = 0x%x\n", _A_);
        PRINTDEBUG( "P = 0x%x\n", _P_);
        PRINTDEBUG( "T = 0x%x\n", _T_);
        break;
    }
    
    return ret_val;//relocation_val;
}
#undef _S_
#undef _A_
#undef _P_
#undef _T_

/*------------------------------------------------------
  リストから指定インデックスのELSymExを取り出す
 -----------------------------------------------------*/
static ELSymEx* ELi_GetSymExfromList( ELSymEx* SymExStart, u32 index)
{
    u32         i;
    ELSymEx*    SymEx;

    SymEx = SymExStart;
    for( i=0; i<index; i++) {
        SymEx = (ELSymEx*)(SymEx->next);
        if( SymEx == NULL) {
            break;
        }
    }
    return SymEx;
}

/*------------------------------------------------------
  テーブルから指定インデックスのELSymExを取り出す
 -----------------------------------------------------*/
static ELSymEx* ELi_GetSymExfromTbl( ELSymEx** SymExTbl, u32 index)
{
    return( (ELSymEx*)(SymExTbl[index]));
}

/*------------------------------------------------------
  リストから指定インデックスのELShdrExを取り出す
 -----------------------------------------------------*/
ELShdrEx* ELi_GetShdrExfromList( ELShdrEx* ShdrExStart, u32 index)
{
    u32         i;
    ELShdrEx*   ShdrEx;

    ShdrEx = ShdrExStart;
    for( i=0; i<index; i++) {
        ShdrEx = (ELShdrEx*)(ShdrEx->next);
        if( ShdrEx == NULL) {
            break;
        }
    }
    return ShdrEx;
}



/*------------------------------------------------------
  指定インデックスのセクションがデバッグ情報かどうか判定する

＜デバッグ情報の定義＞
・セクション名が".debug"から始まるセクション

・.rel.debug～ セクションなど、sh_info がデバッグ情報セクション番号を
　示しているセクション
 -----------------------------------------------------*/
BOOL ELi_ShdrIsDebug( ELDesc* elElfDesc, u32 index)
{
    Elf32_Shdr  TmpShdr;
    char        shstr[6];

    /*-- セクション名の文字列を6文字取得 --*/
    ELi_GetShdr( elElfDesc, index, &TmpShdr);
    ELi_GetStrAdr( elElfDesc, elElfDesc->CurrentEhdr.e_shstrndx,
                   TmpShdr.sh_name, shstr, 6);
    /*-------------------------------------*/
    
    if( OSAPI_STRNCMP( shstr, ".debug", 6) == 0) {    /*デバッグセクションの場合*/
        return TRUE;
    }else{                        /*デバッグセクションに関する再配置セクションの場合*/
        if( (TmpShdr.sh_type == SHT_REL) || (TmpShdr.sh_type == SHT_RELA)) {
            if( ELi_ShdrIsDebug( elElfDesc, TmpShdr.sh_info) == TRUE) {
                return TRUE;
            }
        }
    }

    return FALSE;
}



/*------------------------------------------------------
  elElfDescのSymExテーブルを調べ、指定インデックスの
　指定オフセットにあるコードがARMかTHUMBかを判定する。
  （予め elElfDesc->SymShdr と elElfDesc->SymEx を設定しておくこと）
    
    sh_index : 調べたいセクションインデックス
    offset : 調べたいセクション内のオフセット
 -----------------------------------------------------*/
u32 ELi_CodeIsThumb( ELDesc* elElfDesc, u16 sh_index, u32 offset)
{
    u32            i;
    u32            thumb_flag;
    Elf32_Shdr*    SymShdr;
    char           str_adr[3];
    ELSymEx*       CurrentSymEx;

    /*シンボルのセクションヘッダとSymExリスト取得*/
    SymShdr = &(elElfDesc->SymShdr);
    CurrentSymEx = elElfDesc->SymEx;

    i = 0;
    thumb_flag = 0;
    while( CurrentSymEx != NULL) {
        
        if( CurrentSymEx->Sym.st_shndx == sh_index) {
            ELi_GetStrAdr( elElfDesc, SymShdr->sh_link, CurrentSymEx->Sym.st_name, str_adr, 3);
            if( OSAPI_STRNCMP( str_adr, "$a\0", OSAPI_STRLEN("$a\0")) == 0) {
                thumb_flag = 0;
            }else if( OSAPI_STRNCMP( str_adr, "$t\0", OSAPI_STRLEN("$t\0")) == 0) {
                thumb_flag = 1;
            }
            if( CurrentSymEx->Sym.st_value > offset) {
                break;
            }
        }
        
        CurrentSymEx = CurrentSymEx->next;
        i++;
    }

    return thumb_flag;
}


/*---------------------------------------------------------
 インポート情報エントリを初期化する
 --------------------------------------------------------*/
static void ELi_InitImport( ELImportEntry* ImportInfo)
{
    ImportInfo->sym_str = NULL;
    ImportInfo->r_type = 0;
    ImportInfo->S_ = 0;
    ImportInfo->A_ = 0;
    ImportInfo->P_ = 0;
    ImportInfo->T_ = 0;
    ImportInfo->Dlld = NULL;
}

/*------------------------------------------------------
  インポート情報テーブルからエントリを抜き出す（ImpEntは削除しない！）
 -----------------------------------------------------*/
BOOL ELi_ExtractImportEntry( ELImportEntry** StartEnt, ELImportEntry* ImpEnt)
{
    ELImportEntry  DmyImpEnt;
    ELImportEntry* CurImpEnt;

    DmyImpEnt.next = (*StartEnt);
    CurImpEnt      = &DmyImpEnt;

    while( CurImpEnt->next != ImpEnt) {
        if( CurImpEnt->next == NULL) {
            return FALSE;
        }else{
            CurImpEnt = (ELImportEntry*)CurImpEnt->next;
        }
    }

    /*リンクリストの繋ぎ直し*/
    CurImpEnt->next = ImpEnt->next;
    (*StartEnt) = DmyImpEnt.next;

    /*開放*/
//    OSAPI_FREE( ImpEnt);
    
    return TRUE;
}

/*---------------------------------------------------------
 インポート情報テーブルにエントリを追加する
 --------------------------------------------------------*/
void ELi_AddImportEntry( ELImportEntry** ELUnrEntStart, ELImportEntry* UnrEnt)
{
    ELImportEntry    DmyUnrEnt;
    ELImportEntry*   CurrentUnrEnt;

    if( !(*ELUnrEntStart)) {
        (*ELUnrEntStart) = UnrEnt;
    }else{
        DmyUnrEnt.next = (*ELUnrEntStart);
        CurrentUnrEnt = &DmyUnrEnt;

        while( CurrentUnrEnt->next != NULL) {
            CurrentUnrEnt = CurrentUnrEnt->next;
        }
        CurrentUnrEnt->next = (void*)UnrEnt;
    }
    UnrEnt->next = NULL;
}

/*------------------------------------------------------
  インポート情報テーブルを全部解放する
 -----------------------------------------------------*/
void ELi_FreeImportTbl( ELImportEntry** ELImpEntStart)
{
    ELImportEntry*    FwdImpEnt;
    ELImportEntry*    CurrentImpEnt;
    
    /*--- ELImportEntryのリストを解放する ---*/
    CurrentImpEnt = (*ELImpEntStart);
    if( CurrentImpEnt) {
        do {
            FwdImpEnt = CurrentImpEnt;
            CurrentImpEnt = CurrentImpEnt->next;
            OSAPI_FREE( FwdImpEnt->sym_str);        //シンボル名文字列
            OSAPI_FREE( FwdImpEnt);              //構造体自身
        }while( CurrentImpEnt != NULL);
        (*ELImpEntStart) = NULL;
    }
    /*------------------------------------*/
}


/*---------------------------------------------------------
 ベニヤテーブルにエントリを追加する
 --------------------------------------------------------*/
static void ELi_AddVeneerEntry( ELVeneer** start, ELVeneer* VenEnt)
{
    ELVeneer    DmyVenEnt;
    ELVeneer*   CurrentVenEnt;

    if( !(*start)) {
        (*start) = VenEnt;
    }else{
        DmyVenEnt.next = (*start);
        CurrentVenEnt = &DmyVenEnt;

        while( CurrentVenEnt->next != NULL) {
            CurrentVenEnt = CurrentVenEnt->next;
        }
        CurrentVenEnt->next = (void*)VenEnt;
    }
    VenEnt->next = NULL;
}

/*------------------------------------------------------
  ベニヤテーブルからエントリを削除する
 -----------------------------------------------------*/
static BOOL ELi_RemoveVenEntry( ELVeneer** start, ELVeneer* VenEnt)
{
    ELVeneer  DmyVenEnt;
    ELVeneer* CurrentVenEnt;

    DmyVenEnt.next = (*start);
    CurrentVenEnt = &DmyVenEnt;

    while( CurrentVenEnt->next != VenEnt) {
        if( CurrentVenEnt->next == NULL) {
            return FALSE;
        }else{
            CurrentVenEnt = (ELVeneer*)CurrentVenEnt->next;
        }
    }

    /*リンクリストの繋ぎ直し*/
    CurrentVenEnt->next = VenEnt->next;
    (*start) = DmyVenEnt.next;

    /*開放*/
    OSAPI_FREE( VenEnt);
    
     return TRUE;
}

/*------------------------------------------------------
  ベニヤテーブルから指定データに該当するエントリを探す
 -----------------------------------------------------*/
static ELVeneer* ELi_GetVenEntry( ELVeneer** start, u32 data)
{
    ELVeneer* CurrentVenEnt;

    CurrentVenEnt = (*start);
    if( CurrentVenEnt == NULL) {
        return NULL;
    }
    while( CurrentVenEnt->data != data) {
        CurrentVenEnt = (ELVeneer*)CurrentVenEnt->next;
        if( CurrentVenEnt == NULL) {
            break;
        }
    }
    return CurrentVenEnt;
}

/*------------------------------------------------------
  ベニヤテーブルを解放する
 -----------------------------------------------------*/
void* ELi_FreeVenTbl( ELDesc* elElfDesc, ELObject* MYObject)
{
    ELVeneer*    FwdVenEnt;
    ELVeneer*    CurrentVenEnt;
    
    /*--- ELVenEntryのリストを解放する ---*/
    CurrentVenEnt = MYObject->ELVenEntStart;
    while( CurrentVenEnt != NULL) {
        FwdVenEnt = CurrentVenEnt;
        CurrentVenEnt = CurrentVenEnt->next;
        (void)ELi_UNREGISTER_VENEER_INFO( ISTDRELOCATIONPROC_AUTO, ISTDVENEERTYPE_AUTO, FwdVenEnt->adr, (elElfDesc != i_eldesc_sim));
        OSAPI_FREE( FwdVenEnt);              //構造体自身
    }
    MYObject->ELVenEntStart = NULL;
    /*------------------------------------*/

    /*--- ELVenEntryのリストを解放する ---*/
    CurrentVenEnt = MYObject->ELV4tVenEntStart;
    while( CurrentVenEnt != NULL) {
        FwdVenEnt = CurrentVenEnt;
        CurrentVenEnt = CurrentVenEnt->next;
        (void)ELi_UNREGISTER_VENEER_INFO( ISTDRELOCATIONPROC_AUTO, ISTDVENEERTYPE_AUTO, FwdVenEnt->adr, (elElfDesc != i_eldesc_sim));
        OSAPI_FREE( FwdVenEnt);              //構造体自身
    }
    MYObject->ELV4tVenEntStart = NULL;
    /*------------------------------------*/
    
    return NULL;
}

/*------------------------------------------------------
  2つのアドレス値の差が (+threshold)～(-threshold)内に
  収まっているかを調べる
 -----------------------------------------------------*/
static BOOL ELi_IsFar( u32 P, u32 S, s32 threshold)
{
    s32 diff;

    diff = (s32)S - (s32)(P);
    if( (diff < threshold)&&( diff > -threshold)) {
        return( FALSE); //Near
    }else{
        return( TRUE);  //Far
    }
}

