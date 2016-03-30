if [ "$1" = "start" ]
iptables -t filter -w -N fil_fwd_global
iptables -t filter -w -N fil_fwd_visitor
iptables -t filter -w -I FORWARD -i br-lan -j fil_fwd_global
iptables -t filter -w -A fil_fwd_global -j fil_fwd_visitor
iptables -t filter -w -A fil_fwd_visitor -j DROP
fi

if [ "$1" = "stop" ]
iptables -t filter -w -D fil_fwd_visitor -j DROP
iptables -t filter -w -D fil_fwd_global -j fil_fwd_visitor
iptables -t filter -w -D FORWARD -i br-lan -j fil_fwd_global
iptables -t filter -w -X fil_fwd_visitor
iptables -t filter -w -X fil_fwd_global
fi


