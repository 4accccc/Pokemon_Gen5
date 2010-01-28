#!/usr/bin/perl

#===============================================================================
# メッセージコンバータ
#
# メッセージエディタのデータファイル（gmm）を読み込んで、各国語版の
# メッセージリソースを出力するためのPerlスクリプトです。
#
# Perlのヴァージョンは、XML::Parser モジュールを使用可能なものにする必要があります。
#（当スクリプト自体は ActivePerl v5.8.6 動作環境で作成しました）
#
# GameFreak taya
#
#
# 2008.8.27
#	１つのメッセージデータに複数言語タイプを持たせることが出来る構造に対応させました。
#
#===============================================================================

use XML::Parser;

use open OUT => ":encoding(shiftjis)";

binmode(STDERR,":encoding(shiftjis)");
binmode(STDOUT,":encoding(shiftjis)");

require 'resource.pl';
require 'idman.pl';

#===================================================
# Params
#===================================================
my %LangParamTbl = (		# 言語コード指定パラメータ

	'LANG_JAPAN',	'日本語',		# スクリプト実行時のパラメータとして'jp'を指定すると、
									# エディタ「日本語」列に書かれたメッセージをコンバート対象とします。
									# 必要に応じてこのテーブルをカスタマイズしてください。
									# エディタ側にも指定言語の列が無ければ正しく動作しないことに注意してください。

	'LANG_ENGLISH',	'英語',
	'LANG_ITALY',	'イタリア語',
	'LANG_FRANCE',	'フランス語',
	'LANG_SPAIN',	'スペイン語',
	'LANG_GERMANY',	'ドイツ語',


);

my $HEADER_OUTPUT_DIR = '../include/text/';
my $HEADER_EXT = '.h';
my $RESOURCE_OUTPUT_DIR = './dst/';
my $RESOURCE_EXT = '.dat';

my $TIMECHECK_DATA_PATH = '../../src/msgdata/msg.narc';	# このファイルと比較して新しいgmmのみをコンバートする

#===================================================
# Constants
#===================================================
use constant SEQ_FREE		=> 0;	# シーケンス：メッセージ読み込み処理外
use constant SEQ_READY		=> 1;	# シーケンス：'row'タグ読み込み開始
use constant SEQ_LANG_START	=> 2;	# シーケンス：'row-language'タグ読み込み途中
use constant SEQ_LANG		=> 3;	# シーケンス：'row-language'タグ読み込み途中
use constant SEQ_FINISH		=> 4;	# シーケンス：指定言語メッセージの処理終了
use constant SEQ_DEFAULT_LANG_START => 5;
use constant SEQ_DEFAULT_LANG_READING => 6;


#===================================================
# Globals
#===================================================
my $ParsingLang = "";		# パース対象の言語（ JPN, JPN_KANJI 等 ）
my $ParsingLangID = 0;		# 日本語（デフォルト）なら0, 日本語（漢字）なら1などのように使う。
my $NumParsedLangCount = 0;	# コンバートした言語数
my $ParsingFileName = "";	# 解析中のファイル名
my $DefaultGroupName = "デフォルト";	# デフォルトのグループ名（読み込んだソースによって可変）
my $ReadingText = "";		# 読み込み中のテキスト内容
my $AreaWidth = 0;			# 読み込み中テキストの表示最低領域幅（ドット）


my @TargetLangs = ();
my $OrgLangFlag = 0;	# 開発オリジナル言語（つまり日本語）かどうかの判定フラグ

use constant ORG_LANG_CODE => "JPN";	# 開発オリジナル言語指定

#===============================================================
# main
# 異常終了時、システムに1を返す。通常時は0を返す
#===============================================================
my $err = &main;
exit($err);

sub main {

	srand(time ^ ($$ + ($$ << 15)));

	if( @ARGV < 2 )
	{
		&usage;
		return 1;
	}

	@TargetLangs = @ARGV[2..(@ARGV-1)];
	$OrgLangFlag = ($TargetLangs[0] == ORG_LANG_CODE);

	my $parser = new XML::Parser;

	$parser->setHandlers(	Start => \&StartHandler,
							End => \&EndHandler,
							Char => \&CharHandler
						);


	if(&conv($ARGV[0], $ARGV[1], \$parser))
	{
		return 1;
	}

	return 0;
}
#===============================================================
# リスト化されていない古いdatファイルを削除する
# input 1: ソースファイルリストへのリファレンス
#       2: datファイルが存在するディレクトリパス
#===============================================================
sub delete_old_datfile {
	my $src_ref = shift;
	my $dat_path = shift;
	my %src_list;
	my $tmp;

	foreach (@$src_ref) {
		$tmp = $_;
		if( $tmp =~ /\\/ )
		{
			my @tmpary = split(/\\/, $tmp);
			$tmp = $tmpary[@tmpary-1];
		}
		$tmp =~ s/\.gmm$/\.dat/;
		$src_list{$tmp} = 1;
	}

	if( opendir(DIR, $dat_path) )
	{
		my @datlist = readdir(DIR);
		my $datfile;
		foreach $datfile (@datlist) {
			unless( $datfile =~ /\.dat$/ )
			{
				next;
			}
			if( $src_list{$datfile} == 1 )
			{
				next;
			}
			unlink($dat_path.$datfile);
		}
		closedir(DIR);
	}

}
#===============================================================
# usage
#===============================================================
sub usage {
	my ($pack,$filename,$line) = caller;

	print ">perl $filename <gmm> <def_lang> [+sub_lang ...]\n";
	print "gmm       メッセージエディタ用データファイル（gmmファイル）\n";
	print "def_lang  コンバート対象となるデフォルト言語（必須）\n";
	print "sub_lang  同一ファイルに含めるサブ言語（任意＆複数指定可）\n";

	my @key = keys(%LangParamTbl);
	my @val = values(%LangParamTbl);

	for(my $i = 0; $i < @key; $i++)
	{
		print "       $key[$i] -> $val[$i]\n";
	}
}
#===============================================================
# コンバートメイン
# input : 0: コンバート対象ファイル名
# 異常終了で1,正常終了で0を返す
#===============================================================
sub conv {
	my $filename = shift;
	my $dst_dir = shift;
	my $parser = shift;
	my $xml;

	if( open(FILE, $filename) )
	{
		my @stat = stat(FILE);
		read(FILE, $xml, $stat[7]);
		close(FILE);
	}
	else
	{
		print STDERR "エラー：$filename が開けません\n";
		return 1;
	}

	&ResetSeq();

	$ParsingFileName = $filename;
	&resource::init();
	$ParsingLangID = 0;
	$NumParsedLangCount = 1;

	eval {$$parser->parse($xml)};
	if($@){
		print STDERR "エラー: $@\n";
		return 1;
	}

	&idman::check_error();

	&idman::output_header($filename, $dst_dir );
	&resource::output($filename, $dst_dir, $NumParsedLangCount);

	return 0;

}
#===============================================================
# 指定言語コードがコンバート対象かどうか判定
# input 0  言語コード（JPN等）
# return   対象なら1, 対象外なら0
#===============================================================
sub is_target_lang {
	my $lang = shift;

	foreach( @TargetLangs ) {
		if( $_ eq $lang ){ return 1; }
	}

	return 0;
}
#===============================================================
# XML開始タグ読み込み時のハンドラ
#===============================================================
sub StartHandler {
	my $expat = shift;
	my $tag = shift;
	my %elems = ();

	my $seq = &GetSeq;


	for(my $i = 0; $i < @_; $i += 2)
	{
		$elems{$_[$i]} = $_[$i+1];
	}

	if($seq == SEQ_FREE)
	{
		if($tag eq "row")
		{
			if($elems{erase} eq "")
			{
#				print "-store -> $elems{id}\n";
				idman::store($elems{id}, $ParsingFileName);
				$ParsingLangID = 0;
				&SetSeq(SEQ_READY);
			}
		}
	}
	elsif($seq == SEQ_READY)
	{
		if($tag eq "language")
		{
			if( &is_target_lang($elems{name}) )
			{
				$ReadingText = "";
				$AreaWidth = $elems{width};
				&FwdSeq;
			}
		}
	}


=pod
	map {print $_,"//"}@{$expat->{Context}} , "\n";
	print "++(";
	map {print "$_"," "}@_;
	print ")\n";
=cut
}
#===============================================================
# XML終了タグ読み込み時のハンドラ
#===============================================================
sub EndHandler
{
	my $expat = shift;
	my $tag = shift;

	my $seq = &GetSeq();

	if($seq == SEQ_LANG || $seq == SEQ_LANG_START)
	{
		if($tag eq "language")
		{
#			print "area_width=$AreaWidth\n";
			if(&resource::add_msg($ReadingText, $ParsingLangID, $OrgLangFlag, $AreaWidth) == 0)
			{
				my $id = &idman::get_latest_id();
#				print "illegal char code：ID[ $id ] in $ParsingFileName\n";
				print &tool::enc_sjis("ERR! Illegal String : ID[ $id ] @ $ParsingFileName\n");
				die;
			}

			$ParsingLangID++;
			&SetSeq(SEQ_READY);
		}
	}


	if($tag eq "row")
	{
		if($seq == SEQ_READY)
		{
			$NumParsedLangCount = $ParsingLangID;
			&ResetSeq();
		}
	}

}
#===============================================================
# XMLタグ内文字列読み込み時のハンドラ
#===============================================================
sub CharHandler
{
	my $expat = shift;
	my $text = shift;

	my $seq = &GetSeq();


	if($seq == SEQ_LANG_START)
	{
		$ReadingText = $text;
		&FwdSeq();
	}
	elsif($seq == SEQ_LANG)
	{
		$ReadingText .= $text;
	}
}


#===============================================================
# シーケンス処理
#===============================================================
my $Sequence = SEQ_FREE;

sub ResetSeq {
	$Sequence = SEQ_FREE;
}

sub GetSeq {
	return $Sequence;
}

sub SetSeq {
	$Sequence = shift;
}

sub FwdSeq {
	$Sequence++;
	if($Sequence > SEQ_FINISH)
	{
		$Sequence = SEQ_FREE;
	}
}








