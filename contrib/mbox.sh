#!/bin/bash
#===============================================================================
#------ Written by Anton Filippov(sqrt@bluebootle.com) ------------------------- 
#------ Licensed under the same terms as Mailfilter, GPL v2 or later. ----------
#
#	This script mails headers of undeleted mails to $addrpre 
#	to preview mails and define new unwanted threads .
#	Simple view of List-ID , size and subject of each mail .
#
#	need verbosity=6
#
#===============================================================================
#	The algorithm:
#	grep nums of strings witn "Ok"
#	grep nums of strings with "Deleted"
#	grep nums of strings with "Quit"
#	for each nums "Ok" - find its corresponding end - in nums "Ok","Del" 
#	or "Quit"
#===============================================================================
p=$HOME/.mailfilter/denythreads
. $p/denythreads.conf
declare -i i in is ie ib

prevmail()	# $1-num of "Ok" line  , $2 - num of "del" or "quit" string , $3 -logfile
{
	is=$1-2
	sz=$(sed -n "$is"p $3|sed -e "s|.* ||g")
        sz=$(echo "scale=0; $sz / 1024"|bc)
        echo size=$sz K
		      
       ib=$1+1;ie=$2-3
       t1=$(sed -n "$ib,$ie"p $3)
       sub=$(echo "$t1"|egrep ^Subject:|sed -e "s|Subject: ||")
#      sub=$(echo "[${sz1}K]" "$sub")
       list=$(echo "$t1"|egrep ^List-Post|sed -e "s|List-Post: <mailto:||; s|@.*||;")
       echo "$t1"|mail -s "$sub" -t "${sz}K=$list <$addrpre>"
       echo "script \"${0##*/}\" sent this letter to $addrpre"
}

takelog()	# $1 - logfile
{
	#we don't need egrep (search from the beginning) - it is only headers
nok=$(grep -in "OK Message follows" $1|sed -e "s|:.*||g")
ndel=$(grep -in "mailfilter: Deleted" $1|sed -e "s|:.*||g")
		#logfile is new - so there is only one string "quit"
nq=$(grep -in "mailfilter: Sending QUIT" $1|sed -e "s|:.*||g")
n=$(echo "$nok"|sed -n "$ =")
for ((i=1;i <= n; i++))
do
	in=i+1
	si=$(echo "$nok"|sed -n "$i"p)
	arg1=$(echo "$nok"|sed -n "$in"p)
	arg2=$(echo "$ndel"|sed -n "1"p)
	if [[ -z "$arg1" ]];then
		if [[ -z "$arg2" ]];then
			arg3=$(echo "$nq")
			prevmail $si $arg3 $1;
		fi
	else
		if [[ $arg1  -lt $arg2 ]];then
			prevmail $si $arg1 $1
		else
			ndel=$(echo "$ndel"|sed -e "1 d")
		fi
	fi
done
}	

for f1 in $a;do
	dos2unix $f1.$log
	takelog $f1.$log
done

#EOF
#===============================================================================
