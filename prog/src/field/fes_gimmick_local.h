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
#include "field/fes_gimmick_item.h"

typedef union _FES_GIMMICK_WORK{
  FES_GIMMICK_ITEM_WORK item_wk;  ///<アイテムミッション用ワーク 
}FES_GIMMICK_WORK;

typedef union _FES_GIMMICK_FLAG_WORK{
  FES_GIMMICK_ITEM_FLAG_WORK  item_flag_wk; //アイテムミッション用ワーク
}FES_GIMMICK_FLAG_WORK;

struct _FES_GIMMICK{
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  LINK_FESTIVAL* fest_sys;  ///<最上位ワークへのポインタを保持
  const FEST_MISSION* fmwk; ///<現在のミッション情報ワーク

  FIELDMAP_WORK* fieldWork;
  FLDEFF_CTRL* fectrl;
  const FIELD_RAIL_MAN* railman;

  u16 zone_id;  ///<現在のゾーンID
  FLDEFF_TASK* eff_task;  ///<フィールドエフェクトタスクのポインタを保持

  FES_GIMMICK_WORK  wk_buf; //ミッション別ローカルワーク
};

extern void FES_GIMMICK_SUB_RndInit( const u32 key, const u16 zone_id, GFL_STD_RandContext* rnd_c );
extern u32 FES_GIMMICK_SUB_RndGet( GFL_STD_RandContext* rnd_c, u32 range, u32* p_tbl, int tbl_max );

