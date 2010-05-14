//============================================================================
/**
 *@file		tr_ai_basic.s
 *@brief	トレーナーAI
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

	.text
	.include	"tr_ai_seq_def.h"

//========================================================
//	基本AI
//========================================================

BasicAISeq:
	IF_MIKATA_ATTACK	BasicAISeq_end				//対象が味方なら終了

	// じわれ、つのドリルはダメージ０でもダメージワザルーチンを通す
	IF_WAZANO	WAZANO_ZIWARE, BasicAIDamageStart
	IF_WAZANO	WAZANO_TUNODORIRU, BasicAIDamageStart
	
	COMP_POWER	LOSS_CALC_OFF
	IF_EQUAL	COMP_POWER_NONE,BasicAI_DmgEnd

//ダメージワザの場合
BasicAIDamageStart:
	CHECK_WAZA_AISYOU	AISYOU_0BAI,AI_DEC10	//効果ない技はださない(ふゆうなどもココ2006.8.9)	


	CHECK_TOKUSEI	CHECK_ATTACK//かたやぶり持ちなら相手特性をスルーして攻撃
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_DmgEnd		// かたやぶり2006.6.7
	
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_TIKUDEN,BasicAI_00_1		// ちくでん
	IF_EQUAL	TOKUSYU_DENKIENZIN,BasicAI_00_1		// でんきエンジン2006.6.7
	IF_EQUAL	TOKUSYU_HIRAISIN,BasicAI_00_1			// ひらいしん2010.2.9
	IF_EQUAL	TOKUSYU_TYOSUI,BasicAI_00_2			// ちょすい
	IF_EQUAL	TOKUSYU_MORAIBI,BasicAI_00_3		// もらいび
	IF_EQUAL	TOKUSYU_HUSIGINAMAMORI,BasicAI_00_4	// ふしぎなまもり
	IF_EQUAL	TOKUSYU_HUYUU,BasicAI_00_5			// ふゆう
	IF_EQUAL	TOKUSYU_HUYUU,BasicAI_00_6			// かんそうはだ2006.6.7
	IF_EQUAL	TOKUSYU_SOUSYOKU,BasicAI_00_7			// そうしょく2010.2.9
	JUMP	BasicAI_DmgEnd

BasicAI_00_1:		// ちくでん＆でんきエンジン＆ひらいしん
	CHECK_TYPE	CHECK_WAZA
	IF_WAZA_TYPE	POKETYPE_DENKI,AI_DEC12		//効果ない技はださない	
	JUMP	BasicAI_DmgEnd

BasicAI_00_2:		// ちょすい
	CHECK_TYPE	CHECK_WAZA
	IF_WAZA_TYPE	POKETYPE_MIZU,AI_DEC12		//効果ない技はださない	
	JUMP	BasicAI_DmgEnd

BasicAI_00_3:		// もらいび
	CHECK_TYPE	CHECK_WAZA
	IF_WAZA_TYPE	POKETYPE_HONOO,AI_DEC12		//効果ない技はださない	
	JUMP	BasicAI_DmgEnd

BasicAI_00_4:		// ふしぎなまもり
	CHECK_WAZA_AISYOU	AISYOU_2BAI,BasicAI_DmgEnd	// 効果ばつぐんしかあたらない	
	CHECK_WAZA_AISYOU	AISYOU_4BAI,BasicAI_DmgEnd	// 効果ばつぐんしかあたらない	
	JUMP	AI_DEC12

BasicAI_00_5:		// ふゆう
	CHECK_TYPE	CHECK_WAZA
	IF_WAZA_TYPE	POKETYPE_JIMEN,AI_DEC12		//効果ない技はださない	
	JUMP	BasicAI_DmgEnd

BasicAI_00_6:		// かんそうはだ
	CHECK_TYPE	CHECK_WAZA
	IF_WAZA_TYPE	POKETYPE_MIZU,AI_DEC12		//効果ない技はださない	
	JUMP	BasicAI_DmgEnd

BasicAI_00_7:		// そうしょく2010.2.9
	CHECK_TYPE	CHECK_WAZA
	IF_WAZA_TYPE	POKETYPE_KUSA,AI_DEC12		//効果ない技はださない	
	JUMP	BasicAI_DmgEnd

BasicAI_DmgEnd:

	CHECK_TOKUSEI	CHECK_DEFENCE
	IFN_EQUAL	TOKUSYU_BOUON,BasicAI_01		// ぼうおん
	
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_01	// かたやぶり

	IF_WAZANO	WAZANO_NAKIGOE,AI_DEC10
	IF_WAZANO	WAZANO_HOERU,AI_DEC10
	IF_WAZANO	WAZANO_UTAU,AI_DEC10
	IF_WAZANO	WAZANO_TYOUONPA,AI_DEC10
	IF_WAZANO	WAZANO_IYANAOTO,AI_DEC10
	IF_WAZANO	WAZANO_IBIKI,AI_DEC10
	IF_WAZANO	WAZANO_SAWAGU,AI_DEC10
	IF_WAZANO	WAZANO_KINZOKUON,AI_DEC10
	IF_WAZANO	WAZANO_KUSABUE,AI_DEC10
	IF_WAZANO	WAZANO_MUSINOSAZAMEKI,AI_DEC10	//2006.6.6
	IF_WAZANO	WAZANO_OSYABERI,AI_DEC10		//2006.6.6
	IF_WAZANO	WAZANO_RINSYOU,AI_DEC10	//2010.2.9
	IF_WAZANO	WAZANO_EKOOBOISU,AI_DEC10		//2010.2.9
	IF_WAZANO	WAZANO_INISIENOUTA,AI_DEC10		//2010.2.9
	IF_WAZANO	WAZANO_BAAKUAUTO,AI_DEC10		//2010.2.9

BasicAI_01:
  TABLE_JUMP TABLE_JUMP_WAZASEQNO,WAZASEQ_TABLE
  AIEND

  TABLE_ADRS  WAZASEQ_TABLE
	ADRS  BasicAI_Dummy	//	  0ダミー
	ADRS  BasicAI_1			//	ねむり
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_7			//	じばく
	ADRS  BasicAI_8			//	ゆめくい
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_10		//	攻撃１アップ

	ADRS  BasicAI_11		//	防御１アップ
	ADRS  BasicAI_12		//	素早さ１アップ
	ADRS  BasicAI_13		//	特攻１アップ
	ADRS  BasicAI_14		//	特防１アップ
	ADRS  BasicAI_15		//	命中１アップ
	ADRS  BasicAI_16		//	回避１アップ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_18		//	攻撃１ダウン
	ADRS  BasicAI_19		//	防御１ダウン
	ADRS  BasicAI_20		//	素早さ１ダウン

	ADRS  BasicAI_21		//	特攻１ダウン
	ADRS  BasicAI_22		//	特防１ダウン
	ADRS  BasicAI_23		//	命中１ダウン
	ADRS  BasicAI_24		//	回避１ダウン
	ADRS  BasicAI_25		//	くろいきり
	ADRS  BasicAI_26		//	がまん
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_28		//	ふきとばし
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_32		//	じこさいせい2006.6.14
	ADRS  BasicAI_33		//	どくどく
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_35		//	ひかりのかべ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_38		//	いちげきひっさつ
	ADRS  BasicAI_39		//	かまいたち	
	ADRS  BasicAI_40		//	いかりのまえば

	ADRS  BasicAI_41		//　りゅうのいかり2006.6.14
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_46		//	しろいきり
	ADRS  BasicAI_47		//	きあいだめ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_49		//	こんらん
	ADRS  BasicAI_50		//	攻撃２アップ

	ADRS  BasicAI_51		//	防御２アップ
	ADRS  BasicAI_52		//	素早さ２アップ
	ADRS  BasicAI_53		//	特攻２アップ
	ADRS  BasicAI_54		//	特防２アップ
	ADRS  BasicAI_55		//	命中２アップ
	ADRS  BasicAI_56		//	回避２アップ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_58		//	攻撃２ダウン
	ADRS  BasicAI_59		//	防御２ダウン
	ADRS  BasicAI_60		//	素早さ２ダウン

	ADRS  BasicAI_61		//	特攻２ダウン
	ADRS  BasicAI_62		//	特防２ダウン
	ADRS  BasicAI_63		//	命中２ダウン
	ADRS  BasicAI_64		//	回避２ダウン
	ADRS  BasicAI_65		//	リフレクター
	ADRS  BasicAI_66		//	どく33
	ADRS  BasicAI_67		//	まひ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_79		//	みがわり
	ADRS  BasicAI_80		//	はかいこうせん

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_84		//	やどりぎ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_86		//　かなしばり
	ADRS  BasicAI_87		//　ちきゅうなげ
	ADRS  BasicAI_88		//　サイコウェーブ
	ADRS  BasicAI_89		//　カウンター
	ADRS  BasicAI_90		//　アンコール

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_92		//　いびき
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_94		//　ロックオン		2006.6.14
	ADRS  BasicAI_92		//　ねごと
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_99		//　じたばた
	ADRS  BasicAI_Dummy	//	  ダミー

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_106		//	くろいまなざし
	ADRS  BasicAI_107		//　あくむ
	ADRS  BasicAI_108		// 	ちいさくなる
	ADRS  BasicAI_109		// 	のろい
	ADRS  BasicAI_Dummy	//	  ダミー

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_112		// 	まきびし
	ADRS  BasicAI_113		// 	みやぶる
	ADRS  BasicAI_114		// 	ほろび
	ADRS  BasicAI_115		// 	すなあらし
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_118		// 	いばる
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_120		// 	メロメロ

	ADRS  BasicAI_121		// 	おんがえし
	ADRS  BasicAI_122		// 	プレゼント
	ADRS  BasicAI_123		// 	やつあたり
	ADRS  BasicAI_124		// 	しんぴのまもり
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_126		// 	マグニチュード
	ADRS  BasicAI_127		//	バトンタッチ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_130		// 	ソニックブーム

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_132		// 	あさのひざし2006.6.14
	ADRS  BasicAI_133		// 	こうごうせい（シーケンス削除）2006.6.14
	ADRS  BasicAI_134		// 	つきのひかり（シーケンス削除）2006.6.14
	ADRS  BasicAI_135		//　めざめるパワー2006.6.14
	ADRS  BasicAI_136		// 	あまごい
	ADRS  BasicAI_137		// 	にほんばれ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_142		// 	はらだいこ
	ADRS  BasicAI_143		//	じこあんじ25	
	ADRS  BasicAI_144		// 	ミラーコート
	ADRS  BasicAI_145		// 	ロケットずつき
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_148		// 	みらいよち
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  AI_DEC10		// 	テレポート （基本的に使わない）
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_156		// 	まるくなる
	ADRS  BasicAI_157		// 	たまごうみ2006.6.14
	ADRS  BasicAI_158		// 	ねこだまし
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_160		// 	たくわえる

	ADRS  BasicAI_161		// 	はきだす
	ADRS  BasicAI_162		// 	のみこむ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_164		// 	あられ
	ADRS  BasicAI_165		// 	いちゃもん
	ADRS  BasicAI_166		// 	おだてる
	ADRS  BasicAI_167		//	やけど
	ADRS  BasicAI_168		// 	おきみやげ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_170		//	きあいパンチ

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_172		// 	このゆびとまれ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_175	  //	  ちょうはつ
	ADRS  BasicAI_176		// 	てだすけ
	ADRS  BasicAI_177		// 	トリック
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー

	ADRS  BasicAI_181		// 	ねをはる
	ADRS  BasicAI_182		// 	ばかぢから
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_184		// 	リサイクル
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_187		// 	あくび
	ADRS  BasicAI_188		// 	はたきおとす
	ADRS  BasicAI_189		// 	がむしゃら
	ADRS  BasicAI_Dummy	//	  ダミー

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_192		// 	ふういん
	ADRS  BasicAI_193		// 	リフレッシュ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_196		// 	けたぐり
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー

	ADRS  BasicAI_201		// 	どろあそび
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_205		// 	くすぐる
	ADRS  BasicAI_206		// 	コスモパワー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_208		// 	ビルドアップ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_210		// 	みずあそび

	ADRS  BasicAI_211		// 	めいそう
	ADRS  BasicAI_212		// 	りゅうのまい
//DPより追加シーケンス
	ADRS  BasicAI_213			// 	ほごしょく（エメラルドまでなし）
	ADRS  BasicAI_214			// 	はねやすみ	
	ADRS  BasicAI_215			// 	じゅうりょく
	ADRS  BasicAI_216			// 	ミラクルアイ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_219			// 	ジャイロボール
	ADRS  BasicAI_220			// 	いやしのねがい

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_222			// 	しぜんのめぐみ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_225			// 	おいかぜ
	ADRS  BasicAI_226			// 	つぼをつく
	ADRS  BasicAI_227			//	メタルバースト 
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_232			// 	さしおさえ
	ADRS  BasicAI_233			// 	なげつける
	ADRS  BasicAI_234			// 	サイコシフト
	ADRS  BasicAI_235			// 	きりふだ
	ADRS  BasicAI_236			// 	かいふくふうじ
	ADRS  BasicAI_237			// 	しぼりとる
	ADRS  BasicAI_238			// 	パワートリック
	ADRS  BasicAI_239			// 	いえき
	ADRS  BasicAI_240			// 	おまじない

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_242			// 	まねっこ
	ADRS  BasicAI_243			// 	パワースワップ
	ADRS  BasicAI_244			// 	ガードスワップ
	ADRS  BasicAI_245			// 	おしおき
	ADRS  BasicAI_246			// 	とっておき
	ADRS  BasicAI_247			// 	なやみのタネ
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_249			// 	どくびし
	ADRS  BasicAI_250			// 	ハートスワップ

	ADRS  BasicAI_251			// 	アクアリング
	ADRS  BasicAI_252			// 	でんじふゆう
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_258			// 	きりばらい
	ADRS  BasicAI_259			// 	トリックルーム
	ADRS  BasicAI_Dummy	//	  ダミー

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_265			// 	ゆうわく
	ADRS  BasicAI_266			// 	ステルスロック
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_270			// 	みかづきのまい

	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
	ADRS  BasicAI_Dummy	//	  ダミー
//WBより追加シーケンス
	ADRS  BasicAI_277	// 	つめとぎ
	ADRS  BasicAI_278	// 	ワイドガード
	ADRS  BasicAI_279	// 	ガードシェア
	ADRS  BasicAI_280	// 	パワーシェア

	ADRS  BasicAI_281	// 	ワンダールーム
	ADRS  BasicAI_282	// 	サイコショック
	ADRS  BasicAI_283	// 	ベノムショック
	ADRS  BasicAI_284	//	ボディパージ 	
	ADRS  BasicAI_285	// 	テレキネシス
	ADRS  BasicAI_286	// 	マジックルーム
	ADRS  BasicAI_287	// 	うちおとす
	ADRS  BasicAI_288	// 	やまあらし
	ADRS  BasicAI_289	// 	はじけるほのお
	ADRS  BasicAI_290	// 	ちょうのまい

	ADRS  BasicAI_291	// 	ヘビーボンバー
	ADRS  BasicAI_292	// 	シンクロノイズ
	ADRS  BasicAI_293	// 	エレクトボール
	ADRS  BasicAI_294	// 	みずびたし
	ADRS  BasicAI_295	// 	ニトロチャージ
	ADRS  BasicAI_296	// 	アシッドボム
	ADRS  BasicAI_297	// 	イカサマ
	ADRS  BasicAI_298	// 	シンプルビーム
	ADRS  BasicAI_299	// 	なかまづくり
	ADRS  BasicAI_300	// 	おさきにどうぞ

	ADRS  BasicAI_301	// 	りんしょう
	ADRS  BasicAI_302	// 	エコーボイス
	ADRS  BasicAI_303	// 	なしくずし
	ADRS  BasicAI_304	// 	クリアスモッグ
	ADRS  BasicAI_305	// 	アシストパワー
	ADRS  BasicAI_306	// 	ファストガード
	ADRS  BasicAI_307	// 	サイドチェンジ
	ADRS  BasicAI_308	// 	からをやぶる
	ADRS  BasicAI_309	// 	いやしのはどう
	ADRS  BasicAI_310	// 	たたりめ

	ADRS  BasicAI_311	// 	フリーフォール
	ADRS  BasicAI_312	// 	ギアチェンジ
	ADRS  BasicAI_313	// 	ともえなげ
	ADRS  BasicAI_314	// 	やきつくす
	ADRS  BasicAI_315	// 	さきおくり
	ADRS  BasicAI_316	// 	せいちょう
	ADRS  BasicAI_317	// 	アクロバット
	ADRS  BasicAI_318	// 	ミラータイプ
	ADRS  BasicAI_319	// 	かたきうち
	ADRS  BasicAI_320	// 	いのちがけ

	ADRS  BasicAI_321	// 	ほたるび
	ADRS  BasicAI_322	// 	とぐろをまく
	ADRS  BasicAI_323	// 	ギフトパス
	ADRS  BasicAI_324	// 	みずのちかい
	ADRS  BasicAI_325	// 	ほのおのちかい
	ADRS  BasicAI_326	// 	くさのちかい
	ADRS  BasicAI_327	// 	ふるいたてる
	ADRS  BasicAI_328	// 	コットンガード
	ADRS  BasicAI_329	// 	いにしえのうた
	ADRS  BasicAI_330	// 	こごえるせかい

	ADRS  BasicAI_331	// 	フリーズボルト
	ADRS  BasicAI_332	// 	コールドフレア
	ADRS  BasicAI_333	// 	どなりつける
	ADRS  BasicAI_Dummy	//	  Ｖジェネレート
	ADRS  BasicAI_Dummy	//	  フレイムソウル
	ADRS  BasicAI_Dummy	//	  サンダーソウル

BasicAI_Dummy:	//	  ダミー
BasicAISeq_end:
	AIEND


//--------------------------------------------------

BasicAI_187:// 	あくび
BasicAI_1://眠らせる
	
	IF_POKESICK	CHECK_DEFENCE,AI_DEC10	// 状態異常時は状態異常攻撃をしない

	IF_SIDEEFF	CHECK_DEFENCE, BTL_SIDEEFF_SINPINOMAMORI, AI_DEC10	// しんぴのまもり
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_HUMIN,AI_DEC10		//2006.6.14追加
	IF_EQUAL	TOKUSYU_YARUKI,AI_DEC10		//2006.6.14追加
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC10		//2010.2.13追加

	AIEND

BasicAI_7://自爆
	CHECK_WAZA_AISYOU	AISYOU_0BAI,AI_DEC10	//効果ない技はださない	

	CHECK_TOKUSEI	CHECK_ATTACK				//かたやぶりなら、しめりけを無視
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_7_katayaburi	

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_SIMERIKE,AI_DEC10		// しめりけ

BasicAI_7_katayaburi://かたやぶり
	CHECK_BENCH_COUNT	CHECK_ATTACK
	IFN_EQUAL	0,BasicAI_7_end					// 味方の控えがいる

	CHECK_BENCH_COUNT	CHECK_DEFENCE
	IFN_EQUAL	0,AI_DEC10
	JUMP		AI_DEC1

BasicAI_7_end:
	AIEND

BasicAI_107://あくむ
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_AKUMU,AI_DEC10		// あくむ中
	IFN_WAZASICK	CHECK_DEFENCE,WAZASICK_NEMURI,AI_DEC8		//寝ていない相手にはださない
	
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,AI_DEC10				// マジックガード	2006.6.6
	AIEND

BasicAI_8://ゆめくい
	IFN_WAZASICK	CHECK_DEFENCE,WAZASICK_NEMURI,AI_DEC8		//寝ていない相手にはださない
	CHECK_WAZA_AISYOU	AISYOU_0BAI,AI_DEC10				//効果ない技はださない	
	AIEND

BasicAI_142://はらだいこ
	IF_HP_UNDER	CHECK_ATTACK,51,AI_DEC10		// HPが1/2以下

BasicAI_10://こうげき１段階アップ         
BasicAI_50://こうげき2段階アップ         
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	AIEND

BasicAI_156:		// 	まるくなる
BasicAI_11://ぼうぎょ１段階アップ
BasicAI_51://ぼうぎょ2段階アップ
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_DEF,12,AI_DEC10
	AIEND

BasicAI_12://すばやさ１段階アップ
BasicAI_52://すばやさ2段階アップ
	FLDEFF_CHECK	BTL_FLDEFF_TRICKROOM,AI_DEC10		//トリックルーム中	

	IF_PARA_EQUAL	CHECK_ATTACK,PARA_AGI,12,AI_DEC10
	AIEND
	
BasicAI_13://とくこう１段階アップ
BasicAI_53://とくこう2段階アップ
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_SPEPOW,12,AI_DEC10
	AIEND

BasicAI_14://とくぼう１段階アップ
BasicAI_54://とくぼう2段階アップ
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_SPEDEF,12,AI_DEC10
	AIEND

BasicAI_15://命中率１段階アップ
BasicAI_55://命中率2段階アップ
	CHECK_TOKUSEI	CHECK_DEFENCE				
	IF_EQUAL	TOKUSYU_NOOGAADO,AI_DEC10	//敵ノーガードなら命中率上げない2006.6.8
	CHECK_TOKUSEI	CHECK_ATTACK				
	IF_EQUAL	TOKUSYU_NOOGAADO,AI_DEC10	//自分ノーガードなら命中率上げない2006.6.8

	IF_PARA_EQUAL	CHECK_ATTACK,PARA_HIT,12,AI_DEC10
	AIEND

BasicAI_108:		// 	ちいさくなる
BasicAI_16://回避率１段階アップ
BasicAI_56://回避率2段階アップ
	CHECK_TOKUSEI	CHECK_DEFENCE				
	IF_EQUAL	TOKUSYU_NOOGAADO,AI_DEC10	//敵ノーガードなら回避率上げない2006.6.8
	CHECK_TOKUSEI	CHECK_ATTACK				
	IF_EQUAL	TOKUSYU_NOOGAADO,AI_DEC10	//自分ノーガードなら回避率上げない2006.6.8

	IF_PARA_EQUAL	CHECK_ATTACK,PARA_AVOID,12,AI_DEC10
	AIEND

BasicAI_18://こうげき１段階下げる
BasicAI_58://こうげき2段階下げる
	IF_PARA_EQUAL	CHECK_DEFENCE,PARA_POW,0,AI_DEC10

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAKENKI,AI_DEC12		// まけんき2010.2.13

	CHECK_TOKUSEI	CHECK_ATTACK				//かたやぶりなら、かいりきバサミを無視  //2010.2.9
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_KURIABODHI	

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_KAIRIKIBASAMI,AI_DEC10		// かいりきバサミ
	JUMP	BasicAI_KURIABODHI

BasicAI_19://ぼうぎょ１段階下げる
BasicAI_59://ぼうぎょ2段階下げる
	IF_PARA_EQUAL	CHECK_DEFENCE,PARA_DEF,0,AI_DEC10

	CHECK_TOKUSEI	CHECK_ATTACK				//かたやぶりなら、はとむねを無視  //2010.2.13
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_KURIABODHI	

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_HATOMUNE,AI_DEC10		// はとむね2010.2.13

	JUMP	BasicAI_KURIABODHI

BasicAI_20://すばやさ１段階下げる
BasicAI_60://すばやさ2段階下げる
	FLDEFF_CHECK	BTL_FLDEFF_TRICKROOM,AI_DEC10		//トリックルーム中	
	IF_PARA_EQUAL	CHECK_DEFENCE,PARA_AGI,0,AI_DEC10

	CHECK_HAVE_TOKUSEI	CHECK_DEFENCE, TOKUSYU_KASOKU
	IF_EQUAL	HAVE_YES,	AI_DEC10

	JUMP	BasicAI_KURIABODHI

BasicAI_21://とくこう１段階下げる
BasicAI_61://とくこう2段階下げる
	IF_PARA_EQUAL	CHECK_DEFENCE,PARA_SPEPOW,0,AI_DEC10
	JUMP	BasicAI_KURIABODHI
BasicAI_22://とくぼう１段階下げる
BasicAI_62://とくぼう2段階下げる
	IF_PARA_EQUAL	CHECK_DEFENCE,PARA_SPEDEF,0,AI_DEC10
	JUMP	BasicAI_KURIABODHI
BasicAI_23://命中率１段階下げる
BasicAI_63://命中率2段階下げる
	IF_PARA_EQUAL	CHECK_DEFENCE,PARA_HIT,0,AI_DEC10
	
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_NOOGAADO,AI_DEC10		// ノーガード2006.6.8
	
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_NOOGAADO,AI_DEC10		// ノーガード2006.6.8

	CHECK_TOKUSEI	CHECK_ATTACK				//かたやぶりなら、するどいめを無視  //2010.2.9
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_KURIABODHI	

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_SURUDOIME,AI_DEC10		// するどいめ

	JUMP	BasicAI_KURIABODHI

BasicAI_24://回避率１段階下げる
BasicAI_64://回避率2段階下げる
	IF_PARA_EQUAL	CHECK_DEFENCE,PARA_AVOID,0,AI_DEC10

	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_NOOGAADO,AI_DEC10		// ノーガード2006.6.8

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_NOOGAADO,AI_DEC10		// ノーガード2006.6.8

BasicAI_KURIABODHI://クリアボディチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_AMANOZYAKU,AI_DEC12		// あまのじゃく2010.2.13

	CHECK_TOKUSEI	CHECK_ATTACK				//かたやぶりなら、クリアボディ、しろいけむりを無視  //2010.2.9
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_KURIABODHI_end	

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_KURIABODHI,AI_DEC10		// クリアボディ
	IF_EQUAL	TOKUSYU_SIROIKEMURI,AI_DEC10	// しろいけむり
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_KURIABODHI_end:
	AIEND

BasicAI_250://	ハートスワップ
BasicAI_143://	じこあんじ
BasicAI_25://くろいけむり：攻撃側が６より上で、守備側が６より下なら減算
	IF_PARA_UNDER	CHECK_ATTACK,PARA_POW,6,BasicAI_25_END
	IF_PARA_UNDER	CHECK_ATTACK,PARA_DEF,6,BasicAI_25_END
	IF_PARA_UNDER	CHECK_ATTACK,PARA_AGI,6,BasicAI_25_END
	IF_PARA_UNDER	CHECK_ATTACK,PARA_SPEPOW,6,BasicAI_25_END
	IF_PARA_UNDER	CHECK_ATTACK,PARA_SPEDEF,6,BasicAI_25_END
	IF_PARA_UNDER	CHECK_ATTACK,PARA_HIT,6,BasicAI_25_END
	IF_PARA_UNDER	CHECK_ATTACK,PARA_AVOID,6,BasicAI_25_END

	IF_PARA_OVER	CHECK_DEFENCE,PARA_POW,6,BasicAI_25_END
	IF_PARA_OVER	CHECK_DEFENCE,PARA_DEF,6,BasicAI_25_END
	IF_PARA_OVER	CHECK_DEFENCE,PARA_AGI,6,BasicAI_25_END
	IF_PARA_OVER	CHECK_DEFENCE,PARA_SPEPOW,6,BasicAI_25_END
	IF_PARA_OVER	CHECK_DEFENCE,PARA_SPEDEF,6,BasicAI_25_END
	IF_PARA_OVER	CHECK_DEFENCE,PARA_HIT,6,BasicAI_25_END
	IF_PARA_OVER	CHECK_DEFENCE,PARA_AVOID,6,BasicAI_25_END
	JUMP		AI_DEC10

BasicAI_25_END:
	AIEND

BasicAI_28://ふきとばし
	CHECK_BENCH_COUNT	CHECK_DEFENCE
	IF_EQUAL	0,AI_DEC10			//	控えがいない
	
	CHECK_TOKUSEI	CHECK_ATTACK				//2006.6.6
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_28_katayaburi	// かたやぶりチェック
	
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_KYUUBAN,AI_DEC10	// きゅうばん
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_28_katayaburi:
	AIEND


//のみこむは162、ためるカウント側でチェックした後飛んでくる
BasicAI_132://あさのひざし
BasicAI_133://こうごうせい
BasicAI_134://つきのひかり
BasicAI_157:// たまごうみ
BasicAI_214://はねやすみ2006.6.14
BasicAI_32://じこさいせい2006.6.14
	IFN_HP_EQUAL	CHECK_ATTACK,100,BasicAI_32_END
	INCDEC		-8
BasicAI_32_END:
	AIEND





BasicAI_66:		//	どく33
BasicAI_33: //どくどく
	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_HAGANE,AI_DEC10				// 相手がはがねタイプ
	IF_EQUAL	POKETYPE_DOKU,AI_DEC10			// 相手がどくタイプ 
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_HAGANE,AI_DEC10				// 相手がはがねタイプ
	IF_EQUAL	POKETYPE_DOKU,AI_DEC10			// 相手がどくタイプ

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MENEKI,AI_DEC10			// めんえき
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,AI_DEC10	// マジックガード	2006.6.6
	IF_EQUAL	TOKUSYU_POIZUNHIIRU,AI_DEC10	// ポイズンヒール	2006.6.6

	CHECK_TOKUSEI	CHECK_ATTACK				//2010.2.9
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_33_NO_RIIHUGAADO	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
 	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加
	IFN_EQUAL	TOKUSYU_RIIHUGAADO,BasicAI_33_NO_RIIHUGAADO	// 	2006.6.7
	CHECK_WEATHER
	IF_EQUAL	WEATHER_HARE,AI_DEC10			//　リーフガードではれ中

BasicAI_33_NO_RIIHUGAADO:
	CHECK_TOKUSEI	CHECK_DEFENCE
	IFN_EQUAL	TOKUSYU_URUOIBODHI,BasicAI_33_NO_URUOIBODHI	// 	2006.6.7
	CHECK_WEATHER
	IF_EQUAL	WEATHER_AME,AI_DEC10			//　うるおいボディで雨
	
BasicAI_33_NO_URUOIBODHI:
	IF_POKESICK	CHECK_DEFENCE,AI_DEC10	// 状態異常時は状態異常攻撃をしない
	IF_SIDEEFF	CHECK_DEFENCE, BTL_SIDEEFF_SINPINOMAMORI, AI_DEC10	// しんぴのまもり
	AIEND

BasicAI_35://ひかりのかべ
	IF_SIDEEFF	CHECK_ATTACK, BTL_SIDEEFF_HIKARINOKABE,AI_DEC8	// ひかりのかべ中
	AIEND

BasicAI_38://一撃必殺
	
	CHECK_WAZA_AISYOU	AISYOU_0BAI,AI_DEC10	//効果ない技はださない	
	
	CHECK_TOKUSEI	CHECK_ATTACK				//2006.6.6
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_38_katayaburi	// かたやぶりチェック
	
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_GANZYOU,AI_DEC10		// がんじょう

BasicAI_38_katayaburi:
	IF_LEVEL	LEVEL_DEFENCE,AI_DEC10			// 相手の方がレベルが高い
	AIEND

BasicAI_126:	// マグニチュード
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_126_katayaburi	// かたやぶりチェック
	
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_HUYUU,AI_DEC10			// ふゆう

BasicAI_126_katayaburi:                   //　←COMP_POWERができるまで保留

BasicAI_26:	// がまん
BasicAI_39:	// かまいたち
BasicAI_40:	// いかりのまえば
BasicAI_80:	// はかいこうせん
BasicAI_87:	// ちきゅうなげ
BasicAI_88:	// サイコウェーブ
BasicAI_89:	// カウンター
BasicAI_99:	// じたばた
BasicAI_121:// おんがえし
BasicAI_122:// プレゼント
BasicAI_123:// やつあたり
BasicAI_130:// ソニックブーム
BasicAI_144:// ミラーコート
BasicAI_145:// ロケットずつき
BasicAI_170:// きあいパンチ
BasicAI_182:// ばかぢから
BasicAI_189:// がむしゃら
BasicAI_196:// けたぐり

BasicAI_219:// ジャイロボール
BasicAI_235:// きりふだ
BasicAI_237:// しぼりとる
BasicAI_245:// おしおき
BasicAI_135:// めざめるパワー
BasicAI_41:// りゅうのいかり
	CHECK_WAZA_AISYOU	AISYOU_0BAI,AI_DEC10	//効果ない技はださない	
//現状だと倍率０だと型破りで不思議な守りを貫けるのにできない

	CHECK_TOKUSEI	CHECK_DEFENCE
	IFN_EQUAL	TOKUSYU_HUSIGINAMAMORI,BasicAI_196_end	// ふしぎなまもり

	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_196_end	// かたやぶり2006.6.7
	CHECK_WAZA_AISYOU	AISYOU_2BAI,BasicAI_196_end	// 効果ばつぐんしかあたらない	
	CHECK_WAZA_AISYOU	AISYOU_4BAI,BasicAI_196_end	// 効果ばつぐんしかあたらない	
	JUMP	AI_DEC10

BasicAI_196_end:
	AIEND


BasicAI_46://しろいきり
	IF_SIDEEFF	CHECK_ATTACK,BTL_SIDEEFF_SIROIKIRI,AI_DEC8		// しろいきり中
	AIEND

BasicAI_47://きあいだめ
  IF_CONTFLG	CHECK_ATTACK,BPP_CONTFLG_KIAIDAME,AI_DEC10		// きあいだめ中
	AIEND

BasicAI_166:// 	おだてる
BasicAI_118://いばる
BasicAI_49://混乱させる
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_KONRAN,AI_DEC5		//混乱時は混乱攻撃をしない
	IF_SIDEEFF	CHECK_DEFENCE, BTL_SIDEEFF_SINPINOMAMORI, AI_DEC10	// しんぴのまもり

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAIPEESU,AI_DEC10		// マイペース

	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_49_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_49_end:
	AIEND

BasicAI_65://リフレクター
	IF_SIDEEFF	CHECK_ATTACK,BTL_SIDEEFF_REFRECTOR,AI_DEC8	// リフレクター中
	AIEND

BasicAI_67://まひさせる
	CHECK_WAZA_AISYOU	AISYOU_0BAI,AI_DEC10	//効果ない技はださない	

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_ZYUUNAN,AI_DEC10		// じゅうなん
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,AI_DEC10	// マジックガード
	
	CHECK_TOKUSEI	CHECK_ATTACK				//2006.6.6
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_67_katayaburi	// かたやぶりチェック
		
	IF_WAZANO	WAZANO_DENZIHA,BasicAI_67_1	// でんじは
	JUMP	BasicAI_67_katayaburi

BasicAI_67_1:
	CHECK_TOKUSEI	CHECK_DEFENCE				//2006.6.6
	IF_EQUAL	TOKUSYU_DENKIENZIN,AI_DEC10	// でんきエンジン
	IF_EQUAL	TOKUSYU_TIKUDEN,AI_DEC10	// ちくでん
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加
	
BasicAI_67_katayaburi://かたやぶりスルー
	IF_POKESICK	CHECK_DEFENCE,AI_DEC10	// 状態異常時は状態異常攻撃をしない

	IF_SIDEEFF	CHECK_DEFENCE, BTL_SIDEEFF_SINPINOMAMORI, AI_DEC10	// しんぴのまもり
	AIEND

BasicAI_79://みがわり
	IF_MIGAWARI	CHECK_ATTACK,AI_DEC8		// みがわり中
	IF_HP_UNDER	CHECK_ATTACK,26,AI_DEC10				// HPが1/4以下
	AIEND

BasicAI_84://やどりぎのタネ
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_YADORIGI,AI_DEC10	// やどられ中
	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_KUSA,AI_DEC10				// 相手がくさタイプ
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_KUSA,AI_DEC10				// 相手がくさタイプ
	
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,AI_DEC10		// マジックガード	2006.6.6
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加
	AIEND
	
BasicAI_86://かなしばり
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_KANASIBARI,AI_DEC8

	CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_86_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加
BasicAI_86_end:
	AIEND

BasicAI_90://アンコール
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_ENCORE,AI_DEC8

	CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_90_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加
BasicAI_90_end:
	AIEND

BasicAI_92://いびき、ねごと
	IFN_WAZASICK	CHECK_ATTACK,WAZASICK_NEMURI,AI_DEC8		//寝ていない時はださない
	AIEND

BasicAI_94://ロックオン2006.6.14
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_MUSTHIT_TARGET,AI_DEC10			//ロックオン中
	CHECK_TOKUSEI	CHECK_ATTACK				
	IF_EQUAL	TOKUSYU_NOOGAADO,AI_DEC10
	CHECK_TOKUSEI	CHECK_DEFENCE				
	IF_EQUAL	TOKUSYU_NOOGAADO,AI_DEC10
	AIEND

BasicAI_106://クモのす、くろいまなざし、とおせんぼう
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_TOOSENBOU,AI_DEC10	// くろいまなざし中
  
  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_106_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_106_end:
	AIEND

BasicAI_109://のろい
	//2006.6.7------------------------------------------------------
	//自分がゴーストタイプかチェック
	CHECK_TYPE	CHECK_ATTACK_TYPE1
	IF_EQUAL	POKETYPE_GHOST,BasicAI_109_ghost	// ゴーストタイプ
	CHECK_TYPE	CHECK_ATTACK_TYPE2
	IF_EQUAL	POKETYPE_GHOST,BasicAI_109_ghost	// ゴーストトタイプ
	//--------------------------------------------------------------
	
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_DEF,12,AI_DEC8
	AIEND
	
	
BasicAI_109_ghost://ゴースト型のろい2006.6.7
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_NOROI,AI_DEC10	// のろい中
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,AI_DEC10		// マジックガード	2006.6.6
	AIEND

BasicAI_112://まきびし
	//IF_SIDEEFF	CHECK_DEFENCE,BTL_SIDEEFF_MAKIBISI,AI_DEC10
	//2006.6.14まきびしを３回撒いていたら
	CHECK_SIDEEFF_COUNT	CHECK_DEFENCE,BTL_SIDEEFF_MAKIBISI
	IF_EQUAL	3,AI_DEC10
	CHECK_BENCH_COUNT	CHECK_DEFENCE
	IF_EQUAL	0,AI_DEC10			//	敵の控えがいない

  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_112_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_112_end:
	AIEND

BasicAI_113://みやぶる
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_MIYABURU,AI_DEC10	// みやぶる中
  
  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_113_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_113_end:
	AIEND

BasicAI_114://ほろびのうた
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_HOROBINOUTA,AI_DEC10	// ほろび中
	AIEND

BasicAI_115://すなあらし
	CHECK_WEATHER
	IF_EQUAL	WEATHER_SUNAARASHI,AI_DEC8	// すなあらし中
	AIEND

BasicAI_120://メロメロ
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_MEROMERO,AI_DEC10		//メロメロ中
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_DONKAN,AI_DEC10				// どんかん

  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_120_katayaburi	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_120_katayaburi:
	CHECK_POKESEX	CHECK_ATTACK
	IF_EQUAL	PTL_SEX_MALE,BasicAI_120_MALE
	IF_EQUAL	PTL_SEX_FEMALE,BasicAI_120_FEMALE
	JUMP		AI_DEC10
BasicAI_120_MALE:
	CHECK_POKESEX	CHECK_DEFENCE
	IF_EQUAL	PTL_SEX_FEMALE,BasicAI_120_END
	JUMP		AI_DEC10
BasicAI_120_FEMALE:
	CHECK_POKESEX	CHECK_DEFENCE
	IF_EQUAL	PTL_SEX_MALE,BasicAI_120_END
	JUMP		AI_DEC10
BasicAI_120_END:
	AIEND

BasicAI_124://しんぴのまもり
	IF_SIDEEFF	CHECK_ATTACK,BTL_SIDEEFF_SINPINOMAMORI,AI_DEC8	// しんぴのまもり中
	AIEND

BasicAI_168://おきみやげ
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_AMANOZYAKU,AI_DEC12		// あまのじゃく2010.2.13

	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_168_check

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_KURIABODHI,AI_DEC10		// クリアボディ
	IF_EQUAL	TOKUSYU_SIROIKEMURI,AI_DEC10	// しろいけむり

BasicAI_168_check://おきみやげ
	IF_PARA_EQUAL	CHECK_DEFENCE,PARA_POW,0,AI_DEC10
	IF_PARA_EQUAL	CHECK_DEFENCE,PARA_SPEPOW,0,AI_DEC8
	CHECK_BENCH_COUNT	CHECK_ATTACK
	IF_EQUAL	0,AI_DEC10			//	味方の控えがいない
	AIEND


BasicAI_127://バトンタッチ
	CHECK_BENCH_COUNT	CHECK_ATTACK
	IF_EQUAL	0,AI_DEC10			//	味方の控えがいない
	AIEND

BasicAI_136://あまごい

	CHECK_TOKUSEI	CHECK_ATTACK	//2006.06.06
	IF_EQUAL	TOKUSYU_SUISUI,BasicAI_136_2		// すいすい
	IF_EQUAL	TOKUSYU_URUOIBODHI,BasicAI_136_2		// うるおいボディ
	
	CHECK_TOKUSEI	CHECK_DEFENCE	//2006.06.06
	IFN_EQUAL	TOKUSYU_URUOIBODHI,BasicAI_136_2		// うるおいボディでない
	IF_POKESICK	CHECK_DEFENCE,AI_DEC8	// 状態異常時
	
BasicAI_136_2:
	CHECK_WEATHER
	IF_EQUAL	WEATHER_AME,AI_DEC8		// あめ中
	AIEND

BasicAI_137://にほんばれ

//	CHECK_TOKUSEI	CHECK_ATTACK	//2006.06.06           // はれになっても問題なし 2010.2.9
//	IF_EQUAL	TOKUSYU_HURAWAAGIHUTO,BasicAI_137_2		// フラワーギフト
//	IF_EQUAL	TOKUSYU_RIIHUGAADO,BasicAI_137_2		// リーフガード
//	IF_EQUAL	TOKUSYU_SANPAWAA,BasicAI_137_2			// サンパワー
//	CHECK_TOKUSEI	CHECK_DEFENCE	//2006.06.06
//	IFN_EQUAL	TOKUSYU_URUOIBODHI,BasicAI_137_2		// リーフガードでない
//	IF_POKESICK	CHECK_DEFENCE,AI_DEC10	// 状態異常時
//BasicAI_137_2:

	CHECK_WEATHER
	IF_EQUAL	WEATHER_HARE,AI_DEC8			//　はれ中
	AIEND

BasicAI_148://みらいよち
	IF_MIRAIYOCHI	CHECK_DEFENCE,  AI_DEC12 // みらいよち中
	//INCDEC 5	//みらいよち中でない場合加算処理を削除2006.8.10
	AIEND

BasicAI_158://ねこだまし
	CHECK_NEKODAMASI	CHECK_ATTACK//１だとできる
	IF_EQUAL	0,AI_DEC10
	AIEND

BasicAI_160://たくわえる
	CHECK_TAKUWAERU		CHECK_ATTACK
	IF_EQUAL	3,AI_DEC10
	AIEND

BasicAI_161://はきだす                    //←見直し予定（ゴースト、ふしぎなまもり）
BasicAI_162://のみこむ                    //←見直し予定（ゴースト、ふしぎなまもり）
	CHECK_WAZA_AISYOU	AISYOU_0BAI,AI_DEC10	//効果ない技はださない	

	CHECK_TAKUWAERU		CHECK_ATTACK
	IF_EQUAL	0,AI_DEC10
	
	IF_WAZA_SEQNO_JUMP	162,BasicAI_32	// のみこむだったら
	AIEND


BasicAI_164://あられ
	CHECK_WEATHER
	IF_EQUAL	WEATHER_ARARE,AI_DEC8		//　あられ中

//                                              こんなのBasicじゃない　2010.2.9
//	CHECK_TOKUSEI	CHECK_DEFENCE	//2006.6.5
//	IFN_EQUAL	TOKUSYU_AISUBODHI,BasicAI_164_1	// アイスボディ
//	INCDEC		-8
//
//	CHECK_TOKUSEI	CHECK_ATTACK	//2006.6.5
//	IFN_EQUAL	TOKUSYU_AISUBODHI,BasicAI_164_1	// アイスボディ
//	INCDEC		8

BasicAI_164_1:
	AIEND


BasicAI_165://いちゃもん
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_ICHAMON,AI_DEC10	//いちゃもん中

  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_165_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_165_end:
	AIEND

BasicAI_167://やけどをおわせる
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MIZUNOBEERU,AI_DEC10	// みずのベール
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,AI_DEC10	// マジックガード
	IF_POKESICK	CHECK_DEFENCE,AI_DEC10	// 状態異常時は状態異常攻撃をしない
	
	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_HONOO,AI_DEC10			// ほのおタイプ
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_HONOO,AI_DEC10			// ほのおタイプ	
	IF_SIDEEFF	CHECK_DEFENCE, BTL_SIDEEFF_SINPINOMAMORI, AI_DEC10	// しんぴのまもり

  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_167_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_167_end:
	AIEND

BasicAI_172:			// 	このゆびとまれ
  CHECK_BTL_RULE
	IF_EQUAL		BTL_RULE_SINGLE,AI_DEC10
  AIEND

BasicAI_175://ちょうはつ
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_TYOUHATSU,AI_DEC10	// ちょうはつ中
	AIEND

BasicAI_176://てだすけ
	CHECK_BTL_RULE
	IF_EQUAL		BTL_RULE_SINGLE, AI_DEC10
	AIEND

BasicAI_188://はたきおとす
BasicAI_177://トリック
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_NENTYAKU,AI_DEC10		// ねんちゃく
	
	CHECK_SOUBI_ITEM	CHECK_DEFENCE
	IF_EQUAL	0,AI_DEC10
	
	AIEND

BasicAI_181://ねをはる
	IF_WAZASICK	CHECK_ATTACK,WAZASICK_NEWOHARU,AI_DEC10	// ねをはる中
	AIEND

BasicAI_184://リサイクル
	CHECK_RECYCLE_ITEM	CHECK_ATTACK
	IF_EQUAL	0,AI_DEC10
	AIEND

BasicAI_192://ふういん
	FLDEFF_CHECK	BTL_FLDEFF_FUIN,AI_DEC10		// ふういん中
	AIEND

BasicAI_193://リフレッシュ
	//どくまひやけどでない時はださない
  IF_WAZASICK CHECK_ATTACK, WAZASICK_DOKU,      BasicAI_193_end
  IF_DOKUDOKU CHECK_ATTACK,                     BasicAI_193_end
  IF_WAZASICK CHECK_ATTACK, WAZASICK_MAHI,      BasicAI_193_end
  IF_WAZASICK CHECK_ATTACK, WAZASICK_YAKEDO,    BasicAI_193_end
  JUMP  AI_DEC10
BasicAI_193_end://リフレッシュ
	AIEND

BasicAI_201://どろあそび
	FLDEFF_CHECK	BTL_FLDEFF_DOROASOBI,AI_DEC10	// どろあそび中
	AIEND

BasicAI_205://くすぐる
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_AMANOZYAKU,AI_DEC12		// あまのじゃく2010.2.13

	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_205_check	// かたやぶり
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_KURIABODHI,AI_DEC10				// クリアボディ
	IF_EQUAL	TOKUSYU_SIROIKEMURI,AI_DEC10			// しろいけむり
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_205_check:
	IF_PARA_EQUAL	CHECK_DEFENCE,PARA_POW,0,AI_DEC10
	IF_PARA_EQUAL	CHECK_DEFENCE,PARA_DEF,0,AI_DEC8
	AIEND

BasicAI_206://コスモパワー
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_DEF,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_SPEDEF,12,AI_DEC8
	AIEND

BasicAI_208://ビルドアップ
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_DEF,12,AI_DEC8
	AIEND

BasicAI_210://みずあそび
	FLDEFF_CHECK	BTL_FLDEFF_MIZUASOBI,AI_DEC10	// みずあそび中
	AIEND

BasicAI_211://めいそう
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_SPEPOW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_SPEDEF,12,AI_DEC8
	AIEND

BasicAI_212://りゅうのまい
	FLDEFF_CHECK	BTL_FLDEFF_TRICKROOM,AI_DEC10		//トリックルーム中	

	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_AGI,12,AI_DEC8
	AIEND

BasicAI_213://ほごしょく2006.6.5:ok
//@todo	IF_CONTFLG	CHECK_ATTACK,BPP_CONTFLG_HOGOSYOKU,AI_DEC10	// ほごしょく中
	AIEND

BasicAI_215://じゅうりょく2006.6.5：ok
	FLDEFF_CHECK	BTL_FLDEFF_JURYOKU,AI_DEC10	//じゅうりょく中はやらない	
	AIEND

BasicAI_216://ミラクルアイ2006.6.5:ok
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_MIYABURU,AI_DEC10	// ミラクルアイ中

  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_216_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_216_end:
	AIEND

BasicAI_220://いやしのねがい2006.6.5
	//繰り出しAI側未対応のためAIでは使用不可にする2006.8.10         ←見直し予定
	INCDEC	-20
	
	CHECK_BENCH_COUNT	CHECK_ATTACK
	IF_EQUAL	0,AI_DEC10			//	味方の控えがいない
	IF_BENCH_COND	CHECK_ATTACK,BasicAI_220_end//控えに状態異常がいない
	//控えポケモンにＨＰ回復できるか
	IF_BENCH_HPDEC	CHECK_ATTACK,BasicAI_220_end
	JUMP		AI_DEC10
BasicAI_220_end:
	AIEND


BasicAI_222://しぜんのめぐみ2006.6.5ok
	CHECK_SOUBI_ITEM	CHECK_ATTACK
	IFN_TABLE_JUMP	BasicAI_222_Table,AI_DEC10	//木の実がない
	CHECK_WAZA_AISYOU	AISYOU_0BAI,AI_DEC10	//効果ない技はださない	
	AIEND

BasicAI_222_Table://木の実ナンバー
	.long	149,150,151,152,153,154,155,156,157,158,
	.long	159,160,161,162,163,164,165,166,167,168,
	.long	169,170,171,172,173,174,175,176,177,178,
	.long	179,180,181,182,183,184,185,186,187,188,
	.long	189,190,191,192,193,194,195,196,197,198,
	.long	199,200,201,202,203,204,205,206,207,208,
	.long	209,210,211,212
	.long	TR_AI_TABLE_END


BasicAI_225://おいかぜ2006.6.5ok
	FLDEFF_CHECK	BTL_FLDEFF_TRICKROOM,AI_DEC8		//トリックルーム中	
	IF_SIDEEFF	CHECK_ATTACK,BTL_SIDEEFF_OIKAZE,AI_DEC10	// おいかぜ中
	AIEND

BasicAI_226://つぼをつく2006.6.5ok
	//パラメーターＭＡＸ時はしない

	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_DEF,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_AGI,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_SPEPOW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_SPEDEF,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_AVOID,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_HIT,12,AI_DEC10
	AIEND


BasicAI_227://メタルバースト2006.6.5ok
	CHECK_WAZA_AISYOU	AISYOU_0BAI,AI_DEC10	//効果ない技はださない	
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_ATODASI,AI_DEC10		//あとだし
  CHECK_SOUBI_EQUIP CHECK_DEFENCE
	IF_EQUAL	SOUBI_KOUKOUNINARU,AI_DEC10	//こうこうのしっぽ
	
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_ATODASI,BasicAI_227_end		//自分があとだし
  CHECK_SOUBI_EQUIP CHECK_ATTACK
	IF_EQUAL	SOUBI_KOUKOUNINARU,BasicAI_227_end	//こうこうのしっぽ
	
	IF_FIRST	IF_FIRST_ATTACK,AI_DEC10//自分が早い場合はしない

BasicAI_227_end:
	AIEND


BasicAI_232://さしおさえ2006.6.5ok
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_SASIOSAE,AI_DEC10	//さしおさえ中

	CHECK_RECYCLE_ITEM	CHECK_DEFENCE//リサイクルできない＝アイテム消費していない
	IF_EQUAL	0,BasicAI_232_end
	CHECK_BTL_COMPETITOR
	IF_EQUAL	BTL_COMPETITOR_SUBWAY, AI_DEC10

  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_232_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_232_end:
	AIEND

BasicAI_233://なげつける2006.6.5ok
	CHECK_WAZA_AISYOU	AISYOU_0BAI,AI_DEC10	//効果ない技はださない	
	CHECK_NAGETSUKERU_IRYOKU	CHECK_ATTACK
	IF_UNDER	10,AI_DEC10
	
	//マルチタイプの場合失敗するのでやらない
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_MARUTITAIPU,AI_DEC10	

	CHECK_SOUBI_EQUIP	CHECK_ATTACK
	IF_TABLE_JUMP	BasicAI_233_Table1,BasicAI_233_doku
	IF_TABLE_JUMP	BasicAI_233_Table2,BasicAI_233_yakedo
	IF_TABLE_JUMP	BasicAI_233_Table3,BasicAI_233_mahi
	AIEND

BasicAI_233_doku:
	IF_SIDEEFF	CHECK_DEFENCE, BTL_SIDEEFF_SINPINOMAMORI,BasicAI_233_doku_check		// しんぴのまもりok
	IF_POKESICK	CHECK_DEFENCE,BasicAI_233_doku_check			// 相手が状態異常の場合やらないok
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_POIZUNHIIRU,BasicAI_233_doku_check					// 自分がポイズンヒールなら無視ok
	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_DOKU,BasicAI_233_doku_check							// どくタイプok
	IF_EQUAL	POKETYPE_HAGANE,BasicAI_233_doku_check							// はがねタイプok
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_DOKU,BasicAI_233_doku_check							// どくタイプok
	IF_EQUAL	POKETYPE_HAGANE,BasicAI_233_doku_check							// はがねタイプok
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MENEKI,BasicAI_233_doku_check						// めんえきは無効なのでやらない
	IF_EQUAL	TOKUSYU_POIZUNHIIRU,BasicAI_233_doku_check					// ポイズンヒールは無効なのでやらない
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,BasicAI_233_doku_check			// 相手がマジックガードなやらない
	AIEND

BasicAI_233_doku_check:
	IF_SIDEEFF	CHECK_ATTACK, BTL_SIDEEFF_SINPINOMAMORI,AI_DEC5	// しんぴのまもりok
	IF_POKESICK	CHECK_ATTACK,AI_DEC5				// 状態異常の場合やらないok
	
	CHECK_TYPE	CHECK_ATTACK_TYPE1
	IF_EQUAL	POKETYPE_DOKU,AI_DEC5						// どくタイプok
	IF_EQUAL	POKETYPE_HAGANE,AI_DEC5						// はがねタイプok
	CHECK_TYPE	CHECK_ATTACK_TYPE2
	IF_EQUAL	POKETYPE_DOKU,AI_DEC5						// どくタイプok
	IF_EQUAL	POKETYPE_HAGANE,AI_DEC5						// はがねタイプok
	
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_BUKIYOU,AI_DEC5					// ぶきよう
	IF_EQUAL	TOKUSYU_MENEKI,AI_DEC5					// めんえき
	IF_EQUAL	TOKUSYU_POIZUNHIIRU,AI_DEC5				// ポイズンヒール
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,AI_DEC5			// マジックガード
	IF_EQUAL	TOKUSYU_KONZYOU,AI_DEC5					// こんじょう
//	INCDEC	+3	//危険なので投げる                              Basicでは扱わない。  2010.2.10

	AIEND


BasicAI_233_yakedo:
	IF_SIDEEFF	CHECK_DEFENCE, BTL_SIDEEFF_SINPINOMAMORI,BasicAI_233_yakedo_check	// しんぴのまもり
	IF_POKESICK	CHECK_DEFENCE,BasicAI_233_yakedo_check			// 相手が状態異常の場合やらないok
	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_HONOO,BasicAI_233_yakedo_check						// ほのおタイプok
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_HONOO,BasicAI_233_yakedo_check						// ほのおトタイプok
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,BasicAI_233_yakedo_check			// 相手がマジックガードなやらない
	IF_EQUAL	TOKUSYU_MIZUNOBEERU,BasicAI_233_yakedo_check			// みずのベール
	AIEND

BasicAI_233_yakedo_check:
	IF_SIDEEFF	CHECK_ATTACK, BTL_SIDEEFF_SINPINOMAMORI,AI_DEC5	// しんぴのまもり
	IF_POKESICK	CHECK_ATTACK,AI_DEC5				// 状態異常の場合やらないok
	CHECK_TYPE	CHECK_ATTACK_TYPE1
	IF_EQUAL	POKETYPE_HONOO,AI_DEC5						// ほのおタイプok
	CHECK_TYPE	CHECK_ATTACK_TYPE2
	IF_EQUAL	POKETYPE_HONOO,AI_DEC5						// ほのおトタイプok
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_BUKIYOU,AI_DEC5					// ぶきよう
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,AI_DEC5			// マジックガード
	IF_EQUAL	TOKUSYU_MIZUNOBEERU,AI_DEC5				// みずのベール
	IF_EQUAL	TOKUSYU_KONZYOU,AI_DEC5					// こんじょう
//	INCDEC	+3	//危険なので投げる                              Basicでは扱わない。  2010.2.10

	AIEND

BasicAI_233_mahi:
	IF_SIDEEFF	CHECK_DEFENCE, BTL_SIDEEFF_SINPINOMAMORI,AI_DEC5	// しんぴのまもり
	IF_POKESICK	CHECK_DEFENCE,AI_DEC5			// 相手が状態異常の場合やらない
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_ZYUUNAN,AI_DEC5					// じゅうなんは無効なのでやらない
	AIEND


BasicAI_233_Table1://どく
	.long	SOUBI_TEKINIMOTASERUTOMOUDOKU
	.long	SOUBI_DOKUBARIUP
	.long	TR_AI_TABLE_END

BasicAI_233_Table2://やけど
	.long	SOUBI_TTEKINIMOTASERUTOYAKEDO
	.long	TR_AI_TABLE_END

BasicAI_233_Table3://まひ	
	.long	SOUBI_PIKATYUUTOKUKOUNIBAI
	.long	TR_AI_TABLE_END
	

BasicAI_234://サイコシフト2006.6.5
	
	IFN_POKESICK	CHECK_ATTACK,AI_DEC10		// 自分が状態異常でない場合やらないok
	IF_POKESICK	CHECK_DEFENCE,AI_DEC10			// 相手が状態異常の場合やらないok

  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_234_katayaburi	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC10		//マジックミラー2010.2.13追加

BasicAI_234_katayaburi:
	IF_SIDEEFF	CHECK_DEFENCE, BTL_SIDEEFF_SINPINOMAMORI, AI_DEC10	// しんぴのまもり
	IF_WAZASICK	CHECK_ATTACK, WAZASICK_DOKU,    BasicAI_234_doku	  // 毒
	IF_DOKUDOKU	CHECK_ATTACK,                   BasicAI_234_doku	  // どくどく
	IF_WAZASICK	CHECK_ATTACK, WAZASICK_YAKEDO,  BasicAI_234_yakedo	// やけど
	IF_WAZASICK	CHECK_ATTACK, WAZASICK_MAHI,    BasicAI_234_mahi	  // まひ
	JUMP	BasicAI_234_end
	
BasicAI_234_doku:
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_POIZUNHIIRU,AI_DEC10		// 自分がポイズンヒールなら無視ok
	
	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_DOKU,AI_DEC10		// どくタイプok
	IF_EQUAL	POKETYPE_HAGANE,AI_DEC10			// はがねタイプok
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_DOKU,AI_DEC10		// どくタイプok
	IF_EQUAL	POKETYPE_HAGANE,AI_DEC10			// はがねタイプok
	
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MENEKI,AI_DEC10		// めんえきは無効なのでやらない
	IF_EQUAL	TOKUSYU_POIZUNHIIRU,AI_DEC10// ポイズンヒールは無効なのでやらない
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,AI_DEC10// 相手がマジックガードなやらない
	JUMP	BasicAI_234_end
	
	//やけどのとき、火タイプにはやらない
BasicAI_234_yakedo:
	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IF_EQUAL	POKETYPE_HONOO,AI_DEC10			// ほのおタイプok
	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_EQUAL	POKETYPE_HONOO,AI_DEC10			// ほのおタイプok
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUGAADO,AI_DEC10			// 相手がマジックガードなやらない
	IF_EQUAL	TOKUSYU_MIZUNOBEERU,AI_DEC10			// みずのベール
	JUMP	BasicAI_234_end

BasicAI_234_mahi:
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_ZYUUNAN,AI_DEC10		// じゅうなんは無効なのでやらない

BasicAI_234_end:
	AIEND


BasicAI_236://かいふくふうじ2006.6.5ok
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_KAIHUKUHUUJI,AI_DEC10	//かいふくふうじ中

  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_236_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_236_end:
	AIEND

BasicAI_238://パワートリック2006.6.14ok
	IF_CONTFLG	CHECK_ATTACK,BPP_CONTFLG_POWERTRICK,AI_DEC10	//パワートリック中
	AIEND

BasicAI_239://いえき2006.6.5ok

	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_IEKI,AI_DEC10	// いえき中
	
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MARUTITAIPU,AI_DEC10		// マルチタイプは無効なのでやらないok
	IF_EQUAL	TOKUSYU_NAMAKE,AI_DEC10				// なまけは相手が有利なのでやらないok
	IF_EQUAL	TOKUSYU_YOWAKI,AI_DEC10				// よわきは相手が有利なのでやらない
	IF_EQUAL	TOKUSYU_SUROOSUTAATO,AI_DEC10		// スロースタートは相手が有利なのでやらないok
	IF_EQUAL	TOKUSYU_AKUSYUU,AI_DEC10			// 戦闘で意味ないのでやらないok
	IF_EQUAL	TOKUSYU_NIGEASI,AI_DEC10			// 戦闘で意味ないのでやらないok
	IF_EQUAL	TOKUSYU_MONOHIROI,AI_DEC10			// 戦闘で意味ないのでやらないok
	IF_EQUAL	TOKUSYU_MITUATUME,AI_DEC10			// 戦闘で意味ないのでやらないok
  
  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_239_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_239_end:
	AIEND

BasicAI_240://おまじない2006.6.5ok
	IF_SIDEEFF	CHECK_ATTACK,BTL_SIDEEFF_OMAJINAI,AI_DEC10	//おまじない中
	AIEND

BasicAI_242://まねっこ2006.6.5ok
	CHECK_TURN
	IFN_EQUAL	0,BasicAI_242_end	//最初のターン

  IF_FIRST	IF_FIRST_DEFENCE,BasicAI_242_end          //　相手の方が素早い  2010.2.11

	IF_FIRST	IF_FIRST_ATTACK,AI_DEC10
BasicAI_242_end:
	AIEND

BasicAI_243://パワースワップ2006.6.5ok
	//相手より自分の方が段階数が高い場合はしない
	CHECK_STATUS_DIFF	CHECK_DEFENCE,BPP_ATTACK_RANK
	IF_UNDER		1,BasicAI_243_1
	JUMP	BasicAI_243_End

BasicAI_243_1:
	CHECK_STATUS_DIFF	CHECK_DEFENCE,BPP_SP_ATTACK_RANK
	IF_UNDER		1,AI_DEC10

BasicAI_243_End:
	AIEND

BasicAI_244://ガードスワップ2006.6.5ok
	//相手より自分の方が段階数が高い場合はしない
	CHECK_STATUS_DIFF	CHECK_DEFENCE,BPP_DEFENCE_RANK
	IF_UNDER		1,BasicAI_244_1
	JUMP	BasicAI_244_End

BasicAI_244_1:
	CHECK_STATUS_DIFF	CHECK_DEFENCE,BPP_SP_DEFENCE_RANK
	IF_UNDER		1,AI_DEC10

BasicAI_244_End:
	AIEND

BasicAI_246://とっておき2006.6.5ok
	IF_TOTTEOKI	CHECK_ATTACK,BasicAI_246_end		// とっておき出せる
	INCDEC	-10
BasicAI_246_end:
	AIEND


BasicAI_247://なやみのタネ2006.6.5ok

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_NAMAKE,AI_DEC10			// なまけは無効なのでやらないok
	IF_EQUAL	TOKUSYU_YOWAKI,AI_DEC10				// よわきは相手が有利なのでやらない
	IF_EQUAL	TOKUSYU_HUMIN,AI_DEC10			// ふみんは無意味なのでやらないok
	IF_EQUAL	TOKUSYU_YARUKI,AI_DEC10			// やるきは無意味なのでやらないok
	IF_EQUAL	TOKUSYU_MARUTITAIPU,AI_DEC10	// マルチタイプは無意味なのでやらないok
	
	IFN_WAZASICK	CHECK_DEFENCE,WAZASICK_NEMURI,BasicAI_247_end	//ねむっていないok

	IF_HAVE_WAZA	CHECK_DEFENCE, WAZANO_NEGOTO, BasicAI_247_end
	IF_HAVE_WAZA	CHECK_DEFENCE, WAZANO_IBIKI, BasicAI_247_end
	INCDEC	-10

  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_247_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_247_end:
	AIEND

BasicAI_249://どくびし2006.6.5ok
	CHECK_SIDEEFF_COUNT	CHECK_DEFENCE,BTL_SIDEEFF_DOKUBISI
	IF_EQUAL	2,AI_DEC10
	CHECK_BENCH_COUNT	CHECK_DEFENCE
	IF_EQUAL	0,AI_DEC10			//	敵の控えがいない
	//控えに状態異常が一匹もいない＆毒＆鋼タイプもいない（やめておく）
  
  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_249_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_249_end:
	AIEND

BasicAI_251://アクアリング2006.6.5ok
	IF_WAZASICK	CHECK_ATTACK,WAZASICK_AQUARING,AI_DEC10	// アクアリング中
	AIEND

BasicAI_252://でんじふゆう2006.6.5ok
	IF_WAZASICK	CHECK_ATTACK,WAZASICK_FLYING,AI_DEC10	// でんじふゆう中
	
	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_HUYUU,AI_DEC10		// ふゆうは無意味なのでやらない
	
	CHECK_TYPE	CHECK_ATTACK_TYPE1
	IF_EQUAL	POKETYPE_HIKOU,AI_DEC10	// ひこうタイプ
	CHECK_TYPE	CHECK_ATTACK_TYPE2
	IF_EQUAL	POKETYPE_HIKOU,AI_DEC10	// ひこうトタイプ

	AIEND

BasicAI_258://きりばらい2006.6.5
	//相手の回避率０かつ、飛ばせるものがない場合はやらない	
	IFN_PARA_EQUAL	CHECK_DEFENCE,PARA_AVOID,0,BasicAI_258_END
	IF_SIDEEFF	CHECK_DEFENCE,BTL_SIDEEFF_HIKARINOKABE,BasicAI_258_END	// ひかりのかべ
	IF_SIDEEFF	CHECK_DEFENCE,BTL_SIDEEFF_REFRECTOR,BasicAI_258_END	// リフレクター

  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_258_katayaburi	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_258_katayaburi:
	
	CHECK_WEATHER
	IF_EQUAL	WEATHER_HUKAIKIRI,BasicAI_258_END	//　ふかいきりok

//	                                                               現在は「きりばらい」で「まきびし」等が消せてない。2010.2.11
	CHECK_BENCH_COUNT	CHECK_DEFENCE
	IF_EQUAL	0,AI_DEC10			//	敵の控えがいない

	IF_SIDEEFF	CHECK_DEFENCE,BTL_SIDEEFF_MAKIBISI,BasicAI_258_END		// まきびし中
	IF_SIDEEFF	CHECK_DEFENCE,BTL_SIDEEFF_STEALTHROCK,BasicAI_258_END	// ステルスロック中
	IF_SIDEEFF	CHECK_DEFENCE,BTL_SIDEEFF_DOKUBISI,BasicAI_258_END		// どくびし中
	
	JUMP		AI_DEC10

BasicAI_258_END:
	AIEND


BasicAI_259://トリックルーム2006.6.14ok
	IF_FIRST	IF_FIRST_ATTACK,AI_DEC10	//アタックの方が早い
	IF_FIRST	IF_FIRST_EQUAL,AI_DEC10		//同じ
	AIEND


BasicAI_265://ゆうわく2006.6.5ok
	
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_AMANOZYAKU,AI_DEC12		// あまのじゃく2010.2.13

	CHECK_TOKUSEI	CHECK_ATTACK
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_265_check	// かたやぶり
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_DONKAN,AI_DEC10					// どんかん
	IF_EQUAL	TOKUSYU_KURIABODHI,AI_DEC10				// クリアボディ
	IF_EQUAL	TOKUSYU_SIROIKEMURI,AI_DEC10			// しろいけむり
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_265_check:	
	//性別が同じ、無性別の場合やらない
	CHECK_POKESEX	CHECK_ATTACK
	IF_EQUAL	PTL_SEX_MALE,BasicAI_265_MALE
	IF_EQUAL	PTL_SEX_FEMALE,BasicAI_265_FEMALE
	JUMP		AI_DEC10
BasicAI_265_MALE:
	CHECK_POKESEX	CHECK_DEFENCE
	IF_EQUAL	PTL_SEX_FEMALE,BasicAI_265_1
	JUMP		AI_DEC10
BasicAI_265_FEMALE:
	CHECK_POKESEX	CHECK_DEFENCE
	IF_EQUAL	PTL_SEX_MALE,BasicAI_265_1
	JUMP		AI_DEC10

BasicAI_265_1:
	IF_PARA_UNDER	CHECK_DEFENCE,PARA_SPEPOW,1,AI_DEC10
	AIEND

BasicAI_266://ステルスロック2006.6.5ok
	IF_SIDEEFF	CHECK_DEFENCE,BTL_SIDEEFF_STEALTHROCK,AI_DEC10 // ステルスロック中
	CHECK_BENCH_COUNT	CHECK_DEFENCE
	IF_EQUAL	0,AI_DEC10			//	敵の控えがいない
  
  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_266_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_266_end:
	AIEND

BasicAI_270://みかづきのまい2006.6.5
	//繰り出しAI側未対応のためAIでは使用不可にする2006.8.10
	INCDEC	-20
	CHECK_BENCH_COUNT	CHECK_ATTACK
	IF_EQUAL	0,AI_DEC10			//	味方の控えがいないok
	//控えポケモンにＨＰ回復できるかok
	IF_BENCH_HPDEC	CHECK_ATTACK,BasicAI_270_end
	//控えポケモンに状態異常回復できるかどうかok
	IF_BENCH_COND	CHECK_ATTACK,BasicAI_270_end
	//控えポケモンにＰＰ回復できるかどうかok
	IF_BENCH_PPDEC	CHECK_ATTACK,BasicAI_270_end
	JUMP		AI_DEC10
BasicAI_270_end:
	AIEND

BasicAI_277:			// 	つめとぎ
	CHECK_TOKUSEI	CHECK_ATTACK				//たんじゅんなら段階数９でやめ2006.6.6  ←見直し予定
	IFN_EQUAL	TOKUSYU_TANZYUN,BasicAI_277_tanzyun_no

	IF_PARA_OVER	CHECK_ATTACK,PARA_POW,8,AI_DEC10
	IF_PARA_OVER	CHECK_ATTACK,PARA_HIT,8,AI_DEC8

BasicAI_277_tanzyun_no:
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_HIT,12,AI_DEC8
	AIEND

BasicAI_278:			// 	ワイドガード
	CHECK_BTL_RULE
	IF_EQUAL		BTL_RULE_SINGLE, AI_DEC10
	AIEND

BasicAI_279:			// 	ガードシェア
	AIEND
BasicAI_280:			// 	パワーシェア
	AIEND

BasicAI_281:			// 	ワンダールーム
	FLDEFF_CHECK	BTL_FLDEFF_WONDERROOM,AI_DEC10		//ワンダールーム中	
	AIEND

BasicAI_282:			// 	サイコショック
	AIEND
BasicAI_283:			// 	ベノムショック
	AIEND

BasicAI_284:			//	ボディパージ 	
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_AGI,12,AI_DEC10
	AIEND

BasicAI_285:			// 	テレキネシス
	IF_WAZASICK	CHECK_DEFENCE,WAZASICK_TELEKINESIS,AI_DEC10	// テレキネシス中
  
  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_285_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_285_end:

	AIEND

BasicAI_286:			// 	マジックルーム
	FLDEFF_CHECK	BTL_FLDEFF_MAGICROOM,AI_DEC10		//マジックルーム中	
	AIEND

BasicAI_287:			// 	うちおとす
	AIEND
BasicAI_288:			// 	やまあらし
	AIEND
BasicAI_289:			// 	はじけるほのお
	AIEND
BasicAI_290:			// 	ちょうのまい
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_DEF,12,AI_DEC8
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_AGI,12,AI_DEC6
	AIEND

BasicAI_291:			// 	ヘビーボンバー
	AIEND

BasicAI_292:			// 	シンクロノイズ   
	AIEND

BasicAI_293:			// 	エレクトボール
	AIEND

BasicAI_294:			// 	みずびたし
	CHECK_TYPE	CHECK_DEFENCE_TYPE1
	IFN_WAZA_TYPE	POKETYPE_MIZU,BasicAI_294_end		//相手のタイプ１が水じゃない

	CHECK_TYPE	CHECK_DEFENCE_TYPE2
	IF_WAZA_TYPE	POKETYPE_MIZU,AI_DEC10       		//相手のタイプ２も水

BasicAI_294_end:
	AIEND

BasicAI_295:			// 	ニトロチャージ
	AIEND
BasicAI_296:			// 	アシッドボム
	AIEND
BasicAI_297:			// 	イカサマ
	AIEND

BasicAI_298:			// 	シンプルビーム
	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MARUTITAIPU,AI_DEC10		// マルチタイプは無効なのでやらないok
	IF_EQUAL	TOKUSYU_NAMAKE,AI_DEC10				// なまけは相手が有利なのでやらないok
	IF_EQUAL	TOKUSYU_YOWAKI,AI_DEC10				// よわきは相手が有利なのでやらない
	IF_EQUAL	TOKUSYU_SUROOSUTAATO,AI_DEC10		// スロースタートは相手が有利なのでやらないok
  
  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_298_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_298_end:

	AIEND

BasicAI_299:			// 	なかまづくり          

  CHECK_TOKUSEI	CHECK_ATTACK		
	IF_EQUAL	TOKUSYU_KATAYABURI,BasicAI_299_end	// かたやぶりチェック

	CHECK_TOKUSEI	CHECK_DEFENCE
	IF_EQUAL	TOKUSYU_MAZIKKUMIRAA,AI_DEC12		//マジックミラー2010.2.13追加

BasicAI_299_end:

	AIEND

BasicAI_300:			// 	おさきにどうぞ
	CHECK_BTL_RULE
	IF_EQUAL		BTL_RULE_SINGLE, AI_DEC10
	AIEND

BasicAI_301:			// 	りんしょう
	CHECK_BTL_RULE
	IF_EQUAL		BTL_RULE_SINGLE, AI_DEC10
	AIEND

BasicAI_302:			// 	エコーボイス
	AIEND

BasicAI_303:			// 	なしくずし
	AIEND

BasicAI_304:			// 	クリアスモッグ
	AIEND

BasicAI_305:			// 	アシストパワー
	AIEND

BasicAI_306:			// 	ファストガード
	AIEND

BasicAI_307:			// 	サイドチェンジ
	CHECK_BTL_RULE
	IF_EQUAL		BTL_RULE_SINGLE, AI_DEC10
	AIEND

BasicAI_308:			// 	からをやぶる
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_SPEPOW,12,AI_DEC8
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_AGI,12,AI_DEC6
	AIEND

BasicAI_309:			// 	いやしのはどう
	CHECK_BTL_RULE
	IF_EQUAL		BTL_RULE_SINGLE, AI_DEC10
	AIEND

BasicAI_310:			// 	たたりめ
	AIEND

BasicAI_311:			// 	フリーフォール
	AIEND

BasicAI_312:			// 	ギアチェンジ
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_AGI,12,AI_DEC8
	AIEND

BasicAI_313:			// 	ともえなげ
	AIEND

BasicAI_314:			// 	やきつくす
	AIEND

BasicAI_315:			// 	さきおくり
	CHECK_BTL_RULE
	IF_EQUAL		BTL_RULE_SINGLE, AI_DEC10
	AIEND

BasicAI_316:			// 	せいちょう
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_SPEPOW,12,AI_DEC8
	AIEND

BasicAI_317:			// 	アクロバット
	AIEND

BasicAI_318:			// 	ミラータイプ     
	AIEND

BasicAI_319:			// 	かたきうち
	AIEND

BasicAI_320:			// 	いのちがけ
	CHECK_BENCH_COUNT	CHECK_ATTACK
	IFN_EQUAL	0,BasicAI_320_end					// 味方の控えがいる

	CHECK_BENCH_COUNT	CHECK_DEFENCE
	IFN_EQUAL	0,AI_DEC10
	JUMP		AI_DEC1

BasicAI_320_end:
	AIEND

BasicAI_321:			// 	ほたるび
	AIEND

BasicAI_322:			// 	とぐろをまく
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_DEF,12,AI_DEC8
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_HIT,12,AI_DEC6
	AIEND

BasicAI_323:			// 	ギフトパス
	CHECK_SOUBI_ITEM	CHECK_DEFENCE
	IFN_EQUAL	0,AI_DEC10

	CHECK_BTL_RULE
	IF_EQUAL		BTL_RULE_SINGLE, AI_DEC10
	AIEND

BasicAI_324:			// 	みずのちかい
	AIEND
BasicAI_325:			// 	ほのおのちかい
	AIEND
BasicAI_326:			// 	くさのちかい
	AIEND
BasicAI_327:			// 	ふるいたてる
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_POW,12,AI_DEC10
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_SPEPOW,12,AI_DEC8
	AIEND

BasicAI_328:			// 	コットンガード
	IF_PARA_EQUAL	CHECK_ATTACK,PARA_DEF,12,AI_DEC10
	AIEND

BasicAI_329:	// 	いにしえのうた
	AIEND
BasicAI_330:	// 	こごえるせかい
	AIEND
BasicAI_331:	// 	フリーズボルト
	AIEND
BasicAI_332:	// 	コールドフレア
	AIEND
BasicAI_333:	// 	どなりつける
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

