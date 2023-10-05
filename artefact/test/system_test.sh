#!/bin/bash
echo "Performing system test"

BASEDIR=$PWD/system_test
PID_FILE=$BASEDIR/artefact.pid
CONF_FILE=$BASEDIR/artefact.conf
LUA_DIR=$BASEDIR/lua
LOG_FILE=$BASEDIR/artefact.log
TESTDIR=$BASEDIR/tests
ERR_CNT=$BASEDIR/error_cnt

PORT=11108
HOST=localhost

echo "0" > $ERR_CNT

# Initialise empty database
PGPASSWORD=artefact psql -U artefact -d artefact < init.psql

rm $BASEDIR/data -Rf
mkdir -p $BASEDIR/data

cat ${CONF_FILE}.in | sed 's|DATAPATH|'$BASEDIR/data'|g' > $CONF_FILE

chmod +x install/bin/artefact
install/bin/artefact -c $CONF_FILE -l$LUA_DIR -D+all -L$LOG_FILE -P$PID_FILE

sleep 2

echo "Server started"

pushd $TESTDIR

ls -1 *.test | sort | while read TEST
do
    INPUT=""
    EXPECT=""
    TITLE=""

    source ./$TEST

    echo -n "$TITLE ($TEST) - "

    RESULT=`basename $TEST .test`_result.xml

    curl -s -d @$INPUT $HOST:$PORT | sed 's/timestamp=".*"/timestamp=""/g' | $FILTER > $RESULT

    diff $RESULT $EXPECT

    if [ "$?" != "0" ]
    then
	echo "FAILED"
	echo "**** Expected: ****"
	cat $EXPECT

	echo "**** Got: ****"
	cat $RESULT
  
	echo $(($(cat $ERR_CNT)+1)) > $ERR_CNT
    else
	echo "SUCCESS"
    fi

done

popd

PID=`cat $PID_FILE`
kill -INT $PID

echo "Server stop signalled (INT)"

sleep 2

# This is only nessecary on server freeze error...
ps -p $PID > /dev/null 2>&1
if [ "$?" == "0" ]
then
    kill -9 $PID
    echo "Server forcefully stopped (-9)"
fi

# Wipe database
PGPASSWORD=artefact psql -U artefact -d artefact < cleanup.psql

FAIL_CNT=$(cat $ERR_CNT)

if [ "$FAIL_CNT" -gt 0 ]
then
    echo "$FAIL_CNT failed tests."
    false
else
    echo "All test successfully completed."
    true
fi