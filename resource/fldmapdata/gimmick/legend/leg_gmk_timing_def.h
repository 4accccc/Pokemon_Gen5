//======================================================================
/**
 * @file  leg_gmk_timing_def.h
 * @brief  パッケージポケイベントタイミング定義
 * @author  Saito
 */
//======================================================================
#pragma once

//↓プログラムで使用します
#define FLASH_FRAME  ( 101 )           //輝度変更開始フレーム
#define LOOP_START_FRAME ( 450 )      //ループ先頭フレーム
#define FADE_WAIT    (15)              //輝度ウェイト

#define LEG_SE0_FRM  (0)              //ＳＥ1つ目上がる音（じょわ）
#define LEG_SE1_FRM  (131)            //ＳＥ2つ目光る音（ぴかあああ)
#define LEG_SE2_FRM  (108)            //ＳＥ3つ目エネルギーループ音
#define LEG_SE3_FRM  (250)            //ＳＥ4つ目エネルギー収縮音
  
