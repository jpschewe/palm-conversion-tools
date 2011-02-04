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

#ifndef CONTACTSHEADER_HPP_
#define CONTACTSHEADER_HPP_

#include <cstdio> // needed for pi-contact
#include <pi-contact.h>

/**
 * Stores information about the ContactAppInfo struct for later processing.
 */
class ContactsHeader {
public:
	ContactsHeader(struct ContactAppInfo *appinfo);
	virtual ~ContactsHeader();
	/**
	 * Get the cell provider for this contact.
	 *
	 * @param the contact to check
	 * @return the cell provider or NULL
	 */
	const char *getCellProvider(struct Contact *contact) const;

	/**
	 * Get the spouse for this contact.
	 *
	 * @param the contact to check
	 * @return the spouse or NULL
	 */
	const char *getSpouse(struct Contact *contact) const;

	const char *getCategoryName(int categoryIdx) const;

	static const char *getGoogleTypeForPhoneType(const char *palmPhoneType);

	static const char *getGoogleTypeForAddrType(const char *palmAddType);

	bool isEmail(int phoneLabelIdx);

private:
	struct ContactAppInfo *mAppinfo;
	int mSpouseIdx;
	int mCellProviderIdx;

};

#endif /* CONTACTSHEADER_HPP_ */
