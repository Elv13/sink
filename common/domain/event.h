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

#include "applicationdomaintype.h"

class QByteArray;

template<typename T>
class ReadPropertyMapper;
template<typename T>
class WritePropertyMapper;
class IndexPropertyMapper;

class TypeIndex;

namespace Sink {
namespace ApplicationDomain {
    namespace Buffer {
        struct Event;
        struct EventBuilder;
    }

/**
 * Implements all type-specific code such as updating and querying indexes.
 * 
 * These are type specifiy default implementations. Theoretically a resource could implement it's own implementation.
 */
template<>
class TypeImplementation<Sink::ApplicationDomain::Event> {
public:
    typedef Sink::ApplicationDomain::Buffer::Event Buffer;
    typedef Sink::ApplicationDomain::Buffer::EventBuilder BufferBuilder;
    static void configure(TypeIndex &);
    static void configure(ReadPropertyMapper<Buffer> &);
    static void configure(WritePropertyMapper<BufferBuilder> &);
    static void configure(IndexPropertyMapper &indexPropertyMapper);
};

}
}
