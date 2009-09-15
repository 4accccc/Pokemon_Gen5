/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     insertsection.c

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include "types.h"
#include "elf.h"
#include "elf_loader.h"
#include "arch.h"
#include "loader_subset.h"
#include <string.h>
#include <stdlib.h>



extern u32 ELi_ALIGN( u32 addr, u32 align_size);



u16 ELi_LoadObject2( ELHandle* ElfHandle, void* obj_offset, void* buf)
{
    u16         i;
#if (SPECIAL_SECTION_ENABLE == 1) /*特殊セクション追加*/
    u16         j;
#endif
    ELShdrEx*   FwdShdrEx;
    ELShdrEx*   CurrentShdrEx;
    ELShdrEx    DmyShdrEx;
    u32         newelf_shoff = 0; //stripped elfイメージへの書き込みポインタ
    u32         buf_shdr;
    u32         section_num = 0;
    u32         stripped_section_num = 0;
    u32         tmp_buf;
    u32         *shdr_table;      //セクションヘッダ新旧番号対応テーブル
    u32         special_sect_name;


    /* ELHandleの初期化チェック */
    if( ElfHandle->process != EL_INITIALIZED) {
        return EL_FAILED;
    }
    /* バッファのNULLチェック */
    if( buf == NULL) {
        return EL_FAILED;
    }
    /* ELFヘッダの取得 */
    ElfHandle->ELi_ReadStub( &(ElfHandle->CurrentEhdr), ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head), (u32)(obj_offset), sizeof( Elf32_Ehdr));

    /* セクションハンドル構築 */
    ElfHandle->elf_offset = obj_offset;
    ElfHandle->buf_current = (void*)((u32)buf + sizeof( Elf32_Ehdr));
    ElfHandle->shentsize = ElfHandle->CurrentEhdr.e_shentsize;

    /*セクションヘッダテーブル構築*/
    shdr_table = (u32*)malloc( 4 * ElfHandle->CurrentEhdr.e_shnum);
  
    /*---------- ELShdrExのリストとshdr_tableを作る ----------*/
    CurrentShdrEx = &DmyShdrEx;
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        CurrentShdrEx->next = (void*)(malloc( sizeof(ELShdrEx)));
        CurrentShdrEx = (ELShdrEx*)(CurrentShdrEx->next);
        memset( CurrentShdrEx, 0, sizeof(ELShdrEx));     //ゼロクリア
        
        /*デバッグ情報かどうかを判別してフラグをセット*/
        if( ELi_ShdrIsDebug( ElfHandle, i) == TRUE) {    /*デバッグ情報の場合*/
            CurrentShdrEx->debug_flag = 1;
            shdr_table[i] = 0xFFFFFFFF;
        }else{                                           /*デバッグ情報でない場合*/
            CurrentShdrEx->debug_flag = 0;
            shdr_table[i] = stripped_section_num;        /*セクション新旧テーブル作成*/
            stripped_section_num++;
        }
        /*セクションヘッダをコピー*/
        ELi_GetShdr( ElfHandle, i, &(CurrentShdrEx->Shdr));
//        printf( "shdr_table[0x%x] = 0x%x\n", i, section_num);
        section_num++;
        /*セクション文字列を取得しておく*/
#if 0 // セクション文字列バッファを動的に確保しない
        CurrentShdrEx->str = (char*)malloc( 128);    //128文字バッファ取得
        ELi_GetStrAdr( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx,
                       CurrentShdrEx->Shdr.sh_name,
                       CurrentShdrEx->str, 128);
#else
        {
            u32 length;
            
            length = ELi_GetStrLen( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx, CurrentShdrEx->Shdr.sh_name ) + 1;
            
            CurrentShdrEx->str = (char*)malloc(length);
            if(CurrentShdrEx->str == 0)
            {
                return EL_FAILED;
            }
            ELi_GetStrAdr( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx,
               CurrentShdrEx->Shdr.sh_name,
               CurrentShdrEx->str, length);
        }
#endif
        /*特殊セクションのセクション名へのインデックス保持*/
        if( i == ElfHandle->CurrentEhdr.e_shstrndx){
            special_sect_name = CurrentShdrEx->Shdr.sh_size;
        }
    }
#if (SPECIAL_SECTION_ENABLE == 1) /*特殊セクション追加*/
    section_num++;                    //特殊セクション追加
    ElfHandle->CurrentEhdr.e_shnum++; //特殊セクション追加
    CurrentShdrEx->next = (void*)(malloc( sizeof(ELShdrEx)));
    CurrentShdrEx = (ELShdrEx*)(CurrentShdrEx->next);
    memset( CurrentShdrEx, 0, sizeof(ELShdrEx));    //ゼロクリア
    CurrentShdrEx->Shdr.sh_name = special_sect_name;
    CurrentShdrEx->Shdr.sh_type = SPECIAL_SECTION_TYPE;
    CurrentShdrEx->Shdr.sh_flags = 0;
    CurrentShdrEx->Shdr.sh_addr = 0;
    CurrentShdrEx->Shdr.sh_offset = 0;
    CurrentShdrEx->Shdr.sh_size = (stripped_section_num)*4; //(ElfHandle->CurrentEhdr.e_shnum)*4;(旧セクション番号がインデックスの場合)
    CurrentShdrEx->Shdr.sh_link = 0;
    CurrentShdrEx->Shdr.sh_info = 0;
    CurrentShdrEx->Shdr.sh_addralign = 4;
    CurrentShdrEx->Shdr.sh_entsize = 4;
#if 0 // セクション文字列バッファを動的に確保しない
    CurrentShdrEx->str = (char*)malloc( 128);
#else
    CurrentShdrEx->str = (char*)malloc( strlen(SPECIAL_SECTION_NAME) + 1 );
    if(CurrentShdrEx->str == 0)
    {
        return EL_FAILED;
    }
#endif
    strcpy( CurrentShdrEx->str, SPECIAL_SECTION_NAME);
#endif
    CurrentShdrEx->next = NULL;
    ElfHandle->ShdrEx = DmyShdrEx.next;
    /*--------------------------------------------------------*/
//printf( "stripped section_num : %d\n", stripped_section_num);
   
    /*---------- 全セクションを調べてコピーする ----------*/
//    printf( "\nLoad to RAM:\n");
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);

#if (SPECIAL_SECTION_ENABLE == 1) /*特殊セクション追加*/
            if( CurrentShdrEx->Shdr.sh_type == SPECIAL_SECTION_TYPE) {
                //ELi_CopySectionToBufferに相当する一連の処理
                u32 load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
                u32 sh_size    = CurrentShdrEx->Shdr.sh_size;
                u32* reverse_shdr_table = (u32*)malloc( 4 * stripped_section_num);
                /*インデックスが新セクション番号の表に変換する*/
                for( j=0; j<(ElfHandle->CurrentEhdr.e_shnum - 1); j++) {
                    if( shdr_table[j] != 0xFFFFFFFF) {
                        reverse_shdr_table[shdr_table[j]] = j;
                    }
                }
                memcpy( (u32*)load_start, reverse_shdr_table, sh_size); //新旧セクション対応テーブルコピー
                free( reverse_shdr_table);
                CurrentShdrEx->loaded_adr = load_start;
                ElfHandle->buf_current = (void*)(load_start + sh_size);
            }else{
#else
            {
#endif
                if( CurrentShdrEx->Shdr.sh_type != SHT_NULL) { //NULLセクション（セクション0）は本体なし
                    //メモリにコピー
                    CurrentShdrEx->loaded_adr = (u32)
                                    ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
                }
#if (SPECIAL_SECTION_ENABLE == 1) /*特殊セクション追加*/
                //セクション文字列セクションの場合
                if( (CurrentShdrEx->Shdr.sh_type == SHT_STRTAB)&&
                    (i == ElfHandle->CurrentEhdr.e_shstrndx)) {
                    strcpy( ElfHandle->buf_current, SPECIAL_SECTION_NAME);
                    //セクションのデータ容量更新
                    CurrentShdrEx->Shdr.sh_size += (strlen( SPECIAL_SECTION_NAME) + 1);
                    //バッファアドレス更新
                    ElfHandle->buf_current += (strlen( SPECIAL_SECTION_NAME) + 1);
                }
#endif
            }
    }
    /*-------------------------------------------------------*/

    
    /*ここまででセクションの中身だけがstripped elfにコピーされた*/

  
    /*---------- セクションヘッダを stripped elfにコピー ----------*/
    buf_shdr = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    ElfHandle->buf_current = (void*)buf_shdr;
//    printf( "buf_shdr = 0x%x\n", buf_shdr);
//    printf( "buf = 0x%x\n", (u32)buf);
    
    newelf_shoff = buf_shdr - ((u32)(buf));
//    printf( "newelf_shoff = 0x%x\n", newelf_shoff);

    section_num = 0;
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        tmp_buf = buf_shdr + ( section_num * sizeof( Elf32_Shdr));
        
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);
        /*オフセット更新*/
        if( CurrentShdrEx->loaded_adr != 0) {
            CurrentShdrEx->Shdr.sh_offset = (CurrentShdrEx->loaded_adr - (u32)buf);
        }
        /*セクションヘッダをstripped elfイメージへコピー*/
        memcpy( (u8*)tmp_buf, &(CurrentShdrEx->Shdr), 
                sizeof( Elf32_Shdr));
        section_num++;                        /*セクション数更新*/
    }
//printf( "section_num : %d\n", section_num);

    // コピー終了後
    ElfHandle->process = EL_COPIED;
    /*------------------------------------------------------------*/

    ElfHandle->newelf_size = (buf_shdr - (u32)buf) + (section_num*sizeof( Elf32_Shdr));
//    printf( "newelf_size = 0x%x\n", ElfHandle->newelf_size);

    /*---------- ELFヘッダ更新 ----------*/
    ElfHandle->CurrentEhdr.e_shnum = section_num; /*増えたセクション数をELFヘッダに反映*/
    //セクションヘッダオフセット更新
    ElfHandle->CurrentEhdr.e_shoff = newelf_shoff;
    memcpy( (u8*)buf, &(ElfHandle->CurrentEhdr), sizeof( Elf32_Ehdr)); /*ELFヘッダをstripped elfイメージにコピー*/
    /*-----------------------------------*/

    /*------- ELShdrExのリストを解放する -------*/
    CurrentShdrEx = ElfHandle->ShdrEx;
    if( CurrentShdrEx) {
        do{
            FwdShdrEx = CurrentShdrEx;
            CurrentShdrEx = CurrentShdrEx->next;
            free( FwdShdrEx->str);
            free( FwdShdrEx);
        }while( CurrentShdrEx != NULL);
        ElfHandle->ShdrEx = NULL;
    }
    /*-----------------------------------------*/

    /*RAM上のDLLが呼ばれる前にキャッシュをフラッシュ*/
//    DC_FlushAll();
//    DC_WaitWriteBufferEmpty();
    
    return (ElfHandle->process);
}
