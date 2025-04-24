#pragma once

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QWidget>
#include <cstdint>
#include <vector>

namespace S2Plugin
{

    class WidgetPagination : public QWidget
    {
        Q_OBJECT
      public:
        explicit WidgetPagination(QWidget* parent = nullptr) : QWidget(parent)
        {
            QHBoxLayout* layout = new QHBoxLayout(this);
            layout->addWidget(new QLabel("Page size:"));
            mComboBox = new QComboBox(this);
            mComboBox->addItems({"50", "100", "200", "500", "1000"});
            mComboBox->setCurrentIndex(1);
            layout->addWidget(mComboBox);
            QObject::connect(mComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                             [this]()
                             {
                                 updateSpinBoxRange();
                                 emit pageUpdate();
                             });
            layout->addStretch();
            mLeftEnd = new QPushButton("|<", this);
            mLeftEnd->setDisabled(true);
            mLeftEnd->setMaximumWidth(mLeftEnd->sizeHint().height());
            mLeft = new QPushButton("<", this);
            mLeft->setDisabled(true);
            mLeft->setMaximumWidth(mLeft->sizeHint().height());
            mSpinBox = new QSpinBox(this);
            mSpinBox->setRange(1, 1);
            mSpinBox->setButtonSymbols(QSpinBox::NoButtons);
            mSpinBox->setAlignment(Qt::AlignCenter);
            mRight = new QPushButton(">", this);
            mRight->setDisabled(true);
            mRight->setMaximumWidth(mRight->sizeHint().height());
            mRightEnd = new QPushButton(">|", this);
            mRightEnd->setDisabled(true);
            mRightEnd->setMaximumWidth(mRightEnd->sizeHint().height());
            QObject::connect(mLeftEnd, &QPushButton::clicked, [this]() { setPage(1); });
            QObject::connect(mLeft, &QPushButton::clicked, [this]() { setPage(static_cast<int64_t>(mCurrentPage) - 1); });
            QObject::connect(mSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int value) { setPage(value); });
            QObject::connect(mRight, &QPushButton::clicked, [this]() { setPage(static_cast<int64_t>(mCurrentPage) + 1); });
            QObject::connect(mRightEnd, &QPushButton::clicked, [this]() { setPage(-1); });
            layout->addWidget(mLeftEnd);
            layout->addWidget(mLeft);
            layout->addWidget(mSpinBox);
            layout->addWidget(mRight);
            layout->addWidget(mRightEnd);
        }
        void setPage(int64_t page)
        {
            if (page == static_cast<int64_t>(mCurrentPage))
                return;

            auto count = static_cast<int64_t>(pageCount());
            if (page == -1 || page >= count)
            {
                page = count;
            }
            else if (page < 1)
            {
                page = 1;
            }
            mCurrentPage = static_cast<size_t>(page);
            mLeftEnd->setDisabled(page == 1);
            mLeft->setDisabled(page == 1);
            mRight->setDisabled(page == count);
            mRightEnd->setDisabled(page == count);
            mSpinBox->setValue(static_cast<int>(page));
            emit pageUpdate();
        }
        void setSize(size_t size)
        {
            mSize = size;
            updateSpinBoxRange();
        }
        std::pair<size_t, size_t> getRange() const
        {
            size_t pageIndex = mCurrentPage - 1;
            size_t upper = pageIndex * recordsPerPage() + recordsPerPage();
            if (upper > mSize)
            {
                upper = mSize;
            }
            return {pageIndex * recordsPerPage(), upper};
        }
        void setPageSizes(const std::vector<uint>& sizes)
        {
            mPageSizes = sizes;
            mComboBox->clear();
            for (auto size : mPageSizes)
            {
                mComboBox->addItem(QString::number(size));
            }
            updateSpinBoxRange();
        }
        size_t recordsPerPage() const
        {
            return mPageSizes[static_cast<size_t>(mComboBox->currentIndex())];
        }
        size_t getCurrentPage() const
        {
            return mCurrentPage;
        }

      signals:
        void pageUpdate();

      private:
        size_t pageCount() const
        {
            size_t count = mSize / recordsPerPage();
            if (mSize % recordsPerPage() > 0)
            {
                count++;
            }
            return count == 0 ? 1 : count;
        }
        void updateSpinBoxRange()
        {
            auto count = pageCount();
            mSpinBox->setRange(1, static_cast<int>(count));
            if (mCurrentPage > count)
            {
                setPage(static_cast<int64_t>(count));
            }
            else if (mCurrentPage == count)
            {
                mRightEnd->setDisabled(true);
                mRight->setDisabled(true);
            }
            else
            {
                mRightEnd->setDisabled(false);
                mRight->setDisabled(false);
            }
        }
        QComboBox* mComboBox;
        QSpinBox* mSpinBox;
        QPushButton* mLeftEnd;
        QPushButton* mLeft;
        QPushButton* mRight;
        QPushButton* mRightEnd;

        std::vector<uint> mPageSizes = {50, 100, 200, 500, 1000};
        size_t mSize = 0;
        size_t mCurrentPage = 1;
    };
}; // namespace S2Plugin
