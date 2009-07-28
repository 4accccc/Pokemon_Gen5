#!/usr/bin/perl

$err = &main();
exit($err);

sub main {
	if( @ARGV < 6 )
	{
		print "usage : >perl makedepend.pl <src_dir> <dst_dir> <IN_resource_list> <OUT_depend_list> <OUT_target_list> <default_lang_code> [+sub_lang_code ...]\n";
		return 1;
	}

	my $SrcDir = $ARGV[0];
	my $DstDir = $ARGV[1];
	my $ResourceList = $ARGV[2];
	my $DependListName = $ARGV[3];
	my $TargetListName = $ARGV[4];
	my @Langs = @ARGV[5..(@ARGV-1)];

	if(open(SRC, "<$ResourceList"))
	{
		my @src = <SRC>;
		my @targets;
		my $i;

		close(SRC);

		for($i=0; $i<@src; $i++)
		{
			$src[$i] =~ s/[\r\n]//g;
			push (@targets, &make_target_path($SrcDir, $DstDir, $src[$i]));
		}

		print_targets(\@targets, $TargetListName);
		print_depends(\@src, \@targets, \@Langs, $DependListName);

		return 0;
	}

	return 1;
}


sub make_target_path {
	my $src_dir = shift;
	my $dst_dir = shift;
	my $file = shift;

	if( $file =~ /^src\/script\//){
		$dst_dir .= "/script";
	}

	if($file =~ /\//)
	{
		my @tmp = split(/\//, $file);
		$file = $tmp[(@tmp-1)];
	}


	$file =~ s/gmm$/dat/;
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

	if(open(DEPEND_FILE, ">$filename"))
	{
		my $i, $L;
		for($i=0; $i<@$src_ref; $i++)
		{
			$dst_dir = make_dirpath( $$target_ref[$i] );
			print DEPEND_FILE "$$target_ref[$i]:\t$$src_ref[$i] resource.pl\n";
#			print DEPEND_FILE "$$target_ref[$i]:\t$$src_ref[$i]\n";
			print DEPEND_FILE "\t perl msgconv.pl $$src_ref[$i] $dst_dir";

			for($L=0; $L<@$lang_ref; $L++)
			{
				print DEPEND_FILE " $$lang_ref[$L]";
			}
			printf DEPEND_FILE "\n\n";
		}
		close(DEPEND_FILE);
	}
}
