#!/usr/bin/python3
import glob

print("Benchmark	Performance_Results(Cycles)	MPKI")
for filename in glob.iglob('*/zsim.out'):
	print(filename.split("/")[0], end="	")
	lines = []
	with open (filename, 'rt') as in_file:  # Open file lorem.txt for reading of text data
		cycles,misses,instr = -1,0,0
		for line in in_file:
			if line.find("westmere-") != -1:
				while line.find("cycles") == -1:
					line = next(in_file)
				cycles_curr = int(line.split(" ")[4]) + int(next(in_file).split(" ")[4])
				if cycles < cycles_curr:
					cycles = cycles_curr
				instr = instr + int(next(in_file).split(" ")[4])
			
			if line.find("l3-") != -1:
				while line.find("mGETS") == -1:
					line = next(in_file)
				misses = misses + int(line.split(" ")[4]) + int(next(in_file).split(" ")[4]) + int(next(in_file).split(" ")[4])

		MPKI = (misses/instr) * 1000
		print(cycles,"	",MPKI)
