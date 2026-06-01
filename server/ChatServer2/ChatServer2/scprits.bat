@echo off
:: 切换控制台编码为 UTF-8，解决中文乱码
chcp 65001 >nul

:: 1. 设置你的 vcpkg 工具路径
set PROTOC_EXE="D:\SoftWare\C++\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe"
set PLUGIN_EXE="D:\SoftWare\C++\vcpkg\installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe"

:: 2. 设置目录（⚠️ 重点：结尾千万不要带反斜杠 \，直接用一个点号 .）
set PROTO_DIR=.
set OUT_DIR=.

:: 3. 假设你的 proto 文件名叫 message.proto
set PROTO_FILE=message.proto

echo 正在生成 Protobuf 数据模型代码...
%PROTOC_EXE% -I="%PROTO_DIR%" --cpp_out="%OUT_DIR%" "%PROTO_DIR%\%PROTO_FILE%"

echo 正在生成 gRPC 服务接口代码...
%PROTOC_EXE% -I="%PROTO_DIR%" --grpc_out="%OUT_DIR%" --plugin=protoc-gen-grpc=%PLUGIN_EXE% "%PROTO_DIR%\%PROTO_FILE%"

echo ==========================
echo 生成完毕！
pause