#!/bin/sh

# Script to manage the aesdsocket daemon
DAEMON=/usr/bin/aesdsocket
DAEMON_NAME=aesdsocket
DAEMON_OPTS="-d"
PIDFILE=/var/run/$DAEMON_NAME.pid

case "$1" in
    start)
        echo "Starting $DAEMON_NAME"
        start-stop-daemon --start --background --make-pidfile --pidfile $PIDFILE --exec $DAEMON -- $DAEMON_OPTS
        ;;
    stop)
        echo "Stopping $DAEMON_NAME"
        start-stop-daemon --stop --pidfile $PIDFILE --signal SIGTERM
        ;;
    restart)
        $0 stop
        sleep 1
        $0 start
        ;;
    status)
        status_of_proc -p $PIDFILE $DAEMON $DAEMON_NAME && exit 0 || exit $?
        ;;
    *)
        echo "Usage: $0 {start|stop|restart|status}"
        exit 1
        ;;
esac

exit 0
