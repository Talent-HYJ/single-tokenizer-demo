#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1

#include <cstring>
#include <iostream>
#include <vector>
#include <new>

struct MyTokenizer
{
  std::vector<std::string> words;
  size_t current;
};

// 函数：将UTF-8字符串分割为单个字符
static std::vector<std::string> split_utf8_string(const std::string &input)
{
  std::vector<std::string> result;
  for (size_t i = 0; i < input.length();)
  {
    unsigned char c = input[i];
    size_t char_len = 1;

    // 判断UTF-8字符的字节长度
    if (c >= 0xF0)
    { // 4字节字符
      char_len = 4;
    }
    else if (c >= 0xE0)
    { // 3字节字符
      char_len = 3;
    }
    else if (c >= 0xC0)
    { // 2字节字符
      char_len = 2;
    }

    // 从原字符串中截取一个完整的字符（字节序列）
    result.push_back(input.substr(i, char_len));
    i += char_len; // 移动到下一个字符
  }
  return result;
}
/**
 * 该函数用于分配和初始化标记器实例。标记器实例需要实际标记文本。
纠错
传递给此函数的第一个参数是当fts5_tokenizer对象使用FTS5（xCreateTokenizer（）的第三个参数）注册时，应用程序提供的（void *）指针的副本。第二个和第三个参数是一个由nul结尾的字符串组成的数组，包含标记器参数（如果有），作为用于创建FTS5表的CREATE VIRTUAL TABLE语句的一部分，在标记器名称后指定
最后一个参数是一个输出变量。如果成功，应将（* ppOut）设置为指向新的标记器句柄并返回SQLITE_OK。如果发生错误，则应返回SQLITE_OK以外的值。在这种情况下，fts5假定* ppOut的最终值未定义
 */
extern "C" int fts5_single_xCreate(void *, const char **azArg, int nArg, Fts5Tokenizer **ppOut)
{

  *ppOut = (Fts5Tokenizer *)new MyTokenizer();

  return SQLITE_OK;
}

/**
 * 调用此函数来删除先前使用 xCreate() 分配的标记器句柄。Fts5 保证每次成功调用 xCreate() 时都会调用此函数一次
 */
extern "C" void fts5_single_xDelete(Fts5Tokenizer *pTokenizer)
{
  MyTokenizer *tokenizer = (MyTokenizer *)pTokenizer;
  delete tokenizer;
}

//  int (*xToken)(
//     void *pCtx,          /* xTokenize() 的第二个参数的副本 */
//     int tflags,          /* FTS5_TOKEN_* 标志的掩码 */
//   const char *pToken, /* 指向包含token的缓冲区的指针 */
//    int nToken,          /* token的大小（以字节为单位） */
//    int iStart,          /* 输入文本中标记的字节偏移量 */
//  int iEnd             /* 输入文本中标记末尾的字节偏移量 */
//    )
extern "C" int fts5_single_xTokenize(Fts5Tokenizer *pTokenizer, void *pCtx, int tflags, const char *pText, int nText, int (*xToken)(void *pCtx, int tflags, const char *pToken, int nToken, int iStart, int iEnd))
{

  MyTokenizer *tokenizer = (MyTokenizer *)pTokenizer;
  tokenizer->words.clear();
  tokenizer->current = 0;
  std::string text(pText, nText);
  tokenizer->words = split_utf8_string(text);

  size_t start = 0;
  size_t end = 0;

  // 调用回调函数
  for (const auto &word : tokenizer->words)
  {
    end = start + word.size();
    const char *wptr = word.c_str();
    xToken(pCtx, tflags, wptr, word.size(), start, end);
    start = end;
  }
  return SQLITE_OK;
}
extern "C" fts5_api *fts5_api_from_db(sqlite3 *db)
{
  fts5_api *pRet = 0;
  sqlite3_stmt *pStmt = 0;

  int rc = sqlite3_prepare(db, "SELECT fts5(?1)", -1, &pStmt, 0);

  if (SQLITE_OK == rc)
  {
    sqlite3_bind_pointer(pStmt, 1, (void *)&pRet, "fts5_api_ptr", NULL);
    sqlite3_step(pStmt);
  }
  sqlite3_finalize(pStmt);
  return pRet;
}
/**
 * 扩展入口函数 函数名称必须是 sqlite3_*_init() *是项目名称
 * @returns {any}
 */
extern "C" int sqlite3_single_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi)
{
  (void)pzErrMsg;
  int rc = SQLITE_OK;
  SQLITE_EXTENSION_INIT2(pApi);
  fts5_api *fts5api;
  fts5_tokenizer tokenizer = {fts5_single_xCreate, fts5_single_xDelete, fts5_single_xTokenize};

  fts5api = fts5_api_from_db(db);

  if (fts5api == nullptr)
  {

    return SQLITE_ERROR;
  }
  rc = fts5api->xCreateTokenizer(fts5api, "single", (void *)fts5api, &tokenizer, NULL);

  return rc;
}