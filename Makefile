# @file Makefile
# @brief cputemp makefile
# @author Jeff Perry <jeffsp@gmail.com>
# @version 1.0
# @date 2013-05-01

waf:
	waf

run: waf
	./build/debug/cputemp -g -f
