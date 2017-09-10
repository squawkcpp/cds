#! /bin/bash -e

PATH=/bin:/usr/bin:/sbin:/usr/sbin

/usr/local/bin/cds $CDS_OPTS

tail -f /var/log/dmesg
