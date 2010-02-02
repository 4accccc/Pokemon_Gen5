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
  case DBGSTR_SETUP_DONE:                    return "セットアップ完了\n";
  case DBGSTR_DEBUGFLAG_BIT:                 return "デバッグフラグbit=%04x\n";
  case DBGSTR_ADAPTER_SEND_START:            return " Adapter-%d, コマンド %d を送信開始します\n";
  case DBGSTR_ADAPTER_RECV_DONE:             return " [Adapter] Received from All Clients\n";
  case DBGSTR_ADAPTER_RECVED_A_CLIENT:       return " [Adapter] Received from AdapterID=%d\n";
  case DBGSTR_CLIENT_RETURN_CMD_START:       return "ID[%d], 返信開始へ\n";
  case DBGSTR_CLIENT_RETURN_CMD_DONE:        return "ID[%d], 返信しました\n";
  case DBGSTR_CLIENT_SelectActionSkip:       return "Act選択(%d体目）スキップ\n";
  case DBGSTR_CLIENT_SelActPokeDead:         return "死んでてスキップなのでCheckedCntそのまま\n";
  case DBGSTR_CLIENT_SelectActionStart:      return "Act選択(%d体目=ID:%d）開始します  checked %d poke\n";
  case DBGSTR_CLIENT_SelectActionDone:       return "カバー位置数(%d)終了、アクション送信へ\n";
  case DBGSTR_CLIENT_SelectAction_Pokemon:   return "「ポケモン」を選んだ->選択画面へ\n";
  case DBGSTR_CLIENT_SelectChangePoke:       return " ポケモン選んだ ... idx=%d\n";
  case DBGSTR_CLIENT_SelectChangePokeCancel: return " ポケモン選ばなかった\n";

  case DBGSTR_CLIENT_ReturnSeqDone:       return "返信シーケンス終了\n";
  case DBGSTR_CLIENT_WazaLockInfo:        return "ワザロック：Client[%d] 前回使ったワザは %d, targetPos=%d\n";
  case DBGSTR_CLIENT_NoMorePuttablePoke:  return "myID=%d もう戦えるポケモンいない\n";
  case DBGSTR_CLIENT_NotDeadMember:       return "myID=%d 誰も死んでないから選ぶ必要なし\n";
  case DBGSTR_CLIENT_ChangePokeCmdInfo:   return "myID=%d %d体選択する必要あり mode=%d\n";
  case DBGSTR_CLIENT_NumChangePokeBegin:  return " 全Client, 選択すべきポケモン数=%d\n　位置=";
  case DBGSTR_CLIENT_NumChangePokeResult: return " 自分[%d]が選択すべきポケモン数=%d\n";
  case DBGSTR_CLIENT_PokeSelCnt:          return "入れ替え%d体選んだ\n";
  case DBGSTR_CLIENT_PokeChangeIdx:       return "ポケモン入れ替え %d体目 <-> %d体目\n";
  case DBGSTR_CLIENT_ForbidEscape_Kagefumi_Chk:     return "逃げ交換禁止チェック「かげふみ」by pokeID(%d)\n";
  case DBGSTR_CLIENT_ForbidEscape_Kagefumi_Enable:  return "  「かげふみ」有効！\n";
  case DBGSTR_CLIENT_ForbidEscape_Arijigoku_Chk:    return "逃げ交換禁止チェック「ありじごく」by pokeID(%d)\n";
  case DBGSTR_CLIENT_ForbidEscape_Arijigoku_Enable: return "  「ありじごく」有効！\n";
  case DBGSTR_CLIENT_ForbidEscape_Jiryoku_Chk:      return "逃げ交換禁止チェック「じりょく」by pokeID(%d)\n";
  case DBGSTR_CLIENT_ForbidEscape_Jiryoku_Enable:   return "  「じりょく」有効！\n";
  case DBGSTR_CLIENT_UpdateWazaProcResult:          return "ワザプロセス情報更新 : PokeID=%d, OrgWaza=%d, ActWaza=%d, ActTargetPos=%d, ActEnable=%d\n";
  case DBGSTR_CLIENT_CmdLimitTimeEnable:  return "コマンド選択制限時間が有効 -> %d sec\n";
  case DBGSTR_CLIENT_CmdLimitTimeOver:    return "コマンド選択タイムアップ ... 強制終了フラグON\n";

  case DBGSTR_BPP_NemuriWakeCheck:        return "ポケ[%d]のねむりターン最大値=%d, 経過ターン=%d\n";

  case DBGSTR_VCORE_PokeListStart:      return "交換リスト画面  モード=%d   すでに%d体選択されている\n";
  case DBGSTR_VCORE_SelPokeEnd:         return "ポケ選択おわった\n";
  case DBGSTR_VCORE_SelPokeEnd_Sel:     return "  [%d]番目 選択された -> %d体 格納\n";
  case DBGSTR_VCORE_SelPokeEnd_Unsel:   return "  [%d]番目 選択されていない\n";


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
  case DBGSTR_SVFL_WazaPower:           return "ワザ[%d] : 威力=%d  (ratio=%08x)\n";
  case DBGSTR_SVFL_UncategoryWazaInfo:  return "未分類ワザ 攻撃PokeID=%d, 対象ポケ数=%d/%d\n";
  case DBGSTR_SVFL_QueWritePtr:         return "[* SVF *]         Que WritePtr=%d\n";
  case DBGSTR_SVFL_ClientPokeStillAlive:return "クライアント_%d (SIDE:%d) のポケはまだ何体か生きている\n";
  case DBGSTR_SVFL_ClientPokeDeadAll:   return "クライアント_%d (SIDE:%d) のポケは全滅した\n";
  case DBGSTR_SVFL_PosEffAdd:           return "位置エフェクトハンドラ生成: pos=%d, effect=%d, pokeID=%d\n";
  case DBGSTR_SVFL_PosEffDupFail:       return "位置(%d)にPOSEffect[%d]ハンドラを追加失敗（使用ポケ=%d）\n";
  case DBGSTR_SVFL_HitCheckInfo1:       return "攻撃ポケ[%d]  命中Rank=%d  ワザ的中率=%d\n";
  case DBGSTR_SVFL_HitCheckInfo2:       return "防御ポケ[%d]  回避Rank=%d\n";
  case DBGSTR_SVFL_HitCheckInfo3:       return "命中ランク=%d, 命中率=%d, 命中率補正=%08x\n";
  case DBGSTR_SVFL_HitCheckInfo4:       return "最終命中率 = %d\n";
  case DBGSTR_SVFL_StartAfterPokeChange: return "ターン途中ポケモン入れ替え後のサーバーコマンド生成\n";
  case DBGSTR_SVFL_PokeChangeRootInfo:  return "クライアント(%d)のポケモン(位置%d) を、%d番目のポケといれかえる\n";
  case DBGSTR_SVFL_UseItemCall:         return "シューター「アイテムコール」使用  対象Poke=%d\n";
  case DBGSTR_SVFL_ChangePokeReqInfo:   return "クライアント[%d]   空いている位置の数=%d\n";
  case DBGSTR_SVFL_TurnStart_Result:    return "【ターン開始コマンド】処理アクション %d / %d\n";
  case DBGSTR_SVFL_ActOrderMainStart:   return "コマンド生成コア 処理アクション開始Index=%d\n";
  case DBGSTR_SVFL_ActOrderMainDropOut: return "Result=%dによる中途サーバ返信\n";
  case DBGSTR_SVFL_ExpCalc_Base:        return "基本となる経験値=%d\n";
  case DBGSTR_SVFL_ExpCalc_MetInfo:     return "死亡ポケ[%d]が対面した相手ポケの数=%d, その内、生きてる数=%d\n";
  case DBGSTR_SVFL_ExpCalc_DivideInfo:  return "メンバーIdx[%d]のポケに経験値%dを分配\n";
  case DBGSTR_SVFL_ExpCalc_Result:      return "メンバーIdx[%d]のポケに対し、最終経験値=%d\n";
  case DBGSTR_SVFL_ExpAdjustCalc:       return "自分Lv=%d, 敵Lv=%d, 倍率=%08x, 基本経験値=%d -> 補正後経験値=%d\n";
  case DBGSTR_SVFL_RecDataSendComped:   return "操作記録データの送信完了\n";
  case DBGSTR_SVFL_SendServerCmd:       return "サーバコマンド送信します ... result=%d\n";
  case DBGSTR_SVFL_AllClientCmdPlayComplete:  return "全クライアントのコマンド再生終了...result=%d\n";
  case DBGSTR_SVFL_GotoQuit:            return "バトル終了へ\n";
  case DBGSTR_SVFL_ReqWazaCallActOrder: return "他ワザ呼び出し [%d] --> [%d], target=%d\n";
  case DBGSTR_SVFL_WazaExeFail_1:       return "ワザ出し失敗（１）  ポケID=%d, 失敗コード=%d\n";
  case DBGSTR_SVFL_WazaExeFail_2:       return "ワザ出し失敗（２）  ポケID=%d, 失敗コード=%d\n";

  case DBGSTR_POSPOKE_Out:              return " poke[%d] out from pos[%d]\n";
  case DBGSTR_POSPOKE_In:               return " poke[%d] in to pos[%d]\n";


  case DBGSTR_SERVER_FlowResult:        return "サーバー処理結果=%d\n";
  case DBGSTR_SERVER_SendShooterChargeCmd:    return "シューターチャージコマンド発行\n";
  case DBGSTR_SERVER_ShooterChargeCmdDoneAll: return "全クライアントでシューターチャージコマンド処理終了\n";
  case DBGSTR_SERVER_SendActionSelectCmd:     return "アクション選択コマンド発行\n";
  case DBGSTR_SERVER_ActionSelectDoneAll:     return "アクション受け付け完了\n";
  case DBGSTR_SV_ChangePokeOnTheTurn:    return "ターン途中のポケ入れ替え発生\n";
  case DBGSTR_SV_PokeInReqForEmptyPos:   return "空き位置への新ポケ投入リクエスト受け付け\n";
  case DBGSTR_SV_StartChangePokeInfo:    return "入れ替えポケモン選択へ  交替されるポケ数=%d\n";


  case DBGSTR_SC_PutCmd:                return "[QUE]PutCmd=%d, Format=%02x, argCnt=%d, args=";
  case DBGSTR_SC_ReservedPos:           return "[QUE]reserved pos=%d, wp=%d\n";
  case DBGSTR_SC_WriteReservedPos:      return "[QUE]Write Reserved Pos ... pos=%d, cmd=%d";
  case DBGSTR_SC_PutMsgParam:           return "[QUE] PUT  MSG SC=%d, StrID=%d";
  case DBGSTR_SC_ReadMsgParam:          return "[QUE] READ MSG SC=%d, StrID=%d";
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
  case DBGSTR_DEADREC_Add:              return " 死亡記録 ターン%d件目 ... pokeID=%d\n";

  case DBGSTR_SCU_RelivePokeAct:        return "Pos[%d]=Vpos[%d] ポケ(%d)が生き返り\n";



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
    { "btl_main.c",           "MAI" },
    { "btl_server.c",         "SVR" },
    { "btl_client.c",         "CLI" },
    { "btl_adapter.c",        "ADP" },
    { "btl_string.c",         "STR" },
    { "btl_net.c",            "NET" },
    { "btl_calc.c",           "CAL" },
    { "btl_sick.c",           "SIC" },
    { "btl_event.c",          "EVE" },
    { "btl_pokeparam.c",      "BPP" },
    { "btl_pospoke_state.c",  "POS" },
    { "btl_server_cmd.c",     "CMD" },
    { "btl_server_flow.c",    "FLW" },
    { "btl_field.c",          "FLD" },
    { "btl_rec.c",            "REC" },
    { "btlv_core.c",          "VIW" },
    { "btlv_scu.c",           "scU" },
    { "btlv_scd.c",           "scD" },
    { NULL,                   "OTR" },
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
