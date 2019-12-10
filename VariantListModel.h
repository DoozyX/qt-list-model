#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QObject>
#include <QVariant>

class VariantListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ count NOTIFY countChanged)

 public:
  explicit VariantListModel();
  ~VariantListModel();

 public:  // QAbstractItemModel interface reimplemented
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  bool setData(const QModelIndex& index, const QVariant& value, int role);
  QVariant data(const QModelIndex& index, int role) const;
  QHash<int, QByteArray> roleNames() const;

 public slots:  // public API
  void clear();
  int count() const;
  bool isEmpty() const;
  void append(const QVariant& item);
  void prepend(const QVariant& item);
  void insert(int idx, const QVariant& item);
  void appendList(const QVariantList& itemList);
  void prependList(const QVariantList& itemList);
  void replace(int pos, const QVariant& item);
  void insertList(int idx, const QVariantList& itemList);
  void move(int idx, int pos);
  void remove(int idx);
  QVariant get(int idx) const;
  QVariantList list() const;

 signals:  // notifiers
  void countChanged(int count);

 protected:
  void updateCounter();

 private:
  int m_count;
  QVariantList m_items;
  QHash<int, QByteArray> m_roles;
};
