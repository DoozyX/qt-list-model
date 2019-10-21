#pragma once

#include <QQmlEngine>
#include <QtQml>

#include "ObjectListModel.h"
#include "VariantListModel.h"

static void registerSmartDataModel(QQmlEngine* engine) {
  Q_UNUSED(engine)

  const char* uri = "QtQmlTricks.SmartDataModels";  // @uri QtQmlTricks.SmartDataModels
  const int maj = 2;
  const int min = 0;
  const char* msg = "!!!";

  qmlRegisterUncreatableType<QQmlObjectListModelBase>(uri, maj, min, "ObjectListModel", msg);
  qmlRegisterUncreatableType<VariantListModel>(uri, maj, min, "VariantListModel", msg);
}
