#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
 
struct iphead{   //该结构体模拟IP首部（代码中，控制套接字不添加IP数据包首部，需要自己添加），
		//关于各变量的含义，可对照IP数据报格式，一目了然。
    unsigned char ip_hl:4, ip_version:4;  //ip_hl,ip_version各占四个bit位。
    unsigned char ip_tos;
    unsigned short int ip_len;   
    unsigned short int ip_id;
    unsigned short int ip_off;  
    unsigned char ip_ttl;
    unsigned char ip_pro;
    unsigned short int ip_sum;
    unsigned int ip_src;
    unsigned int ip_dst;
};
 
struct icmphead{  //该结构体模拟ICMP报文首部
    unsigned char icmp_type;
    unsigned char icmp_code;
    unsigned short int icmp_sum;
    unsigned short int icmp_id;
    unsigned short int icmp_seq;
};
 
unsigned short int cksum(char buffer[], int size){   //计算校验和，具体的算法可自行百度，或查阅资料
    unsigned long sum = 0;
    unsigned short int answer;
    unsigned short int *temp;
    temp = (short int *)buffer;
    for( ; temp<buffer+size; temp+=1){
        sum += *temp;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}
 
int main(){
   
    int sockfd;
    struct sockaddr_in conn;
    struct iphead *ip;
    struct icmphead *icmp;
    unsigned char package[sizeof(struct iphead) + sizeof(struct icmphead)];  //package存储IP数据报的首部和数据
    memset(package, 0, sizeof(package));
 
    ip = (struct iphead*)package;
    icmp = (struct icmphead*)(package+sizeof(struct iphead)); //IP数据报数据字段仅仅包含一个ICMP首部
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); //创建套接字
    if(sockfd < 0){
        printf("Create socket failed\n");
        return -1;
    }
    conn.sin_family = AF_INET;
    conn.sin_addr.s_addr = inet_addr("192.168.230.135");
    int one = 1;
    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0){  //设置套接字行为，此处设置套接字不添加IP首部
        printf("setsockopt failed!\n");
        return -1;
    }
    /*设置IP首部各个字段的值*/  
    ip->ip_version = 4; 
    ip->ip_hl = 5;
    ip->ip_tos = 0;
    ip->ip_len = htons(sizeof(struct iphead) + sizeof(struct icmphead)); //关于htons()、htonl()的作用，可自行百度	
    ip->ip_id = htons(1);
    ip->ip_off = htons(0x4000);
    ip->ip_ttl = 10;
    ip->ip_pro = IPPROTO_ICMP;
    ip->ip_src = htonl(inet_addr("192.168.230.135"));
    ip->ip_dst = htonl(inet_addr("192.168.230.135"));
    printf("ipcksum : %d\n", cksum(package, 20)); 
    ip->ip_sum = cksum(package, 20);  // 计算校验和，应当在其他字段之后设置（实验中发现检验和会被自动添加上）
    
    /*设置ICMP首部各字段值*/
    icmp->icmp_type = 8;
    icmp->icmp_code = 0;
    icmp->icmp_id = 1;
    icmp->icmp_seq = 0;
    icmp->icmp_sum = (cksum(package+20, 8));
    /*接下来发送IP数据报即可*/
    for(int k=0,i<1000,i++){
        
    if(sendto(sockfd, package, htons(ip->ip_len), 0,(struct sockaddr *)&conn, sizeof(struct sockaddr)) < 0){
	printf("send failed\n"); 
	return -1;
    }
    printf("send successful\n");
    }    
    return 0;
}