//============================================================================================
/**
 * @file pokewood_cutin_def.h
 * @brief ポケウッドカットインの定義情報です
 * @author Toru=Nagihashi
 * @note conv_tool.rbから自動生成されています。
*/
//============================================================================================
#pragma once
//スクリプトコマンド情報
//コマ動作
#define PW_CUTIN_CMD_WND_MOVE  (0)
#define PW_CUTIN_WND_MOVE_PARAM_NUM (4)
//コマ動作２
#define PW_CUTIN_CMD_WND_MOVE_EX  (1)
#define PW_CUTIN_WND_MOVE_EX_PARAM_NUM (5)
//コマ表示
#define PW_CUTIN_CMD_WND_VISIBLE  (2)
#define PW_CUTIN_WND_VISIBLE_PARAM_NUM (2)
//コマ揺れ
#define PW_CUTIN_CMD_WND_SHAKE  (3)
#define PW_CUTIN_WND_SHAKE_PARAM_NUM (2)
//リソースチェンジ
#define PW_CUTIN_CMD_RES_CHANGE  (4)
#define PW_CUTIN_RES_CHANGE_PARAM_NUM (3)
//待機
#define PW_CUTIN_CMD_WAIT  (5)
#define PW_CUTIN_WAIT_PARAM_NUM (1)
//メッセージ表示
#define PW_CUTIN_CMD_MESSAGE_DRAW  (6)
#define PW_CUTIN_MESSAGE_DRAW_PARAM_NUM (3)
//メッセージ隠す
#define PW_CUTIN_CMD_MESSAGE_HIDE  (7)
#define PW_CUTIN_MESSAGE_HIDE_PARAM_NUM (0)
//SE再生
#define PW_CUTIN_CMD_SE_PLAY  (8)
#define PW_CUTIN_SE_PLAY_PARAM_NUM (1)
//SE待機
#define PW_CUTIN_CMD_SE_WAIT  (9)
#define PW_CUTIN_SE_WAIT_PARAM_NUM (0)
//SE停止
#define PW_CUTIN_CMD_SE_STOP  (10)
#define PW_CUTIN_SE_STOP_PARAM_NUM (0)
//定義情報
//移動方向：上から下
#define PW_CUTIN_DEF_MOVE_DIR_DOWN  (0)
//移動方向：下から上
#define PW_CUTIN_DEF_MOVE_DIR_UP  (1)
//移動方向：右から左
#define PW_CUTIN_DEF_MOVE_DIR_LEFT  (2)
//移動方向：左から右
#define PW_CUTIN_DEF_MOVE_DIR_RIGHT  (3)
//移動方向：右下から左上
#define PW_CUTIN_DEF_MOVE_DIR_LEFT_UP  (4)
//移動方向：左下から右上
#define PW_CUTIN_DEF_MOVE_DIR_RIGHT_UP  (5)
//移動方向：右上から左下
#define PW_CUTIN_DEF_MOVE_DIR_LEFT_DOWN  (6)
//移動方向：左上から右下
#define PW_CUTIN_DEF_MOVE_DIR_RIGHT_DOWN  (7)
//移動タイプ：等速
#define PW_CUTIN_DEF_MOVE_TYPE_VELOCITY  (0)
//移動タイプ：減速
#define PW_CUTIN_DEF_MOVE_TYPE_SLOWDOWN  (1)
//メッセージ：自分
#define PW_CUTIN_DEF_MES_TYPE_PLAYER  (0)
//メッセージ：相手
#define PW_CUTIN_DEF_MES_TYPE_ENEMY  (1)
//文字位置：上
#define PW_CUTIN_DEF_MES_POS_UP  (0)
//文字位置：下
#define PW_CUTIN_DEF_MES_POS_DOWN  (1)
//揺れタイプ：上下
#define PW_CUTIN_DEF_SHAKE_TYPE_UD  (0)
//揺れタイプ：下上
#define PW_CUTIN_DEF_SHAKE_TYPE_DU  (1)
//揺れタイプ：左右
#define PW_CUTIN_DEF_SHAKE_TYPE_LR  (2)
//揺れタイプ：右左
#define PW_CUTIN_DEF_SHAKE_TYPE_RL  (3)
