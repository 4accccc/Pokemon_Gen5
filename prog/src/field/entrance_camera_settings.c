/////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  特殊出入り口のカメラ演出データ
 * @file   entrance_camera_settings.c
 * @author obata
 * @date   2010.02.16
 */
/////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "entrance_camera_settings.h"

#include "field/eventdata_type.h"  // for EXIT_TYPE_xxxx
#include "arc/arc_def.h"           // for ARCID_xxxx
#include "../../resource/fldmapdata/entrance_camera/entrance_camera.naix"  // for NARC_xxxx


// デバッグ出力スイッチ
#define DEBUG_PRINT_ON


//--------------------------------------------------------------------------------------- 
/**
 * @brief 指定した出入り口タイプに該当するデータを取得する
 *
 * @param dest     取得したデータの格納先
 * @param exitType データ取得対象の出入り口タイプ
 */
//---------------------------------------------------------------------------------------
void ENTRANCE_CAMERA_SETTINGS_LoadData( ENTRANCE_CAMERA_SETTINGS* dest, EXIT_TYPE exitType )
{
  // 出入り口タイプに対応するカメラデータのインデックス
  const ARCDATID dataID[ EXIT_TYPE_MAX ] = 
  {
    0, 0, 0, 0, 0, 0, 0,  // EXIT_TYPE_NONE - EXIT_TYPE_INTRUDE
    NARC_entrance_camera_exit_type_sp1_bin,    // EXIT_TYPE_SP1
    NARC_entrance_camera_exit_type_sp2_bin,    // EXIT_TYPE_SP2
    NARC_entrance_camera_exit_type_sp3_bin,    // EXIT_TYPE_SP3
    NARC_entrance_camera_exit_type_sp4_bin,    // EXIT_TYPE_SP4
    NARC_entrance_camera_exit_type_sp5_bin,    // EXIT_TYPE_SP5
    NARC_entrance_camera_exit_type_sp6_bin,    // EXIT_TYPE_SP6
    NARC_entrance_camera_exit_type_sp7_bin,    // EXIT_TYPE_SP7
    NARC_entrance_camera_exit_type_sp8_bin,    // EXIT_TYPE_SP8
    NARC_entrance_camera_exit_type_sp9_bin,    // EXIT_TYPE_SP9
    NARC_entrance_camera_exit_type_sp10_bin,   // EXIT_TYPE_SP10
    NARC_entrance_camera_exit_type_sp11_bin,   // EXIT_TYPE_SP11
    NARC_entrance_camera_exit_type_sp12_bin,   // EXIT_TYPE_SP12
    NARC_entrance_camera_exit_type_sp13_bin,   // EXIT_TYPE_SP13
    NARC_entrance_camera_exit_type_sp14_bin,   // EXIT_TYPE_SP14
    NARC_entrance_camera_exit_type_sp15_bin,   // EXIT_TYPE_SP15
    NARC_entrance_camera_exit_type_sp16_bin,   // EXIT_TYPE_SP16
    NARC_entrance_camera_exit_type_sp17_bin,   // EXIT_TYPE_SP17
    NARC_entrance_camera_exit_type_sp18_bin,   // EXIT_TYPE_SP18
    NARC_entrance_camera_exit_type_sp19_bin,   // EXIT_TYPE_SP19
    NARC_entrance_camera_exit_type_sp20_bin,   // EXIT_TYPE_SP20
    NARC_entrance_camera_exit_type_sp21_bin,   // EXIT_TYPE_SP21
    NARC_entrance_camera_exit_type_sp22_bin,   // EXIT_TYPE_SP22
    NARC_entrance_camera_exit_type_sp23_bin,   // EXIT_TYPE_SP23
    NARC_entrance_camera_exit_type_sp24_bin,   // EXIT_TYPE_SP24
    NARC_entrance_camera_exit_type_sp25_bin,   // EXIT_TYPE_SP25
    NARC_entrance_camera_exit_type_sp26_bin,   // EXIT_TYPE_SP26
    NARC_entrance_camera_exit_type_sp27_bin,   // EXIT_TYPE_SP27
    NARC_entrance_camera_exit_type_sp28_bin,   // EXIT_TYPE_SP28
    NARC_entrance_camera_exit_type_sp29_bin,   // EXIT_TYPE_SP29
    NARC_entrance_camera_exit_type_sp30_bin,   // EXIT_TYPE_SP30
  };

  // 引数エラー
  GF_ASSERT( exitType < EXIT_TYPE_MAX );

  // 読み込み
  GFL_ARC_LoadData( dest, ARCID_ENTRANCE_CAMERA, dataID[ exitType ] );

  // DEBUG:
#ifdef DEBUG_PRINT_ON
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: frame   = %d\n", dest->frame );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: pitch   = %x\n", dest->pitch );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: yaw     = %x\n", dest->yaw );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: length  = %x\n", dest->length );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: offsetX = %x\n", dest->targetOffsetX );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: offsetY = %x\n", dest->targetOffsetX );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: offsetZ = %x\n", dest->targetOffsetX );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: validFlag_IN  = %x\n", dest->validFlag_IN );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: validFlag_OUT = %x\n", dest->validFlag_OUT );
#endif
}
