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
	IFN_COND	CHECK_ATTACK,0x000000ff,DoubleEnemyAI_Amagoi_Friend

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
	IFN_COND	CHECK_ATTACK_FRIEND,0x000000ff,DoubleEnemyAI_Amagoi_End

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
	IF_COND	CHECK_ATTACK,0x000000ff,DoubleEnemyAI_Nihonbare_Friend
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
	IF_COND	CHECK_ATTACK_FRIEND,0x000000ff,DoubleEnemyAI_Nihonbare_End
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
	FIELD_CONDITION_CHECK	FIELD_CONDITION_JUURYOKU,DoubleMineAI_end	//じゅうりょく中はやらない	

DoubleEnemyAI_Zyuuryoku_mine://自分が浮遊＆飛行
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK,TOKUSYU_HUYUU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_mine_ng
	CHECK_HAVE_TYPE		CHECK_ATTACK,POKETYPE_HIKOU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_mine_ng
	IF_WAZAKOUKA	CHECK_ATTACK,WAZAKOUKA_DENZIHUYUU,DoubleEnemyAI_Zyuuryoku_mine_ng
	JUMP	DoubleEnemyAI_Zyuuryoku_mine_friend

DoubleEnemyAI_Zyuuryoku_mine_ng:
	INCDEC	-5
	JUMP	DoubleEnemyAI_Zyuuryoku_mine_friend

DoubleEnemyAI_Zyuuryoku_mine_friend://パートナーが浮遊＆飛行
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND,TOKUSYU_HUYUU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_mine_friend_ng
	CHECK_HAVE_TYPE		CHECK_ATTACK_FRIEND,POKETYPE_HIKOU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_mine_friend_ng
	IF_WAZAKOUKA	CHECK_ATTACK_FRIEND,WAZAKOUKA_DENZIHUYUU,DoubleEnemyAI_Zyuuryoku_mine_friend_ng
	JUMP	DoubleEnemyAI_Zyuuryoku_enemy

DoubleEnemyAI_Zyuuryoku_mine_friend_ng:
	INCDEC	-5
	JUMP	DoubleEnemyAI_Zyuuryoku_enemy

DoubleEnemyAI_Zyuuryoku_enemy://敵が浮遊＆飛行
	CHECK_HAVE_TOKUSEI	CHECK_DEFENCE,TOKUSYU_HUYUU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_enemy_ok
	CHECK_HAVE_TYPE		CHECK_DEFENCE,POKETYPE_HIKOU
	IF_EQUAL	HAVE_YES,	DoubleEnemyAI_Zyuuryoku_enemy_ok
	IF_WAZAKOUKA	CHECK_DEFENCE,WAZAKOUKA_DENZIHUYUU,DoubleEnemyAI_Zyuuryoku_enemy_ok
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
	IF_WAZAKOUKA	CHECK_DEFENCE_FRIEND,WAZAKOUKA_DENZIHUYUU,DoubleEnemyAI_Zyuuryoku_enemy_friend_ok
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
	IF_WAZA_SEQNO_JUMP	38,DoubleEnemyAI_Tedasuke_End		//	いちげきひっさつ
	IF_WAZA_SEQNO_JUMP	41,DoubleEnemyAI_Tedasuke_End		//　りゅうのいかり
	IF_WAZA_SEQNO_JUMP	87,DoubleEnemyAI_Tedasuke_End		//　ナイトヘッド
	IF_WAZA_SEQNO_JUMP	88,DoubleEnemyAI_Tedasuke_End		//　サイコウエーブ
	IF_WAZA_SEQNO_JUMP	130,DoubleEnemyAI_Tedasuke_End	//　ソニックブーム
	COMP_POWER	LOSS_CALC_OFF
	IFN_EQUAL	COMP_POWER_NONE, AI_INC1
DoubleEnemyAI_Tedasuke_End:
	AIEND


// 自分が「こんじょう」
DoubleEnemyAI_Konzyou:
	IF_COND	CHECK_ATTACK,0x000000ff,DoubleEnemyAI_Konzyou1
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
	IF_WAZAKOUKA	CHECK_ATTACK_FRIEND,WAZAKOUKA_DENZIHUYUU,AI_INC2	// でんじふゆう中
	
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
	JUMP	AI_INC3

//電気タイプ2006.6.16
DoubleMineAI_ElectricType:
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_DENKIENZIN
	IF_EQUAL	HAVE_YES, DoubleMineAI_ElectricType1
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_TIKUDEN
	IF_EQUAL	HAVE_YES, DoubleMineAI_ElectricType2
	JUMP	DoubleMineDamageEnd

DoubleMineAI_ElectricType1:
	//ランダムを入れる
	IF_RND_UNDER	160, DoubleMineAI_ElectricType_End
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_AGI,12,DoubleMineDamageEnd
	JUMP	AI_INC3

DoubleMineAI_ElectricType2:
	IF_HP_EQUAL	CHECK_ATTACK_FRIEND,100,AI_DEC10
	IF_HP_OVER	CHECK_ATTACK_FRIEND,90,DoubleMineAI_ElectricType_End
	IF_HP_OVER	CHECK_ATTACK_FRIEND,75,DoubleMineAI_ElectricType2_1
	IF_HP_OVER	CHECK_ATTACK_FRIEND,50,DoubleMineAI_ElectricType2_2
	JUMP	DoubleMineAI_ElectricType2_3

DoubleMineAI_ElectricType2_1://25%
	IF_RND_UNDER	64, AI_INC3
	JUMP	DoubleMineAI_ElectricType_End	

DoubleMineAI_ElectricType2_2://50%
	IF_RND_UNDER	128, AI_INC3
	JUMP	DoubleMineAI_ElectricType_End	
	
DoubleMineAI_ElectricType2_3://75%
	IF_RND_UNDER	192, AI_INC3
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
	IF_HP_EQUAL	CHECK_ATTACK_FRIEND,100,AI_DEC10
	IF_HP_OVER	CHECK_ATTACK_FRIEND,90,DoubleMineAI_WaterType_End
	IF_HP_OVER	CHECK_ATTACK_FRIEND,75,DoubleMineAI_WaterType1_1
	IF_HP_OVER	CHECK_ATTACK_FRIEND,50,DoubleMineAI_WaterType1_2
	JUMP	DoubleMineAI_WaterType1_3


DoubleMineAI_WaterType1_1://25%
	IF_RND_UNDER	64, AI_INC3
	JUMP	DoubleMineAI_WaterType_End	

DoubleMineAI_WaterType1_2://50%
	IF_RND_UNDER	128, AI_INC3
	JUMP	DoubleMineAI_WaterType_End	
	
DoubleMineAI_WaterType1_3://75%
	IF_RND_UNDER	192, AI_INC3
	JUMP	DoubleMineAI_WaterType_End	

DoubleMineAI_WaterType_End:
	AIEND




// 非ダメージワザ----------------
DoubleMineAI_1:
	IF_WAZANO	WAZANO_SUKIRUSUWAPPU,DoubleMineAI_SkillSwap	// ｽｷﾙｽﾜｯﾌﾟ
	IF_WAZANO	WAZANO_ONIBI, DoubleMineAI_Onibi			// おにび
	IF_WAZANO	WAZANO_DENZIHA, DoubleMineAI_Denziha		// でんじは2006.6.23
	
	//IF_WAZANO	WAZANO_DOKUDOKU, DoubleMineAI_Onibi			// どくどく	2006.6.23
	IF_WAZA_SEQNO_JUMP	33,DoubleMineAI_Doku				// もうどく2006.6.23
	IF_WAZA_SEQNO_JUMP	66,DoubleMineAI_Doku				// どく	2006.6.23
	
	IF_WAZANO	WAZANO_TEDASUKE, DoubleMineAI_Tedasuke		// てだすけ
	IF_WAZANO	WAZANO_IBARU, DoubleMineAI_Ibaru			// いばる
	IF_WAZANO	WAZANO_TORIKKU, DoubleMineAI_Torikku		// トリック2006.6.23
	IF_WAZANO	WAZANO_SURIKAE, DoubleMineAI_Torikku		// いれかえ2006.6.23
	
	IF_WAZANO	WAZANO_IEKI, DoubleMineAI_Ieki				// いえき2006.6.23
	
	IF_WAZANO	WAZANO_TUBOWOTUKU, DoubleMineAI_Tubowotuku	// つぼをつく2006.6.23

	JUMP	DoubleMineAI_end

//ｽｷﾙｽﾜｯﾌﾟ---------
DoubleMineAI_SkillSwap:
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_NAMAKE, AI_INC10		// 相方が「なまけ」
	IF_EQUAL	TOKUSYU_SUROOSUTAATO, AI_INC10	// 相方が「スロースタート」2006.6.14
	
	// 自分が「ふゆう」
	CHECK_TOKUSEI	CHECK_ATTACK
	IFN_EQUAL	TOKUSYU_HUYUU, DoubleMineAI_SkillSwapFukugan
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_HUYUU, DoubleMineAI_end		// 相方も「ふゆう」
	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IFN_EQUAL	POKETYPE_DENKI, DoubleMineAI_SkillSwapFukugan
	INCDEC		1
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IFN_EQUAL	POKETYPE_DENKI, DoubleMineAI_SkillSwapFukugan
	INCDEC		1
	AIEND

DoubleMineAI_SkillSwapFukugan:
	CHECK_TOKUSEI	CHECK_ATTACK
	//IFN_EQUAL	TOKUSYU_HUKUGAN, DoubleMineAI_end
	IF_EQUAL	TOKUSYU_HUKUGAN,DoubleMineAI_SkillSwapFukugan_Check
	IF_EQUAL	TOKUSYU_NOOGAADO, DoubleMineAI_SkillSwapFukugan_Check
	JUMP	DoubleMineAI_end
	
	// 命中率上がる特性
DoubleMineAI_SkillSwapFukugan_Check:
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
	JUMP		DoubleMineAI_end

DoubleMineAI_SkillSwapFukugan1:
	JUMP	AI_INC3


//おにび
DoubleMineAI_Onibi:
	
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_MORAIBI
	IF_EQUAL	HAVE_YES, DoubleMineAI_FireType	//ダメージ０でスルーされるのでここで再チェック
	
	//これに差し替え	
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_KONZYOU
	IFN_EQUAL	HAVE_YES, DoubleMineAI_end	
	
	//CHECK_TOKUSEI	CHECK_DEFENCE
	//IF_EQUAL	TOKUSYU_MORAIBI,DoubleMineAI_FireType	//ダメージ０でスルーされるのでここで
	//IFN_EQUAL	TOKUSYU_KONZYOU, DoubleMineAI_end
	IF_COND	CHECK_ATTACK_FRIEND,0x000000ff,DoubleMineAI_end
	
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
	
	IF_COND	CHECK_DEFENCE,0x000000ff,DoubleMineAI_end
	IF_HAVE_ITEM	CHECK_ATTACK_FRIEND, ITEM_DOKUDOKUDAMA, DoubleMineAI_end
	IF_HP_OVER	CHECK_ATTACK_FRIEND,91,DoubleMineAI_end
	JUMP AI_INC5



//てだすけ
DoubleMineAI_Tedasuke:
	IF_HP_EQUAL	CHECK_ATTACK_FRIEND,0,AI_DEC30	//味方がいない
	IF_HP_OVER	CHECK_ATTACK_FRIEND,50,DoubleMineAI_Tedasuke1
	CHECK_AGI_RANK	CHECK_ATTACK_FRIEND		//味方の順位
	IF_UNDER	1,DoubleMineAI_Tedasuke1
	JUMP	DoubleMineAI_Tedasuke_end

DoubleMineAI_Tedasuke1:
	IF_RND_UNDER	64, AI_DEC1
	INCDEC	+2

DoubleMineAI_Tedasuke_end:
	AIEND


//いばる
DoubleMineAI_Ibaru:
	IF_HAVE_ITEM	CHECK_DEFENCE, ITEM_KIINOMI, DoubleMineAI_Ibaru1
	IF_HAVE_ITEM	CHECK_DEFENCE, ITEM_RAMUNOMI, DoubleMineAI_Ibaru1
	JUMP	DoubleMineAI_end
DoubleMineAI_Ibaru1:
	IF_PARA_OVER	CHECK_DEFENCE, PARA_POW, 7, DoubleMineAI_IbaruEnd
	INCDEC	3
DoubleMineAI_IbaruEnd:
	AIEND

//ダブル
DoubleMineAI_Nagetukeru:
DoubleMineAI_Torikku:
	AIEND

DoubleMineAI_Ieki:
	IF_WAZAKOUKA	CHECK_ATTACK_FRIEND,WAZAKOUKA_IEKI,DoubleMineAI_end		// いえき中
	//CHECK_TOKUSEI	CHECK_ATTACK_FRIEND
	//IF_EQUAL	TOKUSYU_NAMAKE,DoubleMineAI_Ieki_ok				// なまけ
	//IF_EQUAL	TOKUSYU_SUROOSUTAATO,DoubleMineAI_Ieki_ok		// スロースタート
	
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
	//CHECK_TOKUSEI	CHECK_ATTACK_FRIEND				//たんじゅんなら段階数９でやめ2006.6.6
	//IF_EQUAL	TOKUSYU_TANZYUN,DoubleMineAI_Tubowotuku_tanzyun
	
	CHECK_HAVE_TOKUSEI	CHECK_ATTACK_FRIEND, TOKUSYU_TANZYUN
	IF_EQUAL	HAVE_YES, DoubleMineAI_Tubowotuku_tanzyun

	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_POW,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_DEF,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_AGI,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_SPEPOW,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_SPEDEF,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_AVOID,12,DoubleMineAI_end
	IF_PARA_EQUAL	CHECK_ATTACK_FRIEND,PARA_HIT,12,DoubleMineAI_end
	JUMP	DoubleMineAI_Tubowotuku_check	

DoubleMineAI_Tubowotuku_tanzyun:
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_POW,8,AI_DEC10
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_DEF,8,AI_DEC10
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_AGI,8,AI_DEC10
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_SPEPOW,8,AI_DEC10
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_SPEDEF,8,AI_DEC10
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_AVOID,8,AI_DEC10
	IF_PARA_OVER	CHECK_ATTACK_FRIEND,PARA_HIT,8,AI_DEC10

DoubleMineAI_Tubowotuku_check:
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

//========================================================
//	HPで判断
//========================================================

HpCheckAISeq:
	IF_MIKATA_ATTACK	DoubleAI_MineSeq			//対象が味方なら終了

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

