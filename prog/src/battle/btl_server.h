//=============================================================================================
/**
 * @file  btl_server.h
 * @brief ポケモンWB バトルシステム サーバモジュール
 * @author  taya
 *
 * @date  2008.09.06  作成
 */
//=============================================================================================
#ifndef __BTL_SERVER_H__
#define __BTL_SERVER_H__

#include "btl_common.h"
#include "btl_main_def.h"
#include "btl_pokeparam.h"
#include "btl_adapter.h"
#include "btl_action.h"
#include "btl_rec.h"
#include "btl_calc.h"
#include "btl_server_flow_def.h"
#include "app/b_bag.h"

typedef struct _BTL_SERVER       BTL_SERVER;
typedef struct _BTL_SVCL_ACTION  BTL_SVCL_ACTION;

typedef enum {

  SC_RES_CONTINUE,    ///< 次のターンへ。
  SC_RES_WAIT_CHANGE, ///< 入れ替え待ち
  SC_RES_ESCAPE,      ///< どちらかが逃げて終了
  SC_RES_CLOSE,       ///< どちらかが勝って終了

}ServerCmdResult;


extern BTL_SERVER* BTL_SERVER_Create( BTL_MAIN_MODULE* mainWork, const GFL_STD_RandContext* randContext,
  BTL_POKE_CONTAINER* pokeCon, BtlBagMode bagMode, HEAPID heapID );

extern void BTL_SERVER_AttachLocalClient( BTL_SERVER* server, BTL_ADAPTER* adapter, u8 clientID, u8 numCoverPos );
extern void BTL_SERVER_ReceptionNetClient( BTL_SERVER* server, BtlCommMode commMode, GFL_NETHANDLE* netHandle, u8 clientID, u8 numCoverPos );
extern void BTL_SERVER_CmdCheckMode( BTL_SERVER* server, u8 clientID, u8 numCoverPos );


extern void BTL_SERVER_Startup( BTL_SERVER* server );
extern void BTL_SERVER_Delete( BTL_SERVER* wk );

extern BOOL BTL_SERVER_Main( BTL_SERVER* server );
extern void BTL_SERVER_GetEscapeInfo( const BTL_SERVER* sv, BTL_ESCAPEINFO* dst );
extern void BTL_SERVER_NotifyCommError( BTL_SERVER* sv );


extern void BTL_SERVER_IntrLevelup_Start( BTL_SERVER* server );
extern BOOL BTL_SERVER_IntrLevelup_Main( BTL_SERVER* server );


extern BTL_SVFLOW_WORK* BTL_SERVER_GetFlowWork( BTL_SERVER* server );

//-------------------------------------
// server_flow -> server 呼び出し専用
//-------------------------------------
extern void BTL_SERVER_NotifyPokemonLevelUp( BTL_SERVER* server, const BTL_POKEPARAM* bpp );
extern void BTL_SERVER_NotifyPokemonCapture( BTL_SERVER* server, const BTL_POKEPARAM* bpp );
extern void BTL_SERVER_NotifyBattleResult( BTL_SERVER* server, BtlResult result );
extern void BTL_SERVER_AddBonusMoney( BTL_SERVER* server, u32 volume );
extern void BTL_SERVER_SetMoneyDblUp( BTL_SERVER* server );
extern void BTL_SERVER_InitChangePokemonReq( BTL_SERVER* server );
extern void BTL_SERVER_RequestChangePokemon( BTL_SERVER* server, BtlPokePos pos );
extern BOOL BTL_SERVER_IsFrontPokemon( BTL_SERVER* server, const BTL_POKEPARAM* bpp );

//-------------------------------------
// client->コマンド整合性チェック処理
//-------------------------------------

extern void BTL_SERVER_CMDCHECK_RestoreActionData( BTL_SERVER* server, const void* recData, u32 recDataSize );
extern BOOL BTL_SERVER_CMDCHECK_Make( BTL_SERVER* server, BtlRecTiming timingCode, const void* recvedCmd, u32 cmdSize );
extern u8 BTL_SVCL_ACTION_GetNumActPoke( const BTL_SVCL_ACTION* clientAction, u8 clientID );


#endif
