#./boa -c /etc &
sleep 1
./cmd_server &
sleep 1
./up_server &

#/usr/sbin/avahi-autoipd --no-drop-root --no-chroot --force-bind -D eth0
#/usr/sbin/avahi-daemon --no-drop-root -D
#/usr/sbin/avahi-dnsconfd -D
touch /tmp/sem_start 

#ethtool -C eth0 tx-usecs 500
#ethtool -C eth0 rx-usecs 500
