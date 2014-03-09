#!/usr/bin/ruby

`g++ ./main.cpp -o program`

type = %w(FCFS LCFS STJ ROBIN2 ROBIN5 ROBIN20)
title = %w(F L S R2 R5 R20)

(0..5).each do |c1|
  (0..5).each do |c2|
    `./program data1/input#{c2} #{type[c1]} > data2/output#{c2}_#{title[c1]}`
    `diff data1/output#{c2}_#{title[c1]} data2/output#{c2}_#{title[c1]} > diff/output#{c2}_#{title[c1]}`
  end
end
