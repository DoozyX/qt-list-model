#pragma once

#include <QAbstractListModel>

class ObjectListModelBase : public QAbstractListModel {  // abstract Qt base class
  Q_OBJECT
  Q_PROPERTY(int count READ count NOTIFY countChanged)

 public:
  explicit ObjectListModelBase() : QAbstractListModel(nullptr) {}

  [[nodiscard]] Q_INVOKABLE virtual int size() const = 0;
  [[nodiscard]] Q_INVOKABLE virtual int count() const = 0;
  [[nodiscard]] Q_INVOKABLE virtual bool isEmpty() const = 0;
  [[nodiscard]] Q_INVOKABLE virtual bool contains(QObject* item) const = 0;
  [[nodiscard]] Q_INVOKABLE virtual int indexOf(QObject* item) const = 0;
  [[nodiscard]] Q_INVOKABLE virtual int roleForName(const QByteArray& name) const = 0;
  Q_INVOKABLE virtual void clear() = 0;
  Q_INVOKABLE virtual void append(QObject* item) = 0;
  Q_INVOKABLE virtual void prepend(QObject* item) = 0;
  Q_INVOKABLE virtual void insert(int idx, QObject* item) = 0;
  Q_INVOKABLE virtual void move(int idx, int pos) = 0;
  Q_INVOKABLE virtual void remove(QObject* item) = 0;
  Q_INVOKABLE virtual void remove(int idx) = 0;
  [[nodiscard]] Q_INVOKABLE virtual QObject* getAt(int idx) const = 0;
  [[nodiscard]] Q_INVOKABLE virtual QObject* getByID(const QString& uid) const = 0;
  [[nodiscard]] Q_INVOKABLE virtual QObject* getFirst() const = 0;
  [[nodiscard]] Q_INVOKABLE virtual QObject* getLast() const = 0;

 protected Q_SLOTS:  // internal callback
  virtual void onItemPropertyChanged() = 0;

 Q_SIGNALS:  // notifier
  void countChanged();
};
