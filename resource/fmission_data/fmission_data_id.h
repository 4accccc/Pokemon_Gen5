//=========================================================================
/**
 * @file	fmission_data_id.h
 * @brief	フェスミッションデータID定義ファイル
 * @autor	miyuki iwasawa
 * @note	このファイルはresource/fmission_data/tool/fmission_data_conv.rbによって自動生成されたものです。
 */
//=========================================================================
#ifndef __H_FMISSION_DATA_ID_H__
#define __H_FMISSION_DATA_ID_H__

#define  FMISSION_ID_POKEBTL_TABUNNE01  ( 0 )  ///<タブンネを探せ１
#define  FMISSION_ID_POKEBTL_EMONGA01  ( 1 )  ///<エモンガを探せ１
#define  FMISSION_ID_ITEM_KAKERA01  ( 2 )  ///<カケラを求めて
#define  FMISSION_ID_EVENT_TOWER_OF_HEAVEN  ( 3 )  ///<鐘を　鳴らして
#define  FMISSION_ID_MAX  ( 4 )
#define  FMISSION_ID_END  ( FMISSION_ID_MAX-1 )
#define  FMISSION_ID_NULL ( 0xFF )

#ifndef __ASM_NO_DEF_  //ここからアセンブラでは無視

typedef u8 FEST_MISSION_ID;

#endif //__ASM_NO_DEF_

#endif //__H_FMISSION_DATA_ID_H__

