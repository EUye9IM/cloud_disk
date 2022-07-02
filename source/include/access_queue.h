/**
* Access Queue �ӿ�����
* Access Queue����ά��һ��д�ļ����У�ÿ�����µ��ϴ��ļ�����������ʱ�����������󲢷�������
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

// #define configMAX_ACCESS_QUEUE 1024
// #define configFILE_SLICE_SIZE 1024 //��λKiB

// ������ʱ�ļ����ļ���
const std::string TEMP = "./temp/";

using size_q = long long;
/*
* Access Queue������
* Ҫ����Access Queueģ�飬���ȴ���һ��AccessQueue����Ȼ�����startAccessQueue������������
*/

class AccessQueue{
public:
    static const int FILE_SLICE_SIZE = 1024 * 1024; // 1M
    // static const int FILE_SLICE_SIZE = 2; // ����
    explicit AccessQueue() = default;
    ~AccessQueue();

    // ���Ϊ��������ֹ�������캯���뿽����ֵ����
    AccessQueue(const AccessQueue&) = delete;
    AccessQueue& operator=(const AccessQueue&) = delete;

    // ����ģʽ
    static AccessQueue& Instance() {
        static AccessQueue aq;
        return aq;
    }
    
    // �ļ��ϴ���Ϣ
    // ����ʱ�̣�һ���ļ���Ƭ�� �ϴ����/�ϴ���/δ��ʼ�ϴ� ����״̬
    // �ṹ�е���Ŵ� 1 ��ʼ���㣬����籣��һ��
    struct FileUploadInfo{
        size_q min_unfinish_num_;    // ����δ��ɵ���Ƭ����С���
        size_q max_allocate_num_;    // �Ѿ������ϴ�����Ƭ��������
        size_q slice_num_;           // ��Ƭ����
        size_q file_size_;           // �ļ���С
        std::vector<std::string> files_;// ���ļ���Ӧ���û����ļ�·��
        std::string status;
    };
    
    // ����һ���ļ��ϴ����У������ļ�·����md5�ʹ�С����С��byte����
    // ���� 0 ��ʾ�ɹ������ظ�����ʾʧ��
    int startFileQueue(std::string path, std::string file_md5, size_q file_size);
    // �Ӷ����л�ȡ���񣬴����ļ�·���͵�ǰ�յ���Ƭ����Լ�����
    // ���ط����������ţ�0 �����ļ��������
    size_q getTask(const std::string file_md5, size_q& count,
        const size_q current_num=0, const std::string data="");

private:
    // �洢�ļ�md5���ļ��ϴ����
    std::map<std::string, FileUploadInfo> upload_;
    // std::mutex m_files_;    // files_����
    std::mutex m_upload_;   // upload_����
};

#endif