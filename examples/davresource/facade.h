/*
 * Copyright (C) 2014 Christian Mollekopf <chrigi_1@fastmail.fm>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 */

#pragma once

#include "common/facade.h"

class DavResourceContactFacade : public Sink::GenericFacade<Sink::ApplicationDomain::Contact>
{
public:
    DavResourceContactFacade(const Sink::ResourceContext &context);
    virtual ~DavResourceContactFacade();
};

class DavResourceAddressbookFacade : public Sink::GenericFacade<Sink::ApplicationDomain::Addressbook>
{
public:
    DavResourceAddressbookFacade(const Sink::ResourceContext &context);
    virtual ~DavResourceAddressbookFacade();
};
