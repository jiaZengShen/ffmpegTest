#ifdef __cplusplus  
extern "C"   
{  
#include "libavutil/avutil.h"  
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libavdevice/avdevice.h"  
#include "libswscale/swscale.h"  
#include "libavfilter/avfilter.h"
}  
#endif  
#pragma comment(lib,"avutil.lib")  
#pragma comment(lib,"avcodec.lib")  
#pragma comment(lib,"avformat.lib")  
#pragma comment(lib,"swscale.lib") 
#pragma comment(lib,"avfilter.lib")

//

#include <stdint.h>
#include <stdio.h>
#include "windows.h"  
#include "time.h"  
char buffer[512];

//�ο���ַ��http://blog.csdn.net/cffishappy/article/details/7601228
void SaveBmp(AVCodecContext *CodecContex, AVFrame *Picture,int num )  
{  
	AVPicture pPictureRGB;//RGBͼƬ  
	int width = CodecContex->width;
	int height = CodecContex->height;
	static struct SwsContext *img_convert_ctx;  
	img_convert_ctx = sws_getContext(width, height, CodecContex->pix_fmt, width, height,
		PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);  
	avpicture_alloc(&pPictureRGB, PIX_FMT_RGB24, width, height);  
	sws_scale(img_convert_ctx, Picture->data, Picture->linesize,
		0, height, pPictureRGB.data, pPictureRGB.linesize);  

	int lineBytes = pPictureRGB.linesize[0], i=0;  

	char fileName[1024]={0};  
	time_t ltime;  
	time(&ltime);  
	sprintf(fileName, "d://hello//%d.bmp", num);  

	FILE *pDestFile = fopen(fileName, "wb");  
	BITMAPFILEHEADER btfileHeader;  
	btfileHeader.bfType = MAKEWORD(66, 77);   
	btfileHeader.bfSize = lineBytes*height;   
	btfileHeader.bfReserved1 = 0;   
	btfileHeader.bfReserved2 = 0;   
	btfileHeader.bfOffBits = 54;  

	BITMAPINFOHEADER bitmapinfoheader;  
	bitmapinfoheader.biSize = 40;   
	bitmapinfoheader.biWidth = width;   
	bitmapinfoheader.biHeight = height;   
	bitmapinfoheader.biPlanes = 1;   
	bitmapinfoheader.biBitCount = 24;  
	bitmapinfoheader.biCompression = BI_RGB;   
	bitmapinfoheader.biSizeImage = lineBytes*height;   
	bitmapinfoheader.biXPelsPerMeter = 0;   
	bitmapinfoheader.biYPelsPerMeter = 0;   
	bitmapinfoheader.biClrUsed = 0;   
	bitmapinfoheader.biClrImportant = 0;  

	fwrite(&btfileHeader, 14, 1, pDestFile);  
	fwrite(&bitmapinfoheader, 40, 1, pDestFile);  
	for(i=height-1; i>=0; i--)  
	{  
		fwrite(pPictureRGB.data[0]+i*lineBytes, lineBytes, 1, pDestFile);  
	}  

	fclose(pDestFile);  
	avpicture_free(&pPictureRGB);  
}  


int  main()
{
	int result ;//���
	int i ;
	struct AVFormatContext *ic = NULL;
	//char *fileName="hello.264";//�ļ���
	//char *fileName="hello.mpg";//�ļ���
	//char *fileName="hello.mp4";//�ļ���
	char *fileName="workers.mp4";//�ļ���
	int st_index[AVMEDIA_TYPE_NB];
	struct AVDictionaryEntry  *t = NULL;//��Ƶ������

	av_register_all();
	avcodec_register_all();
	avfilter_register_all();

	//���ļ�
	ic = avformat_alloc_context();
	//ic->interrupt_callback.opaque
	result = avformat_open_input(&ic,fileName,NULL,NULL);

	//
	av_format_inject_global_side_data(ic);
	avformat_find_stream_info(ic,NULL);

	//��ӡ����
	t = av_dict_get(ic->metadata,"title",NULL,0);
	if(t)
	{
		sprintf(buffer,"%s\n",t->value);
	}
	av_dump_format(ic,0,fileName,0);
	
	//��λ���ײ�
	{

		int64_t timestamp;
		timestamp = 0;
		/* add the stream start time */
		if (ic->start_time != AV_NOPTS_VALUE)
			timestamp += ic->start_time;
		avformat_seek_file(ic,-1,INT64_MIN,timestamp,INT64_MAX,0);
	}

	st_index[AVMEDIA_TYPE_VIDEO] =
		av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
		-1, -1, NULL, 0);

	if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {//���ҳ�������
		AVStream *st = ic->streams[st_index[AVMEDIA_TYPE_VIDEO]];
		AVCodecContext *avctx = st->codec;
		AVRational sar = av_guess_sample_aspect_ratio(ic, st, NULL);
			
	}else{
		return -1 ;
	}
	//Ѱ�ҽ�����
	struct AVCodecContext *avctx;
	AVCodec *codec;
	avctx = ic->streams[st_index[AVMEDIA_TYPE_VIDEO] ]->codec;
	codec = avcodec_find_decoder(avctx->codec_id);
	if(!codec)
		return -2 ;//û���ҵ�������
	//�򿪽�����
	result =avcodec_open2(avctx,codec,NULL);
	if(result)
		return -3 ;//�򿪽�����ʧ��
	//��ȡ����
	AVPacket InPack;
	result = av_read_play(ic);
	AVFrame* oneFrame = av_frame_alloc(); 
	int nComplete;

	while((result =av_read_frame(ic, &InPack)  )>= 0) 
	{
		static int i = 0;
		result = av_read_play(ic);
		//����
		int len = avcodec_decode_video2(avctx,oneFrame,&nComplete,&InPack);
		if(nComplete>0)
		{
			//д���ļ�
			SaveBmp(avctx,oneFrame,i);
			i++;
		}else{
			
			i++;
			printf("%d\n",i++);
		}


		//�ͷ�
		av_free_packet(&InPack);  
	}
	

	//�������
	avformat_close_input(&ic);
	av_frame_free(&oneFrame);
}