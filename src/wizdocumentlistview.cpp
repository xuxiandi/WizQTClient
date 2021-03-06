#include "wizdocumentlistview.h"
#include "wizcategoryview.h"

#include "wiznotestyle.h"

#include "wiztaglistwidget.h"

#include <QStyledItemDelegate>
#include <QApplication>
#include <QMenu>
#include <QContextMenuEvent>
#include <QScrollBar>
#include <QDrag>
#include <QMimeData>


class CWizDocumentListViewItem : public QListWidgetItem
{
protected:
    WIZDOCUMENTDATA m_data;
    WIZABSTRACT m_abstract;
    CString m_tags;

public:
    CWizDocumentListViewItem(const WIZDOCUMENTDATA& data, QListWidget *view = 0, int type = Type)
        : QListWidgetItem(view, type)
        , m_data(data)
    {
        setText(m_data.strTitle);
    }

    const WIZDOCUMENTDATA& document() const
    {
        return m_data;
    }

    WIZABSTRACT& abstract(CWizDatabase& db)
    {
        if (m_abstract.text.isEmpty())
        {
            db.PadAbstractFromGUID(m_data.strGUID, m_abstract);
            if (m_abstract.text.IsEmpty())
                m_abstract.text = " ";
            m_abstract.text.replace('\n', ' ');
            m_abstract.text.replace("\r", "");
        }

        return m_abstract;
    }

    CString tags(CWizDatabase& db)
    {
        if (m_tags.IsEmpty())
        {
            m_tags = db.GetDocumentTagDisplayNameText(m_data.strGUID);
            m_tags = " " + m_tags;
        }

        return m_tags;
    }

    void reload(CWizDatabase& db)
    {
        db.DocumentFromGUID(m_data.strGUID, m_data);
        m_abstract = WIZABSTRACT();
        m_tags.clear();

        setText("");    //force repaint
        setText(m_data.strTitle);
    }

    virtual bool operator<(const QListWidgetItem &other) const
    {
        const CWizDocumentListViewItem* pOther = dynamic_cast<const CWizDocumentListViewItem*>(&other);
        Q_ASSERT(pOther);

        if (pOther->m_data.tCreated == m_data.tCreated)
        {
            return text().compare(other.text(), Qt::CaseInsensitive) < 0;
        }

        return pOther->m_data.tCreated < m_data.tCreated;
    }

    void resetAbstract()
    {
        m_abstract = WIZABSTRACT();
    }
};

class CWizDocumentListViewDelegate : public QStyledItemDelegate
{
public:
    CWizDocumentListViewDelegate(QWidget* parent)
        : QStyledItemDelegate(parent)
    {
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
    {
        QSize sz = QStyledItemDelegate::sizeHint(option, index);
        sz.setHeight(sz.height() + (option.fontMetrics.height() + 2) * 3 + 2 + 16);
        return sz;
    }
};


CWizDocumentListView::CWizDocumentListView(CWizExplorerApp& app, QWidget *parent /*= 0*/)
    : QListWidget(parent)
    , m_app(app)
    , m_db(app.database())
    , m_category(app.category())
    , m_menu(NULL)
    , m_tagList(NULL)
{
    setFrameStyle(QFrame::NoFrame);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

#ifndef Q_OS_MAC
    // smoothly scroll
    m_vscrollOldPos = 0;
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(on_vscroll_valueChanged(int)));
    connect(verticalScrollBar(), SIGNAL(actionTriggered(int)), SLOT(on_vscroll_actionTriggered(int)));
    connect(&m_vscrollTimer, SIGNAL(timeout()), SLOT(on_vscroll_update()));
#endif //Q_OS_MAC

    setItemDelegate(new CWizDocumentListViewDelegate(this));

    QPalette pal = palette();
    pal.setColor(QPalette::Base, WizGetDocumentsBackroundColor(m_app.userSettings().skin()));
    setPalette(pal);

    setStyle(::WizGetStyle(m_app.userSettings().skin()));

    qRegisterMetaType<WIZTAGDATA>("WIZTAGDATA");
    qRegisterMetaType<WIZDOCUMENTDATA>("WIZDOCUMENTDATA");

    connect(&m_db, SIGNAL(tagCreated(const WIZTAGDATA&)), \
            SLOT(on_tag_created(const WIZTAGDATA&)));

    connect(&m_db, SIGNAL(tagModified(const WIZTAGDATA&, const WIZTAGDATA&)), \
            SLOT(on_tag_modified(const WIZTAGDATA&, const WIZTAGDATA&)));

    connect(&m_db, SIGNAL(documentCreated(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_created(const WIZDOCUMENTDATA&)));

    connect(&m_db, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)), \
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_db, SIGNAL(documentDeleted(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_deleted(const WIZDOCUMENTDATA&)));

    connect(&m_db, SIGNAL(documentAbstractModified(const WIZDOCUMENTDATA&)), \
            SLOT(on_document_AbstractModified(const WIZDOCUMENTDATA&)));

    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void CWizDocumentListView::setDocuments(const CWizDocumentDataArray& arrayDocument)
{
    clear();
    addDocuments(arrayDocument);
}


void CWizDocumentListView::addDocuments(const CWizDocumentDataArray& arrayDocument)
{
    for (CWizDocumentDataArray::const_iterator it = arrayDocument.begin();
    it != arrayDocument.end();
    it++)
    {
        addDocument(*it, false);
    }

    sortItems();

    if (selectedItems().empty())
    {
        setCurrentRow(0);
    }
}

int CWizDocumentListView::addDocument(const WIZDOCUMENTDATA& data, bool sort)
{
    CWizDocumentListViewItem* pItem = new CWizDocumentListViewItem(data, this);

    addItem(pItem);
    if (sort)
    {
        sortItems();
    }

    return count();
}

bool CWizDocumentListView::acceptDocument(const WIZDOCUMENTDATA& document)
{
    return m_category.acceptDocument(document);
}

void CWizDocumentListView::addAndSelectDocument(const WIZDOCUMENTDATA& document)
{
    Q_ASSERT(acceptDocument(document));

    int index = documentIndexFromGUID(document.strGUID);
    if (-1 == index)
    {
        index = addDocument(document, false);
    }
    if (-1 == index)
        return;

    setCurrentItem(item(index), QItemSelectionModel::ClearAndSelect);
    sortItems();
}

void CWizDocumentListView::getSelectedDocuments(CWizDocumentDataArray& arrayDocument)
{
    QList<QListWidgetItem*> items = selectedItems();

    QList<QListWidgetItem*>::const_iterator it;
    for (it = items.begin(); it != items.end(); it++)
    {
        QListWidgetItem* pItem = *it;

        CWizDocumentListViewItem* pDocumentItem = dynamic_cast<CWizDocumentListViewItem*>(pItem);
        if (pDocumentItem)
        {
            arrayDocument.push_back(pDocumentItem->document());
        }
    }
}


void CWizDocumentListView::contextMenuEvent(QContextMenuEvent * e)
{
    if (!m_menu)
    {
        m_menu = new QMenu(this);
        m_menu->addAction(tr("Tags..."), this, SLOT(on_action_selectTags()));
        m_menu->addSeparator();
        m_menu->addAction(tr("Delete..."), this, SLOT(on_action_deleteDocument()));

        m_actionEncryptDocument = new QAction(tr("Encrypt Document"), m_menu);
        connect(m_actionEncryptDocument, SIGNAL(triggered()), SLOT(on_action_encryptDocument()));
        //m_menu->addAction(m_actionEncryptDocument);
    }

    m_menu->popup(mapToGlobal(e->pos()));
}

void CWizDocumentListView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition.setX(event->pos().x());
        m_dragStartPosition.setY(event->pos().y());
    }

    QListWidget::mousePressEvent(event);
}

void CWizDocumentListView::mouseMoveEvent(QMouseEvent* event)
{
    if ((event->buttons() & Qt::LeftButton) && \
            (event->pos() - m_dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
        setState(QAbstractItemView::DraggingState);
    }

    QListWidget::mouseMoveEvent(event);
}


void CWizDocumentListView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);

    CWizStdStringArray arrayGUID;

    QList<QListWidgetItem*> items = selectedItems();
    foreach (QListWidgetItem* it, items)
    {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(it))
        {
            arrayGUID.push_back((item->document().strGUID));
        }
    }

    if (arrayGUID.empty())
        return;

    CString strGUIDs;
    ::WizStringArrayToText(arrayGUID, strGUIDs, ";");

    QDrag* drag = new QDrag(this);

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(WIZNOTE_MIMEFORMAT_DOCUMENTS, strGUIDs.toUtf8());
    drag->setMimeData(mimeData);

    // FIXME: need deal with more then 1 drag event!
    if (items.size() == 1) {
        QRect rect = visualItemRect(items[0]);
        drag->setPixmap(QPixmap::grabWindow(winId(), rect.x(), rect.y(), rect.width(), rect.height()));
    }

    drag->exec();
}

void CWizDocumentListView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_TAGS))
    {
        event->acceptProposedAction();
    }

    //QListWidget::dragEnterEvent(event);
}

void CWizDocumentListView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_TAGS))
    {
        event->acceptProposedAction();
    }

    //QListWidget::dragMoveEvent(event);
}

void CWizDocumentListView::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_TAGS))
    {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(itemAt(event->pos())))
        {
            QByteArray data = event->mimeData()->data(WIZNOTE_MIMEFORMAT_TAGS);
            QString strTagGUIDs = QString::fromUtf8(data, data.length());
            CWizStdStringArray arrayTagGUID;
            ::WizSplitTextToArray(strTagGUIDs, ';', arrayTagGUID);
            foreach (const CString& strTagGUID, arrayTagGUID)
            {
                WIZTAGDATA dataTag;
                if (m_db.TagFromGUID(strTagGUID, dataTag))
                {
                    CWizDocument doc(m_db, item->document());
                    doc.AddTag(dataTag);
                }
            }
        }

        event->acceptProposedAction();
    }
}

void CWizDocumentListView::on_tag_created(const WIZTAGDATA& tag)
{
    Q_UNUSED(tag);
}

void CWizDocumentListView::on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
{
    Q_UNUSED(tagOld);
    Q_UNUSED(tagNew);

}

void CWizDocumentListView::on_document_created(const WIZDOCUMENTDATA& document)
{
    if (acceptDocument(document))
    {
        if (-1 == documentIndexFromGUID(document.strGUID))
        {
            addDocument(document, true);
        }
    }
}

void CWizDocumentListView::on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);

    if (m_category.acceptDocument(documentNew))
    {
        int index = documentIndexFromGUID(documentNew.strGUID);
        if (-1 == index)
        {
            addDocument(documentNew, true);
        }
        else
        {
            if (CWizDocumentListViewItem* pItem = documentItemAt(index))
            {
                pItem->reload(m_db);
            }
        }

    }
    else
    {
        int index = documentIndexFromGUID(documentNew.strGUID);
        if (-1 != index)
        {
            takeItem(index);
        }
    }
}

void CWizDocumentListView::on_document_deleted(const WIZDOCUMENTDATA& document)
{
    int index = documentIndexFromGUID(document.strGUID);
    if (-1 != index)
    {
        takeItem(index);
    }
}

void CWizDocumentListView::on_document_AbstractModified(const WIZDOCUMENTDATA& document)
{
    int index = documentIndexFromGUID(document.strGUID);
    if (-1 == index)
        return;

    CWizDocumentListViewItem* pItem = documentItemAt(index);
    pItem->resetAbstract();

    QRect rc = visualItemRect(pItem);
    repaint(rc);
}

void CWizDocumentListView::on_action_selectTags()
{
    QList<QListWidgetItem*> items = selectedItems();
    if (items.isEmpty())
        return;

    if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(items.at(0)))
    {
        Q_UNUSED(item);

        if (!m_tagList)
        {
            m_tagList = new CWizTagListWidget(m_db, this);
            m_tagList->setLeftAlign(true);
        }

        m_tagList->setDocument(item->document());
        m_tagList->showAtPoint(QCursor::pos());
    }
}

void CWizDocumentListView::on_action_deleteDocument()
{
    QList<QListWidgetItem*> items = selectedItems();

    foreach (QListWidgetItem* it, items)
    {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(it))
        {
            CWizDocument doc(m_db, item->document());
            doc.Delete();
        }
    }
}

void CWizDocumentListView::on_action_encryptDocument()
{
    QList<QListWidgetItem*> items = selectedItems();

    foreach (QListWidgetItem* it, items) {
        if (CWizDocumentListViewItem* item = dynamic_cast<CWizDocumentListViewItem*>(it)) {
            CWizDocument doc(m_db, item->document());
            doc.encryptDocument();
        }
    }
}


int CWizDocumentListView::documentIndexFromGUID(const CString& strGUID)
{
    for (int i = 0; i < count(); i++)
    {
        if (CWizDocumentListViewItem *pItem = documentItemAt(i))
        {
            if (pItem->document().strGUID == strGUID)
            {
                return i;
            }
        }
    }

    return -1;
}

CWizDocumentListViewItem *CWizDocumentListView::documentItemAt(int index)
{
    return dynamic_cast<CWizDocumentListViewItem*>(item(index));
}

CWizDocumentListViewItem *CWizDocumentListView::documentItemFromIndex(const QModelIndex &index) const
{
    return dynamic_cast<CWizDocumentListViewItem*>(itemFromIndex(index));
}

WIZDOCUMENTDATA CWizDocumentListView::documentFromIndex(const QModelIndex &index) const
{
    return documentItemFromIndex(index)->document();
}

WIZABSTRACT CWizDocumentListView::documentAbstractFromIndex(const QModelIndex &index) const
{
    return documentItemFromIndex(index)->abstract(m_db);
}

CString CWizDocumentListView::documentTagsFromIndex(const QModelIndex &index) const
{
    return documentItemFromIndex(index)->tags(m_db);
}


#ifndef Q_OS_MAC
void CWizDocumentListView::updateGeometries()
{
    QListWidget::updateGeometries();

    // singleStep will initialized to item height(94 pixel), reset it
    verticalScrollBar()->setSingleStep(1);
}

void CWizDocumentListView::vscrollBeginUpdate(int delta)
{
    m_vscrollCurrent = 0;
    m_vscrollDelta = delta;

    if (!m_vscrollTimer.isActive())
        m_vscrollTimer.start(5);
}

void CWizDocumentListView::wheelEvent(QWheelEvent* event)
{
    vscrollBeginUpdate(event->delta());
}

void CWizDocumentListView::on_vscroll_update()
{
    if (qAbs(m_vscrollDelta) > m_vscrollCurrent) {
        verticalScrollBar()->setValue(m_vscrollOldPos - m_vscrollDelta/15);
        m_vscrollCurrent += qAbs(m_vscrollDelta/15);
    } else {
        m_vscrollTimer.stop();
    }
}

void CWizDocumentListView::on_vscroll_valueChanged(int value)
{
    m_vscrollOldPos = value;
}

void CWizDocumentListView::on_vscroll_actionTriggered(int action)
{
    switch (action) {
        case QAbstractSlider::SliderSingleStepAdd:
            vscrollBeginUpdate(-120);
            break;
        case QAbstractSlider::SliderSingleStepSub:
            vscrollBeginUpdate(120);
            break;
        default:
            return;
    }
}
#endif // Q_OS_MAC
