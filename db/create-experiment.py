#!/usr/bin/python
import sqlite3
import argparse

# Name of database file
DB_FILE = 'example.db'

# Parse arguments
parser = argparse.ArgumentParser(description='Create an experiment in the database.')
parser.add_argument('sha', metavar='SHA', type=str,
                   help='The SHA of the associated git commit')
parser.add_argument('desc', metavar='DESC', type=str,
                   help='A description of the experiment')

args = parser.parse_args()


# perform DB insertion
with sqlite3.connect(DB_FILE) as conn:
	print args
	conn.execute('insert into experiment (sha, description) values (?,?)', (args.sha, args.desc))
