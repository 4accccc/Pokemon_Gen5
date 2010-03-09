//============================================================================================
/**
 * @file	musical_local.h
 * @brief	ミュージカル内用定義
 * @author	ariizumi
 * @date	2008.11.14
 */
//============================================================================================

#pragma once

//デフォルトでROMに入っている配信データ
#define MUS_PROGRAM_LOCAL_NUM (4)

//ミュージカルデバッグ用
#define USE_MUSICAL_EDIT (1)

//ミュージカルポケモンの装備箇所(パレット対応
typedef enum
{
	MUS_POKE_PLT_SHADOW		=0x04,	//影用座標
	MUS_POKE_PLT_ROTATE		=0x05,	//回転用座標
	
	MUS_POKE_PLT_EAR_R		=0x07,	//右耳
	MUS_POKE_PLT_EAR_L		=0x08,	//左耳
	MUS_POKE_PLT_HEAD		=0x09,	//頭
	MUS_POKE_PLT_EYE		=0x0A,	//目
	MUS_POKE_PLT_FACE		=0x0B,	//顔
	MUS_POKE_PLT_BODY		=0x0C,	//胴
	MUS_POKE_PLT_WAIST		=0x0D,	//腰
	MUS_POKE_PLT_HAND_R		=0x0E,	//右手
	MUS_POKE_PLT_HAND_L		=0x0F,	//左手
	
	MUS_POKE_PLT_START = MUS_POKE_PLT_EAR_R,	//開始番号
}MUS_POKE_EQUIP_PLT;

//装備の最大傾斜
#define MUS_POKE_EQUIP_ANGLE_MAX (15*0x10000/360)

//パレット番号から装備箇所へ変換
#define MUS_POKE_PLT_TO_POS(val) ((MUS_POKE_EQUIP_POS)(val-MUS_POKE_PLT_START))
//グループ番号から回転へ変換(32等分)
//#define MUS_POKE_GRP_TO_ROT(val) (val*(360/32)*65536/360);
#define MUS_POKE_GRP_TO_ROT(val) (val*(65536/32))	//360を通分

//データパック
typedef struct
{
  u32 seqSize;
  u32 bankSize;
  u32 waveSize;
}MUS_DIST_MIDI_HEADER;

typedef struct
{
  u8 programNo;
  
  void *programData;
  void *messageData;
  void *scriptData;
  void *midiData; //↓三つ＋サイズヘッダをパックしてある
  void *midiSeqData;
  void *midiBnkData;
  void *midiWaveData;

  u32  programDataSize;
  u32  messageDataSize;
  u32  scriptDataSize;
  u32  midiDataSize;
}MUSICAL_DISTRIBUTE_DATA;

MUSICAL_DISTRIBUTE_DATA* MUSICAL_SYSTEM_InitDistributeData( HEAPID workHeapId );
void MUSICAL_SYSTEM_TermDistributeData( MUSICAL_DISTRIBUTE_DATA *distData );
void MUSICAL_SYSTEM_LoadDistributeData( MUSICAL_DISTRIBUTE_DATA *distData , SAVE_CONTROL_WORK *saveWork , const u8 programNo , HEAPID strmHeapId);
