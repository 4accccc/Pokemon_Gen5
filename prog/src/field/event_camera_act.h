///////////////////////////////////////////////////////////////////////////////////////////////
/**
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
 * @param p_parent   親イベント
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//----------------------------------------------------------------------
extern void EVENT_CAMERA_ACT_CallDoorInEvent( 
    GMEVENT* p_parent, GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );

//---------------------------------------------------------------------
/**
 * @brief ドアから出てきた際の, カメラの初期設定を行う
 *
 * @param p_fieldmap フィールドマップ
 */
//---------------------------------------------------------------------
extern void EVENT_CAMERA_ACT_PrepareForDoorOut( FIELDMAP_WORK* p_fieldmap );

//--------------------------------------------------------------------
/**
 * @brief カメラ動作イベントを呼び出す( 出た時 )
 *
 * @param p_parent   親イベント
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//--------------------------------------------------------------------
extern void EVENT_CAMERA_ACT_CallDoorOutEvent( 
    GMEVENT* p_parent, GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap ); 

//--------------------------------------------------------------------
/**
 * @breif カメラのNearプレーンとFarプレーンをデフォルト設定に戻す
 *
 * @param p_fieldmap フィールドマップ
 */
//--------------------------------------------------------------------
extern void EVENT_CAMERA_ACT_ResetCameraParameter( FIELDMAP_WORK* p_fieldmap );
