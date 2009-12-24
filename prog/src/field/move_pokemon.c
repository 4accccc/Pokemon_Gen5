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

typedef struct MP_LOC_DATA_tag
{
	u16 BranchNum;
	u16 ZoneIdx[BRANCH_MAX];
}MP_LOC_DATA;

typedef struct MP_TIME_LOC_DATA
{
  u16 num;
  u16 zone[4];
}MP_TIME_LOC_DATA;

enum{
	ZONE_IDX_R01,		//0
	ZONE_IDX_R02,		//1
	ZONE_IDX_R03,		//2
	ZONE_IDX_R04,		//3
	ZONE_IDX_R05,		//4
	ZONE_IDX_R06,		//5
	ZONE_IDX_R07,		//6
	ZONE_IDX_R08,		//7
	ZONE_IDX_R09,		//8
	ZONE_IDX_R10,		//9
	ZONE_IDX_R11,		//10
	ZONE_IDX_R12,		//11
	ZONE_IDX_R13,		//12
	ZONE_IDX_R14,		//13
	ZONE_IDX_R15,		//14
	ZONE_IDX_R16,		//15
	ZONE_IDX_R18,		//16
  ZONE_IDX_MAX,
  ZONE_IDX_HIDE = ZONE_IDX_MAX,
};

static const u16 DATA_ZoneTbl[LOCATION_MAX+1] = {
	//
	ZONE_ID_R01,		//0
	ZONE_ID_R02,		//1
	ZONE_ID_R03,		//2
	ZONE_ID_R04,		//3
	ZONE_ID_R05,		//4
	ZONE_ID_R06,		//5
	ZONE_ID_R07,		//6
	ZONE_ID_R08,		//7
	ZONE_ID_R09,		//8
	ZONE_ID_R10,		//9
	ZONE_ID_R11,		//10
	ZONE_ID_R12,		//11
	ZONE_ID_R13,		//12
	ZONE_ID_R14,		//13
	ZONE_ID_R15,		//14
	ZONE_ID_R16,		//15
	ZONE_ID_R18,		//16
  MVPOKE_ZONE_NULL, //17
};

static const MP_LOC_DATA MovePokeLocationTbl[LOCATION_MAX] =
{
	{2,{1,15,0xffff,0xffff,0xffff,0xffff}},		//0
	{2,{0,2,0xffff,0xffff,0xffff,0xffff}},		//1
	{3,{1,3,7,0xffff,0xffff,0xffff}},			//2
	{3,{2,4,7,0xffff,0xffff,0xffff}},			//3
	{2,{3,5,0xffff,0xffff,0xffff,0xffff}},		//4
	{2,{4,6,0xffff,0xffff,0xffff,0xffff}},		//5
	{2,{5,7,0xffff,0xffff,0xffff,0xffff}},		//6
	{3,{2,3,8,0xffff,0xffff,0xffff}},			//7
	{3,{7,9,11,0xffff,0xffff,0xffff}},			//8
	{3,{8,10,11,0xffff,0xffff,0xffff}},			//9
	{1,{9,0xffff,0xffff,0xffff,0xffff,0xffff}},	//10
	{4,{8,9,12,13,0xffff,0xffff}},				//11
	{2,{11,13,0xffff,0xffff,0xffff,0xffff}},	//12
	{3,{11,12,15,0xffff,0xffff,0xffff}},		//13
	{2,{13,15,0xffff,0xffff,0xffff,0xffff}},	//14
	{2,{0,14,0xffff,0xffff,0xffff,0xffff}},		//15
	{2,{0,14,0xffff,0xffff,0xffff,0xffff}},		//16
};

static const MP_TIME_LOC_DATA DATA_TimeLocation[] = {
 { 4, { ZONE_IDX_R01, ZONE_IDX_R02, ZONE_IDX_R03, ZONE_IDX_R18 }},
 { 3, { ZONE_IDX_R04, ZONE_IDX_R05, ZONE_IDX_R16, 0 }},
 { 3, { ZONE_IDX_R06, ZONE_IDX_R07, ZONE_IDX_R08, 0 }},
 { 3, { ZONE_IDX_R09, ZONE_IDX_R10, ZONE_IDX_R11, 0 }},
 { 4, { ZONE_IDX_R12, ZONE_IDX_R13, ZONE_IDX_R14, ZONE_IDX_R15 }},
};

////////////////////////////////////////////////////////////////
//プロトタイプ
static inline u16 GetZoneID( u8 zone_idx );
static u8 MonsNoToMovePokeID(u16 monsno);

static void JumpMovePokeLocation(	ENC_SV_PTR data, const u8 inTargetPoke, const int inPlayerOldZone);
static void JumpMovePokeTimeLocation( ENC_SV_PTR data, const u8 inTargetPoke );

static void MovePokeLocation(ENC_SV_PTR data, const u8 inTargetPoke, const int inPlayerOldZone);
static void UpdateData(	ENC_SV_PTR data,
						const u8 inTargetPoke,
						const u8  inZoneIdx, const int inZone);
static MPD_PTR GetMovePokeDataByMonsNo(ENC_SV_PTR inEncData, const int inMonsNo);

//--------------------------------------------------------------------------------------------
/**
 * 移動ポケモンジャンプ
 *
 * @param	inEncData		エンカウント関連セーブデータポインタ
 * @param	inTarget		移動対象
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
void MP_JumpMovePokemon(ENC_SV_PTR inEncData, const u8 inTarget)
{
  MPD_PTR	mpd = EncDataSave_GetMovePokeDataPtr( inEncData, inTarget );

  if(EncDataSave_GetMovePokeDataParam( mpd, MP_PARAM_MV_TYPE ) == MVPOKE_TYPE_NORMAL)
  {
	  int player_old_zone;
	  player_old_zone = EncDataSave_GetPlayerOldZone(inEncData);
	  JumpMovePokeLocation(inEncData, inTarget, player_old_zone);
  }else{
    JumpMovePokeTimeLocation( inEncData, inTarget );
  }
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
void MP_JumpMovePokemonAll(ENC_SV_PTR inEncData)
{
	u8 i;
	for(i=0;i<MOVE_POKE_MAX;i++){
		//移動中か？
		if ( EncDataSave_IsMovePokeValid(inEncData, i) ){
			//ジャンプ
			MP_JumpMovePokemon(inEncData, i);
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
void MP_MovePokemonNeighboring(ENC_SV_PTR inEncData)
{
	u8 i;
	
  for(i=0;i<MOVE_POKE_MAX;i++){
    MPD_PTR	mpd = EncDataSave_GetMovePokeDataPtr( inEncData, i );
		//移動中か？
		if ( EncDataSave_IsMovePokeValid(inEncData, i) == FALSE ){
      continue;
    }

		//タイム移動か16分の1の確率でジャンプ
    if( EncDataSave_GetMovePokeDataParam( mpd, MP_PARAM_MV_TYPE ) == MVPOKE_TYPE_TIME || 
		    GFUser_GetPublicRand0(16) == 0){
			IWASAWA_Printf("%d:ジャンプします\n",i);
			//ジャンプ
			MP_JumpMovePokemon(inEncData, i);
		}else{
			IWASAWA_Printf("%d:隣接移動します\n",i);
			//隣接移動
			MovePokeLocation(inEncData, i, EncDataSave_GetPlayerOldZone(inEncData) );
		}
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

	int monsno;
	u8 lv,move_type;
	
	enc = EncDataSave_GetSaveDataPtr( GAMEDATA_GetSaveControlWork( gdata ) );
	mpd = EncDataSave_GetMovePokeDataPtr(enc, inTargetPoke);
	
	switch(inTargetPoke){
	case MOVE_POKE_RAIKAMI:
		monsno = MONSNO_RAIKAMI;
		lv = MOVE_POKE_RAIKAMI_LV;
    move_type = MVPOKE_TYPE_TIME;
		break;
	case MOVE_POKE_KAZAKAMI:
		monsno = MONSNO_KAZAKAMI;
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
	//ＨＰ
	EncDataSave_SetMovePokeDataParam(mpd, MP_PARAM_HP,
											PP_Get( poke_param, ID_PARA_hpmax, NULL ));

	GFL_HEAP_FreeMemory(poke_param);

	//初回移動ポケモン出現場所決定
  MP_JumpMovePokemon( enc, inTargetPoke);
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
      EncDataSave_SetMovePokeState( enc_sv, i, MVPOKE_STATE_NONE );
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
      MP_JumpMovePokemonAll( enc_sv );
		}
    return;
  }
  //移動ポケモンとのエンカウント

	move_poke_id = MonsNoToMovePokeID(monsno);

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
    MP_JumpMovePokemonAll( enc_sv );
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
      return WEATHER_NO_SPARK;
    }
	}
  if( EncDataSave_IsMovePokeValid( enc, MOVE_POKE_KAZAKAMI) ){
    if( zone_id == MP_GetMovePokeZoneID(enc,MOVE_POKE_KAZAKAMI)){
      return WEATHER_NO_SNOWSTORM;
    }
  }
  return WEATHER_NO_NONE; //変更なし 
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
	case MONSNO_RAIKAMI:
		return MOVE_POKE_RAIKAMI;
	case MONSNO_KAZAKAMI:
		return MOVE_POKE_KAZAKAMI;
	}
	GF_ASSERT(0);
	return MOVE_POKE_MAX;
}

//--------------------------------------------------------------------------------------------
/**
 * 移動ポケモン出現場所抽選(ノーマル)
 *
 * @param	data			エンカウント関連セーブデータ
 * @param	inTargetPoke	対象移動ポケモンインデックス
 * @param	inPlayerZone	旧ゾーン
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void JumpMovePokeLocation(ENC_SV_PTR data, const u8 inTargetPoke, const int inPlayerOldZone)
{
	u8 zone_idx;
	int move_poke_now_zone;
	int zone;
	
  //主人公が前々回いたゾーンと
	//今自分がいる場所は対象外とする
	move_poke_now_zone = GetZoneID( EncDataSave_GetMovePokeZoneIdx(data,inTargetPoke));
	
  while(1){
    zone_idx =  GFUser_GetPublicRand0( LOCATION_MAX);
		zone = GetZoneID( zone_idx );
		if ((zone != inPlayerOldZone)&&(zone != move_poke_now_zone)){
			UpdateData(	data, inTargetPoke, zone_idx, zone);
			IWASAWA_Printf("%dへジャンプ player_old_zone:%d\n",zone,inPlayerOldZone);
			break;
		}
	}	
}

//--------------------------------------------------------------------------------------------
/**
 * 移動ポケモン出現場所抽選(カミシリーズ)
 *
 * @param	data			エンカウント関連セーブデータ
 * @param	inTargetPoke	対象移動ポケモンインデックス
 * @param	inPlayerZone	旧ゾーン
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void JumpMovePokeTimeLocation( ENC_SV_PTR data, const u8 inTargetPoke )
{
	u16 move_poke_now_zone;
  const MP_TIME_LOC_DATA* loc;

	//今自分がいる場所は対象外とする
	move_poke_now_zone = GetZoneID( EncDataSave_GetMovePokeZoneIdx(data,inTargetPoke) );
  //タイムローケーション取得
  loc = &DATA_TimeLocation[GFL_RTC_GetTimeZone()];

  if( move_poke_now_zone != MVPOKE_ZONE_NULL &&
      GFUser_GetPublicRand0(100) < 20){
    //隠れる
		UpdateData(	data, inTargetPoke, ZONE_IDX_HIDE, GetZoneID(ZONE_IDX_HIDE));
		IWASAWA_Printf("隠れた %d TimeZone:%d\n", MVPOKE_ZONE_NULL,GFL_RTC_GetTimeZone());
		return;
  }

  while(1){
	  u8 zone_idx = loc->zone[GFUser_GetPublicRand0(loc->num)];
    u16 zone = GetZoneID( zone_idx );

		if( zone != move_poke_now_zone ){
			UpdateData(	data, inTargetPoke, zone_idx, zone);
			IWASAWA_Printf("%dへジャンプ TimeZone:%d\n",zone,GFL_RTC_GetTimeZone());
			break;
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 移動ポケモン隣接移動
 *
 * @param	data			エンカウント関連セーブデータ
 * @param	inTargetPoke	対象移動ポケモンインデックス
 * @param	inPlayerZone	旧ゾーン	
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void MovePokeLocation(ENC_SV_PTR data, const u8 inTargetPoke, const int inPlayerOldZone)
{
	u8 zone_idx;
	int zone;
	const MP_LOC_DATA *loc_data;
	loc_data = &(MovePokeLocationTbl[EncDataSave_GetMovePokeZoneIdx(data,inTargetPoke)]);
	
	if (loc_data->BranchNum == 1){	//変移先が1つしかない場合
		zone_idx = loc_data->ZoneIdx[0];
		zone = GetZoneID( zone_idx );
		if (zone == inPlayerOldZone){	//主人公の前々回のゾーンだったら、ジャンプ
			IWASAWA_Printf("主人公の旧座標だったのでジャンプに変更\n");
			JumpMovePokeLocation(data, inTargetPoke, inPlayerOldZone);
		}else{
			IWASAWA_Printf("%d:%dに隣接移動\n",inTargetPoke,zone);
			UpdateData(	data, inTargetPoke, zone_idx, zone);
		}
	}else{							//変移先が複数ある場合
		u8 idx;
		while(1){
			//隣接場所をランダムで決定
			idx = GFUser_GetPublicRand0(loc_data->BranchNum);
			zone_idx = loc_data->ZoneIdx[idx];
			zone = GetZoneID( zone_idx );
			//主人公の前々回いたゾーン以外を選出して移動
			if (zone != inPlayerOldZone){
				IWASAWA_Printf("%d:%dに隣接移動\n",inTargetPoke,zone);
				UpdateData(	data, inTargetPoke, zone_idx, zone);
				break;
			}
		}
	}
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
