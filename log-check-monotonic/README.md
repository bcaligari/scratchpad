# log-check-monotonic

Disposable script to check log entries for jumps back in time.

Specifically written for `/var/log/pacemaker.log/`.

```{text}
usage: chkmonotinic [-h] [-l] [-d D] logfile [logfile ...]

Check log entries for time travel

positional arguments:
  logfile     a log file

optional arguments:
  -h, --help  show this help message and exit
  -l          list filename only
  -d D        minimum seconds delta (def = 300)
```