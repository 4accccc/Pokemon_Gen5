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

#define PAIR_ID_01                    (0)                     //デバッグ用
#define PAIR_ID_NATURAL               (PAIR_ID_01+1)          //Nの連れ歩き
#define PAIR_ID_C12_SWRIVAL           (PAIR_ID_NATURAL+1)     //C12ライバル
#define PAIR_ID_D11R0101_SWCHAMPION   (PAIR_ID_C12_SWRIVAL+1) //D11R0101チャンピオン
#define PAIR_ID_C14R0501_POKE45       (PAIR_ID_D11R0101_SWCHAMPION+1) //C14R0501 コジョフー
#define PAIR_ID_D02R0101_SWLEADER1    (PAIR_ID_C14R0501_POKE45+1)     //D02R0101 チェレン
#define PAIR_ID_MAX                   (PAIR_ID_C14R0501_POKE45+1)
#define PAIR_ID_NONE                  (0xFFFF)

#define PAIR_TYPE_ONE_ZONE   (0) //クローズマップ(1ゾーン内のみの移動の場合)
#define PAIR_TYPE_FREE_ZONE  (1) //オープンマップ(複数ゾーンにまたがって移動する場合)

#endif //__H_PAIR_SYS_DEF_H__
