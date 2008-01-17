//=========================================================================
//  FILTEREDEVENTLOG.CC - part of
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2006 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <algorithm>
#include "filteredeventlog.h"

USING_NAMESPACE

FilteredEventLog::FilteredEventLog(IEventLog *eventLog)
{
    this->eventLog = eventLog;

    // filter parameters
    tracedEventNumber = -1;
    firstEventNumber = -1;
    lastEventNumber = -1;
    traceCauses = true;
    traceConsequences = true;
    traceMessageReuses = false;
    traceSelfMessages = true;

    maximumNumberOfCauses = maximumNumberOfConsequences = 10;
    maximumCauseDepth = maximumConsequenceDepth = 30;

    enableModuleFilter = false;
    enableMessageFilter = false;
    setModuleExpression("");
    setMessageExpression("");

    clearInternalState();
}

FilteredEventLog::~FilteredEventLog()
{
    deleteAllocatedObjects();
}

void FilteredEventLog::deleteAllocatedObjects()
{
    for (EventNumberToFilteredEventMap::iterator it = eventNumberToFilteredEventMap.begin(); it != eventNumberToFilteredEventMap.end(); it++)
        delete it->second;

    clearInternalState();
}

void FilteredEventLog::clearInternalState(FileReader::FileChangedState change)
{
    approximateNumberOfEvents = -1;
    approximateMatchingEventRatio = -1;
    lastMatchingEvent = NULL;

    if (change == FileReader::OVERWRITTEN) {
        firstMatchingEvent = NULL;

        eventNumberToFilteredEventMap.clear();
        eventNumberToFilterMatchesFlagMap.clear();
        eventNumberToTraceableEventFlagMap.clear();
    }
}

void FilteredEventLog::synchronize()
{
    FileReader::FileChangedState change = getFileReader()->getFileChangedState();

    if (change != FileReader::UNCHANGED) {
        if (change == FileReader::OVERWRITTEN)
            deleteAllocatedObjects();
        else
            clearInternalState(change);

        eventLog->synchronize();

        if (change == FileReader::APPENDED)
            for (EventNumberToFilteredEventMap::iterator it = eventNumberToFilteredEventMap.begin(); it != eventNumberToFilteredEventMap.end(); it++)
                it->second->synchronize();
    }
}

void FilteredEventLog::setPatternMatchers(std::vector<PatternMatcher> &patternMatchers, std::vector<std::string> &patterns, bool dottedPath)
{
    for (std::vector<std::string>::iterator it = patterns.begin(); it != patterns.end(); it++) {
        PatternMatcher matcher;
        matcher.setPattern((*it).c_str(), dottedPath, true, false);
        patternMatchers.push_back(matcher);
    }
}

long FilteredEventLog::getApproximateNumberOfEvents()
{
    if (approximateNumberOfEvents == -1)
    {
        if (tracedEventNumber != -1) {
            // TODO: this is clearly not good and should return a much better approximation
            // TODO: maybe start from traced event number and go forward/backward and return approximation based on that?
            return 1000;
        }
        else {
            // TODO: what if filter is event range limited?
            FilteredEvent *firstEvent = getFirstEvent();
            FilteredEvent *lastEvent = getLastEvent();

            if (firstEvent == NULL)
                approximateNumberOfEvents = 0;
            else
            {
                file_offset_t beginOffset = firstEvent->getBeginOffset();
                file_offset_t endOffset = lastEvent->getEndOffset();
                long sumMatching = 0;
                long sumNotMatching = 0;
                long count = 0;
                int eventCount = 100;

                // TODO: perhaps it would be better to read in random events
                for (int i = 0; i < eventCount; i++)
                {
                    if (firstEvent) {
                        FilteredEvent *previousEvent = firstEvent;
                        sumMatching += firstEvent->getEndOffset() - firstEvent->getBeginOffset();
                        count++;
                        firstEvent = firstEvent->getNextEvent();
                        if (firstEvent)
                            sumNotMatching += firstEvent->getBeginOffset() - previousEvent->getEndOffset();
                    }

                    if (lastEvent) {
                        FilteredEvent *previousEvent = lastEvent;
                        sumMatching += lastEvent->getEndOffset() - lastEvent->getBeginOffset();
                        count++;
                        lastEvent = lastEvent->getPreviousEvent();
                        if (lastEvent)
                            sumNotMatching += previousEvent->getBeginOffset() - lastEvent->getEndOffset();
                    }
                }

                double averageMatching = (double)sumMatching / count;
                double averageNotMatching = (double)sumNotMatching / count;
                approximateMatchingEventRatio = averageMatching / (averageMatching + averageNotMatching);
                approximateNumberOfEvents = (long)((endOffset - beginOffset) / averageMatching * approximateMatchingEventRatio);
            }
        }
    }

    return approximateNumberOfEvents;
}

double FilteredEventLog::getApproximatePercentageForEventNumber(long eventNumber)
{
    if (tracedEventNumber != -1)
        // TODO: this is clearly not good and should return a much better approximation
        return IEventLog::getApproximatePercentageForEventNumber(eventNumber);
    else
        // TODO: what if filter is event range limited
        return IEventLog::getApproximatePercentageForEventNumber(eventNumber);
}

FilteredEvent *FilteredEventLog::getApproximateEventAt(double percentage)
{
    IEvent *event = eventLog->getApproximateEventAt(percentage);

    return this->getMatchingEventInDirection(event, true);
}

FilteredEvent *FilteredEventLog::getNeighbourEvent(IEvent *event, long distance)
{
    return (FilteredEvent *)IEventLog::getNeighbourEvent(event, distance);
}

bool FilteredEventLog::matchesFilter(IEvent *event)
{
    Assert(event);
    EventNumberToBooleanMap::iterator it = eventNumberToFilterMatchesFlagMap.find(event->getEventNumber());

    // if cached, return it
    if (it != eventNumberToFilterMatchesFlagMap.end())
        return it->second;

    //printf("*** Matching filter to event: %ld\n", event->getEventNumber());

    bool matches = matchesEvent(event) && matchesDependency(event);
    eventNumberToFilterMatchesFlagMap[event->getEventNumber()] = matches;
    return matches;
}

bool FilteredEventLog::matchesEvent(IEvent *event)
{
    // the traced event
    if (tracedEventNumber != -1 && event->getEventNumber() == tracedEventNumber)
        return true;

    // event outside of considered range
    if ((firstEventNumber != -1 && event->getEventNumber() < firstEventNumber) ||
        (lastEventNumber != -1 && event->getEventNumber() > lastEventNumber))
        return false;

    // event's module
    if (enableModuleFilter) {
        ModuleCreatedEntry *moduleCreatedEntry = event->getModuleCreatedEntry();

        if (!moduleCreatedEntry || !matchesModuleCreatedEntry(moduleCreatedEntry))
            return false;
    }

    // event's message
    if (enableMessageFilter) {
        BeginSendEntry *beginSendEntry = event->getCauseBeginSendEntry();

        if (!beginSendEntry)
            return false;

        bool matches = matchesBeginSendEntry(beginSendEntry);

        for (int i = 0; i < event->getNumEventLogEntries(); i++) {
            beginSendEntry = dynamic_cast<BeginSendEntry *>(event->getEventLogEntry(i));

            if (beginSendEntry && matchesBeginSendEntry(beginSendEntry)) {
                matches = true;
                break;
            }
        }

        if (!matches)
            return false;
    }

    return true;
}

bool FilteredEventLog::matchesDependency(IEvent *event)
{
    // if there's no traced event
    if (tracedEventNumber == -1)
        return true;

    // if this is the traced event
    if (event->getEventNumber() == tracedEventNumber)
        return true;

    // event is cause of traced event
    if (tracedEventNumber > event->getEventNumber() && traceCauses)
        return isCauseOfTracedEvent(event);

    // event is consequence of traced event
    if (tracedEventNumber < event->getEventNumber() && traceConsequences)
        return isConsequenceOfTracedEvent(event);

    return false;
}

bool FilteredEventLog::matchesModuleCreatedEntry(ModuleCreatedEntry *moduleCreatedEntry)
{
    return
        matchesExpression(moduleExpression, moduleCreatedEntry) ||
        matchesPatterns(moduleNames, moduleCreatedEntry->fullName) ||
        matchesPatterns(moduleClassNames, moduleCreatedEntry->moduleClassName) ||
        matchesList(moduleIds, moduleCreatedEntry->moduleId);
}

bool FilteredEventLog::matchesBeginSendEntry(BeginSendEntry *beginSendEntry)
{
    return
        matchesExpression(messageExpression, beginSendEntry) ||
        matchesPatterns(messageNames, beginSendEntry->messageFullName) ||
        matchesPatterns(messageClassNames, beginSendEntry->messageClassName) ||
        matchesList(messageIds, beginSendEntry->messageId) ||
        matchesList(messageTreeIds, beginSendEntry->messageTreeId) ||
        matchesList(messageEncapsulationIds, beginSendEntry->messageEncapsulationId) ||
        matchesList(messageEncapsulationTreeIds, beginSendEntry->messageEncapsulationTreeId);
}

bool FilteredEventLog::matchesExpression(MatchExpression &matchExpression, EventLogEntry *eventLogEntry)
{
    return matchExpression.matches(eventLogEntry);
}

bool FilteredEventLog::matchesPatterns(std::vector<PatternMatcher> &patterns, const char *str)
{
    if (patterns.empty())
        return false;

    for (std::vector<PatternMatcher>::iterator it = patterns.begin(); it != patterns.end(); it++)
        if ((*it).matches(str))
            return true;

    return false;
}

template <typename T> bool FilteredEventLog::matchesList(std::vector<T> &elements, T element)
{
    if (elements.empty())
        return false;
    else
        return std::find(elements.begin(), elements.end(), element) != elements.end();
}

bool FilteredEventLog::isEmpty()
{
    if (tracedEventNumber != -1)
        return !eventLog->getEventForEventNumber(tracedEventNumber);
    else
        return IEventLog::isEmpty();
}

FilteredEvent *FilteredEventLog::getFirstEvent()
{
    if (!firstMatchingEvent && !eventLog->isEmpty())
    {
        long startEventNumber = firstEventNumber == -1 ? eventLog->getFirstEvent()->getEventNumber() : std::max(eventLog->getFirstEvent()->getEventNumber(), firstEventNumber);
        firstMatchingEvent = getMatchingEventInDirection(startEventNumber, true);
    }

    return firstMatchingEvent;
}

FilteredEvent *FilteredEventLog::getLastEvent()
{
    if (!lastMatchingEvent && !eventLog->isEmpty())
    {
        long startEventNumber = lastEventNumber == -1 ? eventLog->getLastEvent()->getEventNumber() : std::min(eventLog->getLastEvent()->getEventNumber(), lastEventNumber);
        lastMatchingEvent = getMatchingEventInDirection(startEventNumber, false);
    }

    return lastMatchingEvent;
}

FilteredEvent *FilteredEventLog::getEventForEventNumber(long eventNumber, MatchKind matchKind)
{
    Assert(eventNumber >= 0);

    EventNumberToFilteredEventMap::iterator it = eventNumberToFilteredEventMap.find(eventNumber);

    if (it != eventNumberToFilteredEventMap.end())
        return it->second;

    IEvent *event = eventLog->getEventForEventNumber(eventNumber, matchKind);

    if (event) {
        switch (matchKind) {
            case EXACT:
                if (matchesFilter(event))
                    return cacheFilteredEvent(event->getEventNumber());
                break;
            case FIRST_OR_PREVIOUS:
                if (event->getEventNumber() == eventNumber && matchesFilter(event))
                    return cacheFilteredEvent(event->getEventNumber());
                else
                    return getMatchingEventInDirection(event, false);
            case FIRST_OR_NEXT:
                return getMatchingEventInDirection(event, true);
            case LAST_OR_PREVIOUS:
                return getMatchingEventInDirection(event, false);
            case LAST_OR_NEXT:
                if (event->getEventNumber() == eventNumber && matchesFilter(event))
                    return cacheFilteredEvent(event->getEventNumber());
                else
                    return getMatchingEventInDirection(event, true);
        }
    }

    return NULL;
}

FilteredEvent *FilteredEventLog::getEventForSimulationTime(simtime_t simulationTime, MatchKind matchKind)
{
    IEvent *event = eventLog->getEventForSimulationTime(simulationTime, matchKind);

    if (event) {
        switch (matchKind) {
            case EXACT:
                if (matchesFilter(event))
                    return cacheFilteredEvent(event->getEventNumber());
                break;
            case FIRST_OR_PREVIOUS:
                if (event->getSimulationTime() == simulationTime) {
                    IEvent *lastEvent = eventLog->getEventForSimulationTime(simulationTime, LAST_OR_NEXT);
                    FilteredEvent *matchingEvent = getMatchingEventInDirection(event, true, lastEvent->getEventNumber());

                    if (matchingEvent)
                        return matchingEvent;
                }

                return getMatchingEventInDirection(event, false);
            case FIRST_OR_NEXT:
                return getMatchingEventInDirection(event, true);
            case LAST_OR_PREVIOUS:
                return getMatchingEventInDirection(event, false);
            case LAST_OR_NEXT:
                if (event->getSimulationTime() == simulationTime) {
                    IEvent *firstEvent = eventLog->getEventForSimulationTime(simulationTime, FIRST_OR_PREVIOUS);
                    FilteredEvent *matchingEvent = getMatchingEventInDirection(event, false, firstEvent->getEventNumber());

                    if (matchingEvent)
                        return matchingEvent;
                }

                return getMatchingEventInDirection(event, true);
        }
    }

    return NULL;
}

EventLogEntry *FilteredEventLog::findEventLogEntry(EventLogEntry *start, const char *search, bool forward)
{
    EventLogEntry *eventLogEntry = start;

    do {
        eventLogEntry = eventLog->findEventLogEntry(eventLogEntry, search, forward);
    }
    while (!matchesFilter(eventLogEntry->getEvent()));

    return eventLogEntry;
}

FilteredEvent *FilteredEventLog::getMatchingEventInDirection(long eventNumber, bool forward, long stopEventNumber)
{
    Assert(eventNumber >= 0);
    IEvent *event = eventLog->getEventForEventNumber(eventNumber);

    return getMatchingEventInDirection(event, forward, stopEventNumber);
}

FilteredEvent *FilteredEventLog::getMatchingEventInDirection(IEvent *event, bool forward, long stopEventNumber)
{
    Assert(event);

    while (event)
    {
        int eventNumber = event->getEventNumber();

        if (matchesFilter(event))
            return cacheFilteredEvent(eventNumber);

        if (forward)
        {
            eventNumber++;
            event = event->getNextEvent();

            if (lastEventNumber != -1 && eventNumber > lastEventNumber)
                return NULL;

            if (stopEventNumber != -1 && eventNumber > stopEventNumber)
                return NULL;
        }
        else {
            eventNumber--;
            event = event->getPreviousEvent();

            if (firstEventNumber != -1 && eventNumber < firstEventNumber)
                return NULL;

            if (stopEventNumber != -1 && eventNumber < stopEventNumber)
                return NULL;
        }
    }

    return NULL;
}

std::vector<int> FilteredEventLog::getSelectedModuleIds()
{
    std::vector<int> moduleIds;

    for (int i = 0; i < eventLog->getNumModuleCreatedEntries(); i++) {
        ModuleCreatedEntry *moduleCreatedEntry = eventLog->getModuleCreatedEntry(i);

        if (moduleCreatedEntry && matchesModuleCreatedEntry(moduleCreatedEntry))
            moduleIds.push_back(moduleCreatedEntry->moduleId);
    }

    return moduleIds;
}

bool FilteredEventLog::isCauseOfTracedEvent(IEvent *cause)
{
    EventNumberToBooleanMap::iterator it = eventNumberToTraceableEventFlagMap.find(cause->getEventNumber());

    // check cache
    if (it != eventNumberToTraceableEventFlagMap.end())
        return it->second;

    //printf("Checking if %ld is cause of %ld\n", cause->getEventNumber(), tracedEventNumber);

    // returns true if "consequence" can be reached from "cause", using
    // the consequences chain. We use depth-first search.
    bool result = false;
    IMessageDependencyList *consequences = cause->getConsequences();

    for (IMessageDependencyList::iterator it = consequences->begin(); it != consequences->end(); it++)
    {
        IMessageDependency *messageDependency = *it;
        IEvent *consequenceEvent = messageDependency->getConsequenceEvent();

        if (consequenceEvent &&
            (traceSelfMessages || !consequenceEvent->isSelfMessageProcessingEvent()) && 
            (traceMessageReuses || !messageDependency->getIsReuse()))
        {
            // if we reached the consequence event, we're done
            if (tracedEventNumber == consequenceEvent->getEventNumber())
                result = true;

            // try depth-first search if we haven't passed the consequence event yet
            if (tracedEventNumber > consequenceEvent->getEventNumber() &&
                isCauseOfTracedEvent(consequenceEvent))
                result = true;
        }
    }

    eventNumberToTraceableEventFlagMap[cause->getEventNumber()] = result;

    return result;
}

bool FilteredEventLog::isConsequenceOfTracedEvent(IEvent *consequence)
{
    if (!traceSelfMessages && consequence->isSelfMessageProcessingEvent())
        return false;

    EventNumberToBooleanMap::iterator it = eventNumberToTraceableEventFlagMap.find(consequence->getEventNumber());

    // check cache
    if (it != eventNumberToTraceableEventFlagMap.end())
        return it->second;

    //printf("Checking if %ld is consequence of %ld\n", consequence->getEventNumber(), tracedEventNumber);

    // like consequenceEvent(), but searching from the opposite direction
    bool result = false;
    IMessageDependencyList *causes = consequence->getCauses();

    for (IMessageDependencyList::iterator it = causes->begin(); it != causes->end(); it++)
    {
        IMessageDependency *messageDependency = *it;
        IEvent *causeEvent = messageDependency->getCauseEvent();

        if (causeEvent && (traceMessageReuses || !messageDependency->getIsReuse()))
        {
            // if we reached the cause event, we're done
            if (tracedEventNumber == causeEvent->getEventNumber())
                result = true;

            // try depth-first search if we haven't passed the cause event yet
            if (tracedEventNumber < causeEvent->getEventNumber() &&
                isConsequenceOfTracedEvent(causeEvent))
                result = true;
        }
    }

    eventNumberToTraceableEventFlagMap[consequence->getEventNumber()] = result;

    return result;
}

FilteredEvent *FilteredEventLog::cacheFilteredEvent(long eventNumber)
{
    EventNumberToFilteredEventMap::iterator it = eventNumberToFilteredEventMap.find(eventNumber);

    if (it != eventNumberToFilteredEventMap.end())
        return it->second;
    else {
        FilteredEvent *filteredEvent = new FilteredEvent(this, eventNumber);
        eventNumberToFilteredEventMap[eventNumber] = filteredEvent;
        return filteredEvent;
    }
}
