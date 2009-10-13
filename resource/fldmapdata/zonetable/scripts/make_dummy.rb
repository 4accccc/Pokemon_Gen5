###############################################################
#
#   tamada GAMEFREAK inc.
#   2009.07.15
#
###############################################################

require "fileutils"

SCRMSG_PATH = "../../message/src/script_message/"
SCRIPT_PATH = "../script/"


###############################################################
###############################################################

#--------------------------------------------------------------
#--------------------------------------------------------------
def readIDs( filename )
  zone_ids = Array.new
  File.open(filename){|file|
    file.gets #先頭行読み捨て
    while line = file.gets
      column = line.split
      id = column[0].downcase
      if id == "end" then break end
      if zone_ids.index(id) != nil then
        raise Exception, "#{id}が複数あります"
      end
      zone_ids << id
    end
  }
  return zone_ids
end

#--------------------------------------------------------------
#--------------------------------------------------------------
def makeEvents( zone_ids )
  zone_ids.each{|id|
    command = "ruby ../eventdata/dummydata/make_dummy.rb #{id}"
    system( command )
  }
end

#--------------------------------------------------------------
#--------------------------------------------------------------
def makeGMMs( zone_ids )
  zone_ids.each{|id|
    target = SCRMSG_PATH + id + ".gmm"
    unless FileTest.exists?(target) then
      FileUtils.cp("dummyfiles/dummy.gmm", target)
      #system ("svn add " + target)
      puts "#{id}.gmm がみつからなかったのでダミーを作成しました！"
    end
  }
end
#--------------------------------------------------------------
#--------------------------------------------------------------
def checkGMMS( zone_ids )
  filepath = SCRMSG_PATH
  ext = ".gmm"
  check_files( zone_ids, filepath, ext)
end

def check_files( ids, filepath, ext)

  STDERR.puts "#{ext} checking ..."
  real_files = Hash.new

  files = Dir.glob( filepath + "*#{ext}" ).map{|file|
    name = File.basename(file)
    real_files[name] = nil
    name
  }

  ids.each{|id|
    idfile = id + "#{ext}"
    if real_files.has_key?(idfile) then
      real_files[idfile] = true
    else
      puts "#{idfile}:ファイルが存在しません。"
    end
  }
  real_files.each{|key, value|
    if value == nil then
      puts "#{key}:ファイルは存在しますが、定義されていません"
    end
  }
  
  STDERR.puts "#{ext} checking finished."
end

#--------------------------------------------------------------
#--------------------------------------------------------------
def makeScripts( zone_ids )
  zone_ids.each{|id|
    scrfile = SCRIPT_PATH + id + ".ev"
    sp_scrfile = SCRIPT_PATH + "sp_" + id + ".ev"
    command = "ruby ../script/scr_tools/make_dummy_script.rb ../script #{id}"
    command2 = "unix2dos -D #{scrfile}"
    command2 = "unix2dos -D #{sp_scrfile}"
      system (command)
    unless FileTest.exists?(scrfile) then
      puts "#{command}"
      system (command)
      #FileUtils.cp("dummyfiles/dumm
    end
  }
end

def makeScriptsIndex( zone_ids )
  File.open("../script/zone_script.list","w"){|file|
    file.puts "#自動生成ファイル zone_script.list 先頭"
    file.puts "SCRIPTFILES += \\"
    zone_ids.each{|id|
      file.puts "\t#{id}.ev \\"
      file.puts "\tsp_#{id}.ev \\"
    }
    file.puts "\n"
    file.puts "#自動生成ファイル zone_script.list 末尾"
  }

  File.open("../script/zone_script_bin.list","w"){|file|
    file.puts "#自動生成ファイル zone_script_bin.list 先頭"
    zone_ids.each{|id|
      file.puts "\"#{id}.ev\""
      file.puts "\"sp_#{id}.ev\""
    }
    file.puts "#自動生成ファイル zone_script_bin.list 末尾"
  }
end

def checkScripts( zone_ids )
  check_files( zone_ids, SCRIPT_PATH, ".ev")
  check_files( zone_ids, SCRIPT_PATH, "_def.h")
  sp_ids = zone_ids.map{|id| "sp_" + id}
  check_files( sp_ids, SCRIPT_PATH, ".ev")
  check_files( sp_ids, SCRIPT_PATH, "_def.h")
end

###############################################################
###############################################################

if ARGV[1] == "true" then
  ids = readIDs( ARGV[0] )
  makeEvents( ids )
  makeGMMs( ids )
  makeScripts( ids )
  makeScriptsIndex( ids )
elsif ARGV[1] == "false" then
  ids = readIDs( ARGV[0] )
  checkGMMS( ids )
  checkScripts( ids )
else
end

