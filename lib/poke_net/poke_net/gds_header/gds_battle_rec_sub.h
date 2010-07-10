//==============================================================================
/**
 *
 * 戦闘録画のサブ構造体や定数の定義など
 *
 */
//==============================================================================
#ifndef __GT_GDS_BATTLE_REC_SUB_H__
#define __GT_GDS_BATTLE_REC_SUB_H__


//==============================================================================
//  定数定義
//==============================================================================
///録画データの存在有無
#define REC_OCC_MAGIC_KEY   (0xe281)

///録画ヘッダに格納するポケモン番号の最大要素数
#define HEADER_MONSNO_MAX   (12)

#define WAZA_TEMOTI_MAX   (4)   ///<1体のポケモンがもてる技の最大値

#define PERSON_NAME_SIZE  7 // 人物の名前の長さ（自分も含む）
#define MONS_NAME_SIZE    10  // ポケモン名の長さ(バッファサイズ EOM_含まず)
#define EOM_SIZE      1 // 終了コードの長さ

#define TEMOTI_POKEMAX    6 ///< 手持ちポケモン最大数

enum {
  BTL_CLIENT_MAX = 4,       ///< 稼働するクライアント数
};

enum {
  BTLREC_OPERATION_BUFFER_SIZE = 0xc00,   ///< 全クライアントの操作内容保持バッファサイズ
};

typedef u16 ZONEID; ///<ZONEID

//--------------------------------------------------------------
/**
 *  天候
 */
//--------------------------------------------------------------
typedef enum {

  BTL_WEATHER_NONE = 0,   ///< 天候なし

  BTL_WEATHER_SHINE,    ///< はれ
  BTL_WEATHER_RAIN,     ///< あめ
  BTL_WEATHER_SNOW,     ///< あられ
  BTL_WEATHER_SAND,     ///< すなあらし

  BTL_WEATHER_MAX,

}BtlWeather_tag;

typedef u8 BtlWeather;

//--------------------------------------------------------------
/**
 *  戦闘背景指定タイプID
 */
//--------------------------------------------------------------
typedef enum {
  BATTLE_BG_TYPE_GRASS,         ///< 草むら
  BATTLE_BG_TYPE_GRASS_SEASON,  ///< 草むら(四季有り)
  BATTLE_BG_TYPE_CITY,          ///< 街
  BATTLE_BG_TYPE_CITY_SEASON,   ///< 街(四季有り)
  BATTLE_BG_TYPE_CAVE,          ///< 洞窟
  BATTLE_BG_TYPE_CAVE_DARK,     ///< 洞窟(暗い)
  BATTLE_BG_TYPE_FOREST,        ///< 森
  BATTLE_BG_TYPE_MOUNTAIN,      ///< 山
  BATTLE_BG_TYPE_SEA,           ///< 海
  BATTLE_BG_TYPE_ROOM,          ///< 室内
  BATTLE_BG_TYPE_SAND,          ///< 砂漠
   
  BATTLE_BG_TYPE_MAX,
}BtlBgType;

//--------------------------------------------------------------
/*
 *  戦闘背景指定アトリビュートID
 */
//--------------------------------------------------------------
typedef enum {
  //芝生
  BATTLE_BG_ATTR_LAWN,
  //通常地面
  BATTLE_BG_ATTR_NORMAL_GROUND,
  //地面１
  BATTLE_BG_ATTR_GROUND1,
  //地面２
  BATTLE_BG_ATTR_GROUND2,
  //草
  BATTLE_BG_ATTR_GRASS,
  //水上
  BATTLE_BG_ATTR_WATER,
  //雪原
  BATTLE_BG_ATTR_SNOW,
  //砂地
  BATTLE_BG_ATTR_SAND,
  //浅い湿原
  BATTLE_BG_ATTR_MARSH,
  //洞窟
  BATTLE_BG_ATTR_CAVE,
  //水たまり
  BATTLE_BG_ATTR_POOL,
  //浅瀬
  BATTLE_BG_ATTR_SHOAL,
  //氷上
  BATTLE_BG_ATTR_ICE,

  BATTLE_BG_ATTR_MAX,
}BtlBgAttr;


//--------------------------------------------------------------
//  バトルモード
//--------------------------------------------------------------
// 「シングル」「ダブル」「トリプル」「ローテーション」「マルチ」
#define BATTLEMODE_TYPE_BIT 0

//「通信」か「サブウェイ」か。
//通信・サブウェイ、どちらのランキングに含めるかは、こちらを利用します。
#define BATTLEMODE_COMMUNICATION_BIT 3

//「通信」の場合は通信のタイプ、「サブウェイ」の場合は０
#define BATTLEMODE_COMMMODE_BIT 4

// 「ランダムマッチ」の場合、「フリー」か「レーティング戦」か
#define BATTLEMODE_RATING_BIT 6

// シューターが有か無しか
#define BATTLEMODE_SHOOTER_BIT 7

// レベルレギュレーション（フリー、スタンダード、レベル５０）
#define BATTLEMODE_REGULATION_BIT 8

typedef enum{
	BATTLEMODE_TYPE_SINGLE = 0,
	BATTLEMODE_TYPE_DOUBLE,
	BATTLEMODE_TYPE_TRIPLE,
	BATTLEMODE_TYPE_ROTATION,
	BATTLEMODE_TYPE_MULTI,
} BATTLEMODE_TYPE;

typedef enum{
	BATTLEMODE_COMMUNICATION_SUBWAY = 0,
	BATTLEMODE_COMMUNICATION_COMMUNICATION = 1,
} BATTLEMODE_COMMUNICATION;

typedef enum{
	BATTLEMODE_COMMMODE_SUBWAY = 0,
	BATTLEMODE_COMMMODE_COLOSSEUM,
	BATTLEMODE_COMMMODE_RANDOM,
	BATTLEMODE_COMMMODE_CHAMPIONSHIP,
} BATTLEMODE_COMMMODE;

typedef enum {
	BATTLEMODE_RATING_FREE = 0,
	BATTLEMODE_RATING_RATING,
} BATTLEMODE_RATING;

typedef enum {
	BATTLEMODE_SHOOTER_OFF = 0,
	BATTLEMODE_SHOOTER_ON,
} BATTLEMODE_SHOOTER;

typedef enum {
	BATTLEMODE_REGULATION_FREE = 0,
	BATTLEMODE_REGULATION_STANDARD,
	BATTLEMODE_REGULATION_LEVEL50,
} BATTLEMODE_REGULATION;

#define BATTLEMODE_TYPE_MASK 0x7
#define BATTLEMODE_COMMUNICATION_MASK 0x8

//「通信」の場合は通信のタイプ、「サブウェイ」の場合は０
#define BATTLEMODE_COMMMODE_MASK 0x30

// 「ランダムマッチ」の場合、「フリー」か「レーティング戦」か
#define BATTLEMODE_RATING_MASK 0x40

// シューターが有か無しか
#define BATTLEMODE_SHOOTER_MASK 0x80

// レベルレギュレーション（フリー、スタンダード、レベル５０）
#define BATTLEMODE_REGULATION_MASK 0x300


//--------------------------------------------------------------
//  検索条件”指定なし”の定義
//--------------------------------------------------------------
///ポケモン指定無し
#define BATTLE_REC_SEARCH_MONSNO_NONE     (0xffff)
///国コード指定無し
#define BATTLE_REC_SEARCH_COUNTRY_CODE_NONE   (0xff)
///地方コード指定無し
#define BATTLE_REC_SEARCH_LOCAL_CODE_NONE   (0xff)

//--------------------------------------------------------------
//  戦闘モード(施設一覧)
//--------------------------------------------------------------
///バトルモード   ※BattleRecModeBitTblと並びを同じにしておくこと！！
typedef enum{
  //コロシアム：シングル
  BATTLE_MODE_COLOSSEUM_SINGLE_FREE,    //コロシアム　シングル　制限無し
  BATTLE_MODE_COLOSSEUM_SINGLE_50,      //コロシアム　シングル　フラット
  BATTLE_MODE_COLOSSEUM_SINGLE_FREE_SHOOTER,  //コロシアム　シングル　制限無し　シューターあり
  BATTLE_MODE_COLOSSEUM_SINGLE_50_SHOOTER,    //コロシアム　シングル　フラット　シューターあり
  //コロシアム：ダブル
  BATTLE_MODE_COLOSSEUM_DOUBLE_FREE,          //コロシアム　ダブル　制限無し
  BATTLE_MODE_COLOSSEUM_DOUBLE_50,            //コロシアム　ダブル　フラット
  BATTLE_MODE_COLOSSEUM_DOUBLE_FREE_SHOOTER,  //コロシアム　ダブル　制限無し　シューターあり
  BATTLE_MODE_COLOSSEUM_DOUBLE_50_SHOOTER,    //コロシアム　ダブル　フラット　シューターあり
  //コロシアム：トリプル
  BATTLE_MODE_COLOSSEUM_TRIPLE_FREE,          //コロシアム　トリプル　制限無し
  BATTLE_MODE_COLOSSEUM_TRIPLE_50,            //コロシアム　トリプル　フラット
  BATTLE_MODE_COLOSSEUM_TRIPLE_FREE_SHOOTER,  //コロシアム　トリプル　制限無し　シューターあり
  BATTLE_MODE_COLOSSEUM_TRIPLE_50_SHOOTER,    //コロシアム　トリプル　フラット　シューターあり
  //コロシアム：ローテーション
  BATTLE_MODE_COLOSSEUM_ROTATION_FREE,        //コロシアム　ローテーション　制限無し
  BATTLE_MODE_COLOSSEUM_ROTATION_50,          //コロシアム　ローテーション　フラット
  BATTLE_MODE_COLOSSEUM_ROTATION_FREE_SHOOTER,  //コロシアム　ローテ　制限無し　シューターあり
  BATTLE_MODE_COLOSSEUM_ROTATION_50_SHOOTER,    //コロシアム　ローテ　フラット　シューターあり
  //コロシアム：マルチ
  BATTLE_MODE_COLOSSEUM_MULTI_FREE,           //コロシアム　マルチ　制限無し
  BATTLE_MODE_COLOSSEUM_MULTI_50,             //コロシアム　マルチ　フラット
  BATTLE_MODE_COLOSSEUM_MULTI_FREE_SHOOTER,   //コロシアム　マルチ　制限無し　シューターあり
  BATTLE_MODE_COLOSSEUM_MULTI_50_SHOOTER,     //コロシアム　マルチ　フラット　シューターあり
  //地下鉄
  BATTLE_MODE_SUBWAY_SINGLE,     //WIFI DL含む      地下鉄　シングル
  BATTLE_MODE_SUBWAY_DOUBLE,                      //地下鉄　ダブル
  BATTLE_MODE_SUBWAY_MULTI,      //NPC, COMM, WIFI  地下鉄　マルチ
  //ランダムマッチ：フリー
  BATTLE_MODE_RANDOM_FREE_SINGLE,             //ランダムマッチ　フリー　シングル
  BATTLE_MODE_RANDOM_FREE_DOUBLE,             //ランダムマッチ　フリー　ダブル
  BATTLE_MODE_RANDOM_FREE_TRIPLE,             //ランダムマッチ　フリー　トリプル
  BATTLE_MODE_RANDOM_FREE_ROTATION,           //ランダムマッチ　フリー　ローテーション
  BATTLE_MODE_RANDOM_FREE_SHOOTER,            //ランダムマッチ　フリー　シューターバトル
  //ランダムマッチ：レーティング
  BATTLE_MODE_RANDOM_RATING_SINGLE,           //ランダムマッチ　レーティング　シングル
  BATTLE_MODE_RANDOM_RATING_DOUBLE,           //ランダムマッチ　レーティング　ダブル
  BATTLE_MODE_RANDOM_RATING_TRIPLE,           //ランダムマッチ　レーティング　トリプル
  BATTLE_MODE_RANDOM_RATING_ROTATION,         //ランダムマッチ　レーティング　ローテーション
  BATTLE_MODE_RANDOM_RATING_SHOOTER,          //ランダムマッチ　レーティング　シューターバトル
  //大会
  BATTLE_MODE_COMPETITION_SINGLE,             //大会バトル　シングル　シューター無し
  BATTLE_MODE_COMPETITION_SINGLE_SHOOTER,     //大会バトル　シングル　シューターあり
  BATTLE_MODE_COMPETITION_DOUBLE,             //大会バトル　ダブル　シューター無し
  BATTLE_MODE_COMPETITION_DOUBLE_SHOOTER,     //大会バトル　ダブル　シューターあり
  BATTLE_MODE_COMPETITION_TRIPLE,             //大会バトル　トリプル　シューター無し
  BATTLE_MODE_COMPETITION_TRIPLE_SHOOTER,     //大会バトル　トリプル　シューターあり
  BATTLE_MODE_COMPETITION_ROTATION,           //大会バトル　ローテーション　シューター無し
  BATTLE_MODE_COMPETITION_ROTATION_SHOOTER,   //大会バトル　ローテーション　シューターあり
}BATTLE_MODE;

//終端
#define BATTLE_MODE_MAX           (BATTLE_MODE_COMPETITION_ROTATION_SHOOTER + 1)

//==============================================================================
//  型定義
//==============================================================================
///データナンバーの型
typedef u64 DATA_NUMBER;


//----------------------------------------------------------
/**
 * @brief 自分状態データ型定義
 */
//----------------------------------------------------------
typedef struct {
  STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];    // 16
  u32 id;               // 20 PlayerID
  u32 profileID;        // 24
  u8 nation;            ///< 国  25
  u8 area;              ///< 地域 26
  u8 region_code;       //PM_LANG   27
  u8 rom_code;          // PM_VERSION   28
  u8 trainer_view;  // ユニオンルーム内での見た目フラグ29
  u8 sex;            //性別 30
  u8 dummy2;                    // 31
  u8 dummy3;                    // 32
}MYSTATUS;


//--------------------------------------------------------------
/**
 * 対戦録画用にカスタマイズされたポケモンパラメータ
 * 112byte
 */
//--------------------------------------------------------------
typedef struct{
  u32 personal_rnd;             //04h 個性乱数
  u16 pp_fast_mode    :1;         //06h 暗号／復号／チェックサム生成を後回しにして、処理を高速化モード
  u16 ppp_fast_mode   :1;         //06h 暗号／復号／チェックサム生成を後回しにして、処理を高速化モード
  u16 fusei_tamago_flag :1;         //06h ダメタマゴフラグ
  u16 seikaku:8;                  // 性格
  u16           :5;        //06h
  
  u16 monsno;               //02h モンスターナンバー    
  u16 item;               //04h 所持アイテムナンバー  
  u32 id_no;                //08h IDNo
  u32 exp;                //0ch 経験値
  u8  friendshipness;           //0dh なつき度
  u8  speabino;             //0eh 特殊能力
  u8  hp_exp;               //11h HP努力値
  u8  pow_exp;              //12h 攻撃力努力値
  u8  def_exp;              //13h 防御力努力値
  u8  agi_exp;              //14h 素早さ努力値
  u8  spepow_exp;             //15h 特攻努力値
  u8  spedef_exp;             //16h 特防努力値

  u16 waza[WAZA_TEMOTI_MAX];        //08h 所持技
  u8  pp[WAZA_TEMOTI_MAX];        //0ch 所持技PP
  u8  pp_count[WAZA_TEMOTI_MAX];      //10h 所持技PP_COUNT
  u32 hp_rnd      :5;         //    HP乱数
  u32 pow_rnd     :5;         //    攻撃力乱数
  u32 def_rnd     :5;         //    防御力乱数
  u32 agi_rnd     :5;         //    素早さ乱数
  u32 spepow_rnd    :5;         //    特攻乱数
  u32 spedef_rnd    :5;         //    特防乱数
  u32 tamago_flag   :1;         //    タマゴフラグ（0:タマゴじゃない　1:タマゴだよ）
  u32 nickname_flag :1;         //14h ニックネームをつけたかどうかフラグ
  u8  event_get_flag  :1;         //    イベントで配布したことを示すフラグ
  u8  sex       :2;         //    ポケモンの性別
  u8  form_no     :5;         //19h 形状ナンバー
  
  STRCODE nickname[MONS_NAME_SIZE+EOM_SIZE];  //16h ニックネーム
  
  STRCODE oyaname[PERSON_NAME_SIZE+EOM_SIZE]; //10h 親の名前
  u8  get_ball;               //1ch 捕まえたボール
  u8  language;               // 言語コード
  
  u32     condition;              //04h コンディション
  u8      level;                //05h レベル
  u8      cb_id;                //06h カスタムボールID
  u16     hp;                 //08h HP
  u16     hpmax;                //0ah HPMAX
  u16     pow;                //0ch 攻撃力
  u16     def;                //0eh 防御力
  u16     agi;                //10h 素早さ
  u16     spepow;               //12h 特攻
  u16     spedef;               //14h 特防
  
} REC_POKEPARA;

//--------------------------------------------------------------
/**
 *  戦闘録画用にカスタマイズされたPOKEPARTY
 *  676 byte
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

//---------------------------------------------------------------------------
/**
 * @brief 設定データ構造体定義
 *      4byte
 */
//---------------------------------------------------------------------------
typedef struct {
  u16 msg_speed:4;      ///<MSGSPEED    ＭＳＧ送りの速度
  u16 sound_mode:2;     ///<SOUNDMODE   サウンド出力
  u16 battle_rule:1;      ///<BATTLERULE    戦闘ルール
  u16 wazaeff_mode:1;     ///<WAZAEFF_MODE  わざエフェクト
  u16 moji_mode:1;      ///<MOJIMODE
  u16 wirelesssave_mode:1;      ///<WIRELESSSAVE_MODE   ワイヤレスでセーブをはさむかどうか
  u16 network_search:1;         ///<NETWORK_SEARCH_MODE ゲーム中にビーコンサーチするかどうか

  u16 padding1:5;
  u16 padding2;
}CONFIG;


//-----------------------------------------------------------------------------------
/**
 * フィールドの状態から決定されるバトルシチュエーションデータ
 */
//-----------------------------------------------------------------------------------
typedef struct {

  BtlBgType   bgType;
  BtlBgAttr   bgAttr;
  BtlWeather  weather;
  u8          season;

  // ライト設定用パラメータ
  ZONEID      zoneID;
  u8          hour;
  u8          minute;

}BTL_FIELD_SITUATION;

/**
  *  バトル画面セットアップパラメータの録画データ用サブセット
 */
typedef struct _BTLREC_SETUP_SUBSET {
  GFL_STD_RandContext   randomContext;    ///< 乱数のタネ
  BTL_FIELD_SITUATION   fieldSituation;   ///< 背景・お盆などの構築用情報
  CONFIG                config;           ///< ユーザーコンフィグ
  u16  musicDefault;            ///< デフォルトBGM
  u16  musicPinch;              ///< ピンチ時BGM
  u16  debugFlagBit;            ///< デバッグ機能Bitフラグ -> enum BtlDebugFlag @ battle/battle.h
  u8   competitor : 5;          ///< 対戦者タイプ（ゲーム内トレーナー、通信対戦）-> enum BtlCompetitor @ battle/battle.h
  u8   myCommPos  : 3;          ///< 通信対戦時の自分の立ち位置（マルチの時、0,2 vs 1,3 になり、0,1が左側／2,3が右側になる）
  u8   rule       : 4;          ///< ルール（シングル・ダブル・トリプル・ローテ）-> enum BtlRule @ battle/battle.h
  u8   MultiMode :3;            ///< マルチバトルフラグ（ルールは必ずダブル）
  u8   shooterBit:1;

}BTLREC_SETUP_SUBSET;

/**
  *  クライアント操作内容保存バッファ
 */
typedef struct {
  u32  size;
  u8   buffer[ BTLREC_OPERATION_BUFFER_SIZE ];
}BTLREC_OPERATION_BUFFER;

typedef enum {
  BTLREC_CLIENTSTATUS_NONE,
  BTLREC_CLIENTSTATUS_PLAYER,
  BTLREC_CLIENTSTATUS_TRAINER,
}BtlRecClientStatusType;

enum {
  BSP_TRAINERDATA_ITEM_MAX = 4,
};

#define BUFLEN_PERSON_NAME        (16)  // 人物名（プレイヤー含む）（海外14文字まで+EOM）

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

typedef struct {
  u16                       type;         ///< BtlRecClientStatusType
  u16                       voiceLevel;   ///< ペラップボイスレベル
  union {
    MYSTATUS             player;
    BTLREC_TRAINER_STATUS   trainer;
  };
}BTLREC_CLIENT_STATUS;


//----------------------------------------------------------
/**
 *  録画セーブデータ本体（6116byte）
 */
//----------------------------------------------------------
typedef struct {
  BTLREC_SETUP_SUBSET       setupSubset;   ///< バトル画面セットアップパラメータのサブセット
  BTLREC_OPERATION_BUFFER   opBuffer;      ///< クライアント操作内容の保存バッファ

  REC_POKEPARTY         rec_party[ BTL_CLIENT_MAX ];
  BTLREC_CLIENT_STATUS  clientStatus[ BTL_CLIENT_MAX ];

  u16 magic_key;
  u16 padding;
  
  //u32         checksum; //チェックサム  4byte
  GDS_CRC crc;              ///< CRC            4
}BATTLE_REC_WORK;

//--------------------------------------------------------------
/**
 *  戦闘録画のヘッダ
 *    68byte
 */
//--------------------------------------------------------------
typedef struct {
  u16 monsno[HEADER_MONSNO_MAX];  ///<ポケモン番号(表示する必要がないのでタマゴの場合は0) 24
  u8 form_no_and_sex[HEADER_MONSNO_MAX];  ///<6..0bit目：ポケモンのフォルム番号　7bit目：ポケモンの性別		12

  u16 battle_counter;   ///<連勝数                        2
  u16 mode;        ///<戦闘モード(ファクトリー50、ファクトリー100、通信対戦...)
  
  u16 magic_key;      ///<マジックキー
  u8 secure;        ///<TRUE:安全が保障されている。　FALSE：再生した事がない
  
  u8 server_vesion;   ///<バトルのサーバーバージョン
  u8 work[12];      ///< 予備
  
  DATA_NUMBER data_number;    ///<データナンバー(サーバー側でセットされる)  8
  GDS_CRC crc;              ///< CRC            4
}BATTLE_REC_HEADER;


#endif  //__GT_GDS_BATTLE_REC_SUB_H__

