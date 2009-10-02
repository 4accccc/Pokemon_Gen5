//=============================================================================
/**
 * @file	  ircpokemontrade_step.c
 * @bfief	  ポケモン交換アニメーション
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/09/27
 */
//=============================================================================

#include <gflib.h>


#include "arc_def.h"
#include "net/network_define.h"
#include "system\wipe.h"

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


static void _changeDemo_ModelTrade3(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade24(IRC_POKEMON_TRADE* pWork);
static void _setFadeMask(int no);

//------------------------------------------------------------------
/**
 * @brief   次のシーンに必要な値をセット
 * @param   IRC_POKEMON_TRADE ワーク
 * @param   _TRADE_SCENE_NO_E no シーン管理enum
 * @retval  none
 */
//------------------------------------------------------------------

static void _setNextAnim(IRC_POKEMON_TRADE* pWork, int timer)
{
  pWork->anmCount = timer;
}



void IRC_POKMEONTRADE_STEP_ChangeDemo_PokeMove(IRC_POKEMON_TRADE* pWork)
{
  VecFx32 apos;
  int i;

  MCSS_GetPosition(pWork->pokeMcss[0], &apos);
  apos.x += FX32_ONE/32;
  apos.z += FX32_ONE/32;
  MCSS_SetPosition( pWork->pokeMcss[0] ,&apos );

  {
    static int test=0;
    _setFadeMask(test);
    if(test > -16)
      test--;
    
  }
  
  if(apos.x>(8*FX32_ONE)){

    IRC_POKETRADEDEMO_RemoveModel( pWork);

    IRC_POKETRADE_AllDeletePokeIconResource(pWork);
    for(i = 0;i< CUR_NUM;i++){
      if(pWork->curIcon[i]){
        GFL_CLACT_WK_Remove(pWork->curIcon[i]);
      }
    }

    for(i=0;i<2;i++){
      if( pWork->pokeMcss[i] ){
        //MCSS_SetVanishFlag(pWork->pokeMcss[i]);
        // 一時的に画面外に出す
        VecFx32 apos;
        apos.x -= FX32_ONE*30;
        apos.z -= FX32_ONE*30;
        MCSS_SetPosition( pWork->pokeMcss[i] ,&apos );

      }
    }

    
    GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );

    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_OBJ );
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_OBJ );

    IRC_POKETRADE_GraphicFreeVram(pWork);
    IRC_POKETRADE_ResetSubDispGraphic(pWork);

    IRC_POKETRADEDEMO_SetModel( pWork, TRADE01_OBJECT);

    IRC_POKETRADE_SetSubdispGraphicDemo(pWork);
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );

    _setNextAnim(pWork, 0);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade3);
  }
}


static void _changeDemo_ModelTrade3(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->anmCount == _BALL_PARTICLE_START){
    GFL_PTC_CreateEmitterCallback(pWork->ptc, 0, NULL, pWork);
  }
  
  if(pWork->anmCount == _POKEUP_WHITEOUT_START){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                   _POKEUP_WHITEOUT_TIMER, 1, pWork->heapID );

  }
  if(pWork->anmCount == _POKEUP_WHITEIN_START){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_WHITE,
                   _POKEUP_WHITEIN_TIMER, 1, pWork->heapID );
  }
  if(pWork->anmCount == _POKECHANGE_WHITEOUT_START){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                   _POKECHANGE_WHITEOUT_TIMER, 1, pWork->heapID );
  }
  if(pWork->anmCount == _POKECHANGE_WHITEIN_START){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_WHITE,
                   _POKECHANGE_WHITEIN_TIMER, 1, pWork->heapID );
  }
  if(pWork->anmCount == _POKESPLASH_WHITEOUT_START){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                   _POKESPLASH_WHITEOUT_TIMER, 1, pWork->heapID );
  }
  if(pWork->anmCount == _POKESPLASH_WHITEIN_START){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_WHITE,
                   _POKESPLASH_WHITEIN_TIMER, 1, pWork->heapID );
  }
  OS_TPrintf("C %d\n",pWork->anmCount);
  
  if(pWork->anmCount == (_DEMO_END-1)){
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade24);
  }
}
  

static void _changeDemo_ModelTrade24(IRC_POKEMON_TRADE* pWork)
{
  
  IRC_POKETRADEDEMO_RemoveModel( pWork);

  IRC_POKETRADE_SetBgMode(SETUP_TRADE_BG_MODE_NORMAL);
  GFL_BG_FreeBGControl(GFL_BG_FRAME3_S);


  IRC_POKETRADE_InitBoxCursor(pWork);  // タスクバー
  IRC_POKETRADE_CreatePokeIconResource(pWork);  // ポケモンアイコンCLACT+リソース常駐化
  
  IRC_POKETRADE_SetSubDispGraphic(pWork);

  IRC_POKETRADE_GraphicInit(pWork);  //BGを

  IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);
    
    
  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3 );
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  _CHANGE_STATE(pWork,IRC_POKMEONTRADE_ChangeFinish);
  
}

static void _setFadeMask(int no)
{
  G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|
                         GX_BLEND_PLANEMASK_OBJ, no);
}

