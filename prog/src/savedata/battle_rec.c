//==============================================================================
/**
 * @file    battle_rec.c
 * @brief   戦闘録画セーブ
 * @author  matsuda
 * @date    2009.09.01(火)
 */
//==============================================================================
#include <gflib.h>


#include "system/main.h"
#include "savedata/save_tbl.h"
#include "savedata/mystatus.h"
#include "savedata/config.h"
#include "pm_define.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
#include "savedata/battle_rec.h"
#include "battle/battle.h"

#include "../battle/btl_common.h"
#include "../battle/btl_net.h"
#include "../poke_tool/poke_tool_def.h"

#include "mystatus_local.h"
#include "net_app/gds/gds_profile_local.h"

#include "pm_define.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_tool_def.h"
#include "savedata/gds_profile.h"
#include "battle/btl_common.h"
#include "savedata\battle_rec.h"
#include "net_app/gds/gds_profile_local.h"
#include "battle_rec_local.h"
#include "net_app/gds/gds_battle_mode.h"

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* debug                                                                    */
/*--------------------------------------------------------------------------*/
#ifdef PM_DEBUG

#define DEBUG_BATTLE_REC_PRINT_ON //担当者のみのプリントON

#endif //PM_DEBUG


//担当者のみのプリント
#ifdef DEBUG_BATTLE_REC_PRINT_ON

#if defined( DEBUG_ONLY_FOR_matsuda ) 
#define BATTLE_REC_Printf(...) OS_TPrintf(__VA_ARGS__)
#elif  defined( DEBUG_ONLY_FOR_shimoyamada )
#define BATTLE_REC_Printf(...) OS_TPrintf(__VA_ARGS__)
#elif defined( DEBUG_ONLY_FOR_toru_nagihashi )
#define BATTLE_REC_Printf(...) OS_TFPrintf(0,__VA_ARGS__)
#endif //defined
#endif //DEBUG_BATTLE_REC_PRINT_ON

#ifndef BATTLE_REC_Printf
#define BATTLE_REC_Printf(...)  /*  */
#endif //BATTLE_REC_Printf

/*--------------------------------------------------------------------------*/
/* Globals                                                                  */
/*--------------------------------------------------------------------------*/
BATTLE_REC_SAVEDATA * brs=NULL;

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void RecHeaderCreate(SAVE_CONTROL_WORK *sv, BATTLE_REC_HEADER *head, const BATTLE_REC_WORK_PTR rec, int rec_mode, int counter);
static BOOL BattleRec_DataInitializeCheck(SAVE_CONTROL_WORK *sv, BATTLE_REC_SAVEDATA *src);
static  BOOL BattleRecordCheckData(SAVE_CONTROL_WORK *sv, const BATTLE_REC_SAVEDATA * src);
static  void  BattleRec_Decoded(void *data,u32 size,u32 code);



//==============================================================================
//  データ
//==============================================================================
#include "battle_rec_mode.cdat"
SDK_COMPILER_ASSERT(BATTLE_MODE_MAX == NELEMS(BattleRecModeBitTbl));


//--------------------------------------------------------------
/**
 * 対戦録画データ用ワークをAllocする
 *
 * @param   heapID
 *
 * @retval  BATTLE_REC_SAVEDATA *
 */
//--------------------------------------------------------------
static BATTLE_REC_SAVEDATA * _BattleRecSaveAlloc(HEAPID heapID)
{
  BATTLE_REC_SAVEDATA *new_brs;

  new_brs = GFL_HEAP_AllocClearMemory(heapID, SAVESIZE_EXTRA_BATTLE_REC);//sizeof(BATTLE_REC_SAVEDATA));
  BattleRec_WorkInit(new_brs);
  return new_brs;
}

//------------------------------------------------------------------
/**
 * 対戦録画データを作成する
 *
 * @param   heapID  メモリ確保するためのヒープID
 */
//------------------------------------------------------------------
void BattleRec_Init(HEAPID heapID)
{
  BATTLE_REC_Printf("BATTLE_REC_WORK size = %d\n", sizeof(BATTLE_REC_WORK));
  BATTLE_REC_Printf("BATTLE_REC_HEADER size = %d\n", sizeof(BATTLE_REC_HEADER));

  if(brs != NULL){
    GFL_HEAP_FreeMemory(brs);
    brs = NULL;
  }

  brs = _BattleRecSaveAlloc(heapID);
}
//------------------------------------------------------------------
/**
 * 対戦録画データの破棄
 */
//------------------------------------------------------------------
void BattleRec_Exit(void)
{
  GF_ASSERT(brs);
  GFL_HEAP_FreeMemory(brs);
  brs = NULL;
}
//------------------------------------------------------------------
/**
 * 対戦録画データの破棄(ワーク指定)
 *
 * BattleRec_LoadAllocで確保したワークの解放用
 */
//------------------------------------------------------------------
void BattleRec_ExitWork(BATTLE_REC_SAVEDATA *wk_brs)
{
  GFL_HEAP_FreeMemory(wk_brs);
}

//------------------------------------------------------------------
/**
 * セーブデータサイズを返す
 *
 * @retval  int
 */
//------------------------------------------------------------------
u32 BattleRec_GetWorkSize( void )
{
  //セクター2ページ分よりもサイズが大きくなったらExtraSaveDataTableの各録画データの
  //ページオフセットも+3単位に直す
  GF_ASSERT(sizeof(BATTLE_REC_SAVEDATA) < SAVE_SECTOR_SIZE * 2);

  return sizeof(BATTLE_REC_SAVEDATA);
}

//--------------------------------------------------------------
/**
 * @brief   ワーク初期化
 *
 * @param   rec
 */
//--------------------------------------------------------------
void BattleRec_WorkInit(void *rec)
{
  GFL_STD_MemClear32( rec, sizeof(BATTLE_REC_SAVEDATA) );
}
//------------------------------------------------------------------
/**
 * 対戦録画データのメモリを確保しているかチェック
 *
 * @retval  TRUE:存在する　FALSE:存在しない
 */
//------------------------------------------------------------------
BOOL BattleRec_DataExistCheck(void)
{
  return (brs!=NULL);
}
//----------------------------------------------------------------------------
/**
 *  @brief  対戦録画データのメモリ上に録画データがセットされているかチェック
 *
 *  @return TRUE録画データがセットされている　FALSE録画データがセットされていない
 */
//-----------------------------------------------------------------------------
BOOL BattleRec_DataSetCheck(void)
{
  BATTLE_REC_WORK *rec = &brs->rec;

  //何もかきこまれていなければFALSE
  if( GFL_STD_CODED_CheckSum( rec, sizeof(BATTLE_REC_WORK) ) > 0 )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  対戦録画データのメモリ上に録画データをクリアする
 *
 */
//-----------------------------------------------------------------------------
void BattleRec_DataClear(void)
{
  BattleRec_WorkInit(brs);
}
//----------------------------------------------------------------------------
/**
 *  @brief  メモリ上の対戦録画データを復号化する
 *          BattleRec_Saveの後、そのままBRSを使う場合に使用してください
 *
 */
//-----------------------------------------------------------------------------
void BattleRec_DataDecoded(void)
{
  GF_ASSERT(brs != NULL);

  BattleRec_Decoded(&brs->rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE,
    brs->rec.crc.crc16ccitt_hash + ((brs->rec.crc.crc16ccitt_hash ^ 0xffff) << 16));
}

void BattleRec_DataCoded(void)
{
  GF_ASSERT(brs != NULL);

  BattleRec_Coded(&brs->rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE,
      brs->rec.crc.crc16ccitt_hash + ((brs->rec.crc.crc16ccitt_hash ^ 0xffff) << 16));
}


//--------------------------------------------------------------
/**
 * 対戦録画データのロード
 *
 * @param wk_brs    録画データ展開先
 * @param heapID    データをロードするメモリを確保するためのヒープID
 * @param result    ロード結果を格納するワークRECLOAD_RESULT_NULL,OK,NG,ERROR
 * @param bp    ロードしたデータから生成するBATTLE_PARAM構造体へのポインタ
 * @param num   ロードするデータナンバー(LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…)
 *
 * @retval  TRUE
 */
//--------------------------------------------------------------
static BOOL _BattleRec_LoadCommon(SAVE_CONTROL_WORK *sv, BATTLE_REC_SAVEDATA *wk_brs, HEAPID heapID, LOAD_RESULT *result, int num)
{
  BATTLE_REC_WORK *rec;
  BATTLE_REC_HEADER *head;

  //データをwk_brsにロード
  *result = SaveControl_Extra_LoadWork(
    sv, SAVE_EXTRA_ID_REC_MINE + num, heapID, wk_brs, SAVESIZE_EXTRA_BATTLE_REC);

  //wk_brsに展開されたのでセーブシステムは破棄
  SaveControl_Extra_Unload(sv, SAVE_EXTRA_ID_REC_MINE + num);

  if(*result == LOAD_RESULT_NULL){
    BATTLE_REC_Printf("録画データが存在しません\n");
    *result = RECLOAD_RESULT_NULL;  //初期化データの為、データなし
    return TRUE;
  }
  else if(*result != LOAD_RESULT_OK){
    *result = RECLOAD_RESULT_ERROR;
    return TRUE;
  }

  rec = &wk_brs->rec;
  head = &wk_brs->head;

  //復号
  BattleRec_Decoded(rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE,
    rec->crc.crc16ccitt_hash + ((rec->crc.crc16ccitt_hash ^ 0xffff) << 16));

  //読み出したデータに録画データが入っているかチェック
  if(BattleRec_DataInitializeCheck(sv, wk_brs) == TRUE){
    BATTLE_REC_Printf("録画データが初期状態のものです\n");
    *result = RECLOAD_RESULT_NULL;  //初期化データの為、データなし
    return TRUE;
  }

  //データの整合性チェック
  if(BattleRecordCheckData(sv, wk_brs) == FALSE){
    BATTLE_REC_Printf("不正な録画データ\n");
    *result = RECLOAD_RESULT_NG;
    return TRUE;
  }

  #if 0
  //読み出しデータをBATTLE_PARAMにセット
  if(bp){
    BattleRec_BattleParamCreate(bp,sv);
  }
  #endif

  *result = RECLOAD_RESULT_OK;
  return TRUE;
}

//------------------------------------------------------------------
/**
 * 対戦録画データのロード
 *
 * @param sv    セーブデータ構造体へのポインタ
 * @param heapID  データをロードするメモリを確保するためのヒープID
 * @param result  ロード結果を格納するワークRECLOAD_RESULT_NULL,OK,NG,ERROR
 * @param bp    ロードしたデータから生成するBATTLE_PARAM構造体へのポインタ
 * @param num   ロードするデータナンバー(LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…)
 *
 * @retval  TRUE
 */
//------------------------------------------------------------------
BOOL BattleRec_Load( SAVE_CONTROL_WORK *sv, HEAPID heapID, LOAD_RESULT *result, int num )
{
  //すでに読み込まれているデータがあるなら、破棄する
  if(brs){
    BattleRec_WorkInit(brs);
  }
  else{
    BattleRec_Init(heapID);
  }

  return _BattleRec_LoadCommon(sv, brs, heapID, result, num);
}

//------------------------------------------------------------------
/**
 * 対戦録画データのロード(読み込み用のワークをAllocしてロード。　多重読み込み用)
 *
 * @param sv    セーブデータ構造体へのポインタ
 * @param heapID  データをロードするメモリを確保するためのヒープID
 * @param result  ロード結果を格納するワークRECLOAD_RESULT_NULL,OK,NG,ERROR
 * @param bp    ロードしたデータから生成するBATTLE_PARAM構造体へのポインタ
 * @param num   ロードするデータナンバー(LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…)
 *
 * @retval  録画データワーク(ロードに失敗してもワークはAllocしています。
 *                           必ずBattleRec_ExitWorkを使用して解放してください)
 */
//------------------------------------------------------------------
BATTLE_REC_SAVEDATA * BattleRec_LoadAlloc( SAVE_CONTROL_WORK *sv, HEAPID heapID, LOAD_RESULT *result, int num )
{
  BATTLE_REC_SAVEDATA *new_brs;

  new_brs = _BattleRecSaveAlloc(heapID);
  _BattleRec_LoadCommon(sv, new_brs, heapID, result, num);
  return new_brs;
}

//------------------------------------------------------------------
/**
 * 既に対戦録画データが存在しているか調べる(BattleRec_Load関数からデータチェックのみ抜き出したもの)
 *
 * @param sv    セーブデータ構造体へのポインタ
 * @param heapID  データをロードするメモリを確保するためのヒープID
 * @param result  ロード結果を格納するワークRECLOAD_RESULT_NULL,OK,NG,ERROR
 * @param bp    ロードしたデータから生成するBATTLE_PARAM構造体へのポインタ
 * @param num   ロードするデータナンバー（LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…）
 *
 * @retval  TRUE:正常な録画データが存在している
 * @retval  FALSE:正常な録画データは存在していない
 */
//------------------------------------------------------------------
BOOL BattleRec_DataOccCheck(SAVE_CONTROL_WORK *sv,HEAPID heapID,LOAD_RESULT *result,int num)
{
  BATTLE_REC_WORK *rec;
  BATTLE_REC_HEADER *head;
  BATTLE_REC_SAVEDATA *all;

  *result = SaveControl_Extra_Load(sv, SAVE_EXTRA_ID_REC_MINE + num, heapID);
  all = SaveControl_Extra_DataPtrGet(sv, SAVE_EXTRA_ID_REC_MINE + num, 0);
  if(*result == LOAD_RESULT_NULL){
    *result = RECLOAD_RESULT_NULL;  //初期化データの為、データなし
    SaveControl_Extra_Unload(sv, SAVE_EXTRA_ID_REC_MINE + num);
    return FALSE;
  }
  else if(*result != LOAD_RESULT_OK){
    *result = RECLOAD_RESULT_ERROR;
    SaveControl_Extra_Unload(sv, SAVE_EXTRA_ID_REC_MINE + num);
    return FALSE;
  }

  rec = &all->rec;
  head = &all->head;

  //復号
  BattleRec_Decoded(rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE,
    rec->crc.crc16ccitt_hash + ((rec->crc.crc16ccitt_hash ^ 0xffff) << 16));

  //読み出したデータに録画データが入っているかチェック
  if(BattleRec_DataInitializeCheck(sv, all) == TRUE){
    *result = RECLOAD_RESULT_NULL;  //初期化データの為、データなし
    SaveControl_Extra_Unload(sv, SAVE_EXTRA_ID_REC_MINE + num);
    return FALSE;
  }

  //データの整合性チェック
  if(BattleRecordCheckData(sv, all) == FALSE){
    BATTLE_REC_Printf("不正な録画データ\n");
    *result = RECLOAD_RESULT_NG;
    SaveControl_Extra_Unload(sv, SAVE_EXTRA_ID_REC_MINE + num);
    return FALSE;
  }

  *result = RECLOAD_RESULT_OK;
  SaveControl_Extra_Unload(sv, SAVE_EXTRA_ID_REC_MINE + num);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   対戦録画のセーブ処理をまとめたもの
 *
 * @param   sv    セーブデータへのポインタ
 * @param   num   LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…
 * @param   seq   セーブ進行を制御するワークへのポインタ(最初は0クリアした状態で呼んで下さい)
 * @param   heap_id セーブシステム作成に一時的に使用するヒープID
 *
 * @retval  SAVE_RESULT_CONTINUE  セーブ継続中
 * @retval  SAVE_RESULT_LAST    セーブ継続中、最後の部分
 * @retval  SAVE_RESULT_OK      セーブ終了、成功
 * @retval  SAVE_RESULT_NG      セーブ終了、失敗
 */
//--------------------------------------------------------------
SAVE_RESULT Local_BattleRecSave(GAMEDATA *gamedata, BATTLE_REC_SAVEDATA *work, int num, u16 *seq, HEAPID heap_id)
{
  SAVE_RESULT result;
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);

  switch(*seq){
  case 0:
    //セーブ対象の外部セーブ領域のセーブシステムを作成(セーブワークの実体はbrsを渡す)
    SaveControl_Extra_SystemSetup(sv, SAVE_EXTRA_ID_REC_MINE + num, heap_id, brs, SAVESIZE_EXTRA_BATTLE_REC);

    GAMEDATA_ExtraSaveAsyncStart(gamedata, SAVE_EXTRA_ID_REC_MINE + num);
    (*seq)++;
    break;
  case 1:
    result = GAMEDATA_ExtraSaveAsyncMain(gamedata, SAVE_EXTRA_ID_REC_MINE + num);

    if(result == SAVE_RESULT_OK || result == SAVE_RESULT_NG){
      //外部セーブ完了。セーブシステムを破棄
      SaveControl_Extra_UnloadWork(sv, SAVE_EXTRA_ID_REC_MINE + num);
      return result;
    }
    break;
  }
  return SAVE_RESULT_CONTINUE;
}

//------------------------------------------------------------------
/**
 * 対戦録画データのセーブ
 *
 * @param sv        セーブデータ構造体へのポインタ
 * @param heap_id   セーブシステム作成に一時的に使用するヒープID
 * @param   rec_mode    録画モード(BATTLE_MODE_???)
 * @param   fight_count   何戦目か
 * @param num   ロードするデータナンバー（LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…）
 * @param   work0   セーブ進行を制御するワークへのポインタ(最初は0クリアした状態で呼んで下さい)
 * @param   work1   セーブ進行を制御するワークへのポインタ(最初は0クリアした状態で呼んで下さい)
 *
 * @retval  セーブ結果(SAVE_RESULT_OK or SAVE_RESULT_NG が返るまで場合は毎フレーム呼び続けてください)
 */
//------------------------------------------------------------------
SAVE_RESULT BattleRec_Save(GAMEDATA *gamedata, HEAPID heap_id, BATTLE_MODE rec_mode, int fight_count, int num, u16 *work0, u16 *work1)
{
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);
  BATTLE_REC_HEADER *head;
  BATTLE_REC_WORK *rec;
  SAVE_RESULT result;
  u16 test_crc;

  switch(*work0){
  case 0:
    //データがないときは、何もしない
    if(brs==NULL){
      return  SAVE_RESULT_NG;
    }
    head = &brs->head;
    rec = &brs->rec;

    //録画データ本体を元にヘッダデータ作成
    RecHeaderCreate(sv, head, rec, rec_mode, fight_count);

    //CRC作成
    head->magic_key = REC_OCC_MAGIC_KEY;
    head->crc.crc16ccitt_hash = GFL_STD_CrcCalc(head,
      sizeof(BATTLE_REC_HEADER) - GDS_CRC_SIZE - DATANUMBER_SIZE);
    rec->magic_key = REC_OCC_MAGIC_KEY;
    BattleRec_CalcCrcRec( rec );

    //CRCをキーにして暗号化
    test_crc = rec->crc.crc16ccitt_hash;
    BattleRec_Coded(rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE,
      rec->crc.crc16ccitt_hash + ((rec->crc.crc16ccitt_hash ^ 0xffff) << 16));
    //ここに引っかかる場合はCRCの開始位置が構造体上で4バイトアライメントされていない
    GF_ASSERT(rec->crc.crc16ccitt_hash == test_crc);

    *work1 = 0;
    (*work0)++;
    break;
  case 1:
    result = Local_BattleRecSave(gamedata, brs, num, work1, heap_id);
    return result;
  }

  return SAVE_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   録画モードからクライアント数と手持ち数の上限を取得
 *
 * @param   rec_mode      録画モードのbit定義(BattleRecModeBitTblの内容物)
 * @param   client_max    クライアント数代入先
 * @param   temoti_max    手持ち最大数代入先
 */
//--------------------------------------------------------------
void BattleRec_ClientTemotiGet(u16 mode_bit, int *client_max, int *temoti_max)
{
  int i;

  for(i = 0; i < BATTLE_MODE_MAX; i++){
    if(BattleRecModeBitTbl[i] == mode_bit){
      switch(i){
      case BATTLE_MODE_COLOSSEUM_MULTI_FREE:
      case BATTLE_MODE_COLOSSEUM_MULTI_50:
      case BATTLE_MODE_COLOSSEUM_MULTI_FREE_SHOOTER:
      case BATTLE_MODE_COLOSSEUM_MULTI_50_SHOOTER:
      case BATTLE_MODE_SUBWAY_MULTI:
        *client_max = 4;
        *temoti_max = TEMOTI_POKEMAX / 2;
        return;
      }
    }
  }
  *client_max = 2;
  *temoti_max = TEMOTI_POKEMAX;
}

//--------------------------------------------------------------
/**
 * @brief   録画データ本体を元にヘッダーデータを作成
 *
 * @param   head    ヘッダーデータ代入先
 * @param   rec     録画データ本体
 * @param   rec_mode  録画モード(BATTLE_MODE_???)
 * @param   counter   何戦目か
 */
//--------------------------------------------------------------
static void RecHeaderCreate(SAVE_CONTROL_WORK *sv, BATTLE_REC_HEADER *head, const BATTLE_REC_WORK_PTR rec, int rec_mode, int counter)
{
  int client, temoti, client_max, temoti_max, n;
  const REC_POKEPARA *para;

  GFL_STD_MemClear(head, sizeof(BATTLE_REC_HEADER));

  BattleRec_ClientTemotiGet(BattleRecModeBitTbl[rec_mode], &client_max, &temoti_max);

  n = 0;

  for(client = 0; client < client_max; client++){
    for(temoti = 0; temoti < temoti_max; temoti++){
      para = &(rec->rec_party[client].member[temoti]);
      if(para->tamago_flag == 0 && para->fusei_tamago_flag == 0){
        head->monsno[n] = para->monsno;
        head->form_no_and_sex[n] = ( para->form_no & HEADER_FORM_NO_MASK ) | ( para->sex << HEADER_GENDER_MASK );
      }
      n++;
    }
  }

  GF_ASSERT(rec_mode < BATTLE_MODE_MAX);
  head->battle_counter = counter;
  head->mode = BattleRecModeBitTbl[rec_mode];
  head->server_vesion = BTL_NET_SERVER_VERSION;
}

//--------------------------------------------------------------
/**
 * @brief   対戦録画データが初期化されたデータか調べる
 *
 * @param   src   対戦録画データへのポインタ
 *
 * @retval  TRUE:初期化されているデータ
 * @retval  FALSE:何らかのデータが入っている
 */
//--------------------------------------------------------------
static BOOL BattleRec_DataInitializeCheck(SAVE_CONTROL_WORK *sv, BATTLE_REC_SAVEDATA *src)
{
  BATTLE_REC_WORK *rec = &src->rec;
  BATTLE_REC_HEADER *head = &src->head;

  if(rec->magic_key != REC_OCC_MAGIC_KEY || head->magic_key != REC_OCC_MAGIC_KEY){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   セーブデータの整合性チェック
 *
 * @param   src   データの先頭アドレス
 *
 * @retval  TRUE:正しい。　FALSE:不正
 */
//--------------------------------------------------------------
static  BOOL BattleRecordCheckData(SAVE_CONTROL_WORK *sv, const BATTLE_REC_SAVEDATA * src)
{
  const BATTLE_REC_WORK *rec = &src->rec;
  const BATTLE_REC_HEADER *head = &src->head;
  u16 hash;

//  if(rec->crc.crc16ccitt_hash == 0 || head->crc.crc16ccitt_hash == 0){
  if(rec->magic_key != REC_OCC_MAGIC_KEY || head->magic_key != REC_OCC_MAGIC_KEY){
    return FALSE;
  }

  //ヘッダーのCRCハッシュ計算
  hash = GFL_STD_CrcCalc(head,
    sizeof(BATTLE_REC_HEADER) -GDS_CRC_SIZE-DATANUMBER_SIZE);
  if(hash != head->crc.crc16ccitt_hash){
    BATTLE_REC_Printf("ヘッダーのCRCハッシュ不正\n");
    return FALSE;
  }

  //本体全体のCRCハッシュ計算
  hash = GFL_STD_CrcCalc(rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE);
  if (hash != rec->crc.crc16ccitt_hash) {
    BATTLE_REC_Printf("録画データ本体のCRCハッシュ不正\n");
    return FALSE;
  }

  return TRUE;
}
//============================================================================================
/**
 *  復号処理
 *
 * @param[in] data  復号するデータのポインタ
 * @param[in] size  復号するデータのサイズ
 * @param[in] code  暗号化キーの初期値
 */
//============================================================================================
static  void  BattleRec_Decoded(void *data,u32 size,u32 code)
{
  GFL_STD_CODED_Decoded(data,size,code);
}
//============================================================================================
/**
 *  暗号処理
 *
 * @param[in] data  暗号化するデータのポインタ
 * @param[in] size  暗号化するデータのサイズ
 * @param[in] code  暗号化キーの初期値
 */
//============================================================================================
void  BattleRec_Coded(void *data,u32 size,u32 code)
{
  GFL_STD_CODED_Coded(data, size, code);
}


//==============================================================================
//
//  データ取得
//
//==============================================================================


BTLREC_OPERATION_BUFFER*  BattleRec_GetOperationBufferPtr( void )
{
  GF_ASSERT(brs);
  return &(brs->rec.opBuffer);
}




//--------------------------------------------------------------
/**
 * @brief   GDSプロフィールのポインタ取得
 *
 * @param   num         LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…
 *
 * @retval  プロフィールのポインタ
 *
 * BattleRec_Load or BattleRec_Initを使用してbrsにデータをロードしている必要があります。
 */
//--------------------------------------------------------------
GDS_PROFILE_PTR BattleRec_GDSProfilePtrGet(void)
{
  GF_ASSERT(brs != NULL);
  return (GDS_PROFILE_PTR)&brs->profile;
}

//--------------------------------------------------------------
/**
 * @brief   録画本体のポインタ取得
 *
 * @retval  録画本体のポインタ
 *
 * BattleRec_Load or BattleRec_Initを使用してbrsにデータをロードしている必要があります。
 */
//--------------------------------------------------------------
BATTLE_REC_WORK_PTR BattleRec_WorkPtrGet(void)
{
  GF_ASSERT(brs != NULL);
  return &brs->rec;
}

//--------------------------------------------------------------
/**
 * @brief   録画ヘッダのポインタ取得
 *
 * @retval  録画ヘッダのポインタ
 *
 * BattleRec_Load or BattleRec_Initを使用してbrsにデータをロードしている必要があります。
 */
//--------------------------------------------------------------
BATTLE_REC_HEADER_PTR BattleRec_HeaderPtrGet(void)
{
  GF_ASSERT(brs != NULL);
  return &brs->head;
}

//--------------------------------------------------------------
/**
 * @brief   録画ヘッダのポインタ取得(ワーク指定)
 *
 * @retval  録画ヘッダのポインタ
 *
 * BattleRec_LoadAllocを使用したワークのヘッダ取得用です
 */
//--------------------------------------------------------------
BATTLE_REC_HEADER_PTR BattleRec_HeaderPtrGetWork(BATTLE_REC_SAVEDATA *wk_brs)
{
  return &wk_brs->head;
}

//--------------------------------------------------------------
/**
 * @brief   読み込んでいる録画データに対して、指定データで上書きする
 *
 * @param   num         LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…
 * @param   gpp     GDSプロフィール
 * @param   head    録画ヘッダ
 * @param   rec     録画本体
 * @param   sv      セーブデータへのポインタ
 *
 * BattleRec_Loadを使用してデータをロードしている必要があります。
 */
//--------------------------------------------------------------
void BattleRec_DataSet(GDS_PROFILE_PTR gpp, BATTLE_REC_HEADER_PTR head, BATTLE_REC_WORK_PTR rec, SAVE_CONTROL_WORK *sv)
{
  GF_ASSERT(brs != NULL);
  GFL_STD_MemCopy(head, &brs->head, sizeof(BATTLE_REC_HEADER));
  GFL_STD_MemCopy(rec, &brs->rec, sizeof(BATTLE_REC_WORK));
  GFL_STD_MemCopy(gpp, &brs->profile, sizeof(GDS_PROFILE));

  //復号
  BattleRec_Decoded(&brs->rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE,
    brs->rec.crc.crc16ccitt_hash + ((brs->rec.crc.crc16ccitt_hash ^ 0xffff) << 16));
}
//--------------------------------------------------------------
/**
 * @brief   録画ヘッダのパラメータ取得
 *
 * @param   head    録画ヘッダへのポインタ
 * @param   index   データINDEX(RECHEAD_IDX_???)
 * @param   param   パラメータ
 *
 * @retval  取得データ
 */
//--------------------------------------------------------------
u64 RecHeader_ParamGet(BATTLE_REC_HEADER_PTR head, int index, int param)
{
  GF_ASSERT(DATANUMBER_SIZE <= sizeof(u64));

  switch(index){
  case RECHEAD_IDX_MONSNO:
    GF_ASSERT(param < HEADER_MONSNO_MAX);
    if(head->monsno[param] > MONSNO_END){
      return 0;
    }
    return head->monsno[param];
  case RECHEAD_IDX_FORM_NO:
    GF_ASSERT(param < HEADER_MONSNO_MAX);
    return head->form_no_and_sex[param] & HEADER_FORM_NO_MASK;

  case RECHEAD_IDX_COUNTER:
    if ( head->battle_counter > REC_COUNTER_MAX ){
      return REC_COUNTER_MAX;
    }
    return head->battle_counter;

  case RECHEAD_IDX_MODE:
    {
      int i;
      for(i = 0; i < BATTLE_MODE_MAX; i++){
        if(head->mode == BattleRecModeBitTbl[i]){
          return i;
        }
      }
    }
    return BATTLE_MODE_COLOSSEUM_SINGLE_FREE;
  case RECHEAD_IDX_DATA_NUMBER:
    return head->data_number;
  case RECHEAD_IDX_SECURE:
    return head->secure;
  case RECHEAD_IDX_GENDER:
    GF_ASSERT(param < HEADER_MONSNO_MAX);
    return ( head->form_no_and_sex[param] & HEADER_GENDER_MASK ) >> HEADER_GENDER_SHIFT;
  case RECHEAD_IDX_SERVER_VER:
    return head->server_vesion;
  }

  GF_ASSERT(0); //不明なINDEX
  return 0;
}

//--------------------------------------------------------------
/**
 * @brief   自分のROMのサーバーバージョンよりも高いサーバーバージョンが記録されているか確認
 *
 * @param   試合後に録画できるかを確認したい場合はBATTLE_SETUP_PARAMのcommServerVer
 *          バトルビデオのデータを確認したい場合はRecHeader_ParamGet( header, RECHEAD_IDX_SERVER_VER ,0 )で取得した値をいれてください
 *
 * @retval  TRUE:dd自分と同じか以下のサーバーバージョン（保存、再生可能）
 * @retval  FALSE:自分のROMよりも高いサーバーバージョンが記録されている
 */
//--------------------------------------------------------------
BOOL BattleRec_ServerVersionCheck( u8 version )
{
  return version <= BTL_NET_SERVER_VERSION;
}
//----------------------------------------------------------------------------
/**
 *	@brief  REC_WORKのCRCを再計算
 *	        （不正文字対策のためにポケモン名を置き換えるので、その際に使用）
 *
 *	@param	*wk_brs BRS
 */
//-----------------------------------------------------------------------------
void BattleRec_CalcCrcRec( BATTLE_REC_WORK_PTR rec )
{
  rec->crc.crc16ccitt_hash = GFL_STD_CrcCalc(rec,
      sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE);
}


FS_EXTERN_OVERLAY(battle_recorder_tools);

//=============================================================================================
/**
 * 録画データ格納・復元に使うツールオーバーレイモジュールをロードする
 */
//=============================================================================================
void BattleRec_LoadToolModule( void )
{
  GFL_OVERLAY_Load( FS_OVERLAY_ID(battle_recorder_tools) );
}
//=============================================================================================
/**
 * 録画データ格納・復元に使うツールオーバーレイモジュールをアンロードする
 */
//=============================================================================================
void BattleRec_UnloadToolModule( void )
{
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(battle_recorder_tools) );
}

