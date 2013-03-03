#!/usr/bin/python
import sqlite3
import argparse
from sys import stderr

# Maximum number of bytes to read from file
MAX_SIZE = 300000 # ~300 KB - more than enough!

# Name of database file
DB_FILE = 'experiments.db'

# Parse arguments
parser = argparse.ArgumentParser(description='Adds a data point to the database. If successful, writes the data id to stdout.')
parser.add_argument('exp_id', metavar='EXP-ID', type=int,
                   help='The ID of the associated experiment.')
parser.add_argument('stdout', metavar='STDOUT', type=str,
                   help='A file containing the produced standard output.')
parser.add_argument('stderr', metavar='STDERR', type=str,
                   help='A file containing the produced standard error.')

args = parser.parse_args()


# fileToString: read file into string
def fileToString(filename):
	with open(filename, 'r') as f:
		text = f.read(MAX_SIZE)
		if len(text) >= MAX_SIZE:
			print >> stderr, "Warning: '%s' larger than %d bytes. Data beyond that was NOT written to database." (filename, MAX_SIZE)

	return text

# read stdout and stderr files as text
[stdout_txt, stderr_txt] = map(fileToString, [args.stdout, args.stderr])


# perform DB insertion
with sqlite3.connect(DB_FILE) as conn:
	# enable foreign key enforcement...
	conn.execute('PRAGMA foreign_keys = ON;')
	try:
		cur = conn.execute('insert into data (experimentid, stdout, stderr) values (?,?,?)', (args.exp_id, stdout_txt, stderr_txt))
	except sqlite3.IntegrityError:
		print >> stderr, "Experiment %d non-existant" %args.exp_id
		exit(1)

	print cur.lastrowid
