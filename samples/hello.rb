def cube(x)
  x*x*x
end
10000.times do |i|
  10000.times{|x| }
  syslog(4,(cube(i%10)).to_s)
  GC.start
end
