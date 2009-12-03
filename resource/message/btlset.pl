#!/usr/bin/perl

#===============================================================================
# btl_set メッセージコンバータ
#
# バトル用メッセージデータ btl_set_src. gmm を読み込み、
# 必要に応じて「あいての」「やせいの」などの文字列を付加して btl_set.gmm を生成するための
# コンバータです。
#
# GameFreak taya
#
#
#===============================================================================

use utf8;
use open ":encoding(utf8)";

binmode(STDERR,":encoding(shiftjis)");
binmode(STDOUT,":encoding(shiftjis)");

#===============================================================
# main
# 異常終了時、システムに1を返す。通常時は0を返す
#===============================================================
my $err = &main;
exit($err);

sub main {


	if( @ARGV < 1 )
	{
		print "usage:perl btlset.pl <src.gmm> <dst.gmm>\n";
		return 1;
	}

	if( open( OUTPUT, ">$ARGV[1]" ) )
	{
		if( open(FILE, $ARGV[0]) )
		{
			my @file = <FILE>;
			close(FILE);
			my $skip_f = 0;
			my $skip_cnt = 0;
			my $max = @file;
			my $line;
			my $skip_mode = "";
			my $i;
			my $cnt = 0;
			my $blockMode = 0;
			my @srcBlock;

			for($i=0; $i<$max; $i++)
			{
				$line = $file[$i];

				if( $skip_mode eq "" )
				{
					if( $line =~ /attribute japanese=\"コンバートタイプ\"/ ){
						$skip_mode = "attr";
						next;
					}

					if( $blockMode == 0 )
					{
						if( $line =~ /<row id=.*>[\r|\n]+$/ ){
							@srcBlock = ();
							push (@srcBlock, $line);
							$blockMode = 1;
						}
						else
						{
							print OUTPUT $line;
						}
					}
					else
					{
						if( $line =~/attribute name=\"コンバートタイプ\">/ ){
							if( $line =~/そのまま/ ){
								$blockMode = 2;
							}else{
								$blockMode = 3;
							}
							next;
						}
						push (@srcBlock, $line);
						if( $line =~ /<\/row>/ ){
							if( $blockMode == 1 )	# デフォルト変換
							{
								my $id = &get_id_str( $srcBlock[0] );
								my $bl_line;
								my $output_line;
								my $convMode = 0;
								my @subBlock = ();	
								my $commentMode = 0;
								foreach $bl_line (@srcBlock) {
									print OUTPUT $bl_line;
									if( $bl_line =~ /<comment>/ ){
										$commentMode = 1;
									}
									if( $commentMode == 0 ){
										push (@subBlock, $bl_line );
									}
									elsif( $bl_line =~ /<\/comment>/ ){
										$commentMode = 0;
									}
								}

								my $id_line = "\t<row id=\"" . $id . "_Y\">\n";
								$subBlock[0] = $id_line;
								$convMode = 0;
								foreach $bl_line (@subBlock) {
									$output_line = &conv_str( $bl_line, \$convMode, "やせいの　", "野生の　" );
									print OUTPUT $output_line;
								}

								$id_line = "\t<row id=\"" . $id . "_E\">\n";
								$subBlock[0] = $id_line;
								$convMode = 0;
								foreach $bl_line (@subBlock) {
									$output_line = &conv_str( $bl_line, \$convMode, "あいての　", "相手の　" );
									print OUTPUT $output_line;
								}
							}
							elsif( $blockMode == 2 )	# そのまま出力
							{
								my $bl_line;
								foreach $bl_line (@srcBlock) {
									print OUTPUT $bl_line;
								}
								@srcBlock = ();
							}
							elsif( $blockMode == 3 )	# ポケ名２体 -> 7パターン出力
							{
								my $id = &get_id_str( $srcBlock[0] );
								my $bl_line;
								my $output_line;
								my $convMode = 0;
								my $prefixCnt = 0;
								my @subBlock = ();	
								my $commentMode = 0;
								foreach $bl_line (@srcBlock) {
									print OUTPUT $bl_line;
									if( $bl_line =~ /<comment>/ ){
										$commentMode = 1;
									}
									if( $commentMode == 0 ){
										push (@subBlock, $bl_line );
									}
									elsif( $bl_line =~ /<\/comment>/ ){
										$commentMode = 0;
									}
								}

								my $id_line = "\t<row id=\"" . $id . "_2Y\">\n";
								$subBlock[0] = $id_line;
								$convMode = $prefixCnt = 0;
								foreach $bl_line (@subBlock) {
									$output_line = &conv_str_ex( $bl_line, \$convMode, \$prefixCnt, 1, "やせいの　", "野生の　" );
									print OUTPUT $output_line;
								}
								$convMode = @subBlock;
								print "---$convMode \n";

								$id_line = "\t<row id=\"" . $id . "_2E\">\n";
								$subBlock[0] = $id_line;
								$convMode = $prefixCnt = 0;
								foreach $bl_line (@subBlock) {
									$output_line = &conv_str_ex( $bl_line, \$convMode, \$prefixCnt, 1, "あいての　", "相手の　" );
									print OUTPUT $output_line;
								}
								$convMode = @subBlock;
								print "---$convMode \n";


								my $id_line = "\t<row id=\"" . $id . "_Y\">\n";
								$subBlock[0] = $id_line;
								$convMode = $prefixCnt = 0;
								foreach $bl_line (@subBlock) {
									$output_line = &conv_str_ex( $bl_line, \$convMode, \$prefixCnt, 0, "やせいの　", "野生の　" );
									print OUTPUT $output_line;
								}
								$convMode = @subBlock;
								print "---$convMode \n";


								my $id_line = "\t<row id=\"" . $id . "_Y2Y\">\n";
								$subBlock[0] = $id_line;
								$convMode = $prefixCnt = 0;
								foreach $bl_line (@subBlock) {
									$output_line = &conv_str_ex( $bl_line, \$convMode, \$prefixCnt, -1, "やせいの　", "野生の　" );
									print OUTPUT $output_line;
								}
								$convMode = @subBlock;
								print "---$convMode \n";


								$id_line = "\t<row id=\"" . $id . "_E\">\n";
								$subBlock[0] = $id_line;
								$convMode = $prefixCnt = 0;
								foreach $bl_line (@subBlock) {
									$output_line = &conv_str_ex( $bl_line, \$convMode, \$prefixCnt, 0, "あいての　", "相手の　" );
									print OUTPUT $output_line;
								}
								$convMode = @subBlock;
								print "---$convMode \n";


								$id_line = "\t<row id=\"" . $id . "_E2E\">\n";
								$subBlock[0] = $id_line;
								$convMode = $prefixCnt = 0;
								foreach $bl_line (@subBlock) {
									$output_line = &conv_str_ex( $bl_line, \$convMode, \$prefixCnt, -1, "あいての　", "相手の　" );
									print OUTPUT $output_line;
								}
								$convMode = @subBlock;
								print "---$convMode \n";


							}
							$blockMode = 0;
						}
					}
				}
				else
				{
					if( $skip_mode eq "attr" ){
						if( $line =~ /<\/attribute>/ ){
							$skip_mode = "";
						}
					}
				}
			}
			print "-> $ARGV[1]\n";
		}
		else{
			print "cant open $ARGV[0]";
		}
		close( OUTPUT );
	}
	else
	{
		print "cant create $ARGV[1]\n";
	}

	return 0;
}


sub get_id_str {
	my $line = shift;

	my @elems = split( /\"/, $line );
	return $elems[1];
}

sub conv_str {
	my $line = shift;
	my $refMode = shift;
	my $add1 = shift;
	my $add2 = shift;
	my $result = $line;

	if( $$refMode == 0 )	# かな・未返還
	{
		my $p = index( $result, "[1:02:" );
		if( $p >= 0 )
		{
			substr( $result, $p, 0 ) = $add1;
			$$refMode = 1;
		}
	}
	if( $$refMode == 1 )	# かな・変換済み
	{
		if( $result =~ /<\/language>/ ){
			$$refMode = 2;
			return $result;
		}
	}
	if( $$refMode == 2 )	# かな文終了 -> 漢字
	{
		my $p = index( $result, "[1:02:" );
		if( $p >= 0 )
		{
			substr( $result, $p, 0 ) = $add2;
			$$refMode = 3;
		}
	}


	return $result;
}


sub conv_str_ex {
	my $line = shift;
	my $refMode = shift;
	my $refPrefixCnt = shift;
	my $tagIdx = shift;
	my $add1 = shift;
	my $add2 = shift;
	my $result = $line;
	my $i;

	print "tagIdx=$tagIdx, mode=$$refMode, nidx=$$refPrefixCnt";

	if( $$refMode == 0 )	# かな・未返還
	{
		my $p = index( $line, "[1:02:" );
		if( $p >= 0 )
		{
			print " hit .. ";
			if( ($tagIdx < 0)
			||	($$refPrefixCnt == $tagIdx)
			){
				print "  match!";
				substr( $result, $p, 0 ) = $add1;
				if( $tagIdx >= 0 ){
					$$refMode = 1;
				}
			}
			$$refPrefixCnt++;
		}
		if( $tagIdx < 0 ){
			if( $result =~ /<\/language>/ ){
				print "\n";
				$$refMode = 1;
				$$refPrefixCnt = 0;
			}
		}
	}
	if( $$refMode == 1 )	# かな・変換済み
	{
		if( $result =~ /<\/language>/ ){
			print "\n";
			$$refMode = 2;
			$$refPrefixCnt = 0;
			return $result;
		}
	}
	elsif( $$refMode == 2 )	# かな文終了 -> 漢字
	{
		my $p = index( $line, "[1:02:" );
		if( $p >= 0 )
		{
			print " hit .. ";
			if( ($tagIdx < 0)
			||	($$refPrefixCnt == $tagIdx)
			){
				print "  match!";
				substr( $result, $p, 0 ) = $add2;
				if( $tagIdx > 0){
					$$refMode = 3;
				}
			}
			$$refPrefixCnt++;
		}
	}

	print "\n";

	return $result;
}
