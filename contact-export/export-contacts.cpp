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

#include <cstdio> // needed for pi-*
#include <pi-file.h>
#include <pi-macros.h>
#include <pi-contact.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <exception>

#include "ContactsHeader.hpp"

using namespace std;

void output_header(ofstream *output) {
	*output << "\"Name\"";
	*output << "," << "\"Given Name\"" //
			<< "," << "\"Family Name\"" //
			<< "," << "\"Birthday\"" //
			<< "," << "\"Notes\"" //
			<< "," << "\"Group Membership\"";

	for (int i = 1; i < 8; ++i) {
		*output << "," << "\"E-mail " << i << " - Type\"" //
				<< "," << "\"E-mail " << i << " - Value\"";
	}

	for (int i = 1; i < 8; ++i) {
		*output << "," << "\"Phone " << i << " - Type\"" //
				<< "," << "\"Phone " << i << " - Value\"";
	}

	for (int i = 1; i < 4; ++i) {
		*output << "," << "\"Address " << i << " - Formatted\"" //
				<< "," << "\"Address " << i << " - Type\"";
	}

	*output << "," << "\"Event 1 - Type\"" //
			<< "," << "\"Event 1 - Value\"" //
			<< "," << "\"Organization 1 - Name\"" //
			<< "," << "\"Organization 1 - Title\"" //
			<< "," << "\"Relation 1 - Type\"" //
			<< "," << "\"Relation 1 - Value\"" //
			<< "," << "\"Website 1 - Type\"" //
			<< "," << "\"Website 1 - Value\"";

	for (int i = 1; i < 3; ++i) {
		*output << "," << "\"IM " << i << " - Type\"" //
				<< "," << "\"IM " << i << " - Service\""//
				<< "," << "\"IM " << i << " - Value\"";
	}

	// extra fields so that Google pulls all extended attributes
	*output
			<< ",\"Yomi Name\",\"Given Name Yomi\",\"Additional Name Yomi\",\"Family Name Yomi\",\"Name Prefix\",\"Name Suffix\",\"Initials\",\"Nickname\",\"Short Name\",\"Maiden Name\",\"Gender\",\"Location\",\"Billing Information\",\"Directory Server\",\"Mileage\",\"Occupation\",\"Hobby\",\"Sensitivity\",\"Priority\",\"Subject\"";

	*output << endl;
}

const char *guessEmailType(const char *email) {
	if (NULL != strcasestr(email, "comcast.net")) {
		return "Home";
	} else if (NULL != strcasestr(email, "honeywell.com")) {
		return "Work";
	} else if (NULL != strcasestr(email, "bbn.com")) {
		return "Work";
	} else if (NULL != strcasestr(email, "visi.com")) {
		return "Home";
	} else if (NULL != strcasestr(email, "gmail.com")) {
		return "Home";
	} else if (NULL != strcasestr(email, "yahoo.com")) {
		return "Home";
	} else if (NULL != strcasestr(email, "aol.com")) {
		return "Home";
	} else if (NULL != strcasestr(email, "hotmail.com")) {
		return "Home";
	} else if (NULL != strcasestr(email, "netscape.net")) {
		return "Home";
	} else if (NULL != strcasestr(email, "e-mol.com")) {
		return "Home";
	} else if (NULL != strcasestr(email, "earthlink.net")) {
		return "Home";
	} else if (NULL != strcasestr(email, "msn.com")) {
		return "Home";
	} else if (NULL != strcasestr(email, "sift.info")) {
		return "Work";
	} else if (NULL != strcasestr(email, "siftech.com")) {
		return "Work";
	} else if (NULL != strcasestr(email, "ibm.com")) {
		return "Work";
	} else if (NULL != strcasestr(email, "shoutlife.com")) {
		return "Home";
	} else if (NULL != strcasestr(email, ".co.us")) {
		return "work";
	} else if (NULL != strcasestr(email, ".net")) {
		return "Home";
	} else {
		return "Other";
	}
}

const char *BLOB_TYPE_ANNIVERSARY_ID = "Bd01";

void output_contact(ofstream *output, ContactsHeader *header,
		struct Contact *contact, int categoryIdx) {
#if 0
	int numEmails = 0;
	int numPhones = 0;
	for (int labelIdx = 0; labelIdx < 7; ++labelIdx) {

		if (contact->entry[contPhone1 + labelIdx] != NULL) {
			if (header->isEmail(contact->phoneLabel[labelIdx])) {
				++numEmails;
			} else {
				++numPhones;
			}
		}
	}
	cout << "Num email addresses: " << numEmails << " num phones: "
	<< numPhones << endl;
#endif

#if 0
	cout << "phone labels:" << endl;
	for(int labelIdx=0; labelIdx<7; ++labelIdx) {
		cout << "," << contact->phoneLabel[labelIdx] << endl;
	}
	cout << "address labels:" << endl;
	for(int labelIdx=0; labelIdx<3; ++labelIdx) {
		cout << "," << contact->addressLabel[labelIdx] << endl;
	}
	cout << "IM labels:" << endl;
	for(int labelIdx=0; labelIdx<2; ++labelIdx) {
		cout << "," << contact->IMLabel[labelIdx] << endl;
	}
#endif

	bool nameStarted = false;
	*output << "\"";
	if (NULL != contact->entry[contFirstname]) {
		*output << contact->entry[contFirstname];
		nameStarted = true;
	}
	if (NULL != contact->entry[contLastname]) {
		if (nameStarted) {
			*output << " ";
		}
		*output << contact->entry[contLastname];
		nameStarted = true;
	}
	if (!nameStarted) {
		// use company name
		if (NULL != contact->entry[contCompany]) {
			*output << contact->entry[contCompany];
			nameStarted = true;
		}
	}
	if (!nameStarted) {
		*output << "No name";
	}
	*output << "\"";

	if (NULL != contact->entry[contFirstname]) {
		*output << "," << "\"" << contact->entry[contFirstname] << "\"";
	} else {
		*output << "," << "\"" << "\"";
	}

	if (NULL != contact->entry[contLastname]) {
		*output << "," << "\"" << contact->entry[contLastname] << "\"";
	} else {
		*output << "," << "\"\"";
	}

	if (contact->birthdayFlag) {
		*output << "," << "\"" << (contact->birthday.tm_year + 1900) << "-"
				<< (contact->birthday.tm_mon + 1) << "-"
				<< contact->birthday.tm_mday << "\"";
	} else {
		*output << "," << "";
	}

	std::string notes;
	if (NULL != contact->entry[contNote]) {
		notes = notes + contact->entry[contNote];
	}
	const char* cellProvider = header->getCellProvider(contact);
	if (NULL != cellProvider) {
		notes = notes + "\n" + "Cell Provider: " + cellProvider;
	}
	for (int i = contCustom1; i <= contCustom9; ++i) {
		if (NULL != contact->entry[i] //
				&& !(-1 != header->getCellProviderIndex()
						&& header->getCellProviderIndex() + contCustom1 == i) //
				&& !(-1 != header->getSpouseIndex() && header->getSpouseIndex()
						+ contCustom1 == i)) {
			notes = notes + "\n" + contact->entry[i];
		}
	}

	*output << "," << "\"" << notes << "\"";

	*output << "," << "\"" << header->getCategoryName(categoryIdx)
			<< " ::: * My Contacts\"";

	// email addresses
	int emailIdx = 1;
	for (int idx = 0; idx < 7; ++idx) {
		const char *email = contact->entry[contPhone1 + idx];
		if (NULL != email && header->isEmail(contact->phoneLabel[idx])) {
			*output << "," << "\"" << guessEmailType(email) << "\"";
			*output << "," << "\"" << email << "\"";
			++emailIdx;
		}
	}
	// output the extra blanks
	while (emailIdx < 8) {
		*output << "," << "\"\"" << "," << "\"\"";
		++emailIdx;
	}

	// phone numbers
	int phoneIdx = 1;
	for (int idx = 0; idx < 7; ++idx) {
		const char *phone = contact->entry[contPhone1 + idx];
		if (NULL != phone && !header->isEmail(contact->phoneLabel[idx])) {
			*output << "," << "\"" << header->getGoogleTypeForPhoneType(
					contact->phoneLabel[idx]) << "\"";
			*output << "," << "\"" << phone << "\"";
			++phoneIdx;
		}
	}
	// output the extra blanks
	while (phoneIdx < 8) {
		*output << "," << "\"\"" << "," << "\"\"";
		++phoneIdx;
	}

	// addresses
	for (int i = 0; i < 3; ++i) {
		bool hasValue = false;
		stringstream stream;

		const char *address = contact->entry[contAddress1 + (i * 5)];
		if (NULL != address) {
			stream << address;
			hasValue = true;
		}

		const char *city = contact->entry[contCity1 + (i * 5)];
		if (NULL != city) {
			if (hasValue) {
				stream << endl;
			}
			stream << city;
			hasValue = true;
		}

		const char *state = contact->entry[contState1 + (i * 5)];
		if (NULL != state) {
			if (NULL != city) {
				stream << ", ";
			}
			stream << state;
			hasValue = true;
		}

		const char *zip = contact->entry[contZip1 + (i * 5)];
		if (NULL != zip) {
			if (NULL != state) {
				stream << " ";
			}
			stream << zip;
			hasValue = true;
		}

		const char *country = contact->entry[contCountry1 + (i * 5)];
		if (NULL != country) {
			if (hasValue) {
				stream << endl;
			}
			stream << country;
			hasValue = true;
		}

		if (hasValue) {
			*output << ",\"" << stream.str() << "\"";
			*output << "," << "\"" << header->getGoogleTypeForAddrType(
					contact->addressLabel[i]) << "\"";
		} else {
			*output << ",\"\",\"\"";
		}
	}

	// this anniversary code has been submitted to pilot-link, but not merged in
	bool foundAnniversary = false;
	for (int i = 0; i < MAX_CONTACT_BLOBS && !foundAnniversary; ++i) {
		if (NULL != contact->blob[i] && 0 == strncmp(contact->blob[i]->type,
				BLOB_TYPE_ANNIVERSARY_ID, 4)) {
			unsigned short int data = (unsigned short int) get_short(
					contact->blob[i]->data);
			int year = (data >> 9) + 4 + 1900;
			int month = ((data >> 5) & 15);
			int day = data & 31;

			// ignore reminder as it doesn't map to google
			*output << "," << "\"Anniversary\"";
			*output << "," << "\"" << year << "-" << month << "-" << day
					<< "\"";
			foundAnniversary = true;
		}
	}
	if (!foundAnniversary) {
		*output << ",\"\",\"\"";
	}

	if (NULL != contact->entry[contCompany]) {
		*output << "," << "\"" << contact->entry[contCompany] << "\"";
	} else {
		*output << "," << "\"\"";
	}

	if (NULL != contact->entry[contTitle]) {
		*output << "," << "\"" << contact->entry[contTitle] << "\"";
	} else {
		*output << "," << "\"\"";
	}

	// spouse
	const char *spouse = header->getSpouse(contact);
	if (NULL != spouse) {
		*output << "," << "\"Spouse\"" //
				<< "," << "\"" << spouse << "\"";
	} else {
		*output << "," << "\"\"" //
				<< "," << "\"\"";
	}

	if (NULL != contact->entry[contWebsite]) {
		*output << "," << "\"Home Page\"" //
				<< "," << "\"" << contact->entry[contWebsite] << "\"";
	} else {
		*output << "," << "" //
				<< "," << "";
	}

	// IMs
	for (int i = 0; i < 2; ++i) {
		const char *im = contact->entry[contIM1 + i];
		if (NULL != im) {
			//TODO actually decode this rather than just using jabber
			*output << "," << "\"Other\"" //
					<< "," << "\"Jabber\""//
					<< "," << "\"" << im << "\"";
		} else {
			*output << "," << "\"\"" //
					<< "," << "\"\""//
					<< "," << "\"\"";
		}
	}

	// extended fields to keep Google happy
	*output << ",,,,,,,,,,,,,,,,,,,,";

	*output << endl;

}

int main(int argc, char **argv) {
	try {
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

		struct ContactAppInfo cai;
		const int result = unpack_ContactAppInfo(&cai, pi_buf);
		if (-1 == result) {
			cerr << "Error unpacking contact app info" << endl;
			return 1;
		}
		ContactsHeader header(&cai);

		int nentries;
		pi_file_get_entries(pf, &nentries);
		cout << "Number of entries: " << nentries << endl;

		ofstream output("google-contacts.csv", ios::trunc);
		output_header(&output);
		for (int entnum = 0; entnum < nentries; entnum++) {
			unsigned char *buf;
			int attrs, cat;
			size_t size;
			recordid_t uid;
			if (pi_file_read_record(pf, entnum, (void **) &buf, &size, &attrs,
					&cat, &uid) < 0) {
				printf("Error reading record number %d\n", entnum);
				continue;
			}

			pi_buffer_t *pi_buf = pi_buffer_new(size);
			pi_buffer_append(pi_buf, buf, size);

			struct Contact contact;
			const int result = unpack_Contact(&contact, pi_buf, cai.type);
			if (-1 == result) {
				printf("Error unpacking record %d!\n", entnum);
				continue;
			}

			output_contact(&output, &header, &contact, cat);

		}
		output.close();

		pi_file_close(pf);

		return 0;
	} catch (exception &e) {
		cerr << "Unexpected error: " << e.what() << endl;
		return 1;
	}
}
