#ifndef IBDB_PORT_PORT_H
#define IBDB_PORT_PORT_H
/**
 * @brief 跨平台相关代码
 * 
 */
typedef signed char           int8_t;
typedef signed short          int16_t;
typedef signed int            int32_t;
typedef signed long long      int64_t;
typedef unsigned char         uint8_t;
typedef unsigned short        uint16_t;
typedef unsigned int          uint32_t;
typedef unsigned long long    uint64_t;

namespace ibdb {
namespace port {

inline const bool IsLittleEndian() {
    int a = 1;
    if (*(char*)&a == 1) {
      return true;
    } else {
      return false;
    }
}

static const bool kLittleEndian = IsLittleEndian();

enum RpcCode {
    OK = 0,
    FAILED,
    ERROR_CANT_CONNECTION,
    ERROR_NOT_FOUND,
    ERROR_TABLE_DUPLICATE
};

} // port
} // ibdb


#endif // IBDB_PORT_PORT_H