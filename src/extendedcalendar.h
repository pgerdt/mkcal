/*
  This file is part of the mkcal library.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
  Contact: Alvaro Manera <alvaro.manera@nokia.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the ExtendedCalendar class.

  @author Tero Aho \<ext-tero.1.aho@nokia.com\>
  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
 */

/**
  @mainpage

  @author Tero Aho \<ext-tero.1.aho@nokia.com\>

  @section copyright Copyright

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>

  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

  Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.

  Contact: Alvaro Manera <alvaro.manera@nokia.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  @section introduction Introduction

  KCal library maps the ICAL standard (RFC 2445) to a collection of classes.
  The original KCal is part of the kdepimlibs package and was developed
  in the KDE project for the backend of KOrganizer. That effort was
  extended for the Nokia Maemo Organizer backend. Maemo backend library
  is called libextendedkcal and its version number reflects the corresponding
  kdepimlibs version number. The ultimate target is to push all Maemo
  related changes to upstream eventually.

  @section architecture Architecture

  There are two important base classes.
  <ul>
  <li>Abstract class <b>Calendar</b> contain incidences
  (events, todos, journals) and methods for adding/deleting/querying them.
  It is implemented by <b>ExtendedCalendar</b> that also has some Maemo
  specific features.
  <li>Abstract class <b>ExtendedStorage</b> defines the interface to load
  and save calendar incidences into permanent storage. It is implemented
  by a few diffent storages, like <b>SqliteStorage</b> which is the default
  Maemo storage.
  </ul>

  Calendars can exist without storages but storages must always be linked
  to a calendar. The normal way to create calendar and default storage is:
  @code
  ExtendedCalendar calendar( QLatin1String( "UTC" ) );
  ExtendedStorage *storage = calendar.defaultStorage();
  storage->open();
  @endcode

  Initially calendar is empty unless you create new incidences into it,
  or use the storage to load incidences into the calendar.
  Original KCal provided only means for loading all incidences at once,
  but that may be too costly for an embedded device (loading could take
  tens of seconds for very large calendars). ExtendedStorage added
  methods for smart loading only those incidences that must be shown
  to the user at that time.

  When you add incidences to the calendar, or modify/delete existing
  ones, you must call storage->save() to write incidences into persistent
  storage. Calendar methods always operate in memory only, you can make
  a series of changes in a calendar and then 'commit' them all at once
  by calling storage->save().

  Another Maemo extension to KCal are multiple notebooks. Every incidence
  is assigned to a notebook. The correct way to create an incidence
  (simple todo in this case) is this:
  @code
  KDateTime now =
    KDateTime::currentDateTime( KDateTime::Spec( KSystemTimeZones::zone( "Europe/Helsinki" ) ) );
  Notebook *personal;

  Todo::Ptr todo = Todo::Ptr( new Todo() );
  todo->setSummary( "A todo for testing" );
  todo->setDtDue( now.addDays(1), true );
  todo->setHasDueDate( true );
  calendar.addTodo( todo );
  calendar.setNotebook( todo, personal->uid() );
  storage.save();
  @endcode

  Notice that we first create the incidence, then add it to the calendar
  and then tell calendar which notebook the incidence belongs to. Finally
  the note is saved to the storage.

  Notebooks are created/modified/deleted through storages.
  Notebooks are referred by their name only in the calendar.
  To create it, see this example:
  @code
  storage.addNotebook( notebook );
  @endcode

  @section environment Environment

  Here is an ascii art diagram of the typical organiser environment.

  There are usually two processes using libextendedkcal, one is the calendar
  application visible to the user, and the other is synchronization daemon
  that updates calendar data periodically on the background. ExtendedStorage
  may be a separate process but it also can be a statically linked library
  (like SQLite is).

  Change notifications are sent when incidences are saved in either process.
  Also during save storage sets possible alarms to alarm daemon which displays
  a dialog to the user when alarm triggers. Based on user action the alarm
  may be canceled, snoozed, or opened to the user in calendar application.

  @code
       +---------------+                                         +---------------+
       |               |                                         |               |
  +--->| Organiser UI  |                                         |  Sync Service |
  |    |               |                                         |               |
  |    =================            +---------------+            =================
  |    |               |---save---->|               |<--save-----|               |
  |    |libextendedkcal|<--load-----|ExtendedStorage|---load---->|libextendedkcal|
  |    |               |<--changed--|               |---changed->|               |
  |    =================            +---------------+            =================
  |    |    libtimed   |                                         |    libtimed   |
  |    +---------------+                                        +---------------+
  |           |                                                       |
  |           |                                                       |
  |           |                    +--------------+                   |
  |           +----sets alarms---->|              |<----sets alarms---+
  +--------triggers alarms---------| Alarm Daemon |
              +----user action---->|              |---display dialog--+
              |                    +--------------+                   |
              |                                                       |
              |                    +--------------+                   |
              +--------------------| Alarm Dialog |<------------------+
                                   +--------------+
  @endcode

  @section storages Storages

  It is also possible to add more than one storage into a calendar.
  This feature can be used to save some notebooks into a different
  (possibly encrypted etc.) storage. For example, if you have this:

  @code
  ExtendedCalendar::Ptr calendar = new ExtendedCalendar(QLatin1String("UTC"));
  ExtendedStorage *storage1 =
    new SqliteStorage( calendar, QLatin1String("/home/user/publidb" ), false, true );
  ExtendedStorage *storage2 =
    new SqliteStorage( calendar, QLatin1String( "/home/user/privatedb" ), false, true );
  Notebook *notebook1 = new Notebook( "Public", ... );
  Notebook *notebook2 = new Notebook( "Private", ... );
  storage1->addNotebook( notebook1 );
  storage2->addNotebook( notebook2 );
  @endcode

  then all incidences tagged with notebook "Private" will be saved only by
  the second storage into a different database. The last parameter in
  storage constructor tells the storage to always validate notebooks, meaning
  that it will only act for incidences that have the notebook inside the
  storage.

  If you create a storage like this (the last parameter is now false):

  @code
  ExtendedStorage *storage =
    new SqliteStorage( calendar, QLatin1String( "/home/user/publidb" ), false, false );
  @endcode

  it means that this storage is not validating notebooks, meaning that it
  will load incidences from any notebook and it will save incidences
  for any notebook if no other storage is already handling that.

  @section examples Examples

  Package libextendedkcal-tests contains many unit and module test files
  that may provide some assistance.
 */
#ifndef MKCAL_EXTENDEDCALENDAR_H
#define MKCAL_EXTENDEDCALENDAR_H

#include "mkcal_export.h"

#include <memorycalendar.h>
#include <icaltimezones.h>

namespace mKCal {

class ExtendedStorage;

/**
  @brief
  This class provides a calendar cached into memory.
*/
class MKCAL_EXPORT ExtendedCalendar : public KCalCore::MemoryCalendar
{
  public:
    /**
      Incidence sort keys.
      See calendar.h for Event, Todo or Journal specific sorts.
    */
    enum IncidenceSortField {
      IncidenceSortUnsorted,   /**< Do not sort Incidences */
      IncidenceSortDate,       /**< Sort Incidence chronologically,
                                  Events by start date,
                                  Todos by due date,
                                  Journals by date */
      IncidenceSortCreated     /* < Sort Incidences based on creation time */
    };

    /**
      A shared pointer to a ExtendedCalendar
    */
    typedef QSharedPointer<ExtendedCalendar> Ptr;

    /**
      @copydoc
      Calendar::Calendar(const KDateTime::Spec &)
    */
    explicit ExtendedCalendar( const KDateTime::Spec &timeSpec );

    /**
      @copydoc
      Calendar::Calendar(const QString &)
    */
    explicit ExtendedCalendar( const QString &timeZoneId );

    /**
      @copydoc
      Calendar::~Calendar()
    */
    virtual ~ExtendedCalendar();

    /**
      @copydoc
      Calendar::reload()
    */
    bool reload();

    /**
      @copydoc
      Calendar::save()
    */
    bool save();

    /**
      Clears out the current calendar, freeing all used memory etc. etc.
    */
    void close();

    /**
      Creates an ICalTimeZone instance and adds it to calendar's ICalTimeZones
      collection or returns an existing instance for the MSTimeZone component.

      @param tz the MSTimeZone structure to parse

      @see ICalTimeZoneSource::parse( MSTimeZone *, ICalTimeZones & )
    */
    KCalCore::ICalTimeZone parseZone( KCalCore::MSTimeZone *tz );

    /**
      @copydoc
      Calendar::doSetTimeSpec()
    */
    void doSetTimeSpec( const KDateTime::Spec &timeSpec );

    /**
      Dissociate only one single Incidence from a recurring Incidence.
      Incidence for the specified @a date will be dissociated and returned.

      @param incidence is a pointer to a recurring Incidence.
      @param dateTime is the KDateTime within the recurring Incidence on which
      the dissociation will be performed.
      @param spec is the spec in which the @a dateTime is formulated.
      @return a pointer to a dissociated Incidence
    */
    KCalCore::Incidence::Ptr dissociateSingleOccurrence( const KCalCore::Incidence::Ptr &incidence,
                                                         const KDateTime &dateTime,
                                                         const KDateTime::Spec &spec );

  // Event Specific Methods //

    /**
      @copydoc
      Calendar::addEvent()
    */
    bool addEvent( const KCalCore::Event::Ptr &event );

    /**
      @copydoc
      Calendar::deleteEventInstances()
    */
    bool deleteEventInstances( const KCalCore::Event::Ptr &event );

    /**
      @copydoc
      Calendar::deleteEvent()
    */
    bool deleteEvent( const KCalCore::Event::Ptr &event );

    /**
      @copydoc
      Calendar::deleteAllEvents()
    */
    void deleteAllEvents();

    /**
      @copydoc
      Calendar::rawEvents(KCalCore::EventSortField, KCalCore::SortDirection)
    */
    KCalCore::Event::List rawEvents(
      KCalCore::EventSortField sortField = KCalCore::EventSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    /**
      @copydoc
      Calendar::rawEvents(const QDate &, const QDate &, const KDateTime::Spec &, bool)
    */
    KCalCore::Event::List rawEvents(
      const QDate &start, const QDate &end,
      const KDateTime::Spec &timespec = KDateTime::Spec(),
      bool inclusive = false ) const;

    /**
      @copydoc
      Calendar::rawEventsForDate(const QDate &, const KDateTime::Spec &,
                                 KCalCore::EventSortField, KCalCore::SortDirection)
    */
    KCalCore::Event::List rawEventsForDate(
      const QDate &date, const KDateTime::Spec &timespec = KDateTime::Spec(),
      KCalCore::EventSortField sortField = KCalCore::EventSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    /**
      @copydoc
      Calendar::rawEventsForDate(const KDateTime &)
    */
    KCalCore::Event::List rawEventsForDate( const KDateTime &dt ) const;

    /**
      @copydoc
      Calendar::event()
    */
    KCalCore::Event::Ptr event( const QString &uid,
                                const KDateTime &recurrenceId = KDateTime() ) const;

    /**
      @copydoc
      Calendar::deletedEvent()
    */
    KCalCore::Event::Ptr deletedEvent( const QString &uid,
                                       const KDateTime &recurrenceId = KDateTime() ) const;

    /**
      @copydoc
      Calendar::deletedEvents(KCalCore::EventSortField, KCalCore::SortDirection)
    */
    KCalCore::Event::List deletedEvents(
      KCalCore::EventSortField sortField = KCalCore::EventSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    /**
      @copydoc
      Calendar::eventInstances(const KCalCore::Incidence::Ptr &,
                               KCalCore::EventSortField, KCalCore::SortDirection)
    */
    KCalCore::Event::List eventInstances(
      const KCalCore::Incidence::Ptr &event,
      KCalCore::EventSortField sortField = KCalCore::EventSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

  // To-do Specific Methods //

    /**
      @copydoc
      Calendar::addTodo()
    */
    bool addTodo( const KCalCore::Todo::Ptr &todo );

    /**
      @copydoc
      Calendar::deleteTodo()
    */
    bool deleteTodo( const KCalCore::Todo::Ptr &todo );

    /**
      @copydoc
      Calendar::deleteTodoInstances()
    */
    bool deleteTodoInstances( const KCalCore::Todo::Ptr &todo );

    /**
      @copydoc
      Calendar::deleteAllTodos()
    */
    void deleteAllTodos();

    /**
      @copydoc
      Calendar::rawTodos(KCalCore::TodoSortField, KCalCore::SortDirection)
    */
    KCalCore::Todo::List rawTodos(
      KCalCore::TodoSortField sortField = KCalCore::TodoSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    /**
      @copydoc
      Calendar::rawTodos(QDate, QDate, KDateTime::Spec, bool)
    */
    KCalCore::Todo::List rawTodos(
      const QDate &start, const QDate &end,
      const KDateTime::Spec &timespec = KDateTime::Spec(),
      bool inclusive = false ) const;

    /**
      @copydoc
      Calendar::rawTodosForDate()
    */
    KCalCore::Todo::List rawTodosForDate( const QDate &date ) const;

    /**
      @copydoc
      Calendar::todo()
    */
    KCalCore::Todo::Ptr todo( const QString &uid,
                              const KDateTime &recurrenceId = KDateTime() ) const;

    /**
      @copydoc
      Calendar::deletedTodo()
    */
    KCalCore::Todo::Ptr deletedTodo( const QString &uid,
                                     const KDateTime &recurrenceId = KDateTime() ) const;

    /**
      @copydoc
      Calendar::deletedTodos(KCalCore::TodoSortField, KCalCore::SortDirection)
    */
    KCalCore::Todo::List deletedTodos(
      KCalCore::TodoSortField sortField = KCalCore::TodoSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    /**
      @copydoc
      Calendar::todoInstances(const KCalCore::Todo::Ptr &,
                              KCalCore::TodoSortField, KCalCore::SortDirection)
    */
    KCalCore::Todo::List todoInstances(
      const KCalCore::Incidence::Ptr &todo,
      KCalCore::TodoSortField sortField = KCalCore::TodoSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

  // Journal Specific Methods //

    /**
      @copydoc
      Calendar::addJournal()
    */
    bool addJournal( const KCalCore::Journal::Ptr &journal );

    /**
      @copydoc
      Calendar::deleteJournal()
    */
    bool deleteJournal( const KCalCore::Journal::Ptr &journal );

    /**
      @copydoc
      Calendar::deleteJournalInstances()
    */
    bool deleteJournalInstances( const KCalCore::Journal::Ptr &journal );

    /**
      @copydoc
      Calendar::deleteAllJournals()
    */
    void deleteAllJournals();

    /**
      @copydoc
      Calendar::rawJournals()
    */
    KCalCore::Journal::List rawJournals(
      KCalCore::JournalSortField sortField = KCalCore::JournalSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    /**
      Returns an unfiltered list of all Journals occurring within a date range.

      @param start is the starting date
      @param end is the ending date
      @param timespec time zone etc. to interpret @p start and @p end,
                      or the calendar's default time spec if none is specified
      @param inclusive if true only Journals which are completely included
      within the date range are returned.

      @return the list of unfiltered Journals occurring within the specified
      date range.
    */
    KCalCore::Journal::List rawJournals(
      const QDate &start, const QDate &end,
      const KDateTime::Spec &timespec = KDateTime::Spec(),
      bool inclusive = false ) const;

    /**
      @copydoc
      Calendar::rawJournalsForDate()
    */
    KCalCore::Journal::List rawJournalsForDate( const QDate &date ) const;

    /**
      @copydoc
      Calendar::journal()
    */
    KCalCore::Journal::Ptr journal( const QString &uid,
                                    const KDateTime &recurrenceId = KDateTime() ) const;

    /**
      @copydoc
      Calendar::deletedJournal()
    */
    KCalCore::Journal::Ptr deletedJournal( const QString &uid,
                                           const KDateTime &recurrenceId = KDateTime() ) const;

    /**
      @copydoc
      Calendar::deletedJournals(KCalCore::JournalSortField, KCalCore::SortDirection)
    */
    KCalCore::Journal::List deletedJournals(
      KCalCore::JournalSortField sortField = KCalCore::JournalSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    /**
      @copydoc
      Calendar::journalInstances(const KCalCore::Journal::Ptr &,
                                 KCalCore::JournalSortField, KCalCore::SortDirection)
    */
    KCalCore::Journal::List journalInstances(
      const KCalCore::Incidence::Ptr &journal,
      KCalCore::JournalSortField sortField = KCalCore::JournalSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

  // Alarm Specific Methods //

    /**
      @copydoc
      Calendar::alarms()
    */
    KCalCore::Alarm::List alarms( const KDateTime &from, const KDateTime &to ) const;

    /**
      Return a list of Alarms that occur before the specified timestamp.

      @param to is the ending timestamp.
      @return the list of Alarms occurring before the specified KDateTime.
    */
    KCalCore::Alarm::List alarmsTo( const KDateTime &to ) const;

    /**
      Notify the IncidenceBase::Observer that the incidence will be updated.

      @param incidenceBase is a pointer to the IncidenceBase to be updated.
    */
    void incidenceUpdate( const QString &uid );

    /**
      Notify the IncidenceBase::Observer that the incidence has been updated.

      @param incidenceBase is a pointer to the updated IncidenceBase.
    */
    void incidenceUpdated( const QString &uid );

    using QObject::event;   // prevent warning about hidden virtual method

  // Incidence Specific Methods, also see Calendar.h for more //

    /**
      List all attendees currently in the memory.
      Attendees are persons that are associated to calendar incidences.

      @return list of attendees
    */
    QStringList attendees();

    /**
      List all attendee related incidences.

      @param email attendee email address
      @return list of incidences for the attendee
    */
    KCalCore::Incidence::List attendeeIncidences( const QString &email );

    /**
      List all incidences with geographic information in the memory.

      @return list of incidences
    */
    KCalCore::Incidence::List geoIncidences();

    /**
      List incidences with geographic information in the memory.

      @param geoLatitude latitude center
      @param geoLongitude longitude center
      @param diffLatitude maximum latitudinal difference
      @param diffLongitude maximum longitudinal difference
      @return list of incidences
    */
    KCalCore::Incidence::List geoIncidences( float geoLatitude, float geoLongitude,
                                             float diffLatitude, float diffLongitude );

    /**
      Delete all incidences from the memory cache. They will be deleted from
      database when save is called.
    */
    void deleteAllIncidences();

    /**
      Sort a list of Incidences.

      @param list is a pointer to a list of Incidences.
      @param sortField specifies the IncidenceSortField (see this header).
      @param sortDirection specifies the KCalCore::SortDirection (see calendar.h).

      @return a list of Incidences sorted as specified.
    */
    static KCalCore::Incidence::List sortIncidences(
      KCalCore::Incidence::List *list,
      IncidenceSortField sortField = IncidenceSortDate,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending );

    /**
       Single expanded incidence tuple.  The first field contains the
       time in local timezone when the (recurrent) incidence starts.
       The second field contains a pointer to the actual Incidence
       instance.
    */
    typedef QPair<QDateTime,KCalCore::Incidence::Ptr> ExpandedIncidence;

    /**
       List of ExpandedIncidences.
    */
    typedef QList<ExpandedIncidence> ExpandedIncidenceList;

    /**
      Expand recurring incidences in a list.

      The returned list contains ExpandedIncidence QPairs which
      contain both the time of the incidence, as well as pointer to
      the incidence itself.

      @param list is a pointer to a list of Incidences.
      @param start start time for expansion+filtering
      @param end end time for expansion+filtering
      @param maxExpand maximum expanded entries per incidence

      @return a list of ExpandedIncidences sorted by start time (the
      first item in the ExpandedIncidence tuple) in ascending order.
    */
    ExpandedIncidenceList expandRecurrences( KCalCore::Incidence::List *list,
                                             const KDateTime &start,
                                             const KDateTime &end,
                                             int maxExpand = 1000 );

    using KCalCore::MemoryCalendar::incidences;

    /**
      Returns a filtered list of all Incidences occurring within a date range.

      @param start is the starting date
      @param end is the ending date

      @return the list of filtered Incidences occurring within the specified
      date range.
    */
    KCalCore::Incidence::List incidences( const QDate &start, const QDate &end );

    /**
      Creates the default Storage Object used in Maemo.
      The Storage is already linked to this calendar object.

      @return Object used as storage in Maemo. It should be transparent
      to the user which type it is.
      @warning The object has to be freed by the caller.
    */
    ExtendedStorage *defaultStorage();

  // Smart Loading Methods, see ExtendedStorage.h for more //

    /**
      Get all uncompleted todos. Todos may or may not have due date and
      they may or may not have geo location.

      @param hasDate true to get todos that have due date
      @param hasGeo value -1 = don't care, 0 = no geo, 1 = geo defined
      @return list of uncompleted todos
    */
    KCalCore::Todo::List uncompletedTodos( bool hasDate, int hasGeo );

    /**
      Get completed todos between given time.

      @param hasDate true to get todos that have due date
      @param hasGeo value -1 = don't care, 0 = no geo, 1 = geo defined
      @param start start datetime
      @param end end datetime
      @return list of completed todos
    */
    KCalCore::Todo::List completedTodos( bool hasDate, int hasGeo,
                                         const KDateTime &start, const KDateTime &end );

    /**
      Get incidences based on start/due dates or creation dates.

      @param hasDate true to get incidences that have due/start date
      @param start start datetime
      @param end end datetime
      @return list of incidences
    */
    KCalCore::Incidence::List incidences( bool hasDate, const KDateTime &start,
                                          const KDateTime &end );

    /**
      Get incidences that have geo location defined.

      @param hasDate true to get incidences that have due/start date
      @param start start datetime
      @param end end datetime
      @return list of geo incidences
    */
    KCalCore::Incidence::List geoIncidences( bool hasDate, const KDateTime &start,
                                             const KDateTime &end );

    /**
      Get all incidences that have unread invitation status.

      @param person if given only for this person
      @return list of unread incidences
      @see IncidenceBase::setInvitationStatus()
    */
    KCalCore::Incidence::List unreadInvitationIncidences(
      const KCalCore::Person::Ptr &person = KCalCore::Person::Ptr() );

    /**
      Get incidences that have read/sent invitation status.

      @param start start datetime
      @param end end datetime
      @return list of old incidences
      @see IncidenceBase::setInvitationStatus()
    */
    KCalCore::Incidence::List oldInvitationIncidences( const KDateTime &start,
                                                       const KDateTime &end );

    /**
      Get incidences related to given contact. Relation is determined
      by the email address of the person, name doesn't matter.

      @param person for this person
      @param start start datetime
      @param end end datetime
      @return list of related incidences
    */
    KCalCore::Incidence::List contactIncidences( const KCalCore::Person::Ptr &person,
                                                 const KDateTime &start, const KDateTime &end );

    /**
      Add incidences into calendar from a list of Incidences.

      @param list is a pointer to a list of Incidences.
      @param notebookUid is uid of notebook of all incidences in list
      @param duplicateRemovalEnabled default value true
      if true and duplicates are found duplicates are deleted and new incidence is added
      if false and duplicates are found new incidence is ignored

      @return a list of Incidences added into calendar memory.
    */
    KCalCore::Incidence::List addIncidences( KCalCore::Incidence::List *list,
                                             const QString &notebookUid,
                                             bool duplicateRemovalEnabled = true );

  protected:

    /**
       Implement the storageModified to clear ExtendedCalendar
       contents on the storage change.
     */
    virtual void storageModified( ExtendedStorage *storage, const QString &info );

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( ExtendedCalendar )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif