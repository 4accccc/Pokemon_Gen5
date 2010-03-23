//=========================================================================
/*
 *  @file   encount_data.c
 *  @brief  エンカウントデータアーカイブアクセス
 *  @author Miyuki Iwasawa
 *  @date   10.03.23
 */
//=========================================================================

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "system/main.h"

#include "fieldmap/zone_id.h"
#include "field/zonedata.h"
#include "encount_data.h"

//------------------------------------------------------------------
/*
 *  @brief  ゾーンエンカウントデータロード
 *
 *  @param  data    エンカウントデータをロードするワークアドレス
 *  @param  handel  エンカウントデータアーカイブハンドル
 *  @param  zone_id ゾーンID
 *  @param  season_id 季節ID
 *  @retval true  エンカウントデータ取得に成功
 *  @retval false 指定ゾーンにはエンカウントデータがない
 */
//------------------------------------------------------------------
BOOL ENCOUNT_DATA_Load( ENCOUNT_DATA *data, ARCHANDLE* handle, u16 zone_id, u8 season_id )
{
  u32 size;
  u16 id,season;
  MI_CpuClear8( data,sizeof(ENCOUNT_DATA));

  id = ZONEDATA_GetEncountDataID(zone_id);
  if(id == ENC_DATA_INVALID_ID){
    return FALSE;
  }
  //データの個数を取得
  size = GFL_ARC_GetDataSizeByHandle( handle, id ) / sizeof(ENCOUNT_DATA);
  if(size != 1 && size != 4){
    GF_ASSERT(0);
    return FALSE;
  }
  //季節を取得
  if(size == 1){
    season = 0;
  }else{
    season = season_id;
  }
  //ロード
  GFL_ARC_LoadDataOfsByHandle( handle, id,
      season*sizeof(ENCOUNT_DATA), sizeof(ENCOUNT_DATA), data );
  data->enable_f = TRUE;

  return TRUE;
}



