//==============================================================================
/**
 * @file    union_chara.h
 * @brief   ユニオンルームでの人物関連のヘッダ
 * @author  matsuda
 * @date    2009.07.06(月)
 */
//==============================================================================
#pragma once

//==============================================================================
//  定数定義
//==============================================================================
///ビーコンPC：イベントステータス
enum{
  BPC_EVENT_STATUS_NORMAL,    ///<何もしていない
  BPC_EVENT_STATUS_ENTER,     ///<ユニオンルームへ進入
  BPC_EVENT_STATUS_LEAVE,     ///<ユニオンルームから退出
  
  BPC_EVENT_STATUS_MAX,
};

///キャラクタインデックス種類
typedef enum{
  UNION_CHARA_INDEX_PARENT,
  UNION_CHARA_INDEX_CHILD,
}UNION_CHARA_INDEX;

//==============================================================================
//  外部関数宣言
//==============================================================================
extern void UNION_CHAR_Update(UNION_SYSTEM_PTR unisys, GAMEDATA *gdata);
extern UNION_CHARA_INDEX UNION_CHARA_CheckCharaIndex(u16 chara_index);
extern u16 UNION_CHARA_GetCharaIndex_to_ParentNo(u16 chara_index);
extern BOOL UNION_CHARA_CheckCommPlayer(u16 chara_index);

