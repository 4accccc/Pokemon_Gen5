
#include "sound/wb_sound_data.sadl" //サウンドラベルファイル
#include "gamesystem/btl_setup.h"


static void setup_common( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData, BtlLandForm landForm, BtlWeather weather )
{
  dst->netHandle = NULL;
  dst->commMode = BTL_COMM_NONE;
  dst->commPos = 0;
  dst->netID = 0;
  dst->multiMode = 0;
  dst->landForm = landForm;
  dst->weather  = weather;

  dst->partyPlayer = GAMEDATA_GetMyPokemon( gameData );
  dst->partyEnemy1 = NULL;
  dst->partyPartner = NULL;
  dst->partyEnemy2 = NULL;

  dst->statusPlayer = GAMEDATA_GetMyStatus( gameData );
  dst->itemData     = GAMEDATA_GetMyItem( gameData );
  dst->bagCursor    = GAMEDATA_GetBagCursor( gameData );

  dst->musicDefault = SEQ_BGM_VS_NORAPOKE;
  dst->musicPinch = SEQ_BGM_BATTLEPINCH;

  dst->result = BTL_RESULT_WIN;
}


//=============================================================================================
/**
 * シングル 野生戦
 *
 * @param   dst
 * @param   gameData
 * @param   partyEnemy
 * @param   landForm
 * @param   weather
 */
//=============================================================================================
void BTL_SETUP_Single_Wild( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  POKEPARTY* partyEnemy, BtlLandForm landForm, BtlWeather weather )
{
  setup_common( dst, gameData, landForm, weather );

  dst->engine = BTL_ENGINE_ALONE;
  dst->competitor = BTL_COMPETITOR_WILD;
  dst->rule = BTL_RULE_SINGLE;

  dst->partyEnemy1 = partyEnemy;
  dst->trID = TRID_NULL;
}

//=============================================================================================
/**
 * シングル ゲーム内トレーナー対戦
 *
 * @param   dst
 * @param   gameData
 * @param   partyEnemy
 * @param   landForm
 * @param   weather
 * @param   trID
 */
//=============================================================================================
void BTL_SETUP_Single_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  POKEPARTY* partyEnemy, BtlLandForm landForm, BtlWeather weather, TrainerID trID )
{
  setup_common( dst, gameData, landForm, weather );

  dst->engine = BTL_ENGINE_ALONE;
  dst->competitor = BTL_COMPETITOR_TRAINER;
  dst->rule = BTL_RULE_SINGLE;

  dst->partyEnemy1 = partyEnemy;
  dst->trID = trID;
}

//=============================================================================================
/**
 * シングル 通信対戦
 *
 * @param   dst
 * @param   gameData
 * @param   netHandle
 * @param   commMode
 */
//=============================================================================================
void BTL_SETUP_Single_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode )
{
  setup_common( dst, gameData, BTL_LANDFORM_ROOM, BTL_WEATHER_NONE );

  dst->engine = BTL_ENGINE_ALONE;
  dst->competitor = BTL_COMPETITOR_COMM;
  dst->rule = BTL_RULE_SINGLE;

  dst->netHandle = netHandle;
  dst->commMode = commMode;
  dst->netID = GFL_NET_GetNetID( netHandle );
  dst->multiMode = 0;

  dst->trID = TRID_NULL;
}

//=============================================================================================
/**
 * ダブル ゲーム内トレーナー対戦
 *
 * @param   dst
 * @param   gameData
 * @param   partyEnemy
 * @param   landForm
 * @param   weather
 * @param   trID
 */
//=============================================================================================
void BTL_SETUP_Double_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  POKEPARTY* partyEnemy, BtlLandForm landForm, BtlWeather weather, TrainerID trID )
{
  setup_common( dst, gameData, landForm, weather );

  dst->engine = BTL_ENGINE_ALONE;
  dst->competitor = BTL_COMPETITOR_TRAINER;
  dst->rule = BTL_RULE_DOUBLE;

  dst->partyEnemy1 = partyEnemy;
  dst->trID = trID;
}

//=============================================================================================
/**
 * ダブル 通信対戦
 *
 * @param   dst
 * @param   gameData
 * @param   netHandle
 * @param   commMode
 */
//=============================================================================================
void BTL_SETUP_Double_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode )
{
  setup_common( dst, gameData, BTL_LANDFORM_ROOM, BTL_WEATHER_NONE );

  dst->engine = BTL_ENGINE_ALONE;
  dst->competitor = BTL_COMPETITOR_COMM;
  dst->rule = BTL_RULE_DOUBLE;

  dst->netHandle = netHandle;
  dst->commMode = commMode;
  dst->netID = GFL_NET_GetNetID( netHandle );
  dst->multiMode = FALSE;

  dst->trID = TRID_NULL;
}

//=============================================================================================
/**
 * ダブル（マルチ）通信対戦
 *
 * @param   dst
 * @param   gameData
 * @param   netHandle
 * @param   commMode
 * @param   commPos
 */
//=============================================================================================
void BTL_SETUP_Multi_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode, u8 commPos )
{
  setup_common( dst, gameData, BTL_LANDFORM_ROOM, BTL_WEATHER_NONE );

  dst->engine = BTL_ENGINE_ALONE;
  dst->competitor = BTL_COMPETITOR_COMM;
  dst->rule = BTL_RULE_DOUBLE;

  dst->netHandle = netHandle;
  dst->commMode = commMode;
  dst->netID = GFL_NET_GetNetID( netHandle );
  dst->multiMode = TRUE;

  dst->trID = TRID_NULL;
}

//=============================================================================================
/**
 * トリプル ゲーム内トレーナー対戦
 *
 * @param   dst
 * @param   gameData
 * @param   partyEnemy
 * @param   landForm
 * @param   weather
 * @param   trID
 */
//=============================================================================================
void BTL_SETUP_Triple_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  POKEPARTY* partyEnemy, BtlLandForm landForm, BtlWeather weather, TrainerID trID )
{
  setup_common( dst, gameData, landForm, weather );

  dst->engine = BTL_ENGINE_ALONE;
  dst->competitor = BTL_COMPETITOR_TRAINER;
  dst->rule = BTL_RULE_TRIPLE;

  dst->partyEnemy1 = partyEnemy;
  dst->trID = trID;
}

//=============================================================================================
/**
 * トリプル 通信対戦
 *
 * @param   dst
 * @param   gameData
 * @param   netHandle
 * @param   commMode
 */
//=============================================================================================
void BTL_SETUP_Triple_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode )
{
  setup_common( dst, gameData, BTL_LANDFORM_ROOM, BTL_WEATHER_NONE );

  dst->engine = BTL_ENGINE_ALONE;
  dst->competitor = BTL_COMPETITOR_COMM;
  dst->rule = BTL_RULE_TRIPLE;

  dst->netHandle = netHandle;
  dst->commMode = commMode;
  dst->netID = GFL_NET_GetNetID( netHandle );
  dst->multiMode = 0;

  dst->trID = TRID_NULL;
}
