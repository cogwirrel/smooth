import sys

def process_file(path):
  dataFile = open(path);
  count = 0.0;
  summation = 0.0;
  for line in dataFile:
    split = line.split()
    if (split[0] == 'BENCHMARK:'):
      #Remove seconds from split[1] i.e. 2.444s
      summation += float(split[1][0:-1])
      count += 1

  return 0 if (count == 0) else summation / count

if __name__ == '__main__':
  path = sys.argv[1]
  print process_file(path)
