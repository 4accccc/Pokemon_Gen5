//==============================================================================
/**
 * @file    net_old.c
 * @brief   旧通信システムのロード、アンロード管理
 * @author  matsuda
 * @date    2009.06.04(木)
 */
//==============================================================================
#include <gflib.h>
#include "net_old/comm_system.h"
#include "net_old/net_old.h"


//==============================================================================
//  グローバル変数
//==============================================================================
BOOL NetOldLoad = FALSE;


//==============================================================================
//  オーバーレイID
//==============================================================================
FS_EXTERN_OVERLAY(netold);
FS_EXTERN_OVERLAY(dev_wifilib);



//==================================================================
/**
 * 旧通信システムのロード
 */
//==================================================================
void NetOld_Load(void)
{
  GF_ASSERT(NetOldLoad == FALSE);
  if(NetOldLoad == FALSE){
    GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_wifilib ) );
    GFL_OVERLAY_Load(FS_OVERLAY_ID(netold));
    NetOldLoad = TRUE;
  }
}

//==================================================================
/**
 * 旧通信システムのアンロード
 */
//==================================================================
void NetOld_Unload(void)
{
  GF_ASSERT(NetOldLoad == TRUE);
  if(NetOldLoad == TRUE){
    GFL_OVERLAY_Unload(FS_OVERLAY_ID(netold));
    GFL_OVERLAY_Unload(FS_OVERLAY_ID(dev_wifilib));
    NetOldLoad = FALSE;
  }
}

//==================================================================
/**
 * 旧通信システムの更新処理
 */
//==================================================================
void NetOld_Update(void)
{
  if(NetOldLoad == FALSE){
    return;
  }
  
  CommUpdateData();
}
