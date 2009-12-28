//=============================================================================
/**
 * @file	  wih_dwc.c
 * @brief	  ビーコン解析を行い、APの判別を行う
 * @author	k.ohno
 * @date    2009.11.15
 */
//=============================================================================

#include <gflib.h>
#include <nitro/irc.h>
#include "net/network_define.h"
#include "net/wih_dwc.h"
#include "nitroWiFi/ncfg.h"
#include "nitroWiFi/wcm.h"
#include "net/net_whpipe.h"

#define _ALLBEACON_MAX (4)

#define WCM_NETWORK_CAPABILITY_PRIVACY          0x0010

typedef struct{
	WMBssDesc sBssDesc;
  int timer;
} _BEACONCATCH_STR;

struct _WIH_DWC_WORK {
  _BEACONCATCH_STR aBeaconCatch[_ALLBEACON_MAX];
  void* ScanMemory;
  NCFGConfigEx* cfg;
  BOOL bIrc;
  int timer;
  BOOL bStop;
	u16 AllBeaconNum;
  u8 buff[200];  //赤外線受信を行う仮バッファ
  
};


static void _SetScanBeaconData(WMBssDesc* pBss);
//通信の基底にかかわる部分なのでローカルにおき何処からでも参照できる形にしてある
static WIH_DWC_WORK* _localWork=NULL;  
static NCFGConfigEx* _localcfg=NULL;

FS_EXTERN_OVERLAY(dev_irc);


/*---------------------------------------------------------------------------*
  Name:         WHSetAllBeaconFlg
  Description:  ビーコンなら何でも収集して表示するCGEAR用
  Arguments:    bFlg  ONなら収集
 *---------------------------------------------------------------------------*/

WIH_DWC_WORK* WIH_DWC_AllBeaconStart(int num, HEAPID id)
{
  int i;
  WIH_DWC_WORK* pWork;
  
  GF_ASSERT(num <= _ALLBEACON_MAX);
  GF_ASSERT(_localWork==NULL);
  
  pWork = GFL_NET_Align32Alloc(id , sizeof(WIH_DWC_WORK));
  pWork->AllBeaconNum = num;

  pWork->ScanMemory =GFL_NET_Align32Alloc(id ,DWC_GetParseWMBssDescWorkSize());
  WHSetWIHDWCBeaconGetFunc(_SetScanBeaconData);
  _localWork=pWork;

  pWork->cfg = _localcfg;

  GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_irc ) );

  
  IRC_Init(12);  //@todo 12
  
  return(pWork);
}

/*---------------------------------------------------------------------------*
  Name:         WHSetAllBeaconFlg
  Description:  ビーコンなら何でも収集して表示するCGEAR用
  Arguments:    bFlg  ONなら収集
 *---------------------------------------------------------------------------*/

void WIH_DWC_AllBeaconEnd(WIH_DWC_WORK* pWork)
{
  int i;
  GF_ASSERT(_localWork);

  WHSetWIHDWCBeaconGetFunc(NULL);

  IRC_Shutdown();
  GFL_NET_Align32Free(pWork->ScanMemory);
  GFL_NET_Align32Free(pWork);
  _localWork=NULL;
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( dev_irc ) );
}

//------------------------------------------------------------------------------
/**
 * @brief   ビーコン収集を停止する
 */
//------------------------------------------------------------------------------

void WIH_DWC_Stop(void)
{
  if(_localWork){
    _localWork->bStop=TRUE;
    IRC_Init(12);  //@todo 12
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ビーコン収集を再開する
 */
//------------------------------------------------------------------------------

void WIH_DWC_Restart(void)
{
  if(_localWork){
    _localWork->bStop=FALSE;
    IRC_Shutdown();
  }
}


/*---------------------------------------------------------------------------*
  Name:         WHSetAllBeaconFlg
  Description:  ビーコンなら何でも収集して表示するCGEAR用
  Arguments:    bFlg  ONなら収集
 *---------------------------------------------------------------------------*/

static void _SetScanBeaconData(WMBssDesc* pBss)
{
  int i;

  if(_localWork->AllBeaconNum==0){
    return;
  }
  
  for(i=0;i<_localWork->AllBeaconNum;i++){
    _BEACONCATCH_STR* pS = &_localWork->aBeaconCatch[i];
      if(GFL_STD_MemComp(&pS->sBssDesc, pBss, sizeof(WMBssDesc))==0){ //一致
        pS->timer = DEFAULT_TIMEOUT_FRAME;
        return;
      }
  }
  for(i=0;i<_localWork->AllBeaconNum;i++){
    _BEACONCATCH_STR* pS = &_localWork->aBeaconCatch[i];
    if(pS->timer==0){
      GFL_STD_MemCopy(pBss, &pS->sBssDesc, sizeof(WMBssDesc));
      pS->timer = DEFAULT_TIMEOUT_FRAME;
      return;
    }
  }
}

/*---------------------------------------------------------------------------*
  Name:         WH_MainLoopScanBeaconData
  Description:  CGEAR用なんでもビーコン収集MAIN
 *---------------------------------------------------------------------------*/

void WIH_DWC_MainLoopScanBeaconData(void)
{
  int i;

  if(_localWork && _localWork->bStop){
    return;
  }
  
  if(_localWork){
    for(i=0;i<_localWork->AllBeaconNum;i++){
      _BEACONCATCH_STR* pS = &_localWork->aBeaconCatch[i];
      if(pS->timer > 0){
        pS->timer--;
      }
    }
  }
  {
    int size = IRCi_Read(_localWork->buff);
    if(size>0){
      _localWork->bIrc = TRUE;
      _localWork->timer = 60;
    }
  }
  IRC_Move();
}

//------------------------------------------------------------------------------
/**
 * @brief   本体APリストにあるビーコンかどうか
 * @param   検査するビーコン
 * @retval  あったらTRUE
 */
//------------------------------------------------------------------------------

static BOOL _scanAPReserveed( WMBssDesc* pChk )
{
  int i;



  for(i = 0;i < 3;i++){
    if(GFL_STD_MemComp(_localWork->cfg->slot[i].ap.ssid,pChk->ssid,WM_SIZE_SSID)){
      
 //     NET_PRINT("SSIDFIND %s\n",pChk->ssid);
      
      return TRUE;
    }
  }
  return FALSE;
}

/*
　FREESPOTにつきましては、先日ご提供した関数では検知できませんので、以下
の方法での検出をお願い致します。

・FREESPOTについては、SSIDが"FREESPOT"の8文字であることをチェック
・WMBssDescのcapaInfoのbit4が0であること(※)をチェック
*/
//------------------------------------------------------------------------------
/**
 * @brief   FREESPOT(TM)かどうか
 * @param   検査するビーコン
 * @retval  あったらTRUE
 */
//------------------------------------------------------------------------------

static BOOL _scanFreeSpot( WMBssDesc* pChk )
{

  if(pChk->ssidLength==8){
    if(GFL_STD_MemComp("FREESPOT",pChk->ssid,8)){
      if(!(pChk->capaInfo & WCM_NETWORK_CAPABILITY_PRIVACY)){
        return TRUE;
      }
    }
  }
  return FALSE;

}

//------------------------------------------------------------------------------
/**
 * @brief   セキュリティービットがあるかどうか
 * @param   検査するビーコン
 * @retval  あったらTRUE
 */
//------------------------------------------------------------------------------

static BOOL _scanPrivacy( WMBssDesc* pChk )
{

  if(!(pChk->capaInfo & WCM_NETWORK_CAPABILITY_PRIVACY)){
    return TRUE;
  }
  return FALSE;

}


//------------------------------------------------------------------------------
/**
 * @brief   ビーコンのタイプを得る  時間でスキャン優先順位を変える
 * @param   ビーコン数
 * @param   ヒープID
 * @retval  GAME_COMM_STATUS
 */
//------------------------------------------------------------------------------
GAME_COMM_STATUS WIH_DWC_GetAllBeaconType(void)
{
  int i;
  GAME_COMM_STATUS_BIT bit  = WIH_DWC_GetAllBeaconTypeBit();

  //最初に見つかったビットを優先度が高いと判断し、返す
  for( i = 0; i < GAME_COMM_STATUS_MAX; i++ )
  { 
    if( (bit >> i) & 1 )
    { 
      return i;
    }
  }

  return GAME_COMM_STATUS_NULL;
}

//------------------------------------------------------------------------------
/**
 * @brief   ビーコンのタイプを得る  ビットなので、複数の状態を取得可能
 * @retval  GAME_COMM_STATUS_BIT
 */
//------------------------------------------------------------------------------
GAME_COMM_STATUS_BIT WIH_DWC_GetAllBeaconTypeBit(void)
{ 
  int i;
  GAME_COMM_STATUS_BIT retcode = 0;
  const GFLNetInitializeStruct *aNetStruct = GFL_NET_GetNETInitStruct();

  if(_localWork==NULL){
    return retcode;
  }

  if( _localWork->bIrc == TRUE){
    if(_localWork->timer>0){
      _localWork->timer--;
    }
    else{
      _localWork->bIrc = FALSE;
    }
    retcode |= GAME_COMM_STATUS_BIT_IRC;
  }
  
  for( i=0;i < aNetStruct->maxBeaconNum;i++ ){
    if( GFL_NET_GetBeaconData( i ) != NULL ){
      GameServiceID id = GFL_NET_WLGetGameServiceID(i);
      switch(id){
      case WB_NET_UNION:
        retcode |= GAME_COMM_STATUS_BIT_WIRELESS_UN;
        break;
      case WB_NET_FIELDMOVE_SERVICEID:
      case WB_NET_PALACE_SERVICEID:
        retcode |= GAME_COMM_STATUS_BIT_WIRELESS;
        break;
      case WB_NET_NOP_SERVICEID:
        break;
      default:
        retcode |= GAME_COMM_STATUS_BIT_WIRELESS_FU;  //@todo その他は不思議にしてある
      }
    }
  }
  //NAGI_Printf( "ret %d\n", retcode );
  
  for(i=0;i < _localWork->AllBeaconNum;i++){
    _BEACONCATCH_STR* pS = &_localWork->aBeaconCatch[i];
    if(pS->timer == 0){
      continue;
    }
    {
      WMBssDesc* bss = &pS->sBssDesc;
    
      DWCApInfo ap;

      if(_scanAPReserveed(bss)){
        retcode |= GAME_COMM_STATUS_BIT_WIFI;
      }
      else if(_scanFreeSpot(bss)){
        retcode |= GAME_COMM_STATUS_BIT_WIFI_ZONE;
      }
      else if(DWC_ParseWMBssDesc(_localWork->ScanMemory,bss,&ap)){
        OS_TPrintf("DWCApInfo %d\n",ap.aptype);
        switch(ap.aptype){
        case DWC_APINFO_TYPE_USER0:
        case DWC_APINFO_TYPE_USER1:
        case DWC_APINFO_TYPE_USER2:
#ifdef SDK_TWL
        case DWC_APINFO_TYPE_USER3:
        case DWC_APINFO_TYPE_USER4:
        case DWC_APINFO_TYPE_USER5:
#endif //SDK_TWL
        case DWC_APINFO_TYPE_USB://          :  ニンテンドーWi-Fi USBコネクタ
          retcode |= GAME_COMM_STATUS_BIT_WIFI;
          break;
        case DWC_APINFO_TYPE_SHOP://         :  ニンテンドーWi-Fiステーション
        case DWC_APINFO_TYPE_FREESPOT://     :  FREESPOT（フリースポット）
        case DWC_APINFO_TYPE_WAYPORT://      :  Wayport(北米ホットスポット)※現在は使用できません
        case DWC_APINFO_TYPE_NINTENDOZONE:// : Nintendo Zone
          retcode |= GAME_COMM_STATUS_BIT_WIFI_ZONE;
          break;
        }
      }
    }
  }
/*  if(retcode!=GAME_COMM_STATUS_NULL){
    return retcode;
  }
*/
  for(i=0;i < _localWork->AllBeaconNum;i++){
    _BEACONCATCH_STR* pS = &_localWork->aBeaconCatch[i];
    if(pS->timer == 0){
      continue;
    }
    if(!_scanPrivacy(&pS->sBssDesc)){
      retcode |= GAME_COMM_STATUS_BIT_WIFI_FREE;
    }
  }
  if(_localWork->AllBeaconNum>0){
    retcode |= GAME_COMM_STATUS_BIT_WIFI_LOCK;
  }
  ///@todo セキュリティーがかかっているかどうか
  //NAGI_Printf( "ret %d\n", retcode );
  return retcode;
}


const BOOL WIH_DWC_IsEnableBeaconData( const u8 idx )
{
  if( _localWork->aBeaconCatch[idx].timer > 0 )
  {
    return TRUE;
  }
  return FALSE;
}

const WMBssDesc* WIH_DWC_GetBeaconData( const u8 idx )
{
  return &_localWork->aBeaconCatch[idx].sBssDesc;
}


//------------------------------------------------------------------------------
/**
 * @brief   AP情報を得る
 * @retval  GAME_COMM_STATUS
 */
//------------------------------------------------------------------------------
void WIH_DWC_CreateCFG(HEAPID id)
{
  GF_ASSERT(_localcfg==NULL);
  _localcfg = GFL_NET_Align32Alloc(id ,sizeof(NCFGConfigEx));
}




//------------------------------------------------------------------------------
/**
 * @brief   AP情報開放
 * @retval  none
 */
//------------------------------------------------------------------------------
void WIH_DWC_DeleteCFG(void)
{
  GF_ASSERT(_localcfg);
  GFL_NET_Align32Free(_localcfg);
  _localcfg = NULL;
}
//------------------------------------------------------------------------------
/**
 * @brief   AP情報再読み込み
 * @retval  void
 */
//------------------------------------------------------------------------------
void WIH_DWC_ReloadCFG(void)
{
  GF_ASSERT(_localcfg);
  NCFG_ReadConfig(&_localcfg->compat, NULL);
}


