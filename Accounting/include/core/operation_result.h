#ifndef ACCOUNTING_CORE_OPERATION_RESULT_H_
#define ACCOUNTING_CORE_OPERATION_RESULT_H_

#include <string>
#include <optional>

namespace accounting {

/**
 * @brief 操作错误码枚举
 * 
 * 定义系统中所有可能的业务操作错误码，供 UI 层识别并展示相应的错误信息。
 */
enum class ErrorCode {
    Success = 0,                    // 操作成功
    UserAlreadyExists,              // 用户已存在
    UserNotFound,                   // 用户不存在
    PasswordMismatch,               // 密码错误
    InvalidUsername,                // 无效的用户名（格式、长度等）
    InvalidPassword,                // 无效的密码（格式、长度等）
    InvalidBill,                    // 无效的账单（金额、日期等）
    InvalidCategory,                // 无效的分类（名称、类型等）
    InvalidBudget,                  // 无效的预算（金额、限制等）
    BudgetExceeded,                 // 预算超限（总预算）
    CategoryBudgetExceeded,         // 分类预算超限
    CategoryNotFound,               // 分类不存在
    BillNotFound,                   // 账单不存在
    BudgetNotFound,                 // 预算不存在
    DuplicateCategory,              // 分类名称重复
    StorageError,                   // 存储操作失败（文件 I/O、解析等）
    InitializationError,            // 系统初始化失败
    UnknownError = 999,             // 未知错误
};

/**
 * @brief 操作结果模板类
 * 
 * 包装任何业务操作的返回值，提供：
 * 1. 成功/失败标志
 * 2. 实际的返回数据（成功时）
 * 3. 错误码和错误消息（失败时）
 * 
 * @tparam T 操作成功时的返回数据类型
 */
template<typename T>
class OperationResult {
public:
    /**
     * @brief 创建成功结果
     * @param data 操作返回的数据
     * @return 成功的 OperationResult
     */
    static OperationResult<T> Success(const T& data) {
        OperationResult<T> result;
        result.success_ = true;
        result.data_ = data;
        result.error_code_ = ErrorCode::Success;
        result.error_message_ = "";
        return result;
    }

    /**
     * @brief 创建失败结果
     * @param error_code 错误码
     * @param error_message 错误信息
     * @return 失败的 OperationResult
     */
    static OperationResult<T> Failure(ErrorCode error_code, const std::string& error_message) {
        OperationResult<T> result;
        result.success_ = false;
        result.error_code_ = error_code;
        result.error_message_ = error_message;
        return result;
    }

    // 查询结果状态
    bool IsSuccess() const { return success_; }
    bool IsFailure() const { return !success_; }

    // 获取错误信息
    ErrorCode GetErrorCode() const { return error_code_; }
    const std::string& GetErrorMessage() const { return error_message_; }

    // 获取数据（成功时使用）
    const T& GetData() const { return data_; }
    T& GetData() { return data_; }

    // 便利方法：直接访问数据（失败时行为未定义）
    const T& operator*() const { return data_; }
    T& operator*() { return data_; }

    const T* operator->() const { return &data_; }
    T* operator->() { return &data_; }

private:
    bool success_ = false;
    T data_;
    ErrorCode error_code_ = ErrorCode::UnknownError;
    std::string error_message_;
};

/**
 * @brief 针对 void 返回类型的特化版本
 * 
 * 用于只关心成功/失败和错误信息的操作（不需要返回数据）
 */
template<>
class OperationResult<void> {
public:
    /**
     * @brief 创建成功结果
     * @return 成功的 OperationResult<void>
     */
    static OperationResult<void> Success() {
        OperationResult<void> result;
        result.success_ = true;
        result.error_code_ = ErrorCode::Success;
        result.error_message_ = "";
        return result;
    }

    /**
     * @brief 创建失败结果
     * @param error_code 错误码
     * @param error_message 错误信息
     * @return 失败的 OperationResult<void>
     */
    static OperationResult<void> Failure(ErrorCode error_code, const std::string& error_message) {
        OperationResult<void> result;
        result.success_ = false;
        result.error_code_ = error_code;
        result.error_message_ = error_message;
        return result;
    }

    // 查询结果状态
    bool IsSuccess() const { return success_; }
    bool IsFailure() const { return !success_; }

    // 获取错误信息
    ErrorCode GetErrorCode() const { return error_code_; }
    const std::string& GetErrorMessage() const { return error_message_; }

private:
    bool success_ = false;
    ErrorCode error_code_ = ErrorCode::UnknownError;
    std::string error_message_;
};

}  // namespace accounting

#endif  // ACCOUNTING_CORE_OPERATION_RESULT_H_
