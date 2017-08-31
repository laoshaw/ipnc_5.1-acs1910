#./boa -c /etc &
#/usr/sbin/avahi-autoipd --no-drop-root --no-chroot --force-bind -D eth0
#/usr/sbin/avahi-daemon --no-drop-root -D
#/usr/sbin/avahi-dnsconfd -D
#touch /tmp/sem_start 
#sleep 1
#./system_server &
#sleep 1
#./boot_proc 1
#sleep 1
#./cmd_server &

#ethtool -C eth0 tx-usecs 500
#ethtool -C eth0 rx-usecs 500
