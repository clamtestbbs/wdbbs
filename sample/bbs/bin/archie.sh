echo
echo "搜尋中, 請稍候....."
echo "【大地雕塑家】 ARCHIE Service." `date` > /home/bbs/tmp/archie.$USER
echo "" >> /home/bbs/tmp/archie.$USER
echo "Search for: $ARCHIESTRING " >> /home/bbs/tmp/archie.$USER
exec /usr/local/bin/archie -c $ARCHIESTRING >> /home/bbs/tmp/archie.$USER

