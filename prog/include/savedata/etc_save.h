//==============================================================================
/**
 * @file    etc_save.h
 * @brief   第2のMISCヘッダ
 * @author  matsuda
 * @date    2010.04.06(火)
 */
//==============================================================================
#pragma once
#include "field/animal_trail_def.h" // for AT_OBJ_TYPE


//==============================================================================
//  定数定義
//==============================================================================
///その他セーブ構造体の不完全型
typedef struct _ETC_SAVE_WORK ETC_SAVE_WORK;

///マニュアルフラグ最大数
#define MANUAL_FLAG_MAX         ( 256 )

//==============================================================================
//  外部関数宣言
//==============================================================================
extern u32 EtcSave_GetWorkSize( void );
extern void EtcSave_WorkInit(void *work);
extern ETC_SAVE_WORK * SaveData_GetEtc( SAVE_CONTROL_WORK * p_sv );

extern void EtcSave_SetAcquaintance(ETC_SAVE_WORK *etcsave, u32 trainer_id);
extern BOOL EtcSave_CheckAcquaintance(ETC_SAVE_WORK *etcsave, u32 trainer_id);
extern void EtcSave_SetManualFlag( ETC_SAVE_WORK *etcsave, int id );
extern BOOL EtcSave_GetManualFlag( const ETC_SAVE_WORK *etcsave, int id );

// ライバル名
extern void EtcSave_SetRivalName( ETC_SAVE_WORK *etcsave, const STRCODE *name );
extern const STRCODE * EtcSave_GetRivalName( const ETC_SAVE_WORK *etcsave );

extern void *EtcSave_GetPersonStatePtr( ETC_SAVE_WORK *etcsave );

#ifdef PM_DEBUG
extern void EtcSave_DebugPrintOutAcuaintance( ETC_SAVE_WORK *etcsave );
#endif

// けものみち
extern AT_OBJ_TYPE ANIMAL_TRAIL_SAVE_GetObjType( const ETC_SAVE_WORK *p_save, u8 index);
extern void ANIMAL_TRAIL_SAVE_SetMainTableIndex( ETC_SAVE_WORK *p_save ,u8 trail_id ,u8 table_index);
extern void ANIMAL_TRAIL_SAVE_SetDataTableIndex( ETC_SAVE_WORK *p_save ,u8 trail_id ,u8 table_index);
extern BOOL ANIMAL_TRAIL_SAVE_GetObjectSetFlag( ETC_SAVE_WORK *p_save ,u8 trail_id );

extern void ANIMAL_TRAIL_SAVE_SetObjectSetFlag( ETC_SAVE_WORK *p_save ,u8 trail_id ,BOOL is_set);
extern u8 ANIMAL_TRAIL_SAVE_GetMainTableIndex( ETC_SAVE_WORK *p_save ,u8 trail_id );
extern u8 ANIMAL_TRAIL_SAVE_GetDataTableIndex( ETC_SAVE_WORK *p_save ,u8 trail_id );
extern void ANIMAL_TRAIL_SAVE_SetFemaleNum( ETC_SAVE_WORK *p_save ,u8 trail_id ,u8 female_num);
extern u8 ANIMAL_TRAIL_SAVE_GetFemaleNum( ETC_SAVE_WORK *p_save ,u8 trail_id );

