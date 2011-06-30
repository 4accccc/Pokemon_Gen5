//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//   @file		gym_dragon_gym_data_def.h
//   @brief   ドラゴンジム定義データ
//   @author	Masayuki Onoue
//   @date		2011.06.23
//   @note    /resource/fldmapdata/gimmick/gym_dragon/data_cnv.rbが出力しています
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#define  FOG_COLOR_A (31) // フォグカラーA
#define  FOG_SLOPE (6) // フォグのかかり具合
#define  FOG_OFFSET (32495) // フォグオフセット
#define  DRA_BARK_MOVE_STOP_SE_FRAME (200) // 咆える前の停止音再生タイミング
#define  DRA_BARK_FRAME_HERO (180) // ドラゴンがほえるタイミング（アニメーション終了タイミングから値分さかのぼったタイミングでなく）
#define  DRA_BARK_FRAME_ENEMY (120) // ドラゴンがほえるタイミング（アニメーション終了タイミングから値分さかのぼったタイミングでなく）
#define  DRA_BARK_MOVE_START_SE_FRAME (60) // 咆えた後の移動開始音再生タイミング
#define  DRA_BUMP_SE_FRAME (5) // 衝突音の再生タイミング
#define  DRA_STOP_SE_FRAME (5) // ドラゴン停止音の再生タイミング
#define  TURNON_EVENT_WAIT_FRAME1 (20) // 初回ボタン点灯アニメーションのフレーム数（1階用）
#define  TURNON_EVENT_WAIT_FRAME2 (16) // 初回ボタン点灯アニメーションのフレーム数（2
#define  DRA_BARK_CAMERA_SHAKE_HERO (180) // ドラゴンが咆えたときのカメラゆれタイミング（自機ドラゴン）
#define  DRA_BARK_CAMERA_SHAKE_ENEMY (120) // ドラゴンが咆えたときのカメラゆれタイミング（敵ドラゴン）
#define  DRA_BARK_CAMERA_SHAKE_W (1) // 揺れ幅(ドラゴン咆哮時)
#define  DRA_BARK_CAMERA_SHAKE_H (2) // 揺れ高さ(ドラゴン咆哮時)
#define  DRA_BARK_CAMERA_SHAKE_SYNC (3) // 1周期するのにかかるシンク数(ドラゴン咆哮時)
#define  DRA_BARK_CAMERA_SHAKE_TIME (4) // ゆれ回数(ドラゴン咆哮時)
#define  DRA_BARK_EFFECT_CAM_START (185) // ドラゴン咆哮時の演出カメラの開始タイミング
#define  DRA_BARK_EFFECT_CAM_END (50) // ドラゴン咆哮時の演出カメラの終了タイミング
#define  DRA_BUMP_CAMERA_SHAKE_FRAME (5) // 衝突時のカメラゆれ開始タイミング（ドラゴン衝突時）
#define  DRA_BUMP_CAMERA_SHAKE_W (1) // 揺れ幅（ドラゴン衝突時）
#define  DRA_BUMP_CAMERA_SHAKE_H (2) // 揺れ高さ（ドラゴン衝突時）
#define  DRA_BUMP_CAMERA_SHAKE_SYNC (3) // 1周期するのにかかるシンク数（ドラゴン衝突時）
#define  DRA_BUMP_CAMERA_SHAKE_TIME (4) // ゆれ回数（ドラゴン衝突時）
