//===============================================================================
/*
 *  @file   fes_gimmick.c
 *  @brief  フェスミッション フィールドギミック ローカルヘッダ
 *  @author Miyuki Iwasawa
 *  @date   11.08.12
 */
//===============================================================================
#pragma once

#include "field/fes_gimmick.h"
#include "field/fes_gimmick_def.h"

struct _FES_GIMMICK{
  LINK_FESTIVAL* fest_sys;  ///<最上位ワークへのポインタを保持
  const FEST_MISSION* fmwk; ///<現在のミッション情報ワーク

  FIELDMAP_WORK* fieldWork;
  FLDEFF_CTRL* fectrl;

  u16 zone_id;  ///<現在のゾーンID
  FLDEFF_TASK* eff_task;  ///<フィールドエフェクトタスクのポインタを保持
};

extern void FES_GIMMICK_SUB_RndInit( const u32 key, const u16 zone_id, GFL_STD_RandContext* rnd_c );
extern u8 FES_GIMMICK_SUB_RndGet( GFL_STD_RandContext* rnd_c, u32 range, u8* p_tbl, int tbl_max );

