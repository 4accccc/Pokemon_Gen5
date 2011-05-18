/*
 *  @file   plist_call_setup.c(常駐)
 *  @brief  ポケモンリスト呼び出しパラメータセットコモン
 *  @author Miyuki Iwasawa
 *  @date   11.05.18
 *
 *  ポケモンリストの呼び出しパラメータセット構造体
 *  PLIST_DATAをGAMEDATAを使って初期化する
 *
 *
 *  GAMEDATAを使って初期化可能な最低限のものしかこの関数ではセットしないので、
 *  他に必要なパラメータは各呼び出しルーチン側で独自にセットすること
 *  (必要なもの全てが初期化されるわけではないので注意)
 */

#include <gflib.h>
#include "buflen.h"
#include "system/main.h"
#include "system/gfl_use.h"

#include "field/field_skill.h"
#include "poke_tool/pokeparty.h"
#include "savedata/config.h"
#include "savedata/myitem_savedata.h"
#include "savedata/mail.h"
#include "savedata/mail_util.h"
#include "savedata/regulation.h"
#include "savedata/randommap_save.h"
#include "waza_tool/wazano_def.h"

#include "app/pokelist.h"

//--------------------------------------------------------------
/*
 *  @blief  PLIST_DATA構造体のコモン初期化
 *
 *  この関数での初期化後、足りないパラメータは自前で初期化すること
 *
 *  @param  plist   初期化する構造体へのポインタ
 *  @param  gdata   GAMEDATAポインタ
 *  @param  mode    呼び出しモード PL_MODE_TYPE型
 *  @param  party   パーティポインタ
 */
//--------------------------------------------------------------
void PLIST_DATA_Init( PLIST_DATA* plist, GAMEDATA* gdata, PL_MODE_TYPE mode, POKEPARTY* party )
{
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );

  MI_CpuClear32(plist,sizeof(PLIST_DATA));
  plist->pp        = party;
  plist->mode      = mode;

  plist->myitem    = GAMEDATA_GetMyItem( gdata );
  plist->mailblock = GAMEDATA_GetMailBlock( gdata );
  plist->cfg       = SaveData_GetConfig( save );
  plist->rnd_map   = RANDOMMAP_SAVE_GetRandomMapSave( save );
  plist->zkn       = GAMEDATA_GetZukanSave( gdata );   // はっきんだまギラティナ用
  plist->season    = GAMEDATA_GetSeasonID( gdata ); 

  plist->type      = PL_TYPE_SINGLE;
  plist->ret_mode  = PL_RET_NORMAL;
}

//--------------------------------------------------------------
/*
 *  @blief  PLIST_DATA構造体のコモン初期化 メモリアロケート版
 *  この関数での初期化後、足りないパラメータは自前で初期化すること
 *
 *  @param  gdata   GAMEDATAポインタ
 *  @param  mode    呼び出しモード PL_MODE_TYPE型
 *  @param  party   パーティポインタ
 *  @param  heapID  PLIST_DATAワークを確保するヒープ
 */
//--------------------------------------------------------------
PLIST_DATA* PLIST_DATA_AllocInit( GAMEDATA* gdata, PL_MODE_TYPE mode, POKEPARTY* party, HEAPID heapID )
{
  PLIST_DATA* plist = GFL_HEAP_AllocMemory( heapID, sizeof(PLIST_DATA));

  PLIST_DATA_Init( plist, gdata, mode, party );

  return plist;
}

