//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		poke2dgra.c
 *	@brief	ポケモンOBJ,BGグラフィック
 *	@author	Toru=Nagihashi
 *	@data		2009.10.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//アーカイブ
#include "arc_def.h"
#include "pokegra/pokegra_wb.naix"

//リソース
#include "system/pokegra.h"
#include "app/app_menu_common.h"

//外部参照
#include "system/poke2dgra.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//=============================================================================
/**
 *					プロトタイプ
 */
//=============================================================================
static void* POKE2DGRA_LoadCharacterPPP( NNSG2dCharacterData **ncg_data, const POKEMON_PASO_PARAM* ppp, int dir, HEAPID heapID );
static void* POKE2DGRA_LoadCharacter( NNSG2dCharacterData **ncg_data, int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, HEAPID heapID );
//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	NNSG2dCharacterData **ncg_data
 *	@param	POKEMON_PASO_PARAM* ppp
 *	@param	dir
 *	@param	heapID 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void* POKE2DGRA_LoadCharacterPPP( NNSG2dCharacterData **ncg_data, const POKEMON_PASO_PARAM* ppp, int dir, HEAPID heapID )
{	
	//リソース受け取り
	int	mons_no = PPP_Get( ppp, ID_PARA_monsno,	NULL );
	int	form_no = PPP_Get( ppp, ID_PARA_form_no,NULL );
	int	sex		= PPP_Get( ppp, ID_PARA_sex,	NULL );
	int	rare	= PPP_CheckRare( ppp );
	int	egg		= PPP_Get( ppp, ID_PARA_tamago_flag,	NULL );

	return POKE2DGRA_LoadCharacter( ncg_data, mons_no, form_no, sex, rare, dir, egg, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	NNSG2dCharacterData **ncg_data
 *	@param	POKEMON_PASO_PARAM* ppp
 *	@param	dir
 *	@param	heapID 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void* POKE2DGRA_LoadCharacter( NNSG2dCharacterData **ncg_data, int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, HEAPID heapID )
{	
	u32 cgr;
	void *p_buf;

	//リソース受け取り
	cgr	= POKEGRA_GetCgrArcIndex( mons_no, form_no, sex, rare, dir, egg );
	//リソースはOBJとして作っているので、LoadOBJじゃないと読み込めない
	p_buf = GFL_ARC_UTIL_LoadOBJCharacter( POKEGRA_GetArcID(), cgr, FALSE, ncg_data, heapID );

	return p_buf;
}


//=============================================================================
/**
 *					BG関係
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	BG貼り付け用に、キャラ、ﾊﾟﾚｯﾄ読み込み	PPP版
 *
 *	@param	const POKEMON_PASO_PARAM* ppp	ポケモンパーソナル
 *	@param	dir														絵の方向
 *	@param	frm														読込先フレーム
 *	@param	chr_offs											キャラオフセット
 *	@param	plt_offs											パレットオフセット(0～15)
 *	@param	heapID												ヒープID
 */
//-----------------------------------------------------------------------------
void POKE2DGRA_BG_TransResourcePPP( const POKEMON_PASO_PARAM* ppp, int dir, u32 frm, u32 chr_offs, u32 plt_offs, HEAPID heapID )
{
	int	mons_no = PPP_Get( ppp, ID_PARA_monsno,	NULL );
	int	form_no = PPP_Get( ppp, ID_PARA_form_no,NULL );
	int	sex		= PPP_Get( ppp, ID_PARA_sex,	NULL );
	int	rare	= PPP_CheckRare( ppp );
  int	egg		= PPP_Get( ppp, ID_PARA_tamago_flag,	NULL );

	POKE2DGRA_BG_TransResource( mons_no, form_no, sex, rare, dir, egg, frm, chr_offs, plt_offs, heapID );

}

//----------------------------------------------------------------------------
/**
 *	@brief	BG貼り付け用に、キャラ、ﾊﾟﾚｯﾄ読み込みエリアマネージャ	PPP版
 *
 *	@param	const POKEMON_PASO_PARAM* ppp	ポケモンパーソナル
 *	@param	dir														絵の方向
 *	@param	frm														読込先フレーム
 *	@param	chr_offs											キャラオフセット
 *	@param	plt														パレットオフセット(0～15)
 *	@param	heapID												ヒープID
 *
 *
 */
//-----------------------------------------------------------------------------
GFL_ARCUTIL_TRANSINFO POKE2DGRA_BG_TransResourceByAreaManPPP( const POKEMON_PASO_PARAM* ppp, int dir, u32 frm, u32 plt_offs, HEAPID heapID )
{	

	int	mons_no = PPP_Get( ppp, ID_PARA_monsno,	NULL );
	int	form_no = PPP_Get( ppp, ID_PARA_form_no,NULL );
	int	sex		= PPP_Get( ppp, ID_PARA_sex,	NULL );
	int	rare	= PPP_CheckRare( ppp );
  int	egg		= PPP_Get( ppp, ID_PARA_tamago_flag,	NULL );

	return POKE2DGRA_BG_TransResourceByAreaMan( mons_no, form_no, sex, rare, dir, egg, frm, plt_offs, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	BG貼り付け用に、キャラ、ﾊﾟﾚｯﾄ読み込み
 *
 *	@param	mons_no												モンスター番号
 *	@param	form_no												フォルム番号
 *	@param	sex														性別
 *	@param	rare													レア
 *	@param	dir														絵の方向
 *	@param	frm														読込先フレーム
 *	@param	chr_offs											キャラオフセット
 *	@param	plt_offs											パレットオフセット(0～15)
 *	@param	heapID												ヒープID
 */
//-----------------------------------------------------------------------------
void POKE2DGRA_BG_TransResource( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 frm, u32 chr_offs, u32 plt_offs, HEAPID heapID )
{
	PALTYPE	paltype;
	ARCID		arc;
	ARCDATID	plt;

	//パレット読込先取得
	if( frm < GFL_BG_FRAME0_S )
	{	
		paltype	= PALTYPE_MAIN_BG;
	}
	else
	{	
		paltype	= PALTYPE_SUB_BG;
	}
	
	//リソース受け取り
	{	
		arc	= POKEGRA_GetArcID();
		plt	= POKEGRA_GetPalArcIndex( mons_no, form_no, sex, rare, dir, egg );
	}

	//リソース読み込み
	{	
		ARCHANDLE *p_handle;
		void *p_buff;
		NNSG2dCharacterData *ncg_data;

		p_handle	= GFL_ARC_OpenDataHandle( arc, heapID );

		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, plt,
				paltype, plt_offs*0x20, POKEGRA_POKEMON_PLT_SIZE, heapID );

		p_buff	= POKE2DGRA_LoadCharacter( &ncg_data, mons_no, form_no, sex, rare, dir, egg, heapID );
		GFL_BG_LoadCharacter( frm, ncg_data->pRawData, POKEGRA_POKEMON_CHARA_SIZE, chr_offs );

		GFL_ARC_CloseDataHandle( p_handle );

		GFL_HEAP_FreeMemory( p_buff );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	BG貼り付け用に、キャラ、ﾊﾟﾚｯﾄ読み込みエリアマネージャ版
 *
 *	@param	mons_no												モンスター番号
 *	@param	form_no												フォルム番号
 *	@param	sex														性別
 *	@param	rare													レア
 *	@param	dir														絵の方向
 *	@param	frm														読込先フレーム
 *	@param	chr_offs											キャラオフセット
 *	@param	plt														パレットオフセット(0～15)
 *	@param	heapID												ヒープID
 *
 *
 */
//-----------------------------------------------------------------------------
GFL_ARCUTIL_TRANSINFO POKE2DGRA_BG_TransResourceByAreaMan( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 frm, u32 plt_offs, HEAPID heapID )
{	
	PALTYPE	paltype;
	ARCID		arc;
	ARCDATID	plt;
	GFL_ARCUTIL_TRANSINFO info;

	//パレット読込先取得
	if( frm < GFL_BG_FRAME0_S )
	{	
		paltype	= PALTYPE_MAIN_BG;
	}
	else
	{	
		paltype	= PALTYPE_SUB_BG;
	}
	
	//リソース受け取り
	{	
		arc	= POKEGRA_GetArcID();
		plt	= POKEGRA_GetPalArcIndex( mons_no, form_no, sex, rare, dir, egg );
	}

	//リソース読み込み
	{	
		ARCHANDLE *p_handle;
		u32 pos;
		void *p_buff;
		NNSG2dCharacterData *ncg_data;

		p_handle	= GFL_ARC_OpenDataHandle( arc, heapID );

		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, plt,
				paltype, plt_offs, POKEGRA_POKEMON_PLT_SIZE, heapID );

		p_buff	= POKE2DGRA_LoadCharacter( &ncg_data, mons_no, form_no, sex, rare, dir, egg, heapID );
		pos = GFL_BG_LoadCharacterAreaMan( frm, ncg_data->pRawData, POKEGRA_POKEMON_CHARA_SIZE );

		if(  pos == AREAMAN_POS_NOTFOUND )
		{	
			info	= GFL_ARCUTIL_TRANSINFO_FAIL;
		}
		else
		{	
			info	= GFL_ARCUTIL_TRANSINFO_Make(pos, POKEGRA_POKEMON_CHARA_SIZE);
		}

		GFL_ARC_CloseDataHandle( p_handle );
	}

	return info;
}

//----------------------------------------------------------------------------
/**
 *	@brief	スクリーンに書き込み
 *
 *	@param	u32 frm		フレーム
 *	@param	chr_offs	キャラオフセット
 *	@param	pal_offs	パレットオフセット
 *	@param	x					書き込み開始位置X
 *	@param	y					書き込み開始位置Y
 *
 */
//-----------------------------------------------------------------------------
void POKE2DGRA_BG_WriteScreen( u32 frm, u32 chr_offs, u32 pal_offs, u16 x, u16 y )
{	
	int i, j;
	int cnt;

	cnt	= 0;
	//OAMの8x8,4x8,8x4,4x4の順に並んでいる
	for( i = 0; i < 8; i++ ){
		for( j = 0; j < 8; j++ ){
			GFL_BG_FillScreen( frm, chr_offs + cnt, x + j, y + i, 1, 1, pal_offs );
			cnt++;
		}
	}
	for( i = 0; i < 8; i++ ){
		for( j = 0; j < 4; j++ ){
			GFL_BG_FillScreen( frm, chr_offs + cnt, x + j + 8, y + i, 1, 1, pal_offs );
			cnt++;
		}
	}
	for( i = 0; i < 4; i++ ){
		for( j = 0; j < 8; j++ ){
			GFL_BG_FillScreen( frm, chr_offs + cnt, x + j, y + i + 8, 1, 1, pal_offs );
			cnt++;
		}
	}
	for( i = 0; i < 4; i++ ){
		for( j = 0; j < 4; j++ ){
			GFL_BG_FillScreen( frm, chr_offs + cnt, x + j + 8, y + i + 8, 1, 1, pal_offs );
			cnt++;
		}
	}
}

//=============================================================================
/**
 *					OBJ関係
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ハンドルオープン
 *	@param	HEAPID	ヒープID
 *	@return	ハンドル
 */
//-----------------------------------------------------------------------------
ARCHANDLE *POKE2DGRA_OpenHandle( HEAPID heapID )
{	
	return GFL_ARC_OpenDataHandle( POKEGRA_GetArcID(), heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンのパレット読み込み
 *
 *	@param	ARCHANDLE *p_handle			ハンドル
 *	@param	mons_no									モンスター番号
 *	@param	form_no									フォルム番号
 *	@param	sex											性別
 *	@param	rare										レア
 *	@param	dir				ポケモンの方向
 *	@param	vramType	読み込みタイプ
 *	@param	byteOffs	パレット読み込みオフセット
 *	@param	heapID		ヒープID 
 *
 *	@return	登録ID
 */
//-----------------------------------------------------------------------------
u32 POKE2DGRA_OBJ_PLTT_Register( ARCHANDLE *p_handle, int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, CLSYS_DRAW_TYPE vramType, u16 byteOffs, HEAPID heapID )
{	
	u32 plt;
	plt	= POKEGRA_GetPalArcIndex( mons_no, form_no, sex, rare, dir, egg );

	//読み込み
	return GFL_CLGRP_PLTT_RegisterEx( p_handle, plt, vramType, byteOffs, 0, POKEGRA_POKEMON_PLT_NUM, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンキャラクター読み込み
 *
 *	@param	p_handle								アークハンドル
 *	@param	mons_no									モンスター番号
 *	@param	form_no									フォルム番号
 *	@param	sex											性別
 *	@param	rare										レア
 *	@param	dir											ポケモンの方向
 *	@param	vramType								読み込みタイプ
 *	@param	heapID									ヒープID
 *
 *	@return	登録番号
 */
//-----------------------------------------------------------------------------
u32 POKE2DGRA_OBJ_CGR_Register( ARCHANDLE *p_handle, int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, CLSYS_DRAW_TYPE vramType, HEAPID heapID )
{	
	u32 cgr;
	cgr	= POKEGRA_GetCgrArcIndex( mons_no, form_no, sex, rare, dir, egg );
	//読み込み
	return GFL_CLGRP_CGR_Register( p_handle, cgr, FALSE, vramType, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンセル読み込み
 *
 *	@param	mons_no												モンスター番号
 *	@param	form_no												フォルム番号
 *	@param	sex														性別
 *	@param	rare													レア
 *	@param	dir														ポケモンの方向
 *	@param	mapping												マッピングモード
 *	@param	vramType											読み込みタイプ
 *	@param	heapID												ヒープID
 *
 *	@return
 */
//-----------------------------------------------------------------------------
u32 POKE2DGRA_OBJ_CELLANM_Register( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, APP_COMMON_MAPPING mapping, CLSYS_DRAW_TYPE vramType, HEAPID heapID )
{	
	ARCHANDLE *p_handle;
	u32 cel, anm;
	u32 ret;

  if( mons_no == MONSNO_TAMAGO || egg == TRUE )
  {
    //卵の場合はファイル指定
    p_handle  = POKE2DGRA_OpenHandle( heapID );
    ret	= GFL_CLGRP_CELLANIM_Register( p_handle,
        NARC_pokegra_wb_pfwb_egg_normal_NCER,
        NARC_pokegra_wb_pfwb_egg_normal_NANR, heapID );
    GFL_ARC_CloseDataHandle( p_handle );
  }
  else
  { 
    //ポケモン場合は、ダミーセル
    {	
      cel	= APP_COMMON_GetDummyCellArcIdx( mapping );
      anm	= APP_COMMON_GetDummyAnimeArcIdx( mapping );
    }

    p_handle	= GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), heapID );
    ret	= GFL_CLGRP_CELLANIM_Register( p_handle, cel, anm, heapID );
    GFL_ARC_CloseDataHandle( p_handle );
  }

	return ret;
}//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンのパレット読み込み
 *
 *	@param	ARCHANDLE *p_handle			ハンドル
 *	@param	POKEMON_PASO_PARAM* ppp	ポケモンパーソナルパラメータ
 *	@param	dir				ポケモンの方向
 *	@param	vramType	読み込みタイプ
 *	@param	byteOffs	パレット読み込みオフセット
 *	@param	heapID		ヒープID 
 *
 *	@return	登録ID
 */
//-----------------------------------------------------------------------------
u32 POKE2DGRA_OBJ_PLTT_RegisterPPP( ARCHANDLE *p_handle, const POKEMON_PASO_PARAM* ppp, int dir, CLSYS_DRAW_TYPE vramType, u16 byteOffs, HEAPID heapID )
{	
	int	mons_no = PPP_Get( ppp, ID_PARA_monsno,	NULL );
	int	form_no = PPP_Get( ppp, ID_PARA_form_no,NULL );
	int	sex		= PPP_Get( ppp, ID_PARA_sex,	NULL );
	int	rare	= PPP_CheckRare( ppp );
  int egg   = PPP_Get( ppp, ID_PARA_tamago_flag, NULL );

	//読み込み
	return POKE2DGRA_OBJ_PLTT_Register( p_handle, mons_no, form_no, sex, rare, dir, egg, vramType, byteOffs, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンキャラクター読み込み
 *
 *	@param	ARCHANDLE *p_handle			ハンドル
 *	@param	POKEMON_PASO_PARAM* ppp	ポケモンパーソナルパラメータ
 *	@param	dir											ポケモンの方向
 *	@param	vramType								読み込みタイプ
 *	@param	heapID									ヒープID
 *
 *	@return	登録番号
 */
//-----------------------------------------------------------------------------
u32 POKE2DGRA_OBJ_CGR_RegisterPPP( ARCHANDLE *p_handle, const POKEMON_PASO_PARAM* ppp, int dir, CLSYS_DRAW_TYPE vramType, HEAPID heapID )
{	
	int	mons_no = PPP_Get( ppp, ID_PARA_monsno,	NULL );
	int	form_no = PPP_Get( ppp, ID_PARA_form_no,NULL );
	int	sex		= PPP_Get( ppp, ID_PARA_sex,	NULL );
	int	rare	= PPP_CheckRare( ppp );
  int egg   = PPP_Get( ppp, ID_PARA_tamago_flag, NULL );

	//読み込み
	return POKE2DGRA_OBJ_CGR_Register( p_handle, mons_no, form_no, sex, rare, dir, egg, vramType, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンセル読み込み
 *
 *	@param	const POKEMON_PASO_PARAM* ppp	ポケモンパーソナルパラメータ
 *	@param	dir														ポケモンの方向
 *	@param	mapping												マッピングモード
 *	@param	vramType											読み込みタイプ
 *	@param	heapID												ヒープID
 *
 *	@return
 */
//-----------------------------------------------------------------------------
u32 POKE2DGRA_OBJ_CELLANM_RegisterPPP( const POKEMON_PASO_PARAM* ppp, int dir, APP_COMMON_MAPPING mapping, CLSYS_DRAW_TYPE vramType, HEAPID heapID )
{	
	int	mons_no = PPP_Get( ppp, ID_PARA_monsno,	NULL );
	int	form_no = PPP_Get( ppp, ID_PARA_form_no,NULL );
	int	sex		= PPP_Get( ppp, ID_PARA_sex,	NULL );
	int	rare	= PPP_CheckRare( ppp );
  int egg   = PPP_Get( ppp, ID_PARA_tamago_flag, NULL );

	return POKE2DGRA_OBJ_CELLANM_Register( mons_no, form_no, sex, rare, dir, egg, mapping, vramType, heapID );
}
