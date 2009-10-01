
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

  dst->musicDefault = SEQ_WB_BA_TEST_250KB;
  dst->musicPinch = SEQ_WB_BA_PINCH_TEST_150KB;

  dst->result = BTL_RESULT_WIN;
}


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

