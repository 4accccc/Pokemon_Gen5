//=============================================================================
/**
 * @file	  pokemontrade_save.c
 * @bfief	  ポケモン交換セーブ部分
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/11/08
 */
//=============================================================================

#include <gflib.h>


#include "arc_def.h"
#include "net/network_define.h"
#include "system\wipe.h"

#include "net_app/pokemontrade.h"
#include "system/main.h"

#include "poke_icon.naix"
#include "message.naix"
#include "print/printsys.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "pokeicon/pokeicon.h"

#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"

#include "demo/shinka_demo.h"
#include "poke_tool/shinka_check.h"

#include "msg/msg_poke_trade.h"
#include "ircbattle.naix"
#include "trade.naix"
#include "net_app/connect_anm.h"
#include "net/dwc_rapfriend.h"
#include "savedata/wifilist.h"

#include "system/touch_subwindow.h"

#include "poke_tool/poke_tool_def.h"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/status_rcv.h"
#include "tradedemo.naix"

#include "pokemontrade_local.h"
#include "app/mailtool.h"

#include "spahead.h"


static void _changeDemo_ModelTrade20(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade21(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade22(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade23(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade24(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade25(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade30(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade26(POKEMON_TRADE_WORK* pWork);
static void _changeTimingSaveStart(POKEMON_TRADE_WORK* pWork);
static void _changeTimingSaveStart2(POKEMON_TRADE_WORK* pWork);
static void _changeTimingSaveStart3(POKEMON_TRADE_WORK* pWork);
static void _changeTimingSaveStart4(POKEMON_TRADE_WORK* pWork);
static void _changeTimingSaveStart5(POKEMON_TRADE_WORK* pWork);
static void _saveStart(POKEMON_TRADE_WORK* pWork);
static void _mailBoxStart(POKEMON_TRADE_WORK* pWork);
static void _changeTimingSaveStart(POKEMON_TRADE_WORK* pWork);

//------------------------------------------------------------------
/**
 * @brief   次のシーンに必要な値をセット
 * @param   POKEMON_TRADE_WORK ワーク
 * @param   _TRADE_SCENE_NO_E no シーン管理enum
 * @retval  none
 */
//------------------------------------------------------------------

static void _setNextAnim(POKEMON_TRADE_WORK* pWork, int timer)
{
  pWork->anmCount = timer;
}


//------------------------------------------------------------------
/**
 * @brief   ポケモン交換終了。
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------

void POKMEONTRADE_SAVE_Init(POKEMON_TRADE_WORK* pWork)
{
  GFL_CLACT_UNIT_Delete(pWork->cellUnit);
  GFL_CLACT_SYS_Delete();
  IRC_POKETRADE_CLACT_Create(pWork);



  pWork->cellUnit = GFL_CLACT_UNIT_Create( 340 , 0 , pWork->heapID );
  POKETRADE_MESSAGE_HeapInit(pWork);

  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_49, pWork->pMessageStrBufEx );
  {
    POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, 1);
    WORDSET_RegisterPokeNickName( pWork->pWordSet, 1,  pp );
  }
  {
    WORDSET_RegisterPlayerName( pWork->pWordSet, 0, pWork->pFriend  );
  }
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBuf, pWork->pMessageStrBufEx);

  pWork->bgchar = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME2_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
  POKETRADE_MESSAGE_WindowOpen(pWork);
  _setNextAnim(pWork, 0);

  GFL_BG_SetVisible( GFL_BG_FRAME2_S , TRUE );



  _CHANGE_STATE(pWork,_changeDemo_ModelTrade21);

}

static void _changeDemo_ModelTrade21(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(pWork->anmCount < 100){
    return;
  }

  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_50, pWork->pMessageStrBufEx );
  {
    POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, 1);
    WORDSET_RegisterPokeNickName( pWork->pWordSet, 1,  pp );
  }
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBuf, pWork->pMessageStrBufEx);
  POKETRADE_MESSAGE_WindowOpen(pWork);
  _setNextAnim(pWork, 0);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade22);

}

static void _changeDemo_ModelTrade22(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(pWork->anmCount < 100){
    return;
  }
  _setNextAnim(pWork, 0);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade23);
}





static void _changeDemo_ModelTrade23(POKEMON_TRADE_WORK* pWork)
{
  IRCPOKETRADE_PokeDeleteMcss(pWork, 0);
  IRCPOKETRADE_PokeDeleteMcss(pWork, 1);
  IRCPOKETRADE_PokeDeleteMcss(pWork, 2);
  IRCPOKETRADE_PokeDeleteMcss(pWork, 3);



  if(pWork->type == POKEMONTRADE_TYPE_EVENT){
    pWork->pParentWork->ret = POKEMONTRADE_MOVE_END;
    _CHANGE_STATE(pWork,POKEMONTRADE_PROC_FadeoutStart);
    return;
  }
  {   //ここからいつでもPROCCHANGEしても良いように親ワークに交換情報を格納
    POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, 1);
    pWork->pParentWork->selectBoxno = pWork->selectBoxno;
    pWork->pParentWork->selectIndex = pWork->selectIndex;

    if(pWork->pParentWork->pParty){
      PokeParty_Init( pWork->pParentWork->pParty, TEMOTI_POKEMAX );
      PokeParty_Add( pWork->pParentWork->pParty, pp );
    }
  }

  {
    int id = 1-GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
    STATUS_RCV_PokeParam_RecoverAll(pWork->recvPoke[id]);
    //交換する
    // 相手のポケを自分の選んでいた場所に入れる
    if(pWork->selectBoxno == BOXDAT_GetTrayMax(pWork->pBox)){ //てもちの交換の場合
      POKEPARTY* party = pWork->pMyParty;
      PokeParty_SetMemberData(party, pWork->selectIndex, pWork->recvPoke[id]);
    }
    else{
      // メールがあったらボックスに
      _ITEMMARK_ICON_WORK* pIM = &pWork->aItemMark;
      int item = PP_Get( pWork->recvPoke[id] , ID_PARA_item ,NULL);
      if(ITEM_CheckMail(item)){
        GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_08, pWork->pMessageStrBuf );
        POKETRADE_MESSAGE_WindowOpen(pWork);
        _CHANGE_STATE(pWork,_mailBoxStart);
        return;
      }
      else{
        BOXDAT_PutPokemonPos(pWork->pBox, pWork->selectBoxno,
                             pWork->selectIndex, (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst(pWork->recvPoke[id]));
      }
    }
  }
  _CHANGE_STATE(pWork,_saveStart);
}



static void _saveStart(POKEMON_TRADE_WORK* pWork)
{
  if(!pWork->pGameData){
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade30); //ゲームデータの引渡しが無い場合セーブに行かない
    return;
  }
  else{
    if(pWork->pParentWork){
      SHINKA_COND cond;
      POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, 1);
    
      u16 after_mons_no = SHINKA_Check( NULL, pp, SHINKA_TYPE_TUUSHIN, 0, &cond, pWork->heapID );
      if( after_mons_no ){
        pWork->pParentWork->ret = POKEMONTRADE_MOVE_EVOLUTION;
        pWork->pParentWork->after_mons_no = after_mons_no;
        pWork->pParentWork->cond = cond;
        pWork->pParentWork->selectBoxno = pWork->selectBoxno;
        pWork->pParentWork->selectIndex = pWork->selectIndex;

        PokeParty_Init( pWork->pParentWork->pParty, TEMOTI_POKEMAX );
        PokeParty_Add( pWork->pParentWork->pParty, pp );
        _CHANGE_STATE(pWork,POKEMONTRADE_PROC_FadeoutStart);   //交換デモに行く
        return ;
      }
    }
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_51, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork, _changeTimingSaveStart);
  }
}



//-------------------------------------------------
/**
 *	@brief         進化タイミングスタート  メール後の戻り部分
 *	@param[inout]	 _POKEMCSS_MOVE_WORK ワーク
 */
//-------------------------------------------------

void POKMEONTRADE_EVOLUTION_TimingStart(POKEMON_TRADE_WORK* pWork)
{

  POKETRADE_MESSAGE_HeapInit(pWork);
  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_51, pWork->pMessageStrBuf );
  pWork->bgchar = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME2_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
  POKETRADE_MESSAGE_WindowOpen(pWork);
  _setNextAnim(pWork, 0);

  GFL_BG_SetVisible( GFL_BG_FRAME2_S , TRUE );
  GFL_NET_WirelessIconEasy_HoldLCD(TRUE,pWork->heapID); //通信アイコン
  GFL_NET_ReloadIcon();

  {
    POKEMON_PARAM* pp = PokeParty_GetMemberPointer( pWork->pParentWork->pParty, 0 );
    GF_ASSERT(MAILDATA_NULLID != MailSys_MoveMailPoke2Paso(pWork->pMailBlock, pp, pWork->heapID));
  }
  
  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, _BRIGHTNESS_SYNC);

  _CHANGE_STATE(pWork,_changeTimingSaveStart);
}



//-------------------------------------------------
/**
 *	@brief         セーブタイミングスタート  進化後の戻り部分
 *	@param[inout]	 _POKEMCSS_MOVE_WORK ワーク
 */
//-------------------------------------------------

void POKMEONTRADE_SAVE_TimingStart(POKEMON_TRADE_WORK* pWork)
{

//  pWork->cellUnit = GFL_CLACT_UNIT_Create( 340 , 0 , pWork->heapID );
  POKETRADE_MESSAGE_HeapInit(pWork);
  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_51, pWork->pMessageStrBuf );
  pWork->bgchar = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME2_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
  POKETRADE_MESSAGE_WindowOpen(pWork);
  _setNextAnim(pWork, 0);

  GFL_BG_SetVisible( GFL_BG_FRAME2_S , TRUE );
  GFL_NET_WirelessIconEasy_HoldLCD(TRUE,pWork->heapID); //通信アイコン
  GFL_NET_ReloadIcon();

  //戻ってきたポケモンと入れ替え
  pWork->selectBoxno = pWork->pParentWork->selectBoxno;
  pWork->selectIndex = pWork->pParentWork->selectIndex;
  {
    POKEMON_PARAM* pp=PokeParty_GetMemberPointer( pWork->pParentWork->pParty, 0 );
    
    // 相手のポケを自分の選んでいた場所に入れる
    if(pWork->selectBoxno == BOXDAT_GetTrayMax(pWork->pBox)){ //てもちの交換の場合
      POKEPARTY* party = pWork->pMyParty;
      PokeParty_SetMemberData(party, pWork->selectIndex, pp);
    }
    else{
      BOXDAT_PutPokemonPos(pWork->pBox, pWork->selectBoxno,
                           pWork->selectIndex, (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst(pp));
    }
  }

  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, _BRIGHTNESS_SYNC);

  _CHANGE_STATE(pWork,_changeTimingSaveStart);
}

static void _changeTimingSaveStart(POKEMON_TRADE_WORK* pWork)
{  
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVEST,WB_NET_TRADE_SERVICEID);
    _CHANGE_STATE(pWork, _changeTimingSaveStart2);
  }
  else{
  //  GAMEDATA_SaveAsyncStart(pWork->pGameData);
    _CHANGE_STATE(pWork, _changeDemo_ModelTrade30);
  }

}
static void _changeTimingSaveStart2(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVEST,WB_NET_TRADE_SERVICEID)){
    GAMEDATA_SaveAsyncStart(pWork->pGameData);
    _CHANGE_STATE(pWork,_changeTimingSaveStart3);
  }
}

static void _changeTimingSaveStart3(POKEMON_TRADE_WORK* pWork)
{
  if(GAMEDATA_SaveAsyncMain(pWork->pGameData) == SAVE_RESULT_LAST){
    if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
      pWork->saveStep = GFUser_GetPublicRand(_CHANGERAND);
      _CHANGE_STATE(pWork,_changeTimingSaveStart4);
    }
    else{
      _CHANGE_STATE(pWork,_changeDemo_ModelTrade25);
    }
  }
}

static void _changeTimingSaveStart4(POKEMON_TRADE_WORK* pWork)
{
  pWork->saveStep--;
  if(pWork->saveStep<0){
    if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVELAST,WB_NET_TRADE_SERVICEID);
      _CHANGE_STATE(pWork, _changeTimingSaveStart5);
    }
    else{
      _CHANGE_STATE(pWork,_changeDemo_ModelTrade30);
    }
  }
}

static void _changeTimingSaveStart5(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVELAST,WB_NET_TRADE_SERVICEID)){
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade25);
  }
}

static void _changeDemo_ModelTrade25(POKEMON_TRADE_WORK* pWork)
{
  if(GAMEDATA_SaveAsyncMain(pWork->pGameData) == SAVE_RESULT_OK){
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVEEND, WB_NET_TRADE_SERVICEID);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade26);
  }
}

static void _changeDemo_ModelTrade26(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVEEND, WB_NET_TRADE_SERVICEID)){
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade30);
  }
}

static void _changeDemo_ModelTrade30(POKEMON_TRADE_WORK* pWork)
{
  GFL_DISP_GX_SetVisibleControlDirect( 0 );
  GFL_DISP_GXS_SetVisibleControlDirect( 0 );


  GFL_CLACT_UNIT_Delete(pWork->cellUnit);
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 340 , 0 , pWork->heapID );

  POKETRADE_MESSAGE_WindowClose(pWork);

  IRC_POKETRADE_ResetSubDispGraphic(pWork);
  IRC_POKETRADE_ResetSubdispGraphicDemo(pWork);

  IRC_POKETRADE_SetBgMode(SETUP_TRADE_BG_MODE_NORMAL);

  IRC_POKETRADE_SetSubVram(pWork);

  IRC_POKETRADE_InitBoxCursor(pWork);  // タスクバー
  IRC_POKETRADE_CreatePokeIconResource(pWork);  // ポケモンアイコンCLACT+リソース常駐化

  IRC_POKETRADE_GraphicInitMainDisp(pWork);
  IRC_POKETRADE_GraphicInitSubDisp(pWork);

  IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);

  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  POKETRADE_TOUCHBAR_Init(pWork);
  GFL_NET_ReloadIcon();
  _CHANGE_STATE(pWork,IRC_POKMEONTRADE_ChangeFinish);

}




//-------------------------------------------------
/**
 *	@brief      メールをパソコンに入れた > セーブに向かう
 *	@param[inout]	POKEMON_TRADE_WORK ワーク
 */
//-------------------------------------------------

static void _mailSeqEnd(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    //ポケモンを格納
    int id = 1-GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
    BOXDAT_PutPokemonPos(pWork->pBox, pWork->selectBoxno,
                         pWork->selectIndex, (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst(pWork->recvPoke[id]));
    _CHANGE_STATE(pWork,_saveStart);  //セーブ処理に行く
  }
}


//-------------------------------------------------
/**
 *	@brief    メールを捨てる確認
 *	@param[inout]	_POKEMCSS_MOVE_WORK ワーク
 */
//-------------------------------------------------

static void _mailTrashYesOrNo(POKEMON_TRADE_WORK* pWork)
{
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);
    pWork->pAppTask=NULL;

    switch(selectno){
    case 0:  //はい
      {
        int id = 1-GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
        PP_Put(pWork->recvPoke[id], ID_PARA_item, ITEM_DUMMY_ID);
      }
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_11, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      _CHANGE_STATE(pWork,_mailSeqEnd);
      break;
    default: //いいえ
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_08, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      _CHANGE_STATE(pWork,_mailBoxStart);
      break;
    }
  }
}


//-------------------------------------------------
/**
 *	@brief    メールを捨てる確認
 *	@param[inout]	_POKEMCSS_MOVE_WORK ワーク
 */
//-------------------------------------------------

static void _mailTrash(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int msg[]={POKETRADE_STR_27, POKETRADE_STR_28};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
  }
  _CHANGE_STATE(pWork,_mailTrashYesOrNo);
}


//-------------------------------------------------
/**
 *	@brief    パソコンの整理確認
 *	@param[inout]	_POKEMCSS_MOVE_WORK ワーク
 */
//-------------------------------------------------

static void _mailPCArrangementYesOrNo(POKEMON_TRADE_WORK* pWork)
{
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);
    pWork->pAppTask=NULL;

    switch(selectno){
    case 0:  //はい
      pWork->pParentWork->ret = POKEMONTRADE_MOVE_MAIL;
      _CHANGE_STATE(pWork,POKEMONTRADE_PROC_FadeoutStart);   //交換デモに行く
      break;
    default: //いいえ
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_09, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      _CHANGE_STATE(pWork,_mailTrash);
      break;
    }
  }
}



//-------------------------------------------------
/**
 *	@brief    パソコンの整理確認
 *	@param[inout]	_POKEMCSS_MOVE_WORK ワーク
 */
//-------------------------------------------------

static void _mailPCArrangement(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int msg[]={POKETRADE_STR_27, POKETRADE_STR_28};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
  }
  _CHANGE_STATE(pWork,_mailPCArrangementYesOrNo);
}

//-------------------------------------------------
/**
 *	@brief    パソコンにおくるかどうか パソコンがいっぱいならさらに分岐
 *	@param[inout]	_POKEMCSS_MOVE_WORK ワーク
 */
//-------------------------------------------------

static void _mailPCSendYesOrNo(POKEMON_TRADE_WORK* pWork)
{
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);
    pWork->pAppTask=NULL;

    switch(selectno){
    case 0:  //はい
      {
        int friendID = 1-GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
        int ret = MailSys_MoveMailPoke2Paso(pWork->pMailBlock, pWork->recvPoke[friendID],pWork->heapID);
#if DEBUG_ONLY_FOR_ohno
        if(1){ //入らなかった
#else
        if(MAILDATA_NULLID==ret){ //入らなかった
#endif
          GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_10, pWork->pMessageStrBuf );
          POKETRADE_MESSAGE_WindowOpen(pWork);
          _CHANGE_STATE(pWork,_mailPCArrangement);
        }
        else{
          GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_12, pWork->pMessageStrBuf );
          POKETRADE_MESSAGE_WindowOpen(pWork);
          _CHANGE_STATE(pWork,_mailSeqEnd);
        }
      }
      break;
    default: //いいえ
      //@todo
      break;
    }
  }
}

//-------------------------------------------------
/**
 *	@brief      メールの開始
 *	@param[inout]	POKEMON_TRADE_WORK ワーク
 */
//-------------------------------------------------

static void _mailBoxStart(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int msg[]={POKETRADE_STR_27, POKETRADE_STR_28};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
  }
  _CHANGE_STATE(pWork,_mailPCSendYesOrNo);

}





