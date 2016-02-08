/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
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

#include "common/genericresource.h"
#include "common/messagequeue.h"

#include <Async/Async>

#include <flatbuffers/flatbuffers.h>

//TODO: a little ugly to have this in two places, once here and once in Q_PLUGIN_METADATA
#define PLUGIN_NAME "org.kde.dummy"

class DummyEventAdaptorFactory;
class DummyMailAdaptorFactory;
class DummyFolderAdaptorFactory;

class DummyResource : public Sink::GenericResource
{
public:
    DummyResource(const QByteArray &instanceIdentifier, const QSharedPointer<Sink::Pipeline> &pipeline = QSharedPointer<Sink::Pipeline>());
    KAsync::Job<void> synchronizeWithSource(Sink::Storage &mainStore, Sink::Storage &synchronizationStore) Q_DECL_OVERRIDE;
    using GenericResource::synchronizeWithSource;
    void removeDataFromDisk() Q_DECL_OVERRIDE;
    static void removeFromDisk(const QByteArray &instanceIdentifier);
    KAsync::Job<void> inspect(int inspectionType, const QByteArray &inspectionId, const QByteArray &domainType, const QByteArray &entityId, const QByteArray &property, const QVariant &expectedValue) Q_DECL_OVERRIDE;
private:
    KAsync::Job<void> replay(Sink::Storage &synchronizationStore, const QByteArray &type, const QByteArray &key, const QByteArray &value) Q_DECL_OVERRIDE;
    Sink::ApplicationDomain::Event::Ptr createEvent(const QByteArray &rid, const QMap<QString, QVariant> &data, Sink::Storage::Transaction &);
    Sink::ApplicationDomain::Mail::Ptr createMail(const QByteArray &rid, const QMap<QString, QVariant> &data, Sink::Storage::Transaction &);
    Sink::ApplicationDomain::Folder::Ptr createFolder(const QByteArray &rid, const QMap<QString, QVariant> &data, Sink::Storage::Transaction &);
    void synchronize(const QByteArray &bufferType, const QMap<QString, QMap<QString, QVariant> > &data, Sink::Storage::Transaction &transaction, Sink::Storage::Transaction &synchronizationTransaction, DomainTypeAdaptorFactoryInterface &adaptorFactory, std::function<Sink::ApplicationDomain::ApplicationDomainType::Ptr(const QByteArray &ridBuffer, const QMap<QString, QVariant> &data, Sink::Storage::Transaction &)> createEntity);

    QSharedPointer<DummyEventAdaptorFactory> mEventAdaptorFactory;
    QSharedPointer<DummyMailAdaptorFactory> mMailAdaptorFactory;
    QSharedPointer<DummyFolderAdaptorFactory> mFolderAdaptorFactory;
};

class DummyResourceFactory : public Sink::ResourceFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.dummy")
    Q_INTERFACES(Sink::ResourceFactory)

public:
    DummyResourceFactory(QObject *parent = 0);

    Sink::Resource *createResource(const QByteArray &instanceIdentifier) Q_DECL_OVERRIDE;
    void registerFacades(Sink::FacadeFactory &factory) Q_DECL_OVERRIDE;
};

