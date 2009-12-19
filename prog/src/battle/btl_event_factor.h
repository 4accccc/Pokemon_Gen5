//=============================================================================================
/**
 * @file  btl_event_factor.h
 * @brief ポケモンWB バトルシステム イベント＆ハンドラ処理（FACTOR側にのみ公開）
 * @author  taya
 *
 * @date  2008.10.23  作成
 */
//=============================================================================================
#ifndef __BTL_EVENT_FACTOR_H__
#define __BTL_EVENT_FACTOR_H__

#include "waza_tool\wazadata.h"

#include "btl_event.h"
#include "btl_server_flow.h"
#include "btl_server_local.h"
#include "btl_server.h"
#include "btl_pokeparam.h"


enum {
  EVENT_HANDLER_WORK_ELEMS = 8,       ///< ハンドラごとに割り当てられるローカルワーク要素数(int)
  EVENT_HANDLER_TMP_WORK_SIZE = 128,  ///< 全ハンドラから参照される一時使用ワークサイズ

  EVENT_WAZA_STICK_MAX = 8,   ///< １体につき貼り付くことが出来るワザ数の最大値
};

//--------------------------------------------------------------
/**
* イベント実体型
*/
//--------------------------------------------------------------
typedef struct _BTL_EVENT_FACTOR  BTL_EVENT_FACTOR;

//--------------------------------------------------------------
/**
* イベントハンドラ型
*
* myHandle  自身削除用のハンドラ
* flowWk    サーバフローワーク
* pokeID    主体となるポケモンのID
* work      ワーク用配列（要素数はEVENT_HANDLER_WORK_ELEMS, 初回呼び出し時ゼロクリアが保証される）
*/
//--------------------------------------------------------------
typedef void (*BtlEventHandler)( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );


//--------------------------------------------------------------
/**
* イベントスキップチェックハンドラ型
* 特定のハンドラ呼び出しをスキップするかどうか判定に反応する場合、このハンドラを追加でアタッチする
* （※現状、とくせい「かたやぶり」が特定のとくせいを無効化する場合に使用することを想定）
*
* myHandle      自分自身のハンドラ
* factorType    反応ファクタータイプ（ワザ、とくせい、アイテム等）
* eventType     イベントID
* subID         ファクタータイプごとのサブID（ワザなら「なみのり」，とくせいなら「いかく」などのID)
* pokeID        主体となるポケモンのID
*
* return  スキップする場合TRUE
*/
//--------------------------------------------------------------
typedef BOOL (*BtlEventSkipCheckHandler)( BTL_EVENT_FACTOR* myHandle, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID );

//--------------------------------------------------------------
/**
* イベントハンドラ＆タイプの関連付けテーブル
*/
//--------------------------------------------------------------
typedef struct {
  BtlEventType    eventType;
  BtlEventHandler   handler;
}BtlEventHandlerTable;



extern BTL_EVENT_FACTOR* BTL_EVENT_AddFactor( BtlEventFactorType factorType, u16 subID, u16 priority, u8 pokeID,
  const BtlEventHandlerTable* handlerTable, u16 numHandlers );
extern BTL_EVENT_FACTOR* BTL_EVENT_SeekFactorCore( BtlEventFactorType factorType );
extern BTL_EVENT_FACTOR* BTL_EVENT_SeekFactor( BtlEventFactorType factorType, u8 pokeID );
extern BTL_EVENT_FACTOR* BTL_EVENT_GetNextFactor( BTL_EVENT_FACTOR* factor );
extern void BTL_EVENT_FACTOR_Remove( BTL_EVENT_FACTOR* factor );
extern void BTL_EVENT_FACTOR_ChangePokeParam( BTL_EVENT_FACTOR* factor, u8 pokeID, u16 pri );
extern u16 BTL_EVENT_FACTOR_GetSubID( const BTL_EVENT_FACTOR* factor );
extern int BTL_EVENT_FACTOR_GetWorkValue( const BTL_EVENT_FACTOR* factor, u8 workIdx );
extern void BTL_EVENT_FACTOR_SetWorkValue( BTL_EVENT_FACTOR* factor, u8 workIdx, int value );
extern u8 BTL_EVENT_FACTOR_GetPokeID( const BTL_EVENT_FACTOR* factor );
extern void BTL_EVENT_FACTOR_AttachSkipCheckHandler( BTL_EVENT_FACTOR* factor, BtlEventSkipCheckHandler handler );
extern void BTL_EVENT_FACTOR_DettachSkipCheckHandler( BTL_EVENT_FACTOR* factor );

//=============================================================================================
/**
 * 特定タイプ＆サブIDのファクターを１ターン休止させる
 *
 * @param   type
 * @param   subID
 *
 */
//=============================================================================================
extern void BTL_EVENT_SleepFactor( BtlEventFactorType type, u16 subID );

#endif
