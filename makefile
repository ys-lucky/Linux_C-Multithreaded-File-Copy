CC = gcc
#默认放入当前文件夹下的所有.c文件
SRC=$(wildcard *.c)
#将SRC变量中的所有.c后缀的字符串全部替换成.o后缀
#然后将替换结果交过变量OBJ
# %表示是一个  这里是一个.c文件生成一个.o文件
OBJ=$(patsubst %.c,%.o,$(SRC))

#可执行文件名
TARGET=main

$(TARGET):$(OBJ)
	$(CC) -o $@ $^ -lpthread

%.o:%.c
	$(CC) -c -o $@ $< 

clean:
	rm $(OBJ)

