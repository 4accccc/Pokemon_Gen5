//======================================================================
/**
 * @file  th_rank_calc.h
 * @brief トライアルハウスランク計算関数
 *
 */
//======================================================================
#pragma once

#include "savedata/th_rank_def.h"

inline u16 TH_CALC_Rank( const u32 inScore )
{
  u16 rank;
  if (inScore >= 6000) rank = TH_RANK_MASTER;
  else if (inScore >= 5000) rank = TH_RANK_ELITE;
  else if (inScore >= 4000) rank = TH_RANK_HYPER;
  else if (inScore >= 3000) rank = TH_RANK_SUPER;
  else if (inScore >= 2000) rank = TH_RANK_NORMAL;
  else if (inScore >= 1000) rank = TH_RANK_NOVICE;
  else rank = TH_RANK_BEGINNER;

  return rank;
}

