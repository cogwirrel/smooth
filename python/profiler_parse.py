import sys
import re
from collections import defaultdict

# Prints useful statistics for each type of data collected
# Feel free to add to this, current only parses float data
def print_stats(data):
  for key, value in data.items():
    if (isinstance(value[0], float)):
      print "%s average is %.2f" %(key, (sum(value)/len(value)))
      print "   min is %.2f" %(min(value))
      print "   max is %.2f" %(max(value))

# Prints data out as latex table
def print_stats_latex(data):
  print "\\begin{tabular}{ l | l | l | l}"
  print "\hline"
  print "Test & Average & Min & Max \\\\"
  print "\hline\n\hline"
  for key, value in data.items():
    if (isinstance(value[0], float)):
      print "%s & %.2f & %.2f & %.2f \\\\" %(key.replace('_', '\\_'), sum(value)/len(value), min(value),
          max(value))
  print "\hline"
  print "\\end{tabular}"

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
  if len(sys.argv) == 3:
    print_stats_latex(process_file(sys.argv[2]))
  else:
    print_stats(process_file(sys.argv[1]))
