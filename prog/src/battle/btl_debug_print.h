//=============================================================================================
/**
 * @file  btl_debug_print.h
 * @brief ポケモンWB バトルシステム デバッグPrint機能
 * @author  taya
 *
 * @date  2010.01.07  作成
 */
//=============================================================================================
#pragma once


typedef enum {
  BTL_PRINTTYPE_UNKNOWN,    ///< タイプ未確定
  BTL_PRINTTYPE_SERVER,     ///< タイプサーバー
  BTL_PRINTTYPE_CLIENT,     ///< タイプサーバー以外
  BTL_PRINTTYPE_STANDALONE, ///< タイプスタンドアロン実行
}BtlPrintType;

/**
 *  文字列リテラルID
 */
typedef enum {
  DBGSTR_SETUP_DONE = 0,
  DBGSTR_DEBUGFLAG_BIT,
  DBGSTR_ADAPTER_SEND_START,
  DBGSTR_ADAPTER_RECV_DONE,
  DBGSTR_ADAPTER_RECVED_A_CLIENT,
  DBGSTR_CLIENT_RETURN_CMD_START,
  DBGSTR_CLIENT_RETURN_CMD_DONE,
  DBGSTR_CLIENT_SelActPokeDead,
  DBGSTR_CLIENT_SelectActionSkip,
  DBGSTR_CLIENT_SelectActionStart,
  DBGSTR_CLIENT_SelectActionDone,
  DBGSTR_CLIENT_ReturnSeqDone,
  DBGSTR_CLIENT_WazaLockInfo,
  DBGSTR_CLIENT_NoMorePuttablePoke,
  DBGSTR_CLIENT_NotDeadMember,
  DBGSTR_CLIENT_ChangePokeNum,
  DBGSTR_CLIENT_NumChangePokeBegin,
  DBGSTR_CLIENT_NumChangePokeResult,
  DBGSTR_CLIENT_PokeSelCnt,
  DBGSTR_CLIENT_PokeChangeIdx,

  DBGSTR_MAIN_SwapPartyMember,
  DBGSTR_NET_DecideClientID,
  DBGSTR_NET_SendSucceed,
  DBGSTR_NET_RecvedServerParam,
  DBGSTR_NET_RecvedPartyData,
  DBGSTR_NET_PartyDataComplete,
  DBGSTR_NET_SendCmdDone,
  DBGSTR_NET_RecvedAllClientsData,
  DBGSTR_NET_ReturnToServerTrying,
  DBGSTR_NET_RecvedClientData,
  DBGSTR_CALCDMG_BaseDamage,
  DBGSTR_CALCDMG_RangeHosei,
  DBGSTR_CALCDMG_WeatherHosei,
  DBGSTR_CALCDMG_RAW_DAMAGE,
  DBGSTR_CALCDMG_Critical,
  DBGSTR_CALCDMG_RandomHosei,
  DBGSTR_CALCDMG_TypeMatchHosei,
  DBGSTR_CALCDMG_TypeAffInfo,
  DBGSTR_CALCDMG_DamageResult,
  DBGSTR_CALCDMG_DamageMarume,
  DBGSTR_STR_StdStrID,
  DBGSTR_STR_SetStrID,
  DBGSTR_STR_StdStrInfo,
  DBGSTR_STR_Arg,
  DBGSTR_STR_SetPoke1,
  DBGSTR_STR_SetPoke2,
  DBGSTR_PRINT_BufIdx,
  DBGSTR_STR_TagSetNumber,
  DBGSTR_STR_SetPokeNickname,
  DBGSTR_STR_SetTagWazaName,
  DBGSTR_STR_SetItemName,
  DBGSTR_SVFL_AtkPower,
  DBGSTR_SVFL_DefGuard,
  DBGSTR_SVFL_WazaPower,
  DBGSTR_SVFL_UncategoryWazaInfo,
  DBGSTR_SVFL_QueWritePtr,
  DBGSTR_SVFL_ActOrderStart,
  DBGSTR_SVFL_ActOrder_Fight,
  DBGSTR_SVFL_ActOrder_Item,
  DBGSTR_SVFL_ActOrder_Change,
  DBGSTR_SVFL_ActOrder_Escape,
  DBGSTR_SVFL_ActOrder_Dead,
  DBGSTR_SVFL_CorrectTarget_Info,
  DBGSTR_SVFL_CorrectResult,
  DBGSTR_SVFL_CorrectHitFarOn,
  DBGSTR_SVFL_CorrectHitFarOff,
  DBGSTR_SVFL_CorrectTargetNum,
  DBGSTR_SVFL_CorrectTargetDiff,
  DBGSTR_SVFL_CorrectTargetHP,
  DBGSTR_SVFL_ClientPokeStillAlive,
  DBGSTR_SVFL_ClientPokeDeadAll,
  DBGSTR_SVFL_PosEffAdd,
  DBGSTR_SVFL_PosEffDupFail,
  DBGSTR_SVFL_HitCheckInfo1,
  DBGSTR_SVFL_HitCheckInfo2,
  DBGSTR_SVFL_HitCheckInfo3,
  DBGSTR_SVFL_HitCheckInfo4,
  DBGSTR_SVFL_StartAfterPokeChange,
  DBGSTR_SVFL_PokeChangeRootInfo,

  DBGSTR_SC_PutCmd,
  DBGSTR_SC_ReservedPos,
  DBGSTR_SC_WriteReservedPos,
  DBGSTR_SC_PutMsgParam,
  DBGSTR_SC_PutMsg_SE,
  DBGSTR_SC_ArgsEqual,
  DBGSTR_SERVER_SendShooterChargeCmd,
  DBGSTR_SERVER_ShooterChargeCmdDoneAll,
  DBGSTR_SERVER_SendActionSelectCmd,
  DBGSTR_SERVER_ActionSelectDoneAll,
  DBGSTR_SERVER_FlowResult,
  DBGSTR_SV_ChangePokeOnTheTurn,
  DBGSTR_SV_PokeInReqForEmptyPos,
  DBGSTR_SV_StartChangePokeInfo,

  DBGSTR_EV_AddFactor,
  DBGSTR_EV_DelFactor,
  DBGSTR_EV_LinkInfoHeader,
  DBGSTR_EV_LinkInfoFooder,
  DBGSTR_EV_LinkPtr,
  DBGSTR_EV_LinkEmpty,
  DBGSTR_EVAR_Push,
  DBGSTR_EVAR_Pop,

  DBGSTR_VCORE_PokeListStart,

  DBGSTR_POSPOKE_Out,
  DBGSTR_POSPOKE_In,
  DBGSTR_SCU_RelivePokeAct,

  DBGSTR_val_comma,
  DBGSTR_done,
  DBGSTR_LF,

}BtlDebugStrID;


extern void BTL_DEBUGPRINT_SetType( BtlPrintType type );
extern void BTL_DEBUGPRINT_PrintHeader( const char* fileName, u32 line );
extern void BTL_DEBUGPRINT_PrintDump( const char* caption, const void* data, u32 size );
extern const char* BTL_DEBUGPRINT_GetFormatStr( BtlDebugStrID strID );

