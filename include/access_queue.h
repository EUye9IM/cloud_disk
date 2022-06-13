/**
* Access Queue 接口声明
* Access Queue用于维护一个写文件队列，每当有新的上传文件请求加入队列时，评估该请求并分配任务。
*/
#ifndef ACCESS_QUEUE_H
#define ACCESS_QUEUE_H

#define configMAX_ACCESS_QUEUE 1024
#define configFILE_SLICE_SIZE 1024 //单位KiB

/*
* Access Queue类声明
* 要启动Access Queue模块，首先创建一个AccessQueue对象，然后调用startAccessQueue函数来启动。
*/

class AccessQueue{
public:

/* 
* 启动Access Queue线程
* maxQueueAmount指定了服务器最大可同时接收的文件数量
* fileSliceSize指定了文件传输切片的大小，单位为KiB
*/
void startAccessQueue(int maxQueueAmount=configMAX_ACCESS_QUEUE,int fileSliceSize=configFILE_SLICE_SIZE);

/*
* 从访问队列中获得任务
* sumCheck指定了
*/
int getTask(char* sumCheck);
};

#endif