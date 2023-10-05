#!/bin/bash

STOREFILE="inject_prev.time"
BIN="/home/services/artefact/bin/barcodeextract"
HOST="server.dns.addr"
ARTEFACT="127.0.0.1"
ERRORDIR="/home/services/artefact/inject_errors"

now=`date +%s`
prev=`cat $STOREFILE`

echo "Running $BIN -s -t $prev -T $now -h $HOST -p 5432 -a $ARTEFACT -A 11108 -f $ERRORDIR"

$BIN -s -t $prev -T $now -h $HOST -p 5432 -a $ARTEFACT -A 11108 -f $ERRORDIR

if [ "$?" -eq 0 ]; then
  echo $now > $STOREFILE
fi
