#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

#include "custommp4.h"
#include "IpcTest.h"

#include "VVV_Search.h"
#include "VVV_NET_define.h"
#include "VVV_NET.h"


#define MAIN_STREAM_FILE 	"/root/myusb/main_stream.ifv"	//从该文件取主码流帧
#define SUB_STREAM_FILE		"/root/myusb/sub_stream.ifv"	//从该文件取主码流帧

#ifdef WIN32
#define NETSNDRCVFLAG	0
#else
#include <netinet/tcp.h>
#define NETSNDRCVFLAG	MSG_NOSIGNAL
#define INVALID_SOCKET	(-1)
#define INVALID_VAL	(-1)
#define SOCKET_ERROR	(-1)
#endif

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;
typedef unsigned long long	u64;

#if 1
typedef struct
{
	unsigned int ip;
	unsigned short port;
	unsigned int u32DevHandle;
	unsigned int u32ChnHandle;
	unsigned char *pframe_buf;//用于组合I帧，pps sps sei 
	unsigned int buf_used;
	unsigned int buf_size;
	
	unsigned int chn_mask;

	RealStreamCB pStreamCB;
	
	pthread_mutex_t lock;
}klw_client_info;

static klw_client_info *g_klwc_info = NULL;

#endif

//////////////////////////////////////////////////////////////////////////////
typedef struct
{
	//int cap_fd;//数据通道SOCKET
	//custommp4_t* pfile;
	//const char *pfilename;
	int connected;
	//int thread_running;
	RealStreamCB pStreamCB;
	unsigned int dwContext;
	int video_width;
	int video_height;
	volatile char eventLoopWatchVariable;
	pthread_t pid;
	//pthread_cond_t CondThreadExit;
	pthread_mutex_t lock;
}client_info;


static client_info *g_annic_info = NULL;

static unsigned int g_anni_client_count = 0;

static unsigned char g_init_flag = 0;

#define PFrameInterval	(20) //只改变P帧之间的间隔时间(ms)

#if 1
const unsigned char frame_head[4] = {0x00,0x00,0x00,0x01};

int IpcTest_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
	printf("%s\n", __func__);
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(ipcam == NULL)
	{
		return -1;
	}
	
	if(pnw == NULL)
	{
		return -1;
	}
	
	pnw->ip_address = ipcam->dwIp;
	pnw->net_mask = ipcam->net_mask;
	pnw->net_gateway = ipcam->net_gateway;
	pnw->dns1 = ipcam->dns1;
	pnw->dns2 = ipcam->dns2;
	
	return 0;
	
}

int IpcTest_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
	printf("%s do nothing!\n", __func__);
	return 0;
}

static int OnEvenFunc(unsigned int u32Handle,    /* 句柄 */
               unsigned int u32Event,     /* 事件，详见LIVE_NET_STAT_E*/
               void* pUserData,           /* 用户数据*/
               VVV_STREAM_INFO_S* pStreamInfo)/*码流属性*/
{
	int g_klwc_idx = (int)pUserData;
	printf("OnEvenFunc g_klwc_idx: %d event: %d\n", g_klwc_idx, u32Event);

	if (g_klwc_idx != 0 && g_klwc_idx != 1)
	{
		printf("%s pUserData: %d invalid\n", __func__, g_klwc_idx);
		
		return -1;
	}

	pthread_mutex_lock(&g_klwc_info[g_klwc_idx].lock);

	if ((0 == u32Handle) || (u32Handle != g_klwc_info[g_klwc_idx].u32ChnHandle))
	{
		printf("%s u32ChnHandle err\n", __func__);

		pthread_mutex_unlock(&g_klwc_info[g_klwc_idx].lock);
		return -1;
	}

	if ((u32Event == NETSTAT_TYPE_CONNING_FAILED) && (u32Event == NETSTAT_TYPE_ABORTIBE_DISCONNED))
	{
		if (g_klwc_info[g_klwc_idx].u32ChnHandle)
		{
			VVV_NET_StopStream(g_klwc_info[g_klwc_idx].u32ChnHandle);
			g_klwc_info[g_klwc_idx].u32ChnHandle = 0;
		}

		if (g_klwc_info[g_klwc_idx].pframe_buf)
		{
			free(g_klwc_info[g_klwc_idx].pframe_buf);
			g_klwc_info[g_klwc_idx].pframe_buf = NULL;
			g_klwc_info[g_klwc_idx].buf_size = 0;
			g_klwc_info[g_klwc_idx].buf_used = 0;
		}
		
		if (g_klwc_info[g_klwc_idx].u32DevHandle)
		{
			VVV_NET_Logout(g_klwc_info[g_klwc_idx].u32DevHandle);
			g_klwc_info[g_klwc_idx].u32DevHandle = 0;
		}
		
		g_klwc_info[g_klwc_idx].chn_mask = 0;
	}
	
	pthread_mutex_unlock(&g_klwc_info[g_klwc_idx].lock);
	
	return 0;
}

static int KLW_DataCB(unsigned int u32ChnHandle,/* 通道句柄 */
                     unsigned int u32DataType,/* 数据类型，详见VVV_STREAM_TYPE_E 0x01-视频，0x02-音频*/
                     unsigned char* pu8Buffer,/* 数据包含帧头 */
                     unsigned int u32Length,  /* 数据长度 */
                     VVV_U64  u64TimeStamp,    /* 时间戳*/
                     VVV_STREAM_INFO_S *pStreamInfo,/*码流属性*/
                     void* pUserData)         /* 用户数据*/
{
	int chn, chn_start;
	int g_klwc_idx = (int)pUserData;
	real_stream_s stream;
	RealStreamCB pStreamCB = NULL;
	int connected = 0;

	if (g_klwc_idx != 0 && g_klwc_idx != 1)
	{
		printf("%s pUserData: %d invalid\n", __func__, g_klwc_idx);
		
		return -1;
	}

	chn_start = g_klwc_idx * (g_anni_client_count/2);

	if(u32ChnHandle != g_klwc_info[g_klwc_idx].u32ChnHandle)
	{
		printf("%s u32ChnHandle(%u) != g_klwc_info[%d].u32ChnHandle(%u)\n", 
			__func__, u32ChnHandle, g_klwc_idx, g_klwc_info[g_klwc_idx].u32ChnHandle);

		goto fail;
	}
	memset(&stream, 0, sizeof(stream));
	
	if(u32DataType == VVV_STREAM_TYPE_VIDEO 
		&& pStreamInfo->struVencChAttr.enVedioFormat == VVV_VIDEO_FORMAT_H264)
	{
		if(memcmp(pu8Buffer, frame_head, 4) != 0)
		{
			printf("%s g_klwc_idx: %d, frame header err\n", __func__, g_klwc_idx);

			goto fail;
		}

		if (g_klwc_info[g_klwc_idx].buf_used + u32Length > g_klwc_info[g_klwc_idx].buf_size)
		{
			printf("%s g_klwc_idx: %d, buf_used(%d) + u32Length(%d) > buf_size(%d)\n",
				__func__, g_klwc_idx, g_klwc_info[g_klwc_idx].buf_used, u32Length, g_klwc_info[g_klwc_idx].buf_size);

			goto fail;
		}
#if 0
		if (chn == 0)
		{
			printf("%s chn%d buf_used(%d), u32Length(%d), g_klwc_info[g_klwc_idx].buf_size(%d), pu8Buffer[4]: 0x%x\n",
				__func__, chn, g_klwc_info[chn].buf_used, u32Length, g_klwc_info[g_klwc_idx].buf_size, pu8Buffer[4]);
		}
#endif		
		memcpy(g_klwc_info[g_klwc_idx].pframe_buf + g_klwc_info[g_klwc_idx].buf_used, pu8Buffer, u32Length);
		g_klwc_info[g_klwc_idx].buf_used += u32Length;

		if ((pu8Buffer[4]&0x1F) == 0x5)
		{
			stream.frame_type = REAL_FRAME_TYPE_I;
			#if 0
			struct timeval tm;
			gettimeofday(&tm, NULL);
			long long tmp_pts = (long long)1000000*tm.tv_sec + tm.tv_usec;

			if ((0 == g_klwc_info[chn].frame_pts_us)
				|| (llabs(tmp_pts - g_klwc_info[chn].frame_pts_us) > 900*1000))//500ms
			{
				printf("%s chn%d adjust pts %llu to %llu\n", __func__, chn,
					(unsigned long long)g_klwc_info[chn].frame_pts_us,
					(unsigned long long)tmp_pts);

				g_klwc_info[chn].frame_pts_us = tmp_pts;
			}
			#endif
		}
		else if ((pu8Buffer[4]&0x1F) == 0x1)
		{
			stream.frame_type = REAL_FRAME_TYPE_P;
		}
		else //pps sps sei
		{
			return 0;
		}

		//pthread_mutex_lock(&g_klwc_info[g_klwc_idx].lock);

		stream.data = g_klwc_info[g_klwc_idx].pframe_buf;
		stream.len = g_klwc_info[g_klwc_idx].buf_used;
		stream.pts = u64TimeStamp;
		stream.pts *= 1000;
		stream.media_type = MEDIA_PT_H264;
		stream.width = pStreamInfo->struVencChAttr.u32PicWidth;
		stream.height = pStreamInfo->struVencChAttr.u32PicHeight;

		for (chn = chn_start; chn < chn_start+(g_anni_client_count/2); ++chn)
		{
			pthread_mutex_lock(&g_annic_info[chn].lock);
			pStreamCB = g_annic_info[chn].pStreamCB;
			connected = g_annic_info[chn].connected;
			pthread_mutex_unlock(&g_annic_info[chn].lock);
			
			if (connected)
			{
				stream.chn = chn;

				pStreamCB(&stream, chn);
			#if 0
				if (g_klwc_idx == 0)
				{
					printf("callback video chn: %d\n", chn);
				}
			#endif
			}

			
		}

		#if 0
		if (chn == 16)
		{
			printf("chn%d frame type: %d, len: %06d, pts: %llu, local: %u\n",
				chn, stream.frame_type, stream.len, stream.pts/1000, getTimeStamp());
		}
		#endif
		
		g_klwc_info[g_klwc_idx].buf_used = 0;

		//pthread_mutex_unlock(&g_klwc_info[g_klwc_idx].lock);
	}
	else if ((u32DataType == VVV_STREAM_TYPE_AUDIO) )//&& (chn < g_klw_client_count/2))
	{		
		//stream.chn = chn;
		stream.data = (unsigned char *)pu8Buffer + 4;
		stream.len = u32Length - 4;
		stream.pts = u64TimeStamp;
		stream.pts *= 1000;
		
		switch (pStreamInfo->struAencChAttr.enAudioFormat)
		{
			case VVV_AUDIO_FORMAT_G711A :
			{
				stream.media_type = MEDIA_PT_G711;
			} break;
			case VVV_AUDIO_FORMAT_G711Mu :
			{
				stream.media_type = MEDIA_PT_PCMU;
			} break;
			case VVV_AUDIO_FORMAT_G726 :
			{
				stream.media_type = MEDIA_PT_G726;
			} break;
			default :
				printf("enAudioFormat: %d Unknow Audio ENC Format.\n", pStreamInfo->struAencChAttr.enAudioFormat);
				goto fail;
		}
		
		//pthread_mutex_lock(&g_klwc_info[g_klwc_idx].lock);
		
		for (chn = chn_start; chn < chn_start+(g_anni_client_count/2); ++chn)
		{
			pthread_mutex_lock(&g_annic_info[chn].lock);
			pStreamCB = g_annic_info[chn].pStreamCB;
			connected = g_annic_info[chn].connected;
			pthread_mutex_unlock(&g_annic_info[chn].lock);
			
			if (connected)
			{
				stream.chn = chn;

				pStreamCB(&stream, chn);
			#if 0
				if (g_klwc_idx == 0)
				{
					printf("callback video chn: %d\n", chn);
				}
			#endif
			}
		}
		
		//pthread_mutex_unlock(&g_klwc_info[g_klwc_idx].lock);
		
	}

	return 0;
	
fail:
	
	//pthread_mutex_lock(&g_klwc_info[g_klwc_idx].lock);
	
	if (g_klwc_info[g_klwc_idx].u32ChnHandle)
	{
		VVV_NET_StopStream(g_klwc_info[g_klwc_idx].u32ChnHandle);
		g_klwc_info[g_klwc_idx].u32ChnHandle = 0;
	}

	if (g_klwc_info[g_klwc_idx].pframe_buf)
	{
		free(g_klwc_info[g_klwc_idx].pframe_buf);
		g_klwc_info[g_klwc_idx].pframe_buf = NULL;
		g_klwc_info[g_klwc_idx].buf_size = 0;
		g_klwc_info[g_klwc_idx].buf_used = 0;
	}
	
	if (g_klwc_info[g_klwc_idx].u32DevHandle)
	{
		VVV_NET_Logout(g_klwc_info[g_klwc_idx].u32DevHandle);
		g_klwc_info[g_klwc_idx].u32DevHandle = 0;
	}
	
	g_klwc_info[g_klwc_idx].chn_mask = 0;
	
	//pthread_mutex_unlock(&g_klwc_info[g_klwc_idx].lock);
	
	return -1;
}	


int IpcTest_Init(unsigned int max_client_num)
{
	int ret = VVV_SUCCESS;
	short wPort;
	
	printf("%s max_client_num: %d\n", __func__, max_client_num);
	
	if(max_client_num <= 0)
	{
		return -1;
	}
	
	if(g_init_flag)
	{
		return 0;
	}
	
	g_anni_client_count = max_client_num;

	if(VVV_SUCCESS != VVV_NET_Init())
	{
		printf("VVV_NET_Init failed\n");
		return -1;
	}

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		
	int i = 0;

#if 1
	g_annic_info = (client_info *)malloc(g_anni_client_count*sizeof(client_info));
	if(g_annic_info == NULL)
	{
		printf("client_info malloc failed\n");
		return -1;
	}
	memset(g_annic_info, 0, g_anni_client_count*sizeof(client_info));
	
	for (i=0; i<g_anni_client_count; ++i)
	{
		pthread_mutex_init(&g_annic_info[i].lock, &attr);
	}


	u32 ipaddr = inet_addr("192.168.31.218");
	u16 port = 80;
	
	g_klwc_info = (klw_client_info *)malloc(2*sizeof(klw_client_info));
	if(g_klwc_info == NULL)
	{
		printf("klw_client_info malloc failed\n");
		return -1;
	}
	
	memset(g_klwc_info, 0, 2*sizeof(klw_client_info));
	for(i = 0; i < 2; i++)//主副码流
	{
		g_klwc_info[i].ip = ipaddr;
		g_klwc_info[i].port = port;
		
		pthread_mutex_init(&g_klwc_info[i].lock, &attr);

		if (g_klwc_info[i].u32DevHandle == 0)//未连接IPC
		{	
			char devip[64];
			memset(devip, 0, sizeof(devip));
			
			struct in_addr serv;
			serv.s_addr = g_klwc_info[i].ip;
			sprintf(devip, "%s", inet_ntoa(serv));
			//wPort = g_klwc_info[i].port;

			userpassword_s stUserAuth;
			VVV_NET_PROTOCOL_S stNetProtocol;
			stNetProtocol.eNetProtocol = NET_PROTOCOL_QV;
			stNetProtocol.eStreamTransProtocol = TRANS_PROTOCOL_HTTP;
			stNetProtocol.eSocketType = SOCKET_TYPE_TCP;
			stNetProtocol.eControlProtocol = CTL_PROTOCOL_TYPE_PRIVATE;
			
			ret = VVV_NET_Login(&g_klwc_info[i].u32DevHandle, "admin", "admin", devip, port, stNetProtocol, 5000, NULL/*OnEvenFunc*/, &stUserAuth, (void *)i);
			if(ret != VVV_SUCCESS)
			{
				printf("%s idx%d VVV_NET_Login failed, ret: %d\n", __func__, i, ret);
				g_klwc_info[i].u32DevHandle = 0;
				
				//printf("%s unlock1 chn%d\n", __func__, chn);
				//pthread_mutex_unlock(&g_klwc_info[g_klwc_idx].lock);
				//printf("%s unlock2 chn%d\n", __func__, chn);
				return -1;
			}

			//stream
			g_klwc_info[i].buf_used = 0;
			g_klwc_info[i].buf_size = i ? 200*1024 : 640*1024;
			g_klwc_info[i].pframe_buf = (unsigned char *)malloc(g_klwc_info[i].buf_size);
			if (NULL == g_klwc_info[i].pframe_buf)
			{
				printf("%s idx%d malloc frame buf failed\n", __func__, i);

				if (g_klwc_info[i].u32DevHandle)
				{
					VVV_NET_Logout(g_klwc_info[i].u32DevHandle);
					g_klwc_info[i].u32DevHandle = 0;
				}

				//pthread_mutex_unlock(&g_klwc_info[i].lock);
				//printf("%s unlock2 chn%d\n", __func__, chn);
				return -1;
			}		

			VVV_STREAM_INFO_S StreamInfo;
			ret = VVV_NET_StartStream_EX(&g_klwc_info[i].u32ChnHandle, g_klwc_info[i].u32DevHandle, 0, VVV_STREAM_TYPE_VIDEO, i, &StreamInfo, KLW_DataCB, /*OnAlarmFunc*/NULL, (void *)i, g_klwc_info[i].buf_size);
			if(ret != VVV_SUCCESS)
			{
				printf("%s idx%d VVV_NET_StartStream_EX failed, ret: %d\n", __func__, i, ret);

				if (g_klwc_info[i].pframe_buf)
				{
					free(g_klwc_info[i].pframe_buf);
					g_klwc_info[i].pframe_buf = NULL;
				}
				
				if (g_klwc_info[i].u32DevHandle)
				{
					VVV_NET_Logout(g_klwc_info[i].u32DevHandle);
					g_klwc_info[i].u32DevHandle = 0;
				}

				//pthread_mutex_unlock(&g_klwc_info[i].lock);
				//printf("%s unlock2 chn%d\n", __func__, chn);
				return -1;
			}		
			
			//g_klwc_info[g_klwc_idx].pStreamCB = pCB;
		}		
	}

	
#endif
	pthread_mutexattr_destroy(&attr);
	
	g_init_flag = 1;
	
	return 0;
}

int IpcTest_DeInit()
{
	printf("%s\n", __func__);
	
	return 0;
}

int IpcTest_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	//printf("%s chn: %d\n", __func__, chn);

	int ret = VVV_SUCCESS;
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return -1;
	}

	if(IpcTest_GetLinkStatus(chn))
	{
		IpcTest_Stop(chn);
	}

	pthread_mutex_lock(&g_annic_info[chn].lock);
	
	g_annic_info[chn].connected = 1;
	g_annic_info[chn].pStreamCB = pCB;
	g_annic_info[chn].dwContext = dwContext;	
		
	pthread_mutex_unlock(&g_annic_info[chn].lock);

	//printf("%s chn%d \n", __func__, chn);

	return 0;	
}

int IpcTest_Stop(int chn)
{
	//printf("%s chn: %d\n", __func__, chn);
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return -1;
	}

	pthread_mutex_lock(&g_annic_info[chn].lock);
	
	g_annic_info[chn].connected = 0;
	//g_annic_info[chn].pStreamCB = pCB;
	//g_annic_info[chn].dwContext = dwContext;	
		
	pthread_mutex_unlock(&g_annic_info[chn].lock);
	
	return 0;
}


//return value : 1 - Link; 0 - Lost
int IpcTest_GetLinkStatus(int chn)
{
	//printf("%s 1\n", __func__);
	if(!g_init_flag)
	{
		printf("%s g_init_flag: %d\n", __func__, g_init_flag);
		return 0;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		printf("%s chn: %d/%d\n", __func__, chn, g_anni_client_count);
		return 0;
	}
	
	int status = 0;

	pthread_mutex_lock(&g_annic_info[chn].lock);
	
	status = g_annic_info[chn].connected;
		
	pthread_mutex_unlock(&g_annic_info[chn].lock);

	//printf("%s 2\n", __func__);
	return status;
}



#else

void* ThreadPROC(void* pParam)
{
	char *pbuf = NULL;
	u32 buf_size = 0;
	const char *pfile_name = NULL;
	custommp4_t *pfile = NULL;
	u32 file_residue_video_frame_nums = 0;
	u8 read_frame_cnt = 0;
	real_stream_s stream;
	u8 media_type = 0;
	u64 pts = 0;
	u8 key = 0;
	u32 start_time = 0;
	int readlen = 0;
	struct timeval cur_tm; //当前时间
	u64 pre_frame_pts = 0; //上一帧发送的时间
	u32 sleep_us = 0; //两帧之间的间隔
	int chn = (int)pParam;
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		goto ProcOver;
	}
	
	if(chn < (int)(g_anni_client_count/2))
	{
		buf_size = 768*1024;
		pfile_name = MAIN_STREAM_FILE;//主码流
	}
	else
	{
		buf_size = 200*1024;
		pfile_name = SUB_STREAM_FILE;//子码流
	}
	
	pbuf = (char *)malloc(buf_size);
	if(pbuf == NULL)
	{
		goto ProcOver;
	}

	if(chn < (int)(g_anni_client_count/2))
	printf("%s running, chn: %d, file name: %s\n", __func__, chn, pfile_name);
	
	while (g_init_flag)
	{
		pthread_mutex_lock(&g_annic_info[chn].lock);
		if(g_annic_info[chn].eventLoopWatchVariable)
		{
			if(chn < (int)(g_anni_client_count/2))
			printf("%s chn: %d eventLoopWatchVariable == 1, will be exit\n", __func__, chn);
			
			pthread_mutex_unlock(&g_annic_info[chn].lock);
			break;
		}
		pthread_mutex_unlock(&g_annic_info[chn].lock);

		if (file_residue_video_frame_nums == 0)
		{
			
			//线程刚运行，还没有传输
			//或者文件传输完
			//关闭文件再次打开
			if (pfile)
			{
				if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, file tranfer over, reopen it\n", __func__, chn);
				
				custommp4_close(pfile);
				pfile = NULL;
			}
			else
			{
				if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, file tranfer start\n", __func__, chn);
			}

			pfile = custommp4_open(pfile_name, O_R, 0);//该文件必须是从头开始录的
			if (pfile == NULL)
			{
				if(chn < (int)(g_anni_client_count/2))
				printf("error: %s chn: %d, open file(%s) failed\n", __func__, chn, pfile_name);

				break;
			}

			file_residue_video_frame_nums = custommp4_video_length(pfile);
			if(chn < (int)(g_anni_client_count/2))
			printf("%s chn: %d, file_residue_video_frame_nums: %d\n", 
				__func__, chn, file_residue_video_frame_nums);			
		}


		//read frame from file,  and send it
		
		//组合sps pps I帧版本，解决AVI播放卡顿
		readlen = custommp4_read_one_media_frame2(pfile, (u8 *)pbuf, buf_size, &start_time, &key, &pts, &media_type, &read_frame_cnt);
		if (readlen <= 0)
		{
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, custommp4_read failed,errcode=%d,errstr=%s\n",
					__func__, chn, errno, strerror(errno));
						
			break;
		}

		if(0 == media_type) //video frame
		{
			file_residue_video_frame_nums -= read_frame_cnt;//总帧数递减
			/*
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d, read_frame_cnt: %d, file_residue_video_frame_nums: %d\n", 
					__func__, chn, read_frame_cnt, file_residue_video_frame_nums);
			*/
			//处理帧间隔延时
			gettimeofday(&cur_tm, NULL);
			pts = cur_tm.tv_sec;
			pts *= 1000000;
			pts += cur_tm.tv_usec;

			if (pre_frame_pts == 0)//start transfer
			{
				sleep_us = 40*1000; //I frame
			}
			else
			{
				if (key)
				{
					sleep_us = 40*1000; //I frame
				}
				else
				{
					sleep_us = PFrameInterval*1000; //P frame
				}

				//精确延时
				if (pre_frame_pts < pts)//normal
				{
					if (pre_frame_pts+sleep_us > pts)
					{
						sleep_us = pre_frame_pts + sleep_us - pts;
					}
					else
					{
						sleep_us = 0;
					}
				}
				else //u64 rewind
				{
					sleep_us = 30*1000;//简单处理
				}
			}
			/*
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d pre_frame_pts: %llu, sleep_us: %u, pts: %llu\n",
					__func__, chn, pre_frame_pts, sleep_us, pts);
			*/
			usleep(sleep_us);
			
			//延时结束，发送该帧
			gettimeofday(&cur_tm, NULL);
			pts = cur_tm.tv_sec;
			pts *= 1000000;
			pts += cur_tm.tv_usec;

			/*
			if(chn < (int)(g_anni_client_count/2))
				printf("%s chn: %d send frame, type: %c, len: %d, pts_ms: %llu\n",
					__func__, chn, key? 'I':'P', readlen, pts/1000);
			*/
			//frame callback
			memset(&stream, 0, sizeof(stream));
			stream.chn = chn;
			stream.data = (u8 *)pbuf;
			stream.len = readlen;
			stream.pts = pts;
			stream.media_type = MEDIA_PT_H264;
			if(key)
			{
				stream.frame_type = REAL_FRAME_TYPE_I;
			}
			else
			{
				stream.frame_type = REAL_FRAME_TYPE_P;
			}

			if(chn < (int)(g_anni_client_count/2))
			{
				stream.width = 1920;
				stream.height = 1080;//主码流
			}
			else
			{
				stream.width = 704;
				stream.height = 576;//子码流
			}
			
			g_annic_info[chn].pStreamCB(&stream, g_annic_info[chn].dwContext);

			pre_frame_pts = stream.pts;
		}
	}

ProcOver:
	printf("%s quit, chn: %d\n", __func__, chn);
	
	if (pfile)
	{
		custommp4_close(pfile);
		pfile = NULL;
	}

	if(pbuf)
	{
		free(pbuf);
		pbuf = NULL;
	}
	
	return 0;
}

int IpcTest_Init(unsigned int max_client_num)
{
	printf("%s max_client_num: %d\n", __func__, max_client_num);
	
	if(max_client_num <= 0)
	{
		return -1;
	}
	
	if(g_init_flag)
	{
		return 0;
	}
	
	g_anni_client_count = max_client_num;
	
	g_annic_info = (client_info *)malloc(g_anni_client_count*sizeof(client_info));
	if(g_annic_info == NULL)
	{
		return -1;
	}
	memset(g_annic_info, 0, g_anni_client_count*sizeof(client_info));

	int i = 0;	
	for(i = 0; i < (int)g_anni_client_count; i++)
	{
		//g_annic_info[i].cap_fd = INVALID_SOCKET;
		//g_annic_info[i].pfile = NULL;
		//g_annic_info[i].pfilename = NULL;
		g_annic_info[i].pid = INVALID_VAL;
		//g_annic_info[i].eventLoopWatchVariable = 0;
		
		pthread_mutex_init(&g_annic_info[i].lock, NULL);
	}

#if 1
	u32 ipaddr = inet_addr("192.168.31.218");
	u16 port = 80;
	
	g_klwc_info = (klw_client_info *)malloc(2*sizeof(klw_client_info));
	if(g_klwc_info == NULL)
	{
		return -1;
	}

	
	memset(g_klwc_info, 0, 2*sizeof(klw_client_info));
	for(i = 0; i < 2; i++)//主副码流
	{
		g_klwc_info[i].ip = ipaddr;
		g_klwc_info[i].port = port;
		
		pthread_mutex_init(&g_klwc_info[i].lock, NULL);
	}
#endif	
	g_init_flag = 1;
	
	return 0;
}

int IpcTest_DeInit()
{
	printf("%s\n", __func__);
	
	return 0;
}


int IpcTest_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
	printf("%s\n", __func__);
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(ipcam == NULL)
	{
		return -1;
	}
	
	if(pnw == NULL)
	{
		return -1;
	}
	
	pnw->ip_address = ipcam->dwIp;
	pnw->net_mask = ipcam->net_mask;
	pnw->net_gateway = ipcam->net_gateway;
	pnw->dns1 = ipcam->dns1;
	pnw->dns2 = ipcam->dns2;
	
	return 0;
	
}

int IpcTest_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
	printf("%s do nothing!\n", __func__);
	return 0;
}

int IpcTest_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	printf("%s chn: %d\n", __func__, chn);

	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return -1;
	}

	if(IpcTest_GetLinkStatus(chn))
	{
		IpcTest_Stop(chn);
	}

	pthread_mutex_lock(&g_annic_info[chn].lock);

	g_annic_info[chn].pStreamCB = pCB;
	g_annic_info[chn].dwContext = dwContext;
	
	/*
	g_annic_info[chn].pfilename = pfile_name;
	
	g_annic_info[chn].pfile = custommp4_open(g_annic_info[chn].pfilename, O_R, 0);//该文件必须是从头开始录的
	if (g_annic_info[chn].pfile == NULL)
	{
		printf("error: %s chn: %d, open file(%s) failed\n", __func__, chn, g_annic_info[chn].pfilename);

		pthread_mutex_unlock(&g_annic_info[chn].lock);
		
		return -1;
	}
	*/
	
	if(pthread_create(&g_annic_info[chn].pid, NULL, ThreadPROC, (void *)chn) != 0)
	{
		printf("error: %s chn: %d, pthread_create failed\n", __func__, chn);
		
		g_annic_info[chn].pid = INVALID_VAL;
		//custommp4_close(g_annic_info[chn].pfile);
		pthread_mutex_unlock(&g_annic_info[chn].lock);
		
		return -1;
	}
	g_annic_info[chn].connected = 1;
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);

	return 0;	
}

int IpcTest_Stop(int chn)
{
	printf("%s chn: %d\n", __func__, chn);
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return -1;
	}

	pthread_mutex_lock(&g_annic_info[chn].lock);

	if (!g_annic_info[chn].connected)
	{
		pthread_mutex_unlock(&g_annic_info[chn].lock);
		return 0;
	}
	
	if (g_annic_info[chn].pid != INVALID_VAL)
	{
		g_annic_info[chn].eventLoopWatchVariable = 1;
		
		pthread_mutex_unlock(&g_annic_info[chn].lock);
		
		pthread_join(g_annic_info[chn].pid, NULL);
		
		pthread_mutex_lock(&g_annic_info[chn].lock);
	}	
	
	g_annic_info[chn].pid = INVALID_VAL;
	g_annic_info[chn].eventLoopWatchVariable = 0;
	g_annic_info[chn].connected = 0;
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);
	
	return 0;
}


//return value : 1 - Link; 0 - Lost
int IpcTest_GetLinkStatus(int chn)
{
	if(!g_init_flag)
	{
		return 0;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return 0;
	}
	
	int status = 0;
	
	pthread_mutex_lock(&g_annic_info[chn].lock);
	
	//status = (g_annic_info[chn].cap_fd != INVALID_SOCKET);
	status = g_annic_info[chn].connected;
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);
	
	return status;
}

#endif




