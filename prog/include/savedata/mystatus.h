//============================================================================================
/**
 * @file  mystatus.h
 * @brief 自分状態データアクセス用ヘッダ
 * @author  tamada GAME FREAK inc.
 * @date  2005.10.27
 */
//============================================================================================
#ifndef __MYSTATUS_H__
#define __MYSTATUS_H__

#include <strbuf.h>

#include "savedata/save_control.h"  //SAVE_CONTROL_WORK参照のため
#include "buflen.h"

//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief 自分状態データ型定義
 */
//----------------------------------------------------------
typedef struct _MYSTATUS MYSTATUS;

//----------------------------------------------------------
/**
 * @brief 自分状態データ型定義
 */
//----------------------------------------------------------
struct _MYSTATUS {
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
};


#define MYSTATUS_SAVE_SIZE (32)

#define POKEMON_DP_ROM_CODE ( 0 )   ///< PokemonDP で 扱われていた RomCode

//============================================================================================
//============================================================================================
//----------------------------------------------------------
//  セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int MyStatus_GetWorkSize(void);
extern MYSTATUS * MyStatus_AllocWork(u32 heapID);
extern void MyStatus_Copy(const MYSTATUS * from, MYSTATUS * to);

//----------------------------------------------------------
//  MYSTATUS操作のための関数
//----------------------------------------------------------
extern void MyStatus_Init(MYSTATUS * my);

// 名前が０で埋められてりうかチェック
// (データが入っていない常態かをチェックする)
extern BOOL MyStatus_CheckNameClear( const MYSTATUS * my );

//名前
extern void MyStatus_SetMyName(MYSTATUS * my, const STRCODE * name);
extern const STRCODE * MyStatus_GetMyName(const MYSTATUS * my);
extern void MyStatus_CopyNameStrCode( const MYSTATUS * my, STRCODE *buf, int dest_length );
extern STRBUF * MyStatus_CreateNameString(const MYSTATUS * my, int heapID);
extern void MyStatus_CopyNameString( const MYSTATUS * my, STRBUF *buf );
extern void MyStatus_SetMyNameFromString(MYSTATUS * my, const STRBUF * str);

//ID
extern void MyStatus_SetID(MYSTATUS * my, u32 id);
extern u32 MyStatus_GetID(const MYSTATUS * my);
extern u16 MyStatus_GetID_Low(const MYSTATUS * my);

//ProfileID
extern void MyStatus_SetProfileID(MYSTATUS * my, s32 id);
extern s32 MyStatus_GetProfileID(const MYSTATUS * my);

//性別
extern void MyStatus_SetMySex(MYSTATUS * my, int sex);
extern u32 MyStatus_GetMySex(const MYSTATUS * my);

//国 地域
extern int MyStatus_GetMyNation(const MYSTATUS * my);
extern int MyStatus_GetMyArea(const MYSTATUS * my);

// この関数だけだと、地球儀に登録されません。 WIFIHISTORY_SetMyNationAreaを使ってください
extern void MyStatus_SetMyNationArea(MYSTATUS * my, int nation, int area);


// ユニオンルームで使う自分の見た目
extern u8 MyStatus_GetTrainerView( const MYSTATUS *my );
extern void MyStatus_SetTrainerView( MYSTATUS *my, u8 view );

// ROMコード = PM_VERSIONが初期値
extern u8 MyStatus_GetRomCode( const MYSTATUS * my );
extern void MyStatus_SetRomCode( MYSTATUS * my, u8 rom_code );
extern u8 PokemonDP_GetRomCode( void );

// リージョンコード  //PM_LANGが初期値
extern u8  MyStatus_GetRegionCode( const MYSTATUS * my );
extern void  MyStatus_SetRegionCode( MYSTATUS * my, u8 region_code );

// 外部セーブデータ初期化済みフラグ
extern void MyStatus_SetExtraInitFlag(MYSTATUS * my);
extern u32 MyStatus_GetExtraInitFlag(const MYSTATUS * my);


//----------------------------------------------------------
//  セーブデータ取得のための関数
//----------------------------------------------------------
extern MYSTATUS * SaveData_GetMyStatus(SAVE_CONTROL_WORK * sv);


//  myStatus同士が同じものかどうかを調べる
BOOL MyStatus_Compare(const MYSTATUS * my, const MYSTATUS * target);

//----------------------------------------------------------
//  デバッグ用データ生成のための関数
//----------------------------------------------------------
#ifdef PM_DEBUG
extern void DEBUG_MyStatus_DummyNameSet(MYSTATUS *mystatus, HEAPID heap_id);
#endif  //PM_DEBUG

#ifdef CREATE_INDEX
extern void *Index_Get_Mystatus_Name_Offset(MYSTATUS *my);
extern void *Index_Get_Mystatus_Id_Offset(MYSTATUS *my);
extern void *Index_Get_Mystatus_Sex_Offset(MYSTATUS *my);
#endif

#endif //__MYSTATUS_H__
