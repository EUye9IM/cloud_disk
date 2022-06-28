/**
* Access Queue 接口声明
* Access Queue用于维护一个写文件队列，每当有新的上传文件请求加入队列时，评估该请求并分配任务。
*/
#ifndef ACCESS_QUEUE_H
#define ACCESS_QUEUE_H

#include <mysql/mysql.h>
#include <sys/types.h>
#include <cstddef>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#define configMAX_ACCESS_QUEUE 1024
#define configFILE_SLICE_SIZE 1024 //单位KiB

// 放置临时文件的文件夹
const std::string TEMP = "./temp/";

using size_q = long long;
/*
* Access Queue类声明
* 要启动Access Queue模块，首先创建一个AccessQueue对象，然后调用startAccessQueue函数来启动。
*/

class AccessQueue{
public:
    static const int FILE_SLICE_SIZE = 1024 * 1024; // 1M
    explicit AccessQueue() = default;
    ~AccessQueue();

    // 设计为单例，禁止拷贝构造函数与拷贝赋值函数
    AccessQueue(const AccessQueue&) = delete;
    AccessQueue& operator=(const AccessQueue&) = delete;

    
    // 文件上传信息
    // 任意时刻，一个文件切片有 上传完成/上传中/未开始上传 三种状态
    struct FileUploadInfo{
        size_q min_unfinish_num_;    // 传输未完成的切片的最小序号
        size_q max_allocate_num_;    // 已经分配上传的切片的最大序号
        size_q slice_num_;           // 切片总数
        size_q file_size_;           // 文件大小
        std::vector<std::string> files_;// 该文件对应的用户的文件路径
        std::string status;
    };
    
    // 启动一个文件上传队列，传入文件路径、md5和大小，大小以byte计数
    // 返回 0 表示成功，返回负数表示失败
    int startFileQueue(std::string path, std::string file_md5, size_q file_size);
    // 从队列中获取任务，传入文件路径和当前收到切片序号，返回分配的任务序号
    size_q getTask(std::string file_md5, size_q current_num=0);

    /* 
    * 启动Access Queue线程
    * maxQueueAmount指定了服务器最大可同时接收的文件数量
    * fileSliceSize指定了文件传输切片的大小，单位为KiB
    */
    // void startAccessQueue(int maxQueueAmount=configMAX_ACCESS_QUEUE,int fileSliceSize=configFILE_SLICE_SIZE);

    /*
    * 从访问队列中获得任务
    * sumCheck指定了
    */
    // int getTask(char* sumCheck);

private:
    // 存储文件路径与文件md5的信息，不需要每次查询数据库
    // std::map<std::string, std::string> files_;
    // 存储文件md5与文件上传情况
    std::map<std::string, FileUploadInfo> upload_;
    // std::mutex m_files_;    // files_加锁
    std::mutex m_upload_;   // upload_加锁
};

#endif