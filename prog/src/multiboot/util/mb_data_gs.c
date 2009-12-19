//======================================================================
/**
 *
 * @file  mb_data_pt.c 
 * @brief wbダウンロードプレイ　HGSS用機種別コード
 * @author  ariizumi
 * @data  09/11/23
 */
//======================================================================
#include <gflib.h>
#include <string.h>
#include <nitro/irc.h>
#include "system/gfl_use.h"
#include "./pt_save.h"
#include "arc_def.h"

#include "poke_tool/poke_tool.h"
#include "multiboot/mb_data_main.h"
#include "./mb_data_gs.h"

#include "./gs_save.dat"  //mb_data_pt.c 以外で読むな

//======================================================================
//  define
//======================================================================

//======================================================================
//  enum
//======================================================================
enum GS_SAVE_TYPE
{
  GS_MAIN_FIRST = 0,
  GS_BOX_FIRST  = 1,
  GS_MAIN_SECOND = 2,
  GS_BOX_SECOND = 3,
};

//======================================================================
//  typedef struct
//======================================================================

//======================================================================
//  proto
//======================================================================

BOOL  MB_DATA_GS_LoadData( MB_DATA_WORK *dataWork );
BOOL  MB_DATA_GS_SaveData( MB_DATA_WORK *dataWork );

u32   MB_DATA_GS_GetStartAddress( const GS_GMDATA_ID id );
static  BOOL MB_DATA_GS_CheckDataCorrect( GS_SAVE_FOOTER **pFooterArr , MB_DATA_WORK *dataWork );
static u8 MB_DATA_GS_CompareFooterData( GS_SAVE_FOOTER *fData , BOOL fCorr ,
                       GS_SAVE_FOOTER *sData , BOOL sCorr ,
                       u8 *pos );



//  データの読み込み
BOOL  MB_DATA_GS_LoadData( MB_DATA_WORK *dataWork )
{
  const u32 saveSize = MB_DATA_GetSavedataSize( dataWork->cardType );
  switch( dataWork->subSeq )
  {
  case 0: //初期化およびデータ読み込み
    {
#if DLPLAY_FUNC_USE_PRINT
    char str[256];
    sprintf(str,"Start load data. size:[%d]", saveSize );
    //DLPlayFunc_PutString(str,dataWork->msgSys );
#endif
    dataWork->pData    = GFL_HEAP_AllocClearMemory(  dataWork->heapId, saveSize );
    dataWork->pDataMirror= GFL_HEAP_AllocClearMemory(  dataWork->heapId, saveSize );
  
    dataWork->lockID_ = OS_GetLockID();
    GF_ASSERT( dataWork->lockID_ != OS_LOCK_ID_ERROR );
    //プラチナは4MBフラッシュ
    if( dataWork->isDummyCard == TRUE )
    {
      CARD_IdentifyBackup( CARD_BACKUP_TYPE_FLASH_4MBITS );
    }
    else
    {
      CARD_IdentifyBackup( CARD_BACKUP_TYPE_FLASH_4MBITS_IRC );
    }
    CARD_LockBackup( (u16)dataWork->lockID_ );
    CARD_ReadFlashAsync( 0x0000 , dataWork->pData , saveSize , NULL , NULL );
    dataWork->subSeq++;
    }
    break;
  case 1: //ミラーリングデータ読み込み
    //読み込みの完了を待つ
    if( CARD_TryWaitBackupAsync() == TRUE )
    {
      GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
      CARD_ReadFlashAsync( 0x40000 , dataWork->pDataMirror , saveSize , NULL , NULL );
      dataWork->subSeq++;
      
      //DLPlayFunc_PutString("Data1 load complete.",dataWork->msgSys );
    }

    break;
  case 2: //読み込みの完了を待つ
    if( CARD_TryWaitBackupAsync() == TRUE )
    {
      GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
      CARD_UnlockBackup( (u16)dataWork->lockID_ );
      OS_ReleaseLockID( (u16)dataWork->lockID_ );
      dataWork->subSeq++;
      
      //DLPlayFunc_PutString("Data2 load complete.",dataWork->msgSys );
    }
    break;
  case 3: //各ブロックのCRCチェック
    {
      //おそらくGSでは処理が変わる
      //各ブロックのCRCをチェック(一応通常データも見ておく
      u8 i;
      const u32 boxStartAdd = MB_DATA_GetStartAddress(GS_GMDATA_ID_BOXDATA , dataWork->cardType );
      GS_SAVE_FOOTER *footer[4];
      footer[GS_MAIN_FIRST] = (GS_SAVE_FOOTER*)&dataWork->pData[ MB_DATA_GetStartAddress(GS_GMDATA_NORMAL_FOOTER , dataWork->cardType ) ];
      footer[GS_BOX_FIRST]  = (GS_SAVE_FOOTER*)&dataWork->pData[ MB_DATA_GetStartAddress(GS_GMDATA_BOX_FOOTER , dataWork->cardType ) ];
      footer[GS_MAIN_SECOND]= (GS_SAVE_FOOTER*)&dataWork->pDataMirror[ MB_DATA_GetStartAddress(GS_GMDATA_NORMAL_FOOTER , dataWork->cardType ) ];
      footer[GS_BOX_SECOND] = (GS_SAVE_FOOTER*)&dataWork->pDataMirror[ MB_DATA_GetStartAddress(GS_GMDATA_BOX_FOOTER , dataWork->cardType ) ];
#if DEB_ARI
      for( i=0;i<4;i++ ){
        MB_DATA_TPrintf("footer[%d] g_count[%2d] b_count[None] size[%5d] MGNo[%d] blkID[%02x] crc[%d]\n"
              ,i ,footer[i]->g_count ,footer[i]->size
              ,footer[i]->magic_number ,footer[i]->blk_id ,footer[i]->crc);
      }
#endif
      {
        const BOOL isCorrectData = MB_DATA_GS_CheckDataCorrect( &footer[0] , dataWork );
        if( isCorrectData == TRUE )
        {
          if( dataWork->boxSavePos == DDS_FIRST )
          {
            dataWork->pBoxData = dataWork->pData + boxStartAdd;
          }
          else
          {
            dataWork->pBoxData = dataWork->pDataMirror + boxStartAdd;
          }
        }
        else
        {
          dataWork->errorState = DES_MISS_LOAD_BACKUP;
        }
      }
    }
    //GF_ASSERT( dataWork->pBoxData != NULL );
    dataWork->subSeq++;
    break;
  
  case 4: //BOXの解析(テストデータ変換は別の場所で
#if DEB_ARI
    if( dataWork->pBoxData != NULL )
    {
      int i,j;
      for(i=0;i<GS_BOX_MAX_TRAY;i++)
      {
        for(j=0;j<GS_BOX_MAX_POS;j++)
        {
          GS_BOX_DATA *boxData = (GS_BOX_DATA*)dataWork->pBoxData;
          POKEMON_PASO_PARAM *ppp = MB_DATA_GS_GetBoxPPP(dataWork,i,j);
          const u32 monsNo = PPP_Get( ppp , ID_PARA_monsno , NULL );
          MB_DATA_TPrintf("[%3d]",monsNo);
        }
        MB_DATA_TPrintf("\n");
      }
    }
#endif
    dataWork->subSeq++;
    break;
  case 5:
    dataWork->subSeq = 0;
    return TRUE;
    break;
  }
  return FALSE;
}

#define DLPLAY_CHECK_SAVE_TICK 1
#if DLPLAY_CHECK_SAVE_TICK
static OSTick dlplaySaveTick;
#endif

//セーブは１つの関数で、セーブ→同期待ち→ラストセーブを行う。
BOOL  MB_DATA_GS_SaveData( MB_DATA_WORK *dataWork )
{
  switch( dataWork->subSeq )
  {
  case 0: //データの加工
    {
      u8 i,bi;
      GS_BOX_DATA *pBox = (GS_BOX_DATA*)dataWork->pBoxData;
#if 0      
      //ポケモンデータの暗号化
      for( bi=0;bi<BOX_MAX_TRAY;bi++)
      {
        for( i=0;i<BOX_MAX_POS;i++ )
        {
          GS_POKEMON_PARAM *param = &pBox->ppp[bi][i];
          u16 sum;
          sum = GS_PokeParaCheckSum( &param->paradata , sizeof(POKEMON_PASO_PARAM1)*4 );
          GS_CalcTool_Coded( &param->paradata , sizeof(POKEMON_PASO_PARAM1)*4 , param->checksum );
        }
      }
      
      //ポケモンを消す(消した後に暗号化もするので、暗号化の後
      //正しい削除処理
      GF_ASSERT( dataWork->selectBoxNumber_ != SELECT_BOX_INVALID );
      for( i=0;i<BOX_MAX_POS;i++ )
      {
        GS_POKEMON_PARAM *param = &pBox->ppp[dataWork->selectBoxNumber_][i];
        PokePasoParaInit( param );
      }
#endif
      //footerの加工
      //BOX
      {
        GS_SAVE_FOOTER *boxFooter,*mainFooter;
        if( dataWork->boxSavePos == DDS_FIRST ){
          boxFooter = (GS_SAVE_FOOTER*)&dataWork->pData[ MB_DATA_GetStartAddress(GS_GMDATA_BOX_FOOTER,dataWork->cardType) ];
        }
        else{
          boxFooter = (GS_SAVE_FOOTER*)&dataWork->pDataMirror[ MB_DATA_GetStartAddress(GS_GMDATA_BOX_FOOTER,dataWork->cardType) ];
        }

        if( dataWork->mainSavePos == DDS_FIRST ){
          mainFooter = (GS_SAVE_FOOTER*)&dataWork->pData[ MB_DATA_GetStartAddress(GS_GMDATA_NORMAL_FOOTER,dataWork->cardType) ];
          mainFooter->crc = MATH_CalcCRC16CCITT( &dataWork->crcTable_, dataWork->pData, mainFooter->size - sizeof(GS_SAVE_FOOTER) );
        }
        else{
          mainFooter = (GS_SAVE_FOOTER*)&dataWork->pDataMirror[ MB_DATA_GetStartAddress(GS_GMDATA_NORMAL_FOOTER,dataWork->cardType) ];
          mainFooter->crc = MATH_CalcCRC16CCITT( &dataWork->crcTable_, dataWork->pDataMirror, mainFooter->size - sizeof(GS_SAVE_FOOTER) );
        }

        boxFooter->crc = MATH_CalcCRC16CCITT( &dataWork->crcTable_, dataWork->pBoxData, boxFooter->size - sizeof(GS_SAVE_FOOTER) );
        GF_ASSERT( boxFooter->g_count == mainFooter->g_count );
        boxFooter->g_count++;
        mainFooter->g_count++;
        //boxFooter->b_count++;
        //mainFooter->b_count++;
      }
      
      //カードのロック
      dataWork->lockID_ = OS_GetLockID();
      GF_ASSERT( dataWork->lockID_ != OS_LOCK_ID_ERROR );
      CARD_LockBackup( (u16)dataWork->lockID_ );
      dataWork->subSeq++;
      
    }
    break;

  case 1: //進行データのセーブ
    {
      //読み込んだほうとは反対側に書く
      const u32 saveAddress = ( dataWork->mainSavePos == DDS_FIRST ? 0x40000 : 0x00000 );
      const u32 saveSize = MB_DATA_GetStartAddress( GS_GMDATA_NORMAL_FOOTER+1 , dataWork->cardType );
      void *pData;
      if( dataWork->mainSavePos == DDS_FIRST ){
        pData = dataWork->pData;
      }
      else{
        pData = dataWork->pDataMirror;
      }
      //セーブ開始！

      MB_DATA_TPrintf("[[%d]]\n",saveSize);
      CARD_WriteAndVerifyFlashAsync( saveAddress , pData , saveSize , NULL , NULL );
      
      //DLPlayFunc_PutString("saveMain......",dataWork->msgSys );
      dataWork->subSeq++;
#if DLPLAY_CHECK_SAVE_TICK
      dlplaySaveTick = OS_GetTick();
#endif
    }

    break;

  case 2: //BOXデータのCRCセットからセーブ(footerとか一緒に使うから
    if( CARD_TryWaitBackupAsync() == TRUE )
    {
      u32 saveAddress;
      const u32 saveSize = MB_DATA_GetBoxDataSize( dataWork->cardType )+sizeof( GS_SAVE_FOOTER );
      GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );

      //セーブ開始！
      //読み込んだほうとは反対側に書く
      saveAddress = ( dataWork->boxSavePos == DDS_FIRST ? 0x40000 : 0x00000 );
      saveAddress += MB_DATA_GetStartAddress(GS_GMDATA_ID_BOXDATA,dataWork->cardType);

      CARD_WriteAndVerifyFlashAsync( saveAddress , dataWork->pBoxData , saveSize-1 , NULL , NULL );
      
      MB_DATA_TPrintf("[[%d]]\n",saveSize);
      //DLPlayFunc_PutString("saveBox......",dataWork->msgSys);
      dataWork->subSeq++;
#if DLPLAY_CHECK_SAVE_TICK
      {
        const OSTick nowTick = OS_GetTick();
        MB_DATA_TPrintf("-SAVE TICK CHECK- SAVE_MAIN[%d]\n",OS_TicksToMilliSeconds( nowTick - dlplaySaveTick ));
      }
#endif

    }
    break;
  case 3: //書き込みの完了を待つ
    if( CARD_TryWaitBackupAsync() == TRUE )
    {
      GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
      //DLPlayFunc_PutString("Save complete.",dataWork->msgSys );
      //ここでLockは保持したまま
//      CARD_UnlockBackup( (u16)dataWork->lockID_ );
//      OS_ReleaseLockID( (u16)dataWork->lockID_ );
      dataWork->isFinishSaveFirst = TRUE;
      dataWork->subSeq++;
#if DLPLAY_CHECK_SAVE_TICK
      {
        const OSTick nowTick = OS_GetTick();
        MB_DATA_TPrintf("-SAVE TICK CHECK- SAVE_WAIT[%d]\n",OS_TicksToMilliSeconds( nowTick - dlplaySaveTick ));
      }
#endif
    }
    break;
  case 4: //最後の同期を待つ
    if( dataWork->permitLastSaveFirst == TRUE )
    {
      dataWork->subSeq++;
    }
    break;
  case 5: //最後のセーブ
    {
      u32 saveAddress;
      const u32 saveSize = MB_DATA_GetBoxDataSize( dataWork->cardType )+sizeof( GS_SAVE_FOOTER );
      GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );

      //セーブ開始！
      //読み込んだほうとは反対側に書く
      saveAddress = ( dataWork->boxSavePos == DDS_FIRST ? 0x40000 : 0x00000 );
      saveAddress += MB_DATA_GetStartAddress(GS_GMDATA_ID_BOXDATA,dataWork->cardType);
      saveAddress += saveSize-1;

      CARD_WriteAndVerifyFlashAsync( saveAddress , dataWork->pBoxData+saveSize-1 , 1 , NULL , NULL );
      
      //DLPlayFunc_PutString("saveBox......",dataWork->msgSys );
      dataWork->subSeq++;
#if DLPLAY_CHECK_SAVE_TICK
      {
        const OSTick nowTick = OS_GetTick();
        MB_DATA_TPrintf("-SAVE TICK CHECK- SAVE_LAST[%d]\n",OS_TicksToMilliSeconds( nowTick - dlplaySaveTick ));
      }
#endif
    }
    break;

  case 6: //最終確認
    if( CARD_TryWaitBackupAsync() == TRUE )
    {
      GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
      //DLPlayFunc_PutString("Save complete!!.",dataWork->msgSys );
      //ここでLockは保持したまま
      CARD_UnlockBackup( (u16)dataWork->lockID_ );
      OS_ReleaseLockID( (u16)dataWork->lockID_ );
      dataWork->subSeq++;
      dataWork->isFinishSaveSecond = TRUE;  //2番目は無い
      dataWork->isFinishSaveAll_ = TRUE;
    }
    break;
  }
  return FALSE;
}

//データの関連性、整合性をチェックする
static  BOOL MB_DATA_GS_CheckDataCorrect( GS_SAVE_FOOTER **pFooterArr , MB_DATA_WORK *dataWork )
{
  u8 i;
  u8  mainDataNum,boxDataNum;
  BOOL isCorrect[4];  //データ成否チェック
  //まずデータ単体のチェック
  for( i=0;i<4;i++ )
  {
    const u32 mainEndAdd = MB_DATA_GetStartAddress( GS_GMDATA_NORMAL_FOOTER+1 , dataWork->cardType );
    const u32 boxStartAdd = MB_DATA_GetStartAddress( GS_GMDATA_ID_BOXDATA , dataWork->cardType );
    const u32 boxEndAdd = MB_DATA_GetStartAddress( GS_GMDATA_BOX_FOOTER+1 , dataWork->cardType );
    const u32 saveSize = ( i%2==0 ? mainEndAdd : boxEndAdd - boxStartAdd );
    const u32 addOfs = ( i%2==0 ? 0 : boxStartAdd );
    //データフッタから整合性をチェック
    //マジックナンバー
    if( pFooterArr[i]->magic_number == -1 ){
      MB_DATA_TPrintf("Data[%d] is blank\n");
      isCorrect[i] = FALSE;
      continue;
    }
    else if( pFooterArr[i]->magic_number != MAGIC_NUMBER_PT ){
      MB_DATA_TPrintf("Data[%d] is invalid MAGIC_NUMBER[%x]\n",pFooterArr[i]->magic_number);
      isCorrect[i] = FALSE;
      continue;
    }
    //ブロックID(MAINは0 BOXは1
    if( pFooterArr[i]->blk_id != ( i%2==0 ? 0 : 1 ) )
    {
      MB_DATA_TPrintf(" BlockID is NG!!\n");
      isCorrect[i] = FALSE;
      continue;
    }

    //サイズのチェック
    if( pFooterArr[i]->size != saveSize )
    {
      MB_DATA_TPrintf(" Datasize is NG!!\n");
      isCorrect[i] = FALSE;
      continue;
    }
    //CRCチェック
    {
      //サイズが正しいのを確認してからやらないとマズイ
      const u16 crc = MATH_CalcCRC16CCITT( &dataWork->crcTable_ 
          , ( i<2 ? dataWork->pData+addOfs : dataWork->pDataMirror+addOfs )
          , pFooterArr[i]->size - sizeof(GS_SAVE_FOOTER) );
      MB_DATA_TPrintf("Data[%d] crc[%d]:[%d]\n",i,pFooterArr[i]->crc,crc);
      if( pFooterArr[i]->crc != crc ){
        MB_DATA_TPrintf(" crc check is NG!!\n");
        isCorrect[i] = FALSE;
        continue;
      }
    }

    isCorrect[i] = TRUE;
  }
  mainDataNum = MB_DATA_GS_CompareFooterData( pFooterArr[ GS_MAIN_FIRST ] ,isCorrect[ GS_MAIN_FIRST ],
                           pFooterArr[ GS_MAIN_SECOND ] ,isCorrect[ GS_MAIN_SECOND ] ,
                           &dataWork->mainSavePos );
  boxDataNum = MB_DATA_GS_CompareFooterData(  pFooterArr[ GS_BOX_FIRST ] ,isCorrect[ GS_BOX_FIRST ],
                           pFooterArr[ GS_BOX_SECOND ] ,isCorrect[ GS_BOX_SECOND ] ,
                           &dataWork->boxSavePos );
  if( mainDataNum == 0 || boxDataNum == 0 )
  {
    return FALSE;
  }
  else if( mainDataNum == 1 && boxDataNum == 1 )
  {
    if( dataWork->mainSavePos == dataWork->boxSavePos )
    {
      //初回セーブ
      return TRUE;
    }
    else
    {
      //片側破壊
      return FALSE;
    }
  }
  else if( mainDataNum == 1 && boxDataNum == 2 )
  {
    //進行データ破壊
    return FALSE;
  }
  else if( mainDataNum == 2 && boxDataNum == 1 )
  {
    if( pFooterArr[ dataWork->mainSavePos*2 ]->g_count ==
      pFooterArr[ dataWork->boxSavePos*2+1]->g_count )
    {
      //初回セーブ後、全体セーブなし
      return TRUE;
    }
    else
    {
      //BOXデータ破壊
      return FALSE;
    }
  }
  else
  {
    if( pFooterArr[ dataWork->mainSavePos*2 ]->g_count ==
      pFooterArr[ dataWork->boxSavePos*2+1]->g_count )
    {
      //問題なし
      return TRUE;
    }
    else
    {
      //進行とBOXの間でセーブ中断
      return FALSE;
    }
  }
  return TRUE;
}

//データの新しいほうと、正しい個数を取得
static u8 MB_DATA_GS_CompareFooterData( GS_SAVE_FOOTER *fData , BOOL fCorr ,
                       GS_SAVE_FOOTER *sData , BOOL sCorr ,
                       u8 *pos )
{
  if( fCorr == TRUE && sCorr == TRUE )
  {
    //両方正しい
    if( fData->g_count != sData->g_count )
    {
      //グローバルが違うので全体セーブ後
      if( fData->g_count > sData->g_count )
      {
        GF_ASSERT( fData->g_count > sData->g_count );
        *pos = DDS_FIRST;
      }
      else if( fData->g_count < sData->g_count )
      {
        GF_ASSERT( fData->g_count < sData->g_count );
        *pos = DDS_SECOND;
      }
      else
      {
        //ありえないエラー(一応2Mフラッシュがあるらしい・・・
        GF_ASSERT( FALSE );
        *pos = DDS_FIRST;
        return 0;
      }
    }
    else
    {
      //グローバルが一緒な時は部分セーブ
      if( fData->g_count > sData->g_count )
      {
        *pos = DDS_FIRST;
      }
      else if( fData->g_count < sData->g_count )
      {
        *pos = DDS_SECOND;
      }
      else
      {
        //ありえないエラー(一応2Mフラッシュがあるらしい・・・
        GF_ASSERT( FALSE );
        *pos = DDS_FIRST;
        return 0;
      }
    }
    return 2;
  }
  else
  if( fCorr == TRUE && sCorr == FALSE )
  {
    //片方だけ生きてる
    *pos = DDS_FIRST;
    return 1;
  }
  else if( fCorr == FALSE && sCorr == TRUE )
  {
    //片方だけ生きてる
    *pos = DDS_SECOND;
    return 1;
  }
  else
  {
    //両方アウト
    *pos = DDS_ERROR;
    return 0;
  }
}

//各データブロックの開始位置を求める。フッタも一つのブロックとして計算する
u32   MB_DATA_GS_GetStartAddress( const GS_GMDATA_ID id )
{
  u32 temp = 0;
  u8 i;
  for( i=0;i<id;i++ ){
    temp += DLPLAY_GS_SAVESIZETABLE[i];
  }
  OS_TPrintf("[%d][%x]\n",id,temp);
  return temp;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//  各種ROMごとの数値取得関数
//////////////////////////////////////////////////////////////////////////////////////////////

void* MB_DATA_GS_GetBoxPPP( MB_DATA_WORK *dataWork , const u8 tray , const u8 idx )
{
  GS_BOX_DATA *boxData = (GS_BOX_DATA*)dataWork->pBoxData;
  GF_ASSERT( dataWork->pBoxData != NULL );
  
  return &boxData->btd[tray].ppp[idx];

}

u16* MB_DATA_GS_GetBoxName( MB_DATA_WORK *dataWork , const u8 tray )
{
  GS_BOX_DATA *boxData = (GS_BOX_DATA*)dataWork->pBoxData;
  GF_ASSERT( dataWork->pBoxData != NULL );

  return boxData->trayName[tray];
}

void  MB_DATA_GS_ClearBoxPPP( MB_DATA_WORK *dataWork , const u8 tray , const u8 idx )
{
  GS_BOX_DATA *boxData = (GS_BOX_DATA*)dataWork->pBoxData;
  GF_ASSERT( dataWork->pBoxData != NULL );
  
  PPP_Clear( (POKEMON_PASO_PARAM*)&boxData->btd[tray].ppp[idx] );
}
