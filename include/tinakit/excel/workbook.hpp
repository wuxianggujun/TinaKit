/**
 * @file workbook.hpp
 * @brief Excel 工作簿类定义
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include "tinakit/core/types.hpp"
#include "tinakit/core/async.hpp"
#include <filesystem>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace tinakit::excel {

// 前向声明
class Worksheet;
class Range;
class Cell;

} // namespace tinakit::excel

namespace tinakit::internal {
class workbook_impl;
} // namespace tinakit::internal

namespace tinakit::excel {

/**
 * @class Workbook
 * @brief Excel 工作簿轻量级句柄类
 *
 * 这是一个轻量级的句柄类，本身不持有任何重型数据。
 * 所有实际的数据和操作都委托给 internal::workbook_impl。
 *
 * 核心设计原则：
 * 1. 轻量级：只包含指向实现的指针
 * 2. 可复制：复制成本极低，安全共享同一个工作簿
 * 3. 委托模式：所有操作都委托给 workbook_impl
 * 4. 惰性求值：数据按需加载
 *
 * @note 使用句柄-实现分离模式，提供稳定的 ABI 和优秀的性能
 */
class Workbook {
public:
    /**
     * @brief 错误回调函数类型
     */
    using ErrorCallback = std::function<void(const std::exception& error)>;

public:
    /**
     * @brief 默认构造函数（创建空工作簿）
     */
    Workbook();

    /**
     * @brief 拷贝构造函数（轻量级，共享同一个实现）
     */
    Workbook(const Workbook& other) = default;

    /**
     * @brief 移动构造函数
     */
    Workbook(Workbook&& other) noexcept = default;

    /**
     * @brief 拷贝赋值运算符
     */
    Workbook& operator=(const Workbook& other) = default;

    /**
     * @brief 移动赋值运算符
     */
    Workbook& operator=(Workbook&& other) noexcept = default;

    /**
     * @brief 析构函数
     */
    ~Workbook() = default;

public:
    // ========================================
    // 静态工厂方法
    // ========================================

    /**
     * @brief 加载 Excel 文件
     * @param file_path 文件路径
     * @return Workbook 句柄
     * @throws FileNotFoundException 文件不存在
     * @throws CorruptedFileException 文件损坏
     * @throws ParseException 解析错误
     */
    static Workbook load(const std::filesystem::path& file_path);

    /**
     * @brief 异步加载 Excel 文件
     * @param file_path 文件路径
     * @return 返回 Workbook 的 Task
     */
    static async::Task<Workbook> load_async(const std::filesystem::path& file_path);

    /**
     * @brief 创建新的工作簿
     * @return 新的 Workbook 句柄
     */
    static Workbook create();

    // ========================================
    // 工作表访问
    // ========================================

    /**
     * @brief 按名称获取工作表句柄
     * @param name 工作表名称
     * @return Worksheet 句柄
     * @throws WorksheetNotFoundException 工作表不存在
     */
    Worksheet get_worksheet(const std::string& name) const;

    /**
     * @brief 按索引获取工作表句柄
     * @param index 工作表索引（从 0 开始）
     * @return Worksheet 句柄
     * @throws std::out_of_range 索引超出范围
     */
    Worksheet get_worksheet(std::size_t index) const;

    /**
     * @brief 获取活动工作表句柄
     * @return Worksheet 句柄
     */
    Worksheet active_sheet() const;

    /**
     * @brief 按名称获取工作表句柄（操作符重载）
     * @param name 工作表名称
     * @return Worksheet 句柄
     */
    Worksheet operator[](const std::string& name) const;

    /**
     * @brief 按索引获取工作表句柄（操作符重载）
     * @param index 工作表索引
     * @return Worksheet 句柄
     */
    Worksheet operator[](std::size_t index) const;

    // ========================================
    // 工作表管理
    // ========================================

    /**
     * @brief 创建新工作表
     * @param name 工作表名称
     * @return 新工作表句柄
     * @throws DuplicateWorksheetNameException 工作表名称已存在
     */
    Worksheet create_worksheet(const std::string& name);

    /**
     * @brief 删除工作表
     * @param name 工作表名称
     * @throws WorksheetNotFoundException 工作表不存在
     * @throws CannotDeleteLastWorksheetException 不能删除最后一个工作表
     */
    void remove_worksheet(const std::string& name);

    /**
     * @brief 重命名工作表
     * @param old_name 原工作表名称
     * @param new_name 新工作表名称
     * @throws WorksheetNotFoundException 工作表不存在
     * @throws DuplicateWorksheetNameException 新名称已存在
     */
    void rename_worksheet(const std::string& old_name, const std::string& new_name);

    /**
     * @brief 获取所有工作表名称
     * @return 工作表名称列表
     */
    std::vector<std::string> worksheet_names() const;

    /**
     * @brief 获取工作表数量
     * @return 工作表数量
     */
    std::size_t worksheet_count() const noexcept;

    /**
     * @brief 检查工作表是否存在
     * @param name 工作表名称
     * @return 如果存在返回 true
     */
    bool has_worksheet(const std::string& name) const;

    // ========================================
    // 文件操作
    // ========================================

    /**
     * @brief 保存工作簿
     * @param file_path 保存路径（可选，默认使用原路径）
     * @throws IOException 保存失败
     */
    void save(const std::filesystem::path& file_path = {});

    /**
     * @brief 异步保存工作簿
     * @param file_path 保存路径（可选，默认使用原路径）
     * @return 保存任务
     */
    async::Task<void> save_async(const std::filesystem::path& file_path = {});

    // ========================================
    // 属性和状态
    // ========================================

    /**
     * @brief 获取文件路径
     * @return 文件路径
     */
    const std::filesystem::path& file_path() const noexcept;

    /**
     * @brief 检查是否有未保存的更改
     * @return 如果有未保存的更改返回 true
     */
    bool has_unsaved_changes() const noexcept;

    /**
     * @brief 获取文件大小（字节）
     * @return 文件大小
     */
    std::size_t file_size() const;

    // ========================================
    // 内部访问（高级用户）
    // ========================================

    /**
     * @brief 获取内部实现指针（仅供内部使用）
     * @return workbook_impl 指针
     * @warning 此方法仅供 TinaKit 内部使用，不保证 API 稳定性
     */
    std::shared_ptr<internal::workbook_impl> impl() const;

private:
    // 内部构造函数（供静态工厂方法使用）
    explicit Workbook(std::shared_ptr<internal::workbook_impl> impl);

    // 轻量级句柄：只包含指向实现的共享指针
    std::shared_ptr<internal::workbook_impl> impl_;
};

} // namespace tinakit

/**
 * @example workbook_usage.cpp
 * workbook 使用示例：
 * @code
 * #include <tinakit/excel/workbook.hpp>
 *
 * void workbook_example() {
 *     using namespace tinakit::excel;
 *
 *     // 加载现有工作簿
 *     auto wb = workbook::load("financial_report.xlsx");
 *
 *     // 获取工作表句柄
 *     auto ws = wb.get_worksheet("Sales Data");
 *
 *     // 创建新工作簿
 *     auto new_wb = workbook::create();
 *     new_wb.create_worksheet("销售数据");
 *     new_wb.create_worksheet("统计报表");
 *
 *     // 访问工作表
 *     auto sales_sheet = new_wb["销售数据"];
 *     auto stats_sheet = new_wb[1];
 *
 *     // 保存文件
 *     new_wb.save("report.xlsx");
 * }
 * @endcode
 */
