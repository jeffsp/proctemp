#proctemp

Linux processor temperature utilities.

##Installation

	$ ./autogen.sh && ./configure && make
	...
	$ sudo make install

You must have the sensors and ncurses development libraries installed in order to compile the utilities.

##Applications
###proctempview

Graphically show the CPU and GPU temperatures in real time in a console.

###proctempalert

Alert the user via email if the CPU or GPU temperature becomes too high.

##Usage
###proctempview

	user@hostname/~ $ proctempview

![proctempview example image](https://github.com/jeffsp/proctemp/raw/master/proctempview_example.png "proctempview example")

###proctempalert

Temperature alerts are sent via cron(8).  See _Configuration_ below.

##Configuration

###proctempalert

To receive alerts, edit your crontab:

	user@hostname/~ $ crontab -e

and add the following two lines (replace username@email.com with your email
address):

	# every tens minutes, check if cpu temperature is high
	*/10	*	*	*	*	/usr/local/bin/proctempalert --debug=0 --high_cmd='sensors -f | mail -s "`hostname` is HOT" username@email.com' > /dev/null 2>&1

	# every two minutes, check if cpu temperature is critical
	*/2	*	*	*	*	/usr/local/bin/proctempalert --debug=0 --critical_cmd='sensors -f | mail -s "`hostname` is CRITICALLY HOT" username@email.com' > /dev/null 2>&1

If you want to make sure you have it setup correctly, change the first
'--debug=0' with '--debug=1' and the second '--debug=0' with '--debug=2'.  If
you have it setup correctly, you should start receiving email alerts every 10
minutes that your CPU or GPU temperature is too high and every 2 minutes that
your CPU or GPU temperature is critical.
