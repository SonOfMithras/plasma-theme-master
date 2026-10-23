#include "FlowLayout.h"
#include <QWidget>

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing) {
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing) {
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout() {
    QLayoutItem *item;
    while ((item = takeAt(0))) {
        delete item;
    }
}

void FlowLayout::addItem(QLayoutItem *item) {
    m_itemList.append(item);
}

int FlowLayout::horizontalSpacing() const {
    if (m_hSpace >= 0) return m_hSpace;
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const {
    if (m_vSpace >= 0) return m_vSpace;
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int FlowLayout::count() const {
    return m_itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const {
    return m_itemList.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index) {
    if (index >= 0 && index < m_itemList.size())
        return m_itemList.takeAt(index);
    return nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const {
    return {};
}

bool FlowLayout::hasHeightForWidth() const {
    return true;
}

int FlowLayout::heightForWidth(int width) const {
    return doLayout(QRect(0, 0, width, 0), true);
}

void FlowLayout::setGeometry(const QRect &rect) {
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const {
    return minimumSize();
}

QSize FlowLayout::minimumSize() const {
    if (m_fillColumns) {
        int minW = m_minItemWidth > 0 ? m_minItemWidth : 250;
        int minH = 100;
        for (const QLayoutItem *item : m_itemList) {
            if (item->widget() && !item->widget()->isHidden()) {
                minH = qMax(minH, item->minimumSize().height());
            }
        }
        const QMargins margins = contentsMargins();
        return QSize(minW + margins.left() + margins.right(),
                     minH + margins.top() + margins.bottom());
    }

    QSize size;
    for (const QLayoutItem *item : m_itemList) {
        size = size.expandedTo(item->minimumSize());
    }

    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const {
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int availableWidth = effectiveRect.width();
    const int hSpace = horizontalSpacing();
    const int vSpace = verticalSpacing();

    if (m_fillColumns) {
        QList<QLayoutItem *> visibleItems;
        for (QLayoutItem *item : m_itemList) {
            QWidget *wid = item->widget();
            if (!wid || !wid->isHidden()) {
                visibleItems.append(item);
            }
        }

        if (visibleItems.isEmpty()) {
            return top + bottom;
        }

        int targetMin = m_minItemWidth;
        if (targetMin <= 0) {
            targetMin = visibleItems.first()->minimumSize().width();
            if (targetMin <= 0) targetMin = visibleItems.first()->sizeHint().width();
            if (targetMin <= 0) targetMin = 250;
        }

        int cols = 1;
        if (availableWidth > targetMin) {
            cols = qMax(1, (availableWidth + hSpace) / (targetMin + hSpace));
        }

        int totalHSpaces = (cols - 1) * hSpace;
        int colWidth = (availableWidth - totalHSpaces) / cols;
        if (colWidth < targetMin && cols > 1) {
            cols--;
            totalHSpaces = (cols - 1) * hSpace;
            colWidth = (availableWidth - totalHSpaces) / cols;
        }
        if (colWidth < 1) colWidth = 1;

        int curY = effectiveRect.y();
        int rowHeight = 0;

        for (int i = 0; i < visibleItems.size(); ++i) {
            QLayoutItem *item = visibleItems.at(i);
            int col = i % cols;
            int itemH = item->sizeHint().height();
            if (itemH <= 0) itemH = item->minimumSize().height();
            if (itemH <= 0) itemH = 115;
            rowHeight = qMax(rowHeight, itemH);

            int itemX = effectiveRect.x() + col * (colWidth + hSpace);
            int thisWidth = (col == cols - 1) ? (effectiveRect.right() - itemX + 1) : colWidth;
            if (thisWidth < 1) thisWidth = colWidth;

            if (!testOnly) {
                item->setGeometry(QRect(itemX, curY, thisWidth, itemH));
            }

            if (col == cols - 1 || i == visibleItems.size() - 1) {
                curY += rowHeight + vSpace;
                rowHeight = 0;
            }
        }

        return curY - rect.y() + bottom;
    } else {
        int x = effectiveRect.x();
        int y = effectiveRect.y();
        int lineHeight = 0;

        for (QLayoutItem *item : m_itemList) {
            QWidget *wid = item->widget();
            if (wid && wid->isHidden()) continue;

            int spaceX = hSpace;
            if (spaceX == -1)
                spaceX = wid ? wid->style()->layoutSpacing(
                    QSizePolicy::DefaultType, QSizePolicy::DefaultType, Qt::Horizontal) : 0;
            int spaceY = vSpace;
            if (spaceY == -1)
                spaceY = wid ? wid->style()->layoutSpacing(
                    QSizePolicy::DefaultType, QSizePolicy::DefaultType, Qt::Vertical) : 0;

            int itemW = item->sizeHint().width();
            int itemH = item->sizeHint().height();

            if (x + itemW > effectiveRect.right() && lineHeight > 0) {
                x = effectiveRect.x();
                y = y + lineHeight + spaceY;
                lineHeight = 0;
            }

            if (!testOnly) {
                item->setGeometry(QRect(QPoint(x, y), QSize(itemW, itemH)));
            }

            x += itemW + spaceX;
            lineHeight = qMax(lineHeight, itemH);
        }
        return y + lineHeight - rect.y() + bottom;
    }
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const {
    QObject *parent = this->parent();
    if (!parent) return -1;
    if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    return static_cast<QLayout *>(parent)->spacing();
}
