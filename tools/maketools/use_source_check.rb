###########################################
#
# use_source_check.rb 
# 使用されていないソースチェック
#
###########################################


PROGDIR = ENV["PROJECT_PROGDIR"].chomp

def debug_puts arg
  #puts arg
end


#ファイル名配列
class FILE_ARRAY

  attr :array

  def initialize 
    @array = Array.new
  end

  def set path

    add_file = 0

    #*.c or *.h
    if path =~ /.*\.c \\*/ then
      add_file = 1
    end
    if path =~ /.*\.h \\*/ then
      add_file = 1
    end

    #追加の必要なし。
    if add_file == 0 then
      return 
    end
    

    path = path.sub( /.*=/, "" )
    path = path.sub( /\s/, "" )
    path = path.sub( /\t/, "" )
    path = path.sub( /\\/, "" ).chomp

    #BaseNameを保存
    path = File.basename( path )

    #debug_puts( "set #{path}\n" )

    @array << path
    
  end


  def hit check_path

    check_file = 0

    #*.c or *.h
    if check_path =~ /.*\.c$/ then
      check_file = 1
    end
    if check_path =~ /.*\.h$/ then
      check_file = 1
    end

    #checkの必要なし。
    if check_file == 0 then
      return 2
    end
    

    check_path = check_path.sub( /.*=/, "" )
    check_path = check_path.sub( /\s/, "" )
    check_path = check_path.sub( /\t/, "" )
    check_path = check_path.sub( /\\/, "" ).chomp

    #BaseNameをチェック
    check_path = File.basename( check_path )
    
    @array.each{|filename|

      if check_path == filename  then
        return 1
      end
    }
    return 0
  end

  def uniq_array

    @array = @array.uniq
    
  end

  def listDump

    debug_puts( "check list\n" )
    @array.each{|filename|
      debug_puts( "#{filename}\n" )
    }
  end
  
end


#1Directory内のファイルをチェック
def checkDir( tag_str, path, array )

  puts "#{tag_str}check #{path} ...\n"
  puts "#{tag_str}{"

  Dir.glob("#{path}/*"){|file|
    
    if File.ftype( file ) == "file" then

      if array.hit( file ) == 0 then
        puts "#{tag_str}\t[no_use] #{file}\n"
      end
      
    elsif File.ftype( file ) == "directory" then

      #再帰
      sub_tag_str = "#{tag_str}\t"
      checkDir( sub_tag_str, file, array )
      
    end
    
  }

  puts "#{tag_str}}\n"
  
end



#まず、dependとmake_prog_filesからソースリストを作成
all_file_array  = FILE_ARRAY.new


File.open( "#{PROGDIR}/make_prog_files" ){|file|

  file.each{|line|
    all_file_array.set( line )
  }
  
}

Dir.glob( "#{PROGDIR}/depend/ARM9-TS.HYB/Release/*.d" ){|filename|

  File.open( filename ){|file|
    file.each{|line|
      all_file_array.set( line )
    }
  }
}

#重複データを排除
all_file_array.uniq_array()
all_file_array.listDump()

#include以下
checkDir( "", "#{PROGDIR}/include/", all_file_array )

#src以下
checkDir( "", "#{PROGDIR}/src/", all_file_array )

#arc以下
checkDir( "", "#{PROGDIR}/arc/", all_file_array )



