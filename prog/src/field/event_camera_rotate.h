
#pragma once
#include "gflib.h"
#include "gamesystem/gamesystem.h"
#include "field/fieldmap_proc.h"


//----------------------------------------------------------------------
/**
 * @brief カメラ回転イベントを呼び出す( 入った時 )
 *
 * @param p_parent   親イベント
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//----------------------------------------------------------------------
extern void EVENT_CAMERA_ROTATE_CallDoorInEvent( 
    GMEVENT* p_parent, GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );

//---------------------------------------------------------------------
/**
 * @brief 左右にあるドアから出てきた際の, カメラの初期設定を行う
 *
 * @param p_fieldmap フィールドマップ
 */
//---------------------------------------------------------------------
extern void EVENT_CAMERA_ROTATE_PrepareDoorOut( FIELDMAP_WORK* p_fieldmap );

//--------------------------------------------------------------------
/**
 * @brief カメラ回転イベントを呼び出す( 出た時 )
 *
 * @param p_parent   親イベント
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//--------------------------------------------------------------------
extern void EVENT_CAMERA_ROTATE_CallDoorOutEvent( 
    GMEVENT* p_parent, GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap ); 
