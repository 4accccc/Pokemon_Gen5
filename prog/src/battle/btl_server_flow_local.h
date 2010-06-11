//=============================================================================================
/**
 * @file  btl_server_flow_local.h
 * @brief ポケモンWB バトルシステム サーバコマンド生成処理（メイン／サブルーチン群から共有する定数・構造体）
 * @author  taya
 *
 * @date  2010.05.28  作成
 */
//=============================================================================================
#pragma once

#include "btl_pokeset.h"
#include "btl_pospoke_state.h"
#include "btl_event_factor.h"

enum {
  HANDLER_EXHIBISION_WORK_TOTALSIZE = 500,    ///< ハンドラ反応情報を格納するワークのトータルサイズ
  NIGERU_COUNT_MAX = 30,                      ///< 「にげる」選択回数カウンタの最大値
  MEMBER_CHANGE_COUNT_MAX = 255,              ///< 入れ替えカウンタ最大値（バトル検定用）
  AFFCOUNTER_MAX = 9999,                      ///< 相性カウンタ最大値（バトル検定用）

  EVENT_HANDLER_TMP_WORK_SIZE = 320,          ///< ハンドラが一時的に利用するバッファのサイズ
};



//--------------------------------------------------------
/**
 *    サーバフロー処理フラグ
 */
//--------------------------------------------------------
typedef enum {

  FLOWFLG_FIRST_FIGHT,
  FLOWFLG_SET_WAZAEFFECT,

  FLOWFLG_MAX,
  FLOWFLG_BYTE_MAX = ((FLOWFLG_MAX/4)+((FLOWFLG_MAX&3)!=0)*4),

  EVENT_WORK_DEPTH = 16,
  EVENT_WORK_TOTAL_SIZE = 512,

  STRID_NULL = 0xffff,

}FlowFlag;

/**
 *  命令無視パターン
 */
typedef enum {

  SABOTAGE_NONE = 0,    ///< 命令無視しない
  SABOTAGE_OTHER_WAZA,  ///< 他のワザを勝手に出す
  SABOTAGE_GO_SLEEP,    ///< 眠ってしまう
  SABOTAGE_CONFUSE,     ///< 自分を攻撃
  SABOTAGE_DONT_ANY,    ///< 何もしない（メッセージランダム表示）
  SABOTAGE_SLEEPING,    ///< 眠ったまま命令無視（メッセージ表示）

}SabotageType;

/**
 *  溜めワザ判定結果
 */
typedef enum {

  TAMEWAZA_NONE = 0,    ///< 溜めワザではない
  TAMEWAZA_START,       ///< 溜めワザ：発動ターン
  TAMEWAZA_RELEASE,     ///< 溜めワザ：リリースターン

}TameWazaSeq;

/**
 *  ふきとばし系のワザ処理パターン
 */
typedef enum {
  PUSHOUT_EFFECT_ESCAPE,    ///< 逃げたのと同じ（戦闘を終了する）
  PUSHOUT_EFFECT_CHANGE,    ///< 相手を入れ替えさせる（手持ちがいなければ失敗する）
  PUSHOUT_EFFECT_MUSTFAIL,  ///< 必ず失敗する
}PushOutEffect;

/**
 *  他ワザ呼び出しパラメータ
 */
typedef struct {

  WazaID      wazaID;
  BtlPokePos  targetPos;

}REQWAZA_WORK;

/**
 * ヒットチェックパラメータ
 */

typedef struct {

  u8    countMax;         ///< 最大ヒット回数
  u8    count;            ///< 現在処理している回数

  u8    fCheckEveryTime;  ///< 毎回、ヒットチェックを行うか
  u8    fPluralHitWaza;   ///< 複数回ヒットするワザか
  u8    fPutEffectCmd;    ///< ワザエフェクト生成済み
  u8    pluralHitAffinity;


}HITCHECK_PARAM;

/**
 *  ワザエフェクト発動管理
 */
typedef struct {
  u8  fEnable;      ///< 発動確定フラグ
  u8  attackerPos;  ///< 発動開始位置
  u8  targetPos;    ///< 対象位置（位置が明確でないワザは BTL_POS_NULL ）
  u8  effectIndex;  ///< エフェクトIndex
}WAZAEFF_CTRL;

/**
 *  ワザ乗っ取りパラメータ
 */
typedef struct {
  u8  robberCount;                    ///< 乗っ取るポケモン数
  u8  robberPokeID[ BTL_POS_MAX ];    ///< 乗っ取り側ポケモンID
  u8  targetPos[ BTL_POS_MAX ];
  u8  targetPokeID[ BTL_POS_MAX ];
}WAZA_ROB_PARAM;


/**
 *  アクション優先順記録構造体
 */
typedef struct {

  BTL_POKEPARAM      *bpp;
  BTL_ACTION_PARAM   action;
  u32                priority;

  u8                 clientID;    ///< クライアントID
  u8                 fDone;       ///< 処理終了フラグ
  u8                 fIntrCheck;  ///< 割り込みチェック中フラグ
  u8                 defaultIdx;  ///< 特殊優先処理前の処理順（0～）

}ACTION_ORDER_WORK;

/**
 *  戦闘に出ているポケモンデータに順番にアクセスする処理のワーク
 */
typedef struct {
  u8 clientIdx;
  u8 pokeIdx;
  u8 endFlag;
  u8 rotationFlag;
}FRONT_POKE_SEEK_WORK;

/**
 *  イベント - ハンドラ間 連絡ワーク領域
 */
typedef struct {

  u8    work[ EVENT_WORK_TOTAL_SIZE ];
  u16   size[ EVENT_WORK_DEPTH ];
  u32   sp;

}BTL_EVENT_WORK_STACK;


/**
 *  ハンドラ反応保存用ワークマネージャ
 */
typedef struct {

//  u16  stack_ptr;
//  u16  read_ptr;
//  u8   f_prev_result;
//  u8   f_total_result;

  union {
    u32 state;
    struct {
      u32  useItem      : 10;
      u32  stack_ptr    :  9;
      u32  read_ptr     :  9;
      u32  fPrevSucceed :  1;  ///< 前回の使用が成功した
      u32  fSucceed     :  1;  ///< １度でも成功した
      u32  fUsed        :  1;  ///< １度でも実行された
      u32  _padd        :  1;
    };
  };

  u8   workBuffer[ HANDLER_EXHIBISION_WORK_TOTALSIZE ];

}HANDLER_EXHIBISION_MANAGER;

/**
 * ダメージワザ相性記録
 */
typedef struct {

  BtlTypeAff   aff[ BTL_POKEID_MAX ];
} BTL_DMGAFF_REC;


/**
 *  ダメージ計算結果格納用ワーク
 */
typedef struct {

  u8 real_hit_count;
  u8 migawari_hit_count;
  u8 total_hit_count;

  struct {

    u16         damage;
    u16         pokeID     : 5;
    u16         affine     : 4;   ///< ダメージ相性（ BtlTypeAff @ btl_calc.h ）
    u16         koraeru    : 4;   ///< こらえ原因（ BppKoraeruCause @ btl_pokeparam.h )
    u16         fCritical  : 1;
    u16         fFixDamage : 1;
    u16         fMigawari  : 1;


  }record[ BTL_POS_MAX ];

} BTL_CALC_DAMAGE_REC;

/**
 * １アクションごと必要なパラメータスタック構造１件分
 */
typedef struct {

  BTL_POKESET     TargetOrg;
  BTL_POKESET     Target;
  BTL_POKESET     Friend;
  BTL_POKESET     Enemy;
  BTL_POKESET     Damaged;
  BTL_POKESET     RobTarget;
  BTL_POKESET     psetTmp;
  WAZAEFF_CTRL    effCtrl;
  SVFL_WAZAPARAM  wazaParam;
  SVFL_WAZAPARAM  wazaParamOrg;
  HITCHECK_PARAM  hitCheck;
  WAZA_ROB_PARAM  wazaRobParam;
  WAZA_ROB_PARAM  magicCoatParam;
  BTL_CALC_DAMAGE_REC   calcDmgFriend;
  BTL_CALC_DAMAGE_REC   calcDmgEnemy;
  BtlPokePos      defaultTargetPos;
  u8              fMemberChangeReserve;

}POKESET_STACK_UNIT;

/**
 * 複数フラグを u32 で管理
 */
typedef union {
  u32 raw;

  struct {
    u32 hitPluralPoke : 1;   ///< 複数ポケモンにヒットする
    u32 delayAttack   : 1;   ///< みらいよち等、遅延攻撃
    u32 _pad          : 29;
  };

  struct {
    u32 magicCoat    : 1;  ///< マジックコート跳ね返し攻撃中
    u32 yokodori     : 1;  ///< 横取り実行中
    u32 reqWaza      : 1;  ///< 派生ワザ呼び出し中
  };

}FLAG_SET;

/**
 *  メッセージ表示処理コールバック関数型
 */
typedef void (*pMsgCallback)( BTL_SVFLOW_WORK*, const BTL_POKEPARAM*, void* );


/**
 *  経験値計算用ワーク
 */
typedef struct {
  u32  exp;       ///< もらえる経験値
  u8   fBonus;    ///< 多めにもらえたフラグ

  u8   hp;        ///< HP努力値
  u8   pow;       ///< こうげき努力値
  u8   def;       ///< ぼうぎょ努力値
  u8   agi;       ///< すばやさ努力値
  u8   sp_pow;    ///< とくこう努力値
  u8   sp_def;    ///< とくぼう努力値

}CALC_EXP_WORK;


/**
 *  ワザ相性カウンタ（バトル検定用）
 */
typedef struct {
  u16 putVoid;          ///< 「効果なし」を出した
  u16 putAdvantage;     ///< 「効果ばつぐん」を出した
  u16 putDisadvantage;  ///< 「効果いまいち」を出した
  u16 recvVoid;         ///< 「効果無し」を受けた
  u16 recvAdvantage;    ///< 「効果ばつぐん」を受けた
  u16 recvDisadvantage; ///< 「効果いまいち」を受けた
}WAZA_AFF_COUNTER;


/**
 *  HandEx 戻り値
 */
typedef enum {
  HandExResult_NULL,    ///< 何も反応なし
  HandExResult_Fail,    ///< 反応あったが、効果は無かった
  HandExResult_Enable,  ///< 反応もあり、効果もあった
}HandExResult;

/**
 *  ClientID 記録
 */
typedef struct {
  u8  count;
  u8  clientID[ BTL_CLIENT_MAX ];
}CLIENTID_REC;



//-----------------------------------------------------
/**
 *  メインワーク
 */
//-----------------------------------------------------
struct _BTL_SVFLOW_WORK {

  BTL_SERVER*             server;
  const BTL_MAIN_MODULE*  mainModule;
  BTL_POKE_CONTAINER*     pokeCon;
  BTL_SERVER_CMD_QUE*     que;
  u32                     turnCount;
  SvflowResult            flowResult;
  BtlBagMode              bagMode;
  BTL_WAZAREC             wazaRec;
  BTL_DEADREC             deadRec;
  ARCHANDLE*              sinkaArcHandle;
  WAZAEFF_CTRL*           wazaEffCtrl;
  WAZA_ROB_PARAM*         wazaRobParam;
  WAZA_ROB_PARAM*         magicCoatParam;
  HITCHECK_PARAM*         hitCheckParam;
  BTL_ESCAPEINFO          escInfo;
  BTL_LEVELUP_INFO        lvupInfo;
  CLIENTID_REC            clientIDRec;
  u32                     simulationCounter;
  u8                      cmdBuildStep;
  u8                      actOrderStep;
  u8                      turnCheckStep;
  u8                      defaultTargetPos;

  HEAPID  heapID;

  u8      numActOrder;
  u8      numEndActOrder;
  u8      getPokePos;
  u8      numRelivePoke;
  u8      nigeruCount;
  u8      wazaEffIdx;
  u8      MemberOutIntrPokeCount;
  u8      thruDeadMsgPokeID;
  u8      fMemberOutIntr     : 1;
  u8      fWinBGMPlayWild    : 1;
  u8      fEscMsgDisped      : 1;
  u8      fMemberChangeReserve : 1;

  u8      MemberOutIntrPokeID[ BTL_POS_MAX ];
  u8      relivedPokeID[ BTL_POKEID_MAX ];
  u8      pokeDeadFlag[ BTL_POKEID_MAX ];
  u8      pokeInFlag [ BTL_POKEID_MAX ];
  u8      memberChangeCount[ BTL_CLIENT_MAX ];

  ACTION_ORDER_WORK   actOrder[ BTL_POS_MAX ];
  ACTION_ORDER_WORK   actOrderTmp;

  // POKESET の STACK機構用（割り込みが発生するので actOrder 件数+1 分必要）
  BTL_POKESET*   psetTargetOrg;
  BTL_POKESET*   psetTarget;
  BTL_POKESET*   psetFriend;
  BTL_POKESET*   psetEnemy;
  BTL_POKESET*   psetDamaged;
  BTL_POKESET*   psetRobTarget;
  BTL_POKESET*   psetTmp;
  BTL_CALC_DAMAGE_REC* calcDmgFriend;
  BTL_CALC_DAMAGE_REC* calcDmgEnemy;
  POKESET_STACK_UNIT  pokesetUnit[ BTL_POS_MAX+1 ];
  u32            pokesetStackPtr;


  BTL_POKESET         pokesetMemberInProc;

  SVFL_WAZAPARAM         *wazaParam;
  SVFL_WAZAPARAM         *wazaParamOrg;
  BTL_POSPOKE_WORK       pospokeWork;
  BTL_HANDEX_STR_PARAMS  strParam;
  CALC_EXP_WORK          calcExpWork[ BTL_PARTY_MEMBER_MAX ];

  BTL_EVENT_WORK_STACK        eventWork;
  HANDLER_EXHIBISION_MANAGER  HEManager;
  BPP_WAZADMG_REC             wazaDamageRec;
  WazaID                      prevExeWaza;
  SabotageType                currentSabotageType;
  WAZA_AFF_COUNTER            affCounter;
  BTL_DMGAFF_REC              dmgAffRec;

  u8          flowFlags[ FLOWFLG_BYTE_MAX ];
  u8          handlerTmpWork[ EVENT_HANDLER_TMP_WORK_SIZE ];
};



extern HandExResult BTL_SVF_HandEx_Result( BTL_SVFLOW_WORK* wk );
