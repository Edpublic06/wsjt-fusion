#include <QtTest>
#include <algorithm>
#include <vector>

#include "JttyReceiveHistory.hpp"

class TestJttyReceiveHistory final : public QObject
{
  Q_OBJECT
private slots:
  void rotationsRetainExactSuffixAcrossPacketBoundaries ()
  {
    Jtty::ReceiveHistory history {17};
    qint64 const origin = qint64 (1) << 35;
    std::vector<short> received;
    for (int packet = 0; packet < 200; ++packet) {
      int const count = 1 + packet % 7;
      std::vector<short> incoming (count);
      for (int i = 0; i < count; ++i) incoming[i] = short (received.size () + i);
      auto const first = origin + qint64 (received.size ());
      received.insert (received.end (), incoming.begin (), incoming.end ());
      QVERIFY (history.append (12, first, incoming.data (), count));
      auto const retained = std::min<qint64> (17, received.size ());
      auto const end = origin + qint64 (received.size ());
      QCOMPARE (history.size (), retained);
      QCOMPARE (history.first (12), end - retained);
      QCOMPARE (history.end (12), end);
      std::vector<short> expected (received.end () - retained, received.end ());
      QVERIFY (history.snapshot (12, end - retained, end) == expected);
      QVERIFY (history.snapshot (12, end - retained - 1, end).empty ());
      QVERIFY (history.snapshot (12, end - retained, end + 1).empty ());
    }
  }

  void snapshotsNeverJoinReceptionsWithMatchingCoordinates ()
  {
    Jtty::ReceiveHistory history {12};
    short const first[] = {1, 2, 3, 4};
    short const second[] = {5, 6, 7, 8};
    short const third[] = {9, 10, 11, 12};
    QVERIFY (history.append (1, 0, first, 4));
    QVERIFY (history.append (2, 4, second, 4));
    QVERIFY (history.snapshot (1, 0, 8).empty ());
    QVERIFY (history.snapshot (2, 0, 8).empty ());
    QVERIFY (history.append (3, 0, third, 4));
    QVERIFY (history.snapshot (1, 0, 4) == std::vector<short> ({1, 2, 3, 4}));
    QVERIFY (history.snapshot (3, 0, 4) == std::vector<short> ({9, 10, 11, 12}));
    QVERIFY (history.append (3, 4, third, 4));
    QCOMPARE (history.first (1), qint64 (-1));
    QCOMPARE (history.end (1), qint64 (-1));
    QCOMPARE (history.first (2), qint64 (4));
    QCOMPARE (history.first (3), qint64 (0));
  }

  void gapsAndDuplicatesFailWithoutChangingRetainedAudio ()
  {
    Jtty::ReceiveHistory history {10};
    short const samples[] = {1, 2, 3, 4};
    QVERIFY (history.append (3, 7, samples, 4));
    QVERIFY (!history.append (3, 12, samples, 4));
    QVERIFY (!history.append (3, 7, samples, 4));
    QCOMPARE (history.size (), qint64 (4));
    QCOMPARE (history.end (3), qint64 (11));
    QVERIFY (history.snapshot (3, 7, 11) == std::vector<short> ({1, 2, 3, 4}));
    QVERIFY (history.append (3, 11, samples, 4));
    QVERIFY (history.snapshot (3, 9, 13) == std::vector<short> ({3, 4, 1, 2}));
  }

  void snapshotsOwnAudioBeyondInputReuseAndEviction ()
  {
    Jtty::ReceiveHistory history {6};
    short samples[] = {1, 2, 3, 4};
    QVERIFY (history.append (4, 0, samples, 4));
    std::fill (std::begin (samples), std::end (samples), 9);
    auto const saved = history.snapshot (4, 1, 4);
    QVERIFY (saved == std::vector<short> ({2, 3, 4}));
    QVERIFY (history.append (4, 4, samples, 4));
    QVERIFY (history.snapshot (4, 0, 4).empty ());
    history.clear ();
    QCOMPARE (history.size (), qint64 (0));
    QVERIFY (history.snapshot (4, 1, 4).empty ());
    QVERIFY (saved == std::vector<short> ({2, 3, 4}));
  }
};

QTEST_APPLESS_MAIN (TestJttyReceiveHistory)
#include "test_jtty_receive_history.moc"
