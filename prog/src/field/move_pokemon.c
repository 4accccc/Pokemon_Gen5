//============================================================================================
/**
 * @file	move_pokemon.c
 * @brief	移動ポケモン関連処理
 * @author	Nozomu Saito -> Miyuki Iwasawa
 * @date	06.05.11-09.12.14
 */
//============================================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/rtc_tool.h"

#include "arc/fieldmap/zone_id.h"
#include "savedata/encount_sv.h"
#include "savedata/mystatus.h"
#include "savedata/zukan_savedata.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
#include "battle/battle.h"
#include "gamesystem/btl_setup.h"

#include "field/weather_no.h"
#include "move_pokemon_def.h"
#include "move_pokemon.h"

#ifdef PM_DEBUG
//#include "field_common.h"
#endif

//#include "debug_saito.h"


/////////////////////////////////////////////////////////////////
//リテラル定義
#define	BRANCH_MAX		(6)

#define MOVE_POKE_RAIKAMI_LV	(40)
#define MOVE_POKE_KAZAKAMI_LV		(40)

#define TIME_LOC_ZONE_MAX (4)

typedef struct MP_LOC_DATA_tag
{
	u16 BranchNum;
	u16 ZoneIdx[BRANCH_MAX];
}MP_LOC_DATA;

typedef struct MP_TIME_LOC_DATA
{
  u16 num;
  u8  zone_idx[TIME_LOC_ZONE_MAX];
}MP_TIME_LOC_DATA;

enum{
	ZONE_IDX_R02,		//0
	ZONE_IDX_R03,		//1
	ZONE_IDX_R04,		//2
	ZONE_IDX_R05,		//3
	ZONE_IDX_R06,		//4
	ZONE_IDX_R07,		//5
	ZONE_IDX_R08,		//6
	ZONE_IDX_R09,		//7
	ZONE_IDX_R10,		//8
	ZONE_IDX_R11,		//9
	ZONE_IDX_R12,		//10
	ZONE_IDX_R13,		//11
	ZONE_IDX_R14,		//12
	ZONE_IDX_R15,		//13
	ZONE_IDX_R16,		//14
	ZONE_IDX_R18,		//15
  ZONE_IDX_MAX,
  ZONE_IDX_HIDE = ZONE_IDX_MAX,
  ZONE_IDX_NONE = 0xFF,
};

static const u16 DATA_ZoneTbl[MVPOKE_LOCATION_MAX+1] = {
	//
	ZONE_ID_R02,		//0
	ZONE_ID_R03,		//1
	ZONE_ID_R04,		//2
	ZONE_ID_R05,		//3
	ZONE_ID_R06,		//4
	ZONE_ID_R07,		//5
	ZONE_ID_R08,		//6
	ZONE_ID_R09,		//7
	ZONE_ID_R10,		//8
	ZONE_ID_R11,		//9
	ZONE_ID_R12,		//10
	ZONE_ID_R13,		//11
	ZONE_ID_R14,		//12
	ZONE_ID_R15,		//13
	ZONE_ID_R16,		//14
	ZONE_ID_R18,		//15
  MVPOKE_ZONE_NULL, //16
};

static const MP_TIME_LOC_DATA DATA_TimeLocation[] = {
 { 3, { ZONE_IDX_R02, ZONE_IDX_R03, ZONE_IDX_R18 }},
 { 3, { ZONE_IDX_R04, ZONE_IDX_R05, ZONE_IDX_R16, 0 }},
 { 3, { ZONE_IDX_R06, ZONE_IDX_R07, ZONE_IDX_R08, 0 }},
 { 3, { ZONE_IDX_R09, ZONE_IDX_R10, ZONE_IDX_R11, 0 }},
 { 4, { ZONE_IDX_R12, ZONE_IDX_R13, ZONE_IDX_R14, ZONE_IDX_R15 }},
};

////////////////////////////////////////////////////////////////
//プロトタイプ
static inline u16 GetZoneID( u8 zone_idx );
static u8 MonsNoToMovePokeID(u16 monsno);

static void JumpMovePokeTimeLocation( ENC_SV_PTR data, u8 season, const u8 inTargetPoke, const u8 egnore_zone_idx );

static void UpdateData(	ENC_SV_PTR data,
						const u8 inTargetPoke,
						const u8  inZoneIdx, const int inZone);
static MPD_PTR GetMovePokeDataByMonsNo(ENC_SV_PTR inEncData, const int inMonsNo);

//--------------------------------------------------------------------------------------------
/**
 * 移動ポケモンジャンプ
 *
 * @param	inEncData		エンカウント関連セーブデータポインタ
 * @param season      季節
 * @param	inTarget		移動対象
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
void MP_JumpMovePokemon(ENC_SV_PTR inEncData, u8 season, const u8 inTarget)
{
  JumpMovePokeTimeLocation( inEncData, season, inTarget, ZONE_IDX_NONE );
}

//--------------------------------------------------------------------------------------------
/**
 * 移動ポケモン全部ジャンプ
 *
 * @param	inEncData		エンカウント関連セーブデータポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void MP_JumpMovePokemonAll( GAMEDATA* gdata )
{
	u8 i,season;
  ENC_SV_PTR enc_sv = EncDataSave_GetSaveDataPtr( GAMEDATA_GetSaveControlWork( gdata ) );
  season = GAMEDATA_GetSeasonID( gdata );

  for(i=0;i<MOVE_POKE_MAX;i++){
		//移動中か？
		if ( EncDataSave_IsMovePokeValid(enc_sv, i) ){
			//ジャンプ
			MP_JumpMovePokemon(enc_sv, season, i);
		}
	}
}

//-----------------------------------------------------------------------------
/**
 * 移動ポケモンの隣接移動
 *
 * @param	inEncData		エンカウント関連セーブデータポインタ
 *
 * @return	none
 */
//-----------------------------------------------------------------------------
void MP_MovePokemonNeighboring( GAMEDATA* gdata )
{
	u8 i, season;
  ENC_SV_PTR enc_sv = EncDataSave_GetSaveDataPtr( GAMEDATA_GetSaveControlWork( gdata ) );
  season = GAMEDATA_GetSeasonID( gdata );
	
  for(i=0;i<MOVE_POKE_MAX;i++){
    MPD_PTR	mpd = EncDataSave_GetMovePokeDataPtr( enc_sv, i );
		//移動中か？
		if ( EncDataSave_IsMovePokeValid(enc_sv, i) == FALSE ){
      continue;
    }

		//ジャンプ
		MP_JumpMovePokemon(enc_sv, season, i);
	  IWASAWA_Printf("%d:ジャンプします\n",i);
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 移動ポケモンゾーン取得
 *
 * @param	  inTarget  移動ポケモンID
 *
 * @retval  MVPOKE_ZONE_NULL  隠れているので無効
 * @retval  それ以外　移動ポケモンがいるゾーンID
 */
//--------------------------------------------------------------------------------------------
u16 MP_GetMovePokeZoneID( ENC_SV_PTR inEncData, const u8 inTarget)
{
	return GetZoneID( EncDataSave_GetMovePokeZoneIdx( inEncData, inTarget) );
}

//--------------------------------------------------------------------------------------------
/**
 * 移動ポケモンゾーンIdxをゾーンIDに変換
 *
 * @param	  inZoneIdx  移動ポケモンゾーンindex
 *
 * @retval  MVPOKE_ZONE_NULL  隠れているので無効
 * @retval  それ以外　移動ポケモンがいるゾーンID
 */
//--------------------------------------------------------------------------------------------
u16 MP_MovePokeZoneIdx2ID( const u8 inZoneIdx )
{
	return GetZoneID( inZoneIdx );
}

//--------------------------------------------------------------------------------------------
/**
 * 移動ポケモンがいるかをチェック
 *
 * @param	inEncData		エンカウント関連セーブデータポインタ
 *
 * @return BOOL		TRUE:いる	FALSE:いない
 */
//--------------------------------------------------------------------------------------------
BOOL MP_CheckMovePoke(ENC_SV_PTR inEncData)
{
	u8 i;
	for(i=0;i<MOVE_POKE_MAX;i++){
		if ( EncDataSave_IsMovePokeValid(inEncData, i) ){
			return TRUE;
		}
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 主人公ゾーン履歴の更新
 *
 * @param	inEncData		エンカウント関連セーブデータポインタ
 * @param	inZone			更新ゾーン
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void MP_UpdatePlayerZoneHist(ENC_SV_PTR inEncData, const int inZone)
{
	if (MP_CheckMovePoke(inEncData)){
		//履歴更新
		EncDataSave_UpdatePlayerZoneHist( inEncData, inZone );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 指定移動ポケモンを登録
 *
 * @param	sv					セーブデータ
 * @param	inTargetPoke		対象移動ポケモンインデックス
 * 
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void MP_AddMovePoke( GAMEDATA* gdata, const u8 inTargetPoke)
{
	POKEMON_PARAM *poke_param;
	MPD_PTR	 mpd;
	ENC_SV_PTR enc;
	MYSTATUS * my_st;
  SAVE_CONTROL_WORK* save;

	int monsno;
	u8 lv,move_type;
  
  save = GAMEDATA_GetSaveControlWork( gdata );
	enc = EncDataSave_GetSaveDataPtr( save );
	mpd = EncDataSave_GetMovePokeDataPtr(enc, inTargetPoke);
	
	switch(inTargetPoke){
	case MOVE_POKE_RAIKAMI:
		monsno = MONSNO_649;  //ライカミ MONSNO_RAIKAMI;
		lv = MOVE_POKE_RAIKAMI_LV;
    move_type = MVPOKE_TYPE_TIME;
		break;
	case MOVE_POKE_KAZAKAMI:
		monsno = MONSNO_648;  //カザカミ MONSNO_KAZAKAMI;
		lv = MOVE_POKE_KAZAKAMI_LV;
    move_type = MVPOKE_TYPE_TIME;
		break;
	default:
		GF_ASSERT(0);
		return ;
	}

	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_MONSNO, monsno);
	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_LV, lv);
	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_MV_TYPE, move_type);
	
	my_st =  GAMEDATA_GetMyStatus( gdata );

	poke_param = GFL_HEAP_AllocClearMemoryLo( HEAPID_WORLD, POKETOOL_GetWorkSize() );
	PP_SetupEx(poke_param, monsno, lv, MyStatus_GetID(my_st), PTL_SETUP_POW_AUTO,  PTL_SETUP_RND_AUTO );

	//状態異常
	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_COND, 0);
	//エンカウントフラグ
	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_ENC, 1);
	//パワー乱数
	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_POW_RND,
											PP_Get( poke_param, ID_PARA_power_rnd, NULL ));
	//個性乱数
	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_PER_RND,
											PP_Get( poke_param, ID_PARA_personal_rnd, NULL ));
	//性別
	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_SEIKAKU,
											PP_Get( poke_param, ID_PARA_seikaku, NULL ));
	//性別
	//ＨＰ
	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_HP,
											PP_Get( poke_param, ID_PARA_hpmax, NULL ));

  //見たフラグセット
  ZUKANSAVE_SetPokeSee( ZUKAN_SAVEDATA_GetZukanSave( save ), poke_param );

	GFL_HEAP_FreeMemory(poke_param);

	//初回移動ポケモン出現場所決定(追加イベントが起こるR07は、初回の対象から外す)
  JumpMovePokeTimeLocation( enc, GAMEDATA_GetSeasonID( gdata ), inTargetPoke, ZONE_IDX_R07 );
  
  //ステータス更新
  EncDataSave_SetMovePokeState( enc, inTargetPoke, MVPOKE_STATE_MOVE );
}

//--------------------------------------------------------------------------------------------
/**
 * 殿堂入り時移動ポケモン復活処理
 *
 * @param	sv					セーブデータ
 * @param	inTargetPoke		対象移動ポケモンインデックス
 * 
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void MP_RecoverMovePoke( GAMEDATA* gdata )
{
  int i;
  ENC_SV_PTR enc_sv = EncDataSave_GetSaveDataPtr( GAMEDATA_GetSaveControlWork( gdata ) );

  for( i = 0;i < MOVE_POKE_MAX;i++){
    if( EncDataSave_GetMovePokeState( enc_sv, i) == MVPOKE_STATE_DOWN ){
      MP_AddMovePoke( gdata, i);
    }
  }
}


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
void MP_SetAfterBattle(GAMEDATA * gdata, BATTLE_SETUP_PARAM *bsp)
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
      MP_JumpMovePokemonAll( gdata );
		}
    return;
  }
  //移動ポケモンとのエンカウント

	move_poke_id = MonsNoToMovePokeID(monsno);

	//勝ってＨＰが0
	if( (( bsp->result == BTL_RESULT_WIN )||(bsp->result == BTL_RESULT_DRAW)) && (hp == 0) ) {

		//エンカウントした移動ポケモンデータをクリア
		EncDataSave_ClearMovePokeData(mpd);
    EncDataSave_SetMovePokeZoneIdx( enc_sv, move_poke_id, ZONE_IDX_HIDE );

		//倒したセット
		EncDataSave_SetMovePokeState( enc_sv, move_poke_id, MVPOKE_STATE_DOWN );

	//捕獲した
	}else if( bsp->result == BTL_RESULT_CAPTURE ) {

		//エンカウントした移動ポケモンデータをクリア
		EncDataSave_ClearMovePokeData(mpd);
    EncDataSave_SetMovePokeZoneIdx( enc_sv, move_poke_id, ZONE_IDX_HIDE );

		//捕獲したセット
		EncDataSave_SetMovePokeState( enc_sv, move_poke_id, MVPOKE_STATE_GET );
	}else{
  	//パラメータの反映
	  EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_HP, hp);
	  EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_COND, cond);

    //移動ポケモンのジャンプ
    MP_JumpMovePokemonAll( gdata );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * 移動ポケモンゾーン天候変化チェック
 *
 * @param	  zone_id		現在のゾーンID
 *
 * @retval  MVPOKE_ZONE_NULL  隠れているので無効
 * @retval  それ以外　移動ポケモンがいるゾーンID
 */
//--------------------------------------------------------------------------------------------
u16 MP_CheckMovePokeWeather( GAMEDATA * gdata, u16 zone_id )
{
	ENC_SV_PTR enc;
	enc = EncDataSave_GetSaveDataPtr( GAMEDATA_GetSaveControlWork( gdata ) );
	
  if ( EncDataSave_IsMovePokeValid( enc, MOVE_POKE_RAIKAMI) ){
    if( zone_id == MP_GetMovePokeZoneID(enc,MOVE_POKE_RAIKAMI)){
      return WEATHER_NO_RAIKAMI;
    }
	}
  if( EncDataSave_IsMovePokeValid( enc, MOVE_POKE_KAZAKAMI) ){
    if( zone_id == MP_GetMovePokeZoneID(enc,MOVE_POKE_KAZAKAMI)){
      return WEATHER_NO_KAZAKAMI;
    }
  }
  return WEATHER_NO_NONE; //変更なし 
}

//--------------------------------------------------------------------------------------------
/**
 * いずれかの移動ポケモンが移動中かどうかをBOOLで返す
 *
 * @retval  TRUE ライカミ/カザカミが移動中
 * @retval  FALSE 移動中でない
 */
//--------------------------------------------------------------------------------------------
BOOL MP_CheckMovePokeValid( GAMEDATA* gdata )
{
	ENC_SV_PTR enc;
	enc = EncDataSave_GetSaveDataPtr( GAMEDATA_GetSaveControlWork( gdata ) );
	
  if(  EncDataSave_IsMovePokeValid( enc, MOVE_POKE_RAIKAMI) ||
       EncDataSave_IsMovePokeValid( enc, MOVE_POKE_KAZAKAMI) ){
    return TRUE;
  }
  return FALSE;
}

/////////////////////////////////////////////////////////////////////
//ローカル

//--------------------------------------------------------------------------------------------
/*
 *  @brief  ゾーンindexからゾーンIDを取得
 */
//--------------------------------------------------------------------------------------------
static inline u16 GetZoneID( u8 zone_idx )
{
  if( zone_idx > ZONE_IDX_MAX) {
    GF_ASSERT(0);
    return DATA_ZoneTbl[0];
  }
  return DATA_ZoneTbl[zone_idx];
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンナンバー→移動ポケモン指定IDへの変換
 *
 * @date	2009.06.24
 * @param	monsno	移動ポケモンのポケモンナンバー
 * @return	u8		移動ポケモン指定ID
 */
//--------------------------------------------------------------------------------------------
static u8 MonsNoToMovePokeID(u16 monsno)
{
	switch (monsno) {
  case MONSNO_649:  //ライカミ MONSNO_RAIKAMI:
		return MOVE_POKE_RAIKAMI;
  case MONSNO_648:  //カザカミ MONSNO_KAZAKAMI:
		return MOVE_POKE_KAZAKAMI;
	}
	GF_ASSERT(0);
	return MOVE_POKE_MAX;
}

//--------------------------------------------------------------------------------------------
/**
 * 移動ポケモン出現場所抽選(カミシリーズ)
 *
 * @param	data			エンカウント関連セーブデータ
 * @param	season		季節
 * @param	inTargetPoke	対象移動ポケモンインデックス
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void JumpMovePokeTimeLocation( ENC_SV_PTR data, u8 season, const u8 inTargetPoke, const u8 egnore_zone_idx )
{
	u16 now_zone_idx, timezone;
  const MP_TIME_LOC_DATA* loc;

	//今自分がいる場所は対象外とする
	now_zone_idx = EncDataSave_GetMovePokeZoneIdx(data,inTargetPoke);

  //タイムローケーション取得
  timezone = PM_RTC_GetTimeZone( season );
  loc = &DATA_TimeLocation[timezone];

  //連続して隠れることはない
  if( now_zone_idx != ZONE_IDX_HIDE &&
      GFUser_GetPublicRand0(100) < 20){
    //隠れる
		UpdateData(	data, inTargetPoke, ZONE_IDX_HIDE, GetZoneID(ZONE_IDX_HIDE));
		IWASAWA_Printf("隠れた %d TimeZone:%d\n", MVPOKE_ZONE_NULL, timezone);
		return;
  }

  //飛び先候補のゾーンをチェック
  {
    int i,num;
    u8 zone_idx;
    u16 zone;
    u8 zone_tbl[TIME_LOC_ZONE_MAX];

    num = 0;
    for(i = 0;i < loc->num;i++){
	    zone_idx = loc->zone_idx[i];

		  if( zone_idx != now_zone_idx && zone_idx != egnore_zone_idx){
        zone_tbl[num++] = zone_idx;
      }
    }
    zone_idx = zone_tbl[GFUser_GetPublicRand0( num )];
    zone = GetZoneID(zone_idx);
	  UpdateData(	data, inTargetPoke, zone_idx, zone);
	  IWASAWA_Printf("%dへジャンプ TimeZone:%d\n",zone, timezone );
  }
#if 0
  while(1){
	  u8 zone_idx = loc->zone_idx[GFUser_GetPublicRand0(loc->num)];
    u16 zone = GetZoneID( zone_idx );

		if( zone == now_zone_idx ){
      continue;
    }
		UpdateData(	data, inTargetPoke, zone_idx, zone);
		IWASAWA_Printf("%dへジャンプ TimeZone:%d\n",zone, timezone );
		break;
	}
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * データ更新
 *
 * @param	data			エンカウント関連セーブデータ
 * @param	inTargetPoke	対象移動ポケモンインデックス
 * @param	inZoneidx		移動ゾーンインデックス
 * @param	inZone			ゾーンＩＤ	
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void UpdateData(	ENC_SV_PTR data,
						const u8 inTargetPoke,
						const u8  inZoneIdx, const int inZone)
{
	MPD_PTR mpd;
	mpd = EncDataSave_GetMovePokeDataPtr(data, inTargetPoke);
	//更新
	EncDataSave_SetMovePokeZoneIdx(data,inTargetPoke,inZoneIdx);
	//移動ポケモンデータも書き換える
	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_ZONE_ID, inZone);
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

#ifdef PM_DEBUG
//--------------------------------------------------------------------------------------------
/**
 * データ更新
 *
 * @param	
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
void MP_DebugUpdateData(	ENC_SV_PTR data,
						const u8 inTargetPoke,
						const u8  inZoneIdx)
{
	MPD_PTR mpd;
	mpd = EncDataSave_GetMovePokeDataPtr(data, inTargetPoke);
	//更新
	EncDataSave_SetMovePokeZoneIdx(data,inTargetPoke,inZoneIdx);
	//移動ポケモンデータも書き換える
	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_ZONE_ID, GetZoneID( inZoneIdx ));
}
#endif
