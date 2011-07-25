# @file	cdef_perser.rb
# @brief define定義ファイルパーサー
# @author	Miyuki Iwasawa
# 
#構造体定義
$_defID = Struct.new("DefineID", :id, :idx)

#デファイン定義ファイルを解釈する
class CDef_Perser
  #IDからIndexを検索
  def searchID id
	  for m in @data do
	    if m.id == id then
	  	return m.idx
  	  end
  	end
  	return nil
  end
 
  #IndexからIDを検索
  def searchIdx idx
	  for m in @data do
	    if m.idx == idx then
		  return m.ID
	    end
	  end
  	return nil
  end
 
  #print
  def printMember
	  print "Print Member\n"
	  for m in @data do
	    print m.id + " -> " + m.idx.to_s + "\n"
	  end
  end
 
  #def_perser
  def perseDefine fpath,logpath
	  f = nil
	  if logpath != nil then
	    f = open(logpath,"w")
	  end

  	File.open(fpath){ |file| 
	    idx = Array.new

	    while line=file.gets
        buf = line
		  
        idx.clear

		    if buf.slice!(/#define\s+/) == nil then
		      next
		    end
#        print("#{buf}\n")
		
        id = buf.slice!(/\w+/)
        if id == nil then
          print "Error! cdef_perser.rb 60\n ->#{line}\n"
        end
    
        num = buf.slice(/\d+/)
        if num == nil then
          next
		    end
		    @data << $_defID.new
		   
        cr = @data.last()
        cr.id = id
		    cr.idx = num.to_i

        if f != nil then
  		    f.print("#{cr.id} -> #{cr.idx}\n")
        end

		    @data_num += 1
	    end
	  }
	  f.close
  end
  
  def initialize fpath,logpath
	  @data = nil
	  @data_num = 0
	  @data = Array.new

	  perseDefine(fpath,logpath)
  end
end
#end class CDef_Perser 

