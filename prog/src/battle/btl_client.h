//=============================================================================================
/**
 * @file  btl_client.h
 * @brief ポケモンWB バトルシステム クライアントモジュール
 * @author  taya
 *
 * @date  2008.09.06  作成
 */
//=============================================================================================
#ifndef __BTL_CLIENT_H__
#define __BTL_CLIENT_H__

//--------------------------------------------------------------
/**
 *  クライアントモジュールハンドラ型定義
 */
//--------------------------------------------------------------
typedef struct _BTL_CLIENT    BTL_CLIENT;


//--------------------------------------------------------------
/**
 *  逃げ・交換禁止コード
 */
//--------------------------------------------------------------
typedef enum {

  BTL_CANTESC_START = 0,

  BTL_CANTESC_KAGEFUMI = BTL_CANTESC_START,
  BTL_CANTESC_ARIJIGOKU,
  BTL_CANTESC_JIRYOKU,
  BTL_CANTESC_TOOSENBOU,

  BTL_CANTESC_MAX,
  BTL_CANTESC_NULL = BTL_CANTESC_MAX,

}BtlCantEscapeCode;

//--------------------------------------------------------------
/**
 *  逃げ・交換選択コード
 */
//--------------------------------------------------------------
typedef enum {
  BTL_CLIENTASK_CHANGE = 0,
  BTL_CLIENTASK_ESCAPE,
}BtlClientChangeEscSelect;


//----------------------------------------------------------------------
// 参照ヘッダ include
//----------------------------------------------------------------------
#include "battle/battle.h"
#include "btl_main_def.h"
#include "btl_adapter.h"
#include "btl_pokeparam.h"
#include "btlv/btlv_core_def.h"
#include "app/b_bag.h"

extern BTL_CLIENT* BTL_CLIENT_Create(
  BTL_MAIN_MODULE* mainModule, BTL_POKE_CONTAINER* pokecon,
  BtlCommMode commMode, GFL_NETHANDLE* netHandle, u16 clientID, u16 numCoverPos,
  BtlClientType clientType, BtlBagMode bagMode,
  HEAPID heapID );

extern void BTL_CLIENT_Delete( BTL_CLIENT* wk );

extern u8 BTL_CLIENT_GetEscapeClientID( const BTL_CLIENT* wk );
extern BOOL BTL_CLIENT_IsGameTimeOver( const BTL_CLIENT* wk );


extern BTL_ADAPTER* BTL_CLIENT_GetAdapter( BTL_CLIENT* client );

extern void BTL_CLIENT_AttachViewCore( BTL_CLIENT* wk, BTLV_CORE* viewCore );

extern BOOL BTL_CLIENT_Main( BTL_CLIENT* client );


extern u8 BTL_CLIENT_GetClientID( const BTL_CLIENT* client );
extern const BTL_PARTY* BTL_CLIENT_GetParty( const BTL_CLIENT* client );
extern BtlPokePos BTL_CLIENT_GetProcPokePos( const BTL_CLIENT* client );
extern BtlWeather BTL_CLIENT_GetWeather( const BTL_CLIENT* client );


extern const void* BTL_CLIENT_GetRecordData( BTL_CLIENT* wk, u32* size );
extern void BTL_CLIENT_SetRecordPlayType( BTL_CLIENT* wk, const void* recordData, u32 dataSize );


#endif
