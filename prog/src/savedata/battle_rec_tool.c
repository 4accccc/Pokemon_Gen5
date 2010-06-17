//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file     battle_rec_tool.c
 *  @brief    battle_rec_save系
 *  @author   Toru=Nagihashi
 *  @data     2010.05.06
 *
 *  battle_rec.cのオーバーレイが大きくなっていたので、
 * BattleRec_LoadToolModule～BattleRec_UnloadToolModuleを使うモジュールを独立
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
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
#include "net_app/gds/gds_profile_local.h"
#include "battle_rec_local.h"
#include "net_app/gds/gds_battle_mode.h"

//=============================================================================
/**
 *          定数宣言
*/
//=============================================================================

extern BATTLE_REC_SAVEDATA * brs;

//=============================================================================
/**
 *          構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *          プロトタイプ宣言
*/
//=============================================================================
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

//=============================================================================
/**
 *    プロトタイプ
 */
//=============================================================================

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

  if(PokeParty_GetPokeCount(party) == 0){  //不正チェック用に空の時は全て0でクリアされたまま終了
    return;
  }

  rec_party->PokeCountMax = PokeParty_GetPokeCountMax(party);
  rec_party->PokeCount = PokeParty_GetPokeCount(party);

  for(i=0; i < rec_party->PokeCount; i++){
    pp = PokeParty_GetMemberPointer(party, i);
    POKETOOL_PokePara_to_RecPokePara(pp, &rec_party->member[i]);
    NAGI_Printf( "poke[%d]=%d\n", i, PP_Get( pp, ID_PARA_monsno, NULL ) );
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

  //不正チェック用に未使用領域は全て0で埋める必要がある為、一旦バッファ全体を0クリアしておく
  GFL_STD_MemClear(rec->rec_party, sizeof(REC_POKEPARTY) * BTL_CLIENT_MAX);

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
    // ここのチェック順番は変えてはいけません
    // （btl_setup.cでサブウェイトレーナーにplayerStatusを設定している）
    if( setup->tr_data[i] ){
      store_TrainerData( setup->tr_data[i], &rec->clientStatus[i].trainer );
      rec->clientStatus[i].type = BTLREC_CLIENTSTATUS_TRAINER;
    }
    else if( setup->playerStatus[i] ){
      MyStatus_Copy( setup->playerStatus[i], &rec->clientStatus[i].player );
      rec->clientStatus[i].type = BTLREC_CLIENTSTATUS_PLAYER;
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

  {
    const STRCODE* sp = GFL_STR_GetStringCodePointer( bspTrainer->name );
    OS_TPrintf("Rec -> Btl  trID=%d, trType=%d, name= ", bspTrainer->tr_id, bspTrainer->tr_type);
    while( (*sp) != GFL_STR_GetEOMCode() ){
      OS_TPrintf( "%04x,", (*sp) );
      ++sp;
    }
  }
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
  rec->setupSubset.musicWin = setup->musicWin;

  rec->setupSubset.competitor = setup->competitor;
  rec->setupSubset.rule = setup->rule;
  rec->setupSubset.MultiMode = setup->multiMode;
  rec->setupSubset.debugFlagBit = setup->DebugFlagBit;
  rec->setupSubset.myCommPos = setup->commPos;
  rec->setupSubset.shooterBit = (setup->shooterBitWork.shooter_use != 0);

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
  setup->musicWin = rec->setupSubset.musicWin;

  setup->competitor = rec->setupSubset.competitor;
  setup->rule = rec->setupSubset.rule;
  setup->multiMode = rec->setupSubset.MultiMode;
  setup->DebugFlagBit = rec->setupSubset.debugFlagBit;
  setup->commPos = rec->setupSubset.myCommPos;
  setup->shooterBitWork.shooter_use = rec->setupSubset.shooterBit;

// コンフィグは録画データに含めず、再生するユーザのローカル設定に準じる (2010.06.17）
//  CONFIG_Copy( &rec->setupSubset.config, (CONFIG*)(setup->configData) );

  return TRUE;
}

