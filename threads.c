#include "head.h"

pool_t *pool_malloc(void)//申请堆空间
{
	pool_t *p = malloc(sizeof(pool_t));
	if(p == NULL)
	{
		perror("p malloc fialed");
		exit(-1);
	}
	return p;
}
/**
 * [statistics_task A new thread counts the number of tasks to create and delete the corresponding thread]
 * @author ys 2020-01-19
 * @return  [create thread succeed return 0, create thread failed return -1]
 */
int statistics_task(void)//另建一个线程进行统计任务
{
	pthread_t tid;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_attr_t attr;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//设置分离属性，不需要主线程等待

	if(pthread_create(&tid, &attr, count_task, NULL) != 0)
	{
		perror("create pthread failed");
		return -1;
	}
	else
		printf("create pthread succeed\n");

	return 0;
}

/**
 * [init_pool 初始化线程池]
 * @author ys 2020-01-18
 * @param  p           [线程池结构体指针]
 * @param  pthread_num [创建线程数]
 * @return             [succeed return 0,failed return -1]
 */
int init_pool(pool_t *p, unsigned int pthread_num)
{
	p->tid = malloc(sizeof(pool_t) *pthread_num);
	pthread_mutex_init(&p->mutex, NULL);
	pthread_cond_init(&p->cond, NULL);
	pthread_attr_init(&p->attr);
	pthread_attr_setdetachstate(&p->attr, PTHREAD_CREATE_DETACHED);
	p -> pthread_num = 0;
	p -> go_home = 0;
	int i;
	for(i = 0; i < pthread_num; i++, p -> pthread_num++)
	{
		printf("create pthread succeed\n");
		if(pthread_create(&((p->tid)[i]), NULL, routine, NULL) != 0)
		{
			perror("create pthread failed");
			return -1;
		}
	}
	return 0;
}
/**
 * [routine thread execute the task]
 * @author ys 2020-01-19
 * @param  arg [Thread function parameter]
 * @return     [No return value]
 */
void *routine(void *arg)//线程执行任务
{
	while(1)
	{
		pthread_mutex_lock(&pool->mutex);//取任务时先上锁避免互斥
		while( (&head->list)->next == &(head->list) && pool->go_home == 0)
		{
			pthread_cond_wait(&pool->cond, &pool->mutex);//没有任务时先进入条件变量等待
		}

		if((&head->list)->next == &(head->list) && pool->go_home == 1)//如果主线程退出并且没有任务则解锁退出
		{
			pthread_mutex_unlock(&pool->mutex);
			usleep(5000);//设置延时 避免删除过快出现段错误
			pthread_exit(NULL);
		}

		task_t *q = list_entry((&head->list)->next, typeof(*head), list);//头取任务列表
		list_del_init(&q->list);//取任务节点，断除链接
		printf("get=%s\n",q->s_path_name);
		head->task_num--;
		pthread_mutex_unlock(&pool->mutex);//取得任务后解锁
	
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);	//执行任务时，关闭响应取消请求
		copy_file(q);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);	//任务执行完成后，开启响应取消请求
		free(q);
	}
}

/**
 * [count_task Create or delete threads based on the number of tasks]
 * @author ys 2020-01-19
 * @param  arg [parameter ]
 * @return     [No return value]
 */
void *count_task(void *arg)//根据任务数量创建或删除相应的线程
{
	int i=0;

	while(1)
	{
		if (head->task_num != 0 && (&head->list)->next != &(head->list))
		{
			if((head->task_num) % 50 == 0)//以任务个数判断出应有线程数
			{
				i = (head->task_num)/50;
			}
			else
			{
				i = (head->task_num)/50 + 1;
			}

			if(i > pool->pthread_num)//如果应有线程数大于现有线程数，则增加线程
			{
				printf("-----------head->task_num:%d\n",head->task_num );
				printf("-----------pool->pthread_num:%d\n",pool->pthread_num );
				printf("-----------add thread number:%d\n",i-(pool->pthread_num));
				add_thread(pool,i-(pool->pthread_num));
			}
			else if(i < pool->pthread_num)//如果应有线程数小于现有线程数，则减少线程
			{
				printf("-----------head->task_num:%d\n",head->task_num );
				printf("-----------pool->pthread_num:%d\n",pool->pthread_num );
				printf("-----------delete thread number:%d\n",(pool->pthread_num)-i);
				remove_thread(pool, (pool->pthread_num)-i);
			}
		}

		if(pool->go_home == 1)
		{
			pthread_exit(NULL);
		}
		usleep(5000);//设置延时 避免过快出现段错误
	}

}

/**
 * [remove_thread delete thread by thread number]
 * @author ys 2020-01-18
 * @param  pool             [线程池结构体指针]
 * @param  removing_threads [delete thread number]
 * @return                  [delete thread succeed return delete thread num, delete failed return -1]
 */
int remove_thread(pool_t *pool, unsigned int removing_threads)
{
	if(removing_threads == 0)
		return pool->pthread_num;

	int remain_threads = pool->pthread_num - removing_threads;//减掉删除的子线程后的子线程总数
	
	remain_threads = remain_threads > 0 ? remain_threads : 1;//子线程总数必须不能小于1，线程池未销毁，子线程不能全部取消

	int i;
	for(i=pool->pthread_num-1; i>remain_threads-1; i--)//删除子线程
	{
		errno = pthread_cancel(pool->tid[i]);
		if(errno != 0)
		{
			printf("delete failed\n");
			break;
		}
	}

	if(i == pool->pthread_num-1)//一个子线程都不删除，直接退出函数
		return -1;
	else
	{
		pool->pthread_num = i+1;//删除了相应子线程后，对当前活跃的子线程数进行更新
		return i+1;
	}
}

/**
 * [add_thread add thread]
 * @author ys 2020-01-18
 * @param  pool               [线程池结构体指针]
 * @param  additional_threads [增加线程数]
 * @return                    [succeed return add thread num,failed return -1]
 */
int add_thread(pool_t *pool, unsigned additional_threads)
{
	if (additional_threads == 0)
		return 0;
	
	unsigned total_threads = pool->pthread_num + additional_threads;

	int i,actual_increment = 0;
	for (int i = pool->pthread_num; i < total_threads && i <= MAX_THREAD; i++)
	{
		if(pthread_create(&((pool->tid)[i]), NULL, routine, (void *)pool) != 0)
		{
			perror("add thread failed!");
			return -1;

			if (actual_increment == 0)
				return -1;

			break;
		}
		actual_increment++;
	}

	pool->pthread_num += actual_increment;
	return actual_increment;
}