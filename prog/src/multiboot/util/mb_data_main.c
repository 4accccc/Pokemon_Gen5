//======================================================================
/**
 *
 * @file  mb_data_main.c
 * @brief セーブデータを読み込むところ
 *      基本的にバージョンごとの処理を分けるラッパー
 * @author  ariizumi
 * @data  09/11/20
 */
//======================================================================
#include <gflib.h>
#include <string.h>
#include <nnsys.h>
#include <backup_system.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "savedata_def.h"
#include "system\machine_use.h"

#include "multiboot/mb_data_main.h"
#include "./mb_data_pt.h"
#include "./mb_data_gs.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  enum
//======================================================================

//======================================================================
//  typedef struct
//======================================================================

//======================================================================
//  proto
//======================================================================

MB_DATA_WORK* MB_DATA_InitSystem( int heapID );
static void MB_DATA_InitCardSystem( MB_DATA_WORK *dataWork );
void        MB_DATA_TermSystem( MB_DATA_WORK *dataWork );

BOOL  MB_DATA_LoadDataFirst( MB_DATA_WORK *dataWork );
BOOL  MB_DATA_SaveData( MB_DATA_WORK *dataWork );
u8  MB_DATA_GetErrorState( MB_DATA_WORK *dataWork );

BOOL  MB_DATA_IsFinishSaveFirst( MB_DATA_WORK *dataWork );
BOOL  MB_DATA_IsFinishSaveSecond( MB_DATA_WORK *dataWork );
void  MB_DATA_PermitLastSaveFirst( MB_DATA_WORK *dataWork );
void  MB_DATA_PermitLastSaveSecond( MB_DATA_WORK *dataWork );

//初期化
MB_DATA_WORK* MB_DATA_InitSystem( int heapID )
{
  MB_DATA_WORK *dataWork;

  dataWork = GFL_HEAP_AllocClearMemory(  heapID, sizeof( MB_DATA_WORK ) );

  dataWork->heapId = heapID;

  dataWork->mainSeq = 0;
  dataWork->subSeq   = 0;
  dataWork->pData = NULL;
  dataWork->pDataMirror  = NULL;
  dataWork->pDataCrcCheck = NULL;
  dataWork->pBoxData = NULL;
  dataWork->pItemData = NULL;
  dataWork->pMysteryData = NULL;
  dataWork->errorState = DES_NONE;
  dataWork->cardType = CARD_TYPE_INVALID;
  dataWork->isFinishSaveFirst = FALSE;
  dataWork->isFinishSaveSecond = FALSE;
  dataWork->permitLastSaveFirst = FALSE;
  dataWork->permitLastSaveSecond = FALSE;
  dataWork->isDummyCard = FALSE;
  MATH_CRC16CCITTInitTable( &dataWork->crcTable_ ); //CRC初期化
  
  MB_DATA_InitCardSystem( dataWork );
  
  {
    //ROMの識別を行う
    CARDRomHeader *headerData;
    s32 lockID = OS_GetLockID();
    BOOL isPullCard = FALSE;
    
    GF_ASSERT( lockID != OS_LOCK_ID_ERROR );
    CARD_LockRom( (u16)lockID );
    isPullCard = CARD_IsPulledOut();
    //CARD_CheckPulledOut();  //抜き検出
    if( isPullCard == FALSE )
    {
      headerData = (CARDRomHeader*)CARD_GetRomHeader();
    }
    CARD_UnlockRom( (u16)lockID );
    OS_ReleaseLockID( (u16)lockID );
    
    if( isPullCard == FALSE )
    {
      MB_DATA_TPrintf("RomName[%s]\n",headerData->game_name);
      
      if( STD_CompareString( headerData->game_name , "POKEMON D" ) == 0 ||
        STD_CompareString( headerData->game_name , "POKEMON P" ) == 0 )
      {
        dataWork->cardType = CARD_TYPE_DP;
      }
      else if( STD_CompareString( headerData->game_name , "POKEMON PL" ) == 0 )
      {
        dataWork->cardType = CARD_TYPE_PT;
      }
      else if( STD_CompareString( headerData->game_name , "POKEMON HG" ) == 0 ||
        STD_CompareString( headerData->game_name , "POKEMON SS" ) == 0 )
      {
        dataWork->cardType = CARD_TYPE_GS;
      }
  #if PM_DEBUG
      else if( STD_CompareString( headerData->game_name , "NINTENDO    NTRJ01" ) == 0 ||
               STD_CompareString( headerData->game_name , "SKEL" ) == 0 ||
               STD_CompareString( headerData->game_name , "dlplay" ) == 0 ||
               STD_CompareString( headerData->game_name , "SYACHI_MB" ) == 0 )
      {
        //MBでバグROMかsrl直起動
        dataWork->isDummyCard = TRUE;
        dataWork->cardType = CARD_TYPE_DUMMY;
      }
  #endif
    }
  }
  
  return dataWork;
}

//開放
void  MB_DATA_TermSystem( MB_DATA_WORK *dataWork )
{
  if( dataWork->pData != NULL )
  {
    GFL_HEAP_FreeMemory( dataWork->pData );
  }
  if( dataWork->pDataMirror != NULL )
  {
    GFL_HEAP_FreeMemory( dataWork->pDataMirror );
  }
  if( dataWork->pDataCrcCheck != NULL )
  {
    GFL_HEAP_FreeMemory( dataWork->pDataCrcCheck );
  }

  GFL_HEAP_FreeMemory( dataWork );
}

//セーブ触る前の初期化
void  MB_DATA_ResetSaveLoad( MB_DATA_WORK *dataWork )
{
  dataWork->mainSeq = 0;
  dataWork->subSeq   = 0;
  dataWork->isFinishSaveFirst = FALSE;
  dataWork->isFinishSaveSecond = FALSE;
  dataWork->permitLastSaveFirst = FALSE;
  dataWork->permitLastSaveSecond = FALSE;
}

//セーブデータの読み込み
BOOL  MB_DATA_LoadDataFirst( MB_DATA_WORK *dataWork )
{
  //自前で読み込む・・・
  switch( dataWork->cardType )
  {
  case CARD_TYPE_DP:
    return MB_DATA_PT_LoadData( dataWork );
    break;

  case CARD_TYPE_PT:
    return MB_DATA_PT_LoadData( dataWork );
    break;

  case CARD_TYPE_GS:
    return MB_DATA_GS_LoadData( dataWork );
    break;
  }
  return FALSE;
}

BOOL  MB_DATA_SaveData( MB_DATA_WORK *dataWork )
{
    //自前で書き込む・・・
  switch( dataWork->cardType )
  {
  case CARD_TYPE_DP:
    return MB_DATA_PT_SaveData( dataWork );
    break;

  case CARD_TYPE_PT:
    return MB_DATA_PT_SaveData( dataWork );
    break;

  case CARD_TYPE_GS:
    return MB_DATA_GS_SaveData( dataWork );
    break;
  }
  return FALSE;
}

//マルチブート子機ではROMの抜け検出ができないため、セーブ前にCRCの
//チェックを行う。通常、boxのfotterからCRCを抜き出す。
BOOL  MB_DATA_LoadRomCRC( MB_DATA_WORK *dataWork )
{
  switch( dataWork->cardType )
  {
  case CARD_TYPE_DP:
    return MB_DATA_PT_LoadRomCRC( dataWork );
    break;

  case CARD_TYPE_PT:
    return MB_DATA_PT_LoadRomCRC( dataWork );
    break;

  case CARD_TYPE_GS:
    return MB_DATA_GS_LoadRomCRC( dataWork );
    break;
  }
  return FALSE;
}

BOOL  MB_DATA_CheckRomCRC( MB_DATA_WORK *dataWork )
{
  u8 i;
  for( i=0;i<4;i++ )
  {
    if( dataWork->cardCrcTable[i] != dataWork->loadCrcTable[i] )
    {
      return FALSE;
    }
  }
  return TRUE;
}

//Boxデータの取得(PPP・Name
void* MB_DATA_GetBoxPPP( MB_DATA_WORK *dataWork , const u8 tray , const u8 idx )
{
  switch( dataWork->cardType )
  {
  case CARD_TYPE_DP:
    return MB_DATA_PT_GetBoxPPP( dataWork , tray , idx  );
    break;

  case CARD_TYPE_PT:
    return MB_DATA_PT_GetBoxPPP( dataWork , tray , idx );
    break;

  case CARD_TYPE_GS:
    return MB_DATA_GS_GetBoxPPP( dataWork , tray , idx );
    break;
  }
  return NULL;
}

u16* MB_DATA_GetBoxName( MB_DATA_WORK *dataWork , const u8 tray )
{
  switch( dataWork->cardType )
  {
  case CARD_TYPE_DP:
    return MB_DATA_PT_GetBoxName( dataWork , tray );
    break;

  case CARD_TYPE_PT:
    return MB_DATA_PT_GetBoxName( dataWork , tray );
    break;

  case CARD_TYPE_GS:
    return MB_DATA_GS_GetBoxName( dataWork , tray );
    break;
  }
  return NULL;
}


//Boxデータの削除(PPP
void MB_DATA_ClearBoxPPP( MB_DATA_WORK *dataWork , const u8 tray , const u8 idx )
{
  switch( dataWork->cardType )
  {
  case CARD_TYPE_DP:
    MB_DATA_PT_ClearBoxPPP( dataWork , tray , idx  );
    break;

  case CARD_TYPE_PT:
    MB_DATA_PT_ClearBoxPPP( dataWork , tray , idx );
    break;

  case CARD_TYPE_GS:
    MB_DATA_GS_ClearBoxPPP( dataWork , tray , idx );
    break;
  }
}

//アイテムの追加
void MB_DATA_AddItem( MB_DATA_WORK *dataWork , const u16 itemNo )
{
  switch( dataWork->cardType )
  {
  case CARD_TYPE_DP:
    MB_DATA_PT_AddItem( dataWork , itemNo );
    break;

  case CARD_TYPE_PT:
    MB_DATA_PT_AddItem( dataWork , itemNo );
    break;

  case CARD_TYPE_GS:
    MB_DATA_GS_AddItem( dataWork , itemNo );
    break;
  }
}

//アイテム所持個数チェック
const u16 MB_DATA_GetItemNum( MB_DATA_WORK *dataWork , const u16 itemNo )
{
  switch( dataWork->cardType )
  {
  case CARD_TYPE_DP:
    return MB_DATA_PT_GetItemNum( dataWork , itemNo );
    break;

  case CARD_TYPE_PT:
    return MB_DATA_PT_GetItemNum( dataWork , itemNo );
    break;

  case CARD_TYPE_GS:
    return MB_DATA_GS_GetItemNum( dataWork , itemNo );
    break;
  }
  GF_ASSERT( NULL );
  return 0;
}

//ロックカプセル所持チェック
const BOOL MB_DATA_CheckLockCapsule( MB_DATA_WORK *dataWork )
{
  if( dataWork->cardType == CARD_TYPE_GS )
  {
    //HGSSだけ！
    return MB_DATA_GS_CheckLockCapsule( dataWork );
  }
  else
  {
    return FALSE;
  }
}
//ロックカプセルを消す
void MB_DATA_RemoveLockCapsule( MB_DATA_WORK *dataWork )
{
  if( dataWork->cardType == CARD_TYPE_GS )
  {
    //HGSSだけ！
    MB_DATA_GS_RemoveLockCapsule( dataWork );
  }
}


//ダイパプラチナで共通化するための関数
u32   MB_DATA_GetStartAddress( const u16 id , DLPLAY_CARD_TYPE type )
{
  if( type == CARD_TYPE_DP ){
    return MB_DATA_DP_GetStartAddress( id );
  }
  else if( type == CARD_TYPE_PT ){
    return MB_DATA_PT_GetStartAddress( id );
  }
  else if( type == CARD_TYPE_GS ){
    return MB_DATA_GS_GetStartAddress( id );
  }
  
  GF_ASSERT( NULL );
  return 0;
}

const u32 MB_DATA_GetSavedataSize( const DLPLAY_CARD_TYPE type )
{
  GF_ASSERT( type < CARD_TYPE_INVALID );
  {
    const sizeArr[] = { PT_SAVE_SIZE , PT_SAVE_SIZE , GS_SAVE_SIZE };
    return sizeArr[type];
  }
}

const u32 MB_DATA_GetBoxDataSize( const DLPLAY_CARD_TYPE type )
{
  GF_ASSERT( type < CARD_TYPE_INVALID );
  switch( type )
  {
  case CARD_TYPE_DP:
    return MB_DATA_DP_GetStartAddress( PT_GMDATA_BOX_FOOTER ) - MB_DATA_DP_GetStartAddress( PT_GMDATA_ID_BOXDATA );
    break;
  case CARD_TYPE_PT:
    return MB_DATA_PT_GetStartAddress( PT_GMDATA_BOX_FOOTER ) - MB_DATA_PT_GetStartAddress( PT_GMDATA_ID_BOXDATA );
    break;
  case CARD_TYPE_GS:
    return MB_DATA_GS_GetStartAddress( GS_GMDATA_BOX_FOOTER ) - MB_DATA_GS_GetStartAddress( GS_GMDATA_ID_BOXDATA );
    break;
  }
  return 0;
}

const u32 MB_DATA_GetBoxDataStartAddress( const DLPLAY_CARD_TYPE type )
{
  GF_ASSERT( type < CARD_TYPE_INVALID );
  switch( type )
  {
  case CARD_TYPE_DP:
    return MB_DATA_DP_GetStartAddress( PT_GMDATA_ID_BOXDATA );
    break;
  case CARD_TYPE_PT:
    return MB_DATA_PT_GetStartAddress( PT_GMDATA_ID_BOXDATA );
    break;
  case CARD_TYPE_GS:
    return MB_DATA_GS_GetStartAddress( GS_GMDATA_ID_BOXDATA );
    break;
  }
  return 0;
}

//刺さっているカードの種類の取得設定(設定はデバッグ用
const DLPLAY_CARD_TYPE MB_DATA_GetCardType( MB_DATA_WORK *dataWork )
{
  return dataWork->cardType;
}

void MB_DATA_SetCardType( MB_DATA_WORK *dataWork , const DLPLAY_CARD_TYPE type )
{
  dataWork->cardType = type;
}

u8  MB_DATA_GetErrorState( MB_DATA_WORK *dataWork )
{
  return dataWork->errorState;
}

BOOL  MB_DATA_IsFinishSaveFirst( MB_DATA_WORK *dataWork )
{
  return dataWork->isFinishSaveFirst;
}

BOOL  MB_DATA_IsFinishSaveSecond( MB_DATA_WORK *dataWork )
{
  return dataWork->isFinishSaveSecond;
}

BOOL  MB_DATA_IsFinishSaveAll( MB_DATA_WORK *dataWork )
{
  return dataWork->isFinishSaveAll_;
}

void  MB_DATA_PermitLastSaveFirst( MB_DATA_WORK *dataWork )
{
  dataWork->permitLastSaveFirst = TRUE;
}

void  MB_DATA_PermitLastSaveSecond( MB_DATA_WORK *dataWork )
{
  dataWork->permitLastSaveSecond = TRUE;
}

#pragma mark [>CardInit

// 複製 ROM アーカイブ構造体。
typedef struct MyRomArchive
{
    FSArchive   arc[1];
    u32         default_dma_no;
    u32         card_lock_id;
}
MyRomArchive;

// 非同期のROM読み込みが完了したときの処理。
static void MyRom_OnReadDone(void *arc)
{
    // アーカイブへ完了通知。
    FS_NotifyArchiveAsyncEnd((FSArchive *)arc, FS_RESULT_SUCCESS);
}

// FSからアーカイブへのリードアクセスコールバック。
static FSResult MyRom_ReadCallback(FSArchive *arc, void *dst, u32 src, u32 len)
{
    MyRomArchive *const p_rom = (MyRomArchive *)arc;
    CARD_ReadRomAsync(p_rom->default_dma_no,
                      (const void *)(FS_GetArchiveBase(arc) + src), dst, len,
                      MyRom_OnReadDone, arc);
    return FS_RESULT_PROC_ASYNC;
}

// FSからアーカイブへのライトコールバック。
// ユーザプロシージャでFS_RESULT_UNSUPPORTEDを返すので呼ばれない。
static FSResult MyRom_WriteDummyCallback(FSArchive *arc, const void *src, u32 dst, u32 len)
{
    (void)arc;
    (void)src;
    (void)dst;
    (void)len;
    return FS_RESULT_FAILURE;
}

// ユーザプロシージャ。
// 最初のコマンド開始前から最後のコマンド完了後までROMをロック。
// ライト操作はサポート外として応答する。
// それ以外はデフォルトの動作。
static FSResult MyRom_ArchiveProc(FSFile *file, FSCommandType cmd)
{
    MyRomArchive *const p_rom = (MyRomArchive *) FS_GetAttachedArchive(file);
    switch (cmd)
    {
    case FS_COMMAND_ACTIVATE:
        CARD_LockRom((u16)p_rom->card_lock_id);
        CARD_CheckPulledOut();  //抜き検出
        return FS_RESULT_SUCCESS;
    case FS_COMMAND_IDLE:
        CARD_UnlockRom((u16)p_rom->card_lock_id);
        return FS_RESULT_SUCCESS;
    case FS_COMMAND_WRITEFILE:
        return FS_RESULT_UNSUPPORTED;
    default:
        return FS_RESULT_PROC_UNKNOWN;
    }
}

static void MB_DATA_InitCardSystem( MB_DATA_WORK *dataWork )
{
  CARDRomHeader *headerData = (CARDRomHeader*)CARD_GetRomHeader();
#if PM_DEBUG
  if( STD_CompareString( headerData->game_name , "POKEMON D" ) == 0 ||
      STD_CompareString( headerData->game_name , "POKEMON P" ) == 0 ||
      STD_CompareString( headerData->game_name , "POKEMON PL" ) == 0 ||
      STD_CompareString( headerData->game_name , "POKEMON HG" ) == 0 ||
      STD_CompareString( headerData->game_name , "POKEMON SS" ) == 0 ||
      STD_CompareString( headerData->game_name , "NINTENDO    NTRJ01" ) == 0 ||
      STD_CompareString( headerData->game_name , "SKEL" ) == 0 ||
      STD_CompareString( headerData->game_name , "dlplay" ) == 0 ||
      STD_CompareString( headerData->game_name , "SYACHI_MB" ) == 0 )
#else
  if( STD_CompareString( headerData->game_name , "POKEMON D" ) == 0 ||
      STD_CompareString( headerData->game_name , "POKEMON P" ) == 0 ||
      STD_CompareString( headerData->game_name , "POKEMON PL" ) == 0 ||
      STD_CompareString( headerData->game_name , "POKEMON HG" ) == 0 ||
      STD_CompareString( headerData->game_name , "POKEMON SS" ) == 0 )
#endif
  {
    //マルチブートで子機ROMからファイ(アイコン)を読むための処理
    
    u32 file_table_size;
    void* p_table;
    MBParam *multi_p = (MBParam *)MB_GetMultiBootParam();

    // ROMアクセスを解除する。
    CARD_Enable(TRUE);

    multi_p->boot_type = MB_TYPE_NORMAL;	/* FS_Init()にROMをenableにさせるため、MULTIBOOTフラグを一瞬OFFにする */
    OS_EnableIrq();
    FS_Init(GFL_DMA_FS_NO);
    multi_p->boot_type = MB_TYPE_MULTIBOOT;	/* MULTIBOOTフラグを再セットする */

    {
      const u32 base = 0;
      const CARDRomRegion *fnt = &((CARDRomHeader*)CARD_GetRomHeader())->fnt;
      const CARDRomRegion *fat = &((CARDRomHeader*)CARD_GetRomHeader())->fat;
      const char *name = "child_rom";

      static MyRomArchive newRom;

      FS_InitArchive(newRom.arc);
      newRom.default_dma_no = GFL_DMA_FS_NO;
      newRom.card_lock_id = (u32)OS_GetLockID();
      if (!FS_RegisterArchiveName(newRom.arc, name, (u32)STD_GetStringLength(name)))
      {
        OS_TPanic("error! FS_RegisterArchiveName(%s) failed.\n", name);
      }
      else
      {
        FS_SetArchiveProc(newRom.arc, MyRom_ArchiveProc,
                          FS_ARCHIVE_PROC_WRITEFILE | FS_ARCHIVE_PROC_ACTIVATE | FS_ARCHIVE_PROC_IDLE);
        if (!FS_LoadArchive(newRom.arc, base,
                          fat->offset, fat->length, fnt->offset, fnt->length,
                          MyRom_ReadCallback, MyRom_WriteDummyCallback))
        {
            OS_TPanic("error! FS_LoadArchive() failed.\n");
        }
      }
    }
  }  
}