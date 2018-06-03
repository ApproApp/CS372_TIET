-------------
Instructions
-------------
Make ftserver application using provided makefile
with command:
  $ make

Remove executable program file with command:
  $ make clean

ftclient application does not require compilation.

-------------
Usage
-------------

ftserver:
---------
  $ ftserver [port number]

ftserver takes a port number on which to listen for connections from
ftclient on a control port.
Upon a connection, ftclient responds to the supplied command over a separate
data port.
[-l] sends the contents of the current directory (excluding "." and "..")
[-g] validates the supplied filename and transfers it to ftclient.

-----------
important:
-----------
The maximum filesize for transfers is 8192 Bytes

The data connection is closed by ftserver after each transfer.
Child processes close the control connection upon exiting.

After completing a transfer, ftserver will
return to listening for connections until it is terminated with CTRL-C.

ftclient:
----------
  $ python ftclient.py [hostname] [server port] [<-g> or <-l>] [data port] <filename>

ftclient takes as arguments the hostname and port number on which
ftserver is running, and a command ([-l] or [-g]) and data port.  If [-g] is
the supplied command, <filename> is required.
[-l] requests a list of all files in the server's current directory
[-g] requests a transfer of the file supplied in <filename>

-----------
important:
-----------
It is necessary to supply the full hostname for the program to work.
Supplying "localhost" as parameter will likely produce errors.

Occasionally, network errors occur and error messages are provided.
It may sometimes be necessary to repeat commands if this is the case.

After completing a request, ftclient will close the control
connection to the server.
ftclient closes the data connection upon exiting.

------------
Notes
------------
Additional Resources Used:
(Individually noted in source code comments enclosed with
*****************
  resource use
*****************)
(Only the first use of the corresponding resource is noted)

https://docs.python.org/2/library/struct.html
https://docs.python.org/2/howto/sockets.html
http://learnpythonthehardway.org/book/ex16.html
http://www.thegeekstuff.com/2012/06/c-directory/
http://stackoverflow.com/questions/82831/check-whether-a-file-exists-using-python


# Running Program

## Open 2 Putty Clients
![Alt text](img/Capture1.PNG?raw=true "Image")
## input host names press open
![Alt text](img/Capture2.PNG?raw=true "Image")
## input id and pwd and start session
![Alt text](img/Capture3.PNG?raw=true "Image")
## change directory to scratch
![Alt text](img/Capture4.PNG?raw=true "Image")
## Run gcc ftserver.c -o ftserver also make a dummy text file
![Alt text](img/Capture5.PNG?raw=true "Image")
## use nano editor to put content use ctrl+o to save ctrl+x to exit 
![Alt text](img/Capture6.PNG?raw=true "Image")
## Run commands on flip 2 you will get following outputs at flip1
![Alt text](img/Capture7.PNG?raw=true "Image")
## Run commands on flip 2 as given in assignment
![Alt text](img/Capture8.PNG?raw=true "Image")
