/**
* Access Queueģ��
* ����ά��һ��д�ļ����У�ÿ�����µ��ϴ��ļ�����������ʱ�����������󲢷�������
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
    // ��� map �ռ�
    upload_.clear();
}

// ����һ���ļ��ϴ�����
// ���ô˺���֮ǰ���Ѿ������ϴ��ļ���������̲�������
int AccessQueue::startFileQueue(
    std::string path, std::string file_md5, size_q file_size)
{
    lock_guard<mutex> lock(m_upload_);
    // �����Ƿ��Ѿ����ϴ�������
    if (upload_.find(file_md5) == upload_.end()) {
        // �����ϴ����У�����
        size_q slice_num = (file_size + FILE_SLICE_SIZE - 1) / FILE_SLICE_SIZE;

        FileUploadInfo file_info = {
            1, 0, slice_num,
            file_size, {path}, string(slice_num+1, '0')
        };

        // ��¼��Ƭ״̬��Ŀǰ����string��¼

        /// TODO: ��Ҫ����ļ���Ƭ״̬�ļ�¼�ļ�
        // std::string temp_file = TEMP + file_md5 + '~';
        // std::ofstream fout;
        // fout.open(temp_file, ios::binary);
        // if (!fout.is_open()) {
        //     return -1;  // ������ʱ�ļ�ʧ��
        // }
        // auto str = new string(slice_num, '0');
        // fout.write(str->c_str(), slice_num);
        // delete str;
        // fout.close();
        
        upload_.insert(std::make_pair(file_md5, file_info));

        // ����ʵ���ļ������ô���ģ��
        TransferHandler::Instance().createFile(file_size, file_md5);

    } else {
        // �Ѿ������������ϴ�����¼�ļ�·������
        upload_.find(file_md5)->second.files_.push_back(path);
    }

    return 0;
}

size_q AccessQueue::getTask(const std::string file_md5, size_q& count,
            const size_q current_num, const std::string data)
{
    lock_guard<mutex> lock(m_upload_);
    
    auto file = upload_.find(file_md5);
    // δ�����ļ�������Ϣ��˵���Ѿ��������ϴ����
    if (file == upload_.end()) {
        return 0;
    }

    // �����Ƿ���ȷ������ȷֱ�������ϴ�
    do {
        if (current_num == 0)
            break;
        if (current_num < file->second.slice_num_ &&
            data.length() == AccessQueue::FILE_SLICE_SIZE) {
            break;
        }
        // ���һƬ���жϴ���
        size_q final_slice_len = file->second.file_size_ % AccessQueue::FILE_SLICE_SIZE;
        if (final_slice_len == 0)
            final_slice_len = AccessQueue::FILE_SLICE_SIZE;
        
        cout << "final slice len:" << final_slice_len << endl;
        if (current_num == file->second.slice_num_ &&
            static_cast<size_q>(data.length()) == final_slice_len) {
            break;
        }

        // û��ͨ�����飬����δ��ɵ���С���
        cout << "min unfinish num: " << file->second.min_unfinish_num_ << endl;
        return file->second.min_unfinish_num_;
    } while (true);
    
    // ��� current_num �ı�ǣ�����δ��ɵ���С��Ƭ��
    if (current_num > 0 && file->second.status[current_num] == '0') {
        // д��ʵ���ļ�
        int ret = TransferHandler::Instance().fillFileContent(
            TransferHandler::Instance().hashToFPath(file_md5),
            AccessQueue::FILE_SLICE_SIZE * (current_num - 1),
            data.length(),
            (void*)(data.c_str())
        );
        if (ret < 0) {
            // д��ʧ�ܣ���Ҫ�ٴ��ϴ�
            return current_num;
        }

        // ���Ϊ�Ѿ����
        file->second.status[current_num] = '1';
        int num = file->second.min_unfinish_num_;
        while (file->second.status[num] == '1') {
            num++;
        }
        file->second.min_unfinish_num_ = num;
        // ��ȡ�����Ŀ
        count = num -1;
        auto &s = file->second.status;
        for (size_q i = num; i < static_cast<size_q>(s.length()); i++) {
            if (s[i] == '1') {
                count++;
            }
        }
    }

    size_q next_task = 0;   // ��������Ҫ�ϴ�����Ƭ
    // ����δ������ļ���Ƭ
    if (file->second.max_allocate_num_ < file->second.slice_num_) {
        next_task = ++file->second.max_allocate_num_;
    }
    // �Ѿ�ȫ�����䣬ʹ����С��δ��ɵ����
    else {
        next_task = file->second.min_unfinish_num_;
    }

    // �Ѿ�ȫ����ɣ�next_task = 0��ͬʱд���û��ļ���������ʱ����
    // ͬʱҲ���� 0 �ļ�
    if (file->second.min_unfinish_num_ > file->second.slice_num_ || 
        file->second.file_size_ == 0) {
        next_task = 0;
        // д���û��ļ�ϵͳ
        for (const auto& path : file->second.files_) {
            int _ret = file_system_manager().makeFile(
                path, file_md5, file->second.file_size_
            );
            LogC::log_printf("make file %s: %s\n",
                path.c_str(), file_system_manager().error(_ret));
        }
        // �ͷſռ�
        file->second.files_.clear();
        upload_.erase(file);
    }

    return next_task;
}