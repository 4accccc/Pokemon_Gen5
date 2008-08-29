#===============================================================================
# メッセージコンバータ用 文字列ID管理パッケージ
# メインのスクリプトでrequireして使うだけ
# 
# GameFreak taya
#
#===============================================================================
package idman;

require 'tool.pl';
use File::Copy;
use FindBin;

my @IdList = ();
my @FileList = ();
my @Id_File_Idx = ();

my $ErrorMsg = "";

#===============================================================
# 新規IDを追加
# input : 0:ID（文字列）
#         1:グループ名
#         2:処理中ファイル名
#
#===============================================================
sub store {
	my ($id, $filename) = @_;
	my $err_msg;
	my $i;

# 同一IDがあれば死ぬ（別ファイルでも）


	for($i = 0; $i < @IdList; $i++)
	{
		if( $id eq $IdList[$i] )
		{
			my $dup_filename = $FileList[ $Id_File_Idx[$i] ];
			if( $dup_filename eq $filename )
			{
#				die("ID [$id] の重複（$filename）");
				$ErrorMsg .= "ID [$id] Duplicate in ($filename)\n";
			}
			else
			{
				$ErrorMsg .= "ID [$id] Duplicate in ($dup_filename, $filename)\n";
			}

#			die($err_msg);
		}
	}

# ID, ファイル名, グループ名を登録しておく

	push @IdList, $id;

	my $idx = &tool::store_unique_data( $filename, \@FileList );
	push @Id_File_Idx, $idx;

#	print "PUSH UNIQ FILENAME .. $filename = $idx\n";
}
#===============================================================
# store 処理でエラーがあった場合、その詳細を出力して die 
#===============================================================
sub check_error {
	if( $ErrorMsg ne "" )
	{
		die($ErrorMsg);
	}
}
#===============================================================
# ヘッダファイル出力
# input : 0:処理中ファイル名（拡張子を .h に変えたものがヘッダファイル名になる）
#         1:出力先ディレクトリパス
#
# return : エラーがあればdie
#===============================================================
sub output_header {
	my ($file, $path) = @_;
	my $output_file;
	my $max_symbol;
	my $idx;
	my $cnt;

	$file_idx = &tool::get_unique_idx($file, \@FileList);

#	print "POP UNIQ FILENAME .. $file = $file_idx\n";
	if($file_idx < 0)
	{
		die("予期せぬエラー");
		return;
	}

	$output_file = &tool::change_ext($file, ".h");
	$output_file = &tool::get_filename_from_path($output_file);
	$output_file = "msg_" . $output_file;
	$max_symbol = &tool::change_ext( $output_file, "" ) . "_max";
	$output_file = $path . $output_file;

	my $tmp_file = "./tmp.h";

	if( open(HEADER_FILE, ">$tmp_file") )
	{
		my $outputFileName = &tool::get_filename_from_path($output_file);
		print HEADER_FILE &tool::enc_sjis("//==============================================================================\n");
		print HEADER_FILE &tool::enc_sjis("/**\n");
		print HEADER_FILE &tool::enc_sjis(" * \@file		$outputFileName\n");
		print HEADER_FILE &tool::enc_sjis(" * \@brief		メッセージID参照用ヘッダファイル\n");
		print HEADER_FILE &tool::enc_sjis(" *\n");
		print HEADER_FILE &tool::enc_sjis(" * このファイルはコンバータにより自動生成されています\n");
		print HEADER_FILE &tool::enc_sjis(" */\n");
		print HEADER_FILE &tool::enc_sjis("//==============================================================================\n");
		print HEADER_FILE &make_header_tag($output_file);

	# 単一グループしか無い場合の出力
		{
			$cnt = 0;
			for(my $i = 0; $i < @Id_File_Idx; $i++)
			{
				if($Id_File_Idx[$i] == $file_idx)
				{
					my $txt = &tool::enc_sjis("#define\t$IdList[$i]\t\t($cnt)\n");
					print HEADER_FILE "$txt";
					$cnt++;
				}
			}
			print HEADER_FILE "\n";
			print HEADER_FILE "#define\t$max_symbol\t\t($cnt)\n";

		}

		print HEADER_FILE &tool::enc_sjis("\n#endif\n");

		close(HEADER_FILE);
		check_update_copy( $tmp_file, $output_file );
		unlink( $tmp_file );
#		print &tool::enc_sjis("-> $output_file\n");
	}
	else
	{
		die("$tmp_file が作成できません");
	}
}

#===============================================================
# ２つのテキストファイルの中身を比較して、差分があればコピーする
# input	0	コピー元ファイルパス
#		1	コピー先ファイルパス
# return 0:コピーしなかった/1:コピーした1
#===============================================================
sub check_update_copy {
	my $srcpath = shift;
	my $dstpath = shift;

	my @src;
	my @dst;

	if( open(DST, "<$dstpath") )
	{
		@dst = <DST>;
		close(DST);
		if( open(SRC, "<$srcpath") )
		{
			@src = <SRC>;
			close( SRC );
		}
		else
		{
			die("$srcpathが開けない\n");
			return 0;
		}
		if( @src == @dst )
		{
			my $i;
			for($i=0; $i<@src; $i++)
			{
				if( $src[$i] ne $dst[$i] ){ last; }
			}
			if( $i == @src )
			{
				return 0;
			}
		}
	}

	copy( $srcpath, $dstpath );
	unlink( $srcpath );

	print "-> $dstpath updated.\n";

	return 1;

}

#===============================================================
# 最新（現在処理中）のID文字列を返す
# return : ID文字列
#===============================================================
sub get_latest_id {
	my $idx = @IdList - 1;
	if($idx < 0)
	{
		return "";
	}
	return $IdList[$idx];
}
#===============================================================
# 最新（現在処理中）のグループ文字列を返す
# return : グループ文字列
#===============================================================
sub get_latest_group {
	return 0;
}

#------------------------------------------------------------------------------------------
# 以下のサブルーチンは外部からの呼び出しを想定していない
#------------------------------------------------------------------------------------------

#======================================================
# ヘッダーの多重インクルード防止タグ（ #ifndef ～ ）を作る
# input : 0:ヘッダファイル名
# return : 防止タグ文字列
#======================================================
sub make_header_tag {
	my $header_name = shift;

	my @list = split(/[\/\\]/, $header_name);
	my $c = @list;
	$header_name = $list[$cnt-1];

	$header_name = uc ($header_name);

	my @p = split(/[.\/\\]/, $header_name);
	my $sym = '__';

	foreach (@p) {
		$sym .= $_;
		$sym .= "_";
	}
	$sym .= "_";

	my $ret = "#ifndef $sym\n#define $sym\n\n";

	return $ret;
}


return 1;
