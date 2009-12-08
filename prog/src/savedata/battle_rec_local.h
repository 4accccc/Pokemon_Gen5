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


///データナンバーの型
typedef u64 DATA_NUMBER;

///データナンバーのバイトサイズ
#define DATANUMBER_SIZE   (sizeof(DATA_NUMBER))

///録画ヘッダに格納するポケモン番号の最大要素数
#define HEADER_MONSNO_MAX   (12)

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

  u8 work[336];   ///< @todo 内容未定

};

/**
  *  クライアント操作内容保存バッファ
 */
struct _BTLREC_OPERATION_BUFFER {

  u8  buffer[ BTLREC_OPERATION_BUFFER_SIZE ];

};

//----------------------------------------------------------
/**
 *  録画セーブデータ本体（??? bytes）
 */
//----------------------------------------------------------
typedef struct _BATTLE_REC_WORK {
  BTLREC_SETUP_SUBSET       setupSubset;   ///< バトル画面セットアップパラメータのサブセット
  BTLREC_OPERATION_BUFFER   opBuffer;      ///< クライアント操作内容の保存バッファ

  REC_POKEPARTY     rec_party[BTL_CLIENT_MAX];
  MYSTATUS          my_status[BTL_CLIENT_MAX];
  CONFIG            config;
  u16 magic_key;
  u16 padding;

  //CRC(必ず最後尾にしておくこと)
  GDS_CRC       crc;
}BATTLE_REC_WORK;

//--------------------------------------------------------------
/**
 *  戦闘録画のヘッダ
 */
//--------------------------------------------------------------
typedef struct _BATTLE_REC_HEADER{
  u16 monsno[HEADER_MONSNO_MAX];  ///< ポケモン番号(表示する必要がないのでタマゴの場合は0)
  u8  form_no[HEADER_MONSNO_MAX]; ///< ポケモンのフォルム番号

  u16 battle_counter; ///< 連勝数
  u8  mode;           ///< 戦闘モード(ファクトリー50、ファクトリー100、通信対戦...)
  u8  secure;         ///< TRUE:安全が保障されている。　FALSE：再生した事がない

  AAA_REGULATION regulation;      ///<レギュレーションデータ    32

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
