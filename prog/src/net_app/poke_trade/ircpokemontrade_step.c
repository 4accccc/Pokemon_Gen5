//=============================================================================
/**
 * @file	  ircpokemontrade_step.c
 * @bfief	  ポケモン交換アニメーション
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/09/27
 */
//=============================================================================

#include <gflib.h>

#if PM_DEBUG
#define _TRADE_DEBUG (1)
#else
#define _TRADE_DEBUG (0)
#endif

#include "arc_def.h"
#include "net/network_define.h"

#include "ircpokemontrade.h"
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

#include "ircpokemontrade_local.h"


typedef enum{
  _TRADE01_SCENE,
  _TRADE02_SCENE,
  _TRADE03_SCENE,
  _TRADE04_SCENE,
  _TRADE05_SCENE,
  _TRADE06_SCENE,
  _TRADE07_SCENE,
  _TRADE08_SCENE,
  _TRADE09_SCENE,
  _TRADE10_SCENE,
  _TRADE11_SCENE,
  _TRADE12_SCENE,
  _TRADE13_SCENE,
  _TRADE14_SCENE,
  _TRADE15_SCENE,
  _TRADE16_SCENE,
  _TRADE17_SCENE,
  _TRADE18_SCENE,
  _TRADE19_SCENE,
  _TRADE20_SCENE,
  _TRADE21_SCENE,
  _TRADE22_SCENE,
  _TRADE23_SCENE,
  _TRADE24_SCENE,
} _TRADE_SCENE_NO_E;


static u16 tradeTimer[]={
  _TRADE01_ANMCOUNT,   //交換選択
  _TRADE02_ANMCOUNT,    //交換選択から真ん中に移動するまで
  _TRADE03_ANMCOUNT,    //真ん中からぼーるになるまで
  _TRADE04_ANMCOUNT,    //ボール発光
  _TRADE05_ANMCOUNT,   //光の筋 飛び開始
  _TRADE06_ANMCOUNT,    //飛んでいく
  _TRADE07_ANMCOUNT,    //光飛び交う空間
  _TRADE08_ANMCOUNT,    //上下ポケモン登場
  _TRADE09_ANMCOUNT,    //とまる
  _TRADE10_ANMCOUNT,    //ポケモン左右外に
  _TRADE11_ANMCOUNT,    //正面背面入れ替わって中に
  _TRADE12_ANMCOUNT,    //とまる
  _TRADE13_ANMCOUNT,    //上下に移動
  _TRADE14_ANMCOUNT,    //ホワイトアウト
  _TRADE15_ANMCOUNT,    //ボール振ってくる
  _TRADE16_ANMCOUNT,    //落ちてる最中
  _TRADE17_ANMCOUNT,    //ぶつかり波紋
  _TRADE18_ANMCOUNT,    //おちたエフェクト
  _TRADE19_ANMCOUNT,    //はもんひろがる
  _TRADE20_ANMCOUNT,    //ブラックアウト
  _TRADE21_ANMCOUNT,    //ボール落ちる
  _TRADE22_ANMCOUNT,    //？
  _TRADE23_ANMCOUNT,    //発光
  _TRADE24_ANMCOUNT,    //ポケモン出てくる
};



static void _changeDemo_ModelTrade3(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade4(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade5(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade6(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade7(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade8(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade9(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade10(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade11(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade12(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade13(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade14(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade15(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade16(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade17(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade18(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade19(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade20(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade21(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade22(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade23(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade24(IRC_POKEMON_TRADE* pWork);


//------------------------------------------------------------------
/**
 * @brief   次のシーンに必要な値をセット
 * @param   IRC_POKEMON_TRADE ワーク
 * @param   _TRADE_SCENE_NO_E no シーン管理enum
 * @retval  none
 */
//------------------------------------------------------------------

static void _setNextAnim(IRC_POKEMON_TRADE* pWork, _TRADE_SCENE_NO_E no)
{
  pWork->anmCount = tradeTimer[no];
  pWork->anmStep = no;
}



void IRC_POKMEONTRADE_STEP_ChangeDemo_PokeMove(IRC_POKEMON_TRADE* pWork)
{
  VecFx32 apos;
  int i;

  MCSS_GetPosition(pWork->pokeMcss[0],&apos);
  apos.x += FX32_ONE/32;
  apos.z += FX32_ONE/32;
  MCSS_SetPosition( pWork->pokeMcss[0] ,&apos );


  if(apos.x>(8*FX32_ONE)){

    IRC_POKETRADEDEMO_RemoveModel( pWork);

    IRC_POKETRADEDEMO_SetModel( pWork, TRADE01_OBJECT);

    for(i=0;i<2;i++){
      if( pWork->pokeMcss[i] ){
        MCSS_SetVanishFlag(pWork->pokeMcss[i]);
      }
    }
    for(i = 0;i< CUR_NUM;i++){
      if(pWork->curIcon[i]){
        GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[i], FALSE );
      }
    }
    GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );



    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_OBJ );
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_OBJ );
    IRC_POKETRADE_AllDeletePokeIconResource(pWork);

    IRC_POKETRADE_GraphicFreeVram(pWork);

    IRC_POKETRADE_SetSubdispGraphicDemo(pWork);
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );

    _setNextAnim(pWork, _TRADE03_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade3);
  }
}


static void _changeDemo_ModelTrade3(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){



    _setNextAnim(pWork, _TRADE04_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade4);
  }


}

static void _changeDemo_ModelTrade4(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE05_SCENE);

    
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade5);
  }
}
static void _changeDemo_ModelTrade5(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){


    _setNextAnim(pWork, _TRADE06_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade6);
  }
}

static void _changeDemo_ModelTrade6(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE07_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade7);
  }
}
static void _changeDemo_ModelTrade7(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    IRC_POKETRADEDEMO_RemoveModel( pWork);
    IRC_POKETRADEDEMO_SetModel( pWork, TRADE_TRADE_OBJECT);
    _setNextAnim(pWork, _TRADE08_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade8);
  }
}
static void _changeDemo_ModelTrade8(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE09_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade9);
  }
}
static void _changeDemo_ModelTrade9(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE10_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade10);
  }
}
static void _changeDemo_ModelTrade10(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE11_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade11);
  }
}
static void _changeDemo_ModelTrade11(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE12_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade12);
  }
}
static void _changeDemo_ModelTrade12(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE13_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade13);
  }
}
static void _changeDemo_ModelTrade13(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE14_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade14);
  }
}
static void _changeDemo_ModelTrade14(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE15_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade15);
  }
}
static void _changeDemo_ModelTrade15(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    IRC_POKETRADEDEMO_RemoveModel( pWork);
    IRC_POKETRADEDEMO_SetModel( pWork, TRADE_RETURN_OBJECT);

    _setNextAnim(pWork, _TRADE16_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade16);
  }
}
static void _changeDemo_ModelTrade16(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE17_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade17);
  }
}
static void _changeDemo_ModelTrade17(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    IRC_POKETRADEDEMO_RemoveModel( pWork);
    IRC_POKETRADEDEMO_SetModel( pWork, TRADE_SPLASH_OBJECT);
    _setNextAnim(pWork, _TRADE18_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade18);
  }
}
static void _changeDemo_ModelTrade18(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE19_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade19);
  }
}
static void _changeDemo_ModelTrade19(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE20_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade20);
  }
}
static void _changeDemo_ModelTrade20(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE21_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade21);
  }
}
static void _changeDemo_ModelTrade21(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE22_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade22);
  }
}
static void _changeDemo_ModelTrade22(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){
    _setNextAnim(pWork, _TRADE23_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade23);
  }
}
static void _changeDemo_ModelTrade23(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount==0){

    IRC_POKETRADEDEMO_RemoveModel( pWork);
    IRC_POKETRADEDEMO_SetModel( pWork, TRADE_END_OBJECT);

    _setNextAnim(pWork, _TRADE24_SCENE);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade24);
  }
}
static void _changeDemo_ModelTrade24(IRC_POKEMON_TRADE* pWork)
{
  if(GFL_UI_KEY_GetTrg()== PAD_BUTTON_DECIDE){
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3, VISIBLE_ON );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3, VISIBLE_ON );
    _CHANGE_STATE(pWork,IRC_POKMEONTRADE_ChangeFinish);
  }
}
