//
// Created by wuxianggujun on 2025/6/18.
//

#include "tinakit/excel/workbook.hpp"
#include "tinakit/core/openxml_archiver.hpp"

namespace tinakit::excel
{
    class  Workbook::Impl
    {
        public:
        using WorksheetPtr =  std::shared_ptr<WorkSheet>;

        explicit Impl(std::string_view path);
        explicit Impl();

        WorkSheet& getWorksheet(std::string_view name);
        WorkSheet& getWorksheet(size_t index);
        WorkSheet& addWorksheet(std::string_view name);

        void save(std::string_view path);

    private:
        void loadFromArchiver();

        void createDefaultStructure();
        void parseWorkbookRels();
        void parseWorkbookXml();
        void regenerateMetadataOnSave();

        std::shared_ptr<core::OpenXmlArchiver> archiver_;
        std::string original_path_;
        bool is_modified_  = false;

        std::vector<WorksheetPtr> worksheets_;
        std::map<std::string,WorksheetPtr,std::less<>> worksheet_by_name_;
        std::map<std::string,std::string,std::less<>> rels_map_;
    };

    Workbook::Impl::Impl(std::string_view path) : archiver_(std::make_shared<core::OpenXmlArchiver>()),original_path_(path)
    {
        archiver_->open_from_file(path.data());
        loadFromArchiver();
    }

    Workbook::Impl::Impl(): archiver_(std::make_shared<core::OpenXmlArchiver>()),is_modified_(true)
    {
        createDefaultStructure();
    }

    void Workbook::Impl::loadFromArchiver()
    {
        
    }
}
