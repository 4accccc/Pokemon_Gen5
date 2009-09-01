#!ruby -Ks
#!/usr/bin/ruby
#
# gmmからポケモンリストを作る
# 2009.08.31  k.ohno
#
#

require "jcode"
require 'nkf'

#
#	<row id="MONSNAME_000">
#		<language name="JPN">ーーーーー</language>
#	</row>
#



class GmmRead
  
  def initialize
  end
  
  
  
  def convline( gmmfile )
    
    
    fpr = File.new(gmmfile,"r")
    pokeno = 0
    fpr.each{ |line|
      if line =~ /\<row id=\"MONSNAME_(\d+)\"\>/
        pokeno = $1
      elsif line =~ /\<language name\=\"JPN\"\>(\S+)\<\/language\>/
        if pokeno != 0
          p pokeno + " " + NKF.nkf('-m0 -Ws',$1)
          pokeno = 0
        end
      end
    }
    fpr.close
    
  end
end

begin
  
  dep = GmmRead.new
  dep.convline( ARGV[0] )
  exit(0)
end
