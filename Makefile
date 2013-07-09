# @file Makefile
# @brief proctemp makefile
# @author Jeff Perry <jeffsp@gmail.com>
# @version 1.0
# @date 2013-05-01

waf:
	waf build -j 16

run: waf
	./build/debug/proctempview
	./build/debug/proctempalert
	./build/debug/proctempalert -f
	./build/debug/proctempalert -g
	./build/debug/proctempalert -f -g
	-./build/debug/proctempalert -d 1 --high_cmd='echo HIGH'
	-./build/debug/proctempalert -d 2 --critical_cmd='echo CRITICAL'
	-./build/debug/proctempalert -d 0 --high_cmd='echo HIGH' --critical_cmd='echo CRITICAL'
	-./build/debug/proctempalert -d 1 --high_cmd='echo HIGH' --critical_cmd='echo CRITICAL'
	-./build/debug/proctempalert -d 2 --high_cmd='echo HIGH' --critical_cmd='echo CRITICAL'
	-./build/debug/proctempalert -d 2 --critical_cmd='asdf'
	-./build/debug/proctempalert -d 2 --critical_cmd='./build/debug/proctempdump -f | mail -s "`hostname` is CRITICAL" jeffsp@gmail.com'
