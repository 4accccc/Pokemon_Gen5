//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//   @file		bage_gate_def.h
//   @brief   バッジチェックゲート定義データ
//   @author	Masayuki Onoue
//   @date		2011.07.11
//   @note    /resource/fldmapdata/gimmick/badge_gate/def_data_cnv.rbが出力しています
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#define  SHAKE_START_FRAME_0 (135) // 振動が開始するフレーム(0スタート）-柱が沈む
#define  SHAKE_END_FRAME_0 (255) // 振動が終了するフレーム(0スタート）-柱が沈む
#define  SHAKE_LOOP_SYNC_0 (4) // 振動の周期フレーム（○フレームで1回揺れる）-柱が沈む
#define  SHAKE_W_0 (1) // 振動の幅-柱が沈む
#define  SHAKE_H_0 (1) // 振動の高さ-柱が沈む
#define  SHAKE_START_FRAME_1 (290) // 振動が開始するフレーム(0スタート）-門が沈む
#define  SHAKE_END_FRAME_1 (322) // 振動が終了するフレーム(0スタート）-門が沈む
#define  SHAKE_LOOP_SYNC_1 (4) // 振動の周期フレーム（○フレームで1回揺れる）-門が沈む
#define  SHAKE_W_1 (2) // 振動の幅-門が沈む
#define  SHAKE_H_1 (2) // 振動の高さ-門が沈む
#define  GATE_SE_LIGHT_MOVE_FRAME (0) // ゲートの光が立ち上る音
#define  GATE_SE_SYMBOL_LIGHT_FRAME (100) // ゲート起動、シンボルマークが光るところ音
#define  GATE_SE_COLUMNLE_DOWN_FRAME (140) // 石柱と石版がつぎつぎ沈む音
#define  GATE_SE_OPEN_FRAME (260) // 石門が沈む音
#define  MONUMENT_SE_LASER_FRAME (0) // センサーが出すレーザ音の鳴らすタイミング
#define  MONUMENT_SE_OPNE_FRAME (10) // センサーがせりあがる音の鳴らすタイミング
