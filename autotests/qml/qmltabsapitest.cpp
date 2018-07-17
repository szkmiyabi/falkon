/* ============================================================
* Falkon - Qt web browser
* Copyright (C) 2018 Anmol Gautam <tarptaeya@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#include "qmltabsapitest.h"
#include "autotests.h"
#include "qmltesthelper.h"
#include "mainapplication.h"
#include "tabwidget.h"

void QmlTabsApiTest::initTestCase()
{
}

void QmlTabsApiTest::cleanupTestCase()
{
}

void QmlTabsApiTest::testInitWindowCount()
{
    QCOMPARE(mApp->windowCount(), 1);
    QCOMPARE(mApp->getWindow()->tabCount(), 0);
}

void QmlTabsApiTest::testTabsAPI()
{
    // Tab Insertion
    QmlTestHelper qmlTest;
    QObject *qmlTabsObject = qmlTest.evaluateQObject("Falkon.Tabs");
    QVERIFY(qmlTabsObject);
    QSignalSpy qmlTabsInsertedSpy(qmlTabsObject, SIGNAL(tabInserted(QVariantMap)));
    qmlTest.evaluate("Falkon.Tabs.addTab({"
                     "    url: 'https://example.com/'"
                     "})");
    QCOMPARE(qmlTabsInsertedSpy.count(), 1);
    QVariantMap retMap1 = QVariant(qmlTabsInsertedSpy.at(0).at(0)).toMap();
    int index1 = retMap1.value(QSL("index"), -1).toInt();
    int windowId1 = retMap1.value(QSL("windowId"), -1).toInt();
    QCOMPARE(index1, 0);
    QCOMPARE(windowId1, 0);

    QObject *qmlTabObject1 = qmlTest.evaluateQObject("Falkon.Tabs.get({index: 0})");
    QVERIFY(qmlTabObject1);
    QCOMPARE(qmlTabObject1->property("url").toString(), "https://example.com/");
    QCOMPARE(qmlTabObject1->property("index").toInt(), 0);
    QCOMPARE(qmlTabObject1->property("pinned").toBool(), false);

    qmlTest.evaluate("Falkon.Tabs.addTab({"
                     "    url: 'https://another-example.com/',"
                     "})");
    QCOMPARE(qmlTabsInsertedSpy.count(), 2);
    QVariantMap retMap2 = QVariant(qmlTabsInsertedSpy.at(1).at(0)).toMap();
    int index2 = retMap2.value(QSL("index"), -1).toInt();
    int windowId2 = retMap2.value(QSL("windowId"), -1).toInt();
    QCOMPARE(index2, 1);
    QCOMPARE(windowId2, 0);

    bool pinnedTab = qmlTest.evaluate("Falkon.Tabs.pinTab({index: 1})").toBool();
    QVERIFY(pinnedTab);
    QObject *qmlTabObject2 = qmlTest.evaluateQObject("Falkon.Tabs.get({index: 0})");
    QVERIFY(qmlTabObject2);
    QCOMPARE(qmlTabObject2->property("url").toString(), "https://another-example.com/");
    QCOMPARE(qmlTabObject2->property("index").toInt(), 0);
    QCOMPARE(qmlTabObject2->property("pinned").toBool(), true);

    bool unpinnedTab = qmlTest.evaluate("Falkon.Tabs.unpinTab({index: 0})").toBool();
    QVERIFY(unpinnedTab);
    QObject *qmlTabObject3 = qmlTest.evaluateQObject("Falkon.Tabs.get({index: 0})");
    QVERIFY(qmlTabObject3);
    QCOMPARE(qmlTabObject3->property("url").toString(), "https://another-example.com/");
    QCOMPARE(qmlTabObject3->property("index").toInt(), 0);
    QCOMPARE(qmlTabObject3->property("pinned").toBool(), false);

    // Next-Previous-Current
    QCOMPARE(mApp->getWindow()->tabWidget()->currentIndex(), 0);
    qmlTest.evaluate("Falkon.Tabs.nextTab()");
    QCOMPARE(mApp->getWindow()->tabWidget()->currentIndex(), 1);
    qmlTest.evaluate("Falkon.Tabs.nextTab()");
    QCOMPARE(mApp->getWindow()->tabWidget()->currentIndex(), 0);
    qmlTest.evaluate("Falkon.Tabs.previousTab()");
    QCOMPARE(mApp->getWindow()->tabWidget()->currentIndex(), 1);
    qmlTest.evaluate("Falkon.Tabs.previousTab()");
    QCOMPARE(mApp->getWindow()->tabWidget()->currentIndex(), 0);
    qmlTest.evaluate("Falkon.Tabs.setCurrentIndex({index: 1})");
    QCOMPARE(mApp->getWindow()->tabWidget()->currentIndex(), 1);
    qmlTest.evaluate("Falkon.Tabs.setCurrentIndex({index: 0})");
    QCOMPARE(mApp->getWindow()->tabWidget()->currentIndex(), 0);

    // Move Tab
    QSignalSpy qmlTabsMovedSpy(qmlTabsObject, SIGNAL(tabMoved(QVariantMap)));
    qmlTest.evaluate("Falkon.Tabs.moveTab({from: 0, to:1, windowId: 0})");
    QCOMPARE(qmlTabsMovedSpy.count(), 1);

    // Tab Removal
    QCOMPARE(mApp->getWindow()->tabCount(), 2);
    QSignalSpy qmlTabsRemovedSpy(qmlTabsObject, SIGNAL(tabRemoved(QVariantMap)));
    qmlTest.evaluate("Falkon.Tabs.closeTab({index: 0})");
    QCOMPARE(qmlTabsRemovedSpy.count(), 1);
    QCOMPARE(mApp->getWindow()->tabCount(), 1);

    // windowId is different from current window
    BrowserWindow *otherWindow = mApp->createWindow(Qz::BW_NewWindow);
    QCOMPARE(otherWindow->tabCount(), 0);
    qmlTest.evaluate("Falkon.Tabs.addTab({"
                     "    url: 'https://example.com/',"
                     "    windowId: 1"
                     "})");
    QCOMPARE(otherWindow->tabCount(), 1);
    QObject *otherWindowTab = qmlTest.evaluateQObject("Falkon.Tabs.get({index: 0, windowId: 1})");
    QVERIFY(otherWindowTab);
    QObject *windowOfOtherWindowTab = qvariant_cast<QObject*>(otherWindowTab->property("browserWindow"));
    QVERIFY(windowOfOtherWindowTab);
    QCOMPARE(windowOfOtherWindowTab->property("id").toInt(), mApp->windowIdHash().value(otherWindow));
}

FALKONTEST_MAIN(QmlTabsApiTest)