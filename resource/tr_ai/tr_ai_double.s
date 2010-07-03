//============================================================================
/**
 *@file		tr_ai_double.s
 *@brief	トレーナーAI
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

	.text
	.include	"tr_ai_seq_def.h"

//========================================================
//	ダブルバトル
//========================================================
DoubleAISeq:
  CHECK_BTL_RULE
	IF_EQUAL		BTL_RULE_DOUBLE,DoubleAISeq_next
	IF_EQUAL		BTL_RULE_TRIPLE,DoubleAISeq_next

  AIEND

DoubleAISeq_next:
	IF_MIKATA_ATTACK	DoubleAI_MineSeq

//--------------------------------------
//	敵が対象
//--------------------------------------
	COMP_POWER	LOSS_CALC_OFF
	IF_EQUAL	COMP_POWER_NONE, DoubleAI_StrongAI_end	//威力なしの技
	
	//敵２体いるので相性の悪い方は避ける
	//たたし相性０倍以外関係ない固定技は相性チェックをスルーする
	IF_WAZA_SEQNO_JUMP	38,DoubleAI_StrongAI_START	//	いちげきひっさつ
	IF_WAZA_SEQNO_JUMP	41,DoubleAI_StrongAI_START	//　りゅうのいかり
	IF_WAZA_SEQNO_JUMP	87,DoubleAI_StrongAI_START	//　ナイトヘッド
	IF_WAZA_SEQNO_JUMP	88,DoubleAI_StrongAI_START	//　サイコウエーブ
	IF_WAZA_SEQNO_JUMP	130,DoubleAI_StrongAI_START	//　ソニックブーム
	
	CHECK_WAZA_AISYOU	AISYOU_1_2BAI,DoubleAI_Aisyou_Dec1	//1/2
	CHECK_WAZA_AISYOU	AISYOU_1_4BAI,DoubleAI_Aisyou_Dec2	//1/4
	JUMP	DoubleAI_StrongAI_START
	
DoubleAI_Aisyou_Dec1://1/2相性悪い相手には出しづらい（1/4の確率）
	IF_WAZA_HINSHI	LOSS_CALC_OFF,DoubleAI_StrongAI_START	//瀕死にできるならスルー
	IF_HP_EQUAL	CHECK_DEFENCE_FRIEND,0,DoubleAI_StrongAI_START	//相手が１体ならスルー
	IF_RND_UNDER	64, DoubleAI_StrongAI_START
	INCDEC	-1
	JUMP	DoubleAI_StrongAI_START
	
DoubleAI_Aisyou_Dec2://1/4相性悪い相手には出しづらい（1/4の確率）
	IF_WAZA_HINSHI	LOSS_CALC_OFF,DoubleAI_StrongAI_START	//瀕死にできるならスルー
	IF_HP_EQUAL	CHECK_DEFENCE_FRIEND,0,DoubleAI_StrongAI_START	//相手が１体ならスルー
	IF_RND_UNDER	64, DoubleAI_StrongAI_START
	INCDEC	-2
	JUMP	DoubleAI_StrongAI_START

DoubleAI_StrongAI_START:

//自分の技がパートナーと比べて一番高い技----------------------------------------
	COMP_POWER_WITH_PARTNER	LOSS_CALC_OFF
	IFN_EQUAL	COMP_POWER_TOP, DoubleAI_StrongAICheck_Aisyou	//相性チェック

DoubleAI_StrongCheck:
	IF_WAZA_SEQNO_JUMP	7,DoubleAI_StrongAI_end		// 自爆
	IF_WAZA_SEQNO_JUMP	103,DoubleAI_StrongAICheck2	// 先制
	IF_RND_UNDER	128,DoubleAI_StrongAICheck_Aisyou// 相性チェック

	INCDEC		1
	JUMP	DoubleAI_StrongAI_end	//終了
	
DoubleAI_StrongAICheck2:
	IF_RND_UNDER	50, DoubleAI_StrongAICheck_Aisyou	//相性チェック
	INCDEC		1
	JUMP	DoubleAI_StrongAI_end	//終了


//一番高くなくても相性チェックして２倍以上なら出しやすくする
DoubleAI_StrongAICheck_Aisyou:
	IF_WAZA_SEQNO_JUMP	38,DoubleAI_StrongAI_end	//	いちげきひっさつ
	IF_WAZA_SEQNO_JUMP	41,DoubleAI_StrongAI_end	//　りゅうのいかり
	IF_WAZA_SEQNO_JUMP	87,DoubleAI_StrongAI_end	//　ナイトヘッド
	IF_WAZA_SEQNO_JUMP	88,DoubleAI_StrongAI_end	//　サイコウエーブ
	IF_WAZA_SEQNO_JUMP	130,DoubleAI_StrongAI_end	//　ソニックブーム
	
	CHECK_WAZA_AISYOU	AISYOU_2BAI,DoubleAI_StrongAICheck_Aisyou2	//2
	CHECK_WAZA_AISYOU	AISYOU_4BAI,DoubleAI_StrongAICheck_Aisyou4	//4
	JUMP	DoubleAI_StrongAI_end	//終了

DoubleAI_StrongAICheck_Aisyou2://約60%
	IF_RND_UNDER	100, DoubleAI_StrongAI_end
	INCDEC		1
	JUMP	DoubleAI_StrongAI_end

DoubleAI_StrongAICheck_Aisyou4://75%
	IF_RND_UNDER	64, DoubleAI_StrongAI_end
	INCDEC		1
	JUMP	DoubleAI_StrongAI_end



DoubleAI_StrongAI_end:

//--------------------------------------------------------------------------------

	IF_WAZA_SEQNO_JUMP	313,DoubleEnemyAI_Tomoenage       	//　ともえなげ　2010/4/23
	IF_WAZA_SEQNO_JUMP	190,DoubleEnemyAI_Hunka       	    //　ふんか　    2010/4/23

	IF_WAZANO	WAZANO_HUBUKI,DoubleEnemyAI_Hubuki	// ふぶき　    2010/4/23 
	IF_WAZANO	WAZANO_HUBUKI,DoubleEnemyAI_Iwanadare	// いわなだれ　 2010/4/23 
	IF_WAZANO	WAZANO_HUBUKI,DoubleEnemyAI_Daakuhouru	// ダークホール　2010/4/23 

	IF_WAZANO	WAZANO_WAIDOGAADO,DoubleEnemyAI_Waidogaado		// ワイドガード　2010/4/23
	IF_WAZANO	WAZANO_RINSYOU,DoubleEnemyAI_Rinsyou      	//　りんしょう　2010/4/23 
	IF_WAZANO	WAZANO_SAIDOTHENZI,DoubleEnemyAI_Sidechange      	//　サイドチェンジ　2010/4/23 
	IF_WAZANO	WAZANO_SAKIOKURI,DoubleEnemyAI_Sakiokuri      	//　さきおくり　2010/4/23 
	IF_WAZANO	WAZANO_GIHUTOPASU,DoubleEnemyAI_Gihutopasu      	//　ギフトパス　2010/4/23 

	IF_WAZANO	WAZANO_SUKIRUSUWAPPU,DoubleEnemyAI_SkillSwap	// ｽｷﾙｽﾜｯﾌﾟ2006.6.14ok
	CHECK_TYPE	CHECK_WAZA
	IF_WAZANO	WAZANO_ZISIN, DoubleEnemyAI_Zisin				// 地震2006.6.14ok
	IF_WAZANO	WAZANO_MAGUNITYUUDO, DoubleEnemyAI_Zisin		// マグニチュード2006.6.14ok

	IF_WAZANO	WAZANO_MIRAIYOTI, DoubleEnemyAI_Miraiyoti		// みらいよち2006.6.14ok
	IF_WAZANO	WAZANO_HAMETUNONEGAI, DoubleEnemyAI_Miraiyoti	// はめつのねがい2006.6.14ok


	IF_WAZANO	WAZANO_AMAGOI, DoubleEnemyAI_Amagoi				// あまごい2006.6.14ok
	IF_WAZANO	WAZANO_NIHONBARE, DoubleEnemyAI_Nihonbare		// にほんばれ2006.6.14ok
	IF_WAZANO	WAZANO_ARARE, DoubleEnemyAI_Arare				// あられ2006.6.14ok
	IF_WAZANO	WAZANO_SUNAARASI, DoubleEnemyAI_Sunaarasi		// すなあらし2006.6.14ok
	
	IF_WAZANO	WAZANO_ZYUURYOKU, DoubleEnemyAI_Zyuuryoku		// じゅうりょく2006.6.23ok
	IF_WAZANO	WAZANO_TORIKKURUUMU, DoubleEnemyAI_Torikkuruumu	// トリックルーム2006.6.23ok
	IF_WAZANO	WAZANO_KONOYUBITOMARE, DoubleEnemyAI_Konoyubitomare	// このゆびとまれ2006.6.14ok
	
	IF_WAZANO	WAZANO_IYASINOHADOU, DoubleEnemyAI_Iyasinohadou		// いやしのはどう　2010.6.25
	IF_WAZANO	WAZANO_OSAKINIDOUZO, DoubleEnemyAI_Osakinidouzo		// おさきにどうぞ　2010.6.25

	CHECK_TYPE	CHECK_WAZA
	IF_EQUAL	POKETYPE_DENKI,DoubleEnemyAI_ElectricType		// 電気技2006.6.14ok
	IF_EQUAL	POKETYPE_HONOO,DoubleEnemyAI_FireType				// 炎技2006.6.14ok
	IF_EQUAL	POKETYPE_MIZU,DoubleEnemyAI_WaterType				// 水技2006.6.14

	//CHECK_TOKUSEI	CHECK_ATTACK
	//IFN_EQUAL	TOKUSYU_KONZYOU, DoubleEnemyAI_Konzyou	//塞ぐ
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_TEDASUKE, DoubleEnemyAI_Tedasuke	//NG
	
	AIEND

//2006.6.14
//あまごい
DoubleEnemyAI_Amagoi:				
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_URUOIBODHI, DoubleEnemyAI_Amagoi_Mine1
	IF_EQUAL	TOKUSYU_KANSOUHADA, DoubleEnemyAI_Amagoi_Mine2
	JUMP	DoubleEnemyAI_Amagoi_Friend

DoubleEnemyAI_Amagoi_Mine1:
	IFN_POKESICK	CHECK_ATTACK,DoubleEnemyAI_Amagoi_Friend

DoubleEnemyAI_Amagoi_Mine2:
	INCDEC	2
	JUMP	DoubleEnemyAI_Amagoi_Friend

DoubleEnemyAI_Amagoi_Friend:
	//CHECK_TOKUSEI	CHECK_ATTACK_FRIEND
	//IF_EQUAL	TOKUSYU_URUOIBODHI, DoubleEnemyAI_Amagoi_Friend1
	//IF_EQUAL	TOKUSYU_KANSOUHADA, DoubleEnemyAI_Amagoi_Friend2
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_URUOIBODHI
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Amagoi_Friend1
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_KANSOUHADA
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Amagoi_Friend2
	JUMP	DoubleEnemyAI_Amagoi_End

DoubleEnemyAI_Amagoi_Friend1:
	IFN_POKESICK	CHECK_ATTACK_FRIEND,DoubleEnemyAI_Amagoi_End

DoubleEnemyAI_Amagoi_Friend2:
	INCDEC	2
	JUMP	DoubleEnemyAI_Amagoi_End

DoubleEnemyAI_Amagoi_End:
	AIEND


//にほんばれ	
DoubleEnemyAI_Nihonbare:			
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_RIIHUGAADO, DoubleEnemyAI_Nihonbare_Mine1	//ok
	IF_EQUAL	TOKUSYU_HURAWAAGIHUTO, DoubleEnemyAI_Nihonbare_Mine2//ok
	IF_EQUAL	TOKUSYU_KANSOUHADA, DoubleEnemyAI_Nihonbare_Mine3	//ok
	IF_EQUAL	TOKUSYU_SANPAWAA, DoubleEnemyAI_Nihonbare_Mine4		//ok
	JUMP	DoubleEnemyAI_Nihonbare_Friend

DoubleEnemyAI_Nihonbare_Mine1:
	IF_POKESICK	CHECK_ATTACK,DoubleEnemyAI_Nihonbare_Friend
	IF_HP_UNDER	CHECK_ATTACK,30,DoubleEnemyAI_Nihonbare_Friend

DoubleEnemyAI_Nihonbare_Mine2:
	INCDEC	2
	JUMP	DoubleEnemyAI_Nihonbare_Friend

DoubleEnemyAI_Nihonbare_Mine3:
	INCDEC	-2
	JUMP	DoubleEnemyAI_Nihonbare_Friend

DoubleEnemyAI_Nihonbare_Mine4:
	IF_HP_UNDER	CHECK_ATTACK,50,DoubleEnemyAI_Nihonbare_Mine4_1
	INCDEC	1

DoubleEnemyAI_Nihonbare_Mine4_1:
	IF_RND_UNDER	128, DoubleEnemyAI_Nihonbare_Friend
	INCDEC	-2

DoubleEnemyAI_Nihonbare_Friend:
	//CHECK_TOKUSEI	CHECK_ATTACK_FRIEND
	//IF_EQUAL	TOKUSYU_RIIHUGAADO, DoubleEnemyAI_Nihonbare_Friend1
	//IF_EQUAL	TOKUSYU_HURAWAAGIHUTO, DoubleEnemyAI_Nihonbare_Friend2
	//IF_EQUAL	TOKUSYU_KANSOUHADA, DoubleEnemyAI_Nihonbare_Friend3
	//IF_EQUAL	TOKUSYU_SANPAWAA, DoubleEnemyAI_Nihonbare_Friend4
	
	//特性直接見
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_RIIHUGAADO
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Nihonbare_Friend1
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_HURAWAAGIHUTO
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Nihonbare_Friend2
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_KANSOUHADA
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Nihonbare_Friend3
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_SANPAWAA
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Nihonbare_Friend4
	
	JUMP	DoubleEnemyAI_Nihonbare_End

DoubleEnemyAI_Nihonbare_Friend1:
	IF_POKESICK	CHECK_ATTACK_FRIEND,DoubleEnemyAI_Nihonbare_End
	IF_HP_UNDER	CHECK_ATTACK_FRIEND,30,DoubleEnemyAI_Nihonbare_End

DoubleEnemyAI_Nihonbare_Friend2:
	INCDEC	2
	JUMP	DoubleEnemyAI_Nihonbare_End

DoubleEnemyAI_Nihonbare_Friend3:
	INCDEC	-2
	JUMP	DoubleEnemyAI_Nihonbare_End

DoubleEnemyAI_Nihonbare_Friend4:
	IF_HP_UNDER	CHECK_ATTACK_FRIEND,50,DoubleEnemyAI_Nihonbare_Friend4_1
	INCDEC	1

DoubleEnemyAI_Nihonbare_Friend4_1:
	IF_RND_UNDER	128, DoubleEnemyAI_Nihonbare_End
	INCDEC	-2

DoubleEnemyAI_Nihonbare_End:
	AIEND

//あられ
DoubleEnemyAI_Arare:			
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_AISUBODHI, DoubleEnemyAI_Arare_Mine1	//ok
	IF_EQUAL	TOKUSYU_YUKIGAKURE, DoubleEnemyAI_Arare_Mine1	//ok
	IF_HAVE_WAZA	CHECK_ATTACK, WAZANO_HUBUKI, DoubleEnemyAI_Arare_Mine1	//ok
	JUMP	DoubleEnemyAI_Arare_Friend

DoubleEnemyAI_Arare_Mine1:
	INCDEC	+2

DoubleEnemyAI_Arare_Friend:	
	//CHECK_TOKUSEI	CHECK_ATTACK_FRIEND
	//IF_EQUAL	TOKUSYU_AISUBODHI, DoubleEnemyAI_Arare_Friend1
	//IF_EQUAL	TOKUSYU_YUKIGAKURE, DoubleEnemyAI_Arare_Friend1
	
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_AISUBODHI
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Arare_Friend1
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_YUKIGAKURE
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Arare_Friend1
	
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_HUBUKI, DoubleEnemyAI_Arare_Friend1
	
	JUMP	DoubleEnemyAI_Arare_End

DoubleEnemyAI_Arare_Friend1:
	INCDEC	+2

DoubleEnemyAI_Arare_End:	
	AIEND

//すなあらし
DoubleEnemyAI_Sunaarasi:			
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_SUNAGAKURE, DoubleEnemyAI_Sunaarasi_Mine1
	
	CHECK_TYPE	CHECK_ATTACK_TYPE1
	IF_EQUAL	POKETYPE_IWA, DoubleEnemyAI_Sunaarasi_Mine1
	CHECK_TYPE	CHECK_ATTACK_TYPE2
	IF_EQUAL	POKETYPE_IWA, DoubleEnemyAI_Sunaarasi_Mine1
	JUMP	DoubleEnemyAI_Sunaarasi_Friend

	
DoubleEnemyAI_Sunaarasi_Mine1:
	INCDEC	+2
	JUMP	DoubleEnemyAI_Sunaarasi_Friend


DoubleEnemyAI_Sunaarasi_Friend:	
	//CHECK_TOKUSEI	CHECK_ATTACK
	//IF_EQUAL	TOKUSYU_SUNAGAKURE, DoubleEnemyAI_Sunaarasi_Friend1
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_SUNAGAKURE
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Sunaarasi_Friend1
	
	//味方のタイプをチェックが必要
	CHECK_TYPE	CHECK_ATTACK_FRIEND_TYPE1
	IF_EQUAL	POKETYPE_IWA, DoubleEnemyAI_Sunaarasi_Friend1
	CHECK_TYPE	CHECK_ATTACK_FRIEND_TYPE2
	IF_EQUAL	POKETYPE_IWA, DoubleEnemyAI_Sunaarasi_Friend1
	JUMP	DoubleEnemyAI_Sunaarasi_End

DoubleEnemyAI_Sunaarasi_Friend1:
	INCDEC	+2

DoubleEnemyAI_Sunaarasi_End:	
	AIEND


//じゅうりょく
DoubleEnemyAI_Zyuuryoku:
	FLDEFF_CHECK	BTL_FLDEFF_JURYOKU,DoubleMineAI_end	//じゅうりょく中はやらない	

DoubleEnemyAI_Zyuuryoku_mine://自分が浮遊＆飛行
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK,TOKUSYU_HUYUU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_mine_ng
	CHECK_HAVE_TYPE		CHECK_ATTACK,POKETYPE_HIKOU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_mine_ng
	IF_WAZASICK	CHECK_ATTACK,WAZASICK_FLYING,DoubleEnemyAI_Zyuuryoku_mine_ng
	JUMP	DoubleEnemyAI_Zyuuryoku_mine_friend

DoubleEnemyAI_Zyuuryoku_mine_ng:
	INCDEC	-5
	JUMP	DoubleEnemyAI_Zyuuryoku_mine_friend

DoubleEnemyAI_Zyuuryoku_mine_friend://パートナーが浮遊＆飛行
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_HUYUU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_mine_friend_ng
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_HIKOU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_mine_friend_ng
	IF_WAZASICK	CHECK_ATTACK_FRIEND,WAZASICK_FLYING,DoubleEnemyAI_Zyuuryoku_mine_friend_ng
	JUMP	DoubleEnemyAI_Zyuuryoku_enemy

DoubleEnemyAI_Zyuuryoku_mine_friend_ng:
	INCDEC	-5
	JUMP	DoubleEnemyAI_Zyuuryoku_enemy

DoubleEnemyAI_Zyuuryoku_enemy://敵が浮遊＆飛行
	CHECK_HAVE_TOKUSEI	CHECK_DEFENCE,TOKUSYU_HUYUU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_enemy_ok
	CHECK_HAVE_TYPE		CHECK_DEFENCE,POKETYPE_HIKOU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_enemy_ok
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_FLYING,DoubleEnemyAI_Zyuuryoku_enemy_ok
	JUMP	DoubleEnemyAI_Zyuuryoku_enemy_friend

DoubleEnemyAI_Zyuuryoku_enemy_ok:
	IF_RND_UNDER	64, DoubleEnemyAI_Zyuuryoku_enemy_friend
	INCDEC	+3
	JUMP	DoubleEnemyAI_Zyuuryoku_enemy_friend

DoubleEnemyAI_Zyuuryoku_enemy_friend://敵パートナーが浮遊＆飛行
	CHECK_HAVE_TOKUSEI	CHECK_DEFENCE_FRIEND,TOKUSYU_HUYUU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_enemy_friend_ok
	CHECK_HAVE_TYPE		CHECK_DEFENCE_FRIEND,POKETYPE_HIKOU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_enemy_friend_ok
	IF_WAZASICK	CHECK_DEFENCE_FRIEND,WAZASICK_FLYING,DoubleEnemyAI_Zyuuryoku_enemy_friend_ok
	JUMP	DoubleEnemyAI_Zyuuryoku_End

DoubleEnemyAI_Zyuuryoku_enemy_friend_ok:
	IF_RND_UNDER	64, DoubleEnemyAI_Zyuuryoku_End
	INCDEC	+3
	JUMP	DoubleEnemyAI_Zyuuryoku_End

DoubleEnemyAI_Zyuuryoku_End:
AIEND


//トリックルーム
DoubleEnemyAI_Torikkuruumu:
	//全員いない場合はやらない
	IF_HP_EQUAL	CHECK_ATTACK_FRIEND,0,AI_DEC30	//味方がいない
	IF_HP_EQUAL	CHECK_DEFENCE_FRIEND,0,AI_DEC30	//敵のパートナーがいない
	IF_HP_EQUAL	CHECK_DEFENCE,0,AI_DEC30		//敵がいない
	
	//自分とパートナーの繰り出し順位が３位、４位なら加算
	//自分とパートナーの繰り出し順位が１位、２位なら減算
	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IF_EQUAL	0,DoubleEnemyAI_Torikkuruumu_mine1
	IF_EQUAL	1,DoubleEnemyAI_Torikkuruumu_mine2
	IF_EQUAL	2,DoubleEnemyAI_Torikkuruumu_mine3
	IF_EQUAL	3,DoubleEnemyAI_Torikkuruumu_mine4
	JUMP	DoubleEnemyAI_Torikkuruumu_End


DoubleEnemyAI_Torikkuruumu_mine1:
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND		//味方の順位
	IF_EQUAL	1,AI_DEC30
	IF_EQUAL	0,AI_DEC30	//１位タイ
	JUMP	DoubleEnemyAI_Torikkuruumu_End1
	
DoubleEnemyAI_Torikkuruumu_mine2:
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND		//味方の順位
	IF_EQUAL	0,AI_DEC30
	JUMP	DoubleEnemyAI_Torikkuruumu_End1

DoubleEnemyAI_Torikkuruumu_mine3:
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND		//味方の順位
	IFN_EQUAL	3,DoubleEnemyAI_Torikkuruumu_End1
	IF_RND_UNDER	64, DoubleEnemyAI_Torikkuruumu_End1
	INCDEC	+5
	JUMP	DoubleEnemyAI_Torikkuruumu_End

DoubleEnemyAI_Torikkuruumu_mine4:
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND		//味方の順位
	IFN_EQUAL	2,DoubleEnemyAI_Torikkuruumu_End1
	IF_RND_UNDER	64, DoubleEnemyAI_Torikkuruumu_End1
	INCDEC	+5
	JUMP	DoubleEnemyAI_Torikkuruumu_End

DoubleEnemyAI_Torikkuruumu_End1:
	INCDEC	-5
DoubleEnemyAI_Torikkuruumu_End:
	AIEND


//このゆびとまれ
DoubleEnemyAI_Konoyubitomare:
	IF_HP_OVER	CHECK_ATTACK,90,DoubleEnemyAI_Konoyubitomare1
	IF_HP_OVER	CHECK_ATTACK,50,DoubleEnemyAI_Konoyubitomare2
	IF_HP_OVER	CHECK_ATTACK,30,DoubleEnemyAI_Konoyubitomare3
	IF_RND_UNDER	64, DoubleEnemyAI_Konoyubitomare_End
	JUMP	AI_DEC5


DoubleEnemyAI_Konoyubitomare1://自分残り90以上
	IF_HP_OVER	CHECK_ATTACK_FRIEND,90,DoubleEnemyAI_Konoyubitomare_ng1
	IF_HP_OVER	CHECK_ATTACK_FRIEND,50,DoubleEnemyAI_Konoyubitomare_ok1
	IF_HP_OVER	CHECK_ATTACK_FRIEND,30,DoubleEnemyAI_Konoyubitomare_ok2
	JUMP	DoubleEnemyAI_Konoyubitomare_ok3


DoubleEnemyAI_Konoyubitomare2://自分残り50以上90未満
	IF_HP_OVER	CHECK_ATTACK_FRIEND,90,DoubleEnemyAI_Konoyubitomare_ng2
	IF_HP_OVER	CHECK_ATTACK_FRIEND,50,DoubleEnemyAI_Konoyubitomare_ng1
	IF_HP_OVER	CHECK_ATTACK_FRIEND,30,DoubleEnemyAI_Konoyubitomare_ok1
	JUMP	DoubleEnemyAI_Konoyubitomare_ok2


DoubleEnemyAI_Konoyubitomare3://自分残り30以上50未満
	IF_HP_OVER	CHECK_ATTACK_FRIEND,90,DoubleEnemyAI_Konoyubitomare_ng2
	IF_HP_OVER	CHECK_ATTACK_FRIEND,50,DoubleEnemyAI_Konoyubitomare_ng2
	IF_HP_OVER	CHECK_ATTACK_FRIEND,30,DoubleEnemyAI_Konoyubitomare_ok1
	JUMP	DoubleEnemyAI_Konoyubitomare_ok2


DoubleEnemyAI_Konoyubitomare_ng1:
	IF_RND_UNDER	64, DoubleEnemyAI_Konoyubitomare_End
	INCDEC	-1
	JUMP	DoubleEnemyAI_Konoyubitomare_End

DoubleEnemyAI_Konoyubitomare_ng2:
	IF_RND_UNDER	64, DoubleEnemyAI_Konoyubitomare_End
	INCDEC	-2
	JUMP	DoubleEnemyAI_Konoyubitomare_End

DoubleEnemyAI_Konoyubitomare_ok1:
	IF_RND_UNDER	64, DoubleEnemyAI_Konoyubitomare_End
	INCDEC	+1
	JUMP	DoubleEnemyAI_Konoyubitomare_End

DoubleEnemyAI_Konoyubitomare_ok2:
	IF_RND_UNDER	64, DoubleEnemyAI_Konoyubitomare_End
	INCDEC	+2
	JUMP	DoubleEnemyAI_Konoyubitomare_End

DoubleEnemyAI_Konoyubitomare_ok3:
	IF_RND_UNDER	64, DoubleEnemyAI_Konoyubitomare_End
	INCDEC	+3
	JUMP	DoubleEnemyAI_Konoyubitomare_End

DoubleEnemyAI_Konoyubitomare_End:
	AIEND


//味方が「てだすけ」
DoubleEnemyAI_Tedasuke:
	IF_HP_OVER	CHECK_ATTACK,50,DoubleEnemyAI_Tedasuke1
	CHECK_AGI_RANK	CHECK_ATTACK      		//自分の順位
	IF_UNDER	1,DoubleEnemyAI_Tedasuke1

	JUMP	DoubleEnemyAI_Tedasuke_End

DoubleEnemyAI_Tedasuke1:
	IF_WAZA_SEQNO_JUMP	38,DoubleEnemyAI_Tedasuke_End		//	いちげきひっさつ
	IF_WAZA_SEQNO_JUMP	41,DoubleEnemyAI_Tedasuke_End		//　りゅうのいかり
	IF_WAZA_SEQNO_JUMP	87,DoubleEnemyAI_Tedasuke_End		//　ナイトヘッド
	IF_WAZA_SEQNO_JUMP	88,DoubleEnemyAI_Tedasuke_End		//　サイコウエーブ
	IF_WAZA_SEQNO_JUMP	130,DoubleEnemyAI_Tedasuke_End	//　ソニックブーム
	COMP_POWER	LOSS_CALC_OFF
	IF_EQUAL	COMP_POWER_NONE,DoubleEnemyAI_Tedasuke_End 

	IF_COMMONRND_UNDER	128,DoubleEnemyAI_Tedasuke_End
	INCDEC	+3

DoubleEnemyAI_Tedasuke_End:
	AIEND


// 自分が「こんじょう」
DoubleEnemyAI_Konzyou:
	IF_POKESICK	CHECK_ATTACK,DoubleEnemyAI_Konzyou1
	AIEND
DoubleEnemyAI_Konzyou1:
	COMP_POWER	LOSS_CALC_OFF
	IF_EQUAL	COMP_POWER_NONE, AI_DEC5
	INCDEC		1
	IF_EQUAL	COMP_POWER_TOP, AI_INC2
	AIEND


// 地震・ﾏｸﾞﾆﾁｭｰﾄﾞok
DoubleEnemyAI_Zisin:
	//味方がでんじふうをしているかどうか？2006.6.14
	IF_WAZASICK	CHECK_ATTACK_FRIEND,WAZASICK_FLYING,AI_INC2	// でんじふゆう中
	
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_HUYUU
	IF_EQUAL	HAVE_YES,	AI_INC2
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_HIKOU
	IF_EQUAL	HAVE_YES,	AI_INC2
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_HONOO
	IF_EQUAL	HAVE_YES,	AI_DEC10
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_DENKI
	IF_EQUAL	HAVE_YES,	AI_DEC10
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_DOKU
	IF_EQUAL	HAVE_YES,	AI_DEC10
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_IWA
	IF_EQUAL	HAVE_YES,	AI_DEC10
	JUMP	AI_DEC3

//　みらいよち
DoubleEnemyAI_Miraiyoti:
	IF_HP_EQUAL	CHECK_ATTACK_FRIEND,0,DoubleEnemyAI_Miraiyoti_End	//相手が１体ならスルー
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_MIRAIYOTI, DoubleEnemyAI_Miraiyoti_Dec
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_HAMETUNONEGAI, DoubleEnemyAI_Miraiyoti_Dec
	JUMP	DoubleEnemyAI_Miraiyoti_End

DoubleEnemyAI_Miraiyoti_Dec://遅い同じ時は控える
	CHECK_AGI_RANK	CHECK_ATTACK	
	IF_EQUAL	3,AI_DEC3						//4
	IF_EQUAL	2,DoubleEnemyAI_Miraiyoti_Rank3	//3
	IF_EQUAL	1,DoubleEnemyAI_Miraiyoti_Rank2	//2
	IF_EQUAL	0,DoubleEnemyAI_Miraiyoti_Rank1	//1	
	JUMP	DoubleEnemyAI_Miraiyoti_End

DoubleEnemyAI_Miraiyoti_Rank3:
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	
	IF_EQUAL	0,AI_DEC3	//1
	IF_EQUAL	1,AI_DEC3	//2
	IF_RND_UNDER	128, DoubleEnemyAI_Miraiyoti_End
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	
	IF_EQUAL	2,AI_DEC3	//3位タイ
	JUMP	DoubleEnemyAI_Miraiyoti_End

DoubleEnemyAI_Miraiyoti_Rank2:
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	
	IF_EQUAL	0,AI_DEC3	//1
	IF_RND_UNDER	128, DoubleEnemyAI_Miraiyoti_End
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	
	IF_EQUAL	1,AI_DEC3	//2位タイ
	JUMP	DoubleEnemyAI_Miraiyoti_End

DoubleEnemyAI_Miraiyoti_Rank1:
	IF_RND_UNDER	128, DoubleEnemyAI_Miraiyoti_End
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	
	IF_EQUAL	0,AI_DEC3	//1位タイ
	JUMP	DoubleEnemyAI_Miraiyoti_End

DoubleEnemyAI_Miraiyoti_End:
	AIEND


// ｽｷﾙｽﾜｯﾌﾟ2006.6.16ok
DoubleEnemyAI_SkillSwap:
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_NAMAKE, AI_INC5
	IF_EQUAL	TOKUSYU_SUROOSUTAATO, AI_INC5	//2006.6.14
	IF_EQUAL	TOKUSYU_ATODASI, AI_INC5		//2006.6.14
	IF_EQUAL	TOKUSYU_BUKIYOU, AI_INC5		//2006.6.14
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_KAGEHUMI, AI_INC2
	IF_EQUAL	TOKUSYU_YOGAPAWAA, AI_INC2
	IF_EQUAL	TOKUSYU_TIKARAMOTI, AI_INC2		//2006.6.14
	IF_EQUAL	TOKUSYU_KATAYABURI, AI_INC2		//2006.6.14
	IF_EQUAL	TOKUSYU_HAADOROKKU, AI_INC2		//2006.6.14
	IF_EQUAL	TOKUSYU_FIRUTAA, AI_INC2		//2006.6.14
	IF_EQUAL	TOKUSYU_HURAWAAGIHUTO, AI_INC2	//2006.6.14
	AIEND

// 電気ﾜｻﾞ
DoubleEnemyAI_ElectricType:
	IF_WAZANO	WAZANO_HOUDEN,DoubleEnemyAI_ElectricType_FriendCheck_Houden	//ほうでん
	
	CHECK_HAVE_TOKUSEI	CHECK_DEFENCE_FRIEND,TOKUSYU_HIRAISIN
	IF_EQUAL	HAVE_YES, DoubleEnemyAI_ElectricType_Enemy_Hiraisin
	JUMP	DoubleEnemyAI_ElectricType_FriendCheck

DoubleEnemyAI_ElectricType_Enemy_Hiraisin:
	INCDEC	-1
	
	CHECK_HAVE_TYPE		CHECK_DEFENCE_FRIEND,POKETYPE_JIMEN
	IF_EQUAL	HAVE_NO, DoubleEnemyAI_ElectricType_FriendCheck
	INCDEC	-8

DoubleEnemyAI_ElectricType_FriendCheck:
	CHECK_HAVE_TOKUSEI		CHECK_ATTACK_FRIEND,TOKUSYU_HIRAISIN
	IF_EQUAL	HAVE_YES,	AI_DEC10

	//ほうでんでパートナー電気でアップ特性2006.6.16
	IF_WAZANO	WAZANO_HOUDEN,DoubleEnemyAI_ElectricType_FriendCheck_Houden	//ほうでん
	JUMP	DoubleEnemyAI_ElectricTypeEnd
	//CHECK_TOKUSEI	CHECK_ATTACK_FRIEND
	//IF_EQUAL	TOKUSYU_DENKIENZIN, AI_INC3		// 相方が「でんきエンジン」2006.6.16
	//IF_EQUAL	TOKUSYU_TIKUDEN, AI_INC3		// 相方が「ちくでん」2006.6.16

DoubleEnemyAI_ElectricType_FriendCheck_Houden:
	CHECK_HAVE_TOKUSEI		CHECK_ATTACK_FRIEND,TOKUSYU_DENKIENZIN
	IF_EQUAL	HAVE_YES,	AI_INC3
	CHECK_HAVE_TOKUSEI		CHECK_ATTACK_FRIEND,TOKUSYU_TIKUDEN
	IF_EQUAL	HAVE_YES,	AI_INC3

	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_MIZU
	IF_EQUAL	HAVE_YES,	AI_DEC10
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_HIKOU
	IF_EQUAL	HAVE_YES,	AI_DEC10
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_JIMEN
	IF_EQUAL	HAVE_YES,	AI_INC3
	INCDEC	-3

DoubleEnemyAI_ElectricTypeEnd:
	AIEND


//	水ﾜｻﾞ2006.6.16
DoubleEnemyAI_WaterType:
	IF_WAZANO	WAZANO_NAMINORI,DoubleEnemyAI_WaterType_FriendCheck_Naminori	//なみのり
	CHECK_HAVE_TOKUSEI	CHECK_DEFENCE_FRIEND,TOKUSYU_YOBIMIZU
	IF_EQUAL	HAVE_NO, DoubleEnemyAI_WaterType_FriendCheck
	INCDEC	-1

DoubleEnemyAI_WaterType_FriendCheck:
	CHECK_HAVE_TOKUSEI		CHECK_ATTACK_FRIEND,TOKUSYU_YOBIMIZU
	IF_EQUAL	HAVE_YES,	AI_DEC10
	
	//IFN_WAZANO	WAZANO_NAMINORI,DoubleEnemyAI_WaterTypeEnd	//なみのり
	IF_WAZANO	WAZANO_NAMINORI,DoubleEnemyAI_WaterType_FriendCheck_Naminori	//なみのり
	JUMP	DoubleEnemyAI_WaterTypeEnd	//なみのりでない


DoubleEnemyAI_WaterType_FriendCheck_Naminori:
	//CHECK_TOKUSEI	CHECK_ATTACK_FRIEND
	//IF_EQUAL	TOKUSYU_KANSOUHADA, AI_INC3		// 相方が「かんそうはだ」2006.6.16

	CHECK_HAVE_TOKUSEI		CHECK_ATTACK_FRIEND,TOKUSYU_KANSOUHADA
	IF_EQUAL	HAVE_YES,	AI_INC3

	CHECK_HAVE_TOKUSEI		CHECK_ATTACK_FRIEND,TOKUSYU_TYOSUI
	IF_EQUAL	HAVE_YES,	AI_INC3


	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_JIMEN
	IF_EQUAL	HAVE_YES,	AI_DEC10
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_HONOO
	IF_EQUAL	HAVE_YES,	AI_DEC10
	INCDEC	-3

DoubleEnemyAI_WaterTypeEnd:
	AIEND


// 炎ワザ
//DoubleEnemyAI_FireType:
//	IF_ALREADY_MORAIBI	CHECK_ATTACK, DoubleEnemyAI_FireType1//もらいびでパワーアップ状態か？
//	AIEND
//DoubleEnemyAI_FireType1:
//	JUMP	AI_INC1

DoubleEnemyAI_FireType:
	IF_ALREADY_MORAIBI	CHECK_ATTACK, DoubleEnemyAI_FireType1//もらいびでパワーアップ状態か？
	JUMP	DoubleEnemyAI_FireType_Check
DoubleEnemyAI_FireType1:
	INCDEC	+1

//dpより追加
DoubleEnemyAI_FireType_Check:
	//IFN_WAZANO	WAZANO_HUNEN,DoubleEnemyAI_FireTypeEnd	//ふんえん
	IF_WAZANO	WAZANO_HUNEN,DoubleEnemyAI_FireType_Check_Hunen	//ふんえん
	JUMP	DoubleEnemyAI_FireTypeEnd	//ふんえでない

DoubleEnemyAI_FireType_Check_Hunen:
	//CHECK_TOKUSEI	CHECK_ATTACK_FRIEND
	//IF_EQUAL	TOKUSYU_KANSOUHADA, AI_DEC3		// 相方が「かんそうはだ」2006.6.16
	//IF_EQUAL	TOKUSYU_MORAIBI,	AI_INC3		// 相方が「もらいび」2006.6.16

	CHECK_HAVE_TOKUSEI		CHECK_ATTACK_FRIEND,TOKUSYU_KANSOUHADA
	IF_EQUAL	HAVE_YES,	AI_DEC3
	CHECK_HAVE_TOKUSEI		CHECK_ATTACK_FRIEND,TOKUSYU_MORAIBI
	IF_EQUAL	HAVE_YES,	AI_INC3

	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_KUSA
	IF_EQUAL	HAVE_YES,	AI_DEC10
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_HAGANE
	IF_EQUAL	HAVE_YES,	AI_DEC10
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_KOORI
	IF_EQUAL	HAVE_YES,	AI_DEC10
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_MUSHI
	IF_EQUAL	HAVE_YES,	AI_DEC10
	INCDEC	-3

DoubleEnemyAI_FireTypeEnd:
	AIEND

DoubleEnemyAI_Waidogaado:			// 	ワイドガード

	IF_RND_UNDER	50,DoubleEnemyAI_Waidogaado_1

	CHECK_LAST_WAZA	CHECK_ATTACK
	IF_EQUAL  WAZANO_WAIDOGAADO,DoubleEnemyAI_Waidogaado_ng	//前回出してる

DoubleEnemyAI_Waidogaado_1:
	CHECK_LAST_WAZA	CHECK_DEFENCE
	IFN_TABLE_JUMP	DoubleEnemyAI_Waidogaado_Table,DoubleEnemyAI_Waidogaado_ok
  AIEND

DoubleEnemyAI_Waidogaado_ng:
	INCDEC		-4
  JUMP  DoubleEnemyAI_Waidogaado_end

DoubleEnemyAI_Waidogaado_ok:
	IF_RND_UNDER	80,DoubleEnemyAI_Waidogaado_end
	INCDEC		2
DoubleEnemyAI_Waidogaado_end:
	AIEND

DoubleEnemyAI_Waidogaado_Table: //強い複数技
	.long	59,157,257,284,323,330,549,555
	.long	57,89,435,436,482,523,545
	.long	0xffffffff

DoubleEnemyAI_Rinsyou:      	//　りんしょう　2010/4/23 
	IFN_HAVE_WAZA	CHECK_ATTACK_FRIEND,WAZANO_RINSYOU,DoubleEnemyAI_Rinsyou_ng

	IF_COMMONRND_UNDER	128,DoubleEnemyAI_Rinsyou_ng
	INCDEC	+3
	JUMP  DoubleEnemyAI_Rinsyou_End

DoubleEnemyAI_Rinsyou_ng:
	INCDEC	-1

DoubleEnemyAI_Rinsyou_End:
	AIEND

DoubleEnemyAI_Sidechange:      	//　サイドチェンジ　2010/4/23 
  CHECK_MONSNO	CHECK_DEFENCE
	IF_TABLE_JUMP	DoubleEnemyAI_Sidechange_Table,DoubleEnemyAI_Sidechange_neko    // 相手がねこだましを出しそう
	
  JUMP  DoubleEnemyAI_Sidechange_end

DoubleEnemyAI_Sidechange_neko:
	CHECK_NEKODAMASI	CHECK_DEFENCE
	IFN_EQUAL	0,DoubleEnemyAI_Sidechange_end

	IF_RND_UNDER	128,DoubleEnemyAI_Sidechange_end
	INCDEC	+2
  JUMP  DoubleEnemyAI_Sidechange_end

DoubleEnemyAI_Sidechange_ng:
	IF_RND_UNDER	128,DoubleEnemyAI_Sidechange_end
	INCDEC	-1
DoubleEnemyAI_Sidechange_end:
	AIEND

DoubleEnemyAI_Sidechange_Table:    // 相手がねこだましを出しそう
	.long	MONSNO_KAMEKKUSU
	.long	MONSNO_PERUSIAN
	.long	MONSNO_ZYUGON
	.long	MONSNO_GARUURA
	.long	MONSNO_BARIYAADO
	.long	MONSNO_PIKATYUU
	.long	MONSNO_RAITYUU
	.long	MONSNO_ETEBOOSU
	.long	MONSNO_MANYUURA
	.long	MONSNO_EBIWARAA
	.long	MONSNO_SAWAMURAA
	.long	MONSNO_KAPOERAA
	.long	MONSNO_RUUZYURA
	.long	MONSNO_RUNPAPPA
	.long	MONSNO_DAATENGU
	.long	MONSNO_HARITEYAMA
	.long	MONSNO_ENEKORORO
	.long	MONSNO_YAMIRAMI
	.long	MONSNO_TYAAREMU
	.long	MONSNO_PATTIIRU
	.long	MONSNO_KAKUREON
	.long	MONSNO_GOUKAZARU
	.long	MONSNO_MIMIROPPU
	.long	MONSNO_BUNYATTO
	.long	MONSNO_GUREGGURU
	.long	MONSNO_DERIBAADO
	.long	MONSNO_638 //　ズルズキン
	.long	MONSNO_547 //　レパルダス
	.long	0xffffffff

DoubleEnemyAI_Gihutopasu:       	//　ギフトパス　2010/4/23 

	JUMP	AI_DEC10 

DoubleEnemyAI_Sakiokuri:      	//　さきおくり　2010/4/30 

	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IF_EQUAL	0,AI_DEC10            	  //1位
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IF_EQUAL	1,AI_DEC10            	  //2位

	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IF_EQUAL	0,DoubleEnemyAI_Sakiokuri_ok        	//1位

	JUMP	AI_DEC10 

DoubleEnemyAI_Sakiokuri_ok:
	IF_COMMONRND_UNDER	128,DoubleEnemyAI_Sakiokuri_end

  INCDEC  1
DoubleEnemyAI_Sakiokuri_end:
	AIEND

DoubleEnemyAI_Daakuhouru:      	//　ダークホール　2010/4/23 
DoubleEnemyAI_Iwanadare:      	//　いわなだれ　2010/4/23 
DoubleEnemyAI_Hubuki:	          //　ふぶき　    2010/4/23 
DoubleEnemyAI_Tomoenage:       	//　ともえなげ　2010/4/23
DoubleEnemyAI_Hunka:       	    //　ふんか　    2010/4/23
DoubleEnemyAI_rendou_Osakinidouzo:    // 2010/4/22
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND,WAZANO_OSAKINIDOUZO, DoubleEnemyAI_rendou_Osakinidouzo_1
	AIEND

DoubleEnemyAI_rendou_Osakinidouzo_1:
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IFN_EQUAL	0,AI_DEC10        	//1位じゃない

	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IF_EQUAL	0,AI_DEC10        	//1位タイ
	IF_EQUAL	1,AI_DEC10        	//2位

	IF_COMMONRND_UNDER	128,DoubleEnemyAI_rendou_Osakinidouzo_end
	INCDEC		3	

DoubleEnemyAI_rendou_Osakinidouzo_end:
	AIEND

DoubleEnemyAI_Iyasinohadou:		// いやしのはどう　2010.6.25
DoubleEnemyAI_Osakinidouzo:		// おさきにどうぞ　2010.6.25
	INCDEC		-10	
	AIEND

//--------------------------------------
//	味方が対象
//--------------------------------------
DoubleAI_MineSeq:
	IF_HINSHI	CHECK_ATTACK_FRIEND,DoubleMineAI_end	//味方がいない場合はやらない	
	COMP_POWER	LOSS_CALC_OFF
	IF_EQUAL	COMP_POWER_NONE, DoubleMineAI_1

// ダメージワザ------------------
	CHECK_TYPE	CHECK_WAZA
	IF_EQUAL	POKETYPE_HONOO, DoubleMineAI_FireType
	IF_EQUAL	POKETYPE_DENKI, DoubleMineAI_ElectricType
	IF_EQUAL	POKETYPE_MIZU, DoubleMineAI_WaterType

	IF_WAZANO	WAZANO_NAGETUKERU, DoubleMineAI_Nagetukeru	//なげつける2006.6.23

DoubleMineDamageEnd:
	JUMP	AI_DEC30	// 基本的にダメージワザは出さない

//炎タイプ
DoubleMineAI_FireType:
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_MORAIBI
	IF_EQUAL	HAVE_YES, DoubleMineAI_FireType1
	JUMP	DoubleMineDamageEnd
DoubleMineAI_FireType1:
	IF_ALREADY_MORAIBI	CHECK_ATTACK_FRIEND, DoubleMineDamageEnd

//-------2010.7.3修正　かたやぶりは味方に攻撃しない----------

	CHECK_TOKUSEI	CHECK_ATTACK                  // 特性がかたやぶり等ならダメージを与えてしまうので攻撃しない。
	IF_EQUAL	TOKUSYU_KATAYABURI,DoubleMineDamageEnd
	IF_EQUAL	TOKUSYU_TAABOBUREIZU,DoubleMineDamageEnd
	IF_EQUAL	TOKUSYU_TERABORUTEEZI,DoubleMineDamageEnd

	IF_RND_UNDER	150,DoubleMineDamageEnd   // ランダムを少し入れる。 

//-------2010.7.3修正　かたやぶりは味方に攻撃しない----------

	JUMP	AI_INC1

//電気タイプ2006.6.16
DoubleMineAI_ElectricType:
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_DENKIENZIN
	IF_EQUAL	HAVE_YES, DoubleMineAI_ElectricType1
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_TIKUDEN
	IF_EQUAL	HAVE_YES, DoubleMineAI_ElectricType2
	JUMP	DoubleMineDamageEnd

DoubleMineAI_ElectricType1:

//-------2010.7.3修正　かたやぶりは味方に攻撃しない----------

	CHECK_TOKUSEI	CHECK_ATTACK                  // 特性がかたやぶり等ならダメージを与えてしまうので攻撃しない。
	IF_EQUAL	TOKUSYU_KATAYABURI,DoubleMineDamageEnd
	IF_EQUAL	TOKUSYU_TAABOBUREIZU,DoubleMineDamageEnd
	IF_EQUAL	TOKUSYU_TERABORUTEEZI,DoubleMineDamageEnd

//-------2010.7.3修正　かたやぶりは味方に攻撃しない----------

	//ランダムを入れる
	IF_RND_UNDER	160, DoubleMineAI_ElectricType_End
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_AGI,7,DoubleMineDamageEnd    // 2010.7.3　OVERに修正

	JUMP	AI_INC1

DoubleMineAI_ElectricType2:

//-------2010.7.3修正　かたやぶりは味方に攻撃しない----------

	CHECK_TOKUSEI	CHECK_ATTACK                  // 特性がかたやぶり等ならダメージを与えてしまうので攻撃しない。
	IF_EQUAL	TOKUSYU_KATAYABURI,DoubleMineDamageEnd
	IF_EQUAL	TOKUSYU_TAABOBUREIZU,DoubleMineDamageEnd
	IF_EQUAL	TOKUSYU_TERABORUTEEZI,DoubleMineDamageEnd

	IF_RND_UNDER	150,DoubleMineDamageEnd   // ランダムを少し入れる。 

//-------2010.7.3修正　かたやぶりは味方に攻撃しない----------

	IF_HP_EQUAL	CHECK_ATTACK_FRIEND,100,AI_DEC10
	IF_HP_OVER	CHECK_ATTACK_FRIEND,90,DoubleMineAI_ElectricType_End
	IF_HP_OVER	CHECK_ATTACK_FRIEND,75,DoubleMineAI_ElectricType2_1
	IF_HP_OVER	CHECK_ATTACK_FRIEND,50,DoubleMineAI_ElectricType2_2
	JUMP	DoubleMineAI_ElectricType2_3

DoubleMineAI_ElectricType2_1://25%
	IF_RND_UNDER	64, AI_INC1
	JUMP	DoubleMineAI_ElectricType_End	

DoubleMineAI_ElectricType2_2://50%
	IF_RND_UNDER	128, AI_INC1
	JUMP	DoubleMineAI_ElectricType_End	
	
DoubleMineAI_ElectricType2_3://75%
	IF_RND_UNDER	192, AI_INC1
	JUMP	DoubleMineAI_ElectricType_End	

DoubleMineAI_ElectricType_End:
	AIEND



//水タイプ2006.6.16
DoubleMineAI_WaterType:
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_TYOSUI
	IF_EQUAL	HAVE_YES, DoubleMineAI_WaterType1
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_KANSOUHADA
	IF_EQUAL	HAVE_YES, DoubleMineAI_WaterType1
	JUMP	DoubleMineDamageEnd

DoubleMineAI_WaterType1:

//-------2010.7.3修正　かたやぶりは味方に攻撃しない----------

	CHECK_TOKUSEI	CHECK_ATTACK                  // 特性がかたやぶり等ならダメージを与えてしまうので攻撃しない。
	IF_EQUAL	TOKUSYU_KATAYABURI,DoubleMineDamageEnd
	IF_EQUAL	TOKUSYU_TAABOBUREIZU,DoubleMineDamageEnd
	IF_EQUAL	TOKUSYU_TERABORUTEEZI,DoubleMineDamageEnd

	IF_RND_UNDER	150,DoubleMineDamageEnd   // ランダムを少し入れる。 

//-------2010.7.3修正　かたやぶりは味方に攻撃しない----------

	IF_HP_EQUAL	CHECK_ATTACK_FRIEND,100,AI_DEC10
	IF_HP_OVER	CHECK_ATTACK_FRIEND,90,DoubleMineAI_WaterType_End
	IF_HP_OVER	CHECK_ATTACK_FRIEND,75,DoubleMineAI_WaterType1_1
	IF_HP_OVER	CHECK_ATTACK_FRIEND,50,DoubleMineAI_WaterType1_2
	JUMP	DoubleMineAI_WaterType1_3


DoubleMineAI_WaterType1_1://25%
	IF_RND_UNDER	64, AI_INC1
	JUMP	DoubleMineAI_WaterType_End	

DoubleMineAI_WaterType1_2://50%
	IF_RND_UNDER	128, AI_INC1
	JUMP	DoubleMineAI_WaterType_End	
	
DoubleMineAI_WaterType1_3://75%
	IF_RND_UNDER	192, AI_INC1
	JUMP	DoubleMineAI_WaterType_End	

DoubleMineAI_WaterType_End:
	AIEND




// 非ダメージワザ----------------
DoubleMineAI_1:
	IF_WAZANO	WAZANO_SUKIRUSUWAPPU,DoubleMineAI_SkillSwap	// ｽｷﾙｽﾜｯﾌﾟ 2010/4/23
	IF_WAZANO	WAZANO_NARIKIRI,DoubleMineAI_Narikiri	// なりきり 2010/4/23
	IF_WAZANO	WAZANO_ONIBI, DoubleMineAI_Onibi			// おにび
	IF_WAZANO	WAZANO_DENZIHA, DoubleMineAI_Denziha		// でんじは2006.6.23
	
	//IF_WAZANO	WAZANO_DOKUDOKU, DoubleMineAI_Onibi			// どくどく	2006.6.23
	IF_WAZA_SEQNO_JUMP	33,DoubleMineAI_Doku				// もうどく2006.6.23
	IF_WAZA_SEQNO_JUMP	66,DoubleMineAI_Doku				// どく	2006.6.23
	
	IF_WAZANO	WAZANO_TEDASUKE, DoubleMineAI_Tedasuke		// てだすけ 2010/4/22
	IF_WAZANO	WAZANO_IBARU, DoubleMineAI_Ibaru			// いばる 2010/4/22

	IF_WAZANO	WAZANO_TORIKKU, DoubleMineAI_Torikku		// トリック2010.4.29
	IF_WAZANO	WAZANO_SURIKAE, DoubleMineAI_Torikku		// すりかえ2010.4.29
	IF_WAZANO	WAZANO_GIHUTOPASU,DoubleMineAI_Gihutopasu      	//　ギフトパス　2010/4/27 
	
	IF_WAZANO	WAZANO_IEKI, DoubleMineAI_Ieki				// いえき2006.6.23
	
	IF_WAZANO	WAZANO_TUBOWOTUKU, DoubleMineAI_Tubowotuku	// つぼをつく2006.6.23

	IF_WAZANO	WAZANO_OSAKINIDOUZO, DoubleMineAI_Osakinidouzo	// おさきにどうぞ2010.4.22

	IF_WAZANO	WAZANO_IYASINOHADOU,DoubleMineAI_Iyasinohadou      	//　いやしのはどう　2010/4/27 


	JUMP	DoubleMineAI_end

//ｽｷﾙｽﾜｯﾌﾟ---------
DoubleMineAI_SkillSwap:
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_NAMAKE, AI_INC10		// 相方が「なまけ」
	IF_EQUAL	TOKUSYU_SUROOSUTAATO, AI_INC10	// 相方が「スロースタート」2006.6.14

	CHECK_TOKUSEI	CHECK_ATTACK
	// 自分が「ふゆう」
	IF_EQUAL	TOKUSYU_HUYUU,DoubleMineAI_SkillSwapFuyuu	
	// 自分が「ふくがん」
	IF_EQUAL	TOKUSYU_HUKUGAN,DoubleMineAI_SkillSwapFukugan	
	// 自分が「ノーガード」
	IF_EQUAL	TOKUSYU_NOOGAADO,DoubleMineAI_SkillSwapFukugan	
	// 自分が「ふみん」
	IF_EQUAL	TOKUSYU_HUMIN,DoubleMineAI_SkillSwapFumin	
	// 自分が「マイペース」
	IF_EQUAL	TOKUSYU_MAIPEESU,DoubleMineAI_SkillSwapMaipeesu	

	JUMP	DoubleMineAI_end


DoubleMineAI_SkillSwapFuyuu:	// 自分が「ふゆう」
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_HUYUU,AI_DEC10		// 相方も「ふゆう」

	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_HIKOU,AI_DEC8    // ひこう
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_HIKOU,AI_DEC8    // ひこう

	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_KUSA,AI_DEC5    // くさ
	IF_EQUAL	POKETYPE_MUSHI,AI_DEC5    // むし

	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_KUSA,AI_DEC5    // くさ
	IF_EQUAL	POKETYPE_MUSHI,AI_DEC5    // むし

	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_FLYING,AI_DEC2	// でんじふゆう中

	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_DENKI,AI_INC3    //　でんき
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_DENKI,AI_INC3    //　でんき

	JUMP	DoubleMineAI_end


DoubleMineAI_SkillSwapFukugan:	// 自分が「ふくがん ノーガード」
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_DAIMONZI, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_KAMINARI, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_KUROSUTYOPPU, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_HAIDOROPONPU, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_BAKURETUPANTI, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_HUBUKI, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_DENZIHOU, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_MEGAHOON, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_KIAIDAMA, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_DASUTOSYUUTO, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_MAGUMASUTOOMU, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_PAWAAWHIPPU, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_SIIDOHUREA, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_MOROHANOZUTUKI, DoubleMineAI_SkillSwapFukugan1

	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_ZETTAIREIDO, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_ZIWARE, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_HASAMIGIROTIN, DoubleMineAI_SkillSwapFukugan1
	IF_HAVE_WAZA	CHECK_ATTACK_FRIEND, WAZANO_TUNODORIRU, DoubleMineAI_SkillSwapFukugan1
	JUMP	DoubleMineAI_end

DoubleMineAI_SkillSwapFukugan1:
	JUMP	AI_INC3

DoubleMineAI_SkillSwapFumin:  // 自分が「ふみん」
	IFN_WAZASICK	CHECK_DEFENCE,WAZASICK_NEMURI,AI_DEC8		//寝ていない相手にはださない
	JUMP	AI_INC3

DoubleMineAI_SkillSwapMaipeesu: // 自分が「マイペース」
	IFN_WAZASICK	CHECK_DEFENCE,WAZASICK_KONRAN,AI_DEC8		//混乱していない相手にはださない
	JUMP	AI_INC3


// なりきり
DoubleMineAI_Narikiri:
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_TABLE_JUMP	DoubleMineAI_Narikiri1_Table,DoubleMineAI_Narikiri_ok

	IF_RND_UNDER	128,DoubleMineAI_end

	IF_TABLE_JUMP	DoubleMineAI_Narikiri2_Table,DoubleMineAI_Narikiri_ok

	JUMP	DoubleMineAI_end

DoubleMineAI_Narikiri_ok:
	IF_RND_UNDER	50,DoubleMineAI_Narikiri_end
	INCDEC		1
DoubleMineAI_Narikiri_end:
	AIEND

DoubleMineAI_Narikiri1_Table:
	.long	TOKUSYU_TEREPASII
	.long	TOKUSYU_HURENDOGAADO
	.long	TOKUSYU_KASOKU
	.long	TOKUSYU_IKAKU
	.long	TOKUSYU_YOGAPAWAA
	.long	TOKUSYU_NOROWAREBODHI
	.long	0xffffffff

DoubleMineAI_Narikiri2_Table:
	.long	TOKUSYU_SANPAWAA
	.long	TOKUSYU_KANSOUHADA
	.long	TOKUSYU_DENKIENZIN
	.long	TOKUSYU_AMEUKEZARA
	.long	TOKUSYU_TIKARAMOTI
	.long	TOKUSYU_YOURYOKUSO
	.long	TOKUSYU_SUISUI
	.long	TOKUSYU_AISUBODHI
	.long	TOKUSYU_SYUUKAKU
	.long	TOKUSYU_SUNAKAKI
	.long	TOKUSYU_MAZIKKUMIRAA
	.long	TOKUSYU_ITAZURAGOKORO
	.long	0xffffffff


//おにび
DoubleMineAI_Onibi:
	
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_MORAIBI
	IF_EQUAL	HAVE_YES, DoubleMineAI_FireType	//ダメージ０でスルーされるのでここで再チェック
	
	//これに差し替え	
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_KONZYOU
	IFN_EQUAL	HAVE_YES, DoubleMineAI_end	
	
	IF_POKESICK	CHECK_ATTACK_FRIEND,DoubleMineAI_end
	
	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_HONOO, DoubleMineAI_end			// ほのおタイプ
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_HONOO, DoubleMineAI_end			// ほのおトタイプ	
	
	IF_HAVE_ITEM	CHECK_ATTACK_FRIEND, ITEM_KAENDAMA, DoubleMineAI_end//2006.6.23
	IF_HAVE_ITEM	CHECK_ATTACK_FRIEND, ITEM_DOKUDOKUDAMA, DoubleMineAI_end//2006.6.23
	IF_HP_UNDER	CHECK_ATTACK_FRIEND,81,DoubleMineAI_end
	JUMP AI_INC5


DoubleMineAI_Denziha://でんじは
	
	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_JIMEN, DoubleMineAI_end			// じめんタイプ
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_JIMEN, DoubleMineAI_end			// じめんトタイプ		
	
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_DENKIENZIN
	IF_EQUAL	HAVE_YES, DoubleMineAI_ElectricType
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_TIKUDEN
	IF_EQUAL	HAVE_YES, DoubleMineAI_ElectricType

	//CHECK_TOKUSEI	CHECK_DEFENCE
	//IF_EQUAL	TOKUSYU_DENKIENZIN,DoubleMineAI_ElectricType	//ダメージ０でスルーされるのでここで
	//IF_EQUAL	TOKUSYU_TIKUDEN,DoubleMineAI_ElectricType		//ダメージ０でスルーされるのでここで
	JUMP	DoubleMineAI_end


//どく・どくどく2006.6.23
DoubleMineAI_Doku:
	//CHECK_TOKUSEI	CHECK_DEFENCE
	//IFN_EQUAL	TOKUSYU_POIZUNHIIRU, DoubleMineAI_Onibi//こんじょうチェックへ
	
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_POIZUNHIIRU
	//IFN_EQUAL	HAVE_YES, DoubleMineAI_Onibi//リスクが高いので入れない
	IFN_EQUAL	HAVE_YES, DoubleMineAI_end//リスクが高いので入れない
	
	IF_POKESICK	CHECK_DEFENCE,DoubleMineAI_end
	IF_HAVE_ITEM	CHECK_ATTACK_FRIEND, ITEM_DOKUDOKUDAMA, DoubleMineAI_end
	IF_HP_OVER	CHECK_ATTACK_FRIEND,91,DoubleMineAI_end
	JUMP AI_INC5



//てだすけ
DoubleMineAI_Tedasuke:
	IF_HP_EQUAL	CHECK_ATTACK_FRIEND,0,AI_DEC30	//味方がいない
	IF_HP_OVER	CHECK_ATTACK_FRIEND,50,DoubleMineAI_Tedasuke1
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND		//味方の順位
	IF_UNDER	1,DoubleMineAI_Tedasuke1

	INCDEC	-1
	JUMP	DoubleMineAI_Tedasuke_end

DoubleMineAI_Tedasuke1:
	IF_COMMONRND_UNDER	128,DoubleMineAI_Tedasuke_end
	INCDEC	+3

DoubleMineAI_Tedasuke_end:
	AIEND


//いばる      2010/4/21
DoubleMineAI_Ibaru:
  IF_DMG_PHYSIC_UNDER CHECK_DEFENCE,DoubleMineAI_Ibaru0         //特攻の方が高い場合はやらない

	IF_HAVE_ITEM	CHECK_DEFENCE, ITEM_KIINOMI, DoubleMineAI_Ibaru1
	IF_HAVE_ITEM	CHECK_DEFENCE, ITEM_RAMUNOMI, DoubleMineAI_Ibaru1

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAIPEESU,DoubleMineAI_Ibaru1                     		  // マイペース

  IF_SIDEEFF	CHECK_DEFENCE,BTL_SIDEEFF_SINPINOMAMORI,DoubleMineAI_Ibaru1	// しんぴのまもり
	
DoubleMineAI_Ibaru0:
	JUMP	DoubleMineAI_end

DoubleMineAI_Ibaru1:
	IF_PARA_OVER	CHECK_DEFENCE, PARA_POW, 7, DoubleMineAI_IbaruEnd
	INCDEC	3
DoubleMineAI_IbaruEnd:
	AIEND


DoubleMineAI_Gihutopasu:      	//　ギフトパス　2010/4/27 
DoubleMineAI_Nagetukeru:
DoubleMineAI_Torikku:
	IF_HAVE_ITEM	CHECK_ATTACK, ITEM_RAMUNOMI, DoubleMineAI_Torikku_ramunomi
	IF_HAVE_ITEM	CHECK_ATTACK, ITEM_KAGONOMI, DoubleMineAI_Torikku_kagonomi
	IF_HAVE_ITEM	CHECK_ATTACK, ITEM_KURABONOMI, DoubleMineAI_Torikku_kurabonomi
	IF_HAVE_ITEM	CHECK_ATTACK, ITEM_TIIGONOMI, DoubleMineAI_Torikku_tiigonomi
	IF_HAVE_ITEM	CHECK_ATTACK, ITEM_NANASINOMI, DoubleMineAI_Torikku_nanashinomi
	IF_HAVE_ITEM	CHECK_ATTACK, ITEM_MOMONNOMI, DoubleMineAI_Torikku_momonnomi
	IF_HAVE_ITEM	CHECK_ATTACK, ITEM_KIINOMI, DoubleMineAI_Torikku_kiinomi
	IF_HAVE_ITEM	CHECK_ATTACK, ITEM_SIROIHAABU, DoubleMineAI_Torikku_siroihaabu
	IF_HAVE_ITEM	CHECK_ATTACK, ITEM_OBONNOMI, DoubleMineAI_Torikku_obonnomi
	IF_HAVE_ITEM	CHECK_ATTACK, ITEM_MENTARUHAABU, DoubleMineAI_Torikku_mentaruhaabu

	JUMP	DoubleMineAI_end


DoubleMineAI_Torikku_ramunomi:
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_NEMURI,DoubleMineAI_Torikku_nemuri		// ねむり中
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_MAHI,DoubleMineAI_Torikku_mahi 		// まひ中
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_KOORI,DoubleMineAI_Torikku_yakedo 		// やけど中
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_KOORI,DoubleMineAI_Torikku_koori 		// こおり中
	IF_DOKUDOKU	CHECK_DEFENCE,DoubleMineAI_Torikku_dokudoku	     	// どくどく中
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_KONRAN,DoubleMineAI_Torikku_konran 		// こんらん中
	JUMP	DoubleMineAI_end

DoubleMineAI_Torikku_kagonomi:
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_NEMURI,DoubleMineAI_Torikku_nemuri		// ねむり中
	JUMP	DoubleMineAI_end

DoubleMineAI_Torikku_kurabonomi:
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_MAHI,DoubleMineAI_Torikku_mahi 		// まひ中
	JUMP	DoubleMineAI_end

DoubleMineAI_Torikku_tiigonomi:
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_KOORI,DoubleMineAI_Torikku_yakedo 		// やけど中
	JUMP	DoubleMineAI_end

DoubleMineAI_Torikku_nanashinomi:
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_KOORI,DoubleMineAI_Torikku_koori 		// こおり中
	JUMP	DoubleMineAI_end

DoubleMineAI_Torikku_momonnomi:
	IF_DOKUDOKU	CHECK_DEFENCE,DoubleMineAI_Torikku_dokudoku	     	// どくどく中
	JUMP	DoubleMineAI_end

DoubleMineAI_Torikku_kiinomi:
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_KONRAN,DoubleMineAI_Torikku_konran 		// こんらん中
	JUMP	DoubleMineAI_end

DoubleMineAI_Torikku_siroihaabu:
	IF_PARA_UNDER	CHECK_ATTACK_FRIEND,PARA_POW,5,DoubleMineAI_Torikku_statusdown
	IF_PARA_UNDER	CHECK_ATTACK_FRIEND,PARA_DEF,5,DoubleMineAI_Torikku_statusdown
	IF_PARA_UNDER	CHECK_ATTACK_FRIEND,PARA_SPEPOW,5,DoubleMineAI_Torikku_statusdown
	IF_PARA_UNDER	CHECK_ATTACK_FRIEND,PARA_SPEDEF,5,DoubleMineAI_Torikku_statusdown
	IF_PARA_UNDER	CHECK_ATTACK_FRIEND,PARA_AVOID,5,DoubleMineAI_Torikku_statusdown
	IF_PARA_UNDER	CHECK_ATTACK_FRIEND,PARA_AGI,5,DoubleMineAI_Torikku_statusdown
	IF_PARA_UNDER	CHECK_ATTACK_FRIEND,PARA_SPEDEF,5,DoubleMineAI_Torikku_statusdown
	IF_PARA_UNDER	CHECK_ATTACK_FRIEND,PARA_AVOID,5,DoubleMineAI_Torikku_statusdown
	JUMP	DoubleMineAI_end

DoubleMineAI_Torikku_mentaruhaabu:
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_MEROMERO,DoubleMineAI_Torikku_meromero 		// メロメロ中
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_KONRAN,DoubleMineAI_Torikku_konran 		// こんらん中
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_ICHAMON,DoubleMineAI_Torikku_ityamon 		// いちゃもん中
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_TYOUHATSU,DoubleMineAI_Torikku_tyouhatu 		// ちょうはつ中
	JUMP	DoubleMineAI_end

DoubleMineAI_Torikku_obonnomi:
	IF_HP_UNDER	CHECK_ATTACK_FRIEND,50,DoubleMineAI_end
	JUMP	DoubleMineAI_Torikku_ok

DoubleMineAI_Torikku_nemuri:		// ねむり中
  IF_HAVE_WAZA	CHECK_ATTACK_FRIEND,WAZANO_IBIKI,DoubleMineAI_end
  IF_HAVE_WAZA	CHECK_ATTACK_FRIEND,WAZANO_NEGOTO,DoubleMineAI_end

	IF_HP_OVER	CHECK_ATTACK_FRIEND,40,DoubleMineAI_Torikku_ok

	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_DEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_SPEDEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_AVOID,7,DoubleMineAI_Torikku_ok

	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IFN_EQUAL	0,DoubleMineAI_end     	//1位じゃない

	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IFN_EQUAL	1,DoubleMineAI_end     	//2位じゃない

	JUMP	DoubleMineAI_Torikku_ok


DoubleMineAI_Torikku_mahi: 		// まひ中
  IF_HAVE_WAZA	CHECK_ATTACK_FRIEND,WAZANO_NEMURU,DoubleMineAI_end
	IF_HP_UNDER	CHECK_ATTACK_FRIEND,80,DoubleMineAI_end

	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IFN_EQUAL	3,DoubleMineAI_end     	//4位じゃない

	JUMP	DoubleMineAI_Torikku_ok


DoubleMineAI_Torikku_yakedo: 		// やけど中
  IF_HAVE_WAZA	CHECK_ATTACK_FRIEND,WAZANO_NEMURU,DoubleMineAI_end
	IF_HP_UNDER	CHECK_ATTACK_FRIEND,40,DoubleMineAI_end
  
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_DEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_SPEDEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_AVOID,7,DoubleMineAI_Torikku_ok

  IF_DMG_PHYSIC_UNDER CHECK_ATTACK_FRIEND,DoubleMineAI_end

	IF_HP_OVER	CHECK_ATTACK_FRIEND,80,DoubleMineAI_Torikku_ok

	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IFN_EQUAL	0,DoubleMineAI_end     	//1位じゃない

	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IFN_EQUAL	1,DoubleMineAI_end     	//2位じゃない

	JUMP	DoubleMineAI_Torikku_ok


DoubleMineAI_Torikku_koori: 		// こおり中
	IF_HP_OVER	CHECK_ATTACK_FRIEND,40,DoubleMineAI_Torikku_ok

	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_DEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_SPEDEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_AVOID,7,DoubleMineAI_Torikku_ok

	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IFN_EQUAL	0,DoubleMineAI_end     	//1位じゃない

	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IFN_EQUAL	1,DoubleMineAI_end     	//2位じゃない

	JUMP	DoubleMineAI_Torikku_ok


DoubleMineAI_Torikku_konran: 		// こんらん中
	IF_HP_OVER	CHECK_ATTACK_FRIEND,40,DoubleMineAI_Torikku_ok

	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_DEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_SPEDEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_AVOID,7,DoubleMineAI_Torikku_ok

	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IFN_EQUAL	0,DoubleMineAI_end     	//1位じゃない

	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IFN_EQUAL	1,DoubleMineAI_end     	//2位じゃない

	JUMP	DoubleMineAI_Torikku_ok


DoubleMineAI_Torikku_dokudoku: 		// どくどく中
  IF_HAVE_WAZA	CHECK_ATTACK_FRIEND,WAZANO_NEMURU,DoubleMineAI_end

	IF_HP_OVER	CHECK_ATTACK_FRIEND,80,DoubleMineAI_Torikku_ok
	IF_HP_UNDER	CHECK_ATTACK_FRIEND,40,DoubleMineAI_end

	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_DEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_SPEDEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_AVOID,7,DoubleMineAI_Torikku_ok

  JUMP  DoubleMineAI_end


DoubleMineAI_Torikku_statusdown: 		// ステータスダウン中
	IF_HP_UNDER	CHECK_ATTACK_FRIEND,40,DoubleMineAI_end

	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_DEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_SPEDEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_AVOID,7,DoubleMineAI_Torikku_ok

	IF_HP_OVER	CHECK_ATTACK_FRIEND,80,DoubleMineAI_Torikku_ok

	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IFN_EQUAL	0,DoubleMineAI_end     	//1位じゃない

	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IFN_EQUAL	1,DoubleMineAI_end     	//2位じゃない

	JUMP	DoubleMineAI_Torikku_ok


DoubleMineAI_Torikku_meromero: 		// メロメロ中
DoubleMineAI_Torikku_ityamon: 		// いちゃもん中
DoubleMineAI_Torikku_tyouhatu: 		// ちょうはつ中
	IF_HP_OVER	CHECK_ATTACK_FRIEND,40,DoubleMineAI_Torikku_ok

	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_DEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_SPEDEF,7,DoubleMineAI_Torikku_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_AVOID,7,DoubleMineAI_Torikku_ok

	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IFN_EQUAL	0,DoubleMineAI_end     	//1位じゃない

	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IFN_EQUAL	1,DoubleMineAI_end     	//2位じゃない

	JUMP	DoubleMineAI_Torikku_ok

DoubleMineAI_Torikku_ok:
	IF_COMMONRND_UNDER	128,DoubleMineAI_Torikku_end  //まもる連動
	INCDEC	3

DoubleMineAI_Torikku_end:
	AIEND


DoubleMineAI_Ieki:
	IF_WAZASICK	CHECK_ATTACK_FRIEND,WAZASICK_IEKI,DoubleMineAI_end		// いえき中
	
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_NAMAKE
	IF_EQUAL	HAVE_YES, DoubleMineAI_Ieki_ok
	
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_SUROOSUTAATO
	IF_EQUAL	HAVE_YES, DoubleMineAI_Ieki_ok
	JUMP	DoubleMineAI_Ieki_end

DoubleMineAI_Ieki_ok:
	INCDEC	+5
DoubleMineAI_Ieki_end:
	AIEND


DoubleMineAI_Tubowotuku:	//つぼをつく:一人用と基本は同じ
	
	//パラメーターＭＡＸ時はしない
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_POW,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_DEF,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_AGI,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_SPEPOW,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_SPEDEF,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_AVOID,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_HIT,12,DoubleMineAI_end

	IF_HP_UNDER	CHECK_ATTACK_FRIEND,51,DoubleMineAI_Tubowotuku_ng
	IF_HP_OVER	CHECK_ATTACK_FRIEND,90,DoubleMineAI_Tubowotuku_ok
	IF_RND_UNDER	128,DoubleMineAI_Tubowotuku_end

DoubleMineAI_Tubowotuku_ok:	// つぼをつく
	IF_RND_UNDER	80,DoubleMineAI_Tubowotuku_end
	INCDEC	2
	JUMP	DoubleMineAI_Tubowotuku_end

DoubleMineAI_Tubowotuku_ng:
	INCDEC	-1
DoubleMineAI_Tubowotuku_end:
	AIEND

DoubleMineAI_Osakinidouzo:	// おさきにどうぞ2010.4.22

	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IFN_EQUAL	0,AI_DEC10        	//1位じゃない

	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IF_EQUAL	0,AI_DEC10        	//1位タイ
	IF_EQUAL	1,AI_DEC10        	//2位

	IF_HAVE_WAZA_SEQNO	CHECK_DEFENCE,313,DoubleMineAI_Osakinidouzo_ok    //　ともえなげ

	FLDEFF_CHECK	BTL_FLDEFF_TRICKROOM,DoubleMineAI_Osakinidouzo_Trickroom_pass		//トリックルーム中	

	IF_HAVE_WAZA	CHECK_DEFENCE, WAZANO_TORIKKURUUMU, DoubleMineAI_Osakinidouzo_ok

DoubleMineAI_Osakinidouzo_Trickroom_pass:

	IF_HAVE_WAZA_SEQNO	CHECK_DEFENCE,190,DoubleMineAI_Osakinidouzo_ok    //　ふんか

	IF_HAVE_WAZA	CHECK_DEFENCE, WAZANO_DAAKUHOORU, DoubleMineAI_Osakinidouzo_ok

	IF_HAVE_WAZA	CHECK_DEFENCE, WAZANO_HUBUKI, DoubleMineAI_Osakinidouzo_ok
	IF_HAVE_WAZA	CHECK_DEFENCE, WAZANO_IWANADARE, DoubleMineAI_Osakinidouzo_ok
	IF_HAVE_WAZA	CHECK_DEFENCE, WAZANO_IWANADARE, DoubleMineAI_Osakinidouzo_ok

	IF_HAVE_WAZA_SEQNO	CHECK_DEFENCE,28,DoubleMineAI_Osakinidouzo_end    //　ほえる

	IF_RND_UNDER	50,DoubleMineAI_Osakinidouzo_end
	INCDEC		-2	
	JUMP	DoubleMineAI_Osakinidouzo_end

DoubleMineAI_Osakinidouzo_ok:
	IF_COMMONRND_UNDER	128,DoubleMineAI_Osakinidouzo_end
	INCDEC		3	

DoubleMineAI_Osakinidouzo_end:
	AIEND


DoubleMineAI_Iyasinohadou:      	//　いやしのはどう　2010/4/27 
	IF_HP_EQUAL	CHECK_DEFENCE,0,DoubleMineAI_end
	IF_HP_OVER	CHECK_DEFENCE,100,DoubleMineAI_Iyasinohadou_end

	IF_HP_OVER	CHECK_DEFENCE,70,DoubleMineAI_Iyasinohadou_70up
	IF_HP_OVER	CHECK_DEFENCE,30,DoubleMineAI_Iyasinohadou_ok

	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_DEF,7,DoubleMineAI_Iyasinohadou_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_SPEDEF,7,DoubleMineAI_Iyasinohadou_ok
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_AVOID,7,DoubleMineAI_Iyasinohadou_ok

	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IF_EQUAL	0,DoubleMineAI_Iyasinohadou_ok        	//1位

	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND	//味方の順位
	IFN_EQUAL	0,DoubleMineAI_Iyasinohadou_end        	//1位じゃない

	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IF_EQUAL	1,DoubleMineAI_Iyasinohadou_ok        	//2位

	JUMP	DoubleMineAI_Iyasinohadou_end

DoubleMineAI_Iyasinohadou_70up:      	
	CHECK_AGI_RANK	CHECK_ATTACK	//自分の順位
	IF_EQUAL	3,DoubleMineAI_Iyasinohadou_ok        	//4位
	JUMP	DoubleMineAI_Iyasinohadou_end

DoubleMineAI_Iyasinohadou_ok:

	IF_COMMONRND_UNDER	128,DoubleMineAI_Iyasinohadou_end   //　まもると連動
	INCDEC		3	
	JUMP  DoubleMineAI_Iyasinohadou_end

DoubleMineAI_Iyasinohadou_end:
	INCDEC		-1	
	AIEND

DoubleMineAI_end:
	INCDEC	-30		//基本的に味方には出さない
	AIEND

AI_DEC1:
	INCDEC		-1	
	AIEND
AI_DEC2:
	INCDEC		-2	
	AIEND
AI_DEC3:
	INCDEC		-3	
	AIEND
AI_DEC5:
	INCDEC		-5	
	AIEND
AI_DEC6:
	INCDEC		-6	
	AIEND
AI_DEC8:
	INCDEC		-8	
	AIEND
AI_DEC10:
	INCDEC		-10
	AIEND
AI_DEC12:
	INCDEC		-12
	AIEND
AI_DEC30:
	INCDEC		-30
	AIEND
AI_INC1:
	INCDEC		1	
	AIEND
AI_INC2:
	INCDEC		2	
	AIEND
AI_INC3:
	INCDEC		3	
	AIEND
AI_INC5:
	INCDEC		5	
	AIEND
AI_INC10:
	INCDEC		10	
	AIEND

