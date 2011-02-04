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

#include "ContactsHeader.hpp"

using namespace std;

void output_header(ofstream *output) {
	*output << "\"Given Name\"" //
			<< "\t" << "\"Family Name\"" //
			<< "\t" << "\"Birthday\"" //
			<< "\t" << "\"Notes\"" //
			<< "\t" << "\"Group Membership\"" //
			<< "\t" << "\"E-mail 1 - Type\"" //
			<< "\t" << "\"E-mail 1 - Value\"" //
			<< "\t" << "\"E-mail 2 - Type\"" //
			<< "\t" << "\"E-mail 2 - Value\"" //
			<< "\t" << "\"E-mail 3 - Type\"" //
			<< "\t" << "\"E-mail 3 - Value\"" //
			<< "\t" << "\"E-mail 4 - Type\"" //
			<< "\t" << "\"E-mail 4 - Value\"" //
			<< "\t" << "\"E-mail 5 - Type\"" //
			<< "\t" << "\"E-mail 5 - Value\"" //
			<< "\t" << "\"E-mail 6 - Type\"" //
			<< "\t" << "\"E-mail 6 - Value\"" //
			<< "\t" << "\"E-mail 7 - Type\"" //
			<< "\t" << "\"E-mail 7 - Value\"" //
			<< "\t" << "\"Phone 1 - Type\"" //
			<< "\t" << "\"Phone 1 - Value\"" //
			<< "\t" << "\"Phone 2 - Type\"" //
			<< "\t" << "\"Phone 2 - Value\"" //
			<< "\t" << "\"Phone 3 - Type\"" //
			<< "\t" << "\"Phone 3 - Value\"" //
			<< "\t" << "\"Phone 4 - Type\"" //
			<< "\t" << "\"Phone 4 - Value\"" //
			<< "\t" << "\"Phone 5 - Type\"" //
			<< "\t" << "\"Phone 5 - Value\"" //
			<< "\t" << "\"Phone 6 - Type\"" //
			<< "\t" << "\"Phone 6 - Value\"" //
			<< "\t" << "\"Phone 7 - Type\"" //
			<< "\t" << "\"Phone 7 - Value\"" //
			<< "\t" << "\"Address 1 - Type\"" //
			<< "\t" << "\"Address 1 - Formatted\"" //
			<< "\t" << "\"Address 2 - Type\"" //
			<< "\t" << "\"Address 2 - Formatted\"" //
			<< "\t" << "\"Address 3 – Type\"" //
			<< "\t" << "\"Address 3 - Formatted\"" //
			<< "\t" << "\"Event 1 - Type\"" //
			<< "\t" << "\"Event 1 - Value\"" //
			<< "\t" << "\"Organization 1 - Name\"" //
			<< "\t" << "\"Organization 1 - Title\"" //
			<< "\t" << "\"Relation 1 - Type\"" //
			<< "\t" << "\"Relation 1 - Value\"" //
			<< "\t" << "\"Website 1 - Type\"" //
			<< "\t" << "\"Website 1 - Value\"" //
			<< "\t" << "\"IM 1 - Type\"" //
			<< "\t" << "\"IM 1 - Service\"" //
			<< "\t" << "\"IM 1 - Value\"" //
			<< "\t" << "\"IM 2 - Type\"" //
			<< "\t" << "\"IM 2 - Service\"" //
			<< "\t" << "\"IM 2 - Value\"" //
			<< endl;
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

void output_contact(ofstream *output, ContactsHeader *header,
		struct Contact *contact, int categoryIdx) {
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

#if 0
	cout << "phone labels:" << endl;
	for(int labelIdx=0; labelIdx<7; ++labelIdx) {
		cout << "\t" << contact->phoneLabel[labelIdx] << endl;
	}
	cout << "address labels:" << endl;
	for(int labelIdx=0; labelIdx<3; ++labelIdx) {
		cout << "\t" << contact->addressLabel[labelIdx] << endl;
	}
	cout << "IM labels:" << endl;
	for(int labelIdx=0; labelIdx<2; ++labelIdx) {
		cout << "\t" << contact->IMLabel[labelIdx] << endl;
	}
#endif

	if (NULL != contact->entry[contFirstname]) {
		*output << "\"" << contact->entry[contFirstname] << "\"";
	} else {
		*output << "\"" << "\"";
	}

	if (NULL != contact->entry[contLastname]) {
		*output << "\t" << "\"" << contact->entry[contLastname] << "\"";
	} else {
		*output << "\t" << "\"\"";
	}

	//FIXME need to format struct tm into reasonable date
	if (contact->birthdayFlag) {
		*output << "\t" << "\"Birthday\"";
	} else {
		*output << "\t" << "";
	}

	std::string notes;
	if (NULL != contact->entry[contNote]) {
		notes = notes + contact->entry[contNote];
	}
	const char* cellProvider = header->getCellProvider(contact);
	if (NULL != cellProvider) {
		notes = notes + "\n" + "Cell Provider: " + cellProvider;
	}

	*output << "\t" << "\"" << notes << "\"";

	*output << "\t" << "\"" << header->getCategoryName(categoryIdx) << "\"";

	// email addresses
	int emailIdx = 1;
	for (int idx = 0; idx < 7; ++idx) {
		const char *email = contact->entry[contPhone1 + idx];
		if (NULL != email && header->isEmail(contact->phoneLabel[idx])) {
			*output << "\t" << "\"" << guessEmailType(email) << "\"";
			*output << "\t" << "\"" << email << "\"";
			++emailIdx;
		}
	}
	// output the extra blanks
	while (emailIdx < 8) {
		*output << "\t" << "\"\"" << "\t" << "\"\"";
		++emailIdx;
	}


	// phone numbers
	int phoneIdx = 1;
	for (int idx = 0; idx < 7; ++idx) {
		const char *phone = contact->entry[contPhone1 + idx];
		if (NULL != phone && !header->isEmail(contact->phoneLabel[idx])) {
			*output << "\t" << "\"" << header->getGoogleTypeForPhoneType(contact->phoneLabel[idx]) << "\"";
			*output << "\t" << "\"" << phone << "\"";
			++phoneIdx;
		}
	}
	// output the extra blanks
	while (phoneIdx < 8) {
		*output << "\t" << "\"\"" << "\t" << "\"\"";
		++phoneIdx;
	}


	// FIXME addresses
	*output << "\t" << "\"Address 1 - Type\"" //
			<< "\t" << "\"Address 1 - Formatted\"" //
			<< "\t" << "\"Address 2 - Type\"" //
			<< "\t" << "\"Address 2 - Formatted\"" //
			<< "\t" << "\"Address 3 – Type\"" //
			<< "\t" << "\"Address 3 - Formatted\"";

	//FIXME need to decode anniversary blob - see python code
	*output << "\t" << "\"Event 1 - Type\"" //
			<< "\t" << "\"Event 1 - Value\"";

	if (NULL != contact->entry[contCompany]) {
		*output << "\t" << "\"" << contact->entry[contCompany] << "\"";
	} else {
		*output << "\t" << "\"\"";
	}

	if (NULL != contact->entry[contTitle]) {
		*output << "\t" << "\"" << contact->entry[contTitle] << "\"";
	} else {
		*output << "\t" << "\"\"";
	}

	// spouse
	const char *spouse = header->getSpouse(contact);
	if (NULL != spouse) {
		*output << "\t" << "\"Spouse\"" //
				<< "\t" << "\"" << spouse << "\"";
	} else {
		*output << "\t" << "\"\"" //
				<< "\t" << "\"\"";
	}

	if (NULL != contact->entry[contWebsite]) {
		*output << "\t" << "\"Home Page\"" //
				<< "\t" << "\"" << contact->entry[contWebsite] << "\"" //
				<< endl;
	} else {
		*output << "\t" << "" //
				<< "\t" << "" //
				<< endl;
	}

	//FIXME handle IMs
	*output << "\t" << "\"IM 1 - Type\"" //
			<< "\t" << "\"IM 1 - Service\"" //
			<< "\t" << "\"IM 1 - Value\"" //
			<< "\t" << "\"IM 2 - Type\"" //
			<< "\t" << "\"IM 2 - Service\"" //
			<< "\t" << "\"IM 2 - Value\"";

}

int main(int argc, char **argv) {
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
}
