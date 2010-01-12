//=============================================================================================
/**
 * @file  btl_debug_print.c
 * @brief ポケモンWB バトルシステム デバッグPrint機能
 * @author  taya
 *
 * @date  2010.01.07  作成
 */
//=============================================================================================

#include "btl_util.h"
#include "btl_debug_print.h"


/*--------------------------------------------------------------------------*/
/* Globals                                                                  */
/*--------------------------------------------------------------------------*/
static BtlPrintType GPrintType;

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void print_type( void );
static void print_file_info( const char* filename, int line );



//=============================================================================================
/**
 * プリント時のヘッダタイプ通知を受け取り
 *
 * @param   type
 */
//=============================================================================================
void BTL_DEBUGPRINT_SetType( BtlPrintType type )
{
  GPrintType = type;
}

void BTL_DEBUGPRINT_PrintDump( const char* caption, const void* data, u32 size )
{
  OS_TPrintf( "[DUMP] %s (%dbytes)\n", caption, size );

  if( size > 8 ){ size = 8; }

  {
    const u8* p = data;
    u32 i;
    for(i=0; i<size; i++)
    {
      OS_TPrintf("%02x ", p[i]);
    }
    OS_TPrintf("\n");
  }
}

const char* BTL_DEBUGPRINT_GetFormatStr( BtlDebugStrID strID )
{
  switch( strID ){
  case DBGSTR_SETUP_DONE:               return "セットアップ完了\n";
  case DBGSTR_DEBUGFLAG_BIT:            return "デバッグフラグbit=%04x\n";
  case DBGSTR_ADAPTER_SEND_START:       return " Adapter-%d, コマンド %d を送信開始します\n";
  case DBGSTR_ADAPTER_RECV_DONE:        return " [Adapter] Received from All Clients\n";
  case DBGSTR_ADAPTER_RECVED_A_CLIENT:  return " [Adapter] Received from AdapterID=%d\n";
  case DBGSTR_CLIENT_RETURN_CMD_START:  return "ID[%d], 返信開始へ\n";
  case DBGSTR_CLIENT_RETURN_CMD_DONE:   return "ID[%d], 返信しました\n";
  case DBGSTR_CLIENT_SelectActionSkip:  return "Act選択(%d体目）スキップ\n";
  case DBGSTR_CLIENT_SelActPokeDead:    return "死んでてスキップなのでCheckedCntそのまま\n";
  case DBGSTR_CLIENT_SelectActionStart: return "Act選択(%d体目=ID:%d）開始します  checked %d poke\n";
  case DBGSTR_CLIENT_SelectActionDone:  return "カバー位置数(%d)終了、アクション送信へ\n";
  case DBGSTR_CLIENT_ReturnSeqDone:     return "返信シーケンス終了\n";
  case DBGSTR_CLIENT_WazaLockInfo:      return "ワザロック：Client[%d] 前回使ったワザは %d, idx=%d, targetPos=%d\n";
  case DBGSTR_MAIN_SwapPartyMember:     return "パーティメンバー入れ替え %d <-> %d\n";
  case DBGSTR_NET_DecideClientID:       return "NetID:%d -> clientID=%d\n";
  case DBGSTR_NET_SendSucceed:          return "  送信成功\n";
  case DBGSTR_NET_RecvedServerParam:    return "netID=%d, サーバパラメータ受信しました。\n";
  case DBGSTR_NET_RecvedPartyData:      return "netID=%d, clientID=%d のパーティデータ受信完了, pData=%p, buf=%p\n";
  case DBGSTR_NET_PartyDataComplete:    return "パーティデータ相互受信完了  member=%d\n";
  case DBGSTR_NET_SendCmdDone:          return "マシン(%d) に %d bytes 送信完了\n";
  case DBGSTR_NET_RecvedAllClientsData: return "全クライアントからのデータ返ってきた\n";
  case DBGSTR_NET_ReturnToServerTrying: return "Try Return to Server %d byte ...";
  case DBGSTR_NET_RecvedClientData:     return "[BTLNET] recv from netID[%d], size=%d\n";
  case DBGSTR_CALCDMG_BaseDamage:       return "基礎ダメージ値 (%d)\n";
  case DBGSTR_CALCDMG_RangeHosei:       return "対象数によるダメージ補正:%d\n";
  case DBGSTR_CALCDMG_WeatherHosei:     return "天候による補正が発生, 補正率=%08x, dmg=%d->%d\n";
  case DBGSTR_CALCDMG_RAW_DAMAGE:       return "威力:%d, Lv:%d, こうげき:%d, ぼうぎょ:%d,  ... 素ダメ:%d\n";
  case DBGSTR_CALCDMG_Critical:         return "クリティカルだから素ダメ->%d\n";
  case DBGSTR_CALCDMG_RandomHosei:      return "ランダム補正:%d%%  -> 素ダメ=%d\n";
  case DBGSTR_CALCDMG_TypeMatchHosei:   return "タイプ一致補正:%d%%  -> 素ダメ=%d\n";
  case DBGSTR_CALCDMG_TypeAffInfo:      return "タイプ相性:%02d -> ダメージ値：%d\n";
  case DBGSTR_CALCDMG_DamageResult:     return "ダメ受けポケモン=%d, ratio=%08x, Damage=%d -> %d\n";
  case DBGSTR_CALCDMG_DamageMarume:     return "ダメ受けポケモンのＨＰ値にまるめ->%d\n";
  case DBGSTR_STR_StdStrID:             return " STD:strID=%d\n";
  case DBGSTR_STR_SetStrID:             return " SET:strID=%d\n";
  case DBGSTR_STR_StdStrInfo:           return "STD STR ID=%d, argCnt=%d\n";
  case DBGSTR_STR_Arg:                  return "  arg(%d)=%d\n";
  case DBGSTR_STR_SetPoke1:             return "ms set_poke1 strID=%d, args[0]=%d, args[1]=%d\n";
  case DBGSTR_STR_SetPoke2:             return "ms set_poke2 strID=%d, args[0]=%d, args[1]=%d\n";
  case DBGSTR_PRINT_BufIdx:             return "bufIdx=%d ....\n";
  case DBGSTR_STR_TagSetNumber:         return "[TAG] SetNumber : keta=%d, value=%d\n";
  case DBGSTR_STR_SetPokeNickname:      return "setPokeNickName ID=%d ....\n";
  case DBGSTR_STR_SetTagWazaName:       return "[TAG] Set WazaName ... waza=%d\n";
  case DBGSTR_STR_SetItemName:          return "set Item Name argIdx=%d, ID=%d ....\n";
  case DBGSTR_SVFL_ActOrderStart:       return "ポケ[%d =monsno:%d]のアクション実行...\n";
  case DBGSTR_SVFL_ActOrder_Fight:      return "【たたかう】を処理。ワザ[%d]を、位置[%d]の相手に。\n";
  case DBGSTR_SVFL_ActOrder_Item:       return "【どうぐ】を処理。アイテム%dを、%d番の相手に。\n";
  case DBGSTR_SVFL_ActOrder_Change:     return "【ポケモン】を処理。位置%d <- ポケ%d \n";
  case DBGSTR_SVFL_ActOrder_Escape:     return "【にげる】を処理。\n";
  case DBGSTR_SVFL_ActOrder_Dead:       return "【ひんし】なので何もせず\n";

  case DBGSTR_SVFL_CorrectTarget_Info:  return "攻撃ポケ[%d]（位置=%d） -> 狙ったポケ[%d]（位置=%d）死んでいるので補正する\n";
  case DBGSTR_SVFL_CorrectHitFarOn:     return "ワザ[%d] 遠隔ON の補正対象決定\n";
  case DBGSTR_SVFL_CorrectHitFarOff:    return "ワザ[%d] 遠隔OFF の補正対象決定\n";
  case DBGSTR_SVFL_CorrectTargetNum:    return "補正対象数  %d / %d \n";
  case DBGSTR_SVFL_CorrectTargetDiff:   return "対象候補の距離 Poke[%d]->%d, Poke[%d]->%d\n";
  case DBGSTR_SVFL_CorrectTargetHP:     return "対象候補の残HP Poke[%d]->%d, Poke[%d]->%d\n";
  case DBGSTR_SVFL_CorrectResult:       return "補正後の対象ポケモンは 位置=%d, ID=%d\n";
  case DBGSTR_SVFL_AtkPower:            return "攻撃力=%d  (Critical=%d, ratio=%08x\n";
  case DBGSTR_SVFL_DefGuard:            return "防御力=%d  (Critical=%d, ratio=%08x\n";
  case DBGSTR_SVFL_WazaPower:           return "ワザ威力は%d  (ratio=%08x)\n";
  case DBGSTR_SVFL_UncategoryWazaInfo:  return "未分類ワザ 攻撃PokeID=%d, 対象ポケ数=%d/%d\n";
  case DBGSTR_SVFL_QueWritePtr:         return "[* SVF *]         Que WritePtr=%d\n";
  case DBGSTR_SVFL_ClientPokeStillAlive:  return "クライアント_%d (SIDE:%d) のポケはまだ何体か生きている\n";
  case DBGSTR_SVFL_ClientPokeDeadAll:   return "クライアント_%d (SIDE:%d) のポケは全滅した\n";

  case DBGSTR_SERVER_FlowResult:        return "サーバー処理結果=%d\n";
  case DBGSTR_SERVER_SendShooterChargeCmd:    return "シューターチャージコマンド発行\n";
  case DBGSTR_SERVER_ShooterChargeCmdDoneAll: return "全クライアントでシューターチャージコマンド処理終了\n";
  case DBGSTR_SERVER_SendActionSelectCmd:     return "アクション選択コマンド発行\n";
  case DBGSTR_SERVER_ActionSelectDoneAll:     return "アクション受け付け完了\n";

  case DBGSTR_SC_PutCmd:                return "[QUE]PutCmd=%d, Format=%02x, argCnt=%d, args=";
  case DBGSTR_SC_ReservedPos:           return "[QUE]reserved pos=%d, wp=%d\n";
  case DBGSTR_SC_WriteReservedPos:      return "[QUE]Write Reserved Pos ... pos=%d, cmd=%d";
  case DBGSTR_SC_PutMsgParam:           return "[QUE] PUT MSG SC=%d, StrID=%d";
  case DBGSTR_SC_PutMsg_SE:             return "  SE_ID=%d\n";
  case DBGSTR_SC_ArgsEqual:             return " args = ";
  case DBGSTR_EV_AddFactor:             return "[ADD] Factor=%p Depend=%d Type=%d, Pri=%06x [ADD]\n";
  case DBGSTR_EV_DelFactor:             return "[DEL] DEL Factor=%p Depend=%d, Type=%d [DEL]\n";
  case DBGSTR_EV_LinkInfoHeader:        return "***** [[EV Chain]] *****\n";
  case DBGSTR_EV_LinkInfoFooder:        return "************************\n\n";
  case DBGSTR_EV_LinkPtr:               return "%p->";
  case DBGSTR_EV_LinkEmpty:             return " empty...\n";
  case DBGSTR_EVAR_Push:                return "PUSH [%5d] SP=%d\n";
  case DBGSTR_EVAR_Pop:                 return "Pop! [%5d] SP=%d\n";



  case DBGSTR_val_comma:  return "%d,";
  case DBGSTR_done: return "done!";
  case DBGSTR_LF: return "\n";

  }
  return NULL;
}


//=============================================================================================
/**
 * デバッグ用ヘッダPrint
 *
 * @param   fileName
 * @param   line
 */
//=============================================================================================
void BTL_DEBUGPRINT_PrintHeader( const char* fileName, u32 line )
{
  print_type();
  print_file_info( fileName, line );
}
static void print_type( void )
{
  switch( GPrintType ){
  case BTL_PRINTTYPE_SERVER:      OS_TPrintf("SV/"); break;
  case BTL_PRINTTYPE_CLIENT:      OS_TPrintf("CL/"); break;
  case BTL_PRINTTYPE_STANDALONE:  OS_TPrintf("SA/"); break;
  case BTL_PRINTTYPE_UNKNOWN:
  default:
    OS_TPrintf("UN/");
    break;
  }
}
static void print_file_info( const char* filename, int line )
{
  static const struct {
    char* longName;
    char* shortName;
  }names[] = {
    { "btl_main.c",       "MAI" },
    { "btl_server.c",     "SVR" },
    { "btl_client.c",     "CLI" },
    { "btl_adapter.c",    "ADP" },
    { "btl_string.c",     "STR" },
    { "btl_net.c",        "NET" },
    { "btl_calc.c",       "CAL" },
    { "btl_sick.c",       "SIC" },
    { "btl_event.c",      "EVE" },
    { "btl_pokeparam.c",  "BPP" },
    { "btl_server_cmd.c", "CMD" },
    { "btl_server_flow.c","FLW" },
    { "btl_field.c",      "FLD" },
    { "btl_rec.c",        "REC" },
    { "btlv_core.c",      "VIW" },
    { "btlv_scu.c",       "scU" },
    { "btlv_scd.c",       "scD" },
    { NULL,               "OTR" },
  };
  u32 i;

  for(i=0; names[i].longName!=NULL; ++i)
  {
    if( !GFL_STD_StrCmp(names[i].longName, filename) )
    {
      break;
    }
  }
  OS_TPrintf( "[%s-%4d]", names[i].shortName, line);
}
