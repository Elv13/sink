/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pipeline.h"

#include <QByteArray>
#include <QStandardPaths>
#include <QVector>

namespace Akonadi2
{

class Pipeline::Private
{
public:
    Private(const QString &resourceName)
        : storage(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/akonadi2/storage", resourceName),
          stepScheduled(false)
    {
    }

    Storage storage;
    QVector<PipelineFilter *> nullPipeline;
    QVector<PipelineFilter *> newPipeline;
    QVector<PipelineFilter *> modifiedPipeline;
    QVector<PipelineFilter *> deletedPipeline;
    QVector<PipelineState> activePipelines;
    bool stepScheduled;
};

Pipeline::Pipeline(const QString &resourceName, QObject *parent)
    : QObject(parent),
      d(new Private(resourceName))
{
}

Pipeline::~Pipeline()
{
    delete d;
}

Storage &Pipeline::storage() const
{
    return d->storage;
}

void Pipeline::null()
{
    //TODO: is there really any need for the null pipeline? if so, it should be doing something ;)
    PipelineState state(this, NullPipeline, QByteArray(), d->nullPipeline);
    d->activePipelines << state;
    state.step();
}

void Pipeline::newEntity(const QByteArray &key, flatbuffers::FlatBufferBuilder &entity)
{
    PipelineState state(this, NewPipeline, key, d->newPipeline);
    d->activePipelines << state;
    state.step();
}

void Pipeline::modifiedEntity(const QByteArray &key, flatbuffers::FlatBufferBuilder &entityDelta)
{
    PipelineState state(this, ModifiedPipeline, key, d->modifiedPipeline);
    d->activePipelines << state;
    state.step();
}

void Pipeline::deletedEntity(const QByteArray &key)
{
    PipelineState state(this, DeletedPipeline, key, d->deletedPipeline);
    d->activePipelines << state;
    state.step();
}

void Pipeline::pipelineStepped(const PipelineState &state)
{
    scheduleStep();
}

void Pipeline::scheduleStep()
{
    if (!d->stepScheduled) {
        d->stepScheduled = true;
        QMetaObject::invokeMethod(this, "stepPipelines", Qt::QueuedConnection);
    }
}

void Pipeline::stepPipelines()
{
    for (PipelineState &state: d->activePipelines) {
        if (state.isIdle()) {
            state.step();
        }
    }

    d->stepScheduled = false;
}

void Pipeline::pipelineCompleted(const PipelineState &state)
{
    //TODO finalize the datastore, inform clients of the new rev
    const int index = d->activePipelines.indexOf(state);
    if (index > -1) {
        d->activePipelines.remove(index);
    }

    if (state.type() != NullPipeline) {
        emit revisionUpdated();
    }
    scheduleStep();
}


class PipelineState::Private : public QSharedData
{
public:
    Private(Pipeline *p, Pipeline::Type t, const QByteArray &k, QVector<PipelineFilter *> filters)
        : pipeline(p),
          type(t),
          key(k),
          filterIt(filters),
          idle(true)
    {}

    Private()
        : pipeline(0),
          filterIt(QVector<PipelineFilter *>()),
          idle(true)
    {}

    Pipeline *pipeline;
    Pipeline::Type type;
    QByteArray key;
    QVectorIterator<PipelineFilter *> filterIt;
    bool idle;
};

PipelineState::PipelineState()
    : d(new Private())
{

}

PipelineState::PipelineState(Pipeline *pipeline, Pipeline::Type type, const QByteArray &key, const QVector<PipelineFilter *> &filters)
    : d(new Private(pipeline, type, key, filters))
{
}

PipelineState::PipelineState(const PipelineState &other)
    : d(other.d)
{
}

PipelineState::~PipelineState()
{
}

PipelineState &PipelineState::operator=(const PipelineState &rhs)
{
    d = rhs.d;
    return *this;
}

bool PipelineState::operator==(const PipelineState &rhs)
{
    return d == rhs.d;
}

bool PipelineState::isIdle() const
{
    return d->idle;
}

QByteArray PipelineState::key() const
{
    return d->key;
}

Pipeline::Type PipelineState::type() const
{
    return d->type;
}

void PipelineState::step()
{
    if (!d->pipeline) {
        return;
    }

    d->idle = false;
    if (d->filterIt.hasNext()) {
        d->filterIt.next()->process(*this);
    } else {
        d->pipeline->pipelineCompleted(*this);
    }
}

void PipelineState::processingCompleted(PipelineFilter *filter)
{
    if (d->pipeline && filter == d->filterIt.peekPrevious()) {
        d->idle = true;
        d->pipeline->pipelineStepped(*this);
    }
}

PipelineFilter::PipelineFilter()
    : d(0)
{
}

PipelineFilter::~PipelineFilter()
{
}

void PipelineFilter::process(PipelineState state)
{
    processingCompleted(state);
}

void PipelineFilter::processingCompleted(PipelineState state)
{
    state.processingCompleted(this);
}

} // namespace Akonadi2
