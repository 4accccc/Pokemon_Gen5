//=============================================================================
/**
 * @file  medalbox_rank_def.h
 * @brief メダルボックスのランクに関する定義
 * @date  2011.02.08
 */
//=============================================================================
#pragma once
//===========================
/**
 * @brief ボックスランク
 */
//===========================
enum{
  MEDALBOX_RANK_IRON,
  MEDALBOX_RANK_COPPER,
  MEDALBOX_RANK_SILVER,
  MEDALBOX_RANK_GOLD,
  MEDALBOX_RANK_BW,
  MEDALBOX_RANK_MAX,
};

//===========================
/**
 * @brief ボックスランク毎のメダル最大数
 */
//===========================
enum{
  MEDALBOX_RANK_IRON_MAX    = 49,
  MEDALBOX_RANK_COPPER_MAX  = 99,
  MEDALBOX_RANK_SILVER_MAX  = 149,
  MEDALBOX_RANK_GOLD_MAX    = 199,
  MEDALBOX_RANK_BW_MAX      = 200,
};

