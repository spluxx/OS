#!/bin/bash

# script directory
SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

if [ "$#" -ne 6 ]; then
    echo "usage: startservers.sh <num servers> <rmi port> <num servers to fail> <time to fail servers (seconds)> <time for servers to restart (seconds)> <time to simulate system (seconds)>"
    exit;
fi

NUM_SERVERS="$1"
echo "Simulating $NUM_SERVERS servers"
RMI_PORT="$2"
NUM_TO_FAIL="$3"
echo "Failing $NUM_TO_FAIL servers at a time"
TIME_TO_FAIL="$4"
echo "Waiting $TIME_TO_FAIL seconds between failiures"
TIME_TO_RESTART="$5"
echo "Failed servers restart after $TIME_TO_RESTART seconds"
TIME_TO_SIMULATE="$6"
echo "Simulation will last $TIME_TO_SIMULATE seconds"

echo "Restarting rmiregistry"

RMI_PID=`$SCRIPT_DIR/pidof rmiregistry`

if [ -n "$RMI_PID" ]; then
    echo "Killing old rmiregistry"
    kill -9 "$RMI_PID" 2> /dev/null
fi

cd "$SCRIPT_DIR"
rmiregistry "$RMI_PORT" &


declare -a SERVER_PIDS

for (( id=1; id<=$NUM_SERVERS; id++ ))
do
    java -classpath "$SCRIPT_DIR" -Djava.rmi.server.codebase=file://localhost/$SCRIPT_DIR/ edu.duke.raft.StartServer 1098 $id &
    PID="$!"
    SERVER_PIDS[$id]="$PID"
    echo "Starting server S$id at $PID"
done

START=`date +%s`
while [ $(( $(date +%s) - $TIME_TO_SIMULATE )) -lt $START ]; do
    echo "Going to sleep"
    sleep $TIME_TO_FAIL

    for (( failures=0; failures<$NUM_TO_FAIL; failures++ ))
    do
	id=$RANDOM
	let "id %= $NUM_SERVERS"
	let "id += 1"

	echo "Failing S$id"
	kill -9 ${SERVER_PIDS[$id]} 2> /dev/null
	SERVER_PIDS[$id]=""
    done

    sleep $TIME_TO_RESTART

    for (( id=1; id<=$NUM_SERVERS; id++ ))
    do
	if [ -z "${SERVER_PIDS[$id]}" ]; then
	    java -classpath "$SCRIPT_DIR" -Djava.rmi.server.codebase=file://localhost/$SCRIPT_DIR/ edu.duke.raft.StartServer 1098 $id &
	    PID="$!"
	    SERVER_PIDS[$id]="$PID"
	    echo "Restarted server S$id at $PID"
	fi
    done
done

echo "Shutting down simulation"

for (( id=1; id<=$NUM_SERVERS; id++ ))
do
    echo "Shutting down server S$id"
    kill -9 ${SERVER_PIDS[$id]} 2> /dev/null
done
