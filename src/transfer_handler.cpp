#include "transfer_handler.h"
#include "utility.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <iostream>

TransferHandler::TransferHandler(std::string rootpath){
    _rootPath=rootpath;
    //建立根目录
    if(access(_rootPath.c_str(),F_OK)==-1){
        if(mkdir(_rootPath.c_str(),S_IRWXU)==-1){
            std::cout<<"Please check user permission.";
            exit(-1);
        }
    }
    if(rootpath.back()!='/') _rootPath.append("/");
}

std::string TransferHandler::createFile(int size,std::string hash){
    std::string fpath=_rootPath;
    /*struct timeval tv;
    struct tm* tfd;
    time_t timenow;
    timenow=time(nullptr);
    tfd=localtime(&timenow);
    gettimeofday(&tv,nullptr);

    fpath.append(std::to_string((*tfd).tm_year));
    if((*tfd).tm_mon<10){
        fpath.append("0");
    }
    fpath.append(std::to_string((*tfd).tm_mon));
    if((*tfd).tm_mday<10){
        fpath.append("0");
    }
    fpath.append(std::to_string((*tfd).tm_mday)).append("/");
    if(access(fpath.c_str(),F_OK)==-1){
        if(mkdir(fpath.c_str(),S_IRWXU)==-1){
            std::cout<<"Please check user permission.";
            exit(-1);
        }
    }*/

    fpath.append(hash);
    int fd;
    if ((fd=open(fpath.c_str(), O_RDWR|O_CREAT),0644)==-1){
        std::cout<<"check permission"<<std::endl;
        exit(-1);
    }
    ftruncate(fd,size);
    close(fd);
    return fpath;
}
int TransferHandler::getFileContent(std::string fpath,int begin,int size,void* content){
    int fd;
    fd=open(fpath.c_str(),O_RDONLY);
    if(fd==0){
        return -1;
    }
    lseek(fd,begin,SEEK_SET);
    read(fd,content,size);
    close(fd);
    return 0;
}
int TransferHandler::removeFile(std::string fpath){
    return remove(fpath.c_str());
}
int TransferHandler::fillFileContent(std::string fpath,int begin,int size,void* content){
    int fd;
    fd=open(fpath.c_str(),O_WRONLY);
    if(fd==0){
        return -1;
    }
    lseek(fd,begin,SEEK_SET);
    write(fd,content,size);
    close(fd);
    return 0;
}

std::string TransferHandler::hashToFPath(std::string hash){
    std::string toRet=_rootPath;
    toRet.append(hash);
    return toRet;
}
std::string TransferHandler::pathToFPath(std::string path){
    std::string toRet=_rootPath;
    FNode fn;
    file_system_manager().getFile(path,fn);
    toRet.append(fn.file_hash);
    return toRet;
}