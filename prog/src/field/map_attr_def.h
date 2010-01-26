///////////////////////////////////////////////////////////////////
/*
	@file		map_attr_def.h
	@brief	アトリビュートラベル定義

	このファイルはwb_data3d/tool/attr.batコンバータから出力され
	wb_data3d/fieldフォルダから手動でコピーしています
*/
///////////////////////////////////////////////////////////////////


#pragma once

#define MATTR_NORMAL              (0x00)		//移動可能
#define MATTR_NOT_MOVE            (0x01)		//移動不可
#define MATTR_ZIMEN_S01           (0x02)		//四季変化地面１
#define MATTR_ZIMEN_01            (0x03)		//通常地面
#define MATTR_E_GRASS_LOW         (0x04)		//草むらエンカウント弱
#define MATTR_SNOW_GRASS_LOW      (0x05)		//豪雪地帯草むら弱
#define MATTR_E_GRASS_HIGH        (0x06)		//草むらエンカウント強い
#define MATTR_SNOW_GRASS_HIGH     (0x07)		//豪雪地帯草むら強
#define MATTR_E_LGRASS_LOW        (0x08)		//長い草（エンカウント弱）
#define MATTR_E_LGRASS_HIGH       (0x09)		//長い草（エンカウント強）
#define MATTR_E_ZIMEN_01          (0x0a)		//地面（エンカウント）
#define MATTR_DESERT_01           (0x0b)		//砂漠
#define MATTR_E_DESERT_01         (0x0c)		//砂漠（エンカウント）
#define MATTR_E_ROOM_01           (0x0d)		//屋内
#define MATTR_SNOW_01             (0x0e)		//雪
#define MATTR_SNOW_02             (0x0f)		//雪２
#define MATTR_ICE_02              (0x10)		//氷上２
#define MATTR_POOL_01             (0x14)		//水たまり（四季なし）
#define MATTR_POOL_S01            (0x15)		//水たまり（四季あり）
#define MATTR_SHOAL_01            (0x17)		//浅瀬
#define MATTR_ICE_01              (0x18)		//氷上
#define MATTR_SAND_01             (0x19)		//砂浜
#define MATTR_WATERFALL_BACK      (0x1a)		//滝の裏特殊地形
#define MATTR_MIRROR_01           (0x1b)		//鏡床
#define MATTR_MARSH_01            (0x1c)		//浅い湿原
#define MATTR_KAIRIKI_ANA         (0x1d)		//かいりき穴
#define MATTR_ZIMEN_S02           (0x1e)		//四季変化地面２
#define MATTR_LAWN_01             (0x1f)		//芝生
#define MATTR_BRIDGE_01           (0x20)		//エフェクトエンカウント橋
#define MATTR_ELECTORIC_FLOOR     (0x30)		//ビリビリ床
#define MATTR_FLOAT_FLOOR         (0x31)		//浮遊石小揺れ
#define MATTR_ELECTORIC_ROCK      (0x32)		//電気岩
#define MATTR_UPDOWN_FLOOR        (0x33)		//自機上下揺れ
#define MATTR_WATER_01            (0x3d)		//淡水（四季変化なし）
#define MATTR_WATER_S01           (0x3e)		//淡水（四季変化あり）
#define MATTR_SEA_01              (0x3f)		//海
#define MATTR_WATERFALL_01        (0x40)		//滝
#define MATTR_SHORE_01            (0x41)		//岸
#define MATTR_DEEP_MARSH_01       (0x42)		//深い湿原
#define MATTR_NMOVE_RIGHT         (0x51)		//壁（右方向へ移動不可）
#define MATTR_NMOVE_LEFT          (0x52)		//壁（左方向へ移動不可）
#define MATTR_NMOVE_UP            (0x53)		//壁（上方向へ移動不可）
#define MATTR_NMOVE_DOWN          (0x54)		//壁（下方向へ移動不可）
#define MATTR_NMOVE_RU            (0x55)		//壁（右・上方向へ移動不可）
#define MATTR_NMOVE_LU            (0x56)		//壁（左・上方向へ移動不可）
#define MATTR_NMOVE_RD            (0x57)		//壁（右・下方向へ移動不可）
#define MATTR_NMOVE_LD            (0x58)		//壁（左・下方向へ移動不可）
#define MATTR_JUMP_RIGHT          (0x72)		//右方向へジャンプ移動
#define MATTR_JUMP_LEFT           (0x73)		//左方向へジャンプ移動
#define MATTR_JUMP_UP             (0x74)		//上方向へジャンプ移動
#define MATTR_JUMP_DOWN           (0x75)		//下方向へジャンプ移動
#define MATTR_MOVEF_RIGHT         (0x76)		//右方向へ強制移動
#define MATTR_MOVEF_LEFT          (0x77)		//左方向へ強制移動
#define MATTR_MOVEF_UP            (0x78)		//上方向へ強制移動
#define MATTR_MOVEF_DOWN          (0x79)		//下方向へ強制移動
#define MATTR_DRIFT_SAND_01       (0x7c)		//流砂
#define MATTR_CURRENT_RIGHT       (0x94)		//右水流・海
#define MATTR_CURRENT_LEFT        (0x95)		//左水流・海
#define MATTR_CURRENT_UP          (0x96)		//上水流・海
#define MATTR_CURRENT_DOWN        (0x97)		//下水流・海
#define MATTR_CURRENTR_LEFT       (0x98)		//水流・川・左
#define MATTR_CURRENTR_RIGHT      (0x99)		//水流・川・右
#define MATTR_CURRENTR_UP         (0x9a)		//水流・川・上
#define MATTR_CURRENTR_DOWN       (0x9b)		//水流・川・下
#define MATTR_SLIP_01             (0xa0)		//滑り降り
#define MATTR_SLIP_GRASS_LOW      (0xa1)		//滑り降りの草むら
#define MATTR_SLIP_GRASS_HIGH     (0xa2)		//滑り降りの草むら(強)
#define MATTR_HYBRID_CHANGE       (0xa3)		//ハイブリッド乗り換え用
#define MATTR_SLIP_RIGHT          (0xa4)		//右回り滑り
#define MATTR_SLIP_LEFT           (0xa5)		//左回り滑り
#define MATTR_SLIP_JUMP_RIGHT     (0xa6)		//氷上ジャンプ音（右向き）
#define MATTR_SLIP_JUMP_LEFT      (0xa7)		//氷上ジャンプ音（左向き）
#define MATTR_TURN_RIGHT          (0xa8)		//右を向く
#define MATTR_OZE_01              (0xbe)		//尾瀬
#define MATTR_OZE_STAIRS          (0xbf)		//尾瀬階段
#define MATTR_COUNTER_01          (0xd4)		//カウンター
#define MATTR_MAT_01              (0xd5)		//マット
#define MATTR_PC_01               (0xd6)		//パソコン
#define MATTR_WORLDMAP_01         (0xd7)		//マップ
#define MATTR_TV_01               (0xd8)		//テレビ
#define MATTR_BOOKSHELF_01        (0xd9)		//本棚１
#define MATTR_BOOKSHELF_02        (0xda)		//本棚２
#define MATTR_BOOKSHELF_03        (0xdb)		//本棚３
#define MATTR_BOOKSHELF_04        (0xdc)		//本棚４
#define MATTR_VASE_01             (0xdd)		//壷
#define MATTR_DUST_BOX            (0xde)		//ゴミ箱
#define MATTR_SHOPSHELF_01        (0xdf)		//デパート＆ショップの棚１
#define MATTR_SHOPSHELF_02        (0xe0)		//デパート＆ショップの棚２
#define MATTR_SHOPSHELF_03        (0xe1)		//デパート＆ショップの棚３
#define MATTR_NO_ATTR             (0xff)		//アトリビュートなし

