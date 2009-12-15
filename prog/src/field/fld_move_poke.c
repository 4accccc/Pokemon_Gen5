//============================================================================================
/**
 * @file	fld_move_poke.c
 * @brief	フィールド移動ポケモン関連処理
 * @author	Nozomu Saito
 * @date	06.05.13
 */
//============================================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/rtc_tool.h"

#include "arc/fieldmap/zone_id.h"
#include "savedata/encount_sv.h"
#include "savedata/mystatus.h"

#include "battle/battle.h"
#include "gamesystem/btl_setup.h"
#include "poke_tool/poketype.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "move_pokemon_def.h"
#include "move_pokemon.h"
#include "fld_move_poke.h"

static void MovePokeLocation(ENC_SV_PTR inEncData);
static void JumpMovePokeAffterBattle(ENC_SV_PTR inEncData, const int inZoneID);
static MPD_PTR GetMovePokeDataByMonsNo(ENC_SV_PTR inEncData, const int inMonsNo);

//-----------------------------------------------------------------------------
/**
 * 戦闘後の処理
 *
 * @param	gdata			フィールドシステムポインタ
 * @param	bsp	バトルパラムポインタ
 *
 * @return	none
 */
//-----------------------------------------------------------------------------
void FLD_MP_SetAfterBattle(GAMEDATA * gdata, BATTLE_SETUP_PARAM *bsp)
{
	u16 hp;
	u8 cond;
  u8 move_poke_id;
	u16 monsno;
  u16 zone_id;
	
	ENC_SV_PTR enc_sv;
	MPD_PTR mpd;

	//移動ポケモンと戦闘したかをチェック
  if( bsp->rule != BTL_RULE_SINGLE || bsp->competitor != BTL_COMPETITOR_WILD ){
    return;
  }

  zone_id = PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork( gdata ));
  {
	  POKEMON_PARAM *poke_param;

	  poke_param = PokeParty_GetMemberPointer( BATTLE_PARAM_GetPokePartyPointer( bsp, BTL_CLIENT_ENEMY1 ), 0);

	  monsno = PP_Get(poke_param, ID_PARA_monsno, NULL);
	  hp = (u16)(PP_Get(poke_param,ID_PARA_hp, NULL));
	  cond = (u8)(PP_Get(poke_param,ID_PARA_condition, NULL));
  }

  enc_sv = EncDataSave_GetSaveDataPtr( GAMEDATA_GetSaveControlWork( gdata ) );
	mpd = GetMovePokeDataByMonsNo( enc_sv, monsno);
	
	if(mpd == NULL){			//野生ポケモンとエンカウント
		//30％の確率で、3匹の移動ポケモンのジャンプ
		if ( GFUser_GetPublicRand0( 100 ) < 30 ){
			JumpMovePokeAffterBattle( enc_sv, zone_id );
		}
    return;
  }
  //移動ポケモンとのエンカウント

	move_poke_id = MP_MonsNoToMovePokeID(monsno);

	//勝ってＨＰが0
	if( (( bsp->result == BTL_RESULT_WIN )||(bsp->result == BTL_RESULT_DRAW)) && (hp == 0) ) {

		//エンカウントした移動ポケモンデータをクリア
		EncDataSave_ClearMovePokeData(&mpd);

		//倒したセット
		EncDataSave_SetMovePokeState( enc_sv, move_poke_id, MVPOKE_STATE_DOWN );

	//捕獲した
	}else if( bsp->result == BTL_RESULT_CAPTURE ) {

		//エンカウントした移動ポケモンデータをクリア
		EncDataSave_ClearMovePokeData(&mpd);

		//捕獲したセット
		EncDataSave_SetMovePokeState( enc_sv, move_poke_id, MVPOKE_STATE_GET );
	}else{
  	//パラメータの反映
	  EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_HP, hp);
	  EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_COND, cond);

    //移動ポケモンのジャンプ
		JumpMovePokeAffterBattle( enc_sv, zone_id);
  }
}

//-----------------------------------------------------------------------------
/**
 * 戦闘後の移動ポケモンジャンプ
 *
 * @param	inEncData			エンカウントデータポインタ
 * @param	inZoneID			ゾーンID
 *
 * @return	none
 */
//-----------------------------------------------------------------------------
static void JumpMovePokeAffterBattle(ENC_SV_PTR inEncData, const int inZoneID)
{
	int zone;
	u8 i;
	for(i=0;i<MOVE_POKE_MAX;i++){
		//移動中で、且つ、主人公と同じゾーンにいるか？
		if ( EncDataSave_IsMovePokeValid(inEncData, i) ){
			zone = MP_GetMovePokeZone(EncDataSave_GetMovePokeZoneIdx(inEncData, i));
			if (inZoneID == zone){
				//ジャンプ
				MP_JumpMovePokemon(inEncData, i);
			}
		}
	}
}

//-----------------------------------------------------------------------------
/**
 * モンスターナンバーから移動ポケモンデータを取得
 *
 * @param	inEncData	エンカウントデータポインタ
 * @param	inMonsNo	モンスターナンバー
 *
 * @return	MPD_PTR		移動ポケモンデータポインタ ない場合はNULLを返す
 *
*/ 
//-----------------------------------------------------------------------------
static MPD_PTR GetMovePokeDataByMonsNo(ENC_SV_PTR inEncData, const int inMonsNo)
{
	u8 i;
	MPD_PTR mpd;
	for(i=0;i<MOVE_POKE_MAX;i++){
		if ( EncDataSave_IsMovePokeValid(inEncData, i) ){
			mpd = EncDataSave_GetMovePokeDataPtr(inEncData, i);
			if (inMonsNo == EncDataSave_GetMovePokeDataParam(mpd, MP_PARAM_MONSNO)){
				return mpd;
			}
		}
	}
	return NULL;
}

