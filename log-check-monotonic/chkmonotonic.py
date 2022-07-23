#!/usr/bin/env python3
#
# One time use bodge to track jumps back in time in log files,
# more specifically the pacemaker log.
#
#   License: GNU Affero General Public License
#       https://www.gnu.org/licenses/agpl-3.0.en.html
#
import re
import argparse


def get_time_string(logstr):
    m = re.match("^.*?((?<=\D)\d{1,2})?\D?(\d\d):(\d\d):(\d\d)\D", logstr[:32])
    if m:
        return (int(m[1]) if m[1] else None,
            (((int(m[2]) * 60) + int(m[3])) * 60) + int(m[4]))
    else:
        return (None, None)


def loganal(filename, name_only, sec_delta):
    line_number = 0
    (prev, cur) = ((None, None), (None, None))
    with open(filename) as f:
        for line in f.readlines():
            line_number += 1
            cur = get_time_string(line)
            if cur[1]:
                if prev[1]:
                    if ((prev[1] - cur[1]) > sec_delta) and (cur[0] == prev[0]):
                        if name_only:
                            print(f"{filename}")
                            break
                        print(f"{filename}: {line_number} ({prev[1] - cur[1]})")
                prev = cur


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="chkmonotinic",
        description="Check log entries for time travel",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("-l", action='store_true', help="list filename only")
    parser.add_argument("-d", type=int, default=300, help="minimum seconds delta (def = 300)")
    parser.add_argument("logfile", nargs='+', help="a log file")
    args = parser.parse_args()
    for filename in args.logfile:
        loganal(filename, args.l, args.d)