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

#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>

#include <time.h>

#include <pi-file.h>
#include <pi-macros.h>
#include <pi-calendar.h>

#include "palm.pb.h"

namespace nmeg = net::mtu::eggplant::google;
using namespace std;

void write_appinfo(pi_file_t *pf, nmeg::Calendar &proto_calendar) {
  void *app_info;
  size_t app_info_size;

  pi_file_get_app_info(pf, &app_info, &app_info_size);
  if (app_info == NULL) {
    throw "Unable to get app info";
  }

  pi_buffer_t *pi_buf = pi_buffer_new(0);
  pi_buf->data = static_cast<unsigned char *> (app_info);
  pi_buf->used = app_info_size;
  pi_buf->allocated = app_info_size;

  CalendarAppInfo_t cab;
  const int result = unpack_CalendarAppInfo(&cab, pi_buf);
  if (-1 == result) {
    cerr << "Error unpacking calendar app info" << endl;
    return;
  }

  pi_buf->data = NULL;
  pi_buf->used = 0;
  pi_buf->allocated = 0;

  pi_buffer_free(pi_buf);

  for (int i = 0; i < 16; ++i) {
    proto_calendar.mutable_category()->add_name(cab.category.name[i]);
    proto_calendar.mutable_category()->add_renamed(cab.category.renamed[i]);
  }
  proto_calendar.set_start_of_week(cab.startOfWeek);
  proto_calendar.set_internal(cab.internal, 18);

}

void convert_tm_to_protobuf(const struct tm *unix_time,
    nmeg::Timestamp *protobuf_time) {
  protobuf_time->set_tm_sec(unix_time->tm_sec);
  protobuf_time->set_tm_min(unix_time->tm_min);
  protobuf_time->set_tm_hour(unix_time->tm_hour);
  protobuf_time->set_tm_mday(unix_time->tm_mday);
  protobuf_time->set_tm_mon(unix_time->tm_mon);
  protobuf_time->set_tm_year(unix_time->tm_year);
  protobuf_time->set_tm_wday(unix_time->tm_wday);
  protobuf_time->set_tm_yday(unix_time->tm_yday);
  protobuf_time->set_tm_isdst(unix_time->tm_isdst != 0);
}

void fill_blob(const Blob_t *palm_blob, nmeg::Blob *protobuf_blob) {
  uint32_t type;
  memcpy(&type, palm_blob->type, 4);
  protobuf_blob->set_type(type);
  protobuf_blob->set_data(palm_blob->data, palm_blob->length);
}

void fill_dst(const DST_t *palm, nmeg::DST *proto) {
  switch (palm->dayOfWeek) {
  case sunday:
    proto->set_day_of_week(nmeg::SUNDAY);
    break;
  case monday:
    proto->set_day_of_week(nmeg::MONDAY);
    break;
  case tuesday:
    proto->set_day_of_week(nmeg::TUESDAY);
    break;
  case wednesday:
    proto->set_day_of_week(nmeg::WEDNESDAY);
    break;
  case thursday:
    proto->set_day_of_week(nmeg::THURSDAY);
    break;
  case friday:
    proto->set_day_of_week(nmeg::FRIDAY);
    break;
  case saturday:
    proto->set_day_of_week(nmeg::SATURDAY);
    break;
  default:
    cerr << "Unknown day of week: " << palm->dayOfWeek << endl;
    return;
  }

  switch (palm->weekOfMonth) {
  case first:
    proto->set_week_of_month(nmeg::FIRST);
    break;
  case second:
    proto->set_week_of_month(nmeg::SECOND);
    break;
  case third:
    proto->set_week_of_month(nmeg::THIRD);
    break;
  case fourth:
    proto->set_week_of_month(nmeg::FOURTH);
    break;
  case last:
    proto->set_week_of_month(nmeg::LAST);
    break;
  default:
    cerr << "Unknown week of month: " << palm->weekOfMonth << endl;
    return;
  }

  switch (palm->month) {
  case january:
    proto->set_month(nmeg::JANUARY);
    break;
  case february:
    proto->set_month(nmeg::FEBRUARY);
    break;
  case march:
    proto->set_month(nmeg::MARCH);
    break;
  case april:
    proto->set_month(nmeg::APRIL);
    break;
  case may:
    proto->set_month(nmeg::MAY);
    break;
  case june:
    proto->set_month(nmeg::JUNE);
    break;
  case july:
    proto->set_month(nmeg::JULY);
    break;
  case august:
    proto->set_month(nmeg::AUGUST);
    break;
  case september:
    proto->set_month(nmeg::SEPTEMBER);
    break;
  case october:
    proto->set_month(nmeg::OCTOBER);
    break;
  case november:
    proto->set_month(nmeg::NOVEMBER);
    break;
  case december:
    proto->set_month(nmeg::DECEMBER);
    break;
  default:
    cerr << "Unknown month: " << palm->month << endl;
    return;
  }

  proto->set_unknown(palm->unknown);
}

void convert_tz(const Timezone_t *palm_tz, nmeg::Timezone *protobuf_tz) {
  protobuf_tz->set_offset(palm_tz->offset);
  protobuf_tz->set_t2(palm_tz->t2);
  fill_dst(&palm_tz->dstStart, protobuf_tz->mutable_dst_start());
  fill_dst(&palm_tz->dstEnd, protobuf_tz->mutable_dst_end());
  protobuf_tz->set_dst_observed(palm_tz->dstObserved);
  protobuf_tz->set_t4(palm_tz->t4);
  protobuf_tz->set_unknown(palm_tz->unknown);
  protobuf_tz->set_name(palm_tz->name);
}

/**
 * Parse the database and dump to the specified protobuf stream.
 */
void write_events(pi_file_t *pf, nmeg::Calendar &proto_calendar) {
  int nentries;
  pi_file_get_entries(pf, &nentries);
  cout << "Number of entries: " << nentries << endl;

  for (int entnum = 0; entnum < nentries; entnum++) {
    unsigned char *buf;
    int attrs, cat;
    size_t size;
    recordid_t uid;
    if (pi_file_read_record(pf, entnum, (void **) &buf, &size, &attrs, &cat,
        &uid) < 0) {
      printf("Error reading record number %d\n", entnum);
      continue;
    }

    nmeg::CalendarEvent *protobuf_event = proto_calendar.add_events();
    protobuf_event->mutable_palm_record()->set_category(cat);

    if (attrs & dlpRecAttrArchived) {
      protobuf_event->mutable_palm_record()->add_attributes(nmeg::ARCHIVED);
    }
    if (attrs & dlpRecAttrBusy) {
      protobuf_event->mutable_palm_record()->add_attributes(nmeg::BUSY);
    }
    if (attrs & dlpRecAttrDeleted) {
      protobuf_event->mutable_palm_record()->add_attributes(nmeg::DELETED);
    }
    if (attrs & dlpRecAttrDirty) {
      protobuf_event->mutable_palm_record()->add_attributes(nmeg::DIRTY);
    }
    if (attrs & dlpRecAttrSecret) {
      protobuf_event->mutable_palm_record()->add_attributes(nmeg::SECRET);
    }

    protobuf_event->mutable_palm_record()->set_uid(uid);

    pi_buffer_t *pi_buf = pi_buffer_new(size);
    pi_buffer_append(pi_buf, buf, size);

    CalendarEvent_t appt;
    const int result = unpack_CalendarEvent(&appt, pi_buf, calendar_v1);
    if (-1 == result) {
      printf("Error unpacking record %d!\n", entnum);
      continue;
    }

    protobuf_event->set_event(appt.event);
    convert_tm_to_protobuf(&appt.begin, protobuf_event->mutable_begin());
    convert_tm_to_protobuf(&appt.end, protobuf_event->mutable_end());
    protobuf_event->set_alarm(appt.alarm != 0);
    if (appt.alarm) {
      protobuf_event->set_advance(appt.advance);
      switch (appt.advanceUnits) {
      case calendar_advMinutes:
        protobuf_event->set_advance_units(nmeg::MINUTES);
        break;
      case calendar_advHours:
        protobuf_event->set_advance_units(nmeg::HOURS);
        break;
      case calendar_advDays:
        protobuf_event->set_advance_units(nmeg::DAYS);
        break;
      default:
        cerr << "Unknown advance units: " << appt.advanceUnits << endl;
        continue;
      }
    }

    switch (appt.repeatType) {
    case calendarRepeatNone:
      protobuf_event->set_repeat_type(nmeg::NONE);
      break;
    case calendarRepeatDaily:
      protobuf_event->set_repeat_type(nmeg::DAILY);
      break;
    case calendarRepeatWeekly:
      protobuf_event->set_repeat_type(nmeg::WEEKLY);
      break;
    case calendarRepeatMonthlyByDay:
      protobuf_event->set_repeat_type(nmeg::MONTHLY_BY_DAY);
      break;
    case calendarRepeatMonthlyByDate:
      protobuf_event->set_repeat_type(nmeg::MONTHLY_BY_DATE);
      break;
    case calendarRepeatYearly:
      protobuf_event->set_repeat_type(nmeg::YEARLY);
      break;
    default:
      cerr << "Unknown repeat type: " << appt.repeatType << endl;
      continue;
    }

    if (appt.repeatType != calendarRepeatNone) {

      protobuf_event->set_repeatforever(appt.repeatForever != 0);

      convert_tm_to_protobuf(&appt.repeatEnd,
          protobuf_event->mutable_repeat_end());
      protobuf_event->set_repeat_frequency(appt.repeatFrequency);
      switch (appt.repeatDay) {
      case calendar_1stSun:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_1ST_SUN);
        break;
      case calendar_1stMon:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_1ST_MON);
        break;
      case calendar_1stTue:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_1ST_TUE);
        break;
      case calendar_1stWen:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_1ST_WEN);
        break;
      case calendar_1stThu:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_1ST_THU);
        break;
      case calendar_1stFri:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_1ST_FRI);
        break;
      case calendar_1stSat:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_1ST_SAT);
        break;
      case calendar_2ndSun:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_2ND_SUN);
        break;
      case calendar_2ndMon:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_2ND_MON);
        break;
      case calendar_2ndTue:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_2ND_TUE);
        break;
      case calendar_2ndWen:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_2ND_WEN);
        break;
      case calendar_2ndThu:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_2ND_THU);
        break;
      case calendar_2ndFri:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_2ND_FRI);
        break;
      case calendar_2ndSat:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_2ND_SAT);
        break;
      case calendar_3rdSun:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_3RD_SUN);
        break;
      case calendar_3rdMon:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_3RD_MON);
        break;
      case calendar_3rdTue:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_3RD_TUE);
        break;
      case calendar_3rdWen:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_3RD_WEN);
        break;
      case calendar_3rdThu:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_3RD_THU);
        break;
      case calendar_3rdFri:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_3RD_FRI);
        break;
      case calendar_3rdSat:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_3RD_SAT);
        break;
      case calendar_4thSun:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_4TH_SUN);
        break;
      case calendar_4thMon:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_4TH_MON);
        break;
      case calendar_4thTue:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_4TH_TUE);
        break;
      case calendar_4thWen:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_4TH_WEN);
        break;
      case calendar_4thThu:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_4TH_THU);
        break;
      case calendar_4thFri:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_4TH_FRI);
        break;
      case calendar_4thSat:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_4TH_SAT);
        break;
      case calendar_LastSun:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_LAST_SUN);
        break;
      case calendar_LastMon:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_LAST_MON);
        break;
      case calendar_LastTue:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_LAST_TUE);
        break;
      case calendar_LastWen:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_LAST_WEN);
        break;
      case calendar_LastThu:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_LAST_THU);
        break;
      case calendar_LastFri:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_LAST_FRI);
        break;
      case calendar_LastSat:
        protobuf_event->set_repeat_day(nmeg::CALENDAR_LAST_SAT);
        break;
      default:
        cerr << "Unknown repeatDay: " << appt.repeatDay << endl;
        continue;
      }

      for (int i = 0; i < 7; ++i) {
        protobuf_event->add_repeat_days(appt.repeatDays[i] != 0);
      }

      protobuf_event->set_repeat_weekstart(appt.repeatWeekstart);
      for (int i = 0; i < appt.exceptions; ++i) {
        convert_tm_to_protobuf(&appt.exception[i],
            protobuf_event->add_exception());
      }
    } // end if repeating event

    if (NULL != appt.description) {
      protobuf_event->set_description(appt.description);
    }
    if (NULL != appt.note) {
      protobuf_event->set_note(appt.note);
    }
    if (NULL != appt.location) {
      protobuf_event->set_location(appt.location);
    }

    for (int i = 0; i < MAX_BLOBS; ++i) {
      if (NULL != appt.blob[i]) {
        fill_blob(appt.blob[i], protobuf_event->add_blob());
      }
    }
    if (NULL != appt.tz) {
      convert_tz(appt.tz, protobuf_event->mutable_tz());
    }

    pi_buffer_free(pi_buf);
    free_CalendarEvent(&appt);
  }
  cout << "End of converting events" << endl;
}

int main(int argc, char **argv) {
  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  pi_file_t *pf;
  struct DBInfo info;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s [.pdb file]\n", *argv);
    return 1;
  }

  if ((pf = pi_file_open(*(argv + 1))) == NULL) {
    perror("pi_file_open");
    return 1;
  }

  pi_file_get_info(pf, &info);

  nmeg::Calendar proto_calendar;
  write_appinfo(pf, proto_calendar);

  write_events(pf, proto_calendar);

  fstream output("palm.pb", ios::out | ios::trunc | ios::binary);
  if (!proto_calendar.SerializeToOstream(&output)) {
    cerr << "Failed to write calendar." << endl;
    return -1;
  }
  output.close();

  pi_file_close(pf);

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  cout << "end" << endl;
  return 0;
}
