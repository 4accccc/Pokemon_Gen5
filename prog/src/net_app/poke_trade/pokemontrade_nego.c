//=============================================================================
/**
 * @file	  pokemontrade_nego.c
 * @brief	  ポケモン交換して通信を終了する ネゴシエーションのフロー
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/11/21
 */
//=============================================================================

#include <gflib.h>

#if PM_DEBUG
#include "debug/debugwin_sys.h"
#include "test/debug_pause.h"
#include "poke_tool/monsno_def.h"
#endif

#include "arc_def.h"
#include "net/network_define.h"

#include "net_app/pokemontrade.h"
#include "system/main.h"

#include "message.naix"
#include "print/printsys.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "pokeicon/pokeicon.h"


#include "gamesystem\msgspeed.h"

#include "msg/msg_poke_trade.h"
#include "msg/msg_chr.h"
#include "msg/msg_trade_head.h"
#include "ircbattle.naix"
#include "trade.naix"
#include "net_app/connect_anm.h"
#include "net/dwc_rapfriend.h"
#include "savedata/wifilist.h"

#include "system/touch_subwindow.h"

#include "item/item.h"
#include "app/app_menu_common.h"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"

#include "pokemontrade_local.h"
#include "app_menu_common.naix"

static void _pokeNickNameMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,BOOL bEgg,POKEMON_TRADE_WORK* pWork);
static void _pokeLvMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork);
static void _pokeSexMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork);


//６体のポケモン選択
static GFL_UI_TP_HITTBL _tp_data[]={
  { 3*8, 7*8, 1*8, 15*8 },
  { 9*8, 13*8, 1*8, 15*8 },
  { 15*8, 19*8, 1*8, 15*8 },
  { 3*8, 7*8, 17*8, 31*8 },
  { 9*8, 13*8, 17*8, 31*8 },
  { 15*8, 19*8, 17*8, 31*8 },
  {GFL_UI_TP_HIT_END,0,0,0},
};


void POKE_GTS_EndWork(POKEMON_TRADE_WORK* pWork)
{
  int i,j;
  for(j = 0;j < GTS_PLAYER_WORK_NUM;j++){
    for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
      if(pWork->GTSSelectPP[j][i]){
        GFL_HEAP_FreeMemory(pWork->GTSSelectPP[j][i]);
        pWork->GTSSelectPP[j][i]=NULL;
      }
    }
  }

}

void POKE_GTS_InitWork(POKEMON_TRADE_WORK* pWork)
{
  int i,j;
  for(j = 0;j < GTS_PLAYER_WORK_NUM;j++){
    for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
      pWork->GTSSelectIndex[j][i] = -1;
      if(pWork->GTSSelectPP[j][i]==NULL){
        pWork->GTSSelectPP[j][i] = GFL_HEAP_AllocClearMemory( pWork->heapID, POKETOOL_GetWorkSize() );
      }
    }
  }

}

// GTSPOKEを選択してるかどうか判断する
int POKE_GTS_IsSelect(POKEMON_TRADE_WORK* pWork,int boxno,int index)
{
  int i,j;
  
  for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
    if((pWork->GTSSelectIndex[0][i] == index) &&   (pWork->GTSSelectBoxno[0][i] == boxno)){
      return i;
    }
  }
  return -1;
}



//ダイレクトにポケモン追加 相手用 
void POKE_GTS_DirectAddPokemon(POKEMON_TRADE_WORK* pWork,int index,const POKEMON_PARAM* pp)
{
  

  pWork->GTSSelectIndex[1][index] = index;
  pWork->GTSSelectBoxno[1][index] = index;
  GFL_STD_MemCopy(pp,pWork->GTSSelectPP[1][index],POKETOOL_GetWorkSize());
}

//ポケモン追加
static int _addPokemon(POKEMON_TRADE_WORK* pWork,int side,int index,int boxno,const POKEMON_PARAM* pp)
{
  int i;
  
  for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
    if((pWork->GTSSelectIndex[side][i] == -1) || (i==GTS_NEGO_POKESLT_MAX-1)){
      OS_TPrintf("代入 %d %d\n",boxno,index);

      pWork->GTSSelectIndex[side][i] = index;
      pWork->GTSSelectBoxno[side][i] = boxno;
      GFL_STD_MemCopy(pp,pWork->GTSSelectPP[side][i],POKETOOL_GetWorkSize());
      return i;
    }
  }
  return -1;  //帰ることは無い
}





//ポケモン削除
void POKE_GTS_DeletePokemonDirect(POKEMON_TRADE_WORK* pWork,int side,int index)
{
  pWork->GTSSelectIndex[side][index] = -1;
  pWork->GTSSelectBoxno[side][index] = -1;
}



//------------------------------------------------------------------------------
/**
 * @brief   ポケモンアイコンリソースの作成
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------


void POKE_GTS_CreatePokeIconResource(POKEMON_TRADE_WORK* pWork,int mainsub)
{
  ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );
  pWork->cellRes[PLT_GTS_POKEICON] =
    GFL_CLGRP_PLTT_RegisterComp( arcHandlePoke ,
                                 POKEICON_GetPalArcIndex() , mainsub ,
                                 _OBJPLT_GTS_POKEICON_OFFSET_M , pWork->heapID  );
  pWork->cellRes[ANM_GTS_POKEICON] =
    GFL_CLGRP_CELLANIM_Register( arcHandlePoke ,
                                 POKEICON_GetCellSubArcIndex() , POKEICON_GetAnmArcIndex(), pWork->heapID  );
  
  GFL_ARC_CloseDataHandle(arcHandlePoke);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンアイコンリソースの開放
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------


void POKE_GTS_ReleasePokeIconResource(POKEMON_TRADE_WORK* pWork)
{
  
  if(pWork->cellRes[PLT_GTS_POKEICON]!=0){
    GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_GTS_POKEICON] );
    pWork->cellRes[PLT_GTS_POKEICON]=0;
  }
  if(pWork->cellRes[ANM_GTS_POKEICON]!=0){
    GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_GTS_POKEICON] );
    pWork->cellRes[ANM_GTS_POKEICON]=0;
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモン登録＋相手に情報転送
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL POKE_GTS_PokemonsetAndSendData(POKEMON_TRADE_WORK* pWork,int index,int boxno)
{ //選択ポケモン表示
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
  POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->selectBoxno, pWork->selectIndex,pWork);
  POKEMON_PARAM* pp = PP_CreateByPPP(ppp,pWork->heapID);
  int no;

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    no = _addPokemon(pWork,0,index,boxno, pp);
    if( FALSE == GFL_NET_SendDataEx(pNet, GFL_NET_SENDID_ALLUSER, _NETCMD_THREE_SELECT1+no,
                                    POKETOOL_GetWorkSize(),pWork->GTSSelectPP[0][no],
                                    FALSE, FALSE, TRUE)){
      POKE_GTS_DeletePokemonDirect(pWork,0,no);  //取り消し
      GFL_HEAP_FreeMemory(pp);
      return FALSE;
    }
  }
  else{
    no = _addPokemon(pWork,0,index,boxno, pp);
  }
  POKETRADE_2D_GTSPokemonIconSet(pWork,0,no,pp,TRUE);
#if PM_DEBUG
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    no = _addPokemon(pWork, 1, index,boxno, pp);
    POKETRADE_2D_GTSPokemonIconSet(pWork,1,no,pp,TRUE);
  }
#endif
  GFL_HEAP_FreeMemory(pp);

  if(pWork->pAppTask==NULL){
    int msg[]={POKETRADE_STR2_26};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
  }

  return TRUE;
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモンネゴシエーション ２人の説明を表示
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_StatusMessageDisp(POKEMON_TRADE_WORK* pWork)
{
  int side;
  int sidex[] = {1, 17};
  int xdotpos[]={0,128};
  int i=0,lv;
  int frame = GFL_BG_FRAME3_M;
  GFL_BMPWIN* pWin;
  MYSTATUS* aStBuf[2];

  aStBuf[0] = pWork->pMy;
  aStBuf[1] = pWork->pFriend;
    
  GFL_ARC_UTIL_TransVramPalette( APP_COMMON_GetArcId() , 
                                 NARC_app_menu_common_task_menu_NCLR ,
                                 PALTYPE_MAIN_BG , _BUTTON_MSG_PAL*32 , 32*1 , pWork->heapID );	
//  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
  //                              0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  
  if(pWork->GTSInfoWindow){
    GFL_BMPWIN_Delete(pWork->GTSInfoWindow);
  }
  pWork->GTSInfoWindow = GFL_BMPWIN_Create(frame,	0, 0, 32, 10,	_BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_F);

  pWin = pWork->GTSInfoWindow;
  GFL_FONTSYS_SetColor( 0xe, 0xf, 0x0 );
  
  GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR2_22 + pWork->GTSlv[0], pWork->pStrBuf );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), 64,  2, pWork->pStrBuf, pWork->pFontHandle);

  GFL_FONTSYS_SetColor( 4, 1, 0x0 );
  

  for(side = 0 ; side < GTS_PLAYER_WORK_NUM; side++){
    if(aStBuf[side]){
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_16, pWork->pExStrBuf );
      WORDSET_RegisterPlayerName( pWork->pWordSet, 0,  aStBuf[side]  );
      WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), sidex[side]*8, 16+8+2, pWork->pStrBuf, pWork->pFontHandle);

      GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR2_17 + pWork->GTStype[side], pWork->pStrBuf );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), sidex[side]*8, 48+8+2, pWork->pStrBuf, pWork->pFontHandle);

//      GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR2_14 + pWork->bGTSSelect[side], pWork->pStrBuf );
  //    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), sidex[side]*8, 128+8, pWork->pStrBuf, pWork->pFontHandle);

      GFL_BMPWIN_TransVramCharacter(pWin);
      GFL_BMPWIN_MakeScreen(pWin);
    }
  }
  GFL_BG_LoadScreenV_Req( frame );
}



//------------------------------------------------------------------------------
/**
 * @brief   ポケモンネゴシエーション ２人の説明を表示
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_SelectStatusMessageDisp(POKEMON_TRADE_WORK* pWork, int side, BOOL bSelected)
{
  int sidex[] = {1, 17};
  int i=0,lv;
  int frame = GFL_BG_FRAME3_M;
  GFL_BMPWIN* pWin;

    
  GFL_ARC_UTIL_TransVramPalette( APP_COMMON_GetArcId() , 
                                 NARC_app_menu_common_task_menu_NCLR ,
                                 PALTYPE_MAIN_BG , _BUTTON_MSG_PAL*32 , 32*1 , pWork->heapID );	
//  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
  //                              0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  
  if(pWork->StatusWin[side]){
    GFL_BMPWIN_Delete(pWork->StatusWin[side]);
  }
  pWork->StatusWin[side] = GFL_BMPWIN_Create(frame,	sidex[side], 15, 14, 2,	_BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_F);

  pWin = pWork->StatusWin[side];
  GFL_FONTSYS_SetColor( 4, 1, 0x0 );
  


  GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR2_14 + bSelected, pWork->pStrBuf );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), 0, 0, pWork->pStrBuf, pWork->pFontHandle);
  
  GFL_BMPWIN_TransVramCharacter(pWin);
  GFL_BMPWIN_MakeScreen(pWin);
  GFL_BG_LoadScreenV_Req( frame );
}



//------------------------------------------------------------------------------
/**
 * @brief   ポケモンネゴシエーション ２人の説明を消去
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------
void POKE_GTS_SelectStatusMessageDelete(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->GTSInfoWindow){
    GFL_BMPWIN_ClearScreen(pWork->StatusWin[0]);
    GFL_BMPWIN_ClearScreen(pWork->StatusWin[1]);
    GFL_BMPWIN_ClearScreen(pWork->GTSInfoWindow);
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
    GFL_BMPWIN_Delete(pWork->StatusWin[0]);
    GFL_BMPWIN_Delete(pWork->StatusWin[1]);
    GFL_BMPWIN_Delete(pWork->GTSInfoWindow);
    pWork->StatusWin[0]=NULL;
    pWork->StatusWin[1]=NULL;
    pWork->GTSInfoWindow=NULL;
  }
}


static void _menuFriendPokemon(POKEMON_TRADE_WORK* pWork)
{


  
}



//交換選択待ち
static void _changePokemonSendData(POKEMON_TRADE_WORK* pWork)
{
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);  //下のメッセージを消す
    pWork->pAppTask=NULL;
    switch(selectno){
    case 0:  //こうかん
      _CHANGE_STATE(pWork,POKETRE_MAIN_ChangePokemonSendDataNetwork);
      break;
    case 1:  //もどる

      if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
        pWork->userNetCommand[0]=0;
        pWork->userNetCommand[1]=0;
      }
      else{
        int myID = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
        pWork->userNetCommand[myID]=0;
        GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CANCEL_POKEMON,0,NULL);
      }
      _CHANGE_STATE(pWork, POKETRADE_NEGO_Select6keywait);
      break;
    }
  }
}



static void _networkFriendsStandbyWait2(POKEMON_TRADE_WORK* pWork)
{

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int msg[]={POKETRADE_STR_05, POKETRADE_STR_06};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  }
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);

  _CHANGE_STATE(pWork,_changePokemonSendData);


}


static void _networkFriendsStandbyWait(POKEMON_TRADE_WORK* pWork)
{
  int i;
  int targetID = 1 - GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  if(pWork->userNetCommand[targetID]==_NETCMD_LOOKATPOKE){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_20, pWork->pMessageStrBufEx );
    for(i=0;i<2;i++){
      POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, i);
      WORDSET_RegisterPokeNickName( pWork->pWordSet, i,  pp );
    }
    WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBuf, pWork->pMessageStrBufEx  );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork,_networkFriendsStandbyWait2);
  }
}





static void _networkFriendsStandbyWait_Send(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_LOOKATPOKE, 0,NULL)){
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait);// あいても選ぶまで待つ
  }
}

//交換選択通信送信

static void _friendSelectWait(POKEMON_TRADE_WORK* pWork)
{

#if PM_DEBUG
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    POKE_MAIN_Pokemonset(pWork, 1, pWork->GTSSelectPP[1][0]);
    {
      const STRCODE *name;
      POKEMON_PARAM* pp2; 

      pp2 = PP_Create(MONSNO_ONOKKUSU, 100, 123456, GFL_HEAPID_APP);
      name = MyStatus_GetMyName( pWork->pMy );
      PP_Put( pp2 , ID_PARA_oyaname_raw , (u32)name );
      PP_Put( pp2 , ID_PARA_oyasex , MyStatus_GetMySex( pWork->pMy ) );
      POKE_MAIN_Pokemonset(pWork,1,pp2); 
      GFL_STD_MemCopy(pp2,pWork->recvPoke[1],POKETOOL_GetWorkSize());
      GFL_HEAP_FreeMemory(pp2);
    }
  }
#endif
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    pWork->userNetCommand[0]=_NETCMD_LOOKATPOKE;
    pWork->userNetCommand[1]=_NETCMD_LOOKATPOKE;
  }
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait);// あいても選ぶまで待つ
  }
  else{
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait_Send);// あいても選ぶまで待つ
  }
}

// ポケモンのステータス表示待ち
static void _changePokemonStatusDispAuto(POKEMON_TRADE_WORK* pWork,int sel)
{
  POKEMON_PARAM* pp = pWork->GTSSelectPP[0][0];

  POKETRADE_MESSAGE_ChangePokemonStatusDisp(pWork,pp);
}

// ポケモンのステータス表示待ち
static void _pokemonStatusWait(POKEMON_TRADE_WORK* pWork)
{
  BOOL bReturn=FALSE;
  GFL_BG_SetVisible( GFL_BG_FRAME3_M , TRUE );

  switch(GFL_UI_TP_HitTrg(_tp_data)){
  case 0:
    if(pWork->pokemonselectno!=0){
      pWork->pokemonselectno = 0;
      _changePokemonStatusDispAuto(pWork,pWork->pokemonselectno);
    }
    break;
  case 1:
    if(pWork->pokemonselectno!=1){
      pWork->pokemonselectno = 1;
      _changePokemonStatusDispAuto(pWork,pWork->pokemonselectno);
    }
    break;
  case 2:
    bReturn=TRUE;
    break;
  default:
    break;
  }

  if(GFL_UI_KEY_GetTrg()==PAD_KEY_LEFT || GFL_UI_KEY_GetTrg()==PAD_KEY_RIGHT){
    pWork->pokemonselectno = 1 - pWork->pokemonselectno;
    PMSND_PlaySystemSE(POKETRADESE_CUR);
    _changePokemonStatusDispAuto(pWork,pWork->pokemonselectno);
  }

  TOUCHBAR_Main(pWork->pTouchWork);
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
    bReturn=TRUE;
    break;
  default:
    break;
  }

  if(bReturn){
    // 消す
    GFL_BG_SetVisible( GFL_BG_FRAME3_M , FALSE );


    POKETRADE_MESSAGE_ResetPokemonStatusMessage(pWork);
    
    GFL_BG_SetVisible( GFL_BG_FRAME0_S , FALSE );
    IRC_POKETRADE_GraphicInitMainDisp(pWork);
    IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
    G2_BlendNone();
    POKE_MAIN_Pokemonset(pWork, 0, IRC_POKEMONTRADE_GetRecvPP(pWork, 0));
    POKE_MAIN_Pokemonset(pWork, 1, IRC_POKEMONTRADE_GetRecvPP(pWork, 1));

    if(POKEMONTRADEPROC_IsTriSelect(pWork)){
      _CHANGE_STATE(pWork, POKETRADE_NEGO_Select6keywait);
    }
    else{
      _CHANGE_STATE(pWork, _networkFriendsStandbyWait2);
    }
  }
}


// ポケモンのステータス表示
static void _pokemonStatusStart(POKEMON_TRADE_WORK* pWork)
{
//  POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, pWork->pokemonselectno);
  POKEMON_PARAM* pp = pWork->GTSSelectPP[0][0];

  if(pp && PP_Get(pp,ID_PARA_poke_exist,NULL) ){
    POKETRADE_MESSAGE_CreatePokemonParamDisp(pWork,pp);
  }
  _CHANGE_STATE(pWork, _pokemonStatusWait);
}



static void _menuMyPokemon(POKEMON_TRADE_WORK* pWork)
{

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);  //下のメッセージを消す
    pWork->pAppTask=NULL;
    switch(selectno){
    case 0:  //こうかん
      _CHANGE_STATE(pWork, _friendSelectWait);//POKETRE_MAIN_ChangePokemonSendDataNetwork);
      break;
    case 1:  //つよさをみる
      _CHANGE_STATE(pWork, _pokemonStatusStart);
      break;
    case 2:  //もどる
      _CHANGE_STATE(pWork, POKETRADE_NEGO_Select6keywait);
      break;
    }
  }
}




//------------------------------------------------------------------------------
/**
 * @brief   選択ポケモンを送る
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _menuMyPokemonSend(POKEMON_TRADE_WORK* pWork)
{
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_SELECT_POKEMON,
                         POKETOOL_GetWorkSize(), IRC_POKEMONTRADE_GetRecvPP(pWork,0))){
      return;
    }
  }
  
  {    
    int msg[]={POKETRADE_STR_05, POKETRADE_STR_04, POKETRADE_STR_06};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
    _CHANGE_STATE(pWork,_menuMyPokemon);
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   6体表示まち
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKETRADE_NEGO_Select6keywait(POKEMON_TRADE_WORK* pWork)
{
  BOOL bTouch=FALSE;
  int trgno,target;
  GFL_BG_SetVisible( GFL_BG_FRAME3_M , TRUE );

  trgno = GFL_UI_TP_HitTrg(_tp_data);
  if(trgno != -1){
    POKEMON_PARAM* pp = pWork->GTSSelectPP[trgno/GTS_NEGO_POKESLT_MAX][trgno%GTS_NEGO_POKESLT_MAX];
    if(pp && PP_Get(pp,ID_PARA_poke_exist,NULL)){
      if(trgno/GTS_NEGO_POKESLT_MAX){
        int msg[]={ POKETRADE_STR_04, POKETRADE_STR_06};
        POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
        _CHANGE_STATE(pWork,_menuFriendPokemon);
      }
      else{
        POKE_MAIN_Pokemonset(pWork, trgno/GTS_NEGO_POKESLT_MAX, pp);
        GFL_STD_MemCopy(pp, IRC_POKEMONTRADE_GetRecvPP(pWork,0) ,POKETOOL_GetWorkSize());
        _CHANGE_STATE(pWork, _menuMyPokemonSend);
      }
    }
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   6体表示
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _Select6Init(POKEMON_TRADE_WORK* pWork)
{
  GFL_BMPWIN* pWin;
  int side,poke;
  MYSTATUS* aStBuf[2];

  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_POKETRADE, pWork->heapID );
	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_trade_wb_gts_bg05_NSCR,
																				 GFL_BG_FRAME0_S, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar1), 0, 0,
																				 pWork->heapID);
  GFL_BG_SetPriority( GFL_BG_FRAME0_S , 3 );
  GFL_BG_SetVisible( GFL_BG_FRAME0_S , TRUE );
  GFL_BG_SetVisible( GFL_BG_FRAME3_S , FALSE );
	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_trade_wb_trade_bg01_back_NSCR,
																				 GFL_BG_FRAME2_M, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
																				 pWork->heapID);
	GFL_ARC_CloseDataHandle( p_handle );
  POKE_GTS_ReleasePokeIconResource(pWork);
  POKE_GTS_CreatePokeIconResource(pWork, CLSYS_DRAW_SUB);

  GFL_BG_ClearScreenCodeVReq(GFL_BG_FRAME1_S,0);

  //GFL_BG_SetVisible( GFL_BG_FRAME1_S , FALSE );

  IRC_POKETRADE_ResetBoxNameWindow(pWork);


  IRC_POKETRADE_AllDeletePokeIconResource(pWork);
  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], FALSE );


  POKE_GTS_SelectStatusMessageDelete(pWork);
  
  POKETRADE_MESSAGE_SixStateDisp(pWork); 
  
  pWork->bgscroll=0;
  pWork->bgscrollRenew = TRUE;
  //GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_S );
  GFL_BG_SetVisible( GFL_BG_FRAME1_S , TRUE );

  _CHANGE_STATE(pWork,POKETRADE_NEGO_Select6keywait);


}

//------------------------------------------------------------------------------
/**
 * @brief   6体表示確認まち
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _send5Wait(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->pokemonThreeSet){   //相手から届いたかどうかいまだとずっとまつ
    POKE_GTS_SelectStatusMessageDisp(pWork, 1, TRUE);
    _CHANGE_STATE(pWork,_Select6Init);
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   6体表示OK転送
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_Select6Init(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    u8 flg = 1;
    if( GFL_NET_SendData(pNet,_NETCMD_THREE_SELECT_END,1,&flg)){
      _CHANGE_STATE(pWork, _send5Wait);
    }
  }
  else{
    _CHANGE_STATE(pWork,_Select6Init);
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモンステータスウインドウ用のパレットを合成する
 * @param   line       横順番
 * @param   height     縦順番
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL POKETRADE_NEGO_IsSelect(POKEMON_TRADE_WORK* pWork,int line , int height)
{
  int l,m;
  int boxno = IRC_TRADE_LINE2TRAY(line,pWork);
  int index = IRC_TRADE_LINE2POKEINDEX(line, height);


  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    for(l=0;l<GTS_PLAYER_WORK_NUM;l++){
      for(m=0;m<GTS_NEGO_POKESLT_MAX;m++){
        if(pWork->GTSSelectIndex[l][m] != -1){
          if((pWork->GTSSelectIndex[l][m]==index) && (pWork->GTSSelectBoxno[l][m]==boxno)){
          return TRUE;
          }
        }
      }
    }
  }
  else{
    if((pWork->selectIndex==index) && (pWork->selectBoxno==boxno)){
      if((pWork->workPokeIndex==-1) && (pWork->workBoxno==-1)){
        OS_TPrintf("selectIndex %d %d\n",boxno, index);
        return TRUE;
      }
    }
    if((pWork->selectIndex==-1) && (pWork->selectBoxno==-1)){
      if((pWork->workPokeIndex==index) && (pWork->workBoxno==boxno)){
        OS_TPrintf("workPokeIndex %d %d\n",boxno, index);
        return TRUE;
      }
    }
  }
  return FALSE;
}






void POKE_GTS_DeletePokemonState(POKEMON_TRADE_WORK* pWork)
{
  int no = POKE_GTS_IsSelect(pWork,pWork->underSelectBoxno,pWork->underSelectIndex );


  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if( !GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), _NETCMD_THREE_SELECT_CANCEL+no, sizeof(no), &no)){
      return;
    }
  }
  POKE_GTS_DeletePokemonDirect(pWork, 0, no);
  POKETRADE_2D_GTSPokemonIconReset(pWork,0, no);
  _CHANGE_STATE(pWork, POKE_TRADE_PROC_TouchStateCommon);
}


