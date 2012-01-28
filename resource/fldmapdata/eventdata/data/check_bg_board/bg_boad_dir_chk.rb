#@note 看板の話しかけ向きをチェックします

#mev収集

class BG_info
  attr_accessor  :dir, :scr_id, :normal
  @dir
  @scr_id
  @normal
end


gCheck = false
gNormal = 0

def Search(file, ary)
  mode = 0
  while line = file.gets
    str = line.chomp("\n").chomp("\r")

    if mode == 0 then
      #イベントラベルを調べとく
      rc = str.include?("Bg Event Label")
      if rc == true then
        mode = 1
      end
    elsif mode == 1 then
      #
      rc1 = str.include?("_BOARD")
      rc2 = str.include?("_SIGN")
      if (rc1 == true) || (rc2 == true) then
        gCheck = true
      else
        gCheck = false
      end
      mode = 2
    elsif mode == 2 then
      #BG_TALK_TYPE_BOADを検索
      rc = str.include?("BG_TALK_TYPE_BOARD")
      if rc == true then
        mode = 3
        gNormal = 0
      else
        rc = str.include?("BG_TALK_TYPE_NORMAL")
        if (gCheck == true) && (rc == true) then
#          p "normal hit"
          mode = 3
          gNormal = 1
        end
      end
    elsif mode == 3 then
      #Bg Event Reaction Direction IDを検索
      rc = str.include?("Bg Event Reaction Direction ID")
      if rc == true then
        mode = 4
      end
    elsif mode == 4
      #話しかけ方向を収集
      info = BG_info.new
      info.dir = str
      info.normal = gNormal
      mode = 5
    elsif mode == 5
      #Bg Script Nameを検索
      rc = str.include?("Bg Script Name")
      if rc == true then
        mode = 6
      end
    else
      info.scr_id = str
      ary << info
      mode = 0
#      if gCheck == true then
#       p info.scr_id
#      end
    end
  end
end

ary = Array.new

all_ary = Dir.glob("../*.mev")

all_ary.each{|i|
#  p i
  file = open(i,"r")
  Search(file, ary)
}

ary.each{|i|
  printf("%s ==> %s  NRM-->%d\n", i.dir, i.scr_id, i.normal)
}





