//======================================================================
/**
 * @file  gym_elec_local_def.h
 * @bfief  電気ジムフレーム制御定義
 * @author  Saito
 */
//======================================================================
#ifndef __GYM_ELEC_LOCAL_DEF_H__

#define __GYM_ELEC_LOCAL_DEF_H__

//基準プラットフォーム以外の停車フレーム定義
//※レール3,5,7はプラットフォームが1箇所しかない（基準フレーム）なので定義をきっていません
#define R_STOP_FRM1   (15)  //レール1
#define R_STOP_FRM2   (33)  //レール2
#define R_STOP_FRM4   (125) //レール4
#define R_STOP_FRM6   (57)  //レール6
#define R_STOP_FRM8   (113) //レール8

//カプセルごとのレール移動アニメ共通部分フレーム（アニメデータ依存）
#define COMM_FRM1   (4)
#define COMM_FRM2   (120)
#define COMM_FRM3   (40)
#define COMM_FRM4   (70)

#endif    //__GYM_ELEC_LOCAL_DEF_H__

