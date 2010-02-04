#!/usr/bin/perl

#===============================================================================
# ワザデータコンバータ for WB
#
# FileMakerから出力した csv ファイル（UTF-8）を読み込み、
# 以下のファイルを生成します。
#
# ・waza_tbl.narc, naix（ワザデータテーブル）
#
# GameFreak taya
#
#===============================================================================

use Encode;

#============================================================================================
# Tables
#============================================================================================
#----------------------------------------------------------------------------------
# レコード順番（waza.tabの並び順と一致している必要があります）
#----------------------------------------------------------------------------------

use constant IDX_WAZANAME			=> 0;	# 名称
use constant IDX_ATKMSG				=> 1;	# 攻撃メッセージ
use constant IDX_INFOMSG1			=> 2;	# 説明文１
use constant IDX_INFOMSG2			=> 3;	# 説明文２
use constant IDX_INFOMSG3			=> 4;	# 説明文３
use constant IDX_INFOMSG4			=> 5;	# 説明文４
use constant IDX_INFOMSG5			=> 6;	# 説明文５
use constant IDX_Type				=> 7;	# タイプ
use constant IDX_Category			=> 8;	# カテゴリ
use constant IDX_DamageType			=> 9;	# ダメージタイプ
use constant IDX_Power				=> 10;	# 威力
use constant IDX_HitPer				=> 11;	# 命中率
use constant IDX_BasePP				=> 12;	# 基本PPMax
use constant IDX_Priority			=> 13;	# 優先度
use constant IDX_HitCntMax			=> 14;	# 最大ヒット回数
use constant IDX_HitCntMin			=> 15;	# 最小ヒット回数
use constant IDX_SickID				=> 16;	# 状態異常効果
use constant IDX_SickPer			=> 17;	# 状態異常の発生率
use constant IDX_SickCont			=> 18;	# 状態異常の継続パターン
use constant IDX_SickTurnMin		=> 19;	# 状態異常の継続ターン数　最小
use constant IDX_SickTurnMax		=> 20;	# 状態異常の継続ターン数　最大
use constant IDX_CriticalRank		=> 21;	# クリティカルランク
use constant IDX_ShrinkPer			=> 22;	# ひるみ確率
use constant IDX_SeqNo				=> 23;	# AI用シーケンスナンバー
use constant IDX_DmgRecoverRatio	=> 24;	# ダメージ回復率
use constant IDX_HPRecoverRatio		=> 25;	# ＨＰ回復率
use constant IDX_Target				=> 26;	# 効果範囲
use constant IDX_RankEffType1		=> 27;	# ステータス効果１
use constant IDX_RankEffType2		=> 28;	# ステータス効果２
use constant IDX_RankEffType3		=> 29;	# ステータス効果３
use constant IDX_RankEffValue1		=> 30;	# ステータス効果１の変動値
use constant IDX_RankEffValue2		=> 31;	# ステータス効果２の変動値
use constant IDX_RankEffValue3		=> 32;	# ステータス効果３の変動値
use constant IDX_RankEffPer1		=> 33;	# ステータス効果１の発生率
use constant IDX_RankEffPer2		=> 34;	# ステータス効果２の発生率
use constant IDX_RankEffPer3		=> 35;	# ステータス効果３の発生率
use constant IDX_FLG_Touch			=> 36;	# 【接触ワザ】
use constant IDX_FLG_Tame			=> 37;	# 【１ターン溜め】
use constant IDX_FLG_Tire			=> 38;	# 【１ターン反動】
use constant IDX_FLG_Mamoru			=> 39;	# 【まもる対象】
use constant IDX_FLG_MagicCoat		=> 30;	# 【マジックコート対象】
use constant IDX_FLG_Yokodori		=> 41;	# 【よこどり対象】
use constant IDX_FLG_Oumu			=> 42;	# 【オウムがえし対象】
use constant IDX_FLG_TetsuNoKobusi	=> 43;	# 【てつのこぶし対象】
use constant IDX_FLG_Bouon			=> 44;	# 【ぼうおん対象】
use constant IDX_FLG_Juryoku		=> 45;	# 【重力で失敗する】
use constant IDX_FLG_KoriMelt		=> 46;	# 【凍り解除】
use constant IDX_FLG_TripleFar		=> 47;	# 【トリプル遠隔ワザ】

use constant IDX_FLG_End			=> 48;	# フラグ最後（waza.tabにはありません）
use constant IDX_FLG_Start			=> 36;	# フラグ最初


#----------------------------------------------------------------------------------
#レコード名称（レコード順番に従って自動でソートされます）
#----------------------------------------------------------------------------------
%RecOrder = (
	"名称",				IDX_WAZANAME,
	"攻撃メッセージ",	IDX_ATKMSG,
	"説明文１",			IDX_INFOMSG1,
	"説明文２",			IDX_INFOMSG2,
	"説明文３",			IDX_INFOMSG3,
	"説明文４",			IDX_INFOMSG4,
	"説明文５",			IDX_INFOMSG5,
	"タイプ",			IDX_Type,
	"カテゴリ",			IDX_Category,
	"ダメージタイプ",	IDX_DamageType,
	"威力",				IDX_Power,
	"命中率",			IDX_HitPer,
	"基本PPMax",		IDX_BasePP,
	"優先度",			IDX_Priority,
	"クリティカルランク",	IDX_CriticalRank,
	"最大ヒット回数",		IDX_HitCntMax,
	"最小ヒット回数",		IDX_HitCntMin,
	"ひるみ確率",			IDX_ShrinkPer,
	"状態異常効果",			IDX_SickID,
	"状態異常の発生率",		IDX_SickPer,
	"状態異常の継続パターン",	IDX_SickCont,
	"状態異常の継続ターン数　最小",	IDX_SickTurnMin,
	"状態異常の継続ターン数　最大",	IDX_SickTurnMax,
	"ステータス効果１",				IDX_RankEffType1,
	"ステータス効果２",				IDX_RankEffType2,
	"ステータス効果３",				IDX_RankEffType3,
	"ステータス効果１の変動値",		IDX_RankEffValue1,
	"ステータス効果２の変動値",		IDX_RankEffValue2,
	"ステータス効果３の変動値",		IDX_RankEffValue3,
	"ステータス効果１の発生率",		IDX_RankEffPer1,
	"ステータス効果２の発生率",		IDX_RankEffPer2,
	"ステータス効果３の発生率",		IDX_RankEffPer3,
	"ダメージ回復率",				IDX_DmgRecoverRatio,
	"ＨＰ回復率",					IDX_HPRecoverRatio,
	"効果範囲",						IDX_Target,
	"詳細説明",						IDX_SeqNo,
	"【接触ワザ】",					IDX_FLG_Touch,
	"【１ターン溜め】",				IDX_FLG_Tame,
	"【１ターン反動】",				IDX_FLG_Tire,
	"【まもる対象】",				IDX_FLG_Mamoru,
	"【マジックコート対象】",		IDX_FLG_MagicCoat,
	"【よこどり対象】",				IDX_FLG_Yokodori,
	"【オウムがえし対象】",			IDX_FLG_Oumu,
	"【てつのこぶし対象】",			IDX_FLG_TetsuNoKobusi,
	"【ぼうおん対象】",				IDX_FLG_Bouon,
	"【重力で失敗する】",			IDX_FLG_Juryoku,
	"【凍り解除】",					IDX_FLG_KoriMelt,
	"【トリプル遠隔ワザ】",			IDX_FLG_TripleFar,
);

#----------------------------------------------------------------------------------
# タイプ名（順番はポケモンプログラム内の定数と一致している必要があります）
# 参照：poketool/poketype_def.h
#----------------------------------------------------------------------------------
@TypeName = (
	"ノーマル",
	"格闘",
	"飛行",
	"毒",
	"地面",
	"岩",
	"虫",
	"ゴースト",
	"鋼",
	"炎",
	"水",
	"草",
	"電気",
	"エスパー",
	"氷",
	"ドラゴン",
	"悪",
);
#----------------------------------------------------------------------------------
# ワザカテゴリ名（順番はポケモンプログラム内の定数と一致している必要があります）
# 参照：waza_tool/wazadata.h
#----------------------------------------------------------------------------------
@CategoryName = (
	"シンプルダメージ",
	"シンプルシック",
	"シンプルエフェクト",
	"シンプルリカバー",
	"ダメージシック",
	"エフェクトシック",
	"ダメージエフェクト",
	"ダメージエフェクトユーザー",
	"ダメージドレイン",
	"一撃",
	"フィールドエフェクト",
	"サイドエフェクト",
	"プッシュアウト",
	"その他",
);

use constant CATEGORY_SIMPLE_DAMAGE			=> 0;	# 
use constant CATEGORY_SIMPLE_SICK			=> 1;	# 攻撃メッセージ
use constant CATEGORY_SIMPLE_EFFECT			=> 2;	# 説明文１
use constant CATEGORY_SIMPLE_RECOVER		=> 3;	# 説明文２
use constant CATEGORY_DAMAGE_SICK			=> 4;	# 説明文３
use constant CATEGORY_EFFECT_SICK			=> 5;	# カテゴリ
use constant CATEGORY_DAMAGE_EFFECT			=> 6;	# 説明文５
use constant CATEGORY_DAMAGE_EFFECT_USER	=> 7;	# タイプ
use constant CATEGORY_DAMAGE_DRAIN			=> 8;	# カテゴリ
use constant CATEGORY_ICHIGEKI				=> 9;	# カテゴリ
use constant CATEGORY_FIELD_EFFECT			=> 10;	# カテゴリ
use constant CATEGORY_SIDE_EFFECT			=> 11;	# カテゴリ
use constant CATEGORY_PUSHOUT				=> 12;	# カテゴリ
use constant CATEGORY_OTHER					=> 13;	# カテゴリ


#----------------------------------------------------------------------------------
# ダメージタイプ名（順番はポケモンプログラム内の定数と一致している必要があります）
# 参照：waza_tool/wazadata.h
#----------------------------------------------------------------------------------
@DamageTypeName = (
	"なし",
	"物理",
	"特殊",
);
#----------------------------------------------------------------------------------
# 状態異常名（順番はポケモンプログラム内の定数と一致している必要があります）
# 参照：waza_tool/wazadata.h
#----------------------------------------------------------------------------------
@SickName = (
	"なし",
	"まひ",
	"ねむり",
	"こおり",
	"やけど",
	"どく",
	"こんらん",
	"メロメロ",
	"バインド",
	"あくむ",
	"のろい",
	"ちょうはつ",
	"いちゃもん",
	"かなしばり",
	"あくび",
	"かいふくふうじ",
	"いえき",
	"みやぶる",
	"やどりぎ",
	"さしおさえ",
	"ほろびのうた",
	"ねをはる",
	"とおせんぼう",
	"アンコール",
	"特殊処理",
);
use constant SICKID_SPECIAL			=> 0xffff;	# 特殊処理コード

#----------------------------------------------------------------------------------
# 状態異常の継続パターン名（順番はポケモンプログラム内の定数と一致している必要があります）
# 参照：waza_tool/wazadata.h
#----------------------------------------------------------------------------------
@SickContName = (
	"なし",
	"永続型",
	"ターン数型",
	"使用ポケモン依存型",
	"ターン数＆使用ポケモン依存型",
);
#----------------------------------------------------------------------------------
# ランク効果名（順番はポケモンプログラム内の定数と一致している必要があります）
# 参照：waza_tool/wazadata.h
#----------------------------------------------------------------------------------
@RankEffectName = (
	"なし",
	"こうげき",
	"ぼうぎょ",
	"とくこう",
	"とくぼう",
	"すばやさ",
	"めいちゅうりつ",
	"かいひりつ",
	"特殊処理",
);
#----------------------------------------------------------------------------------
# ワザ効果範囲名（順番はポケモンプログラム内の定数と一致している必要があります）
# 参照：waza_tool/wazadata.h
#----------------------------------------------------------------------------------
@TargetName = (
	"[選択]　通常ポケ",
	"[選択]　味方ポケ",
	"[選択]　自分以外の味方ポケ",
	"[選択]　相手ポケ",
	"自分以外の全ポケ",
	"相手全ポケ",
	"味方全ポケ",
	"自分のみ",
	"全ポケ",
	"相手ポケランダム",
	"全体場",
	"相手場",
	"味方場",
	"特殊",
);

#===============================================================================================
# Globals     
#===============================================================================================
@RecName = ();
@Record = ();



#===============================================================
# main
# 異常終了時、システムに1を返す。通常時は0を返す
#===============================================================
my $err = &main;
exit($err);

sub main {

	$argcnt = @ARGV;

	if( @ARGV != 1 )
	{
		&usage;
		return 1;
	}

	# %RecOrderの指定順にRecName配列を生成
	foreach(sort {$RecOrder{$a} <=> $RecOrder{$b}} keys %RecOrder){
		push @RecName, $_;
	}

#	my $p=0;
#	foreach (@RecName) {
#		_print("$p = $_\n");
#		$p++;
#	}

	if( !outputBin( 0, make_outfile_name(0) ))	#ダミーデータ出力
	{
		_print ("エラー：ダミーデータファイル出力できません\n");
	}

	if( open(IN_FILE, "<$ARGV[0]") )
	{
		my @inFile = <IN_FILE>;
		close( IN_FILE );
		my $err_msg = "";
		my $rec_num = 1;
		foreach $line (@inFile) {
#			$line =~ s/[\r]$//g;
			my @elems = split_record( $line );
			$err_msg .= storeRecord( \@elems, $rec_num );
			my $outfile = make_outfile_name( $rec_num );
			if( !outputBin( $rec_num, $outfile ) ){
				_print("エラー：$outfile が出力できません\n");
				return 1;
			}
			$rec_num++;
		}
		if( $err_msg ne "" ){
			_print ( $err_msg );
			return 1;
		}

	}else{
		_print ("エラー：$ARGV[0]が開けません\n");
		return 1;
	}

	return 0;
}
#===============================================================
# usage
#===============================================================
sub usage {
	my ($pack,$filename,$line) = caller;

	_print (">perl $filename <input_file>\n");
	_print ("<input_file       ファイルメーカー書類からエクスポートしたtab区切りファイル（UTF-8）\n");
}

#===============================================================
# タブ区切りテキスト１行を要素ごとに分割、配列を返す
#===============================================================
sub split_record {
	my $tab_line = shift;

	return split( /\t/, $tab_line );

}
#===============================================================
# utf8文字列を Shift-Jis化して出力
#===============================================================
sub _print {
	my $str = shift;
	$str = enc_sjis( $str );
	print $str;
}
#======================================================
# 文字列のutf8フラグを落とし、sjisコンバートする（ヘッダ出力用）
# input : 0:文字列
# return 変換後の文字列
#======================================================
sub enc_sjis {
	my $txt = shift;

	if(utf8::is_utf8($txt))
	{
		utf8::encode($txt);
	}
	
	Encode::from_to($txt, 'utf8', 'shiftjis');
	return $txt;
}
#======================================================
# レコード１件の配列をエラーチェック
# input[0] : レコード配列（リファレンス）
# input[1] : レコードナンバー（1～）
# return エラーがあればエラーメッセージ/なければ空文字列
#======================================================
sub storeRecord {
	my $refElems = shift;
	my $recID = shift;
	my $result = "";
	my $errorHeader;
	my $i;


	$errorHeader = "エラー：レコード[$recID]　";

	my $numElems = @$refElems;
	if( $numElems != IDX_FLG_End){
		my $honrai =IDX_FLG_End; 
		return $errorHeader . "データ量が不正です\n";
	}

	$Record[ IDX_Type ] = checkMatch( \@TypeName, $$refElems[IDX_Type] );
	if( $Record[ IDX_Type ] < 0 ){
		$result .= ($errorHeader . "タイプが指定されていません\n");
	}

	$Record[ IDX_Category ] = checkMatch( \@CategoryName, $$refElems[IDX_Category] );
	if( $Record[ IDX_Category ] < 0 ){
		$result .= ($errorHeader . "カテゴリが指定されていません\n");
	}

	$Record[ IDX_DamageType ] = checkMatch( \@DamageTypeName, $$refElems[IDX_DamageType] );
	if( $Record[ IDX_DamageType ] < 0 ){
		$Record[ IDX_DamageType ] = 0;
	}


	$Record[ IDX_Power ] = $$refElems[IDX_Power];
	if( $Record[ IDX_Power ] eq "" ){
		$Record[ IDX_Power ] = 0;
	}

	if( ($Record[ IDX_Power ] == 0) && ($Record[ IDX_DamageType ] != 0) ){
		$result .= ($errorHeader . "攻撃ワザに威力が設定されていません\n");
	}

	$Record[ IDX_HitPer ] = $$refElems[IDX_HitPer];
	if( $Record[ IDX_HitPer ] eq "" ){
		$result .= ($errorHeader . "命中率が設定されていません\n");
	}

	$Record[ IDX_BasePP ] = $$refElems[IDX_BasePP];
	if( $Record[ IDX_BasePP ] eq "" ){
		$result .= ($errorHeader . "基本PP値が設定されていません\n");
	}

	$Record[ IDX_Priority ] = $$refElems[IDX_Priority];
	if( $Record[ IDX_Priority ] eq "" ){
		$Record[ IDX_Priority ] = 0;
	}

	$Record[ IDX_CriticalRank ] = $$refElems[IDX_CriticalRank];
	if( $Record[ IDX_CriticalRank ] eq "" ){
		$Record[ IDX_CriticalRank ] = 0;
	}

	$Record[ IDX_HitCntMax ] = $$refElems[IDX_HitCntMax];
	if( $Record[ IDX_HitCntMax ] eq "" ){
		$Record[ IDX_HitCntMax ] = 0;
	}
	$Record[ IDX_HitCntMin ] = $$refElems[IDX_HitCntMin];
	if( $Record[ IDX_HitCntMin ] eq "" ){
		$Record[ IDX_HitCntMin ] = 0;
	}
	if( $Record[ IDX_HitCntMin ] > $Record[ IDX_HitCntMax ] ){
		$result .= ($errorHeader . "ヒット回数の最小値が最大値を超えています\n");
	}

	# ひるみ確率
	$Record[ IDX_ShrinkPer ] = $$refElems[IDX_ShrinkPer];

	# 状態異常効果
	$Record[ IDX_SickID ] = checkMatch( \@SickName, $$refElems[IDX_SickID] );
	if( $Record[ IDX_SickID ] < 0 ){
		$Record[ IDX_SickID ] = 0;
	}
	if( $Record[ IDX_SickID ] == (@SickName - 1) ){
		_print( "rec_$recID: 特殊処理の状態異常です\n");
		$Record[ IDX_SickID ] = SICKID_SPECIAL;
	}
	if( $Record[ IDX_SickID ] == 0 )
	{
		if( ($Record[ IDX_Category ] == CATEGORY_SIMPLE_SICK)
		||	($Record[ IDX_Category ] == CATEGORY_DAMAGE_SICK)
		||	($Record[ IDX_Category ] == CATEGORY_EFFECT_SICK)
		){
			$result .= ($errorHeader . "このカテゴリ ($Record[ IDX_Category ]) には状態異常効果の指定が必要です\n");
		}
	}

	# 状態異常の発生率
	$Record[ IDX_SickPer ] = $$refElems[IDX_SickPer];
	if( $Record[ IDX_SickPer ] == 0 )
	{
		if( ($Record[ IDX_Category ] == CATEGORY_DAMAGE_SICK)
		){
			$result .= ($errorHeader . "このカテゴリ ($Record[ IDX_Category ]) には状態異常の発生率の指定が必要です\n");
		}
	}
	if( $Record[ IDX_SickPer ] < 0 ){
		$Record[ IDX_SickPer ] = 0;
	}

	# 状態異常の継続パターン
	$Record[ IDX_SickCont ] = checkMatch( \@SickContName, $$refElems[IDX_SickCont] );
	if( $Record[ IDX_SickCont ] < 0 ){
		$Record[ IDX_SickCont ] = 0;
	}
	if( $Record[ IDX_SickCont ] == 0 )
	{
		if( $Record[ IDX_SickID ] != 0 ){
			$result .= ($errorHeader . "状態異常を指定する場合、継続パターンも指定してください\n");
		}
	}
	# 状態異常の継続ターン
	$Record[ IDX_SickTurnMax ] = $$refElems[ IDX_SickTurnMax ];
	$Record[ IDX_SickTurnMin ] = $$refElems[ IDX_SickTurnMin ];

	# ステータス効果１
	$Record[ IDX_RankEffType1 ] = checkMatch( \@RankEffectName, $$refElems[IDX_RankEffType1] );
	if( $Record[ IDX_RankEffType1 ] < 0 ){
		$Record[ IDX_RankEffType1 ] = 0;
	}
	if( $Record[ IDX_RankEffType1 ] == 0 )
	{
		if( ($Record[ IDX_Category ] == CATEGORY_SIMPLE_EFFECT)
		||	($Record[ IDX_Category ] == CATEGORY_DAMAGE_EFFECT)
		||	($Record[ IDX_Category ] == CATEGORY_DAMAGE_EFFECT_USER)
		||	($Record[ IDX_Category ] == CATEGORY_EFFECT_SICK)
		){
			$result .= ($errorHeader . "このカテゴリ($Record[ IDX_Category ])にはステータス効果１の指定が必要です\n");
		}

	}
	# ステータス効果１の変動値
	$Record[ IDX_RankEffValue1 ] = $$refElems[IDX_RankEffValue1];
	if( $Record[ IDX_RankEffValue1 ] == 0 ){
		if( $Record[ IDX_RankEffType1 ] != 0 ){
			$result .= ($errorHeader . "ステータス効果１の変動値が指定されていません\n");
		}
	}
	# ステータス効果１の発生率
	$Record[ IDX_RankEffPer1 ] = $$refElems[IDX_RankEffPer1];
	if( $Record[ IDX_RankEffPer1 ] == 0 )
	{
		if( ($Record[ IDX_Category ] == CATEGORY_DAMAGE_EFFECT)
		||	($Record[ IDX_Category ] == CATEGORY_DAMAGE_EFFECT_USER)
		){
			$result .= ($errorHeader . "このカテゴリ($Record[ IDX_Category ])にはステータス効果１の変動率の指定が必要です\n");
		}
	}


	# ステータス効果２
	$Record[ IDX_RankEffType2 ] = checkMatch( \@RankEffectName, $$refElems[IDX_RankEffType2] );
	if( $Record[ IDX_RankEffType2 ] < 0 ){
		$Record[ IDX_RankEffType2 ] = 0;
	}
	# ステータス効果２の変動値＆発生率
	$Record[ IDX_RankEffValue2 ] = $$refElems[IDX_RankEffValue2];
	$Record[ IDX_RankEffPer2 ] = $$refElems[IDX_RankEffPer2];

	# ステータス効果３
	$Record[ IDX_RankEffType3 ] = checkMatch( \@RankEffectName, $$refElems[IDX_RankEffType3] );
	if( $Record[ IDX_RankEffType3 ] < 0 ){
		$Record[ IDX_RankEffType3 ] = 0;
	}
	# ステータス効果３の変動値＆発生率
	$Record[ IDX_RankEffValue3 ] = $$refElems[IDX_RankEffValue3];
	$Record[ IDX_RankEffPer3 ] = $$refElems[IDX_RankEffPer3];

	# ＨＰ回復率
	$Record[ IDX_HPRecoverRatio ] = $$refElems[IDX_HPRecoverRatio];
	if( $Record[ IDX_HPRecoverRatio ] == 0 )
	{
		if( $Record[ IDX_Category ] == CATEGORY_SIMPLE_RECOVER )
		{
			$result .= ($errorHeader . "このカテゴリにはＨＰ回復率の指定が必要です\n");
		}
	}
	# ダメージ回復率
	$Record[ IDX_DmgRecoverRatio ] = $$refElems[IDX_DmgRecoverRatio];
	if( $Record[ IDX_DmgRecoverRatio ] == 0 )
	{
		if( $Record[ IDX_Category ] == CATEGORY_DAMAGE_DRAIN )
		{
			$result .= ($errorHeader . "このカテゴリにはダメージ回復率の指定が必要です $Record[ IDX_HPRecoverRatio ]\n");
		}
	}

	# 効果範囲
	$Record[ IDX_Target ] = checkMatch( \@TargetName, $$refElems[IDX_Target] );
	if( $Record[ IDX_Target ] < 0 ){
		$result .= ($errorHeader . "正しい効果範囲が指定されていません\n");
	}

	# AI用シーケンスナンバー
	{
		my $str_num = substr( $$refElems[IDX_SeqNo], 0, 3 );
		if( $str_num =~ /[0-9][0-9][0-9].*/ )
		{
			$str_num =~ s/^0+//g;
			$Record[ IDX_SeqNo ] = $str_num;
		}
		else
		{
			$result .= ($errorHeader . "シーケンスナンバー不正" . $str_num . "\n");
		}
	}


	# 各種フラグ
	$i = IDX_FLG_Touch;
	while( $i < IDX_FLG_End ){

		$Record[ $i ] = $$refElems[ $i ];
		$i++;
	}

	return $result;
}
#======================================================
# 配列内から文字列完全一致検索
# input[0] : 配列リファレンス
# input[1] : 文字列
# return 配列内に指定文字列と完全に一致するデータがあれば、そのindex／なければ-1
#======================================================
sub checkMatch {
	my $refAry = shift;
	my $str = shift;
	my $nElems = @$refAry;
	my $i;
	for($i=0; $i<$nElems; $i++)
	{
		if( $$refAry[$i] eq $str ){
			return $i;
		}
	}
	return -1;
}
#===============================================================
# $Record配列の内容をバイナリファイルとして出力
#===============================================================
sub outputBin {
	my $recID = shift;
	my $fileName = shift;
	if( open(OUT_FILE, ">$fileName") )
	{
		binmode OUT_FILE;
		my $i;
		my $buf;
		my $write_size = 0;
		my $idx_start = IDX_Type;
		for($i = $idx_start; $i<IDX_FLG_Touch; $i++)
		{
			# 最大ヒット・最小ヒットはまとめて1byte
			if( ($i == IDX_HitCntMax) ){
				if( $Record[$i] >= 16 ){
					_print("エラーA：レコード[$recID]  不正な値 ($RecName[$i] = $Record[$i]）\n");
					return 0;
				}
				$buf = $Record[$i];
			}
			elsif( ($i == IDX_HitCntMin) ){
				if( $Record[$i] >= 16 ){
					_print("エラーB：レコード[$recID]  不正な値 ($RecName[$i] = $Record[$i]）\n");
					return 0;
				}
				$buf = ($buf << 4) | $Record[$i];
				$buf = pack('c', $buf );
				syswrite(OUT_FILE, $buf, 1);
				$write_size += 1;
			}
			elsif( ($i == IDX_SickID) ){
				if( $Record[$i] > 0xffff ){
					_print("エラーC：レコード[$recID]  不正な値 ($RecName[$i] = $Record[$i]）\n");
					return 0;
				}
				$buf = pack('S', $Record[$i] );
				syswrite(OUT_FILE, $buf, 2);
				$write_size += 2;
			}elsif( ($i == IDX_Priority )
			||	($i == IDX_RankEffValue1)
			||	($i == IDX_RankEffValue2)
			||	($i == IDX_RankEffValue3)
			||	($i == IDX_DmgRecoverRatio)
			||	($i == IDX_HPRecoverRatio)
			){
				if( $Record[$i] < -128 || $Record[$i] > 127){
					_print("エラーD：レコード[$recID]  不正な値 ($RecName[$i] = $Record[$i]）\n");
					return 0;
				}
				$buf = pack('c', $Record[$i]);
				syswrite( OUT_FILE, $buf, 1);
				$write_size += 1;
			}
			elsif( $i == IDX_SeqNo )
			{
				if( $Record[$i] < 0 || $Record[$i] > 65535){
					_print("エラーD：レコード[$recID]  不正な値 ($RecName[$i] = $Record[$i]）\n");
					return 0;
				}
				$buf = pack('S', $Record[$i]);
				syswrite( OUT_FILE, $buf, 2 );
				$write_size += 2;
			}
			else{
				if( $Record[$i] < 0 || $Record[$i] > 255){
					_print("エラーE：レコード[$recID]  不正な値 ($RecName[$i] = $Record[$i]）\n");
					return 0;
				}
				$buf = pack('C', $Record[$i]);
				syswrite(OUT_FILE, $buf, 1);
				$write_size += 1;
			}
		}

		# 4byte アラインメント
		my $size = $write_size;
		while( $size % 4 ){
			$buf = pack('C', 83);
			syswrite( OUT_FILE, $buf, 1 );
			$size++;
		}


		# フラグ出力
		my $flag = 0;
		my $cnt = 0;
		my $bitPos = 0;
		for($i=IDX_FLG_Start; $i<IDX_FLG_End; $i++)
		{
			if( $Record[$i] != 0){
				$flag |= (1 << $bitPos);
			}
			$cnt++;
			if( $cnt >= 32 ){	# 32bitごとに書き出し
				$buf = pack('I', $flag);
				syswrite( OUT_FILE, $buf, 4 );
				$cnt = 0;
				$flag = 0;
			}
			$bitPos++;
		}
		if( $cnt )
		{
			$buf = pack('I', $flag);
			syswrite( OUT_FILE, $buf, 4 );
		}
		_print ("->$fileName  category=$Record[ IDX_Category ]\n");
		close( OUT_FILE );
		return 1;
	}
	return 0;
}

sub make_outfile_name {
	my $recID = shift;
	return sprintf ("bin\\waza%03d.bin", $recID);
}
