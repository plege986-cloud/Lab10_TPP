#include "Repository.h"
#include "Patterns.h"
#include <cmath>

class EventService
{
private:
    StateRepository& repo;
    Publisher& pub;

public:
    EventService(StateRepository& r, Publisher& p)
        : repo(r), pub(p)
    {
    }

    int receive(LogEntry e)
    {
        int id = repo.create(e);
        pub.notify({ id, "Received" });
        return id;
    }

    bool transition(int id, const std::string& next)
    {
        auto state = stateFor(repo.get(id).status);

        if (!state->allows(next))
        {
            return false;
        }

        repo.change(id, next);

        if (next == "Processed")
        {
            pub.notify({ id, "Processed" });
        }

        return true;
    }
};

int main(int argc, char**)
{
    try
    {
        // Initialize the database and repositories.
        auto db = createStorage(argc > 1 ? ":memory:" : "lab10_events.db");
        db.sync_schema();

        EventSourceRepository sources(db);
        StateRepository states(db);
        Publisher publisher;
        EventService service(states, publisher);

        // Register independent observers.
        auto audit = std::make_shared<Audit>();
        auto stats = std::make_shared<Statistics>();
        publisher.subscribe(audit);
        publisher.subscribe(stats);

        int source = sources.add({ 0, "Monitoring", "system", "active" });

        auto entry = [&]
            {
                return LogEntry{ 0, source, "2026-11-25 10:00", "INFO", "Demo event", 25 };
            };

        int n = 0;
        auto check = [&](bool ok, const char* label)
            {
                if (!ok)
                {
                    throw std::runtime_error(label);
                }

                ++n;
                std::cout << "PASS: " << label << '\n';
            };

        // State: valid and invalid transitions.
        int id = service.receive(entry());
        check(states.get(id).status == "New", "Initial state");
        check(!service.transition(id, "Processed"), "Invalid transition");
        check(service.transition(id, "Verified"), "Verify");

        // Verify atomic rollback of state and history.
        int before = states.historyCount();
        states.change(id, "Processed", true);
        check(states.get(id).status == "Verified" &&
            states.historyCount() == before, "Transaction rollback");

        check(service.transition(id, "Processed"), "Process");
        check(states.historyCount() == before + 1, "History saved");
        check(!service.transition(id, "New"), "Terminal state");

        // Observer: notification and unsubscription.
        check(stats->received == 1 && stats->processed == 1 &&
            audit->count == 2, "Two observers");

        publisher.unsubscribe(stats);
        int other = service.receive(entry());
        check(stats->received == 1 && audit->count == 3, "Unsubscribe");
        check(service.transition(other, "Rejected"), "Reject");

        // Strategy and Factory Method: switch priority algorithms.
        Priority p;
        Inputs x{ 8, 6, 30 };
        Factory<Maximum> a;
        Factory<Weighted> b;
        Factory<Normalized> c;

        p.set(a);
        check(p.calculate(x) == 30, "Maximum");

        p.set(b);
        check(std::abs(p.calculate(x) - 11.8) < 1e-9, "Weighted");

        p.set(c);
        check(std::abs(p.calculate(x) - .68) < 1e-9, "Normalized");

        std::cout << "All " << n << " checks passed\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}