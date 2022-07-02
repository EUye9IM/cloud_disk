/***
 * CommandHandler ������
 * CommandHandler ��Ϊhttp server�ˣ������û����󣬲�ת�����²�ģ��
 **/
#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

/* ��������������� */
#include <vector>
#define __ANAKIN_DEBUG__

#include <string>
#include <memory>
#include "httplib.h"
#include "jwt-cpp/token.hpp"
#include "user_info_manager.h"
#include "file_system_manager.h"
#include "json.hpp"

#ifdef __ANAKIN_DEBUG__
void CommandHandlerDebug();
#endif

class CommandHandler {
public:
    explicit CommandHandler(const std::string="0.0.0.0", const int port=8000);
    ~CommandHandler();
    /* ��ֹ�������캯���뿽����ֵ���� */
    CommandHandler(const CommandHandler&) = delete;
    CommandHandler& operator = (const CommandHandler&) = delete;

protected:
    /* ��ʼ�����������󶨵�ַ�Լ��˿ڣ���������·�ɣ�ע���̻߳����� */
    int initServer(const std::string ip_address, const int port);
    /* ����������������� */
    void resolveCORS(std::string);
    /* �û����·������ */
    void userRouterConfigure();
    void userLogin();
    void userSignup();
    void userLogout();
    void userChangepass();
    /* �ļ�·������ */
    void fileRouterConfigure();
    void fileList();
    void fileNewFolder();
    void fileRename();
    void fileDelete();
    void fileCopy();
    void fileMove();
    void filePreDownload();
    void fileDownload();
    void filePreUpload();
    void fileUpload();

    /* �ļ����� */
    void mountDisk();

    void routeTest();

    // �����ļ���
    int generateFileTree(std::string path, int& count, std::vector<nlohmann::json>&);

    /* ��ʼ�� UserInfoManager */
    int initUserManager();

private:
    /* ��֤response��token���׳�token_exception�쳣 */
    std::string verify_token(const httplib::Request&) const;

    /* http server */
    httplib::Server server_;

    /* user ���ݿ� */
    std::unique_ptr<UserInfoManager> user_manager_;

};

#endif