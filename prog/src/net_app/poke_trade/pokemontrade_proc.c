//=============================================================================
/**
 * @file    pokemontrade_proc.c
 * @brief   ポケモン交換して通信を終了する
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date    09/07/09
 */
//=============================================================================

#include <gflib.h>
#include "net/network_define.h"
#include "arc_def.h"

#if PM_DEBUG
#include "debug/debugwin_sys.h"
#include "test/debug_pause.h"
#include "poke_tool/monsno_def.h"
#endif
#include "pokeicon/pokeicon.h"

#include "savedata/wifilist.h"

#include "message.naix"
#include "print/printsys.h"
#include "print/global_font.h"
#include "print/str_tool.h"
#include "font/font.naix"

#include "system/main.h"
#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"
#include "system/touch_subwindow.h"
#include "system/wipe.h"

#include "msg/msg_poke_trade.h"
#include "msg/msg_chr.h"
#include "msg/msg_trade_head.h"

#include "ircbattle.naix"
#include "trade.naix"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"


#include "net_app/pokemontrade.h"
#include "net_app/connect_anm.h"
#include "net_app/gtsnego.h"
#include "net/dwc_rapfriend.h"

#include "item/item.h"

#include "app/app_menu_common.h"

#include "pokemontrade_local.h"

#define HEAPSIZE_POKETRADE (0xd2000)   //全共通 WiFiだとほぼマックス

//#define _ENDTABLE  {192-32, 192, 256-32, 256}     //終了
//#define _SEARCHTABLE  {192-32, 192, 0, 32}        //検索ボタン

#include "pokemontrade.cdat"




static void _recvSelectPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvChangePokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvChangeCancel(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvEnd(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);

static void _touchState(POKEMON_TRADE_WORK* pWork);
static void _touchStateGTS(POKEMON_TRADE_WORK* pWork);
static u8* _setChangePokemonBuffer(int netID, void* pWork, int size);
static void _changeWaitState(POKEMON_TRADE_WORK* pWork);
static void _endWaitState(POKEMON_TRADE_WORK* pWork);
static BOOL _PokemonsetAndSendData(POKEMON_TRADE_WORK* pWork);
static void _recvFriendScrollBar(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _changeFinish(POKEMON_TRADE_WORK* pWork);
static void _networkFriendsStandbyWait2(POKEMON_TRADE_WORK* pWork);

static void _endCancelState(POKEMON_TRADE_WORK* pWork);
static void _recvLookAtPoke(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvEggAndBattle(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _cancelPokemonSendDataNetwork(POKEMON_TRADE_WORK* pWork);
static void _scrollMainFunc(POKEMON_TRADE_WORK* pWork,BOOL bSE, BOOL bNetSend);
static void _recvCancelPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _PokemonReset(POKEMON_TRADE_WORK *pWork, int side );
static void _changeMenuOpen(POKEMON_TRADE_WORK* pWork);
static BOOL IsTouchCLACTPosition(POKEMON_TRADE_WORK* pWork, BOOL bCatch);
static int _boxScrollLine2Num(int line);

static u8* _setThreePokemon(int netID, void* pWk, int size);
static void _recvThreePokemon1(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvThreePokemon2(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvThreePokemon3(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvThreePokemonEnd(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvThreePokemonCancel(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _scrollResetAndIconReset(POKEMON_TRADE_WORK* pWork);


///通信コマンドテーブル
static const NetRecvFuncTable _PacketTbl[] = {
  {_recvSelectPokemon,   _setChangePokemonBuffer},    ///_NETCMD_SELECT_POKEMON 2 ポケモン見せ合う
  {_recvChangePokemon,   NULL},    ///_NETCMD_CHANGE_POKEMON   3 こうかんけってい
  {_recvEggAndBattle, NULL },     ///_NETCMD_EGG_AND_BATTLE  3 たまごと最後のバトルポケモン
  {_recvLookAtPoke, NULL},       ///_NETCMD_LOOKATPOKE   2 ポケモン見せ合う
  {_recvChangeCancel,   NULL},    ///_NETCMD_CHANGE_CANCEL  3 最後にキャンセルする
  {_recvEnd, NULL},         //_NETCMD_END           1 おわり
  {_recvCancelPokemon, NULL},         //  _NETCMD_CANCEL_POKEMON
  {_recvThreePokemon1,   _setThreePokemon},    ///_NETCMD_THREE_SELECT1 ポケモン３匹みせあい
  {_recvThreePokemon2,   _setThreePokemon},    ///_NETCMD_THREE_SELECT2 ポケモン３匹みせあい
  {_recvThreePokemon3,   _setThreePokemon},    ///_NETCMD_THREE_SELECT3 ポケモン３匹みせあい
  {_recvThreePokemonEnd,   NULL},    ///_NETCMD_THREE_SELECT_END ポケモン３匹みせあい
  {_recvThreePokemonCancel,   NULL},    ///_NETCMD_THREE_SELECT_CANCEL キャンセル
  {_recvFriendScrollBar, NULL}, //_NETCMD_SCROLLBAR

};

//------------------------------------------------------------------
/**
 * @brief   通信モードで動いているかどうか
 * @param   POKEMON_TRADE_WORK
 * @retval  TRUEなら通信モード
 */
//------------------------------------------------------------------

BOOL POKEMONTRADEPROC_IsNetworkMode(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->pParentWork==NULL){
    return FALSE;
  }
  if(pWork->type==POKEMONTRADE_TYPE_EVENT){
    return FALSE;
  }
  if(GFL_NET_IsInit()){
    return TRUE;
  }
  return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief   ３体見せ合いモードかどうか
 * @param   POKEMON_TRADE_WORK
 * @retval  TRUEなら通信モード
 */
//------------------------------------------------------------------

BOOL POKEMONTRADEPROC_IsTriSelect(POKEMON_TRADE_WORK* pWork)
{
  BOOL ret = FALSE;
  
  switch(pWork->type){
  case POKEMONTRADE_TYPE_IRC:   ///< 赤外線
  case POKEMONTRADE_TYPE_EVENT:  ///< イベント用
    break;
//  case POKEMONTRADE_TYPE_WIFICLUB: ///< WIFIクラブ
//  case POKEMONTRADE_TYPE_UNION: ///< ユニオン
//  case POKEMONTRADE_TYPE_GTSNEGO: ///< GTSネゴシエーション
//  case POKEMONTRADE_TYPE_GTS: ///< GTS
  default:
    ret = TRUE;
    break;
  }
  return ret;
}


int IRC_TRADE_LINE2TRAY(int lineno,POKEMON_TRADE_WORK* pWork)
{
  if(lineno >= HAND_HORIZONTAL_NUM){
    return (lineno - HAND_HORIZONTAL_NUM) / BOX_HORIZONTAL_NUM;
  }
  return pWork->BOX_TRAY_MAX;
}

int IRC_TRADE_LINE2POKEINDEX(int lineno,int verticalindex)
{
  if(lineno >= HAND_HORIZONTAL_NUM){
    return ((lineno - HAND_HORIZONTAL_NUM) % BOX_HORIZONTAL_NUM) + (BOX_HORIZONTAL_NUM * verticalindex);
  }
  if(verticalindex < 3){
    return lineno + verticalindex * HAND_HORIZONTAL_NUM;
  }
  return -1;
}


//------------------------------------------------------------------
/**
 * @brief   ポケモンアイコンを再描画
 * @param   POKEMON_TRADE_WORK
 * @retval  void
 */
//------------------------------------------------------------------

static void _PokemonIconRenew(POKEMON_TRADE_WORK *pWork)
{
  pWork->oldLine++;
  pWork->bgscrollRenew = TRUE;
  _scrollMainFunc(pWork,FALSE,FALSE);
}

//------------------------------------------------------------------
/**
 * @brief   ベクトル計算
 * @param   POKEMON_TRADE_WORK
 * @retval  void
 */
//------------------------------------------------------------------

static void _vectorUpMath(POKEMON_TRADE_WORK *pWork)
{
  u32 x,y;

  {
    if(GFL_UI_TP_GetPointCont(&x,&y)){   //ベクトルを監視  // 上向き判定
      if(!pWork->pCatchCLWK){  //ペンにポケモンがくっついている時
        pWork->bUpVec = FALSE;
      }
      if(pWork->pCatchCLWK){  //ペンにポケモンがくっついている時
        if(pWork->y > (y+2)){   // 上に移動した場合跳ね上げる処理
//          if(pWork->aPanWork.bAreaOver){
//            pWork->bUpVec = TRUE;
//          }
        }
        else{
//          pWork->bUpVec = FALSE;
        }
        if(GFL_STD_Abs(pWork->aCatchOldPos.x - pWork->aDifferencePos.x - x) > 2){
//          NET_PRINT("POSNOX %d %d %d\n",pWork->aCatchOldPos.x , pWork->aDifferencePos.x , x);
          
          pWork->bUpVec = TRUE;
          //pWork->aPanWork.bAreaOver = TRUE;
        }
        if(GFL_STD_Abs(pWork->aCatchOldPos.y - pWork->aDifferencePos.y - y) > 2){
  //        NET_PRINT("POSNOY %d %d %d \n",pWork->aCatchOldPos.y , pWork->aDifferencePos.y , y);
          pWork->bUpVec = TRUE;
          //pWork->aPanWork.bAreaOver = TRUE;
        }
        if(GFL_STD_Abs(pWork->aCatchOldPos.x - x) > 6  && GFL_STD_Abs(pWork->aCatchOldPos.y - y) > 6){
          pWork->bStatusDisp = FALSE;
        }

      }
      if((y >=  2*8) && ((18*8) > y)){
        pWork->x = x;
        pWork->y = y;
      }
    }
  }


}


//------------------------------------------------------------------
/**
 * @brief   つかんだポケモンをうごかす
 * @param   POKEMON_TRADE_WORK
 * @retval  void
 */
//------------------------------------------------------------------

static void _CatchPokemonMoveFunc(POKEMON_TRADE_WORK *pWork)
{
    // ついてくる
  if(pWork->pCatchCLWK != NULL){  //タッチパネル操作中のアイコン移動
    u32 x,y;
    GFL_CLACTPOS pos;
    if(GFL_UI_TP_GetPointCont(&x,&y)){
      GFL_CLACT_WK_GetPos( pWork->pCatchCLWK, &pWork->aVecPos, CLSYS_DRAW_SUB);
      pos.x = x + pWork->aDifferencePos.x;
      pos.y = y + pWork->aDifferencePos.y;
      if(pos.y>150){
        pos.y=150;
      }
      GFL_CLACT_WK_SetPos( pWork->pCatchCLWK, &pos, CLSYS_DRAW_SUB);
    }
  }
}


//------------------------------------------------------------------
/**
 * @brief   アイコンを元に戻す状態にする
 * @param   POKEMON_TRADE_WORK
 * @retval  void
 */
//------------------------------------------------------------------

static void _CatchPokemonRelease(POKEMON_TRADE_WORK *pWork)
{
  if(pWork->pCatchCLWK){
    pWork->pCatchCLWK=NULL;
  }
  POKEMONTRADE_RemovePokemonCursor(pWork);
}

//------------------------------------------------------------------
/**
 * @brief   つかんだポケモンを元の位置に戻す
 * @param   POKEMON_TRADE_WORK
 * @retval  void
 */
//------------------------------------------------------------------

static void _CatchPokemonPositionRewind(POKEMON_TRADE_WORK *pWork)
{
  if(pWork->pCatchCLWK){
    GFL_CLACT_WK_SetSoftPri(pWork->pCatchCLWK,_CLACT_SOFTPRI_POKELIST);
    GFL_CLACT_WK_SetPos(pWork->pCatchCLWK, &pWork->aCatchOldPos, CLSYS_DRAW_SUB);
    GFL_CLACT_WK_SetDrawEnable( pWork->pCatchCLWK, TRUE);
    
    pWork->workPokeIndex = -1;
    pWork->workBoxno = -1;

    _CatchPokemonRelease(pWork);
    GFL_STD_MemClear(&pWork->aPanWork,sizeof(PENMOVE_WORK));
  }
}

//------------------------------------------------------------------
/**
 * @brief   つかみじょうたいにする
 * @param   POKEMON_TRADE_WORK
 * @retval  void
 */
//------------------------------------------------------------------

static void _CatchPokemonPositionActive(POKEMON_TRADE_WORK *pWork,GFL_CLWK* pCL,int line ,int index,POKEMON_PASO_PARAM*ppp)
{
//  int pltNum;


  NET_PRINT("つかみその２\n");
  pWork->pCatchCLWK = pCL;
  pWork->pSelectCLWK = pCL;
  GFL_CLACT_WK_GetPos(pWork->pCatchCLWK, &pWork->aCatchOldPos, CLSYS_DRAW_SUB);

 // pltNum = POKEICON_GetPalNumGetByPPP( ppp );
  POKMEONTRADE2D_IconGray(pWork, pWork->pCatchCLWK, TRUE);
  //GFL_CLACT_WK_SetPlttOffs( pWork->pCatchCLWK , pltNum , CLWK_PLTTOFFS_MODE_PLTT_TOP );


  //  GFL_CLACT_WK_SetSoftPri(pWork->pCatchCLWK,_CLACT_SOFTPRI_CATCHPOKE);
//  GFL_CLACT_WK_SetBgPri(pWork->pCatchCLWK,0);

  {
    u32 x,y;
    GFL_CLACTPOS pos;
    if(GFL_UI_TP_GetPointCont(&x,&y)){
      pWork->padMode=FALSE;

      pWork->aDifferencePos.x = pWork->aCatchOldPos.x - x;
      pWork->aDifferencePos.y = pWork->aCatchOldPos.y - y;
      
      NET_PRINT("POSX %d %d %d\n",x,pWork->aDifferencePos.x,pWork->aCatchOldPos.x);
      NET_PRINT("POSY %d %d %d\n",y,pWork->aDifferencePos.y,pWork->aCatchOldPos.y);
      pos.x = x + pWork->aDifferencePos.x;
      pos.y = y + pWork->aDifferencePos.y;

      POKEMONTRADE_StartCatched( pWork, line, index, x + pWork->aDifferencePos.x, y + pWork->aDifferencePos.y,
                                 ppp);

      pWork->pCatchCLWK = pWork->curIcon[CELL_CUR_POKE_KEY];
      pWork->pSelectCLWK = pWork->curIcon[CELL_CUR_POKE_KEY];
      
    }
  }

}


//------------------------------------------------------------------------------
/**
 * @brief   下画面ポケモン位置を計算
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

#if 1

static void _getPokeIconPos(int index, GFL_CLACTPOS* pos)
{
  static const u8 iconSize = 24;
  static const u8 iconTop = 72-24;
  static const u8 iconLeft = 72-48;

  pos->x = (index % 6) * iconSize + iconLeft;
  pos->y = (index / 6) * iconSize + iconTop;
}


POKEMON_PASO_PARAM* IRCPOKEMONTRADE_GetPokeDataAddress(BOX_MANAGER* boxData , int trayNo, int index,POKEMON_TRADE_WORK* pWork)
{
  POKEMON_PASO_PARAM* ppp;

  if(trayNo!=-1){
    if(trayNo != pWork->BOX_TRAY_MAX && (index != -1)){
      return BOXDAT_GetPokeDataAddress(boxData,trayNo,index);
    }
    else{
      POKEPARTY* party = pWork->pMyParty;
      if(index < PokeParty_GetPokeCount(party) && (index != -1)){
        return (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst( PokeParty_GetMemberPointer( party , index ) );
      }
    }
  }
  return NULL;
}

#endif

//------------------------------------------------------------------------------
/**
 * @brief   コマンド保存バッファをクリア
 * @param   POKEMON_TRADE_WORKポインタ
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _userNetCommandClear(POKEMON_TRADE_WORK* pWork)
{
  GFL_STD_MemClear(pWork->userNetCommand,sizeof(pWork->userNetCommand));
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void IRC_POKMEONTRADE_changeState(POKEMON_TRADE_WORK* pWork,StateFunc state)
{
  pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#if _TRADE_DEBUG
void IRC_POKMEONTRADE_changeStateDebug(POKEMON_TRADE_WORK* pWork,StateFunc state, int line)
{
  OS_TPrintf("trade: %d\n",line);
  IRC_POKMEONTRADE_changeState(pWork, state);
}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンの受信バッファを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
u8* _setChangePokemonBuffer(int netID, void* pWk, int size)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  if((netID >= 0) && (netID < 2)){
    return (u8*)pWork->recvPoke[netID];
  }
  return NULL;
}


//------------------------------------------------------------------------------
/**
 * @brief   ３体格納ポケモンの受信バッファを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static u8* _setThreePokemon(int netID, void* pWk, int size)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  if((netID >= 0) && (netID < 2)){
    return (u8*)pWork->TempBuffer[netID];
  }
  return NULL;
}






//------------------------------------------------------------------------------
/**
 * @brief   自分と相手でindex指定でバッファを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
POKEMON_PARAM* IRC_POKEMONTRADE_GetRecvPP(POKEMON_TRADE_WORK *pWork, int index)
{
  BOOL bMode = POKEMONTRADEPROC_IsNetworkMode(pWork);
  GF_ASSERT(index < 2);

  if(bMode==FALSE){
    return (POKEMON_PARAM*)pWork->recvPoke[index];
  }
  if(0 == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return (POKEMON_PARAM*)pWork->recvPoke[index];
  }
  return (POKEMON_PARAM*)pWork->recvPoke[1-index];
}




//_NETCMD_SELECT_POKEMON
static void _recvSelectPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }

  //ポケモンセットをコール  処理が重いためここではやらない
  pWork->pokemonsetCall = netID+1;

}


static void _recvThreePokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle,int num)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }
  GF_ASSERT(netID==0 || netID==1);
  
  if(PP_Get( pWork->TempBuffer[netID], ID_PARA_poke_exist, NULL  )){
    POKE_GTS_DirectAddPokemon(pWork, num, pWork->TempBuffer[netID]);
    POKETRADE_2D_GTSPokemonIconSet(pWork, 1, num, pWork->TempBuffer[netID],TRUE);
  }
  else{
    GF_ASSERT(0);
  }
}


//_NETCMD_THREE_SELECT1
static void _recvThreePokemon1(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  _recvThreePokemon(netID, size, pData, pWk, pNetHandle, 0);
}

//_NETCMD_THREE_SELECT1
static void _recvThreePokemon2(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  _recvThreePokemon(netID, size, pData, pWk, pNetHandle, 1);
}

//_NETCMD_THREE_SELECT1
static void _recvThreePokemon3(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  _recvThreePokemon(netID, size, pData, pWk, pNetHandle, 2);
}




//_NETCMD_THREE_SELECT_END
static void _recvThreePokemonEnd(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  const u8* pRecvData = pData;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }
  pWork->pokemonThreeSet = pRecvData[0];
}


//_NETCMD_THREE_SELECT_CANCEL
static void _recvThreePokemonCancel(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  const int* pNo=pData;
  

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }

  POKE_GTS_DeletePokemonDirect(pWork, 1, pNo[0]);
  POKETRADE_2D_GTSPokemonIconReset(pWork,1, pNo[0]);
}





//------------------------------------------------------------------------------
/**
 * @brief   ポケモンのキャンセルが送られてきた
 *          _NETCMD_CANCEL_POKEMON
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _recvCancelPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは受け取らない
  }
  _PokemonReset(pWork,1);
  pWork->userNetCommand[netID] = 0;

}

//------------------------------------------------------------------------------
/**
 * @brief   相手のスクロールバーの移動位置が送られてきた
 *          _NETCMD_SCROLLBAR
 * @retval  none
 */
//------------------------------------------------------------------------------

//
static void _recvFriendScrollBar(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  short* pRecvData = (short*)pData;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_SystemGetCurrentID()){
    return; //自分のデータは要らない
  }
  pWork->FriendBoxScrollNum = pRecvData[0];
}

//_NETCMD_LOOKATPOKE
static void _recvLookAtPoke(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  pWork->userNetCommand[netID] = _NETCMD_LOOKATPOKE;
}

//_NETCMD_EGG_AND_BATTLE
static void _recvEggAndBattle(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  pWork->userNetCommand[netID] = _NETCMD_EGG_AND_BATTLE;
}

//_NETCMD_CHANGE_CANCEL
static void _recvChangeCancel(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  pWork->userNetCommand[netID] = _NETCMD_CHANGE_CANCEL;
}

//_NETCMD_END
static void _recvEnd(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }

  pWork->userNetCommand[netID] = _NETCMD_END;
}


// _NETCMD_CHANGE_POKEMON
static void _recvChangePokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  pWork->userNetCommand[netID] = _NETCMD_CHANGE_POKEMON;

}

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeout(POKEMON_TRADE_WORK* pWork)
{
  if(WIPE_SYS_EndCheck()){
    _CHANGE_STATE(pWork, NULL);        // 終わり
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKEMONTRADE_PROC_FadeoutStart(POKEMON_TRADE_WORK* pWork)
{
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  _CHANGE_STATE(pWork, _modeFadeout);        // 終わり
}


//----------------------------------------------------------------------------
/**
 *  @brief  終了同期を取ったのでプロセス終了
 *  @param  POKEMON_TRADE_WORK    ワーク
 */
//-----------------------------------------------------------------------------

static void _sendTimingCheck(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ENDNO,WB_NET_TRADE_SERVICEID)){
    pWork->pParentWork->ret = POKEMONTRADE_MOVE_END;
    _CHANGE_STATE(pWork,NULL);
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  終了同期を取る
 *  @param  POKEMON_TRADE_WORK    ワーク
 */
//-----------------------------------------------------------------------------

static void _sendTiming(POKEMON_TRADE_WORK* pWork)
{
  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ENDNO,WB_NET_TRADE_SERVICEID);
  _CHANGE_STATE(pWork,_sendTimingCheck);
}

static void _noneState(POKEMON_TRADE_WORK* pWork)
{
  //なにもしない
}




//--------------------------------------------------------------
//  ポケモン表示セット
//--------------------------------------------------------------
void POKE_MAIN_Pokemonset(POKEMON_TRADE_WORK *pWork, int side, POKEMON_PARAM* pp )
{
  IRCPOKETRADE_PokeDeleteMcss(pWork, side);
  IRCPOKETRADE_PokeCreateMcss(pWork, side, 1-side, pp);
  POKETRADE_MESSAGE_SetPokemonStatusMessage(pWork,side ,pp);
}


//--------------------------------------------------------------
//  ポケモン表示リセット
//--------------------------------------------------------------
static void _PokemonReset(POKEMON_TRADE_WORK *pWork, int side )
{
  if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
    IRCPOKETRADE_PokeDeleteMcss(pWork, side);
    IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork, side);
  }
  else{
    int i;
    for(i=0;i<GTS_NEGO_POKESLT_MAX;i++){
      POKETRADE_2D_GTSPokemonIconReset(pWork,side,i);
    }
  }
}





static BOOL _IsEggAndLastBattlePokemonChange(POKEMON_TRADE_WORK* pWork)
{
  POKEMON_PARAM* pp1 = IRC_POKEMONTRADE_GetRecvPP(pWork, 0); //自分のポケモン
  POKEMON_PARAM* pp2 = IRC_POKEMONTRADE_GetRecvPP(pWork, 1); //相手のポケモン
  BOOL bEgg1 = PP_Get(pp1,ID_PARA_tamago_flag,NULL);
  BOOL bEgg2 = PP_Get(pp2,ID_PARA_tamago_flag,NULL);

  if(1==PokeParty_GetBattlePokeNum(pWork->pMyParty) && !bEgg1 && bEgg2 && (pWork->selectBoxno == pWork->BOX_TRAY_MAX)){
    return TRUE;
  }
  return FALSE;
}


//交換選択通信送信
void POKETRE_MAIN_ChangePokemonSendDataNetwork(POKEMON_TRADE_WORK* pWork)
{
  BOOL bSend = FALSE;
  BOOL bMode = POKEMONTRADEPROC_IsNetworkMode(pWork);

  if( bMode ){
    if(_IsEggAndLastBattlePokemonChange(pWork)){
      bSend=GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_EGG_AND_BATTLE,0,NULL);
    }
    else{
      bSend=GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGE_POKEMON,0,NULL);
    }
  }
  else{
    pWork->userNetCommand[0]=_NETCMD_CHANGE_POKEMON;
    pWork->userNetCommand[1]=_NETCMD_CHANGE_POKEMON;
    bSend = TRUE;
  }
  if( bSend ){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_09, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork,_changeWaitState);
  }
}


static void _changePokemonStatusDispAuto(POKEMON_TRADE_WORK* pWork,int sel)
{
  if(sel == 0){
    GFL_CLACTPOS pos = {_POKEMON_SELECT1_CELLX,_POKEMON_SELECT1_CELLY+4};
    GFL_CLACT_WK_SetPos( pWork->curIcon[CELL_CUR_POKE_SELECT], &pos, CLSYS_DRAW_SUB );
  }
  else{
    GFL_CLACTPOS pos = {_POKEMON_SELECT2_CELLX,_POKEMON_SELECT2_CELLY+4};
    GFL_CLACT_WK_SetPos( pWork->curIcon[CELL_CUR_POKE_SELECT], &pos, CLSYS_DRAW_SUB );
  }
  {
    POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, pWork->pokemonselectno);
    POKETRADE_MESSAGE_ChangePokemonStatusDisp(pWork,pp);
  }
}



// ポケモンのステータス表示待ち
static void _pokemonStatusWait(POKEMON_TRADE_WORK* pWork)
{
  BOOL bReturn=FALSE;
  GFL_UI_TP_HITTBL tp_data[]={
    {_POKEMON_SELECT1_CELLY-8,_POKEMON_SELECT1_CELLY+16,_POKEMON_SELECT1_CELLX-8, _POKEMON_SELECT1_CELLX+16},
    {_POKEMON_SELECT2_CELLY-8,_POKEMON_SELECT2_CELLY+16,_POKEMON_SELECT2_CELLX-8, _POKEMON_SELECT2_CELLX+16},
    {GFL_UI_TP_HIT_END,0,0,0},
  };
  GFL_BG_SetVisible( GFL_BG_FRAME3_M , TRUE );

  switch(GFL_UI_TP_HitTrg(tp_data)){
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
  POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, pWork->pokemonselectno);
  
  POKETRADE_MESSAGE_CreatePokemonParamDisp(pWork,pp);
  _CHANGE_STATE(pWork, _pokemonStatusWait);

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
    case 1:  //つよさをみる
      _CHANGE_STATE(pWork, _pokemonStatusStart);
      break;
    case 2:  //もどる
      _CHANGE_STATE(pWork, _cancelPokemonSendDataNetwork);
      break;
    }
  }
}

//こうかんにだす　通信相手の準備待ち
static void _networkFriendsStandbyWait2(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int msg[]={POKETRADE_STR_05, POKETRADE_STR_04, POKETRADE_STR_06};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  }
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);

  _CHANGE_STATE(pWork,_changePokemonSendData);

}


//こうかんにだす　通信相手の準備待ち
static void _networkFriendsStandbyWait(POKEMON_TRADE_WORK* pWork)
{
  int i;

  int targetID = 1 - GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }

  switch(pWork->userNetCommand[targetID]){
  case _NETCMD_END:
  case _NETCMD_CHANGE_CANCEL:
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_97, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork, _endCancelState);
    break;
  case _NETCMD_LOOKATPOKE:
    if(pWork->userNetCommand[1-targetID]==_NETCMD_LOOKATPOKE){
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_20, pWork->pMessageStrBufEx );
      for(i=0;i<2;i++){
        POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, i);
        WORDSET_RegisterPokeNickName( pWork->pWordSet, i,  pp );
      }
      WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBuf, pWork->pMessageStrBufEx  );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      _CHANGE_STATE(pWork,_networkFriendsStandbyWait2);
    }
    break;
  case 0:
    break;
  default:
    GF_ASSERT_MSG(0,"%d\n",pWork->userNetCommand[targetID]);//返事がおかしい
    break;
  }
}

// ポケモンデータを相手に送る
static void _pokeSendDataState(POKEMON_TRADE_WORK* pWork)
{
  BOOL bMode = POKEMONTRADEPROC_IsNetworkMode(pWork);

  if( !bMode ){
    pWork->userNetCommand[0]=_NETCMD_LOOKATPOKE;
    pWork->userNetCommand[1]=_NETCMD_LOOKATPOKE;
  }
  if(bMode){
     if( !GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_LOOKATPOKE, 0,NULL) ){
       return;
     }
  }
  _CHANGE_STATE(pWork, _networkFriendsStandbyWait);// あいても選ぶまで待つ
}


//こうかんにだす、メニュー待ち
static void _changeMenuWait(POKEMON_TRADE_WORK* pWork)
{
  u32 x,y;


  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    pWork->pAppTask=NULL;

    if(selectno==0){  //交換に出す
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_09, pWork->pMessageStrBuf );

      POKETRADE_MESSAGE_WindowOpen(pWork);

   //   G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 8 );

      _CHANGE_STATE(pWork, _pokeSendDataState);

    }
    else{  //キャンセル
      //      _PokemonReset(pWork,0);
      _CHANGE_STATE(pWork, _touchState);
    }
  }
}


//こうかんにだす、メニュー表示
static void _changeMenuOpen(POKEMON_TRADE_WORK* pWork)
{
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_R, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_L, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_RETURN ,FALSE );
  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], FALSE );


  if( _PokemonsetAndSendData(pWork) )
  {
    int msg[]={POKETRADE_STR_01,POKETRADE_STR_06};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));

    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 8 );

    _CHANGE_STATE(pWork, _changeMenuWait);
  }
}


//ラインインデックスの位置にカーソル表示
static void IRC_POKETRADE_CursorEnable(POKEMON_TRADE_WORK* pWork,int line,int index)
{
  int i,j,k;

  pWork->MainObjCursorLine = line;   //OBJカーソルライン
  pWork->MainObjCursorIndex = index;  //OBJカーソルインデックス

  _PokemonIconRenew(pWork);

}

//タッチした場所にいるかどうか
static BOOL _SerchTouchCLACTPosition(POKEMON_TRADE_WORK* pWork, int* boxno,int* boxindex)
{
  BOOL bChange=FALSE;
  {
    u32 x,y;
    int line,index;

    if(GFL_UI_TP_GetPointTrg(&x, &y)==TRUE){
      GFL_CLWK* pCL = IRC_POKETRADE_GetCLACT(pWork,x+12,y+12, boxno, boxindex, &line, &index);
      if(pCL){
        POKEMON_PASO_PARAM* ppp =
          IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, *boxno, *boxindex, pWork);
        if(ppp && PPP_Get( ppp, ID_PARA_poke_exist, NULL  ) != 0 ){
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}


static BOOL IsTouchCLACTPosition(POKEMON_TRADE_WORK* pWork, BOOL bCatch)
{
  BOOL bChange=FALSE;
  {
    u32 x,y;
    int line,index;

    if(GFL_UI_TP_GetPointTrg(&x, &y)==TRUE){
      GFL_CLWK* pCL = IRC_POKETRADE_GetCLACT(pWork,x+12,y+12, &pWork->workBoxno, &pWork->workPokeIndex, &line, &index);

      if(pCL){
        POKEMON_PASO_PARAM* ppp =
          IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->workBoxno, pWork->workPokeIndex,pWork);
        _CatchPokemonPositionRewind(pWork);          //今のつかんでる物を元の位置に戻す
        if(ppp && PPP_Get( ppp, ID_PARA_poke_exist, NULL  ) != 0 ){

          _CatchPokemonPositionActive(pWork,pCL, line, index, ppp);

          IRC_POKETRADE_CursorEnable(pWork,line, index);  //OAMカーソル移動
          OS_TPrintf("GET %d %d\n",pWork->workBoxno,pWork->workPokeIndex);
          if(bCatch){
            pWork->underSelectBoxno  = pWork->workBoxno;
            pWork->underSelectIndex = pWork->workPokeIndex;
          }
          _PokemonIconRenew(pWork);
          bChange = TRUE;
//          pWork->bTouch=TRUE;
        }
      }
    }
  }
  return bChange;
}



//自分画面にステータス表示 相手に見せるかどうか待ち
static void _dispSubStateWait(POKEMON_TRADE_WORK* pWork)
{
  BOOL bExit=FALSE;
  BOOL bChange=FALSE;
  int selectno=-1;
  u32 x,y;
  int boxindex,boxno;

  if(GFL_UI_TP_GetTrg()){
    _CatchPokemonPositionRewind(pWork);
//    pWork->underSelectBoxno = -1;
//    pWork->underSelectIndex = -1;
  }

  
  TOUCHBAR_Main(pWork->pTouchWork);
  switch( TOUCHBAR_GetTouch(pWork->pTouchWork )){
  case TOUCHBAR_ICON_CUR_L:
  case TOUCHBAR_ICON_CUR_R:
    if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
      APP_TASKMENU_SetActive(pWork->pAppTask,FALSE);
    }
    break;
  }
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_CUR_L:
  case TOUCHBAR_ICON_CUR_R:
    {
      pWork->pokemonselectno = 1 - pWork->pokemonselectno;
      bChange=TRUE;
    }
    break;
  }

 // _CatchPokemonMoveFunc(pWork);

  if(GFL_UI_TP_GetPointTrg(&x, &y)==TRUE){
    //if(IsTouchCLACTPosition(pWork,TRUE)){
    if(_SerchTouchCLACTPosition(pWork,&pWork->underSelectBoxno,&pWork->underSelectIndex)){
      pWork->x = x;
      pWork->y = y;
      PMSND_PlaySystemSE(POKETRADESE_CUR);
      bChange = TRUE;
    }
    else{
      if((y >=  2*8) && ((18*8) > y)){
        bExit = TRUE;  //とじる
      }
    }
  }

  if(bChange){
    POKEMON_PASO_PARAM* ppp =
      IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox,
                                         pWork->underSelectBoxno, pWork->underSelectIndex,pWork);
    POKEMON_PARAM* pp = PP_CreateByPPP(ppp,pWork->heapID);
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->MyInfoWin), 0 );
    POKETRADE_MESSAGE_ResetPokemonMyStDisp(pWork);
    POKETRADE_MESSAGE_ChangePokemonMyStDisp(pWork, pWork->pokemonselectno , (pWork->x < 128), pp);
    _PokemonIconRenew(pWork);
    GFL_HEAP_FreeMemory(pp);
    return;
  }


  _CatchPokemonMoveFunc(pWork);

  if(bExit==FALSE){
    if(GFL_UI_TP_GetCont()==FALSE && pWork->bTouch){ //タッチパネルを離した直後
      if((pWork->pCatchCLWK != NULL) && pWork->bUpVec){ //ポケモンを上に登録
        PMSND_PlaySystemSE(POKETRADESE_UPPOKE);
        selectno = 0;  //決定と同じ
        bExit=TRUE;
      }
      else{  //ポケモンを元の位置に戻す
        _CatchPokemonPositionRewind(pWork);
      }
    }
  }
  _vectorUpMath(pWork);
  _CatchPokemonMoveFunc(pWork);

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    bExit=TRUE;
  }
  if(bExit){
    if(selectno==0){         //相手に見せる
      pWork->selectIndex = pWork->underSelectIndex;
      pWork->selectBoxno = pWork->underSelectBoxno;
      if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
        POKMEONTRADE2D_IconGray(pWork, pWork->pSelectCLWK, TRUE);
        
        _CHANGE_STATE(pWork, _changeMenuOpen);
      }
      else{
        POKE_GTS_PokemonsetAndSendData(pWork,pWork->selectIndex,pWork->selectBoxno);  //記録
        _CHANGE_STATE(pWork, _touchStateGTS);//タッチに戻る
      }
    }
    else{
      //やめる
      _CHANGE_STATE(pWork, POKE_TRADE_PROC_TouchStateCommon);
    }
    pWork->underSelectBoxno = -1;
    pWork->underSelectIndex = -1;
    _scrollResetAndIconReset(pWork);
    IRC_POKETRADE_ItemIconReset(&pWork->aItemMark);
    IRC_POKETRADE_ItemIconReset(&pWork->aPokerusMark);
    POKETRADE_MESSAGE_ResetPokemonMyStDisp(pWork);
    IRC_POKETRADE_SubStatusEnd(pWork);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    pWork->pAppTask=NULL;
  }

  pWork->bTouch = GFL_UI_TP_GetCont();
}


// 自分の画面にステータスを表示

static void _dispSubState(POKEMON_TRADE_WORK* pWork)
{

  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUR_R, TRUE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUR_L, TRUE);

  {
    int msg[]={POKETRADE_STR_03,POKETRADE_STR_06};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
  }

  {
    POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox,
                                                                 pWork->underSelectBoxno, pWork->underSelectIndex,pWork);
    POKEMON_PARAM* pp = PP_CreateByPPP(ppp,pWork->heapID);
    pWork->pokemonselectno = 0;
    POKETRADE_MESSAGE_ChangePokemonMyStDisp(pWork, pWork->pokemonselectno , (pWork->x < 128), pp);
    GFL_HEAP_FreeMemory(pp);
  }

  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], FALSE );
  _CatchPokemonPositionRewind(pWork);

  _PokemonIconRenew(pWork);

  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
    APP_TASKMENU_SetActive(pWork->pAppTask,FALSE);
  }
  else{
    APP_TASKMENU_SetActive(pWork->pAppTask,TRUE);
  }

  _CHANGE_STATE(pWork,_dispSubStateWait);
}

//通信同期を取る
static void _changeTimingDemoStart(POKEMON_TRADE_WORK* pWork)
{
  BOOL bMode = POKEMONTRADEPROC_IsNetworkMode(pWork);

  if(bMode){
    if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_TRADEDEMO_START,WB_NET_TRADE_SERVICEID)){
      return;
    }
  }
  

  if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
    _CHANGE_STATE(pWork,POKMEONTRADE_IRCDEMO_ChangeDemo);
  }
  else{
    _CHANGE_STATE(pWork,IRC_POKMEONTRADE_STEP_ChangeDemo_PokeMove);
  }
}

// 交換の返事を待つ
static void _changeWaitState(POKEMON_TRADE_WORK* pWork)
{
  int i;
  int myID = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  int targetID = 1 - myID;

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }

  switch(pWork->userNetCommand[myID]){
  case _NETCMD_CHANGE_CANCEL:
    if(pWork->userNetCommand[targetID]==_NETCMD_CHANGE_POKEMON || pWork->userNetCommand[targetID]==_NETCMD_EGG_AND_BATTLE ||
       pWork->userNetCommand[targetID]==_NETCMD_CHANGE_CANCEL){
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_97, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      _CHANGE_STATE(pWork, _endCancelState);
    }
    break;
  case _NETCMD_CHANGE_POKEMON:
    if(pWork->userNetCommand[targetID]==_NETCMD_CHANGE_POKEMON){
      if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
        GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_TRADEDEMO_START,WB_NET_TRADE_SERVICEID);
      }
      _CHANGE_STATE(pWork, _changeTimingDemoStart);
    }
    else if(pWork->userNetCommand[targetID]==_NETCMD_EGG_AND_BATTLE ||
            pWork->userNetCommand[targetID]==_NETCMD_CHANGE_CANCEL){
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_97, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      _CHANGE_STATE(pWork, _endCancelState);
    }
    break;
  case _NETCMD_EGG_AND_BATTLE:
    if(pWork->userNetCommand[targetID]==_NETCMD_CHANGE_POKEMON || pWork->userNetCommand[targetID]==_NETCMD_EGG_AND_BATTLE||
       pWork->userNetCommand[targetID]==_NETCMD_CHANGE_CANCEL){
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_98, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      _CHANGE_STATE(pWork, _endCancelState);
    }
    break;
  default:
    GF_ASSERT_MSG(0,"%d\n",pWork->userNetCommand[myID]);
    break;
  }
}


//セーブルーチンから戻ってきたところ
void IRC_POKMEONTRADE_ChangeFinish(POKEMON_TRADE_WORK* pWork)
{
  int id = 1-GFL_NET_SystemGetCurrentID();

  pWork->selectBoxno = 0;
  pWork->selectIndex = -1;
  pWork->pCatchCLWK = NULL;
  pWork->pSelectCLWK = NULL;
  pWork->pokemonThreeSet = FALSE;
  IRCPOKETRADE_PokeDeleteMcss(pWork, 0);
  IRCPOKETRADE_PokeDeleteMcss(pWork, 1);

  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    POKE_GTS_InitWork(pWork);
  }
  
  {
    int i;
    for(i = 0;i< CELL_DISP_NUM;i++){
      if(pWork->curIcon[i]){
        GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[i], TRUE );
      }
    }
  }


  POKETRADE_MESSAGE_WindowClear(pWork);


  _CHANGE_STATE(pWork, _touchState);
}


//キャンセルまち
static void _cancelPokemonSendDataNetwork(POKEMON_TRADE_WORK* pWork)
{
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_97, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork, _endCancelState);
  }
  else if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGE_CANCEL,0,NULL)){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_09, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork,_changeWaitState);
  }
}


//キャンセル画面待ち
static void _endCancelState(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    POKETRADE_MESSAGE_WindowClear(pWork);
    _CHANGE_STATE(pWork,_touchState);
  }
}


//終了待ち
static void _endWaitStateNetwork2(POKEMON_TRADE_WORK* pWork)
{
  int targetID = 1 - GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  if((pWork->userNetCommand[0] == _NETCMD_END) && (pWork->userNetCommand[1] == _NETCMD_END)){
    pWork->pParentWork->ret = POKEMONTRADE_MOVE_END;
    _CHANGE_STATE(pWork, POKEMONTRADE_PROC_FadeoutStart);
  }
  if(pWork->userNetCommand[targetID] == _NETCMD_LOOKATPOKE){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_04, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork, _endCancelState);
  }

}


//終わるのを相手に伝えている
static void _endWaitStateNetwork(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_END,0,NULL)){
      _CHANGE_STATE(pWork,_endWaitStateNetwork2);
    }
  }
  else{
    pWork->pParentWork->ret = POKEMONTRADE_MOVE_END;
    _CHANGE_STATE(pWork, POKEMONTRADE_PROC_FadeoutStart);
  }
}

// おわりまち
static void _endWaitState(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    pWork->pAppTask=NULL;
    switch(selectno){
    case 0:
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_06, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      _CHANGE_STATE(pWork,_endWaitStateNetwork);
      break;
    case 1:
      POKETRADE_MESSAGE_WindowClear(pWork);
      _CHANGE_STATE(pWork,POKE_TRADE_PROC_TouchStateCommon);
      TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_RETURN ,TRUE );
      
      //_CHANGE_STATE(pWork,_touchState);
      break;
    }
  }

}



static BOOL _PokemonsetAndSendData(POKEMON_TRADE_WORK* pWork)
{ //選択ポケモン表示
  BOOL bRet = FALSE;
  POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->selectBoxno, pWork->selectIndex,pWork);
  POKEMON_PARAM* pp = PP_CreateByPPP(ppp,pWork->heapID);

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_SELECT_POKEMON, POKETOOL_GetWorkSize(), pp)){
      GFL_HEAP_FreeMemory(pp);
      return FALSE;
    }
  }
    
  POKE_MAIN_Pokemonset(pWork,0,pp);
#if PM_DEBUG
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    const STRCODE *name;
    POKEMON_PARAM* pp2;

    pp2 = PP_Create(MONSNO_ONOKKUSU, 100, 123456, GFL_HEAPID_APP);
    name = MyStatus_GetMyName( pWork->pMy );
    PP_Put( pp2 , ID_PARA_oyaname_raw , (u32)name );
    PP_Put( pp2 , ID_PARA_oyasex , MyStatus_GetMySex( pWork->pMy ) );
    POKE_MAIN_Pokemonset(pWork,1,pp2); 
    GFL_STD_MemCopy(pp2,pWork->recvPoke[1],POKETOOL_GetWorkSize());
    GFL_HEAP_FreeMemory(pp2);
    GFL_STD_MemCopy(pp,pWork->recvPoke[0],POKETOOL_GetWorkSize());
    
  }
#endif
  bRet= TRUE;
  GFL_HEAP_FreeMemory(pp);
  return bRet;
}


//終了コールバック処理
static void _recvEndReqFunc(POKEMON_TRADE_WORK *pWork)
{
  //  _msgWindowCreate(pWork, POKETRADE_STR_22);

  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_22, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);

  {
    int msg[]={POKETRADE_STR_27, POKETRADE_STR_28};
    POKETRADE_MESSAGE_AppMenuOpenCustom(pWork,msg,elementof(msg),32,12);
  }

  pWork->bTouchReset=TRUE;

  _CHANGE_STATE(pWork, _endWaitState);
}



// 終了しますか？の問いかけそうしん
static void _endRequestState(POKEMON_TRADE_WORK* pWork)
{
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);
  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE );

  _CHANGE_STATE(pWork,_recvEndReqFunc);

}


static void _MoveSearchPoke(POKEMON_TRADE_WORK* pWork,int moji)
{
  int i,j,monsno;
  BOOL bFind = FALSE;
  int line = POKETRADE_boxScrollNum2Line(pWork);

  for(i = 0;i < pWork->TRADEBOX_LINEMAX;i++){
    line++;
    if(line >= pWork->TRADEBOX_LINEMAX){
      line=0;
    }
    for(j = 0;j < BOX_VERTICAL_NUM ;j++){
      POKEMON_PASO_PARAM* ppp =
        IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, IRC_TRADE_LINE2TRAY(line,pWork),
                                           IRC_TRADE_LINE2POKEINDEX(line,j),pWork);
      if(ppp){
        monsno = PPP_Get(ppp,ID_PARA_monsno_egg,NULL);
        if(POKEMONTRADE_IsPokeLanguageMark(monsno, moji)){

          OS_TPrintf("みつかった BOX %d\n",line/6);
          pWork->oldLine++;
          pWork->BoxScrollNum = _boxScrollLine2Num(line) - 32;
          pWork->bgscrollRenew = TRUE;
          _scrollMainFunc(pWork,FALSE,TRUE);
          return;
        }
      }
    }
  }
}

// 検索文字パネル表示中
static void _loopSearchMojiState(POKEMON_TRADE_WORK* pWork)
{
  BOOL endflg=FALSE;
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int ans = GFL_UI_TP_HitTrg(tp_mojidata);
    if(ans != GFL_UI_TP_HIT_NONE){
      PMSND_PlaySystemSE(POKETRADESE_LANG_SEARCH);
      pWork->selectMoji = ans + 1;
      endflg = TRUE;
      _MoveSearchPoke(pWork,ans);
    }
  }

  TOUCHBAR_Main(pWork->pTouchWork);
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
    pWork->selectMoji = 0;
    endflg=TRUE;
    break;
  default:
    break;
  }

  if(endflg){
    OS_TPrintf("SELECTMOJI %d\n", pWork->selectMoji);
    IRC_POKETRADE_EndSubMojiBG(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);
    GXS_SetVisibleWnd( GX_WNDMASK_NONE );
    _CHANGE_STATE(pWork,_touchState);
  }

}



// 検索文字パネル開始
static void _startSearchMojiState(POKEMON_TRADE_WORK* pWork)
{
  IRC_POKETRADE_InitSubMojiBG(pWork);

  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_52, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpenCustom(pWork,TRUE,FALSE);

//  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE );

  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ, 8 );

  
  if(1){
    G2S_SetWnd0InsidePlane(
      GX_WND_PLANEMASK_BG0|
      GX_WND_PLANEMASK_OBJ,
      FALSE );
    G2S_SetWnd0Position( 224, 169, 255, 192 );
    G2S_SetWndOutsidePlane(
      GX_WND_PLANEMASK_BG0|
      GX_WND_PLANEMASK_BG1|
      GX_WND_PLANEMASK_BG2|
      GX_WND_PLANEMASK_BG3|
      GX_WND_PLANEMASK_OBJ,
      TRUE );
    GXS_SetVisibleWnd( GX_WNDMASK_W0 );
  }



  
  _CHANGE_STATE(pWork,_loopSearchMojiState);
}

//------------------------------------------------------------------------------
/**
 * @brief   Lineの値からドットピッチを得る
 * @param   line
 * @retval  none
 */
//------------------------------------------------------------------------------

static int _boxScrollLine2Num(int line)
{
  int num=0, pos, page;

  if(line==0){
    num = 0;
  }
  else if(line==1){
    num = _TEMOTITRAY_MAX/2;
  }
  else{
    pos = line - 2;
    page = pos / 6;
    pos = pos % 6;
    num = _TEMOTITRAY_MAX + _BOXTRAY_MAX*page + (_BOXTRAY_MAX/6)*pos;
  }
  return num;
}

//------------------------------------------------------------------------------
/**
 * @brief   画面の外にカーソルがあるかどうか
 * @param   line
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _isCursorInScreen(POKEMON_TRADE_WORK* pWork, int line)
{
  int i=_boxScrollLine2Num(line);

  if((pWork->BoxScrollNum < i) && ((pWork->BoxScrollNum + 224) > i)){
    return TRUE;  //範囲内
  }
  i += pWork->_DOTMAX;
  if((pWork->BoxScrollNum < i) && ((pWork->BoxScrollNum + 224) > i)){
    return TRUE;
  }
  return FALSE;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief 基本の十字キー操作
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------
static void _padUDLRFunc(POKEMON_TRADE_WORK* pWork)
{
  BOOL bChange=FALSE;
  int num,i,line;
  int key = GFL_UI_KEY_GetRepeat();

  if(key & PAD_BUTTON_R ){   //ベクトルを監視
    pWork->padMode=TRUE;
    for(i=0;i < (pWork->BOX_TRAY_MAX+1) ; i++){
      if(centerPOS[i] > pWork->BoxScrollNum){
        pWork->BoxScrollNum = centerPOS[i];
        bChange=TRUE;
        break;
      }
    }
    if(!bChange){
      pWork->BoxScrollNum = centerPOS[0];
      bChange=TRUE;
    }
  }
  else if(key & PAD_BUTTON_L ){   //ベクトルを監視
    pWork->padMode=TRUE;
    for(i = pWork->BOX_TRAY_MAX ; i >= 0 ; i--){
      if(centerPOS[i] < pWork->BoxScrollNum){
        pWork->BoxScrollNum = centerPOS[i];
        bChange=TRUE;
        break;
      }
    }
    if(!bChange){
      pWork->BoxScrollNum = centerPOS[pWork->BOX_TRAY_MAX];
      bChange=TRUE;
    }
  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_UP ){   //ベクトルを監視
    pWork->padMode=TRUE;
    pWork->MainObjCursorIndex--;
    if(pWork->MainObjCursorIndex<0){
      pWork->MainObjCursorIndex=0;
    }
    pWork->oldLine--;
    bChange=TRUE;
  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_DOWN ){   //ベクトルを監視
    pWork->padMode=TRUE;
    pWork->MainObjCursorIndex++;
    if(pWork->MainObjCursorLine < HAND_HORIZONTAL_NUM){
      if(pWork->MainObjCursorIndex >= HAND_VERTICAL_NUM){
        pWork->MainObjCursorIndex = HAND_VERTICAL_NUM-1;
      }
    }
    else{
      if(pWork->MainObjCursorIndex >= BOX_VERTICAL_NUM){
        pWork->MainObjCursorIndex = BOX_VERTICAL_NUM-1;
      }
    }
    pWork->oldLine--;
    bChange=TRUE;
  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_RIGHT ){   //ベクトルを監視
    pWork->padMode=TRUE;
    if(POKETRADE_IsMainCursorDispIn(pWork, &line)==FALSE){
      pWork->MainObjCursorLine=line;
    }
    else{
      pWork->MainObjCursorLine++;
      if(FALSE == _isCursorInScreen(pWork,pWork->MainObjCursorLine)){
        pWork->BoxScrollNum = _boxScrollLine2Num(pWork->MainObjCursorLine+1)-256;
      }
    }
    if(pWork->MainObjCursorLine >= pWork->TRADEBOX_LINEMAX){
      pWork->MainObjCursorLine = 0;
    }
    if(pWork->MainObjCursorLine < HAND_HORIZONTAL_NUM){  //手持ちトレイ上ではカーソル位置の補正がある
      if(pWork->MainObjCursorIndex >= HAND_VERTICAL_NUM){
        pWork->MainObjCursorIndex = HAND_VERTICAL_NUM-1;
      }
    }
    pWork->oldLine--;
    bChange=TRUE;
  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_LEFT ){   //ベクトルを監視
    pWork->padMode=TRUE;
    if(POKETRADE_IsMainCursorDispIn(pWork, &line)==FALSE){  //移動するカーソルが画面内にいるかどうか
      pWork->MainObjCursorLine=line;
    }
    else{
      pWork->MainObjCursorLine--;
      if(pWork->MainObjCursorLine < 0){
        pWork->MainObjCursorLine = pWork->TRADEBOX_LINEMAX-1;
      }
      if(FALSE == _isCursorInScreen(pWork,pWork->MainObjCursorLine )){
        pWork->BoxScrollNum = _boxScrollLine2Num(pWork->MainObjCursorLine);
      }
    }
    if(pWork->MainObjCursorLine < 0){
      pWork->MainObjCursorLine = pWork->TRADEBOX_LINEMAX-1;
    }
    if(pWork->MainObjCursorLine < HAND_HORIZONTAL_NUM){  //手持ちトレイ上ではカーソル位置の補正がある
      if(pWork->MainObjCursorIndex >= HAND_VERTICAL_NUM){
        pWork->MainObjCursorIndex = HAND_VERTICAL_NUM-1;
      }
    }
    pWork->oldLine++;
    bChange=TRUE;
  }

  if(bChange){
    pWork->bgscrollRenew = TRUE;
    PMSND_PlaySystemSE(POKETRADESE_CUR);
    _scrollMainFunc(pWork,FALSE,TRUE);
  }


}

//--------------------------------------------------------------------------------------------
/**
 * @brief スクロールする実行関数
 * @param POKEMON_TRADE_WORK* ワーク
 * @param bSE      SEならすかどうか
 * @param bNetSend 通信位置を相手に送るかどうか
 */
//--------------------------------------------------------------------------------------------

static void _scrollMainFunc(POKEMON_TRADE_WORK* pWork,BOOL bSE, BOOL bNetSend)
{

  if(0 > pWork->BoxScrollNum){
    pWork->BoxScrollNum+=pWork->_DOTMAX;
  }
  if(pWork->_DOTMAX <= pWork->BoxScrollNum){
    pWork->BoxScrollNum-=pWork->_DOTMAX;
  }

  IRC_POKETRADE_TrayDisp(pWork);
  IRC_POKETRADE_SendScreenBoxNameChar(pWork);
  IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork);

  if(!PMSND_CheckPlaySE() && bSE){
    PMSND_PlaySystemSE(POKETRADESE_RIBBON);
  }

  if(POKEMONTRADEPROC_IsNetworkMode(pWork) && bNetSend){  // 特に失敗してもかまわない通信 相手に位置を知らせているだけ
    GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(),GFL_NET_SENDID_ALLUSER,
                       _NETCMD_SCROLLBAR,2,&pWork->BoxScrollNum,FALSE,TRUE,TRUE);
  }
  else{
    pWork->FriendBoxScrollNum = pWork->BoxScrollNum;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief スクロールする実行関数
 * @param POKEMON_TRADE_WORK* ワーク
 * @param bSE      SEならすかどうか
 * @param bNetSend 通信位置を相手に送るかどうか
 */
//--------------------------------------------------------------------------------------------

static void _panelScroll(POKEMON_TRADE_WORK* pWork)
{
  u32 x,y;
  
  if(GFL_UI_TP_GetPointCont(&x,&y)){     // パネルスクロール
    if(pWork->touchON){
      if((x >=  64) && ((192) > x)){
        if((y >=  152+12) && ((176+12) > y)){
          GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[CELL_CUR_SCROLLBAR] , TRUE );
          pWork->speed = (x - pWork->xspeed)*2;
          pWork->xspeed = x;
          pWork->BoxScrollNum -= pWork->speed;
          if(pWork->speed > 12){
            _scrollMainFunc(pWork,TRUE,TRUE);
          }
          else{
            _scrollMainFunc(pWork,FALSE,TRUE);
          }
        }
      }
    }
    //pWork->x = x;
    //pWork->y = y;
    pWork->touchON = TRUE;
    pWork->xspeed = x;
  }
  else{
    pWork->xspeed=0;
  }

  if(GFL_UI_TP_GetCont()==FALSE){  //慣性
    if(pWork->speed!=0){
      int k=2;
      while(k && pWork->speed){
        if(pWork->speed < 0){
          pWork->speed++;
        }
        if(pWork->speed > 0){
          pWork->speed--;
        }
        k--;
      }
      pWork->BoxScrollNum -= pWork->speed;
      _scrollMainFunc(pWork,TRUE,TRUE);
    }
  }
}



static void _DeletePokemonState(POKEMON_TRADE_WORK* pWork)
{
  int no = POKE_GTS_IsSelect(pWork,pWork->underSelectBoxno,pWork->underSelectIndex );

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CANCEL_POKEMON,0,NULL)){
      return;
    }
  }
  pWork->selectIndex = -1;
  pWork->selectBoxno = -1;
  _PokemonReset(pWork,0);

  _PokemonIconRenew(pWork);
  
  _CHANGE_STATE(pWork, POKE_TRADE_PROC_TouchStateCommon);
}




//--------------------------------------------------------------------------------------------
/**
 * @brief 基本ステート初期化
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------
static void _touchState(POKEMON_TRADE_WORK* pWork)
{
  int i;

  GFL_STD_MemClear(pWork->pokeIconNo,sizeof(pWork->pokeIconNo));

  _CatchPokemonPositionRewind(pWork);


  if(pWork->pAppTask!=NULL){
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }


  _PokemonReset(pWork,0);
  
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    _PokemonReset(pWork,1);
    _userNetCommandClear(pWork);
  }
  else{
    int myID = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

    pWork->userNetCommand[myID]=0;
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CANCEL_POKEMON,0,NULL);
  }

  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_R, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_L, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_RETURN ,TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], TRUE );

  IRC_POKETRADE_SendVramBoxNameChar(pWork); // ボックス名初期化
  POKEMONTRADE_2D_AlphaSet(pWork); //G2S_BlendNone();

  pWork->selectIndex=-1;
  pWork->selectBoxno=-1;
  
  _PokemonIconRenew(pWork);

//  _userNetCommandClear(pWork);
  
  _CHANGE_STATE(pWork,POKE_TRADE_PROC_TouchStateCommon);

}

//--------------------------------------------------------------------------------------------
/**
 * @brief スクロールリセットとアイコンのリセット
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------

static void _scrollResetAndIconReset(POKEMON_TRADE_WORK* pWork)
{
  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], TRUE );
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, TRUE);
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_R, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_L, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_RETURN ,TRUE );
  _PokemonIconRenew(pWork);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief もう一回ポケモンを選択する為にtouchStateに戻ってきた場合この関数
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------
static void _touchStateGTS(POKEMON_TRADE_WORK* pWork)
{
  int i;

  GFL_STD_MemClear(pWork->pokeIconNo,sizeof(pWork->pokeIconNo));

  _CatchPokemonPositionRewind(pWork);


  if(pWork->pAppTask!=NULL){
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
  if(pWork->GTSSelectIndex[0][0]!=-1){
    int msg[]={POKETRADE_STR2_26};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
  }


  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_R, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_L, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_RETURN ,TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], TRUE );

  IRC_POKETRADE_SendVramBoxNameChar(pWork); // ボックス名初期化
  POKEMONTRADE_2D_AlphaSet(pWork); //G2S_BlendNone();

  _PokemonIconRenew(pWork);

  _CHANGE_STATE(pWork,POKE_TRADE_PROC_TouchStateCommon);

}


static void _POKE_SetAndSendData(POKEMON_TRADE_WORK* pWork)
{

  if(POKEMONTRADE_SuckedMain(pWork)){
    _PokemonsetAndSendData(pWork);

    pWork->workPokeIndex = -1;
    pWork->workBoxno = -1;
    
    _CHANGE_STATE(pWork,POKE_TRADE_PROC_TouchStateCommon);
  }
}


//--------------------------------------------------------------------------------------------
/**
 * @brief タッチ操作で他のアクションに飛ぶ基本ステート
 * @param POKEMON_TRADE_WORK* ワーク


   選択ポケモンが消えるのではなく
   ３体影になる

   
 */
//--------------------------------------------------------------------------------------------
void POKE_TRADE_PROC_TouchStateCommon(POKEMON_TRADE_WORK* pWork)
{
  u32 x,y,i;
  GFL_CLACTPOS pos;
  BOOL bDecided=FALSE;
  int backBoxNo = pWork->nowBoxno;
  POKEMON_PASO_PARAM* ppp;

  // 監視処理
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){  //速度リセット
    pWork->speed=0;
  }
  if(GFL_UI_TP_GetTrg()){  //ポケモン選択リセット
    _CatchPokemonPositionRewind(pWork);
    pWork->underSelectBoxno = -1;
    pWork->underSelectIndex = -1;
  }
  
  _vectorUpMath(pWork);         // 上にポケモンを登録するかどうかの判定
  _CatchPokemonMoveFunc(pWork); // タッチ中のポケモン移動処理
  _padUDLRFunc(pWork);   //スクロールキー処理

  //決定処理
  if(GFL_UI_KEY_GetTrg()== PAD_BUTTON_DECIDE){
    int line;
    if(POKETRADE_IsMainCursorDispIn(pWork, &line)==FALSE){
      pWork->MainObjCursorLine=line;
    }
    {
      ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, IRC_TRADE_LINE2TRAY(pWork->MainObjCursorLine,pWork),
                                               IRC_TRADE_LINE2POKEINDEX(pWork->MainObjCursorLine,pWork->MainObjCursorIndex),pWork);
//    i = IRC_TRADE_LINE2POKEINDEX(pWork->MainObjCursorLine,pWork->MainObjCursorIndex);  //カーソル下のポケモンを探す
      //ポケモンがいた場合
      if(ppp && PPP_Get( ppp, ID_PARA_poke_exist, NULL  ) != 0 ){

        pWork->padMode=TRUE;
        pWork->underSelectBoxno = IRC_TRADE_LINE2TRAY(pWork->MainObjCursorLine,pWork);
        pWork->underSelectIndex = IRC_TRADE_LINE2POKEINDEX(pWork->MainObjCursorLine,pWork->MainObjCursorIndex);
        GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);

        if((POKEMONTRADEPROC_IsTriSelect(pWork)) &&  //GTSの場合 選択してあるポケモンだと動作が違う
           (-1 != POKE_GTS_IsSelect(pWork,pWork->underSelectBoxno,pWork->underSelectIndex ))){
          
          _CHANGE_STATE(pWork, POKE_GTS_DeletePokemonState);
          return;
          
        }
        else{
          ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->underSelectBoxno, pWork->underSelectIndex,pWork);
          
          _CatchPokemonPositionRewind(pWork);
          _CatchPokemonPositionActive(pWork,
                                      pWork->pokeIcon[POKETRADE_Line2RingLineIconGet(pWork->MainObjCursorLine)][pWork->MainObjCursorIndex],
                                      pWork->MainObjCursorLine,pWork->MainObjCursorIndex,ppp);
          IRC_POKETRADE_SetCursorXY(pWork);
          IRC_POKETRADE_CursorEnable(pWork,pWork->MainObjCursorLine,pWork->MainObjCursorIndex);
          bDecided=TRUE;
        }
      }
    }
  }

  if(IsTouchCLACTPosition(pWork,FALSE)){
    GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
    pWork->bStatusDisp=TRUE;
  }


  if( !GFL_UI_TP_GetCont() ){
    if(pWork->touchON && pWork->bUpVec && pWork->pCatchCLWK){
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, TRUE);
      TOUCHBAR_SetActive(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, TRUE);
      pWork->touchON = FALSE;
      pWork->bUpVec = FALSE;
      PMSND_PlaySystemSE(POKETRADESE_UPPOKE);
      pWork->underSelectBoxno  = pWork->workBoxno;
      pWork->underSelectIndex = pWork->workPokeIndex;


      pWork->selectIndex = pWork->underSelectIndex;
      pWork->selectBoxno = pWork->underSelectBoxno;
      if(!POKEMONTRADEPROC_IsTriSelect(pWork)){        // 選択したポケモンを投げるところ
        pWork->pCatchCLWK = NULL;
        POKEMONTRADE_StartSucked(pWork);
        _CHANGE_STATE(pWork, _POKE_SetAndSendData);
        return;
      }
      else if((POKEMONTRADEPROC_IsTriSelect(pWork)) &&  //GTSの場合 選択してあるポケモンだと動作が違う
              (-1 != POKE_GTS_IsSelect(pWork,pWork->underSelectBoxno,pWork->underSelectIndex ))){
        _CHANGE_STATE(pWork, POKE_GTS_DeletePokemonState);
        return;
      }
      else if(POKE_GTS_PokemonsetAndSendData(pWork,pWork->selectIndex,pWork->selectBoxno)){  //記録
        _CatchPokemonPositionRewind(pWork);
      }
      pWork->underSelectIndex = -1;
      pWork->underSelectBoxno = -1;
    }
    else if(pWork->touchON && pWork->pCatchCLWK && pWork->bStatusDisp){
      bDecided = TRUE;
      pWork->underSelectBoxno = pWork->workBoxno;
      pWork->underSelectIndex = pWork->workPokeIndex;
    }
    else{
      _CatchPokemonPositionRewind(pWork);
    }
    pWork->touchON = FALSE;
    pWork->bUpVec = FALSE;
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[CELL_CUR_SCROLLBAR] , FALSE );
  }

  
  if( bDecided ){
    POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->underSelectBoxno, pWork->underSelectIndex,pWork);

    if((pWork->selectIndex == pWork->underSelectIndex) &&
       (pWork->selectBoxno == pWork->underSelectBoxno) &&
       (!POKEMONTRADEPROC_IsTriSelect(pWork))){
      //おなじポケモンを選択したらキャンセル
      _CHANGE_STATE(pWork,_DeletePokemonState);
      return;
    }

    if(ppp && PPP_Get( ppp, ID_PARA_poke_exist, NULL  ) != 0 ){
      PMSND_PlaySystemSE(POKETRADESE_MARKON);
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);

      POKETRADE_MESSAGE_AppMenuClose(pWork);
      _CHANGE_STATE(pWork,_dispSubState);


      return;
    }
    else{
      _CatchPokemonPositionRewind(pWork);
      pWork->underSelectBoxno = -1;
      pWork->underSelectIndex = -1;
    }
  }

  //パネルのスクロール
  _panelScroll(pWork);

  TOUCHBAR_Main(pWork->pTouchWork);   //タッチバー
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
    _CHANGE_STATE(pWork,_endRequestState);
    return;
  case TOUCHBAR_ICON_CUTSOM1:
    _CHANGE_STATE(pWork,_startSearchMojiState);
    return;
  case TOUCHBAR_ICON_CUTSOM2:
    if(pWork->selectIndex == -1){  //ポケモン選んで無い場合元に戻す
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE);
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
    }
    else{
      _CHANGE_STATE(pWork, _changeMenuOpen);
    }
    return;
    break;
  default:
    break;
  }
  if(pWork->pAppTask){  //GTS決定ボタン
    if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
      int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
      POKETRADE_MESSAGE_AppMenuClose(pWork);
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE);
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
      _CHANGE_STATE(pWork, POKE_GTS_Select6Init);
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  GTSネゴシエーションだと最初に台詞が入る
 *  @param  POKEMON_TRADE_WORK
 *  @return none
 */
//-----------------------------------------------------------------------------

static void _gtsFirstMsgState(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }

  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){

    POKETRADE_MESSAGE_WindowClear(pWork);
    POKEMONTRADE_2D_AlphaSet(pWork); //G2S_BlendNone();

    _CHANGE_STATE(pWork,_touchState);

  }
}

//------------------------------------------------------------------------------
/**
 * @brief   MCSSと3D画面システムの初期化と開放  この関数はデモにいっても有効にする為LOWからとってある
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _mcssSystemHeapInit(POKEMON_TRADE_WORK* pWork)
{
  IRC_POKETRADE_3DGraphicSetUp( pWork );

  pWork->mcssSys = MCSS_Init( 4 , GetHeapLowID(pWork->heapID) );
  MCSS_SetTextureTransAdrs( pWork->mcssSys , 0x30000 );
  MCSS_SetOrthoMode( pWork->mcssSys );
}

static void _mcssSystemHeapEnd(POKEMON_TRADE_WORK* pWork)
{
  IRCPOKETRADE_PokeDeleteMcss(pWork, 0);
  IRCPOKETRADE_PokeDeleteMcss(pWork, 1);
  MCSS_Exit(pWork->mcssSys);
  pWork->mcssSys=NULL;
  GFL_G3D_CAMERA_Delete(pWork->pCamera);
  GFL_G3D_Exit();

  
}




//------------------------------------------------------------------------------
/**
 * @brief   画面システム周りの初期化と開放
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _dispSystemHeapInit(POKEMON_TRADE_WORK* pWork)
{
  GFL_BG_Init( pWork->heapID );
  GFL_BMPWIN_Init( pWork->heapID );

  IRC_POKETRADE_SetMainDispGraphic(pWork);
  IRC_POKETRADE_SetMainVram(pWork);
  IRC_POKETRADE_SetSubVram(pWork);

}

static void _dispSystemHeapEnd(POKEMON_TRADE_WORK* pWork)
{
  GFL_CLACT_SYS_Delete();
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

//------------------------------------------------------------------------------
/**
 * @brief   画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _dispInit(POKEMON_TRADE_WORK* pWork)
{

  IRC_POKETRADE_CommonCellInit(pWork);  //共通CELL

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);

  IRC_POKETRADE_InitBoxCursor(pWork);

  IRC_POKETRADE_CreatePokeIconResource(pWork);

  IRC_POKETRADE_SendVramBoxNameChar(pWork); // ボックス名初期化

  IRC_POKETRADE_GraphicInitSubDisp(pWork);  //BGを
  IRC_POKETRADE_GraphicInitMainDisp(pWork);

  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, _BRIGHTNESS_SYNC);

  GFL_NET_WirelessIconEasy_HoldLCD(TRUE,pWork->heapID); //通信アイコン
  GFL_NET_ReloadIcon();

  IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork);  //ポケモンの表示

}


//--------------------------------------------------------------
/**
 * G3D VBlank処理
 * @param TCB GFL_TCB
 * @param work tcb work
 * @retval nothing
 */
//--------------------------------------------------------------
static void _VBlank( GFL_TCB *tcb, void *work )
{
  POKEMON_TRADE_WORK *pWork=work;

  GFL_CLACT_SYS_VBlankFunc(); //セルアクターVBlank

  if(pWork->bgscrollRenew){

    GFL_BG_SetScroll(GFL_BG_FRAME3_S,GFL_BG_SCROLL_X_SET, pWork->bgscroll);
    GFL_BG_SetScroll(GFL_BG_FRAME1_S,GFL_BG_SCROLL_X_SET, pWork->bgscroll);
    pWork->bgscrollRenew=FALSE;
  }
}


#if PM_DEBUG
//--------------------------------------------------------------
/**
 * @brief   デバッグ用に適当に手持ちポケモンをAdd
 * @param   gsys
 */
//--------------------------------------------------------------
static void DEBUG_MyPokeAdd(POKEPARTY *party,MYSTATUS *myStatus,HEAPID heapID)
{
  POKEMON_PARAM *pp;
  const STRCODE *name;
  int i,itemno;
  MAIL_DATA* pMail;
  STRBUF* pStrBuf= GFL_STR_CreateBuffer( 128, heapID );

  while( PokeParty_GetPokeCount(party)){
    PokeParty_Delete(party,0);
  }

  name = MyStatus_GetMyName( myStatus );

  pp = PP_Create(93, 100, 123456, GFL_HEAPID_APP);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );

  pMail=MailData_MailEventDataMake(pp, 1, MyStatus_GetMySex( myStatus ),pStrBuf,
                                   123, heapID);
  itemno = ITEM_MailNumGet(MailData_GetDesignNo(pMail));
  PP_Put(pp,ID_PARA_mail_data,(u32)pMail);
  PP_Put(pp,ID_PARA_item,itemno);

  PokeParty_Add(party, pp);

  PP_Setup(pp, MONSNO_MUSYAANA, 100, 123456);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PokeParty_Add(party, pp);
  PokeParty_Add(party, pp);
  PokeParty_Add(party, pp);
  PP_Setup(pp, MONSNO_MANAFI, 100, 123456);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PokeParty_Add(party, pp);
  PokeParty_Add(party, pp);
  GFL_HEAP_FreeMemory(pp);
  GFL_HEAP_FreeMemory(pMail);
  GFL_HEAP_FreeMemory(pStrBuf);
}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   セーブ関連の初期化と開放
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _savedataHeapInit(POKEMON_TRADE_WORK* pWork,GAMEDATA* pGameData)
{
  pWork->TempBuffer[0] = GFL_HEAP_AllocClearMemory( HEAPID_IRCBATTLE, POKETOOL_GetWorkSize() );
  pWork->TempBuffer[1] = GFL_HEAP_AllocClearMemory( HEAPID_IRCBATTLE, POKETOOL_GetWorkSize() );

  if(pGameData){
    pWork->pGameData=pGameData;
    pWork->pBox = GAMEDATA_GetBoxManager(pGameData);
    pWork->pMy = GAMEDATA_GetMyStatus( pGameData );
    pWork->pMyParty = GAMEDATA_GetMyPokemon(pGameData);
    pWork->pMailBlock = GAMEDATA_GetMailBlock(pGameData);
    if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
      pWork->pFriend = GAMEDATA_GetMyStatusPlayer(pGameData, 1-GFL_NET_SystemGetCurrentID());
    }
    else{
      pWork->pFriend = GAMEDATA_GetMyStatusPlayer(pGameData, 1);
    }
  }
#if PM_DEBUG
  else{
    pWork->pBox = BOX_DAT_InitManager(pWork->heapID,SaveControl_GetPointer());
    pWork->pMy = MyStatus_AllocWork(pWork->heapID);

    MyStatus_Init(pWork->pMy);
    
    pWork->pMailBlock = GFL_HEAP_AllocClearMemory(pWork->heapID,MAIL_GetBlockWorkSize());
    MAIL_Init(pWork->pMailBlock);
    {
      POKEMON_PARAM *pp;
      int i,j;
      BOX_MANAGER* pBox = pWork->pBox;
      pp = PP_Create(MONSNO_ONOKKUSU, 100, 123456, HEAPID_IRCBATTLE);

      for(i=0;i < 3;i++){
        for(j=0;j < 2;j++){
          u16 oyaName[5] = {L'デ',L'バ',L'ッ',L'グ',0xFFFF};
          POKEMON_PERSONAL_DATA* ppd = POKE_PERSONAL_OpenHandle(MONSNO_MARIRU+i, 0, GFL_HEAPID_APP);
          u32 ret = POKE_PERSONAL_GetParam(ppd,POKEPER_ID_sex);

          PP_SetupEx(pp, 90+i+j, i+j, 123456,PTL_SETUP_POW_AUTO, ret);
          PP_Put( pp , ID_PARA_oyaname_raw , (u32)oyaName );
          PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex(  pWork->pMy ) );
          PP_Put( pp , ID_PARA_item , 18 );
          PP_Put( pp , ID_PARA_pokerus , 1 );

          BOXDAT_PutPokemonBox(pBox, i, (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst(pp));

          POKE_PERSONAL_CloseHandle(ppd);
        }
      }
      GFL_HEAP_FreeMemory(pp);
    }

    pWork->pMyParty = PokeParty_AllocPartyWork(pWork->heapID);
  }
#endif
}


static void _savedataHeapEnd(POKEMON_TRADE_WORK* pWork,BOOL bParentWork)
{
  GFL_HEAP_FreeMemory(pWork->TempBuffer[0]);
  GFL_HEAP_FreeMemory(pWork->TempBuffer[1]);

  if(bParentWork){
  }
#if PM_DEBUG
  else{
    BOX_DAT_ExitManager(pWork->pBox);
    GFL_HEAP_FreeMemory(pWork->pMy);
    GFL_HEAP_FreeMemory(pWork->pFriend);
    GFL_HEAP_FreeMemory(pWork->pMyParty);
    GFL_HEAP_FreeMemory(pWork->pMailBlock );
  }
#endif

}

//------------------------------------------------------------------------------
/**
 * @brief        MAX_TRAY_NUMに関しての変数を初期化
 * @param pwk    EVENT_IRCBATTLE_WORKが必要
 * @retval       none
 */
//------------------------------------------------------------------------------

static void _maxTrayNumInit(POKEMON_TRADE_WORK *pWork)
{
  int max = BOXDAT_GetTrayMax(pWork->pBox);
  
  pWork->TRADEBOX_LINEMAX = max * 6 + 2;
  pWork->BOX_TRAY_MAX = max;
  pWork->_SRCMAX = ((max*_BOXTRAY_SCR_MAX)+_TEMOTITRAY_SCR_MAX);
  pWork->_DOTMAX =  ((max*_BOXTRAY_MAX)+_TEMOTITRAY_MAX);
//  pWork->_DOT_START_POS = (pWork->_DOTMAX - 80);
}


//------------------------------------------------------------------------------
/**
 * @brief        交換のコアのメモリを開放
 * @param pwk    EVENT_IRCBATTLE_WORKが必要
 * @retval       none
 */
//------------------------------------------------------------------------------

void POKMEONTRADE_RemoveCoreResource(POKEMON_TRADE_WORK* pWork)
{
  POKETRADE_2D_GTSPokemonIconResetAll(pWork);
  IRC_POKETRADEDEMO_RemoveModel( pWork);
  POKE_GTS_ReleasePokeIconResource(pWork);
  IRC_POKETRADE_ResetBoxNameWindow(pWork);
  IRCPOKETRADE_PokeDeleteMcss(pWork,1);


  POKETRADE_MESSAGE_SixStateDelete(pWork);

  IRC_POKETRADE_AllDeletePokeIconResource(pWork);
  IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork,0);
  IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork,1);
  if(pWork->pTouchWork){
    TOUCHBAR_Exit(pWork->pTouchWork);
    pWork->pTouchWork=NULL;
  }
  IRC_POKETRADE_EndIconResource(pWork);
  if(pWork->cellUnit){
    GFL_CLACT_UNIT_Delete(pWork->cellUnit);
    pWork->cellUnit=NULL;
  }
  POKETRADE_MESSAGE_HeapEnd(pWork);
  
  POKE_GTS_EndWork(pWork);
    
  IRC_POKETRADE_MainGraphicExit(pWork);
  IRC_POKETRADE_SubGraphicExit(pWork);

  IRC_POKETRADE_ResetSubDispGraphic(pWork);

  if(pWork->pD2Fade){
    GFL_HEAP_FreeMemory(pWork->pD2Fade);
    pWork->pD2Fade=NULL;
  }
  if(pWork->pMoveMcss[0]){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
  }
  if(pWork->pMoveMcss[1]){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[1]);
    pWork->pMoveMcss[1]=NULL;
  }

  GFL_HEAP_DEBUG_PrintExistMemoryBlocks(pWork->heapID);

}





FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(app_mail);

//------------------------------------------------------------------------------
/**
 * @brief        PROCスタート
 * @param pwk    EVENT_IRCBATTLE_WORKが必要
 * @retval       none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT PokemonTradeProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk,int type )
{
  int i;
  POKEMONTRADE_PARAM* pParentWork = pwk;
  POKEMON_TRADE_WORK *pWork = mywk;

  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  GFL_OVERLAY_Load( FS_OVERLAY_ID(app_mail));
  GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);

  pWork->heapID = HEAPID_IRCBATTLE;
  pWork->type = type;
  pWork->pParentWork = pParentWork;
  pWork->recvPoke[0] = GFL_HEAP_AllocClearMemory(pWork->heapID, POKETOOL_GetWorkSize());
  pWork->recvPoke[1] = GFL_HEAP_AllocClearMemory(pWork->heapID, POKETOOL_GetWorkSize());

  if(pParentWork){
    _savedataHeapInit(pWork,pParentWork->gamedata);
  }
  _maxTrayNumInit(pWork);
  _mcssSystemHeapInit(pWork);
  _dispSystemHeapInit(pWork);

  //セル系システムの作成
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 340 , 0 , pWork->heapID );
  // 通信テーブル追加
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    GFL_NET_AddCommandTable(GFL_NET_CMD_IRCTRADE,_PacketTbl,NELEMS(_PacketTbl), pWork);
  }
  // ワークの初期化
  pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );
  pWork->pCatchCLWK = NULL;
  pWork->selectIndex = -1;
  pWork->modelno = -1;
  pWork->workPokeIndex = -1;
  pWork->workBoxno = -1;
  pWork->underSelectBoxno = -1;
  pWork->underSelectIndex = -1;

  
  IRC_POKETRADEDEMO_Init(pWork);
  if(POKEMONTRADE_MOVE_EVOLUTION == pParentWork->ret){
    _CHANGE_STATE(pWork, POKMEONTRADE_SAVE_TimingStart);
    return GFL_PROC_RES_FINISH;
  }
  if(POKEMONTRADE_MOVE_MAIL == pParentWork->ret){
    _CHANGE_STATE(pWork, POKMEONTRADE_EVOLUTION_TimingStart);
    return GFL_PROC_RES_FINISH;
  }
  
  POKETRADE_MESSAGE_HeapInit(pWork);
  _dispInit(pWork);
  POKETRADE_TOUCHBAR_Init(pWork);
  

#if DEBUG_ONLY_FOR_ohno | DEBUG_ONLY_FOR_ibe_mana
  DEBUGWIN_InitProc( GFL_BG_FRAME3_M , pWork->pFontHandle );
  DEBUG_PAUSE_SetEnable( TRUE );
#endif

  IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);

  if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
    _CHANGE_STATE(pWork, _touchState);
  }
  else{
    POKE_GTS_InitWork(pWork);
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_13, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 8 );
    
    POKE_GTS_CreatePokeIconResource(pWork,CLSYS_DRAW_MAIN);      
    POKE_GTS_StatusMessageDisp(pWork);
    POKE_GTS_SelectStatusMessageDisp(pWork,0,FALSE);
    POKE_GTS_SelectStatusMessageDisp(pWork,1,FALSE);
    _CHANGE_STATE(pWork, _gtsFirstMsgState);
  }

  return GFL_PROC_RES_FINISH;
}

//クラブ用データ
static GFL_PROC_RESULT PokemonTradeClubProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMON_TRADE_WORK *pWork;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, HEAPSIZE_POKETRADE );
  pWork = GFL_PROC_AllocWork( proc, sizeof( POKEMON_TRADE_WORK ), HEAPID_IRCBATTLE );
  GFL_STD_MemClear(pWork, sizeof(POKEMON_TRADE_WORK));

  return PokemonTradeProcInit(proc,seq,pwk,pWork, POKEMONTRADE_TYPE_WIFICLUB);
}

// 赤外線用
static GFL_PROC_RESULT PokemonTradeIrcProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMON_TRADE_WORK *pWork;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, HEAPSIZE_POKETRADE );
  pWork = GFL_PROC_AllocWork( proc, sizeof( POKEMON_TRADE_WORK ), HEAPID_IRCBATTLE );
  GFL_STD_MemClear(pWork, sizeof(POKEMON_TRADE_WORK));

  return PokemonTradeProcInit(proc,seq,pwk,pWork,POKEMONTRADE_TYPE_IRC);
}


// UNION用
static GFL_PROC_RESULT PokemonTradeUnionNegoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMONTRADE_PARAM* pParent=pwk;
  POKEMON_TRADE_WORK *pWork;
  int i;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, HEAPSIZE_POKETRADE );
  pWork = GFL_PROC_AllocWork( proc, sizeof( POKEMON_TRADE_WORK ), HEAPID_IRCBATTLE );
  GFL_STD_MemClear(pWork, sizeof(POKEMON_TRADE_WORK));


  for(i=0;i<2;i++){
    pWork->GTStype[i] = 0;  //
    pWork->GTSlv[i] = 1;  //
  }
  pWork->pFriend = GAMEDATA_GetMyStatusPlayer(pParent->gamedata, 1-GFL_NET_SystemGetCurrentID());

  return PokemonTradeProcInit(proc,seq,pParent ,pWork,POKEMONTRADE_TYPE_GTSNEGO);
}

// GTSネゴシエーション用
static GFL_PROC_RESULT PokemonTradeGTSNegoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMONTRADE_PARAM* pParent=pwk;
  EVENT_GTSNEGO_WORK* pGTS=NULL;
  POKEMON_TRADE_WORK *pWork;
  int i;

  pGTS = pParent->pNego;
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, HEAPSIZE_POKETRADE );
  pWork = GFL_PROC_AllocWork( proc, sizeof( POKEMON_TRADE_WORK ), HEAPID_IRCBATTLE );
  GFL_STD_MemClear(pWork, sizeof(POKEMON_TRADE_WORK));


  if(pGTS){
    for(i=0;i<2;i++){
      pWork->GTStype[i]=pGTS->aUser[i].selectType;  //
      pWork->GTSlv[i]=pGTS->aUser[i].selectLV;  //
      NET_PRINT("GTStype %d %d\n",pGTS->aUser[i].selectType ,pGTS->aUser[i].selectLV);
      NET_PRINT("GTStype %d %d\n",pWork->GTStype[i],pWork->GTSlv[i]);
    }
//    pWork->pFriend=pGTS->pStatus[1];
  }
  return PokemonTradeProcInit(proc,seq, pParent ,pWork, POKEMONTRADE_TYPE_GTSNEGO);
}

//IRデモのみ表示
static GFL_PROC_RESULT PokemonTradeDemoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GFL_PROC_RESULT ret;
  POKEMONTRADE_DEMO_PARAM* pParent=pwk;
  POKEMONTRADE_PARAM aParam;
  POKEMON_TRADE_WORK *pWork;
  int i;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, HEAPSIZE_POKETRADE );
  pWork = GFL_PROC_AllocWork( proc, sizeof( POKEMON_TRADE_WORK ), HEAPID_IRCBATTLE );
  GFL_STD_MemClear(pWork, sizeof(POKEMON_TRADE_WORK));

  pParent->aParam.gamedata = pParent->gamedata;
  ret = PokemonTradeProcInit(proc, seq, &pParent->aParam, pWork, POKEMONTRADE_TYPE_EVENT);


  GFL_STD_MemCopy(pParent->pMyPoke, pWork->recvPoke[0] , POKETOOL_GetWorkSize());
  GFL_STD_MemCopy(pParent->pNPCPoke, pWork->recvPoke[1] , POKETOOL_GetWorkSize());

  IRCPOKETRADE_PokeCreateMcss(pWork, 0, 1, pParent->pMyPoke);
  IRCPOKETRADE_PokeCreateMcss(pWork, 1, 0, pParent->pNPCPoke);

  
  POKMEONTRADE_IRCDEMO_ChangeDemo(pWork);

  return ret;
}




//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT PokemonTradeProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMON_TRADE_WORK* pWork = mywk;
  GFL_PROC_RESULT retCode = GFL_PROC_RES_FINISH;
  StateFunc* state = pWork->state;

  if(DEBUGWIN_IsActive()){
    return GFL_PROC_RES_CONTINUE;
  }


  if(state != NULL){
    state(pWork);
    pWork->anmCount++;
    retCode = GFL_PROC_RES_CONTINUE;
  }
  //ポケモンセットをコール
  if(pWork->pokemonsetCall != 0){
    POKE_MAIN_Pokemonset(pWork, 1, pWork->recvPoke[pWork->pokemonsetCall-1]);
    pWork->pokemonsetCall=0;
  }

  GFL_CLACT_SYS_Main(); // CLSYSメイン
  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }

  if(pWork->pMsgTcblSys){
   u8 defL, defS, defB;
    GFL_FONTSYS_GetColor( &defL, &defS, &defB );
    GFL_FONTSYS_SetColor(FBMP_COL_BLACK, FBMP_COL_BLK_SDW, 15);
    GFL_TCBL_Main( pWork->pMsgTcblSys );
    GFL_FONTSYS_SetColor( defL, defS, defB );
    PRINTSYS_QUE_Main(pWork->SysMsgQue);
  }

  {
    static int backup;

    if(backup !=pWork->BoxScrollNum){
      OS_TPrintf("pWork->BoxScrollNum %d \n",pWork->BoxScrollNum);
    }
    backup = pWork->BoxScrollNum;
  }

  
  GFL_G3D_DRAW_Start();
  GFL_G3D_CAMERA_Switching( pWork->pCamera );

  GFL_G3D_DRAW_SetLookAt();

  MCSS_Main( pWork->mcssSys );
  MCSS_Draw( pWork->mcssSys );
  //IRC_POKETRADE_G3dDraw(pWork);

  IRC_POKETRADEDEMO_Main(pWork);

  GFL_G3D_DRAW_End();


  //  ConnectBGPalAnm_Main(&pWork->cbp);

  return retCode;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT PokemonTradeProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  int i;
  POKEMON_TRADE_WORK* pWork = mywk;

  POKMEONTRADE_RemoveCoreResource(pWork);

  DEBUGWIN_ExitProc();
  IRC_POKETRADE_ItemIconReset(&pWork->aItemMark);
  IRC_POKETRADE_ItemIconReset(&pWork->aPokerusMark);

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    GFL_NET_DelCommandTable(GFL_NET_CMD_IRCTRADE);
  }

  IRC_POKETRADEDEMO_End(pWork);

  GFL_HEAP_FreeMemory(pWork->recvPoke[0]);
  GFL_HEAP_FreeMemory(pWork->recvPoke[1]);

  GFL_TCB_DeleteTask( pWork->g3dVintr );


  _savedataHeapEnd(pWork,(pwk!=NULL));
  _mcssSystemHeapEnd(pWork);
  _dispSystemHeapEnd(pWork);

  GFL_PROC_FreeWork(proc);
#if PM_DEBUG
  GFL_HEAP_DEBUG_PrintExistMemoryBlocks(HEAPID_IRCBATTLE);
#endif// PM_DEBUG
  GFL_HEAP_DeleteHeap(HEAPID_IRCBATTLE);
  //オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(app_mail));

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------

// WIFICLUB用
const GFL_PROC_DATA PokemonTradeClubProcData = {
  PokemonTradeClubProcInit,
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};

//UNION用
const GFL_PROC_DATA PokemonTradeProcData = {
  PokemonTradeUnionNegoProcInit,
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};



//IR用
const GFL_PROC_DATA PokemonTradeIrcProcData = {
#if DEBUG_ONLY_FOR_ohno
//  PokemonTradeGTSNegoProcInit,
  PokemonTradeIrcProcInit,
#else
  PokemonTradeIrcProcInit,
#endif
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};

//wifiネゴシエーション用
const GFL_PROC_DATA PokemonTradeWiFiProcData = {
  PokemonTradeGTSNegoProcInit,
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};

//デモ起動用
const GFL_PROC_DATA PokemonTradeDemoProcData = {
  PokemonTradeDemoProcInit,
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};
