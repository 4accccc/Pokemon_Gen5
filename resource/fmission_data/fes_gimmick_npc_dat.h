//=========================================================================
/**
 * @file	fes_gimmick_npc_dat.h
 * @brief	フェス NPCミッション用アイテムデータテーブル ヘッダ
 * @autor	miyuki iwasawa
 * @note	このファイルはresource/fmission_data/tool/fmission_item_conv.rbによって自動生成されたものです。
 */
//=========================================================================


#ifndef __H_FES_GIMMICK_NPC_DAT_H__
#define __H_FES_GIMMICK_NPC_DAT_H__

#define FES_NITEM_PRESENT_DATA_MAX    ( 30 )  ///<プレゼントアイテムデータ要素数

#define FES_NITEM_TRADE_DATA_MAX      ( 3 )
#define FES_NITEM_TRADE_CATEGORY_MAX  ( 4 )
#define FES_NITEM_TRADE_RANK_MAX      ( 6 )

#define FES_NITEM_SHOPPING_DATA_MAX   ( 5 )
#define FES_NITEM_SHOPPING_LIST_MAX   ( 16 )

#define FES_NITEM_GET_A_PRESENT_DATA_MAX  ( 2 )
#define FES_NITEM_GET_A_PRESENT_LIST_MAX  ( 6 )

#define FES_NPC_TRAINER_SET_NUM   ( 6 )
#define FES_NPC_TRAINER_DATA_MAX  ( 3 )


#ifndef __ASM_NO_DEF_

typedef struct _NITEM_TRADE_DATA{
  u16 item[FES_NITEM_TRADE_CATEGORY_MAX][FES_NITEM_TRADE_RANK_MAX];
}NITEM_TRADE_DATA;


typedef struct _NITEM_SHOPPING_PARAM{
  u16 item;
  u16 price;
}NITEM_SHOPPING_PARAM;


typedef struct _NITEM_SHOPPING_DATA{
  NITEM_SHOPPING_PARAM  list[FES_NITEM_SHOPPING_LIST_MAX];
}NITEM_SHOPPING_DATA;


typedef struct _NITEM_GET_A_PRESENT_PARAM{
  u16 item;
  u16 prob;      ///<抽選確率
}NITEM_GET_A_PRESENT_PARAM;


typedef struct _NITEM_GET_A_PRESENT_DATA{
  NITEM_GET_A_PRESENT_PARAM  list[FES_NITEM_GET_A_PRESENT_LIST_MAX];
}NITEM_GET_A_PRESENT_DATA;


extern const u16 DATA_NpcItemPresent[];
extern const NITEM_TRADE_DATA DATA_NpcItemTrade[];
extern const NITEM_SHOPPING_DATA DATA_NpcItemShopping[];
extern const NITEM_GET_A_PRESENT_DATA DATA_NpcItemGetAPresent[];
#endif //__ASM_NO_DEF_



#endif //__H_FES_GIMMICK_NPC_DAT_H__

