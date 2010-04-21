//=============================================================================
/**
 * @file	  dwc_rapsound.c
 * @bfief	  VCHATの為の音量管理
 * @author	k.ohno
 * @date	  10/04/21
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"

#include "net/dwc_rap.h"
#include "dwc_rapfriend.h"
#include "savedata/wifilist.h"
#include "sound/pm_sndsys.h"



//------------------------------------------------------------------
/**
 * @brief   WIFIVCT用ラップＢＧＭボリューム変更  VCT時は変更を受け付けない
 */ 
//------------------------------------------------------------------
void GFL_NET_DWC_PMSND_ChangeBGMVolume( u16 trackBit, int volume )
{
  if(GFL_NET_IsWifiConnect()){  //WIFI接続時
    if(GFL_NET_DWC_IsVChat()){
      return;
    }
  }
  PMSND_ChangeBGMVolume( trackBit, volume );
}

