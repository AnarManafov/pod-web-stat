#! /usr/bin/env bash

if [ -z "$1" ]; then
   echo "error: specify a working directory."
   exit 1
fi

eval output="$1/ip_to_country.txt"

# list of all IP addresses
IPs=( $(cat "$1/download.log" | awk '{print $1}' | sort -u) )

IPs_dir="$1/IPs"
mkdir -p $IPs_dir

rm $output

for i in "${IPs[@]}"
do
   # filter private IPs out
   a=$(echo $i | awk -F. '{print $1}')
   if (( $a == 10  )); then
      continue
   fi
   ip_file="$IPs_dir/$i"
   # don't process IP, which has been already processed
   if [ ! -r $ip_file ]; then
      whois $i > $ip_file
   fi
   
   country=( $(cat $ip_file | egrep -i country | awk '{gsub(/[[:space:]]*/,"",$2); print $2}')  )
   country_code=""
   if (( ${#country[*]} > 0  )); then
      let index=${#country[*]}-1
      country_code=${country[$index]}
   fi
   if [ -z "$country_code" ]; then
      # special cases, which where not found by whois
      # TODO: find a way to detect countries for all IPs
      # so far we handle them manually :(
      # TODO: Need to find a better way than "whois"
      ip_first_half=$(echo $i | awk -F'.' '{printf "%s.%s", $1, $2}')
      case "$ip_first_half" in
          "137.138" | "128.141" )
                          country_code="CH"
                          ;;
          "159.93")
                          country_code="RU"
                          ;;
      esac
    fi

    if [ -z "$country_code" ]; then

      case "$i" in
          "69.124.102.191" | "216.35.68.45" | "68.197.13.156")
	                  country_code="US"
			  ;;
          # South KOREA - KISTI
          "150.183.234.136")
                          country_code="KR"
                          ;;
          # CHINA
          "122.235.163.106")
                          country_code="CN"
                          ;;
          "142.90.84.255")
                          country_code="CA"
                          ;;
          "132.230.203.5" | "140.181.91.152")
                          country_code="DE"
                          ;;
          "192.84.128.42")
                          country_code="IT"
                          ;;
          "134.158.193.136")
                          country_code="FR"
                          ;;
          *)
	    country_code="???"
	    ;;
      esac
   fi
   echo -e "$i\t$country_code" >> $output
done

