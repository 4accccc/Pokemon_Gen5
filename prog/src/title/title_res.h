//======================================================================
/**
 * @file	title_res.h
 * @brief	タイトルのバージョン違い吸収用
 * @author	ariizumi
 * @data	100515
 *
 * モジュール名：TITLE_RES
 */
//======================================================================
#pragma once

#include "pm_version.h"

#define TITLE_RES_ARCID (ARCID_TITLE)

#if PM_VERSION == VERSION_BLACK

#include "title_black.naix"

#define TITLE_RES_LOGO_NCLR (NARC_title_black_blk_logo_NCLR)
#define TITLE_RES_LOGO_NCGR (NARC_title_black_blk_logo_NCGR)
#define TITLE_RES_LOGO_NSCR (NARC_title_black_blk_logo_NSCR)

#else

#include "title_white.naix"

#define TITLE_RES_LOGO_NCLR (NARC_title_white_wht_logo_NCLR)
#define TITLE_RES_LOGO_NCGR (NARC_title_white_wht_logo_NCGR)
#define TITLE_RES_LOGO_NSCR (NARC_title_white_wht_logo_NSCR)

#endif