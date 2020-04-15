from subprocess import Popen, PIPE, run
from argparse import ArgumentParser
import math
import random
import itertools

FAIL = '\033[91m\033[1m'
GREEN = '\033[32m\033[1m'
ENDC = '\033[0m'
BOLD = '\033[1m'


def main():
    parser = ArgumentParser()
    parser.add_argument('integers', metavar='N', type=int, nargs='*')
    parser.add_argument('-i', '--input',dest='input', type=str)

    argv = parser.parse_args()

    if argv.integers == [] and argv.input != None:
        #print(argv.input)
        #print(argv.input.split(','))

        altitudes = list(map(int, argv.input.split(',')))
    elif argv.integers != []:
        altitudes = argv.integers
    else:
        print("No arguments passed!")
        return



    angles = [math.atan((x - altitudes[0]) / float(i))
            for i, x in enumerate(altitudes[1:], 1)]
    #print("angles")
    #for a in angles:
    #    print(a)

    max_prescan = [-math.pi] + list(itertools.accumulate(angles, max))
    max_prescan.pop()
    #print("prescan")
    #for a in max_prescan:
    #    print(a)
    visibilities = ["v" if (angle > max_prev_angle) else "u"
                    for (angle, max_prev_angle) in zip(angles, max_prescan)]

    ref_output = "_,"+",".join(visibilities)
    print(ref_output)


if __name__ == "__main__":
    main()
