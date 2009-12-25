//============================================================================
/**
 *@file		tr_ai_hp_check.s
 *@brief	トレーナーAI
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

	.text
	.include	"tr_ai_seq_def.h"

//========================================================
//	HPで判断
//========================================================
HpCheckAISeq:
	IF_MIKATA_ATTACK	HpCheckAI_end			//対象が味方なら終了

	IF_HP_OVER	CHECK_ATTACK,70,HpCheckAI_my70
	IF_HP_OVER	CHECK_ATTACK,30,HpCheckAI_my30

	CHECK_WAZASEQNO
	IF_TABLE_JUMP	HpCheckAI_my00_ng_tbl,HpCheckAI_my_ng
	JUMP		HpCheckAI_ene

HpCheckAI_my70:
	CHECK_WAZASEQNO
	IF_TABLE_JUMP	HpCheckAI_my70_ng_tbl,HpCheckAI_my_ng
	JUMP		HpCheckAI_ene

HpCheckAI_my30:
	CHECK_WAZASEQNO
	IF_TABLE_JUMP	HpCheckAI_my30_ng_tbl,HpCheckAI_my_ng
	JUMP		HpCheckAI_ene

HpCheckAI_my_ng:
	IF_RND_UNDER	50,HpCheckAI_ene
	INCDEC		-2

HpCheckAI_ene:
	IF_HP_OVER	CHECK_DEFENCE,70,HpCheckAI_ene70
	IF_HP_OVER	CHECK_DEFENCE,30,HpCheckAI_ene30

	//30未満
	CHECK_WAZASEQNO
	IF_TABLE_JUMP	HpCheckAI_ene00_ng_tbl,HpCheckAI_ene_ng
	JUMP		HpCheckAI_end

HpCheckAI_ene70://70以上
	CHECK_WAZASEQNO
	IF_TABLE_JUMP	HpCheckAI_ene70_ng_tbl,HpCheckAI_ene_ng
	JUMP		HpCheckAI_end

HpCheckAI_ene30://30以上
	CHECK_WAZASEQNO
	IF_TABLE_JUMP	HpCheckAI_ene30_ng_tbl,HpCheckAI_ene_ng
	JUMP		HpCheckAI_end

HpCheckAI_ene_ng:
	IF_RND_UNDER	50,HpCheckAI_end
	INCDEC		-2


HpCheckAI_end:
	AIEND



HpCheckAI_my70_ng_tbl://71以上あるときに
	.long	7,32,37,98,99,116,132
	.long	168,194
	.long	214,220,270
	.long	0xffffffff
//------------------------------------
//テーブル調整2006.6.16
//133,134,157未使用なので削除
//204使用しても問題ないので削除
//214,220,270ＤＰより追加
//------------------------------------


HpCheckAI_my30_ng_tbl://31以上70未満
	.long	7,10,11,12,13,14,15,16,18,19
	.long	20,21,22,23,24,26,30,35,46,47
	.long	50,51,52,53,54,55,56,58,59,60
	.long	61,62,63,64,93,124,142,205,206,208
	.long	211,212
	.long	240,243,244,265
	.long	0xffffffff
//------------------------------------
//テーブル調整2006.6.16
//240,243,244,265ＤＰより追加
//------------------------------------


HpCheckAI_my00_ng_tbl://30以下
	.long	10,11,12,13,14,15,16,18,19,20
	.long	21,22,23,24,26,30,35,46,47,50
	.long	51,52,53,54,55,56,58,59,60,61
	.long	62,63,64,81,93,94,124,142,143,144
	.long	190,205,206,208,211,212
	.long	201,210			//追加2006.6.16
	.long	226,227,265		//追加2006.6.16
	.long	0xffffffff

//------------------------------------
//テーブル調整2006.6.16
//151ソーラービーム使用しても問題ないので削除
//201,210		追加2006.6.16
//226,227,265	DP追加2006.6.16
//------------------------------------

HpCheckAI_ene70_ng_tbl://71以上
	.long	0xffffffff

HpCheckAI_ene30_ng_tbl://31以上71未満
	.long	10,11,12,13,14,15,16,18,19,20
	.long	21,22,23,24,46,47,50,51,52,53
	.long	54,55,56,58,59,60,61,62,63,64
	.long	66,91,114,124,205,206,208,211,212
	.long	226,237,265	
	.long	0xffffffff
//------------------------------------
//テーブル調整2006.6.16
//226,227,265	DP追加2006.6.16
//------------------------------------


HpCheckAI_ene00_ng_tbl://31未満
	.long	1,7,10,11,12,13,14,15,16,18
	.long	19,20,21,22,23,24,26,30,33,35
	.long	38,40,40,46,47,49,50,51,52,53
	.long	54,55,56,58,59,60,61,62,63,64
	.long	66,67,91,93,94,100,114,118,119,120
	.long	124,143,144,167,205,206,208,211,212
	.long	226,237,265	
	.long	0xffffffff

//------------------------------------
//テーブル調整2006.6.16
//226,227,265	DP追加2006.6.16
//------------------------------------

