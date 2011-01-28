/*
 * Copyright (c) 2011
 *      Jon Schewe.  All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * I'd appreciate comments/suggestions on the code jpschewe@mtu.net
 */
package net.mtu.eggplant.google;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JTextField;

import net.mtu.eggplant.google.Palm.CalendarEvent;
import net.mtu.eggplant.google.Palm.CalendarRepeatType;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.GnuParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;

import com.google.gdata.client.calendar.CalendarService;
import com.google.gdata.data.DateTime;
import com.google.gdata.data.PlainTextConstruct;
import com.google.gdata.data.calendar.CalendarEntry;
import com.google.gdata.data.calendar.CalendarEventEntry;
import com.google.gdata.data.calendar.CalendarFeed;
import com.google.gdata.data.extensions.BaseEventEntry.EventStatus;
import com.google.gdata.data.extensions.OriginalEvent;
import com.google.gdata.data.extensions.Recurrence;
import com.google.gdata.data.extensions.Reminder;
import com.google.gdata.data.extensions.Reminder.Method;
import com.google.gdata.data.extensions.When;
import com.google.gdata.util.AuthenticationException;
import com.google.gdata.util.ServiceException;

/**
 * @author jpschewe
 * 
 */
public class GoogleImport {

	private static final Logger LOGGER = Logger.getLogger(GoogleImport.class
			.getName());

	public static void main(final String[] args) {
		final Options options = buildOptions();
		final CommandLineParser parser = new GnuParser();
		try {
			// parse the command line arguments
			final CommandLine line = parser.parse(options, args);

			// String userName = EMAIL_ADDRESS;
			// String userPassword = getPassword();

			// // Create the necessary URL objects.
			// try {
			// metafeedUrl = new URL(METAFEED_URL_BASE + userName);
			// eventFeedUrl = new URL(METAFEED_URL_BASE + userName
			// + EVENT_FEED_URL_SUFFIX);
			// } catch (MalformedURLException e) {
			// // Bad URL
			// System.err.println("Uh oh - you've got an invalid URL.");
			// e.printStackTrace();
			// return;
			// }

			final File protofile = new File(
					line.getOptionValue(CALENDAR_FILE_OPTION));
			if (!protofile.canRead()) {
				LOGGER.log(Level.SEVERE,
						"File is not readable: " + protofile.getAbsolutePath());
				System.exit(1);
			}

			final Map<String, String> calendarNameCategory = new HashMap<String, String>();
			for (final String value : line
					.getOptionValues(CATEGORY_MAPPING_OPTION)) {
				final int sepIndex = value.indexOf('=');
				if (sepIndex < 1) {
					LOGGER.log(Level.SEVERE,
							"Category mappings must be of the form category=calendar: "
									+ value);
					System.exit(1);
				} else {
					final String category = value.substring(0, sepIndex);
					final String calendarName = value.substring(sepIndex + 1);
					calendarNameCategory.put(calendarName, category);
				}
			}
			if (LOGGER.isLoggable(Level.FINER)) {
				LOGGER.fine("Category mappings: " + calendarNameCategory);
			}

			Calendar startRange = null;
			Calendar endRange = null;
			try {
				final DateFormat rangeFormat = new SimpleDateFormat(
						"MM-dd-yyyy");
				if (line.hasOption(START_OPTION)) {
					startRange = Calendar.getInstance();
					startRange.setTime(rangeFormat.parse(line
							.getOptionValue(START_OPTION)));
				}
				if (line.hasOption(END_OPTION)) {
					endRange = Calendar.getInstance();
					endRange.setTime(rangeFormat.parse(line
							.getOptionValue(END_OPTION)));
				}
			} catch (final ParseException e) {
				LOGGER.log(Level.SEVERE, "Unable to parse date range options",
						e);
				System.exit(1);
			}

			final GoogleImport importer = new GoogleImport(startRange, endRange);
			importer.load(protofile);
			importer.connectToGoogle(calendarNameCategory);
			importer.importEvents();

			System.exit(0);
		} catch (final org.apache.commons.cli.ParseException exp) {
			// oops, something went wrong
			LOGGER.log(Level.SEVERE, "Parsing of commandline failed.", exp);
			usage(options);
		} catch (final IOException e) {
			LOGGER.log(Level.SEVERE, "Error talking to google", e);
			System.exit(1);
		} catch (final ServiceException e) {
			LOGGER.log(Level.SEVERE, "Error talking to google", e);
			System.exit(1);
		}
	}

	/**
	 * Import all palm events in the range to the appropriate calendars.
	 * 
	 * @throws ServiceException
	 * @throws IOException
	 */
	public void importEvents() throws IOException, ServiceException {
		for (final Map.Entry<String, CalendarEntry> entry : categoryCalendar
				.entrySet()) {
			importEvents(entry.getKey(), entry.getValue());
		}
	}

	private URL getCalendarURL(final CalendarEntry calendar) {
		try {
			final String id = calendar.getId();
			// final String url = id.replaceAll("%40", "@");
			final String[] pieces = id.split("/");
			final String last = pieces[pieces.length - 1];
			final String lastConverted = last.replaceAll("%40", "@");
			// may need to convert '%40' to '@'
			return new URL(METAFEED_URL_BASE + lastConverted
					+ EVENT_FEED_URL_SUFFIX);
		} catch (final MalformedURLException e) {
			LOGGER.log(Level.SEVERE, "Unable to convert calendar ID '"
					+ calendar.getId() + "' to URL");
			throw new RuntimeException(e);
		}
	}

	private void importEvents(final String category,
			final CalendarEntry calendar) throws IOException, ServiceException {
		LOGGER.log(Level.FINE, "Importing events from " + category
				+ " into calendar id: " + calendar.getId() + " self link: "
				+ calendar.getSelfLink().getHref());
		final URL calendarURL = getCalendarURL(calendar);
		for (final Palm.CalendarEvent palmEvent : categoryToEvents
				.get(category)) {
			LOGGER.log(Level.FINER, "Importing event: " + palmEvent);

			CalendarEventEntry googleEvent = convertToGoogle(palmEvent);
			int count = 0;
			boolean success = false;
			while (!success && count < 5) {
				++count;
				try {
					googleEvent = service.insert(calendarURL, googleEvent);
					success = true;
				} catch (final ServiceException e) {
					success = false;
					LOGGER.warning("Error importing event, trying again: "
							+ e.getMessage());
				}
			}
			if (!success) {
				LOGGER.severe("Error importing event. Retries failed");
				throw new RuntimeException("Import failed");
			}

			addExceptions(calendarURL, palmEvent, googleEvent);

			// LOGGER.log(Level.INFO, "Exiting after 1 event for debugging");
			// return;
		}
	}

	private DateTime convertToGoogle(final Calendar cal) {
		final DateTime google = new DateTime(cal.getTime(), cal.getTimeZone());
		return google;
	}

	private CalendarEventEntry convertToGoogle(
			final Palm.CalendarEvent palmEvent) {
		final CalendarEventEntry googleEvent = new CalendarEventEntry();
		googleEvent
				.setTitle(new PlainTextConstruct(palmEvent.getDescription()));

		if (palmEvent.getRepeatType() == CalendarRepeatType.NONE) {
			final Calendar start = convertToJava(palmEvent.getBegin(),
					palmEvent.getTz());
			final Calendar end = convertToJava(palmEvent.getEnd(),
					palmEvent.getTz());
			if (end.before(start)) {
				end.add(Calendar.DATE, 1);
			}

			final DateTime startTime = convertToGoogle(start);
			final DateTime endTime = convertToGoogle(end);

			if (palmEvent.getEvent()) {
				startTime.setDateOnly(true);
				endTime.setDateOnly(true);
			}
			final When eventTimes = new When();
			eventTimes.setStartTime(startTime);
			eventTimes.setEndTime(endTime);
			googleEvent.addTime(eventTimes);
		} else {
			final Recurrence recur = createRecurrenceData(palmEvent);
			googleEvent.setRecurrence(recur);
		}

		if (palmEvent.getAlarm()) {
			final Reminder alarm = new Reminder();
			alarm.setMethod(Method.ALERT);
			switch (palmEvent.getAdvanceUnits()) {
			case MINUTES:
				alarm.setMinutes(palmEvent.getAdvance());
				break;
			case HOURS:
				alarm.setHours(palmEvent.getAdvance());
				break;
			case DAYS:
				alarm.setDays(palmEvent.getAdvance());
				break;
			default:
				throw new RuntimeException("Unknown advance units: "
						+ palmEvent.getAdvanceUnits());
			}
			googleEvent.getReminder().add(alarm);
		} else {
			final Reminder reminder = new Reminder();
			reminder.setMethod(Method.NONE);
			reminder.setMinutes(0);
			googleEvent.getReminder().add(reminder);
		}

		if (palmEvent.hasNote()) {
			googleEvent.setContent(new PlainTextConstruct(palmEvent.getNote()));
		}

		if (palmEvent.hasLocation()) {
			googleEvent.addLocation(new com.google.gdata.data.extensions.Where(
					"", "", palmEvent.getLocation()));
		}

		return googleEvent;
	}

	private void addExceptions(final URL calendarURL,
			final CalendarEvent palmEvent, final CalendarEventEntry googleEvent)
			throws IOException, ServiceException {

		final Calendar begin = convertToJava(palmEvent.getBegin(),
				palmEvent.getTz());
		final Calendar end = convertToJava(palmEvent.getEnd(),
				palmEvent.getTz());
		if (end.before(begin)) {
			end.add(Calendar.DATE, 1);
		}

		for (final Palm.Timestamp palmException : palmEvent.getExceptionList()) {
			final Calendar exceptionBegin = convertToJava(palmException,
					palmEvent.getTz());
			final Calendar exceptionEnd = (Calendar) exceptionBegin.clone();

			if (!palmEvent.getEvent()) {
				// use the times from the original event
				exceptionBegin.set(Calendar.HOUR_OF_DAY,
						begin.get(Calendar.HOUR_OF_DAY));
				exceptionBegin.set(Calendar.MINUTE, begin.get(Calendar.MINUTE));
				exceptionBegin.set(Calendar.SECOND, begin.get(Calendar.SECOND));

				exceptionEnd.set(Calendar.HOUR_OF_DAY,
						end.get(Calendar.HOUR_OF_DAY));
				exceptionEnd.set(Calendar.MINUTE, end.get(Calendar.MINUTE));
				exceptionEnd.set(Calendar.SECOND, end.get(Calendar.SECOND));
			}

			// TODO get this right eventually
			final boolean hackExceptions = true;

			final OriginalEvent original = new OriginalEvent();
			final String urlId = googleEvent.getId();
			original.setOriginalId(urlId.substring(urlId.lastIndexOf('/') + 1));
			final When originalWhen = new When();

			if (LOGGER.isLoggable(Level.FINE)) {
				LOGGER.fine("Adding exception at: "
						+ RECURRENCE_DATETIME_FORMAT.get().format(
								exceptionBegin.getTime())//
						+ " urlId: " + urlId //
						+ " subid: " + original.getOriginalId() //
						+ " icalUID: " + googleEvent.getIcalUID());
			}

			originalWhen.setStartTime(convertToGoogle(exceptionBegin));
			if (!palmEvent.getEvent()) {
				originalWhen.setEndTime(convertToGoogle(exceptionEnd));
			}
			original.setOriginalStartTime(originalWhen);

			CalendarEventEntry exception = new CalendarEventEntry();
			if (!hackExceptions) {
				exception.setOriginalEvent(original);
				exception.setStatus(EventStatus.CANCELED);
			} else {
				exception.setTitle(new PlainTextConstruct("EXCEPTION: "
						+ palmEvent.getDescription()));
				exception.addTime(originalWhen);

			}
			service.insert(calendarURL, exception);
		}

	}

	private static final ThreadLocal<DateFormat> RECURRENCE_DATE_FORMAT = new ThreadLocal<DateFormat>() {
		protected DateFormat initialValue() {
			return new SimpleDateFormat("yyyyMMdd");
		}
	};
	private static final ThreadLocal<DateFormat> RECURRENCE_DATETIME_FORMAT = new ThreadLocal<DateFormat>() {
		protected DateFormat initialValue() {
			return new SimpleDateFormat("yyyyMMdd'T'HHmmss");
		}
	};

	private Recurrence createRecurrenceData(final CalendarEvent palmEvent) {
		final Recurrence recur = new Recurrence();
		final Calendar start = convertToJava(palmEvent.getBegin(),
				palmEvent.getTz());
		final Calendar end = convertToJava(palmEvent.getEnd(),
				palmEvent.getTz());
		final String tz = convertToGoogleTimezone(palmEvent.getTz());

		final String dtstart;
		final String dtend;
		if (palmEvent.getEvent()) {
			dtstart = String.format("DTSTART;TZID=%s:%s\r\n", tz,
					RECURRENCE_DATE_FORMAT.get().format(start.getTime()));
			dtend = String.format("DTEND;TZID=%s:%s\r\n", tz,
					RECURRENCE_DATE_FORMAT.get().format(end.getTime()));

		} else {
			dtstart = String.format("DTSTART;TZID=%s:%s\r\n", tz,
					RECURRENCE_DATETIME_FORMAT.get().format(start.getTime()));
			dtend = String.format("DTEND;TZID=%s:%s\r\n", tz,
					RECURRENCE_DATETIME_FORMAT.get().format(end.getTime()));

		}

		final String recurData = dtstart + dtend + getFrequency(palmEvent);
		LOGGER.fine("Recur: " + recurData);
		recur.setValue(recurData);

		return recur;
	}

	public static String join(final Collection<String> s, final String delimiter) {
		final StringBuffer buffer = new StringBuffer();
		final Iterator<String> iter = s.iterator();
		while (iter.hasNext()) {
			buffer.append(iter.next());
			if (iter.hasNext()) {
				buffer.append(delimiter);
			}
		}
		return buffer.toString();
	}

	private String getFrequency(final CalendarEvent palmEvent) {
		String rule = "";
		switch (palmEvent.getRepeatType()) {
		case NONE:
			return "";
		case DAILY:
			rule = "RRULE:FREQ=DAILY";
			break;
		case WEEKLY:
			final List<String> repeatList = new LinkedList<String>();

			if (palmEvent.getRepeatDays(0)) {
				repeatList.add("SU");
			}
			if (palmEvent.getRepeatDays(1)) {
				repeatList.add("MO");
			}
			if (palmEvent.getRepeatDays(2)) {
				repeatList.add("TU");
			}
			if (palmEvent.getRepeatDays(3)) {
				repeatList.add("WE");
			}
			if (palmEvent.getRepeatDays(4)) {
				repeatList.add("TH");
			}
			if (palmEvent.getRepeatDays(5)) {
				repeatList.add("FR");
			}
			if (palmEvent.getRepeatDays(6)) {
				repeatList.add("SA");
			}
			rule = "RRULE:FREQ=WEEKLY;BYDAY=" + join(repeatList, ",");
			break;
		case MONTHLY_BY_DAY:
			String dayStr = "";
			switch (palmEvent.getRepeatDay()) {

			case CALENDAR_1ST_SUN:
				dayStr = "1SU";
				break;
			case CALENDAR_1ST_MON:
				dayStr = "1MO";
				break;
			case CALENDAR_1ST_TUE:
				dayStr = "1TU";
				break;
			case CALENDAR_1ST_WEN:
				dayStr = "1WE";
				break;
			case CALENDAR_1ST_THU:
				dayStr = "1TH";
				break;
			case CALENDAR_1ST_FRI:
				dayStr = "1FR";
				break;
			case CALENDAR_1ST_SAT:
				dayStr = "1SA";
				break;
			case CALENDAR_2ND_SUN:
				dayStr = "2SU";
				break;
			case CALENDAR_2ND_MON:
				dayStr = "2MO";
				break;
			case CALENDAR_2ND_TUE:
				dayStr = "2TU";
				break;
			case CALENDAR_2ND_WEN:
				dayStr = "2WE";
				break;
			case CALENDAR_2ND_THU:
				dayStr = "2TH";
				break;
			case CALENDAR_2ND_FRI:
				dayStr = "2FR";
				break;
			case CALENDAR_2ND_SAT:
				dayStr = "2SA";
				break;
			case CALENDAR_3RD_SUN:
				dayStr = "3SA";
				break;
			case CALENDAR_3RD_MON:
				dayStr = "3MO";
				break;
			case CALENDAR_3RD_TUE:
				dayStr = "3TU";
				break;
			case CALENDAR_3RD_WEN:
				dayStr = "3WE";
				break;
			case CALENDAR_3RD_THU:
				dayStr = "3TH";
				break;
			case CALENDAR_3RD_FRI:
				dayStr = "3FR";
				break;
			case CALENDAR_3RD_SAT:
				dayStr = "3SA";
				break;
			case CALENDAR_4TH_SUN:
				dayStr = "4SU";
				break;
			case CALENDAR_4TH_MON:
				dayStr = "4MO";
				break;
			case CALENDAR_4TH_TUE:
				dayStr = "4TU";
				break;
			case CALENDAR_4TH_WEN:
				dayStr = "4WE";
				break;
			case CALENDAR_4TH_THU:
				dayStr = "4TH";
				break;
			case CALENDAR_4TH_FRI:
				dayStr = "4FR";
				break;
			case CALENDAR_4TH_SAT:
				dayStr = "4SA";
				break;
			case CALENDAR_LAST_SUN:
				dayStr = "-1SU";
				break;
			case CALENDAR_LAST_MON:
				dayStr = "-1MO";
				break;
			case CALENDAR_LAST_TUE:
				dayStr = "-1TU";
				break;
			case CALENDAR_LAST_WEN:
				dayStr = "-1WE";
				break;
			case CALENDAR_LAST_THU:
				dayStr = "-1TH";
				break;
			case CALENDAR_LAST_FRI:
				dayStr = "-1FR";
				break;
			case CALENDAR_LAST_SAT:
				dayStr = "-1SA";
				break;
			default:
				throw new RuntimeException("Invalid day of month type found: "
						+ palmEvent.getRepeatDay());
			}

			rule = "RRULE:FREQ=MONTHLY;BYDAY=" + dayStr;
			break;
		case MONTHLY_BY_DATE:
			rule = "RRULE:FREQ=MONTHLY";
			break;
		case YEARLY:
			rule = "RRULE:FREQ=YEARLY";
			break;
		default:
			throw new RuntimeException("Invalid repeat type: "
					+ palmEvent.getRepeatType());
		}

		// add the interval
		if (palmEvent.getRepeatFrequency() != 1) {
			rule = rule + ";INTERVAL=" + palmEvent.getRepeatFrequency();
		}

		if (!palmEvent.getRepeatForever()) {
			final Calendar repeatEnd = convertToJava(palmEvent.getRepeatEnd(),
					palmEvent.getTz());
			rule = rule + ";UNTIL="
					+ RECURRENCE_DATE_FORMAT.get().format(repeatEnd.getTime());
		}

		rule = rule + "\r\n";

		return rule;

	}

	/**
	 * 
	 * @param startRange
	 *            start of interval to import (inclusive). If null then start at
	 *            the beginning of the palm calendar
	 * @param endRange
	 *            end of interval to import (exclusive). If null then import
	 *            until the end of the palm calendar.
	 */
	public GoogleImport(final Calendar startRange, final Calendar endRange) {
		this.startRange = startRange;
		this.endRange = endRange;
		LOGGER.log(Level.CONFIG, "Start range: "
				+ (null == startRange ? "NULL" : startRange.getTime()));
		LOGGER.log(Level.CONFIG, "End range: "
				+ (null == endRange ? "NULL" : endRange.getTime()));

		service = new CalendarService("jpschewe-palm-import-1");
	}

	/**
	 * 
	 * @param calendarNameCategory
	 *            mapping of calendar names to palm categories
	 */
	public void connectToGoogle(final Map<String, String> calendarNameCategory) {
		getCredentials();
		try {
			service.setUserCredentials(username, password);
		} catch (final AuthenticationException e) {
			LOGGER.log(Level.SEVERE, "Error authenticating", e);
			System.exit(1);
		}

		try {
			populateCategoryMappings(calendarNameCategory);

		} catch (final IOException e) {
			LOGGER.log(Level.SEVERE, "Error talking to Google", e);
		} catch (final ServiceException e) {
			LOGGER.log(Level.SEVERE, "Error talking to Google", e);
		}
	}

	/**
	 * Combine the palm timestamp and timezone into a {@link Calendar} object.
	 * 
	 * @param palmTS
	 * @param palmTZ
	 * @return
	 */
	private static Calendar convertToJava(final Palm.Timestamp palmTS,
			final Palm.Timezone palmTZ) {
		final String timezoneName = convertToGoogleTimezone(palmTZ);
		if (null == timezoneName) {
			throw new RuntimeException("Unknown timezone: " + palmTZ);
		}
		final TimeZone tz = TimeZone.getTimeZone(timezoneName);
		if (null != palmTZ
				&& tz.getRawOffset() != (palmTZ.getOffset() * 60 * 1000L)) {
			throw new RuntimeException("Timezone name: '" + timezoneName
					+ "' is not recognized");
		}
		final Calendar cal = Calendar.getInstance(tz);
		cal.set(palmTS.getTmYear() + 1900, palmTS.getTmMon(),
				palmTS.getTmMday(), palmTS.getTmHour(), palmTS.getTmMin(),
				palmTS.getTmSec());

		return cal;
	}

	private boolean isEventInDateRange(final Palm.CalendarEvent event) {
		// TODO could cache the Calendar objects to make things more efficient
		final Calendar begin = convertToJava(event.getBegin(), event.getTz());
		if (null == startRange && null == endRange) {
			return true;
		} else if (null == startRange) {
			return begin.before(endRange);
		} else if (null == endRange) {
			return !begin.before(startRange);
		} else {
			return !begin.before(startRange) && begin.before(endRange);
		}
	}

	/**
	 * Map palm timezones to google timezones.
	 * 
	 * @param palmTZ
	 *            the palm timezone
	 * @return the google timezone, null if cannot be determined
	 */
	private static String convertToGoogleTimezone(final Palm.Timezone palmTZ) {
		if (null == palmTZ) {
			return TimeZone.getDefault().getID();
		} else if (palmTZ.getOffset() == -360 && palmTZ.getDstObserved()) {
			return "America/Chicago";
		} else if (palmTZ.getOffset() == -300 && palmTZ.getDstObserved()) {
			return "America/New_York";
		} else if (palmTZ.getOffset() == -420 && palmTZ.getDstObserved()) {
			return "America/Denver";
		} else if (palmTZ.getOffset() == -420 && !palmTZ.getDstObserved()) {
			return "America/Phoenix";
		} else if (palmTZ.getOffset() == -480 && palmTZ.getDstObserved()) {
			return "America/Los_Angeles";
		} else if (palmTZ.getOffset() == 0 && !palmTZ.getDstObserved()) {
			return "UTC";
		} else {
			return null;
		}
	}

	private void populateCategoryMappings(
			final Map<String, String> calendarNameCategory) throws IOException,
			ServiceException {
		try {
			final URL metafeedUrl = new URL(METAFEED_URL_BASE + username);
			CalendarFeed resultFeed = service.getFeed(metafeedUrl,
					CalendarFeed.class);

			for (final CalendarEntry entry : resultFeed.getEntries()) {
				final String calendarName = entry.getTitle().getPlainText();
				if (calendarNameCategory.containsKey(calendarName)) {
					categoryCalendar.put(
							calendarNameCategory.get(calendarName), entry);
				}
			}
		} catch (final MalformedURLException e) {
			LOGGER.log(Level.SEVERE,
					"Calendar feed URL invalid - this is an internal error", e);
			System.exit(20);
		}

		if (calendarNameCategory.size() != categoryCalendar.size()) {
			LOGGER.log(Level.WARNING,
					"Some categories didn't map. Categories that matched: "
							+ categoryCalendar.keySet());
		}

		LOGGER.log(Level.FINE, "Category mappings: " + categoryCalendar);
	}

	private final Map<String, List<Palm.CalendarEvent>> categoryToEvents = new HashMap<String, List<Palm.CalendarEvent>>();

	/**
	 * Load protofile into categoryToEvents. Each list in the map is sorted by
	 * start time of event.
	 * 
	 * @param protofile
	 *            the file to load
	 */
	public void load(final File protofile) {
		try {
			final InputStream input = new FileInputStream(protofile);
			final Palm.Calendar calendar = Palm.Calendar.parseFrom(input);
			final String[] names = calendar.getCategory().getNameList()
					.toArray(new String[0]);
			// group events by category
			for (final Palm.CalendarEvent event : calendar.getEventsList()) {
				final String category = names[event.getPalmRecord()
						.getCategory()];
				if (!categoryToEvents.containsKey(category)) {
					categoryToEvents.put(category,
							new LinkedList<Palm.CalendarEvent>());
				}
				if (isEventInDateRange(event)) {
					categoryToEvents.get(category).add(event);
				}
			}

		} catch (final IOException e) {
			LOGGER.log(Level.SEVERE, "Error loading protobuf file", e);
			System.exit(2);
		}

		// sort by start time
		for (Map.Entry<String, List<Palm.CalendarEvent>> entry : categoryToEvents
				.entrySet()) {
			Collections.sort(entry.getValue(), EventBeginComparator.INSTANCE);
		}

		// debug
		for (Map.Entry<String, List<Palm.CalendarEvent>> entry : categoryToEvents
				.entrySet()) {
			final String category = entry.getKey();
			final List<Palm.CalendarEvent> events = entry.getValue();
			if (!events.isEmpty()) {
				LOGGER.log(Level.FINER, "First event in '" + category
						+ "' in range is " + events.get(0));
			}
		}

	}

	private void getCredentials() {
		final JPanel cpane = new JPanel(new GridBagLayout());

		final GridBagConstraints labelgbc = new GridBagConstraints();
		labelgbc.fill = GridBagConstraints.NONE;
		labelgbc.weightx = 0;
		labelgbc.weighty = 0;

		final GridBagConstraints entrygbc = new GridBagConstraints();
		entrygbc.fill = GridBagConstraints.HORIZONTAL;
		entrygbc.weightx = 1;
		entrygbc.weighty = 0;
		entrygbc.gridwidth = GridBagConstraints.REMAINDER;

		final JLabel ulabel = new JLabel("Google Username: ");
		cpane.add(ulabel, labelgbc);
		final JTextField user = new JTextField(40);
		cpane.add(user, entrygbc);

		final JLabel plabel = new JLabel("Google Password: ");
		cpane.add(plabel, labelgbc);

		final JPasswordField passField = new JPasswordField(10);
		cpane.add(passField, entrygbc);

		final int result = JOptionPane.showConfirmDialog(null, cpane,
				"Enter Google Credentials", JOptionPane.OK_CANCEL_OPTION);
		if (result == JOptionPane.OK_OPTION) {
			username = user.getText();
			password = String.valueOf(passField.getPassword());
		} else {
			LOGGER.log(Level.INFO, "No credentials, exiting");
			System.exit(10);
		}

	}

	private static void usage(final Options options) {
		HelpFormatter formatter = new HelpFormatter();
		formatter.printHelp("GoogleImport", options);
		System.exit(1);
	}

	private static final String CALENDAR_FILE_OPTION = "calendar";
	private static final String CATEGORY_MAPPING_OPTION = "category";
	private static final String START_OPTION = "start";
	private static final String END_OPTION = "end";

	private static Options buildOptions() {
		final Options options = new Options();

		final Option calendarFile = new Option(CALENDAR_FILE_OPTION, true,
				"protobuf file to read");
		calendarFile.setRequired(true);
		calendarFile.setArgName("file");
		options.addOption(calendarFile);

		final Option categoryMapping = new Option(CATEGORY_MAPPING_OPTION,
				true, "Map a Palm category to a Google calendar");
		categoryMapping.setRequired(true);
		categoryMapping.setArgName("category=calendar");
		categoryMapping.setArgs(Option.UNLIMITED_VALUES);
		options.addOption(categoryMapping);

		final Option start = new Option(START_OPTION, true,
				"Start of range to import (inclusive)");
		start.setArgName("MM-DD-YYYY");
		options.addOption(start);

		final Option end = new Option(END_OPTION, true,
				"End of range to import (exclusive)");
		end.setArgName("MM-DD-YYYY");
		options.addOption(end);

		return options;
	}

	private final Calendar startRange;
	private final Calendar endRange;
	private String username;
	private String password;
	private final CalendarService service;
	private final Map<String, CalendarEntry> categoryCalendar = new HashMap<String, CalendarEntry>();

	/** The base URL for a user's calendar metafeed (needs a username appended). */
	private static final String METAFEED_URL_BASE = "http://www.google.com/calendar/feeds/";
	private static final String EVENT_FEED_URL_SUFFIX = "/private/full";

}
