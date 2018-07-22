#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>				//struct ifreq
#include <sys/ioctl.h>			//ioctl、SIOCGIFADDR
#include <sys/socket.h>
#include <netinet/ether.h>		//ETH_P_ALL
#include <netpacket/packet.h>	//struct sockaddr_ll
#include <sys/time.h>
 
 
unsigned short checksum(unsigned short *buf, int nword);//校验和函数
int main(int argc, char *argv[])
{
    
	//1.创建通信用的原始套接字
	int sock_raw_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    srand(time(NULL));
   // int count;
	unsigned char ip24[2];
    unsigned char ip16[2];
    unsigned char ip8[2];
    unsigned char ip0[2];
	//unsigned char send_msg[1024];
	while(1){
   //for(count = 0;count<5000;count++)
   //{
		/*
	unsigned char ip24[2];
    unsigned char ip16[2];
    unsigned char ip8[2];
    unsigned char ip0[2];
	*/
	/*
    char *s1 = ip24;
    char *s2 = ip16;
    char *s3 = ip8;
    char *s4 = ip0;
	*/
    int random1 = rand()%231+10;
    int random2 = rand()%231+10;
    int random3 = rand()%231+10;
    int random4 = rand()%231+10;
    printf("%d\t%d\t%d\t%d\n",random1,random2,random3,random4);
    //sprintf(ip24,"%d",random1);
    //sprintf(ip16,"%d",random2);
    //sprintf(ip8,"%d",random3);
    //sprintf(ip0,"%d",random4);               ///********************************************

	//printf("%d\t%d\t%d\t%d\n",ip24,ip16,ip8,ip0);
	//2.根据各种协议首部格式构建发送数据报
	unsigned char send_msg[1024] = {
		//--------------组MAC--------14------
		0x66, 0x6b, 0x1b, 0x49, 0x29, 0x66 , //dst_mac: 74-27-EA-B5-FF-D8
		0xee, 0xd8, 0x7f, 0x64, 0xeb, 0x26 , //src_mac: rz0x0a, 0x05, 0x05, 0xe6, 0x0b, 0xf2 ,

		0x08, 0x00,                         //类型：0x0800 IP协议
		//--------------组IP---------20------rz
		0x45, 0x00, 0x00, 0x00,             //版本号：4, 首部长度：20字节, TOS:0, --总长度--：
		0x00, 0x00, 0x00, 0x00,				//16位标识、3位标志、13位片偏移都设置0
		0x80, 17,   0x00, 0x00,				//TTL：128、协议：UDP（17）、16位首部校验和
		10,  0,   0,  1,				//src_ip: 
		random1,  random2,   random3,  random4			//dst_ip:
        /*
		//--------------组UDP--------8+78=86------
		0x1f, 0x90, 0x1f, 0x90,             //src_port:0x1f90(8080), dst_port:0x1f90(8080)
		0x00, 0x00, 0x00, 0x00,               //#--16位UDP长度--30个字节、#16位校验和
        */
	};
    //printf("%s\t%s\t%s\t%s\n",send_msg[28],send_msg[29],send_msg[30],send_msg[31]); 
	int k;
	for(k=30;k<34;k++){
		printf("%d\t",send_msg[k]);
	}
	

	//int len = sprintf(send_msg+34, "%s", "this is for the test");
    int len = 0;
	/*
	if(len % 2 == 1)//判断len是否为奇数
	{
		len++;//如果是奇数，len就应该加1(因为UDP的数据部分如果不为偶数需要用0填补)
	}
    */
	
	*((unsigned short *)&send_msg[16]) = htons(20);//IP总长度 = 20 + 8 + len
    
	// *((unsigned short *)&send_msg[14+20+4]) = htons(8+len);//udp总长度 = 8 + len
	/*
    //3.UDP伪头部
	unsigned char pseudo_head[1024] = {
		//------------UDP伪头部--------12--
		10,  0,   0,  1,				//src_ip: 10.221.20.11
		ip24,  ip16,   ip8,  ip0,				//dst_ip: 10.221.20.10
		0x00, 17,   0x00, 0x00,             	//0,17,#--16位UDP长度--20个字节
	};
	
	*((unsigned short *)&pseudo_head[10]) = htons(8 + len);//为头部中的udp长度（和真实udp长度是同一个值）
	//4.构建udp校验和需要的数据报 = udp伪头部 + udp数据报
	memcpy(pseudo_head+12, send_msg+34, 8+len);//--计算udp校验和时需要加上伪头部--
	*/
    //5.对IP首部进行校验
	*((unsigned short *)&send_msg[24]) = htons(checksum((unsigned short *)(send_msg+14),20/2));
    /*
	//6.--对UDP数据进行校验--
	*((unsigned short *)&send_msg[40]) = htons(checksum((unsigned short *)pseudo_head,(12+8+len)/2));
	*/
	//printf("log2\n");
	//6.发送数据
	struct sockaddr_ll sll;					//原始套接字地址结构
	struct ifreq req;					//网络接口地址
	
	strncpy(req.ifr_name, "h2-eth0", IFNAMSIZ);			//指定网卡名称
	if(-1 == ioctl(sock_raw_fd, SIOCGIFINDEX, &req))	//获取网络接口
	{
		perror("ioctl");
		close(sock_raw_fd);
		exit(-1);
	}
	
	/*将网络接口赋值给原始套接字地址结构*/
	bzero(&sll, sizeof(sll));
	sll.sll_ifindex = req.ifr_ifindex;
	len = sendto(sock_raw_fd, send_msg, 14+20+8+len, 0 , (struct sockaddr *)&sll, sizeof(sll));
	if(len == -1)
	{
		perror("sendto");
	}
    printf("\nsend success\n");
    //usleep(200);
    //}
    }
	return 0;
}
 
unsigned short checksum(unsigned short *buf, int nword)
{
	unsigned long sum;
	for(sum = 0; nword > 0; nword--)
	{
		sum += htons(*buf);
		buf++;
	}
	sum = (sum>>16) + (sum&0xffff);
	sum += (sum>>16);
	return ~sum;
}