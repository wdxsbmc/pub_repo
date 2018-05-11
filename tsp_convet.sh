#!/bin/bash

tsppath=$PWD
index=0;
echo $tsppath;
#con= ./convertbmp2bin

rm bmp_res*.*
rm -rf ./out
#echo 100 >bmp_res_idx.head
myfunc()
{ 
    for x in $(ls) 
       do
                if [ -f "$x" ];then
                        echo "$x";
						convertbmp2bin "$x" "$PWD" "$tsppath"
                elif [ -L "$x" ];then
                        echo "this is a link";
                else
						echo "$x";
						cd "$x";
                        myfunc; 
                       cd ..
        fi 
				 done
}


myfunc



DATE=$(date +%Y%m%d)
echo "Resource sn:""$DATE"
echo $DATE>>bmp_res_table_sn.bin
DATA_ID="#define RESOURCE_SN                         "
data_flag="$DATA_ID""$DATE"
echo $data_flag>>bmp_res_table.head

cat table_flag.bin bmp_res_table_sn.bin >bmp_res_table_flag_sn.bin
cat bmp_res_table_flag_sn.bin utf8.bin >bmp_res_table_flag_sn_utf8.bin
cat bmp_res_table_flag_sn_utf8.bin en_96_24x16_zh1_b0a1_f7fe_24x24_6768.bin >bmp_res_table_flag_word.bin
cat bmp_res_table_flag_word.bin bmp_res.bin >bmp_res_table_flag_word_bmp.bin
cat bmp_res_table_flag_word_bmp.bin bmp_res_table_offset.bin >bmp_res_table_all.bin

flag_filesize=`wc -c < bmp_res_table_flag_sn.bin`
flag_filesize_t=`printf "#define TABLE_WORD_UTF8                     0x%08X"  $flag_filesize`
echo $flag_filesize_t>>bmp_res_table.head

utf8_filesize=`wc -c < bmp_res_table_flag_sn_utf8.bin`
utf8_filesize_t=`printf "#define TABLE_WORD                     0x%08X"  $utf8_filesize`
echo $utf8_filesize_t>>bmp_res_table.head

word_filesize=`wc -c < bmp_res_table_flag_word.bin`
word_filesize_t=`printf "#define TABLE_BMP_ZH                     0x%08X"  $word_filesize`
echo $word_filesize_t>>bmp_res_table.head

offset_filesize=`wc -c < bmp_res_table_flag_word_bmp.bin`
offset_filesize_t=`printf "#define TABLE_BMP_ZH_OFFSET                     0x%08X"  $offset_filesize`
echo $offset_filesize_t>>bmp_res_table.head
cat bmp_res_table.head bmp_res.head >bmp_res_tmp.head

mkdir out
cp bmp_res_table_all.bin ./out/res_table.bin
cp bmp_res_tmp.head ./out/res_table.head


rm bmp_res*.*

echo "######################Resource done!#############################"
