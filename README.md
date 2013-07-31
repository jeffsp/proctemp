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

Add the following two lines (replace username@email.com with your email
address):

	# every tens minutes, check if cpu temperature is high
	*/10	*	*	*	*	/usr/local/bin/proctempalert --debug=0 --high_cmd='sensors -f | mail -s "`hostname` is HOT" username@email.com' > /dev/null 2>&1

	# every two minutes, check if cpu temperature is critical
	*/2	*	*	*	*	/usr/local/bin/proctempalert --debug=0 --critical_cmd='sensors -f | mail -s "`hostname` is CRITICALLY HOT" username@email.com' > /dev/null 2>&1

And exit you editor to install the new crontab.

If you want to make sure you have it setup correctly, change the first
'--debug=0' with '--debug=1' and the second '--debug=0' with '--debug=2'.  If
you have it setup correctly, you should start receiving email alerts every 10
minutes that your CPU or GPU temperature is too high and every 2 minutes that
your CPU or GPU temperature is critical.

###proctempview HTML output

You may optionally configure proctempview to write the output to an html file
that displays temperatures using the Google Charts API.  This will allow you to
view the temperatures through a browser.  The charts will automatically refresh
themselves every few seconds.

Configuration is achieved by editing the ~/.config/proctemp/proctempviewrc file.

- Don't change the order of the options in this file.
- Enable the feature by changing the html\_output variable to '1'.
- Change the 'html\_filename' variable to a filename to which you will have
  write access.  For example, '~/public\_html/proctemp.html'.  Don't put
  spaces in the filename.
- Run proctempview.  If it can't write to the html file, the program will
  report an error and exit.

Now point your browser to the location of the file that is visible through the web
server interface.  For example, 'http://servername/~username/proctemp.html'.  The output will look similar to this:

![proctempview html output example image](https://github.com/jeffsp/proctemp/raw/master/proctempview_html_example.png "proctempview html output example")

