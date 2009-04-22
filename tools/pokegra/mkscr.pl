
#====================================================================================
#
#	narc用のscrファイルを生成する
#	実際はパーソナルデータから生成されるものであるが、
#	そちらから生成できるようになるまでの暫定処置
#
#====================================================================================

#====================================================================================
#
#	処理開始
#
#====================================================================================
use File::Basename;

	open( LIST, @ARGV[0] ) or die "[@ARGV[0]]", $!;
	@list = <LIST>;
	close( LIST );

	open( SCR, "> pokegra_wb.scr");

	$list_count = @list;
	$no = 0;

	while( $list_count != 0 ){
		$file_name = @list[ $no ];
		$file_name =~ s/\x0d\x0a|\x0d|\x0a//g;

		$ncgr_m_name_f = $file_name;
		$ncgr_m_name_f =~ s/.NMCR/_m.NCGR/g;

		$ncgr_f_name_f = $ncgr_m_name_f;
		$ncgr_f_name_f =~ s/_m/_f/g;

		$ncbr_m_name_f = $ncgr_m_name_f;
		$ncbr_m_name_f =~ s/_m.NCGR/c_m.NCBR/g;

		$ncbr_f_name_f = $ncbr_m_name_f;
		$ncbr_f_name_f =~ s/_m/_f/g;

		$ncer_name_f = $file_name;
		$ncer_name_f =~ s/.NMCR/.NCER/g;

		$nanr_name_f = $file_name;
		$nanr_name_f =~ s/.NMCR/.NANR/g;

		$nmcr_name_f = $file_name;

		$nmar_name_f = $file_name;
		$nmar_name_f =~ s/.NMCR/.NMAR/g;

		$ncec_name_f = $file_name;
		$ncec_name_f =~ s/.NMCR/.NCEC/g;

		$ncgr_m_name_b = $ncgr_m_name_f;
		$ncgr_m_name_b =~ s/pfwb/pbwb/g;

		$ncgr_f_name_b = $ncgr_f_name_f;
		$ncgr_f_name_b =~ s/pfwb/pbwb/g;

		$ncbr_m_name_b = $ncbr_m_name_f;
		$ncbr_m_name_b =~ s/pfwb/pbwb/g;

		$ncbr_f_name_b = $ncbr_f_name_f;
		$ncbr_f_name_b =~ s/pfwb/pbwb/g;

		$ncer_name_b = $ncer_name_f;
		$ncer_name_b =~ s/pfwb/pbwb/g;

		$nanr_name_b = $nanr_name_f;
		$nanr_name_b =~ s/pfwb/pbwb/g;

		$nmcr_name_b = $nmcr_name_f;
		$nmcr_name_b =~ s/pfwb/pbwb/g;

		$nmar_name_b = $nmar_name_f;
		$nmar_name_b =~ s/pfwb/pbwb/g;

		$ncec_name_b = $ncec_name_f;
		$ncec_name_b =~ s/pfwb/pbwb/g;

		$nclr_name_n = $file_name;
		$nclr_name_n =~ s/pfwb/pmwb/g;
		$nclr_name_n =~ s/.NMCR/_n.NCLR/g;

		$nclr_name_r = $nclr_name_n;
		$nclr_name_r =~ s/_n.NCLR/_r.NCLR/g;

		print SCR "\"$ncgr_m_name_f\"\n";
		print SCR "\"$ncgr_f_name_f\"\n";
		print SCR "\"$ncbr_m_name_f\"\n";
		print SCR "\"$ncbr_f_name_f\"\n";
		print SCR "\"$ncer_name_f\"\n";
		print SCR "\"$nanr_name_f\"\n";
		print SCR "\"$nmcr_name_f\"\n";
		print SCR "\"$nmar_name_f\"\n";
		print SCR "\"$ncec_name_f\"\n";
		print SCR "\"$ncgr_m_name_b\"\n";
		print SCR "\"$ncgr_f_name_b\"\n";
		print SCR "\"$ncbr_m_name_b\"\n";
		print SCR "\"$ncbr_f_name_b\"\n";
		print SCR "\"$ncer_name_b\"\n";
		print SCR "\"$nanr_name_b\"\n";
		print SCR "\"$nmcr_name_b\"\n";
		print SCR "\"$nmar_name_b\"\n";
		print SCR "\"$ncec_name_b\"\n";
		print SCR "\"$nclr_name_n\"\n";
		print SCR "\"$nclr_name_r\"\n";

		$no++;
		$list_count--;
	}

	close( SCR );

	#ポケモンビューワー用のヘッダーファイルを生成
	open( DEF, "> pokemon_viewer_def.h");

	print	DEF	"//============================================================================================\n";
	print	DEF	"/**\n";
	print	DEF	"* \@file		pokenmon_viewer_def.h\n";
	print	DEF	"* \@brief	ポケモンビューワー用定義\n";
	print	DEF	"* \@author	soga\n";
	print	DEF	"* コンバータから自動生成\n";
	print	DEF	"*/\n";
	print	DEF	"//============================================================================================\n";
	print	DEF	"\n\n";
	print	DEF	"#pragma once\n\n";
	print DEF "#define	NEW_POKEMON_COUNT	( " . ( $no - 493 ) . " ) \t//WBからの新ポケの総数\n\n";

	close( DEF );


