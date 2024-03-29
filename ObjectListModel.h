#pragma once

#include <QAbstractListModel>
#include <QByteArray>
#include <QChar>
#include <QDebug>
#include <QHash>
#include <QList>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QObject>
#include <QString>
#include <QStringBuilder>
#include <QVariant>
#include <QVector>
#include <memory>
#include <utility>

#include "ObjectListModelBase.h"

template <class ItemType>
class ObjectListModel : public ObjectListModelBase {
 public:
  explicit ObjectListModel(QByteArray uidRole = QByteArray(), QByteArray displayRole = QByteArray())
      : ObjectListModelBase(),
        m_uidRoleName(std::move(uidRole)),
        m_dispRoleName(std::move(displayRole)),
        m_metaObj(ItemType::staticMetaObject) {
    static QSet<QByteArray> roleNamesBlacklist;
    if (roleNamesBlacklist.isEmpty()) {
      roleNamesBlacklist << QByteArrayLiteral("id") << QByteArrayLiteral("index") << QByteArrayLiteral("class")
                         << QByteArrayLiteral("model") << QByteArrayLiteral("modelData");
    }
    static const char* HANDLER = "onItemPropertyChanged()";
    m_handler = metaObject()->method(metaObject()->indexOfMethod(HANDLER));
    if (!m_dispRoleName.isEmpty()) {
      m_roles.insert(Qt::DisplayRole, QByteArrayLiteral("display"));
    }
    m_roles.insert(baseRole(), QByteArrayLiteral("qtObject"));
    const auto len = m_metaObj.propertyCount();
    for (auto propertyIdx = 0, role = (baseRole() + 1); propertyIdx < len; propertyIdx++, role++) {
      auto metaProp = m_metaObj.property(propertyIdx);
      const auto propName = QByteArray(metaProp.name());
      if (!roleNamesBlacklist.contains(propName)) {
        m_roles.insert(role, propName);
        if (metaProp.hasNotifySignal()) {
          m_signalIdxToRole.insert(metaProp.notifySignalIndex(), role);
        }
      } else {
        static const QByteArray CLASS_NAME = (QByteArrayLiteral("QQmlObjectListModel<") % m_metaObj.className() % '>');
        qWarning() << "Can't have" << propName << "as a role name in" << qPrintable(CLASS_NAME);
      }
    }
  }
  bool setData(const QModelIndex& index, const QVariant& value, int role) override {
    bool ret = false;
    auto item = at(index.row());
    const auto roleName = (role != Qt::DisplayRole ? m_roles.value(role, emptyBA()) : m_dispRoleName);
    if (item != Q_NULLPTR && role != baseRole() && !roleName.isEmpty()) {
      ret = item->setProperty(roleName, value);
    }
    return ret;
  }
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
    QVariant ret;
    auto item = at(index.row());
    const auto roleName = (role != Qt::DisplayRole ? m_roles.value(role, emptyBA()) : m_dispRoleName);
    if (item != Q_NULLPTR && !roleName.isEmpty()) {
      ret.setValue(role != baseRole() ? item->property(roleName) : QVariant::fromValue(static_cast<QObject*>(item)));
    }
    return ret;
  }
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override { return m_roles; }
  using const_iterator = typename QList<ItemType*>::const_iterator;
  [[nodiscard]] const_iterator begin() const { return m_items.begin(); }
  [[nodiscard]] const_iterator end() const { return m_items.end(); }
  [[nodiscard]] const_iterator constBegin() const { return m_items.constBegin(); }
  [[nodiscard]] const_iterator constEnd() const { return m_items.constEnd(); }

 public:  // C++ API
  [[nodiscard]] ItemType* at(int idx) const {
    ItemType* ret = Q_NULLPTR;
    if (idx >= 0 && idx < m_items.size()) {
      ret = m_items.value(idx);
    }
    return ret;
  }
  [[nodiscard]] ItemType* getByUid(const QString& uid) const {
    return (!m_indexByUid.isEmpty() ? m_indexByUid.value(uid, Q_NULLPTR) : Q_NULLPTR);
  }
  [[nodiscard]] int roleForName(const QByteArray& name) const override { return m_roles.key(name, -1); }
  [[nodiscard]] int count() const override { return m_count; }
  [[nodiscard]] int size() const override { return m_count; }
  [[nodiscard]] bool isEmpty() const override { return m_items.isEmpty(); }
  bool contains(ItemType* item) const { return m_items.contains(item); }
  int indexOf(ItemType* item) const { return m_items.indexOf(item); }
  void clear() override {
    if (!m_items.isEmpty()) {
      beginRemoveRows(noParent(), 0, m_items.count() - 1);
      for (auto item : m_items) {
        dereferenceItem(item);
      }
      m_items.clear();
      updateCounter();
      endRemoveRows();
    }
  }
  void append(ItemType* item) {
    if (item != Q_NULLPTR) {
      const int pos = m_items.count();
      beginInsertRows(noParent(), pos, pos);
      m_items.append(item);
      referenceItem(item);
      updateCounter();
      endInsertRows();
    }
  }
  void prepend(ItemType* item) {
    if (item != Q_NULLPTR) {
      beginInsertRows(noParent(), 0, 0);
      m_items.prepend(item);
      referenceItem(item);
      updateCounter();
      endInsertRows();
    }
  }
  void insert(int idx, ItemType* item) {
    if (item != Q_NULLPTR) {
      beginInsertRows(noParent(), idx, idx);
      m_items.insert(idx, item);
      referenceItem(item);
      updateCounter();
      endInsertRows();
    }
  }
  void append(const QList<ItemType*>& itemList) {
    if (!itemList.isEmpty()) {
      const auto pos = m_items.count();
      beginInsertRows(noParent(), pos, pos + itemList.count() - 1);
      m_items.reserve(m_items.count() + itemList.count());
      m_items.append(itemList);
      for (auto item : itemList) {
        referenceItem(item);
      }
      updateCounter();
      endInsertRows();
    }
  }
  void prepend(const QList<ItemType*>& itemList) {
    if (!itemList.isEmpty()) {
      beginInsertRows(noParent(), 0, itemList.count() - 1);
      m_items.reserve(m_items.count() + itemList.count());
      auto offset = 0;
      for (auto item : itemList) {
        m_items.insert(offset, item);
        referenceItem(item);
        offset++;
      }
      updateCounter();
      endInsertRows();
    }
  }
  void insert(int idx, const QList<ItemType*>& itemList) {
    if (!itemList.isEmpty()) {
      beginInsertRows(noParent(), idx, idx + itemList.count() - 1);
      m_items.reserve(m_items.count() + itemList.count());
      auto offset = 0;
      for (auto item : itemList) {
        m_items.insert(idx + offset, item);
        referenceItem(item);
        offset++;
      }
      updateCounter();
      endInsertRows();
    }
  }
  void move(int idx, int pos) override {
    if (idx != pos) {
      // FIXME : use begin/end MoveRows when supported by Repeater, since then use remove/insert pair
      // beginMoveRows (noParent (), idx, idx, noParent (), (idx < pos ? pos +1 : pos));
      beginRemoveRows(noParent(), idx, idx);
      beginInsertRows(noParent(), pos, pos);
      m_items.move(idx, pos);
      endRemoveRows();
      endInsertRows();
      // endMoveRows ();
    }
  }
  void remove(ItemType* item) {
    if (item != Q_NULLPTR) {
      const auto idx = m_items.indexOf(item);
      remove(idx);
    }
  }
  void remove(int idx) override {
    if (idx >= 0 && idx < m_items.size()) {
      beginRemoveRows(noParent(), idx, idx);
      auto item = m_items.takeAt(idx);
      dereferenceItem(item);
      updateCounter();
      endRemoveRows();
    }
  }
  [[nodiscard]] ItemType* first() const { return m_items.first(); }
  [[nodiscard]] ItemType* last() const { return m_items.last(); }
  [[nodiscard]] const QList<ItemType*>& toList() const { return m_items; }

 public:  // QML slots implementation
  void append(QObject* item) override { append(qobject_cast<ItemType*>(item)); }
  void prepend(QObject* item) override { prepend(qobject_cast<ItemType*>(item)); }
  void insert(int idx, QObject* item) override { insert(idx, qobject_cast<ItemType*>(item)); }
  void remove(QObject* item) override { remove(qobject_cast<ItemType*>(item)); }
  [[nodiscard]] bool contains(QObject* item) const override { return contains(qobject_cast<ItemType*>(item)); }
  [[nodiscard]] int indexOf(QObject* item) const override { return indexOf(qobject_cast<ItemType*>(item)); }
  [[nodiscard]] int indexOf(const QString& uid) const { return indexOf(getByID(uid)); }
  [[nodiscard]] QObject* getAt(int idx) const override { return static_cast<QObject*>(at(idx)); }
  [[nodiscard]] QObject* getByID(const QString& uid) const override { return static_cast<QObject*>(getByUid(uid)); }
  [[nodiscard]] QObject* getFirst() const override { return static_cast<QObject*>(first()); }
  [[nodiscard]] QObject* getLast() const override { return static_cast<QObject*>(last()); }

 protected:  // internal stuff
  static const QString& emptyStr() {
    static const auto ret = QString();
    return ret;
  }
  static const QByteArray& emptyBA() {
    static const auto ret = QByteArrayLiteral("");
    return ret;
  }
  static const QModelIndex& noParent() {
    static const auto ret = QModelIndex();
    return ret;
  }
  static const int& baseRole() {
    static const int ret = Qt::UserRole;
    return ret;
  }
  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override {
    return (!parent.isValid() ? m_items.count() : 0);
  }
  void referenceItem(ItemType* item) {
    if (item != Q_NULLPTR) {
      if (!item->parent()) {
        item->setParent(this);
      }
      for (QHash<int, int>::const_iterator it = m_signalIdxToRole.constBegin(); it != m_signalIdxToRole.constEnd();
           ++it) {
        connect(item, item->metaObject()->method(it.key()), this, m_handler, Qt::UniqueConnection);
      }
      if (!m_uidRoleName.isEmpty()) {
        const QString key = m_indexByUid.key(item, emptyStr());
        if (!key.isEmpty()) {
          m_indexByUid.remove(key);
        }
        const QString value = item->property(m_uidRoleName).toString();
        if (!value.isEmpty()) {
          m_indexByUid.insert(value, item);
        }
      }
    }
  }
  void dereferenceItem(ItemType* item) {
    if (item != Q_NULLPTR) {
      disconnect(this, Q_NULLPTR, item, Q_NULLPTR);
      disconnect(item, Q_NULLPTR, this, Q_NULLPTR);
      if (!m_uidRoleName.isEmpty()) {
        const auto key = m_indexByUid.key(item, emptyStr());
        if (!key.isEmpty()) {
          m_indexByUid.remove(key);
        }
      }
      if (item->parent() == this) {  // FIXME : maybe that's not the best way to test ownership ?
        item->deleteLater();
      }
    }
  }
  void onItemPropertyChanged() override {
    auto item = qobject_cast<ItemType*>(sender());
    const auto row = m_items.indexOf(item);
    const auto sig = senderSignalIndex();
    const auto role = m_signalIdxToRole.value(sig, -1);
    if (row >= 0 && role >= 0) {
      const auto index = QAbstractListModel::index(row, 0, noParent());
      QVector<int> rolesList;
      rolesList.append(role);
      if (m_roles.value(role) == m_dispRoleName) {
        rolesList.append(Qt::DisplayRole);
      }
      Q_EMIT dataChanged(index, index, rolesList);
    }
    if (!m_uidRoleName.isEmpty()) {
      const auto roleName = m_roles.value(role, emptyBA());
      if (!roleName.isEmpty() && roleName == m_uidRoleName) {
        const auto key = m_indexByUid.key(item, emptyStr());
        if (!key.isEmpty()) {
          m_indexByUid.remove(key);
        }
        const auto value = item->property(m_uidRoleName).toString();
        if (!value.isEmpty()) {
          m_indexByUid.insert(value, item);
        }
      }
    }
  }
  inline void updateCounter() {
    if (m_count != m_items.count()) {
      m_count = m_items.count();
      Q_EMIT countChanged();
    }
  }

 private:  // data members
  int m_count{0};
  QByteArray m_uidRoleName;
  QByteArray m_dispRoleName;
  QMetaObject m_metaObj;
  QMetaMethod m_handler;
  QHash<int, QByteArray> m_roles;
  QHash<int, int> m_signalIdxToRole;
  QList<ItemType*> m_items;
  QHash<QString, ItemType*> m_indexByUid;
};

#define UNIQUE_OBJMODEL_PROPERTY(type, name)                           \
 protected:                                                            \
  Q_PROPERTY(ObjectListModelBase* name READ get_##name CONSTANT)       \
 private:                                                              \
  std::unique_ptr<ObjectListModel<type>> m_##name;                     \
                                                                       \
 public:                                                               \
  ObjectListModel<type>* get_##name() const { return m_##name.get(); } \
                                                                       \
 private:

#define UNIQUE_OBJMODEL_PROPERTY_INIT(type, name, idRole)                                            \
 protected:                                                                                          \
  Q_PROPERTY(ObjectListModelBase* name READ get_##name CONSTANT)                                     \
 private:                                                                                            \
  std::unique_ptr<ObjectListModel<type>> m_##name = std::make_unique<ObjectListModel<type>>(idRole); \
                                                                                                     \
 public:                                                                                             \
  ObjectListModel<type>* get_##name() const { return m_##name.get(); }                               \
                                                                                                     \
 private:
