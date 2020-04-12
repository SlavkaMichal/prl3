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
    parser.add_argument('integers', metavar='N', type=int, nargs='+')

    argv = parser.parse_args()

    altitudes = argv.integers

    angles = [math.atan((x - altitudes[0]) / float(i))
            for i, x in enumerate(altitudes, 1)]
    print("angles")
    for a in angles:
        print(a)

    max_prescan = [-math.pi] + list(itertools.accumulate(angles[1:], max))
    max_prescan.pop()
    print("prescan")
    for a in max_prescan:
        print(a)
    visibilities = ["v" if (angle > max_prev_angle) else "u"
                    for (angle, max_prev_angle) in zip(angles[1:], max_prescan)]

    ref_output = ",".join(visibilities)
    print(ref_output)


if __name__ == "__main__":
    main()
