//======================================================================
/**
 *
 * @file	field_comm_actor.c
 * @brief	
 * @author
 * @data
 *
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "net/network_define.h"
#include "arc_def.h"

#include "field_main.h"
#include "field_common.h"
#include "field_actor.h"

#include "field_comm_actor.h"
#include "test_graphic/fld_act.naix"

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
///	ビルボードアクターアニメ番号
//--------------------------------------------------------------
enum
{
	PCACTSTOP_UP = 0,
	PCACTSTOP_DOWN,
	PCACTSTOP_LEFT,
	PCACTSTOP_RIGHT,

	PCACTWALK_UP,
	PCACTWALK_DOWN,
	PCACTWALK_LEFT,
	PCACTWALK_RIGHT,

	PCACTRUN_UP,
	PCACTRUN_DOWN,
	PCACTRUN_LEFT,
	PCACTRUN_RIGHT,

	PCACTJUMP_UP,
	PCACTJUMP_DOWN,
	PCACTJUMP_LEFT,
	PCACTJUMP_RIGHT,

	PCACTANMNO_MAX,
};

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FLD_COMM_ACTOR
//--------------------------------------------------------------
struct _TAG_FLD_COMM_ACTOR
{
	const PLAYER_WORK *player;
	
	HEAPID heapID;
	u16 dir;
	u16 old_dir;
	int anm_id;
	int next_anm_id;
	u32 id;
	VecFx32 pos;
	VecFx32 old_pos;
	
	GFL_BBDACT_SYS *bbdActSys;
	GFL_BBDACT_RESUNIT_ID	bbdActResUnitID;
	GFL_BBDACT_ACTUNIT_ID	bbdActActUnitID;
};

//======================================================================
//	proto
//======================================================================
static void commActBBDActFunc(
	GFL_BBDACT_SYS *bbdActSys, int actIdx, void* work );

static const GFL_BBDACT_RESDATA playerBBDactResData;
static const GFL_BBDACT_ANM *playerBBDactAnmTable[PCACTANMNO_MAX];

//======================================================================
//	通信用アクター
//======================================================================
//--------------------------------------------------------------
/**
 * 通信用アクター　初期化
 * @param	player		参照するPLAYER_WORK *
 * @param	bbdActSys	使用するGFL_BBDACT_SYS
 * @param	resUnitID	使用するGFL_BBDACT_RESUNIT_ID
 * @param	heapID		リソース確保用ヒープID
 * @retval	FLD_COMM_ACTOR	追加されたFLD_COMM_ACTOR *
 */
//--------------------------------------------------------------
FLD_COMM_ACTOR * FldCommActor_Init(
	const PLAYER_WORK *player,
	GFL_BBDACT_SYS *bbdActSys,
	GFL_BBDACT_RESUNIT_ID TestresUnitID, HEAPID heapID, u32 id )
{
	u32 dir;
	FLD_COMM_ACTOR *act;
	GFL_BBDACT_ACTDATA actData;
	GFL_BBDACT_RESUNIT_ID resUnitID;
	const VecFx32 *pos;
	
	pos = PLAYERWORK_getPosition( player );
	dir = PLAYERWORK_getDirection( player );
	
	resUnitID = GFL_BBDACT_AddResourceUnit(
			bbdActSys, &playerBBDactResData, 1 );
	
	act = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLD_COMM_ACTOR) );
	act->heapID = heapID;
	act->dir = dir;
	act->old_dir = DIR_NOT;
	act->pos = *pos;
	act->old_pos = *pos;
	act->bbdActSys = bbdActSys;
	act->player = player;	//追加しましたAri1114
	act->anm_id = -1;
	act->id = id;
	act->bbdActResUnitID = resUnitID;

	actData.resID = 0;
	actData.sizX = FX16_ONE*8-1;
	actData.sizY = FX16_ONE*8-1;
	actData.trans.x = 0;
	actData.trans.y = 0;
	actData.trans.z = 0;
	actData.alpha = 31;
	actData.drawEnable = TRUE;
	actData.func = commActBBDActFunc;
	actData.work = act;
	
	act->bbdActActUnitID =
		GFL_BBDACT_AddAct( bbdActSys, resUnitID, &actData, 1 ); 
	
	GFL_BBDACT_BindActTexResLoad(
		bbdActSys, act->bbdActActUnitID,
		ARCID_FLDMAP_ACTOR, NARC_fld_act_hero_nsbtx );
	GFL_BBDACT_SetAnimeTable(
		bbdActSys, act->bbdActActUnitID,
		playerBBDactAnmTable, PCACTANMNO_MAX );
	GFL_BBDACT_SetAnimeIdxOn( bbdActSys, act->bbdActActUnitID, 0 );
	
	return( act );
}

//--------------------------------------------------------------
/**
 * フィールド通信用アクター　削除
 * @param	
 * @retval
 */
//--------------------------------------------------------------
void FldCommActor_Delete( FLD_COMM_ACTOR *act )
{
	GFL_BBDACT_RemoveResourceUnit( act->bbdActSys, act->bbdActResUnitID, 1 );
	GFL_HEAP_FreeMemory( act );
}

//======================================================================
//	パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド通信用アクター　更新
 * @param	act	FLD_COMM_ACTOR *
 * @param	pos 表示座標
 * @param	dir 表示方向
 * @retval	nothing
 */
//--------------------------------------------------------------
void FldCommActor_Update( FLD_COMM_ACTOR *act )
{
	u16 dir;
	int anmBase;
	const VecFx32 *pos;
	
	pos = PLAYERWORK_getPosition( act->player );
	dir = PLAYERWORK_getDirection( act->player );
	
	act->old_pos = act->pos;
	act->pos = *pos;
	act->old_dir = act->dir;
	act->dir = dir;
	
	if( act->pos.x != act->old_pos.x ||
		act->pos.y != act->old_pos.y ||
		act->pos.z != act->old_pos.z ){
		anmBase = 4;
	}else{
		anmBase = 0;
	}
	
	act->next_anm_id = anmBase + dir;
}

//======================================================================
///	ビルボードアクター
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド通信用アクター　アクター処理
 * @param	
 * @retval
 */
//--------------------------------------------------------------
static void commActBBDActFunc(
	GFL_BBDACT_SYS *bbdActSys, int actIdx, void* work )
{
	VecFx32 pos;
	FLD_COMM_ACTOR *act = work;
	
	pos = act->pos;
	pos.x += FX32_ONE * 16;
	pos.y += FX32_ONE * 10;
//	pos.z += 1;

	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(act->bbdActSys), actIdx, &pos );

	if( act->anm_id != act->next_anm_id ){
		act->anm_id = act->next_anm_id;
		GFL_BBDACT_SetAnimeIdx(
			act->bbdActSys, actIdx, act->anm_id );
	}
}

//======================================================================
//	data
//======================================================================
//--------------------------------------------------------------
///	ビルボードリソース
//--------------------------------------------------------------
static const GFL_BBDACT_RESDATA playerBBDactResData =
{
	ARCID_FLDMAP_ACTOR, NARC_fld_act_tex32x32_nsbtx,
	GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x1024,
	32, 32, GFL_BBDACT_RESTYPE_DATACUT 
};

//--------------------------------------------------------------
///	ビルボードアクターアニメ
//--------------------------------------------------------------
static const GFL_BBDACT_ANM PCstopLAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopRAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopUAnm[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopDAnm[] = {
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCwalkLAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkRAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkUAnm[] = {
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 20, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkDAnm[] = {
	{ 22, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 23, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCrunLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 13, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCjumpLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};

static const GFL_BBDACT_ANM* playerBBDactAnmTable[] = { 
	PCstopUAnm, PCstopDAnm, PCstopLAnm, PCstopRAnm,
	PCwalkUAnm, PCwalkDAnm, PCwalkLAnm, PCwalkRAnm,
	PCrunUAnm, PCrunDAnm, PCrunLAnm, PCrunRAnm,
	PCjumpUAnm, PCjumpDAnm, PCjumpLAnm, PCjumpRAnm,
};
