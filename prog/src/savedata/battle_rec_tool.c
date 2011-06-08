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
#include "pokewood_rec_local.h"

#ifdef PM_DEBUG
#define DEBUG_BATTLE_REC_TOOL_PRINT
#endif //PM_DEBUG

#ifdef DEBUG_BATTLE_REC_TOOL_PRINT
#if defined(DEBUG_ONLY_FOR_nagihashi_toru) || defined(DEBUG_ONLY_FOR_mori)
#define BATTLE_REC_TOOL_Printf(...) OS_TPrintf(__VA_ARGS__)
#endif //defined
#endif //DEBUG_BATTLE_REC_TOOL_PRINT

#ifndef BATTLE_REC_TOOL_Printf
#define BATTLE_REC_TOOL_Printf(...) /*  */
#endif //BATTLE_REC_TOOL_Printf

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
//汎用
static void store_TrainerData( const BSP_TRAINER_DATA* bspTrainer, BTLREC_TRAINER_STATUS* recTrainer );
static void restore_TrainerData( BSP_TRAINER_DATA* bspTrainer, const BTLREC_TRAINER_STATUS* recTrainer );
static BOOL store_SetupSubset( const BATTLE_SETUP_PARAM* setup, BTLREC_SETUP_SUBSET* setupSubset );
static BOOL restore_SetupSubset( BATTLE_SETUP_PARAM* setup, const BTLREC_SETUP_SUBSET* setupSubset );
static void store_ClientStatus( const BATTLE_SETUP_PARAM* setup, BTLREC_CLIENT_STATUS clientStatus[], int client_num );
static void restore_ClientStatus( BATTLE_SETUP_PARAM* setup, const BTLREC_CLIENT_STATUS clientStatus[], int client_num );

//通常録画
static void btl_store_Party( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec );
static void btl_restore_Party( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec, HEAPID heapID );
static BOOL btl_store_OperationBuffer( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec );
static BOOL btl_restore_OperationBuffer( BATTLE_SETUP_PARAM* setup, const BATTLE_REC_WORK* rec );
static void Btl_PokeParty_to_RecPokeParty( const POKEPARTY *party, REC_POKEPARTY *rec_party );
static void Btl_RecPokeParty_to_PokeParty( REC_POKEPARTY *rec_party, POKEPARTY *party, HEAPID heapID );

//ポケウッド用
static void pokewood_store_Party( const BATTLE_SETUP_PARAM* setup, POKEWOOD_REC_WORK* rec );
static void pokewood_restore_Party( const BATTLE_SETUP_PARAM* setup, POKEWOOD_REC_WORK* rec, HEAPID heapID );
static BOOL pokewood_store_OperationBuffer( const BATTLE_SETUP_PARAM* setup, POKEWOOD_REC_WORK* rec );
static BOOL pokewood_restore_OperationBuffer( BATTLE_SETUP_PARAM* setup, const POKEWOOD_REC_WORK* rec );
static void pokewood_PokeParty_to_RecPokeParty( const POKEPARTY *party, POKEWOOD_REC_POKEPARTY *rec_party );
static void pokewood_RecPokeParty_to_PokeParty( POKEWOOD_REC_POKEPARTY *rec_party, POKEPARTY *party, HEAPID heapID );

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
  switch( setup->PokewoodMode )
  {
  default:
    GF_ASSERT(0);
  case BTL_POKEWOOD_NONE:
    {
      BATTLE_REC_WORK  *rec = &brs->rec;

      btl_store_Party( setup, rec );
      store_ClientStatus( setup, rec->clientStatus, BTL_CLIENT_NUM );
      btl_store_OperationBuffer( setup, rec );
      store_SetupSubset( setup, &rec->setupSubset );
    }
    break;

  case BTL_POKEWOOD_REC:
  case BTL_POKEWOOD_SHOW:
    {
      POKEWOOD_REC_WORK  *rec = POKEWOOD_REC_GetWork();

      pokewood_store_Party( setup, rec );
      store_ClientStatus( setup, rec->clientStatus, POKEWOOD_REC_CLIENT_MAX );
      pokewood_store_OperationBuffer( setup, rec );
      store_SetupSubset( setup, &rec->setupSubset );
    }
    break;
  }
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
void BattleRec_RestoreSetupParam( BATTLE_SETUP_PARAM* setup, BTLREC_RESOTRE_MODE mode, HEAPID heapID )
{

  switch( mode )
  {
  default:
    GF_ASSERT(0);
  case BTLREC_RESOTRE_MODE_NORMAL:
    {
      BATTLE_REC_WORK  *rec = &brs->rec;

	    BATTLE_REC_TOOL_Printf("*** Btl Rec Info ***\n");
	    BATTLE_REC_TOOL_Printf(" recWorkSize = %d byte\n", sizeof(BATTLE_REC_WORK));
	    BATTLE_REC_TOOL_Printf("   ->setupSubset  = %d byte\n", sizeof(rec->setupSubset) );
	    BATTLE_REC_TOOL_Printf("   ->opBuffer     = %d byte\n", sizeof(rec->opBuffer) );
	    BATTLE_REC_TOOL_Printf("   ->clientStatus = %d byte\n", sizeof(rec->clientStatus) );
	    BATTLE_REC_TOOL_Printf("   ->rec_party    = %d byte\n", sizeof(rec->rec_party) );
	
	    btl_restore_Party( setup, rec, heapID );
	    restore_ClientStatus( setup, rec->clientStatus, BTL_CLIENT_NUM );
	    btl_restore_OperationBuffer( setup, rec );
	    restore_SetupSubset( setup, &rec->setupSubset );
    }
    break;

  case BTLREC_RESOTRE_MODE_POKEWOOD:
    {
      POKEWOOD_REC_WORK  *rec = POKEWOOD_REC_GetWork();

	    BATTLE_REC_TOOL_Printf("*** PokeWood Rec Info ***\n");
	    BATTLE_REC_TOOL_Printf(" recWorkSize = %d byte\n", sizeof(BATTLE_REC_WORK));
	    BATTLE_REC_TOOL_Printf("   ->setupSubset  = %d byte\n", sizeof(rec->setupSubset) );
	    BATTLE_REC_TOOL_Printf("   ->opBuffer     = %d byte\n", sizeof(rec->opBuffer) );
	    BATTLE_REC_TOOL_Printf("   ->clientStatus = %d byte\n", sizeof(rec->clientStatus) );
	    BATTLE_REC_TOOL_Printf("   ->rec_party    = %d byte\n", sizeof(rec->rec_party) );
	
	    pokewood_restore_Party( setup, rec, heapID );
	    restore_ClientStatus( setup, rec->clientStatus, POKEWOOD_REC_CLIENT_MAX );
	    pokewood_restore_OperationBuffer( setup, rec );
	    restore_SetupSubset( setup, &rec->setupSubset );
    }
    break;
  }
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
static void Btl_PokeParty_to_RecPokeParty( const POKEPARTY *party, REC_POKEPARTY *rec_party )
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
static void Btl_RecPokeParty_to_PokeParty( REC_POKEPARTY *rec_party, POKEPARTY *party, HEAPID heapID )
{
  int i;
  POKEMON_PARAM *pp;
  u8 cb_id_para = 0;

  pp = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(heapID), POKETOOL_GetWorkSize() );
  BATTLE_REC_TOOL_Printf("RecParty pokeCnt=%d\n", rec_party->PokeCount);

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
static void btl_store_Party( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec )
{
  u32 i;

  //不正チェック用に未使用領域は全て0で埋める必要がある為、一旦バッファ全体を0クリアしておく
  GFL_STD_MemClear(rec->rec_party, sizeof(REC_POKEPARTY) * BTL_CLIENT_MAX);

  for(i=0; i<BTL_CLIENT_NUM; ++i)
  {
    if( setup->party[i] ){
      Btl_PokeParty_to_RecPokeParty( setup->party[i], &(rec->rec_party[i]) );
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
static void btl_restore_Party( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec, HEAPID heapID )
{
  u32 i;
  for(i=0; i<BTL_CLIENT_NUM; ++i)
  {
    if( setup->party[i] ){
      Btl_RecPokeParty_to_PokeParty( &(rec->rec_party[i]), setup->party[i], heapID );
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
static void store_ClientStatus( const BATTLE_SETUP_PARAM* setup, BTLREC_CLIENT_STATUS clientStatus[], int client_num )
{
  u32 i;
  for(i=0; i<client_num; ++i)
  {
    // ここのチェック順番（トレーナーデータ->プレイヤーデータ）は変えてはいけません
    // （btl_setup.cでサブウェイトレーナーにplayerStatusを設定しているので）
    if( setup->tr_data[i] ){
      store_TrainerData( setup->tr_data[i], &clientStatus[i].trainer );
      clientStatus[i].type = BTLREC_CLIENTSTATUS_TRAINER;
    }
    else if( setup->playerStatus[i] ){
      MyStatus_Copy( setup->playerStatus[i], &clientStatus[i].player );
      clientStatus[i].type = BTLREC_CLIENTSTATUS_PLAYER;
    }
    else{
      clientStatus[i].type = BTLREC_CLIENTSTATUS_NONE;
    }
    clientStatus[i].voiceLevel = setup->perappVoiceLevel[ i ];
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
static void restore_ClientStatus( BATTLE_SETUP_PARAM* setup, const BTLREC_CLIENT_STATUS clientStatus[], int client_num )
{
  u32 i;
  for(i=0; i<client_num; ++i)
  {
    switch( clientStatus[i].type ){
    case BTLREC_CLIENTSTATUS_PLAYER:
      GF_ASSERT(setup->playerStatus[i]!=NULL);
      MyStatus_Copy( &clientStatus[i].player, (MYSTATUS*)setup->playerStatus[i] );
      break;

    case BTLREC_CLIENTSTATUS_TRAINER:
      GF_ASSERT(setup->tr_data[i]!=NULL);
      restore_TrainerData( setup->tr_data[i], &clientStatus[i].trainer );
      break;
    }

    setup->perappVoiceLevel[i] = clientStatus[i].voiceLevel;
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
static BOOL btl_store_OperationBuffer( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec )
{
  if( (setup->recBuffer != NULL)
  &&  (setup->recDataSize < sizeof(rec->opBuffer.buffer))
  ){
    rec->opBuffer.size = setup->recDataSize;
    GFL_STD_MemCopy( setup->recBuffer, rec->opBuffer.buffer, setup->recDataSize );
    BATTLE_REC_TOOL_Printf("Store Operation Buffer %dbyte\n", rec->opBuffer.size);
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
static BOOL btl_restore_OperationBuffer( BATTLE_SETUP_PARAM* setup, const BATTLE_REC_WORK* rec )
{
  setup->recDataSize = rec->opBuffer.size;
  BATTLE_REC_TOOL_Printf("Restore Operation Buffer %dbyte\n", setup->recDataSize );
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
static BOOL store_SetupSubset( const BATTLE_SETUP_PARAM* setup, BTLREC_SETUP_SUBSET* setupSubset )
{
  setupSubset->fieldSituation = setup->fieldSituation;
  setupSubset->randomContext = setup->recRandContext;
  setupSubset->musicDefault = setup->musicDefault;
  setupSubset->musicWin = setup->musicWin;

  setupSubset->competitor = setup->competitor;
  setupSubset->rule = setup->rule;
  setupSubset->MultiMode = setup->multiMode;
  setupSubset->debugFlagBit = setup->DebugFlagBit;
  setupSubset->myCommPos = setup->commPos;
  setupSubset->shooterBit = (setup->shooterBitWork.shooter_use != 0);

  CONFIG_Copy( setup->configData, &setupSubset->config );

  {
    u32 i;
    const u8* p = (u8*)(&(setupSubset->randomContext));
    BATTLE_REC_TOOL_Printf("*** 書き戻したRandomContext ***\n");
    for(i=0; i<sizeof(setup->recRandContext); ++i)
    {
      BATTLE_REC_TOOL_Printf("%02x ", p[i]);
      if( (i+1)%8 == 0 ){
        BATTLE_REC_TOOL_Printf("\n");
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
static BOOL restore_SetupSubset( BATTLE_SETUP_PARAM* setup, const BTLREC_SETUP_SUBSET* setupSubset )
{
  setup->fieldSituation = setupSubset->fieldSituation;
  setup->recRandContext = setupSubset->randomContext;
  setup->musicDefault = setupSubset->musicDefault;
  setup->musicWin = setupSubset->musicWin;

  setup->competitor = setupSubset->competitor;
  setup->rule = setupSubset->rule;
  setup->multiMode = setupSubset->MultiMode;
  setup->DebugFlagBit = setupSubset->debugFlagBit;
  setup->commPos = setupSubset->myCommPos;
  setup->shooterBitWork.shooter_use = setupSubset->shooterBit;

// コンフィグは録画データに含めず、再生するユーザのローカル設定に準じる (2010.06.17）
//  CONFIG_Copy( &setupSubset->config, (CONFIG*)(setup->configData) );

  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief ポケウッド用　パーティ格納
 *
 *	@param	const BATTLE_SETUP_PARAM* setup バトル
 *	@param	rec   録画
 */
//-----------------------------------------------------------------------------
static void pokewood_store_Party( const BATTLE_SETUP_PARAM* setup, POKEWOOD_REC_WORK* rec )
{
  u32 i;

  //不正チェック用に未使用領域は全て0で埋める必要がある為、一旦バッファ全体を0クリアしておく
  GFL_STD_MemClear(rec->rec_party, sizeof(POKEWOOD_REC_POKEPARTY) * POKEWOOD_REC_CLIENT_MAX);

  //TODO　今後、敵のデータはROMから読み込むかも。

  for(i=0; i<POKEWOOD_REC_CLIENT_MAX; ++i)
  {
    if( setup->party[i] ){
      pokewood_PokeParty_to_RecPokeParty( setup->party[i], &(rec->rec_party[i]) );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケウッド用  パーティ格納
 *
 *	@param	const BATTLE_SETUP_PARAM* setup バトル
 *	@param	rec　録画
 *	@param	heapID 　ヒープID
 */
//-----------------------------------------------------------------------------
static void pokewood_restore_Party( const BATTLE_SETUP_PARAM* setup, POKEWOOD_REC_WORK* rec, HEAPID heapID )
{
  u32 i;
  for(i=0; i<POKEWOOD_REC_CLIENT_MAX; ++i)
  {
    if( setup->party[i] ){
      pokewood_RecPokeParty_to_PokeParty( &(rec->rec_party[i]), setup->party[i], heapID );
    }
  }
}
static BOOL pokewood_store_OperationBuffer( const BATTLE_SETUP_PARAM* setup, POKEWOOD_REC_WORK* rec )
{
  if( (setup->recBuffer != NULL)
  &&  (setup->recDataSize < sizeof(rec->opBuffer.buffer))
  ){
    rec->opBuffer.size = setup->recDataSize;
    GFL_STD_MemCopy( setup->recBuffer, rec->opBuffer.buffer, setup->recDataSize );
    BATTLE_REC_TOOL_Printf("Store Operation Buffer %dbyte\n", rec->opBuffer.size);
    return TRUE;
  }
  return FALSE;
}
static BOOL pokewood_restore_OperationBuffer( BATTLE_SETUP_PARAM* setup, const POKEWOOD_REC_WORK* rec )
{
  setup->recDataSize = rec->opBuffer.size;
  BATTLE_REC_TOOL_Printf("Restore Operation Buffer %dbyte\n", setup->recDataSize );
  GFL_STD_MemCopy( rec->opBuffer.buffer, setup->recBuffer, setup->recDataSize );
  return TRUE;
}
static void pokewood_PokeParty_to_RecPokeParty( const POKEPARTY *party, POKEWOOD_REC_POKEPARTY *rec_party )
{
  int i;
  POKEMON_PARAM *pp;

  GFL_STD_MemClear(rec_party, sizeof(REC_POKEPARTY));

  if(PokeParty_GetPokeCount(party) == 0){  //不正チェック用に空の時は全て0でクリアされたまま終了
    return;
  }

  rec_party->PokeCountMax = MATH_IMin( POKEWOOD_REC_TEMOTI_MAX, PokeParty_GetPokeCountMax(party) );

  rec_party->PokeCount = MATH_IMin( PokeParty_GetPokeCount(party), rec_party->PokeCountMax );

  for(i=0; i < rec_party->PokeCount; i++){
    pp = PokeParty_GetMemberPointer(party, i);
    POKETOOL_PokePara_to_RecPokePara(pp, &rec_party->member[i]);
    NAGI_Printf( "poke[%d]=%d\n", i, PP_Get( pp, ID_PARA_monsno, NULL ) );
  }
}
static void pokewood_RecPokeParty_to_PokeParty( POKEWOOD_REC_POKEPARTY *rec_party, POKEPARTY *party, HEAPID heapID )
{
  int i;
  POKEMON_PARAM *pp;
  u8 cb_id_para = 0;

  pp = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(heapID), POKETOOL_GetWorkSize() );
  BATTLE_REC_TOOL_Printf("RecParty pokeCnt=%d\n", rec_party->PokeCount);

  PokeParty_Init(party, TEMOTI_POKEMAX);
  for(i = 0; i < rec_party->PokeCount; i++){
    POKETOOL_RecPokePara_to_PokePara(&rec_party->member[i], pp);
    PP_Put(pp, ID_PARA_cb_id, cb_id_para);  //カスタムボールは出ないようにする
    PokeParty_Add(party, pp);
  }

  GFL_HEAP_FreeMemory( pp );
}
