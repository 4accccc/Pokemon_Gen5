/*
 *  @file   beacon_status.c
 *  @brief  ゲーム中保持するビーコンログ管理
 *  @author Miyuki Iwasawa
 *  @date   10.01.21
 */

#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/beacon_status.h"

struct _TAG_BEACON_STATUS{
  GAMEBEACON_INFO_TBL view_log; 
};

/*
 *  @brief  ビーコンステータスワーク生成
 */
BEACON_STATUS* BEACON_STATUS_Create( HEAPID heapID )
{
  BEACON_STATUS* wk;

  wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BEACON_STATUS));

  return wk;
}

/*
 *  @brief  ビーコンステータスワーク破棄
 */
void BEACON_STATUS_Delete( BEACON_STATUS* wk )
{
  MI_CpuClear8( wk, sizeof(BEACON_STATUS));
  GFL_HEAP_FreeMemory( wk );
}

/*
 *  @brief  InfoTblを取得
 */
GAMEBEACON_INFO_TBL* BEACON_STATUS_GetInfoTbl( BEACON_STATUS* wk )
{
  return &wk->view_log;
}

