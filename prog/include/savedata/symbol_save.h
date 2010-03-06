//==============================================================================
/**
 * @file    symbol_save.h
 * @brief   シンボルエンカウント用セーブデータ
 * @author  matsuda
 * @date    2010.01.06(水)
 */
//==============================================================================
#pragma once


//==============================================================================
//  型定義
//==============================================================================
//シンボルエンカウントセーブデータ
typedef struct _SYMBOL_SAVE_WORK SYMBOL_SAVE_WORK;


//==============================================================================
//  定数定義
//==============================================================================
///シンボルポケモン最大数
#define SYMBOL_POKE_MAX           (530)

///配置No
enum{
  SYMBOL_NO_START_KEEP_LARGE = 0,                             ///<キープゾーン(64x64)開始No
  SYMBOL_NO_END_KEEP_LARGE = SYMBOL_NO_START_KEEP_LARGE + 10, ///<キープゾーン(64x64)終了No
  SYMBOL_NO_START_KEEP_SMALL = SYMBOL_NO_END_KEEP_LARGE,      ///<キープゾーン(32x32)開始No
  SYMBOL_NO_END_KEEP_SMALL = SYMBOL_NO_START_KEEP_SMALL + 10, ///<キープゾーン(32x32)終了No
  
  SYMBOL_NO_START_FREE_LARGE = SYMBOL_NO_END_KEEP_SMALL,      ///<フリーゾーン(64x64)開始No
  SYMBOL_NO_END_FREE_LARGE = SYMBOL_NO_START_FREE_LARGE + 30, ///<フリーゾーン(64x64)終了No
  SYMBOL_NO_START_FREE_SMALL = SYMBOL_NO_END_FREE_LARGE,      ///<フリーゾーン(32x32)開始No
  SYMBOL_NO_END_FREE_SMALL = SYMBOL_NO_START_FREE_SMALL + 480,///<フリーゾーン(32x32)終了No
};

/// ※SymbolZoneTypeDataNoと並びを同じにしておくこと！
typedef enum{
  SYMBOL_ZONE_TYPE_KEEP_LARGE,      ///<キープゾーン(64x64)専用エリア
  SYMBOL_ZONE_TYPE_KEEP_SMALL,      ///<キープゾーン(32x32)専用エリア
  SYMBOL_ZONE_TYPE_FREE_LARGE,      ///<フリーゾーン(64x64)
  SYMBOL_ZONE_TYPE_FREE_SMALL,      ///<フリーゾーン(32x32)
}SYMBOL_ZONE_TYPE;

///対象ゾーンに空きが無い事を示す値
#define SYMBOL_SPACE_NONE        (0xffff)

///シンボルマップ(64x64)のマップレベル(数字が大きいほど広い)
typedef enum{
  SYMBOL_MAP_LEVEL_LARGE_NONE,      ///<マップが存在しない
  SYMBOL_MAP_LEVEL_LARGE_LEVEL_1,
  
  SYMBOL_MAP_LEVEL_LARGE_MAX = SYMBOL_MAP_LEVEL_LARGE_LEVEL_1,
}SYMBOL_MAP_LEVEL_LARGE;

///シンボルマップ(32x32)のマップレベル(数字が大きいほど広い)
typedef enum{
  SYMBOL_MAP_LEVEL_SMALL_1,
  SYMBOL_MAP_LEVEL_SMALL_2,
  SYMBOL_MAP_LEVEL_SMALL_3,
  SYMBOL_MAP_LEVEL_SMALL_4,
  SYMBOL_MAP_LEVEL_SMALL_5,
  SYMBOL_MAP_LEVEL_SMALL_6,
  SYMBOL_MAP_LEVEL_SMALL_7,
  
  SYMBOL_MAP_LEVEL_SMALL_MAX = SYMBOL_MAP_LEVEL_SMALL_7,
}SYMBOL_MAP_LEVEL_SMALL;

///1マップ(32x32)でのストック数
#define SYMBOL_MAP_STOCK_SMALL    (30)
///1マップ(64x64)でのストック数
#define SYMBOL_MAP_STOCK_LARGE    (10)
///キープゾーン、フリーゾーンの中で最も1マップ中、多くストック出来る数
#define SYMBOL_MAP_STOCK_MAX      (SYMBOL_MAP_STOCK_SMALL)


//==============================================================================
//  構造体定義
//==============================================================================
///シンボルエンカウント1匹分のセーブ
typedef struct _SYMBOL_POKEMON{
  u32 monsno:11;        ///<ポケモン番号
  u32 wazano:10;        ///<技番号
  u32 sex:2;            ///<性別(PTL_SEX_MALE, PTL_SEX_FEMALE, PTL_SEX_UNKNOWN)
  u32 form_no:6;        ///<フォルム番号
  u32        :3;        //余り
}SYMBOL_POKEMON;


///SYMBOL_ZONE_TYPE毎の配置Noの開始位置と終端位置を管理する構造体
typedef struct{
  u16 start;
  u16 end;
}SYMBOL_ZONE_TYPE_DATA_NO;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern u32 SymbolSave_GetWorkSize( void );
extern void SymbolSave_WorkInit(void *work);

//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
extern SYMBOL_SAVE_WORK* SymbolSave_GetSymbolData(SAVE_CONTROL_WORK* pSave);

//--------------------------------------------------------------
//  アクセス関数
//--------------------------------------------------------------
extern u32 SymbolSave_CheckFreeZoneSpace(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_ZONE_TYPE zone_type);
extern void SymbolSave_DataShift(SYMBOL_SAVE_WORK *symbol_save, u32 no);
extern void SymbolSave_SetFreeZone(SYMBOL_SAVE_WORK *symbol_save, u16 monsno, u16 wazano, u8 sex, u8 form_no, SYMBOL_ZONE_TYPE zone_type);

//==============================================================================
//  外部データ
//==============================================================================
extern const SYMBOL_ZONE_TYPE_DATA_NO SymbolZoneTypeDataNo[];
