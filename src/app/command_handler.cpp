/***
 * CommandHandler 模块实现
 **/

#include "command_handler.h"
#include <bits/types/FILE.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "access_queue.h"
#include "file_system_manager.h"
#include "httplib.h"
#include "json.hpp"
#include "jwt-cpp/token.hpp"
#include "logc/logc.h"
#include "user_info_manager.h"
#include "utility.hpp"
#include <bits/types/time_t.h>
#include <time.h>

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
    : user_manager_(std::make_unique<UserInfoManager>())
{
    // 初始并创建数据库
    if (!initUserManager()) {
        LogC::log_println("userinfomanager init failure.");
        exit(EXIT_FAILURE);
    }

    // 将会进入线程阻塞中
    if (initServer(ip_address, port) < 0) {
        exit(EXIT_FAILURE);
    }
}

CommandHandler::~CommandHandler()
{
    // 析构函数
    server_.stop();
}

int CommandHandler::initServer(const std::string ip_address, const int port)
{
    /* CORS */
    // resolveCORS();
    /* 用户相关路由设置 */
    userRouterConfigure();
    /* 文件路由 */
    fileRouterConfigure();
    /* 测试路由 */
    routeTest();

    LogC::log_printf("cloud-disk service run on %s:%d\n",
        ip_address.c_str(), port);
    if (server_.listen(ip_address.c_str(), port) == false) {
        LogC::log_printf("cloud-disk service listen on %s:%d\n",
            ip_address.c_str(), port);
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
		// if (ret == _UserInfoManager::_RET_SQL_ERR)
        cout << user_manager_->getMysqlError() << endl;
		return 0;
	}
#ifdef __DATABASE_INIT__
	if ((ret = user_manager_->initDatabase())) {
		cout << user_manager_->error(ret) << endl;
		// if (ret == _UserInfoManager::_RET_SQL_ERR)
        cout << user_manager_->getMysqlError() << endl;
		return 0;
	}
#endif
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
    fileDownload();
    filePreUpload();
    fileUpload();
}

/* 解决 CORS 问题 */
void CommandHandler::resolveCORS(std::string route)
{
    server_.Options(route, [this](const Request& req, Response& res){
        res.status = 204;
        res.reason = "No Content";
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS, GET");
    });
}

/* 用户登录 */
void CommandHandler::userLogin()
{
    resolveCORS("/api/login");
    server_.Post("/api/login", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "login success";
        std::string token, _user{};

        try {
            int _ret;
            auto user = req_body.at("user");
            _user = user;
            auto password = req_body.at("password");
            
            // 数据库处理用户
            if ((_ret = user_manager_->check(user, password))) {
                // 获取错误信息
                msg = user_manager_->error(_ret);
                ret = -1;
            } else {
                /* 登录成功需要携带token返回 */
                // 创建token
                token = Anakin::Token::create(user, 100);
                token = "Bearer " + token;
                // res.set_header("Authorization", token);
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
        res_body["token"] = token;

        res.set_content(res_body.dump(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");

        // 日志记录登录信息
        LogC::log_printf("%s user %s login: %s\n", 
            req.remote_addr.c_str(), _user.c_str(), msg.c_str());
    });
}

/* 用户注册 */
void CommandHandler::userSignup()
{
    resolveCORS("/api/signup");
    server_.Post("/api/signup", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "sign up success";
        std::string user{};

        try {
            user = req_body.at("user");
            auto password = req_body.at("password");
            auto confirm_password = req_body.at("confirmpassword");
            if (password != confirm_password) {
                ret = -1;
                msg = "password and confirm password not same";
            }
            else {
                int _ret;
                /// TODO: 需要判断用户是否存在，不存在添加用户
                if ((_ret = user_manager_->add(user, password))) {
                    msg = user_manager_->error(_ret); 
                    ret = -1;
                }
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
        res.set_header("Access-Control-Allow-Origin", "*");
        // 日志记录注册信息
        LogC::log_printf("%s signup: %s\n", 
                        req.remote_addr.c_str(), msg.c_str());
        // 用户注册成功后添加用户根目录
        if (ret == 0) {
            int _ret = file_system_manager().makeFolder(path_join(user, {""}));
            LogC::log_printf("user %s mkdir /%s %s\n", 
                user.c_str(), user.c_str(), file_system_manager().error(_ret));
        }
    });
}

/* 用户登出 */
void CommandHandler::userLogout()
{
    resolveCORS("/api/logout");
    server_.Post("/api/logout", [](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "logout success";

        try {
            auto user = req_body.at("user");
            
            /// TODO:此处需要token过期操作(不过期似乎也可以)
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
        res.set_header("Access-Control-Allow-Origin", "*");
        // 日志记录登出信息
        LogC::log_printf("%s logout: %s\n", 
                        req.remote_addr.c_str(), msg.c_str());
    });
}

/* 用户改密 */
void CommandHandler::userChangepass()
{
    resolveCORS("/api/changepass");
    server_.Post("/api/changepass", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "change password success";

        try {
            auto _user = verify_token(req);
            auto user = req_body.at("user");
            auto old_password = req_body.at("oldpassword");
            auto new_password = req_body.at("newpassword");
            auto confirm_password = req_body.at("confirmpassword");

            if (new_password != confirm_password) {
                ret = -1;
                msg = "new password and confirm password are not same!";
            } 
            else if (_user != user) {
                ret = -1;
                msg = "invalid token";
            }
            else {
                int _ret;
                // 数据库操作
                if ((_ret = user_manager_->change(user, new_password))) {
                    msg = user_manager_->error(_ret);
                    ret = -1;
                }
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
        res.set_header("Access-Control-Allow-Origin", "*");
        // 日志记录登录信息
        LogC::log_printf("%s changepass: %s\n", 
                        req.remote_addr.c_str(), msg.c_str());
    });
}

std::string CommandHandler::verify_token(const httplib::Request& req) const 
{
    const auto token = req.get_header_value("Authorization");

    if (token.substr(0, 7) != "Bearer ") {
        cout << "token is not bearer" << endl;
        throw Anakin::token_exception();
    }

    try {
        auto user = Anakin::Token::verify(token.substr(7));
        return user;
    } catch (const std::exception& e) {
        cout << e.what() << endl;
        throw Anakin::token_exception();
    }
}

// 递归生成文件树
int CommandHandler::generateFileTree(std::string path, int& count, vector<json>& files)
{
    int _ret;
    vector<FNode> list;
    _ret = file_system_manager().list(path, list);
    if (_ret == 0) {
        for (const auto& f : list) {
            json file;
            file["name"] = f.name;
            file["type"] = "folder";
            file["size"] = 0;
            if (f.is_file) {
                file["type"] = "file";
                file["size"] = f.file_size;
            }

            // 设置时间
            char buf[32]{};
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", 
                localtime(&(f.modufy_time)));

            file["time"] = std::string(buf);
            file["id"] = count++;
            
            if (!f.is_file) {
                vector<json> _files;
                generateFileTree(path_join({path, f.name}), count, _files);
                file["children"] = _files;
            }
            files.push_back(file);
        }
    }

    return _ret;
}

/* 文件列表 */
void CommandHandler::fileList()
{
    resolveCORS("/api/file/list");

    server_.Post("/api/file/list", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "list success";
        vector<json> files;
        std::string user{}, path;

        try {
            user = verify_token(req);
            // 获取到绝对路径目录
            path = req_body.at("path");

            // 获取目录下的文件信息
            int _ret;
            int count = 1;
            _ret = generateFileTree(path_join(user, {path}), count, files);
            if (_ret != 0) {
                ret = -1;
                msg = file_system_manager().error(_ret);
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
        res_body["files"] = files;

        res.set_content(res_body.dump(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        LogC::log_printf("%s user %s list %s: %s\n",
            req.remote_addr.c_str(), user.c_str(), 
            path.c_str(), msg.c_str());
    });

}

void CommandHandler::fileNewFolder()
{
    resolveCORS("/api/file/newfolder");
    server_.Post("/api/file/newfolder", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "new folder success";
        std::string user{}, path{};

        try {
            user = verify_token(req);
            // 获取到绝对路径目录
            auto cwd = req_body.at("cwd");
            auto folder_name = req_body.at("foldername");

            // 创建文件夹
            path = path_join(user, {cwd, folder_name});
            int _ret = file_system_manager().makeFolder(path);
            if (_ret != 0) {
                ret = -1;
                msg = file_system_manager().error(_ret);
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
        res.set_header("Access-Control-Allow-Origin", "*");
        LogC::log_printf("%s user %s mkdir %s: %s\n",
            req.remote_addr.c_str(), user.c_str(), path.c_str(), msg.c_str());
    });

}

void CommandHandler::fileRename()
{
    resolveCORS("/api/file/rename");
    server_.Post("/api/file/rename", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "rename success";

        try {
            auto user = verify_token(req);
            // 获取到绝对路径目录
            auto cwd = req_body.at("cwd");
            auto old_name = req_body.at("oldname");
            auto new_name = req_body.at("newname");

            /// @TODO: 文件改名
            int _ret = file_system_manager().move(
                path_join(user, {cwd, old_name}),
                path_join(user, {cwd, new_name})
            );
            // 改名失败
            if (_ret != 0) {
                ret = -1;
                msg = file_system_manager().error(_ret);
            }
            // 日志记录
            LogC::log_printf("%s user %s rename %s to %s in %s: %s\n",
                req.remote_addr.c_str(), user.c_str(), 
                string(old_name).c_str(), string(new_name).c_str(),
                string(cwd).c_str(), msg.c_str());
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
        res.set_header("Access-Control-Allow-Origin", "*");
    });
}

void CommandHandler::fileDelete()
{
    resolveCORS("/api/file/delete");
    server_.Post("/api/file/delete", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "file delete success";

        try {
            auto user = verify_token(req);
            // 获取到绝对路径目录
            auto paths = req_body.at("paths");

            // 批量删除文件
            vector<string> hash_list;
            for (const auto& path : paths) {
                int _ret = file_system_manager().remove(
                    path_join(user, {path}), hash_list
                );
                
                // 日志记录
                LogC::log_printf("%s user %s delete %s: %s\n", 
                    req.remote_addr.c_str(), user.c_str(), string(path).c_str(), 
                    file_system_manager().error(_ret));
                
                if (_ret != 0) {
                    ret = -1;
                    msg = file_system_manager().error(_ret);
                    break;
                }
            }
            /// TODO: 对删除文件的哈希值进行操作，计数为0删除
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
        res.set_header("Access-Control-Allow-Origin", "*");

    });

}

void CommandHandler::fileCopy()
{
    resolveCORS("/api/file/copy");
    server_.Post("/api/file/copy", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "copy success";
        std::string user{}, old_cwd{}, new_cwd{};

        try {
            user = verify_token(req);
            // 获取到绝对路径目录
            old_cwd = req_body.at("oldcwd");
            new_cwd = req_body.at("newcwd");
            auto files = req_body.at("files");

            int _ret;
            // 复制文件操作
            for (const auto& f : files) {
                _ret = file_system_manager().copy(
                    path_join(user, {old_cwd, f}),
                    path_join(user, {new_cwd})
                );

                // 日志记录
                LogC::log_printf("%s user %s copy %s from %s to %s: %s\n", 
                    req.remote_addr.c_str(), user.c_str(), string(f).c_str(),
                    old_cwd.c_str(), new_cwd.c_str(), 
                    file_system_manager().error(_ret));

                // 如果出现错误，则停止复制，但之前操作不回滚
                if (_ret != 0) {
                    ret = -1;
                    msg = file_system_manager().error(_ret);
                    break;
                }
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

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(res_body.dump(), "application/json");
        

    });

}

void CommandHandler::fileMove()
{
    resolveCORS("/api/file/move");
    server_.Post("/api/file/move", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "move success";
        std::string user{}, old_cwd{}, new_cwd{};

        try {
            user = verify_token(req);
            // 获取到绝对路径目录
            old_cwd = req_body.at("oldcwd");
            new_cwd = req_body.at("newcwd");
            auto files = req_body.at("files");

            int _ret;
            // 移动文件操作
            for (const auto& f : files) {
                _ret = file_system_manager().move(
                    path_join(user, {old_cwd, f}),
                    path_join(user, {new_cwd})
                );
                
                // 日志记录
                LogC::log_printf("%s user %s move %s from %s to %s: %s\n", 
                    req.remote_addr.c_str(), user.c_str(), string(f).c_str(),
                    old_cwd.c_str(), new_cwd.c_str(), 
                    file_system_manager().error(_ret));
                
                if (_ret != 0) {
                    ret = -1;
                    msg = file_system_manager().error(_ret);
                    break;
                }
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

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(res_body.dump(), "application/json");

    });

}

// 文件预上传
void CommandHandler::filePreUpload()
{
    resolveCORS("/api/file/preupload");
    server_.Post("/api/file/preupload", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        std::string user{}, path{};
        int ret = 0;
        std::string msg = "preupload success";
        
        try {
            user = verify_token(req);
            // 获取预上传文件等信息
            path = req_body.at("path");
            auto size = req_body.at("size");
            auto md5 = req_body.at("md5");

            int _ret;
            FNode f;
            // 首先检查文件是否重复
            _ret = file_system_manager().getFile(
                path_join(user, {path}), f
            );
            if (_ret == 0) {
                // 当前文件已经存在
                ret = -1;
                msg = "file exited";
            } else {
                // 文件不存在，检查是否满足秒传
                // 根据md5检查是否满足秒传
                bool is_exist;
                file_system_manager().hashExist(md5, is_exist);
                if (!is_exist) {
                    // 如果没有此文件
                    _ret = AccessQueue::Instance().startFileQueue(
                        path_join(user, {path}), md5, size
                    );
                    // 不想判断错误了
                    // 第一次，获取到分配的切片号
                    ret = AccessQueue::Instance().getTask(md5);
                } else {
                    // 文件已存在，秒传
                    file_system_manager().makeFile(
                        path_join(user, {path}), md5, size
                    );
                }
                if (ret == 0) {
                    msg = "file transfer completed in seconds";
                }
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
        res_body["slicesize"] = AccessQueue::FILE_SLICE_SIZE;

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(res_body.dump(), "application/json");
        LogC::log_printf("%s user %s preupload %s: %s\n", 
            req.remote_addr.c_str(), user.c_str(), path.c_str(), 
            msg.c_str());
    });
}

// 文件上传
void CommandHandler::fileUpload()
{
    resolveCORS("/api/file/upload");
    server_.Post("/api/file/upload", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        std::string user{}, md5{};
        int ret = 0, next = 0, num{};
        std::string msg = "upload success";
        
        try {
            user = verify_token(req);
            // 获取上传文件切片信息
            md5 = req_body.at("md5");
            auto data = req_body.at("data");
            num = req_body.at("num");
            // 写入信息，获取下一任务
            next = AccessQueue::Instance().getTask(md5, num, data);
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

        if (next == 0) {
            msg = "upload finish";
        }

        json res_body;
        res_body["ret"] = ret;
        res_body["msg"] = msg;
        res_body["next"] = next;

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(res_body.dump(), "application/json");
        LogC::log_printf("%s user %s upload %s~%d: %s\n", 
            req.remote_addr.c_str(), user.c_str(), md5.c_str(), 
            num, msg.c_str());
    });
}

/**
 * 生成 http Content-Range 字段
 * -1 表示未知
 **/
static std::string make_content_range(
    const long long start=-1, const long long end=-1, const long long size=-1, 
    const std::string unit="bytes"
) {
    std::string field = unit + ' ';
    if (start == -1 || end == -1) {
        field += "*";
    } else {
        field += std::to_string(start) + '-' + std::to_string(end);
    }

    if (size == -1) {
        field += "/*";
    } else {
        field += '/' + std::to_string(size);
    }

    return field;
}

// 文件下载
void CommandHandler::fileDownload()
{
    resolveCORS("/api/download");
    server_.Post("/api/download", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        // 创建一个未知大小Content-Range
        auto content_range = make_content_range();
        
        try {
            // verify_token(req);
            // 获取下载文件等信息
            auto method = req_body.at("method");
            auto path = req_body.at("path");
            auto offset = req_body.at("offset");

            /// TODO: 取出用户 path offset 的文件信息
            auto size = 100;
            std::string res_body = "filebody";
            auto length = res_body.length();
            
            // 设置 type/range/length 字段
            content_range = make_content_range(offset, length, size);
            // 放置 body 数据内容
            res.set_content(res_body, "application/octet-stream");
        }
        catch (const json::exception& e) {
            cout << e.what() << '\n';
        }
        catch (const Anakin::token_exception& e) {
            cout << e.what() << '\n';
        }

        // 设置Content-Range
        res.set_header("Content-Range", content_range);
    });

}


void CommandHandler::routeTest()
{
    server_.Get("/(.*)", [&](const Request& req, Response& res) {
        // res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("Hello World!", "text/plain");
    });
}