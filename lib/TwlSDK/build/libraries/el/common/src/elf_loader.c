/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ELF Loader
  File:     elf_loader.c

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

#include "elf.h"
#include "arch.h"
#include "elf_loader.h"
#include "loader_subset.h"
#include <istdbglibpriv.h>


#ifndef SDK_TWL
OSHeapHandle    EL_Heap;
#endif


#if 1

/*------------------------------------------------------
  IS-TWL_DEBUGGERへの通知関数
 -----------------------------------------------------*/
void    _ISTDbgLib_StartRegistRelocationInfo( ISTDOVERLAYPROC nProc );
BOOL    _ISTDbgLib_RegistRelocationInfo( ISTDOVERLAYPROC nProc, u32 nDll, u32 nELF, u32 nSection, u32 nLMA );
void    _ISTDbgLib_EndRegistRelocationInfo( ISTDOVERLAYPROC nProc );
BOOL    _ISTDbgLib_UnregistRelocationInfo( ISTDOVERLAYPROC nProc, u32 nDll );

SDK_WEAK_SYMBOL void _ISTDbgLib_StartRegistRelocationInfo( ISTDOVERLAYPROC nProc )
{
    (void)nProc;
    return;
}

SDK_WEAK_SYMBOL BOOL _ISTDbgLib_RegistRelocationInfo( ISTDOVERLAYPROC nProc, u32 nDll, u32 nELF, u32 nSection, u32 nLMA )
{
    (void)nProc;
    (void)nDll;
    (void)nELF;
    (void)nSection;
    (void)nLMA;
    return TRUE;
}

SDK_WEAK_SYMBOL void _ISTDbgLib_EndRegistRelocationInfo( ISTDOVERLAYPROC nProc )
{
    (void)nProc;
    return;
}

SDK_WEAK_SYMBOL BOOL _ISTDbgLib_UnregistRelocationInfo( ISTDOVERLAYPROC nProc, u32 nDll )
{
    (void)nProc;
    (void)nDll;
    return TRUE;
}

/* stub */
static void ELi_START_REGISTER_RELOCATION_INFO( ISTDOVERLAYPROC nProc, BOOL enable );
static BOOL ELi_REGISTER_RELOCATION_INFO( ISTDOVERLAYPROC nProc, u32 nDll, u32 nELF, u32 nSection, u32 nLMA, BOOL enable );
static void ELi_END_REGISTER_RELOCATION_INFO( ISTDOVERLAYPROC nProc, BOOL enable );
static BOOL ELi_UNREGISTER_RELOCATION_INFO( ISTDOVERLAYPROC nProc, u32 nDll, BOOL enable );

static void ELi_START_REGISTER_RELOCATION_INFO( ISTDOVERLAYPROC nProc, BOOL enable )
{
    if( enable) {
        _ISTDbgLib_StartRegistRelocationInfo( nProc);
    }
}
static BOOL ELi_REGISTER_RELOCATION_INFO( ISTDOVERLAYPROC nProc, u32 nDll, u32 nELF, u32 nSection, u32 nLMA, BOOL enable )
{
    if( enable) {
        return( _ISTDbgLib_RegistRelocationInfo( nProc, nDll, nELF, nSection, nLMA));
    }
    return( FALSE);
}
static void ELi_END_REGISTER_RELOCATION_INFO( ISTDOVERLAYPROC nProc, BOOL enable )
{
    if( enable) {
        _ISTDbgLib_EndRegistRelocationInfo( nProc);
    }
}
static BOOL ELi_UNREGISTER_RELOCATION_INFO( ISTDOVERLAYPROC nProc, u32 nDll, BOOL enable )
{
    if( enable) {
        return( _ISTDbgLib_UnregistRelocationInfo( nProc, nDll));
    }
    return( FALSE);
}
#endif


/*------------------------------------------------------
  extern関数
 -----------------------------------------------------*/
extern ELDesc* i_eldesc_sim;


/*------------------------------------------------------
  グローバル変数
 -----------------------------------------------------*/
static ELReadImage i_elReadImage;
ELAlloc     i_elAlloc;
ELFree      i_elFree;

static BOOL    i_el_initialized = FALSE;
static OSMutex i_el_mutex;


/*------------------------------------------------------
  ローカル関数の宣言
 -----------------------------------------------------*/
static void ELi_FreeObject( ELObject** ELObjEntStart);
static void ELi_InitObject( ELObject* MYObject);
BOOL elRemoveObjEntry( ELObject** StartEnt, ELObject* ObjEnt);
static void elAddObjEntry( ELObject** StartEnt, ELObject* ObjEnt);

static BOOL ELi_ReInitDesc( ELDesc* elElfDesc);
static ELResult elLoadSegments( ELDesc* elElfDesc, ELObject* MYObject);
static ELResult elLoadSections( ELDesc* elElfDesc, ELObject* MYObject, u32 dll_fileid, u32 elf_num);

// ELFオブジェクトまたはそのアーカイブをバッファに再配置する
static ELDlld ELi_LoadLibrary( ELDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf, u32 buf_size, u32 dll_fileid);
// ELFオブジェクトをバッファに再配置するコア関数
static ELResult ELi_LoadObject( ELDesc* elElfDesc, ELObject* MYObject, void* obj_offset, void* buf, u32 dll_fileid, u32 elf_num);
// ELFオブジェクトからデータを読み出すスタブ関数
static BOOL ELi_ReadFile( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
static BOOL ELi_ReadMem( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
static BOOL ELi_ReadUsr( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
     
// アドレステーブルからエントリを削除する
//BOOL elRemoveAdrEntry( ELAdrEntry* AdrEnt);

// アドレステーブルにエントリを追加する
void elAddAdrEntry( ELAdrEntry** ELAdrEntStart, ELAdrEntry* AdrEnt);

// アドレステーブルから指定文字列に該当するエントリを探す
ELAdrEntry* elGetAdrEntry( ELDesc* elElfDesc, const char* ent_name, ELObject** ExpObjEnt);

// アドレステーブルを全部解放する
static void elFreeAdrTbl( ELAdrEntry** ELAdrEntStart);


/*---------------------------------------------------------
 ELFオブジェクトのサイズを求める
    
    buf : ELFイメージのアドレス
 --------------------------------------------------------*/
u32 EL_GetElfSize( const void* buf)
{
    Elf32_Ehdr  Ehdr;
    u32         size;
    
    if( ELF_LoadELFHeader( buf, &Ehdr) == NULL) {
        return 0;
    }
    size = (u32)(Ehdr.e_shoff + (Ehdr.e_shentsize * Ehdr.e_shnum));
    return size;
}


/*---------------------------------------------------------
 リンクされたライブラリのサイズを求める
 --------------------------------------------------------*/
u32 EL_GetLibSize( ELDlld my_dlld)
{
    if( my_dlld == 0) {
        return( 0);
    }
    return( ((ELObject*)my_dlld)->lib_size);
}


/*------------------------------------------------------
  ダイナミックリンクシステムを初期化する
 -----------------------------------------------------*/
#if 0
//#ifndef SDK_TWL
void ELi_Init( void)
{
    void* heap_start;

    if( i_el_initialized) {
        return;
    }

    i_el_initialized = TRUE;
  
    /*--- メモリアロケーション関係の設定 ---*/
    OS_InitArena();
    heap_start = OS_InitAlloc( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo( heap_start );
    EL_Heap = OS_CreateHeap( OS_ARENA_MAIN, heap_start, (void*)((u32)(OS_GetMainArenaHi())+1));
    OS_SetCurrentHeap( OS_ARENA_MAIN, EL_Heap);
    /*--------------------------------------*/

    OS_InitMutex( &i_el_mutex);
}
#else
void ELi_Init( ELAlloc alloc, ELFree free)
{
    i_elAlloc = alloc;
    i_elFree = free;
  
    if( i_el_initialized) {
        return;
    }

    i_el_initialized = TRUE;
  
    OS_InitMutex( &i_el_mutex);
}

void* ELi_Malloc( ELDesc* elElfDesc, ELObject* MYObject, size_t size)
{
    void* ptr;

    ptr = OSAPI_MALLOC( size);
    if( ptr == NULL) {
        ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_NO_MORE_RESOURCE);
        return( NULL);
    }
    return( ptr);
}
#endif

/*------------------------------------------------------
  ELDesc構造体を初期化する
 -----------------------------------------------------*/
BOOL ELi_InitDesc( ELDesc* elElfDesc)
{
    if( elElfDesc == NULL) {    /*NULLチェック*/
        return FALSE;
    }

    /*初期値の設定*/
    elElfDesc->ShdrEx   = NULL;
    elElfDesc->SymEx    = NULL;
    elElfDesc->SymExTbl = NULL;
    elElfDesc->SymExTarget = 0xFFFFFFFF;

    elElfDesc->process = (u32)EL_PROC_INITIALIZED;    /*フラグの設定*/
    elElfDesc->result  = (u32)EL_RESULT_SUCCESS;

    /**/
    elElfDesc->ELObjectStart = NULL;
    elElfDesc->ELStaticObj = NULL;
      
    return TRUE;
}

/*------------------------------------------------------
  オブジェクトが読み込み終わったらELDesc構造体を再初期化する
 -----------------------------------------------------*/
static BOOL ELi_ReInitDesc( ELDesc* elElfDesc)
{
    if( elElfDesc == NULL) {    /*NULLチェック*/
        return FALSE;
    }

    /*初期値の設定*/
    elElfDesc->ShdrEx   = NULL;
    elElfDesc->SymEx    = NULL;
    elElfDesc->SymExTbl = NULL;
    elElfDesc->SymExTarget = 0xFFFFFFFF;

    elElfDesc->process = (u32)EL_PROC_INITIALIZED;    /*フラグの設定*/
    elElfDesc->result  = (u32)EL_RESULT_SUCCESS;
      
    return TRUE;
}

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをバッファに再配置する
    
    elElfDesc : ヘッダ構造体
    ObjFile : OBJファイルまたはアーカイブファイルの構造体
    buf : ロードするバッファ
 -----------------------------------------------------*/
ELDlld EL_LoadLibraryfromFile( ELDesc* elElfDesc, const char* FilePath, void* buf, u32 buf_size)
{
    ELDlld   dlld;
    u32      len;
    FSFile   file[1];
    FSFileID file_id[1];

    if( elElfDesc == NULL) {
        return( 0);
    }
  
    OS_LockMutex( &i_el_mutex);

    FS_InitFile(file);
    
    if( !FS_OpenFileEx(file, FilePath, FS_FILEMODE_R) )
    {
        ELi_SetResultCode( elElfDesc, NULL, EL_RESULT_CANNOT_ACCESS_ELF);
        OS_UnlockMutex( &i_el_mutex);
        return 0;
    }

    if( !FS_ConvertPathToFileID( file_id, FilePath ) )
    {
        (void)FS_CloseFile(file);
        OS_UnlockMutex( &i_el_mutex);
        return 0;
    }

    /*リード関数の設定*/
    elElfDesc->i_elReadStub = ELi_ReadFile;
    elElfDesc->FileStruct = (int*)file;
    
    len = FS_GetFileLength( file );
    
    dlld = ELi_LoadLibrary( elElfDesc, NULL, len, buf, buf_size, file_id[0].file_id);
  
    if( dlld != 0) {
        ((ELObject*)dlld)->file_id = file_id[0].file_id;
    }

    if( !FS_CloseFile(file)) {
        ELi_SetResultCode( elElfDesc, NULL, EL_RESULT_CANNOT_ACCESS_ELF);
        dlld = 0;
    }

    OS_UnlockMutex( &i_el_mutex);
    return( dlld);
}

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをバッファに再配置する
    
    elElfDesc : ヘッダ構造体
    readfunc : OBJファイルまたはアーカイブファイルを読み出すユーザ関数
    buf : ロードするバッファ
 -----------------------------------------------------*/
ELDlld EL_LoadLibrary( ELDesc* elElfDesc, ELReadImage readfunc, u32 len, void* buf, u32 buf_size)
{
    ELDlld dlld;
  
    if( elElfDesc == NULL) {
        return( 0);
    }
  
    OS_LockMutex( &i_el_mutex);
  
    i_elReadImage = readfunc;
    elElfDesc->i_elReadStub = ELi_ReadUsr;

    dlld = ELi_LoadLibrary( elElfDesc, NULL, len, buf, buf_size, 0xFFFFFFFF);

    if( dlld != 0) {
        ((ELObject*)dlld)->file_id = 0xFFFFFFFF;
    }

    OS_UnlockMutex( &i_el_mutex);
    return( dlld);
}

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをバッファに再配置する
    
    elElfDesc : ヘッダ構造体
    obj_image : OBJファイルまたはアーカイブファイルのRAM上イメージアドレス
    buf : ロードするバッファ
 -----------------------------------------------------*/
ELDlld EL_LoadLibraryfromMem( ELDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf, u32 buf_size)
{
    ELDlld dlld;

    if( elElfDesc == NULL) {
        return( 0);
    }
  
    OS_LockMutex( &i_el_mutex);
  
    /*リード関数の設定*/
    elElfDesc->i_elReadStub = ELi_ReadMem;
    elElfDesc->FileStruct = NULL;
    
    dlld = ELi_LoadLibrary( elElfDesc, obj_image, obj_len, buf, buf_size, 0xFFFFFFFF);

    if( dlld != 0) {
        ((ELObject*)dlld)->file_id = 0xFFFFFFFF;
    }

    OS_UnlockMutex( &i_el_mutex);
    return( dlld);
}

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをバッファに再配置する
    
    elElfDesc : ヘッダ構造体
    obj_image : OBJファイルまたはアーカイブファイルのRAM上イメージアドレス
    buf : ロードするバッファ
 -----------------------------------------------------*/
static ELDlld ELi_LoadLibrary( ELDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf, u32 buf_size, u32 dll_fileid)
{
    ELResult   result;
    u32        image_pointer;
    u32        arch_size;
    u32        elf_num = 0;                /*ELFオブジェクトの数*/
    ArchHdr    ArHdr;
    char       OBJMAG[8];
    char       ELFMAG[4] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};
    ELObject*  MYObject;

    /*-------*/
    if( elElfDesc->i_elReadStub == NULL) {
        ELi_SetResultCode( elElfDesc, NULL, EL_RESULT_CANNOT_ACCESS_ELF);
        return( 0);
    }
    /*-------*/

    /* 個別オブジェクト管理構造体確保 */
    MYObject = (ELObject*)ELi_Malloc( elElfDesc, NULL, sizeof( ELObject));
    if(MYObject == NULL)
        return 0;
    ELi_InitObject( MYObject);
    elAddObjEntry( &(elElfDesc->ELObjectStart), MYObject);

    elElfDesc->ar_head = obj_image;
    image_pointer = 0;
    MYObject->lib_start = buf;
    MYObject->buf_current = buf;
    /* バッファチェック用限界アドレス設定 */
    if( buf_size > (0xFFFFFFFF - (u32)buf)) {
        MYObject->buf_limit_addr = 0xFFFFFFFF;
    }else{
        MYObject->buf_limit_addr = ((u32)buf + buf_size);
    }

    if( elElfDesc->i_elReadStub( OBJMAG, elElfDesc->FileStruct, (u32)obj_image, 0, 8) == FALSE) { /*OBJの文字列を取得*/
        ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_CANNOT_ACCESS_ELF);
        return( 0);
    }
    /*--------------- アーカイブファイルの場合 ---------------*/
    if( OSAPI_STRNCMP( OBJMAG, ARMAG, 8) == 0) {
        arch_size = sizeof( ArchHdr);
        image_pointer += 8;                /*最初のエントリへ*/
        
        while( image_pointer < obj_len) {
            if( elElfDesc->i_elReadStub( OBJMAG, elElfDesc->FileStruct, (u32)(obj_image), (image_pointer+arch_size), 4)
                == FALSE) {    /*OBJの文字列を取得*/
                ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_CANNOT_ACCESS_ELF);
                break;
            }
            if( OSAPI_STRNCMP( OBJMAG, ELFMAG, 4) == 0) {
                /*初期値の再設定*/
                (void)ELi_ReInitDesc( elElfDesc);
                result = ELi_LoadObject( elElfDesc, MYObject, (void*)(image_pointer+arch_size), MYObject->buf_current, dll_fileid, elf_num);
                if( result != EL_RESULT_SUCCESS) {
                    break;
                }
                elf_num++;
            }else{
            }
            /*次のエントリへ*/
            if( elElfDesc->i_elReadStub( &ArHdr, elElfDesc->FileStruct, (u32)(obj_image), image_pointer, arch_size)
                == FALSE) {
                ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_CANNOT_ACCESS_ELF);
                return( 0);
            }
            image_pointer += arch_size + AR_GetEntrySize( &ArHdr);
        }
    }else{/*--------------- ELFファイルの場合 ---------------*/
        if( OSAPI_STRNCMP( OBJMAG, ELFMAG, 4) == 0) {
            /*初期値の再設定*/
            (void)ELi_ReInitDesc( elElfDesc);
            result = ELi_LoadObject( elElfDesc, MYObject, 0, MYObject->buf_current, dll_fileid, elf_num);
            if( result != EL_RESULT_SUCCESS) {
            }else{
                elf_num++;
            }
        }else{
            ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_INVALID_ELF);
        }
    }
    /*-------------------------------------------------------*/

    if( elf_num) {
        if( OS_IsRunOnDebugger() == FALSE) {
            (void)ELi_FreeVenTbl( elElfDesc, MYObject);    /*非デバッグ時はベニヤのリンクリストを開放*/
        }
        MYObject->stat = elf_num;
        //(MEMO)Segmentをロードした場合はbuf_currentが進んでいないのでlib_sizeに反映されない
        MYObject->lib_size = ((u32)(MYObject->buf_current)) - ((u32)(MYObject->lib_start));
        PRINTDEBUG( "library size : 0x%x\n", MYObject->lib_size);
        return( (ELDlld)MYObject);
    }else{
        /* オブジェクトが処理できなかったので構造体破棄 */
        (void)elRemoveObjEntry( &(elElfDesc->ELObjectStart), MYObject);
        return 0; //NULL
    }
}

/*------------------------------------------------------
  ELFオブジェクトをバッファに再配置する
    
    elElfDesc : ヘッダ構造体
    MYObject : 個別のオブジェクト管理構造体
    obj_offset : OBJファイルのRAM上イメージアドレスからのオフセット
    buf : ロードするバッファ(TODO:バッファオーバーフロー対策)
 -----------------------------------------------------*/
static ELResult ELi_LoadObject( ELDesc* elElfDesc, ELObject* MYObject, void* obj_offset, void* buf, u32 dll_fileid, u32 elf_num)
{
    ELResult ret_val;
    
    /* ELDescの初期化チェック */
    if( elElfDesc->process != (u32)EL_PROC_INITIALIZED) {
        return( EL_RESULT_FAILURE);
    }
    /* ELFヘッダの取得 */
    if( elElfDesc->i_elReadStub( &(elElfDesc->CurrentEhdr), elElfDesc->FileStruct,
                                (u32)(elElfDesc->ar_head), (u32)(obj_offset), sizeof( Elf32_Ehdr))
        == FALSE) {
        ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_CANNOT_ACCESS_ELF);
        return( EL_RESULT_CANNOT_ACCESS_ELF);
    }

    /* ELDesc構造体の構築 */
    elElfDesc->elf_offset = obj_offset;
    elElfDesc->shentsize = elElfDesc->CurrentEhdr.e_shentsize;
    elElfDesc->entry_adr = elElfDesc->CurrentEhdr.e_entry;

    /* ELFファイルタイプ別に処理 */
    switch( elElfDesc->CurrentEhdr.e_type) {
        
      case ET_NONE:
        PRINTDEBUG( "ERROR : Elf type \"ET_NONE\"\n");
        ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_UNSUPPORTED_ELF);
        ret_val = EL_RESULT_UNSUPPORTED_ELF;
        break;
        
      case ET_REL:  /* 実行×、再配置○ */
        PRINTDEBUG( "Elf type \"ET_REL\"\n");
        if( buf == NULL) {        /* バッファのNULLチェック */
            ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_NO_MORE_RESOURCE);
            return EL_RESULT_NO_MORE_RESOURCE;
        }
        ret_val = elLoadSections( elElfDesc, MYObject, dll_fileid, elf_num);
        break;
        
      case ET_EXEC: /* 実行○、再配置× */
        PRINTDEBUG( "Elf type \"ET_EXEC\"\n");
        ret_val = elLoadSegments( elElfDesc, MYObject);
        break;
        
      case ET_DYN:  /* 実行○、再配置○ (TODO:未テスト)*/
        PRINTDEBUG( "Elf type \"ET_DYN\"\n");
        if( buf == NULL) { //ロードアドレスが指定されてないときはET_EXEC扱い
            ret_val = elLoadSegments( elElfDesc, MYObject);
        }else{             //ロードアドレスが指定されていればET_REL扱い
            ret_val = elLoadSections( elElfDesc, MYObject, dll_fileid, elf_num);
        }
        break;
        
      case ET_CORE:
        PRINTDEBUG( "ERROR : Elf type \"ET_CORE\"\n");
        ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_UNSUPPORTED_ELF);
        ret_val = EL_RESULT_UNSUPPORTED_ELF;
        break;
        
      default:
        PRINTDEBUG( "ERROR : Invalid Elf type 0x%x\n",
                    elElfDesc->CurrentEhdr.e_type);
        ELi_SetResultCode( elElfDesc, MYObject, EL_RESULT_INVALID_ELF);
        ret_val = EL_RESULT_INVALID_ELF;
        break;
    }

    return( ret_val);
}


/*------------------------------------------------------
  全セグメントを調べてコピーする
    
    elElfDesc : ヘッダ構造体
 -----------------------------------------------------*/
static ELResult elLoadSegments( ELDesc* elElfDesc, ELObject* MYObject)
{
    u16        i;
    //u32        load_start;
    Elf32_Phdr CurrentPhdr;
    
    for( i=0; i<(elElfDesc->CurrentEhdr.e_phnum); i++) {
        /*プログラムヘッダをコピー*/
        ELi_GetPhdr( elElfDesc, i, &CurrentPhdr);
        
        if( CurrentPhdr.p_type == PT_LOAD) {
            /*ロード可能セグメントならメモリにロード*/
            if( ELi_CopySegmentToBuffer( elElfDesc, MYObject, &CurrentPhdr) == NULL) {
                return( EL_RESULT_CANNOT_ACCESS_ELF);
            }
        }else{
            PRINTDEBUG( "WARNING : skip segment (type = 0x%x)\n",
                        CurrentPhdr.p_type);
        }
    }
    ELi_SetProcCode( elElfDesc, MYObject, EL_PROC_COPIED);
    return( EL_RESULT_SUCCESS);
}


/*------------------------------------------------------
  全セクションを調べてコピーする
    
    elElfDesc : ヘッダ構造体
 -----------------------------------------------------*/
static ELResult elLoadSections( ELDesc* elElfDesc, ELObject* MYObject, u32 dll_fileid, u32 elf_num)
{
    u16         i;
    ELShdrEx*   FwdShdrEx;
    ELShdrEx*   CurrentShdrEx;
    ELShdrEx*   InfoShdrEx;      //例えばCurrentShdrExがrel.textの場合.textをさす
    ELShdrEx    DmyShdrEx;
#if (DEBUG_PRINT_ON == 1)
    u16         j;
    u32         num_of_entry;
    char        sym_str[128];    //デバッグプリント用
    u32         offset;          //デバッグプリント用
#endif
    
    /*---------- ELShdrExのリストを作る ----------*/
    CurrentShdrEx = &DmyShdrEx;
    for( i=0; i<(elElfDesc->CurrentEhdr.e_shnum); i++) {
        CurrentShdrEx->next = ELi_Malloc( elElfDesc, MYObject, sizeof(ELShdrEx));
        if(CurrentShdrEx->next == NULL) {
            return( EL_RESULT_NO_MORE_RESOURCE);
        }
        CurrentShdrEx = (ELShdrEx*)(CurrentShdrEx->next);
        OSAPI_CPUFILL8( CurrentShdrEx, 0, sizeof(ELShdrEx));    //ゼロクリア
        
        /*デバッグ情報かどうかを判別してフラグをセット*/
        if( ELi_ShdrIsDebug( elElfDesc, i) == TRUE) {    /*デバッグ情報の場合*/
            CurrentShdrEx->debug_flag = 1;
        }else{                                           /*デバッグ情報でない場合*/
            /*セクションヘッダをコピー*/
            ELi_GetShdr( elElfDesc, i, &(CurrentShdrEx->Shdr));
            CurrentShdrEx->debug_flag = 0;
        }
    }
    CurrentShdrEx->next = NULL;
    elElfDesc->ShdrEx = DmyShdrEx.next;
    /*--------------------------------------------*/

    ELi_START_REGISTER_RELOCATION_INFO( ISTDRELOCATIONPROC_AUTO, (elElfDesc != i_eldesc_sim));

    /*---------- 全セクションを調べてコピーする ----------*/
    PRINTDEBUG( "\nLoad to RAM:\n");
    for( i=0; i<(elElfDesc->CurrentEhdr.e_shnum); i++) {
        /**/
        CurrentShdrEx = ELi_GetShdrExfromList( elElfDesc->ShdrEx, i);
//        PRINTDEBUG( "section:%d sh_flag=0x%x\n", i, CurrentShdrEx->Shdr.sh_flags);
//        PRINTDEBUG( "section:%d sh_type=0x%x\n", i, CurrentShdrEx->Shdr.sh_type);

        if( CurrentShdrEx->debug_flag == 1) {              /*デバッグ情報の場合*/
            PRINTDEBUG( "skip debug-section %02x\n", i);
        }else{                                             /*デバッグ情報でない場合*/
            BOOL bLocate = FALSE;
            
            /* .text section */
            if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //メモリにコピー
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( elElfDesc, MYObject, &(CurrentShdrEx->Shdr));
                bLocate = TRUE;
            }
            /* .data, .data1 section (初期化済みデータ) */
            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //メモリにコピー
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( elElfDesc, MYObject, &(CurrentShdrEx->Shdr));
                bLocate = TRUE;
            }
            /* .bss section */
            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_NOBITS)) {
                //コピーしない
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_AllocSectionToBuffer( elElfDesc, MYObject, &(CurrentShdrEx->Shdr));
                bLocate = TRUE;
            }
            /* .rodata, .rodata1 section */
            else if( (CurrentShdrEx->Shdr.sh_flags == SHF_ALLOC)&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //メモリにコピー
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( elElfDesc, MYObject, &(CurrentShdrEx->Shdr));
                bLocate = TRUE;
            }
            
            if (bLocate) {
                if( CurrentShdrEx->loaded_adr == NULL) { //ELi_CopySectionToBufferに失敗した場合
                    ELi_END_REGISTER_RELOCATION_INFO( ISTDRELOCATIONPROC_AUTO, (elElfDesc != i_eldesc_sim));
                    return( EL_RESULT_CANNOT_ACCESS_ELF);
                }
                (void)ELi_REGISTER_RELOCATION_INFO( ISTDRELOCATIONPROC_AUTO, dll_fileid, elf_num, i, CurrentShdrEx->loaded_adr, (elElfDesc != i_eldesc_sim));
            }

            PRINTDEBUG( "section %02x relocated at %08x\n",
                        i, CurrentShdrEx->loaded_adr);
        }
    }
    /* コピー終了後 */
    ELi_SetProcCode( elElfDesc, MYObject, EL_PROC_COPIED);
    /*----------------------------------------------------*/

    ELi_END_REGISTER_RELOCATION_INFO( ISTDRELOCATIONPROC_AUTO, (elElfDesc != i_eldesc_sim));

    /*---------- グローバルシンボルの公開とローカルシンボルの再配置 ----------*/
    PRINTDEBUG( "\nRelocate Symbols:\n");
    for( i=0; i<(elElfDesc->CurrentEhdr.e_shnum); i++) {
        /**/
        CurrentShdrEx = ELi_GetShdrExfromList( elElfDesc->ShdrEx, i);
        
        if( CurrentShdrEx->debug_flag == 1) {                /*デバッグ情報の場合*/
        }else{                                               /*デバッグ情報でない場合*/

            if( CurrentShdrEx->Shdr.sh_type == SHT_REL) {
                /*リロケート*/
                InfoShdrEx = ELi_GetShdrExfromList( elElfDesc->ShdrEx,
                                                    CurrentShdrEx->Shdr.sh_info);
                if( InfoShdrEx->loaded_adr != 0) { //対象セクションがロードされていれば内部を再配置
                    if(ELi_RelocateSym( elElfDesc, MYObject, i) == FALSE)
                        return( (ELResult)(elElfDesc->result)); //EL_RESULT_NO_MORE_RESOURCE or EL_RESULT_UNSUPPORTED_ELF
                }
#if (DEBUG_PRINT_ON == 1)
                num_of_entry = (CurrentShdrEx->Shdr.sh_size) /
                                (CurrentShdrEx->Shdr.sh_entsize);

                PRINTDEBUG( "num of REL = %x\n", num_of_entry);
                PRINTDEBUG( "Section Header Info.\n");
                PRINTDEBUG( "link   : %x\n", CurrentShdrEx->Shdr.sh_link);
                PRINTDEBUG( "info   : %x\n", CurrentShdrEx->Shdr.sh_info);
                PRINTDEBUG( " Offset     Info    Type            Sym.Value  Sym.Name\n");
                offset = 0;
                for( j=0; j<num_of_entry; j++) {
                    ELi_GetSent( elElfDesc, i, &(elElfDesc->Rel), offset, sizeof(Elf32_Rel));
                    ELi_GetShdr( elElfDesc, CurrentShdrEx->Shdr.sh_link, &(elElfDesc->SymShdr));
                    ELi_GetSent( elElfDesc, CurrentShdrEx->Shdr.sh_link, &(elElfDesc->Sym),
                                 (u32)(elElfDesc->SymShdr.sh_entsize * ELF32_R_SYM( elElfDesc->Rel.r_info)), sizeof(Elf32_Sym));
                    ELi_GetStrAdr( elElfDesc, elElfDesc->SymShdr.sh_link, elElfDesc->Sym.st_name, sym_str, 128);
                
                    PRINTDEBUG( "%08x  ", elElfDesc->Rel.r_offset);
                    PRINTDEBUG( "%08x ", elElfDesc->Rel.r_info);
                    PRINTDEBUG( "                  ");
                    PRINTDEBUG( "%08x ", elElfDesc->Sym.st_value);
                    PRINTDEBUG( sym_str);
                    PRINTDEBUG( "\n");
                    /*次のエントリへ*/
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
#endif
            }
            else if( CurrentShdrEx->Shdr.sh_type == SHT_RELA) {
                /*リロケート*/
                InfoShdrEx = ELi_GetShdrExfromList( elElfDesc->ShdrEx,
                                                    CurrentShdrEx->Shdr.sh_info);
                if( InfoShdrEx->loaded_adr != 0) { //対象セクションがロードされていれば内部を再配置
                    if(ELi_RelocateSym( elElfDesc, MYObject, i) == FALSE)
                        return( EL_RESULT_NO_MORE_RESOURCE);
                }
                
#if (DEBUG_PRINT_ON == 1)
                num_of_entry = (CurrentShdrEx->Shdr.sh_size) /
                                (CurrentShdrEx->Shdr.sh_entsize);
                PRINTDEBUG( "num of RELA = %x\n", num_of_entry);
                PRINTDEBUG( "Section Header Info.\n");
                PRINTDEBUG( "link   : %x\n", CurrentShdrEx->Shdr.sh_link);
                PRINTDEBUG( "info   : %x\n", CurrentShdrEx->Shdr.sh_info);
                PRINTDEBUG( " Offset     Info    Type            Sym.Value  Sym.Name\n");
                offset = 0;
                for( j=0; j<num_of_entry; j++) {
                    ELi_GetSent( elElfDesc, i, &(elElfDesc->Rela), offset, sizeof(Elf32_Rel));
                    ELi_GetShdr( elElfDesc, CurrentShdrEx->Shdr.sh_link, &(elElfDesc->SymShdr));
                    ELi_GetSent( elElfDesc, CurrentShdrEx->Shdr.sh_link, &(elElfDesc->Sym),
                                 (u32)(elElfDesc->SymShdr.sh_entsize * ELF32_R_SYM( elElfDesc->Rela.r_info)), sizeof(Elf32_Sym));
                    ELi_GetStrAdr( elElfDesc, elElfDesc->SymShdr.sh_link, elElfDesc->Sym.st_name, sym_str, 128);
                
                    PRINTDEBUG( "%08x  ", elElfDesc->Rela.r_offset);
                    PRINTDEBUG( "%08x ", elElfDesc->Rela.r_info);
                    PRINTDEBUG( "                  ");
                    PRINTDEBUG( "%08x ", elElfDesc->Sym.st_value);
                    PRINTDEBUG( sym_str);
                    PRINTDEBUG( "\n");
                    /*次のエントリへ*/
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
#endif
            }
            else if( CurrentShdrEx->Shdr.sh_type == SHT_SYMTAB) {
                /*グローバルシンボルをアドレステーブルに登録*/
                if(ELi_GoPublicGlobalSym( elElfDesc, MYObject, i) == FALSE)
                    return( EL_RESULT_NO_MORE_RESOURCE);
            }
        }
    }
    /*ELi_RelocateSymやELi_GoPublicGlobalSym内で作成&使い回したシンボルリストを開放*/
    ELi_FreeSymList( elElfDesc);

    /*------- ELShdrExのリストを解放する -------*/
    CurrentShdrEx = elElfDesc->ShdrEx;
    if( CurrentShdrEx) {
        do {
            FwdShdrEx = CurrentShdrEx;
            CurrentShdrEx = CurrentShdrEx->next;
            OSAPI_FREE( FwdShdrEx);
        }while( CurrentShdrEx != NULL);
        elElfDesc->ShdrEx = NULL;
    }
    /*-----------------------------------------*/

    /*RAM上のDLLが呼ばれる前にキャッシュをフラッシュ*/
#if (TARGET_ARM_V5 == 1)
    OSAPI_FLUSHCACHEALL();
    OSAPI_WAITCACHEBUF();
#endif
    
    return( EL_RESULT_SUCCESS);
}

/*------------------------------------------------------
  未解決のシンボルがあればアドレステーブルを使って解決する
 -----------------------------------------------------*/
ELProcess ELi_ResolveAllLibrary( ELDesc* elElfDesc)
{
    ELAdrEntry*       AdrEnt;
    ELImportEntry*    UnrEnt;
    ELImportEntry*    NextEnt;
//    ELImportEntry*    CurrentUnrEnt;
//    ELImportEntry*    FwdUnrEnt;
    ELObject*         ObjEnt;
    ELObject*         ExpObjEnt;
    BOOL              ret_val;
  
    if( elElfDesc == NULL) {
        return( EL_PROC_NOTHING);
    }
    ObjEnt = elElfDesc->ELObjectStart;
    if( (ObjEnt == NULL)||((ObjEnt == elElfDesc->ELStaticObj)&&(ObjEnt->next == NULL))) {
        return( EL_PROC_NOTHING); /* dllが無い場合 */
    }
    while( ObjEnt != NULL) {
        UnrEnt = (ELImportEntry*)ObjEnt->UnresolvedImportAdrEnt;
        PRINTDEBUG( "\nResolve all symbols:\n");
        while( UnrEnt != NULL) {
            NextEnt = UnrEnt->next;
            AdrEnt = elGetAdrEntry( elElfDesc, UnrEnt->sym_str, &ExpObjEnt);        /*アドレステーブルから検索*/
            if( AdrEnt) {                                    /*アドレステーブルから見つかった場合*/
                UnrEnt->S_ = (u32)(AdrEnt->adr);
                UnrEnt->T_ = (u32)(AdrEnt->thumb_flag);
                PRINTDEBUG( "\n symbol found %s : %8x\n", UnrEnt->sym_str, UnrEnt->S_);
                ret_val = ELi_DoRelocate( elElfDesc, ObjEnt, UnrEnt);           /*シンボル解決*/
                if( ret_val == FALSE) {
                    return( (ELProcess)(ObjEnt->process)); //EL_PROC_COPIEDのはず。osPanicの方がいい? TODO続行する
                }else{
                    PRINTDEBUG( "%s ... ObjEnt:0x%x, ExpObjEnt:0x%x\n", UnrEnt->sym_str, ObjEnt, ExpObjEnt);
                    UnrEnt->Dlld = (struct ELObject*)ExpObjEnt;           /*解決に参照したObjEntを記録*/
                    /* UnresolvedからResolvedに繋ぎかえる */
                    (void)ELi_ExtractImportEntry( &(ObjEnt->UnresolvedImportAdrEnt), UnrEnt);
                    ELi_AddImportEntry( &(ObjEnt->ResolvedImportAdrEnt), UnrEnt);
                }
            }else{                                           /*アドレステーブルから見つからなかった場合*/
                if( elElfDesc != i_eldesc_sim) {
                    PRINTDEBUG( "\n ERROR! cannot find symbol : %s\n\n", UnrEnt->sym_str);
                    return( (ELProcess)(ObjEnt->process)); //EL_PROC_COPIEDのはず。 osPanicの方がいい? TODO:続行する
                }
                /*----- simulationの場合は強引に解決したつもりにする -----*/
                UnrEnt->S_ = (u32)0;
                UnrEnt->T_ = (u32)0;
                ret_val = ELi_DoRelocate( elElfDesc, ObjEnt, UnrEnt);
                UnrEnt->Dlld = (struct ELObject*)NULL;           /*解決に参照したObjEntを記録*/
                /* UnresolvedからResolvedに繋ぎかえる */
                (void)ELi_ExtractImportEntry( &(ObjEnt->UnresolvedImportAdrEnt), UnrEnt);
                ELi_AddImportEntry( &(ObjEnt->ResolvedImportAdrEnt), UnrEnt);
                /*--------------------------------------------------------*/
            }
            UnrEnt = NextEnt;                           /*次の未解決エントリへ*/
        }
        ELi_SetProcCode( NULL, ObjEnt, EL_PROC_RELOCATED);
        ObjEnt->lib_size = ((u32)(ObjEnt->buf_current)) - ((u32)(ObjEnt->lib_start)); //veneerが追加されている可能性があるのでサイズを更新
        ObjEnt = ObjEnt->next;
    }
    ELi_SetProcCode( elElfDesc, NULL, EL_PROC_RELOCATED);

    /*TODO:Fixedのとき、ここで解決できなかったUnrEntエントリだけ削除するべき*/
#if 0    
    /*--- ELImportEntryのリストを解放する ---*/
    ELi_FreeImportTbl( &ELUnrEntStart);
    /*-------------------------------------------*/
#endif
    /*ベニヤのリンクリストを開放*/
//    (void)ELi_FreeVenTbl();

    /*RAM上のDLLが呼ばれる前にキャッシュをフラッシュ*/
#if (TARGET_ARM_V5 == 1)
    OSAPI_FLUSHCACHEALL();
    OSAPI_WAITCACHEBUF();
#endif
    
    return EL_PROC_RELOCATED;
}


/*------------------------------------------------------
  オブジェクトをアンリンクする
 -----------------------------------------------------*/
BOOL ELi_Unlink( ELDesc* elElfDesc, ELDlld my_dlld)
{
    ELObject*      MYObject;
    ELObject*      ObjEnt;
    ELImportEntry* ResEnt;
    ELImportEntry* NextEnt;
    u32            file_id;

    if( elElfDesc == NULL) {
        return( FALSE);
    }
  
    if( my_dlld == 0) {
        ELi_SetResultCode( elElfDesc, NULL, EL_RESULT_INVALID_PARAMETER);
        return( FALSE);
    }

    /* アンリンクするオブジェクト */
    MYObject = (ELObject*)my_dlld;

    /* アンリンクするオブジェクトのファイルIDを取得 */
    file_id = MYObject->file_id;

#if 0
    /* エクスポート情報を全て隠す */
    if( MYObject->ExportAdrEnt) {
        MYObject->HiddenAdrEnt = MYObject->ExportAdrEnt;
        MYObject->ExportAdrEnt = NULL;
    }
#else
    /* エクスポート情報を全て破棄する */
    elFreeAdrTbl( &(MYObject->ExportAdrEnt));
#endif

    /* 他のオブジェクトが参照して解決している部分を未解決にする */
    ObjEnt = elElfDesc->ELObjectStart;
    while( ObjEnt != NULL) {
        ResEnt = ObjEnt->ResolvedImportAdrEnt;
        while( ResEnt != NULL) {
            NextEnt = ResEnt->next;
            PRINTDEBUG( "Compare ObjEnt:0x%x, ExpObjEnt:0x%x\n", ResEnt->Dlld, MYObject);
            if( ResEnt->Dlld == (struct ELObject*)MYObject) {
                PRINTDEBUG( "Unlink from ObjEnt:0x%x, ExpObjEnt:0x%x\n", ObjEnt, MYObject);
                /* ResolvedからUnresolvedに繋ぎかえる */
                (void)ELi_ExtractImportEntry( &(ObjEnt->ResolvedImportAdrEnt), ResEnt);
                ELi_AddImportEntry( &(ObjEnt->UnresolvedImportAdrEnt), ResEnt);
                ResEnt->Dlld = NULL;
            }
            ResEnt = NextEnt;
        }
        ObjEnt = ObjEnt->next;
    }

    /*インポート情報の破棄や自分自身の破棄はしない*/


    /*ベニヤテーブルを破棄する*/
    (void)ELi_FreeVenTbl( elElfDesc, MYObject);    /*ベニヤのリンクリストを開放*/

    ELi_SetProcCode( elElfDesc, MYObject, EL_PROC_NOTHING);

    ELi_START_REGISTER_RELOCATION_INFO( ISTDRELOCATIONPROC_AUTO, (elElfDesc != i_eldesc_sim));
    (void)ELi_UNREGISTER_RELOCATION_INFO( ISTDRELOCATIONPROC_AUTO, file_id, (elElfDesc != i_eldesc_sim));
    ELi_END_REGISTER_RELOCATION_INFO( ISTDRELOCATIONPROC_AUTO, (elElfDesc != i_eldesc_sim));

    return( TRUE);
}


/*------------------------------------------------------
  オブジェクトのリストを全部解放する
 -----------------------------------------------------*/
static void ELi_FreeObject( ELObject** ELObjEntStart)
{
    ELObject* FwdObjEnt;
    ELObject* CurrentObjEnt;

    /*--- ELObjectのリストを開放する ---*/
    CurrentObjEnt = (*ELObjEntStart);
    if( CurrentObjEnt) {
        do {
            FwdObjEnt = CurrentObjEnt;
            CurrentObjEnt = CurrentObjEnt->next;
            /**/
            elFreeAdrTbl( &(FwdObjEnt->ExportAdrEnt));
            elFreeAdrTbl( &(FwdObjEnt->HiddenAdrEnt));
            ELi_FreeImportTbl( &(FwdObjEnt->ResolvedImportAdrEnt));
            ELi_FreeImportTbl( &(FwdObjEnt->UnresolvedImportAdrEnt));
            OSAPI_FREE( FwdObjEnt);
        }while( CurrentObjEnt != NULL);
        (*ELObjEntStart) = NULL;
    }
    /*----------------------------------*/
}


/*------------------------------------------------------
  オブジェクトに未解決な外部参照が残っていないかを調べる
 -----------------------------------------------------*/
BOOL EL_IsResolved( ELDlld my_dlld)
{
    ELObject* MYObject;

    if( my_dlld == 0) {
        return( FALSE);
    }
    
    MYObject = (ELObject*)my_dlld;
    /*TODO:MYObjectがリストに繋がっているか調べる*/
    if( (MYObject->stat != 0)&&(MYObject->UnresolvedImportAdrEnt == NULL)) {
        return( TRUE);
    }else{
        return( FALSE);
    }
}


/*------------------------------------------------------
  オブジェクト管理構造体を初期化する
 -----------------------------------------------------*/
static void ELi_InitObject( ELObject* MYObject)
{
    MYObject->next = NULL;
    MYObject->lib_size = 0;
    MYObject->ExportAdrEnt = NULL;
    MYObject->HiddenAdrEnt = NULL;
    MYObject->ResolvedImportAdrEnt = NULL;
    MYObject->UnresolvedImportAdrEnt = NULL;
    MYObject->ELVenEntStart = NULL;
    MYObject->ELV4tVenEntStart = NULL;
    MYObject->stat = 0;

    MYObject->process = (u32)EL_PROC_INITIALIZED;
    MYObject->result = (u32)EL_RESULT_SUCCESS;
}


/*------------------------------------------------------
  オブジェクトテーブルからエントリを削除する
 -----------------------------------------------------*/
BOOL elRemoveObjEntry( ELObject** StartEnt, ELObject* ObjEnt)
{
    ELObject  DmyObjEnt;
    ELObject* CurObjEnt;

    DmyObjEnt.next = (*StartEnt);
    CurObjEnt      = &DmyObjEnt;

    while( CurObjEnt->next != ObjEnt) {
        if( CurObjEnt->next == NULL) {
            return FALSE;
        }else{
            CurObjEnt = (ELObject*)CurObjEnt->next;
        }
    }

    /*リンクリストの繋ぎ直し*/
    CurObjEnt->next = ObjEnt->next;
    (*StartEnt) = DmyObjEnt.next;

    /*開放*/
    elFreeAdrTbl( &(ObjEnt->ExportAdrEnt));
    elFreeAdrTbl( &(ObjEnt->HiddenAdrEnt));
    ELi_FreeImportTbl( &(ObjEnt->ResolvedImportAdrEnt));
    ELi_FreeImportTbl( &(ObjEnt->UnresolvedImportAdrEnt));
    ELi_InitObject( ObjEnt); //stat = 0;
    OSAPI_FREE( ObjEnt);
    
    return TRUE;
}

/*------------------------------------------------------
  オブジェクトテーブルにエントリを追加する
 -----------------------------------------------------*/
static void elAddObjEntry( ELObject** StartEnt, ELObject* ObjEnt)
{
    ELObject  DmyObjEnt;
    ELObject* CurObjEnt;

    if( (*StartEnt) == NULL) {
        (*StartEnt) = ObjEnt;
    }else{
        DmyObjEnt.next = (*StartEnt);
        CurObjEnt      = &DmyObjEnt;

        while( CurObjEnt->next != NULL) {
            CurObjEnt = (ELObject*)CurObjEnt->next;
        }
        CurObjEnt->next = (void*)ObjEnt;
    }
    ObjEnt->next = NULL;
}


/*------------------------------------------------------
  アドレステーブルからエントリを削除する
 -----------------------------------------------------*/
#if 0
BOOL elRemoveAdrEntry( ELAdrEntry* AdrEnt)
{
    ELAdrEntry  DmyAdrEnt;
    ELAdrEntry* CurrentAdrEnt;

    DmyAdrEnt.next = ELAdrEntStart;
    CurrentAdrEnt = &DmyAdrEnt;

    while( CurrentAdrEnt->next != AdrEnt) {
        if( CurrentAdrEnt->next == NULL) {
            return FALSE;
        }else{
            CurrentAdrEnt = (ELAdrEntry*)CurrentAdrEnt->next;
        }
    }

    /*リンクリストの繋ぎ直し*/
    CurrentAdrEnt->next = AdrEnt->next;
    ELAdrEntStart = DmyAdrEnt.next;

    /*開放*/
    OSAPI_FREE( AdrEnt);
    
    return TRUE;
}
#endif

/*------------------------------------------------------
  アドレステーブルにエントリを追加する
 -----------------------------------------------------*/
void elAddAdrEntry( ELAdrEntry** ELAdrEntStart, ELAdrEntry* AdrEnt)
{
    ELAdrEntry  DmyAdrEnt;
    ELAdrEntry* CurrentAdrEnt;

    if( (*ELAdrEntStart) == NULL) {
        (*ELAdrEntStart) = AdrEnt;
    }else{
        DmyAdrEnt.next = (*ELAdrEntStart);
        CurrentAdrEnt = &DmyAdrEnt;

        while( CurrentAdrEnt->next != NULL) {
            CurrentAdrEnt = (ELAdrEntry*)CurrentAdrEnt->next;
        }
        CurrentAdrEnt->next = (void*)AdrEnt;
    }
    AdrEnt->next = NULL;
}

/*------------------------------------------------------
  アプリケーションからアドレステーブルにエントリを追加する
 -----------------------------------------------------*/
BOOL ELi_Export( ELDesc* elElfDesc, ELAdrEntry* AdrEnt)
{
    ELAdrEntry  DmyAdrEnt;
    ELAdrEntry* CurrentAdrEnt;

    if( elElfDesc == NULL) {
        return( FALSE);
    }
    if( elElfDesc->ELStaticObj == NULL) {
        elElfDesc->ELStaticObj = (ELObject*)ELi_Malloc( elElfDesc, NULL, sizeof( ELObject));
        if(elElfDesc->ELStaticObj == NULL) {
            return( FALSE);
        }
        ELi_InitObject( elElfDesc->ELStaticObj);
        elAddObjEntry( &(elElfDesc->ELObjectStart), elElfDesc->ELStaticObj);
    }
  
    if( !(elElfDesc->ELStaticObj->ExportAdrEnt)) {
        elElfDesc->ELStaticObj->ExportAdrEnt = (ELAdrEntry*)AdrEnt;
    }else{
        DmyAdrEnt.next = (ELAdrEntry*)elElfDesc->ELStaticObj->ExportAdrEnt;
        CurrentAdrEnt = &DmyAdrEnt;

        while( CurrentAdrEnt->next != NULL) {
            CurrentAdrEnt = (ELAdrEntry*)CurrentAdrEnt->next;
        }
        CurrentAdrEnt->next = (void*)AdrEnt;
    }
    AdrEnt->next = NULL;
    return( TRUE);
}

/*------------------------------------------------------
  アドレステーブルにスタティック側のエントリを追加する
 -----------------------------------------------------*/
#ifndef SDK_TWL
SDK_WEAK_SYMBOL void EL_AddStaticSym( void)
//__declspec(weak) void EL_AddStaticSym( void)
#else
SDK_WEAK_SYMBOL void EL_AddStaticSym( void)
#endif
{
    PRINTDEBUG( "please link file which is generated by \"makelst\".\n");
    while( 1) {};
}

/*------------------------------------------------------
  アドレステーブル全体から指定文字列に該当するエントリを返し、
  そのエントリをエクスポートしているオブジェクトをExpObjEntにセットする
 -----------------------------------------------------*/
ELAdrEntry* elGetAdrEntry( ELDesc* elElfDesc, const char* ent_name, ELObject** ExpObjEnt)
{
    ELObject*   ObjEnt;
    ELAdrEntry* CurrentAdrEnt;

    ObjEnt = elElfDesc->ELObjectStart;
    while( ObjEnt != NULL) {
        CurrentAdrEnt = ObjEnt->ExportAdrEnt;
        while( CurrentAdrEnt != NULL) {
            if( OSAPI_STRCMP( CurrentAdrEnt->name, ent_name) == 0) {
                (*ExpObjEnt) = ObjEnt; //エクスポートしているObjを特定
                goto get_end;
            }
            CurrentAdrEnt = (ELAdrEntry*)CurrentAdrEnt->next;
        }
        ObjEnt = ObjEnt->next;
    }
    (*ExpObjEnt) = NULL; //誰もエクスポートしていない
get_end:
    return CurrentAdrEnt;
}

/*------------------------------------------------------
  アドレステーブルから指定文字列に該当するアドレスを返す
 -----------------------------------------------------*/
void* ELi_GetGlobalAdr( ELDesc* elElfDesc, ELDlld my_dlld, const char* ent_name)
{
    u32         adr;
    ELAdrEntry* CurrentAdrEnt;
    ELObject*   DmyObjEnt;

    if( elElfDesc == NULL) {
        return( 0);
    }
  
    if( my_dlld == 0) { /*全体から探す*/
        CurrentAdrEnt = elGetAdrEntry( elElfDesc, ent_name, &DmyObjEnt);
    }else{              /*特定のオブジェクトから探す*/
        CurrentAdrEnt = ((ELObject*)my_dlld)->ExportAdrEnt;
        while( CurrentAdrEnt != NULL) {
            if( OSAPI_STRCMP( CurrentAdrEnt->name, ent_name) == 0) {
                break;
            }
            CurrentAdrEnt = (ELAdrEntry*)CurrentAdrEnt->next;
        }
    }

    if( CurrentAdrEnt) {
        if( CurrentAdrEnt->thumb_flag) { //TODO:func_flagのときのみ
            adr = (u32)(CurrentAdrEnt->adr) + 1;
        }else{
            adr = (u32)(CurrentAdrEnt->adr);
        }
    }else{
        adr = 0;
    }

    return (void*)(adr);
}


/*------------------------------------------------------
  アドレステーブルを全部解放する
 -----------------------------------------------------*/
static void elFreeAdrTbl( ELAdrEntry** ELAdrEntStart)
{
    ELAdrEntry*    FwdAdrEnt;
    ELAdrEntry*    CurrentAdrEnt;
    
    /*--- ELAdrEntryのリストを解放する ---*/
    CurrentAdrEnt = (*ELAdrEntStart);
    if( CurrentAdrEnt) {
        do {
            FwdAdrEnt = CurrentAdrEnt;
            CurrentAdrEnt = CurrentAdrEnt->next;
            OSAPI_FREE( FwdAdrEnt->name);        //シンボル名文字列
            OSAPI_FREE( FwdAdrEnt);              //構造体自身
        }while( CurrentAdrEnt != NULL);
        (*ELAdrEntStart) = NULL;
    }
    /*------------------------------------*/
}


/*------------------------------------------------------
  ELFオブジェクトからデータを読み出すスタブ
 -----------------------------------------------------*/
static BOOL ELi_ReadFile( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size)
{
#pragma unused( file_base)
    if( FS_SeekFile( file_struct, (s32)(file_offset), FS_SEEK_SET) == FALSE) {
        return( FALSE);
    }
    if( FS_ReadFile( file_struct, buf, (s32)(size)) == (s32)size) {
        return( TRUE);
    }else{
        return( FALSE);
    }
}

static BOOL ELi_ReadMem( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size)
{
#pragma unused(file_struct)
    OSAPI_CPUCOPY8( (void*)(file_base + file_offset),
                    buf,
                    size);
    return( TRUE);
}

static BOOL ELi_ReadUsr( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size)
{
#pragma unused(file_struct)
#pragma unused(file_base)
    if( i_elReadImage( file_offset, buf, size) == 0) {
        return( TRUE);
    }
    return( FALSE);
}


/*------------------------------------------------------
  エラーコード/プロセスコードのセット
  ELResultは初期値がSUCCESS, エラー発生時にセットする
  ELProcessは初期値がINITIALIZED, 進むに従ってセットする
 -----------------------------------------------------*/
void ELi_SetResultCode( ELDesc* elElfDesc, ELObject* MYObject, ELResult result)
{
    if( elElfDesc != NULL) {
        elElfDesc->result = (u32)result;
    }
    if( MYObject != NULL) {
        MYObject->result = (u32)result;
    }
}

void ELi_SetProcCode( ELDesc* elElfDesc, ELObject* MYObject, ELProcess process)
{
    if( elElfDesc != NULL) {
        elElfDesc->process = (u32)process;
    }
    if( MYObject != NULL) {
        MYObject->process = (u32)process;
    }
}

