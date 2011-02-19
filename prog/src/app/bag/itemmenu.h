//======================================================================
/**
 * @file    itemmenu.h
 * @brief   アイテムメニュー
 * @author  k.ohno
 * @date    2009.06.30
 */
//======================================================================

#pragma once

#include <gflib.h>
#include "app/bag.h"


// メニュー項目
enum {
  BAG_MENU_TSUKAU = 0,  ///< つかう
  BAG_MENU_ORIRU,       ///< おりる
  BAG_MENU_MIRU,        ///< みる
  BAG_MENU_SUTERU,      ///< すてる
  BAG_MENU_TOUROKU,     ///< とうろく
  BAG_MENU_KAIZYO,      ///< かいじょ
  BAG_MENU_MOTASERU,    ///< もたせる
  BAG_MENU_KETTEI,      ///< けってい
  BAG_MENU_SHIMAU,      ///< しまう
  BAG_MENU_MODOSU,      ///< もどす
  BAG_MENU_YAMERU,      ///< やめる
  BAG_MENU_URU,         ///< うる

  BAG_MENU_MAX
};

// メニュー項目格納場所
enum{
  BAG_MENU_USE=0,     // つかう・みる・おりる
  BAG_MENU_GIVE,      // もたせる
  BAG_MENU_SUB,       // すてる・とうろく・かいじょ
  BAG_MENU_KEEP,      // しまう・もどす
  BAG_MENU_CANCEL,
  BAG_MENUTBL_MAX,
};

// ソートメニュー項目格納場所
enum{
  BAG_SORT_MENU_TYPE_NUM=0, // しゅるいじゅん
  BAG_SORT_MENU_NAME,       // あいうえおじゅん
  BAG_SORT_MENU_BIG_TOP,    // おおいじゅん
  BAG_SORT_MENU_SMALL_TOP,  // すくないじゅん
  BAG_SORT_MENU_CANCEL,     // やめる
  BAG_SORT_MENUTBL_MAX,
};

// ソートメニュー項目
enum{
  BAG_SORTMENU_STR_TYPE=0,    // しゅるいじゅん
  BAG_SORTMENU_STR_NUM,       // 番号順
  BAG_SORTMENU_STR_NAME,      // あいうえおじゅん
  BAG_SORTMENU_STR_BIG_TOP,   // おおいじゅん
  BAG_SORTMENU_STR_SMALL_TOP, // すくないじゅん
  BAG_SORTMENU_STR_CANCEL,    // やめる
  BAG_SORTMENU_STRTBL_MAX,
};

// ソートモード
enum{
  SORT_MODE_TYPE,
  SORT_MODE_NUM,
  SORT_MODE_NAME,
  SORT_MODE_BIG_TOP,
  SORT_MODE_SMALL_TOP,
  SORT_MODE_MAX,
};

// 保管用ソートメニュー項目格納場所
enum{
  BAG_REPSORT_MENU_FULL=0,  // ぜんぶ
  BAG_REPSORT_MENU_NORMAL,  // どうぐ
  BAG_REPSORT_MENU_DRUG,    // かいふく
  BAG_REPSORT_MENU_WAZA,    // わざましん
  BAG_REPSORT_MENU_NUTS,    // きのみ
  BAG_REPSORT_MENU_EVENT,   // たいせつなもの
  BAG_REPSORT_MENU_CANCEL,  // やめる
  BAG_REPSORT_MENUTBL_MAX,
};

// 保管用ソートメニュー項目(全部とやめる以外はポケット名IDと一致させてください)
enum{
  BAG_REPSORT_MENU_STR_NORMAL,  // どうぐ
  BAG_REPSORT_MENU_STR_DRUG,    // かいふく
  BAG_REPSORT_MENU_STR_WAZA,    // わざましん
  BAG_REPSORT_MENU_STR_NUTS,    // きのみ
  BAG_REPSORT_MENU_STR_EVENT,   // たいせつなもの
  BAG_REPSORT_MENU_STR_FULL,    // ぜんぶ
  BAG_REPSORT_MENU_STR_CANCEL,  // やめる
  BAG_REPSORT_MENU_STRTBL_MAX,
};
