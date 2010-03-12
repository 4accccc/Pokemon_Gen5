#include "msg/msg_poke_trade.h"

//------------------------------------------------------------------
/**
 * @brief   登場時の演出の検査
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------

static void _apperFlgCheck(POKEMON_TRADE_WORK* pWork, u8* msg,u8* nojump,MYSTATUS* pMystatus)
{
  POKEMON_PARAM* pp= IRC_POKEMONTRADE_GetRecvPP(pWork, POKEMONTRADEPROC_IsTriSelect(pWork));
  int id = PP_Get(pp,ID_PARA_id_no,NULL);
  int monsno = PP_Get(pp,ID_PARA_monsno_egg,NULL);
  int frm = PP_Get( pp, ID_PARA_form_no, NULL );
  BOOL nojimpflg;

  {
    POKEMON_PERSONAL_DATA* ppd = POKE_PERSONAL_OpenHandle(monsno, frm, pWork->heapID);
    nojimpflg = POKE_PERSONAL_GetParam(ppd, POKEPER_ID_no_jump);
    POKE_PERSONAL_CloseHandle(ppd);
  }
  if( PP_Get(pp,ID_PARA_tamago_flag,NULL) ){
    *nojump = TRUE;
  }
  else if(nojimpflg){
    *nojump = TRUE;
  }
  if(MyStatus_GetID(pMystatus) != id){
    *msg = FALSE;
  }
  else{
    *msg = TRUE;
  }
}

//------------------------------------------------------------------
/**
 * @brief   ポケモン登場CALCTをだす
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------
static void _pokemonCreateCLACTAdd(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->pPokemonTradeDemo->pPokeCreateCLWK==NULL){
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 99;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    pWork->pPokemonTradeDemo->pPokeCreateCLWK = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                  pWork->cellRes[CHAR_POKECREATE],
                                                  pWork->cellRes[PLT_POKECREATE],
                                                  pWork->cellRes[ANM_POKECREATE],
                                                  &cellInitData ,CLSYS_DRAW_MAIN , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->pPokemonTradeDemo->pPokeCreateCLWK , TRUE );
    GFL_CLACT_WK_SetDrawEnable( pWork->pPokemonTradeDemo->pPokeCreateCLWK, TRUE );
  }
  else{
    GFL_CLACT_WK_ResetAnm(pWork->pPokemonTradeDemo->pPokeCreateCLWK);
  }
}

//------------------------------------------------------------------
/**
 * @brief   ポケモン登場CALCT素材INIT
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------
static void _pokemonCreateCLACTInit(POKEMON_TRADE_WORK* pWork)
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
  pWork->pPokemonTradeDemo->pPokeCreateCLWK=NULL;



}

//------------------------------------------------------------------
/**
 * @brief   ポケモン登場CALCT素材Exit
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------

static void _pokemonCreateCLACTExit(POKEMON_TRADE_WORK* pWork)
{
  
  if(pWork->pPokemonTradeDemo->pPokeCreateCLWK){
    GFL_CLACT_WK_Remove( pWork->pPokemonTradeDemo->pPokeCreateCLWK);
  }
  GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_POKECREATE] );
  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_POKECREATE] );
  GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_POKECREATE] );
  pWork->cellRes[CHAR_POKECREATE] = 0;
  pWork->cellRes[PLT_POKECREATE] = 0;
  pWork->cellRes[ANM_POKECREATE] = 0;
  pWork->pPokemonTradeDemo->pPokeCreateCLWK=NULL;
     
}

//速度調整関数
static int ANMCNTC(int no)
{
  return no;
}


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
 * @brief   デモに入る前にBGM変更
 * @param   POKEMON_TRADE_WORK ワーク
 * @param   _TRADE_SCENE_NO_E no シーン管理enum
 * @retval  none
 */
//------------------------------------------------------------------

static void _demoBGMChange(void)
{
  PMSND_PushBGM();
  PMSND_PlayBGM(  SEQ_BGM_KOUKAN );
  PMSND_FadeInBGM( 8 );
}

//------------------------------------------------------------------
/**
 * @brief   ポケモン登場CALCTをだす
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------
static void _changeDemo_ModelT1(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->mcssStop[0]!=TRUE){  //アニメ終了待ち
    return;
  }

  ///フラグ設定
  {
    _apperFlgCheck(pWork, &pWork->bByebyeMessageEach, &pWork->bByebyeNoJump, pWork->pFriend);
    _apperFlgCheck(pWork, &pWork->bEncountMessageEach, &pWork->bEncountNoJump, pWork->pMy);
  }
  
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ANIMEEND,WB_NET_TRADE_SERVICEID);
    _CHANGE_STATE(pWork,_changeDemo_ModelT2);
  }
  else{
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade0);
  }
}

//ネットワーク同期処理
static void _changeDemo_ModelT2(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ANIMEEND,WB_NET_TRADE_SERVICEID)){
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade0);
  }

}

//------------------------------------------------------------------
/**
 * @brief   ポケモン同期待ち終了後 移動開始
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------

static void _changeDemo_ModelTrade0(POKEMON_TRADE_WORK* pWork)
{
  VecFx32 apos;
  int i;

  _setNextAnim(pWork, 0);

  pWork->pD2Fade = GFL_HEAP_AllocClearMemory(pWork->heapID, sizeof(_D2_PAL_FADE_WORK));
  pWork->pD2Fade->pal_fade_time = ANMCNTC(_POKEMON_CENTER_TIME/3);
  pWork->pD2Fade->pal_fade_nowcount = 0;
  pWork->pD2Fade->pal_start = 0;
  pWork->pD2Fade->pal_end = -16;

  if(pWork->modelno!=-1){
//    pWork->pModelFade = _createPaletteFade(GFL_G3D_UTIL_GetResHandle(pWork->g3dUtil,0), pWork->heapID);
//    _FIELD_StartPaletteFade( pWork->pModelFade, 0, 16, ANMCNTC(_POKEMON_CENTER_TIME/3)/16, 0 );
  }

  // ポケモン中央に移動開始
  {
    VecFx32 pos={_POKEMON_PLAYER_CENTER_POSX,_POKEMON_PLAYER_CENTER_POSY, _POKEMON_PLAYER_CENTER_POSZ};
    pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[0], ANMCNTC(_POKEMON_CENTER_TIME), &pos, pWork->heapID);
  }
  {
    VecFx32 pos={_POKEMON_FRIEND_CENTER_POSX,_POKEMON_FRIEND_CENTER_POSY, _POKEMON_FRIEND_CENTER_POSZ};
    pWork->pMoveMcss[1] = _pokeMoveCreate(pWork->pokeMcss[1], ANMCNTC(_POKEMON_CENTER_TIME*2), &pos, pWork->heapID);
  }

  MCSS_SetAnimCtrlCallBack(pWork->pokeMcss[0], (u32)pWork, _McssAnmStop, NNS_G2D_ANMCALLBACKTYPE_LAST_FRM);

  MCSS_SetPaletteFade( pWork->pokeMcss[1], 0, 16, ANMCNTC(_POKEMON_CENTER_TIME/3)/16, 0 );
  POKETRADE_MESSAGE_WindowClose(pWork);

  GFL_BG_SetBackGroundColor(GFL_BG_FRAME1_M ,0);
  GFL_BG_SetBackGroundColor(GFL_BG_FRAME1_S ,0);

  _setNextAnim(pWork, 0);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade1);

}

static void _changeDemo_ModelTrade1(POKEMON_TRADE_WORK* pWork)
{
  int i;

  {  // フェード中
    _setFadeMask(pWork->pD2Fade);
    _pokeMoveFunc(pWork->pMoveMcss[0]);
    _pokeMoveFunc(pWork->pMoveMcss[1]);
  }

  if(pWork->anmCount > ANMCNTC(_POKEMON_CENTER_TIME)){  //フェード完了
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_OBJ );
    GFL_DISP_GXS_SetVisibleControlDirect( 0 );
//    POKMEONTRADE_RemoveCoreResource( pWork);
    
 //   pWork->cellUnit = GFL_CLACT_UNIT_Create( 3 , 0 , pWork->heapID );

    _setNextAnim(pWork, 0);

    {
      POKEMON_PARAM* pp= IRC_POKEMONTRADE_GetRecvPP(pWork, POKEMONTRADEPROC_IsTriSelect(pWork));
      int id = PP_Get(pp,ID_PARA_id_no,NULL);
      if(! PP_Get(pp,ID_PARA_tamago_flag,NULL) ){
        int monsno = PP_Get(pp,ID_PARA_monsno_egg,NULL);
        int formno = PP_Get(pp,ID_PARA_form_no,NULL);
        PMVOICE_Play( monsno, formno, 64, FALSE, 0, 0, FALSE, 0 );
      }
      if(pWork->pMoveMcss[0]){
        GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
        pWork->pMoveMcss[0]=NULL;
      }
      if(MyStatus_GetID(pWork->pFriend) != id){
        pWork->bByebyeMessageEach = FALSE;
      }
      else{
        pWork->bByebyeMessageEach = TRUE;
      }


      if(!pWork->bByebyeMessageEach){     // 不一致なのでジャンプ+回転
        VecFx32 apos;
        POKEMON_PARAM* pp;
        
        MCSS_GetPosition(pWork->pokeMcss[0], &apos);
        if(pWork->bByebyeNoJump){
          pWork->pMoveMcss[0] = _pokeTblMoveCreate(pWork->pokeMcss[0], elementof(_noJumpTbl), &apos, _noJumpTbl,  pWork->heapID);
        }
        else{
          pWork->pMoveMcss[0] = _pokeTblMoveCreate(pWork->pokeMcss[0], elementof(_shortJumpTbl), &apos, _shortJumpTbl,  pWork->heapID);
        }

        pp = IRC_POKEMONTRADE_GetRecvPP(pWork,POKEMONTRADEPROC_IsTriSelect(pWork));
        IRCPOKETRADE_PokeCreateMcss(pWork, 2, 0, pp,TRUE );  //みかた裏
        MCSS_SetAnmStopFlag(pWork->pokeMcss[2]);
        MCSS_SetVanishFlag( pWork->pokeMcss[2] );

      }
      else{// 一致しているのでジャンプ
        VecFx32 apos;
        MCSS_GetPosition(pWork->pokeMcss[0], &apos);
        if(pWork->bByebyeNoJump){
          pWork->pMoveMcss[0] = _pokeTblMoveCreate(pWork->pokeMcss[0], elementof(_noJumpTbl), &apos, _noJumpTbl,  pWork->heapID);
        }
        else{
          pWork->pMoveMcss[0] = _pokeTblMoveCreate(pWork->pokeMcss[0], elementof(_triJumpTbl), &apos, _triJumpTbl,  pWork->heapID);
        }
      }
    }

    _CHANGE_STATE(pWork,_changeDemo_ModelTrade2_jump);
  }
}


static void _byebyeMessage(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->bByebyeMessageEach){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_29, pWork->pMessageStrBufEx );
  }
  else{
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_28, pWork->pMessageStrBufEx );
  }
  {
    POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, POKEMONTRADEPROC_IsTriSelect(pWork));
    WORDSET_RegisterPokeNickName( pWork->pWordSet, 0,  pp );
  }
  WORDSET_RegisterPlayerName( pWork->pWordSet, 1, pWork->pFriend  );
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBuf, pWork->pMessageStrBufEx);
  GFL_BG_FillScreen( GX_PLANEMASK_BG2, 0x0000, 0, 0, 32, 24, GFL_BG_SCRWRT_PALIN );
  POKETRADE_MESSAGE_WindowOpenCustom(pWork,FALSE,FALSE);
  
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG2 );
  G2S_BlendNone();
}

static void _changeDemo_ModelTrade2_jump(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->anmCount == 10){
    _byebyeMessage(pWork);  //別れのメッセージ
  }

  if(!pWork->bByebyeMessageEach && (pWork->anmCount == _JUMP_PEEK)){
    {
      VecFx32 apos;
      MCSS_GetPosition( pWork->pokeMcss[0] ,&apos );
      MCSS_SetPosition( pWork->pokeMcss[2] ,&apos );
    }
    MCSS_ResetVanishFlag( pWork->pokeMcss[2] );
    MCSS_SetVanishFlag( pWork->pokeMcss[0] );
    pWork->pMoveMcss[0]->pMcss = pWork->pokeMcss[2];
  }

  if((pWork->anmCount > _POKEMON_VOICE_TIME) && (POKETRADE_MESSAGE_EndCheck(pWork))){
    POKETRADE_MESSAGE_WindowClose(pWork);
    if(!pWork->bByebyeMessageEach){
      {
        VecFx32 apos;
        MCSS_GetPosition( pWork->pokeMcss[2] ,&apos );
        MCSS_SetPosition( pWork->pokeMcss[0] ,&apos );
      }
      MCSS_ResetVanishFlag( pWork->pokeMcss[0] );
      MCSS_SetVanishFlag( pWork->pokeMcss[2] );
      IRCPOKETRADE_PokeDeleteMcss(pWork,2);
    }
    if(pWork->pMoveMcss[0]){
      GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
      pWork->pMoveMcss[0]=NULL;
    }
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade2);
  }
  _pokeMoveFunc(pWork->pMoveMcss[0]);
}


static void _changeDemo_ModelTrade20(POKEMON_TRADE_WORK* pWork)
{
  _pokemonCreateCLACTExit(pWork);
  POKEMONTRADE_DEMO_PTC_End(pWork->pPokemonTradeDemo, IRPTC_KIND_NUM_MAX);
  POKEMONTRADE_DEMO_IRICA_Delete(pWork->pPokemonTradeDemo);
  GFL_HEAP_FreeMemory(pWork->pPokemonTradeDemo);
  pWork->pPokemonTradeDemo = NULL;
  _CHANGE_STATE(pWork,POKMEONTRADE_SAVE_Init);
}


static void _moveMcssDelete(POKEMON_TRADE_WORK* pWork,int no)
{
  if(pWork->pMoveMcss[0]){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
  }
}


