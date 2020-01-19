#include "head.h"

/**
 * [main Program main entry]
 * @author ys 2020-01-18
 * @param  argc [copy Source file path]
 * @param  argv [copy Destination file path]
 * @return      [0]
 */
int main(int argc, char const *argv[])
{
	if(argc < 3)
	{
		printf("please input two parameter!(Source file path and Destination file path)\n");
		return -1;
	}

	/*初始化头节点*/
	head = init_node();

	/*初始化线程池 开始新增1个线程*/
	pool = pool_malloc();
	init_pool(pool,1);
	
	/*创建一条线程统计文件多少创建相应的子线程*/
	statistics_task();

	/*复制源文件路径至链表中*/
	find_file(argv[1], argv[2]);

	/*主线程给所有子线程发送条件变量*/
	pool->go_home = 1;
	pthread_cond_broadcast(&pool->cond);

	/*等待所有子线程退出*/
	int i;
	for(i=0; i<pool->pthread_num ; i++)
	{
		pthread_join((pool->tid[i]), NULL );
	}

	free(head);
	free(pool);

	pthread_exit(NULL);
	return 0;
}
