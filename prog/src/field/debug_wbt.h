//============================================================================================
/**
 * @file  debug_wbt.h
 * @brief   WBT用デバッグワーク領域
 * @author  tamada GAMEFREAK inc.
 * @date  2011.12.16
 */
//============================================================================================


#ifdef  PM_DEBUG

#include "pm_define.h"  //TEMOTI_POKEMAX
#include "field/wbt.h"
#include "field/wbt_system.h"

extern BOOL DEBUG_WBT_ForceTrMode;
extern BOOL DEBUG_WBT_ReverseJudgeMode;
extern u16 DEBUG_WBT_ForceTrList[ WBT_TRAINER_NUM ];
extern BOOL  DEBUG_WBT_ForceRentalMode;
extern u16 DEBUG_WBT_ForceRental[ TEMOTI_POKEMAX ];

//------------------------------------------------------------------
/// WBTデバッグ：強制トレーナーリストの設定
//------------------------------------------------------------------
static inline void DEBUG_WBT_SetTrainerList( const u16 * array )
{
  GFL_STD_MemCopy16( array, DEBUG_WBT_ForceTrList, sizeof(u16) * WBT_TRAINER_NUM );
}

//------------------------------------------------------------------
/// WBTデバッグ：強制トレーナーリストの取得
//------------------------------------------------------------------
static inline void DEBUG_WBT_GetTrainerList( u16 * array )
{
  GFL_STD_MemCopy16( DEBUG_WBT_ForceTrList, array, sizeof(u16) * WBT_TRAINER_NUM );
}

//------------------------------------------------------------------
//  WBTデバッグ：強制トレーナーリストが空かどうか？のチェック
//------------------------------------------------------------------
static inline BOOL DEBUG_WBT_IsEmptyTrainerList( void )
{
  int i;
  for ( i = 1; i < WBT_TRAINER_NUM; i ++ )
  {
    //一つでも違う要素があればデータがあるとみなす
    if ( DEBUG_WBT_ForceTrList[0] != DEBUG_WBT_ForceTrList[i]) return FALSE;
  }
  return TRUE;
}

#endif
