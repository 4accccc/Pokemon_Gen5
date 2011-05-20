#!/usr/bin/perl

$err = &main();
exit($err);

sub main {
	if( @ARGV < 6 )
	{
		print "usage : >perl makedepend.pl <dst_dir> <IN_resource_list> <OUT_depend_list> <OUT_target_list> <default_lang_code> [+sub_lang_code ...]\n";
		return 1;
	}

	my $DstDir = $ARGV[0];
	my $ResourceList = $ARGV[1];
	my $DependListName = $ARGV[2];
	my $TargetListName = $ARGV[3];
	my @Langs = @ARGV[4..(@ARGV-1)];

	if(open(SRC, "<$ResourceList"))
	{
		my @src = <SRC>;
		my @targets;
		my $i;

		close(SRC);

		for($i=0; $i<@src; $i++)
		{
			$src[$i] =~ s/[\r\n]//g;
			push (@targets, &make_target_path($DstDir, $src[$i]));
		}

		print_targets(\@targets, $TargetListName);
		print_depends(\@src, \@targets, \@Langs, $DependListName);

		return 0;
	}

	return 1;
}


sub make_target_path {
	my $dst_dir = shift;
	my $file = shift;

	if( $file =~ /^src\/script_message\// || $file =~ /^msproj\/src\/script_message\//){
		$dst_dir .= "/script";
	}
	if( $file =~ /^src\/debug\// || $file =~ /^msproj\/src\/debug\//){
		$dst_dir .= "/debug";
	}

	if($file =~ /\//)
	{
		my @tmp = split(/\//, $file);
		$file = $tmp[(@tmp-1)];
	}


	$file =~ s/(gmm|mstxt)$/dat/;
	return $dst_dir . '/' . $file;
}


sub print_targets {
	my $ref = shift;
	my $filename = shift;

	if(open(TARGET_FILE, ">$filename"))
	{
		print TARGET_FILE "TARGETS = \\\n";
		foreach (@$ref)
		{
			print TARGET_FILE "\t$_ \\\n";
		}
		close(TARGET_FILE);
	}
}

sub make_dirpath {
	my $file_path = shift;

	my @elems = split( /\//, $file_path );
	pop @elems;

	my $dirpath = "";
	my $tmp = "";
	foreach $tmp (@elems) {
		$dirpath .= "$tmp/";
	}
	return $dirpath;
}

sub print_depends {
	my $src_ref = shift;
	my $target_ref = shift;
	my $lang_ref = shift;
	my $filename = shift;
	my $dst_dir;
	my $lang_str = join(" ", @$lang_ref);
	my @src_mstxt_list;

	if(open(DEPEND_FILE, ">$filename"))
	{
		my $i, $L;

		# 1つのコマンド(MsgConv.exe)で複数のファイルを生成する時の、擬似ターゲット
		# これにdatを依存させ、かつこれを更新することにより、datが更新されたことを示す
		my $pseudo_target_name = "pseudo_target_$$lang_ref[0]";

		for($i=0; $i<@$src_ref; $i++)
		{
			if ($$src_ref[$i] =~ /\.gmm$/)
			{
				$dst_dir = make_dirpath( $$target_ref[$i] );
				print DEPEND_FILE "$$target_ref[$i]:\t$$src_ref[$i] resource.pl\n";
#				print DEPEND_FILE "$$target_ref[$i]:\t$$src_ref[$i]\n";
				print DEPEND_FILE "\t perl msgconv.pl $$src_ref[$i] $dst_dir $lang_str\n\n";
			}
			else
			{
				push (@src_mstxt_list, $$src_ref[$i]);
				print DEPEND_FILE "$$target_ref[$i]:\t$pseudo_target_name\n";
			}
		}
		print DEPEND_FILE "\n";
		print DEPEND_FILE "SRC_MSTXT_LIST = \\\n\t".join(" \\\n\t", @src_mstxt_list)."\n\n";
		print DEPEND_FILE "$pseudo_target_name : \$(SRC_MSTXT_LIST)\n";
		print DEPEND_FILE "\t \$(PROJECT_ROOT)/tools/message/MsgConv.exe config.xml msproj/src dst log/unused_$$lang_ref[0].txt $lang_str > log/error_$$lang_ref[0].txt\n";
		print DEPEND_FILE "\t touch \$@\n";
		close(DEPEND_FILE);
	}
}
