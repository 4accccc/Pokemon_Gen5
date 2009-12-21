///////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   event_camera_act.h
 * @brief  ドアに出入りする際のカメラ動作イベント
 * @author obata
 * @date   2009.08.17
 */
///////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "gflib.h"
#include "gamesystem/gamesystem.h"
#include "field/fieldmap_proc.h"


//----------------------------------------------------------------------
/**
 * @brief カメラ動作イベントを呼び出す( 入った時 )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//----------------------------------------------------------------------
extern void EVENT_CAMERA_ACT_CallDoorInEvent( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );

//---------------------------------------------------------------------
/**
 * @brief ドアから出てきた際の, カメラの初期設定を行う
 *
 * @param fieldmap フィールドマップ
 */
//---------------------------------------------------------------------
extern void EVENT_CAMERA_ACT_PrepareForDoorOut( FIELDMAP_WORK* fieldmap );

//--------------------------------------------------------------------
/**
 * @brief カメラ動作イベントを呼び出す( 出た時 )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//--------------------------------------------------------------------
extern void EVENT_CAMERA_ACT_CallDoorOutEvent( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap ); 

//--------------------------------------------------------------------
/**
 * @breif カメラのNearプレーンとFarプレーンをデフォルト設定に戻す
 *
 * @param fieldmap フィールドマップ
 */
//--------------------------------------------------------------------
extern void EVENT_CAMERA_ACT_ResetCameraParameter( FIELDMAP_WORK* fieldmap );
