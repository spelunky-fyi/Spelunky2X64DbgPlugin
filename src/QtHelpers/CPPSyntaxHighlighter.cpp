#include "QtHelpers/CPPSyntaxHighlighter.h"

S2Plugin::CPPSyntaxHighlighter::CPPSyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent)
{
    mFormatReservedKeywords.setForeground(QColor("#569CD6"));
    mFormatVariables.setForeground(QColor("#9CDCFE"));
    mFormatTypes.setForeground(QColor("#4EC9B0"));
    mFormatComments.setForeground(QColor("#6A9955"));
    mFormatText.setForeground(QColor("#FFFFFF"));
    mFormatNumber.setForeground(QColor("#B5CEA8"));
    mFormatFunction.setForeground(QColor("#DCDCAA"));
    clearRules();
}

void S2Plugin::CPPSyntaxHighlighter::addRule(const QString& pattern, HighlightColor color)
{
    switch (color)
    {
        case HighlightColor::Variable:
            if (!mSeenVariables.contains(pattern))
            {
                mRules.emplace_back(QRegularExpression(pattern), color);
                mSeenVariables.insert(pattern);
            }
            break;
        case HighlightColor::Type:
            if (!mSeenTypes.contains(pattern))
            {
                mRules.emplace_back(QRegularExpression(pattern), color);
                mSeenTypes.insert(pattern);
            }
            break;
        default:
            mRules.emplace_back(QRegularExpression(pattern), color);
    }
}

void S2Plugin::CPPSyntaxHighlighter::highlightBlock(const QString& text)
{

    auto space = text.lastIndexOf(" ");
    for (const auto& [expr, color] : mRules)
    {
        QRegularExpressionMatchIterator matchIterator = expr.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            switch (color)
            {
                case HighlightColor::ReservedKeyword:
                    setFormat(match.capturedStart(), match.capturedLength(), mFormatReservedKeywords);
                    break;
                case HighlightColor::Variable:
                {
                    if (space < match.capturedStart())
                        setFormat(match.capturedStart(), match.capturedLength(), mFormatVariables);
                    break;
                }
                case HighlightColor::Type:
                    setFormat(match.capturedStart(), match.capturedLength(), mFormatTypes);
                    break;
                case HighlightColor::Comment:
                    setFormat(match.capturedStart(), match.capturedLength(), mFormatComments);
                    break;
                case HighlightColor::Text:
                    setFormat(match.capturedStart(), match.capturedLength(), mFormatText);
                    break;
                case HighlightColor::Number:
                    setFormat(match.capturedStart(), match.capturedLength(), mFormatNumber);
                    break;
                case HighlightColor::Function:
                    setFormat(match.capturedStart(), match.capturedLength(), mFormatFunction);
                    break;
            }
        }
    }
    static const std::pair<QRegularExpression, QTextCharFormat> lastRules[10] = {
        {QRegularExpression("\\b[0-9]+\\b"), mFormatNumber},
        {QRegularExpression(R"(\bvoid\b)"), mFormatReservedKeywords},
        {QRegularExpression(R"(\bbool\b)"), mFormatReservedKeywords},
        {QRegularExpression(R"(\bfloat\b)"), mFormatReservedKeywords},
        {QRegularExpression(R"(\bdouble\b)"), mFormatReservedKeywords},
        {QRegularExpression(R"(\bconst\b)"), mFormatReservedKeywords},
        {QRegularExpression(R"(\bchar\b)"), mFormatReservedKeywords},
        {QRegularExpression(R"(\bchar16_t\b)"), mFormatReservedKeywords},
        {QRegularExpression(R"(\bvirtual\b)"), mFormatReservedKeywords},
        {QRegularExpression(R"(\/\/.*?$)"), mFormatComments},
    };
    for (const auto& [expr, format] : lastRules)
    {
        QRegularExpressionMatchIterator matchIterator = expr.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), format);
        }
    }
}

void S2Plugin::CPPSyntaxHighlighter::clearRules()
{
    mRules.clear();
    mSeenTypes.clear();
    mSeenVariables.clear();
    addRule(R"(\bclass\b)", HighlightColor::ReservedKeyword);
    addRule(R"(\bpublic\b)", HighlightColor::ReservedKeyword);
    addRule(R"(\bstruct\b)", HighlightColor::ReservedKeyword);
}
