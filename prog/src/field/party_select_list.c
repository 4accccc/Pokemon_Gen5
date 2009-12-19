//==============================================================================
/**
 * @file    party_select_list.c
 * @brief   手持ち or バトルボックス 選択リスト
 * @author  matsuda
 * @date    2009.12.18(金)
 */
//==============================================================================
#include <gflib.h>


//==============================================================================
//  定数定義
//==============================================================================
///アクター最大数
#define PSL_ACTOR_MAX   (32)
///セルアクターユニットのプライオリティ
#define PSL_ACTOR_UNIT_PRI  (10)

enum{
  POKEICON_POS_START_X = 32,    ///<ポケモンアイコン配置開始位置X
  POKEICON_POS_SPACE_X = 32,    ///<ポケモンアイコン毎の配置間隔X
  POKEICON_POS_Y = 64,          ///<ポケモンアイコンY座標
};

enum{
  ITEMICON_POS_START_X = 32,    ///<アイテムアイコン配置開始位置X
  ITEMICON_POS_SPACE_X = 32,    ///<アイテムアイコン毎の配置間隔X
  ITEMICON_POS_Y = 64,          ///<アイテムアイコンY座標
};


//==============================================================================
//  構造体定義
//==============================================================================
typedef struct _PARTY_SELECT_LIST{
  FIELD_MAIN_WORK *fieldWork;
  const POKEPARTY *temoti_party;  ///<手持ちパーティ
  const POKEPARTY *bbox_party;    ///<バトルボックスパーティ(作成していない場合はNULL)
  HEAPID heap_id;
  
  GFL_CLUNIT *clunit;
  GFL_CLWK *temoti_clwk[TEMOTI_POKEMAX];
  GFL_CLWK *bbox_clwk[TEMOTI_POKEMAX];
}PARTY_SELECT_LIST;


PARTY_SELECT_LIST_PTR PARTY_SELECT_LIST_Setup(FIELD_MAIN_WORK *fieldWork, const POKEPARTY *temoti, const POKEPARTY *bbox, BOOL temoti_reg_fail, BOOL bbox_reg_fail, HEAPID heap_id)
{
  PARTY_SELECT_LIST_PTR psl;
  
  psl = GFL_HEAP_AllocClearMemory(heap_id, sizeof(PARTY_SELECT_LIST));
  psl->fieldWork = fieldWork;
  psl->temoti_party = temoti;
  psl->bbox_party = bbox;
  psl->heap_id = heap_id;

  psl->clunit = GFL_CLACT_UNIT_Create( PSL_ACTOR_MAX, PSL_ACTOR_UNIT_PRI, psl->heap_id );
}

void PARTY_SELECT_LIST_Exit(PARTY_SELECT_LIST_PTR psl)
{
  GFL_CLACT_UNIT_Delete(psl->clunit);
}

//--------------------------------------------------------------
/**
 * ポケモンアイコンリソース全登録
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _PokeIcon_ResourceSet(PARTY_SELECT_LIST_PTR psl)
{
  ARCHANDLE *handle;
  int temoti_max, bbox_max;
  POKEMON_PARAM *pp;
  
  handle = GFL_ARC_OpenDataHandle(ARCID_POKEICON, psl->heap_id);

  psl->pokeicon_pltt_index = GFL_CLGRP_PLTT_RegisterEx( handle, POKEICON_GetPalArcIndex(), 
    CLSYS_DRAW_MAIN, FLDOAM_PALNO_FREE_PSL_POKEICON * 0x20, 0, POKEICON_PAL_MAX, psl->heap_id );
  psl->pokeicon_cell_index = GFL_CLGRP_CELLANIM_Register( handle, POKEICON_GetCellArcIndex(), 
    POKEICON_GetAnmArcIndex(), psl->heap_id );
  
  //手持ちCGX
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    pp = PokeParty_GetMemberPointer(psl->temoti_party, i);
    psl->temoti_cgr_index[i] = GFL_CLGRP_CGR_Register( handle, 
      POKEICON_GetCgxArcIndex( PP_GetPPPPointer(pp) ), FALSE, CLSYS_DRAW_MAIN, psl->heap_id );
  }
  
  //バトルボックスCGX
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      pp = PokeParty_GetMemberPointer(psl->bbox_party, i);
      psl->bbox_cgr_index[i] = GFL_CLGRP_CGR_Register( handle, 
        POKEICON_GetCgxArcIndex( PP_GetPPPPointer(pp) ), FALSE, CLSYS_DRAW_MAIN, psl->heap_id );
    }
  }

  GFL_ARC_CloseDataHandle(handle);
}

//--------------------------------------------------------------
/**
 * ポケモンアイコンリソース全解放
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _PokeIcon_ResourceFree(PARTY_SELECT_LIST_PTR psl)
{
  int i;
  
  //バトルボックスCGX
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      GFL_CLGRP_CGR_Release(psl->bbox_cgr_index[i]);
    }
  }
  
  //手持ちCGX
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    GFL_CLGRP_CGR_Release(psl->temoti_cgr_index[i]);
  }
  
  GFL_CLGRP_CELLANIM_Release(psl->pokeicon_cell_index);
  GFL_CLGRP_PLTT_Release(psl->pokeicon_pltt_index);
}

//--------------------------------------------------------------
/**
 * ポケモンアイコンアクター全登録
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _PokeIcon_ActorAdd(PARTY_SELECT_LIST_PTR psl)
{
  int i, temoti_max, bbox_max;
  GFL_CLWK_DATA head = {
    0, POKEICON_POS_Y, 
    0,
    SOFTPRI_POKEICON,
    BGPRI_POKEICON,
  };
  
  //手持ち
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    head.pos_x = POKEICON_POS_START_X + POKEICON_POS_SPACE_X * i;
    psl->temoti_clwk[i] = GFL_CLACT_WK_Create( psl->clunit, psl->temoti_cgr_index[i], 
      psl->pokeicon_pltt_index, psl->pokeicon_cell_index, 
      &head, CLSYS_DRAW_MAIN, psl->heap_id);
  }
  
  //バトルボックス
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      head.pos_x = POKEICON_POS_START_X + POKEICON_POS_SPACE_X * i;
      psl->bbox_clwk[i] = GFL_CLACT_WK_Create( psl->clunit, psl->bbox_cgr_index[i], 
        psl->pokeicon_pltt_index, psl->pokeicon_cell_index, 
        &head, CLSYS_DRAW_MAIN, psl->heap_id);
    }
  }
}

//--------------------------------------------------------------
/**
 * ポケモンアイコンアクター全削除
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _PokeIcon_ActorDel(PARTY_SELECT_LIST_PTR psl)
{
  int i, temoti_max, bbox_max;

  //手持ち
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    GFL_CLACT_WK_Remove(psl->temoti_clwk[i]);
  }
  
  //バトルボックス
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      GFL_CLACT_WK_Remove(psl->bbox_clwk[i]);
    }
  }
}

//--------------------------------------------------------------
/**
 * 道具アイコンリソース全登録
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Item_ResourceSet(PARTY_SELECT_LIST_PTR psl)
{
  ARCHANDLE *handle;
  
  handle = GFL_ARC_OpenDataHandle(ARCID_APP_MENU_COMMON, psl->heap_id);

  psl->item_pltt_index = GFL_CLGRP_PLTT_RegisterEx( handle, 
    APP_COMMON_GetPokeItemIconPltArcIdx(), 
    CLSYS_DRAW_MAIN, FLDOAM_PALNO_FREE_PSL_ITEM * 0x20, 0, APP_COMMON_ITEMICON_PLT_NUM, 
    psl->heap_id );
  
  psl->item_cell_index = GFL_CLGRP_CELLANIM_Register( handle, 
    APP_COMMON_GetPokeItemIconCellArcIdx(APP_COMMON_MAPPING_64K), 
    APP_COMMON_GetPokeItemIconAnimeArcIdx(APP_COMMON_MAPPING_64K), psl->heap_id );
  
  psl->item_cgr_index = GFL_CLGRP_CGR_Register( handle, 
    APP_COMMON_GetPokeItemIconCharArcIdx(), FALSE, CLSYS_DRAW_MAIN, psl->heap_id );

  GFL_ARC_CloseDataHandle(handle);
}

//--------------------------------------------------------------
/**
 * 道具アイコンリソース全解放
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Item_ResourceFree(PARTY_SELECT_LIST_PTR psl)
{
  GFL_CLGRP_CGR_Release(psl->item_cgr_index);
  GFL_CLGRP_CELLANIM_Release(psl->item_cell_index);
  GFL_CLGRP_PLTT_Release(psl->item_pltt_index);
}

//--------------------------------------------------------------
/**
 * 道具アイコンアクター全登録
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Item_ActorAdd(PARTY_SELECT_LIST_PTR psl)
{
  int i, temoti_max, bbox_max, item;
  BOOL enbale;
  GFL_CLWK_DATA head = {
    0, ITEMICON_POS_Y, 
    APP_COMMON_ITEMICON_ITEM,   //anm_seq
    SOFTPRI_ITEMICON,
    BGPRI_ITEMICON,
  };
  
  //手持ち
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    enable = TRUE;
    if(item == 0){
      enable = FALSE;
    }
    else if(ITEM_CheckMail( item ) == TRUE){
      head.anm_seq = APP_COMMON_ITEMICON_MAIL;
    }
    else{
      head.anm_seq = APP_COMMON_ITEMICON_ITEM;
    }
    head.pos_x = ITEMICON_POS_START_X + ITEMICON_POS_SPACE_X * i;
    psl->item_temoti_clwk[i] = GFL_CLACT_WK_Create( psl->clunit, psl->item_cgr_index, 
      psl->item_pltt_index, psl->item_cell_index, 
      &head, CLSYS_DRAW_MAIN, psl->heap_id);
    GFL_CLACT_WK_SetDrawEnable( psl->item_temoti_clwk[i], enable );
  }
  
  //バトルボックス
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      enable = TRUE;
      if(item == 0){
        enable = FALSE;
      }
      else if(ITEM_CheckMail( item ) == TRUE){
        head.anm_seq = APP_COMMON_ITEMICON_MAIL;
      }
      else{
        head.anm_seq = APP_COMMON_ITEMICON_ITEM;
      }
      head.pos_x = ITEMICON_POS_START_X + ITEMICON_POS_SPACE_X * i;
      psl->item_bbox_clwk[i] = GFL_CLACT_WK_Create( psl->clunit, psl->item_cgr_index, 
        psl->item_pltt_index, psl->item_cell_index, 
        &head, CLSYS_DRAW_MAIN, psl->heap_id);
      GFL_CLACT_WK_SetDrawEnable( psl->item_bbox_clwk[i], enable );
    }
  }
}

//--------------------------------------------------------------
/**
 * 道具アイコンアクター全削除
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Item_ActorDel(PARTY_SELECT_LIST_PTR psl)
{
  int i, temoti_max, bbox_max;

  //手持ち
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    GFL_CLACT_WK_Remove(psl->item_temoti_clwk[i]);
  }
  
  //バトルボックス
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      GFL_CLACT_WK_Remove(psl->item_bbox_clwk[i]);
    }
  }
}

