/**
 * @file
 * Contains common macros
 * */

 #ifndef SN_COMMON_H_
 #define SN_COMMON_H_

#define SN_MAX(a, b) ((a) > (b) ? (a) : (b))

#define SN_MIN(a, b) ((a) < (b) ? (a) : (b))

#define SN_UNUSED(expr) (void)(expr)

#define SN_ASSERT_COMPILE(val) \
  void sn_common_assert_compile_helper(char x[0 - (!(val))])

 #endif/*SN_COMMON_H_*/
