import sys
import re
from collections import defaultdict

# Prints useful statistics for each type of data collected
# Feel free to add to this, current only parses float data
def print_stats(data):
  for key, value in data.items():
    if (isinstance(value[0], float)):
      print "%s average is %f.2" %(key, (sum(value)/len(value)))
      print "   min is %f.2" %(min(value))
      print "   max is %f.2" %(max(value))


# Processes data file
# Parses lines in file that look liket
# method=[ smooth ] gputime=[ 149.824 ] cputime=[ 165.000 ] occupancy=[ 0.031 ]
# divergent_branch=[ 8 ]
# In this case will create dictionary
# data = {gputime: [149.824], cputime: [165.000], occupancy [0.031],
# divergent_branch: [8]}
# Obviously for multiple lines it'll append to the lists
# I expect all values to be floats, but if not appends the string representation
def process_file(path):
  dataFile = open(path);

  #Maps counter to a list of values
  data = defaultdict(list)
  regex = re.compile("((?P<type>\w+)=\[ (?P<value>[\w0-9\.]+) \])[\b]?")
  for line in dataFile:
    for match in regex.finditer(line):
      m_type = match.group('type')
      m_value = match.group('value')
      if (m_type == 'method' and m_value != 'smooth'):
        break
      if (m_type != 'method'):
        try:
          data[m_type].append(float(m_value))
        except ValueError:
          data[m_type].append(m_value)

  return data

if __name__ == '__main__':
  path = sys.argv[1]
  data = process_file(path)
  print_stats(data)
