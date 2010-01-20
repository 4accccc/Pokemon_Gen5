/*
 *  @file encount_data.h
 *  @brief  フィールド　エンカウントデータテーブル構造体
 *  @author Miyuki Iwasawa
 *  @date 09.09.25
 */

#ifndef __H_ENCOUNT_DATA_H__
#define __H_ENCOUNT_DATA_H__

#define ENC_DATA_INVALID_ID (0xFFFF)  ///<無効エンカウトデータID

#define ENC_MONS_NUM_GROUND_L   (12)   ///<通常エンカウント(ロークラス)
#define ENC_MONS_NUM_GROUND_H   (12)   ///<通常エンカウント２(ハイクラス)
#define ENC_MONS_NUM_GROUND_SP  (12)   ///<特殊エンカウント

#define ENC_MONS_NUM_WATER         (5) ///<水上エンカウント
#define ENC_MONS_NUM_WATER_SP      (5) ///<特殊水上エンカウント
#define ENC_MONS_NUM_FISHING       (5) ///<釣りエンカウント
#define ENC_MONS_NUM_FISHING_SP    (5) ///<特殊釣りエンカウント
#define ENC_MONS_NUM_BINGO        (15)  ///<ビンゴエンカウントモンスター数

#define ENC_MONS_NUM_MAX          (ENC_MONS_NUM_BINGO)  ///<現状ビンゴが最大

#define ENC_MONS_FORM_RND_CODE    (0x1F)  //フォルムをランダムで決める特殊コード

///エンカウントロケーションID
typedef enum{
 ENC_LOCATION_GROUND_L,
 ENC_LOCATION_GROUND_H,
 ENC_LOCATION_GROUND_SP,
 ENC_LOCATION_WATER,
 ENC_LOCATION_WATER_SP,
 ENC_LOCATION_FISHING,
 ENC_LOCATION_FISHING_SP,
 ENC_LOCATION_MAX,
 ENC_LOCATION_NONE = 0xFF,  //無効ID
}ENCOUNT_LOCATION;

///エンカウントタイプ
typedef enum{
 ENC_TYPE_NORMAL,   ///<通常エンカウント
 ENC_TYPE_EFFECT,   ///<エフェクトエンカウント
 ENC_TYPE_FORCE,    ///<強制エンカウント(甘い香り/甘い蜜)
 ENC_TYPE_WFBC,    ///<WFBCエンカウント(ランダムマップ)
 ENC_TYPE_MAX,
}ENCOUNT_TYPE;

///エンカウントモンスターテーブルデータ
typedef struct _ENC_COMMON_DAT{
  u16 monsNo:11;
  u16 form:5;
  u8  minLevel;
  u8  maxLevel;
}ENC_COMMON_DATA;

typedef struct _ENC_MONS_DATA{
  u16 monsNo;
  u8  level;
  u8  form;
}ENC_MONS_DATA;

typedef struct _ENCOUNT_DATA{
  union{
    struct{
      u8  probGroundLow;   ///<地面ロークラスエンカウント率
      u8  probGroundHigh;  ///<地面ハイクラスエンカウント率
      u8  probGroundSp;    ///<地面特殊エンカウント率(0or1)
      u8  probWater;     ///<水上エンカウト率
      u8  probWaterSp;   ///<水上特殊エンカウント率(0or1)
      u8  probFishing;   ///<釣りエンカウント率
      u8  probFishingSp; ///<釣り特殊エンカウント率(0or1)
      u8  itemTblIdx:7;   ///<フィールドエフェクトアイテムテーブル参照idx
      u8  enable_f:1;     ///<有効無効フラグ
    };
    u8 prob[ENC_LOCATION_MAX+1];
  };

  ENC_COMMON_DATA  groundLmons[ENC_MONS_NUM_GROUND_L];
  ENC_COMMON_DATA  groundHmons[ENC_MONS_NUM_GROUND_H];
  ENC_COMMON_DATA  groundSmons[ENC_MONS_NUM_GROUND_SP];
  
  ENC_COMMON_DATA  waterMons[ENC_MONS_NUM_WATER];
  ENC_COMMON_DATA  waterSpMons[ENC_MONS_NUM_WATER_SP];
  ENC_COMMON_DATA  fishingMons[ENC_MONS_NUM_FISHING];
  ENC_COMMON_DATA  fishingSpMons[ENC_MONS_NUM_FISHING_SP];
}ENCOUNT_DATA;

typedef struct _ENCOUNT_TABLE{
  ENCOUNT_DATA  enc_tbl[4];
}ENCOUNT_TABLE;

#endif  //__H_ENCOUNT_DATA_H__
