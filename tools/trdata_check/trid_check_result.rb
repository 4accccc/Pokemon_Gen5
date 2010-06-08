class TrIDCheckResult
  def initialize
    @result_hash = Hash.new
  end

  def get( trid )
    return @result_hash[ trid ]
  end

  def set_key( trid )
    @result_hash[ trid ] = String.new
  end

  def has_key( trid )
    return @result_hash.has_key?( trid )
  end

  def add( trid, result )
    if @result_hash.has_key?( trid ) == false then
      self.set_key( trid )
    end
    @result_hash[ trid ] += result
  end
end
