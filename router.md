# 路由API设置
本部分内容是http服务端与客户端交互的路由设置约定。

如果没有特殊说明，默认为**json**格式。

默认开启主机任意网卡的**8000**号端口。

## 用户账号相关
**更新内容：添加token功能，将会在登录成功时response header中附带token信息，格式如下：**

```json
"Authorization": token
```

其中的token实际为"Bearer " + token串，客户端不必关心，直接存储转发即可。

在改密等用户操作时需要header中携带token，现在**约定ret=-2表示token错误**

用户账号相关，包括登录、登出、改密等相关
### 登录
```
post /api/login
send
{
    user: 账号
    password: 密码
}
return 
{
    ret: 0/-1  --- 成功/失败
    msg: 提示信息
}
```

### 注册
```
post /api/signup
send
{
    user: 账号
    password: 密码
    confirmpassword: 确认密码(前端比较确认后)
}
return
{
    ret: 0/-1  --- 成功/失败
    msg: 提示信息
}
```

### 登出
```
post /api/logout
send
{
    user: 账号
}
return 
{
    ret: 0/-1  --- 成功/失败
    msg: 提示信息
}
```

### 改密
```
post /api/changepass
send
{
    user: 账号
    oldpassword: 旧密码
    newpassword: 新密码
    confirmpassword: 新密码的确认密码
}
return 
{
    ret: 0/-1/-2  --- 成功/失败/token验证失败
    msg: 提示信息
}
```

## 文件相关
写在前面： 文件相关的请求均需要携带token，-2表示token验证失败

路径使用'/'连接，不要连续如'//'

### 文件列表
```
post /api/file/list
send
{
    path: 绝对路径
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
    files: [
        {
            name: 名称
            type: folder/file
            size: 大小(byte)    // 文件夹该参数无效，为0
            time: 上次修改时间 格式为 %Y-%m-%d %H:%M:%S, eg 2022-06-27 21:27:46
        },
        ....
    ] // files 参数在成功时才具有意义
}
```

### 新建文件夹
```
post /api/file/newfolder
send
{
    cwd: 当前工作目录（末尾不加'/'符号）
    foldername: 新建文件夹名称
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
}
```


### 文件重命名
```
post /api/file/rename
send
{
    cwd: 当前工作目录（末尾不加'/'符号）
    oldname: 文件旧名称
    newname: 文件新名称
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
}
```

### 文件删除
```
post /api/file/delete
send
{
    paths: [, ,]    // 文件绝对路径，支持批量删除，对于父目录和子目录都含有的情形（以删除父目录所有内容为准）
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
}
```

### 文件移动
```post /api/file/move
send
{
    oldcwd: 旧当前目录（末尾不加'/'符号）
    newcwd: 新当前目录（末尾不加'/'符号）
    files: [, ,]      需要移动的文件名称
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
}
```

### 文件复制
- 考虑到文件复制会出现同名情况出现，目前暂时以覆盖的方式进行处理

```post /api/file/copy
send
{
    oldcwd: 旧当前目录（末尾不加'/'符号）
    newcwd: 新当前目录（末尾不加'/'符号）
    files: [, ,]      需要移动的文件名称
}
return 
{
    ret: 0/-1/-2   --- 成功/失败/..
    msg: 提示信息
}
```

### 文件上传
- 文件上传和下载为核心模块

下面提出文件上传的具体流程，考虑可能片面，如有问题及时提醒：

#### 文件预上传
上传文件之前，客户端计算文件的md5值，上传服务端，如下：
```
post /api/file/preupload
send
{
    "path": <path/to/file>  // 在个人网盘中的绝对路径
    "size": 文件大小
    "md5": <file md5>       
}
return 
{
    "ret": 0+/-1/-2         // 0+表示上传的切片序号/失败/token非法
                            // 约定 0 表示秒传，正数表示切片序号
    "slicesize": 切片大小
}
```

#### 文件正式上传
下面是文件的正式上传部分，注意文件上传时客户端应该获取到权限，不允许修改文件
```
send
{
    md5: <file md5>
    data: 数据    // 文件数据
    num: 块数序号（从1计数）
}
return 
{
    "ret": 0/-1/-2     --- 成功/失败/token非法
    "msg": 提示信息
    "next": 下一块切片号 --- 0 表示完成
}

```

#### 文件夹上传(deprecated)

为区分文件上传和文件夹上传，提供文件夹预上传接口，但是考虑到已经有新建文件夹接口，所以暂时不采用此方案

```
客户端将要上传的文件夹的结构信息发送到服务端，注意只发送文件夹

post /api/file/uploadfolder
send
{
    folders: {
        folder: [folder1, folder2],  // 本层目录
        folder1: {  // folder1的情况，与folder结构相同
            folder: []

        }
        folder2: {  // folder2的情况，与folder结构相同

        }
        ...
    }
}
return 
{
    "ret": 0/-1/-2    --- 成功/失败/token非法
    "msg": 提示信息
}
```

### 文件下载
TODO: 目前下载采用http自带断点续传功能，但因为服务端并没有完成实现此功能，需要明确过程，方便编写服务端程序

```
考虑到用户同时登录多个平台的可能，服务端只是按照客户端的要求发送数据

post /api/download
send
{
    "method": "download",
    "path": <path/to/file>,    
    "offset": <offset in byte> // 从第几字节开始
}
return 
{
    通过http的相应首部的 Content-Type/Content-Range/Content-Length三个字段回复，body中放置裸二进制数据
}

问题：如果下载文件时服务端文件发生变化，此时已下载内容受损，所以考虑添加Last-Modified字段
```