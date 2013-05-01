waf:
	waf

run: waf
	./build/debug/cputemp
	./build/debug/cputemp -f

python:
	python cputemp.py -h
	python cputemp.py
