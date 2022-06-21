/***
 * CommandHandler 模块实现
 **/

#include "command_handler.h"
#include <cstdlib>
#include <cstring>
#include <string>

#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace httplib;
using namespace std;

/* 调试函数 */
#ifdef __ANAKIN_DEBUG__
void CommandHandlerDebug()
{
    CommandHandler command_handler;
}
#endif

CommandHandler::CommandHandler(const std::string ip_address, const int port)
{
    if (initServer(ip_address, port) < 0) {
        exit(EXIT_FAILURE);
    }
}

int CommandHandler::initServer(const std::string ip_address, const int port)
{
    server_.Get("/hi", [](const Request& req, Response& res) {
        res.set_content("hello", "text/plain");
    });

    /* 用户相关路由设置 */
    userRouterConfigure();

    if (server_.listen(ip_address.c_str(), port) == false) {
        cout << "listen " << ip_address << ":" << port << " failure!\n";
        return -1;
    }

    return 0;
}

/* 用户相关路由 */
void CommandHandler::userRouterConfigure()
{
    userLogin();
    userSignup();
    userChangepass();
    userLogout();

}

/* 用户登录 */
void CommandHandler::userLogin()
{
    server_.Post("/api/login", [](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "登录成功";

        try {
            auto user = req_body.at("user");
            auto password = req_body.at("password");
            
            // TODO:此处需要数据库处理用户
            // ...
        }
        catch (const json::exception& e) {
            cout << e.what() << '\n';
        }

        
        json res_body;
        res_body["ret"] = ret;
        res_body["msg"] = msg;

        res.set_content(res_body.dump(), "application/json");
    });
}

/* 用户注册 */
void CommandHandler::userSignup()
{
    server_.Post("/api/signup", [](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "注册成功";

        try {
            auto user = req_body.at("user");
            auto password = req_body.at("password");
            auto confirm_password = req_body.at("confirmpassword");
            if (password != confirm_password) {
                ret = -1;
                msg = "前后密码不一致！";
            }
            else {
                // TODO:此处需要数据库处理用户
                // ...
            }
        }
        catch (const json::exception& e) {
            cout << e.what() << '\n';
        }

        
        json res_body;
        res_body["ret"] = ret;
        res_body["msg"] = msg;

        res.set_content(res_body.dump(), "application/json");
    });
}

/* 用户登出 */
void CommandHandler::userLogout()
{
    server_.Post("/api/logout", [](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "logout success";

        try {
            auto user = req_body.at("user");
            
            // TODO:此处需要cookie等操作
            // ...
        }
        catch (const json::exception& e) {
            cout << e.what() << '\n';
        }
        
        json res_body;
        res_body["ret"] = ret;
        res_body["msg"] = msg;

        res.set_content(res_body.dump(), "application/json");
    });
}

/* 用户改密 */
void CommandHandler::userChangepass()
{
    server_.Post("/api/changepass", [](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "change password success";

        try {
            auto user = req_body.at("user");
            auto old_password = req_body.at("oldpassword");
            auto new_password = req_body.at("newpassword");
            auto confirm_password = req_body.at("confirmpassword");

            if (new_password != confirm_password) {
                ret = -1;
                msg = "new password and confirm password are not same!";
            }
            else {
                // TODO:此处需要数据库操作
                // ...
            }
            
        }
        catch (const json::exception& e) {
            cout << e.what() << '\n';
        }
        
        json res_body;
        res_body["ret"] = ret;
        res_body["msg"] = msg;

        res.set_content(res_body.dump(), "application/json");
    });
}