#include "head.h"

/**
 * [find_file read file add task]
 * @author ys 2020-01-18
 * @param  src_name  [Source file path]
 * @param  dest_name [Destination file path]
 * @return           [succeed return 0, failed return -1]
 */
int find_file(const char *src_name,const char *dest_name)//读取源文件路径加入新节点
{
	int i;
	DIR *ap = opendir(dest_name);//打开目录失败这创建这个目录
	if(ap == NULL)
	{
		int ret = mkdir(dest_name, 0777);
		if(ret == -1)
		{
			perror("mkdir failed");
			return -1;
		}
	}

	DIR *dp = opendir(src_name);//打开文件源目录
	if(dp == NULL)
	{
		perror("opendir failure");
		return -1;
	}

	struct dirent *ptr;
	while((ptr = readdir(dp)) != NULL)
	{
		char new_src_name[512];
		char new_dest_name[512];
		bzero(new_src_name, 512);
		bzero(new_dest_name, 512);

		printf("-%s-\n",ptr->d_name);

		if((strcmp(ptr->d_name,".") == 0) || (strcmp(ptr->d_name,"..") == 0))//碰到"."或者 ".."不进入继续下一个循环
					continue;

		sprintf(new_src_name,"%s/%s",src_name,ptr->d_name);//拼接新目录下的路径
		sprintf(new_dest_name,"%s/%s",dest_name,ptr->d_name);

		switch(ptr->d_type)
		{
			case DT_REG:
			{
				int block = file_size(new_src_name);//计算文件大小
				for(i=0; i<block; i++)//如果文件过多则分段创建节点
				{
					pthread_mutex_lock(&pool->mutex);	//创建节点先上锁
					task_t *new = init_insert_node();	//创建新任务节点
					strcpy(new->s_path_name,new_src_name);	//复制新路径至任务队列中
					strcpy(new->o_path_name,new_dest_name);	//复制新目标路径
					new->start_site = i*SIXTEEN;		//计算文件分段后开头位置
					new->end_site = (i+1)*SIXTEEN;		//计算文件分段后结尾位置
					pthread_cond_signal(&pool->cond);	//加入一个任务就给线程发送一次条件变量
					pthread_mutex_unlock(&pool->mutex);	//打开锁
					head->task_num++;	//头结点统计任务数	
				}

				break;
			}
			case DT_DIR:
				if(-1 == find_file(new_src_name, new_dest_name))//递归查找目录文件
					return -1;
				break;
		}
	}

	closedir(ap);
	closedir(dp);
	return 0;
}

/**
 * [file_size Calculate the number of segments in the file]
 * @author ys 2020-01-18
 * @param  src_name [description]
 * @return          [Returns the number of file segments]
 */
int file_size(char *src_name)//计算文件大小 分段
{
	int block=1;
	int fd_r = open(src_name, O_RDONLY);//打开源文件

	int size = lseek(fd_r, 0, SEEK_END);//计算源文件大小
	while(size > SIXTEEN)//如果大于16M则计算分段
	{
		if(size % SIXTEEN == 0)//刚好是16的倍数
		{
			block = size/SIXTEEN;
			break;
		}
		else//不是16的倍数则+1	多分段一次
		{
			block = size/SIXTEEN+1;
			break;
		}
	}

	close(fd_r);
	return block;
}

/**
 * [copy_file copy file]
 * @author ys 2020-01-18
 * @param  task [任务指针]
 * @return      [copy succeed return 0, copy failed reutrn -1]
 */
int copy_file(task_t *task)//复制文件
{
	int fd_r = open(task->s_path_name, O_RDONLY);
	int fd_w = open(task->o_path_name, O_RDWR|O_CREAT, 0777);

	lseek(fd_r, task->start_site, SEEK_SET);//将文件置于开始位置
	lseek(fd_w, task->start_site, SEEK_SET);

	while(1)
	{  
		int trail = lseek(fd_w, 0, SEEK_CUR);//判断当前位置是否在文件尾
		if(trail >= task->end_site)
			break;
		
		char buf[1024+1];
		bzero(buf, 1024+1);
		size_t ret_r = read(fd_r, buf, 1024);//读取源文件512个字节
		if(ret_r == -1)
		{
			perror("read failed");
			return -1;
		}
		size_t ret_w = write(fd_w, buf, ret_r);//写入目标文件512个字节
		if(ret_w == -1)
		{
			perror("write failed");
			return -1;
		}

		if(ret_r < 1024)
			break;
	}

	close(fd_r);
	close(fd_w);
	return 0;
}