# @file Makefile
# @brief cputemp makefile
# @author Jeff Perry <jeffsp@gmail.com>
# @version 1.0
# @date 2013-05-01

waf:
	waf configure
	waf

run: waf
	./build/debug/cputemp
	./build/debug/cputemp -f
	./build/debug/cputemp -g
	./build/debug/cputemp -f -g
	./build/debug/cputempx
