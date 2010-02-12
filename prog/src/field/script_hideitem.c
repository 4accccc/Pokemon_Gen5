//======================================================================
/**
 * @file	script_hideitem.c
 * @brief	スクリプト制御：隠しアイテム関連
 * @date	05.08.04
 *
 * 01.11.07	Sousuke Tamada
 * 03.04.15	Satoshi Nohara
 * 05.04.25 Hiroyuki Nakamura
 *
 * 09.09.13 tamada  隠しアイテム関連を独立させた
 */
//======================================================================

#include <gflib.h>
#include "script.h"

#include "script_hideitem.h"

#include "eventwork_def.h"


#ifndef SCRIPT_PL_NULL
u8 GetHideItemResponseByScriptId( u16 scr_id );
static BOOL HideItemParamSet( SCRIPT_WORK* sc, u16 scr_id );
void HideItemFlagOneDayClear( FLDCOMMON_WORK* fsys );
#endif


//FLAG_HIDEITEM_AREA_START参照のため
#include "../../../resource/fldmapdata/flagwork/flag_define.h"

//ID_HIDE_ITEM_OFFSET参照のため
#include "../../../resource/fldmapdata/script/scrid_offset/scr_offset_id.h"

//============================================================================================
//	隠しアイテムデータ
//============================================================================================
#ifndef SCRIPT_PL_NULL
typedef struct{
	u16 itemno;									//アイテムナンバー
	u8	num;									//個数
	u8	response;								//反応度
	u16	sp;										//特殊(未使用)
	u16	index;									//フラグインデック
}HIDE_ITEM_DATA;

//#include "../fielddata/script/hide_item.dat"	//隠しアイテムデータ
#endif

typedef struct {
  u16 index;
  u8 world_flag;
  u8 revival_flag;
  u16 zone_id;
  u16 x, z;
}HIDE_ITEM_DATA;

//static const u16 hide_item_data[];
#include "../../../resource/fldmapdata/script/scr_hideitem/hide_item.cdat"

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトIDから隠しアイテムIDを取得
 * @param scr_id  スクリプトID
 * @param 隠しアイテムデータのインデックス
 */
//--------------------------------------------------------------
u16 HIDEITEM_GetIDByScriptID( u16 scr_id )
{
  if ( scr_id < ID_HIDE_ITEM_OFFSET || scr_id > ID_HIDE_ITEM_OFFSET_END )
  {
    GF_ASSERT_MSG( 0, "not hide-item script id(%d)!!\n", scr_id );
    return 0;
  }
  return scr_id - ID_HIDE_ITEM_OFFSET;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトIDから隠しアイテムに対応したフラグIDを取得
 * @param scr_id  スクリプトID
 * @return  u16 イベントフラグNo
 */
//--------------------------------------------------------------
u16 HIDEITEM_GetFlagNoByScriptID( u16 scr_id )
{
  u16 item_id = HIDEITEM_GetIDByScriptID( scr_id );
  const HIDE_ITEM_DATA * data = &hide_item_data[ item_id ];
  return FLAG_HIDEITEM_AREA_START + data->index;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void HIDEITEM_Revival( EVENTWORK * ev )
{
  int i;
  if ( EVENTWORK_PopTimeRequest( ev ) == FALSE ) return;
  for ( i = 0; i < NELEMS(hide_item_data); i++ )
  {
    const HIDE_ITEM_DATA * data = &hide_item_data[i];
    if ( data->revival_flag == 0 ) continue;
    if ( GFUser_GetPublicRand( 100 ) >= 20 ) continue;
    EVENTWORK_ResetEventFlag( ev, FLAG_HIDEITEM_AREA_START + data->index );
  }
}


//============================================================================================
//============================================================================================
#ifndef SCRIPT_PL_NULL
static u16 oneday_hide_item1[][2] = {		//鋼鉄島
	{ ZONE_ID_D24R0103, 52 },
	{ ZONE_ID_D24R0104, 53 },
	{ ZONE_ID_D24R0105, 54 },
	{ ZONE_ID_D24R0105, 55 },
};

#define ONEDAY_HIDE_ITEM1_MAX	( NELEMS(oneday_hide_item1) )

static u16 oneday_hide_item2[] = {			//ソノオの花園
	58,59,219,220,221,222,
};

#define ONEDAY_HIDE_ITEM2_MAX	( NELEMS(oneday_hide_item2) )
#endif

#ifndef SCRIPT_PL_NULL
//--------------------------------------------------------------
/**
 * 0時で隠しフラグが復活する
 *
 *
 * @retval  none
 */
//--------------------------------------------------------------
//extern void HideItemFlagOneDayClear( FLDCOMMON_WORK* fsys );
void HideItemFlagOneDayClear( FLDCOMMON_WORK* fsys )
{
	u8 index;

	//鋼鉄島で2箇所の隠しアイテムが復活することがある
	index = ( gf_rand() % ONEDAY_HIDE_ITEM1_MAX );
	OS_Printf( "0_0 index = %d\n", index );
	if( fsys->location->zone_id != oneday_hide_item1[index][0] ){
		ResetEventFlag( fsys, (FH_FLAG_START + oneday_hide_item1[index][1]) );
	}

	index = ( gf_rand() % ONEDAY_HIDE_ITEM1_MAX );
	OS_Printf( "0_1 index = %d\n", index );
	if( fsys->location->zone_id != oneday_hide_item1[index][0] ){
		ResetEventFlag( fsys, (FH_FLAG_START + oneday_hide_item1[index][1]) );
	}

	//ソノオの花園で2箇所の隠しアイテムが復活することがある
	if( fsys->location->zone_id != ZONE_ID_D13R0101 ){
		index = ( gf_rand() % ONEDAY_HIDE_ITEM2_MAX );
		OS_Printf( "1_0 index = %d\n", index );
		ResetEventFlag( fsys, (FH_FLAG_START + oneday_hide_item2[index]) );

		index = ( gf_rand() % ONEDAY_HIDE_ITEM2_MAX );
		OS_Printf( "1_1 index = %d\n", index );
		ResetEventFlag( fsys, (FH_FLAG_START + oneday_hide_item2[index]) );
	}
	return;
}
#endif

//--------------------------------------------------------------
/**
 * スクリプトIDから、隠しアイテムの反応を取得
 * @param   scr_id		スクリプトID
 * @retval  "反応"
 * 隠しアイテムのスクリプトを見つけたら呼ぶようにする！
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
u8 GetHideItemResponseByScriptId( u16 scr_id )
{
	int i;
	u16	index;
	const HIDE_ITEM_DATA* data;

	data	= &hide_item_data[0];
	index	= HIDEITEM_GetIDByScriptID(scr_id);			//フラグインデックス取得

	//サーチする
	for( i=0; i < HIDE_ITEM_DATA_MAX ;i++ ){
		if( data[i].index == index ){
			break;
		}
	}

	//見つからなかった
	if( i >= HIDE_ITEM_DATA_MAX ){
		GF_ASSERT( (0) && "隠しアイテムデータに該当するデータがありません！" );
		return 0;
	}

	return data[i].response;
}
#endif

//--------------------------------------------------------------
/**
 * 隠しアイテムパラメータをワークにセット
 *
 * @param   sc			SCRIPT型のポインタ
 * @param   scr_id		スクリプトID
 *
 * @retval  "0=セット失敗、1=セット成功"
 *
 * 使用している！
 * SCWK_PARAM0
 * SCWK_PARAM1
 * SCWK_PARAM2
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
static BOOL HideItemParamSet( SCRIPT_WORK* sc, u16 scr_id )
{
	int i;
	u16	index;
	const HIDE_ITEM_DATA* data;
	u16* param0 = getTempWork( sc, SCWK_PARAM0 );
	u16* param1 = getTempWork( sc, SCWK_PARAM1 );
	u16* param2 = getTempWork( sc, SCWK_PARAM2 );

	data	= &hide_item_data[0];
	index	= HIDEITEM_GetIDByScriptID(scr_id);		//フラグインデックス取得

	//サーチする
	for( i=0; i < HIDE_ITEM_DATA_MAX ;i++ ){
		if( data[i].index == index ){
			break;
		}
	}

	//見つからなかった
	if( i >= HIDE_ITEM_DATA_MAX ){
		GF_ASSERT( (0) && "隠しアイテムデータに該当するデータがありません！" );
		return 0;
	}

	*param0 = data[i].itemno;						//アイテムナンバー
	*param1 = data[i].num;							//個数
	*param2 = HIDEITEM_GetFlagNoByScriptID(scr_id);	//フラグナンバー

	return 1;
}
#endif


//============================================================================================
//============================================================================================
#define HIDE_LIST_SX				(7)		//検索範囲
#define HIDE_LIST_SZ				(7)		//検索範囲(未使用)
#define HIDE_LIST_SZ_2				(6)		//検索範囲(未使用)
#define HIDE_LIST_TOP				(7)		//検索範囲(主人公から画面上)
#define HIDE_LIST_BOTTOM			(6)		//検索範囲(主人公から画面下)
#define HIDE_LIST_RESPONSE_NONE		(0xff)	//終了コード

#define DEBUG_HIDE_ITEM_LIST	//デバック有効
//--------------------------------------------------------------
/**
 * 画面内にある隠しアイテムを検索して確保したリストに登録
 *
 * @param   fsys		FLDCOMMON_WORK型のポインタ
 * @param   heapid		ヒープID
 *
 * @retval  "リストのアドレス"
 *
 * 解放処理を忘れずに！
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
HIDE_ITEM_LIST * HideItem_CreateList( FLDCOMMON_WORK * fsys, int heapid )
{
	HIDE_ITEM_LIST* list;
	const BG_TALK_DATA* bg;
	int	hero_gx,hero_gz,i,max,count,tmp;
	int l,r,u,d;

	count = 0;

	//BGデータ数取得
	max = EventData_GetNowBgTalkDataSize( fsys );
	max++;		//終了コードを入れるので+1

	//メモリ確保
	list = sys_AllocMemory( heapid, sizeof(HIDE_ITEM_LIST) * max );

	//BGデータが存在していなかった時
	if( max == 1 ){
		//終了コードセット
		list[0].response= HIDE_LIST_RESPONSE_NONE;
		list[0].gx		= 0xffff;
		list[0].gz		= 0xffff;
		return list;
	}

	//BGデータ取得
	bg	= EventData_GetNowBgTalkData( fsys );
	if( bg == NULL ){
		//終了コードセット
		list[0].response= HIDE_LIST_RESPONSE_NONE;
		list[0].gx		= 0xffff;
		list[0].gz		= 0xffff;
		return list;
	}

	//主人公の位置取得
	hero_gx = Player_NowGPosXGet( fsys->player );
	hero_gz = Player_NowGPosZGet( fsys->player );

	//検索範囲をセット(3Dは意識せずの簡易版)
	l = hero_gx - HIDE_LIST_SX;
	r = hero_gx + HIDE_LIST_SX;
	u = hero_gz - HIDE_LIST_TOP;
	d = hero_gz + HIDE_LIST_BOTTOM;

	//補正
	if( l < 0 ){ l = 0; }
	//if( r < 0 ){ 0 };
	if( u < 0 ){ u = 0; }
	//if( d < 0 ){ 0 };

#ifdef DEBUG_HIDE_ITEM_LIST
	OS_Printf( "\n＜検索範囲＞\n" );
	OS_Printf( "l = %d\n", l );
	OS_Printf( "r = %d\n", r );
	OS_Printf( "u = %d\n", u );
	OS_Printf( "d = %d\n", d );
#endif

	//BGデータ分サーチをかける
	for( i=0; i < max ;i++ ){

		//隠しアイテムタイプで、まだ入手していなかったら
		if( (bg[i].type == BG_TALK_TYPE_HIDE) &&
			(CheckEventFlag(fsys, HIDEITEM_GetFlagNoByScriptID(bg[i].id)) == 0) ){

			//検索範囲内にあるかチェック
			if( (bg[i].gx >= l) &&
				(bg[i].gx <= r) &&
				(bg[i].gz >= u) &&
				(bg[i].gz <= d) ){

				//スクリプトIDから、隠しアイテムの反応を取得
				list[count].response= GetHideItemResponseByScriptId( bg[i].id );

#if 0
				//見つけた座標をそのまま代入
				list[count].gx		= bg[i].gx;
				list[count].gz		= bg[i].gz;

				//ローカル座標
				list[count].gx		= (bg[i].gx % 32);
				list[count].gz		= (bg[i].gz % 32);
#endif	

				//左上を0,0として取得した座標
				//(主人公の位置からの差分を求める)
				tmp = ( hero_gx - bg[i].gx );
				list[count].gx = abs(HIDE_LIST_SX - tmp);
				tmp = ( hero_gz - bg[i].gz );
				list[count].gz = abs(HIDE_LIST_TOP - tmp);

#ifdef DEBUG_HIDE_ITEM_LIST
				OS_Printf( "\n＜主人公の位置＞\n" );
				OS_Printf( "hero_gx = %d\n", hero_gx );
				OS_Printf( "hero_gz = %d\n", hero_gz );

				OS_Printf( "\n＜見つけた隠しアイテムの位置＞\n" );
				OS_Printf( "bg[i].gx = %d\n", bg[i].gx );
				OS_Printf( "bg[i].gz = %d\n", bg[i].gz );

				OS_Printf( "\n＜左上を0,0として取得した座標＞\n" );
				OS_Printf( "list[count].gx = %d\n", list[count].gx );
				OS_Printf( "list[count].gz = %d\n", list[count].gz );
#endif

				count++;
			}
		}
	}

	//終了コードセット
	list[count].response= HIDE_LIST_RESPONSE_NONE;
	list[count].gx		= 0xffff;
	list[count].gz		= 0xffff;

	return list;
}
#endif


