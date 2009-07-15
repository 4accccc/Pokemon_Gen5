###############################################################
#
#   tamada GAMEFREAK inc.
#   2009.07.15
#
###############################################################

require "fileutils"

SCRMSG_PATH = "../../message/src/script/"


def readIDs( filename )
  zone_ids = Array.new
  File.open(filename){|file|
    file.gets #æ“ªs“Ç‚İÌ‚Ä
    while line = file.gets
      column = line.split
      id = column[0].downcase
      if id == "end" then break end
      if zone_ids.index(id) != nil then
        raise Exception, "#{id}‚ª•¡”‚ ‚è‚Ü‚·"
      end
      zone_ids << id
    end
  }
  return zone_ids
end

def makeGMMs( zone_ids )
  zone_ids.each{|id|
    target = SCRMSG_PATH + id + ".gmm"
    unless FileTest.exists?(target) then
      FileUtils.cp("dummyfiles/dummy.gmm", target)
      system ("svn add " + target)
      puts "#{id}.gmm not found. made it!!"
    end
  }
end

ids = readIDs( ARGV[0] )
makeGMMs( ids )

