#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

// 列出指定目录的内容
void ls(const char *path) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[1024];

    printf("列出目录: %s\n", path);
    printf("------------------------------------\n");
    
    // 打开目录
    dir = opendir(path);
    if (dir == NULL) {
        printf("无法打开目录 '%s': %s\n", path, strerror(errno));
        return;
    }
    
    // 读取目录条目
    while ((entry = readdir(dir)) != NULL) {
        // 构建完整路径
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        // 获取文件信息
        if (stat(full_path, &file_stat) == -1) {
            printf("  %-20s [错误: %s]\n", entry->d_name, strerror(errno));
            continue;
        }
        
        // 显示文件类型和名称
        printf("  ");
        if (S_ISDIR(file_stat.st_mode)) {
            printf("[目录] ");
        } else if (S_ISREG(file_stat.st_mode)) {
            printf("[文件] ");
        } else if (S_ISLNK(file_stat.st_mode)) {
            printf("[链接] ");
        } else if (S_ISCHR(file_stat.st_mode)) {
            printf("[字符设备] ");
        } else if (S_ISBLK(file_stat.st_mode)) {
            printf("[块设备] ");
        } else if (S_ISFIFO(file_stat.st_mode)) {
            printf("[管道] ");
        } else if (S_ISSOCK(file_stat.st_mode)) {
            printf("[套接字] ");
        } else {
            printf("[未知] ");
        }
        
        printf("%-20s %10ld 字节\n", entry->d_name, file_stat.st_size);
    }
    
    // 关闭目录
    closedir(dir);
    printf("\n");
}

int main() {
    // 列出根目录内容
    ls("/");
    
    // 列出设备目录内容
    ls("/dev");
    
    return 0;
}