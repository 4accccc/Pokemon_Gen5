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
	int	mons_no = PPP_Get( ppp, ID_PARA_monsno_egg,	NULL );
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
	maw->ncbr = POKEGRA_GetCbrArcIndex( mons_no, form_no, sex, rare,  dir, egg );
	maw->nclr = POKEGRA_GetPalArcIndex( mons_no, form_no, sex, rare,  dir, egg );
	maw->ncer = POKEGRA_GetCelArcIndex( mons_no, form_no, sex, rare,  dir, egg );
	maw->nanr = POKEGRA_GetAnmArcIndex( mons_no, form_no, sex, rare,  dir, egg );
	maw->nmcr = POKEGRA_GetMCelArcIndex( mons_no, form_no, sex, rare,  dir, egg );
	maw->nmar = POKEGRA_GetMAnmArcIndex( mons_no, form_no, sex, rare,  dir, egg );
	maw->ncec = POKEGRA_GetNcecArcIndex( mons_no, form_no, sex, rare,  dir, egg );
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
	int	file_start = TRGRA_FILE_MAX * tr_type;	//トレーナータイプからファイルのオフセットを計算
	int	file_offset = 0;	//向きの計算（現状はない）

	maw->arcID = ARCID_TRFGRA;
	maw->ncbr = file_start + file_offset + TRGRA_NCBR;
	maw->nclr = file_start + file_offset + TRGRA_NCLR;
	maw->ncer = file_start + file_offset + TRGRA_NCER;
	maw->nanr = file_start + file_offset + TRGRA_NANR;
	maw->nmcr = file_start + file_offset + TRGRA_NMCR;
	maw->nmar = file_start + file_offset + TRGRA_NMAR;
	maw->ncec = file_start + file_offset + TRGRA_NCEC;
}

