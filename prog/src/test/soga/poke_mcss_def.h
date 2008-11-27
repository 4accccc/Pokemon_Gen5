
//============================================================================================
/**
 * @file	poke_mcss_def.h
 * @brief	ポケモンMCSS管理
 * @author	soga
 * @date	2008.11.14
 */
//============================================================================================

#ifndef	__POKE_MCSS_DEF_H_
#define	__POKE_MCSS_DEF_H_

#define	POKE_MCSS_MAX	( 6 )	//POKE_MCSSで管理するMCSSのMAX値

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

#endif	//__POKE_MCSS_DEF_H_
