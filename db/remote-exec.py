#!/usr/bin/python

import paramiko
from paramiko import SSHException
import argparse
from getpass import getpass
from os import _exit
from pipes import quote
from shutil import copyfileobj
from socket import timeout as TimeoutException
from sys import stdin, stdout, stderr
from time import sleep

# Default timeout (in seconds) for SSH connection
TIMEOUT=10

# Called on timeout
def timed_out(name='Connection'):
	print >> stderr, name, "timed out"
	_exit(255)

# Called on ssh exception
def ssh_exception(e):
	print >> stderr, type(e).__name__ + ': ',
	print >> stderr, e
	_exit(255)



# Parse arguments
parser = argparse.ArgumentParser(description='Runs the specified command on the given host via SSH. Outputs stdout and stderr of executed command. Exit status is same as the command run, or 255 in case of connection error.')
parser.add_argument('hostname', metavar='HOSTNAME', type=str,
                   help='The host to connect to.')
parser.add_argument('cmd', metavar='COMMANDS', type=str,
                    nargs='+',#argparse.REMAINDER,
                    help='Commands to execute on remote host.')
parser.add_argument('--timeout', metavar='TIMEOUT', type=float,
                    default=10.0,help='Connection timeout.')
parser.add_argument('--username', metavar='USERNAME', type=str,
                    help='Username to connect as.')
parser.add_argument('--no-password', action='store_false',
                    default=True, help="Don't ask for password / SSH key passphrase")
parser.add_argument('--path', metavar='PATH',
                    help="Set remote current directory.")
args = parser.parse_args()


# Get password
if args.no_password:
	pw = getpass('SSH Key passphrase OR account password:')
else:
	pw = None

try:

	try:
		# Connect via SSH
		client = paramiko.SSHClient()
		client.set_missing_host_key_policy(paramiko.WarningPolicy())
		client.connect(args.hostname, username=args.username, password=pw, timeout=TIMEOUT)
	except TimeoutException:
		timed_out()
	finally:
		del pw


	try:
		# Setup communication and send command
		bufsize = -1

		args.path = quote(args.path)
		command = 'cd ' + args.path + ' && ' if args.path else ''
		command += ' '.join(args.cmd)

		chan = client.get_transport().open_session()
		chan.settimeout(TIMEOUT)
		chan.exec_command(command)

		ssh_stdin = chan.makefile('wb', bufsize)
		ssh_stdout = chan.makefile('rb', bufsize)
		ssh_stderr = chan.makefile_stderr('rb', bufsize)

		ssh_stdin.close()

		# Print stdout and stderr
		copyfileobj(ssh_stdout, stdout)
		copyfileobj(ssh_stderr, stdout)

		ssh_stdout.close()
		ssh_stderr.close()
	except TimeoutException:
		timed_out('Stream receive')
except SSHException, e:
	ssh_exception(e)


# Poll for exit status until timeout
countdown = TIMEOUT
while not chan.exit_status_ready() and countdown >= 0:
	start_time = clock()
	sleep(1)
	countdown -= max( 1.0 , clock() - start_time)

# Timeout
if countdown < 0:
	timed_out('Exit status receive')

# Return exit status
else:
	_exit(chan.recv_exit_status())

