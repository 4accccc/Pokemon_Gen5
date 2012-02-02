#! ruby
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		multiboot_banner.rb
#	@brief  マルチブート用バナー作成
#	@author	Toru=Nagihashi
#	@data		2012.02.02
#
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
BINFILE     = ARGV[0]
OUTPUTNAME  = ARGV[1]

#=============================================================================
#
#   メイン処理
#
#=============================================================================
begin


charfile = File.new( "#{OUTPUTNAME}.char", "wb" )
charfile.binmode
pltfile = File.new( "#{OUTPUTNAME}.plt", "wb" )
pltfile.binmode

readfile  = File.open(BINFILE,"rb")
readfile.binmode

charfile.write( readfile.read( 0x200 ) )

readfile.seek( 0x1000, IO::SEEK_SET )

pltfile.write( readfile.read(0x20) )

readfile.close

pltfile.close
charfile.close

end

