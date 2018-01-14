echo
echo "翻字典中, 請稍候....."
echo "【風與塵埃】英漢、漢英翻譯機" > /home/bbs/tmp/cdict.$USER
echo "" >> /home/bbs/tmp/cdict.$USER
echo "查詢單字: $WORD " >> /home/bbs/tmp/cdict.$USER
exec /home/bbs/bin/cdict5 $WORD >> /home/bbs/tmp/cdict.$USER
