/***
 * CommandHandler 类声明
 * CommandHandler 作为http server端，接收用户请求，并转发给下层模块
 **/

/* 个人条件编译调试 */
// #define __ANAKIN_DEBUG__

#include "httplib.h"

#ifdef __ANAKIN_DEBUG__
void CommandHandlerDebug();
#endif

class CommandHandler {
public:
    explicit CommandHandler(const std::string="0.0.0.0", const int port=8000);
    /* 禁止拷贝构造函数与拷贝赋值操作 */
    CommandHandler(const CommandHandler&) = delete;
    CommandHandler& operator = (const CommandHandler&) = delete;

protected:
    /* 初始化服务器，绑定地址以及端口，并且配置路由 */
    int initServer(const std::string ip_address, const int port);
    /* 用户相关路由设置 */
    void userRouterConfigure();
    void userLogin();
    void userSignup();
    void userLogout();
    void userChangepass();

private:
    /* http server */
    httplib::Server server_;


};