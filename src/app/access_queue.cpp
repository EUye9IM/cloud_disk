/**
* Access Queue模块
* 用于维护一个写文件队列，每当有新的上传文件请求加入队列时，评估该请求并分配任务。
*/

#include "access_queue.h"
#include <cmath>
#include <fstream>
#include <mutex>
#include <string>
#include <iostream>
#include "logc/logc.h"
#include "utility.hpp"
#include "transfer_handler.h"

using namespace std;

AccessQueue::~AccessQueue()
{
    // 清空 map 空间
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
            1, 0, slice_num,
            file_size, {path}, string(slice_num+1, '0')
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

        // 创建实际文件，调用传输模块
        TransferHandler::Instance().createFile(file_size, file_md5);

    } else {
        // 已经有其他人在上传，记录文件路径即可
        upload_.find(file_md5)->second.files_.push_back(path);
    }

    return 0;
}

size_q AccessQueue::getTask(const std::string file_md5, size_q& count,
            const size_q current_num, const std::string data)
{
    lock_guard<mutex> lock(m_upload_);
    
    auto file = upload_.find(file_md5);
    // 未发现文件传输信息，说明已经由他人上传完成
    if (file == upload_.end()) {
        return 0;
    }

    // 检验是否正确，不正确直接重新上传
    do {
        if (current_num == 0)
            break;
        if (current_num < file->second.slice_num_ &&
            data.length() == AccessQueue::FILE_SLICE_SIZE) {
            break;
        }
        // 最后一片的判断处理
        size_q final_slice_len = file->second.file_size_ % AccessQueue::FILE_SLICE_SIZE;
        if (final_slice_len == 0)
            final_slice_len = AccessQueue::FILE_SLICE_SIZE;
        
        cout << "final slice len:" << final_slice_len << endl;
        if (current_num == file->second.slice_num_ &&
            static_cast<size_q>(data.length()) == final_slice_len) {
            break;
        }

        // 没有通过检验，返回未完成的最小序号
        cout << "min unfinish num: " << file->second.min_unfinish_num_ << endl;
        return file->second.min_unfinish_num_;
    } while (true);
    
    // 完成 current_num 的标记，更新未完成的最小切片号
    if (current_num > 0 && file->second.status[current_num] == '0') {
        // 写入实际文件
        int ret = TransferHandler::Instance().fillFileContent(
            TransferHandler::Instance().hashToFPath(file_md5),
            AccessQueue::FILE_SLICE_SIZE * (current_num - 1),
            data.length(),
            (void*)(data.c_str())
        );
        if (ret < 0) {
            // 写入失败，需要再次上传
            return current_num;
        }

        // 标记为已经完成
        file->second.status[current_num] = '1';
        int num = file->second.min_unfinish_num_;
        while (file->second.status[num] == '1') {
            num++;
        }
        file->second.min_unfinish_num_ = num;
        // 获取完成数目
        count = num -1;
        auto &s = file->second.status;
        for (size_q i = num; i < static_cast<size_q>(s.length()); i++) {
            if (s[i] == '1') {
                count++;
            }
        }
    }

    size_q next_task = 0;   // 接下来需要上传的切片
    // 还有未分配的文件切片
    if (file->second.max_allocate_num_ < file->second.slice_num_) {
        next_task = ++file->second.max_allocate_num_;
    }
    // 已经全部分配，使用最小的未完成的序号
    else {
        next_task = file->second.min_unfinish_num_;
    }

    // 已经全部完成，next_task = 0，同时写入用户文件，销毁临时数据
    // 同时也处理 0 文件
    if (file->second.min_unfinish_num_ > file->second.slice_num_ || 
        file->second.file_size_ == 0) {
        next_task = 0;
        // 写入用户文件系统
        for (const auto& path : file->second.files_) {
            int _ret = file_system_manager().makeFile(
                path, file_md5, file->second.file_size_
            );
            LogC::log_printf("make file %s: %s\n",
                path.c_str(), file_system_manager().error(_ret));
        }
        // 释放空间
        file->second.files_.clear();
        upload_.erase(file);
    }

    return next_task;
}