proctemp
========

Linux processor temperature utilities.

These utilities all require sensors(1).

Run the sensors command to ensure that you have it installed.  You should see
something similar to this:

	user@hostname/~ $ sensors
	coretemp-isa-0000
	Adapter: ISA adapter
	Physical id 0:  +46.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 0:         +42.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 1:         +44.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 2:         +45.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 3:         +42.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 4:         +42.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 5:         +40.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 6:         +44.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 7:         +40.0°C  (high = +82.0°C, crit = +92.0°C)

	coretemp-isa-0001
	Adapter: ISA adapter
	Physical id 1:  +50.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 0:         +46.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 1:         +50.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 2:         +48.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 3:         +48.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 4:         +50.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 5:         +48.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 6:         +44.0°C  (high = +82.0°C, crit = +92.0°C)
	Core 7:         +44.0°C  (high = +82.0°C, crit = +92.0°C)

	nouveau-pci-0500
	Adapter: PCI adapter
	temp1:        +62.0°C  (high = +100.0°C, crit = +110.0°C)

proctempview
============

Graphically show the CPU and GPU temperatures in real time.

This program will display the temperatures using ncurses.  Similar to top(1) or
htop(1), this will allow you to check temperatures in a console.

Simply run the program and it will display the temperatures at one second
intervals.

![proctempview example image](https://github.com/jeffsp/proctemp/raw/master/proctempview_example.png "proctempview example")

html output
-----------

You may optionally configure proctempview to write the output to an html file
that displays temperatures using the Google Charts API.  This will allow you to
view the temperatures through a browser.  The charts will automatically refresh
themselves every few seconds.

To use this feature, you have to know what you are doing.  This feature is not
useful unless you have a web server running on your system.

Configuration is achieved by editing the ~/.config/proctemp/proctempviewrc file.

	- Don't change the order of the options in this file.
	- Enable the feature by changing the html_output variable to '1'.
	- Change the 'html_filename' variable to a filename to which you will have
	  write access.  For example, '~/public_html/proctemp.html'.  Don't put
	  spaces in the filename.
	- Run proctempview.  If it can't write to the html file, the program will
	  report an error and exit.

Now point your browser to the location of the file that is visible through the web
server interface.  For example, 'http://servername/~username/proctemp.html'.

![proctempview html output example image](https://github.com/jeffsp/proctemp/raw/master/proctempview_html_example.png "proctempview html output example")

proctempalert
=============

Alert the user if the CPU or GPU temperature is too high.

To receive alerts, edit your crontab:

	user@hostname/~ $ crontab -e

It should look something like this:

	# This file contains tasks to be run by cron.
	#
	# Each task to run has to be defined through a single line
	...
	# For more information see the manual pages of crontab(5) and cron(8)
	#
	# m h  dom mon dow   command
	# mirror home every night
	0	2	*	*	*	~/bin/mirror_home.sh 2>&1 | mail -s "`hostname` mirror_home.sh `date`" username@email.com
	# backup on saturdays
	0	3	*	*	sat	~/bin/backup_home.sh 2>&1 | mail -s "`hostname` backup_home.sh `date`" username@email.com
	# report disk usage on mondays
	0	6	*	*	mon	df -h | mail -s "`hostname` df `date`" username@email.com
	...

Add the following two lines (replace username@email.com with your email
address):

	# every tens minutes, check if cpu temperature is high
	*/10	*	*	*	*	/usr/local/bin/proctempalert --debug=0 --high_cmd='sensors | mail -s "`hostname` is HIGH" username@email.com' &> /dev/null

	# every two minutes, check if cpu temperature is critical
	*/2	*	*	*	*	/usr/local/bin/proctempalert --debug=0 --critical_cmd='sensors | mail -s "`hostname` is CRITICAL" username@email.com' &> /dev/null

And exit you editor to install the new crontab tab.

If you want to make sure you have it setup correctly, change the first
'--debug=0' with '--debug=1' and the second '--debug=0' with '--debug=2'.  If
you have it setup correctly, you should start receiving email alerts every 10
minutes that your CPU or GPU temperature is too high and every 2 minutes that
your CPU or GPU temperature is critical.  When you are convinced it is setup
correctly, change the debug's back to 0's or you will get tons of emails.
