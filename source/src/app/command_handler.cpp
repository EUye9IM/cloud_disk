/***
 * CommandHandler ģ��ʵ��
 **/

#include "command_handler.h"
#include <bits/types/FILE.h>
#include <algorithm>
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
#include <system_error>
#include <valarray>
#include <vector>

#include "access_queue.h"
#include "file_system_manager.h"
#include "httplib.h"
#include "json.hpp"
#include "jwt-cpp/token.hpp"
#include "logc/logc.h"
#include "transfer_handler.h"
#include "user_info_manager.h"
#include "utility.hpp"
#include <bits/types/time_t.h>
#include <time.h>

using json = nlohmann::json;
using namespace httplib;
using namespace std;


/* ���Ժ��� */
#ifdef __ANAKIN_DEBUG__
void CommandHandlerDebug()
{
    CommandHandler command_handler;
}
#endif

CommandHandler::CommandHandler(const std::string ip_address, const int port)
    : user_manager_(std::make_unique<UserInfoManager>())
{
    // ��ʼ���������ݿ�
    if (!initUserManager()) {
        LogC::log_println("userinfomanager init failure.");
        exit(EXIT_FAILURE);
    }

    // ��������߳�������
    if (initServer(ip_address, port) < 0) {
        exit(EXIT_FAILURE);
    }
}

CommandHandler::~CommandHandler()
{
    // ��������
    server_.stop();
}

int CommandHandler::initServer(const std::string ip_address, const int port)
{
    /* CORS */
    server_.set_payload_max_length(1024 * 1024 * 512); // 512MB
    // resolveCORS();
    /* �û����·������ */
    userRouterConfigure();
    /* �ļ�·�� */
    fileRouterConfigure();
    /* ����·�� */
    routeTest();

    // mountDisk();

    LogC::log_printf("cloud-disk service run on %s:%d\n",
        ip_address.c_str(), port);
    if (server_.listen(ip_address.c_str(), port) == false) {
        LogC::log_printf("cloud-disk service listen on %s:%d\n",
            ip_address.c_str(), port);
        return -1;
    }

    return 0;
}

/* ��������ʼ��sql������ */
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
    // ��ʼ���ɹ�
    return 1;
}


/* �û����·�� */
void CommandHandler::userRouterConfigure()
{
    userLogin();
    userSignup();
    userChangepass();
    userLogout();
}

/* �ļ�·������ */
void CommandHandler::fileRouterConfigure()
{
    fileList();
    fileNewFolder();
    fileRename();
    fileDelete();
    fileCopy();
    fileMove();
    filePreDownload();
    fileDownload();
    filePreUpload();
    fileUpload();
}

/* ��� CORS ���� */
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

/* �û���¼ */
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
            
            // ���ݿ⴦���û�
            if ((_ret = user_manager_->check(user, password))) {
                // ��ȡ������Ϣ
                msg = user_manager_->error(_ret);
                ret = -1;
            } else {
                /* ��¼�ɹ���ҪЯ��token���� */
                // ����token
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

        // ��־��¼��¼��Ϣ
        LogC::log_printf("%s user %s login: %s\n", 
            req.remote_addr.c_str(), _user.c_str(), msg.c_str());
    });
}

static bool passwordCheck(const std::string pass, const int min_len,
    const int min_kinds)
{
    if (pass.length() < size_t(min_len))
        return false;
    int symbol[4]{}, count = 0;    // ���δ�����д��Сд�����ֺ���������
    for (const auto& s : pass) {
        if (s >= 'A' && s <= 'Z')
            symbol[0]++;
        else if (s >= 'a' && s <= 'z') 
            symbol[1]++;
        else if (s >= '0' && s <= '9')
            symbol[2]++;
        else 
            symbol[3]++;
    }

    for (int i = 0; i < 4; i++) {
        if (symbol[i])
            count++;
    }
    if (count >= min_kinds)
        return true;
    return false;
}

/* �û�ע�� */
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
            else if (!passwordCheck(password, 12, 3)) {
                ret = -1;
                msg = "password is not strong";
            }
            else {
                int _ret;
                /// TODO: ��Ҫ�ж��û��Ƿ���ڣ������������û�
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
        // ��־��¼ע����Ϣ
        LogC::log_printf("%s signup: %s\n", 
                        req.remote_addr.c_str(), msg.c_str());
        // �û�ע��ɹ��������û���Ŀ¼
        if (ret == 0) {
            int _ret = file_system_manager().makeFolder(path_join(user, {""}));
            LogC::log_printf("user %s mkdir /%s %s\n", 
                user.c_str(), user.c_str(), file_system_manager().error(_ret));
        }
    });
}

/* �û��ǳ� */
void CommandHandler::userLogout()
{
    resolveCORS("/api/logout");
    server_.Post("/api/logout", [](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "logout success";

        try {
            auto user = req_body.at("user");
            
            /// TODO:�˴���Ҫtoken���ڲ���(�������ƺ�Ҳ����)
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
        // ��־��¼�ǳ���Ϣ
        LogC::log_printf("%s logout: %s\n", 
                        req.remote_addr.c_str(), msg.c_str());
    });
}

/* �û����� */
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
                // ���ݿ����
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
        // ��־��¼��¼��Ϣ
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

// �ݹ������ļ���
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

            // ����ʱ��
            char buf[32]{};
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", 
                localtime(&(f.modify_time)));

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

/* �ļ��б� */
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
            // ��ȡ������·��Ŀ¼
            path = req_body.at("path");

            // ��ȡĿ¼�µ��ļ���Ϣ
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
            // ��ȡ������·��Ŀ¼
            auto cwd = req_body.at("cwd");
            auto folder_name = req_body.at("foldername");

            // �����ļ���
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
            // ��ȡ������·��Ŀ¼
            auto cwd = req_body.at("cwd");
            auto old_name = req_body.at("oldname");
            auto new_name = req_body.at("newname");

            /// @TODO: �ļ�����
            int _ret = file_system_manager().move(
                path_join(user, {cwd, old_name}),
                path_join(user, {cwd, new_name})
            );
            // ����ʧ��
            if (_ret != 0) {
                ret = -1;
                msg = file_system_manager().error(_ret);
            }
            // ��־��¼
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
            // ��ȡ������·��Ŀ¼
            auto paths = req_body.at("paths");

            // ����ɾ���ļ�
            vector<string> hash_list;
            for (const auto& path : paths) {
                int _ret = file_system_manager().remove(
                    path_join(user, {path}), hash_list
                );
                
                // ��־��¼
                LogC::log_printf("%s user %s delete %s: %s\n", 
                    req.remote_addr.c_str(), user.c_str(), string(path).c_str(), 
                    file_system_manager().error(_ret));
                
                if (_ret != 0) {
                    ret = -1;
                    msg = file_system_manager().error(_ret);
                    break;
                }
            }
            // ��ɾ���ļ��Ĺ�ϣֵ���в���������Ϊ0ɾ��
            // hash_list.erase(std::unique(hash_list.begin(), hash_list.end()), 
            //     hash_list.end());

            for (const auto& hash : hash_list) {
                bool is_exist;
                int _ret = file_system_manager().hashExist(hash, is_exist);
                if (_ret == 0 && !is_exist) {
                    // �ļ����û�������ɾ��
                    TransferHandler::Instance().removeFile(
                        TransferHandler::Instance().hashToFPath(hash)
                    );
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
            // ��ȡ������·��Ŀ¼
            old_cwd = req_body.at("oldcwd");
            new_cwd = req_body.at("newcwd");
            auto files = req_body.at("files");

            int _ret;
            // �����ļ�����
            for (const auto& f : files) {
                _ret = file_system_manager().copy(
                    path_join(user, {old_cwd, f}),
                    path_join(user, {new_cwd})
                );

                // ��־��¼
                LogC::log_printf("%s user %s copy %s from %s to %s: %s\n", 
                    req.remote_addr.c_str(), user.c_str(), string(f).c_str(),
                    old_cwd.c_str(), new_cwd.c_str(), 
                    file_system_manager().error(_ret));

                // ������ִ�����ֹͣ���ƣ���֮ǰ�������ع�
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
            // ��ȡ������·��Ŀ¼
            old_cwd = req_body.at("oldcwd");
            new_cwd = req_body.at("newcwd");
            auto files = req_body.at("files");

            int _ret;
            // �ƶ��ļ�����
            for (const auto& f : files) {
                _ret = file_system_manager().move(
                    path_join(user, {old_cwd, f}),
                    path_join(user, {new_cwd})
                );
                
                // ��־��¼
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

// �ļ�Ԥ�ϴ�
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
            // ��ȡԤ�ϴ��ļ�����Ϣ
            path = req_body.at("path");
            auto size = req_body.at("size");
            auto md5 = req_body.at("md5");

            int _ret;
            FNode f;
            // ���ȼ���ļ��Ƿ��ظ�
            _ret = file_system_manager().getFile(
                path_join(user, {path}), f
            );
            if (_ret == 0) {
                // ��ǰ�ļ��Ѿ�����
                ret = -1;
                msg = "file exited";
            } else {
                // �ļ������ڣ�����Ƿ������봫
                // ����md5����Ƿ������봫
                bool is_exist;
                file_system_manager().hashExist(md5, is_exist);
                if (!is_exist) {
                    // ���û�д��ļ�
                    _ret = AccessQueue::Instance().startFileQueue(
                        path_join(user, {path}), md5, size
                    );
                    // �����жϴ�����
                    // ��һ�Σ���ȡ���������Ƭ��
                    size_q count{};
                    ret = AccessQueue::Instance().getTask(md5, count);
                } else {
                    // �ļ��Ѵ��ڣ��봫
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

// �ļ��ϴ�
void CommandHandler::fileUpload()
{
    resolveCORS("/api/file/upload");
    server_.Post("/api/file/upload", [this](const Request& req, Response& res) {
        std::string user{}, md5{};
        int ret = 0, next = 0, num{};
        std::string msg = "upload success";
        size_q count{};
        
        try {
            user = verify_token(req);
            // ��ȡ�ϴ��ļ���Ƭ��Ϣ
            md5 = req.get_param_value("md5");
            num = stoi(req.get_param_value("num"));
            
            auto data = req.body;
            // д����Ϣ����ȡ��һ����
            next = AccessQueue::Instance().getTask(md5, count, num, data);
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
        res_body["count"] = count;

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(res_body.dump(), "application/json");
        LogC::log_printf("%s user %s upload %s~%d: %s file has %lld block now.\n", 
            req.remote_addr.c_str(), user.c_str(), md5.c_str(), 
            num, msg.c_str(), count);
    });
}

/**
 * ���� http Content-Range �ֶ�
 * -1 ��ʾδ֪
 **/
// static std::string make_content_range(
//     const long long start=-1, const long long end=-1, const long long size=-1, 
//     const std::string unit="bytes"
// ) {
//     std::string field = unit + ' ';
//     if (start == -1 || end == -1) {
//         field += "*";
//     } else {
//         field += std::to_string(start) + '-' + std::to_string(end);
//     }

//     if (size == -1) {
//         field += "/*";
//     } else {
//         field += '/' + std::to_string(size);
//     }

//     return field;
// }

// �ļ�Ԥ����
void CommandHandler::filePreDownload()
{
    resolveCORS("/api/predownload");
    server_.Post("/api/predownload", [this](const Request& req, Response& res) {
        auto req_body = json::parse(req.body);
        int ret = 0;
        std::string msg = "predownload success";
        std::string hash{};
        size_q size{};
        
        try {
            auto user = verify_token(req);
            // ��ȡ�����ļ�����Ϣ
            std::string path = req_body.at("path");
            FNode f;
            int _ret = file_system_manager().getFile(
                path_join(user, {path}), f
            );

            if (_ret == 0) {
                // ��ѯ�ɹ�
                // url = "/download/" + f.file_hash;
                hash = f.file_hash;
                size = f.file_size;
            } else {
                ret = -1;
                msg = file_system_manager().error(_ret);
            }
            LogC::log_printf("%s user %s predownload %s get url: %s\n", 
                req.remote_addr.c_str(), user.c_str(), path.c_str(), msg.c_str());
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
        res_body["hash"] = hash;
        res_body["size"] = size;

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(res_body.dump(), "application/json");
    });

}

 
// static long long get_file_size(const char * filename)
// {
//     FILE *fp=fopen(filename,"r");  
//     if(!fp) return -1;  
//     fseek(fp,0L,SEEK_END);  
//     int size=ftell(fp);  
//     fclose(fp);  
      
//     return size; 
// }

// �ļ�����
void CommandHandler::fileDownload()
{
    resolveCORS("/api/download");
    server_.Post("/api/download", [this](const Request& req, Response& res) {
        std::string user{}, md5{};
        int ret = 0;
        size_q offset = 0, length = 0;

        std::string msg = "download success";
        
        try {
            // user = verify_token(req);
            // ��ȡ�ϴ��ļ���Ƭ��Ϣ
            md5 = req.get_param_value("md5");
            offset = stoll(req.get_param_value("offset"));
            length = stoll(req.get_param_value("length"));
            // ��ȡ�ļ�����
            // ����md5ֵ�����ļ���С
            string file_path = TransferHandler::Instance().hashToFPath(md5);

            char* buf = new char[length];
            int _ret = TransferHandler::Instance().getFileContent(file_path, offset, length, buf);
            if (_ret == 0) {
                // д��body��
                res.body = string(buf, length);
                // ����content-range
                res.set_header("Accept-Ranges", "bytes");
                res.set_header("Content-Type", "application/octet-stream");
                
                LogC::log_printf("%s user %s download file %s %lld bytes offset %lld length %lld\n", 
                    req.remote_addr.c_str(), user.c_str(), 
                    md5.c_str(), res.body.length(), offset, length);
            } else {
                // nothing
                ret = -1;
            }
            delete []buf;
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
        if (ret < 0) {
            res.status = 404;
        }
        res.set_header("Access-Control-Allow-Origin", "*");
    });
}

// void CommandHandler::fileDownload()
// {
//     static const long long MAX_DOWNLOAD_LEN = 1024 * 1024 * 100;
//     resolveCORS(R"(/download/(.*))");
//     resolveCORS("/(.*)");
//     server_.Get(R"(/download/(.*))", [this](const Request& req, Response& res) {
//         // auto req_body = json::parse(req.body);
//         int ret = 0;
//         std::string msg = "download success";
//         string user{}, file_md5;
//         string range{};
//        	long long byte_beg = 0, byte_end = -1;
//         if (req.has_header("Range")) {
//             range = req.get_header_value("Range");
//             sscanf(range.c_str(), "bytes=%lld", &byte_beg);
//             sscanf(range.c_str(), "bytes=%*[^-]-%lld", &byte_end); 
//             cout << "read range: " << byte_beg << " - " << byte_end << endl;
//         }

//         /// TODO: file download
//         try {
//             user = verify_token(req);
//             // ��ȡ�����ļ�����Ϣ
//             file_md5 = req.path.substr(string("/download/").length());
//             // ����md5ֵ�����ļ���С
//             string file_path = TransferHandler::Instance().hashToFPath(file_md5);
//             auto file_size = get_file_size(file_path.c_str());
//             if (byte_end > file_size || byte_end == -1) {
//                 byte_end = file_size;
//             }

//             if (byte_end - byte_beg + 1 > MAX_DOWNLOAD_LEN) {
//                 byte_end = byte_beg -1 + MAX_DOWNLOAD_LEN;
//             }

//             char* buf = new char[byte_end - byte_beg + 1];
//             int _ret = TransferHandler::Instance().getFileContent(file_path, byte_beg,
//                 byte_end - byte_beg + 1, buf);
            
//             if (_ret == 0) {
//                 // д��body��
//                 res.body = string(buf, byte_end - byte_beg + 1);
//                 // ����content-range
//                 res.set_header("Accept-Ranges", "bytes");
//                 res.set_header("Content-Range",
//                     make_content_range(byte_beg, byte_end, file_size));
//                 res.set_header("Content-Length", to_string(res.body.length()));
//                 res.set_header("Content-Type", "application/octet-stream");
                
//                 LogC::log_printf("%s user %s download file %s %lld bytes from %lld to %lld\n", 
//                     req.remote_addr.c_str(), user.c_str(), 
//                     file_md5.c_str(), res.body.length(), byte_beg, byte_end);
//             } else {
//                 // nothing
//                 ret = -1;
//             }
//             delete []buf;
//         }
//         catch (const json::exception& e) {
//             cout << e.what() << '\n';
//             ret = -1;
//             msg = "json data error!";
//         }
//         catch (const Anakin::token_exception& e) {
//             cout << e.what() << '\n';
//             ret = -2;
//             msg = "invalid login";
//         }

//         res.set_header("Access-Control-Allow-Origin", "*");
//         if (ret < 0) {
//             res.status = 404;
//             LogC::log_printf("%s user %s download file %s: %s\n", 
//                 req.remote_addr.c_str(), user.c_str(), 
//                 file_md5.c_str(), msg.c_str());
//         }
//     });
// }


void CommandHandler::mountDisk()
{
    // ����Ŀ¼
    if (access(ROOT_PATH.c_str(), F_OK) == -1) {
        if (mkdir(ROOT_PATH.c_str(), S_IRWXU) == -1) {
            LogC::log_printf("user permission deny.\n");
            exit(-1);
        }
    }
    server_.set_mount_point("/download", ROOT_PATH);
    resolveCORS(R"(/download/(.*))");
    resolveCORS("/(.*)");
    server_.set_file_request_handler([this](const Request &req, Response &res) {
        int ret = 0;
        std::string msg = "download success";
        std::string user{}, file_md5{};
        try {
            file_md5 = req.path.substr(string("/download/").length());
            // user = verify_token(req);
            user = "someone";

            // �����ж��û��Ƿ�ӵ�д��ļ�
            // bool is_include;
            // int _ret = file_system_manager().include(
            //     path_join(user, {"/"}), file_md5, is_include
            // );
            // if (_ret == 0 && is_include) {
            //     // �û�ȷʵӵ�д��ļ�

            // } 
            
            LogC::log_printf("%s user %s download file %s %lld bytes\n", 
                req.remote_addr.c_str(), user.c_str(), 
                file_md5.c_str(), res.body.length());
            
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
        if (ret < 0) {
            // �����˾ͱ����õ���ȷ��Ϣ
            res.body = msg;
            LogC::log_printf("%s user %s download file %s: %s\n", 
                req.remote_addr.c_str(), user.c_str(), 
                file_md5.c_str(), msg.c_str());
        } else {
            // ��Ϣ��ȷ������disposition
            string disposition = "attachment;filename=" + file_md5;
            res.set_header("Content-Disposition", disposition);
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Content-Type", "application/octet-stream");
        }

    });
}


void CommandHandler::routeTest()
{
    server_.Get(R"(/numbers/(.*))", [&](const Request& req, Response& res) {
        auto numbers = req.matches[1];
        res.set_content(numbers, "text/plain");
    });
    }