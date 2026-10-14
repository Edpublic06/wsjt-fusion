#include <QtTest>
#include <QMouseEvent>
#include <QSignalSpy>

#include "commons.h"
#include "widgets/plotter.h"

namespace {
dec_data_t storage;
}
dec_data_t& dec_data = storage;
QVector<QColor> g_ColorTbl (256, Qt::black);

class TestablePlotter final : public CPlotter
{
public:
  using CPlotter::mouseDoubleClickEvent;

  TestablePlotter ()
  {
    setMode ("JTTY");
    setNsps (0, 6912);
    setCurrent (false);
    setPlot2dGain (0);
    setPlot2dZero (0);
    setCumulative (true);
    setLinearAvg (false);
    setReference (false);
    setQ65_Sync (false);
    setVHF (false);
    setSingleDecode (false);
    setFlatten (false, false);
    setWaterfallAvg (2);
    setBars (false);
    setTimestamp (1);
    setDiskUTC (-1);
    setDataFromDisk (false);
    setTol (100);
    resize (700, 350);
    show ();
    QCoreApplication::processEvents ();
  }

  void push (JttyWaterfallRow const& row)
  {
    std::array<float, 2048> values {};
    if (row.gap) values.fill (1.e30f);
    setJttyRow (row);
    draw (values.data (), true, false);
  }

  void clickRow (int row)
  {
    QMouseEvent event {QEvent::MouseButtonDblClick, QPointF (100, 30 + row),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
    mouseDoubleClickEvent (&event);
  }
};

class TestJttyWaterfall final : public QObject
{
  Q_OBJECT
private slots:
  void rowIdentitySurvivesAverageResizeAndReplot ()
  {
    TestablePlotter plotter;
    QSignalSpy selected (&plotter, &CPlotter::jttyDecodeAgainAtSample);
    qint64 const start = qint64 (1) << 34;
    plotter.push ({12, start, start + 6912, 180000, false});
    plotter.setWaterfallAvg (11);
    plotter.push ({12, start + 6912, start + 6912 + 38016, 183168, false});
    plotter.resize (1000, 450);
    QCoreApplication::processEvents ();
    plotter.replot ();
    plotter.clickRow (1);
    QCOMPARE (selected.size (), 1);
    QCOMPARE (selected[0][0].toULongLong (), quint64 (12));
    QCOMPARE (selected[0][1].toLongLong (), start + 6912);
    plotter.clickRow (0);
    QCOMPARE (selected.size (), 2);
    QCOMPARE (selected[1][1].toLongLong (), start + 6912 + 38016);
  }

  void gapsAndUnpopulatedRowsDoNotSelectUnrelatedAudio ()
  {
    TestablePlotter plotter;
    QSignalSpy selected (&plotter, &CPlotter::jttyDecodeAgainAtSample);
    plotter.push ({4, 0, 3456, 288, false});
    plotter.push ({4, 3456, 3456, 288, true});
    plotter.push ({5, 0, 3456, 190000, false});
    plotter.clickRow (1);
    plotter.clickRow (10);
    QCOMPARE (selected.size (), 0);
    plotter.clickRow (2);
    QCOMPARE (selected.size (), 1);
    QCOMPARE (selected[0][0].toULongLong (), quint64 (4));
    plotter.clickRow (0);
    QCOMPARE (selected.size (), 2);
    QCOMPARE (selected[1][0].toULongLong (), quint64 (5));
  }
};

QTEST_MAIN (TestJttyWaterfall)
#include "test_jtty_waterfall.moc"
