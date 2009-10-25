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
#include "poke_tool/pokeparty.h"
#include "poke_tool/status_rcv.h"
#include "tradedemo.naix"

#include "ircpokemontrade_local.h"
#include "spahead.h"


static void _changeDemo_ModelTrade0(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade1(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade2(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade3(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade20(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade21(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade22(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade23(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade24(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade25(IRC_POKEMON_TRADE* pWork);
static void _changeDemo_ModelTrade30(IRC_POKEMON_TRADE* pWork);
static void _changeTimingSaveStart(IRC_POKEMON_TRADE* pWork);
static void _changeTimingSaveStart2(IRC_POKEMON_TRADE* pWork);
static void _changeTimingSaveStart3(IRC_POKEMON_TRADE* pWork);
static void _changeTimingSaveStart4(IRC_POKEMON_TRADE* pWork);
static void _changeTimingSaveStart5(IRC_POKEMON_TRADE* pWork);
static void _setFadeMask(_D2_PAL_FADE_WORK* pD2Fade);
static void	_FIELD_StartPaletteFade( _EFFTOOL_PAL_FADE_WORK* epfw, u8 start_evy, u8 end_evy, u8 wait, u16 rgb );
static _EFFTOOL_PAL_FADE_WORK* _createPaletteFade(GFL_G3D_RES* g3DRES,HEAPID heapID);
static void	_freePaletteFade( _EFFTOOL_PAL_FADE_WORK* pwk );
static void  _EFFTOOL_CalcPaletteFade( _EFFTOOL_PAL_FADE_WORK *epfw );
static void _pokeMoveRenew(_POKEMCSS_MOVE_WORK* pPoke,int time, const VecFx32* pPos);
static _POKEMCSS_MOVE_WORK* _pokeMoveCreate(MCSS_WORK* pokeMcss, int time, const VecFx32* pPos, HEAPID heapID);
static void _pokeMoveFunc(_POKEMCSS_MOVE_WORK* pMove);





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

//------------------------------------------------------------------
/**
 * @brief   ポケモン登場CALCT素材INIT
 * @param   IRC_POKEMON_TRADE ワーク
 * @retval  none
 */
//------------------------------------------------------------------
static void _pokemonCreateCLACTInit(IRC_POKEMON_TRADE* pWork)
{
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_POKETRADEDEMO, pWork->heapID );

  pWork->cellRes[CHAR_POKECREATE] =
    GFL_CLGRP_CGR_Register( p_handle , NARC_tradedemo_trade_obj01_NCGR ,
                            FALSE , CLSYS_DRAW_MAIN , pWork->heapID );
  pWork->cellRes[PLT_POKECREATE] =
    GFL_CLGRP_PLTT_RegisterEx(
      p_handle ,NARC_tradedemo_trade_obj01_NCLR , CLSYS_DRAW_MAIN ,
      PLTID_OBJ_DEMO_M * 32 , 0, 1, pWork->heapID  );
  pWork->cellRes[ANM_POKECREATE] =
    GFL_CLGRP_CELLANIM_Register(
      p_handle , NARC_tradedemo_trade01_NCER,
      NARC_tradedemo_trade01_NANR , pWork->heapID  );
  GFL_ARC_CloseDataHandle( p_handle );
  pWork->pPokeCreateCLWK=NULL;



}
//------------------------------------------------------------------
/**
 * @brief   ポケモン登場CALCT素材Exit
 * @param   IRC_POKEMON_TRADE ワーク
 * @retval  none
 */
//------------------------------------------------------------------

static void _pokemonCreateCLACTExit(IRC_POKEMON_TRADE* pWork)
{
  GFL_CLACT_WK_Remove( pWork->pPokeCreateCLWK);
  GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_POKECREATE] );
  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_POKECREATE] );
  GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_POKECREATE] );
  pWork->cellRes[CHAR_POKECREATE] = 0;
  pWork->cellRes[PLT_POKECREATE] = 0;
  pWork->cellRes[ANM_POKECREATE] = 0;
  pWork->pPokeCreateCLWK=NULL;
}

//------------------------------------------------------------------
/**
 * @brief   ポケモン登場CALCTをだす
 * @param   IRC_POKEMON_TRADE ワーク
 * @retval  none
 */
//------------------------------------------------------------------
static void _pokemonCreateCLACTAdd(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->pPokeCreateCLWK==NULL){
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 99;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    pWork->pPokeCreateCLWK = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                  pWork->cellRes[CHAR_POKECREATE],
                                                  pWork->cellRes[PLT_POKECREATE],
                                                  pWork->cellRes[ANM_POKECREATE],
                                                  &cellInitData ,CLSYS_DRAW_MAIN , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->pPokeCreateCLWK , TRUE );
    GFL_CLACT_WK_SetDrawEnable( pWork->pPokeCreateCLWK, TRUE );
  }
  else{
    GFL_CLACT_WK_ResetAnm(pWork->pPokeCreateCLWK);
  }
}


static void _WIPE_SYS_StartRap(int pattern, int wipe_m, int wipe_s, u16 color, int division, int piece_sync, int heap)
{
  WIPE_SYS_Start(pattern, wipe_m, wipe_s, color, division, piece_sync, heap);
}

//MCSS終了の為のコールバック
static void _McssAnmStop( u32 data, fx32 currentFrame )
{
  IRC_POKEMON_TRADE* pWork = (IRC_POKEMON_TRADE*)data;

  MCSS_SetAnmStopFlag( pWork->pokeMcss[0]);
  pWork->mcssStop[0] = TRUE;
}


static void _pEmitCBInFunc( GFL_EMIT_PTR pEmiter, unsigned int flag)
{
  if( flag == SPL_EMITTER_CALLBACK_FRONT  ){
    return;
  }
  if(pEmiter ){
    IRC_POKEMON_TRADE* pWork = (IRC_POKEMON_TRADE*)GFL_PTC_GetUserData( pEmiter );
    if( pWork->icaBallin){
      VecFx32 pos={0,0,0};
      ICA_ANIME_GetTranslate( pWork->icaBallin, &pos );
      GFL_PTC_SetEmitterPosition(pEmiter, &pos);
    }
  }
}

static void _ballinEmitFunc(GFL_EMIT_PTR pEmiter)
{
  IRC_POKEMON_TRADE* pWork = (IRC_POKEMON_TRADE*)GFL_PTC_GetTempPtr();

  GFL_PTC_SetUserData( pEmiter, pWork );
  GFL_PTC_SetCallbackFunc(pEmiter, &_pEmitCBInFunc);

}

static void _pEmitCBOutFunc( GFL_EMIT_PTR pEmiter, unsigned int flag)
{
  if( flag == SPL_EMITTER_CALLBACK_FRONT  ){
    return;
  }
  if(pEmiter  ){
    IRC_POKEMON_TRADE* pWork = (IRC_POKEMON_TRADE*)GFL_PTC_GetUserData( pEmiter );
    if(pWork->icaBallout){
      VecFx32 pos={0,0,0};
      ICA_ANIME_GetTranslate( pWork->icaBallout, &pos );
      OS_TPrintf("---%d %d %d \n",pos.x / FX32_ONE,pos.y / FX32_ONE,pos.z / FX32_ONE );
      
      GFL_PTC_SetEmitterPosition(pEmiter, &pos);
    }
  }
}


static void _balloutEmitFunc(GFL_EMIT_PTR pEmiter)
{
  IRC_POKEMON_TRADE* pWork = (IRC_POKEMON_TRADE*)GFL_PTC_GetTempPtr();

  GFL_PTC_SetUserData( pEmiter, pWork );
  GFL_PTC_SetCallbackFunc(pEmiter, &_pEmitCBOutFunc);
}


void IRC_POKMEONTRADE_STEP_ChangeDemo_PokeMove(IRC_POKEMON_TRADE* pWork)
{
  VecFx32 apos;
  int i;

  
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade0);

}





void _changeDemo_ModelTrade0(IRC_POKEMON_TRADE* pWork)
{
  VecFx32 apos;
  int i;

  //待機アニメが止まるのを待つ
  MCSS_SetAnimCtrlCallBack(pWork->pokeMcss[0], (u32)pWork, _McssAnmStop, NNS_G2D_ANMCALLBACKTYPE_LAST_FRM);

  pWork->mcssStop[0] = FALSE;
  _setNextAnim(pWork, 0);

  pWork->pD2Fade = GFL_HEAP_AllocClearMemory(pWork->heapID, sizeof(_D2_PAL_FADE_WORK));
  pWork->pD2Fade->pal_fade_time = _POKEMON_CENTER_TIME;
  pWork->pD2Fade->pal_fade_nowcount = 0;
  pWork->pD2Fade->pal_start = 0;
  pWork->pD2Fade->pal_end = -16;

  if(pWork->modelno!=-1){
    pWork->pModelFade = _createPaletteFade(GFL_G3D_UTIL_GetResHandle(pWork->g3dUtil,0), pWork->heapID);
    _FIELD_StartPaletteFade( pWork->pModelFade, 0, 16, _POKEMON_CENTER_TIME/16, 0 );
  }

  {
    VecFx32 pos={_POKEMON_PLAYER_CENTER_POSX,_POKEMON_PLAYER_CENTER_POSY, _POKEMON_PLAYER_CENTER_POSZ};
    pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[0], _POKEMON_CENTER_TIME, &pos, pWork->heapID);
  }

  MCSS_SetAnimCtrlCallBack(pWork->pokeMcss[0], (u32)pWork, _McssAnmStop, NNS_G2D_ANMCALLBACKTYPE_LAST_FRM);

  MCSS_SetPaletteFade( pWork->pokeMcss[1], 0, 16, _POKEMON_CENTER_TIME/16, 0 );
  IRC_POKETRADE_MessageWindowClose(pWork);

  GFL_BG_SetBackGroundColor(GFL_BG_FRAME1_M ,0);
  GFL_BG_SetBackGroundColor(GFL_BG_FRAME1_S ,0);

  _setNextAnim(pWork, 0);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade1);

  pWork->pBallInPer = NULL;
  pWork->pBallOutPer = NULL;


}

static void _changeDemo_ModelTrade1(IRC_POKEMON_TRADE* pWork)
{
  int i;

  {  // フェード中
    _setFadeMask(pWork->pD2Fade);
    if(pWork->pModelFade){
      _EFFTOOL_CalcPaletteFade(pWork->pModelFade);
    }
    _pokeMoveFunc(pWork->pMoveMcss[0]);
  }

  if(pWork->mcssStop[0]!=TRUE){  //アニメ終了待ち
    return;
  }

  if(pWork->anmCount > _POKEMON_CENTER_TIME){  //フェード完了
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_OBJ );
    GFL_DISP_GXS_SetVisibleControlDirect( 0 );
    IRC_POKETRADEDEMO_RemoveModel( pWork);

//    IRC_POKETRADEDEMO_End(pWork);
//    IRC_POKETRADEDEMO_Init(pWork);
    
    for(i = 0;i< CELL_DISP_NUM;i++){
      if(pWork->curIcon[i]){
        GFL_CLACT_WK_Remove(pWork->curIcon[i]);
      }
    }
    IRC_POKETRADE_ResetBoxNameWindow(pWork);
    IRCPOKETRADE_PokeDeleteMcss(pWork,1);
    IRC_POKETRADE_AllDeletePokeIconResource(pWork);
    IRC_POKETRADE_GraphicFreeVram(pWork);
    IRC_POKETRADE_ResetSubDispGraphic(pWork);
    IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork,0);
    IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork,1);
    GFL_HEAP_FreeMemory(pWork->pD2Fade);
    pWork->pD2Fade=NULL;
    _freePaletteFade(pWork->pModelFade);
    pWork->pModelFade = NULL;
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
    _setNextAnim(pWork, 0);

    _CHANGE_STATE(pWork,_changeDemo_ModelTrade2);
  }
}

static void _changeDemo_ModelTrade2(IRC_POKEMON_TRADE* pWork)
{
  int i;
  
//  if(pWork->anmCount == 1){

  //ボール変更アニメ
  G2_BlendNone();
  //  return;
//  }
  


  IRC_POKETRADEDEMO_SetModel( pWork, TRADE01_OBJECT);

  IRC_POKETRADE_SetSubdispGraphicDemo(pWork);
  G2S_BlendNone();


  _setNextAnim(pWork, 0);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade3);


}






static void _changeDemo_ModelTrade3(IRC_POKEMON_TRADE* pWork)
{
  _pokeMoveFunc(pWork->pMoveMcss[0]);
  _pokeMoveFunc(pWork->pMoveMcss[1]);


  if(pWork->anmCount == (_POKEMON_DELETE_TIME-5)){
    //自分を白くする
    MCSS_SetPaletteFade( pWork->pokeMcss[0], 8, 8, 0, 0xffff );
  }
  if(pWork->anmCount == (_POKEMON_DELETE_TIME-4)){
    //自分を白くする
    MCSS_SetPaletteFade( pWork->pokeMcss[0], 16, 16, 0, 0xffff );
  }

  if(pWork->anmCount == (_POKEMON_DELETE_TIME-3)){
    _pokemonCreateCLACTInit(pWork);
    _pokemonCreateCLACTAdd(pWork);
  }
  
  if(pWork->anmCount == _POKEMON_DELETE_TIME){
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_OBJ );
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  }

  if(pWork->anmCount == _POKEMON_DELETE_TIME){
    if( pWork->pokeMcss[0] ){
      IRCPOKETRADE_PokeDeleteMcss(pWork,0);
    }
  }


  if(pWork->anmCount == _BALL_PARTICLE_START){
    GFL_PTC_CreateEmitterCallback(pWork->ptc[PTC_KIND_DEMO1], DERMO_TEX001, NULL, pWork);
  }
  if(pWork->anmCount == _PARTICLE_DEMO2_START){
    GFL_PTC_CreateEmitterCallback(pWork->ptc[PTC_KIND_DEMO2], DEMO_TEX002, NULL, pWork);
  }
  if(pWork->anmCount == _PARTICLE_DEMO3_START){
    GFL_PTC_CreateEmitterCallback(pWork->ptc[PTC_KIND_DEMO3], DEMO_TEX003, NULL, pWork);
  }
  if(pWork->anmCount == _PARTICLE_DEMO3_START2){
    GFL_PTC_CreateEmitterCallback(pWork->ptcOrthogonal, DEMO_TEX003, NULL, pWork);
  }
  if(pWork->anmCount == _PARTICLE_DEMO4_START){
    GFL_PTC_CreateEmitterCallback(pWork->ptc[PTC_KIND_DEMO4], DEMO_TEX004,  NULL, pWork);
  }
  if(pWork->anmCount == _PARTICLE_DEMO4_START2){
    GFL_PTC_CreateEmitterCallback(pWork->ptc[PTC_KIND_DEMO11], DEMO_TEX004,  NULL, pWork);
  }
  if(pWork->anmCount == _PARTICLE_DEMO5_START){
    GFL_PTC_CreateEmitterCallback(pWork->ptc[PTC_KIND_DEMO5], DEMO_TEX005, _ballinEmitFunc, pWork);
  }
  if(pWork->anmCount == _PARTICLE_DEMO6_START){
    GFL_PTC_CreateEmitterCallback(pWork->ptc[PTC_KIND_DEMO6], DEMO_TEX006, NULL, pWork);
  }
  if(pWork->anmCount == _PARTICLE_DEMO7_START){
    GFL_PTC_CreateEmitterCallback(pWork->ptc[PTC_KIND_DEMO7], DEMO_TEX007,_balloutEmitFunc, pWork);
  }
  if(pWork->anmCount == _PARTICLE_DEMO8_START){
    GFL_PTC_CreateEmitterCallback(pWork->ptc[PTC_KIND_DEMO8], DEMO_TEX008,NULL, pWork);
  }
  if(pWork->anmCount == _PARTICLE_DEMO9_START){
    GFL_PTC_CreateEmitterCallback(pWork->ptc[PTC_KIND_DEMO9], DEMO_TEX009,NULL, pWork);
  }
  if(pWork->anmCount == _PARTICLE_DEM10_START){
    GFL_PTC_CreateEmitterCallback(pWork->ptc[PTC_KIND_DEMO10], DEMO_TEX010,NULL, pWork);
  }

  /*  if(pWork->anmCount == _OAM_POKECREATE_START){
    _pokemonCreateCLACTAdd(pWork);
  }*/
  if(pWork->anmCount == _OAM_POKECREATE_START2){
    _pokemonCreateCLACTAdd(pWork);
  }
  if(pWork->anmCount == _POKEUP_WHITEOUT_START){
    _WIPE_SYS_StartRap(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                       _POKEUP_WHITEOUT_TIMER, 1, pWork->heapID );

  }
  if(pWork->anmCount == _POKEUP_WHITEIN_START){
    _WIPE_SYS_StartRap(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_WHITE,
                       _POKEUP_WHITEIN_TIMER, 1, pWork->heapID );
  }

  if(pWork->anmCount == _POKE_APPEAR_START){
    IRCPOKETRADE_PokeCreateMcss(pWork, 0, 0, IRC_POKEMONTRADE_GetRecvPP(pWork,0) );
    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_PLAYER_APPEAR_POSX;
      apos.y = _POKEMON_PLAYER_APPEAR_POSY;
      apos.z = _POKEMON_PLAYER_APPEAR_POSZ;
      MCSS_SetPosition( pWork->pokeMcss[0] ,&apos );
      apos.x = _POKEMON_PLAYER_UP_POSX;
      apos.y = _POKEMON_PLAYER_UP_POSY;
      apos.z = _POKEMON_PLAYER_UP_POSZ;
      pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[0], _POKE_APPEAR_TIME, &apos, pWork->heapID);
      MCSS_SetAnmStopFlag(pWork->pokeMcss[0]);

      MCSS_GetScale( pWork->pokeMcss[0], &apos );
      apos.x *= _POKMEON_SCALE_SIZE;
      apos.y *= _POKMEON_SCALE_SIZE;
      MCSS_SetScale( pWork->pokeMcss[0], &apos );
    }
  }

  if(pWork->anmCount == _POKE_APPEAR_START+1){
    IRCPOKETRADE_PokeCreateMcss(pWork, 1, 1, IRC_POKEMONTRADE_GetRecvPP(pWork,1) );
    {
      VecFx32 apos;
      apos.x = _POKEMON_FRIEND_APPEAR_POSX;
      apos.y = _POKEMON_FRIEND_APPEAR_POSY;
      apos.z = _POKEMON_FRIEND_APPEAR_POSZ;
      MCSS_SetPosition( pWork->pokeMcss[1] ,&apos );

      apos.x = _POKEMON_FRIEND_DOWN_POSX;
      apos.y = _POKEMON_FRIEND_DOWN_POSY;
      apos.z = _POKEMON_FRIEND_DOWN_POSZ;
      pWork->pMoveMcss[1] = _pokeMoveCreate(pWork->pokeMcss[1], _POKE_APPEAR_TIME, &apos , pWork->heapID);
      MCSS_SetAnmStopFlag(pWork->pokeMcss[1]);
    }
    //移動設定
  }

  if(pWork->anmCount == _POKE_APPEAR_START2){
    VecFx32 apos;
    apos.x = _POKEMON_PLAYER_UP_POSX;
    apos.y = _POKEMON_PLAYER_UP_POSY;
    apos.z = _POKEMON_PLAYER_UP_POSZ;
    _pokeMoveRenew(pWork->pMoveMcss[0],_POKE_APPEAR_TIME2,&apos);
    apos.x = _POKEMON_FRIEND_DOWN_POSX;
    apos.y = _POKEMON_FRIEND_DOWN_POSY;
    apos.z = _POKEMON_FRIEND_DOWN_POSZ;
    _pokeMoveRenew(pWork->pMoveMcss[1],_POKE_APPEAR_TIME2,&apos);
  }
  
  if(_POKE_SIDEOUT_START == pWork->anmCount){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[1]);
    pWork->pMoveMcss[1]=NULL;

    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_PLAYER_SIDEOUT_POSX;
      apos.y = _POKEMON_PLAYER_SIDEOUT_POSY;
      apos.z = _POKEMON_PLAYER_SIDEOUT_POSZ;
      pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[0], _POKE_SIDEOUT_TIME, &apos, pWork->heapID);
      apos.x = _POKEMON_FRIEND_SIDEOUT_POSX;
      apos.y = _POKEMON_FRIEND_SIDEOUT_POSY;
      apos.z = _POKEMON_FRIEND_SIDEOUT_POSZ;
      pWork->pMoveMcss[1] = _pokeMoveCreate(pWork->pokeMcss[1], _POKE_SIDEOUT_TIME, &apos , pWork->heapID);
    }
  }

  if(_POKE_SIDEIN_START == pWork->anmCount){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;

    IRCPOKETRADE_PokeDeleteMcss(pWork, 0);
    IRCPOKETRADE_PokeCreateMcss(pWork, 0, 1, IRC_POKEMONTRADE_GetRecvPP(pWork,0) );


    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_PLAYER_SIDEST_POSX;
      apos.y = _POKEMON_PLAYER_SIDEST_POSY;
      apos.z = _POKEMON_PLAYER_SIDEST_POSZ;
      MCSS_SetPosition( pWork->pokeMcss[0] ,&apos );
      apos.x = _POKEMON_PLAYER_SIDEIN_POSX;
      apos.y = _POKEMON_PLAYER_SIDEIN_POSY;
      apos.z = _POKEMON_PLAYER_SIDEIN_POSZ;
      pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[0], _POKE_SIDEIN_TIME, &apos, pWork->heapID);
      //MCSS_SetAnmStopFlag(pWork->pokeMcss[0]);
    }
  }
  if(_POKE_SIDEIN_START+1 == pWork->anmCount){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[1]);
    pWork->pMoveMcss[1]=NULL;

    IRCPOKETRADE_PokeDeleteMcss(pWork, 1);
    IRCPOKETRADE_PokeCreateMcss(pWork, 1, 0, IRC_POKEMONTRADE_GetRecvPP(pWork,1) );


    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_FRIEND_SIDEST_POSX;
      apos.y = _POKEMON_FRIEND_SIDEST_POSY;
      apos.z = _POKEMON_FRIEND_SIDEST_POSZ;
      MCSS_SetPosition( pWork->pokeMcss[1] ,&apos );
      apos.x = _POKEMON_FRIEND_SIDEIN_POSX;
      apos.y = _POKEMON_FRIEND_SIDEIN_POSY;
      apos.z = _POKEMON_FRIEND_SIDEIN_POSZ;
      pWork->pMoveMcss[1] = _pokeMoveCreate(pWork->pokeMcss[1], _POKE_SIDEIN_TIME, &apos , pWork->heapID);

      MCSS_GetScale( pWork->pokeMcss[1], &apos );
      apos.x *= _POKMEON_SCALE_SIZE;
      apos.y *= _POKMEON_SCALE_SIZE;
      MCSS_SetScale( pWork->pokeMcss[1], &apos );
      //MCSS_SetAnmStopFlag(pWork->pokeMcss[1]);

    }
  }
  if(_POKE_LEAVE_START == pWork->anmCount){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[1]);
    pWork->pMoveMcss[1]=NULL;


    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_PLAYER_LEAVE_POSX;
      apos.y = _POKEMON_PLAYER_LEAVE_POSY;
      apos.z = _POKEMON_PLAYER_LEAVE_POSZ;
      pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[0], _POKE_LEAVE_TIME, &apos, pWork->heapID);
      apos.x = _POKEMON_FRIEND_LEAVE_POSX;
      apos.y = _POKEMON_FRIEND_LEAVE_POSY;
      apos.z = _POKEMON_FRIEND_LEAVE_POSZ;
      pWork->pMoveMcss[1] = _pokeMoveCreate(pWork->pokeMcss[1], _POKE_LEAVE_TIME, &apos , pWork->heapID);
    }
  }
  if((_POKEMON_CREATE_TIME-3) == pWork->anmCount){
    IRCPOKETRADE_PokeDeleteMcss(pWork, 1);
    IRCPOKETRADE_PokeCreateMcss(pWork, 1, 1, IRC_POKEMONTRADE_GetRecvPP(pWork,1) );
    {
      VecFx32 apos;
      apos.x = _POKEMON_PLAYER_CENTER_POSX;
      apos.y = _POKEMON_PLAYER_CENTER_POSY;
      apos.z = _POKEMON_PLAYER_CENTER_POSZ;
      MCSS_SetPosition( pWork->pokeMcss[1] ,&apos );
      MCSS_SetAnmStopFlag(pWork->pokeMcss[1]);
      MCSS_SetVanishFlag(pWork->pokeMcss[1]);
    }

  }

#if 1
  if((_POKEMON_CREATE_TIME) == pWork->anmCount){
    MCSS_ResetVanishFlag(pWork->pokeMcss[1]);
    MCSS_SetPaletteFade( pWork->pokeMcss[1], 16, 16, 0, 0x7fff );
  }
  if((_POKEMON_CREATE_TIME+1) == pWork->anmCount){
    MCSS_SetPaletteFade( pWork->pokeMcss[1], 8, 8, 0, 0x7fff );
  }
  if((_POKEMON_CREATE_TIME+2) == pWork->anmCount){
    MCSS_SetPaletteFade( pWork->pokeMcss[1], 0, 0, 0, 0x7fff  );
  }
#endif


  if(pWork->anmCount == _POKECHANGE_WHITEOUT_START){
    _WIPE_SYS_StartRap(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                       _POKECHANGE_WHITEOUT_TIMER, 1, pWork->heapID );
  }
  if(pWork->anmCount == _POKECHANGE_WHITEIN_START){
    _WIPE_SYS_StartRap(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_WHITE,
                       _POKECHANGE_WHITEIN_TIMER, 1, pWork->heapID );
  }
  if(pWork->anmCount == _POKESPLASH_WHITEOUT_START){
    _WIPE_SYS_StartRap(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                       _POKESPLASH_WHITEOUT_TIMER, 1, pWork->heapID );
  }
  if(pWork->anmCount == _POKESPLASH_WHITEIN_START){
    _WIPE_SYS_StartRap(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_WHITE,
                       _POKESPLASH_WHITEIN_TIMER, 1, pWork->heapID );
  }
  OS_TPrintf("C %d\n",pWork->anmCount);

  if(pWork->anmCount == (_DEMO_END-1)){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[1]);
    pWork->pMoveMcss[1]=NULL;
    IRC_POKETRADEDEMO_RemoveModel( pWork);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade20);
  }
}


static void _changeDemo_ModelTrade20(IRC_POKEMON_TRADE* pWork)
{

  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_49, pWork->pMessageStrBufEx );
  {
    POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, 1);
    WORDSET_RegisterPokeNickName( pWork->pWordSet, 1,  pp );
  }
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBuf, pWork->pMessageStrBufEx);

  pWork->bgchar = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME2_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
  IRC_POKETRADE_MessageWindowOpen(pWork);
  _setNextAnim(pWork, 0);

  GFL_BG_SetVisible( GFL_BG_FRAME2_S , TRUE );
  
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade21);

}

static void _changeDemo_ModelTrade21(IRC_POKEMON_TRADE* pWork)
{
  if(!IRC_POKETRADE_MessageEndCheck(pWork)){
    return;
  }
  if(pWork->anmCount < 100){
    return;
  }
  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_50, pWork->pMessageStrBuf );
  IRC_POKETRADE_MessageWindowOpen(pWork);
  _setNextAnim(pWork, 0);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade22);

}

static void _changeDemo_ModelTrade22(IRC_POKEMON_TRADE* pWork)
{
  if(!IRC_POKETRADE_MessageEndCheck(pWork)){
    return;
  }
  if(pWork->anmCount < 100){
    return;
  }
  _setNextAnim(pWork, 0);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade23);
}


static void _changeDemo_ModelTrade23(IRC_POKEMON_TRADE* pWork)
{
  IRCPOKETRADE_PokeDeleteMcss(pWork, 0);
  IRCPOKETRADE_PokeDeleteMcss(pWork, 1);


  {
    int id = 1-GFL_NET_SystemGetCurrentID();
    //交換する
    // 相手のポケを自分の選んでいた場所に入れる
    if(pWork->selectBoxno == BOX_MAX_TRAY){ //もちものの交換の場合
      POKEPARTY* party = pWork->pMyParty;

      STATUS_RCV_PokeParam_RecoverAll(pWork->recvPoke[id]);

      PokeParty_SetMemberData(party, pWork->selectIndex, pWork->recvPoke[id]);
    }
    else{
      // @todo  メールがあったらボックスに
      // @todo  メールボックスが満タンなら削除処理に
      BOXDAT_PutPokemonPos(pWork->pBox, pWork->selectBoxno, pWork->selectIndex, (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst(pWork->recvPoke[id]));
      //_CHANGE_STATE(pWork,_mailBox);
    }
  }

  if(!pWork->pGameData){
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade30); //ゲームデータの引渡しが無い場合セーブに行かない
    return;
  }
  else{
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_51, pWork->pMessageStrBuf );
    IRC_POKETRADE_MessageWindowOpen(pWork);
    _CHANGE_STATE(pWork, _changeTimingSaveStart);
  }
}



static void _changeTimingSaveStart(IRC_POKEMON_TRADE* pWork)
{
  if(!IRC_POKETRADE_MessageEndCheck(pWork)){
    return;
  }
  if(GFL_NET_IsInit()){
    GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVEST);
    _CHANGE_STATE(pWork, _changeTimingSaveStart2);
  }
  else{
    GAMEDATA_SaveAsyncStart(pWork->pGameData);
    _CHANGE_STATE(pWork, _changeTimingSaveStart3);
  }

}
static void _changeTimingSaveStart2(IRC_POKEMON_TRADE* pWork)
{
  if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVEST)){
    GAMEDATA_SaveAsyncStart(pWork->pGameData);
    _CHANGE_STATE(pWork,_changeTimingSaveStart3);
  }
}

static void _changeTimingSaveStart3(IRC_POKEMON_TRADE* pWork)
{
  if(GAMEDATA_SaveAsyncMain(pWork->pGameData) == SAVE_RESULT_LAST){
    if(GFL_NET_IsInit()){
      pWork->saveStep = GFUser_GetPublicRand(_CHANGERAND);
      _CHANGE_STATE(pWork,_changeTimingSaveStart4);
    }
    else{
      _CHANGE_STATE(pWork,_changeDemo_ModelTrade25);
    }
  }
}

static void _changeTimingSaveStart4(IRC_POKEMON_TRADE* pWork)
{
  pWork->saveStep--;
  if(pWork->saveStep<0){
    if(GFL_NET_IsInit()){
      GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVELAST);
      _CHANGE_STATE(pWork, _changeTimingSaveStart5);
    }
  }
}

static void _changeTimingSaveStart5(IRC_POKEMON_TRADE* pWork)
{
  if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_SAVELAST)){
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade25);
  }
}

static void _changeDemo_ModelTrade25(IRC_POKEMON_TRADE* pWork)
{
  if(GAMEDATA_SaveAsyncMain(pWork->pGameData) == SAVE_RESULT_OK){
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade30);
  }
}

static void _changeDemo_ModelTrade30(IRC_POKEMON_TRADE* pWork)
{
  GFL_DISP_GX_SetVisibleControlDirect( 0 );
  GFL_DISP_GXS_SetVisibleControlDirect( 0 );

  _pokemonCreateCLACTExit(pWork);
  IRC_POKETRADE_MessageWindowClose(pWork);
  IRC_POKETRADE_GraphicFreeVram(pWork);

  GFL_BG_FillCharacterRelease(GFL_BG_FRAME2_S,1,0);
  GFL_BG_FreeBGControl(GFL_BG_FRAME2_S);
  GFL_BG_FreeBGControl(GFL_BG_FRAME3_S);

  IRC_POKETRADE_SetBgMode(SETUP_TRADE_BG_MODE_NORMAL);

  IRC_POKETRADE_SetSubDispGraphic(pWork);

  IRC_POKETRADE_InitBoxCursor(pWork);  // タスクバー
  IRC_POKETRADE_CreatePokeIconResource(pWork);  // ポケモンアイコンCLACT+リソース常駐化

  IRC_POKETRADE_GraphicInitMainDisp(pWork);
  IRC_POKETRADE_GraphicInitSubDisp(pWork);

  IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);

  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3 );
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  _CHANGE_STATE(pWork,IRC_POKMEONTRADE_ChangeFinish);

}

static void _setFadeMask(_D2_PAL_FADE_WORK* pD2Fade)
{
  if(!pD2Fade){
    return;
  }
  if(pD2Fade->pal_fade_time < pD2Fade->pal_fade_nowcount){
    return;
  }
  pD2Fade->pal_fade_nowcount++;
  {
    int upper = pD2Fade->pal_end - pD2Fade->pal_start;
    upper *= 4096;
    upper = upper / pD2Fade->pal_fade_time;
    upper = upper * pD2Fade->pal_fade_nowcount;
    upper /= 4096;

    G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|
                           GX_BLEND_PLANEMASK_OBJ, upper);
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|
                            GX_BLEND_PLANEMASK_OBJ, upper);
  }

}

//--------------------------------------------------
/**
 * @brief パレットフェードスタート
 * @param[in]	bfw	      _EFFTOOL_PAL_FADE_WORK管理ワークへのポインタ
 * @param[in]	start_evy	セットするパラメータ（フェードさせる色に対する開始割合16段階）
 * @param[in]	end_evy		セットするパラメータ（フェードさせる色に対する終了割合16段階）
 * @param[in]	wait			セットするパラメータ（ウェイト）
 * @param[in]	rgb				セットするパラメータ（フェードさせる色）
 */
//--------------------------------------------------
static void	_FIELD_StartPaletteFade( _EFFTOOL_PAL_FADE_WORK* epfw, u8 start_evy, u8 end_evy, u8 wait, u16 rgb )
{

  epfw->pal_fade_flag      = 1;
  epfw->pal_fade_start_evy = start_evy;
  epfw->pal_fade_end_evy   = end_evy;
  epfw->pal_fade_wait      = 0;
  epfw->pal_fade_wait_tmp  = wait;
  epfw->pal_fade_rgb       = rgb;
}


//----------------------------------------
/**
 * @brief パレットフェード構造体作成
 * @param[in]	g3DRES	  GFL_G3D_RES
 * @param[in]	heapID	  HEAPID
 * @retturn   パレットフェード構造体
 */
//----------------------------------------

static _EFFTOOL_PAL_FADE_WORK* _createPaletteFade(GFL_G3D_RES* g3DRES,HEAPID heapID)
{
  //パレットフェード用ワーク生成
  NNSG3dResFileHeader*	header = GFL_G3D_GetResourceFileHeader( g3DRES );
  NNSG3dResTex*		    	pTex = NNS_G3dGetTex( header );
  u32                   size = (u32)pTex->plttInfo.sizePltt << 3;
  _EFFTOOL_PAL_FADE_WORK* pwk = GFL_HEAP_AllocMemory(heapID,sizeof(_EFFTOOL_PAL_FADE_WORK));

  pwk->g3DRES = g3DRES;
  pwk->pData_dst  = GFL_HEAP_AllocMemory( heapID, size );
  return pwk;
}

//----------------------------------------
/**
 * @brief パレットフェード構造体開放
 * @param[in]	pwk	_EFFTOOL_PAL_FADE_WORKポインタ
 */
//----------------------------------------
static void	_freePaletteFade( _EFFTOOL_PAL_FADE_WORK* pwk )
{
  if(pwk){
    GFL_HEAP_FreeMemory( pwk->pData_dst );
    GFL_HEAP_FreeMemory( pwk );
  }
}



//-------------------------------------------------
/**
 *	@brief      3Dモデルのパレットフェードアニメ実行処理   BTLVから転用
 *	@param[in]	epfw  パレットフェード管理構造体へのポインタ
 */
//-------------------------------------------------
static void  _EFFTOOL_CalcPaletteFade( _EFFTOOL_PAL_FADE_WORK *epfw )
{

  if(	(epfw == NULL) || (epfw->pal_fade_flag == 0) )
  {
    return;
  }
  if( epfw->pal_fade_wait == 0 )
  {
    NNSG3dResFileHeader*	header;
    NNSG3dResTex*		    	pTex;
    u32                   size;
    const void*           pData_src;
    u32                   from;

    epfw->pal_fade_wait = epfw->pal_fade_wait_tmp;

    {
      //テクスチャリソースポインタの取得
      header = GFL_G3D_GetResourceFileHeader( epfw->g3DRES );
      pTex = NNS_G3dGetTex( header );

      size = ( u32 )pTex->plttInfo.sizePltt << 3;
      pData_src = NNS_G3dGetPlttData( pTex );
      from = NNS_GfdGetTexKeyAddr( pTex->plttInfo.vramKey );

      SoftFade( pData_src, epfw->pData_dst, ( size / 2 ), epfw->pal_fade_start_evy, epfw->pal_fade_rgb );

      NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT, from, epfw->pData_dst, size );
    }
    if( epfw->pal_fade_start_evy == epfw->pal_fade_end_evy )
    {
      epfw->pal_fade_flag = 0;
    }
    else if( epfw->pal_fade_start_evy > epfw->pal_fade_end_evy )
    {
      epfw->pal_fade_start_evy--;
    }
    else
    {
      epfw->pal_fade_start_evy++;
    }
  }
  else
  {
 	  epfw->pal_fade_wait--;
  }
}

//-------------------------------------------------
/**
 *	@brief      MCSS移動命令
 *	@param[in,out]	_POKEMCSS_MOVE_WORK   移動ポインタ
 *	@param[in]	time   移動時間
 *	@param[in]	pPos    最終移動先
 */
//-------------------------------------------------

static void _pokeMoveRenew(_POKEMCSS_MOVE_WORK* pPoke,int time, const VecFx32* pPos)
{
  {
    VecFx32 apos;
    MCSS_GetPosition(pPoke->pMcss, &apos);
    pPoke->time = time;
    GFL_STD_MemCopy(pPos, &pPoke->end, sizeof(VecFx32));
    GFL_STD_MemCopy(&apos, &pPoke->start, sizeof(VecFx32));
  }
}

//-------------------------------------------------
/**
 *	@brief      MCSS移動命令作成
 *	@param[in]	pokeMcss   対象MCSS
 *	@param[in]	time   移動時間
 *	@param[in]	pPos    最終移動先
 *	@param[in]	HEAPID  HEAPID
 */
//-------------------------------------------------

static _POKEMCSS_MOVE_WORK* _pokeMoveCreate(MCSS_WORK* pokeMcss, int time, const VecFx32* pPos, HEAPID heapID)
{
  _POKEMCSS_MOVE_WORK* pPoke = GFL_HEAP_AllocClearMemory(heapID, sizeof(_POKEMCSS_MOVE_WORK));

  pPoke->pMcss = pokeMcss;
  _pokeMoveRenew(pPoke,time,pPos);
  return pPoke;

}

//-------------------------------------------------
/**
 *	@brief      MCSS移動命令実行
 *	@param[in]	pokeMcss   対象MCSS
 *	@param[in]	time   移動時間
 *	@param[in]	xend    Xの移動先
 *	@param[in]	zend    Zの移動先
 *	@param[in]	HEAPID  HEAPID
 */
//-------------------------------------------------

static fx32 _movemath(fx32 st,fx32 en,_POKEMCSS_MOVE_WORK* pMove)
{
  fx32 re;
  re = en - st;
  re = re / pMove->time;
  re = st + re * pMove->nowcount;
  return re;
}


static void _pokeMoveFunc(_POKEMCSS_MOVE_WORK* pMove)
{
  if(!pMove){
    return;
  }
  if(pMove->time < pMove->nowcount){
    return;
  }
  pMove->nowcount++;
  if(pMove->time != pMove->nowcount)
  {
    VecFx32 apos;

    apos.x = _movemath(pMove->start.x, pMove->end.x ,pMove);
    apos.y = _movemath(pMove->start.y, pMove->end.y ,pMove);
    apos.z = _movemath(pMove->start.z, pMove->end.z ,pMove);
    
    MCSS_SetPosition( pMove->pMcss ,&apos );
  }
  else{
    MCSS_SetPosition( pMove->pMcss ,&pMove->end );
  }
}


