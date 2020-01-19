#ifndef __HEAD_H__
#define __HEAD_H__


#include <stdio.h>
#include <pthread.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "kernel_list.h"//内核链表

#define SIXTEEN 1048576//16777216	//16777216 16M
#define MAX_THREAD 100		//最大线程数

//内核链表 任务队列
typedef struct order_list
{
	char s_path_name[1024];	//源文件路径
	char o_path_name[1024];	//目标文件路径
	int start_site;			//开始位置
	int end_site;			//结束位置
	unsigned int task_num;	//任务序号
	struct list_head list;	//内核链表

}task_t;

//线程池
typedef struct
{
	unsigned int pthread_num;	//线程序号
	pthread_t *tid;				//线程指针
	pthread_mutex_t mutex;		//互斥锁
	pthread_cond_t cond;		//条件变量
	pthread_attr_t attr;		//分离属性
	int go_home;

}pool_t;

task_t *head;
pool_t *pool;


//thread
extern pool_t *pool_malloc(void);	//申请堆空间
extern void *routine(void *arg);	//线程执行任务
extern int statistics_task(void);	//另建一个线程进行统计任务
extern void *count_task(void *arg);	//计算文件数量来增删线程
extern int init_pool(pool_t *p, unsigned int pthread_num);				//初始化线程池
extern int add_thread(pool_t *pool, unsigned additional_threads);		//增加指点线程数
extern int remove_thread(pool_t *pool, unsigned int removing_threads);	//删除指定线程数

//kernel list queue
extern task_t *init_node(void);			//初始化队列
extern task_t *init_insert_node(void);	//创建新节点并尾插入队列

//file IO
extern int copy_file(task_t *q);		//复制文件
extern int file_size(char *src_name);	//计算文件大小
extern int find_file(const char *src_name,const char *dest_name);	//读取源文件路径加入新节点


#endif