/***
 * CommandHandler 模块实现
 **/

#include "command_handler.h"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>

#include "httplib.h"
#include "json.hpp"
#include "jwt-cpp/token.hpp"

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
        std::string msg = "login success";

        try {
            auto user = req_body.at("user");
            auto password = req_body.at("password");
            
            // TODO:此处需要数据库处理用户
            // ...

            /* 登录成功需要携带token返回 */
            auto token = Anakin::Token::create();
            token = "Bearer " + token;
            res.set_header("Authorization", token);
        }
        catch (const json::exception& e) {
            cout << e.what() << '\n';
            ret = -1;
            msg = "data error!";
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
        std::string msg = "sign up success";

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
            ret = -1;
            msg = "data error!";
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
            ret = -1;
            msg = "json data error!";
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
    server_.Post("/api/changepass", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "change password success";

        try {
            verify_token(req);

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
            ret = -1;
            msg = "json data error!";
        }
        catch (const Anakin::token_exception& e) {
            cout << e.what() << '\n';
            ret = -2;
            msg = "invalid login";
        }
        
        json res_body;
        res_body["ret"] = ret;
        res_body["msg"] = msg;

        res.set_content(res_body.dump(), "application/json");
    });
}

void CommandHandler::verify_token(const httplib::Request& req) const
{
    const auto token = req.get_header_value("Authorization");

    if (token.substr(0, 7) != "Bearer ") {
        cout << "token is not bearer" << endl;
        throw Anakin::token_exception();
    }

    try {
        Anakin::Token::verify(token.substr(7));
    } catch (const std::exception& e) {
        cout << e.what() << endl;
        throw Anakin::token_exception();
    }
}