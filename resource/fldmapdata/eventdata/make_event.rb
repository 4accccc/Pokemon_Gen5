#============================================================================
#
#
#   イベントデータコンバーター
#
#   2009.05.24    tamada
#
#
#============================================================================

class DataFormatError < Exception; end


#============================================================================
#
#   イベントデータクラス
#
#============================================================================
class EventData

  attr :zonename
  attr :path
  attr :section_name
  attr :version
  attr :num
  attr :type

  def initialize stream, section_name
    @zonename = File.basename(stream.path,".*")
    @path = stream.path
    @section_name = section_name
    @items = Array.new
    flag = false
    text_start = Regexp.new "\#SECTION START:#{@section_name}"
    text_end = Regexp.new "\#SECTION End:#{@section_name}"

    while(true) 
      line = stream.gets
      if line == nil then
        puts "stream read end"
        break
      end
      if line =~text_start then
        puts "SECTION START: #{@section_name}"
        flag = true
        next
      end
      if line =~text_end then
        puts "SECTION END: #{@section_name}"
        break
      end
      if flag == false then
        next
      end

      if line =~/Map Event List data/ then
        puts "START METHOD read_items"
        read_items stream
      end

    end
    if @version == nil || @num == nil || @type == nil then
      raise DataFormatError
    end
  end


  def read_items stream

    if @num == 0 then return end

    dummy = stream.gets #save data
    @version = Integer(stream.gets) #version
    puts "Version = #{version}"

    line = stream.gets
    if line =~ /^#hold event type:/ then
      @type = Integer(stream.gets) #event type
      puts "event type = #{type}"
    else
      raise DataFormatError, line
    end

    if stream.gets =~ /^#hold event number:/ then
      @num = Integer(stream.gets) #event number
      puts "event num = #{num}"
    else
      raise DataFormatError 
    end

    count = 0
    line = stream.gets

    while (count < @num - 1)
      #puts "count:#{count} < @num:#{@num}"
      string = ""
      raise DataFormatError unless line =~/^#event number/
      string += line

      while(true)
        line = stream.gets
        if line =~/^\#event number/ then break end
        string += line
      end

      @items << string
      count += 1
    end
    puts "end of read_items"
  end


  def dump outputstream
    puts "number of events:#{@num}"
    @items.each{|item|
      outputstream.puts"#{item}"
    }
  end

end   # end of class EventData

#============================================================================
#
# ドアイベントクラス
#
#============================================================================
class DoorEvent

  attr :number
  attr :door_id
  attr :next_zone_id
  attr :next_door_id
  attr :posx 
  attr :posy
  attr :posz


  def initialize lines
    line = lines.shift
    if line =~/#event number:/ then
      column = line.split
      @number = Integer(column[2])
    else
      raise DataFormatError
    end

    line = lines.shift
    if line =~/#Door Event Label/ then
      @door_id = lines.shift
    else
      raise DataFormatError
    end
    line = lines.shift
    if line =~/#Next Zone ID Name/ then
      @next_zone_id = lines.shift
    else
      raise DataFormatError
    end
    line = lines.shift
    if line =~/#Next Door ID Name/ then
      @next_door_id = lines.shift
    else
      raise DataFormatError
    end
    line = lines.shift
    if line=~/#position/ then
      column = lines.shift.split
      @x = Integer(column[0])
      @y = Integer(column[1])
      @z = Integer(column[2])
    else
      raise DataFormatError
    end
  end

  def dump outputstream
    outputstream.puts "\t{//#{@door_id} = #{@number}"
    outputstream.puts "\t\t{#{@x}, #{@y}, #{@z}},"
    outputstream.puts "\t\t#{@next_zone_id}, #{@next_door_id},"
    outputstream.puts "\t\tEXIT_DIR_DOWN, EXIT_TYPE_NONE"
    outputstream.puts "\t},"
  end

end

class ObjEvent
end

#============================================================================
#
# ドアイベントデータクラス
#
#============================================================================
class DoorEventData < EventData

  def initialize stream, section_name
    super
    @doors = Array.new
    @items.each{|item|
      lines = item.split(/\r\n/)
      @doors << DoorEvent.new(lines)
    }
  end

  def dump outputstream

    outputstream.puts "const CONNECT_DATA ConnectData_#{@zonename}[] = {"
    @doors.each{|door|
      door.dump outputstream
    }
    outputstream.puts "};"

    outputstream.puts "const int ConnectCount_#{@zonename} = NELEMS(ConnectData_#{@zonename})"
  end

end


#============================================================================
#
# コンバート本体
#
#============================================================================
begin
  File.open(ARGV[0]){|file|
    obj_events = EventData.new(file, "OBJ_EVENT")
    bg_events = EventData.new(file, "BG_EVENT")
    pos_events = EventData.new(file, "POS_EVENT")
    door_events = DoorEventData.new(file, "DOOR_EVENT")

    door_events.dump STDOUT
  }
end




