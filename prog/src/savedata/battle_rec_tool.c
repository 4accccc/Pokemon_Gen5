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
#include "gamesystem/btl_setup.h"
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
#include "tr_tool/trtype_def.h"

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

//通常録画
static void btl_restore_ClientStatus( BATTLE_SETUP_PARAM* setup, const BTLREC_CLIENT_STATUS clientStatus[], int client_num );
static void btl_store_ClientStatus( const BATTLE_SETUP_PARAM* setup, BTLREC_CLIENT_STATUS clientStatus[], int client_num );
static void btl_store_Party( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec );
static void btl_restore_Party( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec, HEAPID heapID );
static BOOL btl_store_OperationBuffer( const BATTLE_SETUP_PARAM* setup, BATTLE_REC_WORK* rec );
static BOOL btl_restore_OperationBuffer( BATTLE_SETUP_PARAM* setup, const BATTLE_REC_WORK* rec );
static void Btl_PokeParty_to_RecPokeParty( const POKEPARTY *party, REC_POKEPARTY *rec_party );
static void Btl_RecPokeParty_to_PokeParty( REC_POKEPARTY *rec_party, POKEPARTY *party, HEAPID heapID );

//ポケウッド用
static void pokewood_restore_ClientStatus( BATTLE_SETUP_PARAM* setup, const BTLREC_CLIENT_STATUS clientStatus[], int client_num, u16 player_tr_type );
static void pokewood_store_ClientStatus( const BATTLE_SETUP_PARAM* setup, BTLREC_CLIENT_STATUS clientStatus[], int client_num, u16 *player_tr_type );
static void pokewood_store_Party( const BATTLE_SETUP_PARAM* setup, POKEWOOD_REC_WORK* rec );
static void pokewood_restore_Party( const BATTLE_SETUP_PARAM* setup, POKEWOOD_REC_WORK* rec, HEAPID heapID );
static BOOL pokewood_store_OperationBuffer( const BATTLE_SETUP_PARAM* setup, POKEWOOD_REC_WORK* rec );
static BOOL pokewood_restore_OperationBuffer( BATTLE_SETUP_PARAM* setup, const POKEWOOD_REC_WORK* rec );
static void pokewood_PokeParty_to_RecPokeParty( const POKEPARTY *party, POKEWOOD_REC_POKEPARTY *rec_party );
static void pokewood_RecPokeParty_to_PokeParty( POKEWOOD_REC_POKEPARTY *rec_party, POKEPARTY *party, HEAPID heapID );
static void pokewood_store_Senario( const BATTLE_SETUP_PARAM* setup, POKEWOOD_REC_SENARIO *senario );
static void pokewood_restore_Senario( BATTLE_SETUP_PARAM* setup, const POKEWOOD_REC_SENARIO *senario );

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
      btl_store_ClientStatus( setup, rec->clientStatus, BTL_CLIENT_NUM );
      btl_store_OperationBuffer( setup, rec );
      store_SetupSubset( setup, &rec->setupSubset );
    }
    break;

  case BTL_POKEWOOD_REC:
  case BTL_POKEWOOD_SHOW:
    {
      POKEWOOD_REC_WORK  *rec = POKEWOOD_REC_GetWork();

      pokewood_store_Party( setup, rec );
      pokewood_store_ClientStatus( setup, rec->clientStatus, POKEWOOD_REC_CLIENT_MAX, &rec->player_tr_type );
      pokewood_store_OperationBuffer( setup, rec );
      store_SetupSubset( setup, &rec->setupSubset );
      pokewood_store_Senario( setup, &rec->senario );
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
	    btl_restore_ClientStatus( setup, rec->clientStatus, BTL_CLIENT_NUM );
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
	    pokewood_restore_ClientStatus( setup, rec->clientStatus, POKEWOOD_REC_CLIENT_MAX, rec->player_tr_type );
	    pokewood_restore_OperationBuffer( setup, rec );
	    restore_SetupSubset( setup, &rec->setupSubset );
      pokewood_restore_Senario( setup, &rec->senario );

      //ポケウッド再生は強制上映モード
      setup->PokewoodMode = BTL_POKEWOOD_SHOW;
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
static void btl_store_ClientStatus( const BATTLE_SETUP_PARAM* setup, BTLREC_CLIENT_STATUS clientStatus[], int client_num )
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
    OS_Printf("client[%d] sex=%d\n", i, MyStatus_GetMySex(&clientStatus[i].player));
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
static void pokewood_store_ClientStatus( const BATTLE_SETUP_PARAM* setup, BTLREC_CLIENT_STATUS clientStatus[], int client_num, u16 *player_tr_type )
{
  u32 i;
  for(i=0; i<client_num; ++i)
  {
    switch(i)
    {
    case BTL_CLIENT_PLAYER:
      MyStatus_Copy( setup->playerStatus[i], &clientStatus[i].player );
      *player_tr_type = setup->tr_data[i]->tr_type;
      clientStatus[i].type = BTLREC_CLIENTSTATUS_TRAINER;
      break;
    case BTL_CLIENT_ENEMY1:
      store_TrainerData( setup->tr_data[i], &clientStatus[i].trainer );
      clientStatus[i].type = BTLREC_CLIENTSTATUS_TRAINER;
      break;
    default:
      GF_ASSERT(0);
    }
    clientStatus[i].voiceLevel = setup->perappVoiceLevel[ i ];
    OS_Printf("client[%d] sex=%d\n", i, MyStatus_GetMySex(&clientStatus[i].player));
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
static void btl_restore_ClientStatus( BATTLE_SETUP_PARAM* setup, const BTLREC_CLIENT_STATUS clientStatus[], int client_num )
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
//----------------------------------------------------------------------------------
/**
 * クライアントステータス：録画セーブデータから復元
 *
 * @param   setup
 * @param   rec
 */
//----------------------------------------------------------------------------------
static void pokewood_restore_ClientStatus( BATTLE_SETUP_PARAM* setup, const BTLREC_CLIENT_STATUS clientStatus[], int client_num, u16 player_tr_type )
{
  u32 i;
  for(i=0; i<client_num; ++i)
  {
    switch( i ){
    case BTL_CLIENT_PLAYER:
      GF_ASSERT(setup->playerStatus[i]!=NULL);
      GF_ASSERT(setup->tr_data[i]!=NULL);
      MyStatus_Copy( &clientStatus[i].player, (MYSTATUS*)setup->playerStatus[i] );
      setup->tr_data[i]->tr_id = 1;//1とすることでトレーナーとしてだましている
      setup->tr_data[i]->tr_type = player_tr_type;
      break;

    case BTL_CLIENT_ENEMY1:
      GF_ASSERT(setup->tr_data[i]!=NULL);
      restore_TrainerData( setup->tr_data[i], &clientStatus[i].trainer );
      break;

    default:
      GF_ASSERT(0);
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
 *	@brief  ポケウッド用  パーティ復元
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
//----------------------------------------------------------------------------
/**
 *	@brief ポケウッド用操作バッファ格納
 *
 *	@param	const BATTLE_SETUP_PARAM* setup
 *	@param	rec 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
/**
 *	@brief ポケウッド用操作バッファ復元
 *
 *	@param	const BATTLE_SETUP_PARAM* setup
 *	@param	rec 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL pokewood_restore_OperationBuffer( BATTLE_SETUP_PARAM* setup, const POKEWOOD_REC_WORK* rec )
{
  setup->recDataSize = rec->opBuffer.size;
  BATTLE_REC_TOOL_Printf("Restore Operation Buffer %dbyte\n", setup->recDataSize );
  GFL_STD_MemCopy( rec->opBuffer.buffer, setup->recBuffer, setup->recDataSize );
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief　ポケウッド用パーティ変換
 *
 *	@param	const POKEPARTY *party
 *	@param	*rec_party 
 */
//-----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
/**
 *	@brief　ポケウッド用パーティ変換
 *
 *	@param	POKEWOOD_REC_POKEPARTY *rec_party
 *	@param	*party
 *	@param	heapID 
 */
//-----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
/**
 *	@brief　ポケウッド用シナリオデータ
 *
 *	@param	const BATTLE_SETUP_PARAM* setup
 *	@param	*senario 
 */
//-----------------------------------------------------------------------------
static void pokewood_store_Senario( const BATTLE_SETUP_PARAM* setup, POKEWOOD_REC_SENARIO *senario )
{
  senario->senario_no = setup->pokewoodSenario;
  senario->success    = setup->pokewoodSuccessFlag;
  GFL_STD_MemCopy( setup->pokewoodChoiceTbl, senario->choiceTbl, BTL_POKEWOOD_CHOICE_MAX );
}
//----------------------------------------------------------------------------
/**
 *	@brief　ポケウッド用シナリオデータ
 *
 *	@param	BATTLE_SETUP_PARAM* setup
 *	@param	POKEWOOD_REC_SENARIO *senario 
 */
//-----------------------------------------------------------------------------
static void pokewood_restore_Senario( BATTLE_SETUP_PARAM* setup, const POKEWOOD_REC_SENARIO *senario )
{
  setup->pokewoodSenario = senario->senario_no;
  setup->pokewoodSuccessFlag = senario->success;
  GFL_STD_MemCopy( senario->choiceTbl, setup->pokewoodChoiceTbl, BTL_POKEWOOD_CHOICE_MAX );
}


//----------------------------------------------------------------------------
/**
 *	@brief ポケウッドの撮影データを上映データに変換
 */
//-----------------------------------------------------------------------------
void POKEWOOD_REC_ConvertRecToShow( BATTLE_SETUP_PARAM* bsp, HEAPID heapID )
{
  enum
  {
    POKEWOOD_CONV_DATA_POKE_NUM = 3,
  };

  enum
  {
    POKEWOOD_TRDATA_PLAYER_M,
    POKEWOOD_TRDATA_ENEMY_M,
    POKEWOOD_TRDATA_PLAYER_F,
    POKEWOOD_TRDATA_ENEMY_F,
    POKEWOOD_TRDATA_MAX,
  };

  //TODO　本来ROMから読み出す予定だが、ないので
  //データべた書き
  static const struct _POKEWOOD_REC_CONV_DATA
  {
    struct _POKEWOOD_REC_TR_DATA
    {
      u32 tr_type;
      STRCODE trainer_name[PERSON_NAME_SIZE + EOM_SIZE];
      u32 trainer_name_length;
    } tr_data[POKEWOOD_TRDATA_MAX];

    struct _POKEWOOD_REC_POKE_DATA
    {
      u16 src_monsno; //変更前
      u16 dst_monsno; //変更先
    } poke_data[POKEWOOD_CONV_DATA_POKE_NUM];
  } sc_rec_to_show_data[] =
  {
    //シナリオ０
    {
      //トレーナーデータ
      {
        //味方
        {
          0,
          L"",
          0,
        },
        //敵
        {
          TRTYPE_SCIENTISTW,
          L"サトミ",
          3,
        },
        //味方
        {
          0,
          L"",
          0,
        },
        //敵
        {
          TRTYPE_SCIENTISTW,
          L"サトミ",
          3,
        },
      },
      //敵のポケモンデータ
      {
        MONSNO_ufo,
        MONSNO_ufo,
      }
    },
    //シナリオ１
    {
      //トレーナーデータ
      {
        //味方
        {
          TRTYPE_PKWD_HERO,
          L"",
          0,
        },
        //敵
        {
          TRTYPE_PKWD_VILLAIN,
          L"ハチクマン",
          5,
        },
        //味方（女）
        {
          TRTYPE_PKWD_HEROINE,
          L"",
          0,
        },
        //敵（女）
        {
          TRTYPE_PKWD_VILLAIN,
          L"ハチクマン",
          5,
        },
      },
      //敵のポケモンデータ
      {
        0,
      },
    },
  };

  const struct _POKEWOOD_REC_CONV_DATA *p_setup_data = &sc_rec_to_show_data[ bsp->pokewoodSenario ];

  GF_ASSERT(bsp->pokewoodSenario < NELEMS(sc_rec_to_show_data) );


  //敵トレーナーの設定
  {
    int i;
    int ofs;

    if( MyStatus_GetMySex( bsp->playerStatus[ BTL_CLIENT_PLAYER ] ) == PM_MALE )
    {
      ofs = POKEWOOD_TRDATA_PLAYER_M;
    }
    else
    {
      ofs = POKEWOOD_TRDATA_PLAYER_F;
    }

    for( i = BTL_CLIENT_PLAYER; i <= BTL_CLIENT_ENEMY1; i++ )
    {
      const struct _POKEWOOD_REC_TR_DATA *p_tr_data = &p_setup_data->tr_data[i+ofs];

      if( p_tr_data->tr_type != 0 || p_tr_data->trainer_name_length != 0 )
      {
        if( bsp->tr_data[i] == NULL )
        {

          bsp->tr_data[i] = BSP_TRAINER_DATA_Create( heapID );
        }
      }


      if( p_tr_data->tr_type != 0 )
      {
        bsp->tr_data[i]->tr_id   = 1;//1とすることでトレーナーとしてだましている
        bsp->tr_data[i]->tr_type = p_tr_data->tr_type;
        bsp->tr_data[i]->ai_bit  = 0;
      }

      if( p_tr_data->trainer_name_length != 0 )
      {
        STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];
        GFL_STD_MemCopy( p_tr_data->trainer_name, name, sizeof(STRCODE)*p_tr_data->trainer_name_length );
        name[p_tr_data->trainer_name_length] = GFL_STR_GetEOMCode();
        GFL_STR_SetStringCode( bsp->tr_data[i]->name, name );
      }
    }
  }

  //敵ポケモンの設定
  //置換するものがあったら変える
  {
    int i,j;

    POKEPARTY *p_party = bsp->party[BTL_CLIENT_ENEMY1];

    for( i = 0; i < PokeParty_GetPokeCount(p_party); i++ )
    {
      POKEMON_PARAM *p_pp = PokeParty_GetMemberPointer(p_party, i );
      if( PP_Get( p_pp, ID_PARA_poke_exist, NULL ) == 0 )
      {
        continue;
      }
      for( j = 0; j < POKEWOOD_CONV_DATA_POKE_NUM; j++ )
      {
        const struct _POKEWOOD_REC_POKE_DATA *p_poke_data = &p_setup_data->poke_data[j];

        if( p_poke_data->src_monsno == PP_Get( p_pp, ID_PARA_monsno, NULL ) )
        {
          PP_Put( p_pp, ID_PARA_monsno, p_poke_data->dst_monsno );
        }
      }
    }
  }
}
