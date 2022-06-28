/**
* Access Queue模块
* 用于维护一个写文件队列，每当有新的上传文件请求加入队列时，评估该请求并分配任务。
*/

#include "access_queue.h"
#include <cmath>
#include <fstream>
#include <mutex>
#include <string>

using namespace std;

AccessQueue::~AccessQueue()
{
    // 清空 map 空间
    // files_.clear();
    upload_.clear();
}

// 启动一个文件上传队列
// 启用此函数之前，已经表明上传文件服务端网盘并不存在
int AccessQueue::startFileQueue(
    std::string path, std::string file_md5, size_q file_size)
{
    lock_guard<mutex> lock(m_upload_);
    // 查找是否已经在上传队列中
    if (upload_.find(file_md5) == upload_.end()) {
        // 不在上传队列，创建
        size_q slice_num = (file_size + FILE_SLICE_SIZE - 1) / FILE_SLICE_SIZE;

        FileUploadInfo file_info = {
            0, 0, slice_num,
            file_size, {path}, string(slice_num, '0')
        };

        // 记录切片状态，目前采用string记录

        /// TODO: 需要添加文件切片状态的记录文件
        // std::string temp_file = TEMP + file_md5 + '~';
        // std::ofstream fout;
        // fout.open(temp_file, ios::binary);
        // if (!fout.is_open()) {
        //     return -1;  // 创建临时文件失败
        // }
        // auto str = new string(slice_num, '0');
        // fout.write(str->c_str(), slice_num);
        // delete str;
        // fout.close();
        
        upload_.insert(std::make_pair(file_md5, file_info));
    } else {
        // 已经有其他人在上传，记录文件路径即可
        upload_.find(file_md5)->second.files_.push_back(path);
    }

    return 0;
}

size_q AccessQueue::getTask(std::string file_md5, size_q current_num)
{
    lock_guard<mutex> lock(m_upload_);
    
    auto file = upload_.find(file_md5);
    // 未发现文件传输信息，说明已经由他人上传完成
    if (file == upload_.end()) {
        return 0;
    }
    
    size_q next_task = 0;   // 接下来需要上传的切片
    if (file->second.max_allocate_num_ == file->second.slice_num_) {
        
    }
    // 预上传阶段
    if (current_num == 0) {
        ++upload_.find(file_md5)->second.max_allocate_num_;
    }

    return 0;
}