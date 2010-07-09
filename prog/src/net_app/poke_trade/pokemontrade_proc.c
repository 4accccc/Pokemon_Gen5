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

#include "system/wipe.h"

#include "msg/msg_poke_trade.h"
#include "msg/msg_chr.h"
#include "msg/msg_trade_head.h"
#include "msg/msg_wifi_system.h"


#include "ircbattle.naix"
#include "trade.naix"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"
#include "wb_trade_obj01_NANR_LBLDEFS.h"
#include "net/dwc_rapcommon.h"
#include "net/dwc_rap.h"

#include "net_app/pokemontrade.h"
#include "net_app/connect_anm.h"
#include "net_app/gtsnego.h"
#include "net/dwc_rapfriend.h"
#include "system/net_err.h"
#include "net/dwc_error.h"

#include "savedata/wifihistory.h"
#include "savedata/undata_update.h"

#include "item/item.h"

#include "app/app_menu_common.h"

#include "pokemontrade_local.h"
#include "waza_tool/wazano_def.h"
#include "field/field_skill_check.h"

#include "savedata/wifihistory_local.h"

//#define _ENDTABLE  {192-32, 192, 256-32, 256}     //終了
//#define _SEARCHTABLE  {192-32, 192, 0, 32}        //検索ボタン

#include "pokemontrade.cdat"




static void _recvSelectPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvChangePokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvEnd(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);

static void _touchState(POKEMON_TRADE_WORK* pWork);
static void _touchStated(POKEMON_TRADE_WORK* pWork);
static u8* _setChangePokemonBuffer(int netID, void* pWork, int size);
static void _changeWaitState(POKEMON_TRADE_WORK* pWork);
static void _endWaitState(POKEMON_TRADE_WORK* pWork);
static BOOL _PokemonsetAndSendData(POKEMON_TRADE_WORK* pWork);
static void _recvFriendScrollBar(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _changeFinish(POKEMON_TRADE_WORK* pWork);
static void _networkFriendsStandbyWait3(POKEMON_TRADE_WORK* pWork);
static void _networkFriendsStandbyWait2(POKEMON_TRADE_WORK* pWork);
static BOOL _IsBothPokemonSelect(POKEMON_TRADE_WORK* pWork,int boxno , int selectIndex);
static void _DeletePokemonState(POKEMON_TRADE_WORK* pWork);

//static void _endCancelState(POKEMON_TRADE_WORK* pWork);
static void _recvLookAtPoke(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvEggAndBattle(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _cancelPokemonSendDataNetwork(POKEMON_TRADE_WORK* pWork);
static void _scrollMainFunc(POKEMON_TRADE_WORK* pWork,BOOL bSE, BOOL bNetSend);
static void _recvCancelPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _PokemonReset(POKEMON_TRADE_WORK *pWork, int side );
static void _changeMenuOpen(POKEMON_TRADE_WORK* pWork);
static BOOL checkTouchCLACTPosition(POKEMON_TRADE_WORK* pWork, BOOL bCatch);
static int _boxScrollLine2Num(int line);
static void _notWazaChangePoke2(POKEMON_TRADE_WORK* pWork);

static u8* _setThreePokemon(int netID, void* pWk, int size);
static void _recvThreePokemon1(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvThreePokemon2(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvThreePokemon3(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvThreePokemonEnd(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvThreePokemonCancel(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _scrollResetAndIconReset(POKEMON_TRADE_WORK* pWork);
static void _recvFriendFaceIcon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvPokemonColor(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static u8* _setPokemonColorBuffer(int netID, void* pWk, int size);
static void _recvSeqNegoNo(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _gtsFirstMsgState(POKEMON_TRADE_WORK* pWork);
static void _recvFriendBoxNum(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvChangeFactor(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvEvilCheck(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvUNData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvGTSSelectPokemonIndex(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _dispSubStateWait(POKEMON_TRADE_WORK* pWork);
static void _touchPreMode_TouchStateCommon(POKEMON_TRADE_WORK* pWork);
static void _recvThreePokemon1Box(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvThreePokemon2Box(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvThreePokemon3Box(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvSelectPokemonBox(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvPokemonDataReturn(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);

static void _createEasyPokeInfo(POKEMON_TRADE_WORK *pWork,int index);

static void CalcLineAfterLR(POKEMON_TRADE_WORK* pWork, const BOOL bChange); //20100630 add Saito

///通信コマンドテーブル
static const NetRecvFuncTable _PacketTbl[] = {
  {_recvChangeFactor, NULL },   ///< _NETCMD_CHANGEFACTOR
  {_recvSelectPokemon,   _setChangePokemonBuffer},    ///_NETCMD_SELECT_POKEMON 2 ポケモン見せ合う
  {_recvChangePokemon,   NULL},    ///_NETCMD_CHANGE_POKEMON   3 こうかんけってい
  {_recvGTSSelectPokemonIndex, NULL },     ///_NETCMD_POKEMONSELECT_GTS
  {_recvLookAtPoke, NULL},       ///_NETCMD_LOOKATPOKE   2 ポケモン見せ合う
  {_recvEnd, NULL},         //_NETCMD_END           1 おわり
  {_recvCancelPokemon, NULL},         //  _NETCMD_CANCEL_POKEMON
  {_recvThreePokemon1,   _setThreePokemon},    ///_NETCMD_THREE_SELECT1 ポケモン３匹みせあい
  {_recvThreePokemon2,   _setThreePokemon},    ///_NETCMD_THREE_SELECT2 ポケモン３匹みせあい
  {_recvThreePokemon3,   _setThreePokemon},    ///_NETCMD_THREE_SELECT3 ポケモン３匹みせあい
  {_recvThreePokemonEnd,   NULL},    ///_NETCMD_THREE_SELECT_END ポケモン３匹みせあい
  {_recvThreePokemonCancel,   NULL},    ///_NETCMD_THREE_SELECT_CANCEL キャンセル
  {_recvFriendScrollBar, NULL}, //_NETCMD_SCROLLBAR
  {_recvFriendFaceIcon, NULL},//_NETCMD_FACEICON
  {_recvPokemonColor, _setPokemonColorBuffer},//_NETCMD_POKEMONCOLOR
  {_recvSeqNegoNo,   NULL},    ///_NETCMD_GTSSEQNO
  {_recvFriendBoxNum,   NULL},   ///_NETCMD_FRIENDBOXNUM
  {_recvEvilCheck,   NULL},    ///_NETCMD_EVILCHECK
  {_recvUNData,   NULL},    ///_NETCMD_UN
  {_recvThreePokemon1Box,   NULL},    ///_NETCMD_THREE_SELECT1_BOX ポケモン３匹みせあい BOXか手持ちか
  {_recvThreePokemon2Box,   NULL},    ///_NETCMD_THREE_SELECT2_BOX ポケモン３匹みせあい BOXか手持ちか
  {_recvThreePokemon3Box,   NULL},    ///_NETCMD_THREE_SELECT3_BOX ポケモン３匹みせあい BOXか手持ちか
  {_recvSelectPokemonBox,   NULL},    ///_NETCMD_SELECT_POKEMON_BOX 2 ポケモン見せ合う BOXか手持ちか
  {_recvPokemonDataReturn,   NULL},   ///_NETCMD_SELECT_RETURN  ポケモンデータを受け取った事を返す

};

void POKEMONTRADE_TOUCHBAR_SetReturnIconActiveTRUE(POKEMON_TRADE_WORK* pWork)
{
  TOUCHBAR_ReStartEx(pWork->pTouchWork, TOUCHBAR_ICON_RETURN);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, TRUE);
  TOUCHBAR_SetActive(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, TRUE);
}



// メールボックスがフルかどうか
static BOOL _IsMailBoxFull(POKEMON_TRADE_WORK* pWork)
{
  int id;
  int itemno = ITEM_DUMMY_DATA;
  MAIL_DATA* src = NULL;
  MAIL_BLOCK* pMailBlock = GAMEDATA_GetMailBlock(pWork->pGameData);

  //空き領域検索
  id = MAIL_SearchNullID(pMailBlock,MAILBLOCK_PASOCOM);
  if(id == MAILDATA_NULLID){
    return TRUE;
  }
  return FALSE;
}


void POKEMONTRADE_CancelCall(POKEMON_TRADE_WORK* pWork)
{
  GFL_NET_SetAutoErrorCheck(FALSE);
  GFL_NET_SetNoChildErrorCheck(FALSE);
  GFL_NET_StateWifiMatchEnd(FALSE);
  pWork->pParentWork->ret = POKEMONTRADE_MOVE_END;

}

BOOL POKEMONTRADE_IsInPokemonRecvPoke(POKEMON_PARAM* pp)
{
  u8* data = (u8*)pp;
  int i,size = POKETOOL_GetWorkSize();

  if(!pp){
    return FALSE;
  }

  for(i=0;i<size;i++){
    if(data[i] != 0){
      return TRUE;
    }
  }
  return FALSE;
}

//ともだちのボックス数
int POKEMONTRADE_GetFriendBoxNum(POKEMON_TRADE_WORK* pWork)
{
  return pWork->friendBoxNum;
}

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
  if(pWork->type>=POKEMONTRADE_TYPE_VISUAL){
    return FALSE;
  }
  if(GFL_NET_IsInit()){
    return TRUE;
  }
  return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief   デモに入る前にBGM変更
 * @param   POKEMON_TRADE_WORK ワーク
 * @param   _TRADE_SCENE_NO_E no シーン管理enum
 * @retval  none
 */
//------------------------------------------------------------------

void POKEMONTRADE_DEMOBGMChange(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->demoBGM!=0){
    pWork->demoBGM++;
    if(pWork->demoBGM==2){
      PMSND_FadeOutBGM( PMSND_FADE_FAST );
    }
    else if(pWork->demoBGM==8){
      PMSND_PauseBGM( TRUE );
      PMSND_PushBGM();
      pWork->pushSound++;
      NET_PRINT("pWork->pushSound%d\n",pWork->pushSound);
      PMSND_PlayBGM(  SEQ_BGM_KOUKAN );
      PMSND_FadeInBGM( 1 );
      pWork->demoBGM=0;
    }
  }
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
  case POKEMONTRADE_TYPE_GTSMID:
  case POKEMONTRADE_TYPE_GTSUP:
  case POKEMONTRADE_TYPE_GTSDOWN:
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


//------------------------------------------------------------------
/**
 * @brief   時間の負荷をかけるかどうかのモードかどうか
 * @param   POKEMON_TRADE_WORK
 * @retval  TRUEならwait
 */
//------------------------------------------------------------------

BOOL POKEMONTRADEPROC_IsTimeWaitSelect(POKEMON_TRADE_WORK* pWork)
{
  BOOL ret = FALSE;

  switch(pWork->type){
  case POKEMONTRADE_TYPE_GTSNEGO: ///< GTSネゴシエーション
  case POKEMONTRADE_TYPE_GTSNEGODEMO:
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

static void _CatchPokemonPositionActive(POKEMON_TRADE_WORK *pWork,GFL_CLWK* pCL,int Ringline ,int index,POKEMON_PASO_PARAM*ppp)
{
  //  int pltNum;


//  NET_PRINT("つかみその２\n");
  pWork->pCatchCLWK = pCL;
//  pWork->pSelectCLWK = pCL;
  GFL_CLACT_WK_GetPos(pWork->pCatchCLWK, &pWork->aCatchOldPos, CLSYS_DRAW_SUB);

  POKMEONTRADE2D_IconGray(pWork, pWork->pCatchCLWK, TRUE);

  {
    u32 x,y;
    GFL_CLACTPOS pos;
    if(GFL_UI_TP_GetPointCont(&x,&y)){

      pWork->aDifferencePos.x = pWork->aCatchOldPos.x - x;
      pWork->aDifferencePos.y = pWork->aCatchOldPos.y - y;

  //    NET_PRINT("POSX %d %d %d\n",x,pWork->aDifferencePos.x,pWork->aCatchOldPos.x);
  //    NET_PRINT("POSY %d %d %d\n",y,pWork->aDifferencePos.y,pWork->aCatchOldPos.y);
  //    NET_PRINT("lineindex %d %d \n",Ringline , index);
      pos.x = x + pWork->aDifferencePos.x;
      pos.y = y + pWork->aDifferencePos.y;

      POKEMONTRADE_StartCatched( pWork, Ringline, index, x + pWork->aDifferencePos.x, y + pWork->aDifferencePos.y,
                                 ppp);

      pWork->pCatchCLWK = pWork->curIcon[CELL_CUR_POKE_KEY];
   //   pWork->pSelectCLWK = pWork->curIcon[CELL_CUR_POKE_KEY];

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


POKEMON_PARAM* POKEMONTRADE_CreatePokeData(BOX_MANAGER* boxData , int trayNo, int index,POKEMON_TRADE_WORK* pWork)
{
  POKEMON_PARAM* pp = NULL;

  if(trayNo!=-1){
    if(trayNo > pWork->BOX_TRAY_MAX){
      return NULL;
    }
    if(trayNo != pWork->BOX_TRAY_MAX && (index != -1)){
      pp = PP_CreateByPPP(BOXDAT_GetPokeDataAddress(boxData,trayNo,index),pWork->heapID);
    }
    else{
      POKEPARTY* party = pWork->pMyParty;
      if(index < PokeParty_GetPokeCount(party) && (index != -1)){
        pp = PP_Create(1,1,1,pWork->heapID);
        GFL_STD_MemCopy( PokeParty_GetMemberPointer( party , index ), pp, POKETOOL_GetWorkSize());
      }
    }
  }
  return pp;
}


POKEMON_PASO_PARAM* IRCPOKEMONTRADE_GetPokeDataAddress(BOX_MANAGER* boxData , int trayNo, int index,POKEMON_TRADE_WORK* pWork)
{
  POKEMON_PASO_PARAM* ppp;

  if(trayNo!=-1){
    if(trayNo > pWork->BOX_TRAY_MAX){
      return NULL;
    }
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

POKE_EASY_INFO* IRCPOKEMONTRADE_GetPokeInfo(int trayNo, int index, POKEMON_TRADE_WORK* pWork)
{
  POKE_EASY_INFO *info;
  int one_tray_pokenum = BOX_MAX_COLUMN*BOX_MAX_RAW;

  if(trayNo!=-1){
    if(trayNo > pWork->BOX_TRAY_MAX){
      NOZOMU_Printf("NULL trayno = %d\n",trayNo);
      return NULL;
    }
    if(trayNo != pWork->BOX_TRAY_MAX && (index != -1)){
      //ボックス
      return &pWork->pokeInfo[6+trayNo*one_tray_pokenum+index];
    }
    else{
      //手持ち
      if ( (0 <= index) && (index < 6) ) return &pWork->pokeInfo[index];
      else{
        NOZOMU_Printf("ERROR index %d\n",index);
        return NULL;
      }
    }
  }
  NOZOMU_Printf("_NULL trayno = -1\n");
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
  //GFL_STD_MemClear(pWork->userNetCommand,sizeof(pWork->userNetCommand));
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

void IRC_POKMEONTRADE_changeState(POKEMON_TRADE_WORK* pWork,StateFunc state)
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
//    return (u8*)pWork->recvPoke[netID];
    return (u8*)pWork->TempBuffer[netID];
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
 * @brief   ポケモンカラーの受信バッファを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static u8* _setPokemonColorBuffer(int netID, void* pWk, int size)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return (u8*)pWork->FriendPokemonCol[0];
  }
  return (u8*)pWork->FriendPokemonCol[1];
}



//_NETCMD_GTSSEQNO
static void _recvSeqNegoNo(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  const u8* pBuff = pData;
  int i;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    i = 0;
  }
  else{
    i = 1;
  }
  //  POKE_GTS_SelectStatusMessageDisp(pWork, i, pBuff[0]);

  pWork->pokemonGTSSeq[i] = pBuff[0];

}

//------------------------------------------------------------------------------
/**
 * @brief   自分と相手でindex指定でボックスから選んだかどうかを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
BOOL IRC_POKEMONTRADE_GetRecvBoxFlg(POKEMON_TRADE_WORK *pWork, int index)
{
  BOOL bMode = POKEMONTRADEPROC_IsNetworkMode(pWork);
  GF_ASSERT(index < 2);

  if(bMode==FALSE){
    return pWork->bRecvPokeBox[index];
  }
  if(0 == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return pWork->bRecvPokeBox[index];
  }
  return pWork->bRecvPokeBox[1-index];
}

//------------------------------------------------------------------------------
/**
 * @brief   自分と相手でindex指定でボックスから選んだかどうかを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
void IRC_POKEMONTRADE_SetRecvBoxFlg(POKEMON_TRADE_WORK *pWork, int index, BOOL bFlg)
{
  BOOL bMode = POKEMONTRADEPROC_IsNetworkMode(pWork);
  GF_ASSERT(index < 2);

  if(bMode==FALSE){
    pWork->bRecvPokeBox[index]=bFlg;
  }
  else if(0 == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    pWork->bRecvPokeBox[index]=bFlg;
  }
  else{
    pWork->bRecvPokeBox[1-index]=bFlg;
  }
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
  GFL_STD_MemCopy(pWork->TempBuffer[netID], pWork->recvPoke[netID], POKETOOL_GetWorkSize());

  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }

  //ポケモンセットをコール  処理が重いためここではやらない
  pWork->pokemonsetCall = netID+1;
  pWork->pokemonEnableSendFlg=TRUE;//ポケモンを受け取ったという事を相手に送信する

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
    POKETRADE_2D_GTSPokemonIconSet(pWork, 1, num, pWork->TempBuffer[netID],TRUE, FALSE);
    POKEMONTRADE_NEGO_SlideInit(pWork, 1, pWork->TempBuffer[netID]);
    
  }
  else{
    GF_ASSERT(0);
  }
  pWork->pokemonEnableSendFlg=TRUE;//ポケモンを受け取ったという事を相手に送信する
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


static void _recvThreePokemonBoxRecv(u8 bFlg, u8 num, int netID,void* pWk,GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    pWork->bGTSSelectPokeBox[0][num] = bFlg;
  }
  else{
    pWork->bGTSSelectPokeBox[1][num] = bFlg;
  }
}


//_NETCMD_THREE_SELECT1_BOX ポケモン３匹みせあい BOXか手持ちか
static void _recvThreePokemon1Box(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  const u8* pRecvData = pData;

  _recvThreePokemonBoxRecv(pRecvData[0], 0, netID, pWk, pNetHandle);
}

//_NETCMD_THREE_SELECT2_BOX ポケモン３匹みせあい BOXか手持ちか
static void _recvThreePokemon2Box(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  const u8* pRecvData = pData;

  _recvThreePokemonBoxRecv(pRecvData[0], 1, netID, pWk, pNetHandle);
}
//_NETCMD_THREE_SELECT3_BOX ポケモン３匹みせあい BOXか手持ちか
static void _recvThreePokemon3Box(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  const u8* pRecvData = pData;

  _recvThreePokemonBoxRecv(pRecvData[0], 2, netID, pWk, pNetHandle);
}

//_NETCMD_SELECT_POKEMON_BOX 2 ポケモン見せ合う BOXか手持ちか
static void _recvSelectPokemonBox(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  const u8* pRecvData = pData;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  pWork->bRecvPokeBox[netID] = pRecvData[0];
}

//はじいている＋相手に見せる ポケモンを受け取った事を返す//_NETCMD_SELECT_RETURN
static void _recvPokemonDataReturn(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  const u8* pRecvData = pData;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }
  pWork->pokemonSendDisableFlg = FALSE;  //このフラグがTRUEの時は送信不可
}

//_NETCMD_POKEMONCOLOR
static void _recvPokemonColor(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  const u8* pRecvData = pData;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }
  //すでに受信済み
}

//_NETCMD_FRIENDBOXNUM
static void _recvFriendBoxNum(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  const u8* pRecvData = pData;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }
  pWork->friendBoxNum = pRecvData[0];
  pWork->friendMailBoxFULL = pRecvData[1];
  pWork->FriendBoxScrollNum = pWork->friendBoxNum*_BOXTRAY_MAX + _TEMOTITRAY_MAX - 80;
  NET_PRINT("メールボックス%d\n",pWork->friendMailBoxFULL);
}




//_NETCMD_EVILCHECK
static void _recvEvilCheck(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  const u8* pRecvData = pData;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }
  pWork->evilCheck[1] = pRecvData[0];
}

//_NETCMD_UN


static void _recvUNData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  const u8* pRecvData = pData;
  UNITEDNATIONS_SAVE aUN;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }
  { //国連データ格納
    WIFI_HISTORY* pWH = SaveData_GetWifiHistory(GAMEDATA_GetSaveControlWork(pWork->pGameData));
    UNITEDNATIONS_SAVE* pUN = (UNITEDNATIONS_SAVE*)pData;
    int nation,nation2;
    int area,area2;

    GFL_STD_MemCopy( &aUN , pUN, sizeof(UNITEDNATIONS_SAVE));

    nation = MyStatus_GetMyNation(&aUN.aMyStatus);
    area = MyStatus_GetMyArea(&aUN.aMyStatus);
    nation2 = WIFI_COUNTRY_GetNGTestCountryCode(nation, area, MyStatus_GetRegionCode(&aUN.aMyStatus));
    area2 = WIFI_COUNTRY_GetNGTestLocalCode(nation, area, MyStatus_GetRegionCode(&aUN.aMyStatus));

    if((nation == nation2) && (area == area2)){
      MyStatus_SetMyNationArea( &aUN.aMyStatus, nation, area);
    }
    else{
      MyStatus_SetMyNationArea( &aUN.aMyStatus, 0, 0);
    }
    UNDATAUP_Update(pWH, (UNITEDNATIONS_SAVE*)&aUN);
  }
}



//_NETCMD_CHANGEFACTOR
static void _recvChangeFactor(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  const u8* pRecvData = pData;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  // 保存
  pWork->changeFactor[netID] = pRecvData[0];
  OS_TPrintf("factor %d %d\n",netID , pWork->changeFactor[netID]);
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
  //  pWork->userNetCommand[netID] = 0;

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

//------------------------------------------------------------------------------
/**
 * @brief   相手の顔アイコン情報が送られてきた
 *          _NETCMD_SCROLLBAR
 * @retval  none
 */
//------------------------------------------------------------------------------

//
static void _recvFriendFaceIcon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  u8* pRecvData = (u8*)pData;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_SystemGetCurrentID()){
    return; //自分のデータは要らない
  }
  POKE_GTS_InitEruptedIcon(pWork, pRecvData[0], 1);
}





//_NETCMD_LOOKATPOKE
static void _recvLookAtPoke(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  //  pWork->userNetCommand[netID] = _NETCMD_LOOKATPOKE;
}

//_NETCMD_POKEMONSELECT_GTS
static void _recvGTSSelectPokemonIndex(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  u8* pBuff = (u8*)pData;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_SystemGetCurrentID()){
    return; //自分のデータは要らない
  }
  pWork->pokemonselectnoGTS = pBuff[0];
}


//_NETCMD_END
static void _recvEnd(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }

  //  pWork->userNetCommand[netID] = _NETCMD_END;
}


// _NETCMD_CHANGE_POKEMON
static void _recvChangePokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  //  pWork->userNetCommand[netID] = _NETCMD_CHANGE_POKEMON;

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



static void _noneState(POKEMON_TRADE_WORK* pWork)
{
  //なにもしない
}




//--------------------------------------------------------------
//  ポケモン表示セット
//--------------------------------------------------------------
void POKE_MAIN_Pokemonset(POKEMON_TRADE_WORK *pWork, int side, POKEMON_PARAM* pp )
{
  if(pp){
    IRCPOKETRADE_PokeDeleteMcss(pWork, side);
    IRCPOKETRADE_PokeCreateMcssGTS(pWork, side, 1-side, pp, TRUE, POKEMONTRADEPROC_IsTriSelect(pWork));
    POKETRADE_MESSAGE_SetPokemonStatusMessage(pWork,side ,pp);
  }
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





BOOL POKEMONTRADE_IsEggAndLastBattlePokemonChange(POKEMON_TRADE_WORK* pWork)
{
  BOOL bBox;
  POKEMON_PARAM* pp;//1 = IRC_POKEMONTRADE_GetRecvPP(pWork, 0); //自分のポケモン
  //POKEMON_PARAM* pp;//2 = IRC_POKEMONTRADE_GetRecvPP(pWork, 1); //相手のポケモン
  //  BOOL bEgg1 = PP_Get(pp1,ID_PARA_tamago_flag,NULL);
  //  BOOL bEgg2 = PP_Get(pp2,ID_PARA_tamago_flag,NULL);

  if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
    pp = IRC_POKEMONTRADE_GetRecvPP(pWork, 0); //自分のポケモン
    bBox = IRC_POKEMONTRADE_GetRecvBoxFlg(pWork, 0); //自分の交換に出す位置
  }
  else{
    pp = IRC_POKEMONTRADE_GetRecvPP(pWork, 1);//相手のポケモン
    bBox = IRC_POKEMONTRADE_GetRecvBoxFlg(pWork, 1); //自分の交換に出す位置
  }

  if (PP_Get(pp, ID_PARA_hp, NULL) == 0) {
    return FALSE;
  }
  if (PP_Get(pp, ID_PARA_tamago_flag, NULL)) {
    return FALSE;
  }
  //てもち
  if(1==PokeParty_GetBattlePokeNum(pWork->pMyParty) && (bBox)  ){
    //  if(1==PokeParty_GetBattlePokeNum(pWork->pMyParty) && !bEgg1 && bEgg2 && (pWork->selectBoxno == pWork->BOX_TRAY_MAX)){
    return TRUE;
  }
  return FALSE;
}



static BOOL POKEMONTRADE_IsWazaPokemon(POKEMON_TRADE_WORK* pWork,int boxno,int index)
{
  BOOL flg = FALSE;
  POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox,
                                                               boxno,
                                                               index,pWork);

  if(pWork->type != POKEMONTRADE_TYPE_IRC){
    return FALSE;
  }
  if(boxno != pWork->BOX_TRAY_MAX){
    return FALSE;
  }
  flg = FIELD_SKILL_CHECK_CanTradePoke( ppp,0 );

  return !flg;
}


static BOOL _pokemonIllegalCheck(POKEMON_PARAM* pp)
{
  int mons_no;
  int form_no;

  if (PP_Get(pp, ID_PARA_fusei_tamago_flag, NULL) == 1) {
    return TRUE;
  }
  mons_no = PP_Get( pp, ID_PARA_monsno, NULL  );
  form_no = PP_Get( pp, ID_PARA_form_no, NULL  );
  if( form_no != POKETOOL_CheckPokeFormNo( mons_no, form_no ) ){
    return TRUE;
  }
  return FALSE;

}


BOOL POKEMONTRADE_IsIllegalPokemon(POKEMON_TRADE_WORK* pWork)
{
  int no = 0;
  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    no=1;
  }
  return _pokemonIllegalCheck(IRC_POKEMONTRADE_GetRecvPP(pWork, no));
}

BOOL POKEMONTRADE_IsIllegalPokemonFriend(POKEMON_TRADE_WORK* pWork)
{
  int no = 1;
  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    no=0;
  }
  return _pokemonIllegalCheck(IRC_POKEMONTRADE_GetRecvPP(pWork, no));
}

BOOL POKEMONTRADE_IsMailPokemon(POKEMON_TRADE_WORK* pWork)
{
  int item;
  BOOL bBox;
  POKEMON_PARAM* pp;

  if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
    pp = IRC_POKEMONTRADE_GetRecvPP(pWork, 0); //自分のポケモン
    bBox = IRC_POKEMONTRADE_GetRecvBoxFlg(pWork, 1); //相手のコピー位置
  }
  else{
    pp = IRC_POKEMONTRADE_GetRecvPP(pWork, 1);//相手のポケモン
    bBox = IRC_POKEMONTRADE_GetRecvBoxFlg(pWork, 0); //自分のコピー位置
  }

  item = PP_Get( pp , ID_PARA_item ,NULL);

  //OS_TPrintf("メール%d FULL%d BOX%d \n", ITEM_CheckMail(item), pWork->friendMailBoxFULL, bBox);
  if(ITEM_CheckMail(item)){
    if(pWork->friendMailBoxFULL && !bBox){
      return TRUE;
    }
  }
  return FALSE;
}

//交換選択通信送信



static void POKETRE_MAIN_ChangePokemonSendDataNetwork2(POKEMON_TRADE_WORK* pWork)
{
  BOOL bSend = FALSE;
  BOOL bMode = POKEMONTRADEPROC_IsNetworkMode(pWork);

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }

  if( bMode ){
    if(POKEMONTRADE_IsEggAndLastBattlePokemonChange(pWork)){
      u8 cmd = POKETRADE_FACTOR_EGG;
      bSend=GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, 1, &cmd);
    }
    else if(POKEMONTRADE_IsIllegalPokemon(pWork)){
      u8 cmd = POKETRADE_FACTOR_ILLEGAL;
      bSend=GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, 1, &cmd);
    }
    else if(POKEMONTRADE_IsIllegalPokemonFriend(pWork)){
      u8 cmd = POKETRADE_FACTOR_ILLEGAL_FRIEND;
      bSend=GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, 1, &cmd);
    }
    else if(POKEMONTRADE_IsMailPokemon(pWork)){
      u8 cmd = POKETRADE_FACTOR_MAIL;
      bSend=GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, 1, &cmd);
    }
    else{
      u8 cmd = POKETRADE_FACTOR_SINGLE_OK;
      bSend=GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, 1, &cmd);
    }
    if(bSend){
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_C,WB_NET_TRADE_SERVICEID);
      _CHANGE_STATE(pWork,_changeWaitState);
    }
  }
  else{
    pWork->changeFactor[0]=POKETRADE_FACTOR_SINGLE_OK;
    pWork->changeFactor[1]=POKETRADE_FACTOR_SINGLE_OK;
    _CHANGE_STATE(pWork,_changeWaitState);
  }
}


//交換へ遷移 メッセージだけ先に表示
void POKETRE_MAIN_ChangePokemonSendDataNetwork(POKEMON_TRADE_WORK* pWork)
{
  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_09, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
  _CHANGE_STATE(pWork,POKETRE_MAIN_ChangePokemonSendDataNetwork2);
}




static void _changePokemonStatusDispAuto(POKEMON_TRADE_WORK* pWork,int sel, BOOL bRenew)
{
  if(sel == 0){
    GFL_CLACTPOS pos = {_POKEMON_SELECT1_CELLX,_POKEMON_SELECT1_CELLY+4};
    GFL_CLACT_WK_SetPos( pWork->curIcon[CELL_CUR_POKE_SELECT], &pos, CLSYS_DRAW_SUB );
  }
  else{
    GFL_CLACTPOS pos = {_POKEMON_SELECT2_CELLX,_POKEMON_SELECT2_CELLY+4};
    GFL_CLACT_WK_SetPos( pWork->curIcon[CELL_CUR_POKE_SELECT], &pos, CLSYS_DRAW_SUB );
  }

  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
    IRC_POKETRADE_PosChangeSubStatusIcon(pWork, sel,FALSE);
    GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_POKE_SELECT], FALSE );
  }
  else{
    IRC_POKETRADE_PosChangeSubStatusIcon(pWork, sel,TRUE);
    GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_POKE_SELECT] , TRUE );
  }
  if(bRenew){
    POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, pWork->pokemonselectno);
    POKETRADE_MESSAGE_ChangePokemonStatusDisp(pWork,pp, pWork->pokemonselectno ,TRUE);
  }
}


static void _networkFriendsStandbyWaitFadeout(POKEMON_TRADE_WORK* pWork)
{
  if(WIPE_SYS_EndCheck()){
    GFL_BG_SetVisible( GFL_BG_FRAME3_M , FALSE );

    POKETRADE_MESSAGE_ResetPokemonStatusMessage(pWork, TRUE); //20100624 mod Saito BTS6424

    GFL_BG_SetVisible( GFL_BG_FRAME0_S , FALSE );
    IRC_POKETRADE_GraphicInitMainDisp(pWork);
    IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
    G2_BlendNone();
    POKE_MAIN_Pokemonset(pWork, 0, IRC_POKEMONTRADE_GetRecvPP(pWork, 0));
    POKE_MAIN_Pokemonset(pWork, 1, IRC_POKEMONTRADE_GetRecvPP(pWork, 1));
    _PokemonIconRenew(pWork);
    WIPE_SYS_Start( WIPE_PATTERN_M , WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN ,
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait3);
  }
}


const static GFL_UI_TP_HITTBL _tp_data[]={
  {_POKEMON_SELECT1_CELLY-8,_POKEMON_SELECT1_CELLY+16,_POKEMON_SELECT1_CELLX-8, _POKEMON_SELECT1_CELLX+16},
  {_POKEMON_SELECT2_CELLY-8,_POKEMON_SELECT2_CELLY+16,_POKEMON_SELECT2_CELLX-8, _POKEMON_SELECT2_CELLX+16},
  {GFL_UI_TP_HIT_END,0,0,0},
};


// ポケモンのステータス表示待ち
static void _pokemonStatusWait(POKEMON_TRADE_WORK* pWork)
{
  BOOL bReturn=FALSE;
  int line;

  if ( pWork->vtask ) return;    //20100603 add saito
  if( WIPE_SYS_EndCheck() == FALSE ) return;    //20100603 add saito

  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){  // 最初にキーが入った場合
    if(GFL_UI_KEY_GetTrgAndSet()){

      GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
      _changePokemonStatusDispAuto(pWork, pWork->pokemonselectno, FALSE);

      return;
    }
  }

  GFL_BG_SetVisible( GFL_BG_FRAME3_M , TRUE );

  switch(GFL_UI_TP_HitTrg(_tp_data)){
  case 0:
    if(pWork->pokemonselectno!=0  || (GFL_UI_CheckTouchOrKey()==GFL_APP_END_KEY)){
      pWork->pokemonselectno = 0;
      GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
      PMSND_PlaySystemSE(POKETRADESE_CUR);
      _changePokemonStatusDispAuto(pWork,pWork->pokemonselectno,TRUE);
    }
    break;
  case 1:
    if(pWork->pokemonselectno!=1 || (GFL_UI_CheckTouchOrKey()==GFL_APP_END_KEY)){
      pWork->pokemonselectno = 1;
      GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
      PMSND_PlaySystemSE(POKETRADESE_CUR);
      _changePokemonStatusDispAuto(pWork,pWork->pokemonselectno,TRUE);
    }
    break;
  case 2:
    bReturn=TRUE;
    break;
  default:
    break;
  }

  if(GFL_UI_KEY_GetTrgAndSet()==PAD_KEY_LEFT || GFL_UI_KEY_GetTrgAndSet()==PAD_KEY_RIGHT){
    pWork->pokemonselectno = 1 - pWork->pokemonselectno;
    PMSND_PlaySystemSE(POKETRADESE_CUR);
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
    _changePokemonStatusDispAuto(pWork,pWork->pokemonselectno,TRUE);
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
    WIPE_SYS_Start( WIPE_PATTERN_M , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
    _CHANGE_STATE(pWork, _networkFriendsStandbyWaitFadeout);
  }
}


// ポケモンのステータス表示
static void _pokemonStatusStart(POKEMON_TRADE_WORK* pWork)
{
  POKEMON_PARAM* pp;
  if(WIPE_SYS_EndCheck()){
    pWork->pokemonselectno = 0; // 自分のから表示
    pp = IRC_POKEMONTRADE_GetRecvPP(pWork, pWork->pokemonselectno);
    POKETRADE_MESSAGE_CreatePokemonParamDisp(pWork,pp);
    _PokemonIconRenew(pWork);
    WIPE_SYS_Start( WIPE_PATTERN_M , WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN ,
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

    _CHANGE_STATE(pWork, _pokemonStatusWait);
  }
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
      WIPE_SYS_Start( WIPE_PATTERN_M , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                      WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
      _CHANGE_STATE(pWork, _pokemonStatusStart);
      break;
    case 2:  //もどる
      _CHANGE_STATE(pWork, _cancelPokemonSendDataNetwork);
      break;
    }
  }
}



//こうかんにだす 通信相手の準備待ち
static void _networkFriendsStandbyWait3(POKEMON_TRADE_WORK* pWork)
{
  int i;

    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_20, pWork->pMessageStrBufEx );
    for(i=0;i<2;i++){
      POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, i);
      WORDSET_RegisterPokeNickName( pWork->pWordSet, i,  pp );
    }
    WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBuf, pWork->pMessageStrBufEx  );
    POKETRADE_MESSAGE_WindowOpen(pWork);
  _CHANGE_STATE(pWork,_networkFriendsStandbyWait2);
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


static void _preFadeOut2(POKEMON_TRADE_WORK* pWork)
{

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(GFL_NET_HANDLE_IsTimeSync( GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_RETURN2,WB_NET_TRADE_SERVICEID )){
      POKETRADE_MESSAGE_WindowClear(pWork);
      _CHANGE_STATE(pWork,POKEMONTRADE_PROC_FadeoutStart);
    }
  }
  else{
    POKETRADE_MESSAGE_WindowClear(pWork);
    _CHANGE_STATE(pWork,POKEMONTRADE_PROC_FadeoutStart);
  }
}


static void _preFadeOut(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(pWork->anmCount > 30){
    if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
      GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_RETURN2,WB_NET_TRADE_SERVICEID );
    }
    _CHANGE_STATE(pWork,_preFadeOut2);
  }
}

//こうかんにだす　通信相手の準備待ち
static void _networkFriendsStandbyWait(POKEMON_TRADE_WORK* pWork)
{
  int i;
  int msgno;
  int myID = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  int targetID = 1 - myID;

  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    myID=0;
    targetID=1;
  }


  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_B, WB_NET_TRADE_SERVICEID)){
      POKE_GTS_CreateCancelButton(pWork);  //GTSネゴのみ強制切断処理
      return;
    }
  }
  POKE_GTS_DeleteCancelButton(pWork,TRUE,TRUE);

  if((pWork->changeFactor[myID]==POKETRADE_FACTOR_SINGLECHANGE) &&
     (pWork->changeFactor[targetID]==POKETRADE_FACTOR_SINGLECHANGE)){
    _CHANGE_STATE(pWork,_networkFriendsStandbyWait3);
    return;
  }
  else if((pWork->changeFactor[myID]==POKETRADE_FACTOR_END) &&
          (pWork->changeFactor[targetID]==POKETRADE_FACTOR_END)){
    pWork->pParentWork->ret = POKEMONTRADE_MOVE_END;
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_08, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
    pWork->anmCount=0;
    _CHANGE_STATE(pWork, _preFadeOut);
    return;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_END){
    msgno = POKETRADE_STR_97;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_END){
    msgno = POKETRADE_STR2_04;
  }
  GFL_MSG_GetString( pWork->pMsgData, msgno, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->changeFactor[0] = POKETRADE_FACTOR_NONE;
  pWork->changeFactor[1] = POKETRADE_FACTOR_NONE;
  //  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_A,WB_NET_TRADE_SERVICEID);
  _CHANGE_STATE(pWork, POKEMONTRAE_EndCancelState);
}



// ポケモンデータを相手に送る
static void _pokeSendDataState(POKEMON_TRADE_WORK* pWork)
{
  BOOL bMode = POKEMONTRADEPROC_IsNetworkMode(pWork);

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }


  if( !bMode ){
    pWork->changeFactor[0]=POKETRADE_FACTOR_SINGLECHANGE;
    pWork->changeFactor[1]=POKETRADE_FACTOR_SINGLECHANGE;
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait);// あいても選ぶまで待つ
  }
  else{
    u8 cmd = POKETRADE_FACTOR_SINGLECHANGE;
    if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, 1, &cmd)){
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_B,WB_NET_TRADE_SERVICEID);
      _CHANGE_STATE(pWork, _networkFriendsStandbyWait);// あいても選ぶまで待つ
    }
  }
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
      POKETRADE_MESSAGE_WindowTimeIconStart(pWork);

      //   G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 8 );

      _CHANGE_STATE(pWork, _pokeSendDataState);

    }
    else{  //キャンセル
      //      _PokemonReset(pWork,0);
      _CHANGE_STATE(pWork, _touchState);
    }
  }
}



static void _changeMenuOpen2(POKEMON_TRADE_WORK* pWork)
{
  if( _PokemonsetAndSendData(pWork) )
  {
    int msg[]={POKETRADE_STR_01,POKETRADE_STR_06};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));

    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , -8 );

    _CHANGE_STATE(pWork, _changeMenuWait);
  }
}

static void _changeMenuOpen1(POKEMON_TRADE_WORK* pWork)
{
  BOOL bSet = FALSE;
  if(pWork->selectBoxno == pWork->BOX_TRAY_MAX){
    bSet = TRUE;
  }
  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_SELECT_POKEMON_BOX, 1, &bSet)){
    _CHANGE_STATE(pWork, _changeMenuOpen2);
  }
}

//こうかんにだす、メニュー表示
static void _changeMenuOpen(POKEMON_TRADE_WORK* pWork)
{
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM3, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_RETURN ,FALSE );
  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], FALSE );

  _CatchPokemonRelease(pWork);

  _CHANGE_STATE(pWork, _changeMenuOpen1);
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
static BOOL _SerchTouchCLACTPosition(POKEMON_TRADE_WORK* pWork, int* boxno,int* boxindex,int* line,int* index)
{
  BOOL bChange=FALSE;
  {
    u32 x,y;
    int RingLine;

    if(GFL_UI_TP_GetPointTrg(&x, &y)==TRUE){
      GFL_CLWK* pCL = IRC_POKETRADE_GetCLACT(pWork,x+12,y+12, boxno, boxindex, line, index, &RingLine);
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




static BOOL checkTouchCLACTPosition(POKEMON_TRADE_WORK* pWork, BOOL bCatch)
{
  BOOL bChange=FALSE;
  {
    u32 x,y;
    int line,index,Ringline;

    if(GFL_UI_TP_GetPointTrg(&x, &y)==TRUE){
      GFL_CLWK* pCL = IRC_POKETRADE_GetCLACT(pWork,x+12,y+12, &pWork->workBoxno, &pWork->workPokeIndex, &line, &index, &Ringline);

      if(pCL){
        POKEMON_PASO_PARAM* ppp =
          IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->workBoxno, pWork->workPokeIndex,pWork);

        //おなじポケモンを選択したらキャンセル
        if( _IsBothPokemonSelect(pWork, pWork->workBoxno, pWork->workPokeIndex) ){
          PMSND_PlaySE(SEQ_SE_MSCL_05);
          if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
            _CHANGE_STATE(pWork,_DeletePokemonState);
            return 3;
          }
          else{
            pWork->underSelectBoxno  = pWork->workBoxno;
            pWork->underSelectIndex = pWork->workPokeIndex;
            _CHANGE_STATE(pWork, POKE_GTS_DeletePokemonState);
            return 3;
          }
        }


        _CatchPokemonPositionRewind(pWork);          //今のつかんでる物を元の位置に戻す
        if(ppp && PPP_Get( ppp, ID_PARA_poke_exist, NULL  ) != 0 ){

          if(POKE_GTS_BanPokeCheck(pWork,ppp)){

            GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_09, pWork->pMessageStrBuf );
            POKETRADE_MESSAGE_WindowOpenCustom(pWork,TRUE,FALSE);
            _CHANGE_STATE(pWork, _notWazaChangePoke2);
            return 2;
          }
          else{
            _CatchPokemonPositionActive(pWork,pCL, Ringline, index, ppp);

            IRC_POKETRADE_CursorEnable(pWork,line, index);  //OAMカーソル移動
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
  }
  return bChange;
}



//ひでんわざ検査に引っかかった
static void _notWazaChangePoke2(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrgAndSet() || GFL_UI_TP_GetTrgAndSet()){
    pWork->workBoxno=-1;
    pWork->workPokeIndex=-1;
    POKETRADE_MESSAGE_WindowClear(pWork);
    _CHANGE_STATE(pWork, POKE_TRADE_PROC_TouchStateCommon);

    //20100629 カーソル状態を再描画 add Saito BTS7053
    pWork->oldLine = -1;
    IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork , TRUE ); 
  }
}



//ひでんわざ検査に引っかかった
static void _notWazaChangePoke(POKEMON_TRADE_WORK* pWork)
{
  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_03, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->oldLine = -1;
  IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork , FALSE );//再描画
  _CHANGE_STATE(pWork,_notWazaChangePoke2);
}

//特別なポケモン検査に引っかかった
static void _notLegendChangePoke(POKEMON_TRADE_WORK* pWork)
{
  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_09, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpenCustom(pWork,TRUE,FALSE);
//  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_03, pWork->pMessageStrBuf );
//  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->oldLine = -1;
  IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork , FALSE );//再描画
  _CHANGE_STATE(pWork,_notWazaChangePoke2);
}

static void _dispSubStateWait2(POKEMON_TRADE_WORK* pWork)
{
#if 0   //20100603 del    Saito
  POKEMON_PASO_PARAM* ppp =
    IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox,
                                       pWork->underSelectBoxno, pWork->underSelectIndex,pWork);
  POKEMON_PARAM* pp = PP_CreateByPPP(ppp,pWork->heapID);
#else
  POKEMON_PARAM* pp = POKEMONTRADE_CreatePokeData(pWork->pBox,
                                                    pWork->underSelectBoxno, pWork->underSelectIndex,pWork);
#endif
  POKETRADE_MESSAGE_ResetPokemonMyStDisp(pWork,FALSE);
  POKETRADE_MESSAGE_ChangePokemonMyStDisp(pWork, pWork->pokemonselectno ,  pp);
  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
    _PokemonIconRenew(pWork);
  }
  GFL_HEAP_FreeMemory(pp);
  _CHANGE_STATE(pWork,_dispSubStateWait);
}



static void _dispSubStateWait_SendPoke(POKEMON_TRADE_WORK* pWork)
{
  if(POKE_GTS_PokemonsetAndSendData(pWork,pWork->selectIndex,pWork->selectBoxno)){  //記録
    _CHANGE_STATE(pWork, POKETRADE_TouchStateGTS);//タッチに戻る
  }
}

static void _dispSubStateWait_SendFlg(POKEMON_TRADE_WORK* pWork)
{
  int no;
  BOOL bSet = FALSE;
  if(pWork->selectBoxno == pWork->BOX_TRAY_MAX){
    bSet = TRUE;
  }
  no = POKEMONTRADE_NEGO_CheckGTSSetPokemon(pWork,0,pWork->selectIndex,pWork->selectBoxno);
  if(no != -1){
    if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), _NETCMD_THREE_SELECT1_BOX + no, 1, &bSet)){
      _CHANGE_STATE(pWork,_dispSubStateWait_SendPoke);
    }
  }
  else{
    _CHANGE_STATE(pWork,_dispSubStateWait_SendPoke);
  }
}



static void _dispSubStateWait_SendPoke6(POKEMON_TRADE_WORK* pWork)
{
  if(POKE_GTS_PokemonsetAndSendData(pWork,pWork->selectIndex,pWork->selectBoxno)){  //記録
    _CHANGE_STATE(pWork, POKE_GTS_Select6MessageInit); //６交換に行く
  }
}


static void _dispSubStateWait_SendFlg6(POKEMON_TRADE_WORK* pWork)
{
  int no;
  BOOL bSet = FALSE;
  if(pWork->selectBoxno == pWork->BOX_TRAY_MAX){
    bSet = TRUE;
  }
  no = POKEMONTRADE_NEGO_CheckGTSSetPokemon(pWork,0,pWork->selectIndex,pWork->selectBoxno);
  if(no != -1){
    if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), _NETCMD_THREE_SELECT1_BOX + no, 1, &bSet)){
      _CHANGE_STATE(pWork,_dispSubStateWait_SendPoke6);
    }
  }
  else{
    _CHANGE_STATE(pWork,_dispSubStateWait_SendPoke6);
  }
}


//自分画面にステータス表示 相手に見せるかどうか待ち
static void _dispSubStateWait(POKEMON_TRADE_WORK* pWork)
{
  BOOL bExit=FALSE;
  BOOL bChange=FALSE;
  BOOL bIconReset=TRUE;
  int selectno=-1;
  u32 x,y;
  int boxindex,boxno;
  int line,index;

  //ＡＰＰタスクで決定がされていないときのみ機能させる　add Saito BTS6605
  if ( !APP_TASKMENU_IsDecide( pWork->pAppTask ) )
  {
    if(GFL_UI_TP_GetTrgAndSet()){
      _CatchPokemonPositionRewind(pWork);
    }

    TOUCHBAR_Main(pWork->pTouchWork);
    switch( TOUCHBAR_GetTouch(pWork->pTouchWork )){
    case TOUCHBAR_ICON_CUTSOM3:
      if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
        APP_TASKMENU_SetActive(pWork->pAppTask,FALSE);
      }
      break;
    }
    switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
    case TOUCHBAR_ICON_CUTSOM3:
      {
        pWork->pokemonselectno = 1 - pWork->pokemonselectno;
        bChange=TRUE;
      }
      break;
    }
    
    // _CatchPokemonMoveFunc(pWork);

    if(GFL_UI_TP_GetPointTrg(&x, &y)==TRUE){
      GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
      APP_TASKMENU_SetActive(pWork->pAppTask,FALSE);
      if(_SerchTouchCLACTPosition(pWork,&pWork->underSelectBoxno,&pWork->underSelectIndex,&line,&index)){
        pWork->MainObjCursorIndex = index;
        pWork->MainObjCursorLine = line;
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
      POKETRADE_MESSAGE_ResetPokemonMyStDisp(pWork,TRUE);
      _CHANGE_STATE(pWork, _dispSubStateWait2);
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
  }

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    bExit=TRUE;
  }
  if(bExit){
    if(selectno==0){         //相手に見せる
      POKEMON_PASO_PARAM* ppp =
        IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->underSelectBoxno, pWork->underSelectIndex,pWork);

      //選択済みポケモンを選択したか？  20100630 add Saito BTS5249 + BTS7362
      if ( _IsBothPokemonSelect(pWork, pWork->underSelectBoxno, pWork->underSelectIndex) && POKEMONTRADEPROC_IsTriSelect(pWork))
      {
        pWork->selectIndex = pWork->underSelectIndex;
        pWork->selectBoxno = pWork->underSelectBoxno;
        _CHANGE_STATE(pWork, _dispSubStateWait_SendFlg);
      }
      else if(POKEMONTRADE_IsWazaPokemon(pWork,pWork->underSelectBoxno,pWork->underSelectIndex)){// 交換できない技もち
        POKE_GTS_VisibleFaceIcon(pWork,TRUE);
        _CHANGE_STATE(pWork,_notWazaChangePoke);
      }
      else if( POKE_GTS_BanPokeCheck(pWork,ppp) ){// 交換できない技もち
        _CHANGE_STATE(pWork, _notLegendChangePoke);
        POKE_GTS_VisibleFaceIcon(pWork,TRUE);
      }
      else if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
        pWork->selectIndex = pWork->underSelectIndex;
        pWork->selectBoxno = pWork->underSelectBoxno;
//        POKMEONTRADE2D_IconGray(pWork, pWork->pSelectCLWK, TRUE);
        _CHANGE_STATE(pWork, _changeMenuOpen);
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE);
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM3, FALSE);
        bIconReset=FALSE;
      }
      else if( (POKE_GTS_IsFullMode(pWork) &&  (GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY) )){
        pWork->selectIndex = pWork->underSelectIndex;
        pWork->selectBoxno = pWork->underSelectBoxno;
        _CHANGE_STATE(pWork, _dispSubStateWait_SendFlg6);
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE);
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM3, FALSE);
        bIconReset=FALSE;
      }
      else{
        pWork->selectIndex = pWork->underSelectIndex;
        pWork->selectBoxno = pWork->underSelectBoxno;
        _CHANGE_STATE(pWork, _dispSubStateWait_SendFlg);
      }
    }
    else{
      //やめる
      POKE_GTS_VisibleFaceIcon(pWork,TRUE);
      _CHANGE_STATE(pWork, POKE_TRADE_PROC_TouchStateCommon);
    }
    pWork->underSelectBoxno = -1;
    pWork->underSelectIndex = -1;
    if(bIconReset){
      _scrollResetAndIconReset(pWork);
      GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], TRUE );
    }
    IRC_POKETRADE_ItemIconReset(&pWork->aItemMark);
    IRC_POKETRADE_ItemIconReset(&pWork->aPokerusMark);
    IRC_POKETRADE_ItemIconReset(&pWork->aPokerusFaceMark);
    POKETRADE_MESSAGE_ResetPokemonMyStDisp(pWork,TRUE);
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
    IRC_POKETRADE_SubStatusEnd(pWork);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    pWork->pAppTask=NULL;
  }

  pWork->bTouch = GFL_UI_TP_GetCont();
}


// 自分の画面にステータスを表示

static void _dispSubState(POKEMON_TRADE_WORK* pWork)
{
  POKE_GTS_VisibleFaceIcon(pWork,FALSE);

  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM3, TRUE);

  {
    int msg[]={POKETRADE_STR_03,POKETRADE_STR_06};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
  }

  {
#if 0   //20100603 del    Saito
    POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox,
                                                                 pWork->underSelectBoxno, pWork->underSelectIndex,pWork);
    POKEMON_PARAM* pp = PP_CreateByPPP(ppp,pWork->heapID);
#else
    POKEMON_PARAM* pp = POKEMONTRADE_CreatePokeData(pWork->pBox,
                                                    pWork->underSelectBoxno, pWork->underSelectIndex,pWork);
#endif

    pWork->pokemonselectno = 0;

    POKETRADE_MESSAGE_ChangePokemonMyStDisp(pWork, pWork->pokemonselectno , pp);
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


// 選択したポケモン番号送る
static void _SendPokemonSelect(POKEMON_TRADE_WORK* pWork)
{
  u8 sdata = pWork->pokemonselectno;

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), _NETCMD_POKEMONSELECT_GTS, 1, &sdata)){
      return;
    }
  }
  GXS_SetVisibleWnd( GX_WNDMASK_NONE );
  if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
    _CHANGE_STATE(pWork,POKMEONTRADE_IRCDEMO_ChangeDemo);
  }
  else{
    //  _CHANGE_STATE(pWork,IRC_POKMEONTRADE_STEP_ChangeDemo_PokeMove);
    _CHANGE_STATE(pWork,POKMEONTRADE_DEMO_GTSMID_ChangeDemo);
  }
}

//通信同期を取る
void POKEMONTRADE_changeTimingDemoStart(POKEMON_TRADE_WORK* pWork)
{
  BOOL bMode = POKEMONTRADEPROC_IsNetworkMode(pWork);

  if(bMode){
    if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_TRADEDEMO_START,WB_NET_TRADE_SERVICEID)){
      return;
    }
  }
  _CHANGE_STATE(pWork, _SendPokemonSelect);
}



// 交換の返事を待つ  タイミングＣ用
static void _changeWaitState(POKEMON_TRADE_WORK* pWork)
{
  int i;
  int myID = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  int targetID = 1 - myID;
  int msgno;

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_C, WB_NET_TRADE_SERVICEID)){
      POKE_GTS_CreateCancelButton(pWork);  //GTSネゴのみ強制切断処理
      return;
    }
  }
  POKE_GTS_DeleteCancelButton(pWork,TRUE,TRUE);

  if((pWork->changeFactor[myID]==POKETRADE_FACTOR_SINGLE_OK) &&
     (pWork->changeFactor[targetID]==POKETRADE_FACTOR_SINGLE_OK)){
    if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_TRADEDEMO_START,WB_NET_TRADE_SERVICEID);
    }
    _CHANGE_STATE(pWork, POKEMONTRADE_changeTimingDemoStart);
    return;
  }
  // else if((pWork->changeFactor[myID]==POKETRADE_FACTOR_SINGLE_NG) &&
  //        (pWork->changeFactor[targetID]==POKETRADE_FACTOR_SINGLE_NG)){
  //  pWork->pParentWork->ret = POKEMONTRADE_MOVE_END;
  //  _CHANGE_STATE(pWork, POKEMONTRADE_PROC_FadeoutStart);
  // return;
  //  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_EGG){
    msgno = POKETRADE_STR_97;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_EGG){
    msgno = POKETRADE_STR_98;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_ILLEGAL){
    msgno = POKETRADE_STR2_01;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_ILLEGAL){
    msgno = POKETRADE_STR2_02;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_ILLEGAL_FRIEND){
    msgno = POKETRADE_STR2_02;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_ILLEGAL_FRIEND){
    msgno = POKETRADE_STR2_01;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_WAZA){
    msgno = POKETRADE_STR_97;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_WAZA){
    msgno = POKETRADE_STR2_03;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_MAIL){
    msgno = POKETRADE_STR_97;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_MAIL){
    msgno = POKETRADE_STR2_32;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_SINGLE_NG){
    msgno = POKETRADE_STR_97;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_SINGLE_NG){
    msgno = POKETRADE_STR2_04;
  }
  GFL_MSG_GetString( pWork->pMsgData, msgno, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->changeFactor[0] = POKETRADE_FACTOR_NONE;
  pWork->changeFactor[1] = POKETRADE_FACTOR_NONE;


  // GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_A,WB_NET_TRADE_SERVICEID);
  _CHANGE_STATE(pWork, POKEMONTRAE_EndCancelState);
}



static void _touchState_BeforeTimeing3(POKEMON_TRADE_WORK* pWork)
{
  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    _touchState(pWork);
    _touchStated(pWork);
    _CHANGE_STATE(pWork, _gtsFirstMsgState);
  }
  else{
    _touchState(pWork);
    _touchStated(pWork);
  }
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

}

static void _touchState_BeforeTimeing2(POKEMON_TRADE_WORK* pWork)
{
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork) || GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_POKECOLOR,WB_NET_TRADE_SERVICEID))
  {
  }
  else{
    return;
  }

//  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  POKETRADE_MESSAGE_WindowClear(pWork);

  if(POKEMONTRADEPROC_IsTriSelect(pWork)){

    POKE_GTS_InitWork(pWork);
    POKE_GTS_InitEruptedIconResource(pWork);

    if(POKEMONTRADE_TYPE_WIFICLUB == pWork->type){
//      GFL_NET_DWC_pausevchat(FALSE);
    }
    
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_01, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
  //  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , -8 );
    POKE_GTS_CreatePokeIconResource(pWork,CLSYS_DRAW_MAIN);
    POKE_GTS_StatusMessageDisp(pWork);

    pWork->pokemonGTSSeq[0]= POKEMONTORADE_SEQ_WAIT;
    pWork->pokemonGTSSeq[1]= POKEMONTORADE_SEQ_WAIT;

    POKE_GTS_InitFaceIcon(pWork);
    {
      POKE_GTS_VisibleFaceIcon(pWork,TRUE);
      GFL_STD_MemClear(pWork->pokeIconNo,sizeof(pWork->pokeIconNo));
      GFL_STD_MemClear(pWork->pokeIconForm,sizeof(pWork->pokeIconForm));
      GFL_STD_MemClear(pWork->pokeIconSex,sizeof(pWork->pokeIconSex));
      
      _CatchPokemonPositionRewind(pWork);

      if(pWork->pAppTask!=NULL){
        APP_TASKMENU_CloseMenu(pWork->pAppTask);
        pWork->pAppTask=NULL;
      }
      IRC_POKETRADE_SendVramBoxNameChar(pWork); // ボックス名初期化
      POKEMONTRADE_2D_AlphaSet(pWork);
      _scrollResetAndIconReset(pWork);
    }
  }
  _CHANGE_STATE(pWork, _touchState_BeforeTimeing3);
  //初期座標を送る
  _scrollMainFunc(pWork,FALSE,TRUE);
}


static void _touchState_BeforeTimeingFi(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(!WIPE_SYS_EndCheck()){
    return;
  }
  POKETRADE_MESSAGE_WindowClear(pWork);
  _CHANGE_STATE(pWork, _touchState_BeforeTimeing2);
}




static void _touchState_BeforeTimeingFo2(POKEMON_TRADE_WORK* pWork)
{
  WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  _CHANGE_STATE(pWork, _touchState_BeforeTimeingFi);
}


//カラーテクスチャーを作成
static void _touchState_BeforeTimeingFo(POKEMON_TRADE_WORK* pWork)
{
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork) || GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_POKECOLOR,WB_NET_TRADE_SERVICEID)){
    IRC_POKETRADE3D_SetColorTex(pWork);
    _CHANGE_STATE(pWork, _touchState_BeforeTimeingFo2);
  }
}


static void _touchState_BeforeTimeing14(POKEMON_TRADE_WORK* pWork)
{
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(),GFL_NET_SENDID_ALLUSER,
                          _NETCMD_POKEMONCOLOR,
                          BOX_POKESET_MAX + TEMOTI_POKEMAX, pWork->FriendPokemonCol[0], FALSE,FALSE,TRUE)){
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_POKECOLOR, WB_NET_TRADE_SERVICEID);
      _CHANGE_STATE(pWork, _touchState_BeforeTimeingFo);
    }
  }
  else{
    IRC_POKETRADE3D_SetColorTex(pWork);
    _CHANGE_STATE(pWork, _touchState_BeforeTimeingFo);
  }
}

static void _touchState_BeforeTimeing13(POKEMON_TRADE_WORK* pWork)
{
  OS_TPrintf("xxx++ %d \n",OS_GetVBlankCount());
  _createEasyPokeInfo(pWork, pWork->anmCount-1);  //ポケモン情報

  if(pWork->anmCount >= BOX_MAX_TRAY){
    _CHANGE_STATE(pWork, _touchState_BeforeTimeing14);
  }
}

static void _touchState_BeforeTimeing12(POKEMON_TRADE_WORK* pWork)
{
#if PM_DEBUG
  OS_TPrintf("++++ %d \n",OS_GetVBlankCount());
#endif
  if(PokemonTrade_SetMyPokeColor(pWork, pWork->anmCount-1)){  //色設定
    pWork->anmCount=0;
    _CHANGE_STATE(pWork, _touchState_BeforeTimeing13);
  }
}

//トレイの数＋メールボックスの状態を送る
static void _touchState_BeforeTimeing1Send(POKEMON_TRADE_WORK* pWork)
{

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    u8 sdata[2];
    sdata[0] = pWork->BOX_TRAY_MAX;
    sdata[1] = _IsMailBoxFull(pWork);
    if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), _NETCMD_FRIENDBOXNUM, 2, &sdata)){
    pWork->anmCount=0;
      _CHANGE_STATE(pWork, _touchState_BeforeTimeing12);
    }
  }
  else{
    pWork->friendBoxNum = pWork->BOX_TRAY_MAX;
    pWork->FriendBoxScrollNum = pWork->friendBoxNum*_BOXTRAY_MAX + _TEMOTITRAY_MAX - 80;
    pWork->anmCount=0;
    _CHANGE_STATE(pWork, _touchState_BeforeTimeing12);
  }
}


//両者タイミングそろえ
static void _touchState_BeforeTimeing12Send(POKEMON_TRADE_WORK* pWork)
{
  POKEMONTRADE_PokeIconCgxLoad( pWork, pWork->anmCount-1 );
#if PM_DEBUG
  OS_TPrintf("++**++ %d \n",OS_GetVBlankCount());
#endif
  if(pWork->anmCount > BOX_MAX_TRAY+1){
    IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork , FALSE );
    _CHANGE_STATE(pWork,_touchState_BeforeTimeing1Send);
  }
}

//両者タイミングそろえ
static void _touchState_BeforeTimeing11Send(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_FIRSTBOTH,WB_NET_TRADE_SERVICEID)){
    GFL_NET_SetAutoErrorCheck(TRUE);
    GFL_NET_SetNoChildErrorCheck(TRUE);
    pWork->anmCount=0;
    _CHANGE_STATE(pWork,_touchState_BeforeTimeing12Send);
  }
}



static void _touchState_BeforeTimeing1W(POKEMON_TRADE_WORK* pWork)
{
  if(!WIPE_SYS_EndCheck()){
    return;
  }
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(), _TIMING_FIRSTBOTH, WB_NET_TRADE_SERVICEID );
    _CHANGE_STATE(pWork,_touchState_BeforeTimeing11Send);
  }
  else{
    _CHANGE_STATE(pWork,_touchState_BeforeTimeing1Send);
  }
}



//初期化＆２回目
static void _touchState_BeforeTimeing1(POKEMON_TRADE_WORK* pWork)
{
  GFL_NET_ReloadIconTopOrBottom(TRUE, pWork->heapID);


//  if(POKEMONTRADE_TYPE_WIFICLUB == pWork->type){
//    GFL_NET_DWC_pausevchat(TRUE);
//  }
  
  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_09, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpenCustom(pWork,TRUE,FALSE);
  //メッセージ時計アイコン
  POKETRADE_MESSAGE_WindowTimeIconStart(pWork);

  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_OBJ );
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG2 );
  WIPE_ResetBrightness(WIPE_DISP_MAIN);  //通信アイコン表示のため

  WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

  _CHANGE_STATE(pWork,_touchState_BeforeTimeing1W);
}


//セーブルーチンから戻ってきたところ
void IRC_POKMEONTRADE_ChangeFinish(POKEMON_TRADE_WORK* pWork)
{
  int id = 1-GFL_NET_SystemGetCurrentID();

  pWork->selectBoxno = -1;
  pWork->selectIndex = -1;
  pWork->selectMoji = 0;
  pWork->pCatchCLWK = NULL;
//  pWork->pSelectCLWK = NULL;
  pWork->pokemonThreeSet = FALSE;
  IRCPOKETRADE_PokeDeleteMcss(pWork, 0);
  IRCPOKETRADE_PokeDeleteMcss(pWork, 1);

  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    POKE_GTS_InitWork(pWork);
  }

  GFL_STD_MemClear(pWork->recvPoke[0],POKETOOL_GetWorkSize());
  GFL_STD_MemClear(pWork->recvPoke[1],POKETOOL_GetWorkSize());

  {
    int i;
    for(i = 0;i< CELL_DISP_NUM;i++){
      if(pWork->curIcon[i]){
        GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[i], TRUE );
      }
    }
  }
  POKETRADE_MESSAGE_WindowClear(pWork);
  _CHANGE_STATE(pWork, _touchState_BeforeTimeing1);

}


static void _cancelPokemonSendDataNetwork2(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_C,WB_NET_TRADE_SERVICEID);
  _CHANGE_STATE(pWork,_changeWaitState);
}

//１体見せ合いのキャンセル
static void _cancelPokemonSendDataNetwork(POKEMON_TRADE_WORK* pWork)
{
  u8 cmd = POKETRADE_FACTOR_SINGLE_NG;

  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_97, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork, POKEMONTRAE_EndCancelState);
  }
  else if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, sizeof(u8), &cmd)){
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_C,WB_NET_TRADE_SERVICEID);
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_09, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
    _CHANGE_STATE(pWork,_cancelPokemonSendDataNetwork2);
  }
}



static void _cancelTimingSync2(POKEMON_TRADE_WORK* pWork)
{
  int i;

  if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),
                                POKETRADE_FACTOR_TIMING_A,WB_NET_TRADE_SERVICEID)){
    POKE_GTS_CreateCancelButton(pWork);
    return;
  }
  POKE_GTS_DeleteCancelButton(pWork, TRUE, TRUE);
  POKETRADE_MESSAGE_WindowClear(pWork);
  pWork->changeFactor[0]=POKETRADE_FACTOR_NONE;
  pWork->changeFactor[1]=POKETRADE_FACTOR_NONE;
  pWork->pokemonGTSSeq[0]= POKEMONTORADE_SEQ_WAIT;
  pWork->pokemonGTSSeq[1]= POKEMONTORADE_SEQ_WAIT;
  for(i=0;i<6;i++){
    pWork->GTSSelectIndex[i/3][i%3]=-1;
    if(pWork->GTSSelectPP[i/3][i%3]){
      GFL_STD_MemClear(pWork->GTSSelectPP[i/3][i%3], POKETOOL_GetWorkSize());
    }
  }
  _CHANGE_STATE(pWork,_touchState);
}

static void _cancelTimingSync(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_A,WB_NET_TRADE_SERVICEID);
  _CHANGE_STATE(pWork,_cancelTimingSync2);
}


//キャンセル画面待ち
void POKEMONTRAE_EndCancelState(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrgAndSet() || GFL_UI_TP_GetTrgAndSet()){
    POKETRADE_MESSAGE_WindowClear(pWork);
    if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_51, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
      _CHANGE_STATE(pWork, _cancelTimingSync);
    }
    else{
      _CHANGE_STATE(pWork,_touchState);
    }
  }
}

//終わるのを相手に伝えている
static void _endWaitStateNetwork22(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    u8 flg = POKEMONTORADE_SEQ_CANCEL;
    if( GFL_NET_SendData(pNet, _NETCMD_GTSSEQNO,1,&flg)){
      _CHANGE_STATE(pWork, _networkFriendsStandbyWait);
    }
  }
  else{
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait);
  }
}


//終わるのを相手に伝えている
static void _endWaitStateNetwork(POKEMON_TRADE_WORK* pWork)
{
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    u8 cmd = POKETRADE_FACTOR_END;
    if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR,1, &cmd)){
      GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_B,WB_NET_TRADE_SERVICEID);
      _CHANGE_STATE(pWork, _endWaitStateNetwork22);
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

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    pWork->pAppTask=NULL;
    switch(selectno){
    case 0:
      _CHANGE_STATE(pWork,_endWaitStateNetwork);
      break;
    case 1:
      POKETRADE_MESSAGE_WindowClear(pWork);
      _CHANGE_STATE(pWork,POKE_TRADE_PROC_TouchStateCommon);
      pWork->oldLine = -1;
      IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork , FALSE );//再描画 BTS6400

      POKEMONTRADE_2D_AlphaSet(pWork);
      break;
    }
  }

}



static BOOL _PokemonsetAndSendData(POKEMON_TRADE_WORK* pWork)
{ //選択ポケモン表示
  BOOL bRet = FALSE;
  POKEMON_PARAM* pp = POKEMONTRADE_CreatePokeData(pWork->pBox, pWork->selectBoxno, pWork->selectIndex,pWork);

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

    pp2 = PP_Create(MONSNO_509, 100, 123456, GFL_HEAPID_APP);
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



static void _recvEndReqFunc2(POKEMON_TRADE_WORK *pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int msg[]={POKETRADE_STR_27, POKETRADE_STR_28};
    POKETRADE_MESSAGE_AppMenuOpenCustom(pWork,msg,elementof(msg),32,12);
  }
  pWork->oldLine = -1;
  IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork , FALSE );//再描画 BTS6400
  _CHANGE_STATE(pWork, _endWaitState);
}




//終了コールバック処理
static void _recvEndReqFunc(POKEMON_TRADE_WORK *pWork)
{
  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_07, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->bTouchReset=TRUE;
  _CHANGE_STATE(pWork, _recvEndReqFunc2);
}





// 終了しますか？の問いかけそうしん
static void _endRequestState(POKEMON_TRADE_WORK* pWork)
{
  //  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);
  //  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE );
  // TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM3, FALSE );

  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , -8 );

  _CHANGE_STATE(pWork,_recvEndReqFunc);

}


static BOOL _MoveSearchPoke(POKEMON_TRADE_WORK* pWork,int moji)
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
          if(!PPP_Get(ppp,ID_PARA_tamago_flag,NULL)){
            //OS_TPrintf("みつかった BOX %d\n",line/6);
            pWork->oldLine++;
            pWork->BoxScrollNum = _boxScrollLine2Num(line) - 32;
            pWork->bgscrollRenew = TRUE;
            _scrollMainFunc(pWork,FALSE,TRUE);
            return TRUE;
          }
        }
      }
    }
  }
  return FALSE;
}



static void _mojiSelectEnd3(POKEMON_TRADE_WORK* pWork)
{
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  _CHANGE_STATE(pWork, POKE_TRADE_PROC_TouchStateCommon);
}

static void _mojiSelectEnd2(POKEMON_TRADE_WORK* pWork)
{
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

  IRC_POKETRADE_EndSubMojiBG(pWork);
  POKETRADE_MESSAGE_WindowClear(pWork);
  GXS_SetVisibleWnd( GX_WNDMASK_NONE );
  POKE_GTS_VisibleFaceIcon(pWork,TRUE);
  POKEMONTRADE2D_ChangePokemonPalette(pWork,FALSE);
  _scrollResetAndIconReset(pWork);
  POKEMONTRADE_2D_AlphaSet(pWork); //G2S_BlendNone();
  POKEMONTRADE_EndMojiSelect(pWork);

  _CHANGE_STATE(pWork, _mojiSelectEnd3);
}





static void _mojiSelectEnd(POKEMON_TRADE_WORK* pWork)
{
  if(FALSE == POKEMONTRADE_CheckMojiSelect(pWork)){

    _CHANGE_STATE(pWork, _mojiSelectEnd2);
  }

}


// 検索文字パネル表示中
static void _loopSearchMojiState(POKEMON_TRADE_WORK* pWork)
{
  BOOL endflg=FALSE;

  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int ans = GFL_UI_TP_HitTrg(tp_mojidata);
    if(ans != GFL_UI_TP_HIT_NONE){
      pWork->selectMoji = ans + 1;
      POKEMONTRADE_StartMojiSelect(pWork,tp_mojidata[ans].rect.left,tp_mojidata[ans].rect.top);
      if(_MoveSearchPoke(pWork,ans)){
        PMSND_PlaySystemSE(POKETRADESE_LANG_SEARCH);
        GXS_SetVisibleWnd( GX_WNDMASK_NONE );
        _CHANGE_STATE(pWork, _mojiSelectEnd);
        return;
      }
      else{
        GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_99, pWork->pMessageStrBuf );
        POKETRADE_MESSAGE_WindowOpenXY(pWork, TRUE, 1,1,29,4);
        PMSND_PlaySystemSE(POKETRADESE_LANG_CANCEL);
      }
    }
  }

  TOUCHBAR_Main(pWork->pTouchWork);
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
    pWork->selectMoji = 0;
    IRC_POKETRADE_EndSubMojiBG(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);
    GXS_SetVisibleWnd( GX_WNDMASK_NONE );
    POKE_GTS_VisibleFaceIcon(pWork,TRUE);
    POKEMONTRADE2D_ChangePokemonPalette(pWork,FALSE);
    _scrollResetAndIconReset(pWork);
    POKEMONTRADE_2D_AlphaSet(pWork); //G2S_BlendNone();
    POKEMONTRADE_EndMojiSelect(pWork);
    _CHANGE_STATE(pWork, _mojiSelectEnd2);
    break;
  default:
    break;
  }
}



// 検索文字パネル開始２
static void _startSearchMojiState2(POKEMON_TRADE_WORK* pWork)
{
  IRC_POKETRADE_InitSubMojiBG(pWork);
  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_52, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpenXY(pWork, TRUE, 1,1,29,4);
  _CHANGE_STATE(pWork,_loopSearchMojiState);
}

// 検索文字パネル開始
static void _startSearchMojiState(POKEMON_TRADE_WORK* pWork)
{
  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE );
  POKE_GTS_VisibleFaceIcon(pWork, FALSE);
  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 , -8 );
  POKEMONTRADE2D_ChangePokemonPalette(pWork,TRUE);
  if(POKE_GTS_IsMyIn(pWork)){
    TOUCHBAR_SetActive(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
  }
  _CHANGE_STATE(pWork,_startSearchMojiState2);
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


  if(pWork->pCatchCLWK){  //タッチパネル操作中のアイコン移動
    return;
  }


  if(key & PAD_BUTTON_R ){   //ベクトルを監視
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
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

    CalcLineAfterLR(pWork, bChange);    //20100630 add Saito
    
  }
  else if(key & PAD_BUTTON_L ){   //ベクトルを監視
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
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

    CalcLineAfterLR(pWork, bChange);    //20100630 add Saito

  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_UP ){   //ベクトルを監視
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
    if(POKETRADE_IsMainCursorDispIn(pWork, &line)==FALSE){
      pWork->MainObjCursorLine=line;
    }
    pWork->MainObjCursorIndex--;
    if(pWork->MainObjCursorIndex<0){
      if(pWork->MainObjCursorLine < HAND_HORIZONTAL_NUM){
        pWork->MainObjCursorIndex = HAND_VERTICAL_NUM-1;
      }
      else{
        pWork->MainObjCursorIndex = BOX_VERTICAL_NUM-1;
      }
    }
    pWork->oldLine--;
    bChange=TRUE;
  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_DOWN ){   //ベクトルを監視
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
    if(POKETRADE_IsMainCursorDispIn(pWork, &line)==FALSE){
      pWork->MainObjCursorLine=line;
    }
    pWork->MainObjCursorIndex++;
    if(pWork->MainObjCursorLine < HAND_HORIZONTAL_NUM){
      if(pWork->MainObjCursorIndex >= HAND_VERTICAL_NUM){
        pWork->MainObjCursorIndex = 0;
      }
    }
    else if(pWork->MainObjCursorIndex >= BOX_VERTICAL_NUM){
      pWork->MainObjCursorIndex = 0;
    }
    pWork->oldLine--;
    bChange=TRUE;
  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_RIGHT ){   //ベクトルを監視
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
    pWork->oldLine--;
    bChange=TRUE;
    if(POKETRADE_IsMainCursorDispIn(pWork, &line)==FALSE){
      pWork->MainObjCursorLine=line;
    }
    else{
      pWork->MainObjCursorLine++;
      if(FALSE == _isCursorInScreen(pWork,pWork->MainObjCursorLine)){
        pWork->BoxScrollNum = _boxScrollLine2Num(pWork->MainObjCursorLine+1)-256;
        pWork->oldLine = -1;    //書き直し  20100630 add Saito BTS7084
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
  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_LEFT ){   //ベクトルを監視
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
    pWork->oldLine++;
    bChange=TRUE;
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
        pWork->oldLine = -1;    //書き直し  20100630 add Saito BTS7084
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
  }

  // NET_PRINT("MainObjCursorIndex %d MainObjCursorLine %d  \n",pWork->MainObjCursorIndex,pWork->MainObjCursorLine);

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

  IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork , TRUE );
  IRC_POKETRADE_TrayDisp(pWork);
  IRC_POKETRADE_SendScreenBoxNameChar(pWork);

  if(!PMSND_CheckPlaySE() && bSE){
    PMSND_PlaySystemSE(POKETRADESE_RIBBON);
  }

  if(POKEMONTRADEPROC_IsNetworkMode(pWork) && bNetSend){  // 特に失敗してもかまわない通信 相手に位置を知らせているだけ

    if(GFL_NET_IsWifiConnect()){
      if( (pWork->BoxScrollNum % 16 == 0) ){
        GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(),GFL_NET_SENDID_ALLUSER,
                           _NETCMD_SCROLLBAR,2,&pWork->BoxScrollNum,FALSE,TRUE,TRUE);
      }
    }
    else{
      GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(),GFL_NET_SENDID_ALLUSER,
                         _NETCMD_SCROLLBAR,2,&pWork->BoxScrollNum,FALSE,TRUE,TRUE);
    }
  }
  else if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
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
//スクロール範囲
#define _SCROLLBAR_X1 (64)
#define _SCROLLBAR_X2 (192)
#define _SCROLLBAR_Y1 (152+12+4)
#define _SCROLLBAR_Y2 (176+12)

static void _panelScroll(POKEMON_TRADE_WORK* pWork)
{
  u32 x,y;

  if(pWork->pCatchCLWK){
    return;
  }

  if(GFL_UI_TP_GetPointCont(&x,&y)){     // パネルスクロール
    if(pWork->touchON){
      if((x >=  _SCROLLBAR_X1) && (_SCROLLBAR_X2 > x) && (y >=  _SCROLLBAR_Y1) && (_SCROLLBAR_Y2 > y)){
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
      else{
        pWork->speed=0;  // 範囲外に出た時は0 BTS5908
      }
    }
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
      _scrollMainFunc(pWork,TRUE,TRUE);  //移動全般
    }
  }
}


static void _DeletePokemonStateRelease(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_UI_TP_GetCont()==FALSE){
    _CHANGE_STATE(pWork, POKE_TRADE_PROC_TouchStateCommon);
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
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
  pWork->selectIndex = -1;
  pWork->selectBoxno = -1;
  pWork->workPokeIndex = -1;
  pWork->workBoxno = -1;
  pWork->underSelectIndex = -1;
  pWork->underSelectBoxno = -1;
  _PokemonReset(pWork,0);

  _PokemonIconRenew(pWork);

  _CHANGE_STATE(pWork, _DeletePokemonStateRelease);
}


//--------------------------------------------------------------------------------------------
/**
 * @brief 基本ステート初期化
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------

static void _touchStateSendCommand(POKEMON_TRADE_WORK* pWork)
{
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    _PokemonReset(pWork,1);
    _userNetCommandClear(pWork);
    _CHANGE_STATE(pWork,_touchStated);
  }
  else{
    if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CANCEL_POKEMON,0,NULL)){
      _CHANGE_STATE(pWork,_touchStated);
    }
  }
}


//--------------------------------------------------------------------------------------------
/**
 * @brief 基本ステート初期化
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------
static void _touchState(POKEMON_TRADE_WORK* pWork)
{
  pWork->touchON=FALSE;
  pWork->xspeed=0;
  pWork->speed=0;
  GFL_STD_MemClear(pWork->pokeIconNo,sizeof(pWork->pokeIconNo));
  GFL_STD_MemClear(pWork->pokeIconForm,sizeof(pWork->pokeIconForm));
  GFL_STD_MemClear(pWork->pokeIconSex,sizeof(pWork->pokeIconSex));

  _CatchPokemonPositionRewind(pWork);
  POKE_GTS_VisibleFaceIcon(pWork, TRUE);


  if(pWork->pAppTask!=NULL){
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
  _PokemonReset(pWork,0);

  _CHANGE_STATE(pWork,_touchStateSendCommand);
}



//--------------------------------------------------------------------------------------------
/**
 * @brief 基本ステート初期化
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------
static void _touchStated(POKEMON_TRADE_WORK* pWork)
{

  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM3, FALSE );
  POKEMONTRADE_TOUCHBAR_SetReturnIconActiveTRUE(pWork);
  
  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], TRUE );

  IRC_POKETRADE_SendVramBoxNameChar(pWork); // ボックス名初期化
  POKEMONTRADE_2D_AlphaSet(pWork); //G2S_BlendNone();

  pWork->selectIndex=-1;
  pWork->selectBoxno=-1;

  _PokemonIconRenew(pWork);

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
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, POKE_GTS_IsMyIn(pWork));
  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, POKE_GTS_IsMyIn(pWork) );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM3, FALSE );
  POKEMONTRADE_TOUCHBAR_SetReturnIconActiveTRUE(pWork);
  _PokemonIconRenew(pWork);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief もう一回ポケモンを選択する為にtouchStateに戻ってきた場合この関数
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------
void POKETRADE_TouchStateGTS(POKEMON_TRADE_WORK* pWork)
{
  int i;

  POKE_GTS_VisibleFaceIcon(pWork,TRUE);
  GFL_STD_MemClear(pWork->pokeIconNo,sizeof(pWork->pokeIconNo));
  GFL_STD_MemClear(pWork->pokeIconForm,sizeof(pWork->pokeIconForm));
  GFL_STD_MemClear(pWork->pokeIconSex,sizeof(pWork->pokeIconSex));

  _CatchPokemonPositionRewind(pWork);

  if(pWork->pAppTask!=NULL){
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
  //  G2S_BlendNone();

  IRC_POKETRADE_SendVramBoxNameChar(pWork); // ボックス名初期化
  POKEMONTRADE_2D_AlphaSet(pWork); //G2S_BlendNone();

  _scrollResetAndIconReset(pWork);

  _CHANGE_STATE(pWork,POKE_TRADE_PROC_TouchStateCommon);
}









static void _POKE_SetAndSendData2(POKEMON_TRADE_WORK* pWork)
{
  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    if(!POKE_GTS_PokemonsetAndSendData(pWork,pWork->selectIndex,pWork->selectBoxno)){  //記録
      return;
    }
  }
  else{
    if( !_PokemonsetAndSendData(pWork)){
      return;
    }
  }
  pWork->underSelectBoxno = -1;
  pWork->underSelectIndex = -1;
  pWork->workPokeIndex = -1;
  pWork->workBoxno = -1;
  _PokemonIconRenew(pWork);
  _CHANGE_STATE(pWork,POKE_TRADE_PROC_TouchStateCommon);
}

static void _POKE_SetAndSendData1(POKEMON_TRADE_WORK* pWork)
{
  BOOL bSet = FALSE;
  int no;
  if(pWork->selectBoxno == pWork->BOX_TRAY_MAX){
    bSet = TRUE;
  }
  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    no = POKEMONTRADE_NEGO_CheckGTSSetPokemon(pWork,0,pWork->selectIndex,pWork->selectBoxno);
    if(no != -1){
      if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_THREE_SELECT1_BOX + no, 1, &bSet)){
        _CHANGE_STATE(pWork,_POKE_SetAndSendData2);
      }
    }
    else{
      _CHANGE_STATE(pWork,_POKE_SetAndSendData2);
    }
  }
  else{
    if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_SELECT_POKEMON_BOX, 1, &bSet)){
      _CHANGE_STATE(pWork,_POKE_SetAndSendData2);
    }
  }
}


static void _POKE_SetAndSendData(POKEMON_TRADE_WORK* pWork)
{
  if(POKEMONTRADE_SuckedMain(pWork)){
    if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
      _CHANGE_STATE(pWork,_POKE_SetAndSendData1);
    }
    else{
      _CHANGE_STATE(pWork,_POKE_SetAndSendData2);
    }
  }
}

//同じポケモンを選択しているかどうか
static BOOL _IsBothPokemonSelect(POKEMON_TRADE_WORK* pWork,int boxno , int selectIndex)
{
  if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
    if((pWork->selectIndex == selectIndex) &&
       (pWork->selectBoxno == boxno) ){
      return TRUE;
    }
  }
  else{
    if( -1 != POKE_GTS_IsSelect(pWork,boxno,selectIndex) ){
      return TRUE;
    }
  }
  return FALSE;
}




static BOOL _TouchStateCommonDecide(POKEMON_TRADE_WORK* pWork)
{

  POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->underSelectBoxno, pWork->underSelectIndex,pWork);

  //おなじポケモンを選択したらキャンセル
  if( _IsBothPokemonSelect(pWork, pWork->underSelectBoxno, pWork->underSelectIndex) ){
    PMSND_PlaySE(SEQ_SE_MSCL_05);   //20100622 add Saito BTS6372
    if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
      _CHANGE_STATE(pWork,_DeletePokemonState);
      return TRUE;
    }
    else{
      _CHANGE_STATE(pWork, POKE_GTS_DeletePokemonState);
      return TRUE;
    }
  }

  if(ppp && PPP_Get( ppp, ID_PARA_poke_exist, NULL  ) != 0 ){
    PMSND_PlaySystemSE(POKETRADESE_MARKON);
    TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    _CHANGE_STATE(pWork,_dispSubState);   ///ステートウインドウ表示
    return TRUE;
  }
  else{
    _CatchPokemonPositionRewind(pWork);
    pWork->underSelectBoxno = -1;
    pWork->underSelectIndex = -1;
  }
  return FALSE;
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
  int backBoxNo = pWork->nowBoxno;
  POKEMON_PASO_PARAM* ppp;
  int line;

  pWork->pokemonGTSStateMode=TRUE;


  //画面タッチしてるか?         20100628 add Saito BTS6942 BTS5189
  if ( GFL_UI_TP_GetCont() )
  {
    //操作系をタッチにする
    GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);

    //キーも操作されていたら、処理をフックする
    if ( GFL_UI_KEY_GetCont() ) return;
  }

  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){  // 最初にキーが入った場合
    if(GFL_UI_KEY_GetTrgAndSet())
    {
      GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
      if(POKETRADE_IsMainCursorDispIn(pWork, &line)==FALSE){
        pWork->MainObjCursorLine=line;
        //手持ちの列の場合縦位置補正      20100626 add Saito BTS6598
        if(pWork->MainObjCursorLine < HAND_HORIZONTAL_NUM)
        {
          if(pWork->MainObjCursorIndex >= HAND_VERTICAL_NUM)
          {
            pWork->MainObjCursorIndex = HAND_VERTICAL_NUM-1;
          }
        }
      }
      else    //20100625 add Saito  BTS6599
      {
        //カーソルがスクリーン内にあるか？
        if(FALSE == _isCursorInScreen(pWork,pWork->MainObjCursorLine )) //無しのとき
        {
          NOZOMU_Printf("カーソルがスクリーン外なのでスクロール\n");
          {
            int linest = POKETRADE_boxScrollNum2Line(pWork);  //有効範囲開始列
            int lineend = linest + 10;  //有効範囲最終列
            int linemid = linest + 5;   //有効範囲中間列
            int tmp;
            NOZOMU_Printf("cur %d  st %d\n",pWork->MainObjCursorLine, linest);
            tmp = pWork->MainObjCursorLine;   //ターゲット列をセット
            //終了列がボックスと手持ちをまたか？
            if ( (lineend >= pWork->TRADEBOX_LINEMAX)&&(tmp < 20) )
            {
              //またぐ場合は判定列を伸ばす
              tmp += (1 + pWork->TRADEBOX_LINEMAX);
            }

            //判定列が範囲中間列のどちらにあるか？
            if (linemid < tmp )  //右側
            {
              NOZOMU_Printf("右側\n");
              pWork->BoxScrollNum = _boxScrollLine2Num(pWork->MainObjCursorLine+1)-256;
            }
            else                  //左側
            {
              NOZOMU_Printf("左側\n");
              pWork->BoxScrollNum = _boxScrollLine2Num(pWork->MainObjCursorLine);
            }
          }
          pWork->oldLine = -1;//強制書き直し
          pWork->bgscrollRenew = TRUE;
          _scrollMainFunc(pWork,FALSE,TRUE);
          //スクロール停止 20100628 add Saito BTS6919_b
          pWork->speed = 0;
          return;
        }
      }
      _PokemonIconRenew(pWork);
      //スクロール停止 20100628 add Saito BTS6919_b
      pWork->speed = 0;
      return;
    }
  }

  // 監視処理
  if(GFL_UI_KEY_GetTrgAndSet() || GFL_UI_TP_GetTrgAndSet()){  //速度リセット
    pWork->speed=0;
  }
  if(GFL_UI_TP_GetTrgAndSet()){  //ポケモン選択リセット
    _CatchPokemonPositionRewind(pWork);
    pWork->underSelectBoxno = -1;
    pWork->underSelectIndex = -1;
  }

  _vectorUpMath(pWork);         // 上にポケモンを登録するかどうかの判定
  _CatchPokemonMoveFunc(pWork); // タッチ中のポケモン移動処理
  _padUDLRFunc(pWork);   //スクロールキー処理
  POKE_GTS_FaceIconFunc(pWork);    //顔アイコンの処理

  //決定処理
  if(GFL_UI_KEY_GetTrgAndSet()== PAD_BUTTON_DECIDE){
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
    if(POKETRADE_IsMainCursorDispIn(pWork, &line)==FALSE){
      pWork->MainObjCursorLine=line;
      //縦位置補正
      if(pWork->MainObjCursorLine < HAND_HORIZONTAL_NUM){
        if(pWork->MainObjCursorIndex >= HAND_VERTICAL_NUM){
          pWork->MainObjCursorIndex = 0;
        }
      }
      else if(pWork->MainObjCursorIndex >= BOX_VERTICAL_NUM){
        pWork->MainObjCursorIndex = 0;
      }
    }
    {
      ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, IRC_TRADE_LINE2TRAY(pWork->MainObjCursorLine,pWork),
                                               IRC_TRADE_LINE2POKEINDEX(pWork->MainObjCursorLine,pWork->MainObjCursorIndex),pWork);
      //    i = IRC_TRADE_LINE2POKEINDEX(pWork->MainObjCursorLine,pWork->MainObjCursorIndex);  //カーソル下のポケモンを探す
      //ポケモンがいた場合
      if(ppp && PPP_Get( ppp, ID_PARA_poke_exist, NULL  ) != 0 ){
        if(POKE_GTS_BanPokeCheck(pWork,ppp)){
          GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_09, pWork->pMessageStrBuf );
          POKETRADE_MESSAGE_WindowOpenCustom(pWork,TRUE,FALSE);
          _CHANGE_STATE(pWork, _notWazaChangePoke2);
          return;
        }
        else{
          pWork->underSelectBoxno = IRC_TRADE_LINE2TRAY(pWork->MainObjCursorLine,pWork);
          pWork->underSelectIndex = IRC_TRADE_LINE2POKEINDEX(pWork->MainObjCursorLine,pWork->MainObjCursorIndex);
          GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);

          if((POKEMONTRADEPROC_IsTriSelect(pWork)) &&  //GTSの場合 選択してあるポケモンだと動作が違う
             (-1 != POKE_GTS_IsSelect(pWork,pWork->underSelectBoxno,pWork->underSelectIndex ))){
            PMSND_PlaySE(SEQ_SE_MSCL_05);   //20100622 add Saito BTS6372
            _CHANGE_STATE(pWork, POKE_GTS_DeletePokemonState);
            return;

          }
          else{
            ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->underSelectBoxno, pWork->underSelectIndex,pWork);

            _CatchPokemonPositionRewind(pWork);

//            pWork->pSelectCLWK=pWork->pokeIcon[POKETRADE_Line2RingLineIconGet(pWork->MainObjCursorLine)][pWork->MainObjCursorIndex];
            IRC_POKETRADE_SetCursorXY(pWork);
            _TouchStateCommonDecide(pWork);  //決定時の処理
            return;
          }
        }
      }
    }
  }

  switch(checkTouchCLACTPosition(pWork,FALSE)){  //タッチ時の処理
  case 2:
  case 3:
    return;
  case 1:
    GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
    pWork->bStatusDisp=TRUE;
    break;
  }


  if( !GFL_UI_TP_GetCont() ){ //離した  なげる
    if(pWork->touchON && pWork->bUpVec && pWork->pCatchCLWK){
      pWork->touchON = FALSE;
      pWork->bUpVec = FALSE;
      if(POKEMONTRADE_IsWazaPokemon(pWork, pWork->workBoxno, pWork->workPokeIndex)){// 交換できない技もち
        pWork->workBoxno = -1;
        pWork->workPokeIndex = -1;
 //       pWork->selectIndex = -1;
 //       pWork->selectBoxno = -1;
        _CatchPokemonRelease(pWork);
        _CHANGE_STATE(pWork,_notWazaChangePoke);
        return;
      }
      else{
        pWork->underSelectBoxno  = pWork->workBoxno;
        pWork->underSelectIndex = pWork->workPokeIndex;
        pWork->selectIndex = pWork->underSelectIndex;
        pWork->selectBoxno = pWork->underSelectBoxno;
        pWork->pCatchCLWK = NULL;
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, TRUE);
        TOUCHBAR_SetActive(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, TRUE);
        POKEMONTRADE_StartSucked(pWork);
        PMSND_PlaySE( POKETRADESE_THROW );
        _CHANGE_STATE(pWork, _POKE_SetAndSendData);
      }
      return;
    }
    else if(pWork->touchON && pWork->pCatchCLWK && pWork->bStatusDisp){
      pWork->underSelectBoxno = pWork->workBoxno;
      pWork->underSelectIndex = pWork->workPokeIndex;
      _TouchStateCommonDecide(pWork);  //決定時の処理
      return;
    }
    else{
      if(pWork->workPokeIndex != -1 && pWork->workBoxno != -1){
        _CatchPokemonPositionRewind(pWork);
        _PokemonIconRenew(pWork);
      }
      else{
        _CatchPokemonPositionRewind(pWork);
      }
    }
    pWork->touchON = FALSE;
    pWork->bUpVec = FALSE;
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[CELL_CUR_SCROLLBAR] , FALSE );
  }


  //パネルのスクロール
  _panelScroll(pWork);
  if(GFL_UI_TP_GetCont()){
    pWork->touchON = TRUE;
  }


  TOUCHBAR_Main(pWork->pTouchWork);   //タッチバー
  switch( TOUCHBAR_GetTouch(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
  case TOUCHBAR_ICON_CUTSOM1:
  case TOUCHBAR_ICON_CUTSOM2:
    _CHANGE_STATE(pWork, _touchPreMode_TouchStateCommon);
    break;
  }
}

//タッチボタンアニメ完了待ち
static void _touchPreMode_TouchStateCommon(POKEMON_TRADE_WORK* pWork)
{
  TOUCHBAR_Main(pWork->pTouchWork);   //タッチバー
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
    _CHANGE_STATE(pWork,_endRequestState);
    return;
  case TOUCHBAR_ICON_CUTSOM1:
    _CHANGE_STATE(pWork,_startSearchMojiState);
    return;
  case TOUCHBAR_ICON_CUTSOM2:
    if(POKEMONTRADEPROC_IsTriSelect(pWork)){
      if(POKE_GTS_IsMyIn(pWork)){
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE);
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
        _CatchPokemonRelease(pWork);
        _CHANGE_STATE(pWork, POKE_GTS_Select6MessageInit);
        return;
      }
      else{
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE);
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
      }
    }
    else{
      if(pWork->selectIndex == -1){  //ポケモン選んで無い場合元に戻す
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE);
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
      }
      else{
        _CHANGE_STATE(pWork, _changeMenuOpen);
        return;
      }
    }
    _CHANGE_STATE(pWork,POKE_TRADE_PROC_TouchStateCommon);
    break;
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
  BOOL bSelect=FALSE;
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrgAndSet()){
    GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
    bSelect=TRUE;
  }
  if(GFL_UI_TP_GetTrgAndSet()){
    GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
    bSelect=TRUE;
  }
  if(bSelect){
    POKETRADE_MESSAGE_WindowClear(pWork);
    POKEMONTRADE_2D_AlphaSet(pWork);
    _CHANGE_STATE(pWork,POKE_TRADE_PROC_TouchStateCommon);

    //20100629 カーソル状態を再描画 add Saito BTS7053
    pWork->oldLine = -1;
    IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork , TRUE ); 
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
  GFL_BG_Init( HEAPID_POKEMONTRADE_STATIC );
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

  //  if(pWork->type >= POKEMONTRADE_TYPE_VISUAL){
  //  return;
  //  }

  POKE_GTS_InitEruptedIconResource(pWork);

  if(pWork->type < POKEMONTRADE_TYPE_VISUAL){
    IRC_POKETRADE_InitBoxCursor(pWork);
  }
  IRC_POKETRADE_CreatePokeIconResource(pWork);

  IRC_POKETRADE_GraphicInitSubDisp(pWork);  //BGを
  IRC_POKETRADE_GraphicInitMainDisp(pWork);
  IRC_POKETRADE_SendVramBoxNameChar(pWork); // ボックス名初期化

//  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
//  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

//  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, _BRIGHTNESS_SYNC);

//  GFL_NET_ReloadIconTopOrBottom(TRUE, pWork->heapID);

  IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork , FALSE );  //ポケモンの表示

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

//------------------------------------------------------------------------------
/**
 * @brief   セーブ関連の初期化と開放
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _savedataHeapInit(POKEMON_TRADE_WORK* pWork,GAMEDATA* pGameData,BOOL bDebug)
{
  int i;
  
  for(i=0;i<elementof(pWork->TempBuffer);i++){
    pWork->TempBuffer[i] = GFL_HEAP_AllocClearMemory( HEAPID_IRCBATTLE, POKETOOL_GetWorkSize() );
  }

  if(pGameData){
    pWork->pGameData=pGameData;
    pWork->pBox = GAMEDATA_GetBoxManager(pGameData);
    pWork->pMy = GAMEDATA_GetMyStatus( pGameData );
    pWork->pMyParty = GAMEDATA_GetMyPokemon(pGameData);
    //    pWork->pMailBlock = GAMEDATA_GetMailBlock(pGameData);
    if(pWork->pFriend==NULL){
      if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
        pWork->pFriend = GAMEDATA_GetMyStatusPlayer(pGameData, 1-GFL_NET_SystemGetCurrentID());
      }
      else{
        pWork->pFriend = GAMEDATA_GetMyStatusPlayer(pGameData, 1);
      }
    }
  }
#if PM_DEBUG
  if(bDebug){

    GF_ASSERT(pWork->pBox);

    //    pWork->pMailBlock = GFL_HEAP_AllocClearMemory(pWork->heapID,MAIL_GetBlockWorkSize());
    //    MAIL_Init(pWork->pMailBlock);
    {
      POKEMON_PARAM *pp;
      int i,j;
      BOX_MANAGER* pBox = pWork->pBox;
      pp = PP_Create(MONSNO_509, 100, 123456, HEAPID_IRCBATTLE);

      for(i=1;i < 2;i++){
        for(j=0;j < 30;j++){
          u16 oyaName[5] = {L'デ',L'バ',L'ッ',L'グ',0xFFFF};
          POKEMON_PERSONAL_DATA* ppd = POKE_PERSONAL_OpenHandle(MONSNO_364+i, 0, GFL_HEAPID_APP);
          u32 ret = POKE_PERSONAL_GetParam(ppd,POKEPER_ID_sex);
          int monsno = GFUser_GetPublicRand(10)+MONSNO_386;
          PP_SetupEx(pp, monsno, i+j, 123456,PTL_SETUP_POW_AUTO, ret);
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
  int i;
  
  for(i=0;i<elementof(pWork->TempBuffer);i++){
    GFL_HEAP_FreeMemory(pWork->TempBuffer[i]);
  }

  if(bParentWork){
  }
#if PM_DEBUG
  else{
    BOX_DAT_ExitManager(pWork->pBox);
    GFL_HEAP_FreeMemory(pWork->pMy);
    //    GFL_HEAP_FreeMemory(pWork->pFriend);
    GFL_HEAP_FreeMemory(pWork->pMyParty);
    //    GFL_HEAP_FreeMemory(pWork->pMailBlock );
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

#if DEBUG_ONLY_FOR_ohno
  //  max = 24;
#endif
  pWork->TRADEBOX_LINEMAX = max * 6 + 2;
  pWork->BOX_TRAY_MAX = max;
  pWork->_SRCMAX = ((max*_BOXTRAY_SCR_MAX)+_TEMOTITRAY_SCR_MAX);
  pWork->_DOTMAX =  ((max*_BOXTRAY_MAX)+_TEMOTITRAY_MAX);
  //  pWork->_DOT_START_POS = (pWork->_DOTMAX - 80);
}

//------------------------------------------------------------------------------
/**
 * @brief        簡易ポケモン情報作成
 * @param pWork
 * @retval       none
 */
//------------------------------------------------------------------------------
static void _createEasyPokeInfo(POKEMON_TRADE_WORK *pWork, int index)
{
  int i,j;
  int monsno;
  POKEMON_PASO_PARAM *ppp;
  POKE_EASY_INFO *info;

  NOZOMU_Printf("===CreateInfo====\n");
  // 全ボックスの簡易ポケモン情報を作成する　add Saito
  //手持ち
  info = pWork->pokeInfo;

  if(index == 0){
    GFL_STD_MemClear(pWork->pokeInfo,sizeof(pWork->pokeInfo));

    for (i=0;i<6;i++)
    {
      ppp = IRCPOKEMONTRADE_GetPokeDataAddress( pWork->pBox, pWork->BOX_TRAY_MAX, i, pWork);
      if(!ppp) continue;
      if(PPP_Get( ppp, ID_PARA_poke_exist, NULL  ) == 0 ) continue;
      monsno = PPP_Get(ppp,ID_PARA_monsno_egg,NULL);
      if( monsno == 0 ) continue;
      info[i].monsno = monsno;
      info[i].formno = PPP_Get(ppp,ID_PARA_form_no,NULL);
      info[i].sex = PPP_Get(ppp,ID_PARA_sex,NULL);
    }
  }

  //ボックス
  for (i = index  ; i < BOX_MAX_TRAY ;i++)
  {
    for (j=0;j<BOX_MAX_COLUMN*BOX_MAX_RAW;j++)
    {
      int idx;
      idx = 6+i*BOX_MAX_COLUMN*BOX_MAX_RAW+j;
      ppp = IRCPOKEMONTRADE_GetPokeDataAddress( pWork->pBox, i, j, pWork);
      if(!ppp) continue;
      if(PPP_Get( ppp, ID_PARA_poke_exist, NULL  ) == 0 ) continue;
      monsno = PPP_Get(ppp,ID_PARA_monsno_egg,NULL);
      if( monsno == 0 ) continue;
      info[idx].monsno = monsno;
      info[idx].formno = PPP_Get(ppp,ID_PARA_form_no,NULL);
      info[idx].sex = PPP_Get(ppp,ID_PARA_sex,NULL);
    }
    break;
  }
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
  int i;
  GXS_SetVisibleWnd( GX_WNDMASK_NONE );
  POKETRADE_2D_GTSPokemonIconResetAll(pWork);

  IRC_POKETRADEDEMO_RemoveModel( pWork);
  POKE_GTS_ReleasePokeIconResource(pWork);
  POKE_GTS_DeleteEruptedIcon(pWork);
  POKE_GTS_SelectStatusMessageDelete(pWork);

  IRC_POKETRADE_ResetBoxNameWindow(pWork);
  IRCPOKETRADE_PokeDeleteMcss(pWork,1);
  POKEMONTRADE_RemovePokeSelectSixButton(pWork);

  POKE_GTS_DeleteFaceIcon(pWork);
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
  POKEMONTRADE_NEGOBG_SlideMessageDel(pWork,0);
  POKEMONTRADE_NEGOBG_SlideMessageDel(pWork,1);


  IRC_POKETRADE_MainGraphicExit(pWork);
  IRC_POKETRADE_SubGraphicExit(pWork);

  IRC_POKETRADE_ResetSubDispGraphic(pWork);

  if(pWork->pD2Fade){
    GFL_HEAP_FreeMemory(pWork->pD2Fade);
    pWork->pD2Fade=NULL;
  }
  for(i=0;i<4;i++){
    if(pWork->pMoveMcss[i]){
      GFL_HEAP_FreeMemory(pWork->pMoveMcss[i]);
      pWork->pMoveMcss[i]=NULL;
    }
  }

 // GFL_HEAP_DEBUG_PrintExistMemoryBlocks(pWork->heapID);

}


///サウンドのエラー時POP
static void _soundErrRecover(POKEMON_TRADE_WORK* pWork)
{
  switch(pWork->pushSound){
  case 2:
    PMSND_PopBGM();
    PMSND_PauseBGM( FALSE );
    //break throw
  case 1:
    PMSND_PopBGM();
    PMSND_PauseBGM( FALSE );
    PMSND_FadeInBGM( PMSND_FADE_SHORT );
    //break throw
  case 0:
    break;
  default:
    GF_ASSERT(!pWork->pushSound);
  }
  pWork->pushSound = 0;
}

FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(dpw_common);
FS_EXTERN_OVERLAY(pokemon_trade_ir);
FS_EXTERN_OVERLAY(pokemon_trade_co);


// ヒープ量削減の為staticで確保
#define _PROG_AREA_HEAP_SIZE (0xc000)
static u8 _PROG_AREA_HEAP_BUF[_PROG_AREA_HEAP_SIZE];

#define HEAPSIZE_POKETRADE (0xd2000-_PROG_AREA_HEAP_SIZE)   //  全共通 WiFiだとほぼマックス
#define HEAPSIZE_POKETRADE_DEMO (0xa2000-_PROG_AREA_HEAP_SIZE)   //デモ用 GTSにあわせてある


static POKEMON_TRADE_WORK* _initCreateHeap(GFL_PROC * proc,int size)
{
  POKEMON_TRADE_WORK *pWork;

  // ヒープ量削減の為staticで確保
  GFL_HEAP_CreateHeapInBuffer( _PROG_AREA_HEAP_BUF, _PROG_AREA_HEAP_SIZE, HEAPID_POKEMONTRADE_STATIC );
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, size );
  pWork = GFL_PROC_AllocWork( proc, sizeof( POKEMON_TRADE_WORK ), HEAPID_POKEMONTRADE_STATIC );
  GFL_STD_MemClear(pWork, sizeof(POKEMON_TRADE_WORK));
  return pWork;
}


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
  if(type < POKEMONTRADE_TYPE_GTS){
    GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
    GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));
  }

  if(type == POKEMONTRADE_TYPE_IRC || type == POKEMONTRADE_TYPE_EVENT){
    GFL_OVERLAY_Load( FS_OVERLAY_ID(pokemon_trade_ir));
  }
  else{
    GFL_OVERLAY_Load( FS_OVERLAY_ID(pokemon_trade_co));
  }

  GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);

  pWork->heapID = HEAPID_IRCBATTLE;
  pWork->type = type;
  pWork->pParentWork = pParentWork;
  pWork->recvPoke[0] = GFL_HEAP_AllocClearMemory(HEAPID_POKEMONTRADE_STATIC, POKETOOL_GetWorkSize());
  pWork->recvPoke[1] = GFL_HEAP_AllocClearMemory(HEAPID_POKEMONTRADE_STATIC, POKETOOL_GetWorkSize());
  pWork->pVramOBJ = GFL_HEAP_AllocMemory(HEAPID_POKEMONTRADE_STATIC, 16*2*16);  //バックアップ
  pWork->pVramBG = GFL_HEAP_AllocMemory(HEAPID_POKEMONTRADE_STATIC, 16*2*16);  //バックアップ

  {
    ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( pParentWork->gamedata );
    pWork->zenkoku_flag = ZUKANSAVE_GetZenkokuZukanFlag( zkn_sv );  // TRUEのとき全国図鑑
  }

  if(pParentWork){
    _savedataHeapInit(pWork,pParentWork->gamedata,pParentWork->bDebug);
  }
  _maxTrayNumInit(pWork);
  _mcssSystemHeapInit(pWork);
  _dispSystemHeapInit(pWork);

  //セル系システムの作成
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 340 , 0 , pWork->heapID );
  // 通信テーブル追加
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    GFL_NET_AddCommandTable(GFL_NET_CMD_IRCTRADE,_PacketTbl,NELEMS(_PacketTbl), pWork);
    GFL_NET_SetAutoErrorCheck(TRUE);
    GFL_NET_SetNoChildErrorCheck(TRUE);
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
    GFL_DISP_GX_SetVisibleControlDirect(GX_PLANEMASK_OBJ);
    _CHANGE_STATE(pWork, POKMEONTRADE_SAVE_TimingStart);
    pParentWork->ret=POKEMONTRADE_MOVE_START;
    return GFL_PROC_RES_FINISH;
  }

  POKETRADE_MESSAGE_HeapInit(pWork);
  IRC_POKETRADE_GraphicInitSubDispSys(pWork);
  if(pWork->type < POKEMONTRADE_TYPE_VISUAL){
    _dispInit(pWork);
    POKETRADE_TOUCHBAR_Init(pWork);
    GFL_NET_ReloadIconTopOrBottom(TRUE, pWork->heapID);
  }

#if DEBUG_ONLY_FOR_ohno | DEBUG_ONLY_FOR_ibe_mana
  DEBUGWIN_InitProc( GFL_BG_FRAME3_M , pWork->pFontHandle );  
  DEBUG_PAUSE_SetEnable( TRUE );
#endif

  IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);
  GFL_DISP_GX_SetVisibleControlDirect( 0 );
  GFL_DISP_GXS_SetVisibleControlDirect( 0 );

  _CHANGE_STATE(pWork, _touchState_BeforeTimeing1);

  return GFL_PROC_RES_FINISH;
}

//クラブ用データ
static GFL_PROC_RESULT PokemonTradeClubProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMON_TRADE_WORK *pWork;

  pWork = _initCreateHeap(proc,HEAPSIZE_POKETRADE);

  return PokemonTradeProcInit(proc,seq,pwk,pWork, POKEMONTRADE_TYPE_WIFICLUB);
}

// 赤外線用
static GFL_PROC_RESULT PokemonTradeIrcProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMON_TRADE_WORK *pWork;

  pWork = _initCreateHeap(proc,HEAPSIZE_POKETRADE);

  return PokemonTradeProcInit(proc,seq,pwk,pWork,POKEMONTRADE_TYPE_IRC);
}


// UNION用
static GFL_PROC_RESULT PokemonTradeUnionNegoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMONTRADE_PARAM* pParent=pwk;
  POKEMON_TRADE_WORK *pWork;
  int i;

  pWork = _initCreateHeap(proc,HEAPSIZE_POKETRADE);

  for(i=0;i<2;i++){
    pWork->GTStype[i] = 0;  //
    pWork->GTSlv[i] = 1;  //
  }
  pWork->pFriend = GAMEDATA_GetMyStatusPlayer(pParent->gamedata, 1-GFL_NET_SystemGetCurrentID());

  return PokemonTradeProcInit(proc,seq,pParent ,pWork,POKEMONTRADE_TYPE_UNION);
}

// GTSネゴシエーション用
static GFL_PROC_RESULT PokemonTradeGTSNegoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMONTRADE_PARAM* pParent=pwk;
  EVENT_GTSNEGO_WORK* pGTS=NULL;
  POKEMON_TRADE_WORK *pWork;
  int i;

  pGTS = pParent->pNego;
  pWork = _initCreateHeap(proc,HEAPSIZE_POKETRADE);

  if(pGTS){
    if(pGTS->result == EVENT_GTSNEGO_LV){
      for(i=0;i<2;i++){
        pWork->GTStype[i]=pGTS->aUser[i].selectType;  //
        pWork->GTSlv[i]=pGTS->aUser[i].selectLV;  //
      }
    }
    else{
      for(i=0;i<2;i++){
        pWork->GTStype[i]=-1;
        pWork->GTSlv[i]=-1;
      }
    }
    //    pWork->pFriend=pGTS->pStatus[1];
  }
  return PokemonTradeProcInit(proc,seq, pParent ,pWork, POKEMONTRADE_TYPE_GTSNEGO);
}



static void _commonFunc(POKEMONTRADE_DEMO_PARAM* pParent, POKEMON_TRADE_WORK *pWork)
{
  GFL_STD_MemCopy(pParent->pMyPoke, pWork->recvPoke[0] , POKETOOL_GetWorkSize());
  GFL_STD_MemCopy(pParent->pNPCPoke, pWork->recvPoke[1] , POKETOOL_GetWorkSize());

  //  OS_TPrintf("ポケモン自分 %d 相手%d \n",PP_Get(pParent->pMyPoke,ID_PARA_monsno_egg,NULL)
  //           ,PP_Get(pParent->pNPCPoke,ID_PARA_monsno_egg,NULL));


  IRCPOKETRADE_PokeCreateMcss(pWork, 0, 1, pParent->pMyPoke,TRUE);
  IRCPOKETRADE_PokeCreateMcss(pWork, 1, 1, pParent->pNPCPoke,TRUE);
  if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
    MCSS_SetVanishFlag( pWork->pokeMcss[1] );
  }
  else{
    MCSS_SetVanishFlag( pWork->pokeMcss[0] );
  }
  pWork->pMy = pParent->pMy;
  pWork->pFriend = pParent->pNPC;
  GFL_NET_ReloadIconTopOrBottom(FALSE, pWork->heapID);
  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, _BRIGHTNESS_SYNC);
}


//IRデモのみ表示
static GFL_PROC_RESULT PokemonTradeDemoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GFL_PROC_RESULT ret;
  POKEMONTRADE_DEMO_PARAM* pParent=pwk;
  POKEMON_TRADE_WORK *pWork;
  int i;

  pWork = _initCreateHeap(proc,HEAPSIZE_POKETRADE);

  pParent->aParam.gamedata = pParent->gamedata;
  ret = PokemonTradeProcInit(proc, seq, &pParent->aParam, pWork, POKEMONTRADE_TYPE_EVENT);

  _commonFunc(pParent, pWork);
  
  _CHANGE_STATE(pWork,POKMEONTRADE_IRCDEMO_ChangeDemo);

  return ret;
}



//GTSデモのみ表示
static GFL_PROC_RESULT PokemonTradeGTSDemoRecvProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GFL_PROC_RESULT ret;
  POKEMONTRADE_DEMO_PARAM* pParent=pwk;
  POKEMON_TRADE_WORK *pWork;
  int i;

  pWork = _initCreateHeap(proc,HEAPSIZE_POKETRADE_DEMO);

  pParent->aParam.gamedata = pParent->gamedata;
  ret = PokemonTradeProcInit(proc, seq, &pParent->aParam, pWork, POKEMONTRADE_TYPE_GTSDOWN);

  _commonFunc(pParent, pWork);

//  IRC_POKETRADE_ResetSubDispGraphic(pWork);
//  IRC_POKETRADE_SetSubdispGraphicDemo(pWork,0);  //通常背景
  _CHANGE_STATE(pWork,POKMEONTRADE_DEMO_GTSDOWN_ChangeDemo);

  return ret;
}



//GTSデモのみ表示
static GFL_PROC_RESULT PokemonTradeGTSDemoMidProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GFL_PROC_RESULT ret;
  POKEMONTRADE_DEMO_PARAM* pParent=pwk;
  POKEMON_TRADE_WORK *pWork;
  int i;

  pWork = _initCreateHeap(proc,HEAPSIZE_POKETRADE_DEMO);

  pParent->aParam.gamedata = pParent->gamedata;
  ret = PokemonTradeProcInit(proc, seq, &pParent->aParam, pWork, POKEMONTRADE_TYPE_GTSMID);

  _commonFunc(pParent, pWork);

  _CHANGE_STATE(pWork,POKMEONTRADE_DEMO_GTSMID_ChangeDemo);

  return ret;
}






static GFL_PROC_RESULT PokemonTradeGTSDemoSendProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GFL_PROC_RESULT ret;
  POKEMONTRADE_DEMO_PARAM* pParent=pwk;
  POKEMON_TRADE_WORK *pWork;
  int i;

  pWork = _initCreateHeap(proc,HEAPSIZE_POKETRADE_DEMO);

  pParent->aParam.gamedata = pParent->gamedata;
  ret = PokemonTradeProcInit(proc, seq, &pParent->aParam, pWork, POKEMONTRADE_TYPE_GTSUP);

  _commonFunc(pParent, pWork);
  _CHANGE_STATE(pWork,POKMEONTRADE_DEMO_GTSUP_ChangeDemo);
  return ret;
}


static GFL_PROC_RESULT PokemonTradeGTSDemoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GFL_PROC_RESULT ret;
  POKEMONTRADE_DEMO_PARAM* pParent=pwk;
  POKEMON_TRADE_WORK *pWork;
  int i;

  pWork = _initCreateHeap(proc,HEAPSIZE_POKETRADE);

  pParent->aParam.gamedata = pParent->gamedata;
  ret = PokemonTradeProcInit(proc, seq, &pParent->aParam, pWork, POKEMONTRADE_TYPE_GTSNEGODEMO);

  _commonFunc(pParent, pWork);
  _CHANGE_STATE(pWork,POKMEONTRADE_DEMO_GTSMID_ChangeDemo);
  // _CHANGE_STATE(pWork,IRC_POKMEONTRADE_STEP_ChangeDemo_PokeMove);
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
  int k;

  if(DEBUGWIN_IsActive()){
    return GFL_PROC_RES_CONTINUE;
  }

  if(state != NULL){
    state(pWork);
    pWork->anmCount++;

    retCode = GFL_PROC_RES_CONTINUE;
    for(k=0;k<2;k++){
      POKE_GTS_SelectStatusMessageDisp(pWork, k, pWork->pokemonGTSSeq[k], FALSE);
    }
  }
  //ポケモンセットをコール
  if(pWork->pokemonsetCall != 0){
    if(!pWork->statusModeOn){
      POKE_MAIN_Pokemonset(pWork, 1, pWork->recvPoke[pWork->pokemonsetCall-1] );
      pWork->pokemonsetCall=0;
    }
    //読み込む事ができない場合は書かない
  }
  POKEMONTRADE_DEMOBGMChange(pWork);
  POKE_GTS_FaceIconDeleteFunc(pWork);
  GFL_CLACT_SYS_Main(); // CLSYSメイン
  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }
  if(pWork->pAppWin){
    APP_TASKMENU_WIN_Update(pWork->pAppWin);
  }

  if(pWork->pokemonEnableSendFlg){
    if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), _NETCMD_SELECT_RETURN, 0, NULL)){
      pWork->pokemonEnableSendFlg = FALSE;
    }
  }

  if(pWork->pMsgTcblSys){
    u8 defL, defS, defB;
    GFL_FONTSYS_GetColor( &defL, &defS, &defB );
    GFL_FONTSYS_SetColor(FBMP_COL_BLACK, FBMP_COL_BLK_SDW, 15);
    GFL_TCBL_Main( pWork->pMsgTcblSys );
    GFL_FONTSYS_SetColor( defL, defS, defB );
    PRINTSYS_QUE_Main(pWork->SysMsgQue);
  }
#if 0
  {
    static int backup;

    if(backup !=pWork->BoxScrollNum){
      OS_TPrintf("pWork->BoxScrollNum %d \n",pWork->BoxScrollNum);
    }
    backup = pWork->BoxScrollNum;
  }
#endif

  POKEMONTRADE_NEGO_SlideMain(pWork);

  GFL_G3D_DRAW_Start();
  GFL_G3D_CAMERA_Switching( pWork->pCamera );

  GFL_G3D_DRAW_SetLookAt();

  MCSS_Main( pWork->mcssSys );
  MCSS_Draw( pWork->mcssSys );

  IRC_POKETRADEDEMO_Main(pWork);

  GFL_G3D_DRAW_End();

  if(POKEMONTRADEPROC_IsNetworkMode(pWork) && (pWork->pNetSave==NULL)){
    if(NET_ERR_CHECK_NONE != NetErr_App_CheckError()){
      if(pWork->bBackupStart){ //セーブのスタート このフラグが立ってたらエラー復帰不可能
        NetErr_App_FatalDispCallWifiMessage(dwc_message_0023);
      }
      if(pWork->pNHTTP){
        NHTTP_RAP_ErrorClean(pWork->pNHTTP);
        NHTTP_RAP_End(pWork->pNHTTP);
        pWork->pNHTTP  = NULL;
        GFL_NET_DWC_SetErrDisconnectCallback(NULL,NULL);
        DWC_RAPCOMMON_ResetSubHeapID();
      }
      if(GFL_NET_IsWifiConnect()){
        GFL_NET_DWC_ERROR_ReqErrorDisp(TRUE,TRUE);
      }
      else if(pWork->type != POKEMONTRADE_TYPE_UNION){  //ユニオンルームでのエラーはユニオンルーム側で管理するため画面を終了させる事だけを行う
        NetErr_App_ReqErrorDisp();
      }
      pWork->pParentWork->ret = POKEMONTRADE_MOVE_ERROR;
      _soundErrRecover(pWork);
      retCode = GFL_PROC_RES_FINISH;
      WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
      WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
    }
  }
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
  int type = pWork->type;

  if( WIPE_SYS_EndCheck() == FALSE ){
    return GFL_PROC_RES_CONTINUE;
  }

  GFL_HEAP_FreeMemory(pWork->pVramOBJ);
  GFL_HEAP_FreeMemory(pWork->pVramBG);
  IRC_POKETRADE_ItemIconReset(&pWork->aItemMark);
  IRC_POKETRADE_ItemIconReset(&pWork->aPokerusMark);
  IRC_POKETRADE_ItemIconReset(&pWork->aPokerusFaceMark);
  IRC_POKETRADE_EndSubMojiBG(pWork);
  POKMEONTRADE_RemoveCoreResource(pWork);

  DEBUGWIN_ExitProc();

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

  GF_ASSERT(pWork->pushSound == 0);
  
  GFL_PROC_FreeWork(proc);
#if PM_DEBUG
  GFL_HEAP_DEBUG_PrintExistMemoryBlocks(HEAPID_IRCBATTLE);
#endif// PM_DEBUG
  GFL_HEAP_DeleteHeap(HEAPID_IRCBATTLE);

  // ヒープ量削減の為staticで確保
  GFL_HEAP_DeleteHeap( HEAPID_POKEMONTRADE_STATIC );

  //オーバーレイ破棄
  if(type < POKEMONTRADE_TYPE_GTS){
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
  }

  if(type == POKEMONTRADE_TYPE_IRC || type == POKEMONTRADE_TYPE_EVENT){
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(pokemon_trade_ir));
  }
  else{
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(pokemon_trade_co));
  }

  
  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   ＬＲ押下後のラインカーソルライン再計算
 * @note    変更がかかる場合、カーソル位置を左から2列目（0オリジン）にする
 * @note    ただし、手持ちとボックス1のときはずれがあるので、例外処理する
 * 
 * @param   pWork       ワークポインタ
 * @param   bChange     変更があったか？
 *
 * @retval  none
 *
 */
//------------------------------------------------------------------------------
static void CalcLineAfterLR(POKEMON_TRADE_WORK* pWork, const BOOL bChange)
{
  //変更がかかる場合、カーソル位置を左から2列目（0オリジン）にする　20100628 add Saito BTS6919_a
  if ( bChange )
  {
    int setline = POKETRADE_boxScrollNum2Line(pWork);
    setline+=2;
    if (setline >= pWork->TRADEBOX_LINEMAX) setline -= pWork->TRADEBOX_LINEMAX;

    if ( pWork->BoxScrollNum == centerPOS[0] )
    {
      //ボックス1をさすようにする　20100630 add Saito
      NOZOMU_Printf("ボックス1を表示\n");
      setline = HAND_HORIZONTAL_NUM;
    }
    else if ( pWork->BoxScrollNum == centerPOS[pWork->BOX_TRAY_MAX] )
    {
      //手持ちをさすようにする　20100630 add Saito
      NOZOMU_Printf("手持ちを表示\n");
      setline = 0;
      //手持ちトレイ上ではカーソル位置の補正がある
      if(pWork->MainObjCursorIndex >= HAND_VERTICAL_NUM){
        pWork->MainObjCursorIndex = HAND_VERTICAL_NUM-1;
      }
    }
    pWork->MainObjCursorLine = setline;
    //強制書き直し
    pWork->oldLine = -1;
  }
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
#if 0 //DEBUG_ONLY_FOR_ohno
  PokemonTradeUnionNegoProcInit,
  //  PokemonTradeIrcProcInit,
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


//GTS用
const GFL_PROC_DATA PokemonTradeGTSProcData = {
  PokemonTradeGTSDemoProcInit,
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};

//GTSSend用
const GFL_PROC_DATA PokemonTradeGTSSendProcData = {
  PokemonTradeGTSDemoSendProcInit,
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};

//GTSRecv用
const GFL_PROC_DATA PokemonTradeGTSRecvProcData = {
  PokemonTradeGTSDemoRecvProcInit,
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};

//GTSMid用
const GFL_PROC_DATA PokemonTradeGTSMidProcData = {
  PokemonTradeGTSDemoMidProcInit,
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};




