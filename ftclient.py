#!/usr/bin/python

import socket
import sys
from struct import *
import os.path

#-----------
# Functions
#-----------

# takes a port number and validates its range
# returns -1 on failure, port# on success
def validatePort(port):
    if int(port) < 1024 or int(port) > 65535:
        return -1
    else:
        return int(port)

# prints a port usage error message
def portError():
    print "Ports must be in range 1024 - 65535"

# takes a hostname and port number
# opens a socket and connects to the supplied
# host on the supplied port number
# returns the socket file descriptor on success,
# -1 on failure
def serverconnect(host, port):
    # open the socket and connect
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect((host, port))
    except:
        return -1
    return s

# takes a hostname and port number
# connects to the server
def initiateContact(host, port):
    if validatePort(port) < 1:
       portError()
       sys.exit(0)

    s = serverconnect(host, port)

    if s < 0:
        print "Error connecting to " + host + ":" + str(port)
        sys.exit(0)
    return s

# takes a socket file descriptor and outgoing message
# sends the size of the upcoming message over the connection,
# then the message itself
def sendMsg(sockfd, outgoing):
    size = len(outgoing)            # size of the outgoing message
    try:
        # https://docs.python.org/2/library/struct.html
        data = pack('I', size)          # pack the size for transmission
        sockfd.send(data)               # send the size of the message
        sockfd.send(outgoing)           # send the message itself
    except:
        print "An Unknown Error Occurred"
        d.close()
        c.close()
        sys.exit(1)

# takes a socket file descriptor
# receives the size of the upcoming message over the connection,
# then receives the message itself. the message is then returned
def recvMsg(sockfd):
    try:
        data = sockfd.recv(4)               # get the size of the incoming message
        recvsize = unpack('I', data)        # unpack it
        incoming = sockfd.recv(recvsize[0]) # get it out of the python tuple
        return incoming			# return the message received
    except:
        print "An Unknown Error Occurred"
        d.close()
        c.close()
        sys.exit(1)

# takes a socket file descriptor and outgoing port number
# sends the port number over the connection
def sendPort(sockfd, port):
    try:
        data = pack('I', port)
        sockfd.send(data)
    except:
        print "An Unknown Error Occurred"
        d.close()
        c.close()
        sys.exit(1)

# takes a socket file descriptor, port number,
# and outgoing message
# sends the message size and message contents
def makeRequest(sockfd, port, outgoing):
    try:
        sendMsg(sockfd, outgoing)
        sendPort(sockfd, port)
    except:
        print "An Unknown Error Occurred"
        d.close()
        c.close()
        sys.exit(1)

# takes a socket file descriptor and filename
# receives the requested data and writes it to file
# prints error message if file does not exist
def receiveFile(socket, fullfile):
    # split the filename and extension
    filename, file_ext = os.path.splitext(fullfile)

    # get the file
    print "Initiating Transfer Request"
    file = recvMsg(socket)

    overwrite = True
    # check if it already exists

    # http://stackoverflow.com/questions/82831/check-whether-a-file-exists-using-python
    if os.path.isfile(fullfile):
        # prompt for overwrite
        choice = raw_input("File Already Exists; Overwrite? (y/n)")
        if choice == 'y':
            overwrite = True
        else:
            overwrite = False
         
    # if not overwriting, append _copy to filename
    if overwrite == False:
        filename = filename + "_copy"
        fullfile = filename + file_ext

    # write the data to a file
    target_file = open(fullfile, 'w')
    target_file.write(file)

    print("Transfer Complete")


# check the cmd line args
if len(sys.argv) < 5 or len(sys.argv) > 6:
    print "\nUsage:\n $ python ftclient.py [SERVER_HOST] [SERVER_PORT] [COMMAND] [DATA_PORT] <FILENAME>\n<FILENAME> is an optional parameter\n"
    sys.exit(0)
else:
    # connect to the server
    c = initiateContact(sys.argv[1], int(sys.argv[2]))

    # queue the outgoing command
    command = sys.argv[3]

    # send the request
    makeRequest(c, int(sys.argv[4]), command)

    # open data connection for the response
    d = initiateContact(sys.argv[1], int(sys.argv[4]))
    
    # -g (get command)
    if command == "-g":
        try:
            filename = sys.argv[5]	# get the filename for the request
        except:
            print "Error:\n  <FILENAME> Parameter Required with [-g] Command"
            c.close()
            d.close()
            sys.exit(0)

        # send the filename
        makeRequest(c, int(sys.argv[4]), filename)

        # get the response
        incoming = recvMsg(c)

        # data connection for the file
        if incoming == "DATA":
            # get the file
            receiveFile(d, filename)
        else:
            # error message
            print "Requested File Does Not Exist; Please Try Again"

    # list command/bad command
    else:
        # get the response on the control connection
        incoming = recvMsg(c)
        # if we hav acknowledgement to rec'v on the data line
        if incoming == "DATA":
            incoming = recvMsg(d)	# rec'v on the data line
            print incoming

        print incoming	# print the message

    # close the data socket (only at program exit, just to be sure)
    d.close()

    print ("Closing Control Connection to Server")
    # close the control socket
    c.close()