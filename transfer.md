# Transfer
## 来自CommandHandler的需求
根据目前的http接收的请求，需要传输模块完成的功能如下：

### 文件下载
用户下载时，需要根据用户文件路径`file_path`和字节范围`range`获取到文件数据，返回数据string，以及文件的总长度

### 文件上传
目前关于上传的具体流程还没有确定，不过应该会采用切片块标记的方式，因此传输层的主要任务是将第`num`块数据`data`写入用户文件路径`file_path`对应的实际文件中。初始实际文件时获取大小可以考虑额外接口。

### 文件删除
当计数为0时需要删除文件，通过何种方式通知此层，目前还需考虑设计。

### 关于FileSystemManager
目前在CommandHandler层建立了一个FileSystemManager实例，考虑到Tranfer层可能使用此实例，所以采用了单例模式的方式，在`utility.hpp`相关文件可以查看，通过`file_system_manager()`即可以获取到对象单例，可以直接使用。

### 其他
如果完成了此层的设计，可以考虑完善设计AccessQueue上传模块。
