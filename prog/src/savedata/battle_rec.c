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
#include "gds_profile_types.h"

#include "battle_rec_local.h"

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
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
static void PokeParty_to_RecPokeParty( const POKEPARTY *party, REC_POKEPARTY *rec_party );
static void RecPokeParty_to_PokeParty( REC_POKEPARTY *rec_party, POKEPARTY *party, HEAPID heapID );
static void store_Party( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec );
static void restore_Party( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec, HEAPID heapID );
static void store_ClientStatus( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec );
static void restore_ClientStatus( BATTLE_SETUP_PARAM* setup, const BATTLE_REC_WORK* rec );
static void store_TrainerData( const BSP_TRAINER_DATA* bspTrainer, BTLREC_TRAINER_STATUS* recTrainer );
static void restore_TrainerData( BSP_TRAINER_DATA* bspTrainer, const BTLREC_TRAINER_STATUS* recTrainer );
static BOOL store_OperationBuffer( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec );
static BOOL restore_OperationBuffer( BATTLE_SETUP_PARAM* setup, const BATTLE_REC_WORK* rec );
static BOOL store_SetupSubset( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec );
static BOOL restore_SetupSubset( BATTLE_SETUP_PARAM* setup, const BATTLE_REC_WORK* rec );



//------------------------------------------------------------------
/**
 * 対戦録画データを作成する
 *
 * @param   heapID  メモリ確保するためのヒープID
 */
//------------------------------------------------------------------
void BattleRec_Init(HEAPID heapID)
{
#ifdef DEBUG_ONLY_FOR_matsuda
  OS_TPrintf("BATTLE_REC_WORK size = %d\n", sizeof(BATTLE_REC_WORK));
  OS_TPrintf("BATTLE_REC_HEADER size = %d\n", sizeof(BATTLE_REC_HEADER));
#endif  //DEBUG_ONLY_FOR_matsuda

  if(brs != NULL){
    GFL_HEAP_FreeMemory(brs);
    brs = NULL;
  }

  brs = GFL_HEAP_AllocClearMemory(heapID,SAVESIZE_EXTRA_BATTLE_REC);//sizeof(BATTLE_REC_SAVEDATA));
  BattleRec_WorkInit(brs);
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

//--------------------------------------------------------------
/**
 * @brief   認証キーを除いた戦闘録画ワークのアドレスを取得
 *
 * @retval  brsに格納されているワークアドレス(認証キー除く)
 */
//--------------------------------------------------------------
void * BattleRec_RecWorkAdrsGet( void )
{
  u8 *work;

  GF_ASSERT(brs);

  work = (u8*)brs;
  return &work[sizeof(EX_CERTIFY_SAVE_KEY)];
}

//------------------------------------------------------------------
/**
 * 対戦録画データのロード
 *
 * @param sv    セーブデータ構造体へのポインタ
 * @param heapID  データをロードするメモリを確保するためのヒープID
 * @param result  ロード結果を格納するワークRECLOAD_RESULT_NULL,OK,NG,ERROR
 * @param bp    ロードしたデータから生成するBATTLE_PARAM構造体へのポインタ
 * @param num   ロードするデータナンバー（LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…）
 *
 * @retval  TRUE
 */
//------------------------------------------------------------------
BOOL BattleRec_Load( SAVE_CONTROL_WORK *sv, HEAPID heapID, LOAD_RESULT *result, int num )
{
  BATTLE_REC_WORK *rec;
  BATTLE_REC_HEADER *head;

  //すでに読み込まれているデータがあるなら、破棄する
  if(brs){
    BattleRec_WorkInit(brs);
  }
  else{
    BattleRec_Init(heapID);
  }

  //データをbrsにロード
  *result = SaveControl_Extra_LoadWork(
    sv, SAVE_EXTRA_ID_REC_MINE + num, heapID, brs, sizeof(BATTLE_REC_SAVEDATA));

  //brsに展開されたのでセーブシステムは破棄
  SaveControl_Extra_Unload(sv, SAVE_EXTRA_ID_REC_MINE + num);

  if(*result != LOAD_RESULT_OK){
    *result = RECLOAD_RESULT_ERROR;
    return TRUE;
  }

  rec = &brs->rec;
  head = &brs->head;

  //復号
  BattleRec_Decoded(rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE,
    rec->crc.crc16ccitt_hash + ((rec->crc.crc16ccitt_hash ^ 0xffff) << 16));

  //読み出したデータに録画データが入っているかチェック
  #if 1
  if(BattleRec_DataInitializeCheck(sv, brs) == TRUE){
    OS_TPrintf("録画データが初期状態のものです\n");
    *result = RECLOAD_RESULT_NULL;  //初期化データの為、データなし
    return TRUE;
  }

  //データの整合性チェック
  if(BattleRecordCheckData(sv, brs) == FALSE){
  #ifdef OSP_REC_ON
    OS_TPrintf("不正な録画データ\n");
  #endif
    *result = RECLOAD_RESULT_NG;
    return TRUE;
  }
  #endif

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
  if(*result != LOAD_RESULT_OK){
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
  #ifdef OSP_REC_ON
    OS_TPrintf("不正な録画データ\n");
  #endif
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
SAVE_RESULT Local_BattleRecSave(SAVE_CONTROL_WORK *sv, BATTLE_REC_SAVEDATA *work, int num, u16 *seq, HEAPID heap_id)
{
  SAVE_RESULT result;

  switch(*seq){
  case 0:
#if 0 //※check　未作成　録画データの仕様が決まってから 2009.11.18(水)
    sys_SoftResetNG(SOFTRESET_TYPE_VIDEO);
    sys_SioErrorNG_PtrSet(HEAPID_WORLD);
#endif

    //セーブ対象の外部セーブ領域のセーブシステムを作成(セーブワークの実体はbrsを渡す)
    SaveControl_Extra_SystemSetup(sv, SAVE_EXTRA_ID_REC_MINE + num, heap_id, brs, SAVESIZE_EXTRA_BATTLE_REC);

    SaveControl_Extra_SaveAsyncInit(sv, SAVE_EXTRA_ID_REC_MINE + num);
    do{
      result = SaveControl_Extra_SaveAsyncMain(sv, SAVE_EXTRA_ID_REC_MINE + num);
    }while(result == SAVE_RESULT_CONTINUE || result == SAVE_RESULT_LAST);

    //外部セーブ完了。セーブシステムを破棄
    SaveControl_Extra_UnloadWork(sv, SAVE_EXTRA_ID_REC_MINE + num);

#if 0 //※check　未作成　通常セーブは外部との繋がりがしっかりしてから
    if(result == SAVE_RESULT_OK){
      //result = SaveData_Save(sv);
      SaveData_DivSave_Init(sv, SVBLK_ID_MAX);
      (*seq)++;
      return SAVE_RESULT_CONTINUE;
    }
#endif

#if 0 //※check　未作成　録画データの仕様が決まってから 2009.11.18(水)
    sys_SoftResetOK(SOFTRESET_TYPE_VIDEO);
#endif
    return result;
  case 1:
#if 0 //※check　未作成　通常セーブは外部との繋がりがしっかりしてから
    result = SaveData_DivSave_Main(sv);
    if(result == SAVE_RESULT_OK || result == SAVE_RESULT_NG){
      (*seq) = 0;
      sys_SioErrorNG_PtrFree();
      sys_SoftResetOK(SOFTRESET_TYPE_VIDEO);
    }
    return result;
#else
    break;
#endif
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
SAVE_RESULT BattleRec_Save(SAVE_CONTROL_WORK *sv, HEAPID heap_id, int rec_mode, int fight_count, int num, u16 *work0, u16 *work1)
{
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
    rec->crc.crc16ccitt_hash = GFL_STD_CrcCalc(rec,
      sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE);

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
    result = Local_BattleRecSave(sv, brs, num, work1, heap_id);
    return result;
  }

  return SAVE_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   指定位置の録画データを削除(初期化)してセーブ実行
 *
 * @param   sv
 * @param   heap_id   削除用テンポラリ(録画データを展開できるだけのヒープが必要です)
 * @param   num
 * @param   work0   セーブ進行を制御するワークへのポインタ(最初は0クリアした状態で呼んで下さい)
 * @param   work1   セーブ進行を制御するワークへのポインタ(最初は0クリアした状態で呼んで下さい)
 *
 * @retval  SAVE_RESULT_CONTINUE  セーブ処理継続中
 * @retval  SAVE_RESULT_LAST    セーブ処理継続中、最後の一つ前
 * @retval  SAVE_RESULT_OK      セーブ正常終了
 * @retval  SAVE_RESULT_NG      セーブ失敗終了
 *
 * ※消去はオフラインで行うので分割セーブではなく、一括セーブにしています。
 *    ※check　WBでは常時通信の為、削除も分割セーブに変更する予定 2009.11.18(水)
 */
//--------------------------------------------------------------
SAVE_RESULT BattleRec_SaveDataErase(SAVE_CONTROL_WORK *sv, HEAPID heap_id, int num)
{
  SAVE_RESULT result;
  LOAD_RESULT load_result;
  BATTLE_REC_SAVEDATA *all;

  load_result = SaveControl_Extra_Load(sv, SAVE_EXTRA_ID_REC_MINE + num, heap_id);
  all = SaveControl_Extra_DataPtrGet(sv, SAVE_EXTRA_ID_REC_MINE + num, 0);
  BattleRec_WorkInit(all);

  SaveControl_Extra_SaveAsyncInit(sv, SAVE_EXTRA_ID_REC_MINE + num);
  do{
    result = SaveControl_Extra_SaveAsyncMain(sv, SAVE_EXTRA_ID_REC_MINE + num);
  }while(result == SAVE_RESULT_CONTINUE || result == SAVE_RESULT_LAST);

  SaveControl_Extra_Unload(sv, SAVE_EXTRA_ID_REC_MINE + num);
  return result;
}

//--------------------------------------------------------------
/**
 * @brief   録画モードからクライアント数と手持ち数の上限を取得
 *
 * @param   rec_mode    録画モード(BATTLE_MODE_???)
 * @param   client_max    クライアント数代入先
 * @param   temoti_max    手持ち最大数代入先
 */
//--------------------------------------------------------------
void BattleRec_ClientTemotiGet(const BATTLE_REC_WORK_PTR rec, int *client_max, int *temoti_max)
{
  int i;

  *client_max = 0;

  for(i = 0; i < BTL_CLIENT_MAX; i++){
    if(rec->clientStatus[i].type != BTLREC_CLIENTSTATUS_NONE){
      (*client_max)++;
    }
  }

  if((*client_max) == BTL_CLIENT_MAX){
    *temoti_max = TEMOTI_POKEMAX / 2;
  }
  else{
    *temoti_max = TEMOTI_POKEMAX;
  }
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

  BattleRec_ClientTemotiGet(rec, &client_max, &temoti_max);

  n = 0;

  for(client = 0; client < client_max; client++){
    for(temoti = 0; temoti < temoti_max; temoti++){
      para = &(rec->rec_party[client].member[temoti]);
      if(para->tamago_flag == 0 && para->fusei_tamago_flag == 0){
        head->monsno[n] = para->monsno;
        head->form_no[n] = para->form_no;
      }
      n++;
    }
  }

  head->battle_counter = counter;
  head->mode = rec_mode;
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

#if 0 //※check　外部セーブデータの初期化チェックが未作成 2009.11.18(水)
  if(SaveData_GetExtraInitFlag(sv) == FALSE){
    return TRUE;  //外部セーブデータが初期化されていないのでデータ無しと判定する
  }
#endif

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
#if 0 //※check　未作成　録画データの仕様が決まってから 2009.11.18(水)
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
  #ifdef OSP_REC_ON
    OS_TPrintf("ヘッダーのCRCハッシュ不正\n");
  #endif
    return FALSE;
  }

  //本体全体のCRCハッシュ計算
  hash = GFL_STD_CrcCalc(rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE);
  if (hash != rec->crc.crc16ccitt_hash) {
  #ifdef OSP_REC_ON
    OS_TPrintf("録画データ本体のCRCハッシュ不正\n");
  #endif
    return FALSE;
  }

  //ポケモンパラメータの不正チェック
  {
    int client, temoti, wazano;
    const REC_POKEPARA *para;

    for(client = 0; client < BTL_CLIENT_MAX; client++){
      for(temoti = 0; temoti < TEMOTI_POKEMAX; temoti++){
        para = &(rec->rec_party[client].member[temoti]);
      #if 0
        //ダメタマゴ
        if(para->fusei_tamago_flag == 1){
          OS_TPrintf("ダメタマゴが混じっている\n");
          return FALSE;
        }
      #endif
        //不正なポケモン番号
        if(para->monsno > MONSNO_MAX){
        #ifdef OSP_REC_ON
          OS_TPrintf("不正なポケモン番号\n");
        #endif
          return FALSE;
        }
        //不正なアイテム番号
        if(para->item > ITEM_DATA_MAX){
        #ifdef OSP_REC_ON
          OS_TPrintf("不正なアイテム番号\n");
        #endif
          return FALSE;
        }
        //不正な技番号
        for(wazano = 0; wazano < WAZA_TEMOTI_MAX; wazano++){
          if(para->waza[wazano] > WAZANO_MAX){
          #ifdef OSP_REC_ON
            OS_TPrintf("不正な技番号\n");
          #endif
            return FALSE;
          }
        }
      }
    }
  }

  return TRUE;
#else
  return TRUE;
#endif
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
 *  BATTLE_PARAM構造体保存処理
 *
 * @param[in] bp      BATTLE_PARAM構造体へのポインタ
 */
//============================================================================================
void BattleRec_BattleParamRec(BATTLE_PARAM *bp)
{
#if 0 //※check　未作成　録画データの仕様が決まってから 2009.11.18(水)
  int i;
  BATTLE_REC_WORK *rec;
  REC_BATTLE_PARAM *rbp;

  if(brs==NULL){
    return;
  }

  rec = &brs->rec;
  rbp = &rec->rbp;

  rbp->fight_type = bp->fight_type;
  rbp->win_lose_flag = bp->win_lose_flag;
//  rbp->btr = bp->btr;
  rbp->bg_id = bp->bg_id;
  rbp->ground_id = bp->ground_id;
  rbp->place_id = bp->place_id;
  rbp->zone_id = bp->zone_id;
  rbp->time_zone = bp->time_zone;
  rbp->shinka_place_mode = bp->shinka_place_mode;
  rbp->contest_see_flag = bp->contest_see_flag;
  rbp->mizuki_flag = bp->mizuki_flag;
  rbp->get_pokemon_client = bp->get_pokemon_client;
  rbp->weather = bp->weather;
  rbp->level_up_flag = bp->level_up_flag;
  rbp->battle_status_flag = bp->battle_status_flag;
  rbp->safari_ball = bp->safari_ball;
  rbp->regulation_flag = bp->regulation_flag;
  rbp->rand = bp->rand;
  rbp->comm_id = bp->comm_id;
  rbp->dummy = bp->dummy;
  rbp->total_turn = bp->total_turn;
  for(i = 0; i < BTL_CLIENT_MAX; i++){
    rbp->trainer_id[i] = bp->trainer_id[i];
    rbp->trainer_data[i] = bp->trainer_data[i];
    if(bp->server_version[i] == 0){
      rbp->server_version[i] = BTL_NET_SERVER_VERSION;
    }
    else{
      rbp->server_version[i] = bp->server_version[i];
    }
    rbp->comm_stand_no[i] = bp->comm_stand_no[i];
    rbp->voice_waza_param[i] = bp->voice_waza_param[i];
  }

  //-- REC_BATTLE_PARAMではない場所に保存するデータをセット --//
  for(i=0;i<BTL_CLIENT_MAX;i++){
    PokeParty_to_RecPokeParty(bp->poke_party[i], &rec->rec_party[i]);
    MyStatus_Copy(bp->my_status[i],&rec->my_status[i]);
    rbp->voice_waza_param[i]=Snd_PerapVoiceWazaParamGet(bp->poke_voice[i]);
  }
  CONFIG_Copy(bp->config,&rec->config);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   サーバーバージョン更新処理
 *
 * @param   id_no       ID
 * @param   server_version    サーバーバージョン
 */
//--------------------------------------------------------------
void BattleRec_ServerVersionUpdate(int id_no, u32 server_version)
{
#if 0 //※check　未作成　録画データの仕様が決まってから 2009.11.18(水)
  BATTLE_REC_WORK *rec;
  REC_BATTLE_PARAM *rbp;

  if(brs==NULL){
    return;
  }

  rec = &brs->rec;
  rbp = &rec->rbp;
  rbp->server_version[id_no] = server_version;
//  OS_TPrintf("sio server_version = %x\n", rbp->server_version[id_no]);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   自分のROMのサーバーバージョンよりも高いサーバーバージョンが記録されているか確認
 *
 * @retval  TRUE:全て自分と同じか以下のサーバーバージョン
 * @retval  FALSE:自分のROMよりも高いサーバーバージョンが記録されている
 */
//--------------------------------------------------------------
BOOL BattleRec_ServerVersionCheck(void)
{
#if 0 //※check　未作成　録画データの仕様が決まってから 2009.11.18(水)
  int i;
  BATTLE_REC_WORK *rec;
  REC_BATTLE_PARAM *rbp;

  if(brs == NULL){
    return TRUE;
  }

  rec = &brs->rec;
  rbp = &rec->rbp;
  for(i = 0; i < BTL_CLIENT_MAX; i++){
    if(rbp->server_version[i] > BTL_NET_SERVER_VERSION){
      return FALSE;
    }
  }
  return TRUE;
#else
  return TRUE;
#endif
}

//============================================================================================
/**
 *  BATTLE_PARAM構造体生成処理
 *
 * @param[in] bp      BATTLE_PARAM構造体へのポインタ
 * @param[in] sv      セーブ領域へのポインタ
 */
//============================================================================================
void BattleRec_BattleParamCreate(BATTLE_PARAM *bp,SAVE_CONTROL_WORK *sv)
{
  int i;
  BATTLE_REC_WORK *rec = &brs->rec;

#if 0 //※check　未作成　録画データの仕様が決まってから 2009.11.18(水)
  bp->fight_type      = rec->rbp.fight_type;
//  bp->win_lose_flag   = rec->rbp.win_lose_flag;
//  bp->btr         = rec->rbp.btr;
  bp->bg_id       = rec->rbp.bg_id;
  bp->ground_id     = rec->rbp.ground_id;
  bp->place_id      = rec->rbp.place_id;
  bp->zone_id       = rec->rbp.zone_id;
  bp->time_zone     = rec->rbp.time_zone;
  bp->shinka_place_mode = rec->rbp.shinka_place_mode;
  bp->contest_see_flag  = rec->rbp.contest_see_flag;
  bp->mizuki_flag     = rec->rbp.mizuki_flag;
  bp->get_pokemon_client  = rec->rbp.get_pokemon_client;
  bp->weather       = rec->rbp.weather;
//  bp->level_up_flag   = rec->rbp.level_up_flag;
  bp->battle_status_flag  = rec->rbp.battle_status_flag|BATTLE_STATUS_FLAG_REC_BATTLE;
  bp->safari_ball     = rec->rbp.safari_ball;
  bp->regulation_flag   = rec->rbp.regulation_flag;
  bp->rand        = rec->rbp.rand;
  bp->comm_id       = rec->rbp.comm_id;
//  bp->total_turn      = rec->rbp.total_turn;

  bp->win_lose_flag   = 0;
  bp->level_up_flag   = 0;

  ZukanWork_Copy(SaveData_GetZukanWork(sv),bp->zw);

  for(i=0;i<BTL_CLIENT_MAX;i++){
    bp->trainer_id[i]=rec->rbp.trainer_id[i];
    bp->trainer_data[i]=rec->rbp.trainer_data[i];
    bp->server_version[i]=rec->rbp.server_version[i];
    bp->comm_stand_no[i]=rec->rbp.comm_stand_no[i];
    RecPokeParty_to_PokeParty(&rec->rec_party[i], bp->poke_party[i]);
    MyStatus_Copy(&rec->my_status[i],bp->my_status[i]);
    bp->voice_waza_param[i]=rec->rbp.voice_waza_param[i];
  }
//  CONFIG_Copy(&rec->config,bp->config);
  CONFIG_Copy(SaveData_GetConfig(sv), bp->config);
  bp->config->window_type = rec->config.window_type;
  if(bp->config->window_type >= TALK_WINDOW_MAX){
    bp->config->window_type = 0;  //金銀で会話ウィンドウが増える事を考えてケアを入れておく
  }
#endif
}

//--------------------------------------------------------------
/**
 * @brief   POKEPARTYをREC_POKEPARTYに変換する
 *
 * @param   party     変換元データへのポインタ
 * @param   rec_party   変換後のデータ代入先
 */
//--------------------------------------------------------------
static void PokeParty_to_RecPokeParty( const POKEPARTY *party, REC_POKEPARTY *rec_party )
{
  int i;
  POKEMON_PARAM *pp;

  GFL_STD_MemClear(rec_party, sizeof(REC_POKEPARTY));

  rec_party->PokeCountMax = PokeParty_GetPokeCountMax(party);
  rec_party->PokeCount = PokeParty_GetPokeCount(party);

  for(i=0; i < rec_party->PokeCount; i++){
    pp = PokeParty_GetMemberPointer(party, i);
    POKETOOL_PokePara_to_RecPokePara(pp, &rec_party->member[i]);
  }
}

//--------------------------------------------------------------
/**
 * @brief   REC_POKEPARTYをPOKEPARTYに変換する
 *
 * @param   rec_party   変換元データへのポインタ
 * @param   party     変換後のデータ代入先
 */
//--------------------------------------------------------------
static void RecPokeParty_to_PokeParty( REC_POKEPARTY *rec_party, POKEPARTY *party, HEAPID heapID )
{
  int i;
  POKEMON_PARAM *pp;
  u8 cb_id_para = 0;

  pp = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(heapID), POKETOOL_GetWorkSize() );
  TAYA_Printf("RecParty pokeCnt=%d\n", rec_party->PokeCount);

  PokeParty_Init(party, rec_party->PokeCountMax);
  for(i = 0; i < rec_party->PokeCount; i++){
    POKETOOL_RecPokePara_to_PokePara(&rec_party->member[i], pp);
    PP_Put(pp, ID_PARA_cb_id, cb_id_para);  //カスタムボールは出ないようにする
    PokeParty_Add(party, pp);
  }

  GFL_HEAP_FreeMemory( pp );
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

BTLREC_SETUP_SUBSET*  BattleRec_GetSetupSubsetPtr( void )
{
  GF_ASSERT(brs);
  return &(brs->rec.setupSubset);
}

















//--------------------------------------------------------------
/**
 * @brief   戦闘録画データから、ヘッダーを別途確保したワークにコピーする
 *
 * @param   heap_id ヒープID
 *
 * @retval  確保されたヘッダーデータへのポインタ
 *
 * BattleRec_Loadを使用してデータをロードしている必要があります。
 */
//--------------------------------------------------------------
BATTLE_REC_HEADER_PTR BattleRec_HeaderAllocCopy(HEAPID heap_id)
{
  BATTLE_REC_HEADER_PTR head;

  GF_ASSERT(brs != NULL);

  head = GFL_HEAP_AllocMemory(heap_id, sizeof(BATTLE_REC_HEADER));
  GFL_STD_MemCopy32(&brs->head, head, sizeof(BATTLE_REC_HEADER));
  return head;
}

//--------------------------------------------------------------
/**
 * @brief   戦闘録画データから、GDSプロフィールを別途確保したワークにコピーする
 *
 * @param   src   戦闘録画セーブデータへのポインタ
 * @param   heap_id ヒープID
 *
 * @retval  確保されたGDSプロフィールデータへのポインタ
 *
 * BattleRec_Loadを使用してbrsにデータをロードしている必要があります。
 */
//--------------------------------------------------------------
GDS_PROFILE_PTR BattleRec_GDSProfileAllocCopy(HEAPID heap_id)
{
  GDS_PROFILE_PTR profile;

  GF_ASSERT(brs != NULL);

  profile = GFL_HEAP_AllocMemory(heap_id, sizeof(GDS_PROFILE));
  GFL_STD_MemCopy32(&brs->profile, profile, sizeof(GDS_PROFILE));
  return profile;

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
  return &brs->profile;
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
 * @brief   読み込んでいる録画データに対して、指定データで上書きする
 *
 * @param   num         LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…
 * @param   gpp     GDSプロフィール
 * @param   head    録画ヘッダ
 * @param   rec     録画本体
 * @param bp      そのまま再生する場合はBATTLE_PARAM構造体へのポインタを渡す
 * @param   sv      セーブデータへのポインタ
 *
 * BattleRec_Loadを使用してデータをロードしている必要があります。
 */
//--------------------------------------------------------------
void BattleRec_DataSet(GDS_PROFILE_PTR gpp, BATTLE_REC_HEADER_PTR head, BATTLE_REC_WORK_PTR rec, BATTLE_PARAM *bp, SAVE_CONTROL_WORK *sv)
{
  GF_ASSERT(brs != NULL);
  GFL_STD_MemCopy(head, &brs->head, sizeof(BATTLE_REC_HEADER));
  GFL_STD_MemCopy(rec, &brs->rec, sizeof(BATTLE_REC_WORK));
  GFL_STD_MemCopy(gpp, &brs->profile, sizeof(GDS_PROFILE));

  //復号
  BattleRec_Decoded(&brs->rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE,
    brs->rec.crc.crc16ccitt_hash + ((brs->rec.crc.crc16ccitt_hash ^ 0xffff) << 16));

  if(bp != NULL){
    BattleRec_BattleParamCreate(bp,sv);
  }
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
    return head->form_no[param];

  case RECHEAD_IDX_COUNTER:
    if ( head->battle_counter > REC_COUNTER_MAX ){
      return REC_COUNTER_MAX;
    }
    return head->battle_counter;

  case RECHEAD_IDX_MODE:
    if(head->mode >= BATTLE_MODE_MAX){
      return BATTLE_MODE_COLOSSEUM_SINGLE_FREE;
    }
    return head->mode;
  case RECHEAD_IDX_DATA_NUMBER:
    return head->data_number;
  case RECHEAD_IDX_SECURE:
    return head->secure;
  }

  GF_ASSERT(0); //不明なINDEX
  return 0;
}

//--------------------------------------------------------------
/**
 * @brief   戦闘録画ヘッダをAllocする
 *
 * @param   heap_id   ヒープID
 *
 * @retval  GDSプロフィールワークへのポインタ
 */
//--------------------------------------------------------------
BATTLE_REC_HEADER_PTR BattleRec_Header_AllocMemory(HEAPID heap_id)
{
  BATTLE_REC_HEADER_PTR brhp;

  brhp = GFL_HEAP_AllocMemory(heap_id, sizeof(BATTLE_REC_HEADER));
  GFL_STD_MemClear(brhp, sizeof(BATTLE_REC_HEADER));
  return brhp;
}

//--------------------------------------------------------------
/**
 * @brief   戦闘録画ヘッダを解放
 *
 * @param   brhp    GDSプロフィールへのポインタ
 */
//--------------------------------------------------------------
void BattleRec_Header_FreeMemory(BATTLE_REC_HEADER_PTR brhp)
{
  GFL_HEAP_FreeMemory(brhp);
}






//=====================================================================================================
// 録画データの格納・復元ツール
//=====================================================================================================
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




//=============================================================================================
/**
 * バトルセットアップパラメータを録画セーブデータに変換して録画セーブバッファに格納する
 *
 * ※事前に BattleRec_LoadToolModule の呼び出しが必要。
 *    使い終わったら BattleRec_UnoadToolModule を呼び出してください。
 *
 * @param   setup   バトルセットアップパラメータ
 */
//=============================================================================================
void BattleRec_StoreSetupParam( const BATTLE_SETUP_PARAM* setup )
{
  BATTLE_REC_WORK  *rec = &brs->rec;

  store_Party( setup, rec );
  store_ClientStatus( setup, rec );
  store_OperationBuffer( setup, rec );
  store_SetupSubset( setup, rec );
}

//=============================================================================================
/**
 * 録画セーブバッファからバトルセットアップパラメータを復元する
 *
 * ※事前に BattleRec_LoadToolModule の呼び出しが必要。
 *    使い終わったら BattleRec_UnoadToolModule を呼び出してください。
 *
 * @param   setup   [out] 復元先
 */
//=============================================================================================
void BattleRec_RestoreSetupParam( BATTLE_SETUP_PARAM* setup, HEAPID heapID )
{
  BATTLE_REC_WORK  *rec = &brs->rec;

  TAYA_Printf("*** Rec Info ***\n");
  TAYA_Printf(" recWorkSize = %d byte\n", sizeof(BATTLE_REC_WORK));
  TAYA_Printf("   ->setupSubset  = %d byte\n", sizeof(rec->setupSubset) );
  TAYA_Printf("   ->opBuffer     = %d byte\n", sizeof(rec->opBuffer) );
  TAYA_Printf("   ->clientStatus = %d byte\n", sizeof(rec->clientStatus) );
  TAYA_Printf("   ->rec_party    = %d byte\n", sizeof(rec->rec_party) );


  restore_Party( setup, rec, heapID );
  restore_ClientStatus( setup, rec );
  restore_OperationBuffer( setup, rec );
  restore_SetupSubset( setup, rec );
}

//----------------------------------------------------------------------------------
/**
 * パーティデータ：録画セーブデータ化して格納
 *
 * @param   setup
 * @param   rec
 */
//----------------------------------------------------------------------------------
static void store_Party( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec )
{
  u32 i;
  for(i=0; i<BTL_CLIENT_NUM; ++i)
  {
    if( setup->party[i] ){
      PokeParty_to_RecPokeParty( setup->party[i], &(rec->rec_party[i]) );
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * パーティデータ：録画セーブデータ復元
 *
 * @param   setup
 * @param   rec
 */
//----------------------------------------------------------------------------------
static void restore_Party( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec, HEAPID heapID )
{
  u32 i;
  for(i=0; i<BTL_CLIENT_NUM; ++i)
  {
    if( setup->party[i] ){
      RecPokeParty_to_PokeParty( &(rec->rec_party[i]), setup->party[i], heapID );
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * クライアントステータス：録画セーブデータ化して格納
 *
 * @param   setup
 * @param   rec
 */
//----------------------------------------------------------------------------------
static void store_ClientStatus( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec )
{
  u32 i;
  for(i=0; i<NELEMS(setup->playerStatus); ++i)
  {
    if( setup->playerStatus[i] ){
      MyStatus_Copy( setup->playerStatus[i], &rec->clientStatus[i].player );
      rec->clientStatus[i].type = BTLREC_CLIENTSTATUS_PLAYER;
    }
    else if( setup->tr_data[i] ){
      store_TrainerData( setup->tr_data[i], &rec->clientStatus[i].trainer );
      rec->clientStatus[i].type = BTLREC_CLIENTSTATUS_TRAINER;
    }
    else{
      rec->clientStatus[i].type = BTLREC_CLIENTSTATUS_NONE;
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * クライアントステータス：録画セーブデータから復元
 *
 * @param   setup
 * @param   rec
 */
//----------------------------------------------------------------------------------
static void restore_ClientStatus( BATTLE_SETUP_PARAM* setup, const BATTLE_REC_WORK* rec )
{
  u32 i;
  for(i=0; i<NELEMS(setup->playerStatus); ++i)
  {
    switch( rec->clientStatus[i].type ){
    case BTLREC_CLIENTSTATUS_PLAYER:
      GF_ASSERT(setup->playerStatus[i]!=NULL);
      MyStatus_Copy( &rec->clientStatus[i].player, (MYSTATUS*)setup->playerStatus[i] );
      break;

    case BTLREC_CLIENTSTATUS_TRAINER:
      GF_ASSERT(setup->tr_data[i]!=NULL);
      restore_TrainerData( setup->tr_data[i], &rec->clientStatus[i].trainer );
      break;
    }
  }
}

/**
 *  トレーナーデータ：録画セーブデータ用に変換して格納
 */
static void store_TrainerData( const BSP_TRAINER_DATA* bspTrainer, BTLREC_TRAINER_STATUS* recTrainer )
{
  u32 i;

  recTrainer->tr_id     = bspTrainer->tr_id;
  recTrainer->tr_type   = bspTrainer->tr_type;
  recTrainer->ai_bit    = bspTrainer->ai_bit;
  recTrainer->win_word  = bspTrainer->win_word;
  recTrainer->lose_word = bspTrainer->lose_word;

  for(i=0; i<NELEMS(recTrainer->use_item); ++i){
    recTrainer->use_item[i] = bspTrainer->use_item[i];
  }

  GFL_STR_GetStringCode( bspTrainer->name, recTrainer->name, NELEMS(recTrainer->name) );
}
/**
 *  トレーナーデータ：録画セーブデータから復元
 */
static void restore_TrainerData( BSP_TRAINER_DATA* bspTrainer, const BTLREC_TRAINER_STATUS* recTrainer )
{
  u32 i;

  bspTrainer->tr_id     = recTrainer->tr_id;
  bspTrainer->tr_type   = recTrainer->tr_type;
  bspTrainer->ai_bit    = recTrainer->ai_bit;
  bspTrainer->win_word  = recTrainer->win_word;
  bspTrainer->lose_word = recTrainer->lose_word;

  for(i=0; i<NELEMS(bspTrainer->use_item); ++i){
    bspTrainer->use_item[i] = recTrainer->use_item[i];
  }

  GFL_STR_SetStringCode( bspTrainer->name, recTrainer->name );
}
//----------------------------------------------------------------------------------
/**
 * 操作バッファ：録画セーブデータ用に変換して格納
 *
 * @param   setup
 * @param   rec
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL store_OperationBuffer( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec )
{
  if( (setup->recBuffer != NULL)
  &&  (setup->recDataSize < sizeof(rec->opBuffer.buffer))
  ){
    rec->opBuffer.size = setup->recDataSize;
    GFL_STD_MemCopy( setup->recBuffer, rec->opBuffer.buffer, setup->recDataSize );
    TAYA_Printf("Store Operation Buffer %dbyte\n", rec->opBuffer.size);
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 操作バッファ：録画セーブデータから復元
 *
 * @param   setup
 * @param   rec
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL restore_OperationBuffer( BATTLE_SETUP_PARAM* setup, const BATTLE_REC_WORK* rec )
{
  setup->recDataSize = rec->opBuffer.size;
  TAYA_Printf("Restore Operation Buffer %dbyte\n", setup->recDataSize );
  GFL_STD_MemCopy( rec->opBuffer.buffer, setup->recBuffer, setup->recDataSize );
  return TRUE;
}
//----------------------------------------------------------------------------------
/**
 * セットアップパラメータ復元データを録画セーブデータ変換して格納
 *
 * @param   setup
 * @param   rec
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL store_SetupSubset( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec )
{
  rec->setupSubset.fieldSituation = setup->fieldSituation;
  rec->setupSubset.randomContext = setup->recRandContext;
  rec->setupSubset.musicDefault = setup->musicDefault;
  rec->setupSubset.musicPinch = setup->musicPinch;

  rec->setupSubset.competitor = setup->competitor;
  rec->setupSubset.rule = setup->rule;
  rec->setupSubset.fMultiMode = setup->multiMode;
  rec->setupSubset.debugFlagBit = setup->DebugFlagBit;
  rec->setupSubset.myCommPos = setup->commPos;

  CONFIG_Copy( setup->configData, &rec->setupSubset.config );

  {
    u32 i;
    const u8* p = (u8*)(&(rec->setupSubset.randomContext));
    TAYA_Printf("*** 書き戻したRandomContext ***\n");
    for(i=0; i<sizeof(setup->recRandContext); ++i)
    {
      TAYA_Printf("%02x ", p[i]);
      if( (i+1)%8 == 0 ){
        TAYA_Printf("\n");
      }
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------------
/**
 * セットアップパラメータ復元
 *
 * @param   setup
 * @param   rec
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL restore_SetupSubset( BATTLE_SETUP_PARAM* setup, const BATTLE_REC_WORK* rec )
{
  setup->fieldSituation = rec->setupSubset.fieldSituation;
  setup->recRandContext = rec->setupSubset.randomContext;
  setup->musicDefault = rec->setupSubset.musicDefault;
  setup->musicPinch = rec->setupSubset.musicPinch;

  setup->competitor = rec->setupSubset.competitor;
  setup->rule = rec->setupSubset.rule;
  setup->multiMode = rec->setupSubset.fMultiMode;
  setup->DebugFlagBit = rec->setupSubset.debugFlagBit;
  setup->commPos = rec->setupSubset.myCommPos;

  CONFIG_Copy( &rec->setupSubset.config, (CONFIG*)(setup->configData) );

  return TRUE;
}

