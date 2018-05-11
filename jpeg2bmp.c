//#include <iostream>
#include <stdio.h>
#include "jpeglib.h"
#include <stddef.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


//#pragma comment(lib,"jpeg.lib")
//using namespace std;

#pragma pack(2)        //两字节对齐，否则bmp_fileheader会占16Byte
struct bmp_fileheader
{
    unsigned short    bfType;        //若不对齐，这个会占4Byte
    unsigned long    bfSize;
    unsigned short    bfReverved1;
    unsigned short    bfReverved2;
    unsigned long    bfOffBits;
};

struct bmp_infoheader
{
    unsigned long    biSize;
    unsigned long    biWidth;
    unsigned long    biHeight;
    unsigned short    biPlanes;
    unsigned short    biBitCount;
    unsigned long    biCompression;
    unsigned long    biSizeImage;
    unsigned long    biXPelsPerMeter;
    unsigned long    biYpelsPerMeter;
    unsigned long    biClrUsed;
    unsigned long    biClrImportant;
};


typedef struct tagRGBQUAD
{
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;

}RGBQUAD;

struct bmp_res
{
    unsigned short    idx;
};


FILE *input_file;
FILE *output_file;
FILE *head_file;
FILE *table_head_file;
FILE *table_offset_file;
FILE *img_idx_file;


char ID_STR[100];
char ID_NEW_STR[100];
char PATH_STR[20];
char NAME_STR[50];


void update_table_idx(long address)
{
    unsigned char table_idx[4];

	char path_str[100];
	memset(path_str,0,sizeof(path_str));

	strcat(path_str,PATH_STR);
	strcat(path_str,"/bmp_res_table_offset.bin");
		
	table_offset_file = fopen(path_str,"a+");
	
	if (table_offset_file==NULL)  
	{   
		printf("open bmp_res_table_idx.h fail!\n");
		return;
	} 


 	fwrite(&address,sizeof(address),1,table_offset_file);

	
	fclose(table_offset_file);			
}

void  update_table_head()
{
	char path_str[100];
	memset(path_str,0,sizeof(path_str));

	strcat(path_str,PATH_STR);
	strcat(path_str,"/bmp_res_table.head");
		
	table_head_file = fopen(path_str,"w+");
	
	if (table_head_file==NULL)  
	{   
		printf("open res_table.h fail!\n");
		return;
	} 

	fseek(output_file,0,SEEK_END);
	long pos = ftell(output_file);
	char pos_str[10];
	
	memset(pos_str,0,sizeof(pos_str));
	sprintf(pos_str,"0x%08X",pos);

	
	char res_str[200];
	memset(res_str,0,sizeof(res_str));
	strcpy(res_str,"#define      TABLE_OFFSET			");
	
	strcat(res_str, pos_str);

	
	fwrite("\n",2,1,table_head_file);
	fwrite(res_str,sizeof(res_str),1,table_head_file);
	fwrite("\n",2,1,table_head_file);

	
	fclose(table_head_file);
	
}



void  resid2h(long pos,char* address,char* filename)
{

	int i;
	int idx;
    struct bmp_res bres;

	
	char path_str[100];

	//open bmp_res.head
	memset(path_str,0,sizeof(path_str));
	strcat(path_str,PATH_STR);
	strcat(path_str,"/bmp_res.head");		
	head_file = fopen(path_str,"a+");	
	if (head_file==NULL)  
	{   
		printf("open res.h fail!\n");
		return;
	} 



	//fix image resource id
	strcat(ID_NEW_STR,"_");
	strcat(ID_NEW_STR,NAME_STR);
	strcat(ID_NEW_STR,"					");
	for (i=0; i<sizeof(ID_NEW_STR); i++) 
	{ 
	  ID_NEW_STR[i] = toupper(ID_NEW_STR[i]); 
	}

	for (i=0; i<sizeof(ID_NEW_STR); i++) 
	{ 
		if(ID_NEW_STR[i] == '/')
			ID_NEW_STR[i] = '_';
	}
	
	for (i=0; i<sizeof(ID_NEW_STR); i++) 
	{ 
		if(ID_NEW_STR[i] == '-')
			ID_NEW_STR[i] = '_';
	}


	//fix index
	//rcat(ID_NEW_STR, address);
	int res_idx;
	char res_idx_buf[4];
	char res_index_str[10];
	memset(res_index_str,0,sizeof(res_index_str));


	//open bmp_res_idx.head
	memset(path_str,0,sizeof(path_str));
	strcat(path_str,PATH_STR);
	strcat(path_str,"/bmp_res_idx.head");		
	img_idx_file = fopen(path_str,"a+");	
	if (img_idx_file==NULL)  
	{   
		printf("open bmp_res_idx.head fail!\n");
		return;
	} 
	
	
	
	fread(&bres,sizeof(bres),1,img_idx_file); 
	if(&bres == NULL)
		{
			printf("bres is NULL\n");
			bres.idx = 0;
		}
	else
		{
			printf("bres.idx  =%d ",bres.idx );
			
		}

	fclose(img_idx_file);
	

	sprintf(res_index_str,"%d",bres.idx);
	strcat(ID_NEW_STR, res_index_str);
	bres.idx ++;

	//udpate img_idx_file
	img_idx_file = fopen(path_str,"w+");	
	fwrite(&bres,sizeof(bres),1,img_idx_file);
	fclose(img_idx_file);

	
	

	//wirte .h
	//path+filename+offset/index
	char res_str[200];
	memset(res_str,0,sizeof(res_str));
	strcpy(res_str,"#define         RES_ID");
	strcat(res_str,ID_NEW_STR);


	
	fwrite("\n",2,1,head_file);
	fwrite(res_str,sizeof(res_str),1,head_file);
	fwrite("\n",2,1,head_file);

	
	fclose(head_file);	
	


	update_table_head();
	update_table_idx(pos);
}

void write_bmp_header(j_decompress_ptr cinfo)
{
    struct bmp_fileheader bfh;
    struct bmp_infoheader bih;

    unsigned long width;
    unsigned long height;
    unsigned short depth;
    unsigned long headersize;
    unsigned long filesize;

    width=cinfo->output_width;
    height=cinfo->output_height;
    depth=cinfo->output_components;

	printf("depth =%d \n",depth);
    if (depth==1)
    {
    	//1byte/pix, ==>256color==>   4bytes/color (rgb)
        headersize=14+40+256*4; //colortable for 256
        filesize=headersize+width*height;
    }

    if (depth==3)
    {
    	//3bytes/pix==> 24bit color==> 3bytes/pix(no colortable)
        headersize=14+40;
        filesize=headersize+width*height*depth;
    }

    memset(&bfh,0,sizeof(struct bmp_fileheader));
    memset(&bih,0,sizeof(struct bmp_infoheader));
    
    //写入比较关键的几个bmp头参数
    bfh.bfType=0x4D42;
    bfh.bfSize=filesize;
    bfh.bfOffBits=headersize; //65feh = 25854

    bih.biSize=40;  //28h
    bih.biWidth=width;
    bih.biHeight=height;
    bih.biPlanes=1;
    bih.biBitCount=(unsigned short)depth*8;
    //bih.biBitCount=1;
    bih.biSizeImage=width*height*depth;

    fwrite(&bfh,sizeof(struct bmp_fileheader),1,output_file);
    fwrite(&bih,sizeof(struct bmp_infoheader),1,output_file);

    if (depth==1)        //灰度图像要添加调色板
    {
        unsigned char platte[256*4];
        unsigned char j=0;
		int i;
		for (i=0;i<1024;i+=4)
        {
            platte[i]=j;
            platte[i+1]=j;
            platte[i+2]=j;
            platte[i+3]=0;
            j++;
        }
        fwrite(platte,sizeof(unsigned char)*1024,1,output_file);
    }
}


void write_bmp_header_single(j_decompress_ptr cinfo)
{
    struct bmp_fileheader bfh;
    struct bmp_infoheader bih;

    unsigned long width;
    unsigned long height;
    unsigned short depth;
    unsigned long headersize;
    unsigned long filesize;
	unsigned long linebyte;

    width=cinfo->output_width;
    height=cinfo->output_height;
    depth=cinfo->output_components;
	linebyte = width*1/8 + 3; //28byte
	

	printf("depth =%d width=%d height=%d\n",depth,width,height);

    if (depth==1)
    {
    	//1bit/pix, ==>
        headersize=14+40+2*4; //colortable for black&white
        filesize=headersize+linebyte*height;
    }

    memset(&bfh,0,sizeof(struct bmp_fileheader));
    memset(&bih,0,sizeof(struct bmp_infoheader));
    
    //写入比较关键的几个bmp头参数
    bfh.bfType=0x4D42;
    bfh.bfSize=filesize;
    bfh.bfOffBits=headersize; //65feh = 25854

    bih.biSize=40;  //28h
    bih.biWidth=width;
    bih.biHeight=height;
    bih.biPlanes=1;
    bih.biBitCount=1;
    bih.biSizeImage=width*height*depth;

    fwrite(&bfh,sizeof(struct bmp_fileheader),1,output_file);
    fwrite(&bih,sizeof(struct bmp_infoheader),1,output_file);

    if (depth==1)        //灰度图像要添加调色板
    {
        unsigned char platte[2*4];
		platte[0] = 0x00;
		platte[1] = 0x00;
		platte[2] = 0x00;
		platte[3] = 0x00;
		platte[4] = 0xFF;
		platte[5] = 0xFF;
		platte[6] = 0xFF;
		platte[7] = 0x00;
        fwrite(platte,sizeof(unsigned char)*8,1,output_file);
    }
}


void write_bmp_data(j_decompress_ptr cinfo,unsigned char *src_buff)
{
//    unsigned char *dst_width_buff;
    unsigned char *point;

    unsigned long width;
    unsigned long height;
    unsigned short depth;

    width=cinfo->output_width;
    height=cinfo->output_height;
    depth=cinfo->output_components;

    unsigned char dst_width_buff[width*depth];
    memset(dst_width_buff,0,sizeof(unsigned char)*width*depth);

    point=src_buff+width*depth*(height-1);    //倒着写数据，bmp格式是倒的，jpg是正的
	unsigned long i,j;
    for (i=0;i<height;i++)
    {
        for (j=0;j<width*depth;j+=depth)
        {
            if (depth==1)        //处理灰度图
            {
                dst_width_buff[j]=point[j];
            }

            if (depth==3)        //处理彩色图
            {
                dst_width_buff[j+2]=point[j+0];
                dst_width_buff[j+1]=point[j+1];
                dst_width_buff[j+0]=point[j+2];
            }
        }
        point-=width*depth;
        fwrite(dst_width_buff,sizeof(unsigned char)*width*depth,1,output_file);    //一次写一行
    }
}


void write_bmp_data_single(j_decompress_ptr cinfo,unsigned char *src_buff,char* filename)
{
//    unsigned char *dst_width_buff;
    unsigned char *buf;

    unsigned long width;
    unsigned long height;
    unsigned short depth;
	unsigned short linebyte;
    width=cinfo->output_width;
    height=cinfo->output_height;
    depth=cinfo->output_components;

	//linebyte = width*1/8 + 3; //28byte
	linebyte=(width * depth/8+3)/4*4; //4BYTES align	
	
    unsigned char dst_width_buff[linebyte * height];
	unsigned short  byteidx = 0;
	unsigned short  bytepos = 0;
	unsigned long i,j,k;

	fseek(output_file,0,SEEK_END);
	long pos = ftell(output_file);
	char pos_str[10];
	memset(pos_str,0,sizeof(pos_str));
	sprintf(pos_str,"0x%08X",pos);
	printf("%s ",pos_str);


	fwrite(&width, sizeof(width), 1, output_file);
	fwrite(&height, sizeof(height), 1, output_file);		

    buf=src_buff+width*depth*(height-1);    //倒着写数据，bmp格式是倒的，jpg是正的
    for (i=0;i<height;i++)
    {
    	memset(dst_width_buff,0,sizeof(unsigned char)*height*linebyte);
        for (j=0;j<width*depth;j+=depth)
        {
            //convet to bit
			byteidx = j/8;
			bytepos = j%8;
			
        	if(buf[j] > 0x7f)
        	{
           		dst_width_buff[byteidx]=dst_width_buff[byteidx]|(0x80 >> bytepos);	
        	}	
				
        }
		
        buf-=width*depth;
		
        fwrite(dst_width_buff,sizeof(unsigned char)*linebyte,1,output_file);    //一次写一行
    }

	//wirte .h

	resid2h(pos,pos_str,filename);

}

void analyse_jpeg(char* filename)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPARRAY buffer;
    //unsigned char *src_buff;
    unsigned char *point;

    cinfo.err=jpeg_std_error(&jerr);    //一下为libjpeg函数，具体参看相关文档
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo,input_file);
    jpeg_read_header(&cinfo,TRUE);
	//add config
	cinfo.out_color_space=JCS_GRAYSCALE;
	cinfo.output_components = 1;
	
    jpeg_start_decompress(&cinfo);

    unsigned long width=cinfo.output_width;
    unsigned long height=cinfo.output_height;
    unsigned short depth=cinfo.output_components;

	printf(" %d*%d*%d ",width,height,depth*8);

    unsigned char src_buff[width*height*depth];
    memset(src_buff,0,sizeof(unsigned char)*width*height*depth);

    buffer=(*cinfo.mem->alloc_sarray)
        ((j_common_ptr)&cinfo,JPOOL_IMAGE,width*depth,1);

    point=src_buff;
    while (cinfo.output_scanline<height)
    {
        jpeg_read_scanlines(&cinfo,buffer,1);    //读取一行jpg图像数据到buffer
        memcpy(point,*buffer,width*depth);    //将buffer中的数据逐行给src_buff
        point+=width*depth;            //一次改变一行
    }

    //write_bmp_header_single(&cinfo);            //写bmp文件头/
    write_bmp_data_single(&cinfo,src_buff,filename);    //写bmp像素数据
	//write_bmp_header(&cinfo);			//写bmp文件头
	//write_bmp_data(&cinfo,src_buff);	//写bmp像素数据

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    //delete[] src_buff;
}


void jpeg2bin(char* filename)
{

	char path_str[100];
	memset(path_str,0,sizeof(path_str));

	strcat(path_str,PATH_STR);
	strcat(path_str,"/bmp_res.bin");

	

	input_file=fopen(filename,"rb");
    output_file=fopen(path_str,"a+");


    analyse_jpeg(filename);

    fclose(input_file);
    fclose(output_file);

	printf("covert done!\n");
}


void  bmp2bin(char* filename)
{

    struct bmp_fileheader FileHead;
    struct bmp_infoheader InfoHead;
	
	RGBQUAD RGB;  
	int bmpWidth;
	int bmpHeight;
	int biBitCount;
	int lineByte;
	unsigned char *pColorTable = NULL;
	unsigned char *pBmpBuf = NULL;
	char path_str[100];
	memset(path_str,0,sizeof(path_str));

	strcat(path_str,PATH_STR);
	strcat(path_str,"/bmp_res.bin");
    output_file=fopen(path_str,"a+");
	input_file=fopen(filename,"rb");


	if (input_file==NULL)  
	{   
		printf("%s open fail!\n",filename);
		goto DONE;
	}  

	fseek(output_file,0,SEEK_END);
	long pos = ftell(output_file);
	char pos_str[10];
	memset(pos_str,0,sizeof(pos_str));
	sprintf(pos_str,"0x%08X",pos);
	printf("%s",pos_str);

	//read filehead
	fread(&FileHead,sizeof(struct bmp_fileheader),1,input_file);  
	if(FileHead.bfType!=19778) //'bm'  
		{   
			printf("%s type: %d error!\n",filename,FileHead.bfType);
			goto DONE;
		}    

	//read infohead
	fread(&InfoHead,sizeof(struct bmp_infoheader),1,input_file);  
	if(InfoHead.biBitCount!=1)  
		{    
			printf("%s biBitCount=%d invalid!\n",filename,InfoHead.biBitCount);
			goto DONE;

		}
	
	bmpWidth = InfoHead.biWidth; 
	bmpHeight = InfoHead.biHeight; 
	biBitCount = InfoHead.biBitCount;
	lineByte=(bmpWidth * biBitCount/8+3)/4*4; //4BYTES align

	printf(" %d*%d*%d ",bmpWidth,bmpHeight,biBitCount);
	//read color table
	if(biBitCount == 1)
		{
			pColorTable = (unsigned char*)malloc(2*sizeof(RGBQUAD));
			fread(pColorTable,1,2*sizeof(RGBQUAD),input_file);
		}
	
	//read bmp data
	pBmpBuf=(unsigned char*)malloc(lineByte*bmpHeight);

	fread(pBmpBuf,1,lineByte*bmpHeight,input_file);
	if(!pBmpBuf)
		{
			printf("%s bmp data read fail!\n",filename);
			free(pBmpBuf);
			goto DONE;			
		}
	
	//write bin
	fwrite(&bmpWidth, sizeof(bmpWidth), 1, output_file);
	fwrite(&bmpHeight, sizeof(bmpHeight), 1, output_file);	
	fwrite(pBmpBuf, lineByte*bmpHeight, 1, output_file);
	free(pBmpBuf);

	//wirte .h
	
	resid2h(pos,pos_str,filename);

	printf("%s covert done!\n",filename);

DONE:
    fclose(input_file);
    fclose(output_file);

}


int main(int argc,char *argv[])
{
	int idx;

 	if(argc!=4)
	{
		printf("Usage:\tSHOW?Filename.BMP\n");
		exit(1);
	}
	
	if(!(strstr(argv[1],".bmp")||strstr(argv[1],".jpg")))
	{
		printf("Usage:N/A\n");
		exit(1);
	}

	printf("Usage: %d %s %s %s",argc, argv[1],argv[2],argv[3]);


	memset(ID_STR,0,sizeof(ID_STR));	
	memset(ID_NEW_STR,0,sizeof(ID_NEW_STR));	
	memset(NAME_STR,0,sizeof(NAME_STR));
	memset(PATH_STR,0,sizeof(PATH_STR));
	
	if(argv[2] != NULL)
		strcpy(ID_STR,argv[2]);


	if(argv[3] != NULL)
		strcpy(PATH_STR,argv[3]);

	
	strcpy(ID_NEW_STR,ID_STR+strlen(PATH_STR));
		
	
	if(strstr(argv[1],".bmp"))
	{
		idx = strstr(argv[1],".bmp") - argv[1];
		strncpy(NAME_STR,argv[1], idx);
		bmp2bin(argv[1]);
	}
	
	if(strstr(argv[1],".jpg"))
	{
		idx = strstr(argv[1],".jpg") - argv[1];
		strncpy(NAME_STR,argv[1], idx);
		jpeg2bin(argv[1]);
	}

    return 0;
}
