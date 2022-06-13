# Liric Mechan 工作报告
## 2022年6月13日更新
在EUye9IM提出的架构设计的基础下，进一步规划了各模块的功能，大致如下：
1. 用户发送的HTTP请求经过httpd转发到CommandHandler模块【应用层】
2. CommandHandler在解析了请求后做出相应行动（查询数据库（UserInfoManager/FileSysManager）或者调用AccessQueue【缓冲层】）
3. AccessQueue将CommandHandler的每一次调用存入命令队列，并协调后返回所需的文件指针
### CommandHandler处理的命令

#### List命令
作用：列出指定文件夹下的所有文件和文件夹
参数：(string)要求List的路径
返回：(json)List结果

#### Upload命令
作用：通知服务器自己想要上传的文件，并获取上传队列id
参数：(string)Sum Check，(string)文件名，(string)文件尺寸，(string)上传路径
返回：(int)上传队列id

#### UploadMore命令
作用：获取需要上传的文件块指针
参数：(int) 上传队列id
返回：(int) 文件块指针

注：文件块指针就是指向要上传的文件分块的指针

#### Transfer命令
作用：传送文件块
参数：(int) 上传队列id，(RAW)文件块
返回：(bool)是否成功

#### 其他命令
其他命令不涉及AccessQueue，暂时先不写。