def cube(x)
  x*x*x
end
10000.times do |i|
  100000.times{|x| }
  syslog(4,i.to_s+" "+(cube(i)).to_s)
  GC.start
end
