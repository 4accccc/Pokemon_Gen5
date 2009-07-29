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

//============================================================================================
/**
 *	定数宣言
 */
//============================================================================================
//ポケモン一体を構成するMCSS用ファイルの構成
enum{
	POKEGRA_FRONT_M_NCGR = 0,
	POKEGRA_FRONT_F_NCGR,
	POKEGRA_FRONT_M_NCBR,
	POKEGRA_FRONT_F_NCBR,
	POKEGRA_FRONT_NCER,
	POKEGRA_FRONT_NANR,
	POKEGRA_FRONT_NMCR,
	POKEGRA_FRONT_NMAR,
	POKEGRA_FRONT_NCEC,
	POKEGRA_BACK_M_NCGR,
	POKEGRA_BACK_F_NCGR,
	POKEGRA_BACK_M_NCBR,
	POKEGRA_BACK_F_NCBR,
	POKEGRA_BACK_NCER,
	POKEGRA_BACK_NANR,
	POKEGRA_BACK_NMCR,
	POKEGRA_BACK_NMAR,
	POKEGRA_BACK_NCEC,
	POKEGRA_NORMAL_NCLR,
	POKEGRA_RARE_NCLR,

	POKEGRA_FILE_MAX,			//ポケモン一体を構成するMCSS用ファイルの総数

	POKEGRA_M_NCGR = 0,
	POKEGRA_F_NCGR,
	POKEGRA_M_NCBR,
	POKEGRA_F_NCBR,
	POKEGRA_NCER,
	POKEGRA_NANR,
	POKEGRA_NMCR,
	POKEGRA_NMAR,
	POKEGRA_NCEC
};

//トレーナー一体を構成するMCSS用ファイルの構成
enum{
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
	int	mons_no = PPP_Get( ppp, ID_PARA_monsno,	NULL ) - 1;
	int	form_no = PPP_Get( ppp, ID_PARA_form_no,NULL );
	int	sex		= PPP_Get( ppp, ID_PARA_sex,	NULL );
	int	rare	= PPP_CheckRare( ppp );

	MCSS_TOOL_MakeMAWParam( mons_no, form_no, sex, rare, maw, dir );
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
void	MCSS_TOOL_MakeMAWParam( int	mons_no, int form_no, int sex, int rare, MCSS_ADD_WORK *maw, int dir )
{
	int	file_start = POKEGRA_FILE_MAX * mons_no;							//ポケモンナンバーからファイルのオフセットを計算
	int	file_offset = ( dir == MCSS_DIR_FRONT ) ? 0 : POKEGRA_BACK_M_NCGR;	//向きの計算

	//本来は別フォルム処理を入れる
#ifdef DEBUG_ONLY_FOR_sogabe
#warning Another Form Nothing
#endif

	//性別のチェック
	switch( sex ){
	case PTL_SEX_MALE:
		break;
	case PTL_SEX_FEMALE:
		//オスメス書き分けしているかチェックする（サイズが０なら書き分けなし）
		sex = ( GFL_ARC_GetDataSize( ARCID_POKEGRA, file_start + file_offset + 1 ) == 0 ) ? PTL_SEX_MALE : PTL_SEX_FEMALE;
		break;
	case PTL_SEX_UNKNOWN:
		//性別なしは、オス扱いにする
		sex = PTL_SEX_MALE;
		break;
	default:
		//ありえない性別
		GF_ASSERT(0);
		break;
	}

	maw->arcID = ARCID_POKEGRA;
	maw->ncbr = file_start + file_offset + POKEGRA_M_NCBR + sex;
	maw->nclr = file_start + POKEGRA_NORMAL_NCLR + rare;
	maw->ncer = file_start + file_offset + POKEGRA_NCER;
	maw->nanr = file_start + file_offset + POKEGRA_NANR;
	maw->nmcr = file_start + file_offset + POKEGRA_NMCR;
	maw->nmar = file_start + file_offset + POKEGRA_NMAR;
	maw->ncec = file_start + file_offset + POKEGRA_NCEC;
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

