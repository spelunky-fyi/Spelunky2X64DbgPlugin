#pragma once

#include <QWidget>
#include <string>
#include <string_view>
#include <vector>

class QTextEdit;

namespace S2Plugin
{
    class CPPSyntaxHighlighter;

    class ViewCpp : public QWidget
    {
        Q_OBJECT
      public:
        ViewCpp(std::string_view typeName, QWidget* parent = nullptr);
        void addDependency(std::string_view typeName)
        {
            if (std::find(mDependencies.begin(), mDependencies.end(), typeName) == mDependencies.end())
                mDependencies.emplace_back(typeName);
        }

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private:
        void refresh(bool addDependencies);
        void generate(std::string typeName);

        std::string mTypeName;
        QTextEdit* mCPPTextEdit;
        CPPSyntaxHighlighter* mCPPSyntaxHighlighter;
        std::vector<std::string> mDependencies;
    };
} // namespace S2Plugin
