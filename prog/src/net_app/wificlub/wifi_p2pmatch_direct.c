

static void _vctcheckCommon( WIFIP2PMATCH_WORK *pWork)
{
  if(pWork->pParentWork->VCTOn[0] && pWork->pParentWork->VCTOn[1]){
    GFL_NET_DWC_SetVChat(TRUE);
    GFL_NET_DWC_StartVChat();
    DWCRAP_StartVChat();
    _changeBGMVol( pWork,_VOL_DEFAULT/3 );
  }
#if PM_DEBUG
  else{
    NET_PRINT("VCTOFF %d %d\n",pWork->pParentWork->VCTOn[0] ,pWork->pParentWork->VCTOn[1] );
  }
#endif
}



//------------------------------------------------------------------
/**
 * @brief   まず子機が申し込みを受けるかどうか選択 ２回目は台詞が違う WIFIP2PMATCH_PLAYERDIRECT_INIT_NEXT1
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _playerDirectInit1Next( WIFIP2PMATCH_WORK *wk, int seq )
{
  int gmmno,gmmidx;
  u16 friendNo,status,gamemode;
  WIFI_STATUS* p_status;
  MCR_MOVEOBJ* p_player;
  MCR_MOVEOBJ* p_npc;
  u32 way;

 // OS_TPrintf("_playerDirectInit1Next %d\n",wk->friendNo);
  
  GFL_NET_SetClientConnect(GFL_NET_HANDLE_GetCurrentHandle(),FALSE);  //接続禁止
  if(!GFL_NET_IsParentMachine()){
    _friendNameExpand(wk,  wk->friendNo - 1);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1011, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_INIT2);
  }
  else{
    _friendNameExpand(wk,  wk->friendNo - 1);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_073, FALSE);
    WifiP2PMatchMessage_TimeIconStart(wk);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   まず子機が申し込みを受けるかどうか選択 WIFIP2PMATCH_PLAYERDIRECT_INIT1
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectInit1( WIFIP2PMATCH_WORK *wk, int seq )
{
  int gmmno,gmmidx;
  u16 friendNo,status,gamemode;
  WIFI_STATUS* p_status;
  MCR_MOVEOBJ* p_player;
  MCR_MOVEOBJ* p_npc;
  u32 way;

  GFL_NET_SetClientConnect(GFL_NET_HANDLE_GetCurrentHandle(),FALSE);  //接続禁止
  if(!GFL_NET_IsParentMachine()){
    wk->state = WIFIP2PMATCH_STATE_TALK;
    _friendNameExpand(wk,  wk->friendNo - 1);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_083, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_INIT2);
  }
  else{
    wk->state = WIFIP2PMATCH_STATE_RECV;
    _friendNameExpand(wk,  wk->friendNo - 1);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1018, FALSE);
    WifiP2PMatchMessage_TimeIconStart(wk);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   まず子機が申し込みを受けるかどうか選択 WIFIP2PMATCH_PLAYERDIRECT_INIT2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectInit2( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }

  _yenowinCreateM2(wk);

  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_INIT3);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   まず子機が申し込みを受けるかどうか選択 WIFIP2PMATCH_PLAYERDIRECT_INIT3
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectInit3( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret;
  u8 command;

  GFL_FONTSYS_SetDefaultColor();
  ret = _bmpMenu_YesNoSelectMain(wk);

  if(ret == BMPMENU_NULL){  // まだ選択中
    return seq;
  }
  else if(ret == 0){ // はいを選択した場合
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_INIT5;
  }
  else{  // いいえを選択した場合
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_END;
  }
  EndMessageWindowOff(wk);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_INIT5
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectInit5( WIFIP2PMATCH_WORK *wk, int seq )
{
  int gmmno,gmmidx;
  u16 friendNo,status,gamemode;
  WIFI_STATUS* p_status;
  MCR_MOVEOBJ* p_player;
  MCR_MOVEOBJ* p_npc;
  u32 way;

  GFL_NET_SetClientConnect(GFL_NET_HANDLE_GetCurrentHandle(),FALSE);  //接続禁止
  if(GFL_NET_IsParentMachine()){

    wk->state = WIFIP2PMATCH_STATE_TALK;

    _friendNameExpand(wk,  wk->friendNo - 1);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_084, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_INIT6);
  }
  else{

    wk->state = WIFIP2PMATCH_STATE_RECV;

    WifiP2PMatchMessagePrint(wk, msg_wifilobby_073, FALSE);
    WifiP2PMatchMessage_TimeIconStart(wk);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_INIT6
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectInit6( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  _ParentModeSelectMenu(wk);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_INIT7);
  return seq;
}





//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_INIT7
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectInit7( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret = BMPMENULIST_NULL;
  int command;

  PRINT_UTIL_Trans( &wk->SysMsgPrintUtil, wk->SysMsgQue );
  ret = BmpMenuList_Main(wk->sublw);

  switch(ret){
  case WIFI_GAME_TVT:
  case WIFI_GAME_VCT:
    if(!wk->pParentWork->vchatMain){
      return seq;
    }
    break;
  }
  if(BMPMENULIST_NULL!=ret){
    EndMessageWindowOff(wk);
  }
  switch(ret){
  case BMPMENULIST_NULL:
    return seq;
  case BMPMENULIST_CANCEL:
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_CANCELEND;
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
    break;
  case WIFI_GAME_VCT:
    if(wk->pParentWork->VCTOn[1]==FALSE ){
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_1033 , FALSE);
      _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_RETURN);
    }
    else{
      wk->command = WIFIP2PMATCH_PLAYERDIRECT_VCT;
      _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
    }
    break;
  case WIFI_GAME_TVT:
    if(wk->pParentWork->VCTOn[1]==FALSE ){
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_1033 , FALSE);
      _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_RETURN);
    }
    else{
      if((FALSE == DS_SYSTEM_IsRestrictPhotoExchange()) && DS_SYSTEM_IsRunOnTwl()){  //許可
        WifiP2PMatchMessagePrint(wk, msg_wifilobby_1046, FALSE);
        _CHANGESTATE(wk,WIFIP2PMATCH_MODE_TVTMESSAGE3_YESNO);
      }
      else{
        wk->command=WIFIP2PMATCH_PLAYERDIRECT_TVT;
        _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
      }
    }
    break;
  case WIFI_GAME_TRADE:
    {
      if(FALSE == _tradeNumCheck(wk)){
        WifiP2PMatchMessagePrint(wk, msg_wifilobby_1013, FALSE);
        _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_RETURN);
      }
      else{
        if(wk->pParentWork->VCTOn[0]!=wk->pParentWork->VCTOn[1]){  //自分と相手のVCTが違う
          wk->keepGameMode=WIFI_GAME_TRADE;
          _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE1);
        }
        else{
          wk->command = WIFIP2PMATCH_PLAYERDIRECT_TRADE;
          _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
        }
      }
    }
    break;
  case WIFI_GAME_BATTLE_SINGLE_ALL:
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE1);
    break;
  }
  wk->SubListWin = _BmpWinDel(wk->SubListWin);
  BmpMenuList_Exit(wk->sublw, NULL, &wk->singleCur[_MENUTYPE_GAME]);
  BmpMenuWork_ListDelete( wk->submenulist );
  return seq;
}



//

//------------------------------------------------------------------
/**
 * $brief   TVT確認メッセージ  WIFIP2PMATCH_MODE_TVTMESSAGE3_YESNO
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static int _modeTVT3YesNo( WIFIP2PMATCH_WORK* wk, int seq )
{
  WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン
  if( WifiP2PMatchMessageEndCheck(wk) ){
    // はいいいえウインドウを出す
    _yenowinCreateM2(wk);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_TVTMESSAGE3_WAIT);
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   TVT確認メッセージ  WIFIP2PMATCH_MODE_TVTMESSAGE3_WAIT
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static int _modeTVT3Wait( WIFIP2PMATCH_WORK* wk, int seq )
{
  int i;
  int ret;
  ret = _bmpMenu_YesNoSelectMain(wk);

  WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン

  if(ret == BMPMENU_NULL){  // まだ選択中
    return seq;
  }else if(ret == 0){ // はいを選択した場合
    wk->command=WIFIP2PMATCH_PLAYERDIRECT_TVT;
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  }
  else{  // いいえを選択した場合
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_CANCELEND;
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  }
  return seq;
}




//------------------------------------------------------------------
/**
 * $brief   TVT確認メッセージ  WIFIP2PMATCH_MODE_TVTMESSAGE4_YESNO
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static int _modeTVT4YesNo( WIFIP2PMATCH_WORK* wk, int seq )
{
  WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン
  if( WifiP2PMatchMessageEndCheck(wk) ){
    // はいいいえウインドウを出す
    _yenowinCreateM2(wk);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_TVTMESSAGE4_WAIT);
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   TVT確認メッセージ  WIFIP2PMATCH_MODE_TVTMESSAGE4_WAIT
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static int _modeTVT4Wait( WIFIP2PMATCH_WORK* wk, int seq )
{
  int i;
  int ret;
  ret = _bmpMenu_YesNoSelectMain(wk);

  WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン

  if(ret == BMPMENU_NULL){  // まだ選択中
    return seq;
  }else if(ret == 0){ // はいを選択した場合
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_SUBSTARTCALL);
    EndMessageWindowOff(wk);
  }
  else{  // いいえを選択した場合
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_SUB_FAILED;
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);

//    wk->command = WIFIP2PMATCH_PLAYERDIRECT_CANCELEND;
//    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  }
  return seq;
}






//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_RETURN
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectReturn( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  if(GFL_UI_KEY_GetTrg()){
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_INIT5);
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   子機がなにするかどうか選択 
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectSubMain( WIFIP2PMATCH_WORK *wk, int seq )
{
  int gmmno,gmmidx;
  u16 friendNo,status,gamemode;
  u32 way;
  int message[]={msg_wifilobby_085,msg_wifilobby_086,msg_wifilobby_087};

  if(!GFL_NET_IsParentMachine()){
    _friendNameExpand(wk,  wk->friendNo - 1);
    WifiP2PMatchMessagePrint(wk, message[  wk->directmode - WIFIP2PMATCH_PLAYERDIRECT_VCT ], FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_SUB1);
  }
  else{
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_073, FALSE);
    WifiP2PMatchMessage_TimeIconStart(wk);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   子機がVCTするかどうか選択 WIFIP2PMATCH_PLAYERDIRECT_VCT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectVCT( WIFIP2PMATCH_WORK *wk, int seq )
{
  wk->directmode = WIFIP2PMATCH_PLAYERDIRECT_VCT;
  return _playerDirectSubMain(wk,seq);
}

//------------------------------------------------------------------
/**
 * @brief   子機がTVTするかどうか選択 WIFIP2PMATCH_PLAYERDIRECT_TVT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectTVT( WIFIP2PMATCH_WORK *wk, int seq )
{
  wk->directmode = WIFIP2PMATCH_PLAYERDIRECT_TVT;
  return _playerDirectSubMain(wk,seq);
}


//------------------------------------------------------------------
/**
 * @brief   子機がTRADEするかどうか選択 WIFIP2PMATCH_PLAYERDIRECT_TRADE
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectTrade( WIFIP2PMATCH_WORK *wk, int seq )
{
  wk->directmode = WIFIP2PMATCH_PLAYERDIRECT_TRADE;
  return _playerDirectSubMain(wk,seq);
}



//------------------------------------------------------------------
/**
 * @brief   子機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_SUB1
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectSub1( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  _yenowinCreateM2(wk);
  _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_SUB2);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   子機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_SUB2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectSub2( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret;
  u8 command;
  GFL_FONTSYS_SetDefaultColor();
  ret = _bmpMenu_YesNoSelectMain(wk);

  if(ret == BMPMENU_NULL){  // まだ選択中
    return seq;
  }
  else if(ret == 0){ // はいを選択した場合
    if(FALSE == _tradeNumCheck(wk) && (wk->directmode == WIFIP2PMATCH_PLAYERDIRECT_TRADE)){
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_1013, FALSE);
      _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_SUB23);
    }
    else if(((FALSE == DS_SYSTEM_IsRestrictPhotoExchange()) && DS_SYSTEM_IsRunOnTwl()) && (wk->directmode == WIFIP2PMATCH_PLAYERDIRECT_TVT) ){  //許可
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_1046, FALSE);
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_TVTMESSAGE4_YESNO);
    }
    else{
      _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_SUBSTARTCALL);
      EndMessageWindowOff(wk);
    }
  }
  else{  // いいえを選択した場合
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_SUB_FAILED;
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   交換時に数が足りない場合  WIFIP2PMATCH_PLAYERDIRECT_SUB23
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectSub23( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(!WifiP2PMatchMessageEndCheck(wk)){
    return seq;
  }
  if(!GFL_UI_KEY_GetTrg()){
    return seq;
  }
  if((wk->state == WIFIP2PMATCH_STATE_TALK) || (wk->state == WIFIP2PMATCH_STATE_RECV)){
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_SUB_FAILED;
  }
  else{
    wk->command = WIFIP2PMATCH_PLAYERMACHINE_TALKEND;
  }
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   子機がいいえをした場合  WIFIP2PMATCH_PLAYERDIRECT_SUB_FAILED
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectSubFailed( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_NET_IsParentMachine()){
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1038, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
  }
  else{
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1010, FALSE);
    wk->timer = 120;
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_SUB3);
  }
  return seq;
}



//------------------------------------------------------------------
/**
 * @brief   子機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_SUB3
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectSub3( WIFIP2PMATCH_WORK *wk, int seq )
{
  wk->timer--;

  //BTS3550の対処 Saito
  //メッセージを早送りするために読んでいます
  WifiP2PMatchMessageEndCheck(wk);

  if( wk->timer < 0){
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_INIT_NEXT1;
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   子機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_SUBSTARTCALL
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectSubstartCall( WIFIP2PMATCH_WORK *wk, int seq )
{
  u8 command;

  wk->command = WIFIP2PMATCH_PLAYERDIRECT_SUBSTART;
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_ENDCALL
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectEndCall( WIFIP2PMATCH_WORK *wk, int seq )
{
  wk->command = WIFIP2PMATCH_PLAYERDIRECT_END;
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_ENDCALL2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectEndCall2( WIFIP2PMATCH_WORK *wk, int seq )
{
  wk->command = WIFIP2PMATCH_PLAYERDIRECT_END2;
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  return seq;
}




//------------------------------------------------------------------
/**
 * @brief   VCT開始   WIFIP2PMATCH_PLAYERDIRECT_SUBSTART
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectSubStart( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 status,gamemode;


  WIFI_STATUS_ResetVChatMac(wk->pMatch);
  wk->DirectMacSet = 0;
  status = WIFI_STATUS_PLAYING;

  switch(wk->directmode){
  case WIFIP2PMATCH_PLAYERDIRECT_VCT:
    gamemode = WIFI_GAME_VCT;
//    GFL_NET_DWC_SetVChat(TRUE);
    break;
  case WIFIP2PMATCH_PLAYERDIRECT_TVT:
    gamemode = WIFI_GAME_TVT;
 //   GFL_NET_DWC_SetVChat(FALSE);
    break;
  case WIFIP2PMATCH_PLAYERDIRECT_TRADE:
    gamemode = WIFI_GAME_TRADE;
    break;
  }
  _myStatusChange(wk, status,gamemode);  // 接続中になる

  switch(wk->directmode){
  case WIFIP2PMATCH_PLAYERDIRECT_VCT:
    {
      _vctcheckCommon(wk);
      WifiP2PMatchMessagePrintDirect(wk, msg_wifilobby_1015, FALSE);
      _myStatusChange(wk, WIFI_STATUS_PLAYING, WIFI_GAME_VCT);  // VCT中になる
      _changeBGMVol( wk,  0 );
      wk->VChatModeOff = FALSE;
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_CONNECT);
    }
    break;
  case WIFIP2PMATCH_PLAYERDIRECT_TVT:
    {
      WIFI_STATUS* p_status;
      EndMessageWindowOff(wk);

      wk->endSeq = gamemode;

      return SEQ_OUT;            //終了シーケンスへ
    }
    break;
  case WIFIP2PMATCH_PLAYERDIRECT_TRADE:
    _vctcheckCommon(wk);
    {
      WIFI_STATUS* p_status;
      EndMessageWindowOff(wk);

      wk->endSeq = gamemode;

      return SEQ_OUT;            //終了シーケンスへ
    }
    break;
  }
  return seq;
}




//------------------------------------------------------------------
/**
 * @brief   バトルのレギュレーション選択 WIFIP2PMATCH_PLAYERDIRECT_BATTLE1
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattle1( WIFIP2PMATCH_WORK *wk, int seq )
{
  _battleCustomSelectMenu(wk,TRUE);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE2);
  return seq;
}





//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_BATTLE2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattle2( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret = BMPMENULIST_NULL;
  int command;

  PRINT_UTIL_Trans( &wk->SysMsgPrintUtil, wk->SysMsgQue );
  ret = BmpMenuList_Main(wk->sublw);

  if(ret ==  BMPMENULIST_NULL){
    return seq;
  }
  
  EndMessageWindowOff(wk);
  wk->SubListWin = _BmpWinDel(wk->SubListWin);
  BmpMenuList_Exit(wk->sublw, NULL, &wk->singleCur[_MENUTYPE_BATTLE_CUSTOM]);
  BmpMenuWork_ListDelete( wk->submenulist );
  
  switch(ret){
  case BMPMENULIST_CANCEL:
    if((wk->state == WIFIP2PMATCH_STATE_TALK) || (wk->state == WIFIP2PMATCH_STATE_RECV)){  //会話の流れ
      _friendNameExpand(wk,  wk->friendNo - 1);
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_084, FALSE);
      _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_INIT6);
    }
    else{  //掲示板の流れ
      _CHANGESTATE(wk, WIFIP2PMATCH_MODE_SELECT_INIT);
    }
    break;
  default:
    if((wk->state == WIFIP2PMATCH_STATE_TALK) || (wk->state == WIFIP2PMATCH_STATE_RECV)){ //会話の流れ
      _CHANGESTATE(wk, ret);
    }
    else{  //掲示板の流れ
      if(WIFIP2PMATCH_PLAYERDIRECT_BATTLE_DECIDE==ret){
        _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERMACHINE_BATTLE_DECIDE);
      }
      else{
        _CHANGESTATE(wk, ret);
      }
    }
    break;
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_MODE
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleMode( WIFIP2PMATCH_WORK *wk, int seq )
{

  _battleModeSelectMenu(wk);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_MODE2);
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_MODE2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleMode2( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret = BMPMENULIST_NULL;
  int command;

  PRINT_UTIL_Trans( &wk->SysMsgPrintUtil, wk->SysMsgQue );
  ret = BmpMenuList_Main(wk->sublw);

  switch(ret){
  case BMPMENULIST_NULL:
    return seq;
  case BMPMENULIST_CANCEL:
    break;
  default:
    wk->battleMode = ret;
    wk->battleModeSelect  = ret;
    break;
  }
  _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE1);
  EndMessageWindowOff(wk);
  wk->SubListWin = _BmpWinDel(wk->SubListWin);
  BmpMenuList_Exit(wk->sublw, NULL,  &wk->singleCur[_MENUTYPE_BATTLE_MODE]);
  BmpMenuWork_ListDelete( wk->submenulist );
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_RULE
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleRule( WIFIP2PMATCH_WORK *wk, int seq )
{
  _battleRuleSelectMenu(wk);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_RULE2);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_RULE2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleRule2( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret = BMPMENULIST_NULL;
  int command;

  PRINT_UTIL_Trans( &wk->SysMsgPrintUtil, wk->SysMsgQue );
  ret = BmpMenuList_Main(wk->sublw);

  switch(ret){
  case BMPMENULIST_NULL:
    return seq;
  case BMPMENULIST_CANCEL:
    break;
  default:
    wk->battleRule  = ret;
    wk->battleRuleSelect  = ret;
    break;
  }
  _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE1);
  EndMessageWindowOff(wk);
  wk->SubListWin = _BmpWinDel(wk->SubListWin);
  BmpMenuList_Exit(wk->sublw, NULL, &wk->singleCur[_MENUTYPE_BATTLE_RULE]);
  BmpMenuWork_ListDelete( wk->submenulist );
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_SHOOTER
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleShooter( WIFIP2PMATCH_WORK *wk, int seq )
{
  _battleShooterSelectMenu(wk);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_SHOOTER2);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_SHOOTER2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleShooter2( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret = BMPMENULIST_NULL;
  int command;

  PRINT_UTIL_Trans( &wk->SysMsgPrintUtil, wk->SysMsgQue );
  ret = BmpMenuList_Main(wk->sublw);

  switch(ret){
  case BMPMENULIST_NULL:
    return seq;
  case BMPMENULIST_CANCEL:
    break;
  default:
    wk->pParentWork->shooter =  1-ret;
    wk->pParentWork->shooterSelect =  1-ret;
    break;
  }
  _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE1);
  EndMessageWindowOff(wk);
  wk->SubListWin = _BmpWinDel(wk->SubListWin);
  BmpMenuList_Exit(wk->sublw, NULL, &wk->singleCur[_MENUTYPE_BATTLE_SHOOTER]);
  BmpMenuWork_ListDelete( wk->submenulist );
  return seq;
}



//--------------------------------------------------------------
/**
 * 手持ちポケモンのレギュレーションチェック
 *
 * @param   REGULATION
 * @param   GAMEDATA
 * @param   fail_bit		
 * @retval  POKE_REG_RETURN_ENUM		
 */
//--------------------------------------------------------------
static POKE_REG_RETURN_ENUM _CheckRegulation_Temoti(REGULATION* pReg,GAMEDATA* pGameData,u32 *fail_bit)
{
  POKE_REG_RETURN_ENUM reg_ret;
  
  *fail_bit = 0;
  reg_ret =
    PokeRegulationMatchLookAtPokeParty(pReg, 
                                       GAMEDATA_GetMyPokemon(pGameData), fail_bit);
  return reg_ret;
}

//--------------------------------------------------------------
/**
 * バトルボックスのPOKEPARTYをAllocして作成します
 *
 * @param   unisys		
 *
 * @retval  POKEPARTY *		AllocしたPOKEPARTY (バトルボックスが無い場合はNULL)
 */
//--------------------------------------------------------------
static POKEPARTY * _BBox_PokePartyAlloc(GAMEDATA* pGameData)
{
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(pGameData);
  BATTLE_BOX_SAVE *bb_save = BATTLE_BOX_SAVE_GetBattleBoxSave( sv_ctrl );
  
  if(BATTLE_BOX_SAVE_IsIn( bb_save ) == FALSE){
    return NULL;
  }
  
  return BATTLE_BOX_SAVE_MakePokeParty( bb_save, HEAPID_WIFIP2PMATCH );
}

//--------------------------------------------------------------
/**
 * バトルボックスのPOKEPARTYをFreeします
 * @param   party		
 */
//--------------------------------------------------------------
static void _BBox_PokePartyFree(POKEPARTY *party)
{
  if(party){
    GFL_HEAP_FreeMemory(party);
  }
}


//--------------------------------------------------------------
/**
 * バトルボックスのレギュレーションチェック
 *
 * @param   unisys		
 * @param   fail_bit		
 *
 * @retval  POKE_REG_RETURN_ENUM		
 *
 * unisys->alloc.regulationにレギュレーションデータがロードされている必要があります
 */
//--------------------------------------------------------------
static POKE_REG_RETURN_ENUM _CheckRegulation_BBox(REGULATION* pReg,POKEPARTY *bb_party, u32 *fail_bit)
{
  POKE_REG_RETURN_ENUM reg_ret=POKE_REG_NUM_FAILED;
  
  *fail_bit = 0;
  
  if(bb_party != NULL){
    reg_ret = PokeRegulationMatchLookAtPokeParty(
      pReg, bb_party, fail_bit);
  }
  else{ //バトルボックスのセーブデータが存在しない
    *fail_bit = 0xffffffff;
  }
  return reg_ret;
}

static u32 _regtable[]={
    REG_FREE_SINGLE, REG_FREE_DOUBLE, REG_FREE_TRIPLE, REG_FREE_ROTATION,
    REG_FLAT_SINGLE,REG_FLAT_DOUBLE,REG_FLAT_TRIPLE,REG_FLAT_ROTATION
    };

static u32 _createRegulationType(WIFIP2PMATCH_WORK *wk, REG_CREATE_TYPE type )
{
  u32 regulation;
  u16 battleMode;  //バトルのモード
  u16 battleRule;  //バトルのルール
  u8  shooter;

  switch( type )
  {
  default:
    GF_ASSERT(0);
  case REG_CREATE_TYPE_DECIDE: ///<決定しているルールから作る
    battleMode  = wk->battleMode;
    battleRule  = wk->battleRule;
    shooter     = wk->pParentWork->shooter;
    break;
  case REG_CREATE_TYPE_SELECT: ///<選んでいるルールから作る
    battleMode  = wk->battleModeSelect;
    battleRule  = wk->battleRuleSelect;
    shooter     = wk->pParentWork->shooterSelect;
    break;
  }

//  NET_PRINT("regcr %d %d %d\n",battleMode,battleRule,shooter);
  regulation = _regtable[battleMode + (battleRule*4)];
  if(wk->pRegulation){
    GFL_HEAP_FreeMemory((void*)wk->pRegulation);
  }
  wk->pRegulation = (REGULATION*)PokeRegulation_LoadDataAlloc(regulation, HEAPID_WIFIP2PMATCH);
  if(shooter){
    regulation = regulation | 0x80000000;
  }
  return regulation;
}


static u32 _createRegulation(WIFIP2PMATCH_WORK *wk)
{
  //現在選んでいるルールからレギュレーションを作成しなければならなくなり、
  //処理をわけるのはまずいため、以下のようにいたしました nagihashi
  return _createRegulationType(wk, REG_CREATE_TYPE_DECIDE );
}

static void _convertRegulation(WIFIP2PMATCH_WORK *wk,u32 regulation)
{
  int i;
  if(regulation & 0x80000000){
    wk->pParentWork->shooter = 1;
  }
  else{
    wk->pParentWork->shooter = 0;
  }
  regulation = (regulation & 0xfffffff);
  for(i=0;i<elementof(_regtable);i++){
    if(_regtable[i] == regulation){
      wk->battleMode = i % 4;
      wk->battleRule = i / 4;
    }
  }
//  NET_PRINT("regch %d %d %d\n",wk->battleMode,wk->battleRule,wk->pParentWork->shooter);
}

//------------------------------------------------------------------
/**
 * @brief   親機が決定選択  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_DECIDE
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------


static int _playerDirectBattleDecide( WIFIP2PMATCH_WORK *wk, int seq )
{
  u8 command;
  u32 regulation = _createRegulation(wk);
  u32 fail_bit;


  if(!_regulationCheck(wk)){
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_100, FALSE);
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_NOREG_PARENT);
    return seq;
  }
  
  wk->keepGameMode = WIFI_GAME_BATTLE_SINGLE_ALL;
  if(wk->pParentWork->VCTOn[0]!=wk->pParentWork->VCTOn[1]){  //自分と相手のVCTが違う
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE1);
  }
  else{
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE5);
  }
  _friendNameExpand(wk,  wk->friendNo - 1);
  if((wk->state == WIFIP2PMATCH_STATE_TALK) || (wk->state == WIFIP2PMATCH_STATE_RECV)){
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_014, FALSE);
    WifiP2PMatchMessage_TimeIconStart(wk);
  }
  else{
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_073, FALSE);
    WifiP2PMatchMessage_TimeIconStart(wk);
  }

//  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   レギュレーションがあわなかった WIFIP2PMATCH_PLAYERDIRECT_NOREG_PARENT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectNoregParent( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 fail_bit;

  if(!WifiP2PMatchMessageEndCheck(wk)){
    return seq;
  }
  if(GFL_UI_KEY_GetTrg()){
    EndMessageWindowOff(wk);

    _CheckRegulation_Temoti(wk->pRegulation, wk->pGameData, &fail_bit );
    _Menu_RegulationSetup(wk, fail_bit, wk->pParentWork->shooter , REGWIN_TYPE_NG_TEMOTI);

    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_NOREG_PARENT1);
  }
  return seq;
}

//WIFIP2PMATCH_PLAYERDIRECT_NOREG_PARENT1
static int _playerDirectNoregParent1( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 fail_bit;
  if(GFL_UI_KEY_GetTrg()){
    _Menu_RegulationDelete(wk);
    if( wk->bb_party){
      _CheckRegulation_BBox(wk->pRegulation, wk->bb_party, &fail_bit );
      _Menu_RegulationSetup(wk, fail_bit, wk->pParentWork->shooter , REGWIN_TYPE_NG_BBOX);
      _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_NOREG_PARENT2);
    }
    else{
      switch(wk->state ){  
      case WIFIP2PMATCH_STATE_TALK://会話の流れ
        _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE1);
        break;
      case WIFIP2PMATCH_STATE_MACHINE_RECV:
        wk->command = WIFIP2PMATCH_PLAYERMACHINE_TALKEND;
        _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
        break;
      case WIFIP2PMATCH_STATE_RECV:
        wk->command = WIFIP2PMATCH_PLAYERDIRECT_SUB_FAILED;
        _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
        break;
      default:
        _CHANGESTATE(wk, WIFIP2PMATCH_RETURNLIST);
        break;
      }
    }
  }
  return seq;
}


static int _playerDirectNoregParent2( WIFIP2PMATCH_WORK *wk, int seq )
{  
  if(GFL_UI_KEY_GetTrg()){
    _Menu_RegulationDelete(wk);
    EndMessageWindowOff(wk);

    switch(wk->state){
    case WIFIP2PMATCH_STATE_TALK:  //会話の流れ
      _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE1);
      break;
    case WIFIP2PMATCH_STATE_MACHINE_RECV:
      wk->command = WIFIP2PMATCH_PLAYERMACHINE_TALKEND;
      _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
      break;
    case WIFIP2PMATCH_STATE_RECV:
      wk->command = WIFIP2PMATCH_PLAYERDIRECT_SUB_FAILED;
      _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
      break;
    default:
      _CHANGESTATE(wk, WIFIP2PMATCH_RETURNLIST);
      break;
    }
  }
  return seq;
}



static int _playerDirectBattleWatch( WIFIP2PMATCH_WORK *wk, int seq )
{
  //現在選んでいるルールを見るため、選んでいるものから作成
  u32 regulation = _createRegulationType(wk,REG_CREATE_TYPE_SELECT);
  _Menu_RegulationSetup(wk,0,wk->pParentWork->shooterSelect,REGWIN_TYPE_RULE);
  _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_WATCH2);
  return seq;
}


static int _playerDirectBattleWatch2( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_UI_KEY_GetTrg()){
    _Menu_RegulationDelete(wk);
  //  wk->SysMsgWin = _BmpWinDel(wk->SysMsgWin);
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE1);
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   子機が送られたレギュレーションでバトルできるか検査   WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleGO( WIFIP2PMATCH_WORK *wk, int seq )
{
//  u8 command;

  if(!GFL_NET_IsParentMachine()){
    u32 regulation = _createRegulation(wk);

    _Menu_RegulationSetup(wk,0,wk->pParentWork->shooter ,REGWIN_TYPE_RULE_SHORT);

    _friendNameExpand(wk,  wk->friendNo - 1);
//    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1032, FALSE);

    WifiP2PMatchMessagePrintS(wk, msg_wifilobby_1001+wk->battleMode+wk->battleRule*4, FALSE, TRUE);

    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO_12);
  }
  else{
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_073, FALSE);
    WifiP2PMatchMessage_TimeIconStart(wk);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
  }
  return seq;
}


static int _playerDirectBattleGO_12( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  {
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO_13);
  }
  return seq;
  
}


static int _playerDirectBattleGO_13( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_UI_KEY_GetTrg()){
    _Menu_RegulationDelete(wk);
 //   _Menu_RegulationDelete(wk->SysMsgWin);
//    wk->SysMsgWin = _BmpWinDel(wk->SysMsgWin);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO_14);
  }
  return seq;
}

static int _playerDirectBattleGO_14( WIFIP2PMATCH_WORK *wk, int seq )
{
 // WifiP2PMatchMessagePrint(wk, msg_wifilobby_1001+wk->battleMode+wk->battleRule*4, FALSE);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO2);
  return seq;
}


static int _playerDirectBattleGO1( WIFIP2PMATCH_WORK *wk, int seq )
{
  return seq;
}

//WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO2
static int _playerDirectBattleGO2( WIFIP2PMATCH_WORK *wk, int seq )
{
//  if( !WifiP2PMatchMessageEndCheck(wk) ){
  //  return seq;
//  }
  _yenowinCreateM2(wk);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO3);
  return seq;
}

//WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO3
static int _playerDirectBattleGO3( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret;
  u8 command;
  GFL_FONTSYS_SetDefaultColor();
  ret = _bmpMenu_YesNoSelectMain(wk);

  if(ret == BMPMENU_NULL){  // まだ選択中
    return seq;
  }
  else if(ret == 0){ // はいを選択した場合
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO4);
    EndMessageWindowOff(wk);
  }
  else{  // いいえを選択した場合
    if((wk->state == WIFIP2PMATCH_STATE_MACHINE)  || (wk->state == WIFIP2PMATCH_STATE_MACHINE_RECV)){
      wk->command = WIFIP2PMATCH_PLAYERDIRECT_END;
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_071, FALSE);
      _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
    }
    else{
  //    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1010, FALSE);
   //   _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO3_KEYWAIT);
      wk->command = WIFIP2PMATCH_PLAYERDIRECT_SUB_FAILED;
      _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
    }
  }
  return seq;
}


// WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO3_KEYWAIT
static int _playerDirectBattleGo3KeyWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_UI_KEY_GetTrg()){
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_BATTLE_FAILED;
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  }
  return seq;
}


//WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO4
static int _playerDirectBattleGO4( WIFIP2PMATCH_WORK *wk, int seq )
{
  u8 command;
  {  
    u32 regulation = _createRegulation(wk);
    u32 fail_bit;

    if(!_regulationCheck(wk)){        // 選ぶ事ができない
      switch(wk->state ){
      case WIFIP2PMATCH_STATE_TALK:
        _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_SUB45);
        WifiP2PMatchMessagePrint(wk, msg_wifilobby_100, FALSE);
        return seq;
      
      case WIFIP2PMATCH_STATE_MACHINE_RECV:
      case WIFIP2PMATCH_STATE_RECV: //もし選ぶことができないならば原因を表示
        _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_NOREG_PARENT);
        WifiP2PMatchMessagePrint(wk, msg_wifilobby_100, FALSE);
        return seq;

      default:
        _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_SUB23);
        WifiP2PMatchMessagePrint(wk, msg_wifilobby_100, FALSE);
        return seq;
      }
    }
    else{
      wk->command = WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START;
    }
  }
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  return seq;
}



//------------------------------------------------------------------
/**
 * @brief   対戦時に数が足りない場合  WIFIP2PMATCH_PLAYERDIRECT_SUB45
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectSub45( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(!WifiP2PMatchMessageEndCheck(wk)){
    return seq;
  }
  if(!GFL_UI_KEY_GetTrg()){
    return seq;
  }
  wk->command = WIFIP2PMATCH_PLAYERDIRECT_BATTLE_FAILED;
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  return seq;
}



  
//------------------------------------------------------------------
/**
 * @brief   試合開始 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleStart( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 fail_bit;
  int id=0;
  
  if(!GFL_NET_IsParentMachine()){
    id = 1;
  }

  if(POKE_REG_OK!=_CheckRegulation_BBox(wk->pRegulation, wk->bb_party, &fail_bit )
      && wk->bb_party == NULL){
    //てもちが大丈夫＋バトルボックスが空の場合すすむ
    PokeParty_Copy(GAMEDATA_GetMyPokemon(wk->pGameData), wk->pParentWork->pPokeParty[id]);
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START4);
  }
  else{
    //上記以外はかならず選択肢に進む

    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1009, FALSE);
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START2);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   試合開始 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleStart2( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  wk->backupCursor = 2;
  _battlePokePartySelectMenu( wk );

  {
    _pokeIconResourceCreate(wk, GAMEDATA_GetMyPokemon(wk->pGameData), wk->bb_party);
  }
  
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START3);
  return seq;
}
//------------------------------------------------------------------
/**
 * @brief   試合開始 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START3
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleStart3( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret = BMPMENULIST_NULL;
  int command,id=0;
  u32 fail_bit;
  u16 list_bak,cursor_bak;
  
  PRINT_UTIL_Trans( &wk->SysMsgPrintUtil, wk->SysMsgQue );
  ret = BmpMenuList_Main(wk->sublw);

  BmpMenuList_PosGet( wk->sublw, &list_bak, &cursor_bak );
  if(cursor_bak != wk->backupCursor){
    wk->backupCursor = cursor_bak;
    if(cursor_bak==0){
      _Print_DrawPokeStatusBmpWin(wk, GAMEDATA_GetMyPokemon(wk->pGameData));
      _pokeIconDispSwitch(wk,TRUE);
    }
    else{
      _Print_DrawPokeStatusBmpWin(wk, wk->bb_party);
      _pokeIconDispSwitch(wk,FALSE);
    }
  }

  
  if(!GFL_NET_IsParentMachine()){
    id = 1;
  }

  switch(ret){
  case BMPMENULIST_NULL:
    return seq;
  case BMPMENULIST_CANCEL:
    return seq;
  case _TEMOTI:
    if(POKE_REG_OK!=_CheckRegulation_Temoti(wk->pRegulation, wk->pGameData, &fail_bit )){ 
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_100, FALSE);
      _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_NOREG_SELECT_TEMOTI);

      _pokeIconResourceDelete(wk);
      _DeletePokeStatus(wk);
      wk->SubListWin = _BmpWinDel(wk->SubListWin);
      BmpMenuList_Exit(wk->sublw, NULL, &wk->singleCur[_MENUTYPE_POKEPARTY]);
      BmpMenuWork_ListDelete( wk->submenulist );
      return seq;
    }
    else{
      PokeParty_Copy(GAMEDATA_GetMyPokemon(wk->pGameData), wk->pParentWork->pPokeParty[id]);
      _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START4);
    }
    break;
  case _BATTLEBOX:
    if(POKE_REG_OK!=_CheckRegulation_BBox(wk->pRegulation, wk->bb_party, &fail_bit )){
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_100, FALSE);
      _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_NOREG_SELECT_BBOX);

      _pokeIconResourceDelete(wk);
      _DeletePokeStatus(wk);
      wk->SubListWin = _BmpWinDel(wk->SubListWin);
      BmpMenuList_Exit(wk->sublw, NULL, &wk->singleCur[_MENUTYPE_POKEPARTY]);
      BmpMenuWork_ListDelete( wk->submenulist );
      return seq;
    }
    else {
      PokeParty_Copy(wk->bb_party, wk->pParentWork->pPokeParty[id]);
      _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START4);
    }
    break;
  }
  _pokeIconResourceDelete(wk);
  _DeletePokeStatus(wk);
  EndMessageWindowOff(wk);
  wk->SubListWin = _BmpWinDel(wk->SubListWin);
  BmpMenuList_Exit(wk->sublw, NULL, &wk->singleCur[_MENUTYPE_POKEPARTY]);
  BmpMenuWork_ListDelete( wk->submenulist );
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   ポケパーティー転送 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START4
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleStart4( WIFIP2PMATCH_WORK *wk, int seq )
{
  int id=0;
  if(!GFL_NET_IsParentMachine()){
    id = 1;
  }

  if(GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(),GFL_NET_SENDID_ALLUSER,
                        CNM_WFP2PMF_POPEPARTY, PokeParty_GetWorkSize(),
                        wk->pParentWork->pPokeParty[id], FALSE, FALSE, TRUE)){

    WifiP2PMatchMessagePrint(wk, msg_wifilobby_073, FALSE);
    WifiP2PMatchMessage_TimeIconStart(wk);

    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START42);
  }
  return seq;
}


//WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START42

static int _playerDirectBattleStart42( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( WifiP2PMatchMessageEndCheck(wk) ){
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_POKEPARTY_END,WB_NET_WIFICLUB);
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START5);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   ポケパーティー転送  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START5
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------


static int _playerDirectBattleStart5( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(), _TIMING_POKEPARTY_END,WB_NET_WIFICLUB )){
    const int min = Regulation_GetParam( wk->pRegulation,REGULATION_NUM_LO);
    const int max = Regulation_GetParam( wk->pRegulation,REGULATION_NUM_HI);
    int msgno;
    if( min == 3 && max == 3 )
    {
      msgno = msg_wifilobby_1043;
    }
    else if( min == 4 && max == 4 )
    {
      msgno = msg_wifilobby_1044;
    }
    else if( min == 6 && max == 6 )
    {
      msgno = msg_wifilobby_1045;
    }
    else
    {
      msgno = msg_wifilobby_1042;
    }
    WifiP2PMatchMessagePrint(wk, msgno, FALSE);
    WifiP2PMatchMessage_TimeIconStart(wk);
    wk->timer  = 120;

    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START6);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   ポケパーティー転送  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START6
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleStart6( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( WifiP2PMatchMessageEndCheck(wk) ){
    if( wk->timer-- < 0)
    {
      _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START7);
    }
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   ポケパーティー転送  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START7
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectBattleStart7( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 status,gamemode;
  
  WIFI_STATUS_ResetVChatMac(wk->pMatch);
  wk->DirectMacSet = 0;
  status = WIFI_STATUS_PLAYING;


  {
    u32 modetbl[]={
      WIFI_GAME_BATTLE_SINGLE_ALL,      // シングルバトル
      WIFI_GAME_BATTLE_SINGLE_FLAT,      // シングルバトル
      WIFI_GAME_BATTLE_DOUBLE_ALL,      // ダブル
      WIFI_GAME_BATTLE_DOUBLE_FLAT,      // ダブル
      WIFI_GAME_BATTLE_TRIPLE_ALL,      // トリプル
      WIFI_GAME_BATTLE_TRIPLE_FLAT,      // トリプル
      WIFI_GAME_BATTLE_ROTATION_ALL,      // ローテーション
      WIFI_GAME_BATTLE_ROTATION_FLAT,      // ローテーション
    };
    gamemode = modetbl[wk->battleMode*2+wk->battleRule];
    _myStatusChange(wk, status, gamemode);  // 接続中になる
    wk->endSeq = gamemode;
  }
  {
    _vctcheckCommon(wk);

    EndMessageWindowOff(wk);
  }
  return SEQ_OUT;            //終了シーケンスへ
}

//------------------------------------------------------------------
/**
 * @brief   手持ちかバトルボックス選択後、手持ちが不正  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_NOREG_SELECT_TEMOTI
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _playerDirectBattleNoregSelectTemoti( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 fail_bit;

  if(!WifiP2PMatchMessageEndCheck(wk)){
    return seq;
  }

  if(GFL_UI_KEY_GetTrg()){
    EndMessageWindowOff(wk);
    
    _CheckRegulation_Temoti(wk->pRegulation, wk->pGameData, &fail_bit );
    _Menu_RegulationSetup(wk, fail_bit, wk->pParentWork->shooter , REGWIN_TYPE_NG_TEMOTI);

    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_NOREG_SELECT_WAIT);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   手持ちかバトルボックス選択後、ボックスが不正  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_NOREG_SELECT_BBOX
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _playerDirectBattleNoregSelectBBox( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 fail_bit;

  if(!WifiP2PMatchMessageEndCheck(wk)){
    return seq;
  }

  if(GFL_UI_KEY_GetTrg()){
    EndMessageWindowOff(wk);

    _CheckRegulation_BBox(wk->pRegulation, wk->bb_party, &fail_bit );
    _Menu_RegulationSetup(wk, fail_bit, wk->pParentWork->shooter , REGWIN_TYPE_NG_BBOX);

    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_NOREG_SELECT_WAIT);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   手持ちかバトルボックス選択後 不正表示待ち WIFIP2PMATCH_PLAYERDIRECT_BATTLE_NOREG_SELECT_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _playerDirectBattleNoregSelectWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_UI_KEY_GetTrg()){
    _Menu_RegulationDelete(wk);

    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1009, FALSE);
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START2);
  }

  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   子機が遊ぶのかどうか意思選択  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_FAILED
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectFailed( WIFIP2PMATCH_WORK *wk, int seq )
{
  int gmmno,gmmidx;
  u16 friendNo,status,gamemode;
  WIFI_STATUS* p_status;
  MCR_MOVEOBJ* p_player;
  MCR_MOVEOBJ* p_npc;
  u32 way;

  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }

  if(!GFL_NET_IsParentMachine()){
    _friendNameExpand(wk,  wk->friendNo - 1);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1011, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_FAILED2);
  }
  else{
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   子機が遊ぶのかどうか意思選択 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_FAILED2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectFailed2( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }

  _yenowinCreateM2(wk);

  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_BATTLE_FAILED3);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   子機が遊ぶのかどうか意思選択 WIFIP2PMATCH_PLAYERDIRECT_BATTLE_FAILED3
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectFailed3( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret;
  u8 command;
  GFL_FONTSYS_SetDefaultColor();
  ret = _bmpMenu_YesNoSelectMain(wk);

  if(ret == BMPMENU_NULL){  // まだ選択中
    return seq;
  }
  else if(ret == 0){ // はいを選択した場合
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_INIT5;
  }
  else{  // いいえを選択した場合
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_END;
  }
  EndMessageWindowOff(wk);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   指定モード終了 WIFIP2PMATCH_PLAYERDIRECT_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  //BTS2302の対処 naigihashi
  //メッセージを早送りするために読んでいます
  //メッセージを描画していなくてもここに来る場合があります
  WifiP2PMatchMessageEndCheck(wk);

  return seq;
}





//------------------------------------------------------------------
/**
 * @brief   指定モード終了自分でキャンセルした場合   WIFIP2PMATCH_PLAYERDIRECT_CANCELEND
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectCancelEnd( WIFIP2PMATCH_WORK *wk, int seq )
{
  _myStatusChange(wk, WIFI_STATUS_WAIT,WIFI_GAME_LOGIN_WAIT);

  wk->state = WIFIP2PMATCH_STATE_NONE;
  GFL_NET_SetAutoErrorCheck(FALSE);
  GFL_NET_SetNoChildErrorCheck(FALSE);

  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_CANCELEND_NEXT);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   指定モード終了自分でキャンセルした場合   WIFIP2PMATCH_PLAYERDIRECT_CANCELEND_NEXT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectCancelEndNext( WIFIP2PMATCH_WORK *wk, int seq )
{

  if(GFL_NET_IsParentMachine()){
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1016, FALSE);
  }
  else{
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1017, FALSE);
  }
  GFL_NET_StateWifiMatchEnd(TRUE);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_END3);
  return seq;
}



//------------------------------------------------------------------
/**
 * @brief   指定モード終了 WIFIP2PMATCH_PLAYERDIRECT_END
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectEnd( WIFIP2PMATCH_WORK *wk, int seq )
{
  _myStatusChange(wk, WIFI_STATUS_WAIT,WIFI_GAME_LOGIN_WAIT);

  GFL_NET_SetAutoErrorCheck(FALSE);
  GFL_NET_SetNoChildErrorCheck(FALSE);

  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_END_NEXT);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   指定モード終了 WIFIP2PMATCH_PLAYERDIRECT_END_NEXT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectEndNext( WIFIP2PMATCH_WORK *wk, int seq )
{
  
  if((WIFIP2PMATCH_STATE_TALK==wk->state) || (WIFIP2PMATCH_STATE_MACHINE_RECV==wk->state)){
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1016, FALSE);
  }
  else{
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1017, FALSE);
  }
  GFL_NET_StateWifiMatchEnd(TRUE);
  wk->state = WIFIP2PMATCH_STATE_NONE;
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_END3);
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   指定モード終了 WIFIP2PMATCH_PLAYERDIRECT_END_KEYWAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectEndKeyWait( WIFIP2PMATCH_WORK *wk, int seq )
{
//  if(!WifiP2PMatchMessageEndCheck(wk)){
  //  return seq;
//  }
//  if(!GFL_UI_KEY_GetTrg()){
  //  return seq;
//  }
//  WifiP2PMatchMessage_CursorEnd(wk);
  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   指定モード終了2 WIFIP2PMATCH_PLAYERDIRECT_END2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectEnd2( WIFIP2PMATCH_WORK *wk, int seq )
{
  _myStatusChange(wk, WIFI_STATUS_WAIT,WIFI_GAME_LOGIN_WAIT);
  GFL_NET_StateWifiMatchEnd(TRUE);

  if(GFL_NET_IsParentMachine()){
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1038, FALSE);
  }
  else{
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1010, FALSE);
  }
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_END3);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   指定モード終了3 WIFIP2PMATCH_PLAYERDIRECT_END3
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectEnd3( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(!WifiP2PMatchMessageEndCheck(wk)){
    return seq;
  }
  if(GFL_UI_KEY_GetTrg()){
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT2);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   VCTを逆転させる WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE1
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectVCTChange1( WIFIP2PMATCH_WORK *wk, int seq )
{
  int msg;
  
  if( wk->pParentWork->VCTOn[1]){
    msg = msg_wifilobby_069;
  }
  else{
    msg = msg_wifilobby_070;
  }
  WifiP2PMatchMessagePrint(wk, msg, FALSE);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE2);
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   VCTを逆転させる WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectVCTChange2( WIFIP2PMATCH_WORK *wk, int seq )
{

  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }

  _yenowinCreateM2(wk);

  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE3);
  return seq;


}

//------------------------------------------------------------------
/**
 * @brief   VCTを逆転させる WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE3
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectVCTChange3( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret;
  u8 command;
  GFL_FONTSYS_SetDefaultColor();
  ret = _bmpMenu_YesNoSelectMain(wk);

  if(ret == BMPMENU_NULL){  // まだ選択中
    return seq;
  }
  else if(ret == 0){ // はいを選択した場合
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE4);
  }
  else{  // いいえを選択した場合
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE_E);
  }
  EndMessageWindowOff(wk);
  return seq;
}



//------------------------------------------------------------------
/**
 * @brief   VCTを逆転させる WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE_E
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectVCTChangeE( WIFIP2PMATCH_WORK *wk, int seq )
{
  wk->command = WIFIP2PMATCH_PLAYERDIRECT_END;
  _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  return seq;
}



//------------------------------------------------------------------
/**
 * @brief   VCTを逆転させる WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE4
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectVCTChange4( WIFIP2PMATCH_WORK *wk, int seq )
{
  u16 gamemode[2];
  gamemode[0] = _WifiMyGameModeGet( wk, wk->pMatch );
  gamemode[1] = 1 - wk->pParentWork->vchatMain;  //自分の状態を反転して送信

  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_STATUS, sizeof(u16)*2, gamemode)){
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE5);
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * @brief   VCTを逆転させる WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE5
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectVCTChange5( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(wk->keepGameMode==WIFI_GAME_TRADE){
    wk->command = WIFIP2PMATCH_PLAYERDIRECT_TRADE;
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  }
  else{
    u32 regulation = _createRegulation(wk);
    if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_REGLATION,
                         sizeof(u32), &regulation)){
      _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE6);
    }
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   VCTを逆転させる WIFIP2PMATCH_PLAYERDIRECT_VCTCHANGE6
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectVCTChange6( WIFIP2PMATCH_WORK *wk, int seq )
{
  wk->command = WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO;
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND);
  return seq;
}



//

//------------------------------------------------------------------
/**
 * @brief   コマンドをセット WIFIP2PMATCH_PLAYERDIRECT_WAIT_COMMAND
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectWaitSendCommand( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_DIRECT_COMMAND,
                      1, &wk->command)){
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
  }
  return seq;
}




//------------------------------------------------------------------
/**
 * @brief   指定モード終了 WIFIP2PMATCH_PLAYERMACHINE_TALKEND
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerMachineTalkEnd( WIFIP2PMATCH_WORK *wk, int seq )
{
  _myStatusChange(wk, WIFI_STATUS_WAIT,WIFI_GAME_LOGIN_WAIT);

  wk->state = WIFIP2PMATCH_STATE_NONE;
  GFL_NET_SetAutoErrorCheck(FALSE);
  GFL_NET_SetNoChildErrorCheck(FALSE);
  GFL_NET_StateWifiMatchEnd(TRUE);

  if(GFL_NET_IsParentMachine()){
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1038, FALSE);
  }
  else{
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_1016, FALSE);
  }
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_END3);
  return seq;
}

