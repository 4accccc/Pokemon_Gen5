//============================================================================================
/**
 * @file	mcss_tool.c
 * @brief	MCSSツール
 * @author	soga
 * @date	2009.02.10
 */
//============================================================================================

#include <gflib.h>

#include "system/mcss_tool.h"

#include "arc_def.h"
#include "pokegra/pokegra_wb.naix"

#include "patch.cdat"

//============================================================================================
/**
 *	定数宣言
 */
//============================================================================================
//トレーナー一体を構成するMCSS用ファイルの構成
enum{
	TRGRA_NCGR,
	TRGRA_NCBR,
	TRGRA_NCER,
	TRGRA_NANR,
	TRGRA_NMCR,
	TRGRA_NMAR,
	TRGRA_NCEC,
	TRGRA_NCLR,

	TRGRA_FILE_MAX,			//トレーナー一体を構成するMCSS用ファイルの総数
};

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================
static  BOOL  MCSS_TOOL_MakeBuchi( const MCSS_ADD_WORK* maw, TCB_LOADRESOURCE_WORK* tlw, u32 work );
static  void  MCSS_TOOL_AnmRestart( u32 param, fx32 currentFrame );

//============================================================================================
/**
 *	POKEMON_PARAMからMCSS_ADD_WORKを生成する
 *
 * @param[in]	pp		POKEMON_PARAM構造体へのポインタ
 * @param[out]	maw		MCSS_ADD_WORKワークへのポインタ
 * @param[in]	dir		ポケモンの向き（MCSS_DIR_FRONT:正面、MCSS_DIR_BACK:背面）
 */
//============================================================================================
void	MCSS_TOOL_MakeMAWPP( const POKEMON_PARAM *pp, MCSS_ADD_WORK *maw, int dir )
{
	MCSS_TOOL_MakeMAWPPP( PP_GetPPPPointerConst( pp ), maw, dir );
}

//============================================================================================
/**
 *	POKEMON_PASO_PARAMからMCSS_ADD_WORKを生成する
 *
 * @param[in]	ppp		POKEMON_PASO_PARAM構造体へのポインタ
 * @param[out]	maw		MCSS_ADD_WORKワークへのポインタ
 * @param[in]	dir		ポケモンの向き（MCSS_DIR_FRONT:正面、MCSS_DIR_BACK:背面）
 */
//============================================================================================
void	MCSS_TOOL_MakeMAWPPP( const POKEMON_PASO_PARAM *ppp, MCSS_ADD_WORK *maw, int dir )
{
	int	mons_no = PPP_Get( ppp, ID_PARA_monsno,	NULL );
	int	form_no = PPP_Get( ppp, ID_PARA_form_no,NULL );
	int	sex		= PPP_Get( ppp, ID_PARA_sex,	NULL );
	int	rare	= PPP_CheckRare( ppp );
	int	egg	  = PPP_Get( ppp, ID_PARA_tamago_flag,	NULL );

	MCSS_TOOL_MakeMAWParam( mons_no, form_no, sex, rare, egg, maw, dir );
}

//============================================================================================
/**
 *	パラメータからMCSS_ADD_WORKを生成する
 *
 * @param[in]	mons_no モンスターナンバー
 * @param[in]	form_no フォルムナンバー
 * @param[in]	sex		性別
 * @param[in]	rare	レアかどうか
 * @param[out]	maw		MCSS_ADD_WORKワークへのポインタ
 * @param[in]	dir		ポケモンの向き（MCSS_DIR_FRONT:正面、MCSS_DIR_BACK:背面）
 */
//============================================================================================
void	MCSS_TOOL_MakeMAWParam( int	mons_no, int form_no, int sex, int rare, BOOL egg, MCSS_ADD_WORK *maw, int dir )
{
	maw->arcID = POKEGRA_GetArcID();
	maw->ncbr = POKEGRA_GetCbrArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->nclr = POKEGRA_GetPalArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->ncer = POKEGRA_GetCelArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->nanr = POKEGRA_GetAnmArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->nmcr = POKEGRA_GetMCelArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->nmar = POKEGRA_GetMAnmArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
	maw->ncec = POKEGRA_GetNcecArcIndex( maw->arcID , mons_no, form_no, sex, rare,  dir, egg );
  maw->heap_low = FALSE;
}

//============================================================================================
/**
 *	パラメータからMCSS_ADD_WORKを生成する
 *
 * @param[in]   tr_type トレーナータイプ
 * @param[out]  maw     MCSS_ADD_WORKワークへのポインタ
 * @param[in]   dir     トレーナーの向き（MCSS_DIR_FRONT:正面、MCSS_DIR_BACK:背面）
 */
//============================================================================================
void	MCSS_TOOL_MakeMAWTrainer( int	tr_type, MCSS_ADD_WORK *maw, int dir )
{
	maw->arcID = TRGRA_GetArcID( dir );
	maw->ncbr = TRGRA_GetCbrArcIndex( tr_type, dir );
	maw->nclr = TRGRA_GetPalArcIndex( tr_type, dir );
	maw->ncer = TRGRA_GetCelArcIndex( tr_type, dir );
	maw->nanr = TRGRA_GetAnmArcIndex( tr_type, dir );
	maw->nmcr = TRGRA_GetMCelArcIndex( tr_type, dir );
	maw->nmar = TRGRA_GetMAnmArcIndex( tr_type, dir );
	maw->ncec = TRGRA_GetNcecArcIndex( tr_type, dir );
  maw->heap_low = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンのMCSS_Addを行う（パッチールのブチもあり）
 *
 * @param[in] mcss_sys  MCSSシステム管理構造体
 * @param[in] pp        AddするPOKEMON_PARAM構造体
 * @param[in] dir       ポケモンの向き
 * @param[in] pos_x     AddするX座標
 * @param[in] pos_y     AddするY座標
 * @param[in] pos_z     AddするZ座標
 */
//-----------------------------------------------------------------------------
MCSS_WORK*  MCSS_TOOL_AddPokeMcss( MCSS_SYS_WORK* mcss_sys, const POKEMON_PARAM* pp, MCSS_DIR dir,
                                   fx32	pos_x, fx32	pos_y, fx32	pos_z )
{ 
  MCSS_WORK* mcss;
  MCSS_ADD_WORK maw;
  u32 personal_rnd = PP_Get( pp, ID_PARA_personal_rnd, NULL );

  MCSS_TOOL_SetMakeBuchiCallback( mcss_sys, personal_rnd );
  MCSS_TOOL_MakeMAWPP( pp, &maw, dir );
  mcss = MCSS_Add( mcss_sys, pos_x, pos_y, pos_z, &maw );

  return mcss;
}

//----------------------------------------------------------------------------
/**
 *	@brief  キャラクタにブチをつけるためのコールバックセット
 *
 * @param[in] mcss_sys      MCSSシステム管理構造体
 * @param[in] personal_rnd  ブチの生成に必要な個性乱数
 */
//-----------------------------------------------------------------------------
void  MCSS_TOOL_SetMakeBuchiCallback( MCSS_SYS_WORK* mcss_sys, u32 personal_rnd )
{ 
  MCSS_SetCallBackFunc( mcss_sys, MCSS_TOOL_MakeBuchi, personal_rnd );
}

//============================================================================================
/**
 * @brief キャラデータにぶちをつける
 *
 * @param[in] maw  MCSS_ADD_WORK構造体へのポインタ
 * @param[in] tlw  リソースデータワーク構造体
 * @param[in] work 個性乱数が入ってくる
 *
 * @retval  TRUE:コールバックを解除する
 */
//============================================================================================
static  BOOL  MCSS_TOOL_MakeBuchi( const MCSS_ADD_WORK* maw, TCB_LOADRESOURCE_WORK* tlw, u32 work )
{ 
  //パッチールにぶちをつける
  if( maw->ncec == NARC_pokegra_wb_pfwb_327_NCEC )
  { 
    MCSS_TOOL_MakeBuchiCore( tlw->pCharData->pRawData , work );
  }

  return TRUE;
}

void MCSS_TOOL_MakeBuchiCore( u8 *buf , u32 rnd )
{
  const	PATTIIRU_BUCHI_DATA	*pbd;
  int i, j;
  u8	setx, sety, cnt;
  int	pos[ 2 ];

  //1枚目
  for( i = 0 ; i < 4 ; i++ )
  {
    pbd = pbd_table[ i ];
    cnt=0;
    while( pbd[ cnt ].posx != 0xff )
    {
  	  setx = pbd[ cnt ].posx +   ( ( rnd & 0x0f ) - 8 );
  	  sety = pbd[ cnt ].posy + ( ( ( rnd & 0xf0 ) >> 4 ) - 8 );
  	  pos[ 0 ] = setx / 2 + sety * 128;
  	  pos[ 1 ] = setx / 2 + ( sety + 40 ) * 128;
      for( j = 0 ; j < 2 ; j++ )
      { 
  	    if( setx & 1)
        {
  		    if( ( ( buf[ pos[ j ] ] & 0xf0 ) >= 0x10 ) && ( ( buf[ pos[ j ] ] & 0xf0 ) <= 0x30) )
          {
  			    buf[ pos[ j ] ] += 0x50;
  		    }
  	    }
  	    else
        {
  		    if( ( ( buf[ pos[ j ] ] & 0x0f ) >= 0x01 ) && ( ( buf[ pos[ j ] ] & 0x0f ) <= 0x03 ) )
          {
  			    buf[ pos[ j ] ] += 0x05;
  		    }
  	    }
      }
  	  cnt++;
    }
    rnd = rnd >> 8;
  }  
}

//----------------------------------------------------------------------------
/**
 *	@brief  1ループしたらアニメーションリセットを呼ぶためのコールバックセット
 *
 * @param[in] mcss  MCSS_WORK構造体
 */
//-----------------------------------------------------------------------------
void  MCSS_TOOL_SetAnmRestartCallback( MCSS_WORK* mcss )
{ 
  MCSS_SetAnimCtrlCallBack( mcss, (u32)mcss, MCSS_TOOL_AnmRestart, 1 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  1ループしたらアニメーションリセットするコールバックセット
 *
 * @param[in] mcss_sys      MCSSシステム管理構造体
 * @param[in] personal_rnd  ブチの生成に必要な個性乱数
 */
//-----------------------------------------------------------------------------
static  void  MCSS_TOOL_AnmRestart( u32 param, fx32 currentFrame )
{ 
  MCSS_WORK*  mcss = ( MCSS_WORK* )param;

  MCSS_ReqRestartAnime( mcss );
}

