#!/bin/bash

logfile=$2
locreq=$1

# unixtime to datetime
#date -d @1328865527 +%Y%m%d%H%M


# grep connections
#arrin=`grep -A5 "<pentominos:data" /var/log/artefact.log | xargs | awk 'BEGIN{RS="--"}{print $0}'`
#arrin=`grep -A5 "<pentominos:data" /var/log/artefact.log`
arrin=`grep -A5 "<pentominos:data" $logfile | xargs`
#printf $arrin
#arr=$( echo $arrin | tr "--" "\n")
#for elem in $arrayu[*]}

timestamp=;
device=;
location=;

for elem in $arrin
do
#  printf "element: $elem\n"
#  elem=`echo elem | sed 's/^ *//g'`
  if [ "$elem" = "--" ] ; then
    if [ "$locreq" = "$location" ] ; then
      c=${#hits[@]}
#      day=`date -d @$timestamp +%x`
#      time=`date -d $timestamp +%X`
      hits[$c]="$timestamp:$device"
    fi 
  else 
#  command=`echo $elem | sed 's/^
   tuple=(`echo $elem | tr '=' '\n'`)
#    tuple=({$elem//=/ })
#    tuple=`echo $elem | sed -e 's/=/\n/g'`
    
#    tuple=`echo $elem` 
#    tuple=`echo $echo | xargs | awk 'BEGIN{RS=" "}{print $0}'` 
#    printf `echo $(echo $elem | tr "=" "\n") `
    cmd=${tuple[0]}
    value=${tuple[1]}
#    printf "/cmd:${cmd}:cmd/\n"
#    printf "/val:${value}:val/\n"
    if [ "$cmd" = "location" ] ; then
#      printf "updating location\n"
      location=${tuple[1]}
    elif [ "$cmd" = "device_type" ] ; then
#      printf "updating device\n"
      device=${tuple[1]}
    elif [ "$cmd" = "timestamp" ] ; then
#      printf "updating timestamp\n"
#      timestamp=`date -d @${tuple[1]} +%Y%m%d%H%M`
      timestamp=${tuple[1]}
    fi
  fi

#  printf "Next elem:\n"
#  printf "  %s\n" $elem
done


elem=;
day=;
hits=$(echo ${hits[@]} | sort)
c=;
for celem in $hits
do
  if [ "$celem" != "$elem" ] ; then
#  printf "elem: $elem\n"
#  tuple=(`echo $elem | tr ' ' '\n'`)
  tuple=(`echo $celem | tr ':' '\n'`)
#  tuple=(`echo $elem`)
#printf ${tuple[0]}
#  printf "\n"
  timestamp=${tuple[0]}
  device=${tuple[1]}
  cday=`date -d @$timestamp +'%a %b %Y'`
  ctime=`date -d @$timestamp +%X`
  if [ "$cday" != "$day" ] ; then
    printf "\t#=$c\n"
    c=0
    day=$cday
    printf "$cday:\n"
  fi
  printf "\t$ctime: $device\n"
  c=`echo $(($c+1))`
  fi
  elem=$celem
done
printf "\t#=$c\n"
