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



//FLAG_HIDEITEM_AREA_START参照のため
#include "../../../resource/fldmapdata/flagwork/flag_define.h"

//ID_HIDE_ITEM_OFFSET参照のため
#include "../../../resource/fldmapdata/script/scrid_offset/scr_offset_id.h"

#include "arc/fieldmap/zone_id.h"

//============================================================================================
//	隠しアイテムデータ
//============================================================================================

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
/**
 * @brief 隠しアイテム復活処理
 * @param ev
 */
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


//--------------------------------------------------------------
/**
 * @brief           隠しアイテムテーブルを得る
 * @param[out]      テーブルの要素数
 * @return          テーブルの先頭アドレス
 */
//--------------------------------------------------------------
const void* HIDEITEM_GetTable( u16* num )
{
  *num = NELEMS(hide_item_data);
  return hide_item_data;
}





