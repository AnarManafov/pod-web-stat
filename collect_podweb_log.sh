#! /usr/bin/env bash

WORKDIR=$(readlink -f $0)
WORKDIR=$(dirname $WORKDIR)
LOG_TMP="$WORKDIR/log.tmp"
# merged log - the history with only unique entries
LOG="$WORKDIR/log.final"
# filtered log - only PoD downloads
DOWNLOADS="$WORKDIR/download.log"
DOWNLOADS_BUP="$WORKDIR/download_bup.log"
IP_COUNTRY_FILE="$WORKDIR/ip_to_country.txt"
# get the log file from the server
AUTH=$(cat ~/.auth_podwww)

curl -s -u $AUTH http://pod.gsi.de/access.log >> $LOG_TMP

sort -u $LOG_TMP > $LOG

cp $LOG $LOG_TMP

# all PoD downloads
grep "PoD-" $LOG | grep "GET " > $DOWNLOADS

if [ -r $DOWNLOADS_BUP ]; then
   d=$(diff $DOWNLOADS $DOWNLOADS_BUP 2>/dev/null)
   if [ -z "$d" ]; then
      echo "*** There are no new downloads ***"
   else
      echo "new downloads:"
      echo "$d"
   fi
fi
cp $DOWNLOADS $DOWNLOADS_BUP

# updated PoDWebsite with the new statistics
pushd $(pwd)
if [ -d "$WORKDIR/PoDWebsite" ]; then
   pushd $(pwd)
   cd $WORKDIR/PoDWebsite
   git pull
   popd
else
   git clone ssh://anar@demac012.gsi.de//Users/anar/GitRepository/PROOFonDemand/PoDWebsite
fi

$WORKDIR/collect_country.sh $WORKDIR

$WORKDIR/pod-web-stat --logfile $DOWNLOADS --country $IP_COUNTRY_FILE --figures_pat $WORKDIR/PoDWebsite/graphics  --docbook 1> $WORKDIR/PoDWebsite/download_stat.xml

pushd $(pwd)
cd $WORKDIR/PoDWebsite

git commit -m "updated download stats" download_stat.xml
git push

popd
