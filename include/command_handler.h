/***
 * CommandHandler 类声明
 * CommandHandler 作为http server端，接收用户请求，并转发给下层模块
 **/
#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

/* 个人条件编译调试 */
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
    /* 禁止拷贝构造函数与拷贝赋值操作 */
    CommandHandler(const CommandHandler&) = delete;
    CommandHandler& operator = (const CommandHandler&) = delete;

protected:
    /* 初始化服务器，绑定地址以及端口，并且配置路由，注意线程会阻塞 */
    int initServer(const std::string ip_address, const int port);
    /* 解决服务器跨域问题 */
    void resolveCORS(std::string);
    /* 用户相关路由设置 */
    void userRouterConfigure();
    void userLogin();
    void userSignup();
    void userLogout();
    void userChangepass();
    /* 文件路由配置 */
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

    /* 文件挂载 */
    void mountDisk();

    void routeTest();

    // 生成文件树
    int generateFileTree(std::string path, int& count, std::vector<nlohmann::json>&);

    /* 初始化 UserInfoManager */
    int initUserManager();

private:
    /* 验证response的token，抛出token_exception异常 */
    std::string verify_token(const httplib::Request&) const;

    /* http server */
    httplib::Server server_;

    /* user 数据库 */
    std::unique_ptr<UserInfoManager> user_manager_;

};

#endif