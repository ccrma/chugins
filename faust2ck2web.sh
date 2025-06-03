TMP=/tmp
grep declare\ name $1
csplit -s $1 2
# xx00 has first line 
# xx01 has the rest
mv xx00 $TMP/
mv xx01 $TMP/
cat $TMP/xx00 | awk '{print $1}' >> $TMP/tmp 
cat $TMP/xx00 | awk '{print $2}' >> $TMP/tmp 
cat $TMP/xx00 | awk '{print $3}' | sed -e "s/\b\(.\)/\u\1/g" >> $TMP/tmp

cat $TMP/xx00 | awk '{print $3}' | sed -e "s/\b\(.\)/\u\1/g" > $TMP/NAME
sed -i "s/[^[:alnum:]-]//g" $TMP/NAME
NAME=$(< $TMP/NAME)
echo "chugin project name = $NAME"
rm $TMP/NAME

(readarray -t ARRAY < $TMP/tmp; IFS=' '; echo "${ARRAY[*]}") > $TMP/tmp2
rm $TMP/tmp
cat $TMP/tmp2
#sed -i -z 's/$/\n/g' $TMP/tmp2
cat $TMP/xx01 >> $TMP/tmp2
diff $1 $TMP/tmp2
mv $TMP/tmp2 $1 

rm -r $NAME
if test -d $NAME; then
  echo "Directory exists."
  echo "exiting"
  exit
else
  chuginate/chuginate $NAME
fi

cd $NAME

cp ../$1 ${NAME}F.dsp
faust2ck ${NAME}F.dsp
cp ${NAME}F.dsp.cpp ${NAME}.cpp

make web

echo "produced ${NAME}.chug.wasm with parameters:"
grep arg\(\ QUERY *wrapper*
