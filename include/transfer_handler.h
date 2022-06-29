#ifndef TRANSFER_HANDLER_H
#define TRANSFER_HANDLER_H
#include <string>
//#include "file_system_manager.h"
/*
* All fpath words in this file refer to the actual paths in real file system.
* toFPath method is designed to convert hashes or virtual file system paths into real file system paths.
*/
class TransferHandler{
public:
std::string createFile(int size,std::string hash);//size in byte

TransferHandler(std::string rootpath);

int getFileContent(std::string fpath,int begin,int size,void* content);

int removeFile(std::string fpath);

int fillFileContent(std::string fpath,int begin,int size,void* content);

std::string hashToFPath(std::string hash);
std::string pathToFPath(std::string path);

private:
std::string _rootPath;
//FileSystemManager& _fs;

};

#endif
