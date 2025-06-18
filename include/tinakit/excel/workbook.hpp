/**
 * @file workbook.hpp
 * @brief Excel 工作簿类定义
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include "tinakit/core/types.hpp"
#include "tinakit/core/async.hpp"
// #include "worksheet.hpp"
#include <filesystem>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace tinakit::excel {

    class Worksheet;
    
/**
 * @class Workbook
 * @brief Excel 工作簿类
 * 
 * 代表一个 Excel 工作簿文件，包含一个或多个工作表。
 * 支持读取、创建、修改和保存 Excel 文件。
 * 
 * @note 使用 RAII 原则管理资源，对象析构时自动释放相关资源
 */
class Workbook {
    class Impl;  // 前向声明移到这里
public:
    /**
     * @brief 错误回调函数类型
     * @param error 错误信息
     */
    using ErrorCallback = std::function<void(const std::exception& error)>;

public:
    /**
     * @brief 构造函数（私有，通过静态方法创建）
     */
    explicit Workbook(std::unique_ptr<Impl> impl);
    
    /**
     * @brief 析构函数
     */
    ~Workbook();
    
    /**
     * @brief 移动构造函数
     */
    Workbook(Workbook&& other) noexcept;
    
    /**
     * @brief 移动赋值运算符
     */
    Workbook& operator=(Workbook&& other) noexcept;
    
    // 禁止拷贝
    Workbook(const Workbook&) = delete;
    Workbook& operator=(const Workbook&) = delete;

public:
    /**
     * @brief 打开 Excel 文件
     * @param path 文件路径
     * @return Workbook 对象
     * @throws FileNotFoundException 文件不存在
     * @throws CorruptedFileException 文件损坏
     * @throws ParseException 解析错误
     */
    static Workbook open(const std::filesystem::path& path);
    
    /**
     * @brief 异步打开 Excel 文件
     * @param path 文件路径
     * @return 返回 Workbook 的 Task
     */
    static async::Task<Workbook> open_async(const std::filesystem::path& path);
    
    /**
     * @brief 创建新的工作簿
     * @return 新的 Workbook 对象
     */
    static Workbook create();

    /**
     * @brief 按名称获取工作表
     * @param name 工作表名称
     * @return 工作表引用
     * @throws WorksheetNotFoundException 工作表不存在
     */
    Worksheet& operator[](const std::string& name);
    
    /**
     * @brief 按名称获取工作表（只读）
     * @param name 工作表名称
     * @return 工作表常量引用
     * @throws WorksheetNotFoundException 工作表不存在
     */
    const Worksheet& operator[](const std::string& name) const;
    
    /**
     * @brief 按索引获取工作表
     * @param index 工作表索引（从 0 开始）
     * @return 工作表引用
     * @throws std::out_of_range 索引超出范围
     */
    Worksheet& operator[](std::size_t index);
    
    /**
     * @brief 按索引获取工作表（只读）
     * @param index 工作表索引（从 0 开始）
     * @return 工作表常量引用
     * @throws std::out_of_range 索引超出范围
     */
    const Worksheet& operator[](std::size_t index) const;
    
    /**
     * @brief 获取活动工作表
     * @return 活动工作表引用
     */
    Worksheet& active_sheet();
    
    /**
     * @brief 获取活动工作表（只读）
     * @return 活动工作表常量引用
     */
    const Worksheet& active_sheet() const;

public:
    /**
     * @brief 添加新工作表
     * @param name 工作表名称
     * @return 新工作表引用
     * @throws DuplicateWorksheetNameException 工作表名称已存在
     */
    Worksheet& add_sheet(const std::string& name);
    
    /**
     * @brief 删除工作表
     * @param name 工作表名称
     * @throws WorksheetNotFoundException 工作表不存在
     * @throws CannotDeleteLastWorksheetException 不能删除最后一个工作表
     */
    void remove_sheet(const std::string& name);
    
    /**
     * @brief 删除工作表
     * @param index 工作表索引
     * @throws std::out_of_range 索引超出范围
     * @throws CannotDeleteLastWorksheetException 不能删除最后一个工作表
     */
    void remove_sheet(std::size_t index);
    
    /**
     * @brief 获取所有工作表
     * @return 工作表列表
     */
    std::vector<std::reference_wrapper<Worksheet>> worksheets();
    
    /**
     * @brief 获取所有工作表（只读）
     * @return 工作表常量列表
     */
    std::vector<std::reference_wrapper<const Worksheet>> worksheets() const;
    
    /**
     * @brief 获取工作表数量
     * @return 工作表数量
     */
    std::size_t sheet_count() const noexcept;

public:
    /**
     * @brief 保存工作簿
     * @param path 保存路径（可选，默认使用原路径）
     * @throws IOException 保存失败
     */
    void save(const std::filesystem::path& path = {});
    
    /**
     * @brief 异步保存工作簿
     * @param path 保存路径（可选，默认使用原路径）
     * @return 保存任务
     */
    async::Task<void> save_async(const std::filesystem::path& path = {});
    
    /**
     * @brief 另存为
     * @param path 新文件路径
     * @throws IOException 保存失败
     */
    void save_as(const std::filesystem::path& path);
    
    /**
     * @brief 异步另存为
     * @param path 新文件路径
     * @return 保存任务
     */
    async::Task<void> save_as_async(const std::filesystem::path& path);

public:
    /**
     * @brief 设置错误回调
     * @param callback 错误回调函数
     */
    void on_error(ErrorCallback callback);
    
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



private:
    std::unique_ptr<Impl> impl_;  ///< 实现细节（PIMPL 模式）
    friend class Excel;
};

} // namespace tinakit

/**
 * @example workbook_usage.cpp
 * Workbook 使用示例：
 * @code
 * #include <tinakit/excel/workbook.hpp>
 * 
 * void workbook_example() {
 *     using namespace tinakit;
 *     
 *     // 创建新工作簿
 *     auto workbook = Workbook::create()
 *         .add_sheet("销售数据")
 *         .add_sheet("统计报表");
 *     
 *     // 访问工作表
 *     auto& sales_sheet = workbook["销售数据"];
 *     auto& stats_sheet = workbook[1];
 *     
 *     // 设置进度回调
 *     workbook.on_progress([](double progress) {
 *         std::cout << "进度: " << (progress * 100) << "%" << std::endl;
 *     });
 *     
 *     // 保存文件
 *     workbook.save("report.xlsx");
 * }
 * @endcode
 */
