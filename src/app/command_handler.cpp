/***
 * CommandHandler 模块实现
 **/

#include "command_handler.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <mutex>
#include <string>

#include "httplib.h"
#include "json.hpp"
#include "jwt-cpp/token.hpp"
#include "logc/logc.h"
#include "user_info_manager.h"

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

static SqlConfig sql_config = {
    .host = "127.0.0.1",
    .port = 3306,
    .user = "root",
    .pass = "root123",
    .database_name = "disk",
    .charset = "UTF-8",
};

CommandHandler::CommandHandler(const std::string ip_address, const int port)
    : user_manager_(std::make_unique<UserInfoManager>())
{
    // 初始并创建数据库
    /// \TODO: 数据库连接不成功
    // if (!initUserManager()) {
    //     LogC::log_println("userinfomanager init failure.");
    //     exit(EXIT_FAILURE);
    // }

    // 将会进入线程阻塞中
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
    /* 文件路由 */
    fileRouterConfigure();

    if (server_.listen(ip_address.c_str(), port) == false) {
        cout << "listen " << ip_address << ":" << port << " failure!\n";
        return -1;
    }

    return 0;
}

/* 启动并初始化sql服务器 */
int CommandHandler::initUserManager()
{
    // user_manager_ = std::make_unique<UserInfoManager>();

    int ret;
	if ((ret = user_manager_->connect(sql_config))) {
		cout << user_manager_->error(ret) << endl;
		if (ret == _UserInfoManager::_RET_SQL_ERR)
			cout << user_manager_->getMysqlError() << endl;
		return 0;
	}

	if ((ret = user_manager_->initDatabase())) {
		cout << user_manager_->error(ret) << endl;
		if (ret == _UserInfoManager::_RET_SQL_ERR)
			cout << user_manager_->getMysqlError() << endl;
		return 0;
	}

    // 初始化成功
    return 1;
}

/* 用户相关路由 */
void CommandHandler::userRouterConfigure()
{
    userLogin();
    userSignup();
    userChangepass();
    userLogout();
}

/* 文件路由配置 */
void CommandHandler::fileRouterConfigure()
{
    fileList();
    fileNewFolder();
    fileRename();
    fileDelete();
    fileCopy();
    fileMove();
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
            
            /// TODO:此处需要数据库处理用户
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
                /// TODO:此处需要数据库处理用户
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

/* 文件列表 */
void CommandHandler::fileList()
{
    server_.Post("/api/file/list", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "list success";
        vector<json> files;

        try {
            verify_token(req);
            // 获取到绝对路径目录
            auto path = req_body.at("path");

            /// @TODO: 获取目录下的文件信息
            
            /// 测试部分
            // json file;
            // file["name"] = "filename";
            // file["type"] = "file";
            // file["size"] = 1024;
            // file["time"] = "May 2033";
            // files.push_back(file);
            // files.push_back(file);
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
        res_body["files"] = files;

        res.set_content(res_body.dump(), "application/json");
    });

}

void CommandHandler::fileNewFolder()
{
    server_.Post("/api/file/newfolder", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "new folder success";

        try {
            verify_token(req);
            // 获取到绝对路径目录
            auto cwd = req_body.at("cwd");
            auto folder_name = req_body.at("foldername");

            /// @TODO: 创建文件夹
            
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

void CommandHandler::fileRename()
{
    server_.Post("/api/file/rename", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "rename success";

        try {
            verify_token(req);
            // 获取到绝对路径目录
            auto cwd = req_body.at("cwd");
            auto old_name = req_body.at("oldname");
            auto new_name = req_body.at("newname");

            /// @TODO: 文件改名
            
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

void CommandHandler::fileDelete()
{
    server_.Post("/api/file/delete", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "file delete success";

        try {
            verify_token(req);
            // 获取到绝对路径目录
            auto paths = req_body.at("paths");

            /// @TODO: 批量删除文件
            
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

void CommandHandler::fileCopy()
{
    server_.Post("/api/file/copy", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "copy success";

        try {
            verify_token(req);
            // 获取到绝对路径目录
            auto old_cwd = req_body.at("oldcwd");
            auto new_cwd = req_body.at("newcwd");
            auto files = req_body.at("files");

            /// @TODO: 复制文件操作
            
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

void CommandHandler::fileMove()
{
    server_.Post("/api/file/move", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "move success";

        try {
            verify_token(req);
            // 获取到绝对路径目录
            auto old_cwd = req_body.at("oldcwd");
            auto new_cwd = req_body.at("newcwd");
            auto files = req_body.at("files");

            /// @TODO: 移动文件操作
            
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