//==============================================================================
/**
 * @file    mission_types.h
 * @brief   ミッション関連の定義
 * @author  matsuda
 * @date    2009.10.27(火)
 */
//==============================================================================
#pragma once


//==============================================================================
//  定数定義
//==============================================================================
///ミッション確認タイプ
typedef enum{
  CONFIRM_NUM,
  CONFIRM_PEOPLE,
}CONFIRM_TYPE;

///ミッション系統
typedef enum{
  MISSION_TYPE_VICTORY,     ///<勝利(LV)
  MISSION_TYPE_SKILL,       ///<技
  MISSION_TYPE_BASIC,       ///<基礎
  MISSION_TYPE_SIZE,        ///<大きさ
  MISSION_TYPE_ATTRIBUTE,   ///<属性
  MISSION_TYPE_ITEM,        ///<道具
  MISSION_TYPE_OCCUR,       ///<発生(エンカウント)
  MISSION_TYPE_PERSONALITY, ///<性格
  
  MISSION_TYPE_MAX,
  MISSION_TYPE_NONE = MISSION_TYPE_MAX, ///<ミッション実行していない
}MISSION_TYPE;

///「ミッション受注します」の返事
typedef enum{
  MISSION_ENTRY_RESULT_NULL,    ///<返事なし
  MISSION_ENTRY_RESULT_OK,      ///<受注OK
  MISSION_ENTRY_RESULT_NG,      ///<受注NG
}MISSION_ENTRY_RESULT;

///ミッション達成宣言の返事
typedef enum{
  MISSION_ACHIEVE_NULL,       ///<返事無し
  MISSION_ACHIEVE_OK,         ///<達成OK
  MISSION_ACHIEVE_NG,         ///<達成NG
}MISSION_ACHIEVE;

///会話タイプ
typedef enum{
  TALK_TYPE_NORMAL,    ///<主人公の性別に依存(MAN or WOMANに分岐)
  TALK_TYPE_MAN,       ///<主人公：男
  TALK_TYPE_WOMAN,     ///<主人公：女
  TALK_TYPE_PIKACHU,   ///<ピカチュウ
}TALK_TYPE;

//==============================================================================
//  コンバータから出力されるデータの構造体
//==============================================================================
///ミッションデータ
typedef struct{
  u8 mission_no;                  ///<ミッション番号
  u8 type;                        ///<ミッション系統(MISSION_TYPE_???)
  u8 level;                       ///<発生レベル
  u8 odds;                        ///<発生率

  u16 msg_id_contents;            ///<ミッション内容gmmのmsg_id
  u16 msg_id_contents_monolith;   ///<ミッション内容モノリスgmmのmsg_id

  u16 obj_id[4];      ///<変化OBJ ID

  u16 data[3];        ///<データ(ミッション系統毎に扱いが変化)
  u16 time;           ///<時間(秒)

  u16 reward[4];      ///<報酬 0(1位) ～ 3(4位)

  u8 confirm_type;    ///<ミッション確認タイプ (CONFIRM_???)
  u8 limit_run;       ///<制限走り　TRUE:走り禁止
  u8 limit_talk;      ///<制限話　TRUE:話禁止
  u8 padding;
}MISSION_CONV_DATA;

///パワーデータ
typedef struct{
  u8 no;                  ///<パワー番号
  u8 level_w;             ///<必要Whiteレベル
  u8 level_b;             ///<必要Blackレベル
  u8 time;                ///<継続時間(分)

  u16 point;              ///<使用ポイント
  u16 msg_id_title;       ///<パワー項目msg_id

  u16 msg_id_explain;     ///<パワー説明msg_id
  u8 type;                ///<効果系統
  u8 data;                ///<効果データ
}POWER_CONV_DATA;

//--------------------------------------------------------------
//  MISSION_CONV_DATA.data部分の構造体
//--------------------------------------------------------------
///ミッション系統毎に異なるdataの構造体：VICTORY
typedef struct{
  u16 disguise_objcode;   ///<変装OBJCODE
  u16 battle_level;       ///<対戦レベル
  u16 padding;
}MISSION_TYPEDATA_VICTORY;

///ミッション系統毎に異なるdataの構造体：SKILL
typedef struct{
  u16 pms_id;            ///<簡易会話用単語ID
  u16 num;               ///<回数
  u16 padding;
}MISSION_TYPEDATA_SKILL;

///ミッション系統毎に異なるdataの構造体：BASIC
typedef struct{
  u16 padding[3];
}MISSION_TYPEDATA_BASIC;

///ミッション系統毎に異なるdataの構造体：ATTRIBUTE
typedef struct{
  u16 zone_id;          ///<目的地のzone_id
  u16 padding[2];
}MISSION_TYPEDATA_ATTRIBUTE;

///ミッション系統毎に異なるdataの構造体：SIZE
typedef struct{
  u16 padding[3];
}MISSION_TYPEDATA_SIZE;

///ミッション系統毎に異なるdataの構造体：ITEM
typedef struct{
  u16 item_no;            ///<渡すアイテム番号
  u16 padding[2];
}MISSION_TYPEDATA_ITEM;

///ミッション系統毎に異なるdataの構造体：OCCUR
typedef struct{
  u16 padding[3];
}MISSION_TYPEDATA_OCCUR;

///ミッション系統毎に異なるdataの構造体：PERSONALITY
typedef struct{
  u16 num;                ///<話しかけられる回数
  u16 padding[2];
}MISSION_TYPEDATA_PERSONALITY;


//==============================================================================
//  構造体定義
//==============================================================================
///ミッション要求データ
typedef struct{
  u16 zone_id;                ///<ミッション起動に使用したミニモノリスがあったゾーンID
  u8 monolith_type;           ///<石版タイプ  MONOLITH_TYPE_???
  u8 padding;
}MISSION_REQ;

///ミッションデータ
#if 0
typedef struct{
  u8 monolith_type;           ///<石版タイプ  MONOLITH_TYPE_???
  u8 mission_no;              ///<ミッション番号(ミッションが無い場合はMISSION_NO_NULL)
  u8 accept_netid;            ///<ミッション受注者のNetID
  u8 target_netid;            ///<ミッション内容によってターゲットとなるプレイヤーのNetID
  
  u16 zone_id;                ///<ミッション起動に使用したミニモノリスがあったゾーンID
  u8 padding[2];
}MISSION_DATA;
#else

///ミッションでターゲットとなったプレイヤーの情報
typedef struct{
  //ターゲットとなった相手のプレイヤー名
  //(受信した子機のローカルではプロフィールがまだ受信していない、
  //途中離脱してプロフィールが消えたとき、WORDSETが出来ない、などがある為、名前を直接送る)
  STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];

  u8 net_id;
  u8 padding[3];
}MISSION_TARGET_INFO;

///ミッションタイプ毎に異なる拡張パラメータ
typedef struct{
  union{
    struct{
      u32 work;
    }victory;
    struct{
      u32 work;
    }skill;
    struct{
      u32 work;
    }basic;
    struct{
      u32 work;
    }size;
    struct{
      u16 zone_id;
      u8 padding[2];
    }attr;
    struct{
      u32 work;
    }item;
    struct{
      u32 work;
    }occur;
    struct{
      u32 work;
    }personal;
  };
}MISSION_TYPE_WORK;

///ミッションデータ
typedef struct{
  MISSION_CONV_DATA cdata;       ///<ミッションデータ
  MISSION_TYPE_WORK exwork;   ///<タイプ毎に異なる拡張ワーク
  MISSION_TARGET_INFO target_info;  ///<ミッションターゲット
  u8 accept_netid;            ///<ミッション受注者のNetID
  u8 palace_area;             ///<どのパレスエリアで受注したミッションなのか
  
  u8 monolith_type;           ///<石版タイプ  MONOLITH_TYPE_???
  u8 target_netid;            ///<ミッション内容によってターゲットとなるプレイヤーのNetID
  
  u16 zone_id;                ///<ミッション起動に使用したミニモノリスがあったゾーンID
  u8 padding[2];
  
  u16 ready_timer;            ///<開始前のエントリー待ちの残り時間(秒)
  u16 mission_timer;          ///<ミッション残り時間(秒)
}MISSION_DATA;
#endif

///「ミッション受注します」の返事の送信データ
typedef struct{
  u8 result;                    ///<MISSION_ENTRY_RESULT_???
  u8 padding[3];
  MISSION_DATA mdata;           ///<受注OKの場合、発動したミッションデータをセット
}MISSION_ENTRY_ANSWER;

///ミッション結果
typedef struct{
  MISSION_DATA mission_data;  ///<達成したミッションデータ
  u8 achieve_netid[FIELD_COMM_MEMBER_MAX];  ///<達成者のNetID(順位順)
  u8 mission_fail;            ///<ミッション失敗時：TRUE
  u8 padding[3];
}MISSION_RESULT;

///ミッション選択候補リスト構造体
typedef struct{
  MISSION_DATA md[MISSION_TYPE_MAX];  ///<選択候補のミッションデータ
  u8 occ;                             ///<TRUE:リストが作成されている
  u8 padding[3];
}MISSION_CHOICE_LIST;

///ミッションシステム構造体
typedef struct{
  MISSION_CHOICE_LIST list[FIELD_COMM_MEMBER_MAX];  ///<選択候補のミッション一覧
  MISSION_DATA data;          ///<実行しているミッション
  MISSION_RESULT result;      ///<ミッション結果
  MISSION_ENTRY_ANSWER entry_answer[FIELD_COMM_MEMBER_MAX]; ///<「ミッション受信します」の返事
  u32 start_timer;            ///<ミッション開始時のタイム(GFL_RTC_GetTimeBySecond)
  
  //親が持つデータ
  u8 list_send_req[FIELD_COMM_MEMBER_MAX];  ///<TRUE:ミッションリストの送信を行う
  u8 result_mission_achieve[FIELD_COMM_MEMBER_MAX];  ///<ミッション達成宣言の返事(MISSION_ACHIEVE_)
  u8 data_send_req;           ///<TRUE:ミッションデータの送信を行う
  u8 result_send_req;         ///<TRUE:ミッションデータの送信を行う
  u8 send_mission_start;      ///<準備期間終了。ミッション開始を送信(_SEND_MISSION_START_xxx)
  u8 padding;

  //子が持つデータ
  u8 parent_data_recv;        ///<TRUE:親からミッションデータを受信
  u8 parent_result_recv;      ///<TRUE:親から結果を受信
  u8 recv_entry_answer_result;  ///<ミッション受注の返事(MISSION_ENTRY_RESULT_???)
  u8 parent_achieve_recv;     ///<親からミッション達成報告を受信(MISSION_ACHIEVE_???)
}MISSION_SYSTEM;

