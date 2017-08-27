#!/bin/sh

### BEGIN INIT INFO
# Provides:          helios
# Required-Start:    $ALL
# Required-Stop:     $ALL
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Start helios daemon at boot time
# Description:       Enable service provided by daemon.
### END INIT INFO

#. /lib/lsb/init-functions

dir="/var/packages/helios"
cmd="helios"
user="root"

name="helios"
pid_file="/var/packages/helios/$name.pid"
stdout_log="/var/packages/helios/$name.log"
stderr_log="/var/packages/helios/$name.err"

get_pid() {
    cat "$pid_file"
}

is_running() {
    [ -f "$pid_file" ] && ps -p `get_pid` > /dev/null 2>&1
}

case "$1" in
    start)
    if is_running; then
        echo "Already started"
    else
        echo "Starting $name"
	cd "$dir"
	sudo $cmd >> "$stdout_log" 2>> "$stderr_log" &
	echo $! | sudo tee "$pid_file"
        if ! is_running; then
            echo "Unable to start, see $stdout_log and $stderr_log"
            exit 1
        fi
    fi
    ;;
    stop)
    if is_running; then
        echo -n "Stopping $name.."
	sudo kill -2 `get_pid`
        for i in 1 2 3 4 5 6 7 8 9 10
        # for i in `seq 10`
        do
            if ! is_running; then
                break
            fi

            echo -n "."
            sleep 1
        done
        echo

        if is_running; then
            echo "Not stopped; may still be shutting down or shutdown may have failed"
            exit 1
        else
            echo "Stopped"
            if [ -f "$pid_file" ]; then
                sudo rm "$pid_file"
            fi
        fi
    else
        echo "Not running"
    fi
    ;;
    restart)
    $0 stop
    if is_running; then
        echo "Unable to stop, will not attempt to start"
        exit 1
    fi
    $0 start
    ;;
    status)
    if is_running; then
        echo "Running"
    else
        echo "Stopped"
        exit 0
    fi
    ;;
    logview)
    tail -f $stdout_log
    ;;
    *)
    echo "Usage: $0 {start|stop|restart|status|logview}"
    exit 1
    ;;
esac

exit 0
