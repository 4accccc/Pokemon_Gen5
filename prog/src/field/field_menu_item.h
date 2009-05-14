//======================================================================
/**
 * @file	field_menu_item.h
 * @brief	フィールドメニューのアイテム
 * @author	ariizumi
 * @data	09/05/12
 */
//======================================================================

//field_subscreenとfield_menuでincludeがうまくいかなかったので分離

#ifndef FIELD_MENU_ITEM_H__
#define FIELD_MENU_ITEM_H__

typedef enum
{
  FMIT_EXIT,
  FMIT_POKEMON,
  FMIT_ZUKAN,
  FMIT_ITEMMENU,
  FMIT_TRAINERCARD,
  FMIT_REPORT,
  FMIT_CONFING,
  
  FMIT_MAX,
  
}FIELD_MENU_ITEM_TYPE;

#endif //FIELD_MENU_ITEM_H__