/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  橋ISSシステム ( 3Dサウンドシステム )
 * @file   iss_3d_sys.h
 * @author obata
 * @date   2009.09.08
 */
/////////////////////////////////////////////////////////////////////////////////
#pragma once 


//===============================================================================
// ■不完全型の宣言
//===============================================================================
typedef struct _ISS_3DS_SYS ISS_3DS_SYS;


//===============================================================================
// ■定数
//===============================================================================
// 3Dサウンドユニット インデックス
typedef enum
{
  ISS3DS_UNIT_TRACK01,  // トラック 1を操作するユニット
  ISS3DS_UNIT_TRACK02,  // トラック 2を操作するユニット
  ISS3DS_UNIT_TRACK03,  // トラック 3を操作するユニット
  ISS3DS_UNIT_TRACK04,  // トラック 4を操作するユニット
  ISS3DS_UNIT_TRACK05,  // トラック 5を操作するユニット
  ISS3DS_UNIT_TRACK06,  // トラック 6を操作するユニット
  ISS3DS_UNIT_TRACK07,  // トラック 7を操作するユニット
  ISS3DS_UNIT_TRACK08,  // トラック 8を操作するユニット
  ISS3DS_UNIT_TRACK09,  // トラック 9を操作するユニット
  ISS3DS_UNIT_TRACK10,  // トラック10を操作するユニット
  ISS3DS_UNIT_TRACK11,  // トラック11を操作するユニット
  ISS3DS_UNIT_TRACK12,  // トラック12を操作するユニット
  ISS3DS_UNIT_TRACK13,  // トラック13を操作するユニット
  ISS3DS_UNIT_TRACK14,  // トラック14を操作するユニット
  ISS3DS_UNIT_TRACK15,  // トラック15を操作するユニット
  ISS3DS_UNIT_TRACK16,  // トラック16を操作するユニット
  ISS3DS_UNIT_NUM
} ISS3DS_UNIT_INDEX;


//===============================================================================
// ■システム
//===============================================================================
// 作成 / 破棄
extern ISS_3DS_SYS* ISS_3DS_SYS_Create( HEAPID heapID );
extern void         ISS_3DS_SYS_Delete( ISS_3DS_SYS* system );

// 動作 
extern void ISS_3DS_SYS_Main( ISS_3DS_SYS* system );

// 制御
extern void ISS_3DS_SYS_On             ( ISS_3DS_SYS* system );
extern void ISS_3DS_SYS_Off            ( ISS_3DS_SYS* system ); 
extern void ISS_3DS_SYS_ZoneChange     ( ISS_3DS_SYS* system );
extern void ISS_3DS_SYS_SetMasterVolume( ISS_3DS_SYS* system, u8 volume );


//===============================================================================
// ■ユニット
//=============================================================================== 
// 登録
extern void ISS_3DS_SYS_RegisterUnit( ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx, 
                                      fx32 effectiveRange, int maxVolume );

// 登録済みかどうか
extern BOOL ISS_3DS_SYS_IsUnitRegistered( const ISS_3DS_SYS* system, ISS3DS_UNIT_INDEX unitIdx );

// 移動
extern void ISS_3DS_SYS_SetUnitPos( ISS_3DS_SYS* system, 
                                    ISS3DS_UNIT_INDEX unitIdx, const VecFx32* pos );


//===============================================================================
// ■観測者
//===============================================================================
// 移動
extern void ISS_3DS_SYS_SetObserverPos( ISS_3DS_SYS* system, 
                                        const VecFx32* pos, const VecFx32* targetPos );
