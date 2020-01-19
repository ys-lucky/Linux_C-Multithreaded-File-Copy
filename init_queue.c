#include "head.h"

task_t *init_node(void)//初始化新节点
{
	task_t *new_task = malloc(sizeof(task_t));
	if(new_task == NULL)
	{
		printf("file:%s line:%d:malloc error!\n",__FILE__,__LINE__);
		exit(1);
	}

	bzero(new_task->s_path_name, 1024);
	bzero(new_task->o_path_name, 1024);
	new_task->start_site = 0;
	new_task->end_site = 0;
	new_task->task_num = 0;

	INIT_LIST_HEAD(&new_task->list);//自循环

	return new_task;
}

task_t *init_insert_node(void)//新建结点尾插入链表
{
	task_t *node = init_node();
	// 链接到链表中 
	list_add_tail(&(node->list), &(head->list));
	return node;
}