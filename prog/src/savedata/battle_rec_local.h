//==============================================================================
/**
 * @file  battle_rec_local.h
 * @brief 戦闘録画ローカルヘッダ
 * @author  matsuda
 * @date  2009.11.18(水)
 */
//==============================================================================
#ifndef __BATTLE_REC_LOCAL_H__
#define __BATTLE_REC_LOCAL_H__


//----------------------------------------------------------------------
//  includes
//----------------------------------------------------------------------
#include "gds_local_common.h"

///データナンバーのバイトサイズ
#define DATANUMBER_SIZE   (sizeof(DATA_NUMBER))


///録画データの存在有無
#define REC_OCC_MAGIC_KEY   (0xe281)


//--------------------------------------------------------------
/**
 *  戦闘録画用にカスタマイズされたPOKEPARTY
 */
//--------------------------------------------------------------
typedef struct{
  /// 保持できるポケモン数の最大
  u16 PokeCountMax;
  /// 現在保持しているポケモン数
  u16 PokeCount;
  /// ポケモンデータ
  REC_POKEPARA member[TEMOTI_POKEMAX];
}REC_POKEPARTY;

/**
  *  バトル画面セットアップパラメータの録画データ用サブセット
 */
struct _BTLREC_SETUP_SUBSET {

  GFL_STD_RandContext   randomContext;    ///< 乱数のタネ
  BTL_FIELD_SITUATION   fieldSituation;   ///< 背景・お盆などの構築用情報
  CONFIG                config;           ///< ユーザーコンフィグ
  u16  musicDefault;            ///< デフォルトBGM
  u16  musicPinch;              ///< ピンチ時BGM
  u16  debugFlagBit;            ///< デバッグ機能Bitフラグ -> enum BtlDebugFlag @ battle/battle.h
  u8   competitor : 5;          ///< 対戦者タイプ（ゲーム内トレーナー、通信対戦）-> enum BtlCompetitor @ battle/battle.h
  u8   myCommPos  : 3;          ///< 通信対戦時の自分の立ち位置（マルチの時、0,2 vs 1,3 になり、0,1が左側／2,3が右側になる）
  u8   rule       : 7;          ///< ルール（シングル・ダブル・トリプル・ローテ）-> enum BtlRule @ battle/battle.h
  u8   fMultiMode :1;           ///< マルチバトルフラグ（ルールは必ずダブル）


};

/**
  *  録画セーブ用トレーナーデータ
 */
typedef struct {
  u32     ai_bit;
  u16     tr_id;
  u16     tr_type;
  u16     use_item[ BSP_TRAINERDATA_ITEM_MAX ]; // 使用するアイテムu16x4 = 8byte
  /// 16 byte

  STRCODE name[BUFLEN_PERSON_NAME];  // トレーナー名<-32byte

  PMS_DATA  win_word;   //戦闘終了時勝利メッセージ <-8byte
  PMS_DATA  lose_word;  //戦闘終了時負けメッセージ <-8byte

}BTLREC_TRAINER_STATUS;


typedef enum {
  BTLREC_CLIENTSTATUS_NONE,
  BTLREC_CLIENTSTATUS_PLAYER,
  BTLREC_CLIENTSTATUS_TRAINER,
}BtlRecClientStatusType;

typedef struct {
  BtlRecClientStatusType    type;
  union {
    MYSTATUS                player;
    BTLREC_TRAINER_STATUS   trainer;
  };
}BTLREC_CLIENT_STATUS;

/**
  *  クライアント操作内容保存バッファ
 */
struct _BTLREC_OPERATION_BUFFER {
  u32  size;
  u8   buffer[ BTLREC_OPERATION_BUFFER_SIZE ];
};

//----------------------------------------------------------
/**
 *  録画セーブデータ本体（6116 bytes）
 */
//----------------------------------------------------------
typedef struct _BATTLE_REC_WORK {
  BTLREC_SETUP_SUBSET       setupSubset;   ///< バトル画面セットアップパラメータのサブセット
  BTLREC_OPERATION_BUFFER   opBuffer;      ///< クライアント操作内容の保存バッファ

  REC_POKEPARTY         rec_party[ BTL_CLIENT_MAX ];
  BTLREC_CLIENT_STATUS  clientStatus[ BTL_CLIENT_MAX ];

  u16 magic_key;
  u16 padding;

  //CRC(必ず最後尾にしておくこと)
  GDS_CRC       crc;
}BATTLE_REC_WORK;

//--------------------------------------------------------------
/**
 *  戦闘録画のヘッダ  (68 byte)
 */
//--------------------------------------------------------------
typedef struct _BATTLE_REC_HEADER{
  u16 monsno[HEADER_MONSNO_MAX];  ///< ポケモン番号(表示する必要がないのでタマゴの場合は0)
  u8  form_no[HEADER_MONSNO_MAX]; ///< ポケモンのフォルム番号

  u16 battle_counter; ///< 連勝数
  u8  mode;           ///< 戦闘モード(ファクトリー50、ファクトリー100、通信対戦...)
  u8  secure;         ///< TRUE:安全が保障されている。　FALSE：再生した事がない

  u16 magic_key;      ///<マジックキー
  u8  work[14];      ///< 予備                    16

  //CRCとデータナンバー(必ず最後尾にしておくこと)
  DATA_NUMBER   data_number;///<データナンバー(サーバー側でセットされる)。チェックサム対象から外す
  GDS_CRC       crc;
}BATTLE_REC_HEADER;

//--------------------------------------------------------------
/**
 *  録画セーブデータ：GDSプロフィール＋ヘッダ＋本体
 */
//--------------------------------------------------------------
struct _BATTLE_REC_SAVEDATA{
  //殿堂入り以外の外部セーブは必ず一番最初にEX_SAVE_KEY構造体を配置しておくこと
  EX_CERTIFY_SAVE_KEY save_key;     ///<認証キー

  GDS_PROFILE profile;      ///<GDSプロフィール
  BATTLE_REC_HEADER head;     ///<戦闘録画ヘッダ
  BATTLE_REC_WORK rec;      ///<戦闘録画本体
};


#endif  //__BATTLE_REC_LOCAL_H__
