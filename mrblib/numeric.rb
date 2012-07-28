##
# Integer
#  
# ISO 15.2.8
class Integer

  ##
  # Calls the given block +self+ times.
  #
  # ISO 15.2.8.3.22
  def times(&block)
    i = 0
    while(i < self)
      block.call(i)
      i += 1
    end
    self
  end
end

##
# Numeric is comparable
#
# ISO 15.2.7.3
module Comparable; end
class Numeric
  include Comparable
end
