#pragma once

#include <QRegularExpression>
#include <QSet>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <utility>
#include <vector>

namespace S2Plugin
{
    enum class HighlightColor
    {
        ReservedKeyword,
        Variable,
        Type,
        Comment,
        Text,
        Number,
        Function
    };

    class CPPSyntaxHighlighter : public QSyntaxHighlighter
    {
        Q_OBJECT
      public:
        explicit CPPSyntaxHighlighter(QTextDocument* parent = nullptr);
        void addRule(const QString& pattern, HighlightColor color);
        void clearRules();

      protected:
        void highlightBlock(const QString& text) override;

      private:
        std::vector<std::pair<QRegularExpression, HighlightColor>> mRules;
        QSet<QString> mSeenTypes; // keep track of added types and variables, so we don't balloon the rules
        QSet<QString> mSeenVariables;

        QTextCharFormat mFormatReservedKeywords;
        QTextCharFormat mFormatVariables;
        QTextCharFormat mFormatTypes;
        QTextCharFormat mFormatComments;
        QTextCharFormat mFormatText;
        QTextCharFormat mFormatNumber;
        QTextCharFormat mFormatFunction;
    };
} // namespace S2Plugin
