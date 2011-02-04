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

#include "ContactsHeader.hpp"

#include <iostream>
#include <string>
#include <cstring>

using namespace std;

static const string ADDR_WORK_LABEL = "Addr(W)";
static const string ADDR_HOME_LABEL = "Addr(H)";
static const string ADDR_OTHER_LABEL = "Addr(O)";

ContactsHeader::ContactsHeader(struct ContactAppInfo *appinfo) : mAppinfo(appinfo) , mSpouseIdx(-1), mCellProviderIdx(-1) {
  cout << "labels:" << endl;
  for(int labelIdx=0; labelIdx<53; ++labelIdx) {
    cout <<  "\t" << appinfo->labels[labelIdx] << endl;
  }

  for(int labelIdx=0; labelIdx<9; ++labelIdx) {
    if(0 == strncmp("Spouse", appinfo->customLabels[labelIdx], strlen("Spouse"))) {
      mSpouseIdx = labelIdx;
    } else if(0 == strncmp("Cell Provider", appinfo->customLabels[labelIdx], strlen("Cell Provider"))) {
      mCellProviderIdx = labelIdx;
    }
  }
          
  cout << "phone labels:" << endl;
  for(int labelIdx=0; labelIdx<8; ++labelIdx) {
    cout <<  "\t" << appinfo->phoneLabels[labelIdx] << endl;
  }
  cout << "address labels:" << endl;
  for(int labelIdx=0; labelIdx<3; ++labelIdx) {
    cout <<  "\t" << appinfo->addrLabels[labelIdx] << endl;
  }
  cout << "im labels:" << endl;
  for(int labelIdx=0; labelIdx<5; ++labelIdx) {
    cout <<  "\t" << appinfo->IMLabels[labelIdx] << endl;
  }

}

ContactsHeader::~ContactsHeader() {
  
}

const char *ContactsHeader::getSpouse(struct Contact *contact) const {
  if(-1 == mSpouseIdx) {
    return NULL;
  } else if(NULL != contact->entry[contCustom1+mSpouseIdx]) {
    return contact->entry[contCustom1+mSpouseIdx];
  } else {
    return NULL;
  }
}

bool ContactsHeader::isEmailType(const char *palmPhoneType) {
  return 0 == strncmp("E-mail", palmPhoneType, strlen("E-mail"));
}

const char *ContactsHeader::getGoogleTypeForPhoneType(const char *palmPhoneType) {
  if(0 == strncmp("Work", palmPhoneType, strlen("Work"))) {
    return "Work";
  } else if(0 == strncmp("Home", palmPhoneType, strlen("Home"))) {
    return "Home";
  } else if(0 == strncmp("Fax", palmPhoneType, strlen("Fax"))) {
    return "Work Fax";
  } else if(0 == strncmp("Other", palmPhoneType, strlen("Other"))) {
    return "Other";
  } else if(0 == strncmp("E-mail", palmPhoneType, strlen("E-mail"))) {
    throw "Should not see email type here";
  } else if(0 == strncmp("Main", palmPhoneType, strlen("Main"))) {
    return "Main";
  } else if(0 == strncmp("Pager", palmPhoneType, strlen("Pager"))) {
    return "Pager";
  } else if(0 == strncmp("Mobile", palmPhoneType, strlen("Mobile"))) {
    return "Mobile";
  } else {
    throw "Unknown type";
  }
}

const char *ContactsHeader::getGoogleTypeForAddrType(const char *palmAddType) {
  if(0 == strncmp("Addr(W)", palmAddType, strlen("Addr(W)"))) {
    return "Work";
  } else if(0 == strncmp("Addr(H)", palmAddType, strlen("Addr(H)"))) {
    return "Home";
  } else if(0 == strncmp("Addr(O)", palmAddType, strlen("Addr(O)"))) {
    return "Other";
  } else {
    throw "Unknown type";
  }
}
