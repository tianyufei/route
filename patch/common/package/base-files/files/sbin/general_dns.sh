
if [ "$1" = "add" ]
then
iptables -t filter -I fil_fwd_generic -w -d $2 -j RETURN
fi

if [ "$1" = "del" ]
then
iptables -t filter -D fil_fwd_generic -w -d $2 -j RETURN
fi


