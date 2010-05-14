
class Message

  def initialize( filename, str_id )
    @filename = filename
    @str_id = str_id
  end

  attr_reader :filename, :str_id

end
