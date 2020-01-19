# Linux_C-Multithreaded-File-Copy
	本项目基于linux C语言实现多线程文件拷贝。
	
## 实现思路:
	运用Linux内核链表实现任务队列，读取源路径文件，将读取到的文件路径尾插入队列中，初始创建一个线程
	当有任务时，头取任务，进行文件拷贝。另外创建一个线程，统计任务数量，根据任务数量，
	创建相对应的线程，1个线程分配50个任务，多了则删除线程，少了则增加线程。
	
## 相关函数:
```
1.queue
//init_node	初始化新节点
task_t *init_node(void);
	
//init_insert_node	新建结点尾插入链表
task_t *init_insert_node(void)
	
2.threads
//init_pool	初始化线程池
//pool_t *p	线程池结构体指针
//unsigned int pthread_num	初始化线程数
int init_pool(pool_t *p, unsigned int pthread_num);
	
//add_thread	增加指定线程数
//pool_t *p	线程池结构体指针
//unsigned int additional_threads	需要增加的线程数
int add_thread(pool_t *pool, unsigned int additional_threads);
	
//remove_thread	删除指定线程数
//pool_t *p	线程池结构体指针
//unsigned int removing_threads	需要删除的线程数
int remove_thread(pool_t *pool, unsigned int removing_threads);
	
3.FileIO
//file_size 计算文件大小 分段 
//char *src_name	所要计算的文件路径
int file_size(char *src_name);
	
//find_file 读取源文件路径加入任务队列
//const char *src_name	源文件路径
//const char *dest_name	目标文件路径
int find_file(const char *src_name,const char *dest_name);
	
//copy_file	复制文件
//task_t *task	任务节点指针
int copy_file(task_t *task);
```

## 运行：
运行时给定源拷贝路径和目标拷贝路径(example：./main SourceCopyPath TargetCopyPath)
	
## 编译：
`make`	或 `gcc -o main *.c -lphread`
