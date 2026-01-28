#include <sys/stat.h>
#include <unistd.h>

/* * 这一组函数是为了覆盖 newlib/nosys 中的同名函数，
 * 从而消除 "warning: _write is not implemented and will always fail" 这类告警。
 */

// 1. 哑巴 _write: 假装成功写入了 len 个字节
int _write(int file, char *ptr, int len) {
    (void)file; (void)ptr; // 防止 unused warning
    return len; 
}

// 2. 哑巴 _read: 直接返回 0 (表示 EOF，读不到东西)
int _read(int file, char *ptr, int len) {
    (void)file; (void)ptr; (void)len;
    return 0;
}

// 3. 哑巴 _close: 假装关闭成功
int _close(int file) {
    (void)file;
    return 0;
}

// 4. 哑巴 _lseek: 假装不支持定位，或者就在原地
int _lseek(int file, int ptr, int dir) {
    (void)file; (void)ptr; (void)dir;
    return 0;
}

// 5. 哑巴 _fstat: 假装是一个字符设备
int _fstat(int file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

// 6. 哑巴 _isatty: 假装是终端
int _isatty(int file) {
    (void)file;
    return 1;
}