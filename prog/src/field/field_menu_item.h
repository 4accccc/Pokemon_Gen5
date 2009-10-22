//======================================================================
/**
 * @file	field_menu_item.h
 * @brief	フィールドメニューのアイテム
 * @author	ariizumi
 * @date	09/05/12
 */
//======================================================================

//field_subscreenとfield_menuでincludeがうまくいかなかったので分離
#pragma once

typedef enum
{
  FMIT_EXIT,
  FMIT_POKEMON,
  FMIT_ZUKAN,
  FMIT_ITEMMENU,
  FMIT_TRAINERCARD,
  FMIT_REPORT,
  FMIT_CONFING,
  FMIT_NONE,
  
  FMIT_MAX,
  
}FIELD_MENU_ITEM_TYPE;
