




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
    _friendNameExpand(wk,  wk->friendNo - 1);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_083, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_INIT2);
  }
  else{
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
  if( !PRINTSYS_QUE_IsFinished(wk->SysMsgQue) ){
    return seq;
  }
  wk->pYesNoWork =
    BmpMenu_YesNoSelectInit(
      &_yesNoBmpDat,
      MENU_WIN_CGX_NUM, MENU_WIN_PAL,0,
      HEAPID_WIFIP2PMATCH );
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
  int ret = BmpMenu_YesNoSelectMain(wk->pYesNoWork);
  u8 command;

  if(ret == BMPMENU_NULL){  // まだ選択中
    return seq;
  }
  else if(ret == 0){ // はいを選択した場合
    command = WIFIP2PMATCH_PLAYERDIRECT_INIT5;
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_DIRECT_COMMAND,
                     1, &command);
  }
  else{  // いいえを選択した場合
    command = WIFIP2PMATCH_PLAYERDIRECT_END;
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_DIRECT_COMMAND,
                     1, &command);
  }
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
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
    _friendNameExpand(wk,  wk->friendNo - 1);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_084, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_INIT6);
  }
  else{
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
  if( !PRINTSYS_QUE_IsFinished(wk->SysMsgQue) ){
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
  int ret = BmpMenuList_Main(wk->sublw);
  int command;
  
  switch(ret){
  case BMPMENULIST_NULL:
    return seq;
  case BMPMENULIST_CANCEL:
    command = WIFIP2PMATCH_PLAYERDIRECT_END;
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_DIRECT_COMMAND,
                     1, &command);
    break;
  case WIFI_GAME_VCT:
    command = WIFIP2PMATCH_PLAYERDIRECT_VCT;
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_DIRECT_COMMAND,
                     1, &command);
    break;
  case WIFI_GAME_TVT:
    command=WIFIP2PMATCH_PLAYERDIRECT_TVT;
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_DIRECT_COMMAND,
                     1, &command);
    break;
  case WIFI_GAME_TRADE:
    command = WIFIP2PMATCH_PLAYERDIRECT_TRADE;
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_DIRECT_COMMAND,
                     1, &command);
    break;
  case WIFI_GAME_BATTLE_SINGLE_ALL:
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE1);
    return seq;
  }
  EndMessageWindowOff(wk);
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
  wk->SubListWin = _BmpWinDel(wk->SubListWin);
  BmpMenuList_Exit(wk->sublw, NULL, &wk->battleCur);
  BmpMenuWork_ListDelete( wk->submenulist );
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
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_SUB1
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectSub1( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !PRINTSYS_QUE_IsFinished(wk->SysMsgQue) ){
    return seq;
  }
  wk->pYesNoWork =
    BmpMenu_YesNoSelectInit(
      &_yesNoBmpDat,
      MENU_WIN_CGX_NUM, MENU_WIN_PAL,0,
      HEAPID_WIFIP2PMATCH );
  _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_SUB2);
  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   親機が何をするか選択 WIFIP2PMATCH_PLAYERDIRECT_SUB2
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _playerDirectSub2( WIFIP2PMATCH_WORK *wk, int seq )
{
  int ret = BmpMenu_YesNoSelectMain(wk->pYesNoWork);
  u8 command;

  if(ret == BMPMENU_NULL){  // まだ選択中
    return seq;
  }
  else if(ret == 0){ // はいを選択した場合
    command = WIFIP2PMATCH_PLAYERDIRECT_SUBSTART;
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_DIRECT_COMMAND,
                     1, &command);
  }
  else{  // いいえを選択した場合
    command = WIFIP2PMATCH_PLAYERDIRECT_END;
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_DIRECT_COMMAND,
                     1, &command);
  }
  _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_WAIT);
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
  status = WIFI_STATUS_PLAYING;

  switch(wk->directmode){
  case WIFIP2PMATCH_PLAYERDIRECT_VCT:
    gamemode = WIFI_GAME_VCT;
    break;
  case WIFIP2PMATCH_PLAYERDIRECT_TVT:
    gamemode = WIFI_GAME_TVT;
    break;
  case WIFIP2PMATCH_PLAYERDIRECT_TRADE:
    gamemode = WIFI_GAME_TRADE;
    break;
  }
  _myStatusChange(wk, status,gamemode);  // 接続中になる

  switch(wk->directmode){
  case WIFIP2PMATCH_PLAYERDIRECT_VCT:
    {
      GFLNetInitializeStruct* pNetInit = GFL_NET_GetNETInitStruct();
      GFL_NET_DWC_StartVChat();
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_CONNECT);
    }
    break;
  case WIFIP2PMATCH_PLAYERDIRECT_TVT:
  case WIFIP2PMATCH_PLAYERDIRECT_TRADE:
    {
      WIFI_STATUS* p_status;
      EndMessageWindowOff(wk);

      wk->endSeq = gamemode;

      GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0,16,COMM_BRIGHTNESS_SYNC);
      return SEQ_OUT;            //終了シーケンスへ
    }
    break;
  }
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
  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
  return seq;
}

