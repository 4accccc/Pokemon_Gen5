//==============================================================================
/**
 * @file	codein.c
 * @brief	文字入力インターフェース
 * @author	goto
 * @date	2007.07.11(水)
 *
 *	GSより移植 Ari090324
 *	バトルレコーダー用に変更  nagihashi 09.11.23
 *
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
//#include "poke_tool/pokeicon.h"
//#include "poke_tool/boxdata.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"

#include "br_codein_pv.h"




// ----------------------------------------
//
//	プロトタイプ
//
// ----------------------------------------
static void CI_VBlank( GFL_TCB *tcb, void *work );


//----------------------------------------------------------------------------
/**
 *	@brief  コードイン初期化
 *
 *	@param	const BR_CODEIN_PARAM *cp_param  パラメータ
 *	@param	heapID                        ヒープID 
 *
 *	@return ワーク
 *
 * PROCで出来ていたCODEINをシステムに置き換えた
 */
//-----------------------------------------------------------------------------
BR_CODEIN_WORK * BR_CODEIN_Init( const BR_CODEIN_PARAM *cp_param, HEAPID heapID )
{
	BR_CODEIN_WORK* wk;
	
  wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BR_CODEIN_WORK) );
	GFL_STD_MemFill( wk, 0, sizeof( BR_CODEIN_WORK ) );
  wk->param = *cp_param;
  wk->heapID  = heapID;
  wk->sys.cellUnit  = cp_param->p_unit;
  wk->sys.fontHandle  = BR_RES_GetFont( cp_param->p_res );

	
	///< システムワークの作成	
	wk->sys.p_handle = GFL_ARC_OpenDataHandle( ARCID_CODE_INPUT, wk->heapID );	
	
	///< 設定初期化
	CI_pv_ParamInit( wk );
	
	///< Touch Panel設定
	{
		CI_pv_ButtonManagerInit( wk );
	}
	
					   
	wk->sys.vBlankTcb = GFUser_VIntr_CreateTCB( CI_VBlank , wk , 16 );

  return wk;
}


//----------------------------------------------------------------------------
/**
 *	@brief  コードインメイン処理
 *
 *	@param	BR_CODEIN_WORK *p_wk わーく
 *	@param  TRUEならば終了  FALSEならばまだ
 *
 * PROCで出来ていたCODEINをシステムに置き換えた
 */
//-----------------------------------------------------------------------------
void BR_CODEIN_Main( BR_CODEIN_WORK *wk )
{
	BOOL bUpdate;
	
	bUpdate = CI_pv_MainUpdate( wk );
	
	//GFL_CLACT_SYS_Main();
}

//----------------------------------------------------------------------------
/**
 *	@brief  コードイン入力検知
 *
 *	@param	const BR_CODEIN_WORK *wk   ワーク
 *
 *	@return 選択したもの
 */
//-----------------------------------------------------------------------------
BR_CODEIN_SELECT BR_CODEIN_GetSelect( const BR_CODEIN_WORK *wk )
{ 
  return  wk->select;
}


//----------------------------------------------------------------------------
/**
 *	@brief  入力した数値を一括で取得
 *
 *	@param	const BR_CODEIN_WORK *wk ワーク
 *
 *	@return 入力した数値を一括で取得
 */
//-----------------------------------------------------------------------------
u64 BR_CODEIN_GetNumber( const BR_CODEIN_WORK *wk )
{ 
  return wk->code_number;
}


//----------------------------------------------------------------------------
/**
 *	@brief  入力した数値をブロック分割して取得
 *
 *	@param	const BR_CODEIN_WORK *wk  ワーク
 *	@param	*p_tbl                    配列（基本はBR_CODE_BLOCK_MAX分の配列）
 *	@param	tbl_max                   配列の要素数（基本はBR_CODE_BLOCK_MAX）
 */
//-----------------------------------------------------------------------------
void BR_CODEINT_GetNumberBlock( const BR_CODEIN_WORK *wk, int *p_tbl, int tbl_max )
{ 
  int i;
  for( i = 0; i < tbl_max && i < BR_CODE_BLOCK_MAX; i++ )
  { 
    p_tbl[i]  = wk->code_block[i];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  コードイン  終了
 *
 *	@param	BR_CODEIN_WORK *p_wk わーく
 *
 * PROCで出来ていたCODEINをシステムに置き換えた
 */
//-----------------------------------------------------------------------------
void BR_CODEIN_Exit( BR_CODEIN_WORK *wk )
{
	GFL_TCB_DeleteTask( wk->sys.vBlankTcb );
	
	CI_pv_disp_CodeRes_Delete( wk );
	
	GFL_ARC_CloseDataHandle( wk->sys.p_handle );
	
	GFL_BMN_Delete( wk->sys.btn );
	
  GFL_HEAP_FreeMemory( wk );
}





//--------------------------------------------------------------
/**
 * @brief	BR_CODEIN_PARAM のワークを作成する
 *
 * @param	heap_id	
 * @param	word_len	
 * @param	block	
 *
 * @retval	BR_CODEIN_PARAM*	
 *
 */
//--------------------------------------------------------------
BR_CODEIN_PARAM*	BR_CODEIN_ParamCreate( int heap_id, int word_len, GFL_CLUNIT *p_unit, BR_RES_WORK *p_res, BR_BALLEFF_WORK *p_balleff, int block[] )
{
	int i;
	BR_CODEIN_PARAM* wk = NULL;
	
	wk = GFL_HEAP_AllocMemory( heap_id, sizeof( BR_CODEIN_PARAM ) );
  wk->p_unit    = p_unit;
  wk->p_res     = p_res;
	wk->word_len 	= word_len;	
	wk->strbuf		= GFL_STR_CreateBuffer( word_len + 1, heap_id );
  wk->p_balleff = p_balleff;
	
	for ( i = 0; i < BR_CODE_BLOCK_MAX; i++ ){
		wk->block[ i ] = block[ i ];
		OS_Printf( "block %d = %d\n", i, wk->block[ i ] );
	}
	wk->block[ i ] = block[ i - 1 ];

	return wk;	
}

//--------------------------------------------------------------
/**
 * @brief	BR_CODEIN_PARAM のワークを解放
 *
 * @param	codein_param	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void BR_CODEIN_ParamDelete( BR_CODEIN_PARAM* codein_param )
{
	GF_ASSERT_HEAVY( codein_param->strbuf != NULL );
	GF_ASSERT_HEAVY( codein_param != NULL );
	
	GFL_STR_DeleteBuffer( codein_param->strbuf );
	GFL_HEAP_FreeMemory( codein_param );	
}



//--------------------------------------------------------------
/**
 * @brief	VBlank
 *
 * @param	work	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void CI_VBlank( GFL_TCB *tcb, void *work )
{
	BR_CODEIN_WORK* wk = work;
}
