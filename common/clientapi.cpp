
#include "clientapi.h"
#include "resourceaccess.h"
#include "commands.h"
#include "log.h"

namespace async
{
    void run(const std::function<void()> &runner) {
        //TODO use a job that runs in a thread?
        QtConcurrent::run(runner);
    };
} // namespace async

namespace Akonadi2
{

namespace ApplicationDomain
{

template<>
QByteArray getTypeName<Event>()
{
    return "event";
}

template<>
QByteArray getTypeName<Todo>()
{
    return "todo";
}

template<>
QByteArray getTypeName<AkonadiResource>()
{
    return "akonadiresource";
}

} // namespace Domain

void Store::shutdown(const QByteArray &identifier)
{
    Trace() << "shutdown";
    ResourceAccess::connectToServer(identifier).then<void, QSharedPointer<QLocalSocket>>([identifier](const QSharedPointer<QLocalSocket> &socket, Async::Future<void> &future) {
        //We can't currently reuse the socket
        socket->close();
        auto resourceAccess = QSharedPointer<Akonadi2::ResourceAccess>::create(identifier);
        resourceAccess->open();
        resourceAccess->sendCommand(Akonadi2::Commands::ShutdownCommand).then<void>([&future, resourceAccess]() {
            future.setFinished();
        }).exec();
    },
    [](int, const QString &) {
        //Resource isn't started, nothing to shutdown
    }).exec().waitForFinished();
}

} // namespace Akonadi2
