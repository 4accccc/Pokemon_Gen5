//====================================================================
/*
 *  @file pair_sys.h
 *  @brief  連れ歩きシステム関連
 *  @author Miyuki Iwasawa
 *  @date   10.11.16
 */
//====================================================================

#ifndef __H_PAIR_SYS_H__
#define __H_PAIR_SYS_H__

#include "pair_sys_def.h"

typedef struct _PAIR_SYSTEM_WORK{
  HEAPID  heapID;
  u8  pair_id;
  u8  pair_type;
  u16 obj_id;
  u16 obj_code;
  u16 event_id;
  u16 tr_id;
}PAIR_SYSTEM_WORK;

typedef enum{
  PAIR_RECOVER_NORMAL,
  PAIR_RECOVER_FOLLOW,
}PAIR_RECOVER_TYPE;

//---------------------------------------------------
/*
 *  @brief  連れ歩きシステムワーク確保
 */
//---------------------------------------------------
extern PAIR_SYSTEM_WORK* PAIRSYS_AllocWork( HEAPID heapID );

//---------------------------------------------------
/*
 *  @brief  連れ歩きシステムワークフリー
 */
//---------------------------------------------------
extern void PAIRSYS_FreeWork( PAIR_SYSTEM_WORK* wk );

//---------------------------------------------------
/*
 *  @brief  連れ歩きフラグチェック
 */
//---------------------------------------------------
extern BOOL PAIRSYS_CheckPairFlag( GAMEDATA* gdata );

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア トレーナーセット
 *
 *  @param  gdata
 *  @param  pair_id   連れ歩きペア識別ID( PAIR_ID_01～ pair_sys_def.h)
 *  @param  pair_type 連れ歩きペアタイプID( PAIR_TYPE_CLOSE～ pair_sys_def.h)
 *  @param  obj_id    連れ歩きペアのOBJID
 *  @param  obj_code  連れ歩きペアのOBJコード
 *  @param  event_id  連れ歩きペアのイベントID(関連付けないならSCRID_NULL)
 *  @retval tr_id 連れ歩きペアのトレーナーID トレーナーデータ無しならTRID_NULL(定義はtr_tool/trno_def.h)
 */
//---------------------------------------------------
extern BOOL PAIRSYS_PairSet( GAMEDATA* gdata, u8 pair_id, u8 pair_type, u16 obj_id, u16 obj_code, u16 event_id, u16 tr_id );

//---------------------------------------------------
/*
 *  @brief  連れ歩きリセット
 */
//---------------------------------------------------
extern void PAIRSYS_PairFree( GAMEDATA* gdata );

//---------------------------------------------------
/*
 *  @brief  連れ歩きリセット マップチェンジ
 *
 *  @com  連れ歩きがマップチェンジOKタイプでないならフリー
 */
//---------------------------------------------------
extern void PAIRSYS_PairFreeForMapChange( GAMEDATA* gdata );

//---------------------------------------------------
/*
 *  @brief  連れ歩きペアID取得
 *
 *  @retval 0～ 連れ歩きペアID (定義はfield/pair_sys_def.h)
 *  @retval PAID_ID_NONE(0xFFFF)  連れ歩き無し
 */
//---------------------------------------------------
extern u16 PAIRSYS_GetPairID( GAMEDATA* gdata );

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア トレーナーID取得
 *
 *  @retval 1～ 連れ歩きペアのトレーナーID (定義はtr_tool/trno_def.h)
 *  @retval TRID_NULL  連れ歩きトレーナーデータ無し
 */
//---------------------------------------------------
extern u16 PAIRSYS_GetPairTrainerID( GAMEDATA* gdata );

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア オリジナルOBJID取得
 *
 *  @retval 連れ歩きトレーナーの元のOBJID
 */
//---------------------------------------------------
extern u16 PAIRSYS_GetPairTrainerOriginalObjID( GAMEDATA* gdata );

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア タイプ取得
 *
 *  @retval PAIR_TYPE_CLOSE マップチェンジ無し
 *  @retval PAIR_TYPE_OPEN  マップチェンジ有り
 */
//---------------------------------------------------
extern int PAIRSYS_GetPairType( GAMEDATA* gdata );

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア マップチェンジ後復帰
 *  
 *  @com  PAIR_SYS_WORKにセット済みのパラメータを使って初期化する
 *  @param  gdata
 *  @param  PAIR_RECOVER_TYPE PAIR_RECOVER_NORMAL他
 */
//---------------------------------------------------
extern void PAIRSYS_PairMapChangeRecover( GAMEDATA* gdata, PAIR_RECOVER_TYPE type );

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア マップチェンジ後表示復帰
 *  
 *  @param  gdata
 */
//---------------------------------------------------
extern void PAIRSYS_PairViewRecover( GAMEDATA* gdata );

#endif  //__H_PAIR_SYS_H__
