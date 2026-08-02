#ifndef __TYPE_H__
#define __TYPE_H__
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <memory>
// 基础数据类型
using  i8  =  int8_t;
using  i16 =  int16_t;
using  i32 =  int32_t;
using  i64 =  int64_t;
using  u8  =  uint8_t;
using  u16 =  uint16_t;
using  u32 =  uint32_t;
using  u64 =  uint64_t;
using  f32 =  float;
using  f64 =  double;

// 字符串
using String = std::string;

// 数组
template <typename T>
using Array = std::vector<T>;

// 哈希表
template <typename K, typename V,
          typename H = std::hash<K>,
          typename E = std::equal_to<K>>
using Map = std::unordered_map<K, V, H, E>;

// 对组
template<typename K ,typename   V>
using Pair = std::pair<K,V>;

// 栈
template<typename T>
using Stack = std::stack<T>;

// 集合
template<typename T>
using Set = std::unordered_set<T>;


// 智能指针
template <typename T>
using SPtr = std::shared_ptr<T>;

template <typename T>
using UPtr = std::unique_ptr<T>;


#endif