//============================================================================================
/**
 * @file    natsuki.c
 * @bfief   なつき度計算
 * @author  HisashiSogabe
 * @date    10.02.05
 */
//============================================================================================
#include    <gflib.h>

#include  "poke_tool/natsuki.h"
#include  "item/item.h"
#include  "poke_tool/monsno_def.h"

//============================================================================================
/**
 *	なつき度計算テーブル
 */
//============================================================================================
static	const	s8	CalcNatsukiTable[][3]={
	{  5,  3,  2 },		//レベルアップ
	{  0,  0,  0 },		//アイテム使用（アイテムデータから取得）
	{  3,  2,  1 },		//ボス戦に参加（ジムリーダー、四天王、チャンプ）
	{  1,  1,  1 },		//連れ歩き
	{ -1, -1, -1 },		//瀕死
	{ -5, -5,-10 },		//レベル３０差以上の瀕死
	{  3,  2,  1 },		//ミュージカルに参加
};

static  void	NATSUKI_CalcAct( POKEMON_PARAM *pp, const s8* natsuki_table, ZONEID zoneID, HEAPID heapID );

//============================================================================================
/**
 *	なつき度計算（汎用）
 *
 * @param[in]	pp		        なつき度を計算するポケモン構造体のポインタ
 * @param[in]	calcID	      計算するためのID
 * @param[in]	zoneID	      現在地ID
 * @param[in]	heapID	      ヒープID
 */
//============================================================================================
void  NATSUKI_Calc( POKEMON_PARAM* pp, CALC_NATSUKI calcID, ZONEID zoneID, HEAPID heapID )
{ 
  NATSUKI_CalcAct( pp, CalcNatsukiTable[ calcID ], zoneID, heapID );
}

//============================================================================================
/**
 *	なつき度計算（アイテム使用）
 *
 * @param[in]	pp		        なつき度を計算するポケモン構造体のポインタ
 * @param[in]	item_no	      使用したアイテム
 * @param[in]	zoneID	      現在地ID
 * @param[in]	heapID	      ヒープID
 */
//============================================================================================
void  NATSUKI_CalcUseItem( POKEMON_PARAM* pp, u16 item_no, ZONEID zoneID, HEAPID heapID )
{ 
  int i;
  s8  natsuki_table[ 3 ];

  for( i = 0 ; i < 3 ; i++ )
  { 
    natsuki_table[ i ] = ITEM_GetParam( item_no, ITEM_PRM_FRIEND1 + i, heapID );
  }

  NATSUKI_CalcAct( pp, natsuki_table, zoneID, heapID );
}

//============================================================================================
/**
 *	なつき度計算（連れ歩き）
 *
 * @param[in]	pp		        なつき度を計算するポケモン構造体のポインタ
 * @param[in]	zoneID	      現在地ID
 * @param[in]	heapID	      ヒープID
 */
//============================================================================================
void  NATSUKI_CalcTsurearuki( POKEMON_PARAM* pp, ZONEID zoneID, HEAPID heapID )
{ 
	//連れ歩きのなつき度計算の確率は1/2
	if( GFL_STD_MtRand( 2 ) & 1 )
  {
  	return;
	}

  NATSUKI_CalcAct( pp, CalcNatsukiTable[ CALC_NATSUKI_TSUREARUKI ], zoneID, heapID );
}

//============================================================================================
/**
 *	なつき度計算（ボス戦に参加）
 *
 * @param[in]	ppt		        なつき度を計算するポケパーティ構造体のポインタ
 * @param[in]	zoneID	      現在地ID
 * @param[in]	heapID	      ヒープID
 */
//============================================================================================
void  NATSUKI_CalcBossBattle( POKEPARTY* ppt, ZONEID zoneID, HEAPID heapID )
{ 
  int i;
  POKEMON_PARAM* pp;

  for( i = 0 ; i < PokeParty_GetPokeCount( ppt ) ; i++ )
  { 
    pp = PokeParty_GetMemberPointer( ppt, i );
    NATSUKI_CalcAct( pp, CalcNatsukiTable[ CALC_NATSUKI_BOSS_BATTLE ], zoneID, heapID );
  }
}

//============================================================================================
/**
 *	なつき度計算
 *
 * @param[in]	pp		        なつき度を計算するポケモン構造体のポインタ
 * @param[in]	natsuki_table なつき度計算テーブル
 * @param[in]	zoneID	      現在地ID
 * @param[in]	heapID	      ヒープID
 */
//============================================================================================
static  void	NATSUKI_CalcAct( POKEMON_PARAM *pp, const s8* natsuki_table, ZONEID zoneID, HEAPID heapID )
{
	u16	monsno;
	u16	itemno;
	int	natsuki;
	int	natsukitmp;
	u8	ofs;
	u8	eqp;

	monsno = PP_Get( pp, ID_PARA_monsno_egg , NULL );
	if( ( monsno == 0 ) || ( monsno == MONSNO_TAMAGO ) )
  {
		return;
	}

	itemno  = PP_Get( pp, ID_PARA_item, NULL );
	eqp     = ITEM_GetParam( itemno, ITEM_PRM_EQUIP, heapID );

	ofs = 0;
	natsuki = PP_Get( pp, ID_PARA_friend, NULL );

	if( natsuki >= 100 )
  {
		ofs++;
	}
	if( natsuki >= 200 )
  {
		ofs++;
	}

	natsukitmp = natsuki_table[ ofs ];

	// ゴージャスボールだともう1上がる
	if( ( natsukitmp > 0 ) && ( PP_Get( pp, ID_PARA_get_ball, NULL ) == ITEM_GOOZYASUBOORU ) )
  {
		natsukitmp++;
	}

	// 自分が捕獲された場所だともう１上がる
  if( zoneID )
  { 
	  if( ( natsukitmp > 0 ) && ( PP_Get( pp, ID_PARA_get_place, NULL ) == zoneID ) )
    {
		  natsukitmp++;
	  }
  }

	// なつき度アップの道具を持っていたときは1.5倍アップ
	if( ( natsukitmp > 0 ) && ( eqp == SOUBI_NATUKIDOUP ) )
  {
	  natsukitmp = natsukitmp * 150 / 100;
	}

	// 0以下にも255以上にもしない
	natsuki += natsukitmp;

	if( natsuki < 0 )
  {
		natsuki = 0;
	}
	if( natsuki > 255 )
  {
		natsuki = 255;
	}

	PP_Put( pp, ID_PARA_friend, natsuki );
}

