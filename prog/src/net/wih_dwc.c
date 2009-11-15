



/*---------------------------------------------------------------------------*
  Name:         WHSetAllBeaconFlg
  Description:  ビーコンなら何でも収集して表示するCGEAR用
  Arguments:    bFlg  ONなら収集
 *---------------------------------------------------------------------------*/

void WH_AllBeaconStart(int num)
{
  int i;
  GF_ASSERT( _pWmInfo->ScanMemory==NULL);
  GF_ASSERT(num <= _ALLBEACON_MAX);
  _pWmInfo->AllBeaconNum = num;
  for(i=0;i<num;i++){
    _pWmInfo->pBeaconCatch[i] = GFL_NET_Align32Alloc(_pWmInfo->heapID , sizeof(_BEACONCATCH_STR));
  }
  _pWmInfo->ScanMemory =GFL_NET_Align32Alloc(_pWmInfo->heapID ,DWC_GetParseWMBssDescWorkSize());

}

/*---------------------------------------------------------------------------*
  Name:         WHSetAllBeaconFlg
  Description:  ビーコンなら何でも収集して表示するCGEAR用
  Arguments:    bFlg  ONなら収集
 *---------------------------------------------------------------------------*/

void WH_AllBeaconEnd(void)
{
  int i;

  GF_ASSERT( _pWmInfo->ScanMemory);
  for(i=0;i<_pWmInfo->AllBeaconNum;i++){
    GFL_NET_Align32Free(_pWmInfo->pBeaconCatch[i]);
  }
  GFL_NET_Align32Free(_pWmInfo->ScanMemory);
  _pWmInfo->ScanMemory=NULL;
  _pWmInfo->AllBeaconNum = 0;
}

/*---------------------------------------------------------------------------*
  Name:         WHSetAllBeaconFlg
  Description:  ビーコンなら何でも収集して表示するCGEAR用
  Arguments:    bFlg  ONなら収集
 *---------------------------------------------------------------------------*/

static void _SetScanBeaconData(WMBssDesc* pBss)
{
  int i;

  if(_pWmInfo->AllBeaconNum==0){
    return;
  }
  
  for(i=0;i<_pWmInfo->AllBeaconNum;i++){
    _BEACONCATCH_STR* pS = _pWmInfo->pBeaconCatch[i];
      if(GFL_STD_MemComp(&pS->sBssDesc, pBss, sizeof(WMBssDesc))==0){ //一致
        pS->timer = DEFAULT_TIMEOUT_FRAME;
        return;
      }
  }
  for(i=0;i<_pWmInfo->AllBeaconNum;i++){
    _BEACONCATCH_STR* pS = _pWmInfo->pBeaconCatch[i];
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

void WH_MainLoopScanBeaconData(void)
{
  int i;

  if(_pWmInfo && _pWmInfo->pBeaconCatch){
    for(i=0;i<_pWmInfo->AllBeaconNum;i++){
      _BEACONCATCH_STR* pS = _pWmInfo->pBeaconCatch[i];
      if(pS->timer > 0){
        pS->timer--;
      }
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ビーコンのタイプを得る
 * @param   ビーコン数
 * @param   ヒープID
 * @retval  GAME_COMM_STATUS
 */
//------------------------------------------------------------------------------

GAME_COMM_STATUS WH_GetAllBeaconType(void)
{
  int i;
  int retcode = GAME_COMM_STATUS_NULL;

  for(i=0;i < _pWmInfo->AllBeaconNum;i++){
    _BEACONCATCH_STR* pS = _pWmInfo->pBeaconCatch[i];
    if(pS->timer == 0){
      continue;
    }
    {
      WMBssDesc* bss = &pS->sBssDesc;
    
      DWCApInfo ap;

      if(DWC_ParseWMBssDesc(_pWmInfo->ScanMemory,bss,&ap)){
        OS_TPrintf("DWCApInfo %d\n",ap.aptype);
        switch(ap.aptype){
        case DWC_APINFO_TYPE_USER0:
        case DWC_APINFO_TYPE_USER1:
        case DWC_APINFO_TYPE_USER2:
        case DWC_APINFO_TYPE_USER3:
        case DWC_APINFO_TYPE_USER4:
        case DWC_APINFO_TYPE_USER5:
        case DWC_APINFO_TYPE_USB://          :  ニンテンドーWi-Fi USBコネクタ
          retcode = GAME_COMM_STATUS_WIFI;
          break;
        case DWC_APINFO_TYPE_SHOP://         :  ニンテンドーWi-Fiステーション
        case DWC_APINFO_TYPE_FREESPOT://     :  FREESPOT（フリースポット）
        case DWC_APINFO_TYPE_WAYPORT://      :  Wayport(北米ホットスポット)※現在は使用できません
        case DWC_APINFO_TYPE_NINTENDOZONE:// : Nintendo Zone
          retcode = GAME_COMM_STATUS_WIFI_ZONE;
          break;
        }
        if(retcode!=GAME_COMM_STATUS_NULL){
          return retcode;
        }
      }
    }
  }
  ///@todo セキュリティーがかかっているかどうか
  return retcode;
}

