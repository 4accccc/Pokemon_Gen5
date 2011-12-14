/*
 *  @file   pair_sys_def.h
 *  @brief  連れ歩きシステムリテラル定義
 *  @author Miyuki Iwasawa
 *  @date   10.11.16
 *
 *  @com  evで読み込みのでenum禁止
 */

#ifndef __H_PAIR_SYS_DEF_H__
#define __H_PAIR_SYS_DEF_H__

#define PAIR_ID_01                    (0)  //デバッグ用
#define PAIR_ID_N01_BOSS              (1)  //Nの連れ歩き
#define PAIR_ID_C12_SWRIVAL           (2)  //C12ライバル
#define PAIR_ID_D11R0101_SWCHAMPION   (3)  //D11R0101 チャンピオン
#define PAIR_ID_C14R0501_POKE45       (4)  //C14R0501 コジョフー
#define PAIR_ID_D02R0101_SWLEADER1    (5)  //D02R0101 チェレン
#define PAIR_ID_D11R0101_SWRIVAL      (6)  //D02R0101 スワンライバル
#define PAIR_ID_D24_SWBELL            (7)  //D24 スワンベル
#define PAIR_ID_MAX                   (8)
#define PAIR_ID_NONE                  (0xFF)  //セーブ領域が8bitなので

#define PAIR_TYPE_ONE_ZONE   (0) //クローズマップ(1ゾーン内のみの移動の場合)
#define PAIR_TYPE_FREE_ZONE  (1) //オープンマップ(複数ゾーンにまたがって移動する場合)

#endif //__H_PAIR_SYS_DEF_H__
