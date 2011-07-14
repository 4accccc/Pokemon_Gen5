/*
 *  @file   beacon_status_fld.c
 *  @brief  ゲーム中保持するビーコンログ管理 OVERLAY_BEACON_VIEW上
 *  @author Miyuki Iwasawa
 *  @date   11.07.13
 *
 *  beacon_status.c は常駐なので、すれ違いログ画面上からだけアクセスできれば
 *  よい関数をオーバーレイに追い出している
 */
#include <gflib.h>
#include "system/main.h"

#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/beacon_status.h"

#include "arc/arc_def.h"
#include "msgdata.h"
#include "message.naix"
#include "msg/msg_beacon_status.h"

#include "beacon_status_local.h"
#include "gamesystem/beacon_status_fld.h"

//----------------------------------------------------
/*
 *  @brief  特殊Gパワーストック
 */
//----------------------------------------------------
void BEACON_STATUS_SpGPowerSet( BEACON_STATUS* wk, SP_GPOWER_REQ req_id )
{
  int i;

  //既に予約リストにあるかどうか？
  for(i = 0;i < wk->sp_gpower_req_ct;i++){
    if(wk->sp_gpower_req[i] == req_id){
      return;
    }
  }
  wk->sp_gpower_req[wk->sp_gpower_req_ct++] = req_id;
}

//----------------------------------------------------
/*
 *  @brief  特殊Gパワー　一番古いリクエストを取得
 *
 *  @retval SP_GPOWER_REQ_NULL  リクエスト無し
 *  @retval SP_GPOWER_REQ_XXXX  リクエストID
 */
//----------------------------------------------------
SP_GPOWER_REQ BEACON_STATUS_SpGPowerGet( BEACON_STATUS* wk )
{
  return wk->sp_gpower_req[0];
}

//----------------------------------------------------
/*
 *  @brief  特殊Gパワー　一番古いリクエストを削除
 */
//----------------------------------------------------
SP_GPOWER_REQ BEACON_STATUS_SpGPowerDel( BEACON_STATUS* wk )
{
  if( wk->sp_gpower_req_ct == 0){
    GF_ASSERT(0);
    return;
  }
  //テーブルを詰める
  --wk->sp_gpower_req_ct;

  MI_CpuCopy8( &(wk->sp_gpower_req[1]), &(wk->sp_gpower_req[0]), sizeof(u8)*wk->sp_gpower_req_ct);
  wk->sp_gpower_req[wk->sp_gpower_req_ct] = SP_GPOWER_REQ_NULL;
}

//----------------------------------------------------
/*
 *  @brief  ビーコンGパワー　セット　
 */
//----------------------------------------------------
void BEACON_STATUS_BeaconGPowerSet( BEACON_STATUS* wk, GPOWER_ID g_power, STRBUF* name )
{
  if( wk->beacon_gpower_id != GPOWER_ID_NULL ){
    GF_ASSERT(0);
  }
  wk->beacon_gpower_id = g_power;
  GFL_STR_CopyBuffer( wk->str_beacon_gpower_owner, name );
}

//----------------------------------------------------
/*
 *  @brief  ビーコンGパワー　取得
 */
//----------------------------------------------------
GPOWER_ID BEACON_STATUS_BeaconGPowerGet( BEACON_STATUS* wk, STRBUF* name )
{
  if( wk->beacon_gpower_id != GPOWER_ID_NULL ){
    GFL_STR_CopyBuffer( name, wk->str_beacon_gpower_owner);
    return wk->beacon_gpower_id;
  }
  GFL_STR_ClearBuffer( name );
  return GPOWER_ID_NULL;
}

//----------------------------------------------------
/*
 *  @brief  ビーコンGパワー 削除
 */
//----------------------------------------------------
void BEACON_STATUS_BeaconGPowerDel( BEACON_STATUS* wk )
{
  wk->beacon_gpower_id = GPOWER_ID_NULL;
  GFL_STR_ClearBuffer( wk->str_beacon_gpower_owner );
}


