//=============================================================================================
/**
 * @file	btl_action.h
 * @brief	ポケモンWBバトル プレイヤーが選択した動作内容の受け渡し構造体定義
 * @author	taya
 *
 * @date	2008.10.06	作成
 */
//=============================================================================================
#ifndef __BTL_ACTION_H__
#define __BTL_ACTION_H__

//--------------------------------------------------------------
/**
 *	コマンド選択
 */
//--------------------------------------------------------------
typedef enum {
	BTL_ACTION_FIGHT,
	BTL_ACTION_ITEM,
	BTL_ACTION_CHANGE,
	BTL_ACTION_ESCAPE,

	BTL_ACTION_NULL,	///< 何もしない（相手のポケモン選択待ち中など）
}BtlAction;


typedef union {

	struct {
		u32 cmd		: 3;
		u32 param	: 29;
	}gen;

	struct {
		u32 cmd			: 3;
		u32 wazaIdx		: 3;
		u32 targetIdx	: 3;
	}fight;

	struct {
		u32 cmd			: 3;
		u32 number		: 16;
		u32 targetIdx	: 3;
	}item;

	struct {
		u32 cmd			: 3;
		u32 memberIdx	: 3;
	}change;

	struct {
		u32 cmd			: 3;
	}escape;


}BTL_ACTION_PARAM;


static void BTL_ACTION_SetFightParam( BTL_ACTION_PARAM* p, u8 wazaIdx, u8 targetIdx )
{
	p->fight.cmd = BTL_ACTION_FIGHT;
	p->fight.wazaIdx = wazaIdx;
	p->fight.targetIdx = targetIdx;
}

static void BTL_ACTION_SetItemParam( BTL_ACTION_PARAM* p, u16 itemNumber, u8 targetIdx )
{
	p->item.cmd = BTL_ACTION_ITEM;
	p->item.number = itemNumber;
	p->item.targetIdx = targetIdx;
}

static void BTL_ACTION_SetChangeParam( BTL_ACTION_PARAM* p, u8 memberIdx )
{
	p->change.cmd = BTL_ACTION_CHANGE;
	p->change.memberIdx = memberIdx;
}

static void BTL_ACTION_SetEscapeParam( BTL_ACTION_PARAM* p )
{
	p->escape.cmd = BTL_ACTION_ESCAPE;
}

static void BTL_ACTION_SetNULL( BTL_ACTION_PARAM* p )
{
	p->gen.cmd = BTL_ACTION_NULL;
}

#endif

