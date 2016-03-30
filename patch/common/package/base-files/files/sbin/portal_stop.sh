
squid=`uci get portal.@${1}[0].squid`
accept_ip_list=`uci get portal.@${1}[0].accept`
deny_ip_list=`uci get portal.@${1}[0].deny`
mask=`uci get portal.@${1}[0].mask`
default_policy=`uci get portal.@${1}[0].default`
external_shell=`uci get portal.@${1}[0].exfile`

if [ "$squid" = "1" ]; then
	if [ "$mask" != "0" ]; then
		iptables -t nat -w -D PREROUTING ! -d 192.168.0.0/16 -i br-lan -p tcp -m tcp --dport 80 -m mark --mark $mask/$mask -j REDIRECT --to-ports 3128
	else
		iptables -t nat -w -D PREROUTING ! -d 192.168.0.0/16 -i br-lan -p tcp -m tcp --dport 80 -j REDIRECT --to-ports 3128
	fi
fi

iptables -t filter -w -D fil_fwd_global -m mark --mark $mask/$mask -j RETURN
iptables -t filter -w -D fil_fwd_global -m mark --mark $mask/$mask -j fil_fwd_$1

if [ -f $external_shell ]; then
chmod 777 $external_shell
$external_shell fil_fwd_$1 stop
fi

#accept from ip list.
for ip in $accept_ip_list
do
iptables -t filter -w -D fil_fwd_$1 -d $ip -j RETURN
done

#deny form ip list.
for ip in $deny_ip_list
do
iptables -t filter -w -D fil_fwd_$1 -d $ip -j DROP
done

if [ "$default_policy" = "deny" ]; then
iptables -t filter -w -A fil_fwd_$1 -j DROP
fi
if [ "$default_policy" = "accept" ]; then
iptables -t filter -w -A fil_fwd_$1 -j RETURN
fi

iptables -t filter -w -X fil_fwd_$1

