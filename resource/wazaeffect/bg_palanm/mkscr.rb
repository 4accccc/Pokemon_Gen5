=begin
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		\home\swan\resource\wazaeffect\bg_palanm\mkscr.rb
 *	@brief  .datと.NCLRを集めて.scrに書き込む
 *	@author		Katsuhiko=Ichiraku
 *	@date		2010.11.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
=end


	if ARGV.size < 1
		print "error: ruby mkscr.rb write_file\n";
		print "write_file: 書き出すファイル名\n";
		exit( 1 )
	end

	ARGV_WRITE_FILE = 0

	fp_w = open( ARGV[ARGV_WRITE_FILE], "w" )

  #ファイルリスト生成
  Dir::glob("./*.dat").each {|f|
    name = File::basename( f, '.*' )
    fp_w.print( "\"" + name + ".dat\"\n" )
    fp_w.print( "\"" + name + ".NCLR\"\n" )
  }

  fp_w.close

