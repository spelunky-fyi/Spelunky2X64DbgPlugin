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
            QObject::connect(mLeft, &QPushButton::clicked, [this]() { setPage(mCurrentPage - 1); });
            QObject::connect(mSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int value) { setPage(value); });
            QObject::connect(mRight, &QPushButton::clicked, [this]() { setPage(mCurrentPage + 1); });
            QObject::connect(mRightEnd, &QPushButton::clicked, [this]() { setPage(-1); });
            layout->addWidget(mLeftEnd);
            layout->addWidget(mLeft);
            layout->addWidget(mSpinBox);
            layout->addWidget(mRight);
            layout->addWidget(mRightEnd);
        }
        void setPage(int page)
        {
            if (page == (int)mCurrentPage)
                return;

            auto count = pageCount();
            if (page == -1 || (size_t)page >= count)
            {
                page = count;
            }
            else if (page < 1)
            {
                page = 1;
            }
            mCurrentPage = page;
            mLeftEnd->setDisabled(page == 1);
            mLeft->setDisabled(page == 1);
            mRight->setDisabled((size_t)page == count);
            mRightEnd->setDisabled((size_t)page == count);
            mSpinBox->setValue(page);
            emit pageUpdate();
        }
        void setSize(size_t size)
        {
            mSize = size;
            updateSpinBoxRange();
        }
        std::pair<size_t, size_t> getRange() const
        {
            uint pageIndex = mCurrentPage - 1;
            size_t upper = (size_t)pageIndex * recordsPerPage() + recordsPerPage();
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
        uint recordsPerPage() const
        {
            return mPageSizes[mComboBox->currentIndex()];
        }
        uint getCurrentPage() const
        {
            return mCurrentPage;
        }

      signals:
        void pageUpdate();

      private:
        uint pageCount() const
        {
            uint count = (uint)(mSize / recordsPerPage());
            if (mSize % recordsPerPage() > 0)
            {
                count++;
            }
            return count == 0 ? 1 : count;
        }
        void updateSpinBoxRange()
        {
            auto count = pageCount();
            mSpinBox->setRange(1, count);
            if (mCurrentPage > count)
            {
                setPage(count);
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
        uint mCurrentPage = 1;
    };
}; // namespace S2Plugin
