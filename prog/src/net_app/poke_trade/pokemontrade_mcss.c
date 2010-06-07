//=============================================================================
/**
 * @file	  pokemontrade_mcss.c
 * @bfief	  ポケモン交換アニメーション  上に行く部分
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/09/27
 */
//=============================================================================

#include <gflib.h>


#include "arc_def.h"
#include "net/network_define.h"
#include "system\wipe.h"

#include "net_app/pokemontrade.h"
#include "system/main.h"

#include "poke_icon.naix"
#include "pokeicon/pokeicon.h"

#include "ircbattle.naix"
#include "trade.naix"



#include "poke_tool/poke_tool_def.h"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"
#include "poke_tool/status_rcv.h"
#include "tradedemo.naix"

#include "pokemontrade_local.h"
//#include "app/mailtool.h"

#include "spahead.h"

#include "sound/pm_voice.h"

#include "pokemontrade_demo.cdat"




typedef struct{
  int se;
  int frame;
} _SE_TBL;


void POKETRADE_SE_gtsSeStrat(int count)
{
  int i;
  _SE_TBL setbl[]={
    //    {    SEQ_SE_DANSA       ,_GTSSTART_SEQ_SE_DANSA		},
    //    {  SEQ_SE_SYS_83      ,_GTSSTART_SEQ_SE_SYS_83		},
    {SEQ_SE_W554_PAKIN,  _GTSSTART_SEQ_SE_W554_PAKIN},
    {SEQ_SE_BOWA2,       _GTSSTART_SEQ_SE_BOWA2		},
    {SEQ_SE_KON,         _GTSSTART_SEQ_SE_KON			},
    {SEQ_SE_KON,         _GTSSTART_SEQ_SE_KON1			},
    {SEQ_SE_KON,         _GTSSTART_SEQ_SE_KON2			},
    {SEQ_SE_KON,         _GTSSTART_SEQ_SE_KON3			},
    {SEQ_SE_TDEMO_001,   _GTSSTART_SEQ_SE_TDEMO_001},
    {SEQ_SE_TDEMO_002,   _GTSSTART_SEQ_SE_TDEMO_002},
    {SEQ_SE_W028_02,     _GTSSTART_SEQ_SE_W028_02	},
    {SEQ_SE_TDEMO_006,   _GTSSTART_SEQ_SE_TDEMO_006},
    {SEQ_SE_TDEMO_007,   _GTSSTART_SEQ_SE_TDEMO_007},
    {SEQ_SE_TDEMO_008,   _GTSSTART_SEQ_SE_TDEMO_008},
    {SEQ_SE_TDEMO_009,   _GTSSTART_SEQ_SE_TDEMO_009},
    {SEQ_SE_TDEMO_010,   _GTSSTART_SEQ_SE_TDEMO_010},
    {SEQ_SE_TDEMO_011,   _GTSSTART_SEQ_SE_TDEMO_011},
    {SEQ_SE_TDEMO_001,   _GTSSTART2_SEQ_SE_TDEMO_001},
    {SEQ_SE_TDEMO_009,   _GTSSTART2_SEQ_SE_TDEMO_009},
    {SEQ_SE_KON,         _GTSSTART2_SEQ_SE_KON			},
    {SEQ_SE_KON,         _GTSSTART2_SEQ_SE_KON1			},
    {SEQ_SE_KON,         _GTSSTART2_SEQ_SE_KON2			},
    {SEQ_SE_KON,         _GTSSTART2_SEQ_SE_KON3			},
    {SEQ_SE_KON,         _GTSSTART2_SEQ_SE_KON4			},
    {SEQ_SE_TDEMO_004,   _GTSSTART_SEQ_SE_TDEMO_004},
    {SEQ_SE_BOWA2,       _GTSSTART2_SEQ_SE_BOWA2		},
    {SEQ_SE_TDEMO_001,   _GTSSTART3_SEQ_SE_TDEMO_001},
  };

  for(i=0;i<elementof(setbl);i++){
    if(setbl[i].frame == count){
      PMSND_PlaySE(setbl[i].se);
    }
  }
}

void POKETRADE_SE_irSeStrat(int count)
{
  int i;
  _SE_TBL setbl[]={
    //   {SEQ_SE_DANSA,  _IRCSTART_SEQ_SE_DANSA},
    //  {SEQ_SE_SYS_83,  _IRCSTART_SEQ_SE_SYS_83},
    {SEQ_SE_W554_PAKIN,  _IRCSTART_SEQ_SE_W554_PAKIN},
    {SEQ_SE_BOWA2,  _IRCSTART_SEQ_SE_BOWA2},
    {SEQ_SE_KON,  _IRCSTART_SEQ_SE_KON},
    {SEQ_SE_KON,  _IRCSTART_SEQ_SE_KON1},
    {SEQ_SE_KON,  _IRCSTART_SEQ_SE_KON2},
    {SEQ_SE_KON,  _IRCSTART_SEQ_SE_KON3},
    {SEQ_SE_TDEMO_001,  _IRCSTART_SEQ_SE_TDEMO_001},
    {SEQ_SE_W028_02,  _IRCSTART_SEQ_SE_W028_02},
    {SEQ_SE_TDEMO_009,  _IRCSTART_SEQ_SE_TDEMO_009},
    {SEQ_SE_TDEMO_002,  _IRCSTART_SEQ_SE_TDEMO_002},
    {SEQ_SE_TDEMO_003,  _IRCSTART_SEQ_SE_TDEMO_003},
    {SEQ_SE_W054_01,  _IRCSTART_SEQ_SE_W054_01},
    {SEQ_SE_W179_02,  _IRCSTART_SEQ_SE_W179_02},
    {SEQ_SE_W307_03,  _IRCSTART_SEQ_SE_W307_03},
    {SEQ_SE_KON,  _IRCSTART2_SEQ_SE_KON},
    {SEQ_SE_KON,  _IRCSTART2_SEQ_SE_KON1},
    {SEQ_SE_KON,  _IRCSTART2_SEQ_SE_KON2},
    {SEQ_SE_KON,  _IRCSTART2_SEQ_SE_KON3},
    {SEQ_SE_KON,  _IRCSTART2_SEQ_SE_KON4},
    {SEQ_SE_TDEMO_004,  _IRCSTART_SEQ_SE_TDEMO_004},
    {SEQ_SE_TDEMO_005,  _IRCSTART_SEQ_SE_TDEMO_005},
    {SEQ_SE_TDEMO_001,  _IRCSTART2_SEQ_SE_TDEMO_001},
  };

  for(i=0;i<elementof(setbl);i++){
    if(setbl[i].frame == count){
      PMSND_PlaySE(setbl[i].se);
    }
  }
}




static void _jumpSeStrat(int count,POKEMON_TRADE_WORK* pWork)
{
  int i;
  _SE_TBL longsetbl[]={
    {SEQ_SE_DANSA,  _LONGSTART_SEQ_SE_DANSA},
    {SEQ_SE_SYS_83,  _LONGSTART_SEQ_SE_SYS_83},
  };

  _SE_TBL shortsetbl[]={
    {SEQ_SE_DANSA,  _SHORTSTART1_SEQ_SE_DANSA},
    {SEQ_SE_SYS_83,  _SHORTSTART1_SEQ_SE_SYS_83},
    {SEQ_SE_DANSA,  _SHORTSTART2_SEQ_SE_DANSA},
    {SEQ_SE_SYS_83,  _SHORTSTART2_SEQ_SE_SYS_83},
    {SEQ_SE_DANSA,  _SHORTSTART3_SEQ_SE_DANSA},
    {SEQ_SE_SYS_83,  _SHORTSTART3_SEQ_SE_SYS_83},
  };


  if(pWork->bByebyeNoJump){  //ジャンプしない
    return;
  }
  else if(!pWork->bByebyeMessageEach){     // 不一致なのでジャンプ+回転
    for(i=0;i<elementof(longsetbl);i++){
      if(longsetbl[i].frame == count){
        PMSND_PlaySE(longsetbl[i].se);
      }
    }
  }
  else{ // 一致しているのでジャンプ

    for(i=0;i<elementof(shortsetbl);i++){
      if(shortsetbl[i].frame == count){
        PMSND_PlaySE(shortsetbl[i].se);
      }
    }

  }

  OS_TPrintf("C %d\n",count);
}





//-------------------------------------------------
/**
 *	@brief      MCSS移動命令
 *	@param[in,out]	_POKEMCSS_MOVE_WORK   移動ポインタ
 *	@param[in]	time   移動時間
 *	@param[in]	pPos    最終移動先
 */
//-------------------------------------------------

void POKEMONTRADE_pokeMoveRenew(_POKEMCSS_MOVE_WORK* pPoke,int time, const VecFx32* pPos)
{
  {
    VecFx32 apos;
    MCSS_GetPosition(pPoke->pMcss, &apos);
    pPoke->time = time;
    pPoke->nowcount=0;
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

_POKEMCSS_MOVE_WORK* POKEMONTRADE_pokeMoveCreate(MCSS_WORK* pokeMcss, int time, const VecFx32* pPos, HEAPID heapID)
{
  _POKEMCSS_MOVE_WORK* pPoke = GFL_HEAP_AllocClearMemory(heapID, sizeof(_POKEMCSS_MOVE_WORK));

  pPoke->pMcss = pokeMcss;
  POKEMONTRADE_pokeMoveRenew(pPoke,time,pPos);
  pPoke->percent=0.0f;
  return pPoke;

}

//-------------------------------------------------
/**
 *	@brief      MCSSテーブル移動命令作成
 *	@param[in]	pokeMcss   対象MCSS
 *	@param[in]	time   移動時間
 *	@param[in]	pPos    最終移動先
 *	@param[in]	HEAPID  HEAPID
 */
//-------------------------------------------------

_POKEMCSS_MOVE_WORK* POKEMONTRADE_pokeTblMoveCreate(MCSS_WORK* pokeMcss, int time, const VecFx32* pPos, VecFx32* pTbl, HEAPID heapID)
{
  _POKEMCSS_MOVE_WORK* pPoke = GFL_HEAP_AllocClearMemory(heapID, sizeof(_POKEMCSS_MOVE_WORK));

  pPoke->pMcss = pokeMcss;
  POKEMONTRADE_pokeMoveRenew(pPoke,time,pPos);
  pPoke->percent=0.0f;
  pPoke->MoveTbl = pTbl;
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

static fx32 _movemath(fx32 st,fx32 en,_POKEMCSS_MOVE_WORK* pMove,BOOL bWave)
{
  fx32 re;
  re = en - st;
  re = re / pMove->time;
  

  re = st + re * pMove->nowcount;
  if(pMove->wave && bWave){
    re = en + FX_SinIdx(pMove->sins) * _WAVE_NUM;
  }
  return re;
}

//-------------------------------------------------
/**
 *	@brief      テーブルによる移動関数
 *	@param[in]	pMove
 */
//-------------------------------------------------

static void _pokeMoveTblFunc(_POKEMCSS_MOVE_WORK* pMove,POKEMON_TRADE_WORK* pWork)
{
  VecFx32 apos,aposold;
  VecFx32* pPMT = &pMove->MoveTbl[pMove->nowcount];
  
  apos.x = pPMT->x + pMove->start.x;
  apos.y = pPMT->y + pMove->start.y;
  apos.z = pPMT->z + pMove->start.z;

  MCSS_SetPosition( pMove->pMcss ,&apos );

  _jumpSeStrat(pMove->nowcount,pWork);

}

//-------------------------------------------------
/**
 *	@brief      MCSS移動関数
 *	@param[in]	pMove   移動ワーク
 */
//-------------------------------------------------

void POKEMONTRADE_pokeMoveFunc(_POKEMCSS_MOVE_WORK* pMove,POKEMON_TRADE_WORK* pWork)
{
  if(!pMove){
    return;
  }
  pMove->nowcount++;
  if(pMove->time < pMove->nowcount){
    return;
  }
  if(pMove->time != pMove->nowcount)
  {
    if(pMove->MoveTbl){
      _pokeMoveTblFunc(pMove,pWork);
      return;
    }
    else{
      VecFx32 apos,aposold;
      MCSS_GetPosition( pMove->pMcss ,&aposold );
      apos.x = _movemath(pMove->start.x, pMove->end.x ,pMove, FALSE);
      if(pMove->wave){
        float an = FX_FX32_TO_F32(aposold.x - apos.x);
        an = an * 374;
        pMove->sins -= (s32)an;
      }
      apos.y = _movemath(pMove->start.y, pMove->end.y ,pMove, TRUE);
      apos.z = _movemath(pMove->start.z, pMove->end.z ,pMove, FALSE);
      MCSS_SetPosition( pMove->pMcss ,&apos );
    }
  }
  else{
    MCSS_SetPosition( pMove->pMcss ,&pMove->end );
  }
}

//MCSS終了の為のコールバック
void POKEMONTRADE_McssAnmStop( u32 data, fx32 currentFrame )
{
  POKEMON_TRADE_WORK* pWork = (POKEMON_TRADE_WORK*)data;

  MCSS_SetAnmStopFlag( pWork->pokeMcss[0]);
  pWork->mcssStop[0] = TRUE;
}


void POKEMONTRADE_McssMoveDel(_POKEMCSS_MOVE_WORK* pPoke)
{
  if(pPoke){
    GFL_HEAP_FreeMemory(pPoke);
  }
}

