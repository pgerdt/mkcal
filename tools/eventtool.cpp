#include <QDebug>
#include <QCoreApplication>
#include <QDate>

#include "extendedcalendar.h"
#include "extendedstorage.h"

// random
const char * const NotebookId("12345678-9876-1111-2222-565656565656");

using namespace mKCal;

static ExtendedCalendar::Ptr calendar;
static mKCal::Notebook::Ptr notebook;
static QSharedPointer<ExtendedStorage> storage;


void clearNotebook()
{
  storage->deleteNotebook(notebook);
  storage->save();
}

void createNotebook()
{
  notebook = mKCal::Notebook::Ptr(new mKCal::Notebook(NotebookId,
                                                      "test notebook",
                                                      QLatin1String(""),
                                                      "#001122",
                                                      false, // Not shared.
                                                      true, // Is master.
                                                      false, // Not synced to Ovi.
                                                      false, // Writable.
                                                      true, // Visible.
                                                      QLatin1String(""),
                                                      QLatin1String(""),
                                                      0));
  storage->addNotebook(notebook);
  storage->save();
}

void createAllDayEvent()
{
  qDebug() << "Adding all day event";
  auto event = KCalCore::Event::Ptr(new KCalCore::Event);
  QDate startDate(2014, 2, 1);
  event->setDtStart(KDateTime(startDate, QTime(), KDateTime::ClockTime));
  event->setAllDay(true);
  event->setSummary("test event with clock time");
  calendar->addEvent(event, NotebookId);
}

void createNormalEvents()
{
  qDebug() << "Adding some hour long events";

  auto event = KCalCore::Event::Ptr(new KCalCore::Event);
  QDate startDate(2014, 2, 3);
  event->setDtStart(KDateTime(startDate, QTime(12, 0), KDateTime::UTC));
  event->setDtEnd(KDateTime(startDate, QTime(13, 0), KDateTime::UTC));
  event->setSummary("test event with noon utc time");
  calendar->addEvent(event, NotebookId);

  event = KCalCore::Event::Ptr(new KCalCore::Event);
  event->setDtStart(KDateTime(startDate, QTime(17, 0), KDateTime::LocalZone));
  event->setDtEnd(KDateTime(startDate, QTime(18, 0), KDateTime::LocalZone));
  event->setSummary("test event with 17:00 local time (when created)");
  calendar->addEvent(event, NotebookId);

  event = KCalCore::Event::Ptr(new KCalCore::Event);
  event->setDtStart(KDateTime(startDate, QTime(20, 0), KDateTime::ClockTime));
  event->setDtEnd(KDateTime(startDate, QTime(21, 0), KDateTime::ClockTime));
  event->setSummary("test event with 20:00 clock time");
  calendar->addEvent(event, NotebookId);
}

void createRandomAllDay(QDate day)
{
  int length = qrand() % 3;
  auto event = KCalCore::Event::Ptr(new KCalCore::Event);
  KDateTime startDateTime(day, QTime(0, 0), KDateTime::ClockTime);

  event->setDtStart(startDateTime);
  event->setDtEnd(startDateTime.addDays(length));
  event->setAllDay(true);
  event->setSummary("random all day test event");
  calendar->addEvent(event, NotebookId);
}

void createRandomEvent(QDate day)
{
  int start = qrand() % 24;
  int length = qrand() % 8;
  auto event = KCalCore::Event::Ptr(new KCalCore::Event);
  KDateTime startDateTime(day, QTime(start, 0), KDateTime::LocalZone);

  event->setDtStart(startDateTime);
  event->setDtEnd(startDateTime.addSecs(length * 3600));
  event->setSummary("random test event");
  calendar->addEvent(event, NotebookId);
}

void createHugeNotebook()
{
  QDate current = QDate::currentDate().addYears(-2);
  QDate end = QDate::currentDate().addYears(2);

  for (; current < end; current = current.addDays(1)) {
    int eventsForDay = qrand() % 10;
    for (int i = 0; i < eventsForDay; ++i) {
      bool allDay = (qrand() % 100) < 30; // third of events as all-day
      if (allDay) {
        createRandomAllDay(current);
      } else {
        createRandomEvent(current);
      }
    }
  }
}

void printUsage()
{
  qDebug() << "Usage:";
  qDebug() << " --clear          -> clear test notebook";
  qDebug() << " --all-day        -> create all day event on 2014/2/1";
  qDebug() << " --normal-events  -> create some normal events on 2014/2/3";
  qDebug() << " --create-huge    -> create huge notebook with random events +-2 years current time";
}

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  calendar = ExtendedCalendar::Ptr(new ExtendedCalendar(KDateTime::Spec::LocalZone()));
  storage = calendar->defaultStorage(calendar);
  storage->open();
  notebook = storage->notebook(NotebookId);

  bool clear = (argc > 1 && QString(argv[1]) == "--clear");

  if (notebook.data() && clear) {
    clearNotebook();
    return 0;
  }

  if (notebook.isNull()) {
    createNotebook();
  }

  if (argc == 2) {
    QString argument(argv[1]);

    if (argument == "--all-day") {
      createAllDayEvent();
    } else if (argument == "--normal-events") {
      createNormalEvents();
    } else if (argument == "--create-huge") {
      createHugeNotebook();
    } else {
      printUsage();
    }
  } else {
    printUsage();
  }

  storage->save();

  notebook.clear();
  storage.clear();
  calendar.clear();

  return 0;
}

