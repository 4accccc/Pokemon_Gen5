//=============================================================================
/**
 *
 *	@file		ui_easy_clwk.c
 *	@brief  CLWKリソースの、GFL_CLWKの簡易生成＆開放 ユーティリティーです。
 *	@author	hosaka genya
 *	@data		2009.10.08
 *
 *	@note   NCL・NCG・NCE が一対一対応のものにしか使えません。
 *	@note   nsbtx_to_clwk のダミーデータを読み込む際などにご利用ください。
 *
 */
//=============================================================================
#include <gflib.h>

#include "ui/ui_easy_clwk.h"


//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//=============================================================================
/**
 *								static関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJリソース読み込み
 *
 *	@param	UI_EASY_CLWK_RES* res   リソースセット
 *	@param	prm 初期化パラメータ
 *	@param	GFL_CLUNIT* unit ユニット
 *	@param	heapID  ヒープＩＤ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void UI_EASY_CLWK_LoadResource( UI_EASY_CLWK_RES* res, UI_EASY_CLWK_RES_PARAM* prm, GFL_CLUNIT* unit, HEAPID heapID )
{
  ARCHANDLE *handle;

  BOOL comp_ncg = ( prm->comp_flg & UI_EASY_CLWK_RES_COMP_NCGR );
  
  handle	= GFL_ARC_OpenDataHandle( prm->arc_id, heapID );

  if( prm->comp_flg & UI_EASY_CLWK_RES_COMP_NCLR )
  {
    res->res_ncl	= GFL_CLGRP_PLTT_RegisterComp( handle,
        prm->pltt_id, prm->draw_type, prm->pltt_line*0x20, heapID );
  }
  else
  {
    res->res_ncl	= GFL_CLGRP_PLTT_RegisterEx( handle,
        prm->pltt_id, prm->draw_type, prm->pltt_line*0x20, prm->pltt_src_ofs, prm->pltt_src_num, heapID );
  }

  res->res_ncg	= GFL_CLGRP_CGR_Register( handle,
      prm->ncg_id, comp_ncg, prm->draw_type, heapID );

  res->res_nce	= GFL_CLGRP_CELLANIM_Register( handle,
      prm->cell_id,	prm->anm_id, heapID );

  // 描画先を保持
  res->draw_type = prm->draw_type;

  GFL_ARC_CloseDataHandle( handle );	

}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ リソース破棄
 *
 *	@param	UI_EASY_CLWK_RES* res リソースセット
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void UI_EASY_CLWK_UnLoadResource( UI_EASY_CLWK_RES* res )
{
	GFL_CLGRP_PLTT_Release( res->res_ncl );
	GFL_CLGRP_CGR_Release( res->res_ncg );
	GFL_CLGRP_CELLANIM_Release( res->res_nce );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ CLWK生成
 *
 *	@param	UI_EASY_CLWK_RES* res 生成済みのリソースデータ
 *	@param	unit    GFL_CLUNIT*
 *	@param	px      初期X座標
 *	@param	py      初期Y座標
 *	@param	anim    初期アニメシーケンス
 *	@param	heapID  ヒープＩＤ
 *
 *	@retval clwk    アクターポインタ
 */
//-----------------------------------------------------------------------------
GFL_CLWK* UI_EASY_CLWK_CreateCLWK( UI_EASY_CLWK_RES* res, GFL_CLUNIT* unit, u8 px, u8 py, u8 anim, HEAPID heapID )
{
  GFL_CLWK_DATA	cldata = {0};
  GFL_CLWK* clwk;

  GF_ASSERT( res );
  GF_ASSERT( res );

  cldata.pos_x	= px;
  cldata.pos_y	= py;

  clwk = GFL_CLACT_WK_Create( unit,
      res->res_ncg,
      res->res_ncl,
      res->res_nce,
      &cldata,
      res->draw_type,
      heapID );

  GFL_CLACT_WK_SetAnmSeq( clwk, anim );

  return clwk;
}


