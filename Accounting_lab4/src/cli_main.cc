#include "cli/cli.h"
#include <iostream>

using namespace accounting;

int main(int argc, char* argv[]) {
    // 可选：从命令行指定数据目录
    std::string data_dir = "data";
    
    if (argc > 1) {
        data_dir = argv[1];
    }

    CLI cli;
    
    if (!cli.Initialize(data_dir)) {
        std::cerr << "\n[错误] 系统初始化失败\n\n";
        return 1;
    }

    cli.Run();
    
    return 0;
}
